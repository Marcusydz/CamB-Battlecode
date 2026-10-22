#!/usr/bin/env python3
"""Win rates from results/matches.jsonl (fill it with pull_matches.py).

Usage: ladder_stats.py [--since T] [--until T] [--kind ranked] [--min-elo 1600] [--max-elo 1800]
Opponent ratings come from results/ladder*.json (`cambc ladder --json`), current values only.
"""
import argparse, collections, glob, json, os

ROOT = os.path.join(os.path.dirname(__file__), "..")
TEAM = "4620d7df-a9ac-4747-aa77-dce12395dc0c"
N = ["Ryth", "Vesper", "Vendetta", "C&I", "Stellar", "Prism", "Marcella", "Willow", "Ren", "Varg",
     "Tavi", "HoR"]


def wr(w, n):
    return f"{w:3d}-{n - w:<3d} {100 * w / max(n, 1):3.0f}%"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--since", default="")
    ap.add_argument("--until", default="9999")
    ap.add_argument("--kind", default="ranked")
    ap.add_argument("--min-elo", type=float, default=0)
    ap.add_argument("--max-elo", type=float, default=1e9)
    ap.add_argument("--min-games", type=int, default=8)
    o = ap.parse_args()
    elo = {}
    for f in glob.glob(os.path.join(ROOT, "results", "ladder*.json")):
        for t in json.load(open(f))["teams"]:
            elo[t["teamId"]] = t["rating"]
    games = []
    for line in open(os.path.join(ROOT, "results", "matches.jsonl")):
        m = json.loads(line)
        if m["status"] != "finished" or (o.kind and m["kind"] != o.kind):
            continue
        if not (o.since <= m["createdAt"] < o.until):
            continue
        me = next((t for t in m["teams"] if t["teamId"] == TEAM), None)
        them = next((t for t in m["teams"] if t["teamId"] != TEAM), None)
        if not me or not them or not me["draftPicks"] or not them["draftPicks"]:
            continue
        r = elo.get(them["teamId"], 0)
        if not (o.min_elo <= r <= o.max_elo):
            continue
        games.append((m, me, them, me["result"] == "win"))
    n = len(games)
    print(f"{n} games, {wr(sum(g[3] for g in games), n)}")

    def table(title, key, multi=False, top=60):
        c = collections.defaultdict(lambda: [0, 0])
        for g in games:
            ks = key(g)
            for k in (ks if multi else [ks]):
                c[k][0] += g[3]
                c[k][1] += 1
        print(f"\n{title}")
        rows = [(k, w, t) for k, (w, t) in c.items() if t >= o.min_games]
        for k, w, t in sorted(rows, key=lambda r: r[1] / r[2])[:top]:
            print(f"  {str(k):44s} {wr(w, t)}")

    table("By map", lambda g: g[0]["map"])
    table("Our first pick", lambda g: N[g[1]["draftPicks"][0]])
    table("Enemy first pick", lambda g: N[g[2]["draftPicks"][0]])
    table("Enemy has champion", lambda g: [N[i] for i in g[2]["draftPicks"]], multi=True)
    table("We have champion", lambda g: [N[i] for i in g[1]["draftPicks"]], multi=True)
    table("Enemy core (first two)", lambda g: "/".join(sorted(N[i] for i in g[2]["draftPicks"][:2])))
    table("Opponent", lambda g: g[2]["name"][:30], top=40)
    table("Opponent rating", lambda g: int(elo.get(g[2]["teamId"], 0) // 50 * 50))
    table("Picks per side", lambda g: len(g[1]["draftPicks"]))

    # Lineups that beat us, and what wins on the ladder overall (both sides of our games).
    lw = collections.defaultdict(lambda: [0, 0])
    for m, me, them, won in games:
        for side, w in ((me, won), (them, not won)):
            for i in side["draftPicks"]:
                lw[N[i]][0] += w
                lw[N[i]][1] += 1
    print("\nChampion win rate, both teams pooled (includes us)")
    for k, (w, t) in sorted(lw.items(), key=lambda kv: -kv[1][0] / kv[1][1]):
        print(f"  {k:12s} {wr(w, t)}")


if __name__ == "__main__":
    main()
