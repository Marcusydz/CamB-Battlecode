#include <cambc.h>
#include <cstdio>
#include "camp_waiting_area.h"
int main() {
    // Draft has the whole map, including room for this synthetic camp geometry.
    const cambc::vec2 center{cambc::map::width()/2, cambc::map::height()/2};
    bool pass = true;
    for (int range : {3, 4, 5}) {
        const auto area = camp_waiting_area(center, range);
        cambc::bitboard danger;
        danger.draw_chebyshev(center, range);
        const unsigned expected = range == 3 ? 72 : range == 4 ? 40 : 48;
        const bool ok = area.count() == expected && (area & danger).is_empty();
        pass &= ok;
        std::printf("WAITING_PROBE range=%d safe_tiles=%u expected=%u %s\n", range,
                    static_cast<unsigned>(area.count()), expected, ok ? "PASS" : "FAIL");
    }
    std::printf("WAITING_RESULT %s\n", pass ? "PASS" : "FAIL");
    std::fflush(stdout);
    while (true) cambc::end_turn();
}
