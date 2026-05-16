#!/usr/bin/env python3
"""
STRESS TEST 02 - Memory Leak Monitor
======================================
Requirement: Memory must NOT grow indefinitely while the server is under load.

HOW IT WORKS:
  Phase 1 (Load)   - Fire concurrent requests for DURATION seconds and sample RAM.
  Phase 2 (Cooldown) - Stop load, wait for connections to close, sample RAM again.

  A REAL leak would show memory staying HIGH (or growing) after the load stops.
  NORMAL behaviour: RAM rises under load (buffering active connections) and then
  returns close to the baseline once those connections are closed and freed.

Usage: python3 02_memory_monitor.py [process_name_or_pid] [duration_seconds]
       python3 02_memory_monitor.py webserv 20
       python3 02_memory_monitor.py 12345   20
"""

import sys
import os
import time
import threading
import subprocess
import http.client

# --- Configuration -----------------------------------------------------------
TARGET      = sys.argv[1] if len(sys.argv) > 1 else "webserv"
DURATION    = int(sys.argv[2]) if len(sys.argv) > 2 else 20
URL_HOST    = "127.0.0.1"
URL_PORT    = 8080
URL_PATH    = "/empty.html"
CONCURRENCY = 10        # background load threads
SAMPLE_RATE = 1.0       # seconds between memory samples
COOLDOWN    = 8         # seconds to wait after load stops before final check
# Memory is considered "leaked" if it does NOT drop back within this % of baseline
LEAK_THRESHOLD_PCT = 30  # allow up to 30% growth above baseline after cooldown

stop_flag = False


def get_pid(target):
    """Return the PID for the given process name or numeric string."""
    if target.isdigit():
        return int(target)
    try:
        result = subprocess.run(["pgrep", "-x", target], capture_output=True, text=True)
        pids = result.stdout.strip().split()
        return int(pids[0]) if pids else None
    except Exception:
        return None


def get_memory_kb(pid):
    """Read the RSS memory in KB from /proc/<pid>/status."""
    try:
        with open(f"/proc/{pid}/status", "r") as f:
            for line in f:
                if line.startswith("VmRSS:"):
                    return int(line.split()[1])
    except Exception:
        pass
    return None


def load_worker():
    """Background worker that continuously fires GET requests."""
    while not stop_flag:
        try:
            conn = http.client.HTTPConnection(URL_HOST, URL_PORT, timeout=3)
            conn.request("GET", URL_PATH)
            conn.getresponse().read()
            conn.close()
        except Exception:
            pass


def main():
    global stop_flag

    pid = get_pid(TARGET)
    if pid is None:
        print(f"  ERROR: Process '{TARGET}' not found.")
        print("  Make sure the server is running before executing this test.")
        sys.exit(1)

    baseline = get_memory_kb(pid)
    if baseline is None:
        print(f"  ERROR: Could not read memory for PID {pid}.")
        sys.exit(1)

    print("=" * 60)
    print("  STRESS TEST 02 - Memory Leak Monitor")
    print("=" * 60)
    print(f"  Process     : {TARGET} (PID: {pid})")
    print(f"  Load phase  : {DURATION}s  |  Cooldown: {COOLDOWN}s")
    print(f"  Concurrency : {CONCURRENCY} background threads")
    print(f"  Baseline RAM: {baseline} KB")
    print("-" * 60)

    # --- Phase 1: load -------------------------------------------------------
    threads = [threading.Thread(target=load_worker, daemon=True) for _ in range(CONCURRENCY)]
    for t in threads:
        t.start()

    load_samples = []
    t_start      = time.time()

    print(f"  Phase 1 — Load ({DURATION}s)")
    print(f"  {'Time':>6}  {'RAM (KB)':>10}  {'Delta vs baseline':>18}")
    print(f"  {'-'*6}  {'-'*10}  {'-'*18}")

    peak_mem = baseline
    while time.time() - t_start < DURATION:
        elapsed = time.time() - t_start
        mem = get_memory_kb(pid)
        if mem is None:
            print(f"\n  ERROR: Process PID {pid} disappeared. Did the server crash?")
            break
        peak_mem = max(peak_mem, mem)
        delta    = mem - baseline
        sign     = "+" if delta >= 0 else ""
        load_samples.append(mem)
        print(f"  {elapsed:>5.1f}s  {mem:>9} KB  {sign}{delta:>+17} KB")
        time.sleep(SAMPLE_RATE)

    # --- Phase 2: cooldown ---------------------------------------------------
    stop_flag = True
    print()
    print(f"  Phase 2 — Cooldown ({COOLDOWN}s, load stopped)")
    time.sleep(COOLDOWN)

    final_mem = get_memory_kb(pid)
    if final_mem is None:
        print("  ERROR: Could not read final memory.")
        return

    delta_from_baseline = final_mem - baseline
    pct_above_baseline  = (delta_from_baseline / baseline * 100) if baseline > 0 else 0

    print("-" * 60)
    print("  RESULTS")
    print("-" * 60)
    print(f"  Baseline RAM (before load)  : {baseline:>8} KB")
    print(f"  Peak RAM (during load)      : {peak_mem:>8} KB")
    print(f"  Final RAM (after cooldown)  : {final_mem:>8} KB")
    print(f"  Delta vs baseline           : {delta_from_baseline:>+8} KB  ({pct_above_baseline:+.1f}%)")
    print(f"  Leak threshold              : +{LEAK_THRESHOLD_PCT}% above baseline")
    print("-" * 60)

    # Verdict: memory went up during load (normal), did it come back down?
    if pct_above_baseline <= LEAK_THRESHOLD_PCT:
        print(f"  PASS - RAM returned to near-baseline after cooldown.")
        print(f"         Growth under load is NORMAL (active connection buffers).")
        print(f"         After connections closed, memory came back down ({pct_above_baseline:+.1f}%).")
    else:
        print(f"  WARNING - RAM stayed {pct_above_baseline:.1f}% above baseline after cooldown.")
        print(f"         This may indicate a memory leak. Investigate with valgrind.")
    print("=" * 60)


if __name__ == "__main__":
    main()
