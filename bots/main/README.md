# main: Cambridge Battlecode 2026a, C++

The bot we submit. It started as a C++ port of the Python bot (v16) and has been tuned since
through v138 (the `exp/*` branches on the remote hold each probe). The files here are the
version downloaded from the competition site.

## Files

| File | What it is |
| --- | --- |
| `src/main.cpp` | Entry point. The draft instance runs `play_draft()`, champions run `Bot`. A failed turn is logged and still yields. |
| `src/draft.cpp` | Draft preference lists (21) and the per-map choice, by map width, height and wall count. |
| `src/bot.h`, `src/bot.cpp` | All gameplay. Tuning switches are at the top of `bot.cpp`; per-map overrides are in `Bot::setup()`. |
| `src/camp_waiting_area.h` | The ring outside a camp's attack range where champions wait for helpers. |

Build with `cambc build main` (`cambc run` builds automatically). Needs cambc 3.x: older SDKs
lack `heart_of_ruin` and `golem`.

## How it plays

- **Camp choice:** each champion scores camp groups by gems (plus 0.1 per gold) over walking time
  (counted 3x), clear time, capture time and altar distance. Claims go in shared memory (one byte
  per draft slot) so allies spread out, or follow their squad leader's camp (squads of 2, 3 or 5
  depending on the map).
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
