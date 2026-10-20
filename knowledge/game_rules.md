# Game rules that matter (week-2 patch, engine 1.0.0, SDK/cambc 3.0.1)

- Patch notes: public JSON at `GET https://2026a.battlecode.co.uk/api/v1/patch-notes`.
- Camp ranges: wolf/cluster 3, elk 4, golem 4, monolith 5 (Chebyshev). Spells range 4.
- New Golem camp (enum 5): 5000 HP, strength 120, range 4, 36 gems, 160 gold, 80-round respawn.
- Gem/gold clusters 120 HP. Wolf 4 gems / 40 gold, elk 15 gems / 90 gold. Kill 100 gold. Ward cooldown 10.
- Groups respawn roughly 60-80 rounds after a clear.
- Gems count only when submitted at an altar from an adjacent tile (submit range 1). Submitting is a channel of ceil(0.1 x gems) rounds (min 1); recall takes 5 rounds. Items can only be bought on spawn (Tavi shops remotely).
- Win: first to 1000 submitted gems; at round 2000 ties break on gold earned.
- boss_battle: the single altar (25,20) has one open side, (24,20), which is a monolith tile (400-gem, 40000-HP group). Nobody can bank until that camp dies, so games end on the gold tiebreak.
- Fuel is per instruction (~100k per turn): code changes shift fuel and can change decisions elsewhere.
- Shared memory: only bytes 0-7 usable.
- Local testing: Linux x64 cambc 2.2.5 runs OLD rules. The arm64 cambc 3.0.1 under qemu runs the new rules (Claude's setup).
- Unranked tests: `cambc match-groups create TEAM_ID --maps a,b,c,d,e` plays our SUBMITTED bot; results are deterministic for a given pair of bots.
