# How to improve the bot

Status on 17 October 2026: **v16** is on the ladder (`results/my-bot-v16-fight-submission.zip`).
Ranked since upload: 6 of 9 matches won, 23 of 45 maps (51%), rating steady around 1530–1540.
The v15 run just before it: 0 of 5 matches, 6 of 25 maps, rating falling 1564 → 1525.

We lose mostly to teams drafting **Cassi & Issac + Stellar + Varg** (often with Ren). Ladder
evidence: PtitLK 0–5 twice, Brr Brr Battlecode 0–5, Team name 0–5, Forged Token 1–4,
Phishing Season 1–4, Pikachu 1–4. Two causes show up again and again:

1. **They fight better.** On fight-heavy maps (eight, fragment, swirl, collision) our champions are
   dead 30–55% of the game.
2. **They farm faster.** On maps with no fighting (rows, expanse, monolith) they reach 1000 gems
   30–50% sooner. Cassi & Issac are one pick but two champions, so on 3- and 4-pick maps they have
   one more farmer than us.

## To do, in order

### 1. Faster farming
- Measure: our bot vs a passive opponent, rounds to 1000 gems on every map (rows, expanse,
  monolith first). Top teams finish rows by round ~370; we need 500+.
- Improve camp choice, banking timing (how many gems before tributing), walking routes, and the
  item build.
- Fully testable locally, helps on every map. Estimated 150–250k tokens.

### 2. Stay together in contested areas
- Champions die farming alone and get picked off by 2–3 enemies.
- Pair up near the map centre or near recently seen enemies; spread out to farm on safe ground.
- Partly testable locally (must not lose to v15); the real test is the ladder. ~100–150k tokens.

### 3. A sparring bot that plays like the top teams
- Our bot with their draft (Cassi & Issac, Stellar, Varg, Ren) plus focus fire and stun chaining.
- Makes fight changes testable here instead of on the ladder. ~200–300k tokens.

### 4. Smaller items
- **Huge maps** (ambush, 100×100): ~50% of turns walking. Prefer camp clusters near the altar.
- **Draft:** every top team has Varg + Stellar. Retest Varg in our lineup now that his shield is
  also used against camps.
- **Monoliths:** PtitLK scored 1000 gems from monoliths alone on contest. Retry the team assault
  (`SCARCE_GEMS`) once fighting is better; it lost when tried (the enemy stole the reward).
- **Rare crash:** 1 of 110 ladder maps ended with "our team errored" (collision, round 148). Logs
  show no Python error, so likely the sandbox killed the bot. Not reproduced locally.

## Tried and rejected (do not redo without a new idea)

| Idea | Result |
| --- | --- |
| Whole team attacks monoliths on gem-scarce maps (`SCARCE_GEMS`) | 1–2 of 8 vs v15: enemy took the reward |
| Gold mode on gem-scarce maps (`GOLD_MODE`) | 0 of 7 vs v15: backing off fed the enemy kills |
| Cassi & Issac pair logic (`LINKED`) | C&I lineups still lost to our draft on 3-pick maps |
| Waiting beside respawning camps (`MAX_WAIT`) | 9/18, −76 vs v14 |
| Skip a camp after a failed trip (`FAIL_MEMORY`) | worse than without |
| Count recently seen enemies (`GHOSTS`) | +104 vs v15, less than back-off alone (+130) |
| Retreat at 40% HP | +79 vs v15, less than without |

## Useful tools

- `python3 scripts/ladder.py [--since 2026-10-17T14:32]`: every ladder map with result, score and
  the opponent's draft (needs network access to 2026a.battlecode.co.uk).
- `python3 scripts/arena.py A B [--seeds N] [--maps a,b]`: local head-to-head on all 25 maps.
- `scripts/variant.sh NAME PARAM=value`: copy of my-bot with switches changed.
- `results/ladder-versions.txt`: when each version was uploaded to the ladder.
- Full history of fixes and tests: `bots/my-bot/README.md`.
