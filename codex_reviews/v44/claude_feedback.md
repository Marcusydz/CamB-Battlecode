# Claude's evaluation of Codex review v44 (PR #2)

Reviewed: official 44 = commit c10db83 (internal v41). Evaluated on top of the current accepted bot, v43
(official 48, commit 50d5d33), because v43 already changed camp choice. All tests are 5-game unranked match
groups against Nyx (ladder #2) on the live server, which are deterministic per bot, plus local unopposed
farm runs on cambc 3.0.1.

| Change | Decision | Evidence |
|---|---|---|
| P2 `UNREACHED` = SDK `distance_field::unreachable` (and the 60000 cutoffs) | ACCEPT | Confirmed: `min_dist` started at 1e6, so unreached tiles (65535) were scored as valid camps. With both fixes vs Nyx: concentrated_wealth 468 -> 527 gems, lanes 442 -> 520, other maps unchanged. |
| P2 waiting band outside camp range (`camp_waiting_area.h`) | ACCEPT | Correct geometry for range-5 monoliths; only affects gather on gem-scarce maps; no change on the test maps. Accepted together with the sentinel fix (official 51). |
| P1 gold weight 0.15 | MODIFY | Agree that 0 was wrong in contested play. v43 already uses 0.1 together with walking cost x3 (vs Nyx +200 gems on concentrated_wealth, +100 expanse, +90 lanes). Kept 0 on gold_rush (any gold weight made gold_rush much slower unopposed: 1660 -> 2280+ rounds) and on 40x40 maps (broken_curve, which we win). |
| P2 arena.py harness | DEFER | Valid fixes; we currently test with separate scripts on the new engine and server match groups. Keep it under codex_reviews/v44 for now. |
| P3 visibility exceptions, Prism banking cap, squad liveness, item builds | DEFER | Agree these are worth investigating; no evidence yet. |

What Codex got right: the sentinel bug is real and measurably helped; challenging zero gold weight was right.
Disagreement: local self-play vs v43/v44 is a weak proxy; Nyx games showed the bigger issue is camp choice
(Nyx clears ~3x our camp groups by circling nearby small groups, including 0-gem gold clusters).

Next to investigate: boss_battle gives 0 gems unopposed on the new engine (all groups strength 120-200,
monolith 400); gold_rush deaths (Nyx kills our 3 champions ~27 times); lanes middle groups.

Result: accepted bot = official 51 (commit a086d87 on top-team-study).
