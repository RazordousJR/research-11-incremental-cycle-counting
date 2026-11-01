"""Tables for M1C01 from results/screen-c-M1C01/*.csv. Usage: python -I scripts/summarise_window_c.py

Prints markdown. The verdict lines apply the rule of docs/experiments/screen-b-protocol.md, section M1C01, and nothing else.
The ring sensitivity at the end is outside the rule.
"""
import csv

STREAMS = ["CollegeMsg", "email-Eu-core-temporal", "sx-mathoverflow", "sx-askubuntu", "sx-superuser", "wiki-talk-temporal"]
WIDTHS = ["3600", "1"]
NEW = "results/screen-c-M1C01"
OLD = "results/screen-b-M1B01-B"
FIELD = ["W", "W_F", "V", "K", "DI", "B1", "B2_plain", "B2_recency"]
NAMES = {"W": "W", "W_F": "W-F", "V": "V", "K": "K", "DI": "DI", "B1": "B1", "B2_plain": "B2 plain", "B2_recency": "B2 recency", "D_k": "D-k"}
DAYS = [1, 7, 30, 90]


def load(root, stream, w):
    with open(f"{root}/{stream}-k4-w{w}.csv") as f:
        return {r["method"]: r for r in csv.DictReader(f)}


def n(row, col):
    return int(float(row[col]))


def regression():
    bad, cells, arms = 0, 0, set()
    for s in STREAMS:
        for w in WIDTHS:
            a, b = load(NEW, s, w), load(OLD, s, w)
            for m in b:
                arms.add(m)
                for c in b[m]:
                    cells += 1
                    if a[m][c] != b[m][c]:
                        bad += 1
                        print(f"DIFF {s} w={w} {m} {c}: {b[m][c]} -> {a[m][c]}")
    print(f"Old arms {sorted(arms)} against the M1B01-B CSVs, every column: {bad} differences in {cells} cells.\n")
    return bad


def dominates(a, d):
    oa, ob, wa, wb = n(a, "ops"), n(d, "ops"), n(a, "peak_words"), n(d, "peak_words")
    return oa <= ob and wa <= wb and (oa < ob or wa < wb)


def ops_table(w):
    print(f"### Operations, slot width {w} s\n")
    print("| Stream | D-k | " + " | ".join(NAMES[m] for m in FIELD) + " |")
    print("|---|---:|" + "---:|" * len(FIELD))
    for s in STREAMS:
        a = load(NEW, s, w)
        print(f"| {s} | {n(a['D_k'], 'ops'):,} | " + " | ".join(f"{n(a[m], 'ops'):,}" for m in FIELD) + " |")
    print()


def ratio_table(w):
    print(f"### Each arm over D-k, operations, slot width {w} s\n")
    print("| Stream | " + " | ".join(NAMES[m] for m in FIELD) + " |")
    print("|---|" + "---:|" * len(FIELD))
    for s in STREAMS:
        a = load(NEW, s, w)
        d = n(a["D_k"], "ops")
        print(f"| {s} | " + " | ".join(f"{n(a[m], 'ops') / d:.3f}" for m in FIELD) + " |")
    print()


def words_table(w):
    print(f"### Peak words, slot width {w} s\n")
    print("| Stream | D-k | " + " | ".join(NAMES[m] for m in FIELD) + " |")
    print("|---|---:|" + "---:|" * len(FIELD))
    for s in STREAMS:
        a = load(NEW, s, w)
        print(f"| {s} | {n(a['D_k'], 'peak_words'):,} | " + " | ".join(f"{n(a[m], 'peak_words'):,}" for m in FIELD) + " |")
    print()
    print(f"### Each arm over D-k, peak words, slot width {w} s\n")
    print("| Stream | " + " | ".join(NAMES[m] for m in FIELD) + " |")
    print("|---|" + "---:|" * len(FIELD))
    for s in STREAMS:
        a = load(NEW, s, w)
        d = n(a["D_k"], "peak_words")
        print(f"| {s} | " + " | ".join(f"{n(a[m], 'peak_words') / d:.3f}" for m in FIELD) + " |")
    print()


def dominance(w):
    print(f"### Dominance of D-k, slot width {w} s\n")
    print("| Stream | Arms that dominate D-k | Dominated | DI / D-k | K / D-k | V / D-k |")
    print("|---|---|---|---:|---:|---:|")
    by_arm = {m: 0 for m in FIELD}
    free, di_ok = 0, 0
    for s in STREAMS:
        a = load(NEW, s, w)
        hit = [m for m in FIELD if dominates(a[m], a["D_k"])]
        for m in hit:
            by_arm[m] += 1
        free += not hit
        d = n(a["D_k"], "ops")
        di = n(a["DI"], "ops") / d
        di_ok += di >= 1.25
        print(f"| {s} | {', '.join(NAMES[m] for m in hit) or 'none'} | {'yes' if hit else 'no'} | {di:.3f} | "
              f"{n(a['K'], 'ops') / d:.3f} | {n(a['V'], 'ops') / d:.3f} |")
    stop = any(v == 6 for v in by_arm.values())
    go = free >= 4 and di_ok >= 3
    print(f"\nStreams dominated, by arm: {', '.join(f'{NAMES[m]} {v}' for m, v in by_arm.items())}. Streams not dominated: {free} of 6. "
          f"DI / D-k at least 1.25: {di_ok} of 6. Stop clause at this width: {stop}. Go clause at this width: {go}.\n")
    return stop, go


def rehome(w):
    print(f"### Share of D-k that is re-homing, measured, slot width {w} s\n")
    print("| Stream | Insertion scans | Re-home at refresh | Re-home at stale pop | Share of scans | Share of list elements | "
          "Share of all D-k units | Elements per insertion scan | Elements per re-home scan |")
    print("|---|---:|---:|---:|---:|---:|---:|---:|---:|")
    for s in STREAMS:
        d = load(NEW, s, w)["D_k"]
        first, now, stale = n(d, "firsts"), n(d, "scans_rehome_now"), n(d, "scans_rehome_stale")
        ei, er = n(d, "elems_insert"), n(d, "elems_rehome")
        print(f"| {s} | {first:,} | {now:,} | {stale:,} | {(now + stale) / (first + now + stale):.1%} | {er / (ei + er):.1%} | "
              f"{n(d, 'units_rehome') / n(d, 'ops'):.1%} | {ei / max(first, 1):.1f} | {er / max(now + stale, 1):.1f} |")
    print()


def view_work(w):
    print(f"### V, slot width {w} s\n")
    print("| Stream | Peak records | Records born | Moved | Repeats | Scan steps | view | V scan share | V view share | Records per first pair |")
    print("|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|")
    for s in STREAMS:
        v = load(NEW, s, w)["V"]
        t = n(v, "ops")
        print(f"| {s} | {n(v, 'peak_records'):,} | {n(v, 'added'):,} | {n(v, 'moved'):,} | {n(v, 'repeats'):,} | {n(v, 'scan'):,} | {n(v, 'view'):,} | "
              f"{n(v, 'scan') / t:.1%} | {n(v, 'view') / t:.1%} | {n(v, 'added') / max(n(v, 'firsts'), 1):.2f} |")
    print()


def scans(w):
    print(f"### Scans per event, slot width {w} s\n")
    print("| Stream | D-k | K | DI | Better B2 | Repeats per event |")
    print("|---|---:|---:|---:|---:|---:|")
    for s in STREAMS:
        a = load(NEW, s, w)
        e = n(a["W"], "events")
        f = lambda r: (n(r, "scans_event") + n(r, "scans_expiry")) / e
        best = min(a["B2_plain"], a["B2_recency"], key=lambda r: n(r, "ops"))
        print(f"| {s} | {f(a['D_k']):.3f} | {f(a['K']):.3f} | {f(a['DI']):.3f} | {f(best):.3f} | {n(a['W'], 'repeats') / e:.3f} |")
    print()


def categories(w):
    cats = ["scan", "probe", "read", "field", "edit", "slot", "recency", "queue", "count", "view"]
    print(f"### Cost by category, slot width {w} s, share of each total\n")
    print("| Stream | Method | " + " | ".join(cats) + " |")
    print("|---|---|" + "---:|" * len(cats))
    for s in STREAMS:
        a = load(NEW, s, w)
        for m in ("D_k", "V", "K", "DI"):
            t = n(a[m], "ops")
            print(f"| {s} | {NAMES[m]} | " + " | ".join(f"{n(a[m], c) / t:.1%}" for c in cats) + " |")
    print()


def rings(w):
    """Peak words with the fixed arrays over slots taken out of every arm. Outside the rule."""
    ww = int(w)
    ls = [d * 86400 // ww for d in DAYS]
    lmax = ls[-1]
    all_rings = 3.0 * sum(x + 1 for x in ls)
    one_ring = 3.0 * (lmax + 1)
    tree = 1
    while tree < lmax + 1:
        tree *= 2
    fixed = {"D_k": all_rings, "K": all_rings, "DI": all_rings, "B2_plain": all_rings, "B2_recency": all_rings, "B1": all_rings,
             "W": one_ring + lmax + 1, "V": one_ring + lmax + 1, "W_F": one_ring + 2.0 * tree}
    print(f"### Sensitivity, outside the rule: fixed arrays over slots taken out, slot width {w} s\n")
    print(f"Token ring headers and dense count arrays are constants of the slot width. Taken out here: {all_rings:,.0f} words for the four "
          f"ring arms, {one_ring + lmax + 1:,.0f} for W and V, {one_ring + 2.0 * tree:,.0f} for W-F. A real replacement would cost something per "
          "distinct slot, so this is a bound in the other direction, not a design.\n")
    print("| Stream | D-k | V | W | Arms that would dominate D-k |")
    print("|---|---:|---:|---:|---|")
    for s in STREAMS:
        a = load(NEW, s, w)
        adj = {m: n(a[m], "peak_words") - fixed[m] for m in fixed}
        d, dw = n(a["D_k"], "ops"), adj["D_k"]
        hit = [m for m in FIELD if n(a[m], "ops") <= d and adj[m] <= dw and (n(a[m], "ops") < d or adj[m] < dw)]
        print(f"| {s} | {adj['D_k']:,.0f} | {adj['V']:,.0f} | {adj['W']:,.0f} | {', '.join(NAMES[m] for m in hit) or 'none'} |")
    print()


def crosscheck():
    bad = 0
    for s in STREAMS:
        for w in WIDTHS:
            a = load(NEW, s, w)
            bad += n(a["V"], "added") != n(a["W"], "added")
            bad += n(a["V"], "moved") != n(a["W"], "moved")
            bad += n(a["V"], "peak_records") != n(a["V"], "peak_count_longest")
    print(f"V against W, records born and units moved, and peak records against the peak count: {bad} differences.\n")


def margin(w):
    """How far V's charge per record would have to fall to reach D-k on operations. Arithmetic on the CSVs, outside the rule."""
    print(f"### Margin, outside the rule: V's charge per record, slot width {w} s\n")
    print("| Stream | V units per record, list work | Units per record to reach D-k | Share of that charge | V words / D-k | V would then dominate |")
    print("|---|---:|---:|---:|---:|---|")
    for s in STREAMS:
        a = load(NEW, s, w)
        v, d = a["V"], a["D_k"]
        born = n(v, "added")
        gap = n(v, "ops") - n(d, "ops")
        per = n(v, "view") / born
        need = max(gap, 0) / born
        wr = n(v, "peak_words") / n(d, "peak_words")
        print(f"| {s} | {per:.1f} | {need:.2f} | {need / per:.1%} | {wr:.3f} | {'yes' if wr <= 1 else 'no, words'} |")
    print()


def other_forms(w):
    """Estimates from the CSVs, not arms. K with a shared home, and DI with one merged scan per refresh."""
    print(f"### Other forms, estimated and not built, slot width {w} s\n")
    print("K shared home: D-k plus (k - 1) more writes for every counter write, an upper bound. DI one scan: the refresh scans of DI counted "
          "once, taken as (DI + D-k without re-home units) / 2.\n")
    print("| Stream | K as built / D-k | K shared home / D-k, at most | DI as built / D-k | DI one scan / D-k, estimate |")
    print("|---|---:|---:|---:|---:|")
    low = 0
    for s in STREAMS:
        a = load(NEW, s, w)
        d = n(a["D_k"], "ops")
        ks = (d + 3 * n(a["D_k"], "slot")) / d
        base = d - n(a["D_k"], "units_rehome")
        one = (n(a["DI"], "ops") + base) / 2 / d
        low += one >= 1.25
        print(f"| {s} | {n(a['K'], 'ops') / d:.3f} | {ks:.3f} | {n(a['DI'], 'ops') / d:.3f} | {one:.3f} |")
    print(f"\nDI one scan at least 1.25: {low} of 6.\n")


def exact():
    print("### Static checks\n")
    print("| Stream | w=3600 | w=1 |")
    print("|---|---:|---:|")
    for s in STREAMS:
        print(f"| {s} | {n(load(NEW, s, '3600')['W'], 'static_checks'):,} | {n(load(NEW, s, '1')['W'], 'static_checks'):,} |")
    print()


if __name__ == "__main__":
    regression()
    res = {}
    for w in WIDTHS:
        ops_table(w)
        ratio_table(w)
        words_table(w)
        res[w] = dominance(w)
    stop = all(res[w][0] for w in WIDTHS)
    go = all(res[w][1] for w in WIDTHS)
    print(f"Rule: Stop at both widths: {stop}. Go at both widths: {go}. Verdict: {'Stop' if stop else 'Go' if go else 'Pivot'}.\n")
    crosscheck()
    for w in WIDTHS:
        rehome(w)
        view_work(w)
        scans(w)
        rings(w)
        margin(w)
        other_forms(w)
    categories("3600")
    categories("1")
    exact()
