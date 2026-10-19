// Champion logic: farm the best-value camp, capture it, bank gems, shop, and stay alive.
//
// A line-for-line port of bots/my-bot/src/bot.py (v16). Each champion runs its own copy.
// Champions coordinate through shared memory: each writes the camp group it is heading for,
// and the others avoid that group. Switches that were tested and rejected in Python (and were
// off in v16) are not ported; see TODO.md and bots/my-bot/README.md for their results.
#include "bot.h"
#include <algorithm>
#include <cstdio>
#include <tuple>

using cambc::vec2;
using cambc::team;
using cambc::bitboard;
using cambc::distance_field;
using CT = cambc::champion_type;
using Item = cambc::item_type;

// Base camp stats per camp type: (health, strength).
static std::pair<double, double> camp_stats(cambc::camp_type t) {
    switch (t) {
    case cambc::camp_type::wolf: return {1350, 20};
    case cambc::camp_type::elk: return {2400, 60};
    case cambc::camp_type::gem_cluster: return {500, 0};
    case cambc::camp_type::gold_cluster: return {500, 0};
    case cambc::camp_type::monolith: return {20000, 200};
    }
    return {1500, 50};
}

// Tuning parameters (same names and values as my-bot v16).
constexpr int CAPTURE_TIME = 5;
constexpr int UNREACHED = 1000000;
constexpr int SHOP_RECALL_GOLD = 150;   // recall to shop with an empty bag and this much gold
constexpr bool BUY_JUNGLE = true;
constexpr int GEM_CAP = 60;
constexpr int REPLAN_EVERY = 3;
constexpr int PLAN_RADIUS = 50;         // max path length considered when choosing camps
constexpr int LOG_EVERY = 25;           // status line per champion every N rounds (0 disables)
constexpr double ENGAGE_RATIO = 0.8;    // fight when our side's power is at least this fraction of theirs
constexpr double FLEE_RATIO = 0.3;      // carriers run when our power is below this fraction of theirs
constexpr int HUNT_RADIUS = 10;         // how far we chase a winnable fight
constexpr bool USE_LOCKS = true;        // buy altar locks and place them on altars we pass
constexpr bool INTERRUPT = true;        // stun / lock enemies who are tributing
constexpr int CONTEST_MARGIN = 4;       // tiles: a camp is "ours" if this much closer to our spawn
constexpr int THREAT_RANGE = 5;         // enemies this close stop a recall (attack range 4 + a step)
constexpr bool SOLO_CAPTURE = true;     // only one champion holds a capture; others move on
// Fuel thresholds are fractions of the turn budget. C++ gets 100k fuel per turn (Python 2.5M), so
// my-bot's absolute values (700k reserve, low fuel below 1.6M) are scaled: 0.28 and 0.64 of a turn.
constexpr double FUEL_RESERVE = 0.28;   // skip optional work when less than this fraction of the turn remains
constexpr double LOW_FUEL = 0.64;       // below this fraction, keep the current target instead of replanning
constexpr std::uint64_t TURN_FUEL = 100000;
constexpr double LOW_HP = 0.3;          // retreat below this health fraction (+LOW_HP_GEMS when carrying 30+ gems)
constexpr double LOW_HP_GEMS = 0.15;
constexpr double SPAWN_HEAL_TO = 0.95;  // stay on spawn until this healthy
constexpr int GATHER_MAX = 12;          // give up waiting for helpers after this many turns
constexpr double JOIN_BONUS = 1.5;      // score boost for joining a joint target an ally already claimed
constexpr double HEAL_MIN_USE = 0.6;    // heal only if at least this fraction of the heal lands (or the ally is below half HP)
constexpr int PRISM_GEM_CAP = 60;       // Prism gains only +0.2 focus per gem carried, so it banks like the others
constexpr int PRISM_EARLY_BANK = 0;
constexpr bool SOLO_CAMP_FULL = true;   // single-camp groups deal full damage (no 60% "dies progressively" discount)
constexpr double COMMIT = 1.5;          // keep the current target unless another scores this much more
constexpr bool SPREAD_LOWER_ONLY = true;  // the "someone else is on it" penalty only counts higher-priority allies
constexpr bool DISENGAGE = true;        // losing side and an enemy within 4: back off (unless we can kill something now)
constexpr bool PAIR_UP = true;          // weak champions join an ally's joint target instead of starting their own
constexpr double ALTAR_WEIGHT = 0.5;    // camp cost counts this fraction of the camp's distance to an altar
constexpr int SCARCE_GEMS = 0;          // >0: map is gem-scarce if non-monolith groups hold fewer gems; then teams take monoliths. Tested 300: reaches 1000 gems vs a passive enemy, but 0 of 8 vs v16 (we feed kills while attacking)
constexpr int MONOLITH_TEAM = 3;        // champions that take a monolith together on gem-scarce maps
constexpr int MONOLITH_CLEAR = 12;      // no enemy may have been seen this close to a monolith ...
constexpr int MONOLITH_MEMORY = 20;     // ... in this many rounds before (and while) we attack it
constexpr int SEED = 0;                 // >0 adds small deterministic noise to camp choice (testing only)
constexpr bool IGNORE_SPAWN_CAMPERS = true;  // enemies inside their own spawn are not chased or fled from
constexpr int ACT_LOG_EVERY = 200;      // print how many turns each champion spent on each activity (0 disables)

static int chebyshev(vec2 a, vec2 b) {
    return std::max(std::abs(a.x - b.x), std::abs(a.y - b.y));
}

static bitboard around(vec2 position, int radius) {
    bitboard board;
    board.draw_chebyshev(position, static_cast<float>(radius));
    return board;
}

static std::uint32_t handle_of(const cambc::champion& c) {
    return static_cast<std::uint32_t>(static_cast<cambc::handle_champion_t>(c));
}

static std::uint32_t handle_of(const cambc::camp& c) {
    return static_cast<std::uint32_t>(static_cast<cambc::handle_camp_t>(c));
}

// vision::camps(), tolerating the SDK's occasional stale-tile error.
static std::vector<cambc::camp> visible_camps() {
    try {
        return cambc::vision::camps();
    } catch (const cambc::error&) {
        std::vector<cambc::camp> out;
        for (auto t : cambc::vision::camp_tiles().positions()) {
            try {
                out.push_back(cambc::vision::camp_at(t));
            } catch (const cambc::error&) {
            }
        }
        return out;
    }
}

static std::vector<cambc::champion> visible_enemies() {
    try {
        return cambc::vision::champions(team::enemy);
    } catch (const cambc::error&) {
        return {};
    }
}

static double camp_strength(const cambc::camp& c) {
    try {
        return c.strength();
    } catch (const cambc::error&) {
        return 0;
    }
}

static double camp_health(const cambc::camp& c) {
    try {
        return c.health();
    } catch (const cambc::error&) {
        return 0;
    }
}

static const char* kind_name(CT kind) {
    static std::string names[12];
    auto& s = names[static_cast<int>(kind)];
    if (s.empty()) s = cambc::to_string(kind);
    return s.c_str();
}

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
        }
        auto it = std::find(picks.begin(), picks.end(), draft_id);
        if (it == picks.end()) throw std::runtime_error("not in draft");
        mem_slot = kind == CT::issac ? 7 : static_cast<int>(it - picks.begin());
        const bool has_pair = std::find(picks.begin(), picks.end(), 3) != picks.end();
        team_size = static_cast<int>(picks.size()) + (has_pair ? 1 : 0);
    } catch (const std::exception&) {
        team_size = 3;
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

// ------------------------------------------------------------------ shared memory
std::array<std::uint8_t, 8> Bot::claims() const {
    std::array<std::uint8_t, 8> data{};
    cambc::shared_memory.read(0, std::span{data});
    return data;
}

void Bot::claim(int group) {
    const std::uint8_t value = group < 0 ? 0 : static_cast<std::uint8_t>(groups[group].idx + 1);
    auto data = claims();
    if (data[mem_slot] != value) cambc::shared_memory.write(mem_slot, std::span{&value, 1});
}

// ------------------------------------------------------------------ knowledge
void Bot::update_groups(int now, vec2 pos) {
    const auto visible = cambc::vision::visible_tiles();
    for (auto& g : groups) {
        const auto c = g.center;
        if (std::abs(c.x - pos.x) > 11 || std::abs(c.y - pos.y) > 11) continue;
        if ((g.mask & visible).is_empty()) continue;
        cambc::map::camp_group_state state;
        try {
            state = g.group.state();
        } catch (const cambc::error&) {
            continue;
        }
        g.seen_at = now;
        if (std::holds_alternative<cambc::map::camp_group_alive>(state)) {
            g.state = 1;
            g.available_at = 0;
        } else if (std::holds_alternative<cambc::map::camp_group_reward_available>(state)) {
            g.state = 2;
            g.available_at = 0;
        } else {
            g.state = 3;
            g.available_at = now + std::get<cambc::map::camp_group_respawning>(state).timer;
        }
    }
}

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
    if (me.attack_tiles().is_set(target.position())) {
        if (camp_strength(target) > 0 && me.health() < 0.9 * me.max_health())
            self_buff(me);  // Varg's shield used to be PvP-only
        try_spell(target);
        try_attack(target, others);
        return;
    }
    const bool moved = step_towards(around(target.position(), me.attack_range()), false);
    if (try_attack(target, others)) return;
    for (const auto& c : others)
        if (try_attack(c, others)) return;
    if (!moved) try_spell(target);
}

// ------------------------------------------------------------------ shopping
void Bot::shop() {
    auto& me = cambc::self();
    const auto items = me.item_count();
    // Build order: Monster emblem (+10% camp damage), Jungle emblem of health
    // (-30% camp damage taken), then Power strength shards forever.
    Item primary = Item::power_shard_of_strength;
    if (items[Item::monster_emblem_of_strength] == 0)
        primary = Item::monster_shard_of_strength;
    else if (items[Item::jungle_emblem_of_health] == 0 && BUY_JUNGLE)
        primary = Item::jungle_shard_of_health;
    if (USE_LOCKS && (!me.is_alive() || spawn.is_set(me.position()))) {
        const auto lock = cambc::consumable_type::altar_lock;
        try {
            if (me.gold_count() >= 10 && me.can_buy(lock)) me.buy(lock);
        } catch (const cambc::error&) {
        }
    }
    const auto cost = primary.cost();
    const auto count = me.gold_count() / cost;
    if (count > 0) {
        try {
            if (me.can_buy(primary, count)) me.buy(primary, count);
        } catch (const cambc::error&) {
        }
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
        for (const auto& a : cambc::vision::champions(team::ally))
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

static double power(const std::vector<cambc::champion>& units) {
    double hp = 0, dps = 0;
    for (const auto& u : units) {
        hp += std::max<double>(u.health(), 1.0);
        dps += u.strength() + 0.25 * u.focus();
    }
    return hp * dps;
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
    for (const auto& a : cambc::vision::champions(team::ally))
        if (a.is_alive() && a != me && chebyshev(a.position(), pos) <= 5) allies.push_back(a);
    const double enemy_power = power(near);
    const double ratio = power(allies) / std::max(enemy_power, 1.0);
    // 2. Winning fight: focus fire (same rule on every ally), finish kills first.
    if (ratio >= ENGAGE_RATIO && hp_frac > 0.35) {
        const double b = burst(me);
        std::vector<cambc::champion> reach;
        for (const auto& e : near)
            if (chebyshev(e.position(), pos) <= HUNT_RADIUS) reach.push_back(e);
        if (!reach.empty()) {
            auto key = [&](const cambc::champion& e) {
                const bool killable = e.health() <= b && chebyshev(e.position(), pos) <= 4;
                return std::make_tuple(!killable, e.health() / (1.0 + 0.03 * e.gem_count()), handle_of(e));
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
        retreat_and_recall();
        act = "disengage";
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
        std::stable_sort(near.begin(), near.end(),
                         [](const cambc::champion& a, const cambc::champion& b) { return a.health() < b.health(); });
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
bool Bot::retreat_and_recall() {
    auto& me = cambc::self();
    auto pos = me.position();
    const int now = static_cast<int>(cambc::round_number());
    std::vector<vec2> danger;
    for (auto t : threats(now))
        if (chebyshev(pos, t) <= THREAT_RANGE) danger.push_back(t);
    for (const auto& c : visible_camps())
        if (c.is_visible() && camp_strength(c) > 0 && camp_health(c) < c.max_health()
            && chebyshev(pos, c.position()) <= 3)
            danger.push_back(c.position());
    if (danger.empty()) {
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
        return std::make_pair(std::min(d, 6), -static_cast<int>(home_field.get(p)));
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
        auto best = pick((around(pos, 1) & ~blk).positions());
        if (best && *best != pos) {
            try {
                me.move_absolute(*best);
            } catch (const cambc::error&) {
            }
        }
    }
    return true;
}

// Smallest team size that clears g without anyone dying (rough), or 0 if none.
int Bot::helpers_needed(const GroupInfo& g, double dps, double health) const {
    // Monoliths: only on gem-scarce maps (few gems outside monoliths), as a team of MONOLITH_TEAM.
    // A full-team assault on ordinary maps was tested in Python and lost; champions arriving one at a
    // time also died one by one, so the team gathers first (the joint-target logic below).
    if (g.monolith) return scarce ? std::min(team_size, MONOLITH_TEAM) : 0;
    if (g.strength <= 0) return 1;
    for (int n = 1; n <= team_size; ++n) {
        const double clear = g.health / (dps * n);
        // Monsters die progressively, so roughly 60% of full damage lands; spread over n.
        const double factor = (SOLO_CAMP_FULL && g.count == 1) ? 1.0 : 0.6;
        if (g.strength * clear * factor <= 0.75 * health * n) return n;
    }
    return 0;
}

// Rough damage per turn against camps: basic attack plus attack reset (every 8-turn cooldown).
// Never below the old flat estimate.
double Bot::camp_dps(const cambc::champion& me) const {
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
        if (wait > 0) continue;
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
        const double gold_w = 0.15;
        const double value = (g.gems + gold_w * g.gold) / std::max(need, others + 1);
        double cost = eta + wait + clear + CAPTURE_TIME + ALTAR_WEIGHT * g.altar_dist + 1;
        double score_bonus = 1.0;
        if (need > 1) {
            cost += 4 * (need - 1 - std::min(others, need - 1));  // time to gather helpers
            if (others > 0) score_bonus = JOIN_BONUS;
        }
        double score = value / cost * score_bonus;
        if (others && need <= 1 && g.state != 2 && (!SPREAD_LOWER_ONLY || senior.count(g.idx)))
            score *= 0.25;  // someone else is on it; spread out
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

// ------------------------------------------------------------------ turn
void Bot::play_turn() {
    act = "?";
    const auto start = cambc::fuel_remaining();
    const auto r0 = cambc::round_number();
    turn();
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
        std::printf("r%u %s %s hp%d/%d gem%u gold%u tgt=%d st=%d act=%s fuelmax=%llu\n", now, kind_name(kind),
                    where.c_str(), static_cast<int>(me.health()), static_cast<int>(me.max_health()), me.gem_count(),
                    me.gold_count(), target, target < 0 ? -1 : groups[target].state, act.c_str(),
                    static_cast<unsigned long long>(max_fuel));
    }
}

void Bot::turn() {
    auto& me = cambc::self();
    if (!ready) setup();
    const int now = static_cast<int>(cambc::round_number());
    const bool was_waiting_on_spawn = waited_on_spawn;
    waited_on_spawn = false;

    if (!me.is_alive()) {
        last_alive = false;
        claim(-1);
        shop();
        act = "dead";
        return;
    }
    if (me.is_recalling() || me.is_submitting_gems() || me.is_teleporting()) {
        act = "channel";
        return;
    }

    const auto pos = me.position();
    const bool at_spawn = spawn.is_set(pos);
    if (at_spawn || kind == CT::tavi) shop();
    update_groups(now, pos);

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

    const double prev_hp = last_hp;
    last_hp = me.health();
    last_alive = true;
    // Survival.
    if (!at_spawn && hp_frac < (LOW_HP + (gems >= 30 ? LOW_HP_GEMS : 0.0))) {
        if (me.health() < prev_hp) {
            // Being hit right now: running rarely escapes a chaser, so first take
            // whatever is in range (a kill resets the chaser problem entirely).
            bool any = false;
            const auto tiles = me.attack_tiles();
            for (auto& e : visible_enemies())
                if (e.is_alive() && tiles.is_set(e.position())) any = true;
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
    if (gems > 0 && (cambc::global_submitted_gems(team::ally) + gems >= cambc::constants::winning_gems || now > 1940)) {
        tribute();
        return;
    }

    if (pvp(me, pos, gems, hp_frac)) return;

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
                           && target_dist <= 7 && me.gold_count() < 2 * SHOP_RECALL_GOLD;
    bool threatened = false;
    for (auto t : threats(now))
        if (chebyshev(pos, t) <= THREAT_RANGE + 2) threatened = true;
    if (gems == 0 && me.gold_count() >= SHOP_RECALL_GOLD && kind != CT::tavi && !at_spawn && now < 1800
        && !busy_here && !threatened) {
        claim(-1);
        retreat_and_recall();
        act = "shop-recall";
        return;
    }

    int tgt = target;
    const bool valid = tgt >= 0 && now >= groups[tgt].avoid_until && groups[tgt].available_at <= now + 2
                       && (groups[tgt].state != 3 || chebyshev(pos, groups[tgt].center) > 9);
    const bool engaged = valid && (groups[tgt].state == 1 || groups[tgt].state == 2)
                         && chebyshev(pos, groups[tgt].center) <= 6;
    const bool low_fuel = cambc::fuel_remaining() < LOW_FUEL * TURN_FUEL;
    if ((!valid && !(low_fuel && tgt >= 0)) || (now >= next_plan && !engaged && !low_fuel)) {
        distance_field my_field;
        bitboard roots, visited;
        roots.set(pos);
        my_field.compute(static_block, roots, visited, PLAN_RADIUS);
        tgt = choose_group(now, my_field);
        if (tgt < 0 && !low_fuel) {
            // Nothing within PLAN_RADIUS: search the whole map rather than idle.
            bitboard visited2;
            my_field.compute(static_block, roots, visited2);
            tgt = choose_group(now, my_field);
        }
        target = tgt;
        next_plan = now + REPLAN_EVERY;
        target_dist = tgt < 0 ? UNREACHED : min_dist(my_field, groups[tgt]);
    } else {
        target_dist = std::max(0, target_dist - 1);
    }

    if (gems > 0) {
        const int altar_dist = altar_field.get(pos);
        const bool prism = kind == CT::prism;
        const int cap = prism ? PRISM_GEM_CAP : GEM_CAP;
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
                for (const auto& c : camps) ring |= around(c.position(), 5);
                for (const auto& c : camps) ring &= ~around(c.position(), 3);
                claim(tgt);
                act = "gather";
                step_towards(ring);
                return;
            }
        }
        gather_turns = 0;
        // Hit the most dangerous camp first, then the weakest.
        std::stable_sort(camps.begin(), camps.end(), [](const cambc::camp& a, const cambc::camp& b) {
            const bool sa = camp_strength(a) > 0, sb = camp_strength(b) > 0;
            if (sa != sb) return sa;
            return camp_health(a) < camp_health(b);
        });
        std::vector<cambc::camp> rest(camps.begin() + 1, camps.end());
        fight(camps[0], rest);
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
    const int radius = (visible || stuck < 2) ? me.attack_range() : 1;
    bitboard approach;
    for (auto t : tg.tiles) approach |= around(t, radius);
    approach &= ~tg.mask;
    if (tg.mask.is_set(pos)) {
        step_towards(approach);
        act = "unblock";
        return;
    }
    if (step_towards(approach)) {
        stuck = 0;
        act = "approach";
        return;
    }
    stuck += 1;
    act = "approach-wait";
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
