#!/usr/bin/env python3
"""Fit a draft model on ladder games (results/all_matches.jsonl, from pull_all.py / sample_ladder.sh).

P(team 1 wins) = sigmoid( elo_k * (elo1 - elo2) / 400 * ln10
                          + sum_c in T1 (g[c] + m[map][c]) - same for T2
                          + sum pairs in T1 S[c][d]          - same for T2
                          + sum c in T1, d in T2 K[c][d]     - sum c in T2, d in T1 K[c][d] )

Ratings are current ladder ratings (results/ladder*.json), used as a fixed offset so that champion
values are not just "good teams pick X". L2 shrinks map values towards 0 (towards the global value),
synergy and counters towards 0. Writes results/draft_model.json and prints tables.

Usage: draft_model.py [--min-elo 1550] [--epochs 300]
"""
import argparse, collections, glob, json, math, os, random

ROOT = os.path.join(os.path.dirname(__file__), "..")
N = ["Ryth", "Vesper", "Vendetta", "C&I", "Stellar", "Prism", "Marcella", "Willow", "Ren", "Varg",
     "Tavi", "HoR"]
C = len(N)


def load(min_elo, max_elo, since):
    elo = {}
    for f in glob.glob(os.path.join(ROOT, "results", "ladder*.json")):
        for t in json.load(open(f))["teams"]:
            elo[t["teamId"]] = t["rating"]
    rows = []
    seen = set()
    for line in open(os.path.join(ROOT, "results", "all_matches.jsonl")):
        m = json.loads(line)
        if m["id"] in seen or m["t"] < since:
            continue
        seen.add(m["id"])
        (a, pa, ra, _), (b, pb, rb, _) = m["teams"]
        if a not in elo or b not in elo or ra == rb or ra not in ("win", "loss"):
            continue
        if min(elo[a], elo[b]) < min_elo or max(elo[a], elo[b]) > max_elo:
            continue
        rows.append((m["map"], pa, pb, 1.0 if ra == "win" else 0.0, (elo[a] - elo[b]) / 400 * math.log(10)))
    return rows


def features(mp, pa, pb):
    """Sparse feature list [(key, value)] for team a minus team b."""
    f = collections.defaultdict(float)
    for team, s in ((pa, 1.0), (pb, -1.0)):
        for c in team:
            f[("g", c)] += s
            f[("m", mp, c)] += s
        t = sorted(set(team))
        for i in range(len(t)):
            for j in range(i + 1, len(t)):
                f[("s", t[i], t[j])] += s
    for c in set(pa):
        for d in set(pb):
            f[("k", c, d)] += 1.0
            f[("k", d, c)] -= 1.0
    return [(k, v) for k, v in f.items() if v != 0.0]


def reg(k, lam):
    return {"g": lam["g"], "m": lam["m"], "s": lam["s"], "k": lam["k"], "e": 0.0}[k[0]]


def fit(rows, epochs, lam, seed=0):
    X = [(features(mp, pa, pb), y, off) for mp, pa, pb, y, off in rows]
    w = collections.defaultdict(float)
    w[("e",)] = 1.0  # elo scale (fitted: current ratings are noisy)
    m1 = collections.defaultdict(float)
    m2 = collections.defaultdict(float)
    lr, b1, b2, eps = 0.05, 0.9, 0.999, 1e-8
    n = len(X)
    for ep in range(1, epochs + 1):
        g = collections.defaultdict(float)
        ll = 0.0
        for f, y, off in X:
            z = w[("e",)] * off + sum(w[k] * v for k, v in f)
            p = 1.0 / (1.0 + math.exp(-max(-30.0, min(30.0, z))))
            ll += -math.log(max(1e-12, p if y else 1 - p))
            d = p - y
            g[("e",)] += d * off
            for k, v in f:
                g[k] += d * v
        keys = set(g) | set(w)
        for k in keys:
            gk = g[k] / n + reg(k, lam) * w[k]
            m1[k] = b1 * m1[k] + (1 - b1) * gk
            m2[k] = b2 * m2[k] + (1 - b2) * gk * gk
            w[k] -= lr * (m1[k] / (1 - b1 ** ep)) / (math.sqrt(m2[k] / (1 - b2 ** ep)) + eps)
        if ep % 50 == 0 or ep == 1:
            print(f"epoch {ep}: logloss {ll / n:.4f} elo-scale {w[('e',)]:.2f}", flush=True)
    return w


def predict(w, mp, pa, pb, off=0.0):
    z = w[("e",)] * off + sum(w.get(k, 0.0) * v for k, v in features(mp, pa, pb))
    return 1.0 / (1.0 + math.exp(-z))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--min-elo", type=float, default=1550)
    ap.add_argument("--max-elo", type=float, default=1e9)
    ap.add_argument("--since", default="2026-10-21T05:20")
    ap.add_argument("--epochs", type=int, default=300)
    ap.add_argument("--holdout", type=float, default=0.0, help="fraction of games held out to check calibration")
    ap.add_argument("--lam-m", type=float, default=2e-3)
    ap.add_argument("--lam-k", type=float, default=2e-3)
    ap.add_argument("--lam-s", type=float, default=2e-3)
    ap.add_argument("--out", default=os.path.join(ROOT, "results", "draft_model.json"))
    o = ap.parse_args()
    rows = load(o.min_elo, o.max_elo, o.since)
    random.Random(1).shuffle(rows)
    test = rows[: int(len(rows) * o.holdout)]
    train = rows[len(test):]
    print(f"{len(train)} training games, {len(test)} held out")
    lam = {"g": 1e-4, "m": o.lam_m, "s": o.lam_s, "k": o.lam_k}
    w = fit(train, o.epochs, lam)
    if test:
        ll = acc = base = 0.0
        for mp, pa, pb, y, off in test:
            p = predict(w, mp, pa, pb, off)
            p0 = 1.0 / (1.0 + math.exp(-w[("e",)] * off))
            ll += -math.log(p if y else 1 - p)
            base += -math.log(p0 if y else 1 - p0)
            acc += (p > 0.5) == (y > 0.5)
        print(f"held out: logloss {ll / len(test):.4f} (elo only {base / len(test):.4f}), accuracy {acc / len(test):.3f}")
    print("\nGlobal champion value (logit):")
    for c in sorted(range(C), key=lambda c: -w[("g", c)]):
        print(f"  {N[c]:9s} {w[('g', c)]:+.3f}")
    print("\nStrongest synergies:")
    syn = sorted(((w[("s", i, j)], i, j) for i in range(C) for j in range(i + 1, C)), reverse=True)
    for v, i, j in syn[:10] + syn[-10:]:
        print(f"  {N[i]}+{N[j]}: {v:+.3f}")
    print("\nStrongest counters (row beats column):")
    ks = sorted(((w[("k", i, j)], i, j) for i in range(C) for j in range(C) if i != j), reverse=True)
    for v, i, j in ks[:15]:
        print(f"  {N[i]} vs {N[j]}: {v:+.3f}")
    maps = sorted({r[0] for r in rows})
    print("\nPer-map champion value (global + map), best first:")
    for mp in maps:
        vals = sorted(range(C), key=lambda c: -(w[("g", c)] + w[("m", mp, c)]))
        print(f"  {mp:22s} " + " ".join(f"{N[c]}{w[('g', c)] + w[('m', mp, c)]:+.2f}" for c in vals))
    json.dump({"names": N, "maps": maps, "elo_scale": w[("e",)],
               "global": [w[("g", c)] for c in range(C)],
               "map": {mp: [w[("m", mp, c)] for c in range(C)] for mp in maps},
               "synergy": [[w[("s", min(i, j), max(i, j))] if i != j else 0.0 for j in range(C)] for i in range(C)],
               "counter": [[w[("k", i, j)] for j in range(C)] for i in range(C)]},
              open(o.out, "w"), indent=1)
    print("wrote", o.out)


if __name__ == "__main__":
    main()
