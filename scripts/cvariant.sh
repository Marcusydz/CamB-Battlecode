#!/bin/bash
# Usage: cvariant.sh NAME 'PARAM=value' ...  -> bots/NAME = copy of main with constexpr params overridden
cd "$(dirname "$0")/.."
N=$1; shift
SRC=${SRC:-main}; rm -rf bots/$N; cp -r bots/$SRC bots/$N; rm -rf bots/$N/.cambc
for kv in "$@"; do k=${kv%%=*}; v=${kv#*=}
  f=bots/$N/src/bot.cpp; grep -qE "^constexpr [a-z:_0-9]+ $k = " $f || f=bots/$N/src/draft.cpp
  grep -qE "^constexpr [a-z:_0-9]+ $k = " $f || { echo "no param $k"; exit 1; }
  sed -i -E "s|^(constexpr [a-z:_0-9]+ $k = )[^;]*;|\1$v;|" $f
done
sed -i -E 's|^(constexpr int (LOG_EVERY\|ACT_LOG_EVERY) = )[^;]*;|\10;|' bots/$N/src/bot.cpp
echo "made $N"
