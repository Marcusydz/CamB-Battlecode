"""Draft: a fixed preference list. Kept tiny because the draft instance compiles it from source."""
import cambc

D = cambc.ChampionDraft

# Ordered by testing (see README and results/): 888-game slot test, 288-game ordering test and a
# round robin of the top four orders. Maps draft 3-5 champions, so the first five matter.
PREFERENCE = [
    D.REN,
    D.TAVI,
    D.RYTH,
    D.STELLAR,
    D.MARCELLA,
    D.VARG,
    D.VENDETTA,
    D.VESPER_VALE,
    D.PRISM,
    D.CASSI_AND_ISSAC,
    D.WILLOW,
]


def play():
    chosen = set(int(p) for p in cambc.draft.current_draft(cambc.Team.ALLY))
    for pick in PREFERENCE:
        if int(pick) not in chosen:
            cambc.draft.pick(pick)
            return
