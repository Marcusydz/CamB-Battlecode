# Review of internal v123 / Cambridge v208

Frozen source: `d0a2110652b8a6bc1f0c16a334825b394b684284`. This is the team-designated review requested by the user, not the latest experimental upload. Candidate: `bots/candidate/src/` (six complete files). Production difference: one explicit Heart of Ruin draft mapping; **confirmed bookkeeping fix, performance regression in this local panel**. Do not automatically replace the accepted bot.

Read `review.md`, `test_results.md`, and `opponent_analysis.md`. `evidence/corrections.patch` isolates the fix. `evidence/offensive-dash.patch` is a separate, experimentally promising but unproven strategic trial.

Reproduce from repository checkout:

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
python3 scripts/arena.py _dash-trial candidate --seeds 1 --seed-base 11100 --jobs 2 --out runs/dash-trial
python3 scripts/arena.py _control-fixed candidate --seeds 1 --seed-base 11100 --jobs 2 --out runs/dash-control
python3 scripts/arena.py candidate passive --jobs 2 --out runs/passive
cambc run _probe-original _probe-fixed --map 01elk-arena --rounds 2 --release -o runs/probe
cambc run _probe-fixed _probe-original --map 01elk-arena --rounds 2 --release -o runs/probe-reverse
```

The official CLI downloads/generated build support is ignored. `evidence/maps.json` preserves 35 official map bytes and SHA-256s; `prepare.py` restores them and exact original/control/probe/trial sources without touching accepted team files. The passive bot is a test opponent, not a competitive baseline. `scripts/arena.py` and its regression tests preserve the earlier corrected harness. Engine/server versions and limitations are in test_results.md. Do not upload or activate anything from this review automatically.
