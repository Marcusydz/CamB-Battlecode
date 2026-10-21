# Submitted versions

Official ladder version numbers and the commit that holds the exact submitted source (`bots/cpp-bot`). Accepted versions live on `top-team-study`; test candidates on `exp/*` branches.

| Official | Name | Commit | Notes |
|---|---|---|---|
| 44 | v41 | c10db83 | ignore gold when choosing camps; stay off contested camps early |
| 45 | v42 | 588abf3 | v41 + retreat at 40% health; first win vs Nyx (1/5, broken_curve 1013-901). |
| 46 | v42 diagnostic | 588abf3 with `ACT_LOG_EVERY = 400` | temporary, activity logging only; same results vs Nyx as 45 |
| 47 | v43 candidate | f7eda8d (branch exp/v43) | walking counts 3x in camp cost, gold valued at 0.1 gem; vs Nyx 0/5, lost broken_curve |
| 48 | v43 | 671d5de (branch exp/v43; same source on top-team-study) | 47 + v42's camp choice on 40x40 maps; vs Nyx 1/5, gems 468/1013/547/442/594 (CW, broken_curve, expanse, lanes, gold_rush), total 3064 vs v42 2676. |
| 49 | v44 candidate | aeb6a37 (branch exp/v44) | gold_rush draft Varg, Cassi & Issac, Stellar: gold_rush 446 vs 594; rejected |
| 50 | v43 | 5b905b5 (same bot source as 48) | v43 restored after 49 |
| 51 | v45 | a086d87 | v43 + Codex v44 fixes (SDK unreachable sentinel, waiting band); vs Nyx CW 527, expanse 547, lanes 520, gold_rush 594, broken_curve 1013 (win). |
| 52 | v46 | 1f4847e | v45 + gold mode when every altar side is a camp tile (boss_battle: only altar is behind the boss, games end on gold). boss_battle gold 36k (v45 ~3k unopposed) vs Nyx 60k, procless 41k. vs Nyx CW 527, expanse 547, lanes 346, gold_rush 594; vs procless CW 332, expanse 747, lanes 458, gold_rush 484. Review-ready release: releases/official-52.md. |
| 53 | v47 candidate | caa0764 (branch exp/v47) | contest middle camps (CONTEST_FACTOR 1.0): vs Nyx CW 290, expanse 573, lanes 320, gold_rush 506, islands 476; vs procless 207/662/464/357, islands 292. 4-map totals 1689/1690 vs v46 2014/2021: rejected |
| 54 | v46 | 1f4847e (same bot source as 52) | v46 restored after 53. |
| 55 | v48 candidate | 8cf9365 (branch exp/v48) | squad followers wait for helpers: vs procless corridors 110 (v46 44), elk_arena 0 (0), CW 209 (332), lanes 518 (458), gold_rush 484 (484); total 1321 vs 1318, neutral: rejected |
| 56 | v46 | 1f4847e (same bot source as 52) | v46 restored after 55. |
| 57 | v49 | d631fcf | post-patch draft (Heart of Ruin, Varg, Stellar, Ryth, Willow) on all maps except 40x40 (broken_curve unchanged). vs procless CW 239 (v46 332), lanes 549 (458), gold_rush 440 (484), elk_arena 0 (0), corners 463 (339); total 1691 vs 1613; faster unopposed on most maps. Accepted. |
| 58 | v50 candidate | 9211172 (branch exp/v50) | hub maps (elk_arena) move as one team of 5 and wait at the middle: vs procless elk 0 (40 deaths), lanes 447, rest same; unopposed elk never reaches 1000. Rejected |
| 59 | v49 | d631fcf (same bot source as 57) | v49 restored after 58. |
| 60 | v51 candidate | 6422450 (branch exp/v51) | procless's item build (monster emblem, durability emblem of health, bruiser shards forever) except on 40x40: vs procless lanes 478 (v49 549), CW 190 (239), gold_rush 495 (440), corners 276 (463), elk 0; total 1439 vs 1691. Rejected |
| 61 | v49 | d631fcf (same bot source as 57) | v49 restored after 60. |
| 62 | v52 candidate | 1a4d082 (branch exp/v52) | gold weight 1.0 on hub maps (elk_arena): vs procless elk 0 (42 deaths), lanes 447 (v49 549, fuel chaos), rest identical; total 1589 vs 1691. Rejected |
| 63 | v49 | d631fcf (same bot source as 57) | v49 restored after 62. |
| 64 | diagnostic | 29d9076 (branch exp/diag-act) | v49 with activity log every 200 rounds, vs Nyx only; same results as 63 |
| 65 | v49 | d631fcf (same bot source as 57) | v49 restored after 64. |
| 66 | v53 | 073a079 | fight power counts casters' focus fully (except 40x40). vs Nyx: elk_arena WIN 828-324 (v49 72-1008), CW 270 (143), gold_rush 610 (583), lanes 390 (613), corners 253 (345); total 2351 vs 1756. Accepted. |
| 67 | v54 | f608616 | v53 + healthy champions with < 10 gems back off without recalling (except 40x40). vs Nyx: elk WIN 612-540 (v53 828-324), CW 405 (270), gold_rush 610 (610), lanes 418 (390), corners 324 (253); total 2369 vs 2351. Accepted. |
| 68 | v56 | 070fd7a | v54 + ENGAGE_RATIO 1.0 (0.8 on 40x40). vs Nyx: elk WIN 672-432, CW 377 (405), gold_rush 583 (610), lanes 412 (418), corners 368 (324); total 2412 vs 2369. Accepted (small gain). |
| 69 | v57 candidate | b8e5625 (branch exp/v57) | ENGAGE_RATIO 1.3: vs Nyx elk 0-1008 (lost), CW 253, gold_rush 693, lanes 401, corners 283; total 1630. Rejected |
| 70 | v56 | 070fd7a (same bot source as 68) | v56 restored after 69. |
| 71 | v58 candidate | 35123db (branch exp/v58) | solo farming (no squads) except 40x40: vs Nyx elk 288-864 (lost), CW 266, gold_rush 583, lanes 302, corners 405; total 1844 vs 2412. Rejected |
| 72 | v56 | 070fd7a (same bot source as 68) | v56 restored after 71. |
| 73 | v59 candidate | ec0506f (branch exp/v59) | enemy power +30% per Varg/Willow: vs Nyx results identical to 68 (no effect). Rejected |
| 74 | v56 | 070fd7a (same bot source as 68) | v56 restored after 73. |
| 75 | v60 candidate | d402440 (branch exp/v60) | Nyx's draft (HoR, Ren, Stellar, Ryth, Tavi) except 40x40: vs Nyx elk 0, CW 189, gold_rush 605, lanes 118, corners 269; total 1181. Rejected |
| 76 | v56 | 070fd7a (same bot source as 68) | v56 restored after 75. |
| 77 | v61 candidate | 5b17798 (branch exp/v61) | treasure emblem first except 40x40: vs Nyx elk 510-642 (lost), CW 262, gold_rush 627, lanes 319, corners 358; total 2076 vs 2412. Rejected |
| 78 | v56 | 070fd7a (same bot source as 68) | v56 restored after 77. |
| 79 | v62 candidate | 0056b2e (branch exp/v62) | OUTNUMBER 1 except 40x40: vs Nyx elk 72-1008 (lost), CW 351, gold_rush 561, lanes 439, corners 395; total 1818. Rejected |
| 80 | v56 | 070fd7a (same bot source as 68) | v56 restored after 79. |
| 81 | v63 candidate | 2002f8e (branch exp/v63) | no contest penalty on elk_arena's middle group: vs Nyx elk 144-1008 (57 deaths), other 4 maps identical to 68. Rejected |
| 82 | v56 | 070fd7a (same bot source as 68) | v56 restored after 81. |
| 83 | v64 candidate | 7bad316 (branch exp/v64) | leave when visible enemies have 1.5x our power (not 40x40/elk): vs Nyx elk 288-1008 (lost, fuel chaos), CW 460, gold_rush 594, lanes 357, corners 390; farm maps 1801 vs 1740. Rejected (elk loss) |
| 84 | v56 | 070fd7a (same bot source as 68) | v56 restored after 83. |
| 85 | diagnostic | 88c8cdf (branch exp/diag-death) | v56 printing the 8 turns before each death; vs Nyx only |
| 86 | v56 | 070fd7a (same bot source as 68) | v56 restored after 85. |
| 87 | v65 | 42a42aa | v56 + walk home instead of recalling when hit by an unseen attacker (except 40x40). vs Nyx: elk WIN 792-360, CW 579 (v56 377), gold_rush 445 (583), lanes 341 (412), corners 398 (368); total 2555 vs 2412. Accepted. |
| 88 | v66 candidate | 61ad076 (branch exp/v66) | LOW_HP 0.5 except 40x40: vs Nyx elk 216-864 (lost), CW 246, gold_rush 583, lanes 529, corners 205; total 1779. Rejected |
| 89 | v65 | 42a42aa (same bot source as 87) | v65 restored after 88. |
| 90 | v67 candidate | 8876b4c (branch exp/v67) | Willow fights only beside two allies, never joins (except 40x40): vs Nyx elk 432-720 (lost), CW 516, gold_rush 445, lanes 280, corners 494; total 2167. Rejected |
| 91 | v65 | 42a42aa (same bot source as 87) | v65 restored after 90. |
| 92 | v68 | 3fa9d89 | Parting shot: hit anything in range while backing off or disengaging (except 40x40). vs Nyx: elk 864-360 WIN, CW 579, gold_rush 450, lanes 482, corners 398; total 2773 (v65 2555). Accepted. |
| 93 | v69 candidate | d654e51 (branch exp/v69) | Parting shot also when fleeing or at low hp (except 40x40): vs Nyx elk 612-468 WIN, CW 547, gold_rush 450, lanes 482, corners 398; total 2489. Rejected |
| 94 | v68 | 3fa9d89 (same bot source as 92) | v68 restored after 93. |
| 95 | v70 candidate | d30a38a (branch exp/v70) | Unused attack hits an enemy champion in reach at end of every turn (except 40x40): vs Nyx elk 360-792 (lost), CW 547, gold_rush 528, corners 350 (lanes summary missing). Rejected |
| 96 | v68 | 3fa9d89 (same bot source as 92) | v68 restored after 95 (first try hit the 20 uploads per hour limit). |
| 97 | v71 | 0f825d6 (submitted 4dd9930, same code, comment fixed after) | Soft back-off while carrying < 20 gems (was 10; except 40x40): vs Nyx elk 864-360 WIN, CW 579, gold_rush 556, lanes 482, corners 398; total 2879 (v68 2773; only gold_rush changed). Accepted. |
| 98 | v72 candidate | 46d7242 (branch exp/v72) | Soft back-off while carrying < 30 gems (except 40x40): vs Nyx identical to v71 on all 5 maps (2879). Rejected (no effect) |
| 99 | v71 | 0f825d6 (same bot source as 97) | v71 restored after 98. |
| 100 | v73 candidate | f6bf08e (branch exp/v73) | Don't walk to an altar enemies are waiting at; keep farming, bank once clear (except 40x40). vs Nyx: elk 468-540 (lost), CW 213, gold_rush 561, lanes 482, corners 398 (2122); corridors unchanged 234. Rejected |
| 101 | v71 | 0f825d6 (same bot source as 97) | v71 restored after 100. |
| 102 | diag-death2 | 1b01cc8 (branch exp/diag-death2) | Diagnostic: v71 printing the 8 turns before each death (temporary, never kept live). |
| 103 | v71 | 0f825d6 (same bot source as 97) | v71 restored after 102. |
| 104 | v74 candidate | 638f69c (branch exp/v74) | Bank from altar tiles not beside a bush when there are any (except 40x40): vs Nyx elk 288-576 (lost), CW 320, gold_rush 534, lanes 483, corners 331; corridors 307 (+73), expanse 623, islands 277, big_world 685. Rejected |
| 105 | v71 | 0f825d6 (same bot source as 97) | v71 restored after 104. |
| 106 | v75 candidate | 8dd1e8f (branch exp/v75) | Break a tribute channel when hit for 25%+ of max hp and below 60% hp (except 40x40). vs Nyx identical to v71 on all 10 maps (the altar deaths are one- or two-round bursts). Rejected (no effect) |
| 107 | v71 | 0f825d6 (same bot source as 97) | v71 restored after 106. |
| 108 | v76 candidate | b1a5456 (branch exp/v76) | Recall to shop at 90 gold instead of 150 (except 40x40 and islands). vs Nyx: elk 756-324 WIN, CW 400, gold_rush 627, lanes 575, corners 317 (2675 vs 2879); corridors 74, expanse 746, big_world 845, islands same. Rejected |
| 109 | v71 | 0f825d6 (same bot source as 97) | v71 restored after 108. |
| 110 | v77 draft | e1f8ba5 (branch exp/v77) | ETA_W 5, SPREAD 0.5 everywhere; replaced within a minute because SPREAD also changed 40x40 maps. Not tested. |
| 111 | v77 candidate | 8dfc7fb (branch exp/v77) | ETA_W 3 -> 5 (40x40 keeps 1) and SPREAD 0.25 -> 0.5 except 40x40; unopposed farm sweep mean 1245 -> 1202 rounds (CW 1665 -> 1305). vs Nyx: elk 288-576 (lost), CW 251, gold_rush 528, lanes 347, corners 391 (1805 vs 2879); corridors 380 (+146), expanse 662, islands 452, big_world 861. Rejected |
| 112 | v71 | 0f825d6 (same bot source as 97) | v71 restored after 111. |
| 113 | v78 | e5d5b24 | Give up on a camp after 12 failed approach steps even with steps in between (except 40x40); unopposed corridors Stellar/Willow oscillated 600+ turns beside a walled-off camp. vs Nyx: elk 864-360 WIN, CW 522, gold_rush 556, lanes 482, corners 398 (2822); corridors 418 (v71 234), expanse 790, islands 696, big_world 767, boss gold same; ten-map gems 5493 vs 5407. vs Fish 1531 vs 1408 (CW 335 vs 212). Accepted. |
| 114 | v79 candidate | 239f8db (branch exp/v79) | No monolith targets (except 40x40): unopposed on contest the team died 80 times at the two 20000-hp monoliths. vs Nyx: contest still 0-1000 (Nyx won sooner, round 1489 vs 1773, killing us 62 times), monolith 460, hooks 22, fragment 46, boss same as v78. Rejected |
| 115 | v78 | e5d5b24 (same bot source as 113) | v78 restored after 114. |
| 116 | v80 candidate | 827c616 (branch exp/v80) | Big herds (10+ camps) use CAMP_RISK 1.0 instead of 1.5 (except 40x40). Unopposed: stampede 2292 -> 1987 rounds, starvation 2378 -> 1984, starfish 1260 -> 1338. vs Nyx: stampede 0-1120 (v78 0-960), starvation 140 (128), starfish 97 (65), rows 960 (922, unaffected), contest 0-1000 (same); Nyx kills us 22-49 times per game, we kill 0-3. Within noise. Rejected |
| 117 | v78 | e5d5b24 (same bot source as 113) | v78 restored after 116. |
| 118 | v81 candidate | e9008cf (branch exp/v81) | Gem-scarce maps (only contest): farm gold (weight 1.0) until round 600, then take monoliths with 3. Unopposed contest 3666 -> 1412 rounds. vs Nyx contest still 0-1000 (Nyx kills our gold farmers 64 times; our champions dealt 9k damage to champions, theirs 106k). Same-as-v78 maps: pilgrimage 342-1055, split 386 (r742), quadrants 394 (r486), swirl 30-1005 (43 deaths). Rejected (no gain vs Nyx) |
| 119 | v78 | e5d5b24 (same bot source as 113) | v78 restored after 118. |
| 120 | diag-fight | 46b5147 (branch exp/diag-fight) | Diagnostic: v78 plus death context and a fight log line per turn near enemies. Not a candidate. |
| 121 | v78 | e5d5b24 (same bot source as 113) | v78 restored after 120. |
| 122 | v82 candidate | a330f71 (branch exp/v82) | Commit to a fight for 3 rounds after engaging (down to 0.6x engage ratio), except 40x40. vs Nyx: elk 0-1008 (v78 864-360 W), CW 321, lanes 376, gold_rush 567, corners 339 (1603 vs 2822); contest 0, hooks 0 (22), swirl 68 (181), fragment 64 (68), stampede 160 (160). Rejected: committing to fights loses them, elk_arena especially. |
| 123 | v78 | e5d5b24 (same bot source as 113) | v78 restored after 122. |
| 124 | diag-fight2 | 30723c5 (branch exp/diag-fight) | Diagnostic: fight log plus chosen action and hits per fight turn. Not a candidate. |
| 125 | v78 | e5d5b24 (same bot source as 113) | v78 restored after 124. |
| 126 | v83 candidate | ca03f50 (branch exp/v83) | Engage: if the focus target is out of reach after moving, hit whoever is in reach (except 40x40). diag-fight2: ~80% of engage turns hit nothing. vs Nyx: elk 468-684 (v78 864-360 W), CW 348, lanes 335, gold_rush 550, corners 398 (2099 vs 2822); stampede 640-640 tie (v78 0-960, 12 kills), contest 0, hooks 22, swirl 15, fragment 50. Rejected |
| 127 | v78 | e5d5b24 (same bot source as 113) | v78 restored after 126. |
| 128 | v85 candidate | 044db2d (branch exp/v85) | Freeze only broken_curve (40x40, 274 walls), not every 40x40 map: shortcut and interlock now get normal play. broken_curve identical locally. Local h2h vs v78: wins both sides on shortcut and interlock. vs Nyx: interlock 289-1024 (v78 224), shortcut 196-1019 (v78 0-1058); unaffected maps elk 720-432 WIN, CW 348, corridors 430 (same code as v78: Nyx games vary ~150 gems run to run). Accepted. |
| 129 | v78 | e5d5b24 (same bot source as 113) | v78 restored after 128 for a baseline on shortcut/interlock: shortcut 0-1058, interlock 224-1004, pilgrimage 342-1055, split 386, quadrants 394. |
| 130 | v85 | 73660fb (same bot source as 044db2d) | v85 accepted (see 128). vs Fish 1598 (v78 1531). New maps vs Nyx: ambush 940-1021, packs 518, fracture 406, elkring 396, yellow_brick_road 387, the_farm 264, reverse_spiral 157, collision/eight 86 and 0. |
| 131 | v86 candidate | c94fc7d (branch exp/v86) | Before round 150, engage only at 1.5x power (except broken_curve). vs Nyx: elk 432-792 (lost), CW 348, lanes 469, gold_rush 539, corners 382 (2170 vs 2822). Rejected |
| 132 | v85 | 73660fb | v85 restored after 131. |
| 133 | v87 candidate | 66fe781 (branch exp/v87) | Treasure emblem first (except Tavi, broken_curve), as Fish/Sponge/Nyx do. Unopposed 7-map mean 1304 -> 1268 rounds. vs Nyx: elk 324-756 (lost), CW 363, gold_rush 660, corners 332, lanes 466 (2145 vs 2822); rows 1005-991 WIN (v78 922), islands 482 (696), expanse 647 (790), big_world/ambush 833 and 648 (767, 940). Rejected |
| 134 | v85 | 73660fb | v85 restored after 133. |
| 135 | v88 candidate | 02419d5 (branch exp/v88) | Draft HoR, Ren, Stellar, Ryth, Varg (top-3 core). Unopposed 8-map mean 1291 -> 1386 (slower). vs Nyx: elk 468-684, CW 380, lanes 316, gold_rush 484, corners 223 (1871 vs 2822); rows 924, islands 534, expanse 534, big_world/ambush 610 and 670 (3272 vs 4115). Rejected |
| 136 | v85 | 73660fb | v85 restored after 135. |
| 137 | v89 candidate | 7861dac (branch exp/v89) | Top-3 package together: HoR/Ren/Stellar/Ryth/Varg draft + treasure first + shop at 90 gold (except broken_curve). vs Nyx: elk 0-1008, CW 359, lanes 301, gold_rush 539, corners 272 (1471 vs 2822); rows 934, islands 712, expanse 696, big_world/ambush 438 and 652 (3432 vs 4115). Rejected |
| 138 | v85 | 73660fb | v85 restored after 137. |
| 139 | v90 candidate | eadefef (branch exp/v90) | Contested-camp penalty (x0.3) after round 300 only on single-herd maps (elk-type, broken_curve); elsewhere only the first 300 rounds. Local vs v85: ~13-7. vs Nyx: elk/CW/lanes/gold_rush/corners 2488 vs 2822 (1 win, 720-432); expanse/islands/big_world/rows/ambush 3990 vs 4115. First rejected, then accepted as official 143 after a fresh v85 baseline (2340 / 3641) showed the old baselines were stale. |
| 140 | v85 | 73660fb | v85 restored after 139. |
| 141 | v91 | 49869d4 | Bug fix: enemies on their own spawn are skipped unless killable now (they out-heal our hits; corridors vs passive had 2 champions stuck 800 rounds). Local farm speed mean 1551 -> 1538 rounds, no map worse beyond 15, broken_curve identical. vs Nyx: set a 2241 (v85 2822), set b 3641 (v85 4115; rows 474, 60-map 478). Rejected: same as fresh v85 except gold_rush 624 vs 720. |
| 142 | v85 | 73660fb | v85 restored after 141. Fresh baseline vs Nyx: set a 2340 (gold_rush 720, 335, 539, 348, 398), set b 3641; games now repeat exactly, so per-map comparisons are reliable. |
| 143 | v90 | 38fff25 | v90 accepted (vs fresh v85: +148 set a, +349 set b). |
| 144 | v92 candidate | 8f89be7 (branch exp/v92) | v90 + early contest window 150 rounds (was 300) on multi-herd maps. Local vs v90 ~9-7. vs Nyx: 2163 / 3666 (v90 2488 / 3990; rows 525 vs 703, elk 272 vs 280). Rejected. |
| 145 | v90 | 38fff25 | v90 restored after 144. |
| 146 | v93 candidate | 876046e (branch exp/v93) | v90 + gold weight 0.3 (was 0.1): Nyx earns 2.7x our camp gold on rows. Unopposed 1253 -> 1228 rounds. vs Nyx: 2221 / 3412 (v90 2488 / 3990; worse on 8 of 10 maps). Rejected. |
| 147 | v90 | 38fff25 | v90 restored after 146. |
| 148 | v95 candidate | 163be63 (branch exp/v95) | v90 + DEATH_AVOID 120: after enemy champions kill us, avoid camps within 8 tiles for 120 rounds (corners: 17 deaths in the middle). Local vs v90 ~9-7. vs Nyx: 2340 / 3868 (v90 2488 / 3990): lanes +52, corners +63, but CW -108, gold_rush -83, elk 648 vs 720, islands -118. Rejected. |
| 149 | v90 | 38fff25 | v90 restored after 148. vs Fish (CW, expanse, lanes, gold_rush, islands): 2047 (v85 earlier 1598). |
| 150 | v96 candidate | f74d23b (branch exp/v96) | v90 + treasure emblem first except elk-type maps and broken_curve (v87 lost only elk). Unopposed 1270 -> 1249, elk identical. vs Nyx: 2494 / 3573 (v90 2488 / 3990). Won rows 1005-991, lanes +155, corners +92; CW -191, islands -206, big_world+ambush -248. Rejected. |
| 151 | v90 | 38fff25 | v90 restored after 150. |
| 152 | v100 candidate | 7ab8c86 (branch exp/v100) | v90 + treasure emblem first on rows, lanes, corners only (where v96 beat v90). Other maps identical to v90 locally. vs Nyx: 2735 / rows WON 1005-991 (v90 2488, rows 960-1003); other maps identical (islands game not played by the group). Accepted; live since this upload. |
| (152) | v100 | 4d49ece | Accepted onto top-team-study (same source as 7ab8c86). Live as official 152. |
| 153 | v96 again | f74d23b (branch exp/v96) | Treasure first everywhere except elk-type/broken_curve, to pick treasure-first per map on 10 more maps (v100 baseline there: corridors 430, starfish 63, starvation 134, stampede 640-640, the_farm 264, fracture 406, reverse_spiral 112, collision 22, eight 104, packs 876). v96 there: corridors 424, starfish 87, starvation 124, stampede 640-640, the_farm 535, fracture 380, reverse_spiral 68, collision 148, eight 219, packs 538. |
| 154 | v101 candidate | ea7d36b (branch exp/v101) | v100 + treasure first on collision and eight. (the_farm left out: treasure first there gets 25 gems unopposed.) vs Nyx set d: 1761 vs 1520 (collision 148, eight 219), rest identical. Accepted. |
| (154) | v101 | 2f6ed2a | Accepted onto top-team-study. Official 154. Baseline vs Nyx: hooks/quadrants/split/swirl 386, 30, 394, 22 (45x35, 45x30, 45x45, 31x31), pilgrimage 371, monolith 368, fragment 40, rushbait_v2 157, interlock/shortcut 365 and 381. |
| 155 | v96 again | f74d23b (branch exp/v96) | Treasure-first probe on 10 more maps. Quadrants 419, split 746, hooks 44, pilgrimage 490, swirl 45, monolith 455, fragment 44, rushbait_v2 105, interlock/shortcut 237 and 0. |
| 156 | v102 candidate | c30069d (branch exp/v102) | v101 + treasure first on split, pilgrimage, monolith, quadrants, hooks, swirl. vs Nyx exactly as the probe: set e 1744 vs 1203, set f 1398 vs 1311. Accepted. |
| (156) | v102 | aee550d | Accepted onto top-team-study. Live as official 156. |
| 157 | v103 probe | 9d2be05 (branch exp/v103p) | v102 + DEATH_AVOID 120 (not elk-type/broken_curve), probed on 30 maps to choose per map. Won pilgrimage 873-521, corners 487 vs 372; worse on packs (448 vs 876), lanes, CW, gold_rush, reverse_spiral, swirl, quadrants. |
| 158 | v103 candidate | bbb6f57 (branch exp/v103) | v102 + DEATH_AVOID on pilgrimage and corners only. vs Nyx: pilgrimage WIN 873-521, corners 487; lanes 478, split 746, elk 720 W unchanged. Accepted. |
| (158) | v103 | bd6f944 | Accepted onto top-team-study. Live as official 158. |
| 159 | v104 probe | dccd04f (branch exp/v104p) | v103 with the pre-v90 contested-camp penalty (mid-game when enemies seen near) on every map, probed on 30 maps to choose per map. Won starfish 239 vs 63, starvation 208 vs 134, quadrants 444 vs 419, fragment 50 vs 40; worse on pilgrimage (-530), packs, expanse, CW, eight, shortcut. |
| 160 | v104 candidate | a202979 (branch exp/v104) | v103 + pre-v90 contested-camp rule on starfish, starvation, quadrants, fragment only. vs Nyx: starfish 144, starvation 124, fragment 20, quadrants 514, pilgrimage (unchanged code) 564-1014 vs 873-521 before. Rejected pending recheck. |
| 161 | v103 | 50ca47b | v103 restored after 160 (v104 did not repeat its probe: pilgrimage control 564 vs 873). Recheck: v103 repeats exactly (pilgrimage 873-521, starfish 63, starvation 134, quadrants 419, fragment 40), so v104 itself changed pilgrimage (likely per-turn fuel use in the camp-scoring loop). |
| 162 | v104b candidate | 345b043 (branch exp/v104b) | Same per-map rule as v104 with a precomputed flag (identical loop work). vs Nyx exactly as probed: starfish 239, starvation 208, quadrants 444, fragment 50, pilgrimage 873-521 W. Accepted. |
| (162) | v104b | 929bfb1 | Accepted onto top-team-study. Live as official 162. |
| 163 | v105 probe | 18e1b66 (branch exp/v105p) | v104b + gold weight 0.3 via a setup flag on every map except broken_curve, probed on 30 maps. Total 10353 vs 13987; won only islands 660, fragment 70, rows 1017, swirl 53. |
| 164 | v105 candidate | 6669fd0 (branch exp/v105) | v104b + gold weight 0.3 on islands, fragment, rows, swirl. Did not reproduce: rows 915-1003 (lost the win), pilgrimage 564 (was 873 W), islands 454, fragment 26, swirl 15. Rejected. |
| 165 | v104b | 7d9e497 | v104b restored after 164. Baselines (new Nyx): 10156 vs Nyx, 7929 vs Fish on 30 maps. |
| 166 | v106 probe | 1a348c7 (branch exp/v106p) | v104b + treasure first on every map but broken_curve, probed on 30 maps vs Nyx and Fish. Nyx 9940 vs 10156, Fish 8478 vs 7929; won big_world 1024-876 vs Fish (first Fish win), elk 936 vs Nyx; per-map gains on islands, packs, rushbait_v2, CW, expanse, gold_rush; losses on corridors, ambush, fracture, interlock. |
| 167 | v106 candidate | 6425c18 (branch exp/v106) | v104b + treasure first on big_world, elk_arena, islands, packs/gold_rush, rushbait_v2, CW, expanse. Exactly as probed: Nyx 5818 vs 5284, Fish 4535 vs 3895 on the 10 maps; big_world WIN vs Fish. Accepted. |
| (167) | v106 | e15c8a2 | Accepted onto top-team-study. Live as official 167. |
| 168 | v107 probe | a4664b4 (branch exp/v107p) | v106 + death avoid on every map but broken_curve, probed on 30 maps vs Nyx and Fish. Nyx 9128 vs 10690, Fish 8092 vs 8569 overall (elk 0 vs 936, rushbait -361); per-map gains on lanes +273, stampede +160 (first Nyx gems there), swirl +83, CW +7, corridors +4. |
| 169 | v107 candidate | e01d379 (branch exp/v107) | v106 + death avoid also on lanes, stampede, swirl, CW, corridors. Reproduced exactly: Nyx+Fish sum +527 on the 5 maps (lanes 563/484, stampede 160 vs Nyx, swirl 91 vs Fish); 5 controls (elk WIN 936-216, rows, big_world, gold_rush, pilgrimage) identical. Accepted. |
| (169) | v107 | 53c167b | Accepted onto top-team-study. Live as official 169. |
| 170 | v108 probe | 2188fab (branch exp/v108p) | v107 + pre-v90 contested-camp rule on every map but broken_curve, probed on 30 maps vs Nyx and Fish. Nyx 11118 vs 11030, Fish 8721 vs 8756; gains on monolith +250, the_farm +155, collision +148, gold_rush +126, packs +70, rushbait +40, lanes +27, eight +19, interlock +18; losses on islands -296, pilgrimage -185, corners -140. |
| 171 | v108 candidate | 6be730c (branch exp/v108) | v107 + pre-v90 contested-camp rule also on monolith, the_farm, collision, packs/gold_rush, rushbait_v2, lanes, eight, interlock. Reproduced exactly: Nyx+Fish +853 on the 9 maps (monolith 717 vs Nyx, collision 211 vs Fish, the_farm 258 vs Fish); 6 controls (elk WIN, rows, big_world, pilgrimage, CW, swirl) identical. Accepted. |
| (171) | v108 | bd5d9a1 | Accepted onto top-team-study. Live as official 171. |
| 172 | v109 probe | 369a1c7 (branch exp/v109p) | v108 + squads of 2 (was 3) on every map but broken_curve, probed on 30 maps vs Nyx and Fish. Unopposed about equal. Much worse overall (Nyx 9114 vs 11366, Fish 8465 vs 9273); won split +188, starvation +168, interlock +163, reverse_spiral +52, quadrants +51. |
| 173 | v110 probe | c6a0936 (branch exp/v110p) | v108 + one squad of 5 on every map but broken_curve, probed on 30 maps vs Nyx and Fish. Worse overall (Nyx 9916, Fish 8914); won split +234, pilgrimage +164, quadrants +127, fracture +125, expanse +38, shortcut +26. |
| 174 | v109 candidate | d53cbc4 (branch exp/v109) | v108 + squads of 2 on starvation, interlock, reverse_spiral, rushbait_v2 and one squad of 5 on split, pilgrimage, quadrants, fracture, expanse, shortcut. Reproduced exactly: Nyx+Fish +1141 on the 10 maps (split 684/764, pilgrimage 697 vs Nyx, quadrants 665 vs Fish, starvation 196/112); 5 controls (elk WIN, rows, lanes, gold_rush, monolith) identical. Accepted. |
| (174) | v109 | 21dddc2 | Accepted onto top-team-study. Live as official 174. |
| 175 | v110 engage probe | fa41e7c (branch exp/v110e) | v109 + engage ratio 1.3 (was 1.0) on every map but broken_curve, probed on 30 maps vs Nyx and Fish. Worse overall (Nyx 10668 vs 11557, Fish 9067 vs 10223); small wins on starfish +62, big_world +52, fragment +34. |
| 176 | v110 aggressive probe | bcf1bf5 (branch exp/v110a) | v109 + engage ratio 0.8 (was 1.0) on every map but broken_curve (already 0.8 there), probed on 30 maps vs Nyx and Fish. Much worse (Nyx 7892, Fish 8945; elk 288, rushbait -623); won the_farm +85, big_world +45, fragment +35. |
| 177 | v110 candidate | fdaa22d (branch exp/v110) | v109 + engage ratio 1.3 on starfish, big_world, fragment and 0.8 on the_farm. Fish games reproduced the probe exactly (starfish 38, the_farm 302) but every Nyx game changed, controls included (elk 936 to 0, split 684 to 348): Nyx changed its bot again ~02:30 UTC. Re-baselining vs the new Nyx. |
| 178 | v109 | 21dddc2 | v109 resubmitted (same source as official 174) to measure it vs the new Nyx on the 4 maps v110 changes. New Nyx vs v110 on 30 maps: 9309 gems (we lose all; elk 0-1008). v109 vs new Nyx: big_world 837 (v110 673), the_farm 11 (v110 77), starfish 163 (159), fragment 42 (48), elk 0. |
| 179 | v110b candidate | f9cfcd5 (branch exp/v110b) | v109 + engage ratio 0.8 on the_farm (+110 Nyx+Fish) and 1.3 on starfish (+11). Reproduced exactly vs new Nyx and Fish (the_farm 77/302, starfish 159/38); big_world, fragment, split identical to v109. Accepted. |
| (179) | v110b | 89caeab | Accepted onto top-team-study. Live as official 179. |
| 180 | v111 probe | a6bc6e7 (branch exp/v111p) | v110b + death avoid on every map but broken_curve, re-probed because the new Nyx (02:30 UTC) hunts us: on elk it kills us 50-0, deaths all near the middle. Nyx 9184 vs 9467, Fish 9870 vs 10282 overall; elk unchanged (0); won monolith +135, islands +116, interlock +111, hooks +44. |
| 181 | v111 candidate | ab6d2d9 (branch exp/v111) | v110b + death avoid also on monolith, islands, interlock, hooks. Reproduced exactly (+406 Nyx+Fish on the 4 maps), gold_rush control identical. Accepted. |
| (181) | v111 | 6e9540e | Accepted onto top-team-study. Live as official 181. |
| 182 | v112 probe | b73413d (branch exp/v112p) | v111 + one squad of 5 on every map but broken_curve, re-probed vs the new Nyx. Worse (Nyx 8748 vs 9790, Fish 9018 vs 10365); only rows won (+60). Rejected for now. |
| 183 | v111 | ecccf32 | v111 restored after the v112 probe. |
| 184 | v112 contest probe | 0868a36 (branch exp/v112c) | v111 + pre-v90 contested-camp rule on every map but broken_curve, re-probed vs the new Nyx. Worse overall; won split +230 (994-1004 vs Fish, nearly a win), corridors +124, expanse +107. |
| 185 | v112 candidate | 4d4828f (branch exp/v112) | v111 + old contest rule on split, corridors, expanse + one squad of 5 on rows. Reproduced exactly (+521 Nyx+Fish; split 994-1004 vs Fish), gold_rush control identical. Accepted. |
| (185) | v112 | 5116784 | Accepted onto top-team-study. Live as official 185. |
| 186 | v113 probe | 1e9a00a (branch exp/v113p) | v112 with the treasure opening flipped on every map but broken_curve (on where off, off where on), vs the new Nyx and Fish. Worse overall (Nyx 9462 vs 10087, Fish 7967 vs 10589) but won elk +576 (treasure off: 576 vs Nyx, was 0), pilgrimage +175, collision +140, expanse +113, the_farm +97, shortcut +62, hooks +44. |
| 187 | v113 candidate | 9c6a881 (branch exp/v113) | v112 + treasure opening off on elk, pilgrimage, collision, expanse, hooks and on for the_farm, shortcut. Fish matched the probe exactly, but Nyx controls moved (split 348 to 449, gold_rush 693 to 567): Nyx switched back to its pre-02:30 bot (official 188 check: v112 elk 936-216 WIN again). Not accepted yet; judging against both Nyx bots. |
| 188 | v112 | 74fe109 | v112 restored; vs Nyx (pre-02:30 bot again ~04:00): elk 936-216 WIN, pilgrimage 697-777, rows 960, gold_rush 567, split 449. |
| 189 | v113b candidate | 665b08a (branch exp/v113b) | v112 + treasure opening off on pilgrimage, collision, expanse, hooks and on for the_farm, shortcut (each positive summed over both Nyx bots + Fish); elk keeps treasure (keeps the 936-216 win vs the older Nyx). Fish games identical to the probe; elk won vs Nyx again; other Nyx games moved again (split 449 to 519, gold_rush 567 to 660, the_farm 245 to 453), so Nyx is changing its bot repeatedly. Accepted. |
| (189) | v113b | 52eb102 | Accepted onto top-team-study. Live as official 189. |
| 190 | v114 probe | ec326f8 (branch exp/v114p) | v113b + leave when 1 more enemy than allies is near (was 2), every map but broken_curve, vs Nyx and Fish. Worse (Nyx 9766 vs 10906 for v112 at 04:05, Fish 9160 vs 10748; elk 936 to 180). Only ambush looked better on both (+33 Fish, +264 Nyx). Rejected. |
| 191 | v113b | bdf0de5 | v113b restored after the v114 probe. |
| 192 | v115 probe | 048578d (branch exp/v115p) | v113b + retreat at 55% health (was 40%) on every map but broken_curve, vs Nyx and Fish. Much worse overall (Fish 6923 vs 10748) but rows WON vs Nyx 1005-972 (Fish 872 vs 806); elk still a win (720-504). (A top-teams draft probe, HoR/Ren/Stellar/Ryth/Varg, was dropped before upload: 14% slower unopposed.) |
| 193 | v115 candidate | c4cff6c (branch exp/v115) | v113b + retreat at 55% health on rows only. All 30 maps: Fish identical to v113b except rows 872 (+66), big_world still WIN 1024-876; vs Nyx rows WIN 1005-972, total 10885; elk 324 this time (Nyx keeps changing). Accepted. |
| (193) | v115 | ef3cd6f | Accepted onto top-team-study. Live as official 193. |
| 194 | v116 probe | 0e71002 (branch exp/v116p) | v115 with the old contested-camp rule flipped on every map but broken_curve (off where on, on where off), vs Nyx and Fish. Worse overall (Nyx 10145 vs 10885, Fish 9652 vs 10814); eight +107 (rule off), lanes +35, monolith +28. |
| 195 | v117 probe | a9ea521 (branch exp/v117p) | v115 with death avoidance flipped on every map but broken_curve, vs Nyx and Fish. Worse overall (Nyx 9490, Fish 7612); won pilgrimage +417 and lanes +219 (off), starfish +166, fragment +42, reverse_spiral +42 (on), hooks +44 (off). |
| 196 | v116 candidate | 09dff22 (branch exp/v116) | v115 + death avoidance off on pilgrimage, lanes, hooks and on for starfish, fragment, reverse_spiral; old contest rule off on eight. Verify run changed maps it does not touch, vs Fish too (ambush 787 to 614, big_world 1024 to 650), identical to the v117 probe there: Fish and Nyx both seem to have changed bots ~05:20 UTC, so the v117 probe compared against stale baselines. Re-baselining. |
| 197 | v115 | 322c7a6 | v115 restored to re-baseline against the current Nyx and Fish. Confirmed both changed ~05:20: v115 now Nyx 0 on elk, rows 979 (loss), big_world 650 vs Fish (loss). v116 vs this baseline: +197 Nyx+Fish, every changed map >= 0 (starfish +83, hooks +44, pilgrimage +30), all other maps identical. The v117 probe vs this baseline: -800. |
| 198 | v116 | 529c5e5 | Accepted onto top-team-study (same code as official 196, comments corrected). Live as official 198. |
| 199 | v118 probe | 75eefb1 (branch exp/v118p) | v116 with the treasure opening flipped on every map but broken_curve, vs the ~05:20 Nyx and Fish. Better overall (Nyx 10341 vs 10037, Fish 9646 vs 8201); big gains on the_farm, packs, elk, fracture, CW, corners, big_world, islands, shortcut. |
| 200 | v118 candidate | ba2640c (branch exp/v118) | v116 + treasure opening flipped on the_farm, packs/gold_rush, elk, fracture, CW, corners, big_world, islands, shortcut, interlock, stampede, expanse, monolith, eight. Reproduced exactly: every flipped map matched the probe and every other map matched v116 (2 Nyx groups did not start). About +2900 Nyx+Fish (the_farm 571 vs Nyx, packs 553, fracture 406/436, big_world 833 vs Fish, elk 288). Accepted. |
| (200) | v118 | aee7a91 | Accepted onto top-team-study. Live as official 200. |
| 201 | v119 probe | f41c2e7 (branch exp/v119p) | v118 with the old contest rule flipped on every map but broken_curve, vs the ~05:20 rivals. Worse overall (Nyx 11271 vs 11720, Fish 9126 vs 9733); won starvation +144, lanes +116, the_farm +106, eight +93, rushbait +77, monolith +74, CW +64. |
| 202 | v119 candidate | 9f4ce09 (branch exp/v119) | v118 + old contest rule flipped on starvation, lanes, the_farm, eight, rushbait_v2, monolith, CW. All 7 reproduced the probe exactly (+674); quadrants moved although untouched (Fish 719 to 373, Nyx 327 to 427), probably the extra setup work; net +547 Nyx+Fish (Nyx 12231, Fish 9769). Accepted. |
| (202) | v119 | 5cf83c1 | Accepted onto top-team-study. Live as official 202. |
| 203 | v120 probe | 538d91a (branch exp/v120p) | v119 with squad size rotated on every map but broken_curve (one squad of 5 where we use 2 or 3, squads of 3 where we use 5), vs the ~05:20 rivals. Worse overall (Nyx 10942, Fish 8809); won quadrants +326 (3), elk +288 (5), corridors +224 (5), rushbait +83 (5); rows with squads of 3 beat Nyx 1003-982. |
| 204 | v120 candidate | 81d1784 (branch exp/v120) | v119 + squads of 3 on quadrants and rows, one squad of 5 on elk, corridors, rushbait_v2. Reproduced exactly (+905 Nyx+Fish; rows WIN 1003-982 vs Nyx; quadrants back to 782 vs Fish); all other maps identical. Accepted. |
| (204) | v120 | 94a2d39 | Accepted onto top-team-study. Live as official 204. |
| 205 | v121 probe | 5ed172c (branch exp/v121p) | v120 drafting Vesper Vale instead of Heart of Ruin on every map but broken_curve (patch 1.0.2 nerfed HoR, buffed Vesper), vs Nyx and Fish. Won on 12 maps (pilgrimage +365 and a Nyx win 791-543, rows Nyx win 1017-991), lost split -690, corridors -700, fracture, quadrants, packs, lanes. |
| 206 | v121 (accepted) | 40d0838 | v120 + Vesper Vale instead of Heart of Ruin on pilgrimage, expanse, ambush, rushbait_v2, islands, stampede, starfish, monolith, eight, CW, rows, the_farm. Verified on all 30 maps: identical to the probe on those 12 maps, other 18 unchanged. Nyx 12701 -> 13787, Fish 10204 -> 11205. |
| 207 | v122 probe | f0417a7 (branch exp/v122p) | v121 with Vendetta (buffed in 1.0.2) drafted in Stellar's place (nerfed) on every map using the post-patch lists, vs Nyx and Fish. Lost 3300 overall (packs -551, lanes -410, pilgrimage -368); won clearly only on fracture +209, CW +186, islands +118. |
| 208 | v123 (accepted) | d0a2110 (branch exp/v123) | v121 + Vendetta instead of Stellar on fracture, CW, islands. Verified: those 3 maps reproduced (+513), all other Nyx games identical (Fish set e timed out; it doesn't use the change). |
| 209 | v124 probe | 588d8a6 (branch exp/v124p) | v123 drafting Prism (buffed in 1.0.2) instead of Heart of Ruin on HoR maps and instead of Varg on Vesper maps, vs Nyx and Fish. Lost about 8000 overall; won elk_arena +468 (beat Nyx 1044-252), packs +179, shortcut +113, collision +56. |
| 210 | elk probe 1 | 04886c5 (branch exp/elk1) | v123 with fight threshold 1.5 on elk_arena only, vs Nyx and Fish on elk. No result (runner stopped before its games were saved). |
| 211 | v125 candidate | 28e7d3d (branch exp/v125) | v123 + Prism first (list 20) on elk_arena, packs/gold_rush, shortcut, collision. Verifying on all 30 maps. |
