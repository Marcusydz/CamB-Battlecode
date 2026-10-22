# Versions

Each release is a git tag on `main` holding the exact source of `bots/main`.
Bump the major version for a new strategy, minor for behaviour changes, patch for fixes and tuning.

| Version | Official | Notes |
|---|---|---|
| v1.0.0 | v266 | `bots/main` as downloaded from the competition site (v138-era probes). Repo cleaned up. |
| v1.1.0 | v267 | Heart of Ruin dash strike, ranged reset after moving, move retry, Heart of Ruin slot fix; code split into folders. 163/280 vs v1.0.0 locally. [Details](docs/versions/v1.1.0.md) |
| v1.2.0 | v268 | Precomputed camp-group distance maps (`MAP_FIELDS`); fix for a crash when sorting camps. 370/700 vs v1.1.0 locally. [Details](docs/versions/v1.2.0.md) |
| v1.3.0 | — | Adaptive counter-drafting, map-layout profiles, and match-score-aware fighting, farming, and banking. 105/210 vs v1.2.0 over three seeded runs (−17 mean gem margin). [Details](docs/versions/v1.3.0.md) |
| v1.4.0 | v269 | Team-aware drafting scores map fit, role coverage, synergies, matchups, and enemy-composition disruption. 125/210 vs v1.3.0 over three seeded variants (+86 mean gem margin). [Details](docs/versions/v1.4.0.md) |
| v1.5.0 | v271 | Data-backed draft (Vesper Vale, Tavi, Marcella, Vendetta, Ryth; v1.2.0 lineups on seven big open maps) and low-health escape fixes. 184/240 vs v1.2.0 and 189/240 vs v1.4.0 locally; unranked vs 1680–1800 teams 10/40 on our worst maps (v268: 0/40) and 16/40 on all maps (v270 draft only: 10/40). Author: Claude Opus 5.5. [Details](docs/versions/v1.5.0.md) |
