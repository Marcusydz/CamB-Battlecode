#!/usr/bin/env python3
"""Dump every ranked ladder match (all teams) to results/all_matches.jsonl, compactly.

Usage: pull_all.py [--since 2026-10-21T05:20] [--kind ranked] [--cursor C --pages N]
Sampling: scripts/sample_ladder.sh pulls N pages from each hour.
Each line: {"t": time, "map": map, "teams": [[teamId, picks, result, side], ...]}.
Resumable: stops at matches already in the file when paging back from the newest.
"""
import argparse, json, os, subprocess, sys, time

CAMBC = os.path.join(os.path.dirname(__file__), "..", ".venv", "bin", "cambc")
OUT = os.path.join(os.path.dirname(__file__), "..", "results", "all_matches.jsonl")


def page(cursor, kind):
    cmd = [CAMBC, "matches", "list", "--kind", kind, "--limit", "100", "--json"]
    if cursor:
        cmd += ["--cursor", cursor]
    for attempt in range(8):
        r = subprocess.run(cmd, capture_output=True, text=True)
        if r.returncode == 0:
            return json.loads(r.stdout)
        time.sleep(2 * (attempt + 1))
    raise RuntimeError(r.stderr)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--since", default="2026-10-21T05:20")
    ap.add_argument("--kind", default="ranked")
    ap.add_argument("--cursor", default="", help="start here, e.g. 2026-10-21T15:35:00.000000Z_ffffffff-ffff-ffff-ffff-ffffffffffff")
    ap.add_argument("--pages", type=int, default=10**9, help="stop after this many pages (sampling)")
    o = ap.parse_args()
    have = set()
    if os.path.exists(OUT):
        for line in open(OUT):
            have.add(json.loads(line)["id"])
    cursor, n, old, pages = o.cursor or None, 0, 0, 0
    with open(OUT, "a") as f:
        while True:
            d = page(cursor, o.kind)
            ms = d["matches"]
            if not ms:
                break
            for m in ms:
                if m["status"] != "finished" or m["matchId"] in have:
                    old += m["matchId"] in have
                    continue
                if not all(t.get("draftPicks") for t in m["teams"]):
                    continue
                f.write(json.dumps({"id": m["matchId"], "t": m["createdAt"], "map": m["map"],
                                    "eng": m["teams"][0].get("engineVersion"),
                                    "teams": [[t["teamId"], t["draftPicks"], t["result"], t["side"]]
                                              for t in m["teams"]]}) + "\n")
                n += 1
            f.flush()
            pages += 1
            cursor = d.get("nextCursor")
            print(f"{ms[-1]['createdAt']} new={n}", file=sys.stderr, flush=True)
            if not cursor or ms[-1]["createdAt"] < o.since or old > 500 or pages >= o.pages:
                break
    print(f"{n} new matches in {OUT}")


if __name__ == "__main__":
    main()
