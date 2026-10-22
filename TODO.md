# How to improve the bot

The live bot is `bots/main` (C++). Submitted versions are listed in `VERSIONS.md`.
Rules and API references: `docs/GameRules.md`, `docs/BotAPICpp.md`.

## Bugs

1. **Prism's gem cap comment was wrong** (now fixed): the rules give 1.5 focus per gem, not 0.2.
   A cap of 90 tested level with 60 in v1.1.0; worth retrying with smarter banking.

Fixed or tried in v1.1.0 (see `docs/versions/v1.1.0.md`): Heart of Ruin's draft slot (fixed),
hidden blockers (move retry, on), squad membership (`SQUAD_FIX`, lost, off).
Fixed in v1.2.0: a crash when sorting camps by danger (`sorted_by`, see `docs/versions/v1.2.0.md`).

## Next, after v1.5.0 (22 Oct; see `docs/versions/v1.5.0.md`, `docs/v1.5.0_findings.md`)

1. **Escapes fail.** Against 1680–1800 teams, 18.5 of 26 deaths a game happen while retreating below
   `LOW_HP`. Ideas: retreat towards nearby allies rather than straight home; Vesper Vale/Vendetta dash
   away earlier; Marcella (stun immune) turns and fights; recall only from bushes or behind walls.
2. **Deaths alone (8.6 a game) and in pairs.** Try larger squads on core maps now that the lineup is
   duelists; the "hunter" sparring bot (aggressive settings) did not reproduce the ladder, so test on
   unranked groups (`scripts/groups.py`).
3. **broken_curve** is the one map with a negative local margin (4/12, −24).
4. **Counter-drafting** was tried as a fitted model (synergy and counter terms) and lost to the fixed
   list; the ladder model's counters are small (±0.04). Only revisit with a specific, large effect.
5. **Read the death log** on new ladder games: `cambc matches download`, then `scripts/deaths.py`.

## Earlier, from the ladder (21 Oct, 260 ranked games; v1.2.0 era)

1. (Done in v1.5.0: Prism is no longer drafted first.) **Prism first loses: 8–29.** It's drafted first only on elk_arena, packs/gold_rush, shortcut and
   collision, among our worst maps. Try `PRISM_MAPS_LIST` = 17 (Vesper Vale first, 52–28) or 16 on the
   ladder; the local test of it was stopped before it finished.
2. **Other weak maps:** hooks 0–7, starfish 2–8, fragment 4–9.
3. **Counter-draft:** we win 34% when the enemy drafts Vesper Vale (44 games).
4. **`CONTEST`** improved the gem margin (+21) but not wins locally; a ladder try could settle it.
5. **v1.3.0 adaptive strategy:** compare ladder results against the fixed v1.2.0 draft and check whether
   map profiles, enemy counters, and score-aware behavior help across different opponents.

## Rules we don't use yet

- **Teleport crystals** (10 gold, 3 tiles over walls) are never bought.
- **Draft memory** (1024 bytes, 10x draft fuel, full map) is unused.
- **`set_priority`:** stunners first lost in v1.1.0 (`PRIORITY`); other orders untested.
- **Shared memory** slots 5 and 6 are free. Focus fire lost in v1.2.0; "enemy tributing at altar X" untested.
- Smaller: cancel a tribute when a fight turns; hide in bushes while gathering; Tavi selling at
  full refund to rebuild; Vendetta's cooldown reset on camp kills. (Wolf kiting and cancelling a
  recall for a kill tested level in v1.2.0.)

## Code health

- `Bot::setup()` sets over 15 behaviours per map, some flipped several times by later probe blocks.
  Fold them into one table per map. Many were fitted to a few games against two opponents.
- About a dozen switches are off (`KITE`, `MID_SQUAD`, `TEAM_PLAN`, `PACK`, `MAX_WAIT`,
  `MONO_KITE`, ...) but their code is still there. `TEAM_PLAN` lost badly in v1.2.0 (92/280).
- Camp ranges and stats are hard-coded at the top of `bot.cpp`; `camp::attack_range()` and
  `base_stats()` give them directly.

## Tried and rejected (Python era, do not redo without a new idea)

| Idea | Result |
| --- | --- |
| Whole team attacks monoliths on gem-scarce maps (`SCARCE_GEMS`) | 1–2 of 8 vs v15: enemy took the reward |
| Gold mode on gem-scarce maps (`GOLD_MODE`) | 0 of 7 vs v15: backing off fed the enemy kills |
| Cassi & Issac pair logic (`LINKED`) | C&I lineups still lost to our draft on 3-pick maps |
| Waiting beside respawning camps (`MAX_WAIT`) | 9/18, −76 vs v14 |
| Skip a camp after a failed trip (`FAIL_MEMORY`) | worse than without |

Later probes (v43–v138) are recorded in the comments of `bot.cpp` and `draft.cpp`.

## Useful tools

- `python3 scripts/ladder.py [--since 2026-10-17T14:32]`: every ladder map with result, score and
  the opponent's draft (needs network access to 2026a.battlecode.co.uk).
- `python3 scripts/arena.py A B [--maps a,b]`: local head-to-head on every map in `maps/`.
- `python3 scripts/carena.py A B --seeds N`: C++ head-to-head with seeded variation.
- `python3 scripts/farmspeed.py BOT...`: rounds to 1000 gems against `bots/passive`.
- `scripts/cvariant.sh NAME 'PARAM=value'`: copy of `bots/main` with switches changed.
- `scripts/league.sh BOT`: arena against `passive` and `aggro`.
- `results/ladder-versions.txt` (local only, not in git): when each version was uploaded to the ladder.
- v1.5.0 tools (ladder sampling, draft model, unranked batches, death log): see `docs/versions/v1.5.0.md`.
