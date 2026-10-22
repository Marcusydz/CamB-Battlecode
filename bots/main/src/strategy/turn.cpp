// One champion turn: survive, fight, bank, shop, then farm.
#include "../core/bot.h"
#include "../core/config.h"
#include "../core/util.h"
#include <algorithm>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

using cambc::vec2;
using cambc::team;
using cambc::bitboard;
using cambc::distance_field;
using CT = cambc::champion_type;
using Item = cambc::item_type;

void Bot::update_match_state(int now) {
    const int ours = static_cast<int>(cambc::global_submitted_gems(team::ally));
    const int theirs = static_cast<int>(cambc::global_submitted_gems(team::enemy));
    match_lead = ours - theirs;
    // Ignore tiny early swings. A meaningful lead should make us protect carried gems;
    // a deficit should make us value reachable fights and enemy carriers more highly.
    match_ahead = now >= 200 && match_lead >= 60;
    match_behind = now >= 200 && match_lead <= -60;
}

// ------------------------------------------------------------------ turn
void Bot::play_turn() {
    act = "?";
    const auto start = cambc::fuel_remaining();
    const auto r0 = cambc::round_number();
    turn();
    if (WARDS) place_ward();
    if (MAP_FIELDS && ready) precompute_fields();
    const auto used = start - cambc::fuel_remaining();
    if (cambc::round_number() != r0) {
        overruns += 1;
        if (LOG_EVERY) std::printf("OVERRUN r%u act=%s\n", r0, act.c_str());
    }
    if (used > max_fuel && turns > 1) max_fuel = used;
    if (used > TURN_FUEL * 96 / 100 && turns > 1 && LOG_EVERY)
        std::printf("heavy turn r%u used=%llu act=%s\n", cambc::round_number(),
                    static_cast<unsigned long long>(used), act.c_str());
    turns += 1;
    acts[act] += 1;
    prev_act = act;
    const auto now = cambc::round_number();
    if (ACT_LOG_EVERY && now % ACT_LOG_EVERY == 0) {
        std::vector<std::pair<std::string, int>> tally(acts.begin(), acts.end());
        std::stable_sort(tally.begin(), tally.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
        std::printf("acts r%u %s:", now, kind_name(kind));
        for (const auto& [k, v] : tally) std::printf(" %s=%d", k.c_str(), v);
        std::printf("\n");
    }
    if (LOG_EVERY && now % LOG_EVERY == 0) {
        auto& me = cambc::self();
        std::string where = "dead";
        if (me.is_alive()) where = cambc::to_string(me.position());
        std::printf("r%u %s %s foc%d hp%d/%d gem%u gold%u tgt=%d st=%d act=%s fuelmax=%llu\n", now, kind_name(kind),
                    where.c_str(), static_cast<int>(me.focus()), static_cast<int>(me.health()), static_cast<int>(me.max_health()), me.gem_count(),
                    me.gold_count(), target, target < 0 ? -1 : groups[target].state, act.c_str(),
                    static_cast<unsigned long long>(max_fuel));
    }
}

void Bot::turn() {
    auto& me = cambc::self();
    if (!ready) setup();
    const int now = static_cast<int>(cambc::round_number());
    update_match_state(now);
    const bool was_waiting_on_spawn = waited_on_spawn;
    waited_on_spawn = false;

    if (!me.is_alive()) {
        if (last_alive && DEATH_LOG) {
            int enemies = 0;
            for (const auto& [h, s] : seen)
                if (now - s.round <= 3 && chebyshev(s.pos, last_pos) <= 8) enemies += 1;
            std::printf("death r%d %s at %d,%d act=%s gems=%d enemies=%d allies=%d\n", now, kind_name(kind),
                        last_pos.x, last_pos.y, prev_act.c_str(), last_gems, enemies, last_allies);
        }
        if (last_alive && DEATH_AVOID && death_avoid && enemy_near(last_pos, 8, 6, now)) {
            // Killed by champions: stay away from camps there for a while (interlock vs Fish: died
            // 13 times walking back into their side for the same camp).
            for (auto& g : groups)
                if (chebyshev(g.center, last_pos) <= 8) g.avoid_until = std::max(g.avoid_until, now + DEATH_AVOID);
            if (target >= 0) groups[target].avoid_until = std::max(groups[target].avoid_until, now + DEATH_AVOID);
            target = -1;
        }
        if (last_alive && CAMP_DEATH_AVOID && target >= 0 && !enemy_near(last_pos, 8, 6, now)
            && chebyshev(groups[target].center, last_pos) <= 8) {
            // Killed by the camp itself (vs Nyx after the patch: 16 of 21 deaths on lanes, five at
            // the same group): leave it alone for a while.
            groups[target].avoid_until = std::max(groups[target].avoid_until, now + CAMP_DEATH_AVOID);
            target = -1;
        }
        last_alive = false;
        claim(-1);
        shop();
        act = "dead";
        return;
    }
    if (CANCEL_RECALL && me.is_recalling() && me.health() >= 0.6 * me.max_health()) {
        // A kill in reach beats going home: cancel (allowed once a turn) and let pvp() take it.
        const double b = burst(me);
        const auto tiles = me.attack_tiles();
        const auto& enemy_spawn = cambc::map::spawn_region(team::enemy);
        for (auto& e : visible_enemies())
            if (e.is_alive() && tiles.is_set(e.position()) && e.health() <= b && !enemy_spawn.is_set(e.position())) {
                try {
                    me.cancel_recall();
                } catch (const cambc::error&) {
                }
                break;
            }
    }
    if (me.is_recalling() || me.is_submitting_gems() || me.is_teleporting()) {
        act = "channel";
        return;
    }

    const auto pos = me.position();
    const bool at_spawn = spawn.is_set(pos);
    if (at_spawn || kind == CT::tavi) shop();
    update_groups(now, pos);
    if (SEEN_EVERY_TURN) {
        // Enemy memory feeds retreat paths (threats()). It used to be refreshed only in pvp()/outnumbered(),
        // which a low-health champion never reaches: it fled from where enemies were 3 rounds ago, then
        // recalled beside them (ladder, v270: 23 of 29 deaths a game came while below LOW_HP).
        for (auto& e : visible_enemies())
            if (e.is_alive()) seen[handle_of(e)] = {e.position(), now};
    }
    if (TEAM_PLAN) compute_group_fields();

    if (heal_slot) try_heal(me);
    const int gems = static_cast<int>(me.gem_count());
    const double hp_frac = me.health() / me.max_health();

    // On spawn and hurt: stay put for one turn. Spawn heals fully at the END of the turn, so
    // leaving in the same turn (e.g. right after a recall lands) wastes the whole recall.
    // Only one turn: still hurt after that means an enemy outside the spawn is hitting us
    // every round, and standing there just lets it (seen: 46 turns stuck on elkring).
    if (at_spawn && hp_frac < SPAWN_HEAL_TO && !was_waiting_on_spawn) {
        waited_on_spawn = true;
        claim(-1);
        if (gems > 0 && altar_field.get(pos) <= 1)
            tribute();
        else
            hit_in_range(me);
        act = "heal";
        return;
    }

    // Spawn camping (1.0.2 ladder: 66 deaths at our spawn on contest, 53 on fragment): enemies waiting
    // outside our spawn killed champions one at a time as they walked out. Stay on spawn, which heals
    // fully every turn, and hit what is in range until together we would win the fight.
    static int hold_turns = 0;
    if (SPAWN_HOLD && spawn_hold && at_spawn && hold_turns < SPAWN_HOLD_MAX) {
        const auto& enemy_spawn = cambc::map::spawn_region(team::enemy);
        std::vector<cambc::champion> theirs, ours{me};
        for (auto& e : visible_enemies())
            if (e.is_alive() && !enemy_spawn.is_set(e.position()) && chebyshev(e.position(), pos) <= SPAWN_HOLD)
                theirs.push_back(e);
        if (!theirs.empty()) {
            for (const auto& a : cambc::vision::champions(team::ally))
                if (a.is_alive() && a != me && chebyshev(a.position(), pos) <= 4) ours.push_back(a);
            if (power(ours) < engage_ratio * enemy_power(theirs)) {
                hold_turns += 1;
                claim(-1);
                self_buff(me);
                hit_in_range(me);
                act = "spawn-hold";
                return;
            }
        }
    }
    if (!at_spawn) hold_turns = 0;

    const double prev_hp = last_hp;
    if (me.health() < prev_hp) hit_round = now;
    last_hp = me.health();
    last_alive = true;
    last_pos = pos;
    last_gems = gems;
    if (DEATH_LOG) {
        last_allies = 0;
        for (const auto& a : cambc::vision::champions(team::ally))
            if (a != me && a.is_alive() && chebyshev(a.position(), pos) <= 6) last_allies += 1;
    }
    // Survival.
    if (!at_spawn && hp_frac < (low_hp + (gems >= 30 ? LOW_HP_GEMS : 0.0))) {
        if (me.health() < prev_hp) {
            // Being hit right now: running rarely escapes a chaser, so first take
            // whatever is in range (a kill resets the chaser problem entirely).
            bool any = false;
            const auto tiles = me.attack_tiles();
            const double b = LOWHP_FIGHT_KILL_ONLY ? burst(me) : 0.0;
            for (auto& e : visible_enemies())
                if (e.is_alive() && tiles.is_set(e.position()) && (!LOWHP_FIGHT_KILL_ONLY || e.health() <= b))
                    any = true;
            if (any) {
                self_buff(me);
                if (hit_in_range(me)) {
                    act = "lowhp-fight";
                    return;
                }
            }
        }
        claim(-1);
        retreat_and_recall();
        act = "lowhp";
        return;
    }

    // Bank immediately if that wins the game, or at the very end.
    if (gems > 0 && !altar_blocked && (cambc::global_submitted_gems(team::ally) + gems >= cambc::constants::winning_gems || now > 1940)) {
        tribute();
        return;
    }

    if (OUTNUMBER && outnumbered(me, pos, now)) return;
    if (pvp(me, pos, gems, hp_frac)) return;
    if (JOIN && join_fight(me, pos, gems, hp_frac)) return;
    if (CONTEST && hp_frac > 0.5 && gems < 30) {
        // An enemy on a cleared camp is capturing it: any of us in the region resets their timer.
        const auto& enemy_spawn = cambc::map::spawn_region(team::enemy);
        for (auto& e : visible_enemies()) {
            if (!e.is_alive() || chebyshev(e.position(), pos) > 6 || enemy_spawn.is_set(e.position())) continue;
            for (auto& g : groups) {
                if (g.state != 2 || !g.mask.is_set(e.position())) continue;
                std::vector<cambc::champion> theirs, ours{me};
                for (auto& x : visible_enemies())
                    if (x.is_alive() && chebyshev(x.position(), e.position()) <= 6) theirs.push_back(x);
                for (const auto& a : cambc::vision::champions(team::ally))
                    if (a.is_alive() && a != me && chebyshev(a.position(), pos) <= 5) ours.push_back(a);
                if (power(ours) < engage_ratio * enemy_power(theirs)) continue;
                claim(-1);
                if (!g.mask.is_set(pos)) step_towards(g.mask);
                hit_in_range(me);
                act = "contest";
                return;
            }
        }
    }

    // Lock any altar we pass (our own lock never blocks us).
    if (USE_LOCKS && cambc::fuel_remaining() > FUEL_RESERVE * TURN_FUEL) place_lock();

    // Hold a capture in progress, unless an ally (lower handle) is already holding it:
    // one champion captures, the rest move on (rewards split, so nothing is lost).
    for (auto& g : groups) {
        if (g.state == 2 && g.mask.is_set(pos)) {
            bool held = false;
            if (SOLO_CAPTURE) {
                for (const auto& a : cambc::vision::champions(team::ally)) {
                    if (a != me && a.is_alive() && g.mask.is_set(a.position()) && handle_of(a) < handle_of(me)) {
                        held = true;
                        break;
                    }
                }
            }
            if (!held) {
                claim(g.idx);
                act = "capture";
                return;
            }
            g.state = 3;
            g.available_at = now + g.group.max_respawn_timer() + CAPTURE_TIME;
            target = -1;
            next_plan = 0;
        }
    }

    // Spend gold: with an empty bag and enough for a full emblem, recall to shop.
    const bool busy_here = target >= 0 && (groups[target].state == 1 || groups[target].state == 2)
                           && target_dist <= 7 && me.gold_count() < 2 * shop_gold;
    bool threatened = false;
    for (auto t : threats(now))
        if (chebyshev(pos, t) <= THREAT_RANGE + 2) threatened = true;
    if (gems == 0 && me.gold_count() >= shop_gold && kind != CT::tavi && !at_spawn && now < 1800
        && !busy_here && !threatened) {
        claim(-1);
        retreat_and_recall();
        act = "shop-recall";
        return;
    }

    int tgt = target;
    const bool valid = tgt >= 0 && now >= groups[tgt].avoid_until && groups[tgt].available_at <= now + 2 + MAX_WAIT
                       && (MAX_WAIT || groups[tgt].state != 3 || chebyshev(pos, groups[tgt].center) > 9);
    const bool engaged = valid && (groups[tgt].state == 1 || groups[tgt].state == 2)
                         && chebyshev(pos, groups[tgt].center) <= 6;
    const bool low_fuel = cambc::fuel_remaining() < LOW_FUEL * TURN_FUEL;
    if ((!valid && !(low_fuel && tgt >= 0)) || (now >= next_plan && !engaged && !low_fuel)) {
        if (TEAM_PLAN && gfields.size() >= groups.size()) {
            // Static per-group distance fields: no search from our own position needed.
            tgt = -1;
            if (MID_SQUAD && kiter && kind != CT::issac && kiter_rank < MID_SQUAD && me.focus() >= MONO_FOCUS) {
                const GroupInfo* mid = nullptr;
                for (const auto& g : groups)
                    if (g.monolith && g.gems >= 200 && (!mid || g.gems > mid->gems)) mid = &g;
                if (mid) {
                    const int d = gdist(mid->idx, pos);
                    if (mid->state != 3 && now >= mid->avoid_until && mid->available_at <= now + d && d < UNREACHED
                        && !enemy_near(mid->center, MONOLITH_CLEAR, 2, now)) {
                        tgt = mid->idx;
                        need = 1;
                    }
                }
            }
            if (tgt < 0) tgt = team_plan(now);
            target = tgt;
            next_plan = now + REPLAN_EVERY;
            target_dist = tgt < 0 ? UNREACHED : gdist(tgt, pos);
        } else if (MAP_FIELDS && gfields.size() >= groups.size()) {
            plan_cap = PLAN_RADIUS;
            tgt = choose_group(now, nullptr);
            if (tgt < 0 && !low_fuel) {
                plan_cap = UNREACHED;  // nothing within PLAN_RADIUS: the whole map
                tgt = choose_group(now, nullptr);
            }
            target = tgt;
            next_plan = now + REPLAN_EVERY;
            target_dist = tgt < 0 ? UNREACHED : gdist(tgt, pos);
        } else {
        distance_field my_field;
        bitboard roots, visited;
        roots.set(pos);
        my_field.compute(static_block, roots, visited, PLAN_RADIUS);
        tgt = choose_group(now, &my_field);
        if (tgt < 0 && !low_fuel) {
            // Nothing within PLAN_RADIUS: search the whole map rather than idle.
            bitboard visited2;
            my_field.compute(static_block, roots, visited2);
            tgt = choose_group(now, &my_field);
        }
        target = tgt;
        next_plan = now + REPLAN_EVERY;
        target_dist = tgt < 0 ? UNREACHED : min_dist(my_field, groups[tgt]);
        }
    } else {
        target_dist = std::max(0, target_dist - 1);
    }

    if (gems > 0 && !altar_blocked) {
        const int altar_dist = altar_field.get(pos);
        const bool prism = kind == CT::prism;
        const int base_cap = prism ? PRISM_GEM_CAP : gem_cap;
        const int cap = match_ahead ? std::min(base_cap, 40) : base_cap;
        const bool on_the_way = altar_dist <= target_dist && (!prism || gems >= PRISM_EARLY_BANK);
        if (tgt < 0 || gems >= cap || on_the_way) {
            claim(-1);
            tribute();
            act = "tribute";
            return;
        }
    }

    if (tgt < 0) {
        act = "idle";
        claim(-1);
        // Nothing worth doing: go home to heal and shop if useful, else wait near an altar.
        if (!at_spawn && (hp_frac < 0.8 || me.gold_count() >= 60))
            retreat_and_recall();
        else if (step_towards(altar_approach))
            act = "idle-altar";  // guard an altar: lock it, catch enemies tributing
        return;
    }

    auto& tg = groups[tgt];
    if (tg.monolith && tg.state != 2 && enemy_near(tg.center, MONOLITH_CLEAR, 2, now)) {
        // Enemies arrived: a half-dead monolith team is easy kills and hands them the reward. Leave it.
        tg.avoid_until = now + MONOLITH_MEMORY;
        target = -1;
        next_plan = 0;
        claim(-1);
        retreat_and_recall();
        act = "monolith-abort";
        return;
    }
    claim(tgt);
    std::vector<cambc::camp> camps;
    for (auto& c : visible_camps())
        if (tg.camp_handles.count(handle_of(c))) camps.push_back(c);
    if (!camps.empty()) {
        bool untouched = true;
        for (const auto& c : camps)
            if (camp_health(c) < c.max_health()) untouched = false;
        if (need > 1 && untouched) {
            // Untouched joint target: wait just outside camp range until helpers arrive.
            int near = 0;
            for (const auto& a : cambc::vision::champions(team::ally))
                if (a.is_alive() && a != me && chebyshev(a.position(), pos) <= 4) near += 1;
            if (near < need - 1 && gems > 0) {
                // Bank while helpers make their way here, instead of standing with gems.
                tribute();
                act = "gather-bank";
                return;
            }
            gather_turns += 1;
            const bool sw = near < need - 1 && joint_claim_elsewhere(tgt, camp_dps(me), me.max_health());
            if (gather_turns > (tg.monolith ? GATHER_MAX * 5 : GATHER_MAX) || sw) {
                if (sw) {
                    // Leave it to the ally's joint target; without this marker the replan
                    // could pick this same camp again and loop (500 turns on a 100x100 map).
                    tg.avoid_until = now + 30;
                } else {
                    tg.avoid_until = now + 50;
                    no_joint_until = now + 40;  // stop starting joint targets for a while
                }
                target = -1;
                next_plan = 0;
                gather_turns = 0;
                act = sw ? "gather-switch" : "gather-giveup";
                return;
            }
            if (near < need - 1) {
                bitboard ring;
                for (const auto& c : camps) ring |= camp_waiting_area(c.position(), camp_range(c.kind()));
                for (const auto& c : camps) ring &= ~around(c.position(), camp_range(c.kind()));
                claim(tgt);
                act = "gather";
                step_towards(ring);
                return;
            }
        }
        gather_turns = 0;
        // Hit the most dangerous camp first, then the weakest. Sort on keys read once: sorting the
        // camp objects themselves crashed (a virtual health() call on a camp mid-move in stable_sort's
        // buffer hit the pure virtual base: wasm `unreachable`, elk_arena).
        camps = sorted_by(camps, [](const cambc::camp& c) { return std::make_pair(camp_strength(c) <= 0, camp_health(c)); });
        std::vector<cambc::camp> rest(camps.begin() + 1, camps.end());
        fight(camps[0], rest);
        close_turns = 0;
        act = "fight";
        return;
    }
    if (tg.state == 2) {
        // Reward available: stand on a spawn tile to capture.
        step_towards(tg.mask);
        act = "to-capture";
        return;
    }
    // Approach the camp: get within attack range of its tiles, but never stand on a
    // spawn tile of a respawning group (an occupied spawn tile stops the respawn).
    const bool visible = !(tg.mask & cambc::vision::visible_tiles()).is_empty();
    // Bushes can hide a camp from attack range; then close in to adjacent tiles.
    // Close by but never fighting it (hidden in bushes, already taken): the stuck counter alone
    // can loop, since closing in to radius 1 resets it (seen: Cassi 300 rounds on big_world).
    int closest = 1 << 20;
    for (auto t : tg.tiles) closest = std::min(closest, chebyshev(pos, t));
    // Walking away from it (the path to adjacent tiles goes round a wall) keeps the count: resetting
    // there flipped the radius back and forth forever (corridors: two champions 600 rounds).
    if (close_tgt != tgt || visible || tg.state == 3) close_turns = 0;
    else if (closest <= 5) close_turns += 1;
    close_tgt = tgt;
    const int radius = visible || (stuck < 2 && close_turns < 3) ? me.attack_range() : 1;
    bitboard approach;
    for (auto t : tg.tiles) approach |= around(t, radius);
    approach &= ~tg.mask;
    if (close_turns > CLOSE_GIVEUP) {
        tg.avoid_until = now + 40;
        close_turns = 0;
        stuck = 0;
        target = -1;
        next_plan = 0;
        act = "close-giveup";
        return;
    }
    if (tg.mask.is_set(pos)) {
        step_towards(approach);
        act = "unblock";
        return;
    }
    const bool far_walk = (TEAM_PLAN || MAP_FIELDS) && gfields.size() > static_cast<std::size_t>(tgt) && closest > 8;
    if (far_walk ? step_static(tgt) : step_towards(approach)) {
        stuck = 0;
        act = "approach";
        return;
    }
    if (MAX_WAIT && tg.state == 3 && approach.is_set(pos)) {
        stuck = 0;  // in place for the respawn
        act = "wait-respawn";
        return;
    }
    stuck += 1;
    act = "approach-wait";
    // A step forward then a failed step resets `stuck`, so a champion could oscillate beside an
    // unreachable camp for hundreds of turns (corridors: Stellar and Willow 600+ turns each, a wall
    // between them and a respawning camp). Count failed steps per target without resets.
    if (caster_power) {
        if (aw_target != tgt) {
            aw_target = tgt;
            aw_count = 0;
        }
        if (++aw_count > 12) {
            tg.avoid_until = now + 100;
            aw_count = 0;
            stuck = 0;
            target = -1;
            next_plan = 0;
            return;
        }
    }
    if (stuck > 6) {
        // Give up on it for a while (unreachable, hidden, or contested).
        tg.avoid_until = now + 40;
        stuck = 0;
        // Drop it outright: with a target still set, a low-fuel turn skips the replan and the
        // champion stayed on the same unreachable camp for 600 turns (rushbait ladder game).
        target = -1;
        next_plan = 0;
    }
}
