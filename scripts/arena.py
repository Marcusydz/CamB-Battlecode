#!/usr/bin/env python3
"""Head-to-head arena: A vs B on every map, both sides, with final statistics.

Usage: arena.py BOT_A BOT_B [--maps a,b,c] [--jobs 2] [--quiet]
Prints per-match results and a summary for BOT_A: wins, mean gem margin, mean win round.
Gem margin = A's tributed gems - B's tributed gems at the end (range -1000..1000ish).
"""
import argparse, json, os, subprocess, sys, tempfile, time
from concurrent.futures import ThreadPoolExecutor

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def run(a, b, mp, tmp):
    tag = f"{a}-{b}-{os.path.basename(mp)}"
    stats = os.path.join(tmp, tag + ".json")
    out = os.path.join(tmp, tag)
    subprocess.run(["cambc", "run", a, b, "--map", mp, "--statistics", stats, "-o", out],
                   cwd=ROOT, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=1500)
    d = json.load(open(stats))
    gems = [t["submittedGems"] for t in d["teams"]]
    faults = d.get("botFaults", [0, 0])
    kills = [0, 0]
    for c in d["champions"]:
        kills[c["team"]] += c["kills"]
    return {"a": a, "b": b, "map": os.path.basename(mp).split(".")[0], "outcome": d["outcome"],
            "round": d["round"], "gems": gems, "faults": faults, "kills": kills}


def make_seeded(bot, seed):
    """Copy bots/BOT to bots/_sBOT_SEED with SEED set (only if the bot has a SEED param)."""
    import re, shutil
    src = os.path.join(ROOT, "bots", bot)
    name = f"_s{bot}_{seed}"
    dst = os.path.join(ROOT, "bots", name)
    path = os.path.join(src, "src", "bot.py")
    if not os.path.exists(path) or "\nSEED = " not in open(path).read():
        return bot
    if os.path.exists(dst):
        shutil.rmtree(dst)
    shutil.copytree(src, dst, ignore=shutil.ignore_patterns(".cambc"))
    p = os.path.join(dst, "src", "bot.py")
    text = re.sub(r"\nSEED = .*", f"\nSEED = {seed}", open(p).read())
    text = re.sub(r"\nLOG_EVERY = .*", "\nLOG_EVERY = 0", text)
    open(p, "w").write(text)
    subprocess.run(["python3.13", os.path.join(dst, "build_bytecode.py")], stdout=subprocess.DEVNULL, check=True)
    return name


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("A"); ap.add_argument("B")
    ap.add_argument("--maps", default="")
    ap.add_argument("--jobs", type=int, default=2)
    ap.add_argument("--quiet", action="store_true")
    ap.add_argument("--seeds", type=int, default=0, help="also play N seeded copies of each bot")
    o = ap.parse_args()
    maps = sorted(f for f in os.listdir(os.path.join(ROOT, "maps")) if f.endswith(".map26a"))
    if o.maps:
        want = set(o.maps.split(","))
        maps = [m for m in maps if m.split(".")[0] in want]
    tmp = tempfile.mkdtemp(prefix="arena-")
    pairs = [(o.A, o.B)]
    for k in range(1, o.seeds + 1):
        pairs.append((make_seeded(o.A, k), make_seeded(o.B, 100 + k)))
    jobs = []
    for a, b in pairs:
        for m in maps:
            jobs.append((a, b, "maps/" + m, tmp))
            jobs.append((b, a, "maps/" + m, tmp))
    t0 = time.time()
    with ThreadPoolExecutor(o.jobs) as ex:
        results = list(ex.map(lambda j: run(*j), jobs))
    wins = 0; margin = 0; rounds = []; faults = 0
    names_A = {p[0] for p in pairs}
    for r in results:
        a_is_A = r["a"] in names_A
        mine, theirs = (0, 1) if a_is_A else (1, 0)
        won = r["outcome"] == ("teamA" if a_is_A else "teamB")
        wins += won
        m = r["gems"][mine] - r["gems"][theirs]
        margin += m
        faults += r["faults"][mine]
        if won:
            rounds.append(r["round"])
        if not o.quiet:
            side = "A" if a_is_A else "B"
            print(f"{r['map']:10s} {o.A} as {side}: {'WIN ' if won else 'loss'} r{r['round']:4d} "
                  f"gems {r['gems'][mine]:4d}-{r['gems'][theirs]:4d} kills {r['kills'][mine]}-{r['kills'][theirs]}")
    n = len(results)
    print(f"SUMMARY {o.A} vs {o.B}: {wins}/{n} wins, mean margin {margin / n:+.0f}, "
          f"mean win round {sum(rounds) / max(len(rounds), 1):.0f}, faults {faults}, {time.time() - t0:.0f}s")


if __name__ == "__main__":
    main()
