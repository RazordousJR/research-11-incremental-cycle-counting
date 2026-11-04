"""M1D09: the cost model of docs/experiments/m1d09-cost-model.md, "Models, fixed now" and "Reading, fixed now". Read-only. Written before any fit was read.
Usage: m1d09_cost.py [outdir]. Fits U, K and KC on M1D05's three evaluation streams (count builds M1D03 and M1D04/count, timing M1D04) and reads them on M1D07's
12 new streams (count M1D07, timing M1D07T), the snapshot left out. Seconds are the median of the timing repeats on CPU 8. Writes fit.csv and cells.csv to outdir."""
import csv
import glob
import os
import re
import statistics
import sys

import numpy as np

sys.path.insert(0, "scripts")
import gate1_score as g
import m1d07_score as m

COUNTERS = ["scan", "probe", "row", "deg", "tab_read", "tab_write", "graph", "net_ops", "group", "rc"]  # upkeep is a share of these, so it stays out
CACHED = ["probe", "tab_read", "tab_write"]
LLC = 30720 * 1024  # Win32_CacheMemory, L3, read 2026-11-04
TICK = 0.05
COUNT = re.compile(r"^(.+)-(expiry|insert|real)-(\d+)-p(\d)-([A-Z0-9]+)(?:-th\d+)?\.summary\.csv$")
FIT_STREAMS = {"sx-askubuntu", "sx-superuser", "wiki-talk-temporal"}


def counts(*dirs):
    out = {}
    for d in dirs:
        for path in glob.glob(f"{d}/raw/*.summary.csv"):
            mm = COUNT.match(os.path.basename(path))
            if not mm:
                continue
            s, v, c, p, a = mm.groups()
            r = g.one(path)
            if r["status"] == "OK" and r["build"] == "count":
                out[(s, v, int(c), int(p), a)] = r
    return out


def armcells(timing, cnt, keep):
    rows = []
    for cell, arms in timing.items():
        if not keep(cell):
            continue
        for a, reps in arms.items():
            k = cell + (a,)
            if k not in cnt or any(x["status"] != "OK" for x in reps.values()):
                continue
            sec = statistics.median(float(x["seconds"]) for x in reps.values())
            if sec < TICK:
                continue
            c = cnt[k]
            x = {n: float(c[n]) for n in COUNTERS}
            assert sum(x.values()) == float(c["ops"]), k
            rows.append(dict(cell=cell, arm=a, sec=sec, x=x, big=int(c["table_bytes"]) > LLC))
    return rows


def design(rows, model):
    cols = {"U": ["ops"], "K": COUNTERS, "KC": COUNTERS + [f"{n}>LLC" for n in CACHED]}[model]
    A = []
    for r in rows:
        if model == "U":
            A.append([sum(r["x"].values())])
        else:
            line = [r["x"][n] for n in COUNTERS]
            if model == "KC":
                line += [r["x"][n] if r["big"] else 0.0 for n in CACHED]
            A.append(line)
    return cols, np.array(A)


def nnls(A, b, iters=3000):
    """Lawson and Hanson's active-set method."""
    n = A.shape[1]
    x, P = np.zeros(n), np.zeros(n, bool)
    w = A.T @ (b - A @ x)
    for _ in range(iters):
        if P.all() or w[~P].max() <= 1e-12 * max(1.0, np.abs(w).max()):
            break
        j = np.where(~P)[0][np.argmax(w[~P])]
        P[j] = True
        while True:
            z = np.zeros(n)
            z[P] = np.linalg.lstsq(A[:, P], b, rcond=None)[0]
            if (z[P] > 0).all():
                x = z
                break
            neg = P & (z <= 0)
            alpha = np.min(x[neg] / (x[neg] - z[neg]))
            x = x + alpha * (z - x)
            P &= x > 1e-15
            x[~P] = 0.0
        w = A.T @ (b - A @ x)
    return x


def fit(rows, model):
    cols, A = design(rows, model)
    sec = np.array([r["sec"] for r in rows])
    R = A / sec[:, None]
    scale = np.where(R.max(axis=0) > 0, R.max(axis=0), 1.0)
    return cols, nnls(R / scale, np.ones(len(rows))) / scale


def predict(rows, model, coef):
    return design(rows, model)[1] @ coef


def read(rows, pred):
    err = statistics.median(abs(p / r["sec"] - 1) for r, p in zip(rows, pred))
    by = {}
    for r, p in zip(rows, pred):
        by.setdefault(r["cell"], []).append((r["arm"], r["sec"], p))
    win = [(c, min(v, key=lambda t: t[1])[0], min(v, key=lambda t: t[2])[0]) for c, v in by.items() if len(v) >= 2]
    agree = sum(1 for _, a, b in win if a == b)
    passes = []
    for c, v in by.items():
        if c[3] != 5:
            continue
        pr = [t for t in v if t[0] == "PR"]
        oth = [t for t in v if t[0] in g.OTHER[5]]
        if pr and oth:
            meas = pr[0][1] / min(t[1] for t in oth)
            prd = pr[0][2] / min(t[2] for t in oth)
            passes.append((c, meas, prd, (meas <= 0.67) == (prd <= 0.67)))
    return err, agree, win, passes


def main():
    outdir = sys.argv[1] if len(sys.argv) > 1 else "results/cost-M1D09"
    os.makedirs(outdir, exist_ok=True)
    fit_rows = armcells(g.load("results/gate1-M1D04"), counts("results/gate1-M1D03", "results/gate1-M1D04/count"), lambda c: c[0] in FIT_STREAMS)
    ev_rows = armcells(m.load("results/gate1-M1D07T"), counts("results/gate1-M1D07"), lambda c: c[0] != m.SNAPSHOT)
    print(f"fitting set: {len(fit_rows)} arm-cells on {len({r['cell'] for r in fit_rows})} cells; evaluation set: {len(ev_rows)} arm-cells on {len({r['cell'] for r in ev_rows})} cells")
    print(f"arm-cells with a table over the LLC: fitting {sum(r['big'] for r in fit_rows)}, evaluation {sum(r['big'] for r in ev_rows)}")
    results, fits = {}, []
    for model in ("U", "K", "KC"):
        cols, coef = fit(fit_rows, model)
        fits += [(model, n, c * 1e9) for n, c in zip(cols, coef)]
        for name, rows in (("fitting", fit_rows), ("evaluation", ev_rows)):
            results[(model, name)] = read(rows, predict(rows, model, coef))
        results[(model, "pred")] = predict(ev_rows, model, coef)
    with open(f"{outdir}/fit.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["model", "counter", "ns"])
        w.writerows((a, b, f"{c:.4f}") for a, b, c in fits)
    with open(f"{outdir}/cells.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["stream", "variant", "cut", "p", "arm", "seconds", "big", "ops", "U", "K", "KC"])
        for i, r in enumerate(ev_rows):
            w.writerow(list(r["cell"]) + [r["arm"], f"{r['sec']:.4f}", int(r["big"]), int(sum(r["x"].values()))] + [f"{results[(k, 'pred')][i]:.4f}" for k in ("U", "K", "KC")])

    print("\ncoefficients, ns per counted operation")
    for model in ("U", "K", "KC"):
        print(f"{model}: " + ", ".join(f"{n} {c:.3f}" for mm, n, c in fits if mm == model))
    print("\nmodel | set | median error | winner agrees | cells | p = 5 median-score side agrees | p = 5 cells")
    for model in ("U", "K", "KC"):
        for name in ("fitting", "evaluation"):
            err, agree, win, passes = results[(model, name)]
            print(f"| {model} | {name} | {err:.1%} | {agree} | {len(win)} | {sum(x[3] for x in passes)} | {len(passes)} |")

    eu, ek = results[("U", "evaluation")], results[("KC", "evaluation")]
    n = len(ek[2])
    gain = (ek[1] - eu[1]) / n
    c1, c2 = gain >= 0.25, ek[0] <= 0.25
    outcome = "Carries" if c1 and c2 else "Partly" if c1 or c2 else "Does not carry"
    print(f"\nKC against U on the evaluation set: winner agreement {ek[1]} against {eu[1]} of {n}, a gain of {gain:.1%} (needs 25 points: {'yes' if c1 else 'no'}); "
          f"KC median error {ek[0]:.1%} (needs at most 25%: {'yes' if c2 else 'no'})")
    print(f"OUTCOME: {outcome}")

    print("\nevaluation cells where KC names the wrong winner: cell | clock | KC")
    for c, a, b in sorted(ek[2]):
        if a != b:
            print(f"| {c[0]} {c[1]} {c[2]} p{c[3]} | {a} | {b} |")
    print("\np = 5 evaluation cells, PR's median score measured and by KC")
    for c, meas, prd, ok in sorted(ek[3]):
        print(f"| {c[0]} {c[1]} {c[2]} | {meas:.3f} | {prd:.3f} | {'same side' if ok else 'other side'} |")


if __name__ == "__main__":
    main()
