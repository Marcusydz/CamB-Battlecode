// Spells and basic attacks, and fighting camps.
#include "../core/bot.h"
#include "../core/config.h"
#include "../core/util.h"
#include <algorithm>

using cambc::vec2;
using cambc::team;
using cambc::bitboard;
using cambc::distance_field;
using CT = cambc::champion_type;
using Item = cambc::item_type;

// ------------------------------------------------------------------ combat
// Cast the strongest ready damage spell (or a stun if asked) on a visible unit in range.
template <class T> bool Bot::try_spell(const T& target, bool want_stun) {
    auto& me = cambc::self();
    if (!me.can_cast() || !target.is_visible()) return false;
    const auto pos = me.position();
    const auto tpos = target.position();
    if (chebyshev(pos, tpos) > 4) return false;
    const auto abilities = me.abilities();
    std::vector<std::uint32_t> order;
    if (want_stun && stun_slot) order.push_back(*stun_slot);
    for (const auto& n : nukes)
        if (!(want_stun && stun_slot && n.slot == *stun_slot)) order.push_back(n.slot);
    for (auto slot : order) {
        const auto& ability = abilities[slot];
        if (ability.is_on_cooldown()) continue;
        try {
            bool ok;
            if (ability.can_target_ground() && !ability.can_target_enemies())
                ok = ability.use(tpos);
            else
                ok = ability.use(target);
            if (ok) return true;
        } catch (const cambc::error&) {
            try {
                if (ability.use(tpos)) return true;
            } catch (const cambc::error&) {
            }
        }
    }
    return false;
}

template <class T> bool Bot::try_attack(const T& target, const std::vector<T>& others) {
    auto& me = cambc::self();
    if (RANGED_RESET && !melee && reset_slot && !me.can_attack() && me.can_cast() && target.is_visible()
        && target.health() > 0 && me.attack_tiles().is_set(target.position())) {
        // Ranged: walking used the attack. An attack reset gives it back (GameRules, "A champion turn").
        const auto reset = me.abilities()[*reset_slot];
        if (!reset.is_on_cooldown()) {
            try {
                reset.use();
            } catch (const cambc::error&) {
            }
        }
    }
    if (!me.can_attack() || !target.is_visible() || target.health() <= 0
        || !me.attack_tiles().is_set(target.position()))
        return false;
    try {
        me.attack(target);
    } catch (const cambc::error&) {
        return false;
    }
    // Attack reset: a second basic attack this turn.
    if (reset_slot && me.can_cast()) {
        const auto reset = me.abilities()[*reset_slot];
        if (!reset.is_on_cooldown()) {
            try {
                const T* nxt = &target;
                if (!(target.is_visible() && target.health() > 0)) {
                    // First hit killed it: spend the reset on something else in range.
                    const auto tiles = me.attack_tiles();
                    nxt = nullptr;
                    for (const auto& c : others) {
                        if (c.is_visible() && c.health() > 0 && tiles.is_set(c.position())
                            && (!nxt || c.health() < nxt->health()))
                            nxt = &c;
                    }
                }
                if (nxt) {
                    reset.use();
                    if (me.can_attack()) me.attack(*nxt);
                }
            } catch (const cambc::error&) {
            }
        }
    }
    return true;
}

// In range: shield, spell, attack. Out of range: step first (damage spells are exclusive with
// movement, so casting first used to cost the step), then attack whatever is now in range, and
// cast only if we could not move.
void Bot::fight(const cambc::camp& target, const std::vector<cambc::camp>& others) {
    auto& me = cambc::self();
    if (!target.is_visible()) return;
    if (KITE && kiter && kite(target, others)) return;
    if (HOR_DASH && dash_modifier > 0) {
        std::vector<vec2> live;
        bool hostile = camp_strength(target) > 0;
        if (target.is_visible() && camp_health(target) > 0) live.push_back(target.position());
        for (const auto& c : others)
            if (c.is_visible() && camp_health(c) > 0) {
                live.push_back(c.position());
                if (camp_strength(c) > 0) hostile = true;
            }
        if (dash_strike(live, target.position(), hostile ? 1 : 2)) {
            try_attack(target, others);
            for (const auto& c : others)
                if (try_attack(c, others)) break;
            return;
        }
    }
    if (me.attack_tiles().is_set(target.position())) {
        if (camp_strength(target) > 0 && me.health() < 0.9 * me.max_health())
            self_buff(me);  // Varg's shield used to be PvP-only
        try_spell(target);
        try_attack(target, others);
        return;
    }
    bool moved = step_towards(around(target.position(), me.attack_range()), false);
    if (try_attack(target, others)) return;
    for (const auto& c : others)
        if (try_attack(c, others)) return;
    fight_stuck = moved ? 0 : fight_stuck + 1;
    if (!moved && reach_fallback && fight_stuck >= reach_fallback) {
        // Every tile in reach of the target is taken (an elk walled in by its own group's clusters:
        // concentrated_wealth, melee champions stood still for 150 rounds). Go for the nearest camp
        // we can get next to instead.
        std::vector<const cambc::camp*> order;
        bool mono = target.kind() == cambc::camp_type::monolith;
        for (const auto& c : others) {
            if (c.kind() == cambc::camp_type::monolith) mono = true;  // corridors: walked into monolith range
            // Only harmless clusters (the ring around the elk); boss_battle got 2.6x slower otherwise.
            if (c.is_visible() && camp_health(c) > 0 && camp_strength(c) <= 0) order.push_back(&c);
        }
        if (mono) order.clear();
        const auto pos = me.position();
        std::sort(order.begin(), order.end(), [&](const cambc::camp* a, const cambc::camp* b) {
            return chebyshev(pos, a->position()) < chebyshev(pos, b->position());
        });
        for (std::size_t i = 0; i < order.size() && i < 4 && !moved; ++i)
            moved = step_towards(around(order[i]->position(), me.attack_range()), false);
        if (moved)
            for (const auto& c : others)
                if (try_attack(c, others)) return;
    }
    if (!moved) try_spell(target);
}

// Casters: stand where the spells reach the camp (4) but no attacking camp reaches us (3), and only
// cast. Returns false to fall back to the normal fight (no attacking camp, or a melee ally tanking).
bool Bot::kite(const cambc::camp& target, const std::vector<cambc::camp>& others) {
    auto& me = cambc::self();
    std::vector<const cambc::camp*> all{&target};
    for (const auto& c : others) all.push_back(&c);
    std::vector<vec2> hostile;
    for (auto c : all)
        if (c->is_visible() && camp_health(*c) > 0 && camp_strength(*c) > 0
            && !(MONO_KITE == 2 && c->kind() == cambc::camp_type::monolith))
            hostile.push_back(c->position());
    if (hostile.empty()) return false;
    if (KITE == 3) {
        bool mono = false;
        for (auto c : all)
            if (c->kind() == cambc::camp_type::monolith) mono = true;
        if (!mono) return false;  // only monolith fights are kited
    }
    if (KITE == 2) {
        for (const auto& a : cambc::vision::champions(team::ally)) {
            if (a == me || !a.is_alive() || a.attack_range() > 1) continue;
            for (auto h : hostile)
                if (chebyshev(a.position(), h) <= 1) return false;
        }
    }
    // Spell target: the camp with the most other camps within AoE radius 1, then the weakest.
    const cambc::camp* aim = &target;
    int aim_n = -1;
    for (auto c : all) {
        if (!c->is_visible() || camp_health(*c) <= 0) continue;
        int n = 0;
        for (auto d : all)
            if (d != c && d->is_visible() && camp_health(*d) > 0 && chebyshev(c->position(), d->position()) <= 1) n += 1;
        if (n > aim_n || (n == aim_n && camp_health(*c) < camp_health(*aim))) {
            aim = c;
            aim_n = n;
        }
    }
    bitboard ring = around(aim->position(), 4);
    for (auto h : hostile) ring &= ~around(h, 3);
    const auto pos = me.position();
    bool ready_spell = false;
    const auto abilities = me.abilities();
    for (const auto& n : nukes)
        if (!abilities[n.slot].is_on_cooldown()) ready_spell = true;
    if (ring.is_set(pos)) {
        if (ready_spell) try_spell(*aim);
        return true;
    }
    if (!step_towards(ring, false)) return false;  // no safe tile reachable: fight normally
    return true;
}

// Damaging dash (Heart of Ruin): land where it hits the most hostiles (each hit tile is within
// dash_radius of the landing, so a melee champion can still attack one of them this turn).
// `want` counts as three hits. Returns true if the dash was cast.
bool Bot::dash_strike(const std::vector<vec2>& hostiles, std::optional<vec2> want, int min_hits) {
    auto& me = cambc::self();
    if (!HOR_DASH || dash_modifier <= 0 || !dash_slot || !me.can_cast() || hostiles.empty()) return false;
    const auto dash = me.abilities()[*dash_slot];
    if (dash.is_on_cooldown()) return false;
    const auto pos = me.position();
    std::optional<vec2> best;
    int best_hits = 0, best_score = 0;
    for (auto p : (around(pos, 2) & ~blocked()).positions()) {
        if (p == pos) continue;
        int hits = 0, score = 0;
        for (auto h : hostiles)
            if (chebyshev(p, h) <= dash_radius) {
                hits += 1;
                score += (want && h == *want) ? 3 : 1;
            }
        if (hits >= min_hits && score > best_score) {
            best = p;
            best_hits = hits;
            best_score = score;
        }
    }
    if (!best || best_hits < min_hits) return false;
    try {
        return dash.use(*best);
    } catch (const cambc::error&) {
        return false;
    }
}

// try_spell / try_attack are also used by pvp.cpp and turn.cpp.
template bool Bot::try_spell<cambc::camp>(const cambc::camp&, bool);
template bool Bot::try_spell<cambc::champion>(const cambc::champion&, bool);
template bool Bot::try_attack<cambc::camp>(const cambc::camp&, const std::vector<cambc::camp>&);
template bool Bot::try_attack<cambc::champion>(const cambc::champion&, const std::vector<cambc::champion>&);
