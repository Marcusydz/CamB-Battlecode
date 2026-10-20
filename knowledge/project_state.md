# Project state

Updated 2026-10-20 10:40 UTC by Claude.

## Live and accepted bot
- Accepted branch: `top-team-study`. Bot source: `bots/cpp-bot` (C++). Version map: `VERSIONS.md`.
- Live on the ladder: official **54** = same source as official **52** (internal v46), commit `1f4847e`.
- Latest review-ready release: **official 52**, commit `1f4847e`, record `releases/official-52.md`.
  (GitHub tags/releases can't be created from Claude's session; release records are files.)

## Benchmarks of official 52 (5-game unranked groups; our submitted gems vs theirs)
| Map | vs Nyx | vs procless |
|---|---|---|
| broken_curve | 1013-901 WIN | - |
| concentrated_wealth | 527-967 | 332-1062 |
| expanse | 547-1012 | 747-1008 |
| lanes | 346-1021 | 458-1002 |
| gold_rush | 594-1001 | 484-957 |
| boss_battle (gold tiebreak) | 36.7k-59.6k gold | 36.3k-41.0k gold |

## Known issues
- Top teams clear ~3x as many camp groups as we do; we lose most farm races by ~2x gems.
- gold_rush: our 3 champions are killed 27-44 times per game.
- Single panels are chaotic: an unrelated code change can move one map by +-150 gems.

## Current goal (Marcus)
Be close to the top teams on every map, not just win one. Keep broken_curve play unchanged.
Test against several strong opponents (Nyx, procless, Fish when it has a bot), not only Nyx.
