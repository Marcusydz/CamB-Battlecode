// Fights against enemy champions: engage, disengage, join allies, retreat.
#include "../core/bot.h"
#include "../core/config.h"
#include "../core/util.h"
#include <algorithm>
#include <optional>
#include <tuple>

using cambc::vec2;
using cambc::team;
using cambc::bitboard;
using cambc::distance_field;
using CT = cambc::champion_type;
using Item = cambc::item_type;

// ------------------------------------------------------------------ PvP
// Damage we can deal this turn: basic attack (twice with a reset) plus best ready nuke.
double Bot::burst(const cambc::champion& me) const {
    double dmg = me.strength();
    const auto abilities = me.abilities();
    if (reset_slot && !abilities[*reset_slot].is_on_cooldown()) dmg += me.strength();
    for (const auto& n : nukes) {
        if (!abilities[n.slot].is_on_cooldown()) {
            dmg = std::max(dmg, me.strength() + n.modifier * me.focus());
            break;
        }
    }
    return dmg;
}

// Cast the heal on the ally (or self) it helps most. Returns true if cast.
bool Bot::try_heal(const cambc::champion& me) {
    if (!heal_slot || !me.can_cast()) return false;
    const auto ability = me.abilities()[*heal_slot];
    if (ability.is_on_cooldown()) return false;
    double full;
    try {
        full = ability.heal_amount();
    } catch (const cambc::error&) {
        return false;
    }
    if (full <= 0) return false;
    double reach = 0;
    if (ability.info() && std::holds_alternative<cambc::ability_heal>(*ability.info()))
        reach = std::get<cambc::ability_heal>(*ability.info()).range;
    const auto pos = me.position();
    std::vector<cambc::champion> candidates;
    if (ability.can_target_self()) candidates.push_back(me);
    if (ability.can_target_allies())
        for (const auto& a : visible_allies())
            if (a != me && a.is_alive() && a.is_visible()) candidates.push_back(a);
    const cambc::champion* best = nullptr;
    double best_value = 0.0;
    for (const auto& a : candidates) {
        const auto apos = a.position();
        if (chebyshev(apos, pos) > reach || spawn.is_set(apos)) continue;  // out of range, or on spawn (which heals anyway)
        const double missing = a.max_health() - a.health();
        if (missing <= 0) continue;
        const bool low = a.health() < 0.5 * a.max_health();
        if (std::min(missing, full) < HEAL_MIN_USE * full && !low) continue;
        const double value = std::min(missing, full) * (low ? 2.0 : 1.0);
        if (value > best_value) {
            best = &a;
            best_value = value;
        }
    }
    if (!best) return false;
    try {
        if (ability.can_target_ground() && !(ability.can_target_allies() || ability.can_target_self()))
            return ability.use(best->position());
        return ability.use(*best);
    } catch (const cambc::error&) {
        return false;
    }
}

void Bot::self_buff(const cambc::champion& me) {
    if (shield_slot && me.can_cast()) {
        const auto ability = me.abilities()[*shield_slot];
        if (!ability.is_on_cooldown() && me.health() < me.max_health() * 0.95) {
            try {
                ability.use();
            } catch (const cambc::error&) {
            }
        }
    }
}

double power(const std::vector<cambc::champion>& units) {
    double hp = 0, dps = 0;
    for (const auto& u : units) {
        hp += std::max<double>(u.health(), 1.0);
        bool caster = false;
        if (caster_power) {
            try {
                const auto k = u.champion_name();
                caster = k == CT::cassi || k == CT::issac || k == CT::stellar || k == CT::ren || k == CT::tavi
                         || k == CT::prism;
            } catch (const cambc::error&) {
            }
        }
        dps += caster ? u.focus() + 0.25 * u.strength() : u.strength() + 0.25 * u.focus();
    }
    return hp * dps;
}

// Enemy side's power, counting heals and shields the plain estimate leaves out (ladder: we lost
// elkring fights 3-31 to a team with Willow and Varg).
double enemy_power(const std::vector<cambc::champion>& units) {
    int support = 0;
    for (const auto& u : units) {
        try {
            const auto k = u.champion_name();
            if (k == CT::willow || k == CT::varg) support += 1;
        } catch (const cambc::error&) {
        }
    }
    return power(units) * (1.0 + SUPPORT_BONUS * support);
}

// Returns true if the turn was spent on PvP.
bool Bot::pvp(const cambc::champion& me, vec2 pos, int gems, double hp_frac) {
    std::vector<cambc::champion> enemies;
    for (auto& e : visible_enemies())
        if (e.is_alive()) enemies.push_back(e);
    const int now = static_cast<int>(cambc::round_number());
    for (const auto& e : enemies) seen[handle_of(e)] = {e.position(), now};
    if (enemies.empty()) return false;
    // 1. Interrupt enemy tributes: lock the altar or stun the channeler, then hit them.
    if (INTERRUPT) {
        for (const auto& e : enemies) {
            bool tributing;
            try {
                tributing = e.is_submitting_gems();
            } catch (const cambc::error&) {
                tributing = false;
            }
            if (!tributing) continue;
            const auto epos = e.position();
            vec2 near_altar = altars.at(0);
            for (auto a : altars)
                if (chebyshev(a, epos) < chebyshev(near_altar, epos)) near_altar = a;
            bool acted = false;
            if (chebyshev(pos, near_altar) <= 2 && place_lock(near_altar)) acted = true;
            if (try_spell(e, true)) acted = true;
            if (try_attack(e)) acted = true;
            if (acted) {
                act = "interrupt";
                return true;
            }
        }
    }
    std::vector<cambc::champion> near;
    const auto& enemy_spawn = cambc::map::spawn_region(team::enemy);
    const auto my_tiles = me.attack_tiles();
    for (const auto& e : enemies) {
        if (chebyshev(e.position(), pos) > 6) continue;
        // An enemy inside its own spawn heals there and cannot be reached: chasing it wasted
        // whole games (1800 turns of "engage" beside the enemy spawn). Only hit it if already in range.
        if (IGNORE_SPAWN_CAMPERS && enemy_spawn.is_set(e.position()) && !my_tiles.is_set(e.position())) continue;
        near.push_back(e);
    }
    if (near.empty()) return false;
    std::vector<cambc::champion> allies{me};
    for (const auto& a : visible_allies())
        if (a.is_alive() && a != me && chebyshev(a.position(), pos) <= 5) allies.push_back(a);
    // Enemies seen in the last few rounds but now hidden (bushes, out of vision) still fight:
    // scale the visible side up for them (power grows with count squared).
    int hidden = 0;
    if (HIDDEN_ENEMIES) {
        for (const auto& [h, s] : seen) {
            if (now - s.round > 5 || now == s.round || chebyshev(s.pos, pos) > 8) continue;
            if (enemy_spawn.is_set(s.pos)) continue;
            hidden += 1;
        }
    }
    const double scale = static_cast<double>(near.size() + hidden) / near.size();
    const double ratio = power(allies) / std::max(enemy_power(near) * scale * scale, 1.0);
    double fight_threshold = engage_ratio;
    if (!fight_heavy_map) fight_threshold *= 1.05;
    if (match_behind) fight_threshold *= 0.90;
    if (match_ahead && gems > 0) fight_threshold *= 1.12;
    // 2. Winning fight: focus fire (same rule on every ally), finish kills first.
    if (ratio >= fight_threshold && hp_frac > 0.35) {
        const double b = burst(me);
        std::vector<cambc::champion> reach;
        for (const auto& e : near)
            if (chebyshev(e.position(), pos) <= HUNT_RADIUS) reach.push_back(e);
        if (!reach.empty()) {
            auto key = [&](const cambc::champion& e) {
                const bool killable = e.health() <= b && chebyshev(e.position(), pos) <= 4;
                const double carrier_weight = match_behind ? 0.08 : 0.05;
                return std::make_tuple(!killable, e.health() / (1.0 + carrier_weight * e.gem_count()), handle_of(e));
            };
            const cambc::champion* target = &reach[0];
            auto best_key = key(reach[0]);
            for (const auto& e : reach) {
                auto k = key(e);
                if (k < best_key) {
                    best_key = k;
                    target = &e;
                }
            }
            if (FOCUS_FIRE) {
                // The team's focus target, unless we have a kill on someone else.
                const std::uint8_t tag = focus_tag(now);
                const bool kill_here = target->health() <= b && chebyshev(target->position(), pos) <= 4;
                if (tag && !kill_here)
                    for (const auto& e : reach)
                        if (handle_of(e) % 255 + 1 == tag) target = &e;
                set_focus(*target, now);
            }
            if (HOR_DASH && dash_modifier > 0) {
                // Heart of Ruin: open with the landing damage and stun on the kill target.
                std::vector<vec2> at;
                for (const auto& e : near) at.push_back(e.position());
                dash_strike(at, target->position(), 1);
            }
            self_buff(me);
            // Stun the most dangerous enemy in reach if it is not the kill target.
            const cambc::champion* dangerous = &near[0];
            for (const auto& e : near)
                if (e.strength() + 0.5 * e.focus() > dangerous->strength() + 0.5 * dangerous->focus()) dangerous = &e;
            if (*dangerous != *target && stun_slot && chebyshev(dangerous->position(), pos) <= 4
                && target->health() > b)
                try_spell(*dangerous, true);
            try_spell(*target, false);
            if (!try_attack(*target) && target->is_visible()) {
                // (a spell may just have killed it: a dead target has no position)
                step_towards(around(target->position(), me.attack_range()));
                try_attack(*target);
            }
            act = "engage";
            return true;
        }
    }
    // 2b. Losing fight with an enemy close: take a kill if one is on offer, otherwise back off
    // instead of trading hits or farming beside them (ladder: ~45% of turns dead).
    if (DISENGAGE && std::any_of(near.begin(), near.end(),
                                 [&](const cambc::champion& e) { return chebyshev(e.position(), pos) <= 4; })) {
        const double b = burst(me);
        const auto tiles = me.attack_tiles();
        const cambc::champion* kill = nullptr;
        for (const auto& e : near)
            if (tiles.is_set(e.position()) && e.health() <= b && (!kill || e.health() < kill->health())) kill = &e;
        if (kill) {
            try_spell(*kill);
            try_attack(*kill);
            act = "pvp-kill";
            return true;
        }
        claim(-1);
        if (soft_back && hp_frac >= SOFT_BACK_HP && gems < SOFT_BACK_GEMS) {
            // Healthy and carrying little: step out of reach and farm elsewhere instead of recalling
            // (vs Nyx we spent 10-30% of turns dead or walking back from spawn).
            if (!retreat_and_recall(false)) return false;
            if (caster_power) hit_in_range(me);  // parting shot: a chaser keeps pace, so hit it while leaving
            act = "back-off";
            avoid_target_after_backoff();
            return true;
        }
        retreat_and_recall();
        if (caster_power) hit_in_range(me);
        act = "disengage";
        avoid_target_after_backoff();
        return true;
    }
    // 3. Losing fight while carrying gems (or hurt): get out.
    if (ratio < FLEE_RATIO && (gems >= 10 || hp_frac < 0.6)) {
        if (gems > 0 && altar_field.get(pos) <= 2) {
            tribute();
            act = "flee-tribute";
        } else {
            retreat_and_recall();
            act = "flee";
        }
        return true;
    }
    // 4. Otherwise trade hits with anything already in range.
    if (hp_frac > 0.45) {
        near = sorted_by(near, [](const cambc::champion& e) { return e.health(); });  // weakest first
        for (const auto& e : near) {
            if (me.attack_tiles().is_set(e.position())) {
                try_spell(e);
                if (try_attack(e)) {
                    act = "pvp";
                    return true;
                }
            }
        }
    }
    return false;
}

// After backing off, drop the camp we were heading for if it is near: otherwise the next turn walks straight
// back into spell range (v272 traces on contest: approach / back-off alternating every turn beside the enemy).
void Bot::avoid_target_after_backoff() {
    if (!BACKOFF_AVOID || target < 0) return;
    const auto pos = cambc::self().position();
    if (chebyshev(groups[target].center, pos) > 8) return;
    const int now = static_cast<int>(cambc::round_number());
    groups[target].avoid_until = std::max(groups[target].avoid_until, now + BACKOFF_AVOID);
    target = -1;
    next_plan = 0;
}

// Ladder #1 hunts in packs of 3-4 and caught our lone champions ~20 times a game. Leave early when
// clearly outnumbered, before the fight starts, unless a kill is on offer right now.
bool Bot::outnumbered(const cambc::champion& me, vec2 pos, int now) {
    for (auto& e : visible_enemies())
        if (e.is_alive()) seen[handle_of(e)] = {e.position(), now};
    const auto& enemy_spawn = cambc::map::spawn_region(team::enemy);
    int enemies = 0;
    for (const auto& [h, s] : seen)
        if (now - s.round <= 4 && chebyshev(s.pos, pos) <= 8 && !enemy_spawn.is_set(s.pos)) enemies += 1;
    if (enemies < 2) return false;
    int allies = 1;
    for (const auto& a : visible_allies())
        if (a != me && a.is_alive() && chebyshev(a.position(), pos) <= 6) allies += 1;
    if (enemies < allies + OUTNUMBER) return false;
    if (spawn.is_set(pos)) return false;
    const double b = burst(me);
    const auto tiles = me.attack_tiles();
    for (auto& e : visible_enemies())
        if (e.is_alive() && tiles.is_set(e.position()) && e.health() <= b) return false;  // pvp() takes the kill
    claim(-1);
    const int gems = static_cast<int>(me.gem_count());
    if (gems > 0 && altar_field.get(pos) <= 2) tribute();
    else if (soft_back && gems < SOFT_BACK_GEMS && me.health() >= SOFT_BACK_HP * me.max_health()) {
        if (!retreat_and_recall(false)) return false;
        if (caster_power) hit_in_range(me);
        act = "back-off";
        avoid_target_after_backoff();
        return true;
    } else retreat_and_recall();
    if (caster_power) hit_in_range(me);
    act = "outnumbered";
    avoid_target_after_backoff();
    return true;
}

// Ladder replays: top teams kill with 3-5 champions and we die to 2-3 at a time. Help an ally who is
// fighting nearby when the two (or more) of us would win, instead of farming next to the fight.
bool Bot::join_fight(const cambc::champion& me, vec2 pos, int gems, double hp_frac) {
    const int join_bag_limit = match_behind ? 45 : (match_ahead ? 20 : 30);
    if (gems >= join_bag_limit || hp_frac < 0.6) return false;
    std::vector<cambc::champion> enemies;
    const auto& enemy_spawn = cambc::map::spawn_region(team::enemy);
    for (auto& e : visible_enemies())
        if (e.is_alive() && !enemy_spawn.is_set(e.position())) enemies.push_back(e);
    if (enemies.empty()) return false;
    const auto allies_all = visible_allies();
    const cambc::champion* best = nullptr;
    int best_d = 1 << 30;
    for (const auto& a : allies_all) {
        if (!a.is_alive() || a == me) continue;
        const int d = chebyshev(a.position(), pos);
        if (d <= 5 || d > JOIN) continue;
        std::vector<cambc::champion> theirs, ours{me};
        for (const auto& e : enemies)
            if (chebyshev(e.position(), a.position()) <= 6) theirs.push_back(e);
        if (theirs.empty()) continue;
        for (const auto& b : allies_all)
            if (b.is_alive() && b != me && chebyshev(b.position(), a.position()) <= 5) ours.push_back(b);
        const double join_threshold = engage_ratio * (match_behind ? 0.92 : (fight_heavy_map ? 1.0 : 1.05));
        if (power(ours) < join_threshold * enemy_power(theirs)) continue;
        if (d < best_d) {
            best_d = d;
            best = &a;
        }
    }
    if (!best) return false;
    claim(-1);
    step_towards(around(best->position(), 2));
    act = "join";
    return true;
}

// Spell and attack the weakest enemy already in attack range, without moving.
bool Bot::hit_in_range(const cambc::champion& me) {
    const auto tiles = me.attack_tiles();
    std::vector<cambc::champion> targets;
    for (auto& e : visible_enemies())
        if (e.is_alive() && tiles.is_set(e.position())) targets.push_back(e);
    if (targets.empty()) return false;
    const cambc::champion* target = &targets[0];
    for (const auto& e : targets)
        if (e.health() < target->health()) target = &e;
    try_spell(*target);
    return try_attack(*target);
}

// An enemy was seen within `radius` of p in the last `rounds` rounds.
bool Bot::enemy_near(vec2 p, int radius, int rounds, int now) const {
    for (const auto& [h, s] : seen)
        if (now - s.round <= rounds && chebyshev(p, s.pos) <= radius) return true;
    return false;
}

// Recently seen enemy positions (vision is per champion, so remember a few rounds).
std::vector<vec2> Bot::threats(int now) const {
    std::vector<vec2> out;
    for (const auto& [h, s] : seen)
        if (now - s.round <= 3) out.push_back(s.pos);
    return out;
}

// Recall if no enemy can reach us; otherwise walk home, away from enemies.
bool Bot::retreat_and_recall(bool allow_recall) {
    auto& me = cambc::self();
    auto pos = me.position();
    const int now = static_cast<int>(cambc::round_number());
    std::vector<vec2> danger;
    for (auto t : threats(now))
        if (chebyshev(pos, t) <= THREAT_RANGE) danger.push_back(t);
    for (const auto& c : visible_camps())
        if (c.is_visible() && camp_strength(c) > 0 && camp_health(c) < c.max_health()
            && chebyshev(pos, c.position()) <= camp_range(c.kind()) + 1)  // ladder: recalls inside elk range 4 died
            danger.push_back(c.position());
    if (danger.empty() && allow_recall && caster_power && now - hit_round <= 1 && !spawn.is_set(pos)) {
        // Hit by something we cannot see (an enemy in a bush or out of vision): a recall would be cancelled
        // by the next hit and restarted every turn; Nyx diagnostic deaths stood still 6-8 turns like this.
        step_towards(spawn);
        return true;
    }
    if (danger.empty()) {
        if (!allow_recall) return false;
        try {
            me.recall();
            return true;
        } catch (const cambc::error&) {
            return false;
        }
    }
    const auto blk = blocked();
    const int here = home_field.get(pos);
    auto score = [&](vec2 p) {
        int d = UNREACHED;
        for (auto t : danger) d = std::min(d, chebyshev(p, t));
        return std::make_pair(std::min(d, RETREAT_SAFE), -static_cast<int>(home_field.get(p)));
    };
    auto pick = [&](const std::vector<vec2>& options) -> std::optional<vec2> {
        std::optional<vec2> best;
        std::pair<int, int> best_score;
        for (auto p : options) {
            auto s = score(p);
            if (!best || s > best_score) {
                best = p;
                best_score = s;
            }
        }
        return best;
    };
    if (dash_slot && me.can_cast()) {
        const auto dash = me.abilities()[*dash_slot];
        if (!dash.is_on_cooldown()) {
            std::vector<vec2> options;
            for (auto p : (around(pos, 2) & ~blk).positions())
                if (home_field.get(p) <= here) options.push_back(p);
            auto best = pick(options);
            if (best && *best != pos) {
                try {
                    if (dash.use(*best)) pos = *best;
                } catch (const cambc::error&) {
                }
            }
        }
    }
    if (me.can_move()) {
        auto options = around(pos, 1) & ~blk;
        for (int tries = MOVE_RETRY ? 3 : 1; tries > 0; --tries) {
            auto best = pick(options.positions());
            if (!best || *best == pos) break;
            try {
                if (me.move_absolute(*best)) break;
            } catch (const cambc::error&) {
                break;
            }
            options.set(*best, false);  // hidden unit there: try the next best tile
        }
    }
    return true;
}
