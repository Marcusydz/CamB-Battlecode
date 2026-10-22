// Small helpers shared by every module: camp tables, distances, safe vision queries.
#pragma once
#include <cambc.h>
#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

using cambc::vec2;
using cambc::team;
using cambc::bitboard;
using cambc::distance_field;
using CT = cambc::champion_type;
using Item = cambc::item_type;

// Base camp stats per camp type: (health, strength).
// Attack range per camp kind (week-2 patch: elk 3 -> 4, monolith 3 -> 5, golem 4).
inline int camp_range(cambc::camp_type t) {
    if (t == cambc::camp_type::elk) return 4;
    if (t == cambc::camp_type::monolith) return 5;
    if (t == cambc::camp_type::golem) return 4;
    return 3;
}

inline std::pair<double, double> camp_stats(cambc::camp_type t) {
    switch (t) {
    case cambc::camp_type::wolf: return {1350, 20};
    case cambc::camp_type::elk: return {2400, 60};
    case cambc::camp_type::gem_cluster: return {120, 0};   // week-2 patch: 500 -> 120
    case cambc::camp_type::gold_cluster: return {120, 0};
    case cambc::camp_type::monolith: return {20000, 200};
    case cambc::camp_type::golem: return {5000, 120};
    }
    return {1500, 50};
}

inline int chebyshev(vec2 a, vec2 b) {
    return std::max(std::abs(a.x - b.x), std::abs(a.y - b.y));
}

inline bitboard around(vec2 position, int radius) {
    bitboard board;
    board.draw_chebyshev(position, static_cast<float>(radius));
    return board;
}

inline std::uint32_t handle_of(const cambc::champion& c) {
    return static_cast<std::uint32_t>(static_cast<cambc::handle_champion_t>(c));
}

inline std::uint32_t handle_of(const cambc::camp& c) {
    return static_cast<std::uint32_t>(static_cast<cambc::handle_camp_t>(c));
}

// vision::camps(), tolerating the SDK's occasional stale-tile error.
inline std::vector<cambc::camp> visible_camps() {
    try {
        return cambc::vision::camps();
    } catch (const cambc::error&) {
        std::vector<cambc::camp> out;
        for (auto t : cambc::vision::camp_tiles().positions()) {
            try {
                out.push_back(cambc::vision::camp_at(t));
            } catch (const cambc::error&) {
            }
        }
        return out;
    }
}

inline std::vector<cambc::champion> visible_enemies() {
    try {
        return cambc::vision::champions(team::enemy);
    } catch (const cambc::error&) {
        return {};
    }
}

// vision::champions(team::ally), tolerating the same error (an ally unit leaving vision raised 98/97 and cost
// the whole turn).
inline std::vector<cambc::champion> visible_allies() {
    try {
        return cambc::vision::champions(team::ally);
    } catch (const cambc::error&) {
        return {};
    }
}

inline double camp_strength(const cambc::camp& c) {
    try {
        return c.strength();
    } catch (const cambc::error&) {
        return 0;
    }
}

inline double camp_health(const cambc::camp& c) {
    try {
        return c.health();
    } catch (const cambc::error&) {
        return 0;
    }
}

inline const char* kind_name(CT kind) {
    static std::string names[32];
    auto& s = names[static_cast<int>(kind)];
    if (s.empty()) s = cambc::to_string(kind);
    return s.c_str();
}

// Stable sort by a key read once per element. Never sort SDK unit objects with a comparator that
// calls their (virtual) accessors: stable_sort calls it on elements in its move buffer.
template <class T, class Key> std::vector<T> sorted_by(const std::vector<T>& items, Key key) {
    using K = decltype(key(items[0]));
    std::vector<std::pair<K, std::size_t>> keys;
    keys.reserve(items.size());
    for (std::size_t i = 0; i < items.size(); ++i) keys.emplace_back(key(items[i]), i);
    std::stable_sort(keys.begin(), keys.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    std::vector<T> out;
    out.reserve(items.size());
    for (const auto& k : keys) out.push_back(items[k.second]);
    return out;
}

// Leave a nonempty waiting band beyond the camp's attack range. A radius-five
// outer boundary disappears completely when a monolith also attacks at range five.
inline cambc::bitboard camp_waiting_area(cambc::vec2 center, int attack_range) {
    cambc::bitboard outer, danger;
    outer.draw_chebyshev(center, static_cast<float>(std::max(5, attack_range + 1)));
    danger.draw_chebyshev(center, static_cast<float>(attack_range));
    return outer & ~danger;
}
