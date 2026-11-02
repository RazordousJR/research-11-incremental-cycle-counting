"""Timed processes against known background events. Read-only. Run from the project folder.
Each timed process runs from its summary file's write time less its seconds. A process overlaps when it covers a quarter-hour mark (Research 1's tasks at :00, :15, :30,
:45, as ControlPC read them), ControlPC's mail at :06:15 for about a minute, or one of the one-off tasks ControlPC listed for M1D06A.
Usage: overlap_check.py
Prints, per claim, how slow overlapping and clean repeats run against their own arm's median on the cell, and, for every p = 5 pass, whether the best arm's
fastest repeat overlaps, which is the only way background load could make a pass that should not be."""
import csv
import datetime as dt
import glob
import os
import re
import statistics
import sys

sys.path.insert(0, "scripts")
import gate1_score as g

CLAIMS = {
    "M1D04 main": (dt.datetime(2026, 11, 2, 0, 19, 17), dt.datetime(2026, 11, 2, 2, 48, 58)),
    "M1D04 wiki": (dt.datetime(2026, 11, 2, 2, 56, 10), dt.datetime(2026, 11, 2, 8, 17, 13)),
    "M1D04 held": (dt.datetime(2026, 11, 2, 16, 40, 52), dt.datetime(2026, 11, 2, 23, 26, 23)),
    "M1D06A": (dt.datetime(2026, 11, 3, 2, 5, 57), dt.datetime(2026, 11, 3, 4, 55, 33)),
}
ONE_OFF = [(dt.datetime(2026, 11, 3, h, m, s), 60) for h, m, s in ((2, 5, 11), (3, 1, 37), (3, 13, 6), (3, 24, 21), (3, 52, 4), (3, 55, 55), (4, 0, 1), (4, 4, 28))]


def events(t0, t1):
    """Known starts and lengths in seconds inside [t0, t1]. Lengths are guesses where ControlPC gave none."""
    out = []
    t = t0.replace(minute=0, second=0, microsecond=0)
    while t <= t1:
        for m in (0, 15, 30, 45):
            out.append((t + dt.timedelta(minutes=m), 30))
        out.append((t + dt.timedelta(minutes=6, seconds=15), 60))
        t += dt.timedelta(hours=1)
    return out + [(s, n) for s, n in ONE_OFF if t0 <= s <= t1]


def runs(pattern):
    rows = []
    for path in glob.glob(pattern):
        m = re.match(r"^(.+)\.t(\d+)\.summary\.csv$", os.path.basename(path))
        if not m:
            continue
        r = g.one(path)
        end = dt.datetime.fromtimestamp(os.path.getmtime(path))
        sec = float(r["seconds"])
        rows.append(dict(arm=m.group(1), rep=int(m.group(2)), sec=sec, start=end - dt.timedelta(seconds=sec), end=end, ok=r["status"] == "OK"))
    return rows


def hit(r, ev):
    return any(s < r["end"] and s + dt.timedelta(seconds=n) > r["start"] for s, n in ev)


def per_claim(name, rows):
    t0, t1 = CLAIMS[name]
    rows = [r for r in rows if t0 <= r["start"] <= t1]
    ev = events(t0, t1)
    by = {}
    for r in rows:
        by.setdefault(r["arm"], []).append(r)
    over, clean = [], []
    for arm, xs in by.items():
        if len(xs) < 3 or not all(x["ok"] for x in xs):
            continue
        med = statistics.median(x["sec"] for x in xs)
        for x in xs:
            (over if hit(x, ev) else clean).append(x["sec"] / med - 1)
    def summary(v):
        return f"{len(v)} repeats, median {statistics.median(v):+.2%}, over 3% {sum(1 for a in v if a > 0.03)}" if v else "none"
    print(f"{name}: overlapping {summary(over)}; clean {summary(clean)}")
    return ev


all_rows = runs("results/gate1-M1D04/raw/*.summary.csv")
evs = {}
for name in ("M1D04 main", "M1D04 wiki", "M1D04 held"):
    evs[name] = per_claim(name, all_rows)
per_claim("M1D06A", runs("results/gate1-M1D06A/raw/*.summary.csv"))

print("\nM1D04 p = 5 passes: does the best arm's fastest repeat overlap a known event?")
ev = [e for v in evs.values() for e in v]
cells = g.load("results/gate1-M1D04")
index = {(r["arm"], r["rep"]): r for r in all_rows}
for c in sorted(x for x in cells if x[3] == 5):
    sc = g.score(c, cells[c])
    if not sc or not sc["passed"]:
        continue
    s, v, cut, p = c
    tag = f"{s}-{v}-{cut}-p5-{sc['best']}" + ("-th128" if sc["best"] == "M5" else "")
    reps = [index[(tag, k)] for k in cells[c][sc["best"]]]
    fastest = min(reps, key=lambda x: x["sec"])
    print(f"{s} {v} {cut}: best {sc['best']}, fastest repeat {fastest['rep']} at {fastest['start']:%H:%M:%S} for {fastest['sec']:.2f} s, overlaps {'yes' if hit(fastest, ev) else 'no'}; "
          f"its repeats overlapping {sum(hit(x, ev) for x in reps)} of {len(reps)}")
