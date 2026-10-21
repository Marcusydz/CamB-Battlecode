# main: Cambridge Battlecode 2026a, C++

The bot we submit. It started as a C++ port of the Python bot (v16) and has been tuned since
through v138 (the `exp/*` branches on the remote hold each probe). The files here are the
version downloaded from the competition site.

## Files

| File | What it is |
| --- | --- |
| `src/main.cpp` | Entry point. The draft instance runs `play_draft()`, champions run `Bot`. A failed turn is logged and still yields. |
| `src/draft/` | Draft preference lists plus map-fit, team-role, matchup, and enemy-synergy scoring for each legal pick. |
| `src/core/bot.h` | The `Bot` class and `GroupInfo` (what we know about each camp group). |
| `src/core/config.h` | Every tuning switch (`constexpr`, overridden by `scripts/cvariant.sh`) and the per-map settings' defaults. |
| `src/core/util.h` | Small shared helpers: camp tables, distances, safe vision queries, the camp waiting ring. |
| `src/core/setup.cpp` | First-turn setup: camp groups, distance fields, ability roles, shared-memory slot. |
| `src/core/memory.cpp` | Shared-memory camp claims and camp group state. |
| `src/actions/movement.cpp` | Walking and dashing towards goals. |
| `src/actions/combat.cpp` | Spells, basic attacks, fighting camps, kiting, Heart of Ruin's dash strike. |
| `src/actions/pvp.cpp` | Fights against champions: engage, back off, join allies, retreat and recall. |
| `src/actions/economy.cpp` | Shopping, wards, tributes, altar locks. |
| `src/strategy/planning.cpp` | Camp choice: which group to farm next, alone or with helpers. |
| `src/strategy/map_settings.cpp` | Per-map behaviour by map fingerprint, with the probe that chose each setting. |
| `src/strategy/turn.cpp` | One champion turn, in priority order: survive, fight, bank, shop, farm. |

Fuel note: the bot takes different branches near its fuel thresholds (`FUEL_RESERVE`, `LOW_FUEL`),
so even a change with no logic effect shifts a few games. To check that a refactor changes nothing,
compare games with `cambc run ... --fuel 50000000`, where those branches never trigger.

Build with `cambc build main` (`cambc run` builds automatically). Needs cambc 3.x: older SDKs
lack `heart_of_ruin` and `golem`. Every `.cpp` under `src/` is compiled; headers are included
by relative path.

## How it plays

- **Camp choice:** each champion scores camp groups by gems (plus 0.1 per gold) over walking time
  (counted 3x), clear time, capture time and altar distance. Claims go in shared memory (one byte
  per draft slot) so allies spread out, or follow their squad leader's camp (squads of 2, 3 or 5
  depending on the map).
- **Adaptive strategy:** the draft keeps its tested map rankings as a fallback, then scores map
  geometry, missing team roles, ally synergies, revealed enemy matchups, and disruption of the
  enemy's revealed composition. Since mirror picks are allowed, it cannot deny an opponent a
  champion. In play, map geometry and submitted-gem leads shift camp choice, engagement thresholds,
  carrier targeting and banking.
- **Fights:** compares our power (health x damage) against the visible enemies, scaled up for
  enemies seen recently but now hidden. Engages and focus-fires when winning, backs off or recalls
  when losing, leaves early when outnumbered, joins nearby allies' winnable fights, interrupts
  enemy tributes with stuns and altar locks.
- **Survival:** retreats below 40% health (55% on some maps), holds spawn against spawn campers on
  some maps, avoids camps where it was recently killed.
- **Items:** emblems in the order the ladder #1 team builds them, optionally opening with treasure,
  then power shards forever. Buys an altar lock whenever on spawn.
- **Fuel:** thresholds are fractions of the 100k turn budget (`FUEL_RESERVE` 0.28, `LOW_FUEL` 0.64).

## Known issues

See `TODO.md` in the repository root.
