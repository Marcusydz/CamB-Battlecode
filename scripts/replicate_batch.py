#!/usr/bin/env python3
"""Queue the same opponents, maps and sides as an earlier unranked batch (paired A/B on the ladder).

Usage: replicate_batch.py SOURCE NAME     e.g. replicate_batch.py v160_base v160_A
Then: scripts/unranked_high_elo.py --report NAME
"""
import json, os, subprocess, sys, time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CAMBC = os.path.join(ROOT, ".venv", "bin", "cambc")
src, name = sys.argv[1], sys.argv[2]
out = []
for g in json.load(open(os.path.join(ROOT, "results", "unranked", src + ".json"))):
    for attempt in range(10):
        r = subprocess.run([CAMBC, "match-groups", "create", g["teamId"], "--maps", ",".join(g["maps"]),
                            "--sides", g["sides"], "--json"], capture_output=True, text=True)
        if r.returncode == 0:
            break
        time.sleep(2 + attempt)
    else:
        sys.exit(r.stderr)
    out.append({k: g[k] for k in ("opp", "teamId", "maps", "sides") if k in g} | {"resp": json.loads(r.stdout.splitlines()[0])})
    print("queued vs", g["opp"])
json.dump(out, open(os.path.join(ROOT, "results", "unranked", name + ".json"), "w"), indent=1)
