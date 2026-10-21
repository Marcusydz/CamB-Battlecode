// Draft: a preference list chosen per map (maps are recognised by size and wall count).
#include <cambc.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include "draft.h"

using D = cambc::champion_draft;
using List = std::array<D, 11>;

// Candidate lists. 0 is v16's order (Ren, Tavi, Ryth, Stellar, Marcella), chosen by local tests against
// itself. 1 is what teams rated 1700+ pick on almost every map (690 ladder games, October 2026).
static constexpr std::array<List, 21> LISTS{{
    {D::ren, D::tavi, D::ryth, D::stellar, D::marcella, D::varg, D::vendetta, D::vesper_vale, D::prism, D::cassi_and_issac, D::willow},
    {D::cassi_and_issac, D::stellar, D::varg, D::ren, D::ryth, D::tavi, D::prism, D::vendetta, D::marcella, D::vesper_vale, D::willow},
    {D::stellar, D::cassi_and_issac, D::ren, D::ryth, D::varg, D::tavi, D::prism, D::vendetta, D::marcella, D::vesper_vale, D::willow},
    {D::cassi_and_issac, D::stellar, D::ren, D::ryth, D::tavi, D::varg, D::prism, D::vendetta, D::marcella, D::vesper_vale, D::willow},
    {D::varg, D::cassi_and_issac, D::stellar, D::ren, D::ryth, D::tavi, D::prism, D::vendetta, D::marcella, D::vesper_vale, D::willow},
    {D::ren, D::stellar, D::ryth, D::tavi, D::cassi_and_issac, D::varg, D::marcella, D::prism, D::vendetta, D::vesper_vale, D::willow},
    {D::stellar, D::cassi_and_issac, D::varg, D::ryth, D::willow, D::ren, D::tavi, D::prism, D::vendetta, D::marcella, D::vesper_vale},
    // 7-10: Intercontinental Fish (ladder #1, 42 games): their usual draft, expanse, contest/stampede, rows.
    {D::stellar, D::ren, D::cassi_and_issac, D::ryth, D::varg, D::tavi, D::prism, D::vendetta, D::marcella, D::vesper_vale, D::willow},
    {D::cassi_and_issac, D::stellar, D::varg, D::ren, D::ryth, D::tavi, D::prism, D::vendetta, D::marcella, D::vesper_vale, D::willow},
    {D::stellar, D::tavi, D::prism, D::cassi_and_issac, D::ren, D::ryth, D::varg, D::vendetta, D::marcella, D::vesper_vale, D::willow},
    {D::stellar, D::prism, D::cassi_and_issac, D::varg, D::ren, D::ryth, D::tavi, D::vendetta, D::marcella, D::vesper_vale, D::willow},
    // 11-13: no Cassi & Issac (on gold_rush vs Fish they died together 5 times and farmed 46 gems each).
    {D::varg, D::stellar, D::ryth, D::ren, D::tavi, D::prism, D::cassi_and_issac, D::vendetta, D::marcella, D::vesper_vale, D::willow},
    {D::varg, D::ryth, D::stellar, D::ren, D::tavi, D::prism, D::cassi_and_issac, D::vendetta, D::marcella, D::vesper_vale, D::willow},
    {D::varg, D::stellar, D::ren, D::ryth, D::tavi, D::prism, D::cassi_and_issac, D::vendetta, D::marcella, D::vesper_vale, D::willow},
    {D::varg, D::stellar, D::tavi, D::ryth, D::ren, D::prism, D::cassi_and_issac, D::vendetta, D::marcella, D::vesper_vale, D::willow},
    {D::varg, D::stellar, D::prism, D::ryth, D::ren, D::tavi, D::cassi_and_issac, D::vendetta, D::marcella, D::vesper_vale, D::willow},
    // 16: after the week-2 patch (nerfs to Stellar, Varg, Ryth, Ren, Cassi & Issac, Tavi; Willow buffed; new Heart of Ruin).
    // Unopposed on cambc 3.0.1 it farmed faster than list 4 on most of the 21 maps.
    {D::heart_of_ruin, D::varg, D::stellar, D::ryth, D::willow, D::ren, D::marcella, D::cassi_and_issac, D::prism, D::tavi, D::vendetta},
    // 17: patch 1.0.2 nerfed Heart of Ruin (strength 40 -> 30) and buffed Vesper Vale (health 1000, strength 55).
    {D::vesper_vale, D::varg, D::stellar, D::ryth, D::willow, D::ren, D::marcella, D::cassi_and_issac, D::prism, D::tavi, D::vendetta},
    // 18/19: lists 16/17 with Vendetta (buffed in 1.0.2) in Stellar's place (Stellar nerfed).
    {D::heart_of_ruin, D::varg, D::vendetta, D::ryth, D::willow, D::ren, D::marcella, D::cassi_and_issac, D::prism, D::tavi, D::stellar},
    {D::vesper_vale, D::varg, D::vendetta, D::ryth, D::willow, D::ren, D::marcella, D::cassi_and_issac, D::prism, D::tavi, D::stellar},
    // 20: list 16 with Prism (buffed in 1.0.2) first and Heart of Ruin later.
    {D::prism, D::varg, D::stellar, D::ryth, D::willow, D::ren, D::marcella, D::cassi_and_issac, D::heart_of_ruin, D::tavi, D::vendetta},
}};

// Ranked ladder, 21 Oct (260 games): Prism first went 8-29 on its maps (elk_arena 0-9 among them);
// Vesper first 52-28, Heart of Ruin first 58-64. 20 = v1.1.0 (Prism first); 17 = Vesper first; 16 = Heart of Ruin first.
constexpr int PRISM_MAPS_LIST = 20;
constexpr int DRAFT = -1;  // >=0: use this list on every map (testing). -1: per-map table below.
constexpr int FISH_DRAFT = 0;  // 1: Intercontinental Fish's per-map drafts instead of PER_MAP

struct MapDraft {
    int w, h, walls, list;
};
// Per-map choice, from local tests against v16 (see README).
static constexpr std::array<MapDraft, 17> PER_MAP{{
    {100, 100, 540, 2},  // ambush
    {41, 41, 84, 4},     // collision
    {50, 50, 40, 0},     // contest
    {71, 71, 44, 0},     // corners
    {41, 30, 36, 0},     // eight
    {30, 30, 0, 3},      // elkring
    {80, 80, 224, 4},    // expanse
    {30, 30, 24, 0},     // fragment
    {40, 40, 76, 4},     // interlock (list 4 farms 602 -> 462 rounds)
    {90, 30, 0, 4},      // lanes
    {50, 50, 98, 7},     // monolith (list 7 farms 412 -> 354 rounds)
    {31, 31, 0, 15},     // gold_rush (also old packs): no Cassi & Issac (Ren instead of Ryth was worse vs Fish)
    {50, 65, 0, 5},      // pilgrimage
    {50, 50, 214, 4},    // rows
    {30, 80, 80, 5},     // rushbait_v2
    {60, 50, 0, 0},      // stampede
    {60, 60, 0, 13},     // islands
}};
// Intercontinental Fish: list 7 everywhere except these.
static constexpr std::array<MapDraft, 4> FISH_MAP{{
    {80, 80, 224, 8},    // expanse
    {50, 50, 40, 9},     // contest
    {60, 50, 0, 9},      // stampede
    {50, 50, 214, 10},   // rows
}};
constexpr int DEFAULT_LIST = 4;
constexpr int POST_PATCH_LIST = 16;  // every map except 40x40  // maps not in the table (new or unseen ones)

bool is_broken_curve() {
    // (wall count, not camps: camp groups are not known yet during the draft)
    return cambc::map::width() == 40 && cambc::map::height() == 40 && cambc::map::walls().count() == 274;
}

static int choose_list() {
    if (DRAFT >= 0) return DRAFT;
    const int w = cambc::map::width(), h = cambc::map::height();
    const int walls = static_cast<int>(cambc::map::walls().count());
    // Vesper Vale instead of Heart of Ruin won here after patch 1.0.2 (v121 probe, Nyx + Fish): pilgrimage +365
    // (beat Nyx 791-543), expanse +287, ambush +238, rushbait_v2 +231, islands +206, stampede +160, starfish +141,
    // monolith +130, eight +97, CW +82, rows +78 (beat Nyx 1017-991), the_farm +72.
    // Vendetta instead of Stellar won clearly only here (v122 probe 207, Nyx + Fish): fracture +209, CW +186,
    // islands +118. Everywhere else it lost (-3300 overall).
    static constexpr int VENDETTA[][3] = {{61, 71, 374}, {61, 61, 210}, {60, 60, 0}};
    bool vendetta = false;
    for (const auto& m : VENDETTA)
        if (w == m[0] && h == m[1] && walls == m[2]) vendetta = true;
    // Prism first won here (v124 probe 209, Nyx + Fish): elk_arena +468 (beat Nyx 1044-252), packs +179
    // (gold_rush, same fingerprint, +0), shortcut +113, collision +56. It lost 3000+ on the other maps.
    static constexpr int PRISM[][3] = {{50, 50, 222}, {31, 31, 0}, {40, 40, 320}, {41, 41, 84}};
    for (const auto& m : PRISM)
        if (w == m[0] && h == m[1] && walls == m[2]) return PRISM_MAPS_LIST;
    static constexpr int VESPER[][3] = {{50, 65, 0},  {80, 80, 224}, {100, 100, 540}, {30, 80, 80},
                                        {60, 60, 0},  {60, 50, 0},   {50, 50, 32},    {50, 50, 98},
                                        {41, 30, 36}, {61, 61, 210}, {50, 50, 214},   {50, 40, 148}};
    for (const auto& m : VESPER)
        if (w == m[0] && h == m[1] && walls == m[2]) return vendetta ? 19 : 17;
    if (FISH_DRAFT) {
        for (const auto& m : FISH_MAP)
            if (m.w == w && m.h == h && m.walls == walls) return m.list;
        return 7;
    }
    // broken_curve and interlock keep the old lists: we beat Nyx on broken_curve with list 4.
    // (shortcut is 40x40 too: it gets the post-patch list.)
    if (w == 40 && h == 40 && (is_broken_curve() || walls == 76)) {
        for (const auto& m : PER_MAP)
            if (m.w == w && m.h == h && m.walls == walls) return m.list;
        return DEFAULT_LIST;
    }
    return vendetta ? 18 : POST_PATCH_LIST;
}

struct MapProfile {
    double contested = 0.0;
    double wall_density = 0.0;
    double gem_concentration = 0.0;
    double open = 0.0;
};

static MapProfile map_profile() {
    const auto ally_spawn = cambc::map::spawn_region(cambc::team::ally).positions();
    const auto enemy_spawn = cambc::map::spawn_region(cambc::team::enemy).positions();
    double total = 0.0, contested = 0.0, largest_group = 0.0;
    for (const auto& group : cambc::map::camp_groups()) {
        const auto tiles = group.spawn_tiles().positions();
        int ally_dist = 1 << 20, enemy_dist = 1 << 20;
        for (const auto a : ally_spawn)
            for (const auto tile : tiles)
                ally_dist = std::min(ally_dist, std::max(std::abs(a.x - tile.x), std::abs(a.y - tile.y)));
        for (const auto e : enemy_spawn)
            for (const auto tile : tiles)
                enemy_dist = std::min(enemy_dist, std::max(std::abs(e.x - tile.x), std::abs(e.y - tile.y)));
        const double gems = static_cast<double>(group.gem_value());
        total += gems;
        if (std::abs(ally_dist - enemy_dist) <= 6) contested += gems;
        largest_group = std::max(largest_group, gems);
    }
    const double area = static_cast<double>(cambc::map::width()) * cambc::map::height();
    const double walls = area ? cambc::map::walls().count() / area : 0.0;
    // Lots of walls and elongated maps tend to make safe routes and ranged coverage more useful.
    const double aspect = static_cast<double>(std::max(cambc::map::width(), cambc::map::height())) /
                          std::max(1, std::min(cambc::map::width(), cambc::map::height()));
    const double openness = std::clamp(1.0 - walls * 5.0 - (aspect - 1.0) * 0.12, 0.0, 1.0);
    return {total ? contested / total : 0.0, walls, total ? largest_group / total : 0.0, openness};
}

enum Trait : unsigned {
    ranged = 1u << 0, front_line = 1u << 1, burst = 1u << 2, area = 1u << 3,
    control = 1u << 4, sustain = 1u << 5, mobile = 1u << 6, economy = 1u << 7,
};

static unsigned traits(D c) {
    switch (c) {
    case D::ryth: return front_line | area | control | sustain;
    case D::vesper_vale: return ranged | mobile | economy;
    case D::vendetta: return front_line | mobile | burst;
    case D::cassi_and_issac: return ranged | front_line | burst | control;
    case D::stellar: return ranged | burst | area;
    case D::prism: return ranged | burst | area | control;
    case D::marcella: return front_line | burst | sustain;
    case D::willow: return ranged | sustain;
    case D::ren: return front_line | mobile | control;
    case D::varg: return front_line | area | sustain;
    case D::tavi: return ranged | burst | control | economy;
    case D::heart_of_ruin: return front_line | burst | control | sustain | mobile;
    }
    return 0;
}

static const char* champion_name(D c) {
    switch (c) {
    case D::ryth: return "Ryth";
    case D::vesper_vale: return "Vesper Vale";
    case D::vendetta: return "Vendetta";
    case D::cassi_and_issac: return "Cassi & Issac";
    case D::stellar: return "Stellar";
    case D::prism: return "Prism";
    case D::marcella: return "Marcella";
    case D::willow: return "Willow";
    case D::ren: return "Ren";
    case D::varg: return "Varg";
    case D::tavi: return "Tavi";
    case D::heart_of_ruin: return "Heart of Ruin";
    }
    return "unknown";
}

struct DraftScore {
    double map = 0.0;
    double synergy = 0.0;
    double matchup = 0.0;
    double disruption = 0.0;
};

static DraftScore evaluate(D pick, const std::vector<D>& allies, const std::vector<D>& enemies,
                           const MapProfile& map) {
    const unsigned t = traits(pick);
    int front_count = 0, ranged_count = 0, burst_count = 0, area_count = 0, control_count = 0;
    int enemy_ranged = 0, enemy_sustain = 0;
    bool ally_front = false, ally_ranged = false, ally_area = false, ally_control = false;
    for (D c : allies) {
        const unsigned ct = traits(c);
        front_count += (ct & front_line) != 0;
        ranged_count += (ct & ranged) != 0;
        burst_count += (ct & burst) != 0;
        area_count += (ct & area) != 0;
        control_count += (ct & control) != 0;
        ally_front |= (ct & front_line) != 0;
        ally_ranged |= (ct & ranged) != 0;
        ally_area |= (ct & area) != 0;
        ally_control |= (ct & control) != 0;
    }
    for (D c : enemies) {
        const unsigned ct = traits(c);
        enemy_ranged += (ct & ranged) != 0;
        enemy_sustain += (ct & sustain) != 0;
    }

    DraftScore s;
    // Match the existing map ranking with broad geometry-derived needs.
    s.map += map.contested * ((t & front_line) ? 0.75 : 0.0);
    s.map += map.contested * ((t & control) ? 0.55 : 0.0);
    s.map += map.gem_concentration * ((t & area) ? 0.65 : 0.0);
    s.map += (1.0 - map.contested) * ((t & economy) ? 0.7 : 0.0);
    s.map += map.open * ((t & ranged) ? 0.35 : 0.0);
    s.map -= map.wall_density * 1.5 * ((t & ranged) ? 1.0 : 0.0);
    if (pick == D::vesper_vale) s.map += (1.0 - map.contested) * 0.45;
    if (pick == D::cassi_and_issac) s.map -= map.contested * 0.25; // two linked bodies risk losing bags together

    // Reward missing jobs, useful pairings, and rounded compositions; discourage stacking one role.
    if ((t & front_line) && !ally_front) s.synergy += 0.65;
    if ((t & ranged) && !ally_ranged) s.synergy += 0.45;
    if ((t & area) && !ally_area) s.synergy += 0.35;
    if ((t & control) && !ally_control) s.synergy += 0.4;
    if ((t & front_line) && ranged_count > 0) s.synergy += 0.15;
    if ((t & ranged) && front_count > 0) s.synergy += 0.15;
    if ((t & sustain) && front_count > 0) s.synergy += 0.25;
    if (pick == D::willow && front_count > 0) s.synergy += 0.4;
    if ((t & front_line) && front_count >= 2) s.synergy -= 0.45 * (front_count - 1);
    if ((t & burst) && burst_count >= 2 && map.contested < 0.35) s.synergy -= 0.2 * burst_count;
    if ((t & area) && area_count >= 2 && map.gem_concentration < 0.35) s.synergy -= 0.2 * area_count;
    if ((t & control) && control_count >= 2) s.synergy -= 0.15 * control_count;

    // Specific, explainable interactions from the game rules.
    for (D enemy : enemies) {
        if (enemy == D::marcella) {
            if ((t & ranged) || pick == D::vendetta) s.matchup += 0.38;
            if (t & control) s.matchup -= 0.22; // her stun immunity lowers the value of control
        } else if (enemy == D::cassi_and_issac) {
            if ((t & burst) || (t & area)) s.matchup += 0.48; // either linked body dying removes both
        } else if (enemy == D::willow) {
            if (t & burst) s.matchup += 0.28; // burst pressures healing windows
        } else if (enemy == D::varg) {
            if (t & burst) s.matchup += 0.2;
        }
    }
    s.matchup = std::clamp(s.matchup, -1.0, 1.0);

    // Champion selection is mirrored across teams, so we cannot deny a specific pick.
    // Disrupt the revealed plan instead: dive/control into a ranged back line and burst into sustain.
    if (enemy_ranged >= 2 && (t & (mobile | control))) s.disruption += 0.45;
    if (enemy_sustain >= 2 && (t & burst)) s.disruption += 0.35;
    if (enemy_ranged >= 3 && (t & front_line)) s.disruption += 0.2;
    s.disruption = std::min(s.disruption, 0.7);
    return s;
}

void play_draft() {
    static const int list = choose_list();
    static const MapProfile map = map_profile();
    const auto chosen = cambc::draft::current_draft(cambc::team::ally);
    const auto enemies = cambc::draft::current_draft(cambc::team::enemy);
    D best = LISTS[list][0];
    double best_score = -1e9;
    DraftScore best_parts;
    for (std::size_t rank = 0; rank < LISTS[list].size(); ++rank) {
        const D pick = LISTS[list][rank];
        if (std::find(chosen.begin(), chosen.end(), pick) != chosen.end()) continue;
        const auto parts = evaluate(pick, chosen, enemies, map);
        // Keep the established, locally tested map order as the stable fallback signal.
        const double score = static_cast<double>(LISTS[list].size() - rank) * 0.28 +
                             parts.map * 1.25 + parts.synergy * 1.15 +
                             parts.matchup * 2.0 + parts.disruption * 1.25;
        if (score > best_score) {
            best_score = score;
            best = pick;
            best_parts = parts;
        }
    }
    std::printf("draft %dx%d c%.2f w%.3f g%.2f pick=%s score=%.2f map=%.2f team=%.2f matchup=%.2f disrupt=%.2f\n",
                cambc::map::width(), cambc::map::height(), map.contested, map.wall_density,
                map.gem_concentration, champion_name(best), best_score, best_parts.map,
                best_parts.synergy, best_parts.matchup, best_parts.disruption);
    cambc::draft::pick(best);
}
