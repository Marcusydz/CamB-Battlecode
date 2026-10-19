// Draft: a preference list chosen per map (maps are recognised by size and wall count).
#include <cambc.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include "draft.h"

using D = cambc::champion_draft;
using List = std::array<D, 11>;

// Candidate lists. 0 is v16's order (Ren, Tavi, Ryth, Stellar, Marcella), chosen by local tests against
// itself. 1 is what teams rated 1700+ pick on almost every map (690 ladder games, October 2026).
static constexpr std::array<List, 7> LISTS{{
    {D::ren, D::tavi, D::ryth, D::stellar, D::marcella, D::varg, D::vendetta, D::vesper_vale, D::prism, D::cassi_and_issac, D::willow},
    {D::cassi_and_issac, D::stellar, D::varg, D::ren, D::ryth, D::tavi, D::prism, D::vendetta, D::marcella, D::vesper_vale, D::willow},
    {D::stellar, D::cassi_and_issac, D::ren, D::ryth, D::varg, D::tavi, D::prism, D::vendetta, D::marcella, D::vesper_vale, D::willow},
    {D::cassi_and_issac, D::stellar, D::ren, D::ryth, D::tavi, D::varg, D::prism, D::vendetta, D::marcella, D::vesper_vale, D::willow},
    {D::varg, D::cassi_and_issac, D::stellar, D::ren, D::ryth, D::tavi, D::prism, D::vendetta, D::marcella, D::vesper_vale, D::willow},
    {D::ren, D::stellar, D::ryth, D::tavi, D::cassi_and_issac, D::varg, D::marcella, D::prism, D::vendetta, D::vesper_vale, D::willow},
    {D::stellar, D::cassi_and_issac, D::varg, D::ryth, D::willow, D::ren, D::tavi, D::prism, D::vendetta, D::marcella, D::vesper_vale},
}};

constexpr int DRAFT = -1;  // >=0: use this list on every map (testing). -1: per-map table below.

struct MapDraft {
    int w, h, walls, list;
};
// Per-map choice, from local tests against v16 (see README).
static constexpr std::array<MapDraft, 16> PER_MAP{{
    {100, 100, 540, 2},  // ambush
    {41, 41, 84, 4},     // collision
    {50, 50, 40, 0},     // contest
    {71, 71, 44, 0},     // corners
    {41, 30, 36, 0},     // eight
    {30, 30, 0, 3},      // elkring
    {80, 80, 224, 4},    // expanse
    {30, 30, 24, 0},     // fragment
    {40, 40, 76, 5},     // interlock
    {90, 30, 0, 4},      // lanes
    {50, 50, 98, 1},     // monolith
    {31, 31, 0, 4},      // packs
    {50, 65, 0, 5},      // pilgrimage
    {50, 50, 214, 4},    // rows
    {30, 80, 80, 5},     // rushbait_v2
    {60, 50, 0, 0},      // stampede
}};
constexpr int DEFAULT_LIST = 4;  // maps not in the table (new or unseen ones)

static int choose_list() {
    if (DRAFT >= 0) return DRAFT;
    const int w = cambc::map::width(), h = cambc::map::height();
    const int walls = static_cast<int>(cambc::map::walls().count());
    for (const auto& m : PER_MAP)
        if (m.w == w && m.h == h && m.walls == walls) return m.list;
    return DEFAULT_LIST;
}

void play_draft() {
    static const int list = choose_list();
    const auto chosen = cambc::draft::current_draft(cambc::team::ally);
    for (auto pick : LISTS[list]) {
        if (std::find(chosen.begin(), chosen.end(), pick) == chosen.end()) {
            cambc::draft::pick(pick);
            return;
        }
    }
}
