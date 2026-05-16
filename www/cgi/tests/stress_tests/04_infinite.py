#!/usr/bin/env python3
"""
STRESS TEST 04 - Indefinite Stability Test
============================================
Requirement: The server must be usable indefinitely without needing a restart.
             (Equivalent to: siege -b running without a time limit)

HOW IT WORKS:
  Fires concurrent GET requests for DURATION seconds.
  Every CHECK_EVERY seconds it prints a status line that verifies:
    - Availability is still >= 99.5%
    - The server PID has not changed (server did not crash and restart)

Usage: python3 04_infinite.py [duration_seconds]
       python3 04_infinite.py 60     <- 1 minute
       python3 04_infinite.py 300    <- 5 minutes
       python3 04_infinite.py        <- 60s default
"""

import sys
import time
import threading
import http.client
import subprocess

DURATION    = int(sys.argv[1]) if len(sys.argv) > 1 else 60
URL_HOST    = "127.0.0.1"
URL_PORT    = 8080
URL_PATH    = "/empty.html"
CONCURRENCY = 15
CHECK_EVERY = 10   # seconds between status reports

lock       = threading.Lock()
total_ok   = 0
total_fail = 0
stop_flag  = False


def get_webserv_pid():
    """Return the PID of the running webserv process, or None."""
    try:
        result = subprocess.run(["pgrep", "-x", "webserv"], capture_output=True, text=True)
        pids = result.stdout.strip().split()
        return int(pids[0]) if pids else None
    except Exception:
        return None


def worker():
    global total_ok, total_fail
    while not stop_flag:
        try:
            conn = http.client.HTTPConnection(URL_HOST, URL_PORT, timeout=3)
            conn.request("GET", URL_PATH)
            resp = conn.getresponse()
            resp.read()
            with lock:
                if 200 <= resp.status < 400:
                    total_ok += 1
                else:
                    total_fail += 1
            conn.close()
        except Exception:
            with lock:
                total_fail += 1


def main():
    global stop_flag

    initial_pid = get_webserv_pid()

    print("=" * 60)
    print("  STRESS TEST 04 - Indefinite Stability Test")
    print("       (equivalent to: siege -b with no time limit)")
    print("=" * 60)
    print(f"  Duration    : {DURATION}s")
    print(f"  Concurrency : {CONCURRENCY} threads")
    print(f"  Initial PID : {initial_pid or 'Not found'}")
    print("-" * 60)

    if initial_pid is None:
        print("  ERROR: webserv is not running. Start the server first.")
        sys.exit(1)

    threads = [threading.Thread(target=worker, daemon=True) for _ in range(CONCURRENCY)]
    for t in threads:
        t.start()

    t_start    = time.time()
    next_check = CHECK_EVERY
    all_ok     = True

    print(f"  {'Time':>6}  {'OK':>8}  {'FAIL':>6}  {'Avail%':>8}  {'PID':>8}  Status")
    print(f"  {'-'*6}  {'-'*8}  {'-'*6}  {'-'*8}  {'-'*8}  {'-'*6}")

    while True:
        elapsed = time.time() - t_start
        if elapsed >= DURATION:
            break

        # Sleep until the next checkpoint or until duration ends
        sleep_for = min(next_check - elapsed, DURATION - elapsed)
        if sleep_for > 0:
            time.sleep(sleep_for)

        elapsed = time.time() - t_start

        with lock:
            ok   = total_ok
            fail = total_fail

        current_pid = get_webserv_pid()
        total_req   = ok + fail
        avail       = (ok / total_req * 100) if total_req > 0 else 0
        pid_ok      = (current_pid == initial_pid)
        avail_ok    = avail >= 99.5
        status      = "OK" if (avail_ok and pid_ok) else "FAIL"

        if not (avail_ok and pid_ok):
            all_ok = False

        print(f"  {elapsed:>5.0f}s  {ok:>8}  {fail:>6}  {avail:>7.2f}%  {current_pid or '???':>8}  {status}")
        next_check += CHECK_EVERY

    stop_flag = True

    with lock:
        ok   = total_ok
        fail = total_fail

    total_req     = ok + fail
    avail         = (ok / total_req * 100) if total_req > 0 else 0
    final_pid     = get_webserv_pid()
    elapsed_total = time.time() - t_start

    print("-" * 60)
    print("  FINAL RESULTS")
    print("-" * 60)
    print(f"  Total duration       : {elapsed_total:.1f}s")
    print(f"  Total requests       : {total_req}")
    print(f"  Successful           : {ok}")
    print(f"  Failed               : {fail}")
    print(f"  Availability         : {avail:.2f}%")
    print(f"  Initial PID          : {initial_pid}")
    print(f"  Final PID            : {final_pid}")
    print(f"  Server restarted?    : {'YES - FAIL' if final_pid != initial_pid else 'NO - OK'}")
    print("-" * 60)

    if all_ok and avail >= 99.5 and final_pid == initial_pid:
        print("  PASS - Server sustained indefinite load without crashing or restarting.")
    else:
        if avail < 99.5:
            print(f"  FAIL - Availability {avail:.2f}% < 99.5%")
        if final_pid != initial_pid:
            print("  FAIL - Server PID changed (server restarted)")
        if not all_ok:
            print("  FAIL - One or more checkpoints failed (see table above)")
    print("=" * 60)


if __name__ == "__main__":
    main()
