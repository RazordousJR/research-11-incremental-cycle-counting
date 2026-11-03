"""M1D08's cells and arms, by the rule of docs/experiments/m1d08-core-type.md. Read-only on the timing records. Usage: m1d08_cells.py > results/gate1-M1D08/cells.txt
A cell is every pooled p = 5 scored evaluation cell of M1D05 and M1D07 whose recorded margin, PR's slowest over best's fastest, lies within a factor 1.2 of the
pass line 0.67, that is in [0.67 / 1.2, 0.67 * 1.2], and whose best fastest repeat is at least 0.05 s. Its arms are PR and every other arm that finished with a
median at most twice best's median. Cells are listed by their estimated cost, cheapest first, and the estimate is printed for the plan."""
import statistics
import sys

sys.path.insert(0, "scripts")
import gate1_score as g
import m1d07_score as m

LO, HI = 0.67 / 1.2, 0.67 * 1.2
SOURCES = [("results/gate1-M1D04", g.load("results/gate1-M1D04")),
           ("results/gate1-M1D07T", {c: a for c, a in m.load("results/gate1-M1D07T").items() if c[0] != m.SNAPSHOT})]


def data_of(ref, s, v):
    if ref.endswith("M1D04"):
        return f"data/derived/{s}.ins.bin" if v == "insert" else f"data/derived/{s}.bin"
    return f"data/m1d07/derived/{s}.{v}.bin"


rows = []
for ref, cells in SOURCES:
    for cell, r in m.scored(cells, 5):
        if not (LO <= r["margin"] <= HI) or r["be_min"] < 0.05:
            continue
        s, v, c, _ = cell
        med = {a: statistics.median(float(x["seconds"]) for x in reps.values()) for a, reps in cells[cell].items() if reps[1]["status"] == "OK"}
        arms = ["PR"] + [a for a in ("R2", "WT", "WJ", "M5", "A5") if a in med and med[a] <= 2 * r["be_med"]]
        reps = 5 if min(med[a] for a in arms) < 60 else 3
        rows.append((sum(med[a] for a in arms) * reps, s, v, c, data_of(ref, s, v), ref, reps, arms, r["margin"]))
rows.sort()
total = sum(x[0] for x in rows)
print(f"# {len(rows)} cells, {sum(len(x[7]) for x in rows)} arm-cells, one core {total / 3600:.2f} h at the recorded P-core seconds; both cores with the E-core 1.0, 1.5 and 2.0 times slower: "
      f"{2 * total / 3600:.2f}, {2.5 * total / 3600:.2f}, {3 * total / 3600:.2f} h")
print("# stream variant cut data reference repeats margin arms")
for sec, s, v, c, f, ref, reps, arms, margin in rows:
    print(s, v, c, f, ref, reps, f"{margin:.3f}", ",".join(arms))
