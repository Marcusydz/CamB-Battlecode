#!/bin/bash
# Usage: acts.sh REPLAY_DIR BOTNAME -> tally logged champion activities for BOTNAME across replays
D=$1; B=$2; T=$(mktemp -d)
for f in $D/*-AB.repl26a; do unzip -p $f team-a.stdout; done > $T/all 2>/dev/null
for f in $D/*-BA.repl26a; do unzip -p $f team-b.stdout; done >> $T/all 2>/dev/null
strings -n 4 $T/all | grep -o 'act=[a-z-]*' | sort | uniq -c | sort -rn
strings -n 4 $T/all | grep -v '^r[0-9]' | grep -v '^  g\|^map' | cut -c1-150 | sort | uniq -c | sort -rn | head -5
rm -rf $T
