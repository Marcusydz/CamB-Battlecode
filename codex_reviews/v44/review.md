# Independent review of Cambridge submission v44

## A. Version examined

- Team: Crimson Bush Campers. Cambridge **v44**, C++, submission
  `06c5cb2e-4c13-4018-bc8e-33b4ece09650`.
- Website at inspection: v44 **Live / Active**. Its build log completed compilation and linking
  with Clang 22.1.0 and three warnings. A newly submitted **v45 was Waiting and selected for the
  next match**, with v44 still the live fallback. This review freezes the successfully built v44;
  it does not claim to review v45 or later uploads.
- Corresponding original Git revision: **`c10db83cc3bd5f1a146d60a86fdfa15fc82d43bb`**, on
  `top-team-study`. Its internal label is v41, not Cambridge's submission number.
- Review branch: **`codex/review-v44`**, targeting `claude/sharp-mayer-maf56h`, the accepted/default branch after
  team PR #1 merged on 2026-10-20 at 09:46:32 UTC (merge `2c4f698a84594b55fbe18e5e735f9ef8fce09566`). The PR records the corrected candidate's exact commit.
- All five Cambridge source files were read from its Code viewer. Their normalized byte counts,
  line counts and FNV-1a checksums matched the pinned Git files. Normalization removes trailing
  whitespace. **The original archive download was blocked by Brave (`ERR_BLOCKED_BY_CLIENT`);
  no raw ZIP identity is claimed.** The complete matched Git source is preserved by its revision
  and recoverable with `scripts/prepare_workspace.py`.
- Previous submission v43 was also read. Its five normalized files match v44 except two camp
  valuation constants: `GOLD_W` and `GOLD_RUSH_GOLD_W` were 0.15 instead of 0.0. Its test reference
  is reconstructed from those observed differences, not presented as a downloaded ZIP.

- Handoff-time repository verification: PR #1 is merged; `VERSIONS.md` maps official v44 to
  `c10db83` and v45 to `588abf3`. The accepted tree now includes v45's `LOW_HP=0.4`; this
  isolated v44 candidate retains its reviewed `LOW_HP=0.3`. Tests do not establish superiority
  against v45. No accepted source file is changed by this review.

## B. Overall assessment

The bot has a practical structure: fixed/per-map drafting; persistent camp knowledge; distance
fields for movement; survival, combat and banking priorities; item builds; and eight-byte camp
claims. Recent source already adjusts camp ranges for the week-two patch, avoids repeated camp
suicides and groups champions into squads. Those structures were preserved.

The review found two reproducible correctness faults and a camp-economy regression worth
challenging. Restoring a small gold value produced a useful local advantage over v44. That is
not proof of a ladder advantage: opponents are closely related versions, some games are
correlated, and the candidate still logs visibility exceptions. Claude remains the final evaluator.

## C. Findings

### P1 — Zero gold value sacrifices useful economy (experimental strategic correction)

**Location:** `bots/candidate/src/bot.cpp`, `GOLD_W`, `GOLD_RUSH_GOLD_W`, `choose_group`.

**Original:** v44 changed both constants from 0.15 to zero, following faster unopposed farming
measurements. Camp value is `gems + gold_weight * gold`; gold-only groups therefore get zero
value and cannot win the positive-score selection. Gold also buys combat improvements and is
an official tiebreak after tributed gems.

**Evidence:** in a direct comparison changing only those two constants, unchanged v44 won
14/42 against v43, mean gem margin −187. The two source viewers differ only at these lines.
This challenges using unopposed farming as the deciding metric for contested play.

**Correction:** restore 0.15 in the candidate. The fresh final panel won **76/126 against v44
(60.3%), +120 mean gem margin**, then **22/42 against v43, +48 margin**. These are local
results, not a universally optimal weight. The passive panel was slower, so there is a real
tradeoff rather than an improvement on every metric.

**Expected benefit/confidence:** better access to upgrades and competitive camp choices;
moderate evidence against v44, limited evidence against unrelated strong strategies.
**Status:** implemented and empirically tested; remains experimental for Claude's acceptance.

### P2 — Unreachable fields are treated as expensive valid paths (confirmed correctness fix)

**Location:** `bot.cpp`, `UNREACHED`, `choose_group`, `min_dist`, `step_towards`, static-field checks.

**Original:** SDK distance fields use 65535 for unreached tiles; the bot used 1,000,000. Unreached
camps could receive tiny positive scores. A nonnegative chosen target suppresses the caller's
full-map fallback, even when the limited search reached no camp.

**Correction:** use `cambc::distance_field::unreachable` and replace the separate magic 60000
cutoffs with the same SDK constant. No pathfinding algorithm was replaced.

**Verification:** official-runtime test limits the field to the spawn tile. Original v44 selects
an unreached camp at 65535 in all six instances; the corrected planner returns no camp in all
six. This verifies target rejection, not the competitive value of every fallback decision.
**Benefit/confidence:** consistent reachability and functioning fallback trigger; high confidence
in correctness. The combined correctness-only match panel did not establish a strength gain.

### P2 — Range-five monolith leaves no gathering area (confirmed geometry fix)

**Location:** `bot.cpp`, joint-target `gather` branch; new `camp_waiting_area.h`.

**Original:** union disks of radius five, then remove each camp's attack radius. For a single
range-five monolith the result is empty: `disk(5) & ~disk(5)`. A champion waiting for helpers
therefore has no gathering destination. Nearby camp groups can worsen this geometry.

**Correction:** build a waiting band outside the attack radius, with outer radius
`max(5, attack_range + 1)`, and retain the group-wide danger exclusions. Wolf and elk geometry
is unchanged. Monolith waiting extends to radius six. Blocked/unreachable terrain can still
remove legal destinations; the existing timeout remains in place.

**Verification:** the production helper was exercised inside the official engine. Range three
has 72 safe tiles, range four 40, and range five 48, with no tile inside the respective attack
range. Eighteen assertions passed across six instances. This verifies the geometry, not that
helpers always gather successfully in a contested match.
**Benefit/confidence:** avoids an empty-goal condition; high geometry confidence, unmeasured
isolated win-rate effect. Independently removable from the gold experiment.

### P2 — Match reporting can hide failures (confirmed tooling fix, version-local copy)

**Location:** `scripts/arena.py` in this version workspace.

**Original team harness:** ignored process return codes, loaded predictable statistics filenames,
filtered missing results out, and could divide by zero. Caught exceptions were invisible to its
“faults” summary. Main-only C++ opponents were not prebuilt before parallel matches.

**Correction:** fresh attempt directories, checked release builds/runs, retained runner logs,
coverage and failure records, nonzero exit on incomplete panels, validation of map selection
and team rows, and separate counts of the bot's logged exceptions and printed overruns.
C++ prebuild detection covers `main.cpp` bots such as the passive opponent.

**Verification:** eleven automated regression tests pass. All 390 recorded match attempts
completed. This does not establish why earlier team panels had missing games. Printed
`OVERRUN` counts are not a complete fuel-exhaustion measurement when bot logging is disabled.
**Benefit/confidence:** trustworthy denominators and clearer reliability signals; high confidence.

### Unresolved / P3 investigations

- **Visibility exceptions:** final-v44 panel logged eight candidate exceptions versus four for
  v44, despite zero engine bot faults. They are handled by the main loop, but still lose useful
  work. Prior lightweight tracing changed execution and did not reproduce the exception.
  Fuel suspension/stale observations is a hypothesis, not an established cause. Do not label
  either version error-free or this review a fuel audit.
- **Prism banking:** the current patch changes carried-gem focus from 0.2 to 1 per gem. The
  source still explains its 60-gem cap using 0.2. The comment is stale; the cap is not proven
  wrong. Test banking thresholds against survival and tributed score before changing them.
- **Item builds and draft:** Tavi's spell multiplier and multiple champion stats changed. Old
  per-map draft/item conclusions need fresh opponents. No speculative draft rewrite was made.
- **Squad identity:** `squad_alive` uses nonzero camp claims rather than true liveness. Banking
  and recall can reshuffle followers. Measure how often this causes harmful split travel before
  redesigning shared memory.
- No P0 compilation failure, engine crash, or undefined-behavior reproduction was established
  for the actually drafted lineup. New SDK enum warnings remain; adding Heart of Ruin would
  require reviewing the champion mapping and fixed-size debug-name array first.

## D. Changes implemented

All additions are under `codex_reviews/v44/`. No existing team bot, root project configuration,
map or team script was edited in this branch.

1. Complete candidate under `bots/candidate/src/` with the two correctness fixes.
2. Experimental gold weights restored to 0.15; reverting those two constants separates strategy
   from correctness. `evidence/candidate.patch` shows the candidate's full source differences.
3. Isolated SDK 3.0.1 workspace, release build instructions and baseline preparation script.
4. Version-local arena and regression probes, reports, source/map hashes and all match rows.

## E. Testing

See [test_results.md](test_results.md) for the full matrix and reproduction commands. Final
candidate builds and bundles successfully under official cambc 3.0.1 / Clang 22.1.0 / WASI 33.
The final comparison has 126 complete games across 21 maps and both sides; all experiments
combined have 390 complete games. No Cambridge upload, activation or account/team change
was performed.

## F. Questions and challenges for Claude

1. Does zero gold weight still help against unrelated strong opponents, given the controlled
   14/42 result versus v43? Keep unopposed speed as one metric, not the acceptance criterion.
2. Can the visibility exception be reproduced with bounded diagnostics that do not substantially
   change fuel use? Please do not suppress it merely to improve error counters.
3. Would a fuel-aware fallback skip unreachable targets without triggering expensive replans?
   The sentinel correction is sound; the full-map fallback's budget still needs measurement.
4. What is the best risk-adjusted Prism banking policy after its fivefold passive increase?
5. Please test each correction independently where practical. Accepting the arena or geometry
   fix does not require accepting the gold weight, draft, or the whole candidate.

## G. Final handoff

Branch: `codex/review-v44`. Target: `claude/sharp-mayer-maf56h`. The draft PR supplies the exact candidate
commit and submission identity. Retrieve this folder, initialize its SDK, run the preparation
script, and build `candidate`; no source dependency on the accepted bot directory is required.

**Recommended:** accept the verified reachability/geometry corrections and honest testing
infrastructure after your review. **Trial:** restored gold valuation, supported by this local
panel but requiring stronger opponent diversity. **Outstanding:** visibility errors and direct
fuel measurements. Do not automatically replace the team's submission from these results.
