# Project state

Updated 2026-10-20 19:05 UTC by Claude.

## Live and accepted bot
- Accepted branch: `top-team-study`. Bot source: `bots/cpp-bot` (C++). Version map: `VERSIONS.md`.
- Live on the ladder: official **115** (internal v78; 113 was the same source, 114 the rejected v79), commit `e5d5b24`. vs Nyx: elk 864-360 W, CW 522, gold_rush 556, lanes 482, corners 398 (2822), corridors 418, expanse 790, islands 696, big_world 767. vs Fish: 1531 on the five standard maps (0-5).
- Latest review-ready release: **official 97** (v71), commit `4dd9930` (= `0f825d6`), record `releases/official-97.md`. Previous: official 68.
  (GitHub tags/releases can't be created from Claude's session; release records are files.)

## Benchmarks of official 68 vs Nyx (5-game unranked groups; our gems vs theirs)
elk_arena 672-432 WIN, concentrated_wealth 377, lanes 412, gold_rush 583, corners 368, corridors 317,
expanse 578, islands 457, big_world 756, boss_battle gold 39.8k-54.4k. broken_curve unchanged (won 1013-901 as official 45).

## Official 97/99 (v71) on other maps vs Nyx (vs official 68)
corridors 234-1070 (317), expanse 790 (578), islands 696 (457), big_world 808 (756), boss_battle gold 48.2k-53.2k (39.8k-54.4k). corridors is now the weakest map: we earn 2.3k gold to Nyx's 8.3k.

## v78 on more maps vs Nyx (official 113)
contest 0-1000 (Nyx kills us 57 times), monolith 460-1020, hooks 22-1012 (37 deaths), fragment 46-1000 (39 deaths), boss_battle gold tiebreak. hooks and fragment are now the worst maps.
New-pool maps: rows 922-1003, stampede 0-960, starfish 65-1012, starvation 128-984.

## New map pool (35 maps, installed locally 2026-10-20 from Marcus's files)
- stampede (60x50): every camp gives 0 gems except one 20-camp herd (str 360, 160 gems) at the far right, 58 steps from the altar. Unopposed we first bank at round ~937. Vs Nyx we never collect it; Nyx banks it 6 times and hunts our gold farmers.
- starfish (50x50): a 14-camp herd (118 gems, str 360) sits on the altar. Unopposed we still die 7 times to it (retreating at 40% hp).
- starvation (80x80): 12-camp herd (88 gems) next to the altar, plus 2 monoliths. Vs Nyx we died 36 times there.
- Opponent: Nyx and Fish (Marcus 16:44: Fish is back online).

## Known issues
- Top teams clear ~3x as many camp groups as we do; we lose most farm races by ~2x gems.
- gold_rush: our 3 champions are killed 27-44 times per game.
- Single panels are chaotic: an unrelated code change can move one map by +-150 gems.

## Current goal (Marcus)
Be close to the top teams on every map, not just win one. Keep broken_curve play unchanged.
Test opponents: Nyx and Intercontinental Fish (Marcus 16:44).
