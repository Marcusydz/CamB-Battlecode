#!/usr/bin/env python3
"""Validate production waiting geometry inside the official engine."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    out = ROOT / "runs/waiting-probe"
    out.mkdir(parents=True, exist_ok=True)
    probe = Path(tempfile.mkdtemp(prefix="_waiting-probe-", dir=ROOT / "bots"))
    try:
        (probe / "src").mkdir()
        shutil.copyfile(ROOT / "tests/camp_waiting_probe.cpp", probe / "src/main.cpp")
        shutil.copyfile(ROOT / "bots/candidate/src/camp_waiting_area.h", probe / "src/camp_waiting_area.h")
        with (out / "runner.log").open("w") as log:
            subprocess.run(["cambc", "run", probe.name, "passive", "--map", "01islands", "--rounds", "1", "--release", "-o", str(out)],
                           cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=180)
        replays = sorted(out.glob("*.repl26a"), key=lambda p: p.stat().st_mtime)
        if not replays:
            raise RuntimeError("no probe replay")
        with zipfile.ZipFile(replays[-1]) as replay:
            stdout = replay.read("team-a.stdout")
        records = re.findall(rb"WAITING_PROBE[^\n]*", stdout)
        if not records or any(not row.endswith(b" PASS") for row in records):
            raise RuntimeError("waiting geometry failed: " + repr(records))
        for row in records[:3]:
            print(row.decode())
        print("All", len(records), "geometry assertions passed; replay:", replays[-1])
    finally:
        shutil.rmtree(probe)


if __name__ == "__main__":
    main()
