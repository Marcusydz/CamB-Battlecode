// Runs inside the official game runtime. The runner exposes only Bot's private test seam.
#include "bot.h"
#include <algorithm>
#include <cstdio>

int main() {
    if (cambc::is_draft()) {
        while (true) {
            const auto chosen = cambc::draft::current_draft(cambc::team::ally);
            for (int i = 0; i < 11; ++i) {
                auto pick = static_cast<cambc::champion_draft>(i);
                if (std::find(chosen.begin(), chosen.end(), pick) == chosen.end()) {
                    cambc::draft::pick(pick);
                    break;
                }
            }
            cambc::end_turn();
        }
    }
    Bot bot;
    bot.setup();
    auto pos = cambc::self().position();
    cambc::bitboard root, visited;
    root.set(pos);
    cambc::distance_field limited;
    limited.compute(bot.static_block, root, visited, 0);
    // With a radius of zero, no camp away from spawn can be reached by this field.
    // The planner must return none, allowing its caller to run the full-map fallback.
    const int chosen = bot.choose_group(cambc::round_number(), limited);
    const int distance = chosen < 0 ? -1 : bot.min_dist(limited, bot.groups[chosen]);
    std::printf("PATHING_PROBE chosen=%d distance=%d expected=-1 %s\n", chosen, distance,
                chosen < 0 ? "PASS" : "FAIL");
    std::fflush(stdout);
    while (true) cambc::end_turn();
}
