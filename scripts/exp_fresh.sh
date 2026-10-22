#!/bin/bash
# Like exp.sh but on fresh seeds (SEEDBASE+1 .. SEEDBASE+2), to confirm a result chosen on seeds 0-1.
# Usage: SEEDBASE=2 scripts/exp_fresh.sh NAME 'PARAM=value' ...
cd "$(dirname "$0")/.."
export PATH=$PWD/.venv/bin:$PATH
N=$1; shift
scripts/cvariant.sh "$N" "$@" >/dev/null || exit 1
for opp in _v120 _v140; do
    python3 scripts/arena.py "$N" $opp --seeds 2 --seed-base ${SEEDBASE:-2} --jobs 12 --quiet --out results/arena/${N}_fresh_vs$opp > logs/expf_${N}_$opp.txt 2>&1
    echo "$N $* vs $opp: $(tail -n1 logs/expf_${N}_$opp.txt)"
done
