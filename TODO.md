# How to improve the bot

The live bot is `bots/main` (C++). Submitted versions are listed in `VERSIONS.md`.
Rules and API references: `docs/GameRules.md`, `docs/BotAPICpp.md`.

## Bugs

1. **Heart of Ruin has no draft slot.** The `draft_id` switch in `Bot::setup()` has no case for
   `CT::heart_of_ruin`, so it falls into the catch: its shared-memory slot becomes `handle % 8`
   (can collide with an ally's claim slot or Issac's slot 7) and `team_size` becomes 3. Affects
   every map using draft list 16 or 18, where Heart of Ruin is the first pick.
2. **Squad membership drops anyone without a claim.** `squad_alive()` counts a slot as alive only
   if it holds a claim, so a champion retreating, idle or recalling leaves its squad and squads
   reshuffle.
3. **Prism's gem cap is based on a wrong number.** The comment on `PRISM_GEM_CAP` says +0.2 focus
   per gem; the rules say 1.5 (60 gems = +90 focus on a base of 80). Re-test a higher cap.
4. **Hidden blockers waste the step.** `step_towards` / `step_static` try one tile; when
   `move_absolute` returns false (unseen unit in a bush) they don't try the next best tile, though
   the failed attempt costs nothing.

## Rules we don't use yet

- **Move, reset, attack for ranged champions.** An attack reset restores the attack allowance, so
  Vesper Vale, Tavi and Willow can step and still hit. `fight()` steps, then `try_attack` fails.
- **Heart of Ruin's dash** deals 1.5x focus and stuns in radius 1 on landing; we only use it to walk.
- **Teleport crystals** (10 gold, 3 tiles over walls) are never bought.
- **Counter-drafting:** enemy picks are revealed during the draft but never read (e.g. Marcella is
  stun-immune).
- **Draft memory** (1024 bytes, 10x draft fuel, full map) is unused.
- **`set_priority`** is unused: order stunners before damage dealers.
- **Shared memory** slots 5 and 6 are free: focus target, "enemy tributing at altar X".
- **Contesting enemy captures:** entering the region resets their 5-round timer.
- Smaller: cancel recall or tribute when a fight turns; hide in bushes while gathering; Tavi
  selling at full refund to rebuild; casters' 4-range spells outrange wolves (range 3);
  Vendetta's cooldown reset on camp kills.

## Code health

- `Bot::setup()` sets over 15 behaviours per map, some flipped several times by later probe blocks.
  Fold them into one table per map. Many were fitted to a few games against two opponents.
- About a dozen switches are off (`KITE`, `MID_SQUAD`, `TEAM_PLAN`, `PACK`, `MAX_WAIT`,
  `MONO_KITE`, ...) but their code is still there.
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
