// Per-map behaviour, by map fingerprint (width, height, wall count). Each block records the probe that chose it.
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

void Bot::apply_map_settings() {
    // broken_curve kept v42's play (no caster focus in fight estimates, full recalls) until 1.0.2 broke it:
    // normal play scored Nyx 539 + Fish 853 there vs 464 + 476 (v125 map probes 218-222, Marcus allowed the change).
    caster_power = true;
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
                         || (width == 45 && height == 30 && walls == 30)   // swirl (45 vs 30)
                         // v106 probe vs the changed Nyx and Fish (sum of both):
                         || (width == 100 && height == 100 && walls == 636)  // big_world (won 1024-876 vs Fish)
                         || (width == 50 && height == 50 && walls == 222)    // elk_arena (936-216 vs Nyx, was 612-468)
                         || (width == 60 && height == 60 && walls == 0)      // islands (+178)
                         || (width == 31 && height == 31 && walls == 0)      // packs and gold_rush (+120, +22)
                         || (width == 30 && height == 80 && walls == 80)     // rushbait_v2 (+95)
                         || (width == 61 && height == 61 && walls == 210)    // concentrated_wealth (+40)
                         || (width == 80 && height == 80 && walls == 224);   // expanse (+26)
        // DEATH_AVOID won these vs Nyx (v103 probe): pilgrimage 873-521 (was 490-1016), corners 487 vs 372.
        // Death avoidance won on these vs Nyx + Fish (v107 probe): lanes +273, stampede +160, swirl +83, CW, corridors.
        death_avoid = (width == 50 && height == 65 && walls == 0) || (width == 71 && height == 71 && walls == 44)  // pilgrimage, corners
                      || (width == 90 && height == 30 && walls == 0)      // lanes
                      || (width == 60 && height == 50 && walls == 0)      // stampede
                      || (width == 45 && height == 30 && walls == 30)     // swirl
                      || (width == 61 && height == 61 && walls == 210)    // concentrated_wealth
                      || (width == 51 && height == 40 && walls == 378)    // corridors
                      // v111 probe vs Nyx's 02:30 bot + Fish: monolith +135, islands +116, interlock +111, hooks +44
                      || (width == 50 && height == 50 && walls == 98) || (width == 60 && height == 60 && walls == 0)
                      || (width == 40 && height == 40 && walls == 76) || (width == 31 && height == 31 && walls == 88);
        // The pre-v90 mid-game contested-camp penalty won these vs Nyx (v104 probe): starfish 239 vs 63,
        // starvation 208 vs 134, quadrants 444 vs 419, fragment 50 vs 40.
        old_contest_map = (width == 50 && height == 50 && walls == 32) || (width == 80 && height == 80 && walls == 664)
                          || (width == 45 && height == 45 && walls == 204) || (width == 30 && height == 30 && walls == 24)
                          // v108 probe (Nyx + Fish sum): monolith +250, the_farm +155, collision +148, packs +70 and gold_rush +126,
                          // rushbait_v2 +40, lanes +27, eight +19, interlock +18.
                          || (width == 50 && height == 50 && walls == 98)     // monolith
                          || (width == 50 && height == 40 && walls == 148)    // the_farm
                          || (width == 41 && height == 41 && walls == 84)     // collision
                          || (width == 31 && height == 31 && walls == 0)      // packs and gold_rush
                          || (width == 30 && height == 80 && walls == 80)     // rushbait_v2
                          || (width == 90 && height == 30 && walls == 0)      // lanes
                          || (width == 41 && height == 30 && walls == 36)     // eight
                          || (width == 40 && height == 40 && walls == 76)     // interlock
                          // v112 re-probe vs Nyx's 02:30 bot + Fish: split +230, corridors +124, expanse +107
                          || (width == 45 && height == 35 && walls == 116) || (width == 51 && height == 40 && walls == 378)
                          || (width == 80 && height == 80 && walls == 224);
    }
    engage_ratio = caster_power ? ENGAGE_RATIO : 0.8;
    {
        // Fight threshold per map (v110 probes, Nyx + Fish sum; 1.3 and 0.8 were both worse overall).
        // Rechecked vs Nyx's 02:30 bot: 0.8 on the_farm +110, 1.3 on starfish +11 (big_world, fragment lost).
        const int walls = static_cast<int>(cambc::map::walls().count());
        if (width == 50 && height == 50 && walls == 32)
            engage_ratio = 1.3;
        else if (width == 50 && height == 40 && walls == 148)
            engage_ratio = 0.8;
    }
    {
        // elk_arena (all gems in the middle group): v53's full recall won 828-324 vs Nyx, backing off 612-540.
        int total = 0, top = 0;
        for (const auto& g : groups) {
            total += g.gems;
            top = std::max(top, g.gems);
        }
        if (total > 0 && top >= 0.9 * total) soft_back = false;
    }
    {
        // Treasure opening flipped vs the v106 list (v113 probe; summed over Nyx's 02:30 bot, its older bot and Fish):
        // off on pilgrimage +175, collision +139, expanse +41, hooks +66; on for the_farm +65, shortcut +87.
        // Elk keeps treasure: off scored more vs the 02:30 Nyx (576 vs 0) but loses our 936-216 win vs the older one.
        const int walls = static_cast<int>(cambc::map::walls().count());
        if ((width == 50 && height == 65 && walls == 0)
            || (width == 41 && height == 41 && walls == 84) || (width == 80 && height == 80 && walls == 224)
            || (width == 31 && height == 31 && walls == 88))
            treasure_first = false;
        if ((width == 50 && height == 40 && walls == 148) || (width == 40 && height == 40 && walls == 320))
            treasure_first = true;
    }
    {
        // Flip probes vs v115 (official 194/195). Rechecked against v115 after Nyx and Fish changed bots ~05:20 UTC
        // (Nyx + Fish sum): starfish +83, hooks +44, pilgrimage +30, fragment +16, eight +11, lanes +7, reverse_spiral +6.
        const int walls = static_cast<int>(cambc::map::walls().count());
        if ((width == 50 && height == 65 && walls == 0) || (width == 90 && height == 30 && walls == 0)
            || (width == 31 && height == 31 && walls == 88))
            death_avoid = false;
        if ((width == 50 && height == 50 && walls == 32) || (width == 30 && height == 30 && walls == 24)
            || (width == 30 && height == 30 && walls == 102))
            death_avoid = true;
        if (width == 41 && height == 30 && walls == 36) old_contest_map = false;
    }
    {
        // Treasure opening flipped again vs Nyx and Fish's ~05:20 bots (v118 probe, Nyx + Fish sum): the_farm +424,
        // packs +475 and gold_rush +71, elk +288, fracture +279, CW +253, corners +246, big_world +228, islands +220,
        // shortcut +215, interlock +160, stampede +160, expanse +149, monolith +94, eight +72.
        const int walls = static_cast<int>(cambc::map::walls().count());
        static constexpr int FLIP[][3] = {{50, 40, 148}, {31, 31, 0},   {50, 50, 222}, {61, 71, 374}, {61, 61, 210},
                                          {71, 71, 44},  {100, 100, 636}, {60, 60, 0},  {40, 40, 320}, {40, 40, 76},
                                          {60, 50, 0},   {80, 80, 224},  {50, 50, 98},  {41, 30, 36}};
        for (const auto& m : FLIP)
            if (width == m[0] && height == m[1] && walls == m[2]) treasure_first = !treasure_first;
    }
    {
        // Old contest rule flipped vs the ~05:20 rivals (v119 probe, Nyx + Fish sum): starvation +144, lanes +116,
        // the_farm +106, eight +93, rushbait_v2 +77, monolith +74, CW +64.
        const int walls = static_cast<int>(cambc::map::walls().count());
        static constexpr int FLIP[][3] = {{80, 80, 664}, {90, 30, 0}, {50, 40, 148}, {41, 30, 36},
                                          {30, 80, 80},  {50, 50, 98}, {61, 61, 210}};
        for (const auto& m : FLIP)
            if (width == m[0] && height == m[1] && walls == m[2]) old_contest_map = !old_contest_map;
    }
    contest_mid = !soft_back || old_contest_map;
    // rows: retreating at 55% health won 1005-972 vs Nyx and +66 vs Fish (v115 probe; worse on most other maps).
    if (width == 50 && height == 50 && static_cast<int>(cambc::map::walls().count()) == 214) low_hp = 0.55;
    // Squad size per map (v109/v110 probes, Nyx + Fish sum): 2 won starvation +168, interlock +163,
    // reverse_spiral +52, rushbait_v2 +44; one squad of 5 won split +234, pilgrimage +164, quadrants +127,
    // fracture +125, expanse +38, shortcut +26.
    {
        const int walls = static_cast<int>(cambc::map::walls().count());
        if ((width == 80 && height == 80 && walls == 664) || (width == 40 && height == 40 && walls == 76)
            || (width == 30 && height == 30 && walls == 102) || (width == 30 && height == 80 && walls == 80))
            squad = 2;
        else if ((width == 45 && height == 35 && walls == 116) || (width == 50 && height == 65 && walls == 0)
                 || (width == 45 && height == 45 && walls == 204) || (width == 61 && height == 71 && walls == 374)
                 || (width == 80 && height == 80 && walls == 224) || (width == 40 && height == 40 && walls == 320))
            squad = 5;
        // v120 probe vs the ~05:20 rivals: squads of 3 on quadrants +326 and rows (beat Nyx 1003-982);
        // one squad of 5 on elk +288, corridors +224, rushbait_v2 +83.
        if ((width == 45 && height == 45 && walls == 204) || (width == 50 && height == 50 && walls == 214)) squad = 3;
        if ((width == 50 && height == 50 && walls == 222) || (width == 51 && height == 40 && walls == 378)
            || (width == 30 && height == 80 && walls == 80))
            squad = 5;
    }
    {
        // Spawn hold won here vs Nyx + Fish (v126 probe 223): big_world +194, lanes +115, collision +70, CW +61,
        // packs +54 (gold_rush +12), corridors +52, reverse_spiral +45. It lost 1800 on the other maps.
        static constexpr int HOLD[][3] = {{100, 100, 636}, {90, 30, 0},  {41, 41, 84}, {61, 61, 210},
                                          {31, 31, 0},     {51, 40, 378}, {30, 30, 102}};
        const int walls = static_cast<int>(cambc::map::walls().count());
        for (const auto& m : HOLD)
            if (width == m[0] && height == m[1] && walls == m[2]) spawn_hold = true;
    }
    {
        // Probes 226-235 on the maps outside the 30-map set (Nyx + Fish): yellow_brick_road retreat at 55% +462
        // (beats Nyx), broken_curve fight threshold 1.3 +142, boss_battle treasure flip beats Nyx on tiebreak.
        const int walls = static_cast<int>(cambc::map::walls().count());
        if (width == 40 && height == 30) low_hp = 0.55;
        if (width == 40 && height == 40 && walls == 274) engage_ratio = 1.3;
        if (width == 50 && height == 41) treasure_first = !treasure_first;
        // Banking at 30 gems (probe 239, Codex's Fish banking observation) lost 4600 overall but won these:
        // shortcut +308, reverse_spiral +75, hooks +66.
        if ((width == 40 && height == 40 && walls == 320) || (width == 30 && height == 30 && walls == 102)
            || (width == 31 && height == 31 && walls == 88))
            gem_cap = 30;
        // Banking at 90 (probe 245) won broken_curve +186. A ward every 5 tiles instead of 8 (probe 246; every 16
        // lost 6700) won concentrated_wealth +214, yellow_brick_road +135, corridors +117, fracture +78.
        if (width == 40 && height == 40 && walls == 274) gem_cap = 90;
        if ((width == 61 && height == 61 && walls == 210) || (width == 40 && height == 30)
            || (width == 51 && height == 40 && walls == 378) || (width == 61 && height == 71 && walls == 374))
            ward_gap = 5;
    }
    {
        // Flip probes on v133 (251-253, Nyx + Fish): treasure opening flipped won yellow_brick_road +522,
        // packs/gold_rush +48/+76; death avoidance flipped won islands +264; the contested-camp rule flipped
        // won the_farm +441 (beat Fish 1004-473), CW +175, expanse +147, fracture +138, pilgrimage +82.
        const int walls = static_cast<int>(cambc::map::walls().count());
        if ((width == 40 && height == 30) || (width == 31 && height == 31 && walls == 0)) treasure_first = !treasure_first;
        if (width == 60 && height == 60 && walls == 0) death_avoid = !death_avoid;
        if ((width == 50 && height == 40 && walls == 148) || (width == 61 && height == 61 && walls == 210)
            || (width == 80 && height == 80 && walls == 224) || (width == 61 && height == 71 && walls == 374)
            || (width == 50 && height == 65 && walls == 0)) {
            old_contest_map = !old_contest_map;
            contest_mid = !soft_back || old_contest_map;
        }
        if (width == 45 && height == 35 && walls == 116) treasure_first = !treasure_first;  // split only
    }
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
}
