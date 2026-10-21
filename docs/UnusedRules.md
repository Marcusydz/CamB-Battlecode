# Rules we haven't taken advantage of

Mechanics from `GameRules.md` and `BotAPICpp.md` that `bots/main` ignores or only half uses.
Each entry gives the rule, what the bot does today, and an idea for using it. Line numbers refer
to `bots/main/src/bot.cpp` unless another file is named. Effort is a rough guess.

Checked against `bots/main` at v1.0.0. Status after v1.1.0 is in the Status column; details and
test results are in `docs/versions/v1.1.0.md` and `docs/versions/v1.2.0.md`.

## Summary

| # | Rule | Effort | Who benefits | Status |
|---|---|---|---|---|
| 1 | Attack reset gives ranged champions their attack back after moving | Small | Vesper Vale, Tavi, Willow | On in v1.1.0 |
| 2 | Heart of Ruin's dash deals damage and stuns on landing | Small | Heart of Ruin (first pick on many maps) | On in v1.1.0 |
| 3 | A move blocked by a hidden unit costs nothing, so you can retry | Small | Everyone | On in v1.1.0 |
| 4 | Prism gains 1.5 focus per carried gem | Small (tuning) | Prism | Tested 90: level |
| 5 | Enemy picks are visible during the draft | Medium | Draft | Not tried |
| 6 | 1024 bytes of draft memory, computed with 10x fuel | Medium | Setup, team planning | Not tried |
| 7 | `set_priority` sets turn order inside the team | Small | Stun chains, captures | Stunners first: lost |
| 8 | Shared memory has free bytes | Medium | Focus fire, tribute alerts | Focus fire: lost (v1.2.0) |
| 9 | An enemy in a capture region resets the capture | Medium | Denying enemy camps | Tested: better margin, same wins; off |
| 10 | Teleport crystals cross terrain | Medium | Walled maps, escapes | Not tried |
| 11 | Recall and tribute can be cancelled | Small | Fights that turn | Cancel recall for a kill: level; off |
| 12 | Camps attack the nearest champion and sleep after 2 rounds | Medium | Camp clearing | Not tried |
| 13 | Wolves only reach 3 tiles; spells reach 4 | Small | Casters on wolf camps | Tested: level; off |
| 14 | Bushes hide you from anyone outside them | Medium | Gathering, ambushes | Not tried |
| 15 | Smaller items (Tavi selling, Vendetta resets, spawn heals, tribute bag) | Small each | Various | Not tried |

## 1. Ranged champions can move, reset and still attack

**Rule** (GameRules, "A champion turn"): ranged champions choose between walking and a basic
attack, but "attack-reset abilities can restore the attack allowance."

**Today:** `fight()` (line 801) steps towards the target, then calls `try_attack()` (line 761).
That returns straight away because `can_attack()` is false after a ranged move. The reset is only
cast after a successful first attack, so a ranged champion that moved never uses it.

**Idea:** for ranged champions with a reset slot, when the target is out of range: step, cast the
reset, then attack. Vesper Vale's 55 strength makes this nearly a free hit every 8 turns. Check
first that the engine really restores the allowance after a move, not only after an attack.

## 2. Heart of Ruin's dash is an attack

**Rule:** the dash lands within 2 tiles and deals 1.5x focus to every enemy and camp within 1,
stunning them for a turn. The missing-health passive (up to +50%) applies.

**Today:** `setup()` files it as `dash_slot` (`is_dash()`), so it is only used to cover ground in
`step_towards` and `retreat_and_recall`. Heart of Ruin is the first pick in draft lists 16 and 18.

**Idea:** in `pvp()` and `fight()`, when Heart of Ruin has the dash ready, pick the landing tile
that hits the most enemies or camps, and use it as the opening hit or to stun an enemy who is
tributing. Read the damage and radius from the ability's dash payload (`modifier`, `radius`,
`duration`) rather than hard-coding them.

Related bug: the `switch (kind)` at line 293 has no case for `CT::heart_of_ruin`, so its
shared-memory slot and `team_size` are wrong (see `TODO.md`).

## 3. Retry when a hidden unit blocks the step

**Rule** (GameRules, "Hidden enemies still block"): a move onto a tile with an unseen unit returns
`false`. The tile is revealed for the round, and the move, cooldown or consumable is not used up,
so "choose another free tile and retry."

**Today:** `step_towards` (line 720), `step_static` (line 663) and `retreat_and_recall`
(line 1464) try one tile and give up for the turn if it fails.

**Idea:** on `false`, add that tile to the blocked set and try the next-best tile, up to 2–3
times. This matters most when fleeing through bushes.

## 4. Prism's gem focus is much bigger than we assumed

**Rule:** Prism gains 1.5 focus per carried gem. 60 gems is +90 focus on a base of 80.

**Today:** `PRISM_GEM_CAP = 60` (line 99), with a comment saying the bonus is +0.2 per gem.

**Idea:** test higher caps (80, 100) and banking only when a fight is unlikely. Prism's 1.5x focus
stun-nuke at 170 focus out-damages Stellar. The risk is that every carried gem goes to whoever
kills her.

## 5. Counter-drafting

**Rule:** both teams pick at the same time, but each pick is revealed before the next one.

**Today:** `play_draft()` (`draft.cpp` line 123) reads only our own picks
(`current_draft(team::ally)`) and works down a fixed list for the map.

**Idea:** read `current_draft(team::enemy)` and adjust. Examples:
- Marcella is stun-immune, so lower the value of stun picks (Prism, Tavi, Ren).
- Against Cassi & Issac, burst damage and stuns are worth more: killing one kills both.
- Against healers or shielders (Willow, Varg), more burst.

## 6. Draft memory

**Rule:** the draft can write 1024 bytes that every champion reads all game. The draft's first
turn gets 10x fuel and sees the whole map.

**Today:** unused. Every champion recomputes the map fingerprint, camp groups and distance fields
in `setup()`, inside its 2x first-turn budget.

**Idea:** precompute in the draft and write to memory:
- The per-map settings (squad size, gem cap and so on) as a few bytes, replacing the long
  fingerprint chains in `setup()`.
- A first camp for each draft slot, so the team spreads out from round 1.
- Which altar each camp group should bank at.

## 7. Turn order with `set_priority`

**Rule:** within a team, higher priority acts first. The setting takes effect next round and
persists.

**Today:** unused, so champions act in draft order.

**Idea:** in fights, the stunner acts first and the damage dealers follow on the stunned target.
Out of fights, the champion holding a capture or closest to an enemy acts first.

## 8. More use of shared memory

**Rule:** 8 bytes readable and writable by the whole team all game.

**Today:** one byte per draft slot holds a camp claim (`claim()`, line 567). Slots 5 and 6 are
almost always free (at most 5 picks, with Issac in slot 7).

**Idea:** use the free bytes for:
- A focus-fire target (an enemy handle or position), so every ally hits the same champion.
- An alert like "enemy tributing at altar X since round R", so the nearest ally goes to lock or
  stun them.
- A "help" flag from a champion that is losing a fight.

## 9. Contesting enemy captures

**Rule** (GameRules, "Camps & capture"): a capture needs the region uncontested for 5 rounds.
"Leaving the region empty or admitting an enemy resets progress." Any occupied spawn tile also
delays the group's respawn.

**Today:** the bot only fights enemies it meets. It never steps into an enemy's capture region on
purpose.

**Idea:** when an enemy is standing on a cleared camp's tiles (a camp just killed, its reward
available), step into the region to reset their capture, ideally with a stun. On contested
mid-map camps, this denies the whole reward.

## 10. Teleport crystals

**Rule:** 10 gold, one carried at a time. Teleports up to 3 tiles ignoring terrain, after a
3-turn channel that damage does not interrupt (enemy stuns and death do).

**Today:** never bought. `shop()` (line 896) buys only altar locks.

**Idea:**
- Reach camps that are walled in or far round a wall (corridors, rows, interlock).
- Cross a wall to escape a chase, since damage does not cancel the channel.
- Cross to an altar on the other side of a wall.

The rules disagree on one point: the map table says "TP Crystal can reach walls", but the landing
rules say you cannot land on a wall. Test before relying on either.

## 11. Cancelling recall and tribute

**Rule:** recall and tribute can each be cancelled once per turn. Teleport cannot.

**Today:** never cancelled. When a channel is running, `turn()` returns at once ("channel").

**Idea:** cancel a tribute when a winnable fight appears next to the altar. Cancel a recall when
an ally starts a fight nearby that we'd win together. Both are cheap checks at the start of the
turn.

## 12. Camp aggro and sleep

**Rule:** a hit camp wakes up and attacks the nearest champion every turn. It falls asleep after
2 rounds with no champion in range. Monsters do not chase.

**Today:** whichever champion is in range takes the hits. Nobody positions a tank in front.

**Ideas:**
- Melee champions with health or shields (Varg, Ryth, Heart of Ruin) step closest, so casters
  behind them take no camp damage.
- A hurt champion steps out of range for 2 rounds to put the camp back to sleep, instead of
  recalling.

## 13. Wolves don't reach spell range

**Rule:** wolves attack at range 3. Most damage spells reach 4.

**Today:** `KITE = 0` (line 72), because elk, golem and monolith reach 4 or more since the
week-2 patch. That is still not true for wolves.

**Idea:** turn kiting on for wolf-only groups, so casters hit them from 4 tiles and take no damage.
Spells are on an 8-turn cooldown, so this is only worth it when nothing better is near.

## 14. Bushes

**Rule:** you cannot see into a bush from outside, but from inside you see out. Monsters see
through bushes.

**Today:** bushes are only handled when they block the view of a target camp.

**Idea:** wait for helpers in a bush tile in the waiting ring, so enemies don't see a lone
champion. Recall from inside a bush, where an enemy is less likely to find you and break the
channel. Ambush enemy carriers on the path to an altar.

## 15. Smaller items

- **Tavi can sell at full refund on spawn.** Swap a treasure emblem for power shards late in the
  game, when extra gold no longer matters.
- **Vendetta resets her cooldowns on any kill, camps included.** Only one cast is allowed per
  turn, so this can't be chained within a turn. But taking a last hit makes her reset and dash
  ready again on the very next turn. Give her the last hit on camps, and use her dash on the
  first turn of a fight knowing a kill brings it back.
- **Buying health on spawn heals by the amount gained.** If gold is ready, buy health shards before
  waiting a turn to heal.
- **A tribute's length is set by the bag at the start, but it banks the bag at completion.**
  Captures resolve before channels. On maps where a camp tile is next to an altar (boss_battle),
  start a short tribute and capture into it.
- **Kills give 100 gold plus all carried gems.** Target choice weights carried gems only a little
  (`0.03` per gem in `pvp()`). Weight carriers near altars more heavily.
- **Gold breaks ties at round 2000.** On maps where neither team reaches 1000 gems, value gold
  more late in the game.
