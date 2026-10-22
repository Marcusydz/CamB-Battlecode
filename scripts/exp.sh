#!/bin/bash
# Paired experiment: bots/NAME = main with PARAMS, vs _v120 and _v140 on all maps, unseeded + seed 1.
# Usage: scripts/exp.sh NAME 'PARAM=value' ...   (compare with scripts/per_map.py on the same seeds)
cd "$(dirname "$0")/.."
export PATH=$PWD/.venv/bin:$PATH
N=$1; shift
scripts/cvariant.sh "$N" "$@" >/dev/null || exit 1
for opp in _v120 _v140; do
    python3 scripts/arena.py "$N" $opp --seeds 1 --jobs 12 --quiet --out results/arena/${N}_vs$opp > logs/exp_${N}_$opp.txt 2>&1
    echo "$N $* vs $opp: $(tail -n1 logs/exp_${N}_$opp.txt)"
done
