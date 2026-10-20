# Strategy history (significant results only)

Format: date - change - evidence - conclusion. Opponent results are 5-game unranked groups.

## Before the week-2 patch (old engine)
- 2026-10-19: C++ port, top-team item build, per-map drafts (v17). Lesson: draft tables tuned against our own old bot overfit.
- 2026-10-19/20: v19-v33, ~105 games vs Intercontinental Fish, 0 wins; best about 80% of Fish's gems on farm maps. Never submit with printf logging (costs ~15% farming).

## After the week-2 patch (2026-10-20)
- Fish's bot stopped compiling after the SDK change; test opponents are now Nyx (#2) and procless (#1).
- Squads of 3 that follow the first member's camp (v36): better vs Nyx than solo farming.
- Per-kind camp ranges for retreat danger and gather ring (v39): far fewer deaths to camps on lanes.
- GOLD_W 0 (v41) was faster unopposed than 1.0. Codex later showed 0 loses to 0.15 locally; current: 0.1 (0 on gold_rush and 40x40).
- LOW_HP 0.4 (v42, official 45): first win vs Nyx (broken_curve 1013-901). LOW_HP 0.45 was worse.
- Finding: Nyx clears 91 camp groups to our 28 on expanse by circling small near-spawn groups (including 0-gem gold clusters) as they respawn. On concentrated_wealth it farms the 71-gem middle group about 8 times.
- Walking cost x3 in camp choice + gold 0.1 (v43, official 48): +100-200 gems vs Nyx on concentrated_wealth, expanse and lanes. It lost broken_curve, so 40x40 maps keep v42's choice.
- Codex v44 fixes (SDK unreachable sentinel, waiting band) (official 51): accepted; concentrated_wealth +60, lanes +80 vs Nyx.
- boss_battle gold mode (v46, official 52): gold 3k -> 36k; still below Nyx/procless.
- Post-patch draft Heart of Ruin, Varg, Stellar, Ryth, Willow on all maps except 40x40 (v49, official 57): vs procless 1691 vs 1613 gems over elk, CW, lanes, gold_rush, corners.

## Tried and rejected (after the patch)
- Cassi & Issac in the gold_rush draft (official 49): gold_rush 446 vs 594.
- Contest middle camps, CONTEST_FACTOR 1.0 (official 53): 4-map totals 1689/1690 vs 2014/2021 (Nyx/procless).
- Unopposed farm-speed sweeps with no clear gain: MAX_WAIT 20-80, gem cap 30/120, item builds, PLAN_RADIUS 30, SPREAD 0.5, a big-group score bonus, the team-plan rewrite (slower everywhere).
- Unopposed farm speed is a weak proxy for contested games; judge versions on server games against 2+ strong opponents.
- Squad followers wait for helpers (official 55): neutral vs procless (1321 vs 1318).
- elk_arena team of 5 waiting at the middle for its respawn (official 58): still 0 gems vs procless; unopposed never reached 1000.
- procless's item build (monster emblem, durability emblem of health, bruiser shards forever; official 60): 1439 vs 1691 vs procless.
- Gold weight 1.0 on elk_arena (official 62): still 0 gems there.

## Findings vs procless (2026-10-20)
- procless clears about twice our camps (lanes 88 vs 45, gold_rush 233 vs 45) and earns 2-5x our gold (1000-3500 per champion; 5000-8000 on gold_rush), then stacks emblems (bruiser or power of strength, 8-30 of them).
- Its gem rate rises through the game while ours stays flat; we rarely die on farm maps (lanes, CW, corners), so the gap there is farming speed, not fights.
- elk_arena: all 72 gems are in the middle group. procless clears it with all five every 75-120 rounds and banks at its altar 3 tiles away; we lose the early middle fight and never collect a gem (35-42 deaths).

## Nyx again (procless dethroned, Marcus 2026-10-20 13:58)
- Logging build vs Nyx: 10-32% of champion turns were spent dead; most deaths carried 0 gems.
- Fight power counted focus at 0.25 for everyone, so focus-stacking casters (Nyx: Ren, Stellar, Cassi & Issac) looked weak. Counting casters' focus fully (v53, official 66) won elk_arena 828-324 (first elk win) and took the 5-map total from 1756 to 2351.
- Backing off without recalling when healthy with < 10 gems (v54, official 67): 2369. ENGAGE_RATIO 1.0 (v56, official 68): 2412. Both keep 40x40 maps unchanged.
- Rejected: gold weight 0.3/0.6/1.0 (slower unopposed, gold_rush never finishes).
- Rejected vs Nyx (5-map totals vs v56's 2412): ENGAGE_RATIO 1.3 1630 (elk lost 0-1008); solo farming, no squads 1844; Nyx's draft (HoR, Ren, Stellar, Ryth, Tavi) 1181; treasure emblem first 2076; +30% per enemy Varg/Willow no effect (Nyx drafts neither).
- Nyx's items: every champion has durability + treasure emblems, then stacks power emblems (17-42 on gold_rush/elk). Our Willow earns 0-285 gold and dies most, yet dropping her (draft change) lost badly.
- elk_arena vs Nyx swings with small code changes (828-324 win to 0-1008 loss); judge on several maps.
- Death diagnostic (official 85, prints 8 turns before each death): hurt champions hit by unseen attackers recalled, the hit cancelled the recall, and they restarted it every turn, standing still 6-8 turns until dead. Walking home instead (v65, official 87): 2555 vs 2412, elk win 792-360.
- Also rejected: OUTNUMBER 1 (1818), no contest penalty on the elk middle (elk 144-1008), leave at 1.5x visible enemy power (elk lost; farm maps +61). After v65 (vs 2555): LOW_HP 0.5 retreat 1779; Willow fights only beside two allies 2167 (elk lost 432-720).
- Parting shot (v68, official 92): chasers (often a lone Ren at range 2-3) keep pace with a retreating champion, so hit anything in range while backing off/disengaging: 2773 vs 2555 (lanes +141, elk 864-360 W). Extending it to flee/low-hp (v69) gave 2489: rejected. Spending every unused attack on an enemy in reach (v70) lost elk 360-792: rejected (extra hits draw fights).
- Soft back-off while carrying < 20 gems instead of < 10 (v71, official 97): 2879; only gold_rush changed (450 -> 556). < 30 gems: identical (no effect).
- corridors: one shared altar in the middle; our champions die beside it carrying 58-102 gems and Nyx picks them up. Skipping the altar while enemies are seen near it (v73) did not change corridors at all (the killers are unseen) and lost elk: rejected. Death log (official 102): they channel at (14,18) beside the bush row and die mid-channel with no enemy in vision (Stellar from 1125 hp in one round). Banking only from tiles not beside bushes (v74): corridors +73 but islands 696 -> 277 and elk lost: rejected. Breaking the channel when hit hard (v75): no effect.
- Economy: Nyx clears ~2x our camps even on expanse where we never die, buys from round ~40 in 30-gold steps (treasure shards first). Recall to shop at 90 gold instead of 150 (v76): 2675 vs 2879 (lanes/gold_rush up, CW/corners/corridors down): rejected.
- Unopposed local farm speed does not predict Nyx results: ETA_W 5 + SPREAD 0.5 was the fastest unopposed (mean 1245 -> 1202 rounds, CW 1665 -> 1305) but scored 1805 vs 2879 against Nyx (v77): rejected.
- Oscillation fix (v78, official 113): a champion stepping forward then failing reset its stuck counter, so Stellar and Willow sat 600+ turns beside a walled-off respawning camp on corridors. Giving up after 12 failed steps per target: corridors 234 -> 418, CW -57, ten-map +86; vs Fish 1408 -> 1531. Accepted.
- Monoliths (20000 hp, str 200, range 5 since the patch): unopposed on contest our team died 80 times attacking them (killed 3 in 2000 rounds). Never targeting them (v79) wins contest locally on gold but vs Nyx contest stayed 0-1000: rejected, the loss there is to Nyx's champions.

- New pool (35 maps, 2026-10-20 evening). Big herds (stampede 20 camps/160 gems, starfish 14 on the altar, starvation 12): camp risk 1.0 for 10+ camp groups (v80) was faster unopposed (stampede 2292 -> 1987 rounds) but within noise vs Nyx: rejected. Contest's only gems are two monoliths; farming gold until round 600 first (v81) took contest unopposed from 3666 to 1412 rounds, but vs Nyx contest stayed 0-1000 (they kill our gold farmers 64 times): rejected.
- Fight log (official 120, contest vs Nyx): at rounds 36-50 four of ours vs one Nyx champion flipped between engage and back-off as a recently seen hidden enemy entered/left the estimate (power scales with count squared); 3 of ours died, 0 kills. By round 900-1400 Nyx champions had strength 465-556 and focus 1000-1500 (ours 56-68 / 100-110): kill gold (100 each) buys power shards, so every early death snowballs. Committing to a fight for 3 rounds (v82) lost elk 0-1008 and 1603 vs 2822 on the standard maps: rejected. Holding fights is worse than the flip-flopping.
- Fight log 2 (official 124): only ~20% of engage turns hit an enemy champion (we walk after the lowest-hp target). Hitting whoever is in reach when the focus target is not (v83): stampede tied 640-640 (12 kills) but elk lost 468-684 and standard maps 2099 vs 2822: rejected. Logging itself changes play (elk lost 36-1044 with the diag build), so judge only on unlogged builds.
- 40x40 gate (v85, official 130, accepted): the "keep broken_curve unchanged" switch tested only the map size, so shortcut (new) and interlock also ran the frozen rules. Recognising broken_curve by its 274 wall tiles (camp groups are unknown during the draft, so not by its monoliths): broken_curve identical locally; vs Nyx shortcut 0 -> 196, interlock 224 -> 289.
- Top-team comparison (last 30 ranked games of Fish, Sponge, Nyx; /mnt/project-files/battlecode/analysis/top-teams-2026-10-20.md): all draft HoR + Ren + Stellar (+Ryth), open with treasure, shop by round 42-63 (ours 83), earn 1.0-1.7x our gold, 2-2.6x our champion damage. Early caution (v86: engage at 1.5x before round 150): 2170 vs 2822, elk lost. Treasure emblem first (v87): faster unopposed (-3%), won rows 1005-991, but elk lost 324-756 and standard maps 2145: rejected. Every variant since v78 has lost elk (0-470 gems vs v78/v85's 720-864): elk is the most fragile map.
- Top-3 draft again after patch 1.0.1 (v88: HoR, Ren, Stellar, Ryth, Varg): 1871 vs 2822 on standard maps, 3272 vs 4115 on five others: rejected. The top teams' draft only works with their play.
- Whole top-3 package (v89: their draft + treasure first + shop at 90): 1471 vs 2822 standard, elk 0-1008: rejected. Copying their visible choices does not transfer their strength; it is in their per-turn play.
- When the gap opens (v85 vs Nyx, 2026-10-20): first 300 rounds even (CW clears 9-9, gems 120-154); rounds 300-1200 Nyx clears ~2x our camps (CW 32 vs 17, corridors 34 vs 7) with almost no deaths on either side. The loss is mid-game farm rate (stronger items -> faster clears), not early fights. Summary heatmaps only hold kill/death/ward tiles, not paths.
- Nyx games became exactly repeatable (2026-10-20 ~21:30 UTC on): same bots, same map, same result. Old totals were stale; fresh v85 = 2340 (elk/CW/lanes/gold_rush/corners) and 3641 (expanse/islands/big_world/rows/ambush). Always replay the live version next to a test.
- v90 (official 143, accepted): the contested-camp penalty (x0.3) after round 300 applies only on single-herd maps (elk-type, broken_curve). Nyx farms the middle camps mid-game while we farmed edges. 2488 / 3990 vs fresh v85 2340 / 3641 (rows 703 vs 474). Shorter early window 150 (v92): 2163 / 3666, rejected.
- Spawn campers (v91): an enemy on its spawn in our range is attacked forever because it out-heals us (corridors unopposed: 2 champions stuck 800 rounds). Skipping unless killable: unopposed 1551 -> 1538 rounds, but vs Nyx identical except gold_rush 624 vs 720: rejected.
- Economy (rows vs Nyx, 1092 rounds): Nyx 9.5k gold and 183k monster damage, ours 3.5k and 94k. Gold weight 0.3 (v93) farmed 2% faster unopposed but 2221 / 3412 vs v90's 2488 / 3990: rejected. Shop threshold 100/250 and gem cap 90/120 do not change unopposed farm speed; squad size 1/2 and commit 1.2 within 2%.
- Death avoidance (v95, DEATH_AVOID 120 after champion kills): corners 343 vs 280 and lanes +52, but CW -108, gold_rush -83, elk 648 vs 720, islands -118; 2340 / 3868 vs v90 2488 / 3990: rejected. Map sizes in nyx.sh output: 50 = elk_arena (set a) or rows (set b), 61 CW, 90 lanes, 31 gold_rush, 71 corners, 80 expanse, 60 islands, 100 big_world/ambush.
- Treasure emblem first except elk-type maps (v96): 2494 / 3573 vs v90 2488 / 3990. Won rows 1005-991 (first rows win vs Nyx), lanes +155, corners +92, but CW -191, islands -206, big_world+ambush -248: rejected. Nyx buys ~3x as many items as we do, in small shard steps from round ~35, every champion opening with treasure shards.
- Expanse vs Nyx timeline: Nyx re-clears ~8 groups in its own half every respawn (60-80 rounds), including the harmless 0-gem gold clusters (240-360 hp, 40-60 gold) and the 4-gem groups; it leaves the 44-gem groups (5160 hp, str 120) until ~round 500. We skip the gold clusters and our own 4-gem group and only collect 4 times by round 141 (Nyx 12).
- Per-map treasure first (v100, official 152, accepted): only rows, lanes, corners (where v96 won). vs Nyx 2735 on set a (v90 2488) and rows won 1005-991; other maps identical. With repeatable games, per-map choices from one test are safe for that map.
- Treasure first per map, round 2 (v101, official 154, accepted): collision 148 vs 22, eight 219 vs 104. On the_farm it scored 535 vs 264 vs Nyx but only 25 gems in 2000 rounds unopposed (something stalls), so left out. Hurt packs (538 vs 876), reverse_spiral, fracture.
- Treasure first per map, round 3 (v102, official 156, accepted): split 746 vs 386, pilgrimage 490 vs 371, monolith 455 vs 368, quadrants 419 vs 394, hooks 44 vs 22, swirl 45 vs 30. Worse on interlock/shortcut (237/0 vs 365/381) and rushbait_v2. Maps still untested for treasure: elkring, yellow_brick_road, boss_battle, contest, broken_curve (frozen), starfish (unclear), big herds (stampede tie unchanged).
- Death avoidance per map (v103, official 158, accepted): DEATH_AVOID 120 probe on 21 maps won pilgrimage 873-521 (third map we beat Nyx on, with elk and rows) and corners 487 vs 372; it lost on most others (packs 448 vs 876, lanes -125, CW -108, gold_rush -83), so it is on for those two maps only. Tooling: scratchpad permap.py + mapnames.json give per-map results; base_v102.json holds the v102 per-map baseline.
