"""M1D07D's timing plan, from M1D07C's count-build seconds. Protocol: docs/experiments/m1d07-streams.md. Not timing: the count build
runs at 1.0 to 1.1 times the timing build on the same cells (M1D03 against M1D04), so its seconds stand for one timing repeat.
Repeat rule as scripts/gate1_plan.py: 5 in all on a cell whose fastest finished arm takes under 60 s, 3 otherwise, and an arm past
5 times the fastest, or not finished, runs once. Phases as the protocol fixes them. Cells run beside Research 12 are marked.
Usage: python -I scripts/m1d07_plan.py [outdir]               the plan
       python -I scripts/m1d07_plan.py outdir --cells D1 [D2 ...] the cells of those phases, in protocol order, one stream:variant:cut:p a line
"""
import csv
import datetime
import glob
import os
import re
import sys

out = sys.argv[1] if len(sys.argv) > 1 else "results/gate1-M1D07"
NEIGHBOUR = [("10:31:05", "10:57:01"), ("11:51:17", "13:04:03")]   # log.md 0049
HELD = {("soc-youtube-growth", "expiry")}                              # log.md 0048
KONECT_A = {"simplewiki", "nlwiki", "plwiki"}
KONECT_B = {"itwiki", "frwiki", "dewiki"}


def events(s, v):
    with open(f"data/m1d07/derived/{s}.{v}.bin.txt") as f:
        return int(re.search(r" events=(\d+)", f.read()).group(1))


def phase(s, v, p):
    if p == 3:
        return "D1"
    if s in KONECT_A:
        return "D2"
    if s in KONECT_B:
        return "D3"
    return "D1" if events(s, v) < 1_000_000 else "D4"


cells = {}
for path in glob.glob(f"{out}/raw/*.summary.csv"):
    with open(path, newline="") as f:
        m = next(csv.DictReader(f))
    s, v, c, p, a = m["stream"], m["variant"], int(m["cut"]), int(m["pattern"]), m["arm"]
    if (s, v) in HELD:
        continue
    end = datetime.datetime.fromtimestamp(os.path.getmtime(path))
    start = end - datetime.timedelta(seconds=float(m["seconds"]))
    beside = any(start.strftime("%H:%M:%S") < b and end.strftime("%H:%M:%S") > a0 for a0, b in NEIGHBOUR)
    cells.setdefault((s, v, c, p), {})[a] = (m["status"], float(m["seconds"]), beside)

if "--cells" in sys.argv:
    want = sys.argv[sys.argv.index("--cells") + 1:]
    for ph in want:
        for (s_, v_, c_, p_) in sorted(cells, key=lambda k: (k[3], k[0], k[1], -k[2])):
            if phase(s_, v_, p_) == ph:
                print(f"{s_}:{v_}:{c_}:{p_}")
    sys.exit(0)

phases = {}
rows = []
for (s, v, c, p), arms in sorted(cells.items()):
    done = [t for st, t, _ in arms.values() if st == "OK"]
    fastest = min(done) if done else None
    total = 5 if fastest is not None and fastest < 60 else 3
    cost = 0.0
    for a, (st, t, _) in arms.items():
        reps = total if st == "OK" and fastest is not None and t <= 5 * fastest else 1
        cost += t * reps
    beside = any(b for _, _, b in arms.values())
    ph = phase(s, v, p)
    phases[ph] = phases.get(ph, 0.0) + cost
    rows.append((ph, s, v, c, p, cost, total, beside))

print("phase stream variant cut p hours repeats beside_R12")
for ph, s, v, c, p, cost, total, beside in sorted(rows):
    print(f"{ph} {s} {v} {c} {p} {cost / 3600:.2f} {total} {'yes' if beside else ''}")
print()
for ph in sorted(phases):
    print(f"{ph} {phases[ph] / 3600:.2f} h, at 1.1 times {phases[ph] * 1.1 / 3600:.2f} h")
allh = sum(phases.values()) / 3600
print(f"all {allh:.2f} h, at 1.1 times {allh * 1.1:.2f} h")
