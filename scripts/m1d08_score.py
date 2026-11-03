"""M1D08D: the reading of docs/experiments/m1d08-core-type.md, "Reading, fixed now". Read-only. Written before any M1D08 run.
Usage: m1d08_score.py [outdir]. Scoring is gate1_score.score over the arms run, unchanged. The recorded verdicts are M1D07's pooled 50 cells, read as
m1d07_score reads them."""
import statistics
import sys

sys.path.insert(0, "scripts")
import gate1_score as g
import m1d07_score as m

tout = sys.argv[1] if len(sys.argv) > 1 else "results/gate1-M1D08"
listed = [line.split() for line in open(f"{tout}/cells.txt") if not line.startswith("#") and line.strip()]
keys = {(x[0], x[1], int(x[2]), 5) for x in listed}

recorded = dict(m.scored(g.load("results/gate1-M1D04"), 5) + m.scored({c: a for c, a in m.load("results/gate1-M1D07T").items() if c[0] != m.SNAPSHOT}, 5))
core = {k: m.load(f"{tout}/{k}") for k in ("P", "E")}
scores = {k: dict(m.scored(core[k], 5)) for k in core}

print("cell | recorded | P score | P pass | E score | E pass | turns")
turned = []
for cell in sorted(keys):
    rec = recorded[cell]
    row = [cell[0], cell[1], f"{cell[2]:,}", f"{rec['margin']:.3f} {'pass' if rec['passed'] else 'fail'}"]
    for k in ("P", "E"):
        r = scores[k].get(cell)
        row += [f"{r['score']:.3f} best {r['best']}, {r['margin']:.3f}" if r else "not scored", "yes" if r and r["passed"] else "no"]
    pe = [scores[k].get(cell) and scores[k][cell]["passed"] for k in ("P", "E")]
    if pe[0] != pe[1]:
        turned.append(cell)
    print("| " + " | ".join(row) + f" | {'P and E differ' if pe[0] != pe[1] else ''} |")
missing = [c for c in keys if c not in scores["P"] or c not in scores["E"]]
print(f"\n{len(keys)} listed, scored on P {len(scores['P'])}, on E {len(scores['E'])}; not scored on both: {sorted(missing)}")

print("\n1. P-core reproduction")
same = sum(1 for c in keys if c in scores["P"] and scores["P"][c]["passed"] == recorded[c]["passed"])
print(f"P-core verdict equals the recorded verdict on {same} of {len(keys)}; drift of each arm is the last column of the table below")

print("\n2. Cells whose verdict differs between the cores:", [f"{c[0]} {c[1]} {c[2]}" for c in turned] or "none")

print("\n3. Pooled count per core")
outcome = {}
for k in ("P", "E"):
    rows = []
    for cell, r in recorded.items():
        if cell in keys:
            if cell in scores[k]:
                rows.append((cell, scores[k][cell]))
        else:
            rows.append((cell, r))
    ok, passed, parts = m.go(rows, ("expiry",))
    outcome[k] = ok
    print(f"{k}: {len(passed)} of {len(rows)} pass, a third {len(rows) / 3:.2f}, streams {len({c[0] for c in passed})}, expiry {sum(1 for c in passed if c[1] == 'expiry')}: "
          f"Go {'holds' if ok else 'fails'} {parts}")

if not outcome["P"]:
    verdict = "Not reproduced"
elif outcome["E"]:
    verdict = "Core-stable"
else:
    verdict = "Core-dependent"
print(f"\nOUTCOME, Stop aside (exactness is checked per cell by m1d08_check.py): {verdict}")

print("\nBeside it")
print("arm | cells | median E over P, by cell medians | median P over recorded")
rec_cells = {**g.load("results/gate1-M1D04"), **m.load("results/gate1-M1D07T")}
for a in ("PR", "R2", "WT", "WJ", "M5", "A5"):
    ep, pr = [], []
    for c in keys:
        tp, te = [core[k].get(c, {}).get(a) for k in ("P", "E")]
        if tp and te and tp[1]["status"] == "OK" and te[1]["status"] == "OK":
            mp = statistics.median(float(x["seconds"]) for x in tp.values())
            ep.append(statistics.median(float(x["seconds"]) for x in te.values()) / mp)
            old = rec_cells.get(c, {}).get(a)
            if old and old[1]["status"] == "OK":
                pr.append(mp / statistics.median(float(x["seconds"]) for x in old.values()))
    if ep:
        print(f"| {a} | {len(ep)} | {statistics.median(ep):.3f} | {statistics.median(pr):.3f} |" if pr else f"| {a} | {len(ep)} | {statistics.median(ep):.3f} | |")
untested = sorted(c for c in recorded if c not in keys)
print(f"\nuntested, outside the window or under 0.05 s, {len(untested)} cells, kept at their recorded verdicts: " + "; ".join(f"{c[0]} {c[1]} {c[2]} {recorded[c]['margin']:.3f}" for c in untested))
print("\nA verdict held on the E-core is not a second machine: L3 and DRAM are the same.")
