#!/usr/bin/env python3
"""
RUN ALL STRESS TESTS
=====================
Runs all 4 tests in sequence and prints a final summary.
Replaces the full Siege test battery required by 42 evaluation.

Requirements covered:
  1. Siege stress test equivalent       -> 01_availability.py
  2. Availability >= 99.5%             -> 01_availability.py
  3. No memory leaks                   -> 02_memory_monitor.py
  4. No loose connections              -> 03_connections.py
  5. Server usable indefinitely        -> 04_infinite.py

Usage: python3 run_all.py [duration_per_test_seconds]
       python3 run_all.py 15     <- 15s per test (recommended for evaluation)
       python3 run_all.py 30     <- 30s per test (more reliable)
"""

import sys
import os
import subprocess
import time
import http.client

DURATION    = int(sys.argv[1]) if len(sys.argv) > 1 else 15
SCRIPTS_DIR = os.path.dirname(os.path.abspath(__file__))

BANNER = """
+----------------------------------------------------------+
|        WEBSERV - STRESS TEST SUITE  (42 Evaluation)     |
|           Replaces the full Siege -b battery            |
+----------------------------------------------------------+
"""


def server_is_up():
    """Return True if the server responds with 200 on /empty.html."""
    try:
        conn = http.client.HTTPConnection("127.0.0.1", 8080, timeout=3)
        conn.request("GET", "/empty.html")
        resp = conn.getresponse()
        resp.read()
        conn.close()
        return resp.status == 200
    except Exception:
        return False


def run_test(script, args=None):
    """Run a test script and return True if it exited with code 0."""
    cmd = [sys.executable, os.path.join(SCRIPTS_DIR, script)]
    if args:
        cmd += args
    result = subprocess.run(cmd, capture_output=False)
    return result.returncode == 0


def main():
    print(BANNER)

    print("  Checking that the server is running on 127.0.0.1:8080...")
    if not server_is_up():
        print("  ERROR: Server not responding at http://127.0.0.1:8080/empty.html")
        print("  Start your server first: ./webserv conf/default.conf")
        sys.exit(1)
    print("  Server detected. Starting tests...\n")

    tests = [
        (
            "TEST 1/4 - Availability  (siege -b equivalent, must be >= 99.5%)",
            "01_availability.py",
            ["http://127.0.0.1:8080/empty.html", str(DURATION), "25"]
        ),
        (
            "TEST 2/4 - Memory Leak Monitor  (RAM must not grow indefinitely)",
            "02_memory_monitor.py",
            ["webserv", str(DURATION)]
        ),
        (
            "TEST 3/4 - Loose Connections  (no CLOSE_WAIT sockets after load)",
            "03_connections.py",
            ["8080"]
        ),
        (
            "TEST 4/4 - Indefinite Stability  (server must not crash or restart)",
            "04_infinite.py",
            [str(DURATION)]
        ),
    ]

    results = []
    for title, script, args in tests:
        print("-" * 60)
        print(f"  >> {title}")
        print("-" * 60)
        time.sleep(1)
        ok = run_test(script, args)
        results.append((title, ok))
        time.sleep(2)

    # --- Final summary -------------------------------------------------------
    print("\n" + "=" * 60)
    print("  FINAL SUMMARY")
    print("=" * 60)
    passed = 0
    for title, ok in results:
        icon = "PASS" if ok else "FAIL"
        print(f"  [{icon}] {title}")
        if ok:
            passed += 1
    print("-" * 60)
    print(f"  Passed: {passed}/{len(results)}")
    if passed == len(results):
        print("  ALL TESTS PASSED")
    else:
        print("  Some tests failed - Review individual output above.")
    print("=" * 60)


if __name__ == "__main__":
    main()
