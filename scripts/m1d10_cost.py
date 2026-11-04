"""M1D10D: the reading of docs/experiments/m1d10-cost-engineering.md, "Reading, fixed now". Read-only. Written before any M1D10 run.
Usage: m1d10_cost.py [outdir] [fitdir]. fitdir is results/gate1-M1D10, and is set only to dry-run. Fits U, K and KC on the 12 engineering cells (timing and count build both M1D10's) with scripts/m1d09_cost.py unchanged, and reads them
on M1D07's 12 new streams, where the outcome is read, and on M1D05's three streams, beside it. Writes fit.csv and cells.csv to outdir."""
import csv
import os
import statistics
import sys

sys.path.insert(0, "scripts")
import gate1_score as g
import m1d07_score as m
import m1d09_cost as c


def main():
    outdir = sys.argv[1] if len(sys.argv) > 1 else "results/cost-M1D10"
    fitdir = sys.argv[2] if len(sys.argv) > 2 else "results/gate1-M1D10"
    os.makedirs(outdir, exist_ok=True)
    fit_rows = c.armcells(g.load(fitdir), c.counts(f"{fitdir}/count"), lambda cell: True)
    sets = {
        "M1D07": c.armcells(m.load("results/gate1-M1D07T"), c.counts("results/gate1-M1D07"), lambda cell: cell[0] != m.SNAPSHOT),
        "M1D05": c.armcells(g.load("results/gate1-M1D04"), c.counts("results/gate1-M1D03", "results/gate1-M1D04/count"), lambda cell: cell[0] in c.FIT_STREAMS),
    }
    print(f"fitting set: {len(fit_rows)} arm-cells on {len({r['cell'] for r in fit_rows})} cells, {sum(r['big'] for r in fit_rows)} with a table over the LLC, "
          f"on {sorted({r['cell'][0] + ' ' + r['cell'][1] for r in fit_rows if r['big']})}")
    for name, rows in sets.items():
        print(f"{name}: {len(rows)} arm-cells on {len({r['cell'] for r in rows})} cells, {sum(r['big'] for r in rows)} over the LLC")

    results, fits, preds = {}, [], {}
    for model in ("U", "K", "KC"):
        cols, coef = c.fit(fit_rows, model)
        fits += [(model, n, x * 1e9) for n, x in zip(cols, coef)]
        for name, rows in [("fitting", fit_rows)] + list(sets.items()):
            pred = c.predict(rows, model, coef)
            results[(model, name)] = c.read(rows, pred)
            preds[(model, name)] = pred
    with open(f"{outdir}/fit.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["model", "counter", "ns"])
        w.writerows((a, b, f"{x:.4f}") for a, b, x in fits)
    with open(f"{outdir}/cells.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["set", "stream", "variant", "cut", "p", "arm", "seconds", "big", "ops", "U", "K", "KC"])
        for name, rows in sets.items():
            for i, r in enumerate(rows):
                w.writerow([name] + list(r["cell"]) + [r["arm"], f"{r['sec']:.4f}", int(r["big"]), int(sum(r["x"].values()))]
                           + [f"{preds[(k, name)][i]:.4f}" for k in ("U", "K", "KC")])

    print("\ncoefficients, ns per counted operation")
    for model in ("U", "K", "KC"):
        print(f"{model}: " + ", ".join(f"{n} {x:.3f}" for mm, n, x in fits if mm == model))
    print("\nmodel | set | median error | winner agrees | cells | p = 5 median-score side agrees | p = 5 cells")
    for model in ("U", "K", "KC"):
        for name in ("fitting", "M1D07", "M1D05"):
            err, agree, win, passes = results[(model, name)]
            print(f"| {model} | {name} | {err:.1%} | {agree} | {len(win)} | {sum(x[3] for x in passes)} | {len(passes)} |")

    eu, ek = results[("U", "M1D07")], results[("KC", "M1D07")]
    n = len(ek[2])
    gain = (ek[1] - eu[1]) / n
    c1, c2 = gain >= 0.25, ek[0] <= 0.25
    outcome = "Carries" if c1 and c2 else "Partly" if c1 or c2 else "Does not carry"
    print(f"\nKC against U on M1D07: winner agreement {ek[1]} against {eu[1]} of {n}, a gain of {gain:.1%} (needs 25 points: {'yes' if c1 else 'no'}); "
          f"KC median error {ek[0]:.1%} (needs at most 25%: {'yes' if c2 else 'no'})")
    print(f"OUTCOME: {outcome}")

    kk = results[("K", "M1D07")]
    print(f"\ncache, KC against K on M1D07: winners {ek[1]} against {kk[1]} of {n}, median error {ek[0]:.1%} against {kk[0]:.1%}. "
          f"Shown here: {'yes' if ek[1] - kk[1] >= 3 or kk[0] - ek[0] >= 0.05 else 'no'} (the paper needs it in M1D09 as well)")

    print("\nprediction over clock per arm, median, KC: arm | M1D07 | M1D05")
    arms = sorted({r["arm"] for rows in sets.values() for r in rows})
    for a in arms:
        line = []
        for name, rows in sets.items():
            ratio = [p / r["sec"] for r, p in zip(rows, preds[("KC", name)]) if r["arm"] == a]
            line.append(f"{statistics.median(ratio):.2f}" if ratio else "-")
        print(f"| {a} | " + " | ".join(line) + " |")

    print("\nM1D07 cells where KC names the wrong winner: cell | clock | KC")
    for cell, a, b in sorted(ek[2]):
        if a != b:
            print(f"| {cell[0]} {cell[1]} {cell[2]} p{cell[3]} | {a} | {b} |")


if __name__ == "__main__":
    main()
