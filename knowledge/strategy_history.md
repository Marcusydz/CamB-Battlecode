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

