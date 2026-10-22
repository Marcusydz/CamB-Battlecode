// Draft (v1.5.0): a duelist core on most maps, v1.2.0's farming lineup on big open maps.
//
// Why (docs/versions/v1.5.0.md): on 41k ladder games, teams with Vesper Vale, Tavi, Marcella, Vendetta and
// Ryth win more than their rating predicts and teams with Stellar, Ren and Cassi & Issac less. Against
// gold-tier teams we died 30 times a game drafting Stellar/Varg/Willow; their duelists hunted us.
// Locally the core list beat v1.2.0 159/240 (+206 mean gem margin). It lost only on big, open maps where
// v1.2.0's farming lineup also wins on the ladder, so those maps keep it.
// Both teams may draft the same champion, so pick order cannot deny anything: the list order only
// decides which champions make the cut on 3- and 4-pick maps.
#include <cambc.h>
#include <algorithm>
#include <array>
#include <cstdio>
#include "draft.h"

using D = cambc::champion_draft;

namespace {

using List = std::array<D, 12>;

// Duelists first. On 3 picks: Vesper Vale, Tavi, Marcella; 4 adds Vendetta; 5 adds Ryth.
constexpr List CORE{D::vesper_vale, D::tavi, D::marcella, D::vendetta, D::ryth, D::heart_of_ruin,
                    D::prism, D::varg, D::willow, D::stellar, D::ren, D::cassi_and_issac};
// v1.2.0's lists for the maps below (its lists 16, 17, 19).
constexpr List FARM_HOR{D::heart_of_ruin, D::varg, D::stellar, D::ryth, D::willow, D::vesper_vale,
                        D::tavi, D::marcella, D::vendetta, D::prism, D::ren, D::cassi_and_issac};
constexpr List FARM_VESPER{D::vesper_vale, D::varg, D::stellar, D::ryth, D::willow, D::tavi,
                           D::marcella, D::vendetta, D::heart_of_ruin, D::prism, D::ren, D::cassi_and_issac};
constexpr List FARM_VENDETTA{D::vesper_vale, D::varg, D::vendetta, D::ryth, D::willow, D::tavi,
                             D::marcella, D::heart_of_ruin, D::stellar, D::prism, D::ren, D::cassi_and_issac};

struct MapList {
    int w, h, walls, groups;
    const List* list;
    const char* name;
};
// Maps are recognised by size, wall count and camp-group count (packs and gold_rush share
// 31x31 with no walls; elkring and gemfight share 30x30 with no walls).
constexpr std::array<MapList, 7> MAPS{{
    {100, 100, 636, 24, &FARM_HOR, "big_world"},       // core 2/6 -29; ladder 22-2 with this list
    {50, 41, 139, 31, &FARM_HOR, "boss_battle"},       // core 1/6; ladder 21-6
    {50, 65, 0, 23, &FARM_VESPER, "pilgrimage"},       // core 2/6 -312; ladder 14-6
    {50, 50, 214, 9, &FARM_VESPER, "rows"},            // core 0/6 -180; ladder 8-3
    {100, 100, 540, 12, &FARM_VESPER, "ambush"},       // core 2/6 -54; ladder 8-6
    {60, 60, 0, 46, &FARM_VENDETTA, "islands"},        // core 1/6 -242; ladder 13-4
    {61, 61, 210, 11, &FARM_VENDETTA, "concentrated_wealth"},  // core 2/6 -91; ladder 8-7
}};

const MapList* known_map() {
    const int w = cambc::map::width(), h = cambc::map::height();
    const int walls = static_cast<int>(cambc::map::walls().count());
    const int groups = static_cast<int>(cambc::map::camp_groups().size());
    for (const auto& m : MAPS)
        if (m.w == w && m.h == h && m.walls == walls && m.groups == groups) return &m;
    return nullptr;
}

}  // namespace

void play_draft() {
    static const MapList* map = known_map();
    const List& list = map ? *map->list : CORE;
    const auto chosen = cambc::draft::current_draft(cambc::team::ally);
    for (D pick : list) {
        if (std::find(chosen.begin(), chosen.end(), pick) != chosen.end()) continue;
        std::printf("draft %dx%d list=%s pick=%d\n", cambc::map::width(), cambc::map::height(),
                    map ? map->name : "core", static_cast<int>(pick));
        cambc::draft::pick(pick);
        return;
    }
}
