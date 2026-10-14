# my-bot (v14): Cambridge Battlecode 2026a, Python

An aggressive farming bot. It clears the best-value camps, fights whenever its side is
roughly even or better, focus-fires, finishes off gem carriers, and banks gems before it
can lose them.

## Results (local, 8 of the 9 pool maps, both sides; "rows" not tested)

| Opponent | Record | Mean gem margin | Notes |
| --- | --- | --- | --- |
| starter-python | 16/16 | +844 | wins in ~466 rounds (v1 needed ~576) |
| starter-cpp | 16/16 | +831 | different draft and language |
| v11 (first delivered bot) | 15/16 | +502 | pure farmer |
| aggro (all-in hunter) | 10/16 | +289 | |
| v12 (earlier checkpoint) | 11/16 | +257 | |
| v13 (previous checkpoint) | 57/96 | +59 | 96 games with seeded variation |

These are local results against my own sparring bots. Real opponents will differ; see
"What to do next".

## Files

| File | What it is |
| --- | --- |
| `src/main.py` | Entry point. Loads gameplay from bytecode, draft from source. |
| `src/draft.py` | Draft preference list. |
| `src/bot.py` | All gameplay. Tuning parameters are at the top. |
| `src/bot.bin` | Precompiled bytecode of `bot.py`. **Rebuild after every edit.** |
| `build_bytecode.py` | `python3.13 build_bytecode.py` rebuilds `bot.bin`. |

### Why bytecode matters

This Python runtime charges about 44,000 fuel per line to compile source, and a turn gets
2.5M fuel. Compiling `bot.py` (870 lines) would freeze every champion for about 18 rounds
and make the draft pick randomly. `bot.bin` loads in about 1.4M fuel. If it's missing,
`main.py` falls back to source, which works but starts slowly.

## Strategy

**Draft:** Vendetta, Tavi, Ryth, Varg, Marcella. Cassi & Issac come late, because when one
dies both die and the killer takes both bags. Tested: 20/32 against the Cassi-first draft.

**Each turn, per champion** (first match wins):

1. Dead: shop (Monster strength emblem, then Jungle health emblem, then Power strength
   shards), and buy an altar lock.
2. Channelling (recall, tribute): let it finish.
3. **On spawn and below 95% HP: stay put.** Spawn heals at the end of the turn. Before this
   fix, champions left spawn the same turn a recall landed and looped recall forever. This
   was the single biggest gain.
4. Below 30% HP (45% when carrying 30+ gems): recall. If an enemy is within 5 tiles, walk
   or dash home away from them instead (damage cancels a recall).
5. Banking now would win, or round > 1940: tribute.
6. **PvP**, using an evaluator that compares (sum of HP) × (sum of damage) for allies vs
   enemies within 6 tiles:
   - An enemy is tributing: lock its altar (cancels the channel), stun it, hit it.
   - Our side ≥ 0.8× theirs: engage. All allies pick the same target by one rule: an
     enemy we can kill this turn first, then lowest HP adjusted for gems carried. Stun the
     most dangerous enemy, Varg shields, nukes, then basic attack plus attack reset.
   - Our side < 0.3× theirs while carrying gems or hurt: flee or bank.
   - Otherwise hit anything already in range.
7. Pass an unlocked altar: place our lock (ours never blocks us; enemies need 3 hits).
8. On a cleared camp: capture it. Only one champion holds a capture; the others move on.
9. Holding 150+ gold with an empty bag: recall to shop (tested: 22/32).
10. Choose a camp group (every 3 rounds, never mid-fight): gems + 0.15·gold divided by
    travel + clear time + capture + half the altar distance. Camps too strong to solo
    need N champions; they wait up to 12 turns for help, then give up. Joining a joint
    target an ally has claimed gets a 1.5× bonus. Targets are shared through memory.
11. Carrying gems: tribute at 60, or sooner if the altar is closer than the next camp.

## Tested and rejected

| Idea | Result |
| --- | --- |
| Cautious fighting (engage only at 1.3× power) | lost 4–12 to the aggressive version |
| Penalising camps nearer the enemy spawn | lost to contesting everything |
| Rally calls (fighting ally summons others) | 4/16, pulls champions off farming |
| Full-team monolith assault | 2/8 on the monolith map |
| Engage ratio 0.6 or 1.0, gem cap 40, retreat at 40%, no locks, no interrupts | all neutral |
| Shop-recall at 100 or 250 gold | slightly worse than 150 |

## Testing tools (run from the workspace root)

```
python3 scripts/arena.py my-bot OTHER [--seeds N] [--maps a,b] [--quiet]
    # every map, both sides; prints wins, mean gem margin, faults.
    # --seeds N adds N seeded copies (SEED param) for more independent games.
scripts/variant.sh NAME 'PARAM=value' ...   # copy of my-bot with params changed
scripts/queue.sh BASE SEEDS v1 v2 ...       # each variant vs BASE -> logs/queue.txt
scripts/league.sh BOT [opponents...]        # vs a panel -> logs/league-BOT.txt
scripts/acts.sh REPLAY_DIR BOT              # what champions spent their time on
```

Noise is large: with 32 games a ±100 margin can be luck. Use `--seeds 5` (96 games) before
trusting a change. Keep old versions (`bots/v11`, `v12`, `v13`, `aggro`) as sparring partners.

Each champion prints a status line every 25 rounds (`LOG_EVERY`). Overrun turns (fuel ran
out) are printed as `OVERRUN`.

## Toolchain notes

- C++ starter builds need the WASI SDK, CMake and Ninja in `~/.cache/cambc/`. If the CLI's
  own download fails on certificates, download them manually (see the URLs in the error)
  and unpack them there as `wasi-sdk-33`, `cmake-3.31.8-linux-x86_64` and
  `ninja-1.12.1-linux-x86_64`.

## What to do next (in order of value)

1. **Submit and watch real ladder replays.** Every gain here came from finding a concrete
   failure in a log. Real opponents will expose new ones.
2. Test on the `rows` map (`cambc maps pull`).
3. Use real opponents' behaviour to tune `ENGAGE_RATIO` and the draft.
4. Monolith: a version that only commits when every ally has claimed it (and abandons
   when enemies appear) might work. The naive version lost.
