#!/usr/bin/env python3
"""
STRESS TEST 03 - Loose Connections Check
==========================================
Requirement: No sockets must be left in CLOSE_WAIT after a burst of requests.

HOW IT WORKS:
  1. Record connection state BEFORE the burst.
  2. Fire a burst of concurrent requests.
  3. Wait a few seconds for the server to clean up.
  4. Record connection state AFTER and check for CLOSE_WAIT sockets.

Usage: python3 03_connections.py [port]
       python3 03_connections.py 8080
"""

import sys
import subprocess
import time
import threading
import http.client

PORT        = int(sys.argv[1]) if len(sys.argv) > 1 else 8080
URL_HOST    = "127.0.0.1"
URL_PATH    = "/empty.html"
BURST       = 200
CONCURRENCY = 20


def get_connections(port):
    """Return a dict counting TCP socket states for the given port."""
    states = {}
    # Try 'ss' first, fall back to 'netstat'
    for cmd, filter_col in [(["ss", "-tn"], 0), (["netstat", "-tn"], 5)]:
        try:
            result = subprocess.run(cmd, capture_output=True, text=True)
            if result.returncode != 0:
                continue
            for line in result.stdout.strip().split("\n"):
                if str(port) not in line:
                    continue
                parts = line.split()
                if not parts or len(parts) <= filter_col:
                    continue
                state = parts[filter_col]
                # 'ss' output starts with state; skip header lines
                if state in ("State", "Recv-Q", "tcp", "tcp6"):
                    continue
                states[state] = states.get(state, 0) + 1
            break
        except FileNotFoundError:
            continue
    return states


def burst_worker(results):
    """Fire BURST / CONCURRENCY requests and record success/failure."""
    ok = fail = 0
    for _ in range(BURST // CONCURRENCY):
        try:
            conn = http.client.HTTPConnection(URL_HOST, PORT, timeout=3)
            conn.request("GET", URL_PATH)
            conn.getresponse().read()
            conn.close()
            ok += 1
        except Exception:
            fail += 1
    results.append((ok, fail))


def main():
    print("=" * 60)
    print("  STRESS TEST 03 - Loose Connections Check")
    print("=" * 60)
    print(f"  Port        : {PORT}")
    print(f"  Burst       : {BURST} requests ({CONCURRENCY} concurrent)")
    print("-" * 60)

    # State BEFORE
    before = get_connections(PORT)
    print("  Socket state BEFORE burst:")
    if before:
        for state, count in sorted(before.items()):
            print(f"    {state:<20}: {count}")
    else:
        print("    (no active connections - OK)")

    print()
    print("  Launching burst...")
    results = []
    threads = [threading.Thread(target=burst_worker, args=(results,)) for _ in range(CONCURRENCY)]
    for t in threads:
        t.start()
    for t in threads:
        t.join()

    ok_total   = sum(r[0] for r in results)
    fail_total = sum(r[1] for r in results)
    print(f"  Burst done: {ok_total} OK  /  {fail_total} FAIL")

    # Wait for server to process connection closures
    wait = 5
    print(f"\n  Waiting {wait}s for server to clean up connections...")
    time.sleep(wait)

    # State AFTER
    after = get_connections(PORT)
    print("  Socket state AFTER burst:")
    if after:
        for state, count in sorted(after.items()):
            print(f"    {state:<20}: {count}")
    else:
        print("    (no active connections - OK)")

    close_wait = after.get("CLOSE_WAIT", 0)
    print("-" * 60)
    print("  RESULTS")
    print("-" * 60)
    print(f"  CLOSE_WAIT after load  : {close_wait}")

    if close_wait == 0:
        print("  PASS - No loose connections (CLOSE_WAIT = 0)")
    else:
        print(f"  WARNING - {close_wait} socket(s) stuck in CLOSE_WAIT.")
        print("            Wait 30s more (server timeout) and check again.")
    print("=" * 60)


if __name__ == "__main__":
    main()
