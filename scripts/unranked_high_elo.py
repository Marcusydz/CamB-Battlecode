#!/usr/bin/env python3
"""Automated unranked testing: queue groups against strong ladder teams, wait, report.

Runs whatever bot is currently submitted. Picks opponents rated at least MIN_ELO (not us, not dev
teams), queues one five-game group per opponent until MAX_GAMES or the shared quota runs out, then
waits for the games and prints the record, every game's drafts, pooled per-game metrics and the
death log (scripts/summary_stats.py, scripts/deaths.py).

The batch is saved to results/unranked/<name>.json in the format scripts/groups.py reads, so an
interrupted run can be resumed with --report <name>.

Examples:
  scripts/unranked_high_elo.py --dry-run                 # who would be picked, quota left
  scripts/unranked_high_elo.py                           # MIN_ELO / MAX_GAMES below
  scripts/unranked_high_elo.py --min-elo 1750 --max-games 20 --name v272_check
  scripts/unranked_high_elo.py --report v272_check       # results of an earlier batch
"""
import argparse
import datetime
import json
import os
import random
import subprocess
import sys
import time

# Defaults, overridable on the command line.
MIN_ELO = 1700      # opponents rated at least this
MAX_ELO = 1800      # ... and at most this
MAX_GAMES = 15      # games to queue per run (5 per group, rounded down to whole groups)
QUOTA_RESERVE = 0   # leave this many groups of the shared website quota unused

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CAMBC = os.path.join(ROOT, ".venv", "bin", "cambc")
TEAM_ID = "4620d7df-a9ac-4747-aa77-dce12395dc0c"  # Crimson Bush Campers
OUT_DIR = os.path.join(ROOT, "results", "unranked")


def cli(*args):
    """Run cambc, retrying rate limits and transient errors. Returns stdout."""
    for attempt in range(10):
        p = subprocess.run([CAMBC, *args], cwd=ROOT, capture_output=True, text=True)
        if p.returncode == 0:
            return p.stdout
        time.sleep(2 + attempt)
    raise RuntimeError(p.stderr.strip() or p.stdout.strip() or f"cambc exited {p.returncode}")


def cli_json(*args):
    # --json output can be followed by extra lines; the object is on the first one.
    return json.loads(cli(*args, "--json").splitlines()[0])


def ladder_teams():
    teams, page = [], 1
    while True:
        data = cli_json("ladder", "--page", str(page), "--limit", "100")
        teams += data.get("teams", [])
        if not data.get("teams") or len(teams) >= data.get("total", 0):
            return teams
        page += 1


def quota():
    """(groups left, map names) from the shared unranked quota."""
    data = cli_json("match-groups", "quota")
    q = data["quota"]
    return q["limit"] - q["used"], [m["name"] for m in data.get("maps", [])], q.get("resetsAt")


def queue(o):
    teams = ladder_teams()
    eligible = [t for t in teams if t["teamId"] != TEAM_ID and not t.get("isDev")
                and o.min_elo <= t["rating"] <= o.max_elo]
    us = next((t for t in teams if t["teamId"] == TEAM_ID), None)
    if us:
        print(f"Us: {us['rating']:.0f} (rank {us['rank']})")
    print(f"Eligible opponents rated {o.min_elo:g}-{o.max_elo:g}: {len(eligible)}")
    left, maps, resets = quota()
    groups = min(o.max_games // 5, max(0, left - QUOTA_RESERVE), len(eligible))
    print(f"Quota: {left} groups left (resets {resets}); queueing {groups} groups ({groups * 5} games)")
    rng = random.Random(o.seed)
    picks = rng.sample(eligible, groups)
    rng.shuffle(maps)
    if o.dry_run:
        for t in picks:
            print(f"  would play {t['name']} ({t['rating']:.0f})")
        return None
    batch = []
    for i, t in enumerate(picks):
        # Rotate through the whole map pool; alternate sides so each group plays both.
        group_maps = [maps[(i * 5 + j) % len(maps)] for j in range(5)]
        sides = ",".join("ab"[(i + j) % 2] for j in range(5))
        resp = cli_json("match-groups", "create", t["teamId"], "--maps", ",".join(group_maps), "--sides", sides)
        batch.append({"opp": t["name"], "rating": t["rating"], "teamId": t["teamId"],
                      "maps": group_maps, "sides": sides, "resp": resp})
        print(f"  queued vs {t['name']} ({t['rating']:.0f}): {', '.join(group_maps)}")
    return batch


def wait(batch, poll):
    """Poll until every match in the batch has finished (or errored)."""
    while True:
        pending = 0
        for g in batch:
            g["detail"] = cli_json("match-groups", "show", g["resp"]["groupId"])
            matches = g["detail"].get("matches", [])
            pending += sum(m.get("status") not in ("finished", "error") for m in matches) + max(0, 5 - len(matches))
        if not pending:
            return
        print(f"  {pending} games still running...", flush=True)
        time.sleep(poll)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--min-elo", type=float, default=MIN_ELO, help=f"minimum opponent rating (default {MIN_ELO})")
    ap.add_argument("--max-elo", type=float, default=MAX_ELO, help=f"maximum opponent rating (default {MAX_ELO})")
    ap.add_argument("--max-games", type=int, default=MAX_GAMES, help=f"games to queue (default {MAX_GAMES})")
    ap.add_argument("--name", default="", help="batch name (default: unranked_<UTC time>)")
    ap.add_argument("--seed", type=int, default=None, help="opponent/map shuffle seed (default: random)")
    ap.add_argument("--poll", type=int, default=60, help="seconds between status checks")
    ap.add_argument("--dry-run", action="store_true", help="show quota and picks, queue nothing")
    ap.add_argument("--no-wait", action="store_true", help="queue and exit; report later with --report")
    ap.add_argument("--report", default="", help="skip queueing; wait for and report this saved batch")
    o = ap.parse_args()

    os.makedirs(OUT_DIR, exist_ok=True)
    if o.report:
        name = o.report
        path = os.path.join(OUT_DIR, name + ".json")
        batch = json.load(open(path))
    else:
        batch = queue(o)
        if batch is None:
            return
        if not batch:
            print("Nothing queued.")
            return
        name = o.name or "unranked_" + datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M")
        path = os.path.join(OUT_DIR, name + ".json")
        json.dump(batch, open(path, "w"), indent=1)
        print(f"Saved {path}")
        if o.no_wait:
            print(f"Report later: scripts/unranked_high_elo.py --report {name}")
            return

    wait(batch, o.poll)
    json.dump(batch, open(path, "w"), indent=1)
    py = sys.executable
    # Record and drafts; downloads summaries and replays (our stdout carries the death log).
    subprocess.run([py, os.path.join(ROOT, "scripts", "groups.py"), path, "--download"], cwd=ROOT)
    summaries = os.path.join(ROOT, "results", "summaries", name)
    if os.path.isdir(summaries):
        out = subprocess.run([py, os.path.join(ROOT, "scripts", "summary_stats.py"), summaries, path],
                             cwd=ROOT, capture_output=True, text=True).stdout
        print(out[out.find("Pooled"):] if "Pooled" in out else out)
    replays = os.path.join(ROOT, "results", "replays", name)
    if os.path.isdir(replays):
        subprocess.run([py, os.path.join(ROOT, "scripts", "deaths.py"), replays, "main"], cwd=ROOT)


if __name__ == "__main__":
    try:
        main()
    except (RuntimeError, KeyError, json.JSONDecodeError) as e:
        print(f"error: {e}", file=sys.stderr)
        sys.exit(1)
