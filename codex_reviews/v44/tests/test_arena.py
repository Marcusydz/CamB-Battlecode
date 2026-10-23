"""Regression checks for match accounting: failed or stale runs must never count as wins."""
import contextlib
import importlib.util
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
import zipfile
from unittest.mock import patch

spec = importlib.util.spec_from_file_location("arena", Path(__file__).parents[1] / "scripts/arena.py")
arena = importlib.util.module_from_spec(spec)
spec.loader.exec_module(arena)


def summary():
    # Team rows deliberately reversed: list order does not determine team identity.
    return {"match": {"outcome": "teamB", "roundsCompleted": 80},
            "teams": [{"team": "teamB", "submittedGems": "1000"},
                      {"team": "teamA", "submittedGems": "100"}],
            "diagnostics": {"botFaults": {"teamA": 1, "teamB": 0}},
            "champions": [{"team": "teamB", "metrics": {"kills": "2"}}]}


class ArenaTests(unittest.TestCase):
    def test_team_identity_and_numeric_values(self):
        r = arena.parse_statistics("a", "b", "maps/x.map26a", summary())
        self.assertEqual((r["gems"], r["kills"], r["faults"]), ([100, 1000], [0, 2], [1, 0]))

    def test_caught_exceptions_are_separate_from_engine_faults(self):
        with tempfile.TemporaryDirectory() as d:
            with zipfile.ZipFile(Path(d, "match.repl26a"), "w") as replay:
                replay.writestr("team-a.stdout", b"\x00\x00error r20: Unit is not in vision.\nOVERRUN r21 act=fight\n")
                replay.writestr("team-b.stdout", b"draft error r0: failed\nerror r3: failed\n")
            self.assertEqual(arena.replay_diagnostics(d),
                             {"loggedErrors": [1, 2], "printedOverruns": [1, 0]})

    def test_missing_team_is_not_a_zero_score(self):
        d = summary()
        d["teams"].pop()
        with self.assertRaises(ValueError):
            arena.parse_statistics("a", "b", "x", d)

    def test_successful_run_and_release_mode(self):
        def fake(cmd, **kw):
            self.assertIn("--release", cmd)
            Path(cmd[cmd.index("--statistics") + 1]).write_text(json.dumps(summary()))
            return subprocess.CompletedProcess(cmd, 0)
        with tempfile.TemporaryDirectory() as d, patch.object(arena.subprocess, "run", side_effect=fake):
            r = arena.run("a", "b", "maps/x.map26a", d)
            self.assertNotIn("error", r)
            self.assertEqual(r["outcome"], "teamB")

    def test_nonzero_exit_rejects_even_written_statistics(self):
        def fake(cmd, **kw):
            Path(cmd[cmd.index("--statistics") + 1]).write_text(json.dumps(summary()))
            return subprocess.CompletedProcess(cmd, 1)
        with tempfile.TemporaryDirectory() as d, patch.object(arena.subprocess, "run", side_effect=fake):
            self.assertIn("runner exited 1", arena.run("a", "b", "x", d)["error"])

    def test_previous_success_cannot_be_reused(self):
        with tempfile.TemporaryDirectory() as d:
            Path(d, "a-b-x.map26a.json").write_text(json.dumps(summary()))
            with patch.object(arena.subprocess, "run", return_value=subprocess.CompletedProcess([], 0)):
                self.assertIn("error", arena.run("a", "b", "x.map26a", d))

    def test_timeout_is_recorded(self):
        with tempfile.TemporaryDirectory() as d, patch.object(arena.subprocess, "run", side_effect=subprocess.TimeoutExpired("cambc", 1500)):
            self.assertIn("TimeoutExpired", arena.run("a", "b", "x", d)["error"])

    def test_malformed_statistics_are_recorded(self):
        def fake(cmd, **kw):
            Path(cmd[cmd.index("--statistics") + 1]).write_text("not json")
            return subprocess.CompletedProcess(cmd, 0)
        with tempfile.TemporaryDirectory() as d, patch.object(arena.subprocess, "run", side_effect=fake):
            self.assertIn("error", arena.run("a", "b", "x", d))

    def test_main_only_cpp_build_failure_stops_matches(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            (root / "maps").mkdir()
            (root / "maps/x.map26a").touch()
            (root / "bots/a/src").mkdir(parents=True)
            (root / "bots/a/src/main.cpp").touch()
            with patch.object(arena, "ROOT", d), patch.object(sys, "argv", ["arena.py", "a", "b"]), \
                 patch.object(arena.subprocess, "run", side_effect=subprocess.CalledProcessError(1, "cambc")), \
                 patch.object(arena, "run") as matches:
                with self.assertRaises(subprocess.CalledProcessError):
                    arena.main()
                matches.assert_not_called()

    def test_empty_map_selection_is_rejected(self):
        with patch.object(sys, "argv", ["arena.py", "a", "b", "--maps", "missing"]), contextlib.redirect_stderr(io.StringIO()):
            with self.assertRaises(SystemExit) as caught:
                arena.main()
            self.assertEqual(caught.exception.code, 2)

    def test_failed_games_are_reported_and_exit_nonzero(self):
        with tempfile.TemporaryDirectory() as d:
            fail = {"a": "a", "b": "b", "map": "x", "error": "failed", "log": "runner.log"}
            with patch.object(sys, "argv", ["arena.py", "a", "b", "--out", d]), \
                 patch.object(arena.os, "listdir", return_value=["x.map26a"]), \
                 patch.object(arena.os.path, "exists", return_value=False), \
                 patch.object(arena, "run", return_value=fail), \
                 contextlib.redirect_stdout(io.StringIO()) as output, contextlib.redirect_stderr(io.StringIO()):
                self.assertEqual(arena.main(), 1)
                self.assertIn("0/2 completed, 2 failed", output.getvalue())
            audit = json.loads(Path(d, "arena-results.json").read_text())
            self.assertEqual((audit["planned"], audit["completed"], audit["failed"]), (2, 0, 2))


if __name__ == "__main__":
    unittest.main()
