#!/usr/bin/env python3
"""C++ head-to-head with seeded variation: A vs B on every map, both sides, for N seeds.
C++ bots are deterministic, so each seed k makes copies of both bots with SEED set (cvariant.sh),
which perturbs camp choice slightly. Prints the combined record and mean gem margin for A.

Usage: carena.py A B [--seeds N] [--maps a,b] [--jobs 4] [PARAM=value ...applied to A only]
"""
import argparse, os, re, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def variant(name, src, params):
    subprocess.run([os.path.join(ROOT, "scripts", "cvariant.sh"), name] + params, cwd=ROOT,
                   env=dict(os.environ, SRC=src), check=True, stdout=subprocess.DEVNULL)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("A"); ap.add_argument("B")
    ap.add_argument("--seeds", type=int, default=3)
    ap.add_argument("--maps", default="")
    ap.add_argument("--jobs", type=int, default=4)
    o, extra = ap.parse_known_args()
    wins = games = 0
    margin = 0.0
    for k in range(o.seeds):
        a, b = f"_ca{k}_{o.A}", f"_cb{k}_{o.B}"
        variant(a, o.A, extra + ([f"SEED={k}"] if k else []))
        variant(b, o.B, [f"SEED={k + 100}"] if k else [])
        cmd = [sys.executable, os.path.join(ROOT, "scripts", "arena.py"), a, b, "--jobs", str(o.jobs), "--quiet"]
        if o.maps:
            cmd += ["--maps", o.maps]
        out = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True).stdout
        m = re.search(r"(\d+)/(\d+) wins, mean margin ([-+]?\d+)", out)
        if m:
            w, g, mm = int(m.group(1)), int(m.group(2)), int(m.group(3))
            wins += w; games += g; margin += mm * g
            print(f"seed {k}: {w}/{g} margin {mm:+d}", flush=True)
        subprocess.run(["rm", "-rf", os.path.join(ROOT, "bots", a), os.path.join(ROOT, "bots", b)])
    print(f"TOTAL {o.A} vs {o.B}: {wins}/{games} wins ({100.0 * wins / max(games, 1):.0f}%), "
          f"mean margin {margin / max(games, 1):+.0f}")


main()
