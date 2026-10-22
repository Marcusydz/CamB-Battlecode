#!/usr/bin/env python3
"""Aggregate `death ...` lines (DEATH_LOG) from replay bundles.

Usage: deaths.py DIR BOT     (DIR holds arena --out output or downloaded ladder zips)
Counts deaths per game by champion, by last action, and by killer type: no enemy seen near
(camp deaths, mostly), alone vs enemies, or with allies.
"""
import collections, glob, os, re, sys, zipfile

LINE = re.compile(r"death r(\d+) (\S+) at (\d+),(\d+) act=(\S+) gems=(\d+) enemies=(\d+) allies=(\d+)")


def main():
    d, bot = sys.argv[1], sys.argv[2]
    games = 0
    by = {k: collections.Counter() for k in ("champ", "act", "kind", "champ_kind")}
    gems_lost = 0
    for z in glob.glob(os.path.join(d, "**", "*.repl26a"), recursive=True) + glob.glob(os.path.join(d, "**", "*.zip"), recursive=True):
        name = os.path.basename(z)
        try:
            zf = zipfile.ZipFile(z)
        except zipfile.BadZipFile:
            continue
        if name.startswith(bot + "-vs-"):
            side = "team-a.stdout"
        elif "-vs-" + bot + "-" in name:
            side = "team-b.stdout"
        else:
            side = None
        files = [side] if side else ["team-a.stdout", "team-b.stdout"]
        games += 1
        for f in files:
            try:
                text = zf.read(f).decode(errors="replace")
            except KeyError:
                continue
            for m in LINE.finditer(text):
                _, champ, _, _, act, gems, enemies, allies = m.groups()
                enemies, allies = int(enemies), int(allies)
                kind = "no enemy" if enemies == 0 else ("alone" if allies == 0 else "with allies")
                by["champ"][champ] += 1
                by["act"][act] += 1
                by["kind"][kind] += 1
                by["champ_kind"][f"{champ}/{kind}"] += 1
                gems_lost += int(gems)
    print(f"{games} games, {sum(by['champ'].values()) / max(games, 1):.1f} deaths per game, "
          f"{gems_lost / max(games, 1):.0f} carried gems lost per game")
    for k, c in by.items():
        print(f"\n{k}:")
        for v, n in c.most_common(15):
            print(f"  {v:28s} {n / max(games, 1):5.2f}")


if __name__ == "__main__":
    main()
