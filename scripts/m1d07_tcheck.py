"""M1D07D check for one cell: every timing repeat of every arm gives, per batch, the count of that arm's count build in M1D07C.
Hard rule 1. Exit 1 on any difference. The last row of a CAP record is left out, as in check_m1d02.py.
Usage: python -I scripts/m1d07_tcheck.py <timing outdir> <count outdir> <stream> <variant> <cut> <p>"""
import csv
import glob
import os
import re
import sys

tout, cout, s, v, c, p = sys.argv[1:7]
base = f"{s}-{v}-{c}-p{p}-"


def counts(path):
    with open(path, newline="") as f:
        t = [row["t"] for row in csv.DictReader(f)]
    with open(path[:-4] + ".summary.csv", newline="") as f:
        return t[:-1] if next(csv.DictReader(f))["status"] == "CAP" else t


bad = runs = 0
for path in sorted(glob.glob(os.path.join(tout, "raw", base + "*.t*.csv"))):
    if path.endswith(".summary.csv"):
        continue
    arm = re.match(r"(.+)\.t\d+\.csv$", os.path.basename(path)[len(base):]).group(1)
    ref = os.path.join(cout, "raw", base + arm + ".csv")
    if not os.path.exists(ref):
        print(f"MISSING count build for {base}{arm}")
        bad += 1
        continue
    a, b = counts(path), counts(ref)
    runs += 1
    for i in range(min(len(a), len(b))):
        if a[i] != b[i]:
            print(f"MISMATCH {os.path.basename(path)} against the count build, batch {i}: {a[i]} and {b[i]}")
            bad += 1
            break
# Every arm of the count build must have its first timing repeat, or the cell did not run: a check of nothing is not a pass
for ref in glob.glob(os.path.join(cout, "raw", base + "*.csv")):
    if ref.endswith(".summary.csv"):
        continue
    arm = os.path.basename(ref)[len(base):-4]
    if not os.path.exists(os.path.join(tout, "raw", base + arm + ".t1.summary.csv")):
        print(f"MISSING timing repeat 1 of {base}{arm}")
        bad += 1
print(f"tcheck {base[:-1]}: {runs} timing runs against the count build, {bad} differ or missing")
sys.exit(1 if bad else 0)
