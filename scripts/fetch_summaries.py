#!/usr/bin/env python3
"""Download match summaries as JSON: fetch_summaries.py OUTDIR MATCH_ID..."""
import os, subprocess, sys, time

out = sys.argv[1]
os.makedirs(out, exist_ok=True)
for mid in sys.argv[2:]:
    path = os.path.join(out, mid + ".json")
    if os.path.exists(path) and os.path.getsize(path) > 0:
        continue
    for attempt in range(10):
        r = subprocess.run([os.path.join(os.path.dirname(__file__), "..", ".venv", "bin", "cambc"), "matches", "summary",
                            mid, "--json", "-o", path], capture_output=True, text=True)
        if r.returncode == 0 and os.path.exists(path):
            break
        time.sleep(2 + attempt)
    print(mid, "ok" if os.path.exists(path) else r.stderr.strip()[:100], flush=True)
