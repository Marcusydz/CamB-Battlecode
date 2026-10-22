// Prints the map fingerprint (width, height, wall count, camp groups, total gems) during the draft.
#include <cambc.h>
#include <algorithm>
#include <cstdio>

int main() {
    bool done = false;
    while (true) {
        try {
            if (cambc::is_draft()) {
                if (!done) {
                    int gems = 0, groups = 0;
                    for (const auto& g : cambc::map::camp_groups()) {
                        gems += g.gem_value();
                        groups += 1;
                    }
                    std::printf("FP %d %d %d %d %d\n", cambc::map::width(), cambc::map::height(),
                                static_cast<int>(cambc::map::walls().count()), groups, gems);
                    std::fflush(stdout);
                    done = true;
                }
                const auto chosen = cambc::draft::current_draft(cambc::team::ally);
                for (int i = 0; i < 11; ++i) {
                    auto pick = static_cast<cambc::champion_draft>(i);
                    if (std::find(chosen.begin(), chosen.end(), pick) == chosen.end()) {
                        cambc::draft::pick(pick);
                        break;
                    }
                }
            }
        } catch (...) {
        }
        cambc::end_turn();
    }
}
