#!/usr/bin/env python3
"""Ladder results from the competition API: every map of our ranked/unranked matches.

Usage: ladder.py [--team TEAM_ID] [--download DIR] [--limit N]
Prints one line per map (result, score, rounds, both drafts) and a summary per map and per
opponent. --download saves each game's replay bundle (needs `cambc login` for our logs).
"""
import argparse, collections, glob, json, os, urllib.request

BASE = "https://2026a.battlecode.co.uk"
TEAM = "4620d7df-a9ac-4747-aa77-dce12395dc0c"  # Crimson Bush Campers
DRAFT = ["Ryth", "Vesper", "Vendetta", "Cassi&Issac", "Stellar", "Prism", "Marcella", "Willow",
         "Ren", "Varg", "Tavi"]


def token():
    files = glob.glob(os.path.expanduser("~/.config/cambc/session-*.json"))
    return json.load(open(files[0]))["token"] if files else None


def get(path, raw=False):
    req = urllib.request.Request(BASE + path)
    t = token()
    if t:
        req.add_header("Authorization", f"Bearer {t}")
    with urllib.request.urlopen(req, timeout=60) as r:
        data = r.read()
    return data if raw else json.loads(data)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--team", default=TEAM)
    ap.add_argument("--download", default="")
    ap.add_argument("--limit", type=int, default=100, help="max match groups")
    o = ap.parse_args()
    groups, nxt = [], None
    while len(groups) < o.limit:
        q = f"/api/v1/match-groups?team={o.team}" + (f"&cursor={nxt}" if nxt else "")
        d = get(q)
        groups += d["groups"]
        nxt = d.get("next")
        if not d.get("more") or not nxt:
            break
    by_map = collections.defaultdict(lambda: [0, 0])
    by_opp = collections.defaultdict(lambda: [0, 0])
    for g in groups:
        if g["status"] != "finished":
            continue
        detail = get(f"/api/v1/match-groups/{g['id']}")
        for m in detail["matches"]:
            me = next(s for s in m["sides"] if s["teamId"] == o.team)
            them = next(s for s in m["sides"] if s["teamId"] != o.team)
            won = me["result"] == "win"
            by_map[m["map"]][0 if won else 1] += 1
            by_opp[them["teamName"]][0 if won else 1] += 1
            draft = lambda s: ",".join(DRAFT[int(x)] for x in (s.get("draft") or []))
            print(f"{m['createdAt'][:16]} {g['kind']:8s} {them['teamName'][:22]:22s} {m['map'][:18]:18s} "
                  f"{'WIN ' if won else 'loss'} {me['score']:5}-{them['score']:<5} r{m['rounds']:<5} "
                  f"them: {draft(them)}")
            if o.download and m.get("replayUrl"):
                os.makedirs(o.download, exist_ok=True)
                path = os.path.join(o.download, f"{m['createdAt'][:10]}-{them['teamName'][:20].replace(' ', '_')}-{m['map']}-{m['id'][:8]}.zip")
                if not os.path.exists(path):
                    try:
                        open(path, "wb").write(get(m["replayUrl"], raw=True))
                    except Exception as e:
                        print("   download failed:", e)
    print("\nBy map (wins-losses):")
    for k, (w, l) in sorted(by_map.items(), key=lambda kv: kv[1][0] - kv[1][1]):
        print(f"  {k:22s} {w}-{l}")
    print("By opponent:")
    for k, (w, l) in sorted(by_opp.items(), key=lambda kv: kv[1][0] - kv[1][1]):
        print(f"  {k:22s} {w}-{l}")


if __name__ == "__main__":
    main()
