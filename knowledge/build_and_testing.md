# Reproducible build and testing

Verified locally 2026-10-21: official PyPI `cambc==3.0.4`, C++20, WASI SDK 33, Clang 22.1.0; local engine 1.0.2 build `ff41468c05c7be716f7647aaf7ea7314633f7038815386c806fafcec98629114`, matching the sampled server summaries. Existing root `cambc.toml` advertises 2.2.5/Python: it is stale for this C++ review. Preserve it and use the version-local workspace.

## Current v208 review

```sh
python3 -m venv /tmp/cambc-v208-env
/tmp/cambc-v208-env/bin/pip install 'cambc==3.0.4'
export PATH="/tmp/cambc-v208-env/bin:$PATH"
cd codex_reviews/v208
python3 scripts/prepare.py
cambc build candidate --release
python3 -m unittest discover -s tests -v
python3 scripts/arena.py candidate _cambridge-v208 --seeds 2 --seed-base 10100 --jobs 2 --out runs/mapping-v208
python3 scripts/arena.py _control-v208 _cambridge-v208 --seeds 2 --seed-base 10100 --jobs 2 --out runs/control-v208
```

Run `cambc init` in a **new temporary folder** first if SDK/toolchain generation is needed; copy its generated `sdk/` and `cmake/` to the version workspace, or let the review's `prepare.py` initialize a temporary workspace and copy only the required official C++ scaffolding. Do not initialize over team files. The review config pins 3.0.4. Official SDK is available locally in `knowledge/official_docs/sdk-3.0.4`; build-generated CMake files are not source changes.

`prepare.py` reconstructs the exact original from its immutable Git commit, restores the 35 frozen map files from the hash-checked text archive, and creates controls, a separate dash experiment and setup probes. It changes only generated review-local bot folders and maps. Candidate sources are already complete and require no Git reconstruction. `cambc build candidate --release` emits the submission-compatible WASM; `cambc pack candidate` may package locally, but NEVER upload/activate during a Codex review.

## Interpret experiments correctly

The preserved arena runs both starting sides and records fresh per-attempt statistics. It prebuilds copies before parallel runs and fails on missing statistics/runner failure. Eleven regression checks test those properties. `--seeds` changes the C++ camp-score `SEED` constant; it does **not** set an independent engine RNG seed. Engine seeds are deterministic/map-dependent in this panel. Paired map/side comparisons are useful; repeated deterministic outcomes aren't independent evidence. Asymmetric bot seeds need an unchanged-code control.

For each panel report planned/completed/failed counts, wins, gem margins, kills/deaths, engine faults, caught exceptions and fuel-exhausted events. Zero faults can coexist with caught `error` messages or fuel suspension. Local milliseconds and ladder results aren't interchangeable. Check exact engine build, map hash, champion draft and source commit before comparing historical runs.

Team tools exist in root `scripts/` (`arena.py`, `carena.py`, `bench.py`, `farmspeed.py`, `ladder.py`, etc.). Inspect each before use; older Python/manual draft tables can omit Heart. Prefer current official `cambc matches list/show/summary/download` for existing authenticated evidence. Read-only summary/replay retrieval does not schedule games. Never store session tokens, device login links or cookies in Git. New competitive matches should be small, intentional and separately justified; this review queued none.

## Before future reviews

Read `AGENTS.md` then `knowledge/project_state.md`. Confirm explicit release designation and immutable source. Check official rules/API freshness, refresh only changed materials. Use isolated builds; don't overwrite accepted code. Prefer multi-map, both-side, multiple-opponent comparisons, preserve map/engine/source identities, and distinguish mechanics regressions from strategic hypotheses. Archived 3.0.1 results predate the current balance and must retain that label.
