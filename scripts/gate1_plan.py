"""M1D04 helpers, as the protocol states them. Usage:
  gate1_plan.py theta <out>                         pick one theta per pattern, write <out>/theta.txt
  gate1_plan.py repeats <out> <s> <v> <c> <p>       print arm:repeat for the further repeats of one cell
  gate1_plan.py mcheck <out> <s> <v> <c> <p> <th>   the count build of M against its timing build, per batch. Exit 1 on any difference
Theta: the lowest geometric mean of M's seconds over the engineering cells. A value with a run that did not finish counts its limit. A tie goes to the smaller value.
Repeats: 5 in all on a cell whose fastest arm took under 60 s in repeat 1, 3 otherwise. An arm that took more than 5 times the fastest, or did not finish, runs once only."""
import csv
import glob
import math
import os
import re
import sys

GRID = [16, 32, 64, 128, 256, 512, 1024]


def one(path):
    with open(path, newline="") as f:
        return next(csv.DictReader(f))


def theta(out):
    lines = []
    for p in (3, 4, 5):
        scores = {}
        for th in GRID:
            logs = []
            for path in glob.glob(f"{out}/theta/raw/*-p{p}-M{p}-th{th}.t1.summary.csv"):
                m = one(path)
                logs.append(math.log(max(float(m["seconds"]), 1e-4)))
            if not logs:
                sys.exit(f"no runs for p={p}, theta={th}")
            scores[th] = (sum(logs) / len(logs), len(logs))
        if len({n for _, n in scores.values()}) != 1:
            sys.exit(f"p={p}: theta values have different cell counts")
        best = min(GRID, key=lambda t: (round(scores[t][0], 9), t))
        lines.append(f"{p} {best}\n")
        print(f"p={p} theta={best} " + " ".join(f"{t}:{math.exp(scores[t][0]):.4f}" for t in GRID))
    with open(f"{out}/theta.txt", "w") as f:
        f.writelines(lines)


def repeats(out, s, v, c, p):
    base = f"{out}/raw/{s}-{v}-{c}-p{p}-"
    arms = {}
    for path in glob.glob(base + "*.t1.summary.csv"):
        m = one(path)
        name = re.match(r"([A-Z0-9]+)", os.path.basename(path)[len(os.path.basename(base)):]).group(1)
        arms[name] = (m["status"], float(m["seconds"]))
    done = [t for st, t in arms.values() if st == "OK"]
    if not done:
        return
    fastest = min(done)
    total = 5 if fastest < 60 else 3
    keep = [a for a, (st, t) in arms.items() if st == "OK" and t <= 5 * fastest]
    for r in range(2, total + 1):
        for a in sorted(keep, key=lambda x: (hash_rot(x, r))):
            print(f"{a}:{r}")


def hash_rot(a, r):
    """A rotated order across repeats, the same on every run of this script."""
    return (sum(map(ord, a)) * 7 + r * 3) % 11


def counts(path):
    """Per-batch counts of a raw file. The last row of a CAP record is left out, as in check_m1d02.py."""
    with open(path, newline="") as f:
        t = [row["t"] for row in csv.DictReader(f)]
    return t[:-1] if one(path[:-4] + ".summary.csv")["status"] == "CAP" else t


def mcheck(out, s, v, c, p, th):
    tag = f"{s}-{v}-{c}-p{p}-M{p}-th{th}"
    a, b = counts(f"{out}/count/raw/{tag}.csv"), counts(f"{out}/raw/{tag}.t1.csv")
    n = min(len(a), len(b))
    diff = sum(1 for i in range(n) if a[i] != b[i])
    print(f"mcheck {tag}: {n} batches compared, {diff} differ")
    sys.exit(1 if diff else 0)


if __name__ == "__main__":
    if sys.argv[1] == "theta":
        theta(sys.argv[2])
    elif sys.argv[1] == "mcheck":
        mcheck(*sys.argv[2:8])
    else:
        repeats(*sys.argv[2:7])
