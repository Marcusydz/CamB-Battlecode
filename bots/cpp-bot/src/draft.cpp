// Draft: a fixed preference list (same order as my-bot v16).
#include <cambc.h>
#include <algorithm>
#include <array>
#include "draft.h"

using D = cambc::champion_draft;

// Ordered by testing (see my-bot README and results/): 888-game slot test, 288-game ordering test and a
// round robin of the top four orders. Maps draft 3-5 champions, so the first five matter.
static constexpr std::array PREFERENCE{
    D::ren,
    D::tavi,
    D::ryth,
    D::stellar,
    D::marcella,
    D::varg,
    D::vendetta,
    D::vesper_vale,
    D::prism,
    D::cassi_and_issac,
    D::willow,
};

void play_draft() {
    const auto chosen = cambc::draft::current_draft(cambc::team::ally);
    for (auto pick : PREFERENCE) {
        if (std::find(chosen.begin(), chosen.end(), pick) == chosen.end()) {
            cambc::draft::pick(pick);
            return;
        }
    }
}
