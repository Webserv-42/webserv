#!/usr/bin/env python3
"""
STRESS TEST 01 - Availability Test
====================================
Replaces: siege -b -t 15s http://127.0.0.1:8080/empty.html
Requirement: Availability must be >= 99.5%

Usage: python3 01_availability.py [url] [duration_seconds] [concurrency]
       python3 01_availability.py http://127.0.0.1:8080/empty.html 15 25
"""

import sys
import time
import threading
import http.client
from urllib.parse import urlparse

# --- Default configuration ---------------------------------------------------
URL         = sys.argv[1] if len(sys.argv) > 1 else "http://127.0.0.1:8080/empty.html"
DURATION    = int(sys.argv[2]) if len(sys.argv) > 2 else 15   # seconds
CONCURRENCY = int(sys.argv[3]) if len(sys.argv) > 3 else 25   # concurrent threads

# --- Shared counters ---------------------------------------------------------
lock           = threading.Lock()
total_ok       = 0
total_fail     = 0
total_bytes    = 0
response_times = []
stop_flag      = False


def worker(host, port, path, use_ssl):
    global total_ok, total_fail, total_bytes, stop_flag

    while not stop_flag:
        t_start = time.time()
        try:
            if use_ssl:
                conn = http.client.HTTPSConnection(host, port, timeout=5)
            else:
                conn = http.client.HTTPConnection(host, port, timeout=5)
            conn.request("GET", path)
            resp    = conn.getresponse()
            body    = resp.read()
            elapsed = time.time() - t_start
            with lock:
                if 200 <= resp.status < 400:
                    total_ok    += 1
                    total_bytes += len(body)
                    response_times.append(elapsed)
                else:
                    total_fail += 1
            conn.close()
        except Exception:
            with lock:
                total_fail += 1


def main():
    global stop_flag

    parsed  = urlparse(URL)
    host    = parsed.hostname
    port    = parsed.port or (443 if parsed.scheme == "https" else 80)
    path    = parsed.path or "/"
    use_ssl = parsed.scheme == "https"

    print("=" * 60)
    print("  STRESS TEST 01 - Availability (siege -b equivalent)")
    print("=" * 60)
    print(f"  URL         : {URL}")
    print(f"  Duration    : {DURATION}s")
    print(f"  Concurrency : {CONCURRENCY} threads")
    print("-" * 60)

    threads = []
    for _ in range(CONCURRENCY):
        t = threading.Thread(target=worker, args=(host, port, path, use_ssl), daemon=True)
        t.start()
        threads.append(t)

    time_start = time.time()
    while time.time() - time_start < DURATION:
        elapsed = time.time() - time_start
        with lock:
            ok   = total_ok
            fail = total_fail
        avail = (ok / (ok + fail) * 100) if (ok + fail) > 0 else 0
        print(f"\r  [{elapsed:>5.1f}s] Hits: {ok:>7}  Fail: {fail:>5}  Availability: {avail:>6.2f}%", end="", flush=True)
        time.sleep(0.5)

    stop_flag = True
    elapsed_total = time.time() - time_start
    print()

    # --- Final results -------------------------------------------------------
    ok             = total_ok
    fail           = total_fail
    total_requests = ok + fail
    availability   = (ok / total_requests * 100) if total_requests > 0 else 0
    throughput     = ok / elapsed_total
    data_mb        = total_bytes / 1_048_576
    avg_resp       = (sum(response_times) / len(response_times)) if response_times else 0
    max_resp       = max(response_times) if response_times else 0
    min_resp       = min(response_times) if response_times else 0

    print("=" * 60)
    print("  FINAL RESULTS")
    print("=" * 60)
    print(f"  Transactions         : {total_requests:>10} hits")
    print(f"  Successful           : {ok:>10} hits")
    print(f"  Failed               : {fail:>10} hits")
    print(f"  Availability         : {availability:>9.2f} %")
    print(f"  Elapsed time         : {elapsed_total:>9.2f} secs")
    print(f"  Data transferred     : {data_mb:>9.2f} MB")
    print(f"  Transaction rate     : {throughput:>9.2f} trans/sec")
    print(f"  Concurrency          : {CONCURRENCY:>10}")
    print(f"  Response time (avg)  : {avg_resp:>9.4f} secs")
    print(f"  Longest transaction  : {max_resp:>9.4f} secs")
    print(f"  Shortest transaction : {min_resp:>9.4f} secs")
    print("-" * 60)

    if availability >= 99.5:
        print(f"  PASS - Availability {availability:.2f}% >= 99.5% required")
    else:
        print(f"  FAIL - Availability {availability:.2f}% < 99.5% required")
    print("=" * 60)


if __name__ == "__main__":
    main()
