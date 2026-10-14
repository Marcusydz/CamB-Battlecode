#!/usr/bin/env python3
"""Copy a bot with a different draft lineup.

Usage: lineup.py NAME CHAMP,CHAMP,CHAMP [--src my-bot] [--quiet-logs]
Champion names are ChampionDraft names (VENDETTA, TAVI, RYTH, VARG, MARCELLA, CASSI_AND_ISSAC,
STELLAR, PRISM, VESPER_VALE, REN, WILLOW). The listed champions are drafted first, in order;
the rest of the source bot's preference list follows as fallbacks.
"""
import argparse, os, re, shutil, subprocess

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

ap = argparse.ArgumentParser()
ap.add_argument("name"); ap.add_argument("champs")
ap.add_argument("--src", default="my-bot")
ap.add_argument("--quiet-logs", action="store_true", help="set LOG_EVERY and ACT_LOG_EVERY to 0")
o = ap.parse_args()
src = os.path.join(ROOT, "bots", o.src)
dst = os.path.join(ROOT, "bots", o.name)
if os.path.exists(dst):
    shutil.rmtree(dst)
shutil.copytree(src, dst, ignore=shutil.ignore_patterns(".cambc"))
draft = os.path.join(dst, "src", "draft.py")
text = open(draft).read()
old = re.search(r"PREFERENCE = \[(.*?)\]", text, re.S).group(1)
names = re.findall(r"D\.(\w+)", old)
first = o.champs.split(",")
order = first + [n for n in names if n not in first]
text = text.replace(old, "\n" + "".join(f"    D.{n},\n" for n in order))
open(draft, "w").write(text)
if o.quiet_logs:
    bot = os.path.join(dst, "src", "bot.py")
    b = open(bot).read()
    b = re.sub(r"\nLOG_EVERY = \d+", "\nLOG_EVERY = 0", b)
    b = re.sub(r"\nACT_LOG_EVERY = \d+", "\nACT_LOG_EVERY = 0", b)
    open(bot, "w").write(b)
    subprocess.run(["python3.13", os.path.join(dst, "build_bytecode.py")], stdout=subprocess.DEVNULL, check=True)
print("built", o.name, order[:3])
