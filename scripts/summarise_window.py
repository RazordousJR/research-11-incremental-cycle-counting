"""Tables for M1B01 from results/screen-b-M1B01/*.csv. Usage: python -I scripts/summarise_window.py [slot seconds]

Prints markdown. The verdict lines apply the rule of docs/experiments/screen-b-protocol.md and nothing else.
"""
import csv
import sys

STREAMS = ["CollegeMsg", "email-Eu-core-temporal", "sx-mathoverflow", "sx-askubuntu", "sx-superuser", "wiki-talk-temporal"]
ROOT = "results/screen-b-M1B01"


def load(stream, k, w):
    with open(f"{ROOT}/{stream}-k{k}-w{w}.csv") as f:
        return {r["method"]: r for r in csv.DictReader(f)}


def n(row, col):
    return int(float(row[col]))


def main(w):
    k4, k1 = {}, {}
    print(f"### Scores, slot width {w} s, real timestamps, window query\n")
    print("| Stream | W k=4 | B2 plain | B2 recency | Score k=4 | W k=1 | Plain | Death | Score k=1 |")
    print("|---|---:|---:|---:|---:|---:|---:|---:|---:|")
    for s in STREAMS:
        a, b = load(s, 4, w), load(s, 1, w)
        base4 = min(n(a["B2_plain"], "ops"), n(a["B2_recency"], "ops"))
        base1 = min(n(b["P"], "ops"), n(b["D"], "ops"))
        k4[s], k1[s] = n(a["W"], "ops") / base4, n(b["W"], "ops") / base1
        print(f"| {s} | {n(a['W'], 'ops'):,} | {n(a['B2_plain'], 'ops'):,} | {n(a['B2_recency'], 'ops'):,} | **{k4[s]:.3f}** | "
              f"{n(b['W'], 'ops'):,} | {n(b['P'], 'ops'):,} | {n(b['D'], 'ops'):,} | **{k1[s]:.3f}** |")
    go = sum(v <= 0.67 for v in k4.values())
    stop_a = all(v > 0.80 for v in k4.values())
    stop_b = all(v > 1.0 for v in k1.values())
    print(f"\nGo clause: {go} of 6 at k=4 are at most 0.67, so it is {'met' if go >= 3 else 'not met'}. "
          f"Stop clause A (all above 0.80 at k=4): {'met' if stop_a else 'not met'}. "
          f"Stop clause B (all above 1.0 at k=1): {'met' if stop_b else 'not met'}.\n")

    print("### Stress tests on the k=4 score\n")
    print("| Stream | W / B1 | W0 / best B2 | W charged k per unit / best B2 | W / idealised B2 | W0 k=1 / best k=1 |")
    print("|---|---:|---:|---:|---:|---:|")
    for s in STREAMS:
        a, b = load(s, 4, w), load(s, 1, w)
        wops = n(a["W"], "ops")
        best = min(a["B2_plain"], a["B2_recency"], key=lambda r: n(r, "ops"))
        units = n(a["W"], "moved") + n(a["W"], "added")
        # idealised B2: one graph's upkeep, and scans that cost only what B1's per-window graphs pay, with free filters
        upkeep = sum(n(a["B2_plain"], c) for c in ("field", "edit", "queue", "count"))
        ideal = n(a["B1"], "scan") + n(a["B1"], "probe") + upkeep
        print(f"| {s} | {wops / n(a['B1'], 'ops'):.3f} | {n(a['W0'], 'ops') / n(best, 'ops'):.3f} | "
              f"{(wops + 4 * units) / n(best, 'ops'):.3f} | {wops / ideal:.3f} | "
              f"{n(b['W0'], 'ops') / min(n(b['P'], 'ops'), n(b['D'], 'ops')):.3f} |")

    print("\n### Memory and work\n")
    print("| Stream | Peak words W k=4 | B2 plain | B2 recency | Peak words W k=1 | Moved per event k=4 | Added per event | "
          "Scans per event W | Scans per event B2 |")
    print("|---|---:|---:|---:|---:|---:|---:|---:|---:|")
    for s in STREAMS:
        a, b = load(s, 4, w), load(s, 1, w)
        e = n(a["W"], "events")
        best = min(a["B2_plain"], a["B2_recency"], key=lambda r: n(r, "ops"))
        print(f"| {s} | {n(a['W'], 'peak_words'):,} | {n(a['B2_plain'], 'peak_words'):,} | {n(a['B2_recency'], 'peak_words'):,} | "
              f"{n(b['W'], 'peak_words'):,} | {n(a['W'], 'moved') / e:.3f} | {n(a['W'], 'added') / e:.3f} | "
              f"{n(a['W'], 'scans_event') / e:.3f} | {(n(best, 'scans_event') + n(best, 'scans_expiry')) / e:.3f} |")

    print("\n### Recency order, k=4\n")
    print("| Stream | Events | First arrivals | Repeats | Same-slot no-ops | Elements per repeat W0 | W | Cut | Ops W / W0 | Fallbacks per repeat |")
    print("|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|")
    for s in STREAMS:
        a = load(s, 4, w)
        x, y = a["W"], a["W0"]
        rep = max(n(x, "repeats"), 1)
        cut = 1 - n(x, "elems_repeat") / max(n(y, "elems_repeat"), 1)
        print(f"| {s} | {n(x, 'events'):,} | {n(x, 'firsts'):,} | {n(x, 'repeats'):,} | {n(x, 'noops'):,} | "
              f"{n(y, 'elems_repeat') / rep:.1f} | {n(x, 'elems_repeat') / rep:.1f} | {cut:.1%} | "
              f"{n(x, 'ops') / n(y, 'ops'):.3f} | {n(x, 'fallbacks') / rep:.3f} |")

    print("\n### Cost by category, k=4, share of each total\n")
    cats = ["scan", "probe", "read", "field", "edit", "slot", "recency", "queue", "count"]
    print("| Stream | Method | " + " | ".join(cats) + " |")
    print("|---|---|" + "---:|" * len(cats))
    for s in STREAMS:
        a = load(s, 4, w)
        for m in ("W", "B2_recency"):
            t = n(a[m], "ops")
            print(f"| {s} | {m} | " + " | ".join(f"{n(a[m], c) / t:.1%}" for c in cats) + " |")

    print("\n### Exactness\n")
    print("| Stream | Static checks k=4 | k=1 | Peak count, longest window |")
    print("|---|---:|---:|---:|")
    for s in STREAMS:
        a, b = load(s, 4, w), load(s, 1, w)
        print(f"| {s} | {n(a['W'], 'static_checks'):,} | {n(b['W'], 'static_checks'):,} | {n(a['W'], 'peak_count_longest'):,} |")


def sensitivity(widths):
    print("### Sensitivity to slot width\n")
    print("| Stream | " + " | ".join(f"k=4, w={w}" for w in widths) + " | " + " | ".join(f"k=1, w={w}" for w in widths) + " |")
    print("|---|" + "---:|" * (2 * len(widths)))
    for s in STREAMS:
        cells4, cells1 = [], []
        for w in widths:
            a, b = load(s, 4, w), load(s, 1, w)
            cells4.append(f"{n(a['W'], 'ops') / min(n(a['B2_plain'], 'ops'), n(a['B2_recency'], 'ops')):.3f}")
            cells1.append(f"{n(b['W'], 'ops') / min(n(b['P'], 'ops'), n(b['D'], 'ops')):.3f}")
        print(f"| {s} | " + " | ".join(cells4 + cells1) + " |")


if __name__ == "__main__":
    if len(sys.argv) > 2:
        sensitivity(sys.argv[1:])
    else:
        main(sys.argv[1] if len(sys.argv) > 1 else "3600")
