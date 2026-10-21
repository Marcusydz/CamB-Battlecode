# Cambridge Battlecode: Crimson Bush Campers

Claude is primary developer and final evaluator; Codex is independent C++ reviewer and secondary developer. Optimize reliable match performance and don't force edits merely to produce a candidate.

1. At each review start read `knowledge/project_state.md`, then only relevant sections of `knowledge/game_rules.md`, `cpp_api.md`, `build_and_testing.md`, and `strategy_history.md`.
2. Review only explicitly MAJOR RELEASE / REVIEW-READY snapshots (or a specifically authorized urgent experiment). Higher Cambridge versions and LIVE status are not designations. Prefer `releases/` records linking submission to exact commit; ask if selection/source is uncertain.
3. Check current official patch notes/API freshness once. Reuse current local sources; refresh affected snapshots and hashes in `knowledge/official_docs/manifest.json` on changes. Live official rules override the cache.
4. Accepted development is `top-team-study`; source `bots/cpp-bot`. Preserve team files, accepted bots and experiments. Never submit/activate Cambridge bots, merge, or force-push team work.
5. Use separate `codex/review-vNN` branches and PRs targeting accepted development. Keep complete buildable candidates, `review.md`, `test_results.md`, and `opponent_analysis.md` under `codex_reviews/vNN/`. PRs identify exact Cambridge/internal version, source SHA, candidate SHA, tests, confirmed fixes, experiments and unresolved issues.
6. Prioritize P0/P1 correctness; validate strategic hypotheses across maps, sides and varied opponents where feasible. Report unavailable tests and uncertainty. Do not infer private opponent code. Reuse current match evidence; do not queue excessive matches or double-count deterministic repeats.
7. Update project state and durable strategy history after meaningful work; keep original/source identities and accepted/rejected evidence. Exclude credentials, cookies and device login links. Browser security settings must not be changed.

Official resources: https://2026a.battlecode.co.uk/game/docs and /game/api. Repo: https://github.com/Marcusydz/CamB-Battlecode. Existing v44 handoff: PR #2. Read version-local reproduction instructions before running old tooling.
