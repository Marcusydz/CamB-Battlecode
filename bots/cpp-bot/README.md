# cpp-bot: Cambridge Battlecode 2026a, C++

A line-for-line C++ port of `bots/my-bot` (v16, Python). Same draft, same strategy, same
tuning values; see `bots/my-bot/README.md` for how the strategy works and how it was tested.

## Files

| File | What it is |
| --- | --- |
| `src/main.cpp` | Entry point. Draft instance runs `draft.cpp`, champions run `Bot`. |
| `src/draft.cpp` | Draft preference list (same order as my-bot). |
| `src/bot.h`, `src/bot.cpp` | All gameplay. Tuning parameters are at the top of `bot.cpp`. |

Build with `cambc build cpp-bot` (`cambc run` builds automatically). There is no bytecode step.

## Differences from my-bot

- **Fuel.** C++ gets 100k fuel per turn (Python gets 2.5M; the units differ). my-bot's absolute
  fuel thresholds are scaled to fractions of the turn: `FUEL_RESERVE` 0.28, `LOW_FUEL` 0.64.
  Copying the Python numbers unscaled made every turn look "low on fuel" and lost 43 of 49 games.
  A heavy turn uses about 55% of the budget here, against about 88% for the Python bot.
- **Rejected switches are not ported.** `RALLY`, `MONOLITH`, `GHOSTS`, `FAIL_MEMORY`, `MAX_WAIT`,
  `SPELL_DPS`, `SCARCE_GEMS`, `GOLD_MODE`, `LINKED`, `CONTEST_PENALTY` and `SEED` were off in v16
  and lost when tested; their code paths are left out. Switches that were on (`FIGHT_FIX`,
  `CAPTURE_FIX`, `LOWHP_FIGHT`) are now plain behaviour.
- **Quirks kept on purpose**, so the port matches v16 exactly; fix them as separate changes:
  - Distance fields report unreachable tiles as 65535, but the code compares with
    `UNREACHED = 1000000`. Camps outside `PLAN_RADIUS` therefore still get a (tiny) score
    instead of being skipped, and the whole-map fallback search almost never runs.

## Toolchain (cloud sessions)

`cambc` downloads the WASI SDK, CMake and Ninja into `~/.cache/cambc/`, but its downloader
rejects the session proxy's certificate. Fetch them with `curl` instead and unpack them there
as `wasi-sdk-33`, `cmake-3.31.8-linux-x86_64` and `ninja-1.12.1-linux-x86_64` (the URLs are in
the error message).

## v17 changes (from top-team ladder replays, October 2026)

All tested on fresh seeded games on the 16 ladder maps (swirl could not be downloaded).

| Change | Why (ladder data) | Local result vs v16 |
| --- | --- | --- |
| Item build `BUILD=1`: power shards of the main stat (focus for Cassi, Stellar, Ren, Tavi, Prism), then a durability health emblem | 1800+ teams stack focus on casters; we bought strength on everyone | 64/94 (v16 build: 51/94) |
| Per-map draft (`draft.cpp`), 6 candidate lists, picked from 12 games per map | 1700+ teams draft Cassi & Issac, Stellar, Varg first; we picked Marcella | 69/94 (fixed old draft: 61/95) |
| Wards `WARDS=1`: keep one ward forward of where we were | top teams place ~300 wards per 1000 rounds; we placed none | 69/95, margin +206 |
| Join fights `JOIN=10`: walk to an ally fighting a fight we would win together | top teams kill with 3-5 champions; we die to 2-3 at a time | 72/92 |
| All combined | | 72/93 vs v16, 69/96 vs the plain C++ port |
