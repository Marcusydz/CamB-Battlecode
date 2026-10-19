#!/usr/bin/env python3
"""Farming speed: each bot vs bots/passive on every map, both sides. Prints the round each bot reached
1000 gems (or its gems at round 2000) per map, and the mean over maps.

Usage: farmspeed.py BOT [BOT...] [--maps a,b] [--jobs 3]
"""
import argparse, json, os, subprocess, tempfile
from concurrent.futures import ThreadPoolExecutor

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def run(bot, mp, side, tmp):
    a, b = (bot, "passive") if side == 0 else ("passive", bot)
    stats = os.path.join(tmp, f"{bot}-{side}-{os.path.basename(mp)}.json")
    subprocess.run(["cambc", "run", a, b, "--map", mp, "--statistics", stats, "-o", os.path.join(tmp, "r")],
                   cwd=ROOT, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=1500)
    if not os.path.exists(stats):
        return bot, mp, side, None, None
    d = json.load(open(stats))
    return bot, mp, side, d["round"], d["teams"][side]["submittedGems"]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("bots", nargs="+")
    ap.add_argument("--maps", default="")
    ap.add_argument("--jobs", type=int, default=3)
    o = ap.parse_args()
    maps = sorted(f for f in os.listdir(os.path.join(ROOT, "maps")) if f.endswith(".map26a"))
    if o.maps:
        want = set(o.maps.split(","))
        maps = [m for m in maps if m.split(".")[0] in want]
    with tempfile.TemporaryDirectory() as tmp:
        jobs = [(b, "maps/" + m, s, tmp) for m in maps for b in o.bots for s in (0, 1)]
        with ThreadPoolExecutor(o.jobs) as ex:
            res = list(ex.map(lambda j: run(*j), jobs))
    table = {}
    for bot, mp, side, rnd, gems in res:
        # Not finished: count as 2000 * 1000 / gems (projected round), capped at 4000.
        r = 4000 if rnd is None else (rnd if gems >= 1000 else min(4000, int(2000 * 1000 / max(gems, 1))))
        table.setdefault(mp.split("/")[1].split(".")[0], {}).setdefault(bot, []).append(r)
    print("%-16s" % "map" + "".join("%12s" % b for b in o.bots))
    tot = {b: 0 for b in o.bots}
    for m in sorted(table):
        row = "%-16s" % m
        for b in o.bots:
            v = sum(table[m][b]) / len(table[m][b])
            tot[b] += v
            row += "%12.0f" % v
        print(row)
    print("%-16s" % "MEAN" + "".join("%12.0f" % (tot[b] / len(table)) for b in o.bots))


main()
