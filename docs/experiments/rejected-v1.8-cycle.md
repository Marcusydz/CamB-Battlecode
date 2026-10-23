# Rejected experiments after v1.8.1

Author: Copilot (AI assistant using Copilot SDK in VS Code).

This log records candidates that were tested and rejected so later cycles do
not repeat them without new evidence.

## v1.8.2 experimental candidates

| Candidate | Change | Local result | Online result | Decision |
|---|---|---:|---:|---|
| v282 countermaps | Anti-caster draft on starfish, elk-arena, hooks, and starvation | 41/80, +18 margin; weak maps 5/8, +177 | 8/15 vs 1712–1782 | Rejected after near-ELO 22/40, below v1.8.1 repeat baseline |
| v283 threat40 | `LOW_HP_ENEMY=0.40` | 43/80, +31 margin | 12/25 vs 1700–1800 | Rejected; deaths and submitted gems regressed |
| v284 gem25 | `LOW_HP_GEMS=0.25` | 41/80, +33 margin | 3/25 vs 1744–1776 | Rejected decisively |
| v285 combined | `LOW_HP_ENEMY=0.40`, `LOW_HP_GEMS=0.25` | 44/80, +41 margin | 5/25 vs 1700–1800 | Rejected; 17 deaths/game and 626 submitted gems |
| lowfight | `LOWHP_FIGHT_KILL_ONLY=0` | 33/80, −80 margin | Not submitted | Rejected |
| end700 | `ENDGAME_THEIRS=700` | 43/80, +14 margin | Not submitted | Rejected; no local improvement |
| Tavi fifth pick | Tavi before Ryth in the core draft | 39/80, +22 margin | Not submitted | Rejected |
| squad5 | `SQUAD=5` | 40/80, −3 margin | Not submitted | Rejected |
| support15 | `SUPPORT_BONUS=0.15` | 41/80, +10 margin | Not submitted | Rejected |
| support05 | `SUPPORT_BONUS=0.05` | 39/80, −3 margin | Not submitted | Rejected |
| starfish2 | Varg/Stellar/Prism/Ren/Ryth starfish draft | 39/80, −7 margin; weak maps 3/8 | Not submitted | Rejected |
| starfish3 | Ren/Varg/Stellar/Tavi/Ryth starfish draft | Weak maps 4/8, −7 margin | Not submitted | Rejected |
| squad2/3 | Smaller squads on weak maps | Weak maps 3/8, −112 margin | Not submitted | Rejected |

The successful release baseline remains v1.8.1: 48/80 near-ELO wins across
two batches. A controlled local support-heavy lineup test subsequently scored
13/16 for v1.8.1, so future work should prioritize map-conditioned positioning
and resource timing over another universal support counter.
