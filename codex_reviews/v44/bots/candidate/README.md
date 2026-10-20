# Codex candidate for Cambridge v44

Self-contained C++ source copied from Cambridge v44's matched GitHub revision
`c10db83cc3bd5f1a146d60a86fdfa15fc82d43bb`, with two correctness changes:

- use the SDK's unreachable-distance sentinel;
- retain a safe waiting band outside a range-five monolith.

It also restores v43's 0.15 gold valuation as an experimental strategic change.

Build from `codex_reviews/v44/` using `cambc build candidate --release`.
See `../../review.md` and `../../test_results.md` for evidence and limitations.
This is a review candidate, not a proven competitive upgrade or an approved submission.
