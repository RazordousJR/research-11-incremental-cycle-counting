"""Gate 1 scoring from the timing files, by the Rule of docs/experiments/gate1-protocol.md with reading M1D04-R1. Read-only. Usage:
  gate1_score.py exact <out> <opsdir>    every repeat of every arm against every other, per batch, and against the count build in <opsdir> and <out>/count. Exit 1 on any difference
  gate1_score.py cells <out> <opsdir>    the score of every cell present, the plan check, and the verdict counts over the cells present
<out> is the M1D04 folder, <opsdir> the M1D03 folder. Share at p = 4 is the faster of P4 and B4 by median. Best is the other row with the lowest median, B4 aside: an arm stopped by time enters at its limit,
an arm stopped by memory stays out. A cell is scored if share finished and at least one other arm finished. The verdict counts are read as a verdict only over all the evaluation cells of p = 5."""
import csv
import glob
import os
import re
import statistics
import sys

SHARE = {3: ["P3"], 4: ["P4", "B4"], 5: ["PR"]}
OTHER = {3: ["S3", "T3", "M3"], 4: ["S4", "T4", "M4", "A4"], 5: ["R2", "WT", "WJ", "M5", "A5"]}
SCAN = {3: "S3", 4: "S4", 5: "R2"}
ORDER = ["sx-askubuntu", "sx-superuser", "wiki-talk-temporal", "sx-stackoverflow"]
NAME = re.compile(r"^(.+)-(expiry|insert)-(\d+)-p(\d)-([A-Z0-9]+)(?:-th\d+)?\.t(\d+)\.summary\.csv$")


def one(path):
    with open(path, newline="") as f:
        return next(csv.DictReader(f))


def load(out):
    """{(stream, variant, cut, p): {arm: {repeat: summary row}}}"""
    cells = {}
    for path in glob.glob(f"{out}/raw/*.summary.csv"):
        m = NAME.match(os.path.basename(path))
        if not m:
            sys.exit(f"unexpected file {path}")
        s, v, c, p, a, r = m.groups()
        cells.setdefault((s, v, int(c), int(p)), {}).setdefault(a, {})[int(r)] = one(path)
    return cells


def label(v):
    return "real timestamps, synthetic expiry" if v == "expiry" else "real timestamps, insert-only"


def limit(a):
    return 300.0 if a[0] == "A" else 1200.0


def times(a, reps):
    """The seconds of an arm's repeats, or None if memory stopped it. A time stop enters at the limit."""
    st = reps[1]["status"]
    if st == "OK":
        return [float(x["seconds"]) for x in reps.values()]
    return [limit(a)] if st == "TIME" else None


def score(cell, arms):
    p = cell[3]
    shares = [(statistics.median(times(a, arms[a])), a) for a in SHARE[p] if a in arms and arms[a][1]["status"] == "OK"]
    others = [(statistics.median(t), a) for a in OTHER[p] if a in arms for t in [times(a, arms[a])] if t]
    finished = [a for a in OTHER[p] if a in arms and arms[a][1]["status"] == "OK"]
    if not shares or not finished:
        return None
    sh_med, sh = min(shares)
    be_med, be = min(others)
    sh_t, be_t = times(sh, arms[sh]), times(be, arms[be])
    scan = times(SCAN[p], arms[SCAN[p]])
    return dict(share=sh, best=be, sh_med=sh_med, be_med=be_med, sh_max=max(sh_t), be_min=min(be_t), score=sh_med / be_med, passed=max(sh_t) <= 0.67 * min(be_t),
                margin=max(sh_t) / min(be_t), scan=sh_med / statistics.median(scan) if scan else None, stopped_best=arms[be][1]["status"] != "OK")


def expected(arms):
    """The repeats the protocol asks of this cell, from repeat 1."""
    done = {a: float(r[1]["seconds"]) for a, r in arms.items() if r[1]["status"] == "OK"}
    if not done:
        return {a: 1 for a in arms}
    fastest = min(done.values())
    total = 5 if fastest < 60 else 3
    return {a: total if a in done and done[a] <= 5 * fastest else 1 for a in arms}


def rows_of(path):
    with open(path, newline="") as f:
        r = [row["t"] for row in csv.DictReader(f)]
    return r[:-1] if one(path[:-4] + ".summary.csv")["status"] == "CAP" else r


def exact(out, opsdir):
    groups = {}
    for path in glob.glob(f"{out}/raw/*.t*.csv") + glob.glob(f"{out}/count/raw/*.csv") + glob.glob(f"{opsdir}/raw/*.csv"):
        if path.endswith(".summary.csv"):
            continue
        m = re.match(r"^(.+-(?:expiry|insert)-\d+-p\d)-", os.path.basename(path))
        groups.setdefault(m.group(1), []).append(path)
    keep = {key for key in (k for k in groups) if glob.glob(f"{out}/raw/{key}-*")}
    files = pairs = batches = bad = 0
    for key in sorted(keep):
        rows = {path: rows_of(path) for path in groups[key]}
        ref = max(rows, key=lambda x: len(rows[x]))
        files += len(rows)
        for path, r in rows.items():
            if path == ref:
                continue
            pairs += 1
            batches += len(r)
            if any(a != b for a, b in zip(r, rows[ref])):
                print(f"MISMATCH {path} against {ref}")
                bad += 1
    print(f"exact: {len(keep)} groups, {files} files, {pairs} pairs against the longest file, {batches} batch comparisons, {bad} files differ")
    sys.exit(1 if bad else 0)


def cells_report(out, opsdir):
    cells = load(out)
    theta = dict(line.split() for line in open(f"{out}/theta.txt"))
    wrong = 0
    for cell, arms in sorted(cells.items()):
        want = expected(arms)
        for a, n in want.items():
            if sorted(arms[a]) != list(range(1, n + 1)):
                print(f"PLAN {cell} {a}: repeats {sorted(arms[a])}, the protocol asks {n}")
                wrong += 1
    print(f"plan check: {len(cells)} cells, {sum(len(a) for a in cells.values())} arm-cells, {sum(len(r) for a in cells.values() for r in a.values())} processes, {wrong} wrong")

    for p in (3, 4, 5):
        print(f"\np = {p}: stream | variant | cut | share | median s | slowest | best | median s | fastest | score | slowest/fastest | pass | against scan")
        scored = []
        for cell in sorted((x for x in cells if x[3] == p), key=lambda x: (ORDER.index(x[0]), x[1], -x[2])):
            r = score(cell, cells[cell])
            if not r:
                print(f"| {cell[0]} | {cell[1]} | {cell[2]:,} | not scored | " + " ".join(f"{a}={st[1]['status']}" for a, st in sorted(cells[cell].items())))
                continue
            scored.append((cell, r))
            print(f"| {cell[0]} | {cell[1]} | {cell[2]:,} | {r['share']} | {r['sh_med']:.2f} | {r['sh_max']:.2f} | {r['best']}{' (stopped)' if r['stopped_best'] else ''} | {r['be_med']:.2f} | {r['be_min']:.2f} | {r['score']:.3f} | {r['margin']:.3f} | {'yes' if r['passed'] else 'no'} | {r['scan']:.3f} |" if r["scan"] else
                  f"| {cell[0]} | {cell[1]} | {cell[2]:,} | {r['share']} | {r['sh_med']:.2f} | {r['sh_max']:.2f} | {r['best']} | {r['be_med']:.2f} | {r['be_min']:.2f} | {r['score']:.3f} | {r['margin']:.3f} | {'yes' if r['passed'] else 'no'} | scan stopped |")
        if scored:
            passed = [c for c, r in scored if r["passed"]]
            scans = [r["scan"] for _, r in scored if r["scan"]]
            print(f"p = {p}: {len(scored)} scored of {len([x for x in cells if x[3] == p])}, median score {statistics.median(r['score'] for _, r in scored):.3f}, median against scan {statistics.median(scans):.3f}, "
                  f"{len(passed)} pass, on streams {sorted({c[0] for c in passed})}, {sum(1 for c in passed if c[1] == 'expiry')} expiry")

    print("\nstatus: arm | OK | TIME | MEM | CAP")
    tally = {}
    for arms in cells.values():
        for a, r in arms.items():
            tally.setdefault(a, {}).setdefault(r[1]["status"], 0)
            tally[a][r[1]["status"]] += 1
    for a in sorted(tally):
        print(f"| {a} | " + " | ".join(str(tally[a].get(st, 0)) for st in ("OK", "TIME", "MEM", "CAP")) + " |")

    print("\nspread: largest slowest/fastest of one arm on one cell, by pattern")
    for p in (3, 4, 5):
        worst = []
        for cell, arms in cells.items():
            if cell[3] != p:
                continue
            for a, r in arms.items():
                if len(r) > 1:
                    t = [float(x["seconds"]) for x in r.values()]
                    worst.append((max(t) / min(t), cell, a))
        worst.sort(reverse=True, key=lambda x: x[0])
        print(f"p = {p}: median {statistics.median(w[0] for w in worst):.3f}; " + "; ".join(f"{w[0]:.3f} {w[2]} {w[1][0]} {w[1][1]} {w[1][2]}" for w in worst[:3]))

    print("\nclock against operations: cells where the fastest finished arm differs, and ns per charged operation")
    ns = {}
    for cell, arms in sorted(cells.items()):
        s, v, c, p = cell
        ops = {}
        for a, r in arms.items():
            if r[1]["status"] != "OK":
                continue
            path = f"{out}/count/raw/{s}-{v}-{c}-p{p}-{a}-th{theta[str(p)]}.summary.csv" if a[0] == "M" else f"{opsdir}/raw/{s}-{v}-{c}-p{p}-{a}.summary.csv"
            if os.path.exists(path) and one(path)["status"] == "OK":
                ops[a] = float(one(path)["ops"])
                ns.setdefault(a, []).append(statistics.median(float(x["seconds"]) for x in r.values()) * 1e9 / ops[a])
        clock = {a: statistics.median(float(x["seconds"]) for x in arms[a].values()) for a in ops}
        if ops and min(ops, key=ops.get) != min(clock, key=clock.get):
            print(f"| {s} | {v} | {c:,} | p{p} | operations {min(ops, key=ops.get)} | clock {min(clock, key=clock.get)} |")
    print("arm | cells | median ns | lowest | highest")
    for a in sorted(ns):
        print(f"| {a} | {len(ns[a])} | {statistics.median(ns[a]):.2f} | {min(ns[a]):.2f} | {max(ns[a]):.2f} |")

    print("\npeak memory: arm-cells past 12 GB, GB")
    for cell, arms in sorted(cells.items()):
        for a, r in sorted(arms.items()):
            gb = max(float(x["peak_rss"]) for x in r.values()) / 1e9
            if gb > 12:
                print(f"| {cell[0]} | {cell[1]} | {cell[2]:,} | p{cell[3]} | {a} | {r[1]['status']} | {gb:.2f} |")


if __name__ == "__main__":
    (exact if sys.argv[1] == "exact" else cells_report)(sys.argv[2], sys.argv[3])
