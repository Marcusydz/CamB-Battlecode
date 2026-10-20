#pragma once
#include <cambc.h>
#include <algorithm>

// Leave a nonempty waiting band beyond the camp's attack range. A radius-five
// outer boundary disappears completely when a monolith also attacks at range five.
inline cambc::bitboard camp_waiting_area(cambc::vec2 center, int attack_range) {
    cambc::bitboard outer, danger;
    outer.draw_chebyshev(center, static_cast<float>(std::max(5, attack_range + 1)));
    danger.draw_chebyshev(center, static_cast<float>(attack_range));
    return outer & ~danger;
}
