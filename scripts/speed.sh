#!/bin/bash
# Usage: speed.sh BOT TAG -> plays BOT vs starter-python on all maps both sides; prints win rounds and mean.
cd "$(dirname "$0")/.."
B=$1; TAG=$2
OUT=replays/speed-$TAG-$(date +%s); mkdir -p $OUT
MF=matches-$TAG.txt; : > $MF
for m in maps/*.map26a; do n=$(basename $m .map26a)
  echo "bots/$B bots/starter-python $m $OUT/$n-AB.repl26a" >> $MF
  echo "bots/starter-python bots/$B $m $OUT/$n-BA.repl26a" >> $MF
done
cambc run-batch $MF --jobs 2 > $OUT/log.txt 2>&1
rm -f $MF
grep -E "WIN" $OUT/log.txt | sed "s/  */ /g" | awk -v b="($B)" '{won=index($0,b)>0; r=$NF; sub("Round ","",r); split(r,a,"/"); rr=a[1]+0; if(!won) rr=2000+(2000-rr); s+=rr; n++; printf "%s %s %s %d\n", $(NF-2), (won?"W":"L"), $(NF-1), rr} END {printf "MEAN %.0f over %d\n", s/n, n}'
