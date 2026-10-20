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

