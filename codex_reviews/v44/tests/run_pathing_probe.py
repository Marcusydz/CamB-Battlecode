#!/usr/bin/env python3
"""Exercise the real planner under the official WASI engine, with a deliberately limited field.

Run: python3 tests/run_pathing_probe.py [--source bots/cpp-bot] [--expect-failure]
Requires cambc 3.0.1 on PATH and initialized SDK. Keeps the replay and runner log in --out.
"""
import argparse
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=ROOT / "bots/candidate")
    parser.add_argument("--out", type=Path, default=ROOT / "results/pathing-probe")
    parser.add_argument("--expect-failure", action="store_true", help="verify the original bug is reproduced")
    args = parser.parse_args()
    source = args.source.resolve()
    args.out = args.out.resolve()
    args.out.mkdir(parents=True, exist_ok=True)
    attempt = Path(tempfile.mkdtemp(prefix="probe-", dir=args.out))
    probe = Path(tempfile.mkdtemp(prefix="_pathing-probe-", dir=ROOT / "bots"))
    try:
        (probe / "src").mkdir()
        for header in (source / "src").glob("*.h"):
            shutil.copyfile(header, probe / "src" / header.name)
        shutil.copyfile(source / "src/bot.cpp", probe / "src/bot.cpp")
        # Expose the test seam only in the disposable copy; production encapsulation is unchanged.
        (probe / "src/bot.h").write_text((source / "src/bot.h").read_text().replace("private:", "public:"))
        shutil.copyfile(ROOT / "tests/pathing_probe.cpp", probe / "src/main.cpp")
        command = ["cambc", "run", probe.name, "passive", "--map", "01islands", "--rounds", "3",
                   "--release", "-o", str(attempt)]
        with (attempt / "runner.log").open("w") as log:
            subprocess.run(command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=180)
        replays = list(attempt.glob("*.repl26a"))
        if len(replays) != 1:
            raise RuntimeError(f"expected one replay; see {attempt}")
        with zipfile.ZipFile(replays[0]) as archive:
            stdout = archive.read("team-a.stdout")
        records = re.findall(rb"PATHING_PROBE[^\n]*", stdout)
        if not records or b"error r" in stdout:
            raise RuntimeError(f"missing probe results or bot error; see {attempt}")
        expected = b" FAIL" if args.expect_failure else b" PASS"
        for record in records:
            print(record.decode())
        if not all(record.endswith(expected) for record in records):
            raise RuntimeError(f"unexpected planner result; see {attempt}")
        print(f"Verified {len(records)} champion instances; replay: {replays[0]}")
    finally:
        shutil.rmtree(probe)


if __name__ == "__main__":
    main()
