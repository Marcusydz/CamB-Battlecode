# Review-ready release: official 52 (v46)

**Review-ready major release for Codex.** Cambridge official submission **52** (internal label v46). Exact submitted source: `bots/cpp-bot` at commit `1f4847e` (branch `top-team-study`). Built and active on the ladder.

### Changes since official 44 (Codex review v44)
- Camp choice: walking time counts 3x in a camp's cost; gold valued at 0.1 gem (0 on gold_rush). 40x40 maps (broken_curve, interlock) keep v42's choice, since we beat Nyx on broken_curve with it.
- Codex v44 fixes accepted: SDK `distance_field::unreachable` sentinel, waiting band outside camp range.
- `LOW_HP` 0.3 -> 0.4 (retreat earlier).
- boss_battle: the only altar's only open side is a monolith tile, so nobody can bank and games end on gold. When every altar side is a camp tile, we value only gold and never walk to bank.

### Benchmarks (5-game unranked groups, our submitted gems vs theirs; games are deterministic per bot pair)
| Map | vs Nyx (#2) | vs procless (#1) |
|---|---|---|
| broken_curve | 1013-901 WIN (official 51, same choice) | not tested |
| concentrated_wealth | 527-967 | 332-1062 |
| expanse | 547-1012 | 747-1008 |
| lanes | 346-1021 | 458-1002 |
| gold_rush | 594-1001 | 484-957 |
| boss_battle (gold tiebreak) | 36.7k-59.6k gold | 36.3k-41.0k gold |

### Known weaknesses
- Nyx and procless clear about 3x as many camp groups as we do (expanse: 91 vs 28). They circle small groups near their spawn as they respawn, including 0-gem gold clusters, and convert the gold into items early.
- Concentrated_wealth: Nyx takes the 71-gem middle group (21 camps, strength 60) about 8 times per game. We hardly contest it.
- gold_rush: with 3 champions (Varg, Stellar, Prism) we are killed 27-35 times. Drafting Cassi & Issac for a 4th body was worse.
- Tiny code changes reshuffle results on some maps (lanes 520 -> 346 from a change that only runs on boss_battle), because fuel is counted per instruction. Single 5-game panels are noisy in that sense.

### Strategic questions worth investigating
1. A farming policy that circulates nearby respawning groups (route planning over respawn timers) instead of greedy value/cost per trip.
2. Should we contest big central groups (concentrated_wealth) as a full team?
3. Fight logic on small maps (gold_rush): when to group and when to avoid the enemy team.
4. Item purchase timing: Nyx buys from round ~20 onward in many small purchases.
