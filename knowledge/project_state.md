# Project state

Updated 2026-10-20 17:45 UTC by Claude.

## Live and accepted bot
- Accepted branch: `top-team-study`. Bot source: `bots/cpp-bot` (C++). Version map: `VERSIONS.md`.
- Live on the ladder: official **113** (internal v78), commit `e5d5b24`. vs Nyx: elk 864-360 W, CW 522, gold_rush 556, lanes 482, corners 398 (2822), corridors 418, expanse 790, islands 696, big_world 767. vs Fish: 1531 on the five standard maps (0-5).
- Latest review-ready release: **official 97** (v71), commit `4dd9930` (= `0f825d6`), record `releases/official-97.md`. Previous: official 68.
  (GitHub tags/releases can't be created from Claude's session; release records are files.)

## Benchmarks of official 68 vs Nyx (5-game unranked groups; our gems vs theirs)
elk_arena 672-432 WIN, concentrated_wealth 377, lanes 412, gold_rush 583, corners 368, corridors 317,
expanse 578, islands 457, big_world 756, boss_battle gold 39.8k-54.4k. broken_curve unchanged (won 1013-901 as official 45).

## Official 97/99 (v71) on other maps vs Nyx (vs official 68)
corridors 234-1070 (317), expanse 790 (578), islands 696 (457), big_world 808 (756), boss_battle gold 48.2k-53.2k (39.8k-54.4k). corridors is now the weakest map: we earn 2.3k gold to Nyx's 8.3k.

## v78 on more maps vs Nyx (official 113)
contest 0-1000 (Nyx kills us 57 times), monolith 460-1020, hooks 22-1012 (37 deaths), fragment 46-1000 (39 deaths), boss_battle gold tiebreak. hooks and fragment are now the worst maps.

## Known issues
- Top teams clear ~3x as many camp groups as we do; we lose most farm races by ~2x gems.
- gold_rush: our 3 champions are killed 27-44 times per game.
- Single panels are chaotic: an unrelated code change can move one map by +-150 gems.

## Current goal (Marcus)
Be close to the top teams on every map, not just win one. Keep broken_curve play unchanged.
Test opponent: Nyx only (Marcus 13:58; procless was dethroned).
