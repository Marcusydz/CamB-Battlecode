#!/usr/bin/env python3
"""Recover pinned sparring sources and maps without editing any team files."""
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
REPO = ROOT.parents[1]
BASE = "c10db83cc3bd5f1a146d60a86fdfa15fc82d43bb"


def original(path):
    return subprocess.check_output(["git", "-C", str(REPO), "show", BASE + ":" + path])


def main():
    files = ("bot.cpp", "bot.h", "draft.cpp", "draft.h", "main.cpp")
    for name in ("_cambridge-v44", "_cambridge-v43"):
        src = ROOT / "bots" / name / "src"
        src.mkdir(parents=True, exist_ok=True)
        for filename in files:
            data = original("bots/cpp-bot/src/" + filename)
            if name.endswith("v43") and filename == "bot.cpp":
                data = data.replace(b"constexpr double GOLD_W = 0.0;   // camp value per gold, in gems (0 after the week-2 patch: 1538 -> 1370 rounds unopposed on the new engine)",
                                    b"constexpr double GOLD_W = 0.15;  // camp value per gold, in gems")
                data = data.replace(b"constexpr double GOLD_RUSH_GOLD_W = 0.0;", b"constexpr double GOLD_RUSH_GOLD_W = 0.15;")
            (src / filename).write_bytes(data)
    passive = ROOT / "bots/passive/src"
    passive.mkdir(parents=True, exist_ok=True)
    (passive / "main.cpp").write_bytes(original("bots/passive/src/main.cpp"))
    maps = subprocess.check_output(["git", "-C", str(REPO), "ls-tree", "--name-only", BASE + ":maps"], text=True).splitlines()
    (ROOT / "maps").mkdir(exist_ok=True)
    for name in maps:
        if name.endswith(".map26a"):
            (ROOT / "maps" / name).write_bytes(original("maps/" + name))
    print("Prepared pinned v44 and v43 references, passive opponent, and maps in", ROOT)


if __name__ == "__main__":
    main()
