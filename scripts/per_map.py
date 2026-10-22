#!/usr/bin/env python3
"""Per-map record of BOT from arena --out directories (statistics JSONs), pooled.

Usage: per_map.py BOT DIR [DIR...]
BOT matches the bot name prefix in file names (seeded copies _sBOT_k count as BOT).
"""
import collections, glob, json, os, sys


def main():
    bot, dirs = sys.argv[1], sys.argv[2:]
    rec = collections.defaultdict(lambda: [0, 0, 0])
    lineups = collections.defaultdict(collections.Counter)
    for d in dirs:
        for f in glob.glob(os.path.join(d, "*.json")):
            base = os.path.basename(f)
            a = base.split("-")[0]
            mine_a = a == bot or a.startswith(f"_s{bot}_")
            s = json.load(open(f))
            us = "teamA" if mine_a else "teamB"
            them = "teamB" if mine_a else "teamA"
            mp = base.split(".map26a")[0].split("-", 2)[2]
            g = {t["team"]: int(t["submittedGems"]) for t in s["teams"]}
            won = (s["match"]["outcome"] if "match" in s else s["outcome"]) == us
            r = rec[mp]
            r[0] += won
            r[1] += 1
            r[2] += g[us] - g[them]
            lineups[mp]["/".join(c["name"][:5] for c in s["champions"] if c["team"] == us)] += 1
    tw = tn = tm = 0
    for mp, (w, n, m) in sorted(rec.items(), key=lambda kv: kv[1][2] / kv[1][1]):
        print(f"  {mp:24s} {w:2d}/{n:<2d} {m / n:+6.0f}  {lineups[mp].most_common(1)[0][0]}")
        tw += w; tn += n; tm += m
    print(f"TOTAL {tw}/{tn} ({100 * tw / max(tn, 1):.0f}%), mean margin {tm / max(tn, 1):+.0f}")


if __name__ == "__main__":
    main()
