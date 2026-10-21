# Independent review: internal v123 / Cambridge v208

## A. Version examined

- Team: Crimson Bush Campers. User explicitly requested the major milestone around internal version 123, excluding rapid later experiments.
- Official Cambridge **208**, UUID `4764f0d5-5b7a-4eca-80a8-e7bfadced078`; original Git **`d0a2110652b8a6bc1f0c16a334825b394b684284`**. Manifest correspondence is recorded in team `VERSIONS.md` and accepted record commit `54f7c25bca50e700e51671d44c430aed3354914e`.
- All six source-viewer files matched this Git source after trailing-whitespace and final-newline normalization. Original Git bytes remain immutable; archive-byte identity was not established. `evidence/source_manifest.json` records exact Git/candidate hashes.
- Official build log inspected 21 Oct: Clang 22.1.0 / WASI SDK 33, linked successfully, three warnings including omitted Heart switch case. It is a successfully built historical submission, not the current active experiment. During final inspection live was v220 and v221 waiting; these are not review designations and were not audited.
- Review branch `codex/review-v208`; PR target `top-team-study`. Candidate commit is the exact final head identified in the PR body; candidate files and hashes are version-local. Branch begins at accepted-history checkpoint `08e7d4d8ed518f033c5e3a68f91f269b038caec4`, whose bot sources equal d0a2110.

## B. Overall assessment

The bot has a coherent greedy camp-value model, static distance fields, personal-vision tracking, shared eight-byte camp claims, map-specific drafts/squads/retreat rules, capture handoff, banking and shopping. Current source already incorporates earlier SDK-distance and monolith waiting fixes. The 32-entry champion-name table also already handles Heart safely; the obsolete small-table concern is not a current finding.

The main confirmed correctness defect is omitted Heart draft mapping. The main observed competitive weaknesses are combat snowballing on Elk Arena, lost resource throughput despite comparable clear counts on Concentrated Wealth/islands, and fuel suspension hidden by disabled logs. There is no demonstrated compile failure, crash or systematic illegal-action defect in the tested candidate. Catching exceptions and zero faults do not establish optimal or complete turns.

The one-line bookkeeping fix makes state correct but changed squad behavior and **regressed** in the local comparison. Keep this candidate for selective evaluation, not automatic acceptance. A separate offensive dash trial is promising locally but unverified against the contenders.

## C. Findings

### P1 — Confirmed Heart of Ruin draft-to-body mapping omission

Location: original `bot.cpp:288–323`, `Bot::setup()`; candidate adds one case at line 301. The switch translates champion types into draft choices but omits Heart. `draft_id` stays −1, lookup throws, and fallback sets `team_size=3`. `mem_slot` remains handle modulo eight. On a five-pick map original Heart as B has handle 5 and slot 5 rather than draft slot 0; as A slot happens to be 0 but team size is still wrong. This affects helper limits, priority and squad membership. It does not prove an actual claim collision in the current draft; none is claimed.

Correction: explicitly translate `CT::heart_of_ruin` to `cambc::champion_draft::heart_of_ruin`, preserving all other behavior. Expected benefit: truthful team size and stable draft-order claim ownership. Confidence in correctness: high. Real `Bot::setup` probes on both sides confirm original `(slot=5,size=3)` as B versus corrected `(slot=0,size=5)`; all other bodies retained their slots. Verification: release build, probes, 140-game mapping panel. Competitive benefit: **not established; panel regressed**. Fixing an implicit strategy dependency can require squad retuning, so Claude should not silently accept the whole behavior change.

### P1 — Observed fuel exhaustion; diagnostics under-report it

Locations: `play_turn()` ~1750, `place_ward()` ~924, `turn()` and repeated path/visibility queries. `LOG_EVERY=0` suppresses the printed OVERRUN line. Main catches exceptions and yields, but execution can also suspend on fuel without a botFault. Local unchanged control had 1,033 fuel-exhausted events across both teams/140 games; mapping panel 1,070. In selected server CW matches our side had 9/8 events versus zero for Nyx/Fish. This is measured, not a diagnosis of one expensive function.

Proposal (not implemented): count engine exhaustion, sample bounded per-phase fuel deltas outside release-critical action selection, and reserve fuel before optional queries/wards. Cache immutable metadata and per-turn visibility only where semantics remain fresh. Validate completed actions after suspension before choosing thresholds. Expected benefit: fewer stale resumed decisions and more full action turns. Confidence: high that exhaustion occurs; medium that a guard helps; low in any attribution to wards without profiling. No claim of a measured speedup.

### P2/P3 — Heart's damaging dash is absent from deliberate combat spell selection

Locations: setup ability classification, `try_spell()` ~681, `fight()` ~752, `burst()` ~997. Dash is recorded as mobility and is not a nuke; camp approach explicitly disables dash. Heart can therefore attack without deliberately using its landing damage/stun. Official current kit: range-2 dash, radius-1 landing damage at 1.5 focus and one-turn stun, compatible with walking. Mobility uses may incidentally damage; this is not a claim it never casts dash.

Separate experiment: inject a Heart-only landing search into `try_spell`, requiring a free landing in dash range and target in landing radius, nearest first with home-distance tie break, engine legality checked. Bounded candidate set is at most 24 tiles before target filtering, plus sorting and host distance lookups. Risks: shield/escape cooldown opportunity, dangerous landings, API/occupancy changes and fuel cost. `evidence/offensive-dash.patch` applies only to a generated `_dash-trial`, never the primary candidate. Local 38/70 versus mapping candidate, +57.5 gems; unchanged control 28/70, −99.2 using identical seed assignment. +156.8 average paired contrast is suggestive, **not a controlled ladder claim**. Five caught messages across both teams vs three control; inspect before promotion. Confidence in strategic gain: low-to-medium, requires replication and varied opponents.

### P2 — Camp-count-only diagnosis and stale strategic comments are misleading

Locations: team knowledge, source tuning comments, `choose_group()` ~1631. Existing notes say opponents clear roughly 3x as many groups. In these current CW/islands examples clear counts are much closer (44–52, 41–44, 38–39, 33–37) while gem gaps remain large. Elk Arena does exhibit a much larger clear/combat gap. Prioritize reward mix, capture and banking per map rather than globally increasing farm aggression. Also the Prism comment says +0.2 focus/gem while official patch is now +1.5; default draft does not currently use Prism on these maps, so no production strategic change was forced.

Correction implemented in shared knowledge: date/caveat the generalization and preserve current evidence. Source strategic comments/heuristics left unchanged to keep the code patch isolated. Confidence in observed distinction: high; best algorithmic remedy remains unproven.

## D. Changes implemented

- Complete six-file self-contained candidate, only production change is Heart mapping.
- Separate optional offensive dash patch and reproducible generated trial.
- Reproduction workspace, frozen map bytes/hashes, original reconstruction, real setup probes, safe fresh-results arena, eleven harness checks and compact per-match evidence.
- Opponent analysis with attributable server IDs, drafts, builds, banking/camp event extracts and summary hashes. No opponent private source accessed.
- Shared knowledge is a separate PR, preserving Claude's history while adding current official references, SDK/API snapshots, build instructions and concise AGENTS pointers.

## E. Testing

See test_results.md for all 490 completed local games plus two truncated diagnostic probes. 35 maps, both sides; fixed map engine seeds and score-seed perturbations are correlated. No new online games or Cambridge uploads were made. No claimed win-rate or runtime improvement versus current top teams. Original official build and revised release build passed, with warning count reduced from three to two. Eleven harness regression tests passed. Known caught exceptions and fuel exhaustion remain explicitly recorded.

## F. Questions and challenges for Claude

1. Which current squad policies depended on Heart's accidental last-slot leadership and team-size cap? Can the correct mapping be retained while explicitly configuring the previously useful squad behavior?
2. Elk Arena: can item/range/CC-aware local combat value avoid the 50+ death snowball, without sacrificing free farming maps? Flat HP×DPS and greater aggression are not sufficient explanations.
3. Rows: does scheduling actual reward/capture/bank throughput beat current clear-time estimates? Our earlier first bank is not enough; Fish won without any champion combat.
4. Can reserve thresholds reduce measured exhaustion without discarding useful wards or target work? First profile; do not blame ward count alone.
5. Evaluate Heart's offensive dash against multiple compositions and frozen official engine/maps, including broken_curve retention. No assumption that imitating contenders' drafts will reproduce their algorithms.

## G. Final handoff

Recommend accepting the diagnosis, source identities, evidence, reproduction tools and current knowledge references. Treat the Heart mapping as a correct-state fix requiring strategy validation before team adoption. The offensive dash is experimental and independently selectable. Unimplemented proposals are ranked in opponent_analysis.md. Remaining limitations: no contender source, no direct route/pathfinding reconstruction, only A-side server samples, opponent/server source versions not exposed by retrieved summary metadata, no sanitizer/native host emulation, no exhaustive invalid-action proof, no online candidate evaluation. Never merge or submit automatically.

Caught-message detail: `evidence/caught-errors.txt` preserves all eight messages across trial/control. The tested dash variant has one invalid-metadata message on interlock, also present at the same round in the unchanged control. The two additional trial-panel messages occur on the opposing unchanged candidate (unit outside vision). This does not establish that dash introduced a new error path, nor prove it cannot.
