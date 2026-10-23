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
| v1.6.0 | v273 | Fix for ally-vision errors that aborted turns, end-game banking at 900 enemy gems, per-death trace for ladder diagnosis. Unranked vs 1700–1800 teams 15/40 (same as v1.5.0); next steps in the doc. Author: Claude Opus 5.5. [Details](docs/versions/v1.6.0.md) |
| v1.6.1 | v274 | Core draft now follows the top five ladder teams' picks: Vesper Vale, Heart of Ruin, Marcella, Vendetta, Ryth (Tavi dropped). Big-map farming lists unchanged. Untested. Author: Claude Opus 5.5. [Details](docs/versions/v1.6.1.md) |
| v1.7.0 | v275 | Retreat pathing prioritises home progress after preserving threat distance, addressing the v1.6 death traces' approach/back-off dithering. Based on v1.6.1/v274; unranked v1.7.0 batch vs 1708–1766 ELO teams: 7/15. Author: Copilot. [Details](docs/versions/v1.7.0.md) |
| v1.7.1 | v277 | Champion-threat retreat threshold tuned to 35% health. Unranked vs 1703–1725 ELO teams: 9/15, 9.4 deaths/game, 756 submitted gems/game. Author: Copilot. [Details](docs/versions/v1.7.1.md) |
| v1.7.2 | v278 | Champion-threat retreat at 35% is now limited to carriers or isolated champions, preserving group fights when no gems are carried. Unranked vs 1703–1784 ELO teams: 5/15; experimental and weaker than v1.7.1 in this batch. Author: Copilot. [Details](docs/versions/v1.7.2.md) |
| v1.8.0 | v280 | Larger farming/combat squads (`SQUAD=4`) to reduce isolated deaths and match pack-based high-ELO play. Local: 38/80 vs v1.7.0, −10 mean margin, 0 faults. Unranked vs 1712–1798 ELO teams: 7/15. Author: Copilot. [Details](docs/versions/v1.8.0.md) |
| v1.8.1 | v281 | Lowered the normal banking cap from 60 to 45 gems to reduce carried-gem losses. Local: 43/80 vs exact v1.8.0, +17 mean margin, 0 faults. Unranked vs 1600–1700 ELO teams: 25/40. Author: Copilot. [Details](docs/versions/v1.8.1.md) |
