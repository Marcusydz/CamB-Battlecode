# Review-ready release: official 68 (v56)

**Review-ready major release for Codex.** Cambridge official submission **68** (internal label v56). Exact submitted source: `bots/cpp-bot` at commit `070fd7a` (branch `top-team-study`). The same source is live again as official 78 (resubmitted after rejected experiments; see `VERSIONS.md`).

### Changes since official 52 (previous review-ready release, v46)
- Draft (v49): Heart of Ruin, Varg, Stellar, Ryth, Willow on every map except 40x40 (`draft.cpp` list 16).
- Fight power (v53): casters (Cassi, Issac, Stellar, Ren, Tavi, Prism) count focus fully and strength at 0.25; others the reverse. Before, focus counted 0.25 for everyone, so focus-stacking casters looked weak and we started fights we lost.
- Back off without recalling (v54): when disengaging or outnumbered, a champion with >= 60% health and < 10 gems steps out of reach and keeps farming instead of recalling to spawn.
- `ENGAGE_RATIO` 0.8 -> 1.0 (v56).
- All of the above are off on 40x40 maps, so broken_curve plays as official 45/52.

### Benchmarks vs Nyx (5-game unranked groups, our submitted gems vs theirs; deterministic per bot pair)
| Map | official 68 | official 57 (v49, before the fight fixes) |
|---|---|---|
| elk_arena | **672-432 WIN** | 72-1008 |
| concentrated_wealth | 377-1058 | 143-1007 |
| lanes | 412-1020 | 613-1008 |
| gold_rush | 583-1001 | 583-1001 |
| corners | 368-1007 | 345-1028 |
| corridors | 317-1007 | not tested |
| expanse | 578-1040 | not tested |
| islands | 457-1001 | not tested |
| big_world | 756-1013 | not tested |
| boss_battle (gold tiebreak) | 39.8k-54.4k gold | not tested |
| broken_curve | not tested (Marcus: unchanged play, won 1013-901 as official 45) | |

Five-map total (first five rows): 2412 vs 1756.

### Known weaknesses
- Nyx earns 2-4x our gold (e.g. corridors 8.1k vs 1.7k, lanes 7.9k vs 1.9k) and clears about twice our camps under contest; unopposed our clear rate is close to Nyx's contested rate, so most of the gap comes from contact (deaths, retreats, lost camps).
- 10-30% of our champion turns are spent dead vs Nyx; most deaths carry no gems.
- Willow earns almost nothing and dies most, yet a draft without her (Nyx's draft) lost badly (1181 total).
- elk_arena swings with small code changes (from an 828-324 win to a 0-1008 loss); single panels are noisy.
- Rejected after this release: ENGAGE_RATIO 1.3, solo farming, Nyx's draft, treasure emblem first, gold weight 0.3-1.0 (see `knowledge/strategy_history.md`).

### Questions for review
1. Is the fight-power model (health x damage, caster focus) sound, and what would a better one count (items, cooldowns, healing)?
2. Where do we lose time under contest? Ideas welcome on retreat/recall rules and camp choice near enemies.
3. How does Nyx reach 2-4x our gold so early (its first emblems are durability and treasure)?
