# C++ API and programming constraints

Verified 2026-10-21: official C++ API has 633 declarations (the prior 20 Oct snapshot had 631). Use [the current official page](https://2026a.battlecode.co.uk/game/api) to check freshness. Exact SDK 3.0.4 headers, notices and SHA-256 hashes are preserved in [official_docs](official_docs/manifest.json); full expanded documentation is `official_docs/BotAPICpp-rendered-2026-10-21.txt`. Consult only relevant types.

## Actual interfaces used by our bot

Include `<cambc.h>` and compile with the official C++20/WASI toolchain. `imports.h` is the underlying ABI; `wasm_simd128.h` is supplied support. Do not replace host imports with native stubs for a production build. `main()` persists and explicitly yields using `cambc::end_turn()`; draft and each champion have independent instances and ordinary C++ memory. The draft instance is detected with `cambc::is_draft()`; querying `cambc::self()` there throws. `cambc::self()` returns the authenticated champion reference.

| Area | Interfaces / types |
|---|---|
| Draft | `cambc::draft::current_draft(team)`, `pick(champion_draft)`, draft memory |
| Champion | `champion`, `champion_type`, `champion_state`, `champion_stats`; `position()`, `health()`, `max_health()`, `strength()`, `focus()`, `gem_count()`, `gold_count()`, `is_alive()`, `is_visible()` |
| Actions | `can_move()`, `move_absolute(vec2)`, `can_attack()`, `attack(...)`, `attack_tiles()`, `can_cast()`, `abilities()`; channel and ward methods are documented on `champion` |
| Abilities | `ability`, `ability_info` variant: `ability_damage`, `ability_aoe_damage`, `ability_cc`, `ability_dash`, `ability_attack_reset`, `ability_heal`, `ability_shield`; `info()`, `is_on_cooldown()`, `use(...)` overloads |
| Map | `cambc::map`, `camp_group`, `camp`, `camp_type`, `camp_state`, `camp_stats`; immutable map dimensions/geometry and camp-group metadata |
| Visibility | `cambc::vision`, `champions(team)`, visible camps, wards; visibility is personal, not automatically team-shared |
| Geometry | `vec2`, `bitboard`, `distance_field`; `compute(...)`, `compute_to(...)`, `get(vec2)`, `export_linear_buffer()` |
| Resources | `item_type`, `item_info`, passives, `consumable_type`, `altar_lock`, `teleport` |
| Execution | `round_number()`, `fuel_remaining()`, `end_turn()`, `constants`, `shared_memory_buffer`, `draft_memory_buffer`, `error`, `error_code` |

## Hazards that have affected this project

- `champion_draft` and `champion_type` are different enums: Cassi & Issac are one draft choice but two bodies. Heart of Ruin is present in both. Never assume their integer values coincide. v208 omits Heart from the explicit conversion in `Bot::setup`; the review includes a probe of the real method.
- Unreachable distance is `distance_field::unreachable` (uint16 sentinel). Do not compare to an invented large int.
- Distinguish immutable metadata from live host queries. `camp` and `champion` handles are identities; visibility checks are needed before restricted live information queries. Errors propagate as `cambc::error`; catching them prevents crashes but does not prove useful behavior.
- Array/map indices derived from memory or enums require range checks; shared claims encode group index + 1 in eight bytes. Do not expand team protocols without respecting that capacity.
- Fuel exhaustion suspends and later resumes execution. `diagnostics.botFaults == 0` does not mean no exhaustion, and `LOG_EVERY=0` suppresses printed overrun warnings. Inspect statistics `events[kind=fuelExhausted]` and replay fuel traces.
- Release behavior matters: `debug_block({...})` is skipped on the platform and under `--release`; debug success is insufficient. The current API additionally documents free debug windows; rely on the current header/page rather than old macros.

## Freshness procedure

Check official patch notes, API declaration/version changes, and installed `cambc --version` once at review start. If unchanged, reuse saved sources. When changed, refresh affected exports/headers in an isolated workspace and record hashes; don't repeatedly download to reconstruct context. The JSON manifest records provenance, not a promise that the future live server stays fixed.
