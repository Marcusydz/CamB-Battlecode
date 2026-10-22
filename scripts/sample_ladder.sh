#!/bin/bash
# Sample the global ranked ladder: PAGES pages (100 matches each) from every hour between FROM and TO.
# Usage: scripts/sample_ladder.sh 2026-10-21T05 2026-10-22T11 [PAGES]
cd "$(dirname "$0")/.."
from=$(date -u -d "${1}:00" +%s); to=$(date -u -d "${2}:00" +%s); pages=${3:-12}
for ((t = to; t >= from; t -= 3600)); do
    c="$(date -u -d @$t +%Y-%m-%dT%H):35:00.000000Z_ffffffff-ffff-ffff-ffff-ffffffffffff"
    python3 scripts/pull_all.py --cursor "$c" --pages "$pages" --since 2000 2>/dev/null
done
