# v15 (frozen copy of the first ladder submission): Cambridge Battlecode 2026a, Python

An aggressive farming bot. It clears the best-value camps, fights whenever its side is
roughly even or better, focus-fires, finishes off gem carriers, and banks gems before it
can lose them.

## v15 results (all 9 pool maps, both sides)

| Test | Record | Mean gem margin |
| --- | --- | --- |
| v15 vs v14 (bot fixes + new draft) | 26/36 | +239 |
| v15 bot fixes only, v14 draft, vs v14 | 24/36 | +243 |
| v15 vs v15 bot with the v14 draft (draft only) | 26/36 | +192 |

How champions spent their turns (18 games, v15 vs v14):

| | v14 | v15 |
| --- | --- | --- |
| Productive (fighting, capturing, banking, PvP) | 42–52% | 52–62% |
| Dead | 21–24% | 12–20% |
| Within 2 tiles of the map edge | 12–15% | 2–4% |

## v14 results (local, 8 of the 9 pool maps, both sides; "rows" not tested)

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

**Draft:** Ren, Tavi, Ryth, Stellar, Marcella. Maps draft 3 (collision, expanse, packs),
4 (interlock, rows) or 5 (corners, elkring, lanes, monolith) champions. Chosen by:

1. Slot test (888 games, `results/draft-slots.txt`): each untested champion (Cassi & Issac,
   Stellar, Prism, Vesper Vale, Ren, Willow) swapped into each of the 5 slots of the v14
   lineup, against that lineup. Best: Stellar for Varg (+352), Ren for Vendetta (+172),
   Stellar for Vendetta (+171). Every swap for Tavi lost. Willow and Cassi & Issac never
   helped; Prism was neutral at best.
2. Ordering test (288 games, `results/draft-orders.txt`): 8 orders of Ren/Stellar/Tavi/Ryth,
   all +14 to +232 against v14's lineup.
3. Round robin of the top four orders (6 pairings × 54 games): Ren, Tavi, Ryth, Stellar,
   Marcella won (+101 total; never lost a pairing).

**Each turn, per champion** (first match wins):

1. Dead: shop (Monster strength emblem, then Jungle health emblem, then Power strength
   shards), and buy an altar lock.
2. Channelling (recall, tribute): let it finish.
3. **On spawn and below 95% HP: stay put for one turn.** Spawn heals fully at the end of the
   turn. Before v14, champions left spawn the same turn a recall landed. v15: only one
   turn, because an enemy poking from outside the spawn kept a champion standing there
   (46 turns seen), and it hits back at anything in range meanwhile.
4. Below 30% HP (45% when carrying 30+ gems): recall. If an enemy is within 5 tiles, walk
   or dash home away from them instead (damage cancels a recall). v15: if HP dropped since
   last turn and an enemy is in attack range, shield and hit it first (`LOWHP_FIGHT`).
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
   v15: a reward an ally has claimed, or not seen for 25 rounds, is not chased.
9. Holding 150+ gold with an empty bag: recall to shop (tested: 22/32). v15: not when the
   next camp is within 7 tiles or an enemy was seen within 7 tiles.
10. Choose a camp group (every 3 rounds, never mid-fight): gems + 0.15·gold divided by
    travel + clear time + capture + half the altar distance. Camps too strong to solo
    need N champions; they wait up to 12 turns for help, then give up for 50 rounds
    (v15: the give-up is no longer erased when the camp is seen again, and no new joint
    camp is started for 40 rounds). v15: a champion that needs help joins a joint camp a
    higher-priority ally claimed instead of waiting at its own (two weak champions used to
    wait for each other at different camps), and banks its gems while helpers arrive.
    Joining a joint target an ally has claimed gets a 1.5× bonus. Targets are shared
    through memory. Single-camp groups count their full damage (`SOLO_CAMP_FULL`).
11. Carrying gems: tribute at 60, or sooner if the altar is closer than the next camp.
12. Nothing to do: recall if hurt or holding 60+ gold, otherwise go to an altar and guard it.

**Fighting camps (v15):** in range, shield (Varg), cast, attack. Out of range, step first,
then attack whatever is in range, and cast only if we could not move: damage spells are
exclusive with movement, so casting first used to cost the step. Ranged champions now
attack after stepping (they never did). An attack reset is not wasted on a camp the first
hit killed. Equal-length paths prefer tiles away from the map edge.

**Abilities:** Willow heals the ally that gains most (allies only; not on spawn). Prism's
passive adds only 0.2 focus per gem carried (measured), so it banks at 60 like the others;
its spells already use the higher focus.

## Tested and rejected

| Idea | Result |
| --- | --- |
| Cautious fighting (engage only at 1.3× power) | lost 4–12 to the aggressive version |
| Penalising camps nearer the enemy spawn | lost to contesting everything |
| Rally calls (fighting ally summons others) | 4/16, pulls champions off farming |
| Full-team monolith assault | 2/8 on the monolith map |
| Engage ratio 0.6 or 1.0, gem cap 40, retreat at 40%, no locks, no interrupts | all neutral |
| Shop-recall at 100 or 250 gold | slightly worse than 150 |
| Walk to and wait beside respawning camps (`MAX_WAIT` 25) | 9/18, −76; champions stood idle |
| Skip a far camp for 100 rounds after retreating from it (`FAIL_MEMORY`) | +104 vs fixed bot's +122 |
| Count damage spells when judging solo clears (`SPELL_DPS`) | +100 vs +122 without |
| Prism holds up to 100 gems (passive scaling) | scaling too small: +0.2 focus per gem |

## Testing tools (run from the workspace root)

```
python3 scripts/arena.py my-bot OTHER [--seeds N] [--maps a,b] [--quiet]
    # every map, both sides; prints wins, mean gem margin, faults.
    # --seeds N adds N seeded copies (SEED param) for more independent games.
scripts/variant.sh NAME 'PARAM=value' ...   # copy of my-bot with params changed
scripts/queue.sh BASE SEEDS v1 v2 ...       # each variant vs BASE -> logs/queue.txt
scripts/league.sh BOT [opponents...]        # vs a panel -> logs/league-BOT.txt
scripts/acts.sh REPLAY_DIR BOT              # what champions spent their time on
python3 scripts/draft_test.py slots|orders|report ...  # draft experiments, resumable
python3 scripts/lineup.py NAME A,B,C,...    # copy of my-bot with another draft order
python3 scripts/arena.py A B --out DIR      # keep replays; scripts/activity.py DIR summarises
                                            # turn use (needs bots that print DACTS lines)
```

`maps/` holds the 9 pool maps and `cambc.toml` makes the repo a cambc project, so a fresh
cloud session only needs `pip install cambc` and `cambc init . --yes` (for the SDK).

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
