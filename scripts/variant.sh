#!/bin/bash
# Usage: variant.sh NAME 'PARAM=value' ['PARAM=value' ...]  -> bots/NAME = copy of my-bot with params overridden
cd "$(dirname "$0")/.."
N=$1; shift
SRC=${SRC:-my-bot}; rm -rf bots/$N; cp -r bots/$SRC bots/$N; rm -rf bots/$N/.cambc
for kv in "$@"; do k=${kv%%=*}; v=${kv#*=}
  grep -q "^$k = " bots/$N/src/bot.py || { echo "no param $k"; exit 1; }
  sed -i "s|^$k = .*|$k = $v|" bots/$N/src/bot.py
done
sed -i 's/^LOG_EVERY = .*/LOG_EVERY = 0/' bots/$N/src/bot.py
python3.13 bots/$N/build_bytecode.py >/dev/null && echo "built $N"
