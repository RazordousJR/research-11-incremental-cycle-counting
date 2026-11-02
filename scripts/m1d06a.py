"""M1D06A helpers, as docs/experiments/m1d06a-theta.md states them. Read-only but for printing. Usage:
  m1d06a.py repeats <out> <s> <v> <c>      print arm:theta:repeat for the further repeats of one cell, theta 0 for PR and R2
  m1d06a.py read <out> <m1d04>             the reading over every cell present: holds or falls, best theta, drift against M1D04
An arm is PR, R2, or M5 at one theta, and each counts as its own arm for the repeat rule."""
import csv
import glob
import os
import re
import statistics
import sys

NAME = re.compile(r"^(.+)-(expiry|insert)-(\d+)-p5-(PR|R2|M5)(?:-th(\d+))?\.t(\d+)\.summary\.csv$")


def one(path):
    with open(path, newline="") as f:
        return next(csv.DictReader(f))


def load(out):
    """{(stream, variant, cut): {(arm, theta): {repeat: row}}}"""
    cells = {}
    for path in glob.glob(f"{out}/raw/*.summary.csv"):
        m = NAME.match(os.path.basename(path))
        if not m:
            sys.exit(f"unexpected file {path}")
        s, v, c, a, th, r = m.groups()
        cells.setdefault((s, v, int(c)), {}).setdefault((a, int(th or 0)), {})[int(r)] = one(path)
    return cells


def secs(reps):
    return [float(x["seconds"]) for x in reps.values()] if reps[1]["status"] == "OK" else None


def rot(a, th, r):
    return (sum(map(ord, a)) * 7 + th * 5 + r * 3) % 13


def repeats(out, s, v, c):
    arms = load(out).get((s, v, int(c)), {})
    done = {k: float(x[1]["seconds"]) for k, x in arms.items() if x[1]["status"] == "OK"}
    if not done:
        return
    fastest = min(done.values())
    total = 5 if fastest < 60 else 3
    keep = [k for k, t in done.items() if t <= 5 * fastest]
    for r in range(2, total + 1):
        for a, th in sorted(keep, key=lambda k: (rot(k[0], k[1], r), k)):
            print(f"{a}:{th}:{r}")


def read(out, m1d04):
    cells = load(out)
    old = {}
    for path in glob.glob(f"{m1d04}/raw/*-p5-*.summary.csv"):
        m = NAME.match(os.path.basename(path))
        if m and m.group(5) in (None, "128"):
            s, v, c, a, th, r = m.groups()
            old.setdefault((s, v, int(c), a), []).append(float(one(path)["seconds"]))
    held = fell = 0
    print("stream | variant | cut | PR slowest | best other, fastest | margin | holds | best theta | M5 at 128, fastest | PR drift | R2 drift")
    for cell in sorted(cells):
        arms = cells[cell]
        pr = secs(arms[("PR", 0)])
        others = {k: min(t) for k, x in arms.items() if k[0] != "PR" for t in [secs(x)] if t}
        m5 = {k[1]: t for k, t in others.items() if k[0] == "M5"}
        best_k = min(others, key=others.get)
        margin = max(pr) / others[best_k]
        ok = margin <= 0.67
        th = min(m5, key=m5.get) if m5 else None
        drift = []
        for a in ("PR", "R2"):
            now = secs(arms[(a, 0)])
            was = old.get((*cell, a))
            drift.append(f"{statistics.median(now) / statistics.median(was) - 1:+.1%}" if now and was else "n/a")
        print(f"| {cell[0]} | {cell[1]} | {cell[2]:,} | {max(pr):.3f} | {best_k[0]}{'' if best_k[0] == 'R2' else ' theta ' + str(best_k[1])} {others[best_k]:.3f} | {margin:.3f} | {'yes' if ok else 'no'} | {th} | {m5.get(128, float('nan')):.3f} | {drift[0]} | {drift[1]} |")
        held += ok
        fell += not ok
    print(f"cells {len(cells)}: margin at most 0.67 on {held}, above on {fell}")


if __name__ == "__main__":
    if sys.argv[1] == "repeats":
        repeats(*sys.argv[2:6])
    else:
        read(sys.argv[2], sys.argv[3])
