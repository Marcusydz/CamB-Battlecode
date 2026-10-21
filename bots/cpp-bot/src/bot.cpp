// Champion logic: farm the best-value camp, capture it, bank gems, shop, and stay alive.
//
// A line-for-line port of bots/my-bot/src/bot.py (v16). Each champion runs its own copy.
// Champions coordinate through shared memory: each writes the camp group it is heading for,
// and the others avoid that group. Switches that were tested and rejected in Python (and were
// off in v16) are not ported; see TODO.md and bots/my-bot/README.md for their results.
#include "bot.h"
#include "draft.h"
#include "camp_waiting_area.h"
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
// Attack range per camp kind (week-2 patch: elk 3 -> 4, monolith 3 -> 5, golem 4).
static int camp_range(cambc::camp_type t) {
    if (t == cambc::camp_type::elk) return 4;
    if (t == cambc::camp_type::monolith) return 5;
    if (static_cast<int>(t) == 5) return 4;  // golem
    return 3;
}

static std::pair<double, double> camp_stats(cambc::camp_type t) {
    switch (t) {
    case cambc::camp_type::wolf: return {1350, 20};
    case cambc::camp_type::elk: return {2400, 60};
    case cambc::camp_type::gem_cluster: return {120, 0};   // week-2 patch: 500 -> 120
    case cambc::camp_type::gold_cluster: return {120, 0};
    case cambc::camp_type::monolith: return {20000, 200};
    }
    if (static_cast<int>(t) == 5) return {5000, 120};  // golem (week-2 patch; not in our SDK's enum yet)
    return {1500, 50};
}

// Tuning parameters (same names and values as my-bot v16).
constexpr int CAPTURE_TIME = 5;
// Unreached uint16 distances are the SDK sentinel, not a large valid path cost (Codex review of v44).
constexpr int UNREACHED = cambc::distance_field::unreachable;
constexpr int SHOP_RECALL_GOLD = 150;   // recall to shop with an empty bag and this much gold (default of shop_gold)
constexpr bool BUY_JUNGLE = true;
constexpr double SUPPORT_BONUS = 0.0;   // enemy power counts this much extra per enemy healer (Willow) or shielder (Varg) in the fight
constexpr int OUTNUMBER = 2;            // >0: leave when at least this many more enemies were seen near us (last 4 rounds, 8 tiles) than allies within 6
constexpr int JOIN = 10;                 // >0: walk to an ally up to this many tiles away who is fighting a fight we would win together
constexpr int WARDS = 1;                // 1: keep a ward near where we last were, placed towards the enemy side, re-placed after moving WARD_GAP tiles
constexpr int WARD_GAP = 8;
constexpr int BUILD = 4;                // 0: v16 (monster, jungle, power strength). 1: power main stat, then durability health, then power. 2: power main stat only. 3: monster emblem of main stat, then power main stat. 4: Intercontinental Fish's emblem order per champion
constexpr int GEM_CAP = 60;
constexpr int CLOSE_GIVEUP = 12;
constexpr int CAMP_DEATH_AVOID = 150; // after a camp kills us (no enemy nearby), skip that group this many rounds
constexpr int REACH_FALLBACK = 1;     // fight: after this many turns unable to reach the target, walk to the nearest reachable harmless camp (0 off)
constexpr int TEAM_PLAN = 0;          // 1: every champion runs the same greedy camp assignment for the whole team and takes its own share
constexpr int PLAN_KEEP_CLAIMS = 1;   // allies with a claimed camp are fixed in the team plan
constexpr int FIELDS_PER_TURN = 1;
constexpr int PLAN_CHECK = 0;    // group distance fields computed per turn until all exist
constexpr int HIDDEN_ENEMIES = 1;  // count recently seen, now hidden enemies in fight odds
constexpr double GOLD_W = 0.1;   // camp value per gold, in gems (week-2 patch: 0 beat 1.0 unopposed; 0.1 with ETA_W 3 is ~8% faster)
constexpr double ETA_W = 3.0;    // walking time counts this many times in a camp's cost (Nyx circles nearby small camps as they respawn)
constexpr double GOLD_RUSH_GOLD_W = 0.0;  // 0.5 was worse vs Fish (644 vs 828 gems)
constexpr int MID_AOE_ONLY = 0;  // mid squad: only Stellar/Prism (1) or any caster (0)
constexpr int DEATH_AVOID = 120;  // rounds to avoid camps near where enemies killed us  // turns next to a target group without fighting it before dropping it
constexpr int MONO_KITE = 0;            // 1: casters take monoliths on every map (ladder #1 takes rows' 448-gem middle with Stellar and Prism)
constexpr int MID_SQUAD = 0;            // >0: the first N casters in our draft farm the biggest monolith once strong enough (ladder #1: Stellar + Prism on rows)
constexpr int KITER_BUILD = 1;          // 1: casters (never hit by camps) buy a monster emblem of focus, then power focus only, like ladder #1's Stellar and Prism
constexpr double MONO_FOCUS = 200;      // casters take monoliths only with at least this much focus
constexpr int KITE = 0;                 // 0 since the week-2 patch (elk range 4, monolith 5: nothing is out of reach from 4). casters fight attacking camps from spell range (4) only, outside camp range (3): ladder #1's Stellar takes ~0 camp damage. 2: only without a melee ally next to the camps. 3: only monoliths
constexpr int REPLAN_EVERY = 3;
constexpr int MAX_WAIT = 0;             // >0: also pick a camp that respawns up to this many rounds after we arrive, and wait beside it
constexpr int PLAN_RADIUS = 50;         // max path length considered when choosing camps
constexpr int LOG_EVERY = 0;           // status line per champion every N rounds (0 disables)
constexpr double ENGAGE_RATIO = 1.0;    // fight when our side's power is at least this fraction of theirs (0.8 on 40x40 maps)
constexpr double FLEE_RATIO = 0.3;      // carriers run when our power is below this fraction of theirs
constexpr int HUNT_RADIUS = 10;         // how far we chase a winnable fight
constexpr bool USE_LOCKS = true;        // buy altar locks and place them on altars we pass
constexpr bool INTERRUPT = true;        // stun / lock enemies who are tributing
constexpr int CONTEST_MARGIN = 4;
constexpr double CONTEST_FACTOR = 0.3;   // <1: score factor for camps nearer the enemy spawn while early or while enemies were seen near them (collision: we fed the middle)
constexpr int CONTEST_EARLY = 300;        // rounds counted as early       // tiles: a camp is "ours" if this much closer to our spawn
constexpr int THREAT_RANGE = 5;         // enemies this close stop a recall (attack range 4 + a step)
constexpr bool SOLO_CAPTURE = true;     // only one champion holds a capture; others move on
// Fuel thresholds are fractions of the turn budget. C++ gets 100k fuel per turn (Python 2.5M), so
// my-bot's absolute values (700k reserve, low fuel below 1.6M) are scaled: 0.28 and 0.64 of a turn.
constexpr double FUEL_RESERVE = 0.28;   // skip optional work when less than this fraction of the turn remains
constexpr double LOW_FUEL = 0.64;       // below this fraction, keep the current target instead of replanning
constexpr std::uint64_t TURN_FUEL = 100000;
constexpr double LOW_HP = 0.4;          // retreat below this health fraction (+LOW_HP_GEMS when carrying 30+ gems)
constexpr double LOW_HP_GEMS = 0.15;
constexpr double SPAWN_HEAL_TO = 0.95;  // stay on spawn until this healthy
constexpr double CAMP_RISK = 1.5;      // a team may take a camp group whose damage over the clear is up to this many times its total health
constexpr int GATHER_MAX = 12;          // give up waiting for helpers after this many turns
constexpr double JOIN_BONUS = 1.5;      // score boost for joining a joint target an ally already claimed
constexpr double HEAL_MIN_USE = 0.6;    // heal only if at least this fraction of the heal lands (or the ally is below half HP)
constexpr int PRISM_GEM_CAP = 60;       // Prism gains only +0.2 focus per gem carried, so it banks like the others
constexpr int PRISM_EARLY_BANK = 0;
constexpr bool SOLO_CAMP_FULL = true;   // single-camp groups deal full damage (no 60% "dies progressively" discount)
constexpr double SPREAD = 0.25;          // score factor for a camp an ally already took (<1 spreads out; >1 packs up like ladder #1)
constexpr int PACK = 0;
constexpr int SQUAD = 3;               // >1: champions move in squads of this size, following the first member's camp
constexpr int PACK_RANGE = 30;          // follow only if the leader's camp is this close (path length)                 // 1: everyone farms the camp the highest-priority ally claimed (ladder #1 moves as a pack of 3-5)
constexpr double COMMIT = 1.5;          // keep the current target unless another scores this much more
constexpr bool SPREAD_LOWER_ONLY = true;  // the "someone else is on it" penalty only counts higher-priority allies
constexpr bool DISENGAGE = true;        // losing side and an enemy within 4: back off (unless we can kill something now)
constexpr bool PAIR_UP = true;          // weak champions join an ally's joint target instead of starting their own
constexpr double ALTAR_WEIGHT = 0.5;    // camp cost counts this fraction of the camp's distance to an altar
constexpr int SCARCE_GEMS = 1;          // >0: map is gem-scarce if non-monolith groups hold fewer gems; then teams take monoliths. Tested 300: reaches 1000 gems vs a passive enemy, but 0 of 8 vs v16 (we feed kills while attacking)
constexpr int MONOLITH_TEAM = 3;        // champions that take a monolith together on gem-scarce maps
constexpr int MONOLITH_CLEAR = 12;      // no enemy may have been seen this close to a monolith ...
constexpr int MONOLITH_MEMORY = 20;     // ... in this many rounds before (and while) we attack it
constexpr int SEED = 0;                 // >0 adds small deterministic noise to camp choice (testing only)
constexpr bool IGNORE_SPAWN_CAMPERS = true;  // enemies inside their own spawn are not chased or fled from
constexpr int ACT_LOG_EVERY = 0;      // print how many turns each champion spent on each activity (0 disables)
// Casters deal damage from focus. Counting focus at 0.25 made Nyx (Ren, Stellar, Cassi & Issac stacking
// focus) look weak: we engaged and lost 1-12 in kills on lanes. Off on 40x40 maps (broken_curve stays as v42).
static bool caster_power = true;
static double engage_ratio = ENGAGE_RATIO;
static bool treasure_first = false;   // treasure emblem opens the build (per map, see setup)
static bool death_avoid = false;      // avoid camps where enemy champions killed us (per map, see setup)
static bool old_contest_map = false;  // the pre-v90 contested-camp rule won here (per map, see setup)
static bool contest_mid = false;      // mid-game contested-camp penalty: single-herd maps and old_contest_map
static bool soft_back = true;            // see SOFT_BACK_HP; off on 40x40 maps
constexpr double SOFT_BACK_HP = 0.6;     // disengage without recalling when at least this healthy
constexpr int SOFT_BACK_GEMS = 20;       // ... and carrying fewer gems than this (was 10)


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
    static std::string names[32];
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
    caster_power = !is_broken_curve();  // was every 40x40 map, which froze shortcut and interlock too
    soft_back = caster_power;
    {
        const int walls = static_cast<int>(cambc::map::walls().count());
        treasure_first = (width == 50 && height == 50 && walls == 214)     // rows
                         || (width == 90 && height == 30 && walls == 0)    // lanes
                         || (width == 71 && height == 71 && walls == 44)   // corners
                         || (width == 41 && height == 41 && walls == 84)   // collision (148 vs 22)
                         || (width == 41 && height == 30 && walls == 36)   // eight (219 vs 104)
                         || (width == 45 && height == 35 && walls == 116)  // split (746 vs 386)
                         || (width == 50 && height == 65 && walls == 0)    // pilgrimage (490 vs 371)
                         || (width == 50 && height == 50 && walls == 98)   // monolith (455 vs 368)
                         || (width == 45 && height == 45 && walls == 204)  // quadrants (419 vs 394)
                         || (width == 31 && height == 31 && walls == 88)   // hooks (44 vs 22)
                         || (width == 45 && height == 30 && walls == 30);  // swirl (45 vs 30)
        // DEATH_AVOID won these vs Nyx (v103 probe): pilgrimage 873-521 (was 490-1016), corners 487 vs 372.
        death_avoid = (width == 50 && height == 65 && walls == 0) || (width == 71 && height == 71 && walls == 44);
        // The pre-v90 mid-game contested-camp penalty won these vs Nyx (v104 probe): starfish 239 vs 63,
        // starvation 208 vs 134, quadrants 444 vs 419, fragment 50 vs 40.
        old_contest_map = (width == 50 && height == 50 && walls == 32) || (width == 80 && height == 80 && walls == 664)
                          || (width == 45 && height == 45 && walls == 204) || (width == 30 && height == 30 && walls == 24);
    }
    engage_ratio = caster_power ? ENGAGE_RATIO : 0.8;
    {
        // elk_arena (all gems in the middle group): v53's full recall won 828-324 vs Nyx, backing off 612-540.
        int total = 0, top = 0;
        for (const auto& g : groups) {
            total += g.gems;
            top = std::max(top, g.gems);
        }
        if (total > 0 && top >= 0.9 * total) soft_back = false;
    }
    contest_mid = !soft_back || old_contest_map;
    // boss_battle (50x41): the reach fallback made it 2.6x slower (960 -> 2500 rounds unopposed).
    reach_fallback = (width == 50 && height == 41) ? 0 : REACH_FALLBACK;
    // islands (60x60, no walls): v24's settings got 718 gems vs Fish there, v26's 312.
    if (width == 60 && height == 60 && cambc::map::walls().count() == 0) {
        altar_weight = 0.0;
        shop_gold = 300;
    }
    // gold_rush (31x31, no walls): Fish farms the harmless gold groups around the middle for items
    // (twice our gold) and clears its three gem groups more often than we clear ours.
    if (width == 31 && height == 31 && cambc::map::walls().count() == 0) gold_weight = GOLD_RUSH_GOLD_W;
    // broken_curve (40x40; interlock is 40x40 too): v42's choice (walk weight 1, gold 0) beat Nyx 1013-901,
    // the walk weight 3 version lost 609-1050 there while gaining 90-200 gems on the other test maps.
    if (!caster_power) {
        eta_w = 1.0;
        gold_weight = 0.0;
    }
    // boss_battle (week-2 patch): the only altar's only free side is a monolith tile, so nobody can bank
    // until the 20000-HP monolith dies and games end on the gold tiebreak. Farm gold then, never walk to bank.
    {
        bitboard camp_mask;
        for (const auto& g : groups) camp_mask |= g.mask;
        if (!altar_approach.is_empty() && (altar_approach & ~camp_mask).is_empty()) {
            altar_blocked = true;
            gold_weight = 1.0;
            gem_weight = 0.0;
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

// Shared memory has only room for the claims: a slot with a claim counts as a squad member.
std::array<bool, 8> Bot::squad_alive(int) const {
    const auto c = claims();
    std::array<bool, 8> out{};
    for (int i = 0; i < 8; ++i) out[i] = c[i] != 0 || i == mem_slot;
    return out;
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
    if (KITE && kiter && kite(target, others)) return;
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
            if (chebyshev(w, pos) < WARD_GAP) return;
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
static double enemy_power(const std::vector<cambc::champion>& units) {
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
    for (const auto& a : cambc::vision::champions(team::ally))
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
    // 2. Winning fight: focus fire (same rule on every ally), finish kills first.
    if (ratio >= engage_ratio && hp_frac > 0.35) {
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
        if (soft_back && hp_frac >= SOFT_BACK_HP && gems < SOFT_BACK_GEMS) {
            // Healthy and carrying little: step out of reach and farm elsewhere instead of recalling
            // (vs Nyx we spent 10-30% of turns dead or walking back from spawn).
            if (!retreat_and_recall(false)) return false;
            if (caster_power) hit_in_range(me);  // parting shot: a chaser keeps pace, so hit it while leaving
            act = "back-off";
            return true;
        }
        retreat_and_recall();
        if (caster_power) hit_in_range(me);
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
    for (const auto& a : cambc::vision::champions(team::ally))
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
        return true;
    } else retreat_and_recall();
    if (caster_power) hit_in_range(me);
    act = "outnumbered";
    return true;
}

// Ladder replays: top teams kill with 3-5 champions and we die to 2-3 at a time. Help an ally who is
// fighting nearby when the two (or more) of us would win, instead of farming next to the fight.
bool Bot::join_fight(const cambc::champion& me, vec2 pos, int gems, double hp_frac) {
    if (gems >= 30 || hp_frac < 0.6) return false;
    std::vector<cambc::champion> enemies;
    const auto& enemy_spawn = cambc::map::spawn_region(team::enemy);
    for (auto& e : visible_enemies())
        if (e.is_alive() && !enemy_spawn.is_set(e.position())) enemies.push_back(e);
    if (enemies.empty()) return false;
    const auto allies_all = cambc::vision::champions(team::ally);
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
        if (power(ours) < engage_ratio * enemy_power(theirs)) continue;
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
    if (SQUAD > 1) {
        // Squads of SQUAD: champions ranked by memory slot (alive ones only); each squad's first
        // member picks camps, the rest go to its camp (Nyx and Fish kill our lone champions in a round).
        const auto alive = squad_alive(now);
        std::vector<int> slots;
        for (int i = 0; i < 8; ++i)
            if (alive[i]) slots.push_back(i);
        const auto it = std::find(slots.begin(), slots.end(), mem_slot);
        if (it != slots.end()) {
            const int rank = static_cast<int>(it - slots.begin());
            const int lead = slots[rank - rank % SQUAD];
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

// ------------------------------------------------------------------ turn
void Bot::play_turn() {
    act = "?";
    const auto start = cambc::fuel_remaining();
    const auto r0 = cambc::round_number();
    turn();
    if (WARDS) place_ward();
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
    const bool was_waiting_on_spawn = waited_on_spawn;
    waited_on_spawn = false;

    if (!me.is_alive()) {
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
    if (me.is_recalling() || me.is_submitting_gems() || me.is_teleporting()) {
        act = "channel";
        return;
    }

    const auto pos = me.position();
    const bool at_spawn = spawn.is_set(pos);
    if (at_spawn || kind == CT::tavi) shop();
    update_groups(now, pos);
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

    const double prev_hp = last_hp;
    if (me.health() < prev_hp) hit_round = now;
    last_hp = me.health();
    last_alive = true;
    last_pos = pos;
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
    if (gems > 0 && !altar_blocked && (cambc::global_submitted_gems(team::ally) + gems >= cambc::constants::winning_gems || now > 1940)) {
        tribute();
        return;
    }

    if (OUTNUMBER && outnumbered(me, pos, now)) return;
    if (pvp(me, pos, gems, hp_frac)) return;
    if (JOIN && join_fight(me, pos, gems, hp_frac)) return;

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
        } else {
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
        }
    } else {
        target_dist = std::max(0, target_dist - 1);
    }

    if (gems > 0 && !altar_blocked) {
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
                for (const auto& c : camps) ring |= camp_waiting_area(c.position(), camp_range(c.kind()));
                for (const auto& c : camps) ring &= ~around(c.position(), camp_range(c.kind()));
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
    const bool far_walk = TEAM_PLAN && gfields.size() > static_cast<std::size_t>(tgt) && closest > 8;
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
