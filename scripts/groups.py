#!/usr/bin/env python3
"""Results of an unranked batch (results/unranked/batchN.json, written when queueing).

Usage: groups.py results/unranked/batch2.json [--download]
Refreshes each group, prints every game with both drafts, and the batch record. --download fetches
the replay bundles (our stdout has DEATH_LOG lines) into results/replays/<batch>/ and the
summaries into results/summaries/<batch>/.
"""
import json, os, subprocess, sys, time

ROOT = os.path.join(os.path.dirname(__file__), "..")
CAMBC = os.path.join(ROOT, ".venv", "bin", "cambc")
TEAM = "4620d7df-a9ac-4747-aa77-dce12395dc0c"
N = ["Ryth", "Vesp", "Vend", "C&I", "Stel", "Prism", "Marc", "Will", "Ren", "Varg", "Tavi", "HoR"]


def cambc(*args):
    for attempt in range(10):
        r = subprocess.run([CAMBC, *args], capture_output=True, text=True)
        if r.returncode == 0:
            return r.stdout
        time.sleep(2 + attempt)
    raise RuntimeError(r.stderr)


def main():
    path = sys.argv[1]
    batch = os.path.splitext(os.path.basename(path))[0]
    groups = json.load(open(path))
    w = n = pending = 0
    for g in groups:
        g["detail"] = json.loads(cambc("match-groups", "show", g["resp"]["groupId"], "--json").splitlines()[0])
        print(g["opp"])
        for m in g["detail"].get("matches", []):
            me = next((t for t in m.get("teams", []) if t.get("teamId") == TEAM), None)
            th = next((t for t in m.get("teams", []) if t.get("teamId") != TEAM), None)
            if not me or not me.get("result"):
                pending += 1
                print(f"    {m.get('map')}: {m.get('status')}")
                continue
            won = me["result"] == "win"
            w += won
            n += 1
            print(f"    {m['map']:22s} {'WIN ' if won else 'loss'} us={'/'.join(N[i] for i in me['draftPicks'])} "
                  f"them={'/'.join(N[i] for i in th['draftPicks'])}")
    json.dump(groups, open(path, "w"), indent=1)
    print(f"\n{batch}: {w}/{n} won, {pending} pending")
    if "--download" in sys.argv:
        ids = [m["matchId"] for g in groups for m in g["detail"].get("matches", []) if m.get("status") == "finished"]
        subprocess.run([sys.executable, os.path.join(ROOT, "scripts", "fetch_summaries.py"),
                        os.path.join(ROOT, "results", "summaries", batch)] + ids)
        out = os.path.join(ROOT, "results", "replays", batch)
        os.makedirs(out, exist_ok=True)
        for mid in ids:
            dest = os.path.join(out, mid + ".zip")
            if not os.path.exists(dest):
                try:
                    cambc("matches", "download", mid, "-o", dest)
                except RuntimeError as e:
                    print(mid, "download failed:", str(e)[:100])


if __name__ == "__main__":
    main()
