"""Tables for M1B01-B from results/screen-b-M1B01-B/*.csv. Usage: python -I scripts/summarise_window_b.py

Prints markdown. The verdict lines apply the rule of docs/experiments/screen-b-protocol.md, section M1B01-B, and nothing else.
"""
import csv

STREAMS = ["CollegeMsg", "email-Eu-core-temporal", "sx-mathoverflow", "sx-askubuntu", "sx-superuser", "wiki-talk-temporal"]
WIDTHS = ["3600", "1"]
NEW = "results/screen-b-M1B01-B"
OLD = "results/screen-b-M1B01"


def load(root, stream, w):
    with open(f"{root}/{stream}-k4-w{w}.csv") as f:
        return {r["method"]: r for r in csv.DictReader(f)}


def n(row, col):
    return int(float(row[col]))


def regression():
    bad = 0
    for s in STREAMS:
        for w in WIDTHS:
            a, b = load(NEW, s, w), load(OLD, s, w)
            for m in b:
                for c in ("ops", "scan", "probe", "read", "field", "edit", "slot", "recency", "queue", "count", "peak_words"):
                    bad += a[m][c] != b[m][c]
    print(f"Old arms against the M1B01 CSVs, every category: {bad} differences.\n")


def table(w):
    print(f"### Slot width {w} s\n")
    print("| Stream | B1 | Better B2 | best | W | W-F | W-F, sums form | D-k | W-F / best | D-k / best | min / best | W-F / D-k | W-F sums / D-k |")
    print("|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|")
    ms, caps, caps_a = {}, {}, {}
    for s in STREAMS:
        a = load(NEW, s, w)
        b1 = n(a["B1"], "ops")
        b2 = min(n(a["B2_plain"], "ops"), n(a["B2_recency"], "ops"))
        best = min(b1, b2)
        W, WF, DK = n(a["W"], "ops"), n(a["W_F"], "ops"), n(a["D_k"], "ops")
        sums = W + n(a["W_F"], "fen")
        ms[s], caps[s], caps_a[s] = min(WF, DK) / best, WF / DK, sums / DK
        print(f"| {s} | {b1:,} | {b2:,} | {best:,} | {W:,} | {WF:,} | {sums:,} | {DK:,} | {WF / best:.3f} | {DK / best:.3f} | "
              f"**{ms[s]:.3f}** | **{caps[s]:.3f}** | {caps_a[s]:.3f} |")
    print(f"\nStop holds at this width: {all(v > 0.80 for v in ms.values())}. Streams at most 0.67: {sum(v <= 0.67 for v in ms.values())}. "
          f"Capability, W-F / D-k at most 1.5: {sum(v <= 1.5 for v in caps.values())} streams; sums form: {sum(v <= 1.5 for v in caps_a.values())}.\n")
    return ms


def work(w):
    print(f"### Work and memory, slot width {w} s\n")
    print("| Stream | Peak words W-F | D-k | B2 plain | B2 recency | fen per event | query per event | D-k scans per event | "
          "Better B2 scans per event | D-k immediate re-home scans per event | D-k stale-pop scans per event | Re-homed per event |")
    print("|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|")
    for s in STREAMS:
        a = load(NEW, s, w)
        e = n(a["W"], "events")
        f, d = a["W_F"], a["D_k"]
        best = min(a["B2_plain"], a["B2_recency"], key=lambda r: n(r, "ops"))
        print(f"| {s} | {n(f, 'peak_words'):,} | {n(d, 'peak_words'):,} | "
              f"{n(a['B2_plain'], 'peak_words'):,} | {n(a['B2_recency'], 'peak_words'):,} | {n(f, 'fen') / e:.1f} | {n(f, 'query') / e:.1f} | "
              f"{(n(d, 'scans_event') + n(d, 'scans_expiry')) / e:.3f} | {(n(best, 'scans_event') + n(best, 'scans_expiry')) / e:.3f} | "
              f"{(n(d, 'scans_event') - n(d, 'firsts')) / e:.3f} | {n(d, 'scans_expiry') / e:.3f} | {n(d, 'rehomed') / e:.3f} |")
    print()


def breakeven():
    print("### Sums form: ad hoc reads per event before W-F / D-k passes 1.5\n")
    print("A read is two prefixes. Units per prefix are estimated as query per event over k + 1. Negative means already past 1.5.\n")
    print("| Stream | w=3600 | w=1 | W / D-k, w=3600 | W / D-k, w=1 |")
    print("|---|---:|---:|---:|---:|")
    for s in STREAMS:
        cells, ratio = [], []
        for w in WIDTHS:
            a = load(NEW, s, w)
            e = n(a["W"], "events")
            sums = n(a["W"], "ops") + n(a["W_F"], "fen")
            per_read = 2 * n(a["W_F"], "query") / e / 5
            cells.append(f"{(1.5 * n(a['D_k'], 'ops') - sums) / e / per_read:.2f}")
            ratio.append(f"{n(a['W'], 'ops') / n(a['D_k'], 'ops'):.3f}")
        print(f"| {s} | {cells[0]} | {cells[1]} | {ratio[0]} | {ratio[1]} |")
    print()


def categories(w):
    cats = ["scan", "probe", "read", "field", "edit", "slot", "recency", "queue", "count", "fen", "query"]
    print(f"### Cost by category, slot width {w} s, share of each total\n")
    print("| Stream | Method | " + " | ".join(cats) + " |")
    print("|---|---|" + "---:|" * len(cats))
    for s in STREAMS:
        a = load(NEW, s, w)
        for m in ("W_F", "D_k"):
            t = n(a[m], "ops")
            print(f"| {s} | {m} | " + " | ".join(f"{n(a[m], c) / t:.1%}" for c in cats) + " |")
    print()


def exact():
    print("### Static checks\n")
    print("| Stream | w=3600 | w=1 |")
    print("|---|---:|---:|")
    for s in STREAMS:
        print(f"| {s} | {n(load(NEW, s, '3600')['W'], 'static_checks'):,} | {n(load(NEW, s, '1')['W'], 'static_checks'):,} |")
    print()


if __name__ == "__main__":
    regression()
    m = {w: table(w) for w in WIDTHS}
    stop = all(all(v > 0.80 for v in m[w].values()) for w in WIDTHS)
    go = all(sum(v <= 0.67 for v in m[w].values()) >= 3 for w in WIDTHS)
    print(f"Rule: Stop at both widths: {stop}. Go at both widths: {go}.\n")
    for w in WIDTHS:
        work(w)
    breakeven()
    categories("3600")
    exact()
