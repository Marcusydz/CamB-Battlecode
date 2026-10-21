# Claude's evaluation of Codex review v208 (PR #3) and knowledge PR #4

Reviewed: official 208 = internal v123 (commit d0a2110). The accepted bot had moved on to v127 (official 225) by the
time the review arrived, so every proposal was tested on top of v127. Each test is a whole-bot probe: the change is
submitted as our bot, then 5-game unranked groups are played against Nyx and Intercontinental Fish on all 35 ladder
maps (7 sets). These groups are deterministic for fixed binaries, so per-map differences are exact rather than
sampled; they can still shift when an opponent changes its bot or the engine is patched. The score is
per map, our gems vs Nyx plus our gems vs Fish, against v127's numbers.

| Proposal | Decision | Evidence (Nyx + Fish gems vs v127) |
|---|---|---|
| Heart of Ruin in the champion-to-draft mapping (`Bot::setup`) | DEFER (bug confirmed, fix not adopted yet) | The bug is real: Heart threw "not in draft" and ran with team_size 3 and slot 0. Probe 237 (exact one-line fix) changed only the 4 maps with special squad sizes, all worse: reverse_spiral -265, starvation -252, corridors -237, fracture -128 (-882). This agrees with Codex's 68/140 vs 73/140 panel. Squad sizes on those maps were tuned with the buggy ranks, so the fix needs a squad re-tune before it can go in. |
| Heart offensive dash (`offensive-dash.patch`) | REJECT | Probe 238: about -3000 (corridors -630, split -585, quadrants -566, starvation -244, elkring -231). Only hooks +88 and yellow_brick_road +57. The +156.8 local result does not carry over to Nyx and Fish. |
| P1 kit-aware engagement (bounded 2-turn burst/CC model) | DEFER | Agree with the diagnosis (elk_arena: we were 61 deaths vs 2). Since the review, Prism-first drafting on elk_arena (v125) turned that game into a 1044-252 win vs Nyx, so drafting fixed the worst case. Fight-threshold tweaks on elk (1.5, retreat 60%, back off without recall, death avoidance) were all worse (probes 213-216). A real burst model is the next big piece of work. |
| P1/P2 reward-weighted banking: bank more often (Fish banks 47x on CW vs our 16x) | MODIFY | Probe 239 (bank at 30 gems instead of 60) was about -4600 overall (islands -728, split -574, lanes -400), so "bank more often" is not the general answer. It won shortcut +308, reverse_spiral +75, hooks +66; kept on those 3 maps in v133. The opposite (bank at 90, probe 245) was about -3500 but won broken_curve +186, kept there. |
| P2 fuel: fewer wards | REJECT (opposite kept per map) | Probe 240 (ward every 16 tiles instead of 8): about -6700 (elk -900, split -733, lanes -546). Codex warned that disabling vision could hurt, and it did. More wards (every 5 tiles, probe 246) lost about 4500 overall but won concentrated_wealth +214 (where Codex saw our fuel exhaustion), yellow_brick_road +135, corridors +117, fracture +78; kept on those 4 maps. The fuel events do not seem to cost us games there. |
| Opponent analysis: Nyx/Fish drafts | Noted | Copying the top teams' draft order (Cassi & Issac, Stellar, Ren first) on every map was about -8500 (probe 224). Our own post-patch lists (Heart, Vesper, Prism, Vendetta by map) suit our bot better. |
| Rows as a no-combat regression case | ACCEPT | Rows vs Nyx stays a win (1017-991) in every accepted version since v121. |
| PR #4 knowledge base (rules, C++ API, build/testing, AGENTS.md) | ACCEPT in principle | Useful; leaves accepted bot files untouched. Merging is Marcus's call. One correction: project_state.md checkpoints official 208; the accepted bot is now v133 or v127 (see VERSIONS.md). |

What Codex got right: the Heart mapping bug, and the observation that clear counts alone don't explain the gem gap on
CW and islands. The warning not to blindly disable wards was correct.

Disagreements: neither "bank more often" nor "fewer wards" helps in general against the real opponents; both
local-panel effects reversed or vanished on the server. Codex's panel used score-seed perturbations against our own
bot; Nyx/Fish probes on all 35 maps are the stronger test.

Incorporated into the accepted version v133 (official 247; verified on 35 maps, +1157 vs v127 with no map worse): per-map bank caps
(30 on shortcut, reverse_spiral, hooks; 90 on broken_curve) and ward spacing 5 on CW, yellow_brick_road, corridors,
fracture. Unresolved: the burst/CC engagement model, the Heart mapping fix with re-tuned squads, and contest (0
gems vs both: they hold the single middle altar and bank 200 at a time).
