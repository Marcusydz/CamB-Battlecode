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
SHOP_RECALL_GOLD = 10 ** 9  # disabled: tested neutral
BUY_JUNGLE = True
GEM_CAP = 60
REPLAN_EVERY = 3
PLAN_RADIUS = 50  # max path length considered when choosing camps
LOG_EVERY = 25  # status line per champion every N rounds (0 disables)


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
    __slots__ = ("idx", "handle", "group", "mask", "tiles", "gems", "gold", "health", "center",
                 "strength", "count", "monolith", "altar_dist", "available_at", "state")

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
        self.altar_dist = 0
        self.available_at = 0   # round from which we believe it is alive
        self.state = 0          # 0 unknown/alive, 1 alive (seen), 2 reward available, 3 respawning


class Bot:
    def __init__(self):
        self.ready = False
        self.max_fuel = 0
        self.turns = 0

    # ------------------------------------------------------------------ setup
    def setup(self):
        me = cambc.me()
        self.kind = me.champion_name()
        self.melee = me.attack_range() <= 1
        self.static_block = (game_map.walls() | game_map.holes() | game_map.altars()
                             | game_map.spawn_region(Team.ENEMY))
        self.spawn = game_map.spawn_region(Team.ALLY)
        self.altars = game_map.altars().positions()
        approach = cambc.Bitboard()
        for altar in self.altars:
            approach |= around(altar, 1)
        self.altar_approach = approach & ~self.static_block

        # Distance from every tile to the nearest altar-approach tile (static terrain only).
        self.altar_field = cambc.DistanceField()
        self.altar_field.compute(self.static_block & ~self.altar_approach, self.altar_approach, cambc.Bitboard())

        self.groups = [GroupInfo(i, g) for i, g in enumerate(game_map.camp_groups())]
        self.by_handle = {g.handle: g for g in self.groups}
        for g in self.groups:
            g.altar_dist = min((self.altar_field.get(t) for t in g.tiles), default=UNREACHED)

        # Ability roles per slot.
        self.dmg_slot = None    # targeted damage spell
        self.aoe_slot = None    # ground area damage
        self.reset_slot = None
        self.dash_slot = None
        self.shield_slot = None
        for ability in me.abilities():
            slot = ability.slot()
            if ability.is_attack_reset():
                self.reset_slot = slot
            elif ability.is_dash():
                self.dash_slot = slot
            elif ability.is_shield():
                self.shield_slot = slot
            elif ability.is_heal():
                pass
            elif ability.can_target_camps():
                self.dmg_slot = slot
            elif ability.can_target_ground() and (ability.is_aoe_damage() or ability.is_cc()):
                self.aoe_slot = slot

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
        self.target = None
        self.need = 1
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
                best = min(options.positions(), key=field.get, default=None)
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
            best = min(options.positions(), key=field.get, default=None)
            if best is not None and field.get(best) < here:
                try:
                    if me.move_absolute(best):
                        moved = True
                except cambc.Error:
                    pass
        return moved

    # ------------------------------------------------------------------ combat
    def try_spell(self, target):
        me = cambc.me()
        if not me.can_cast() or not target.is_visible():
            return False
        pos = me.position()
        tpos = target.position()
        abilities = me.abilities()
        for slot in (self.dmg_slot, self.aoe_slot):
            if slot is None:
                continue
            ability = abilities[slot]
            if ability.is_on_cooldown() or chebyshev(pos, tpos) > 4:
                continue
            try:
                if slot == self.dmg_slot:
                    if ability.use(target):
                        return True
                elif ability.use(tpos):
                    return True
            except cambc.Error:
                continue
        return False

    def try_attack(self, target):
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
                    reset.use()
                    if target.is_visible() and target.health() > 0 and me.can_attack():
                        me.attack(target)
                except cambc.Error:
                    pass
        return True

    def fight(self, target):
        """Spell, attack, move into range, attack."""
        me = cambc.me()
        self.try_spell(target)
        if self.try_attack(target):
            return
        if not target.is_visible():
            return
        goal = around(target.position(), me.attack_range())
        if self.step_towards(goal, allow_dash=False) and self.melee:
            self.try_attack(target)

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
        for altar in self.altars:
            if chebyshev(pos, altar) <= 1:
                try:
                    me.submit_gems(altar)
                    return True
                except cambc.Error:
                    continue
        return self.step_towards(self.altar_approach)

    def retreat_and_recall(self):
        me = cambc.me()
        pos = me.position()
        threats = [c.position() for c in visible_camps() if c.is_visible() and camp_strength(c) > 0 and camp_health(c) < c.max_health()]
        threats += [e.position() for e in visible_enemies() if e.is_alive()]
        close = [t for t in threats if chebyshev(pos, t) <= 3]
        if not close:
            try:
                me.recall()
                return True
            except cambc.Error:
                return False
        if me.can_move():
            blocked = self.blocked()
            options = around(pos, 1) & ~blocked
            best = max(options.positions(), key=lambda p: min(chebyshev(p, t) for t in close), default=None)
            if best is not None and best != pos:
                try:
                    me.move_absolute(best)
                except cambc.Error:
                    pass
        return True

    def helpers_needed(self, g, dps, health):
        """Smallest team size that clears g without anyone dying (rough), or 0 if none."""
        if g.monolith:
            return 0
        if g.strength <= 0:
            return 1
        for n in range(1, self.team_size + 1):
            clear = g.health / (dps * n)
            # Monsters die progressively, so roughly 60% of full damage lands; spread over n.
            if g.strength * clear * 0.6 <= 0.75 * health * n:
                return n
        return 0

    def choose_group(self, now, my_field):
        me = cambc.me()
        claims = self.claims()
        claim_count = {}
        for i in range(8):
            if i != self.mem_slot and claims[i]:
                claim_count[claims[i] - 1] = claim_count.get(claims[i] - 1, 0) + 1
        dps = max(me.strength(), 20.0) + 15.0
        health = me.max_health()
        best, best_score, best_need = None, 0.0, 1
        for g in self.groups:
            dist = min((my_field.get(t) for t in g.tiles), default=UNREACHED)
            if dist >= UNREACHED:
                continue
            eta = dist
            if g.available_at > now + eta:
                continue
            others = claim_count.get(g.idx, 0)
            if g.state == 2:
                need, clear = 1, 0
            else:
                need = self.helpers_needed(g, dps * (min(g.count, 2) if self.kind == CT.VARG else 1), health)
                if need == 0:
                    continue
                clear = g.health / (dps * max(need, others + 1))
            value = (g.gems + 0.15 * g.gold) / max(need, others + 1)
            cost = eta + clear + CAPTURE_TIME + 0.5 * g.altar_dist + 1
            if need > 1:
                cost += 4 * (need - 1 - min(others, need - 1))  # time to gather helpers
            score = value / cost
            if others and need <= 1 and g.state != 2:
                score *= 0.25   # someone else is on it; spread out
            if self.target is g:
                score *= 1.2    # hysteresis
            if score > best_score:
                best, best_score, best_need = g, score, need
        self.need = best_need
        return best

    # ------------------------------------------------------------------ turn
    def play_turn(self):
        self.act = "?"
        start = cambc.fuel_remaining()
        self.turn()
        used = start - cambc.fuel_remaining()
        if used > self.max_fuel and self.turns > 1:
            self.max_fuel = used
        if used > 2400000 and self.turns > 1 and LOG_EVERY:
            print("heavy turn r%d used=%d act=%s" % (cambc.round_number(), used, self.act), flush=True)
        self.turns += 1
        now = cambc.round_number()
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

        if not me.is_alive():
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

        gems = me.gem_count()
        hp_frac = me.health() / me.max_health()

        # Survival.
        if not at_spawn and hp_frac < (0.3 + (0.15 if gems >= 30 else 0.0)):
            self.claim(None)
            self.retreat_and_recall()
            self.act = "lowhp"
            return

        # Bank immediately if that wins the game, or at the very end.
        if gems > 0 and (cambc.global_submitted_gems(Team.ALLY) + gems >= cambc.constants.WINNING_GEMS
                         or now > 1940):
            self.tribute()
            return

        enemies = [e for e in visible_enemies() if e.is_alive()]
        if enemies:
            enemies.sort(key=lambda e: e.health())
            near = [e for e in enemies if chebyshev(e.position(), pos) <= 4]
            enemy_power = sum(e.strength() for e in near)
            # Chase a weak enemy we can finish quickly: their carried gems become ours.
            weak = near[0] if near else None
            if (weak is not None and hp_frac > 0.4
                    and weak.health() <= 3 * me.strength() + 60
                    and enemy_power * 3 < me.health()):
                self.try_spell(weak)
                if not self.try_attack(weak):
                    self.step_towards(around(weak.position(), me.attack_range()), allow_dash=True)
                    self.try_attack(weak)
                self.act = "hunt"
                return
            # Carrying gems near enemies who could kill us: bank them or run home.
            if gems >= 15 and near and enemy_power * 6 > me.health():
                if self.altar_field.get(pos) <= 4:
                    self.tribute()
                    self.act = "flee-tribute"
                else:
                    self.retreat_and_recall()
                    self.act = "flee"
                return
            # Fight enemy champions already in range when healthy.
            if hp_frac > 0.45:
                for enemy in enemies:
                    if me.attack_tiles().is_set(enemy.position()):
                        self.try_spell(enemy)
                        if self.try_attack(enemy):
                            self.act = "pvp"
                            return

        # Hold a capture in progress.
        for g in self.groups:
            if g.state == 2 and g.mask.is_set(pos):
                self.claim(g)
                self.act = "capture"
                return

        # Spend gold: with an empty bag and enough for a full emblem, recall to shop.
        if (gems == 0 and me.gold_count() >= SHOP_RECALL_GOLD and self.kind != CT.TAVI
                and not at_spawn and now < 1800):
            self.claim(None)
            self.retreat_and_recall()
            self.act = "shop-recall"
            return

        target = self.target
        valid = (target is not None and target.available_at <= now + 2
                 and (target.state != 3 or chebyshev(pos, target.center) > 9))
        engaged = valid and target.state in (1, 2) and chebyshev(pos, target.center) <= 6
        if not valid or (now >= self.next_plan and not engaged):
            my_field = cambc.DistanceField()
            roots = cambc.Bitboard()
            roots.set(pos)
            my_field.compute(self.static_block, roots, cambc.Bitboard(), PLAN_RADIUS)
            target = self.choose_group(now, my_field)
            self.target = target
            self.next_plan = now + REPLAN_EVERY
            self.target_dist = UNREACHED if target is None else min(my_field.get(t) for t in target.tiles)
        else:
            self.target_dist = max(0, self.target_dist - 1)

        if gems > 0:
            altar_dist = self.altar_field.get(pos)
            target_dist = self.target_dist
            if target is None or gems >= GEM_CAP or altar_dist <= target_dist:
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
            return

        self.claim(target)
        camps = [c for c in visible_camps() if c._group == target.handle]
        if camps:
            if self.need > 1 and all(camp_health(c) >= c.max_health() for c in camps):
                # Untouched joint target: wait just outside camp range until helpers arrive.
                near = sum(1 for a in cambc.vision.champions(Team.ALLY)
                           if a.is_alive() and a != me and chebyshev(a.position(), pos) <= 4)
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
            # Hit the most dangerous camp first, then the weakest.
            camps.sort(key=lambda c: (-(camp_strength(c) > 0), camp_health(c)))
            self.fight(camps[0])
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
        if self.step_towards(approach):
            self.stuck = 0
            self.act = "approach"
            return
        self.stuck += 1
        self.act = "approach-wait"
        if self.stuck > 6:
            # Give up on it for a while (unreachable, hidden, or contested).
            target.available_at = now + 40
            self.stuck = 0

