// Tuning switches. Compile-time constants: scripts/cvariant.sh overrides them by name.
#pragma once
#include <cambc.h>
#include <cstdint>

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
constexpr int THREAT_RANGE = 6;         // enemies this close stop a recall (attack range 4 + two steps; 5 until v1.5.0)
constexpr bool SOLO_CAPTURE = true;     // only one champion holds a capture; others move on
// Fuel thresholds are fractions of the turn budget. C++ gets 100k fuel per turn (Python 2.5M), so
// my-bot's absolute values (700k reserve, low fuel below 1.6M) are scaled: 0.28 and 0.64 of a turn.
constexpr double FUEL_RESERVE = 0.28;   // skip optional work when less than this fraction of the turn remains
constexpr double LOW_FUEL = 0.64;       // below this fraction, keep the current target instead of replanning
constexpr std::uint64_t TURN_FUEL = 100000;
constexpr double LOW_HP = 0.3;          // (0.4 until v1.5.0) retreat below this health fraction (+LOW_HP_GEMS when carrying 30+ gems)
constexpr double LOW_HP_GEMS = 0.15;
constexpr double SPAWN_HEAL_TO = 0.95;  // stay on spawn until this healthy
constexpr double CAMP_RISK = 1.5;      // a team may take a camp group whose damage over the clear is up to this many times its total health
constexpr int GATHER_MAX = 12;          // give up waiting for helpers after this many turns
constexpr double JOIN_BONUS = 1.5;      // score boost for joining a joint target an ally already claimed
constexpr double HEAL_MIN_USE = 0.6;    // heal only if at least this fraction of the heal lands (or the ally is below half HP)
constexpr int PRISM_GEM_CAP = 60;       // Prism gains 1.5 focus per carried gem (GameRules); 90 tested level with 60 (v1.1.0)
constexpr int PRISM_EARLY_BANK = 0;
constexpr int SPAWN_HOLD = 6;          // >0: stay on spawn while enemies this close would win the fight there (maps with spawn_hold)
constexpr int SPAWN_HOLD_MAX = 40;     // ... for at most this many turns in a row
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
constexpr int TAVI_STRENGTH = 0;     // 1: Tavi builds strength (basic attacks every turn) instead of focus (a spell every 8)
constexpr int SEEN_EVERY_TURN = 1;  // 1: refresh enemy memory at the start of every turn (was only in pvp, after the low-health retreat)
constexpr int LOWHP_FIGHT_KILL_ONLY = 1;  // 1: below LOW_HP and being hit, fight back only for a kill (else keep retreating)
constexpr int DANGER_RETREAT = 0;   // 1: also retreat when health will not last DANGER_TURNS x the turns needed to get out of reach (2: monoliths only)
constexpr double DANGER_TURNS = 1.0;
constexpr int RETREAT_SAFE = 6;     // retreat steps maximise distance from threats up to this, then distance home wins
constexpr int ENDGAME_THEIRS = 900;   // >0: once the enemy has banked this many gems, bank everything we carry
constexpr int BACKOFF_AVOID = 0;    // >0: after backing off from enemies, skip the camp we were heading for (if within 8) this many rounds
constexpr double LOW_HP_ENEMY = 0.35;  // retreat slightly earlier when an enemy champion is within 5 tiles; camps keep LOW_HP
constexpr int ERROR_LOG = 1;  // 1: say which end-of-turn step raised an engine error
constexpr int DEATH_TRACE = 1;  // 1: after the death line, print our last turns (position, health, act, enemies seen)
constexpr int DEATH_LOG = 1;          // one line per death (ladder replays carry our stdout): where, doing what, odds
constexpr double SOFT_BACK_HP = 0.6;     // disengage without recalling when at least this healthy
constexpr int SOFT_BACK_GEMS = 20;       // ... and carrying fewer gems than this (was 10)

// Rules v1.0.0 did not use (docs/UnusedRules.md). 0 = v1.0.0 behaviour. v1.1.0 turned on the first four that won,
// v1.2.0 adds MAP_FIELDS (docs/versions/v1.2.0.md).
constexpr int HOR_SLOT_FIX = 1;   // 1: Heart of Ruin gets its draft slot (it fell into the catch: slot handle % 8, team_size 3)
constexpr int SQUAD_FIX = 0;      // 1: a live champion without a claim writes 255, so squads still count it
constexpr int RANGED_RESET = 1;   // 1: ranged champions that moved cast their attack reset, then attack
constexpr int MOVE_RETRY = 1;     // 1: a step blocked by a hidden unit tries the next best tiles (the failed step is free)
constexpr int HOR_DASH = 1;       // 1: Heart of Ruin lands its damaging, stunning dash on enemies and camps
constexpr int PRIORITY = 0;       // 1: champions with a stun act first within our team
constexpr int MAP_FIELDS = 1;     // 1: precompute every camp group's distance map from spare fuel, then plan by lookup instead of a search
constexpr int CANCEL_RECALL = 0;  // 1: cancel a recall (when healthy) to take a kill in attack range
constexpr int WOLF_KITE = 0;      // 1: casters fight wolf-only groups from 4 tiles, outside wolf range 3
constexpr int FOCUS_FIRE = 0;     // 1: shared memory slots 5-6 hold the team's focus target; engaging allies prefer it
constexpr int CONTEST = 0;        // 1: walk into a camp region an enemy is capturing (resets their 5-round timer) when we'd win the fight

// Per-map settings: defaults here, set for the current map in Bot::apply_map_settings().
// Casters deal damage from focus. Counting focus at 0.25 made Nyx (Ren, Stellar, Cassi & Issac stacking
// focus) look weak: we engaged and lost 1-12 in kills on lanes. Off on 40x40 maps (broken_curve stays as v42).
inline bool caster_power = true;
inline double engage_ratio = ENGAGE_RATIO;
inline bool treasure_first = false;   // treasure emblem opens the build (per map, see setup)
inline bool death_avoid = false;      // avoid camps where enemy champions killed us (per map, see setup)
inline bool old_contest_map = false;  // the pre-v90 contested-camp rule won here (per map, see setup)
inline bool contest_mid = false;      // mid-game contested-camp penalty: single-herd maps and old_contest_map
inline int squad = SQUAD;             // squad size (per map, see setup)
inline bool spawn_hold = false;       // see SPAWN_HOLD (per map, see setup)
inline int gem_cap = GEM_CAP;          // bank at this many gems (per map, see setup)
inline int ward_gap = WARD_GAP;        // per map, see setup
inline double low_hp = LOW_HP;        // retreat threshold (per map, see setup)
inline bool soft_back = true;            // see SOFT_BACK_HP; off on 40x40 maps
