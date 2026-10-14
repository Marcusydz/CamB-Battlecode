#!/bin/bash
# Usage: league.sh BOT [opponents...]  -> arena vs each opponent; summary lines to logs/league-BOT.txt
cd "$(dirname "$0")/.."
B=$1; shift
OPP=${@:-starter-python starter-cpp v11 aggro}
OUT=logs/league-$B.txt; : > $OUT
for o in $OPP; do python3 scripts/arena.py $B $o --quiet >> $OUT 2>&1; done
echo DONE >> $OUT
