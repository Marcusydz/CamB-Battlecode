# Submitted versions

Official ladder version numbers and the commit that holds the exact submitted source (`bots/cpp-bot`). Accepted versions live on `top-team-study`; test candidates on `exp/*` branches.

| Official | Name | Commit | Notes |
|---|---|---|---|
| 44 | v41 | c10db83 | ignore gold when choosing camps; stay off contested camps early |
| 45 | v42 | 588abf3 | v41 + retreat at 40% health; first win vs Nyx (1/5, broken_curve 1013-901). |
| 46 | v42 diagnostic | 588abf3 with `ACT_LOG_EVERY = 400` | temporary, activity logging only; same results vs Nyx as 45 |
| 47 | v43 candidate | f7eda8d (branch exp/v43) | walking counts 3x in camp cost, gold valued at 0.1 gem; vs Nyx 0/5, lost broken_curve |
| 48 | v43 | 671d5de (branch exp/v43; same source on top-team-study) | 47 + v42's camp choice on 40x40 maps; vs Nyx 1/5, gems 468/1013/547/442/594 (CW, broken_curve, expanse, lanes, gold_rush), total 3064 vs v42 2676. |
| 49 | v44 candidate | aeb6a37 (branch exp/v44) | gold_rush draft Varg, Cassi & Issac, Stellar: gold_rush 446 vs 594; rejected |
| 50 | v43 | 5b905b5 (same bot source as 48) | v43 restored after 49 |
| 51 | v45 | a086d87 | v43 + Codex v44 fixes (SDK unreachable sentinel, waiting band); vs Nyx CW 527, expanse 547, lanes 520, gold_rush 594, broken_curve 1013 (win). |
| 52 | v46 | 1f4847e | v45 + gold mode when every altar side is a camp tile (boss_battle: only altar is behind the boss, games end on gold). boss_battle gold 36k (v45 ~3k unopposed) vs Nyx 60k, procless 41k. vs Nyx CW 527, expanse 547, lanes 346, gold_rush 594; vs procless CW 332, expanse 747, lanes 458, gold_rush 484. Live. Review-ready release (tag review-ready/official-52). |
