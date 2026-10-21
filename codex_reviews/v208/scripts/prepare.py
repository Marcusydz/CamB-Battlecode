#!/usr/bin/env python3
"""Run from any directory after checking out the review branch. Never alters root bot."""
from pathlib import Path
import base64,hashlib,json,shutil,subprocess,tempfile
R=Path(__file__).resolve().parent.parent
repo=R.parent.parent
# Official build scaffolding is generated in an empty folder, never over team files.
if not (R/"cmake/CMakeLists.txt").exists() or not (R/"sdk/cpp/cambc.h").exists():
 with tempfile.TemporaryDirectory(prefix="cambc-v208-init-") as tmp:
  fresh=Path(tmp)/"workspace"
  subprocess.run(["cambc","init",str(fresh),"--yes"],check=True)
  if '3.0.4' not in (fresh/"cambc.toml").read_text():raise RuntimeError("Expected official SDK 3.0.4; inspect downloaded version before proceeding")
  shutil.copytree(fresh/"cmake",R/"cmake",dirs_exist_ok=True)
  shutil.copytree(fresh/"sdk/cpp",R/"sdk/cpp",dirs_exist_ok=True)
sha="d0a2110652b8a6bc1f0c16a334825b394b684284"
for name,record in json.loads((R/"evidence/maps.json").read_text()).items():
 data=base64.b64decode(record["base64"])
 assert hashlib.sha256(data).hexdigest()==record["sha256"]
 p=R/"maps"/name;p.parent.mkdir(exist_ok=True);p.write_bytes(data)
for p in (R/"bots/candidate/src").iterdir():
 out=R/"bots/_cambridge-v208/src"/p.name;out.parent.mkdir(parents=True,exist_ok=True)
 out.write_bytes(subprocess.check_output(["git","show",f"{sha}:bots/cpp-bot/src/{p.name}"],cwd=repo))
for name,source in [("_control-v208","_cambridge-v208"),("_control-fixed","candidate"),("_dash-trial","candidate"),("_probe-original","_cambridge-v208"),("_probe-fixed","candidate")]:
 dst=R/"bots"/name
 if dst.exists():shutil.rmtree(dst)
 shutil.copytree(R/"bots"/source,dst,ignore=shutil.ignore_patterns(".cambc"))
 if name.startswith("_probe"):
  header=dst/"src/bot.h";header.write_text(header.read_text().replace("private:","public:",1))
  shutil.copyfile(R/"tests/probe_main.cpp",dst/"src/main.cpp")
# Apply the independently selectable experimental change only to the trial copy.
p=R/"bots/_dash-trial/src/bot.cpp"
p.write_text(p.read_text())
patch=(R/"evidence/offensive-dash.patch").read_text().replace("bots/candidate/","bots/_dash-trial/")
subprocess.run(["git","apply","--unsafe-paths","-"],input=patch,text=True,cwd=R,check=True)
print("Prepared exact original, controls, two setup probes, dash experiment, and 35 hash-checked maps.")
