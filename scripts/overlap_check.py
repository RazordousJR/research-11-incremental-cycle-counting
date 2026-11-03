"""Timed processes against known background events. Read-only. Run from the project folder.
Each timed process runs from its summary file's write time less its seconds. A process overlaps when it covers a quarter-hour mark (Research 1's tasks at :00, :15, :30,
:45, as ControlPC read them), ControlPC's mail at :06:15 for about a minute, or one of the one-off tasks ControlPC listed for M1D06A.
Usage: overlap_check.py [M1D07D]. With M1D07D, the two M1D07D claims and their passes only.
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

CLAIM2_END = dt.datetime(2026, 11, 4, 6, 32, 19)  # M1D07C-RELEASED in run.log
CLAIMS = {
    "M1D04 main": (dt.datetime(2026, 11, 2, 0, 19, 17), dt.datetime(2026, 11, 2, 2, 48, 58)),
    "M1D04 wiki": (dt.datetime(2026, 11, 2, 2, 56, 10), dt.datetime(2026, 11, 2, 8, 17, 13)),
    "M1D04 held": (dt.datetime(2026, 11, 2, 16, 40, 52), dt.datetime(2026, 11, 2, 23, 26, 23)),
    "M1D06A": (dt.datetime(2026, 11, 3, 2, 5, 57), dt.datetime(2026, 11, 3, 4, 55, 33)),
    "M1D07D claim 1": (dt.datetime(2026, 11, 3, 22, 10, 8), dt.datetime(2026, 11, 4, 2, 30, 10)),
    "M1D07D claim 2": (dt.datetime(2026, 11, 4, 2, 35, 1), CLAIM2_END),
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


def per_claim(name, rows, known=None):
    t0, t1 = CLAIMS[name]
    rows = [r for r in rows if t0 <= r["start"] <= t1]
    ev = (known or events)(t0, t1)
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


def events_m1d07d(t0, t1):
    """M1D07D, Research 1's tasks left running (log.md 0054): FollowOnGate at :00, :15, :30, :45 and Status at :00, :30, taken as 30 s; Backup's git bundle at :00, as 90 s;
    BackupResults at 00:00 and 06:00, as 300 s; ControlPC's mail at about :06, from :05:30 for 120 s. Every length is a guess, made wide."""
    out = []
    t = t0.replace(minute=0, second=0, microsecond=0)
    while t <= t1:
        out += [(t, 300 if t.hour in (0, 6) else 90), (t + dt.timedelta(minutes=15), 30), (t + dt.timedelta(minutes=30), 30), (t + dt.timedelta(minutes=45), 30),
                (t + dt.timedelta(minutes=5, seconds=30), 120)]
        t += dt.timedelta(hours=1)
    # Research 12's one empty echo, a few minutes after 05:23:56 by ControlPC's reading of its plan file's write time; 05:23:56 to 05:34:00 is taken
    echo = dt.datetime(2026, 11, 4, 5, 23, 56)
    return out + ([(echo, 604)] if t0 <= echo <= t1 else [])


if sys.argv[1:] == ["M1D07D"]:
    import m1d07_score as m7
    rows7 = runs("results/gate1-M1D07T/raw/*.summary.csv")
    ev = []
    for name in ("M1D07D claim 1", "M1D07D claim 2"):
        ev += per_claim(name, rows7, events_m1d07d)
    print(f"M1D07D: {len(rows7)} timed processes, {sum(1 for r in rows7 if not any(t0 <= r['start'] <= t1 for t0, t1 in (CLAIMS['M1D07D claim 1'], CLAIMS['M1D07D claim 2'])))} outside both claims")
    print("\nM1D07D p = 5 passes: does the best arm's fastest repeat overlap a known event?")
    cells = m7.load("results/gate1-M1D07T")
    index = {(r["arm"], r["rep"]): r for r in rows7}
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
    sys.exit(0)

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
