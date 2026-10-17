"""Champion logic: farm the best-value camp, capture it, bank gems, shop, and stay alive.

Each champion runs its own copy. Champions coordinate through shared memory: each
writes the camp group it is heading for, and the others avoid that group.
"""
import cambc
from cambc import game_map

Team = cambc.Team
Item = cambc.ItemType
CT = cambc.ChampionType
CampType = cambc.CampType

# Base camp stats per CampType: (health, strength).
CAMP_STATS = {
    CampType.WOLF: (1350, 20),
    CampType.ELK: (2400, 60),
    CampType.GEM_CLUSTER: (500, 0),
    CampType.GOLD_CLUSTER: (500, 0),
    CampType.MONOLITH: (20000, 200),
}
CAPTURE_TIME = 5
UNREACHED = 10 ** 6
SHOP_RECALL_GOLD = 150 # recall to shop with an empty bag and this much gold
BUY_JUNGLE = True
GEM_CAP = 60
REPLAN_EVERY = 3
PLAN_RADIUS = 50  # max path length considered when choosing camps
LOG_EVERY = 25  # status line per champion every N rounds (0 disables)
# PvP tuning (sparring variants override these).
ENGAGE_RATIO = 0.8     # fight when our side's power is at least this fraction of theirs
FLEE_RATIO = 0.3       # carriers run when our power is below this fraction of theirs
HUNT_RADIUS = 10       # how far we chase a winnable fight
USE_LOCKS = True       # buy altar locks and place them on altars we pass
INTERRUPT = True       # stun / lock enemies who are tributing
CONTEST_PENALTY = 1.0  # score multiplier for contested camps when alone (1 = ignore)
CONTEST_MARGIN = 4     # tiles: a camp is "ours" if this much closer to our spawn
THREAT_RANGE = 5       # enemies this close stop a recall (attack range 4 + a step)
RALLY = False          # call allies to fights via shared memory (tested: hurts)
RALLY_RANGE = 14       # answer a rally this many tiles away (Chebyshev)
SOLO_CAPTURE = True    # only one champion holds a capture; others move on
FUEL_RESERVE = 700000  # skip optional work when less fuel than this remains
LOW_HP = 0.3           # retreat below this health fraction (+LOW_HP_GEMS when carrying 30+ gems)
LOW_HP_GEMS = 0.15
SPAWN_HEAL_TO = 0.95   # stay on spawn until this healthy
MONOLITH = False       # full-team monolith assault (tested: loses on monolith map)
GATHER_MAX = 12        # give up waiting for helpers after this many turns
JOIN_BONUS = 1.5       # score boost for joining a joint target an ally already claimed
SEED = 0               # >0 adds small deterministic noise to camp choice (testing only)
HEAL_MIN_USE = 0.6     # heal only if at least this fraction of the heal lands (or the ally is below half HP)
PRISM_GEM_CAP = 60     # Prism gains focus per gem carried (measured: +0.2 focus per gem, so only
PRISM_EARLY_BANK = 0   # +12 at 60 gems); 100/50 means it banks later than the others
MAX_WAIT = 0           # head for a respawning camp if it is back within this many rounds of arrival
                       # (0 = v14 behaviour; tested 25: champions stood beside respawning camps, lost)
SPELL_DPS = False      # count damage spells when judging whether a champion can clear a camp alone
FIGHT_FIX = True       # step before casting when out of range; ranged attack after stepping; Varg shields vs camps
CAPTURE_FIX = True     # one champion per reward; forget rewards not seen for a while; shop-recall only when it costs little
LOWHP_FIGHT = True     # low-HP champion already being hit: finish/hit what is in range before running
FAIL_MEMORY = False    # retreating or dying on the way to a far camp: skip that camp for 100 rounds
SOLO_CAMP_FULL = True  # single-camp groups deal full damage (no 60% "dies progressively" discount)
SCARCE_GEMS = 150      # fewer gems than this in camps the team can take: gem-scarce map, so the
SCARCE_FROM = 0        # whole team attacks monoliths / super groups together from this round
PAIR_UP = True         # weak champions join an ally's joint target instead of starting their own
ACT_LOG_EVERY = 200    # print how many turns each champion spent on each activity (0 disables)


def chebyshev(a, b):
    return max(abs(a.x - b.x), abs(a.y - b.y))


def around(position, radius):
    board = cambc.Bitboard()
    board.draw_chebyshev(position, radius)
    return board


def visible_camps():
    """vision.camps(), tolerating the SDK's occasional stale-tile error."""
    try:
        return cambc.vision.camps()
    except cambc.Error:
        out = []
        for t in cambc.vision.camp_tiles().positions():
            try:
                out.append(cambc.vision.camp_at(t))
            except cambc.Error:
                pass
        return out


def visible_enemies():
    try:
        return cambc.vision.champions(Team.ENEMY)
    except cambc.Error:
        return []


def camp_strength(c):
    try:
        return c.strength()
    except cambc.Error:
        return 0


def camp_health(c):
    try:
        return c.health()
    except cambc.Error:
        return 0


class GroupInfo:
    __slots__ = ("idx", "handle", "group", "mask", "tiles", "gems", "gold", "health", "center", "contested",
                 "strength", "count", "monolith", "altar_dist", "available_at", "state", "avoid_until", "seen_at")

    def __init__(self, idx, group):
        self.idx = idx
        self.handle = int(group)
        self.group = group
        self.mask = group.spawn_tiles()
        self.tiles = self.mask.positions()
        self.gems = group.gem_value()
        self.gold = group.gold_value()
        self.health = 0
        self.strength = 0
        self.monolith = False
        self.count = 0
        for camp in group.camps():
            hp, st = CAMP_STATS.get(camp.kind(), (1500, 50))
            self.health += hp
            self.strength += st
            self.count += 1
            if camp.kind() == CampType.MONOLITH:
                self.monolith = True
        self.center = self.tiles[0]
        self.contested = False
        self.altar_dist = 0
        self.available_at = 0   # round from which we believe it is alive
        self.seen_at = 0
        self.avoid_until = 0    # our own "gave up on it" marker; update_groups never resets it
        self.state = 0          # 0 unknown/alive, 1 alive (seen), 2 reward available, 3 respawning


class Bot:
    def __init__(self):
        self.ready = False
        self.max_fuel = 0
        self.turns = 0
        self.overruns = 0
        self.acts = {}
        self.waited_on_spawn = False
        self.last_hp = 0
        self.last_alive = True

    # ------------------------------------------------------------------ setup
    def setup(self):
        me = cambc.me()
        self.kind = me.champion_name()
        self.melee = me.attack_range() <= 1
        self.static_block = (game_map.walls() | game_map.holes() | game_map.altars()
                             | game_map.spawn_region(Team.ENEMY))
        self.spawn = game_map.spawn_region(Team.ALLY)
        self.width, self.height = game_map.width(), game_map.height()
        self.altars = game_map.altars().positions()
        approach = cambc.Bitboard()
        for altar in self.altars:
            approach |= around(altar, 1)
        self.altar_approach = approach & ~self.static_block

        # Distance from every tile to the nearest altar-approach tile (static terrain only).
        self.altar_field = cambc.DistanceField()
        self.altar_field.compute(self.static_block & ~self.altar_approach, self.altar_approach, cambc.Bitboard())

        # Distances to each spawn, for retreat paths and for judging contested camps.
        self.home_field = cambc.DistanceField()
        self.home_field.compute(self.static_block & ~self.spawn, self.spawn, cambc.Bitboard())
        enemy_spawn = game_map.spawn_region(Team.ENEMY)
        self.enemy_field = cambc.DistanceField()
        self.enemy_field.compute((self.static_block & ~enemy_spawn), enemy_spawn, cambc.Bitboard())
        self.groups = [GroupInfo(i, g) for i, g in enumerate(game_map.camp_groups())]
        self.by_handle = {g.handle: g for g in self.groups}
        for g in self.groups:
            g.altar_dist = min((self.altar_field.get(t) for t in g.tiles), default=UNREACHED)
            mine = min(self.home_field.get(t) for t in g.tiles)
            theirs = min(self.enemy_field.get(t) for t in g.tiles)
            g.contested = mine + CONTEST_MARGIN > theirs
        self.seen = {}  # enemy handle -> (position, round last seen)

        # Ability roles per slot.
        self.nukes = []         # (modifier, slot, is_ground_aoe, stuns) damage spells, best first
        self.stun_slot = None   # any spell that stuns a unit
        self.reset_slot = None
        self.dash_slot = None
        self.shield_slot = None
        self.heal_slot = None
        for ability in me.abilities():
            slot = ability.slot()
            info = ability.info()
            if ability.is_attack_reset():
                self.reset_slot = slot
            elif ability.is_dash():
                self.dash_slot = slot
            elif ability.is_shield():
                self.shield_slot = slot
            elif ability.is_heal():
                self.heal_slot = slot
            else:
                modifier = float(getattr(info, "modifier", 0.0) or 0.0)
                aoe = ability.is_aoe_damage()
                stuns = ability.is_cc() or (aoe and int(getattr(info, "duration", 0) or 0) > 0)
                if stuns:
                    self.stun_slot = slot
                if modifier > 0:
                    self.nukes.append((modifier, slot, aoe, stuns))
        self.nukes.sort(reverse=True)

        # Shared-memory slot: our position in the team's draft order.
        self.mem_slot = int(me) % 8
        try:
            picks = [int(p) for p in cambc.draft.current_draft(Team.ALLY)]
            draft_id = {CT.RYTH: 0, CT.VESPER_VALE: 1, CT.VENDETTA: 2, CT.CASSI: 3, CT.ISSAC: 3,
                        CT.STELLAR: 4, CT.PRISM: 5, CT.MARCELLA: 6, CT.WILLOW: 7, CT.REN: 8,
                        CT.VARG: 9, CT.TAVI: 10}[self.kind]
            self.mem_slot = 7 if self.kind == CT.ISSAC else picks.index(draft_id)
            self.team_size = len(picks) + (1 if 3 in picks else 0)
        except Exception:
            self.team_size = 3
        dps0, hp0 = max(me.strength(), 20.0) + 15.0, me.max_health()
        easy = sum(g.gems for g in self.groups if self.helpers_needed_raw(g, dps0, hp0) > 0)
        self.scarce = SCARCE_GEMS > 0 and easy < SCARCE_GEMS and self.team_size >= 3
        self.target = None
        self.need = 1
        self.gather_turns = 0
        self.no_joint_until = 0
        self.stuck = 0
        self.next_plan = 0
        self.target_dist = UNREACHED
        self.ready = True
        if LOG_EVERY and self.mem_slot == 0:
            print("map %dx%d groups:" % (game_map.width(), game_map.height()), flush=True)
            for g in self.groups:
                print("  g%d n=%d hp=%d str=%d gems=%d gold=%d tiles=%s altar=%d" % (
                    g.idx, g.count, g.health, g.strength, g.gems, g.gold, g.tiles[:2], g.altar_dist), flush=True)

    # ------------------------------------------------------------------ shared memory
    def claims(self):
        data = bytearray(8)
        cambc.shared_memory.read(0, data)
        return data

    def claim(self, group):
        value = 0 if group is None else group.idx + 1
        data = self.claims()
        if data[self.mem_slot] != value:
            cambc.shared_memory.write(self.mem_slot, bytes([value]))

    def rally_point(self):
        data = self.claims()
        if data[5] == 0:
            return None
        return cambc.Vec2(data[5] - 1, data[6])

    def set_rally(self, p):
        if p is None:
            cambc.shared_memory.write(5, bytes([0, 0]))
        else:
            cambc.shared_memory.write(5, bytes([p.x + 1, p.y]))

    # ------------------------------------------------------------------ knowledge
    def update_groups(self, now, pos):
        visible = cambc.vision.visible_tiles()
        px, py = pos.x, pos.y
        for g in self.groups:
            c = g.center
            if abs(c.x - px) > 11 or abs(c.y - py) > 11:
                continue
            if (g.mask & visible).is_empty():
                continue
            try:
                state = g.group.state()
            except cambc.Error:
                continue
            g.seen_at = now
            if isinstance(state, game_map.CampGroupAlive):
                g.state, g.available_at = 1, 0
            elif isinstance(state, game_map.CampGroupRewardAvailable):
                g.state, g.available_at = 2, 0
            else:
                g.state = 3
                g.available_at = now + int(state.timer)

    # ------------------------------------------------------------------ movement
    def blocked(self):
        b = self.static_block | cambc.vision.champion_tiles() | cambc.vision.camp_tiles() | cambc.vision.teleport_tiles()
        b.set(cambc.me().position(), False)
        return b

    def inner(self, p):
        """Distance from the map border: equal-length paths prefer the interior over the edge."""
        return min(p.x, p.y, self.width - 1 - p.x, self.height - 1 - p.y)

    def step_towards(self, goals, allow_dash=True):
        """Walk (and dash when it gains two tiles) towards the nearest goal tile."""
        me = cambc.me()
        origin = me.position()
        if goals.is_set(origin):
            return False
        blocked = self.blocked()
        free_goals = goals & ~blocked
        if free_goals.is_empty():
            return False
        field = cambc.DistanceField()
        # Search from the goals only until our neighbourhood is reached (much cheaper on big maps).
        field.compute_to(blocked & ~free_goals, free_goals, around(origin, 2) & ~blocked, cambc.Bitboard())
        here = field.get(origin)
        if here <= 0 or here >= UNREACHED:
            return False
        moved = False
        # Dash first: it is compatible with walking, so dash + walk covers three tiles.
        if allow_dash and here >= 3 and self.dash_slot is not None and me.can_cast():
            dash = me.abilities()[self.dash_slot]
            if not dash.is_on_cooldown():
                options = around(origin, 2) & ~blocked
                best = min(options.positions(), key=lambda p: (field.get(p), -self.inner(p)), default=None)
                if best is not None and field.get(best) <= here - 2:
                    try:
                        if dash.use(best):
                            moved = True
                            origin = best
                            here = field.get(best)
                    except cambc.Error:
                        pass
        if me.can_move() and here > 0:
            options = around(origin, 1) & ~blocked
            best = min(options.positions(), key=lambda p: (field.get(p), -self.inner(p)), default=None)
            if best is not None and field.get(best) < here:
                try:
                    if me.move_absolute(best):
                        moved = True
                except cambc.Error:
                    pass
        return moved

    # ------------------------------------------------------------------ combat
    def try_spell(self, target, want_stun=False):
        """Cast the strongest ready damage spell (or a stun if asked) on a visible unit in range."""
        me = cambc.me()
        if not me.can_cast() or not target.is_visible():
            return False
        pos = me.position()
        tpos = target.position()
        if chebyshev(pos, tpos) > 4:
            return False
        abilities = me.abilities()
        order = [n[1] for n in self.nukes]
        if want_stun and self.stun_slot is not None:
            order = [self.stun_slot] + [o for o in order if o != self.stun_slot]
        for slot in order:
            ability = abilities[slot]
            if ability.is_on_cooldown():
                continue
            try:
                if ability.can_target_ground() and not ability.can_target_enemies():
                    ok = ability.use(tpos)
                else:
                    ok = ability.use(target)
                if ok:
                    return True
            except cambc.Error:
                try:
                    if ability.use(tpos):
                        return True
                except cambc.Error:
                    pass
        return False

    def try_attack(self, target, others=()):
        me = cambc.me()
        if (not me.can_attack() or not target.is_visible() or target.health() <= 0
                or not me.attack_tiles().is_set(target.position())):
            return False
        try:
            me.attack(target)
        except cambc.Error:
            return False
        # Attack reset: a second basic attack this turn.
        if self.reset_slot is not None and me.can_cast():
            reset = me.abilities()[self.reset_slot]
            if not reset.is_on_cooldown():
                try:
                    nxt = target
                    if not (target.is_visible() and target.health() > 0):
                        # First hit killed it: spend the reset on something else in range.
                        tiles = me.attack_tiles()
                        alive = [c for c in others if c.is_visible() and c.health() > 0
                                 and tiles.is_set(c.position())]
                        nxt = min(alive, key=lambda c: c.health()) if alive else None
                    if nxt is not None:
                        reset.use()
                        if me.can_attack():
                            me.attack(nxt)
                except cambc.Error:
                    pass
        return True

    def fight(self, target, others=()):
        """In range: shield, spell, attack. Out of range: step first (damage spells are
        exclusive with movement, so casting first used to cost the step), then attack
        whatever is now in range, and cast only if we could not move."""
        me = cambc.me()
        if not target.is_visible():
            return
        if me.attack_tiles().is_set(target.position()):
            if FIGHT_FIX and camp_strength(target) > 0 and me.health() < 0.9 * me.max_health():
                self.self_buff(me)   # Varg's shield used to be PvP-only
            self.try_spell(target)
            self.try_attack(target, others)
            return
        if not FIGHT_FIX:
            self.try_spell(target)
            if self.step_towards(around(target.position(), me.attack_range()), allow_dash=False) and self.melee:
                self.try_attack(target)
            return
        moved = self.step_towards(around(target.position(), me.attack_range()), allow_dash=False)
        for c in (target,) + tuple(others):
            if self.try_attack(c, others):
                return
        if not moved:
            self.try_spell(target)

    # ------------------------------------------------------------------ shopping
    def shop(self):
        me = cambc.me()
        items = me.item_count()
        # Build order: Monster emblem (+10% camp damage), Jungle emblem of health
        # (-30% camp damage taken), then Power strength shards forever.
        if items[Item.MONSTER_EMBLEM_OF_STRENGTH] == 0:
            primary = Item.MONSTER_SHARD_OF_STRENGTH
        elif items[Item.JUNGLE_EMBLEM_OF_HEALTH] == 0 and BUY_JUNGLE:
            primary = Item.JUNGLE_SHARD_OF_HEALTH
        else:
            primary = Item.POWER_SHARD_OF_STRENGTH
        if USE_LOCKS and (not me.is_alive() or self.spawn.is_set(me.position())):
            lock = cambc.ConsumableType.ALTAR_LOCK
            try:
                if me.gold_count() >= 10 and me.can_buy(lock):
                    me.buy(lock)
            except cambc.Error:
                pass
        cost = primary.cost()
        count = me.gold_count() // cost
        if count > 0:
            try:
                if me.can_buy(primary, count):
                    me.buy(primary, count)
            except cambc.Error:
                pass

    # ------------------------------------------------------------------ decisions
    def tribute(self):
        me = cambc.me()
        pos = me.position()
        adjacent = [a for a in self.altars if chebyshev(pos, a) <= 1]
        enemy_locks = cambc.vision.altar_lock_tiles(Team.ENEMY)
        adjacent.sort(key=lambda a: enemy_locks.is_set(a))
        for altar in adjacent:
            try:
                me.submit_gems(altar)  # against a lock this becomes an attack on the lock
                return True
            except cambc.Error:
                continue
        return self.step_towards(self.altar_approach)

    def place_lock(self, only_altar=None):
        """Place our altar lock on an altar within range (cancels enemy tributes there)."""
        me = cambc.me()
        try:
            slot = me.consumables()[cambc.ConsumableType.ALTAR_LOCK]
            if slot.count() <= 0:
                return False
        except Exception:
            return False
        pos = me.position()
        locked = cambc.vision.altar_lock_tiles()
        for altar in self.altars:
            if only_altar is not None and altar != only_altar:
                continue
            if chebyshev(pos, altar) <= 2 and not locked.is_set(altar):
                try:
                    if slot.use(altar):
                        return True
                except cambc.Error:
                    continue
        return False

    # ------------------------------------------------------------------ PvP
    def burst(self, me):
        """Damage we can deal this turn: basic attack (twice with a reset) plus best ready nuke."""
        dmg = me.strength()
        abilities = me.abilities()
        if self.reset_slot is not None and not abilities[self.reset_slot].is_on_cooldown():
            dmg += me.strength()
        for modifier, slot, aoe, stuns in self.nukes:
            if not abilities[slot].is_on_cooldown():
                dmg = max(dmg, me.strength() + modifier * me.focus())
                break
        return dmg

    def try_heal(self, me):
        """Cast the heal on the ally (or self) it helps most. Returns True if cast."""
        if self.heal_slot is None or not me.can_cast():
            return False
        ability = me.abilities()[self.heal_slot]
        if ability.is_on_cooldown():
            return False
        try:
            full = ability.heal_amount()
        except cambc.Error:
            return False
        if full <= 0:
            return False
        info = ability.info()
        reach = float(getattr(info, "range", 0) or 0)
        pos = me.position()
        candidates = []
        if ability.can_target_self():
            candidates.append(me)
        if ability.can_target_allies():
            candidates += [a for a in cambc.vision.champions(Team.ALLY)
                           if a != me and a.is_alive() and a.is_visible()]
        best, best_value = None, 0.0
        for a in candidates:
            apos = a.position()
            if chebyshev(apos, pos) > reach or self.spawn.is_set(apos):
                continue  # out of range, or on spawn (which heals anyway)
            missing = a.max_health() - a.health()
            if missing <= 0:
                continue
            low = a.health() < 0.5 * a.max_health()
            if min(missing, full) < HEAL_MIN_USE * full and not low:
                continue
            value = min(missing, full) * (2.0 if low else 1.0)
            if value > best_value:
                best, best_value = a, value
        if best is None:
            return False
        try:
            if ability.can_target_ground() and not (ability.can_target_allies() or ability.can_target_self()):
                return bool(ability.use(best.position()))
            return bool(ability.use(best))
        except cambc.Error:
            return False

    def self_buff(self, me):
        if self.shield_slot is not None and me.can_cast():
            ability = me.abilities()[self.shield_slot]
            if not ability.is_on_cooldown() and me.health() < me.max_health() * 0.95:
                try:
                    ability.use()
                except cambc.Error:
                    pass

    @staticmethod
    def power(units):
        hp = sum(max(u.health(), 1.0) for u in units)
        dps = sum(u.strength() + 0.25 * u.focus() for u in units)
        return hp * dps

    def pvp(self, me, pos, gems, hp_frac):
        """Returns True if the turn was spent on PvP."""
        enemies = [e for e in visible_enemies() if e.is_alive()]
        now = cambc.round_number()
        for e in enemies:
            self.seen[int(e)] = (e.position(), now)
        if not enemies:
            return False
        # 1. Interrupt enemy tributes: lock the altar or stun the channeler, then hit them.
        if INTERRUPT:
            for e in enemies:
                try:
                    tributing = e.is_submitting_gems()
                except cambc.Error:
                    tributing = False
                if not tributing:
                    continue
                epos = e.position()
                near_altar = min(self.altars, key=lambda a: chebyshev(a, epos))
                acted = False
                if chebyshev(pos, near_altar) <= 2 and self.place_lock(near_altar):
                    acted = True
                if self.try_spell(e, want_stun=True):
                    acted = True
                if self.try_attack(e):
                    acted = True
                if acted:
                    self.act = "interrupt"
                    return True
        near = [e for e in enemies if chebyshev(e.position(), pos) <= 6]
        if not near:
            return False
        allies = [me] + [a for a in cambc.vision.champions(Team.ALLY)
                         if a.is_alive() and a != me and chebyshev(a.position(), pos) <= 5]
        ratio = self.power(allies) / max(self.power(near), 1.0)
        # 2. Winning fight: focus fire (same rule on every ally), finish kills first.
        if ratio >= ENGAGE_RATIO and hp_frac > 0.35:
            burst = self.burst(me)
            reach = [e for e in near if chebyshev(e.position(), pos) <= HUNT_RADIUS]
            if reach:
                def key(e):
                    killable = e.health() <= burst and chebyshev(e.position(), pos) <= 4
                    return (not killable, e.health() / (1.0 + 0.03 * e.gem_count()), int(e))
                target = min(reach, key=key)
                self.self_buff(me)
                # Stun the most dangerous enemy in reach if it is not the kill target.
                dangerous = max(near, key=lambda e: e.strength() + 0.5 * e.focus())
                if (dangerous != target and self.stun_slot is not None
                        and chebyshev(dangerous.position(), pos) <= 4 and target.health() > burst):
                    self.try_spell(dangerous, want_stun=True)
                self.try_spell(target, want_stun=False)
                if not self.try_attack(target) and target.is_visible():
                    # (a spell may just have killed it: a dead target has no position)
                    self.step_towards(around(target.position(), me.attack_range()))
                    self.try_attack(target)
                if RALLY and target.is_visible():
                    self.set_rally(target.position())
                self.act = "engage"
                return True
        # 3. Losing fight while carrying gems (or hurt): get out.
        if ratio < FLEE_RATIO and (gems >= 10 or hp_frac < 0.6):
            if gems > 0 and self.altar_field.get(pos) <= 2:
                self.tribute()
                self.act = "flee-tribute"
            else:
                if FAIL_MEMORY:
                    self.mark_failed(cambc.round_number())
                self.retreat_and_recall()
                self.act = "flee"
            return True
        # 4. Otherwise trade hits with anything already in range.
        if hp_frac > 0.45:
            for e in sorted(near, key=lambda e: e.health()):
                if me.attack_tiles().is_set(e.position()):
                    self.try_spell(e)
                    if self.try_attack(e):
                        self.act = "pvp"
                        return True
        return False

    def hit_in_range(self, me):
        """Spell and attack the weakest enemy already in attack range, without moving."""
        tiles = me.attack_tiles()
        targets = [e for e in visible_enemies() if e.is_alive() and tiles.is_set(e.position())]
        if not targets:
            return False
        target = min(targets, key=lambda e: e.health())
        self.try_spell(target)
        return self.try_attack(target)

    def mark_failed(self, now):
        """Retreat or death on the way to a far target: avoid it for a while."""
        t = self.target
        if t is not None and self.target_dist > 9:
            t.avoid_until = now + 100

    def threats(self, now):
        """Recently seen enemy positions (vision is per champion, so remember a few rounds)."""
        return [p for p, r in self.seen.values() if now - r <= 3]

    def retreat_and_recall(self):
        """Recall if no enemy can reach us; otherwise walk home, away from enemies."""
        me = cambc.me()
        pos = me.position()
        now = cambc.round_number()
        threats = [t for t in self.threats(now) if chebyshev(pos, t) <= THREAT_RANGE]
        camp_threats = [c.position() for c in visible_camps()
                        if c.is_visible() and camp_strength(c) > 0 and camp_health(c) < c.max_health()
                        and chebyshev(pos, c.position()) <= 3]
        if not threats and not camp_threats:
            try:
                me.recall()
                return True
            except cambc.Error:
                return False
        danger = threats + camp_threats
        blocked = self.blocked()
        here = self.home_field.get(pos)

        def score(p):
            d = min(chebyshev(p, t) for t in danger)
            return (min(d, 6), -self.home_field.get(p))
        if self.dash_slot is not None and me.can_cast():
            dash = me.abilities()[self.dash_slot]
            if not dash.is_on_cooldown():
                options = [p for p in (around(pos, 2) & ~blocked).positions() if self.home_field.get(p) <= here]
                best = max(options, key=score, default=None)
                if best is not None and best != pos:
                    try:
                        if dash.use(best):
                            pos = best
                    except cambc.Error:
                        pass
        if me.can_move():
            options = (around(pos, 1) & ~blocked).positions()
            best = max(options, key=score, default=None)
            if best is not None and best != pos:
                try:
                    me.move_absolute(best)
                except cambc.Error:
                    pass
        return True

    def helpers_needed(self, g, dps, health):
        """Smallest team size that clears g without anyone dying (rough), or 0 if none."""
        n = self.helpers_needed_raw(g, dps, health)
        if n == 0 and self.scarce and g.gems > 0 and cambc.round_number() >= SCARCE_FROM:
            # Gem-scarce map (e.g. all gems in monoliths): the whole team goes, or nobody scores.
            return self.team_size
        return n

    def helpers_needed_raw(self, g, dps, health):
        if g.monolith:
            return self.team_size if (MONOLITH and self.team_size >= 4) else 0
        if g.strength <= 0:
            return 1
        for n in range(1, self.team_size + 1):
            clear = g.health / (dps * n)
            # Monsters die progressively, so roughly 60% of full damage lands; spread over n.
            factor = 1.0 if (SOLO_CAMP_FULL and g.count == 1) else 0.6
            if g.strength * clear * factor <= 0.75 * health * n:
                return n
        return 0

    def camp_dps(self, me):
        """Rough damage per turn against camps: basic attack, attack reset and damage spells
        (each spell every 8-turn cooldown). Never below the old flat estimate."""
        s = me.strength()
        dps = s + (s / 8.0 if self.reset_slot is not None else 0.0)
        if SPELL_DPS:
            dps += sum(mod for mod, _, _, _ in self.nukes) * me.focus() / 8.0
        return max(dps, max(s, 20.0) + 15.0)

    def joint_claim_elsewhere(self, g, dps, health):
        """A higher-priority ally (lower memory slot) has claimed a different camp that also
        needs helpers for us: go there instead of starting our own joint target. Without this,
        two weak champions each wait at their own camp for the other (seen: 50 turns)."""
        if not PAIR_UP:
            return False
        claims = self.claims()
        for i in range(self.mem_slot):
            c = claims[i]
            if c and c - 1 != (-1 if g is None else g.idx) and c - 1 < len(self.groups):
                other = self.groups[c - 1]
                if not other.monolith and self.helpers_needed(other, dps, health) > 1:
                    return True
        return False

    def choose_group(self, now, my_field):
        me = cambc.me()
        claims = self.claims()
        claim_count = {}
        for i in range(8):
            if i != self.mem_slot and claims[i]:
                claim_count[claims[i] - 1] = claim_count.get(claims[i] - 1, 0) + 1
        dps = self.camp_dps(me)
        health = me.max_health()
        no_new_joint = now < self.no_joint_until or self.joint_claim_elsewhere(None, dps, health)
        best, best_score, best_need = None, 0.0, 1
        for g in self.groups:
            dist = min((my_field.get(t) for t in g.tiles), default=UNREACHED)
            if dist >= UNREACHED:
                continue
            eta = dist
            if now < g.avoid_until:
                continue
            wait = max(0, g.available_at - now - eta)
            if wait > MAX_WAIT:
                continue
            others = claim_count.get(g.idx, 0)
            if CAPTURE_FIX and g.state == 2 and dist > 3 and (others > 0 or now - g.seen_at > 25):
                continue  # an ally is taking that reward, or we saw it so long ago it is likely gone
            if g.state == 2:
                need, clear = 1, 0
            elif g.monolith:
                need = self.helpers_needed(g, dps, health)
                if need == 0:
                    continue
                clear = g.health / (dps * need)
            else:
                need = self.helpers_needed(g, dps * (min(g.count, 2) if self.kind == CT.VARG else 1), health)
                if need == 0:
                    continue
                if need > 1 and others == 0 and no_new_joint:
                    continue  # join the ally already waiting for help, or recently gave up waiting
                clear = g.health / (dps * max(need, others + 1))
            value = (g.gems + 0.15 * g.gold) / max(need, others + 1)
            cost = eta + wait + clear + CAPTURE_TIME + 0.5 * g.altar_dist + 1
            if need > 1:
                cost += 4 * (need - 1 - min(others, need - 1))  # time to gather helpers
                if others > 0:
                    score_bonus = JOIN_BONUS
                else:
                    score_bonus = 1.0
            else:
                score_bonus = 1.0
            score = value / cost * score_bonus
            if others and need <= 1 and g.state != 2:
                score *= 0.25   # someone else is on it; spread out
            if g.contested and others == 0:
                score *= CONTEST_PENALTY
            if self.target is g:
                score *= 1.2    # hysteresis
            if SEED:
                h = (SEED * 1000003 + g.idx * 7919 + self.mem_slot * 104729 + now // 40) * 2654435761 % 4294967296
                score *= 0.9 + 0.2 * (h / 4294967296.0)
            if score > best_score:
                best, best_score, best_need = g, score, need
        self.need = best_need
        return best

    # ------------------------------------------------------------------ turn
    def play_turn(self):
        self.act = "?"
        start = cambc.fuel_remaining()
        r0 = cambc.round_number()
        self.turn()
        used = start - cambc.fuel_remaining()
        if cambc.round_number() != r0:
            self.overruns += 1
            if LOG_EVERY:
                print("OVERRUN r%d act=%s" % (r0, self.act), flush=True)
        if used > self.max_fuel and self.turns > 1:
            self.max_fuel = used
        if used > 2400000 and self.turns > 1 and LOG_EVERY:
            print("heavy turn r%d used=%d act=%s" % (cambc.round_number(), used, self.act), flush=True)
        self.turns += 1
        self.acts[self.act] = self.acts.get(self.act, 0) + 1
        now = cambc.round_number()
        if ACT_LOG_EVERY and now % ACT_LOG_EVERY == 0:
            tally = sorted(self.acts.items(), key=lambda kv: -kv[1])
            print("acts r%d %s: %s" % (now, CT.to_string(self.kind) if hasattr(CT, "to_string") else self.kind,
                                       " ".join("%s=%d" % kv for kv in tally)), flush=True)
        if LOG_EVERY and now % LOG_EVERY == 0:
            me = cambc.me()
            t = self.target
            print("r%d %s %s hp%d/%d gem%d gold%d tgt=%s st=%s act=%s fuelmax=%d" % (
                now, CT.to_string(self.kind) if hasattr(CT, "to_string") else self.kind,
                me.position() if me.is_alive() else "dead", me.health(), me.max_health(),
                me.gem_count(), me.gold_count(), None if t is None else t.idx,
                None if t is None else t.state, self.act, self.max_fuel), flush=True)

    def turn(self):
        me = cambc.me()
        if not self.ready:
            self.setup()
        now = cambc.round_number()
        waited_on_spawn = self.waited_on_spawn
        self.waited_on_spawn = False

        if not me.is_alive():
            if FAIL_MEMORY and self.last_alive:
                self.mark_failed(now)
            self.last_alive = False
            self.claim(None)
            self.shop()
            self.act = "dead"
            return
        if me.is_recalling() or me.is_submitting_gems() or me.is_teleporting():
            self.act = "channel"
            return

        pos = me.position()
        at_spawn = self.spawn.is_set(pos)
        if at_spawn or self.kind == CT.TAVI:
            self.shop()
        self.update_groups(now, pos)

        if self.heal_slot is not None:
            self.try_heal(me)
        gems = me.gem_count()
        hp_frac = me.health() / me.max_health()

        # On spawn and hurt: stay put for one turn. Spawn heals fully at the END of the turn, so
        # leaving in the same turn (e.g. right after a recall lands) wastes the whole recall.
        # Only one turn: still hurt after that means an enemy outside the spawn is hitting us
        # every round, and standing there just lets it (seen: 46 turns stuck on elkring).
        if at_spawn and hp_frac < SPAWN_HEAL_TO and not waited_on_spawn:
            self.waited_on_spawn = True
            self.claim(None)
            if gems > 0 and self.altar_field.get(pos) <= 1:
                self.tribute()
            else:
                self.hit_in_range(me)
            self.act = "heal"
            return

        last_hp, self.last_hp = self.last_hp, me.health()
        self.last_alive = True
        # Survival.
        if not at_spawn and hp_frac < (LOW_HP + (LOW_HP_GEMS if gems >= 30 else 0.0)):
            if FAIL_MEMORY:
                self.mark_failed(now)
            if LOWHP_FIGHT and me.health() < last_hp:
                # Being hit right now: running rarely escapes a chaser, so first take
                # whatever is in range (a kill resets the chaser problem entirely).
                enemies = [e for e in visible_enemies() if e.is_alive()
                           and me.attack_tiles().is_set(e.position())]
                if enemies:
                    self.self_buff(me)
                    if self.hit_in_range(me):
                        self.act = "lowhp-fight"
                        return
            self.claim(None)
            self.retreat_and_recall()
            self.act = "lowhp"
            return

        # Bank immediately if that wins the game, or at the very end.
        if gems > 0 and (cambc.global_submitted_gems(Team.ALLY) + gems >= cambc.constants.WINNING_GEMS
                         or now > 1940):
            self.tribute()
            return

        if self.pvp(me, pos, gems, hp_frac):
            return

        # Answer a rally: help a nearby ally who is fighting.
        if RALLY:
            rp = self.rally_point()
            if rp is not None:
                d = chebyshev(pos, rp)
                if d <= 2:
                    self.set_rally(None)  # arrived and no fight here any more
                elif d <= RALLY_RANGE and hp_frac > 0.6 and gems < 30:
                    if self.step_towards(around(rp, 2)):
                        self.act = "rally"
                        return

        # Lock any altar we pass (our own lock never blocks us).
        if USE_LOCKS and cambc.fuel_remaining() > FUEL_RESERVE:
            self.place_lock()

        # Hold a capture in progress, unless an ally (lower handle) is already holding it:
        # one champion captures, the rest move on (rewards split, so nothing is lost).
        for g in self.groups:
            if g.state == 2 and g.mask.is_set(pos):
                holder = None
                if SOLO_CAPTURE:
                    for a in cambc.vision.champions(Team.ALLY):
                        if a != me and a.is_alive() and g.mask.is_set(a.position()) and int(a) < int(me):
                            holder = a
                            break
                if holder is None:
                    self.claim(g)
                    self.act = "capture"
                    return
                g.state = 3
                g.available_at = now + g.group.max_respawn_timer() + CAPTURE_TIME
                self.target = None
                self.next_plan = 0

        # Spend gold: with an empty bag and enough for a full emblem, recall to shop.
        busy_here = (CAPTURE_FIX and self.target is not None and self.target.state in (1, 2)
                     and self.target_dist <= 7 and me.gold_count() < 2 * SHOP_RECALL_GOLD)
        threatened = CAPTURE_FIX and any(chebyshev(pos, t) <= THREAT_RANGE + 2 for t in self.threats(now))
        if (gems == 0 and me.gold_count() >= SHOP_RECALL_GOLD and self.kind != CT.TAVI
                and not at_spawn and now < 1800 and not busy_here and not threatened):
            self.claim(None)
            self.retreat_and_recall()
            self.act = "shop-recall"
            return

        target = self.target
        if MAX_WAIT:
            valid = (target is not None and now >= target.avoid_until
                     and target.available_at <= now + min(self.target_dist, PLAN_RADIUS) + MAX_WAIT)
        else:
            valid = (target is not None and now >= target.avoid_until and target.available_at <= now + 2
                     and (target.state != 3 or chebyshev(pos, target.center) > 9))
        engaged = valid and target.state in (1, 2) and chebyshev(pos, target.center) <= 6
        low_fuel = cambc.fuel_remaining() < FUEL_RESERVE + 900000
        if (not valid and not (low_fuel and target is not None)) or (now >= self.next_plan and not engaged and not low_fuel):
            my_field = cambc.DistanceField()
            roots = cambc.Bitboard()
            roots.set(pos)
            my_field.compute(self.static_block, roots, cambc.Bitboard(), PLAN_RADIUS)
            target = self.choose_group(now, my_field)
            if target is None and not low_fuel:
                # Nothing within PLAN_RADIUS: search the whole map rather than idle.
                my_field = cambc.DistanceField()
                my_field.compute(self.static_block, roots, cambc.Bitboard())
                target = self.choose_group(now, my_field)
            self.target = target
            self.next_plan = now + REPLAN_EVERY
            self.target_dist = UNREACHED if target is None else min(my_field.get(t) for t in target.tiles)
        else:
            self.target_dist = max(0, self.target_dist - 1)

        if gems > 0:
            altar_dist = self.altar_field.get(pos)
            target_dist = self.target_dist
            prism = self.kind == CT.PRISM
            cap = PRISM_GEM_CAP if prism else GEM_CAP
            on_the_way = altar_dist <= target_dist and (not prism or gems >= PRISM_EARLY_BANK)
            if target is None or gems >= cap or on_the_way:
                self.claim(None)
                self.tribute()
                self.act = "tribute"
                return

        if target is None:
            self.act = "idle"
            self.claim(None)
            # Nothing worth doing: go home to heal and shop if useful, else wait near an altar.
            if not at_spawn and (hp_frac < 0.8 or me.gold_count() >= 60):
                self.retreat_and_recall()
            elif self.step_towards(self.altar_approach):
                self.act = "idle-altar"   # guard an altar: lock it, catch enemies tributing
            return

        self.claim(target)
        camps = [c for c in visible_camps() if c._group == target.handle]
        if camps:
            if self.need > 1 and all(camp_health(c) >= c.max_health() for c in camps):
                # Untouched joint target: wait just outside camp range until helpers arrive.
                near = sum(1 for a in cambc.vision.champions(Team.ALLY)
                           if a.is_alive() and a != me and chebyshev(a.position(), pos) <= 4)
                if near < self.need - 1 and gems > 0:
                    # Bank while helpers make their way here, instead of standing with gems.
                    self.tribute()
                    self.act = "gather-bank"
                    return
                self.gather_turns += 1
                switch = near < self.need - 1 and self.joint_claim_elsewhere(target, self.camp_dps(me), me.max_health())
                if self.gather_turns > (GATHER_MAX * 5 if self.scarce else GATHER_MAX) or switch:
                    if switch:
                        # Leave it to the ally's joint target; without this marker the replan
                        # could pick this same camp again and loop (500 turns on a 100x100 map).
                        target.avoid_until = now + 30
                    else:
                        target.avoid_until = now + 50
                        self.no_joint_until = now + 40   # stop starting joint targets for a while
                    self.target = None
                    self.next_plan = 0
                    self.gather_turns = 0
                    self.act = "gather-switch" if switch else "gather-giveup"
                    return
                if near < self.need - 1:
                    ring = cambc.Bitboard()
                    for c in camps:
                        ring |= around(c.position(), 5)
                    for c in camps:
                        ring &= ~around(c.position(), 3)
                    self.claim(target)
                    self.act = "gather"
                    self.step_towards(ring)
                    return
            self.gather_turns = 0
            # Hit the most dangerous camp first, then the weakest.
            camps.sort(key=lambda c: (-(camp_strength(c) > 0), camp_health(c)))
            self.fight(camps[0], tuple(camps[1:]))
            self.act = "fight"
            return
        if target.state == 2:
            # Reward available: stand on a spawn tile to capture.
            self.step_towards(target.mask)
            self.act = "to-capture"
            return
        # Approach the camp: get within attack range of its tiles, but never stand on a
        # spawn tile of a respawning group (an occupied spawn tile stops the respawn).
        visible = not (target.mask & cambc.vision.visible_tiles()).is_empty()
        # Bushes can hide a camp from attack range; then close in to adjacent tiles.
        radius = me.attack_range() if (visible or self.stuck < 2) else 1
        approach = cambc.Bitboard()
        for t in target.tiles:
            approach |= around(t, radius)
        approach &= ~target.mask
        if target.mask.is_set(pos):
            self.step_towards(approach)
            self.act = "unblock"
            return
        if MAX_WAIT and target.state == 3 and target.available_at > now:
            # Respawning soon (chosen because nothing better was in reach): wait beside it.
            if not self.step_towards(approach):
                self.act = "wait-respawn"
            else:
                self.act = "approach"
            self.stuck = 0
            return
        if self.step_towards(approach):
            self.stuck = 0
            self.act = "approach"
            return
        self.stuck += 1
        self.act = "approach-wait"
        if self.stuck > 6:
            # Give up on it for a while (unreachable, hidden, or contested).
            target.avoid_until = now + 40
            self.stuck = 0

