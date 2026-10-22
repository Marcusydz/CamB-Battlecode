#!/usr/bin/env python3
"""Analyse DEATH_TRACE blocks (our last turns before each death) in replay bundles.

Usage: traces.py DIR [BOT] [--show N]
For deaths whose last act was a retreat (lowhp/lowhp-fight), reports how many turns the retreat lasted,
how much closer to home it got, the health it started at, and how close the nearest enemy was.
--show N prints N example traces.
"""
import collections, glob, os, re, sys, zipfile

DEATH = re.compile(r"death r(\d+) (\S+) at (\d+),(\d+) act=(\S+) gems=(\d+) enemies=(\d+) allies=(\d+)")
ROW = re.compile(r"t r(\d+) (\d+),(\d+) home(\d+) hp(\d+)/(\d+) gems(\d+) act=(\S+) allies(\d+)(.*)")
ENEMY = re.compile(r"\| (\S+) (\d+),(\d+) hp(\d+) d(\d+)")


def deaths(d, bot):
    for z in glob.glob(os.path.join(d, "**", "*.repl26a"), recursive=True) + glob.glob(os.path.join(d, "**", "*.zip"), recursive=True):
        name = os.path.basename(z)
        try:
            zf = zipfile.ZipFile(z)
        except zipfile.BadZipFile:
            continue
        if bot and name.startswith(bot + "-vs-"):
            files = ["team-a.stdout"]
        elif bot and "-vs-" + bot + "-" in name:
            files = ["team-b.stdout"]
        else:
            files = ["team-a.stdout", "team-b.stdout"]
        for f in files:
            try:
                lines = zf.read(f).decode(errors="replace").replace("\x00", "").splitlines()
            except KeyError:
                continue
            cur = None
            for line in lines:
                line = line.strip()
                m = DEATH.match(line)
                if m:
                    if cur:
                        yield cur
                    cur = {"champ": m.group(2), "act": m.group(5), "rows": [], "text": [line]}
                    continue
                r = ROW.match(line)
                if r and cur is not None:
                    es = [(e[0], int(e[4]), int(e[3])) for e in ENEMY.findall(r.group(10))]
                    cur["rows"].append({"round": int(r.group(1)), "home": int(r.group(4)), "hp": int(r.group(5)),
                                        "max": int(r.group(6)), "act": r.group(8), "allies": int(r.group(9)), "enemies": es})
                    cur["text"].append("  " + line)
            if cur:
                yield cur


def main():
    d = sys.argv[1]
    bot = sys.argv[2] if len(sys.argv) > 2 and not sys.argv[2].startswith("--") else ""
    show = int(sys.argv[sys.argv.index("--show") + 1]) if "--show" in sys.argv else 0
    n = 0
    kinds = collections.Counter()
    retreat_len, progress, start_hp, near = [], [], [], collections.Counter()
    examples = []
    for x in deaths(d, bot):
        n += 1
        rows = x["rows"]
        if x["act"] not in ("lowhp", "lowhp-fight") or not rows:
            kinds[x["act"]] += 1
            continue
        k = len(rows)
        while k > 0 and rows[k - 1]["act"].startswith("lowhp"):
            k -= 1
        run = rows[k:]
        if not run:
            continue
        retreat_len.append(len(run))
        progress.append(run[0]["home"] - run[-1]["home"])
        start_hp.append(run[0]["hp"] / max(run[0]["max"], 1))
        dmin = min((e[1] for r in run for e in r["enemies"]), default=99)
        near["adjacent (<=1)" if dmin <= 1 else "range 2-4" if dmin <= 4 else "5+ / none seen"] += 1
        kinds["retreat"] += 1
        if len(examples) < show:
            examples.append("\n".join(x["text"]))
    print(f"{n} deaths: " + ", ".join(f"{k} {v}" for k, v in kinds.most_common()))
    if retreat_len:
        m = len(retreat_len)
        print(f"retreat deaths {m}: turns in retreat mean {sum(retreat_len) / m:.1f} "
              f"(1 turn: {sum(1 for v in retreat_len if v == 1)}, 8+ turns: {sum(1 for v in retreat_len if v >= 8)})")
        print(f"  home distance gained: mean {sum(progress) / m:.1f}, none or negative: {sum(1 for v in progress if v <= 0)}")
        print(f"  health when the retreat began: mean {100 * sum(start_hp) / m:.0f}%")
        print("  nearest enemy seen during the retreat: " + ", ".join(f"{k} {v}" for k, v in near.most_common()))
    for e in examples:
        print("\n" + e)


if __name__ == "__main__":
    main()
