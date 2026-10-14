#!/bin/bash
# Usage: bench.sh BOT_A BOT_B  -> runs A vs B on every map, both sides, prints winners.
cd "$(dirname "$0")/.."
A=$1; B=$2; TAG=${3:-run}
OUT=replays/bench-$TAG-$(date +%s)
mkdir -p $OUT
MF=matches-$TAG.txt; : > $MF
for m in maps/*.map26a; do n=$(basename $m .map26a)
  echo "bots/$A bots/$B $m $OUT/$n-AB.repl26a" >> $MF
  echo "bots/$B bots/$A $m $OUT/$n-BA.repl26a" >> $MF
done
cambc run-batch $MF --jobs 2 > $OUT/log.txt 2>&1
grep -E "WIN|FAIL|crash" $OUT/log.txt | sed "s/  */ /g"
echo "OUT=$OUT"
