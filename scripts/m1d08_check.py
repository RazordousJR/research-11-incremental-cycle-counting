"""M1D08 check for one cell. Hard rule 1. Exit 1 on any difference or any missing run.
Every run on either core must equal, per batch, the recorded first timing repeat of its arm on that cell in <reference>/raw, and every run must agree with
the longest run of the cell. The last row of a CAP record is left out, as in check_m1d02.py. Every listed arm must have every repeat on both cores.
Usage: python -I scripts/m1d08_check.py <outdir> <reference> <stream> <variant> <cut> <repeats> <arms,comma,separated>
NOREF=1, for the dry run only, accepts a missing reference file."""
import csv
import glob
import os
import re
import sys

tout, ref, s, v, c, reps, arms = sys.argv[1:8]
base = f"{s}-{v}-{c}-p5-"


def counts(path):
    with open(path, newline="") as f:
        t = [row["t"] for row in csv.DictReader(f)]
    with open(path[:-4] + ".summary.csv", newline="") as f:
        return t[:-1] if next(csv.DictReader(f))["status"] == "CAP" else t


bad = 0
runs = {}
for core in ("P", "E"):
    for a in arms.split(","):
        tag = a + ("-th128" if a == "M5" else "")
        for r in range(1, int(reps) + 1):
            path = os.path.join(tout, core, "raw", f"{base}{tag}.t{r}.csv")
            if not os.path.exists(path) or not os.path.exists(path[:-4] + ".summary.csv"):
                print(f"MISSING {core} {base}{tag} repeat {r}")
                bad += 1
                continue
            runs[(core, tag, r)] = counts(path)
extra = [p for core in ("P", "E") for p in glob.glob(os.path.join(tout, core, "raw", base + "*.t*.csv"))
         if not p.endswith(".summary.csv") and re.match(r"(.+)\.t(\d+)\.csv$", os.path.basename(p)[len(base):]).group(1).split("-")[0] not in arms.split(",")]
for p in extra:
    print(f"UNLISTED {p}")
    bad += 1

for (core, tag, r), t in runs.items():
    path = os.path.join(ref, "raw", f"{base}{tag}.t1.csv")
    if not os.path.exists(path):
        if os.environ.get("NOREF") != "1":
            print(f"MISSING reference {path}")
            bad += 1
        continue
    want = counts(path)
    if len(t) != len(want):
        print(f"LENGTH {core} {tag} repeat {r}: {len(t)} batches, the reference {len(want)}")
    for i in range(min(len(t), len(want))):
        if t[i] != want[i]:
            print(f"MISMATCH {core} {tag} repeat {r} against the reference, batch {i}: {t[i]} and {want[i]}")
            bad += 1
            break

if runs:
    longest = max(runs.values(), key=len)
    for key, t in runs.items():
        if any(a != b for a, b in zip(t, longest)):
            print(f"MISMATCH {key} against the longest run of the cell")
            bad += 1
print(f"m1d08check {base[:-1]}: {len(runs)} runs, {bad} differ or missing")
sys.exit(1 if bad else 0)
