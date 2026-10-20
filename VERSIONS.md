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
| 154 | v101 candidate | ea7d36b (branch exp/v101) | v100 + treasure first on collision and eight. (the_farm left out: treasure first there gets 25 gems unopposed.) Testing vs Nyx. |
