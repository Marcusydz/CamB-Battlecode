#!/usr/bin/env python3
"""Dump our ladder/unranked matches (via `cambc matches list`) to results/matches.jsonl.

Usage: pull_matches.py [--max N] [--kind ranked|unranked]
Pages back through the API until N matches; merges with whatever is already in the file.
"""
import argparse, json, os, subprocess, time

CAMBC = os.path.join(os.path.dirname(__file__), "..", ".venv", "bin", "cambc")
OUT = os.path.join(os.path.dirname(__file__), "..", "results", "matches.jsonl")


def page(cursor, kind):
    cmd = [CAMBC, "matches", "list", "--team", "me", "--limit", "100", "--json"]
    if cursor:
        cmd += ["--cursor", cursor]
    if kind:
        cmd += ["--kind", kind]
    for attempt in range(5):
        r = subprocess.run(cmd, capture_output=True, text=True)
        if r.returncode == 0:
            return json.loads(r.stdout)
        time.sleep(3 * (attempt + 1))
    raise RuntimeError(r.stderr)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--max", type=int, default=2000)
    ap.add_argument("--kind", default="")
    o = ap.parse_args()
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    seen = {}
    if os.path.exists(OUT):
        for line in open(OUT):
            m = json.loads(line)
            seen[m["matchId"]] = m
    cursor, n = None, 0
    while n < o.max:
        d = page(cursor, o.kind)
        for m in d["matches"]:
            seen[m["matchId"]] = m
        n += len(d["matches"])
        cursor = d.get("nextCursor")
        if not cursor or not d["matches"]:
            break
    with open(OUT, "w") as f:
        for m in sorted(seen.values(), key=lambda m: m["createdAt"]):
            f.write(json.dumps(m) + "\n")
    print(f"{len(seen)} matches in {OUT}")


if __name__ == "__main__":
    main()
