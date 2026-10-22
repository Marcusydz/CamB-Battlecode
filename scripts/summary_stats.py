#!/usr/bin/env python3
"""Per-game and pooled team metrics from match summary JSONs (fetch_summaries.py / cambc run --statistics).

Usage: summary_stats.py DIR [--team-name-file results/unranked/batch1.json]
"Us" is found from the champion names when the group file is given, else teamA.
"""
import collections, glob, json, os, sys

TEAM = "4620d7df-a9ac-4747-aa77-dce12395dc0c"
KEYS = ["submittedGems", "collectedGems", "kills", "deaths", "earnedGold", "championDamageDone",
        "championDamageTaken", "monsterDamageDone", "monsterDamageTaken", "healingDone", "shieldingDone"]


def our_side(groups):
    side = {}
    for g in groups:
        for m in g["detail"]["matches"]:
            for t in m.get("teams", []):
                if t.get("teamId") == TEAM:
                    side[m["matchId"]] = "teamA" if t["side"] == "a" else "teamB"
    return side


def main():
    d = sys.argv[1]
    side = {}
    if len(sys.argv) > 2:
        side = our_side(json.load(open(sys.argv[2])))
    pooled = {"us": collections.Counter(), "them": collections.Counter()}
    per_champ = collections.defaultdict(collections.Counter)
    n = 0
    for f in sorted(glob.glob(os.path.join(d, "*.json"))):
        s = json.load(open(f))
        mid = os.path.basename(f)[:-5]
        us = side.get(mid, "teamA")
        tot = {"us": collections.Counter(), "them": collections.Counter()}
        names = {"us": [], "them": []}
        for c in s["champions"]:
            who = "us" if c["team"] == us else "them"
            names[who].append(c["name"])
            for k in KEYS:
                v = float(c["metrics"].get(k, 0))
                tot[who][k] += v
                if who == "us":
                    per_champ[c["name"]][k] += v
            if who == "us":
                per_champ[c["name"]]["games"] += 1
        for w in tot:
            pooled[w].update(tot[w])
        n += 1
        g = {t["team"]: int(t["submittedGems"]) for t in s["teams"]}
        them_side = "teamB" if us == "teamA" else "teamA"
        print(f"{mid[:8]} r{s['match']['roundsCompleted']:4d} gems {g[us]:4d}-{g[them_side]:<4d} "
              f"collected {tot['us']['collectedGems']:4.0f}-{tot['them']['collectedGems']:<4.0f} "
              f"K/D {tot['us']['kills']:.0f}/{tot['us']['deaths']:.0f} gold {tot['us']['earnedGold']:5.0f}-{tot['them']['earnedGold']:<5.0f} "
              f"{'/'.join(names['us'])} vs {'/'.join(names['them'])}")
    print(f"\nPooled over {n} games (per game):")
    for k in KEYS:
        print(f"  {k:22s} us {pooled['us'][k] / n:9.0f}   them {pooled['them'][k] / n:9.0f}")
    print("\nOur champions (per game played):")
    for c, m in sorted(per_champ.items()):
        g = m["games"]
        print(f"  {c:14s} n={g:3d} " + " ".join(f"{k[:10]}={m[k] / g:.0f}" for k in KEYS))


if __name__ == "__main__":
    main()
