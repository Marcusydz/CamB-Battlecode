// First-turn setup: camp groups, distance fields, ability roles, shared-memory slot.
#include "bot.h"
#include "config.h"
#include "util.h"
#include <algorithm>
#include <cstdio>
#include <stdexcept>
#include <tuple>
#include <variant>

using cambc::vec2;
using cambc::team;
using cambc::bitboard;
using cambc::distance_field;
using CT = cambc::champion_type;
using Item = cambc::item_type;

GroupInfo::GroupInfo(int idx, const cambc::map::camp_group& group)
    : idx(idx), group(group), mask(group.spawn_tiles()), gems(group.gem_value()), gold(group.gold_value()) {
    tiles = mask.positions();
    for (const auto& camp : group.camps()) {
        auto [hp, st] = camp_stats(camp.kind());
        health += hp;
        strength += st;
        count += 1;
        if (camp.kind() == cambc::camp_type::monolith) monolith = true;
        camp_handles.insert(handle_of(camp));
    }
    center = tiles.at(0);
}

// ------------------------------------------------------------------ setup
void Bot::setup() {
    auto& me = cambc::self();
    kind = me.champion_name();
    melee = me.attack_range() <= 1;
    static_block = cambc::map::walls() | cambc::map::holes() | cambc::map::altars()
                   | cambc::map::spawn_region(team::enemy);
    spawn = cambc::map::spawn_region(team::ally);
    width = cambc::map::width();
    height = cambc::map::height();
    altars = cambc::map::altars().positions();
    bitboard approach;
    for (auto altar : altars) approach |= around(altar, 1);
    altar_approach = approach & ~static_block;

    // Distance from every tile to the nearest altar-approach tile (static terrain only).
    // (each compute gets a fresh `visited` board: it is scratch state, not an output to reuse)
    bitboard visited_a, visited_h, visited_e;
    altar_field.compute(static_block & ~altar_approach, altar_approach, visited_a);

    // Distances to each spawn, for retreat paths and for judging contested camps.
    home_field.compute(static_block & ~spawn, spawn, visited_h);
    const auto& enemy_spawn = cambc::map::spawn_region(team::enemy);
    enemy_field.compute(static_block & ~enemy_spawn, enemy_spawn, visited_e);
    auto all = cambc::map::camp_groups();
    groups.reserve(all.size());
    for (std::size_t i = 0; i < all.size(); ++i) groups.emplace_back(static_cast<int>(i), all[i]);
    for (auto& g : groups) {
        int best = UNREACHED, mine = UNREACHED, theirs = UNREACHED;
        for (auto t : g.tiles) {
            best = std::min<int>(best, altar_field.get(t));
            mine = std::min<int>(mine, home_field.get(t));
            theirs = std::min<int>(theirs, enemy_field.get(t));
        }
        g.altar_dist = best;
        g.contested = mine + CONTEST_MARGIN > theirs;
    }

    // Ability roles per slot.
    for (const auto& ability : me.abilities()) {
        const auto slot = ability.slot();
        if (ability.is_attack_reset()) {
            reset_slot = slot;
        } else if (ability.is_dash()) {
            dash_slot = slot;
            if (ability.info())
                if (const auto* d = std::get_if<cambc::ability_dash>(&*ability.info())) {
                    dash_modifier = d->modifier;
                    dash_radius = static_cast<int>(d->radius);
                }
        } else if (ability.is_shield()) {
            shield_slot = slot;
        } else if (ability.is_heal()) {
            heal_slot = slot;
        } else if (ability.info()) {
            double modifier = 0;
            int duration = 0;
            std::visit([&](const auto& info) {
                using T = std::decay_t<decltype(info)>;
                if constexpr (requires { info.modifier; }) modifier = info.modifier;
                if constexpr (requires { info.duration; }) duration = info.duration;
                (void)sizeof(T);
            }, *ability.info());
            const bool aoe = ability.is_aoe_damage();
            const bool stuns = ability.is_cc() || (aoe && duration > 0);
            if (stuns) stun_slot = slot;
            if (modifier > 0) nukes.push_back({modifier, slot, aoe, stuns});
        }
    }
    kiter = me.attack_range() >= 3 && std::any_of(nukes.begin(), nukes.end(), [](const Nuke& n) { return n.modifier >= 1.0; });
    std::sort(nukes.begin(), nukes.end(), [](const Nuke& a, const Nuke& b) {
        return std::tie(a.modifier, a.slot, a.aoe, a.stuns) > std::tie(b.modifier, b.slot, b.aoe, b.stuns);
    });

    // Shared-memory slot: our position in the team's draft order.
    mem_slot = static_cast<int>(handle_of(me) % 8);
    try {
        auto draft = cambc::draft::current_draft(team::ally);
        std::vector<int> picks;
        for (auto p : draft) picks.push_back(static_cast<int>(p));
        int draft_id = -1;
        switch (kind) {
        case CT::ryth: draft_id = 0; break;
        case CT::vesper_vale: draft_id = 1; break;
        case CT::vendetta: draft_id = 2; break;
        case CT::cassi: case CT::issac: draft_id = 3; break;
        case CT::stellar: draft_id = 4; break;
        case CT::prism: draft_id = 5; break;
        case CT::marcella: draft_id = 6; break;
        case CT::willow: draft_id = 7; break;
        case CT::ren: draft_id = 8; break;
        case CT::varg: draft_id = 9; break;
        case CT::tavi: draft_id = 10; break;
        case CT::heart_of_ruin: draft_id = HOR_SLOT_FIX ? 11 : -1; break;
        }
        auto it = std::find(picks.begin(), picks.end(), draft_id);
        if (it == picks.end()) throw std::runtime_error("not in draft");
        mem_slot = kind == CT::issac ? 7 : static_cast<int>(it - picks.begin());
        // Caster rank: how many AoE casters (Stellar 4, Prism 5) were drafted before us. Cassi in the
        // mid squad did nothing all game on big_world vs Fish (930 camp damage), so only these two go.
        if (MID_AOE_ONLY) {
            kiter_rank = (kind == CT::stellar || kind == CT::prism) ? 0 : 9;
            for (auto i = picks.begin(); i != it; ++i)
                if ((*i == 4 || *i == 5) && kiter_rank < 9) kiter_rank += 1;
        } else {
            kiter_rank = 0;
            for (auto i = picks.begin(); i != it; ++i)
                if (*i == 4 || *i == 5 || *i == 3 || *i == 10) kiter_rank += 1;
        }
        const bool has_pair = std::find(picks.begin(), picks.end(), 3) != picks.end();
        team_size = static_cast<int>(picks.size()) + (has_pair ? 1 : 0);
    } catch (const std::exception&) {
        team_size = 3;
    }
    for (const auto& g : groups)
        if (g.monolith && g.gems >= 200) has_mid = true;
    gold_weight = GOLD_W;
    eta_w = ETA_W;
    apply_map_settings();
    if (PRIORITY) {
        // Stunners first: allies acting after them hit a stunned target.
        try {
            me.set_priority(stun_slot ? 2 : 1);
        } catch (const cambc::error&) {
        }
    }
    int ordinary_gems = 0;
    for (const auto& g : groups)
        if (!g.monolith) ordinary_gems += g.gems;
    scarce = ordinary_gems < SCARCE_GEMS;
    target = -1;
    need = 1;
    gather_turns = 0;
    no_joint_until = 0;
    stuck = 0;
    next_plan = 0;
    target_dist = UNREACHED;
    ready = true;
    if (LOG_EVERY && mem_slot == 0) {
        std::printf("map %dx%d groups (scarce=%d):\n", width, height, scarce ? 1 : 0);
        for (const auto& g : groups) {
            std::printf("  g%d n=%d hp=%d str=%d gems=%d gold=%d tile=(%d, %d) altar=%d\n", g.idx, g.count,
                        static_cast<int>(g.health), static_cast<int>(g.strength), g.gems, g.gold, g.tiles[0].x,
                        g.tiles[0].y, g.altar_dist);
        }
    }
}
