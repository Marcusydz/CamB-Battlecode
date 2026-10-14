#!/usr/bin/env python3
"""Summarise how champions spent their turns, from replays kept by `arena.py --out DIR`.

Usage: activity.py DIR [BOT]
Needs bots that print activity tallies (DACTS lines, see ACT_LOG_EVERY / diagnostic copies).
Prints, per bot and champion type, the share of turns spent on each activity.
"""
import collections, os, re, sys, zipfile

d = sys.argv[1]
only = sys.argv[2] if len(sys.argv) > 2 else None
NAMES = ["Ryth", "Vesper", "Vendetta", "Cassi", "Issac", "Stellar", "Prism", "Marcella", "Willow", "Ren", "Varg", "Tavi"]
PRODUCTIVE = {"fight", "capture", "tribute", "channel", "engage", "pvp", "interrupt", "flee-tribute"}
tot = collections.defaultdict(collections.Counter)
for root, _, files in os.walk(d):
    for f in files:
        if not f.endswith(".repl26a"):
            continue
        m = re.match(r"(.+)-vs-(.+)-\d+\.repl26a", f)
        if not m:
            continue
        bots = {"team-a.stdout": m.group(1), "team-b.stdout": m.group(2)}
        z = zipfile.ZipFile(os.path.join(root, f))
        for member, bot in bots.items():
            if only and bot != only:
                continue
            text = z.read(member).decode("utf-8", "replace")
            last = {}
            for line in text.splitlines():
                i = line.find("DACTS r")  # output records carry a binary header; find the payload
                if i >= 0:
                    parts = line[i:].split()
                    last[parts[2]] = parts[3:]
            for kind, items in last.items():
                for kv in items:
                    k, v = kv.rsplit("=", 1)
                    tot[(bot, NAMES[int(kind)])][k] += int(v)
for (bot, champ), c in sorted(tot.items()):
    n = sum(c.values())
    base = collections.Counter()
    edge = 0
    for k, v in c.items():
        a = k.split("@")[0]
        base[a] += v
        if k.endswith("@edge"):
            edge += v
    prod = sum(v for a, v in base.items() if a in PRODUCTIVE)
    print(f"{bot:12s} {champ:9s} turns={n:6d} productive={prod / n:5.1%} dead={base["dead"] / n:5.1%} near-edge={edge / n:5.1%}  " +
          " ".join(f"{a}={v / n:.1%}" for a, v in base.most_common()))
