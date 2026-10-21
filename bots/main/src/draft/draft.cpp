// Draft: a preference list chosen per map (maps are recognised by size and wall count).
#include <cambc.h>
#include <algorithm>
#include <array>
#include <cstdint>
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

static int map_contested_gem_share() {
    const auto ally_spawn = cambc::map::spawn_region(cambc::team::ally).positions();
    const auto enemy_spawn = cambc::map::spawn_region(cambc::team::enemy).positions();
    int total = 0, contested = 0;
    for (const auto& group : cambc::map::camp_groups()) {
        const auto tiles = group.spawn_tiles().positions();
        int ally_dist = 1 << 20, enemy_dist = 1 << 20;
        for (const auto a : ally_spawn)
            for (const auto tile : tiles)
                ally_dist = std::min(ally_dist, std::max(std::abs(a.x - tile.x), std::abs(a.y - tile.y)));
        for (const auto e : enemy_spawn)
            for (const auto tile : tiles)
                enemy_dist = std::min(enemy_dist, std::max(std::abs(e.x - tile.x), std::abs(e.y - tile.y)));
        const int gems = static_cast<int>(group.gem_value());
        total += gems;
        if (std::abs(ally_dist - enemy_dist) <= 6) contested += gems;
    }
    return total ? 100 * contested / total : 0;
}

static double counter_bonus(D pick, const std::vector<D>& enemies) {
    double bonus = 0.0;
    for (const auto enemy : enemies) {
        if (enemy == D::marcella) {
            // Marcella ignores stuns, so prefer direct damage and reliable basic attacks.
            if (pick == D::vesper_vale) bonus += 2.2;
            if (pick == D::vendetta) bonus += 1.8;
            if (pick == D::stellar) bonus += 1.2;
            if (pick == D::ryth || pick == D::prism) bonus -= 1.2;
            if (pick == D::ren || pick == D::tavi) bonus -= 0.8;
        } else if (enemy == D::cassi_and_issac) {
            // The linked pair loses both bodies when either one dies; burst and area damage help.
            if (pick == D::tavi) bonus += 2.4;
            if (pick == D::prism) bonus += 2.0;
            if (pick == D::stellar) bonus += 1.6;
            if (pick == D::ren) bonus += 0.8;
        } else if (enemy == D::willow || enemy == D::varg) {
            // Front-loaded damage pressures healing and shields before they can extend a fight.
            if (pick == D::prism || pick == D::tavi) bonus += 1.8;
            if (pick == D::stellar || pick == D::vendetta) bonus += 1.1;
        }
    }
    return bonus;
}

void play_draft() {
    static const int list = choose_list();
    static const int contested_share = map_contested_gem_share();
    const auto chosen = cambc::draft::current_draft(cambc::team::ally);
    const auto enemies = cambc::draft::current_draft(cambc::team::enemy);
    D best = LISTS[list][0];
    double best_score = -1e9;
    for (std::size_t rank = 0; rank < LISTS[list].size(); ++rank) {
        const D pick = LISTS[list][rank];
        if (std::find(chosen.begin(), chosen.end(), pick) != chosen.end()) continue;
        double score = static_cast<double>(LISTS[list].size() - rank) + counter_bonus(pick, enemies);
        if (contested_share >= 45) {
            // On maps where most rewards are exposed to both teams, increase the value of
            // champions that can hold or finish a fight.
            if (pick == D::vesper_vale || pick == D::vendetta || pick == D::varg || pick == D::ryth)
                score += 1.0;
            if (pick == D::willow) score -= 0.5;
        }
        if (score > best_score) {
            best_score = score;
            best = pick;
        }
    }
    cambc::draft::pick(best);
}
