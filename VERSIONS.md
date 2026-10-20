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
| 87 | v65 | 42a42aa | v56 + walk home instead of recalling when hit by an unseen attacker (except 40x40). vs Nyx: elk WIN 792-360, CW 579 (v56 377), gold_rush 445 (583), lanes 341 (412), corners 398 (368); total 2555 vs 2412. Accepted. Live. |
