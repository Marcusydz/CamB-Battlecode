#pragma once
#include <cambc.h>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// One camp group, with what we know about it (port of my-bot's GroupInfo).
struct GroupInfo {
    int idx;
    cambc::map::camp_group group;
    std::unordered_set<std::uint32_t> camp_handles;  // to match visible camps to this group
    cambc::bitboard mask;
    std::vector<cambc::vec2> tiles;
    int gems, gold;
    double health = 0, strength = 0;
    int count = 0;
    bool monolith = false;
    cambc::vec2 center;
    bool contested = false;
    int altar_dist = 0;
    int available_at = 0;   // round from which we believe it is alive
    int seen_at = 0;
    int avoid_until = 0;    // our own "gave up on it" marker; update_groups never resets it
    int state = 0;          // 0 unknown/alive, 1 alive (seen), 2 reward available, 3 respawning

    GroupInfo(int idx, const cambc::map::camp_group& group);
};

struct Nuke {
    double modifier;
    std::uint32_t slot;
    bool aoe, stuns;
};

class Bot {
public:
    void play_turn();

private:
    bool ready = false;
    std::uint64_t max_fuel = 0;
    int turns = 0;
    int overruns = 0;
    std::map<std::string, int> acts;
    std::string act = "?";
    bool waited_on_spawn = false;
    double last_hp = 0;
    bool last_alive = true;
    cambc::vec2 last_pos{};

    cambc::champion_type kind{};
    bool melee = false;
    bool kiter = false;  // ranged with a damage spell: fights attacking camps from outside their range
    cambc::bitboard static_block, spawn, altar_approach;
    int width = 0, height = 0;
    std::vector<cambc::vec2> altars;
    cambc::distance_field altar_field, home_field, enemy_field;
    std::vector<GroupInfo> groups;
    struct Seen { cambc::vec2 pos; int round; };
    std::unordered_map<std::uint32_t, Seen> seen;  // enemy handle -> last seen

    std::vector<Nuke> nukes;  // damage spells, best first
    std::optional<std::uint32_t> stun_slot, reset_slot, dash_slot, shield_slot, heal_slot;

    int mem_slot = 0;
    int team_size = 3;
    int kiter_rank = 9;
    bool has_mid = false;  // a monolith group worth 200+ gems  // casters drafted before us
    bool scarce = false;  // few gems outside monoliths
    int target = -1;  // index into groups, -1 = none
    int need = 1;
    int gather_turns = 0;
    int no_joint_until = 0;
    int stuck = 0;
    int close_turns = 0, close_tgt = -1;
    int next_plan = 0;
    int target_dist = 0;

    void setup();
    void turn();

    // shared memory
    std::array<std::uint8_t, 8> claims() const;
    void claim(int group);

    void update_groups(int now, cambc::vec2 pos);

    // movement
    cambc::bitboard blocked() const;
    int inner(cambc::vec2 p) const;
    bool step_towards(const cambc::bitboard& goals, bool allow_dash = true);

    // combat
    template <class T> bool try_spell(const T& target, bool want_stun = false);
    template <class T> bool try_attack(const T& target, const std::vector<T>& others = {});
    void fight(const cambc::camp& target, const std::vector<cambc::camp>& others);
    bool kite(const cambc::camp& target, const std::vector<cambc::camp>& others);

    void shop();
    void place_ward();
    bool tribute();
    bool place_lock(std::optional<cambc::vec2> only_altar = std::nullopt);

    // PvP
    double burst(const cambc::champion& me) const;
    bool try_heal(const cambc::champion& me);
    void self_buff(const cambc::champion& me);
    bool pvp(const cambc::champion& me, cambc::vec2 pos, int gems, double hp_frac);
    bool hit_in_range(const cambc::champion& me);
    bool outnumbered(const cambc::champion& me, cambc::vec2 pos, int now);
    bool join_fight(const cambc::champion& me, cambc::vec2 pos, int gems, double hp_frac);
    std::vector<cambc::vec2> threats(int now) const;
    bool enemy_near(cambc::vec2 p, int radius, int rounds, int now) const;
    bool retreat_and_recall();

    // camp choice
    int helpers_needed(const GroupInfo& g, double dps, double health) const;
    double camp_dps(const cambc::champion& me) const;
    bool joint_claim_elsewhere(int g, double dps, double health);
    int choose_group(int now, const cambc::distance_field& my_field);
    int min_dist(const cambc::distance_field& field, const GroupInfo& g) const;
};
