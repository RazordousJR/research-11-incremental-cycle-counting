"""M1D02 check for one cell. Arms of one pattern must agree on the count of every batch that both finished. R2, PR, WT and WJ must match M1C03 on count and on
operations, per batch, wherever M1C03 has a raw file. Exit 1 on any difference. Prints the operations of each arm and nothing about time."""
import csv
import glob
import os
import sys

out, stream, variant, cut = sys.argv[1:5]
base = f"{stream}-{variant}-{cut}"
bad = 0


def rows(path):
    """The rows of a raw file. The last row of a CAP record is left out, since the table stopped inside that batch, as in M1C03."""
    with open(path, newline="") as f:
        r = list(csv.DictReader(f))
    side = path[:-4] + ".summary.csv"
    if os.path.exists(side):
        with open(side, newline="") as f:
            if next(csv.DictReader(f))["status"] == "CAP":
                r = r[:-1]
    return r


for p in (3, 4, 5):
    arms = {}
    for path in sorted(glob.glob(os.path.join(out, "raw", f"{base}-p{p}-*.csv"))):
        if path.endswith(".summary.csv"):
            continue
        name = os.path.basename(path)[len(base) + 1:-4]
        arms[name] = rows(path)
    arms = {a: r for a, r in arms.items() if r}
    if not arms:
        continue
    longest = max(arms, key=lambda a: len(arms[a]))
    ref = arms[longest]
    line = []
    for name, r in arms.items():
        for i, row in enumerate(r):
            if row["t"] != ref[i]["t"]:
                print(f"MISMATCH {base} {name} against {longest}, batch {i}: {row['t']} and {ref[i]['t']}")
                bad += 1
                break
        ops = sum(int(x["ops"]) for x in r)
        line.append(f"{name.split('-')[1]}:{ops}" + ("" if len(r) == len(ref) else f"({len(r)}/{len(ref)})"))
    print(f"{base} p{p} final {ref[-1]['t']} ops " + " ".join(line))

for arm in ("R2", "PR", "WT", "WJ"):
    mine = os.path.join(out, "raw", f"{base}-p5-{arm}.csv")
    old = os.path.join("results", "screen-c-M1C03", "raw", f"{base}-{arm}.csv")
    if not (os.path.exists(mine) and os.path.exists(old)):
        continue
    a, b = rows(mine), rows(old)
    n = min(len(a), len(b))
    diff = sum(1 for i in range(n) if a[i]["t"] != b[i]["t"] or a[i]["ops"] != b[i]["ops"])
    print(f"regression {base} {arm}: {n} batches compared, {diff} differ")
    if diff:
        bad += 1

sys.exit(1 if bad else 0)
