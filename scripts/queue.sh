#!/bin/bash
# Usage: queue.sh BASE SEEDS v1 v2 ... -> arena each variant vs BASE, append summaries to logs/queue.txt
cd "$(dirname "$0")/.."
B=$1; N=$2; shift 2
for v in "$@"; do python3 scripts/arena.py $v $B --quiet --seeds $N >> logs/queue.txt 2>&1; done
echo "QUEUE DONE" >> logs/queue.txt
