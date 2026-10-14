"""Draft: a fixed preference list. Kept tiny because the draft instance compiles it from source."""
import cambc

D = cambc.ChampionDraft

# Ordered by testing (see README). Cassi & Issac last: their linked death is costly in fights.
PREFERENCE = [
    D.VENDETTA,
    D.TAVI,
    D.RYTH,
    D.VARG,
    D.MARCELLA,
    D.CASSI_AND_ISSAC,
    D.STELLAR,
    D.PRISM,
    D.VESPER_VALE,
    D.REN,
    D.WILLOW,
]


def play():
    chosen = set(int(p) for p in cambc.draft.current_draft(cambc.Team.ALLY))
    for pick in PREFERENCE:
        if int(pick) not in chosen:
            cambc.draft.pick(pick)
            return
