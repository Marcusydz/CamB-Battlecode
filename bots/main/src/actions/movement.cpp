// Walking and dashing towards goals.
#include "../core/bot.h"
#include "../core/config.h"
#include "../core/util.h"
#include <optional>

using cambc::vec2;
using cambc::team;
using cambc::bitboard;
using cambc::distance_field;
using CT = cambc::champion_type;
using Item = cambc::item_type;

// ------------------------------------------------------------------ movement
bitboard Bot::blocked() const {
    auto b = static_block | cambc::vision::champion_tiles() | cambc::vision::camp_tiles()
             | cambc::vision::teleport_tiles();
    b.set(cambc::self().position(), false);
    return b;
}

// Distance from the map border: equal-length paths prefer the interior over the edge.
int Bot::inner(vec2 p) const {
    return std::min({p.x, p.y, width - 1 - p.x, height - 1 - p.y});
}

// Walk downhill on a group's static distance field: no search, so long walks cost almost no fuel
// (a goal search from far away on a 60x60 map cost most of the turn).
bool Bot::step_static(int g) {
    auto& me = cambc::self();
    auto origin = me.position();
    const auto blk = blocked();
    int here = gdist(g, origin);
    if (here <= 0 || here >= UNREACHED) return false;
    auto pick = [&](const bitboard& options) -> std::optional<vec2> {
        std::optional<vec2> best;
        int best_d = 0, best_in = 0;
        for (auto p : options.positions()) {
            const int d = gdist(g, p), in = inner(p);
            if (!best || d < best_d || (d == best_d && in > best_in)) {
                best = p;
                best_d = d;
                best_in = in;
            }
        }
        return best;
    };
    bool moved = false;
    if (here >= 3 && dash_slot && me.can_cast()) {
        const auto dash = me.abilities()[*dash_slot];
        if (!dash.is_on_cooldown()) {
            auto best = pick(around(origin, 2) & ~blk);
            if (best && gdist(g, *best) <= here - 2) {
                try {
                    if (dash.use(*best)) {
                        moved = true;
                        origin = *best;
                        here = gdist(g, *best);
                    }
                } catch (const cambc::error&) {
                }
            }
        }
    }
    if (me.can_move() && here > 0) {
        auto best = pick(around(origin, 1) & ~blk);
        if (best && gdist(g, *best) < here) {
            try {
                if (me.move_absolute(*best)) moved = true;
            } catch (const cambc::error&) {
            }
        }
    }
    return moved;
}

// Walk (and dash when it gains two tiles) towards the nearest goal tile.
bool Bot::step_towards(const bitboard& goals, bool allow_dash) {
    auto& me = cambc::self();
    auto origin = me.position();
    if (goals.is_set(origin)) return false;
    const auto blk = blocked();
    const auto free_goals = goals & ~blk;
    if (free_goals.is_empty()) return false;
    distance_field field;
    bitboard visited;
    // Search from the goals only until our neighbourhood is reached (much cheaper on big maps).
    field.compute_to(blk & ~free_goals, free_goals, around(origin, 2) & ~blk, visited);
    int here = field.get(origin);
    if (here <= 0 || here >= UNREACHED) return false;
    auto pick = [&](const bitboard& options) -> std::optional<vec2> {
        std::optional<vec2> best;
        int best_d = 0, best_in = 0;
        for (auto p : options.positions()) {
            const int d = field.get(p), in = inner(p);
            if (!best || d < best_d || (d == best_d && -in < -best_in)) {
                best = p;
                best_d = d;
                best_in = in;
            }
        }
        return best;
    };
    bool moved = false;
    // Dash first: it is compatible with walking, so dash + walk covers three tiles.
    if (allow_dash && here >= 3 && dash_slot && me.can_cast()) {
        const auto dash = me.abilities()[*dash_slot];
        if (!dash.is_on_cooldown()) {
            auto best = pick(around(origin, 2) & ~blk);
            if (best && field.get(*best) <= here - 2) {
                try {
                    if (dash.use(*best)) {
                        moved = true;
                        origin = *best;
                        here = field.get(*best);
                    }
                } catch (const cambc::error&) {
                }
            }
        }
    }
    if (me.can_move() && here > 0) {
        auto best = pick(around(origin, 1) & ~blk);
        if (best && field.get(*best) < here) {
            try {
                if (me.move_absolute(*best)) moved = true;
            } catch (const cambc::error&) {
            }
        }
    }
    return moved;
}
