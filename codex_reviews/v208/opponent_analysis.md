# Competitive opponent analysis — v123 / official v208 milestone

## Evidence and limits

Retrieved 21 Oct 2026 using authorized official CLI `matches list`, `matches summary` and `matches download`. Exact opponent name is **Intercontinental Fish**, team `1890540d-1ffc-4847-adbf-d0d08772f268`; Nyx `6f7fd199-4115-4fe0-8752-525bd8ccad8d`. Fish was rank 1, rating 2245.66 in the saved 21 Oct ladder snapshot; rankings are time-specific. No claim that an older Nyx rank remains current.

Eight existing unranked records selected from 08:01:59–08:05:03 UTC, four contrasting maps, both contenders. All have our team as A and opponent as B. All use engine 1.0.2 build `ff41468c05c7be716f7647aaf7ea7314633f7038815386c806fafcec98629114`. Source dates/drafts match the team's v123 benchmark window, but the retrieved records do **not** expose exact submitted-source hashes. Thus source linkage is contextual, not cryptographically proven. Opponents may change code later. Samples are selected diagnostic cases, not a representative ladder win-rate estimate.

`evidence/opponent_matches.json` preserves match/group IDs, timestamps, map hashes, seeds, rosters, item builds, camp/bank/purchase events, aggregate metrics and SHA-256 of each official raw summary. Use the site's Matches/Match Database or `cambc matches summary ID --json` and `cambc matches download ID` to retrieve exact records. Two existing replay bundles were also retrieved (Nyx CW `35f8450d-...`, Fish rows `0f0e14ef-...`); opponent private stdout/source was not exposed and was not sought. Analysis below uses recorded events/statistics, not a claimed full visual movement audit. No new matches queued.

## Direct comparison

Values are ours–opponent. Deaths include camp deaths; kills need not equal opposing deaths. campClear is the engine event count, not a reward-weighted score or inference about a route algorithm.

| Map | Opponent | Submitted gems | Deaths | campClear events | Bank events | Match ID |
|---|---|---:|---:|---:|---:|---|
| 00rows | Intercontinental Fish | 820–1005 | 0–0 | 48–56 | 34–43 | `0f0e14ef-f59b-40bd-ac40-4ae93b664a8c` |
| 00rows | Nyx | 1017–991 | 0–1 | 57–68 | 42–50 | `958b26c1-12ab-41d4-b8f6-48d72e5a2808` |
| 01concentrated_wealth | Intercontinental Fish | 441–1015 | 7–2 | 41–44 | 16–47 | `84e1cb75-5710-4762-ada9-c9ab35bed02f` |
| 01concentrated_wealth | Nyx | 492–1017 | 3–1 | 44–52 | 16–28 | `35f8450d-cf08-4251-bf9a-07b2eeef81ce` |
| 01elk_arena | Intercontinental Fish | 0–1008 | 50–6 | 22–115 | 0–17 | `0c37b9af-a84c-484c-b74e-8b209ece8b8a` |
| 01elk_arena | Nyx | 576–648 | 61–2 | 64–243 | 11–12 | `61c3e337-f670-4c14-9536-c8e648e88e2c` |
| 01islands | Intercontinental Fish | 732–1007 | 9–4 | 33–37 | 25–30 | `5845d16b-6e86-4b80-957a-528e1a6cf615` |
| 01islands | Nyx | 666–1003 | 7–5 | 38–39 | 19–30 | `5bbb1baa-e8db-42bc-86e3-8e37b5152312` |

### Nyx

Observed draft: Heart of Ruin, Ren, Ryth, Stellar, Tavi on the five-pick maps; rows has the first four. Our CW/islands draft was Vesper, Varg, Vendetta, Ryth, Willow; Elk Heart, Varg, Stellar, Ryth, Willow; rows Vesper, Varg, Stellar, Ryth. These are distinct compositions, so output differences cannot be attributed to one heuristic alone.

On Elk, Nyx dealt 143,022 champion damage versus our 23,358 and earned 19,244 gold versus 3,960. It had 55 champion kills versus our one, while total deaths were 2 versus our 61. Gem totals were only 648 versus 576 at round 2000. This demonstrates a severe combat/economy disadvantage even when final gems look close. First bank was round 115 for Nyx versus 605 for us. Inference: combat control and farming access may reinforce each other; exact targeting, chase and spell sequencing are unknown.

On CW Nyx banked first at round 133 versus our 163; 28 bank events versus 16. It collected 1,032 gems versus 546, with clear counts only 52 versus 44. Early purchases included Stellar Treasure Focus shards at round 44, followed by completion and Jungle Focus at 173. On islands clear counts were 39 versus 38, yet collection 1,003 versus 724 and banking 30 versus 19. The old blanket claim of 3x clear count does not explain these maps. Reward selection/capture distribution and safe bank cycles deserve testing.

Rows was our narrow **1017–991 win**. Neither team dealt champion damage; Nyx collected 1,029 versus our 1,017 but did not bank the last 38 before game end. It banked 50 times versus our 42 (mean batch 19.8 versus 24.2). This is direct evidence that more collection and more deposits do not guarantee a win; finish timing matters. Preserve this farming behavior as a regression case.

### Intercontinental Fish

Observed draft: Stellar, Heart, Ren, Cassi & Issac, Ryth on the five-pick maps; rows first four draft choices, with Cassi and Issac two bodies. On rows neither team dealt any champion damage or died; earned gold was near equal (our 9,697 versus 9,572), yet Fish collected/banked 1,005 versus our collected 846/banked 820, finishing at round 1317. Clear events 56 versus 48 and banks 43 versus 34, mean batch 23.4 versus 24.1. We banked earlier (45 versus 79), so "bank sooner" alone is not an adequate remedy.

Fish's rows Heart bought five Treasure Focus and one Power Focus shard at round 37; Stellar five Treasure Focus at 65. Our Vesper bought five Treasure Strength at 50 and two Power Strength at 51; our Stellar first Treasure Focus at 108. This supports a test of role-specific scaling/first-shop timing, not a conclusion that copying the draft wins. The team already has map-specific treasure openings; do not propose it as a novel untested universal fix.

On CW Fish banked 47 times versus 16 (mean batch 21.6 versus 27.6), first at 82 versus 192, collecting 1,037 versus 520 despite only 44 versus 41 clear events. On islands bank count 30 versus 25, collection 1,007 versus 790. On Elk Fish had 50 champion kills, our team zero gems and 50 deaths; this is a combat-starvation failure, not merely an overlarge bank threshold.

## Observable versus unknown behavior

Camp and altar event coordinates/timings are available in the evidence and show repeated objectives and actual deposits. They do not identify shortest-path, A*, search depth or hidden opponent state. Movement efficiency, retreat triggers and fine team coordination were not measured directly here. Inferences from kill/gold/clear rates are labeled above. Item builds/drafts are directly recorded; adaptation is visible as composition differences across sampled maps but causality is unresolved. No reliable exploitable private algorithm was established.

Our teams placed many more wards on farming maps (CW 420/361 versus Nyx164/Fish246; islands267/286 versus59/54; rows397/336 versus132/170). Our CW fuel events 9/8 versus opponents zero; islands5/5 versus zero. These are correlations. Opponents may need less warding because their routes, composition or visibility differ; blindly disabling wards could make us worse.

## Prioritized, testable hypotheses for Claude

### 1. P1: Reduce combat snowball losses with kit-aware engagement and focus allocation

Observed: Elk death/kill/gold and damage gaps above. Inference: flat aggregate HP×DPS is missing burst, CC, range, readiness, target concentration and escape costs. Our `power`, `enemy_power`, `burst`, `pvp`, `outnumbered`, `join_fight` approximate these coarsely; Heart is not in the caster list and damaging dash is not represented as deliberate burst.

Attempt: estimate a short bounded two-turn damage/CC window for visible units, give enemy uncertainty a conservative margin, and retreat/group before committing into superior spell windows. Keep candidate independent from draft/item changes. Expected benefit fewer deaths and retained farming uptime; risks overretreating, repeated regroup stalls, lost captures. Cost O(visible allies × visible enemies × bounded abilities), cap candidates/fuel. Confidence medium in model deficiency, low in best coefficients. Test against unchanged v208, fixed opponents and both contenders where permitted, both sides, including rows/broken_curve no-combat regressions; report deaths, access to camps and gold, not only gems.

### 2. P1/P2: Reward-weighted farm/capture/bank scheduling

Observed: CW/islands counts close but rewards far apart; Fish rows wins with no combat and similar gold. Inference: reward mix, missed captures, teammate reward splits and bank travel/finish timing may dominate raw monster damage. Current `choose_group`, `helpers_needed`, capture branch, `tribute`, shared `claim` and squad follower logic are implementation areas.

Attempt: log each intended group's predicted clear/capture/bank completion versus observed reward, then prioritize expected submitted gems per full trip and avoid redundant followers on reward-poor groups. Derive reward-weighted completion rates from event data before choosing a threshold. Benefits higher useful throughput; risks de-synchronizing squads and contested-objective exposure; O(groups) scoring with cached immutable paths, no full global search. Confidence high in observed throughput gap, medium/low in algorithm explanation. Test controlled per-map ablations, farm and combat opponents, both sides; preserve rows-vs-Nyx win and do not optimize only CW.

### 3. P2: Deliberately use Heart offensive dash (implemented experiment)

Evidence is own source plus official kit, **not an observation that contenders use this exact algorithm**. Separate patch and 70-game trial/control are in review.md and test_results.md. Trial improves paired local mean by 156.8 gems relative to unchanged-code control but has caught exceptions and fuel events. Benefits damage/stun/position; risks lost shield/escape and bad landings. At most 24 initial landing tiles plus filtering/sort. Confidence low-to-medium. Replicate multiple score panels against varied frozen bots before promotion; inspect error traces and chosen landings. Do not merge experimental patch into primary fix by default.

### 4. P2: Bound optional per-turn work using actual fuel evidence

Observed actual fuel events and excessive relative ward counts; cause unproven. Profile `play_turn`, `place_ward`, `blocked`, `step_towards`, repeated ability/visibility queries. Cache immutable metadata, scope per-turn live caches carefully, and reserve a measured action budget before optional work. Cost of profiling must not alter release behavior; avoid host calls merely to count each host call. Confidence high that exhaustion occurs, medium in reducing it safely. Compare action completion, fuel percentiles, bank/camp progress and match outcomes across 35 maps; never count zero botFaults as proof of no overruns.

## Changes and exclusions

Confirmed correctness fix: Heart mapping only. Observed weaknesses: combat losses, resource throughput and fuel events. Inferred strategies: short combat windows, reward-weighted scheduling, early scaling; opponent implementations unknown. Implemented experiment: Heart landing dash. Unimplemented suggestions: model, scheduling and guarded-work changes above. No private code imitation, broad rewrite, online candidate claim or automatic acceptance is justified.
