# Test results — official v208 / internal v123

Executed 2026-10-21. Source `d0a2110652b8a6bc1f0c16a334825b394b684284`; hashes in evidence/source_manifest.json. CLI/SDK 3.0.4, C++20, WASI SDK 33 Clang 22.1.0. Engine 1.0.2 build `ff41468c05c7be716f7647aaf7ea7314633f7038815386c806fafcec98629114`, same build as selected server summaries. Current 35 official map bytes preserved in evidence/maps.json; older accepted-root map set has only 21 and is not interchangeable.

## Compilation and correctness checks

Official build log: success, three warnings. Candidate release build: success, two retained warnings (golem handled by numeric fallback outside switch; constant KITE/MONO_KITE condition). Original Heart warning removed. Build logs are in evidence/. No native sanitizer result is claimed; the official host API and fuel instrumentation are WASM based.

Eleven arena regression checks passed: fresh-attempt isolation, runner failures/timeouts, malformed/missing results and diagnostic parsing; see evidence/harness-tests.log. Real Bot::setup probes use only test-copy private-to-public visibility plus a minimal driver. On 01elk-arena, original Heart has size 3 instead of 5 on both sides, and slot 5 as B instead of 0. Corrected setup yields slot 0 and size 5 on both sides; other bodies map slots 1–4. These are two deliberately truncated 2-round diagnostic matches, not wins or independent performance evidence.

## Complete local panels

Counts of faults, caught messages and fuel events below sum **both teams**, not just the tested side. Per-side arrays and per-match results are retained. All 490 planned games completed, zero failed. Win count refers to the first named variant, whichever starting side it occupies. Gem margin is that variant's final submitted-gem difference, including gold-tiebreak maps where gems do not fully describe the outcome.

| Panel | Wins | Mean gem margin | Engine faults | Caught messages | Fuel-exhausted events |
|---|---:|---:|---:|---:|---:|
| mapping-v208 | 68/140 | -47.3 | 0 | 0 | 1070 |
| control-v208 | 73/140 | +4.3 | 0 | 0 | 1033 |
| dash-trial | 38/70 | +57.5 | 0 | 5 | 610 |
| dash-control | 28/70 | -99.2 | 0 | 3 | 579 |
| passive | 70/70 | +909.6 | 0 | 0 | 348 |

Mapping and unchanged-code control use two score-seed pairs 10101/10201 and 10102/10202, each on 35 maps and both sides (140 each). Mapping result −47.3 versus control +4.3 gives a −51.5-gem paired mean contrast; 68 versus 73 wins. The correction is not demonstrated stronger.

Dash trial and its unchanged-code control use 11101/11201, 35 maps and both sides (70 each). Trial +57.5 versus control −99.2 gives +156.8-gem paired contrast; 38 versus 28 wins. This is a single correlated panel; significance and generalization are unestablished. Trial sources include the mapping fix; control has the same mapping fix without dash. Five caught messages occurred across both trial teams; exceptions were contained but need inspection.

Passive sweep: candidate won 70/70, no caught messages or faults. This only detects gross regressions; a passive enemy does not establish competitiveness. Fuel exhaustion still occurred. The 140-game panels took approximately 49 seconds each of parallel match running on this machine, excluding preparatory builds; this is panel wall time, not per-turn speed or a speedup benchmark.

## Reproduction and evidence

README.md gives exact commands. Run scripts/prepare.py first. The original is reconstructed from immutable Git bytes and is never overwritten. Evidence retains map hashes, source hashes, all compact per-match outcomes, per-side faults/caught messages/fuel events and original summary hashes. Local full replays/statistics remain in ignored runs/. Engine seed is recorded in each native statistics file; `--seeds` changes only bot camp-score perturbation and does not create independent engine random trials. Repeated deterministic matches are not independent samples.

No candidate was uploaded, activated, merged or matched online by this review. Server opponent evidence consists of eight pre-existing unranked matches across four maps; it is scouting evidence, not a trial of this candidate. No direct candidate-vs-previous-accepted v206 panel was run: exact v208 and unchanged controls were prioritized. No claim that old v44/v51 results under engine/SDK 3.0.1 remain current.

## Outstanding validation

Retest correct mapping with explicit squad priorities; inspect caught-error paths from dash panel; measure phase-level fuel including resumed turns. Replicate dash with different score seeds, maps and frozen varied competitive opponents before acceptance. Obtain both starting sides and exact contender submission IDs when server metadata permits. Revalidate after official rules/API/map changes. Never extrapolate a 70-game self-play panel into a ladder win rate.

Caught-message detail: `evidence/caught-errors.txt` preserves all eight messages across trial/control. The tested dash variant has one invalid-metadata message on interlock, also present at the same round in the unchanged control. The two additional trial-panel messages occur on the opposing unchanged candidate (unit outside vision). This does not establish that dash introduced a new error path, nor prove it cannot.
