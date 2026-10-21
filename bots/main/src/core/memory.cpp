// Shared memory (camp claims) and what we know about camp groups.
#include "bot.h"
#include "config.h"
#include "util.h"
#include <variant>

using cambc::vec2;
using cambc::team;
using cambc::bitboard;
using cambc::distance_field;
using CT = cambc::champion_type;
using Item = cambc::item_type;

// ------------------------------------------------------------------ shared memory
std::array<std::uint8_t, 8> Bot::claims() const {
    std::array<std::uint8_t, 8> data{};
    cambc::shared_memory.read(0, std::span{data});
    if (FOCUS_FIRE) data[FOCUS_ROUND_SLOT] = data[FOCUS_TAG_SLOT] = 0;  // not camp claims
    return data;
}

// FOCUS_FIRE: slot 5 = round & 255 when set, slot 6 = enemy handle % 255 + 1.
std::uint8_t Bot::focus_tag(int now) const {
    std::array<std::uint8_t, 8> data{};
    cambc::shared_memory.read(0, std::span{data});
    const int age = (now - data[FOCUS_ROUND_SLOT] + 256) % 256;
    return age <= 2 ? data[FOCUS_TAG_SLOT] : 0;
}

void Bot::set_focus(const cambc::champion& e, int now) {
    const std::uint8_t v[2] = {static_cast<std::uint8_t>(now & 255), static_cast<std::uint8_t>(handle_of(e) % 255 + 1)};
    cambc::shared_memory.write(FOCUS_ROUND_SLOT, std::span{v, 2});
}

void Bot::claim(int group) {
    // SQUAD_FIX: 255 = alive without a claim (readers check claims against groups.size()).
    const std::uint8_t none = SQUAD_FIX && cambc::self().is_alive() ? 255 : 0;
    const std::uint8_t value = group < 0 ? none : static_cast<std::uint8_t>(groups[group].idx + 1);
    auto data = claims();
    if (data[mem_slot] != value) cambc::shared_memory.write(mem_slot, std::span{&value, 1});
}

// Shared memory has only room for the claims: a slot with a claim counts as a squad member.
std::array<bool, 8> Bot::squad_alive(int) const {
    const auto c = claims();
    std::array<bool, 8> out{};
    for (int i = 0; i < 8; ++i) out[i] = c[i] != 0 || i == mem_slot;
    return out;
}

// ------------------------------------------------------------------ knowledge
void Bot::update_groups(int now, vec2 pos) {
    const auto visible = cambc::vision::visible_tiles();
    for (auto& g : groups) {
        const auto c = g.center;
        if (std::abs(c.x - pos.x) > 11 || std::abs(c.y - pos.y) > 11) continue;
        if ((g.mask & visible).is_empty()) continue;
        cambc::map::camp_group_state state;
        try {
            state = g.group.state();
        } catch (const cambc::error&) {
            continue;
        }
        g.seen_at = now;
        if (std::holds_alternative<cambc::map::camp_group_alive>(state)) {
            g.state = 1;
            g.available_at = 0;
        } else if (std::holds_alternative<cambc::map::camp_group_reward_available>(state)) {
            g.state = 2;
            g.available_at = 0;
        } else {
            g.state = 3;
            g.available_at = now + std::get<cambc::map::camp_group_respawning>(state).timer;
        }
    }
}
