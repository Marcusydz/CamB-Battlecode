# v44 review — test results

## Environment and scope

- Source matched to Cambridge v44: Git `c10db83cc3bd5f1a146d60a86fdfa15fc82d43bb`.
- Official `cambc 3.0.1`, SDK 3.0.1; WASI SDK 33 / Clang 22.1.0; release mode, default fuel.
- Twenty-one maps pinned to that revision; SHA-256 values in `evidence/provenance.json`.
- Engine version/build hash, map hashes, source hashes and experiment definitions are recorded
  in that provenance file. All 390 individual outcomes are in `evidence/matches.csv`.
- Cambridge Code viewer verification covers all five original source files; the ZIP archive
  itself was blocked by Brave. v43 is reconstructed from its observed two-constant difference.
- Initial Cambridge status: v44 Live/Active and successfully linked; v45 Waiting and selected
  next. This report is scoped to v44 rather than claiming later submissions were reviewed.

## Compilation and correctness

| Check | Result |
| --- | --- |
| Final candidate release compilation | Pass |
| Submission source bundle | Pass; generated locally, not uploaded |
| Arena regression tests | 11/11 pass |
| Original v44 limited-field pathing probe | 6/6 reproduce incorrect selection at distance 65535 |
| Corrected limited-field pathing probe | 6/6 select no unreachable camp |
| Waiting-area geometry | 18/18 assertions pass (three ranges × six instances) |
| Safe tile counts on open interior | range 3: 72; range 4: 40; range 5: 48 |
| Changes outside `codex_reviews/v44/` | None in the review branch |

The geometry probe calls the production helper. It verifies a nonempty band beyond attack
range and unchanged wolf/elk geometry. It does not establish that all real-map gathering
positions remain traversable or that all teammates cooperate correctly.

The final build retains existing warnings for numeric Golem handling, an unhandled Heart of
Ruin enum in the legacy mapping, and a constant condition in inactive kiting code. No new
champion was added to the draft. Engine faults and logged exceptions are distinct below.

## Match matrix

Every row completed all planned games. Mean gem margin is from the first bot's perspective.
The final candidate has both correctness fixes and restored gold weights of 0.15.
“Correctness only” retains v44's zero weights and is a separate earlier experimental stage.

| Experiment | Wins / games | Mean gem margin | Engine faults, first/second | Logged exceptions, first/second |
| --- | ---: | ---: | ---: | ---: |
| Correctness only vs v44 | 59/126 | −30.4 | 0/0 | 4/5 |
| Correctness only vs v43 | 14/42 | −166.2 | 0/0 | 2/2 |
| Unchanged v44 vs v43 (gold-weight control) | 14/42 | −186.9 | 0/0 | 1/3 |
| **Final candidate vs v44, fresh seeds** | **76/126** | **+120.1** | **0/0** | **8/4** |
| Final candidate vs v43, fresh seeds | 22/42 | +47.6 | 0/0 | 1/1 |
| Final candidate vs passive, three maps/both sides | 6/6 | +968.5 | 0/0 | 0/0 |
| Unchanged v44 vs passive, same panel | 6/6 | +1013.5 | 0/0 | 0/0 |

**Total: 390/390 completed, zero omitted runs.** Final-v44 win rate: 60.3%.
The candidate reached 1,000 gems in four of six passive games; v44 did so in six of six.
Mean passive match duration was 1,533 rounds for the candidate and 1,416 for v44. A win by
round-limit tiebreak is not treated as reaching 1,000 gems. This is evidence of a tradeoff:
restored gold value helped this contested panel while sacrificing some unopposed tempo.

## Seeds and independence

All head-to-head panels use every tracked map and both sides. Bot scoring seeds:

- Correctness only: 3101–3103 vs 3201–3203 for v44; 4101 vs 4201 for v43.
- Unchanged v44 vs v43 control: 4101 vs 4201, matching the earlier reference panel.
- Final candidate: **6101–6103 vs 6201–6203** for v44; **7101 vs 7201** for v43.
- Passive smoke games: default scoring seed zero, maps `00expanse`, `01gold_rush`, `01islands`.

These seeds perturb camp evaluation in the bot; the CLI does not expose them as engine seeds.
Engine seeds are recorded and fixed per map in these panels. Side reversals and neighboring
scoring seeds are correlated. The final seeds were not used to choose the gold restoration.
No formal independent-sample confidence claim is made. Opponents are predecessor strategies
and a passive bot; there is no reproduced ladder evaluation or broad external opponent panel.

## Reproduce

Follow `README.md` to initialize this version-local workspace and generate the pinned references.
Then run, from `codex_reviews/v44/`:

```sh
python3 -m unittest discover -s tests -v
python3 tests/run_pathing_probe.py --source bots/_cambridge-v44 --expect-failure --out runs/path-original
python3 tests/run_pathing_probe.py --out runs/path-fixed
python3 tests/run_waiting_probe.py
cambc build candidate --release
cambc bundle candidate --output candidate.zip
python3 scripts/arena.py candidate _cambridge-v44 --seeds 3 --seed-base 6100 --jobs 2 --out runs/recheck-v44
python3 scripts/arena.py candidate _cambridge-v43 --seeds 1 --seed-base 7100 --jobs 2 --out runs/recheck-v43
python3 scripts/arena.py _cambridge-v44 _cambridge-v43 --seeds 1 --seed-base 4100 --jobs 2 --out runs/recheck-control
```

To isolate correctness, copy `bots/candidate` into a new ignored `bots/_correctness-only`
folder and set both `GOLD_W` and `GOLD_RUSH_GOLD_W` to 0.0 there. To isolate geometry,
remove the `camp_waiting_area.h` include and restore the original radius-five union in that
copy. Source hashes for the recorded stages are in the provenance file. Use fresh scoring
seeds for additional evaluation rather than retuning on the recorded final panel.

## Outstanding issues and limits

- Caught `Unit is not in vision` exceptions remain. Zero engine faults does not mean zero lost
  turns or invalid-state queries. Their source is unresolved.
- Fuel headroom and suspension frequency have not been comprehensively measured. The bot
  disables its overrun logging, so zero printed overrun lines is not proof of no exhaustion.
- This panel does not prove that a gold weight of 0.15 is optimal or that the candidate beats
  v45 or later versions. The unchanged v43 source is competitive with the candidate.
- Rejected hypotheses and the initial neutral correctness results were retained, not hidden.
- Claude should independently compile, isolate the changes, test stronger opponents and decide
  what to incorporate. Nothing was merged or submitted to Cambridge by this review.

## Official references consulted

Current [handbook downloads](https://2026a.battlecode.co.uk/game/docs/agents), current
[C++ API](https://2026a.battlecode.co.uk/game/api), and
[Week 2 patch](https://2026a.battlecode.co.uk/game/patch-notes/2e172875-28d7-4152-9483-54a3db6d2663).
Relevant rules: monolith attack range five; gold buys upgrades and breaks tied tributed score;
Prism gains one Focus per carried gem; fuel exhaustion suspends execution into later turns.
The installed SDK exposes the 65535 distance sentinel used by the runtime probes.
