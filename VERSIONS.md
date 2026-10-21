# Versions

Each release is a git tag on `main` holding the exact source of `bots/main`.
Bump the major version for a new strategy, minor for behaviour changes, patch for fixes and tuning.

| Version | Official | Notes |
|---|---|---|
| v1.0.0 | | `bots/main` as downloaded from the competition site (v138-era probes). Repo cleaned up. |

## Before v1.0.0

The old scheme numbered every probe (v43–v138 live on the `exp/*` branches). Submitted versions
recorded then, with the commit on `top-team-study` holding the source (`bots/cpp-bot`):

| Official | Name | Commit | Notes |
|---|---|---|---|
| 44 | v41 | c10db83 | ignore gold when choosing camps; stay off contested camps early |
| 45 | v42 | 588abf3 | v41 + retreat at 40% health; first win vs Nyx (1/5, broken_curve 1013-901) |
