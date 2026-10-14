# my-bot — Cambridge Battlecode 2026a (Python)

A farming-focused bot: it drafts fast camp-clearers, picks the best-value camp for each
champion, clears and captures it, banks gems at altars, shops for upgrades, and avoids
dying with a full bag.

**Local results:** beats the Python starter bot 16/16 (8 pool maps × both sides), reaching
1,000 gems in ~576 rounds on average versus the starter's ~1,000+.

## Files

| File | What it is |
| --- | --- |
| `src/main.py` | Entry point. Loads the gameplay code from bytecode, the draft from source. |
| `src/draft.py` | Draft preference list. |
| `src/bot.py` | All gameplay logic (the `Bot` class). |
| `src/bot.bin` | Precompiled bytecode of `bot.py`. **Rebuild after every edit to `bot.py`.** |
| `build_bytecode.py` | Rebuilds `src/bot.bin`. Needs CPython 3.13. |

## Why bytecode? (important)

The game's Python runtime charges roughly **44,000 fuel per line just to compile source**.
A Python bot gets 2.5M fuel per turn, so compiling `bot.py` (~600 lines) takes ~18 rounds,
during which every champion stands still — and the draft bot runs out of fuel and gets
random picks. Loading precompiled bytecode costs ~1.4M fuel instead.

So after editing `bot.py`:

```
python3.13 build_bytecode.py      # from bots/my-bot/
```

If `bot.bin` is missing or stale-incompatible, `main.py` falls back to importing `bot.py`
(slow start, but it still works). The draft lives in `draft.py` so it never depends on
the bytecode. Keep `main.py` and `draft.py` small for the same reason.

> If the organisers upgrade the runtime past Python 3.13, the version check in `main.py`
> makes it fall back to source automatically; rebuild `bot.bin` with the matching version.

## Strategy

**Draft** (`draft.py`): Cassi & Issac (two bodies for one pick), Vendetta, Tavi, Ryth,
Varg, then the rest. Measured against two alternative orders; this one was fastest.

**Each turn, per champion** (`Bot.turn` in `bot.py`), first match wins:

1. Dead → shop. Channelling (recall/tribute/teleport) → let it finish.
2. On spawn (or Tavi anywhere) → buy Monster Emblem of Strength (+10% camp damage),
   then Jungle Emblem of Health (−30% camp damage), then Power strength shards.
3. Health below 30% (45% if carrying ≥15 gems) → step out of camp/enemy range, recall.
4. Banking would win the game, or round > 1940 → tribute.
5. Enemies within 4 tiles: chase and finish a weak one (steal their gems); flee/bank if
   carrying gems and they could kill us; otherwise attack enemies already in range.
6. Standing on a cleared camp's spawn tile → hold the capture.
7. Choose a target camp group (re-planned every 3 rounds, not while engaged): score =
   (gems + 0.15·gold) / (travel + clear time + capture + half the altar distance).
   - Groups too dangerous to solo are marked as needing N champions; the champion waits
     just outside camp range until helpers arrive.
   - Champions announce their target in shared memory; others avoid it.
   - Respawn timers seen in vision are remembered so we don't walk to empty camps.
8. Carrying gems: tribute if ≥60 gems, or if the altar is no farther than the next camp.
9. Fight: damage spell first, basic attack, attack-reset for a second hit; dash on long
   walks. Never stand on a respawning group's spawn tiles (that blocks the respawn).

## Testing

Run from the workspace root (`cambc init` creates it):

```
cambc run my-bot starter-python --map maps/<map>.map26a --view
./scripts/speed.sh my-bot TAG     # all maps, both sides, vs starter; prints mean win round
./scripts/bench.sh my-bot other-bot TAG   # head-to-head, all maps, both sides
./scripts/acts.sh replays/<dir> my-bot    # what champions spent their time doing
```

Each champion prints a status line every 25 rounds (`LOG_EVERY` in `bot.py`; set to 0 to
silence). They show up in the replay bundle's stdout and the Visualiser logs.

## Submitting

```
cambc login
cambc submit my-bot
```

or `cambc bundle my-bot -o submissions` and upload the zip on the Submissions page.
Make sure `src/bot.bin` is rebuilt first.

## Ideas not yet done

- The **monolith** group (20,000 HP, 200 strength, ~392 gems) is always skipped. A late-game
  five-champion assault with Jungle/Durability emblems could be worth it.
- Use wards and altar locks (deny enemy tributes).
- Smarter shared-memory use (8 bytes): broadcast enemy sightings or respawn timers.
- Tune against stronger opponents than the starter once the ladder is live.
