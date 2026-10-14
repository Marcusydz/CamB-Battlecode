"""Draft: a fixed preference list. Kept tiny because the draft instance compiles it from source."""
import cambc

D = cambc.ChampionDraft

# Ordered by measured camp-clearing speed (see README). Picks never block the opponent.
PREFERENCE = [
    D.CASSI_AND_ISSAC,  # two bodies from one pick
    D.VENDETTA,         # 70 strength, attack reset, kills refresh cooldowns
    D.TAVI,             # 2.2x focus stun-nuke, reset, buys shards anywhere
    D.RYTH,             # 1,200 health, area stun, attack reset
    D.VARG,             # splash basic attacks on grouped camps
    D.MARCELLA,
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
