// Shopping, wards, tributes and altar locks.
#include "../core/bot.h"
#include "../core/config.h"
#include "../core/util.h"
#include <algorithm>
#include <optional>
#include <utility>

using cambc::vec2;
using cambc::team;
using cambc::bitboard;
using cambc::distance_field;
using CT = cambc::champion_type;
using Item = cambc::item_type;

// ------------------------------------------------------------------ shopping
void Bot::shop() {
    auto& me = cambc::self();
    const auto items = me.item_count();
    // Build order: Monster emblem (+10% camp damage), Jungle emblem of health
    // (-30% camp damage taken), then Power strength shards forever.
    Item primary = Item::power_shard_of_strength;
    if (BUILD == 0) {
        if (items[Item::monster_emblem_of_strength] == 0)
            primary = Item::monster_shard_of_strength;
        else if (items[Item::jungle_emblem_of_health] == 0 && BUY_JUNGLE)
            primary = Item::jungle_shard_of_health;
    } else {
        // Top ladder teams: casters stack focus, the rest strength, all through power shards.
        const bool caster = kind == CT::cassi || kind == CT::stellar || kind == CT::ren || kind == CT::tavi
                            || kind == CT::prism;
        const Item power = caster ? Item::power_shard_of_focus : Item::power_shard_of_strength;
        const Item power_emblem = caster ? Item::power_emblem_of_focus : Item::power_emblem_of_strength;
        primary = power;
        if (BUILD == 1 && items[power_emblem] >= 1 && items[Item::durability_emblem_of_health] == 0)
            primary = Item::durability_shard_of_health;
        if (BUILD == 3 && items[caster ? Item::monster_emblem_of_focus : Item::monster_emblem_of_strength] == 0)
            primary = caster ? Item::monster_shard_of_focus : Item::monster_shard_of_strength;
        if (BUILD == 4) {
            // Emblems in the order ladder #1 (Intercontinental Fish) completes them, then power forever.
            using P = std::pair<Item, Item>;  // shard, emblem
            const P pw{power, power_emblem};
            const P home{Item::home_shard_of_health, Item::home_emblem_of_health};
            const P dura{Item::durability_shard_of_health, Item::durability_emblem_of_health};
            const P brui{Item::bruiser_shard_of_health, Item::bruiser_emblem_of_health};
            std::vector<P> order;
            switch (kind) {
            case CT::ren: case CT::ryth: order = {brui, dura}; break;
            case CT::cassi: order = {pw, {Item::durability_shard_of_focus, Item::durability_emblem_of_focus}, brui}; break;
            case CT::tavi: order = {{Item::treasure_shard_of_focus, Item::treasure_emblem_of_focus},
                                    {Item::durability_shard_of_focus, Item::durability_emblem_of_focus}}; break;
            default: order = {pw, home, dura}; break;
            }
            // Fish, Sponge and Nyx open with a treasure emblem (20% more gold) in most games (2026-10-20 ladder).
            // Only where it beat v90 vs Nyx (v96: rows won 1005-991, lanes +155, corners +92); it lost
            // elk (v87), CW, islands, big_world and ambush.
            if (treasure_first && kind != CT::tavi) {
                const P treasure = caster ? P{Item::treasure_shard_of_focus, Item::treasure_emblem_of_focus}
                                          : P{Item::treasure_shard_of_strength, Item::treasure_emblem_of_strength};
                order.insert(order.begin(), treasure);
            }
            for (const auto& [shard, emblem] : order) {
                if (items[emblem] == 0) {
                    primary = shard;
                    break;
                }
            }
        }
    }
    if (KITE && kiter && KITER_BUILD && (KITE != 3 || (MID_SQUAD && kind != CT::issac && kiter_rank < MID_SQUAD && has_mid))) {
        primary = items[Item::monster_emblem_of_focus] == 0 ? Item::monster_shard_of_focus : Item::power_shard_of_focus;
    }
    if (USE_LOCKS && (!me.is_alive() || spawn.is_set(me.position()))) {
        const auto lock = cambc::consumable_type::altar_lock;
        try {
            if (me.gold_count() >= 10 && me.can_buy(lock)) me.buy(lock);
        } catch (const cambc::error&) {
        }
    }
    const auto cost = primary.cost();
    auto count = me.gold_count() / cost;
    if (BUILD == 4 && primary != Item::power_shard_of_focus && primary != Item::power_shard_of_strength)
        count = std::min<decltype(count)>(count, std::max(0, 5 - static_cast<int>(items[primary])));  // one emblem's worth
    if (count > 0) {
        try {
            if (me.can_buy(primary, count)) me.buy(primary, count);
        } catch (const cambc::error&) {
        }
    }
}

// Wards are free (one active per champion, recharging every round) and add vision, which feeds the
// threat memory and the fight evaluator. Top ladder teams place one every few rounds.
void Bot::place_ward() {
    auto& me = cambc::self();
    if (!ready || !me.is_alive() || me.is_recalling() || me.is_submitting_gems() || me.is_teleporting()) return;
    if (cambc::fuel_remaining() < FUEL_RESERVE * TURN_FUEL) return;
    try {
        if (me.ward_cooldown() > 0) return;
        const auto pos = me.position();
        const auto mine = me.ward_positions();
        for (auto w : mine)
            if (chebyshev(w, pos) < ward_gap) return;
        if (spawn.is_set(pos)) return;
        const auto tiles = me.ward_placement_tiles() & ~static_block;
        std::optional<vec2> best;
        int best_d = 1 << 30;
        for (auto t : tiles.positions()) {
            const int d = static_cast<int>(enemy_field.get(t));
            if (d < best_d) {
                best_d = d;
                best = t;
            }
        }
        if (best) me.place_ward(*best);
    } catch (const cambc::error&) {
    }
}

// ------------------------------------------------------------------ decisions
bool Bot::tribute() {
    auto& me = cambc::self();
    const auto pos = me.position();
    std::vector<vec2> adjacent;
    for (auto a : altars)
        if (chebyshev(pos, a) <= 1) adjacent.push_back(a);
    const auto enemy_locks = cambc::vision::altar_lock_tiles(team::enemy);
    std::stable_sort(adjacent.begin(), adjacent.end(),
                     [&](vec2 a, vec2 b) { return !enemy_locks.is_set(a) && enemy_locks.is_set(b); });
    for (auto altar : adjacent) {
        try {
            me.submit_gems(altar);  // against a lock this becomes an attack on the lock
            return true;
        } catch (const cambc::error&) {
            continue;
        }
    }
    return step_towards(altar_approach);
}

// Place our altar lock on an altar within range (cancels enemy tributes there).
bool Bot::place_lock(std::optional<vec2> only_altar) {
    auto& me = cambc::self();
    const auto& slot = me.consumables()[cambc::consumable_type::altar_lock];
    try {
        if (slot.count() <= 0) return false;
    } catch (const std::exception&) {
        return false;
    }
    const auto pos = me.position();
    const auto locked = cambc::vision::altar_lock_tiles();
    for (auto altar : altars) {
        if (only_altar && altar != *only_altar) continue;
        if (chebyshev(pos, altar) <= 2 && !locked.is_set(altar)) {
            try {
                if (slot.use(altar)) return true;
            } catch (const cambc::error&) {
                continue;
            }
        }
    }
    return false;
}
