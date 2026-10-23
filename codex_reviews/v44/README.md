# Cambridge v44 — independent Codex review

This directory is a separate cambc workspace. The team's accepted bot and root configuration
are unchanged. Start with [review.md](review.md) and [test_results.md](test_results.md).

`bots/candidate/src/` is the complete corrected C++ bot. It contains no dependency on the
team bot's source directory. The official SDK is downloaded normally by `cambc`.

From this directory, with `cambc==3.0.1` on PATH:

```sh
cambc init . --yes
python3 scripts/prepare_workspace.py
python3 -m unittest discover -s tests -v
python3 tests/run_pathing_probe.py --source bots/_cambridge-v44 --expect-failure --out runs/path-original
python3 tests/run_pathing_probe.py --out runs/path-fixed
python3 tests/run_waiting_probe.py
cambc build candidate --release
cambc bundle candidate --output candidate.zip
python3 scripts/arena.py candidate _cambridge-v44 --seeds 3 --seed-base 6100 --jobs 2 --out runs/recheck-v44
python3 scripts/arena.py candidate _cambridge-v43 --seeds 1 --seed-base 7100 --jobs 2 --out runs/recheck-v43
```

The preparation script recovers v44's source and the exact tested map set from pinned Git
revision `c10db83cc3bd5f1a146d60a86fdfa15fc82d43bb`. It reconstructs v43's two differing
constants; all five normalized files were matched against Cambridge's code viewer.
Generated SDK files, maps, baseline copies, match replays, and builds are ignored here.

The source differences against v44 are in `evidence/candidate.patch`. Match seeds vary bot
camp scoring; they do not set a random engine seed. Inspect per-game JSON and coverage, and
keep caught exceptions separate from engine faults. This candidate requires Claude's final
review and evaluation before any competition submission.
