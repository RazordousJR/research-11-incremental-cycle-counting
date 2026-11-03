"""M1D07E: the reading of docs/experiments/m1d07-streams.md, "Reading, fixed now", from the timing files. Read-only. Written before any M1D07D score was read.
Usage: m1d07_score.py <M1D07 timing folder> <M1D07 count folder> <M1D04 folder>
Scoring is gate1_score.score, unchanged: Gate 1's pass test and reading M1D04-R1. M1D05's cells are M1D04's p = 3 and p = 5 cells, all of them evaluation cells.
soc-youtube-growth is a static snapshot in file order: reported, and never in a verdict or a pooled reading (log.md 0048)."""
import glob
import os
import re
import statistics
import sys

sys.path.insert(0, "scripts")
import gate1_score as g

NAME = re.compile(r"^(.+)-(expiry|insert|real)-(\d+)-p(\d)-([A-Z0-9]+)(?:-th\d+)?\.t(\d+)\.summary\.csv$")
SNAPSHOT = "soc-youtube-growth"
THETA = {3: 16, 5: 128}


def load(out):
    """{(stream, variant, cut, p): {arm: {repeat: summary row}}}, as gate1_score.load with the real-deletion variant."""
    cells = {}
    for path in glob.glob(f"{out}/raw/*.summary.csv"):
        m = NAME.match(os.path.basename(path))
        if not m:
            sys.exit(f"unexpected file {path}")
        s, v, c, p, a, r = m.groups()
        cells.setdefault((s, v, int(c), int(p)), {}).setdefault(a, {})[int(r)] = g.one(path)
    return cells


def label(cell):
    if cell[0] == SNAPSHOT:
        return "static snapshot, file order"
    return "real timestamps, real deletions" if cell[1] == "real" else g.label(cell[1])


def scored(cells, p):
    out = []
    for cell in sorted(x for x in cells if x[3] == p):
        r = g.score(cell, cells[cell])
        if r:
            out.append((cell, r))
    return out


def go(rows, kinds):
    """Gate 1's Go clause over (cell, score) rows: a third pass, on 2 streams, 2 passes of the given variants."""
    passed = [c for c, r in rows if r["passed"]]
    share = len(passed) >= len(rows) / 3
    streams = len({c[0] for c in passed}) >= 2
    kind = sum(1 for c in passed if c[1] in kinds) >= 2
    return share and streams and kind, passed, (share, streams, kind)


def median(v):
    return statistics.median(v) if v else float("nan")


def table(rows):
    print("stream | label | cut | share | median s | slowest | best | median s | fastest | score | slowest/fastest | pass | against scan")
    for cell, r in rows:
        scan = f"{r['scan']:.3f}" if r["scan"] else "scan stopped"
        print(f"| {cell[0]} | {label(cell)} | {cell[2]:,} | {r['share']} | {r['sh_med']:.2f} | {r['sh_max']:.2f} | {r['best']}{' (stopped)' if r['stopped_best'] else ''} | "
              f"{r['be_med']:.2f} | {r['be_min']:.2f} | {r['score']:.3f} | {r['margin']:.3f} | {'yes' if r['passed'] else 'no'} | {scan} |")


def main(tout, cout, old):
    new, prev = load(tout), g.load(old)

    wrong = 0
    for cell, arms in sorted(new.items()):
        for a, n in g.expected(arms).items():
            if sorted(arms[a]) != list(range(1, n + 1)):
                print(f"PLAN {cell} {a}: repeats {sorted(arms[a])}, the protocol asks {n}")
                wrong += 1
    print(f"plan check: {len(new)} cells, {sum(len(a) for a in new.values())} arm-cells, {sum(len(r) for a in new.values() for r in a.values())} processes, {wrong} wrong")

    snap = {c: a for c, a in new.items() if c[0] == SNAPSHOT}
    eval7 = {c: a for c, a in new.items() if c[0] != SNAPSHOT}
    old5, old3 = scored(prev, 5), scored(prev, 3)
    if (len(old5), sum(r["passed"] for _, r in old5)) != (14, 6):
        sys.exit(f"M1D04 p = 5 reads {len(old5)} scored, {sum(r['passed'] for _, r in old5)} pass, not M1D05's 14 and 6")

    for p in (3, 5):
        rows = scored(eval7, p)
        print(f"\nM1D07 p = {p}, {len(rows)} scored of {len([c for c in eval7 if c[3] == p])}")
        table(rows)
        for c in sorted(c for c in eval7 if c[3] == p and not g.score(c, eval7[c])):
            print(f"| {c[0]} | {label(c)} | {c[2]:,} | not scored | " + " ".join(f"{a}={st[1]['status']}" for a, st in sorted(eval7[c].items())))

    print("\nsoc-youtube-growth, static snapshot, file order: reported only")
    for p in (3, 5):
        table(scored(snap, p))

    new5, new3 = scored(eval7, 5), scored(eval7, 3)
    passed7 = [c for c, r in new5 if r["passed"]]
    print("\n1. M1D07 alone, p = 5")
    print(f"scored {len(new5)}, pass {len(passed7)}, on streams {sorted({c[0] for c in passed7})}")
    for lab in ("real timestamps, real deletions", "real timestamps, synthetic expiry", "real timestamps, insert-only"):
        rows = [(c, r) for c, r in new5 if label(c) == lab]
        print(f"| {lab} | scored {len(rows)} | pass {sum(r['passed'] for _, r in rows)} | median score {median([r['score'] for _, r in rows]):.3f} |")

    pooled5, pooled3 = old5 + new5, old3 + new3
    literal, passed, parts = go(pooled5, ("expiry",))
    second, _, parts2 = go(pooled5, ("expiry", "real"))
    print("\n2. Pooled, p = 5, M1D05's 14 scored cells and M1D07's")
    print(f"scored {len(pooled5)}, pass {len(passed)}, a third is {len(pooled5) / 3:.2f}; streams {sorted({c[0] for c in passed})}")
    print(f"expiry passes {sum(1 for c in passed if c[1] == 'expiry')}, real-deletion passes {sum(1 for c in passed if c[1] == 'real')}")
    print(f"Go, literal: {'holds' if literal else 'fails'} (third {parts[0]}, streams {parts[1]}, expiry {parts[2]})")
    print(f"Go, second reading with real-deletion cells beside expiry: {'holds' if second else 'fails'} (third {parts2[0]}, streams {parts2[1]}, expiry or real {parts2[2]})")

    m3, m5 = median([r["score"] for _, r in pooled3]), median([r["score"] for _, r in pooled5])
    trend = m3 >= 1.0 and m5 <= 0.90
    print("\n3. Trend, pooled")
    print(f"p = 3: {len(pooled3)} scored, median {m3:.3f}, needs at least 1.0; p = 5: {len(pooled5)} scored, median {m5:.3f}, needs at most 0.90: {'holds' if trend else 'fails'}")

    if literal and trend:
        outcome = "Holds"
    elif passed7:
        outcome = "Narrows"
    else:
        outcome = "Does not carry"
    if literal and trend and not passed7:
        print("NOTE: pooled Go and trend hold with no M1D07 pass; the protocol's rows overlap here and the PI reads it")
    print(f"\nOUTCOME, Stop aside (exactness is checked apart): {outcome}")

    print("\nBeside it")
    for p, rows in ((3, new3), (5, new5)):
        real = [r for c, r in rows if c[1] == "real"]
        print(f"real-deletion row, p = {p}: scored {len(real)}, pass {sum(r['passed'] for r in real)}, median score {median([r['score'] for r in real]):.3f}")
    for name, rows in (("M1D07 p = 3", new3), ("M1D07 p = 5", new5), ("pooled p = 3", pooled3), ("pooled p = 5", pooled5)):
        print(f"median against the scan alone, {name}: {median([r['scan'] for _, r in rows if r['scan']]):.3f}")
    best = {}
    for c, r in new5:
        best[r["best"]] = best.get(r["best"], 0) + 1
    print(f"best arm at p = 5 in M1D07: {dict(sorted(best.items()))}")

    print("\nclock against operations, M1D07: cells where the fastest finished arm differs, and ns per charged operation")
    ns = {}
    for cell, arms in sorted(new.items()):
        s, v, c, p = cell
        ops = {}
        for a, reps in arms.items():
            if reps[1]["status"] != "OK":
                continue
            path = f"{cout}/raw/{s}-{v}-{c}-p{p}-{a}" + (f"-th{THETA[p]}" if a[0] == "M" else "") + ".summary.csv"
            if os.path.exists(path) and g.one(path)["status"] == "OK" and float(g.one(path)["ops"]) > 0:
                ops[a] = float(g.one(path)["ops"])
                ns.setdefault(a, []).append(statistics.median(float(x["seconds"]) for x in reps.values()) * 1e9 / ops[a])
        clock = {a: statistics.median(float(x["seconds"]) for x in arms[a].values()) for a in ops}
        if ops and min(ops, key=ops.get) != min(clock, key=clock.get):
            print(f"| {s} | {v} | {c:,} | p{p} | operations {min(ops, key=ops.get)} | clock {min(clock, key=clock.get)} |")
    print("arm | cells | median ns | lowest | highest")
    for a in sorted(ns):
        print(f"| {a} | {len(ns[a])} | {statistics.median(ns[a]):.2f} | {min(ns[a]):.2f} | {max(ns[a]):.2f} |")

    print("\nstatus: arm | OK | TIME | MEM | CAP")
    tally = {}
    for arms in new.values():
        for a, r in arms.items():
            tally.setdefault(a, {}).setdefault(r[1]["status"], 0)
            tally[a][r[1]["status"]] += 1
    for a in sorted(tally):
        print(f"| {a} | " + " | ".join(str(tally[a].get(st, 0)) for st in ("OK", "TIME", "MEM", "CAP")) + " |")

    print("\nspread: largest slowest/fastest of one arm on one cell, by pattern")
    for p in (3, 5):
        worst = sorted(((max(t) / min(t), cell, a) for cell, arms in new.items() if cell[3] == p for a, r in arms.items() if len(r) > 1
                        for t in [[float(x["seconds"]) for x in r.values()]]), reverse=True, key=lambda x: x[0])
        print(f"p = {p}: median {statistics.median(w[0] for w in worst):.3f}; " + "; ".join(f"{w[0]:.3f} {w[2]} {w[1][0]} {w[1][1]} {w[1][2]}" for w in worst[:3]))


if __name__ == "__main__":
    main(*sys.argv[1:4])
