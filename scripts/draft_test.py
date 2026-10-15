#!/usr/bin/env python3
"""Draft experiments: play lineup variants of a bot against a baseline lineup.

Usage:
  draft_test.py slots   [--base my-bot] [--lineup VENDETTA,TAVI,RYTH,VARG,MARCELLA] [--champs A,B,..] [--out DIR]
      Every champion in --champs swapped into every slot of --lineup, each vs the base lineup.
      A swap into slot k is only played on maps where teams draft more than k champions.
  draft_test.py orders  --champs A,B,C [--base my-bot] [--lineup ...] [--out DIR]
      Every ordering of A,B,C (followed by the rest of --lineup), each vs the base lineup.
  draft_test.py report  --out DIR
      Summarise results already in DIR/results.jsonl.

Each variant plays every eligible map from both sides (--seeds N adds seeded repeats).
Results are appended to DIR/results.jsonl as they finish, so an interrupted run resumes
where it stopped. The opponent is always the same bot code with the base lineup, so the
draft is the only difference.
"""
import argparse, collections, itertools, json, os, re, shutil, subprocess, sys, tempfile, time
from concurrent.futures import ThreadPoolExecutor

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BASE_LINEUP = "VENDETTA,TAVI,RYTH,VARG,MARCELLA"
UNTESTED = "CASSI_AND_ISSAC,STELLAR,PRISM,VESPER_VALE,REN,WILLOW"
# Champions each team drafts on each map (read from match statistics; see team_sizes()).
TEAM_SIZE = {"collision": 3, "expanse": 3, "packs": 3, "interlock": 4, "rows": 4,
             "corners": 5, "elkring": 5, "lanes": 5, "monolith": 5}
SHORT = {"VENDETTA": "Ven", "TAVI": "Tav", "RYTH": "Ryt", "VARG": "Var", "MARCELLA": "Mar",
         "CASSI_AND_ISSAC": "C&I", "STELLAR": "Ste", "PRISM": "Pri", "VESPER_VALE": "Ves",
         "REN": "Ren", "WILLOW": "Wil"}


def build(name, lineup, base, seed=0):
    """bots/NAME = copy of bots/BASE drafting LINEUP first, logs off, optional SEED."""
    src = os.path.join(ROOT, "bots", base)
    dst = os.path.join(ROOT, "bots", name)
    if os.path.exists(dst):
        shutil.rmtree(dst)
    shutil.copytree(src, dst, ignore=shutil.ignore_patterns(".cambc"))
    draft = os.path.join(dst, "src", "draft.py")
    text = open(draft).read()
    old = re.search(r"PREFERENCE = \[(.*?)\]", text, re.S).group(1)
    names = re.findall(r"D\.(\w+)", old)
    order = list(lineup) + [n for n in names if n not in lineup]
    open(draft, "w").write(text.replace(old, "\n" + "".join(f"    D.{n},\n" for n in order)))
    bot = os.path.join(dst, "src", "bot.py")
    b = open(bot).read()
    for k, v in (("LOG_EVERY", 0), ("ACT_LOG_EVERY", 0), ("SEED", seed)):
        b = re.sub(rf"\n{k} = [^\n#]*", f"\n{k} = {v} ", b)
    open(bot, "w").write(b)
    subprocess.run(["python3.13", os.path.join(dst, "build_bytecode.py")], stdout=subprocess.DEVNULL, check=True)
    return name


def play(job, tmp):
    a, b, mp = job["a"], job["b"], job["map"]
    stats = os.path.join(tmp, f"{a}-{b}-{mp}.json")
    for attempt in range(2):  # an occasional run dies without writing statistics; retry once
        try:
            subprocess.run(["cambc", "run", a, b, "--map", f"maps/{mp}.map26a", "--statistics", stats,
                            "-o", os.path.join(tmp, f"{a}-{b}-{mp}")],
                           cwd=ROOT, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=1800)
        except subprocess.TimeoutExpired:
            pass
        if os.path.exists(stats):
            break
    else:
        print(f"  match failed twice, skipped: {a} vs {b} on {mp}", flush=True)
        return None
    d = json.load(open(stats))
    gems = [t["submittedGems"] for t in d["teams"]]
    var_is_a = job["variant_side"] == "A"
    mine, theirs = (0, 1) if var_is_a else (1, 0)
    won = d["outcome"] == ("teamA" if var_is_a else "teamB")
    shutil.rmtree(os.path.join(tmp, f"{a}-{b}-{mp}"), ignore_errors=True)
    return dict(job, won=won, margin=gems[mine] - gems[theirs], round=d["round"],
                faults=d.get("botFaults", [0, 0])[mine], draft=d["teams"][mine]["draftPicks"])


def run_jobs(jobs, out, n_jobs):
    path = os.path.join(out, "results.jsonl")
    done = set()
    if os.path.exists(path):
        for line in open(path):
            r = json.loads(line)
            done.add(r["id"])
    todo = [j for j in jobs if j["id"] not in done]
    print(f"{len(jobs)} games, {len(jobs) - len(todo)} already done, {len(todo)} to play", flush=True)
    tmp = tempfile.mkdtemp(prefix="draft-")
    t0 = time.time()
    with ThreadPoolExecutor(n_jobs) as ex, open(path, "a") as f:
        for i, r in enumerate(ex.map(lambda j: play(j, tmp), todo)):
            if r is not None:
                f.write(json.dumps(r) + "\n"); f.flush()
            if (i + 1) % 10 == 0:
                el = time.time() - t0
                print(f"  {i + 1}/{len(todo)} games, {el / 60:.0f} min, ~{el / (i + 1) * (len(todo) - i - 1) / 60:.0f} min left", flush=True)


def jobs_for(variant, label, base_bot, maps, seeds, base):
    js = []
    for s in range(seeds + 1):
        v = variant if s == 0 else f"{variant}_s{s}"
        o = base_bot if s == 0 else f"{base_bot}_s{s}"
        for m in maps:
            js.append(dict(id=f"{label}|{m}|A|{s}", label=label, map=m, a=v, b=o, variant_side="A"))
            js.append(dict(id=f"{label}|{m}|B|{s}", label=label, map=m, a=o, b=v, variant_side="B"))
    return js


def report(out):
    rows = collections.defaultdict(list)
    for line in open(os.path.join(out, "results.jsonl")):
        r = json.loads(line)
        rows[r["label"]].append(r)
    table = []
    for label, rs in rows.items():
        n = len(rs)
        w = sum(r["won"] for r in rs)
        m = sum(r["margin"] for r in rs) / n
        se = (sum((r["margin"] - m) ** 2 for r in rs) / max(n - 1, 1)) ** 0.5 / n ** 0.5
        table.append((m, label, w, n, se, sum(r["faults"] for r in rs)))
    table.sort(reverse=True)
    print(f"{'variant':38s} {'wins':>7s} {'win%':>5s} {'margin':>7s} {'±se':>5s}")
    for m, label, w, n, se, faults in table:
        print(f"{label:38s} {w:3d}/{n:<3d} {w / n:5.0%} {m:+7.0f} {se:5.0f}" + (f"  faults={faults}" if faults else ""))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("mode", choices=["slots", "orders", "report"])
    ap.add_argument("--base", default="my-bot")
    ap.add_argument("--lineup", default=BASE_LINEUP)
    ap.add_argument("--champs", default=UNTESTED)
    ap.add_argument("--slots", default="", help="only these slot numbers (1-based), e.g. 1,2,3")
    ap.add_argument("--maps", default="")
    ap.add_argument("--seeds", type=int, default=0)
    ap.add_argument("--jobs", type=int, default=2)
    ap.add_argument("--out", default="logs/draft")
    o = ap.parse_args()
    out = os.path.join(ROOT, o.out)
    os.makedirs(out, exist_ok=True)
    if o.mode == "report":
        return report(out)
    lineup = o.lineup.split(",")
    champs = o.champs.split(",")
    all_maps = sorted(TEAM_SIZE)
    if o.maps:
        all_maps = [m for m in all_maps if m in o.maps.split(",")]
    base_bot = build("_dbase", lineup, o.base)
    for s in range(1, o.seeds + 1):
        build(f"_dbase_s{s}", lineup, o.base, seed=100 + s)
    variants = []
    if o.mode == "slots":
        slots = [int(x) - 1 for x in o.slots.split(",")] if o.slots else range(len(lineup))
        for c in champs:
            for k in slots:
                new = list(lineup)
                if c in new:  # already in the lineup: swap positions instead
                    j = new.index(c)
                    new[j], new[k] = new[k], new[j]
                else:
                    new[k] = c
                maps = [m for m in all_maps if TEAM_SIZE[m] > k]
                label = f"{SHORT[c]}@{k + 1}: " + ",".join(SHORT[n] for n in new[:5])
                variants.append((label, new, maps))
    else:
        for perm in itertools.permutations(champs):
            new = list(perm) + [n for n in lineup if n not in perm]
            label = "order: " + ",".join(SHORT[n] for n in new[:5])
            variants.append((label, new, all_maps))
    jobs = []
    for i, (label, new, maps) in enumerate(variants):
        name = f"_dv{i}"
        build(name, new, o.base)
        for s in range(1, o.seeds + 1):
            build(f"{name}_s{s}", new, o.base, seed=s)
        jobs += jobs_for(name, label, base_bot, maps, o.seeds, o.base)
    run_jobs(jobs, out, o.jobs)
    report(out)


if __name__ == "__main__":
    main()
