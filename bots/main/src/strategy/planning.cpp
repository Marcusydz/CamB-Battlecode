// Camp choice: which group to farm next, alone or with helpers.
#include "../core/bot.h"
#include "../core/config.h"
#include "../core/util.h"
#include <algorithm>
#include <map>

using cambc::vec2;
using cambc::team;
using cambc::bitboard;
using cambc::distance_field;
using CT = cambc::champion_type;
using Item = cambc::item_type;

// Smallest team size that clears g without anyone dying (rough), or 0 if none.
int Bot::helpers_needed(const GroupInfo& g, double dps, double health) const {
    // Monoliths: only on gem-scarce maps (few gems outside monoliths), as a team of MONOLITH_TEAM.
    // A full-team assault on ordinary maps was tested in Python and lost; champions arriving one at a
    // time also died one by one, so the team gathers first (the joint-target logic below).
    if (g.monolith) {
        if (KITE && MONO_KITE) return kiter && cambc::self().focus() >= MONO_FOCUS ? 1 : 0;  // casters hit it from out of reach; melee would only die
        if (!scarce) return 0;
        if (KITE) return kiter ? 1 : 0;
        return std::min(team_size, MONOLITH_TEAM);
    }
    if (g.strength <= 0) return 1;
    if (KITE && KITE != 3 && kiter) return 1;  // out of camp range: nothing hits us
    for (int n = 1; n <= team_size; ++n) {
        const double clear = g.health / (dps * n);
        // Monsters die progressively, so roughly 60% of full damage lands; spread over n.
        const double factor = (SOLO_CAMP_FULL && g.count == 1) ? 1.0 : 0.6;
        if (g.strength * clear * factor <= CAMP_RISK * health * n) return n;
    }
    return 0;
}

// Rough damage per turn against camps: basic attack plus attack reset (every 8-turn cooldown).
// Never below the old flat estimate.
double Bot::camp_dps(const cambc::champion& me) const {
    if (KITE && KITE != 3 && kiter) {
        // Spells only (every 8 rounds), AoE counted on two camps.
        double d = 0;
        for (const auto& n : nukes) d += n.modifier * me.focus() * (n.aoe ? 2.0 : 1.0) / 8.0;
        return std::max(d, 20.0);
    }
    const double s = me.strength();
    const double dps = s + (reset_slot ? s / 8.0 : 0.0);
    return std::max(dps, std::max(s, 20.0) + 15.0);
}

// A higher-priority ally (lower memory slot) has claimed a different camp that also needs helpers
// for us: go there instead of starting our own joint target. Without this, two weak champions each
// wait at their own camp for the other (seen: 50 turns).
bool Bot::joint_claim_elsewhere(int g, double dps, double health) {
    if (!PAIR_UP) return false;
    const auto c = claims();
    for (int i = 0; i < mem_slot; ++i) {
        const int v = c[i];
        if (v && v - 1 != (g < 0 ? -1 : groups[g].idx) && v - 1 < static_cast<int>(groups.size())) {
            const auto& other = groups[v - 1];
            if (!other.monolith && helpers_needed(other, dps, health) > 1) return true;
        }
    }
    return false;
}

int Bot::min_dist(const distance_field& field, const GroupInfo& g) const {
    int best = UNREACHED;
    for (auto t : g.tiles) best = std::min<int>(best, field.get(t));
    return best;
}

// Team plan: the same greedy assignment on every ally (deterministic from shared positions), so
// champions split the camps between them instead of each chasing the best camp for itself
// (ladder replays: ~60% of our turns were walking, often to camps an ally already took).
// Returns our group (and sets need) or -1 when we got nothing or the fields are not ready.
int Bot::team_plan(int now) {
    // Fuel is counted per instruction (~100k a turn): fixed arrays, no allocation in the loops,
    // and solo-camp scores computed once instead of every greedy round.
    if (static_cast<int>(gfields.size()) < static_cast<int>(groups.size())) return -1;
    constexpr int MAXA = 8;
    auto& me = cambc::self();
    struct Ally { std::uint32_t h; vec2 pos; double dps, health; };
    Ally allies[MAXA];
    int A = 0;
    allies[A++] = {handle_of(me), me.position(), camp_dps(me), me.max_health()};
    for (const auto& a : cambc::vision::champions(team::ally)) {
        if (A >= MAXA) break;
        if (a == me || !a.is_alive()) continue;
        allies[A++] = {handle_of(a), a.position(), std::max(static_cast<double>(a.strength()), 20.0) + 15.0, a.max_health()};
    }
    std::sort(allies, allies + A, [](const Ally& x, const Ally& y) { return x.h < y.h; });
    int my_i = 0;
    double sum_dps = 0, min_hp = 1e9;
    for (int i = 0; i < A; ++i) {
        if (allies[i].h == handle_of(me)) my_i = i;
        sum_dps += allies[i].dps;
        min_hp = std::min(min_hp, allies[i].health);
    }
    const double my_dps = camp_dps(me), my_hp = me.max_health();
    struct Cand { int g, need; double value, fixed, health; int dist[MAXA]; std::int8_t order[MAXA]; };
    std::vector<Cand> cands;
    cands.reserve(groups.size());
    for (const auto& g : groups) {
        if (now < g.avoid_until || (g.state == 2 && now - g.seen_at > 25) || (g.monolith && g.state != 2)
            || g.available_at > now + 200)
            continue;
        // One helper count per camp from the team's average damage (per-ally counts cost too much fuel).
        // Helper count from our own damage, as choose_group does (casters solo camps from range).
        const int need = g.state == 2 ? 1
                         : std::min(helpers_needed(g, my_dps * (kind == CT::varg ? std::min(g.count, 2) : 1), my_hp), A);
        if (need <= 0) continue;
        Cand c;
        c.g = g.idx;
        c.need = need;
        c.value = g.gems + gold_weight * g.gold;
        c.fixed = CAPTURE_TIME + altar_weight * g.altar_dist + 1;
        c.health = g.state == 2 ? 0 : g.health;
        const auto& f = gfields[g.idx];
        for (int i = 0; i < A; ++i) {
            const int d = f[allies[i].pos.y * width + allies[i].pos.x];
            // MAX_WAIT 0: only camps alive when we would get there.
            c.dist[i] = (d >= UNREACHED || g.available_at > now + d) ? (1 << 20) : d;
            c.order[i] = static_cast<std::int8_t>(i);
        }
        for (int i = 1; i < A; ++i)  // insertion sort: allies nearest first
            for (int j = i; j > 0 && c.dist[c.order[j]] < c.dist[c.order[j - 1]]; --j) std::swap(c.order[j], c.order[j - 1]);
        cands.push_back(c);
    }
    const int C = static_cast<int>(cands.size());
    bool assigned[MAXA] = {};
    std::vector<char> taken(C, 0);
    if (PLAN_KEEP_CLAIMS) {
        // Allies keep the camp they already claimed: replanning them away is how the plan made
        // everyone walk more (expanse: approach 182 vs 116 turns per champion).
        const auto cl = claims();
        for (int i = 0; i < A; ++i) {
            if (i == my_i) continue;
            const int g = cl[allies[i].h % 8] - 1;
            if (g < 0) continue;
            assigned[i] = true;
            for (int ci = 0; ci < C; ++ci)
                if (cands[ci].g == g) taken[ci] = 1;
        }
    }
    // Solo camps: score depends only on the ally, so compute once.
    std::vector<double> solo(static_cast<std::size_t>(C) * A, 0.0);
    for (int ci = 0; ci < C; ++ci) {
        const auto& c = cands[ci];
        if (c.need != 1) continue;
        for (int i = 0; i < A; ++i)
            if (c.dist[i] < (1 << 20))
                solo[ci * A + i] = c.value / (c.dist[i] + c.health / allies[i].dps + c.fixed)
                                   * (i == my_i && c.g == target ? COMMIT : 1.0);  // hysteresis, as choose_group
    }
    for (int round = 0; round < A; ++round) {
        double best_score = 0;
        int best_c = -1, best_team[MAXA], best_n = 0;
        for (int ci = 0; ci < C; ++ci) {
            if (taken[ci]) continue;
            const auto& c = cands[ci];
            if (c.need == 1) {
                for (int i = 0; i < A; ++i) {
                    if (assigned[i] || solo[ci * A + i] <= best_score) continue;
                    best_score = solo[ci * A + i];
                    best_c = ci;
                    best_team[0] = i;
                    best_n = 1;
                }
                continue;
            }
            // A camp needing several: the nearest free allies go (the leader choice does not matter).
            int team[MAXA], n = 0, far = 0;
            double dps = 0;
            for (int k = 0; k < A && n < c.need; ++k) {
                const int i = c.order[k];
                if (assigned[i] || c.dist[i] >= (1 << 20)) continue;
                team[n++] = i;
                far = std::max(far, c.dist[i]);
                dps += allies[i].dps;
            }
            if (n < c.need) continue;
            const double score = c.value / ((far + c.health / dps + c.fixed) * n);
            if (score > best_score) {
                best_score = score;
                best_c = ci;
                best_n = n;
                for (int k = 0; k < n; ++k) best_team[k] = team[k];
            }
        }
        if (best_c < 0) break;
        taken[best_c] = 1;
        for (int k = 0; k < best_n; ++k) {
            assigned[best_team[k]] = true;
            if (best_team[k] == my_i) {
                need = cands[best_c].need;
                return cands[best_c].g;
            }
        }
    }
    return -1;
}

void Bot::compute_group_fields() {
    int done = 0;
    while (static_cast<int>(gfields.size()) < static_cast<int>(groups.size()) && done < FIELDS_PER_TURN) {
        const auto& g = groups[gfields.size()];
        distance_field f;
        bitboard visited;
        f.compute(static_block & ~g.mask, g.mask, visited);
        gfields.push_back(f.export_linear_buffer());  // plain array: a host get() costs ~60 fuel
        if (PLAN_CHECK && gfields.size() == 1) {
            int bad = 0;
            for (int y = 0; y < height; y += 3)
                for (int x = 0; x < width; x += 3)
                    if (f.get({x, y}) != gdist(0, {x, y})) bad += 1;
            std::printf("fieldcheck bad=%d size=%zu w=%d h=%d\n", bad, gfields[0].size(), width, height);
        }
        done += 1;
    }
}

int Bot::choose_group(int now, const distance_field& my_field) {
    auto& me = cambc::self();
    const auto c = claims();
    std::map<int, int> claim_count;
    std::map<int, bool> senior;  // claims by higher-priority (lower slot) allies
    for (int i = 0; i < 8; ++i) {
        if (i != mem_slot && c[i]) {
            claim_count[c[i] - 1] += 1;
            if (i < mem_slot) senior[c[i] - 1] = true;
        }
    }
    if (MID_SQUAD && kiter && kind != CT::issac && kiter_rank < MID_SQUAD && me.focus() >= MONO_FOCUS) {
        const GroupInfo* mid = nullptr;
        for (const auto& g : groups)
            if (g.monolith && g.gems >= 200 && (!mid || g.gems > mid->gems)) mid = &g;
        if (mid && mid->state != 3 && now >= mid->avoid_until && mid->available_at <= now + min_dist(my_field, *mid)
            && min_dist(my_field, *mid) < UNREACHED && !enemy_near(mid->center, MONOLITH_CLEAR, 2, now)) {
            need = 1;
            return mid->idx;
        }
    }
    if (squad > 1) {
        // Squads of SQUAD: champions ranked by memory slot (alive ones only); each squad's first
        // member picks camps, the rest go to its camp (Nyx and Fish kill our lone champions in a round).
        const auto alive = squad_alive(now);
        std::vector<int> slots;
        for (int i = 0; i < 8; ++i)
            if (alive[i]) slots.push_back(i);
        const auto it = std::find(slots.begin(), slots.end(), mem_slot);
        if (it != slots.end()) {
            const int rank = static_cast<int>(it - slots.begin());
            const int lead = slots[rank - rank % squad];
            if (lead != mem_slot && c[lead] && c[lead] - 1 < static_cast<int>(groups.size())) {
                const auto& g = groups[c[lead] - 1];
                const int dist = min_dist(my_field, g);
                if (!(g.state == 3 || now < g.avoid_until || dist >= UNREACHED || dist > PACK_RANGE
                      || g.available_at > now + dist || (g.monolith && !kiter))) {
                    need = 1;
                    return g.idx;  // follow the squad leader's camp
                }
            }
        }
    }
    if (PACK && mem_slot > 0) {
        for (int i = 0; i < mem_slot; ++i) {
            const int v = c[i];
            if (!v || v - 1 >= static_cast<int>(groups.size())) continue;
            const auto& g = groups[v - 1];
            const int dist = min_dist(my_field, g);
            if (g.state == 2 || g.state == 3 || now < g.avoid_until || dist >= UNREACHED || dist > PACK_RANGE
                || g.available_at > now + dist || (g.monolith && !kiter))
                break;
            need = 1;
            return g.idx;  // follow the leader's camp
        }
    }
    const double dps = camp_dps(me);
    const double health = me.max_health();
    const bool no_new_joint = now < no_joint_until || joint_claim_elsewhere(-1, dps, health);
    int best = -1, best_need = 1;
    double best_score = 0.0;
    for (const auto& g : groups) {
        const int dist = min_dist(my_field, g);
        if (dist >= UNREACHED) continue;
        const int eta = dist;
        if (now < g.avoid_until) continue;
        const int wait = std::max(0, g.available_at - now - eta);
        if (wait > MAX_WAIT) continue;
        const int others = claim_count.count(g.idx) ? claim_count[g.idx] : 0;
        if (g.state == 2 && dist > 3 && (others > 0 || now - g.seen_at > 25))
            continue;  // an ally is taking that reward, or we saw it so long ago it is likely gone
        int need;
        double clear;
        if (g.state == 2) {
            need = 1;
            clear = 0;
        } else if (g.monolith) {
            need = helpers_needed(g, dps, health);
            if (need == 0) continue;
            if (enemy_near(g.center, MONOLITH_CLEAR, MONOLITH_MEMORY, now)) continue;  // they would take the reward
            clear = g.health / (dps * need);
        } else {
            need = helpers_needed(g, dps * (kind == CT::varg ? std::min(g.count, 2) : 1), health);
            if (need == 0) continue;
            if (need > 1 && others == 0 && no_new_joint)
                continue;  // join the ally already waiting for help, or recently gave up waiting
            clear = g.health / (dps * std::max(need, others + 1));
        }
        const double gold_w = gold_weight;
        const double value = (gem_weight * g.gems + gold_w * g.gold) / std::max(need, others + 1);
        double cost = eta_w * eta + wait + clear + CAPTURE_TIME + altar_weight * g.altar_dist + 1;
        double score_bonus = 1.0;
        if (need > 1) {
            cost += 4 * (need - 1 - std::min(others, need - 1));  // time to gather helpers
            if (others > 0) score_bonus = JOIN_BONUS;
        }
        double score = value / cost * score_bonus;
        if (others && need <= 1 && g.state != 2 && !(MONO_KITE && g.monolith) && (!SPREAD_LOWER_ONLY || senior.count(g.idx)))
            score *= SPREAD;  // someone else is on it
        if (MONO_KITE && g.monolith && others > 0) score *= JOIN_BONUS;  // a second caster halves the long monolith fight
        if (CONTEST_FACTOR < 1.0 && g.contested && (now < CONTEST_EARLY || (contest_mid && enemy_near(g.center, 8, 10, now))))
            score *= CONTEST_FACTOR;  // mid-game penalty only where one herd dominates (elk): elsewhere we ceded the middle
        if (target == g.idx) score *= COMMIT;  // hysteresis
        if (SEED) {
            const std::uint64_t h = (static_cast<std::uint64_t>(SEED) * 1000003 + g.idx * 7919 + mem_slot * 104729
                                     + now / 40) * 2654435761ULL % 4294967296ULL;
            score *= 0.9 + 0.2 * (h / 4294967296.0);
        }
        if (score > best_score) {
            best = g.idx;
            best_score = score;
            best_need = need;
        }
    }
    need = best_need;
    return best;
}
