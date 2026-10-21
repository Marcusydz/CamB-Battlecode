# Project state

Updated 2026-10-20 20:05 UTC by Claude.

## Live and accepted bot
- Accepted branch: `top-team-study`. Bot source: `bots/cpp-bot` (C++). Version map: `VERSIONS.md`.
- Live on the ladder: official **179** (internal v110b = v109 + fight threshold 0.8 on the_farm and 1.3 on starfish). Nyx changed its bot again ~02:30 UTC 10-21: we now lose all 30 maps to it (9309 gems total, elk 0-1008 with 50 deaths). v109 = v108 + squad size per map: 2 on starvation, interlock, reverse_spiral, rushbait_v2; one squad of 5 on split, pilgrimage, quadrants, fracture, expanse, shortcut; +1141 Nyx+Fish there). v108 = v107 + pre-v90 contested-camp rule also on monolith, the_farm, collision, packs/gold_rush, rushbait_v2, lanes, eight, interlock: +853 Nyx+Fish there). v107 = v106 + death avoidance also on lanes, stampede, swirl, CW, corridors: Nyx+Fish +527 there, chosen by the 30-map v107 probe). v106: v104b + treasure first on 8 more maps incl. big_world (won vs Fish) and elk; v104b = v102 + death avoidance on pilgrimage and corners + pre-v90 contested-camp rule on starfish, starvation, quadrants, fragment; pilgrimage won 873-521), commit on top-team-study: v85 + v90 (contested-camp penalty after round 300 only on single-herd maps) + treasure emblem first on rows, lanes, corners, collision, eight, split, pilgrimage, monolith, quadrants, hooks, swirl (each chosen by a repeatable test vs Nyx). vs Nyx: elk 720-432 W, rows 1005-991 W, packs 876, split 746, gold_rush 611, CW 554, pilgrimage 490, lanes 478, monolith 455, corridors 430, quadrants 419, fracture 406, corners 372, interlock/shortcut 365/381, expanse 703, islands 598, big_world/ambush 940/789. Weakest: hooks 44, swirl 45, fragment 40, collision 148, starfish 63, starvation 134, reverse_spiral 112. vs Fish (v90): 2047 on CW, expanse, lanes, gold_rush, islands.
- Latest review-ready release: **official 97** (v71), commit `4dd9930` (= `0f825d6`), record `releases/official-97.md`. Previous: official 68.
  (GitHub tags/releases can't be created from Claude's session; release records are files.)

## Benchmarks of official 68 vs Nyx (5-game unranked groups; our gems vs theirs)
elk_arena 672-432 WIN, concentrated_wealth 377, lanes 412, gold_rush 583, corners 368, corridors 317,
expanse 578, islands 457, big_world 756, boss_battle gold 39.8k-54.4k. broken_curve unchanged (won 1013-901 as official 45).

## Official 97/99 (v71) on other maps vs Nyx (vs official 68)
corridors 234-1070 (317), expanse 790 (578), islands 696 (457), big_world 808 (756), boss_battle gold 48.2k-53.2k (39.8k-54.4k). corridors is now the weakest map: we earn 2.3k gold to Nyx's 8.3k.

## v78 on more maps vs Nyx (official 113)
contest 0-1000 (Nyx kills us 57 times), monolith 460-1020, hooks 22-1012 (37 deaths), fragment 46-1000 (39 deaths), boss_battle gold tiebreak. hooks and fragment are now the worst maps.
New-pool maps (v78): rows 922-1003, pilgrimage 342, split 386, quadrants 394, swirl 15-181, shortcut 0 (v85: 196), interlock 224 (v85: 289),, stampede 0-960, starfish 65-1012, starvation 128-984.

## New map pool (35 maps, installed locally 2026-10-20 from Marcus's files)
- stampede (60x50): every camp gives 0 gems except one 20-camp herd (str 360, 160 gems) at the far right, 58 steps from the altar. Unopposed we first bank at round ~937. Vs Nyx we never collect it; Nyx banks it 6 times and hunts our gold farmers.
- starfish (50x50): a 14-camp herd (118 gems, str 360) sits on the altar. Unopposed we still die 7 times to it (retreating at 40% hp).
- starvation (80x80): 12-camp herd (88 gems) next to the altar, plus 2 monoliths. Vs Nyx we died 36 times there.
- Opponent: Nyx and Fish (Marcus 16:44: Fish is back online).

## Known issues
- Nyx results with identical code vary ~150 gems per map between runs (elk 864 vs 720, CW 522 vs 348): judge on totals over many maps.
- Fights: Nyx deals ~10x our champion damage on brawl maps (contest, hooks, fragment, swirl); kill gold snowballs into power shards. v82 (stay in fights) and v83 (hit anything in reach) both lost elsewhere.
- Top teams clear ~3x as many camp groups as we do; we lose most farm races by ~2x gems.
- gold_rush: our 3 champions are killed 27-44 times per game.
- Single panels are chaotic: an unrelated code change can move one map by +-150 gems.

## Current goal (Marcus)
Be close to the top teams on every map, not just win one. Keep broken_curve play unchanged.
Test opponents: Nyx and Intercontinental Fish (Marcus 16:44).
