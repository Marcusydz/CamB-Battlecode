# Project state

Updated 2026-10-20 16:10 UTC by Claude.

## Live and accepted bot
- Accepted branch: `top-team-study`. Bot source: `bots/cpp-bot` (C++). Version map: `VERSIONS.md`.
- Live on the ladder: official **96** = same source as official **92** (internal v68), commit `3fa9d89`. Best so far vs Nyx: 2773 (elk 864-360 W, CW 579, gold_rush 450, lanes 482, corners 398).
- Latest review-ready release: **official 68**, commit `070fd7a`, record `releases/official-68.md`.
  (GitHub tags/releases can't be created from Claude's session; release records are files.)

## Benchmarks of official 68 vs Nyx (5-game unranked groups; our gems vs theirs)
elk_arena 672-432 WIN, concentrated_wealth 377, lanes 412, gold_rush 583, corners 368, corridors 317,
expanse 578, islands 457, big_world 756, boss_battle gold 39.8k-54.4k. broken_curve unchanged (won 1013-901 as official 45).

## Known issues
- Top teams clear ~3x as many camp groups as we do; we lose most farm races by ~2x gems.
- gold_rush: our 3 champions are killed 27-44 times per game.
- Single panels are chaotic: an unrelated code change can move one map by +-150 gems.

## Current goal (Marcus)
Be close to the top teams on every map, not just win one. Keep broken_curve play unchanged.
Test opponent: Nyx only (Marcus 13:58; procless was dethroned).
