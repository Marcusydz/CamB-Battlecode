# Review-ready release: official 97 (v71)

**Review-ready major release for Codex.** Cambridge official submission **97** (internal label v71). Exact submitted source: `bots/cpp-bot` at commit `4dd9930` (branch `exp/v71`); the same code with one corrected comment is `0f825d6` on `top-team-study`. The same source is live again as official 107 (resubmitted after rejected experiments; see `VERSIONS.md`).

### Changes since official 68 (previous review-ready release, v56)
- Walk home when hit by an unseen attacker (v65): a hurt champion hit by something it could not see used to start a recall, have it cancelled by the next hit and restart it every turn, standing still 6-8 turns until dead. Now it walks toward spawn while it was hit in the last round.
- Parting shot (v68): while backing off, disengaging or leaving when outnumbered, a champion also hits whatever enemy is in attack range. Chasers (often a lone Ren at range 2-3) keep pace with a retreating champion, which used to just walk and die.
- Soft back-off threshold (v71): back off without recalling while carrying fewer than 20 gems (was 10).
- All of the above are off on 40x40 maps, so broken_curve plays as official 45/52/68.

### Benchmarks vs Nyx (5-game unranked groups, our submitted gems vs theirs; deterministic per bot pair)
| Map | official 97 | official 68 |
|---|---|---|
| elk_arena | **864-360 WIN** | 672-432 WIN |
| concentrated_wealth | 579-1043 | 377-1058 |
| lanes | 482-1010 | 412-1020 |
| gold_rush | 556-1006 | 583-1001 |
| corners | 398-1008 | 368-1007 |
| corridors | 234-1070 | 317-1007 |
| expanse | 790-1037 | 578-1040 |
| islands | 696-1004 | 457-1001 |
| big_world | 808-1031 | 756-1013 |
| boss_battle (gold tiebreak) | 48.2k-53.2k gold | 39.8k-54.4k gold |

Five-map total (first five rows): 2879 vs 2412. Ten-map gems (excluding boss_battle): 6407 vs 5521.

vs Intercontinental Fish (back online 2026-10-20): 0-5; elk 0-1008, CW 212-1063, lanes 585-1008, gold_rush 402-1007, corners 209-1057; they killed our champions 98 times, we killed 1.

### Known weaknesses
- corridors: one shared altar in a 3-wide corridor between two bush rows. Our champions channel at (14,18) beside the bush row and die mid-channel with no enemy in vision (Stellar from 1125 hp in one round), dropping 58-102 gems that Nyx collects. Tried and rejected: skipping the altar while enemies are seen near it (no change), banking only from tiles away from bushes (corridors +73, islands and elk much worse), breaking the channel when hit hard (no change; the kills are one- or two-round bursts).
- Nyx still earns 1.5-4x our gold and finishes first on every map except elk_arena.
- elk_arena swings with small code changes; judge on totals.
- Rejected after official 68 (see `VERSIONS.md` and `knowledge/strategy_history.md`): ENGAGE_RATIO 1.3, solo farming, Nyx's draft, treasure emblem first, OUTNUMBER 1, no contest penalty on the elk middle, leaving at 1.5x enemy power, LOW_HP 0.5, shy Willow, parting shot when fleeing/low hp, spending every unused attack on an enemy in reach.

### Questions for review
1. What hits a champion channelling beside a bush for 1000+ damage in one round without being seen, and how should we bank on a contested shared altar (corridors)?
2. The parting shot helped but spending every unused attack hurt (it drew fights). Is there a principled rule for when to trade hits while moving?
3. How do Nyx and Fish earn 2-4x our gold? Fish also wins nearly every fight (98 kills to 1).
