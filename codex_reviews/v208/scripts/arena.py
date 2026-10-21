#!/usr/bin/env python3
"""Head-to-head arena: A vs B on every map, both sides, with final statistics.

Usage: arena.py BOT_A BOT_B [--maps a,b,c] [--jobs 2] [--quiet]
Prints per-match results and a summary for BOT_A: wins, mean gem margin, mean win round.
Gem margin = A's tributed gems - B's tributed gems at the end (range -1000..1000ish).
"""
import argparse, json, os, re, subprocess, sys, tempfile, time, zipfile
from concurrent.futures import ThreadPoolExecutor

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def run(a, b, mp, tmp):
    # Each attempt owns its files. Reusing --out can never load a previous game's statistics.
    attempt = tempfile.mkdtemp(prefix="match-", dir=tmp)
    stats = os.path.join(attempt, "statistics.json")
    log = os.path.join(attempt, "runner.log")
    meta = {"a": a, "b": b, "map": os.path.basename(mp).split(".")[0], "log": log}
    try:
        with open(log, "w") as stream:
            process = subprocess.run(
                ["cambc", "run", a, b, "--map", mp, "--statistics", stats,
                 "--json", "--release", "-o", attempt],
                cwd=ROOT, stdout=stream, stderr=subprocess.STDOUT, timeout=1500)
        if process.returncode:
            return dict(meta, error=f"runner exited {process.returncode}")
        with open(stats) as stream:
            data = json.load(stream)
        result = parse_statistics(a, b, mp, data)
        result.update(replay_diagnostics(attempt))
        return dict(result, log=log)
    except (OSError, ValueError, KeyError, TypeError, IndexError, subprocess.TimeoutExpired, zipfile.BadZipFile) as error:
        return dict(meta, error=f"{type(error).__name__}: {error}")


def replay_diagnostics(attempt):
    # Exceptions caught by main.cpp do not appear in engine botFaults.
    errors, overruns = [0, 0], [0, 0]
    for filename in os.listdir(attempt):
        if not filename.endswith(".repl26a"):
            continue
        with zipfile.ZipFile(os.path.join(attempt, filename)) as replay:
            for i, side in enumerate(("a", "b")):
                raw = replay.read(f"team-{side}.stdout")
                errors[i] += len(re.findall(rb"(?:draft )?error r[0-9]+:", raw))
                overruns[i] += raw.count(b"OVERRUN r")
    return {"loggedErrors": errors, "printedOverruns": overruns}


def parse_statistics(a, b, mp, d):
    if "match" in d:  # sdk 2.2 summary format
        ti = {"teamA": 0, "teamB": 1}
        if len(d["teams"]) != 2 or {t["team"] for t in d["teams"]} != set(ti):
            raise ValueError("statistics must contain exactly teamA and teamB")
        gems = [0, 0]
        for t in d["teams"]:
            gems[ti[t["team"]]] = int(t["submittedGems"])
        bf = d["diagnostics"]["botFaults"]
        faults = [bf.get("teamA", 0), bf.get("teamB", 0)]
        kills = [0, 0]
        for c in d["champions"]:
            kills[ti[c["team"]]] += int(c["metrics"]["kills"])
        oc = d["match"]["outcome"]
        outcome = oc
        return {"a": a, "b": b, "map": os.path.basename(mp).split(".")[0], "outcome": outcome,
                "round": d["match"]["roundsCompleted"], "gems": gems, "faults": faults, "kills": kills}
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
    cpp = os.path.join(src, "src", "bot.cpp")
    if os.path.exists(cpp) and "\nconstexpr int SEED = " in open(cpp).read():
        if os.path.exists(dst):
            shutil.rmtree(dst)
        shutil.copytree(src, dst, ignore=shutil.ignore_patterns(".cambc"))
        p = os.path.join(dst, "src", "bot.cpp")
        text = re.sub(r"\nconstexpr int SEED = [^;]*;", f"\nconstexpr int SEED = {seed};", open(p).read())
        open(p, "w").write(text)
        return name
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
    ap.add_argument("--seed-base", type=int, default=0, help="offset seeds, for games independent of an earlier run")
    ap.add_argument("--out", default="", help="keep replays and statistics in this directory")
    ap.add_argument("--one-side", action="store_true", help="play each map once, alternating sides")
    o = ap.parse_args()
    if o.A == o.B:
        ap.error("use two distinct bot names so results can be assigned to the correct side")
    if o.jobs < 1 or o.seeds < 0 or o.seed_base < 0:
        ap.error("jobs must be positive; seeds and seed-base must be nonnegative")
    if o.seed_base and not o.seeds:
        ap.error("--seed-base requires --seeds")
    maps = sorted(f for f in os.listdir(os.path.join(ROOT, "maps")) if f.endswith(".map26a"))
    if o.maps:
        want = set(o.maps.split(","))
        missing = want - {m.split(".")[0] for m in maps}
        if missing:
            ap.error("unknown maps: " + ", ".join(sorted(missing)))
        maps = [m for m in maps if m.split(".")[0] in want]
    if not maps:
        ap.error("no maps selected")
    tmp = os.path.abspath(o.out) if o.out else tempfile.mkdtemp(prefix="arena-")
    os.makedirs(tmp, exist_ok=True)
    pairs = [(o.A, o.B)]
    if o.seed_base:
        pairs = []  # only fresh seeded games
    for k in range(1 + o.seed_base, o.seeds + 1 + o.seed_base):
        pairs.append((make_seeded(o.A, k), make_seeded(o.B, 100 + k)))
    # Build C++ bots once up front: parallel matches building the same bot race and fail to link.
    for bot in sorted({x for p in pairs for x in p}):
        if os.path.exists(os.path.join(ROOT, "bots", bot, "src", "main.cpp")):
            subprocess.run(["cambc", "build", bot, "--release"], cwd=ROOT, check=True)
    jobs = []
    for a, b in pairs:
        for i, m in enumerate(maps):
            if not o.one_side or i % 2 == 0:
                jobs.append((a, b, "maps/" + m, tmp))
            if not o.one_side or i % 2 == 1:
                jobs.append((b, a, "maps/" + m, tmp))
    t0 = time.time()
    with ThreadPoolExecutor(o.jobs) as ex:
        results = list(ex.map(lambda j: run(*j), jobs))
    wins = 0; margin = 0; rounds = []; faults = 0; logged_errors = [0, 0]
    names_A = {p[0] for p in pairs}
    failures = [r for r in results if "error" in r]
    with open(os.path.join(tmp, "arena-results.json"), "w") as stream:
        json.dump({"planned": len(jobs), "completed": len(results) - len(failures),
                   "failed": len(failures), "results": results}, stream, indent=2)
    for r in failures:
        print(f"FAILED {r['a']} vs {r['b']} on {r['map']}: {r['error']}; log: {r['log']}", file=sys.stderr)
    results = [r for r in results if "error" not in r]
    for r in results:
        a_is_A = r["a"] in names_A
        mine, theirs = (0, 1) if a_is_A else (1, 0)
        won = r["outcome"] == ("teamA" if a_is_A else "teamB")
        wins += won
        m = r["gems"][mine] - r["gems"][theirs]
        margin += m
        faults += r["faults"][mine]
        logged_errors[0] += r.get("loggedErrors", [0, 0])[mine]
        logged_errors[1] += r.get("loggedErrors", [0, 0])[theirs]
        if won:
            rounds.append(r["round"])
        if not o.quiet:
            side = "A" if a_is_A else "B"
            print(f"{r['map']:10s} {o.A} as {side}: {'WIN ' if won else 'loss'} r{r['round']:4d} "
                  f"gems {r['gems'][mine]:4d}-{r['gems'][theirs]:4d} kills {r['kills'][mine]}-{r['kills'][theirs]}")
    n = len(results)
    print(f"SUMMARY {o.A} vs {o.B}: {wins}/{n} wins, mean margin {margin / max(n, 1):+.0f}, "
          f"mean win round {sum(rounds) / max(len(rounds), 1):.0f}, faults {faults}, {time.time() - t0:.0f}s")
    print(f"LOGGED ERRORS {o.A}={logged_errors[0]}, {o.B}={logged_errors[1]} "
          "(caught exceptions; separate from engine faults)")
    print(f"COVERAGE {n}/{len(jobs)} completed, {len(failures)} failed; results: {tmp}")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
