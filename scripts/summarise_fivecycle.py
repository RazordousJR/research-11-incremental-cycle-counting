"""M1C02: join the per-batch files, compare every count, apply the written rule. Protocol: docs/experiments/screen-b-protocol.md.

Usage: python -I scripts/summarise_fivecycle.py [outdir]
Writes fivecycle.csv into outdir and prints the tables. The rule is applied here, mechanically: Stop, then Go, then Pivot.

HS, as the protocol writes it: per-batch minimum of R2, WJ, PR, RC, with no selection cost. A batch that does not pick WJ pays no table
upkeep, so no real selector can reach it. HS-T keeps the table honest: a two-state hindsight over batches, table on or off, where a batch
with the table on pays its upkeep even if it uses another kernel, and turning the table on pays 3 per wedge to rebuild it. Turning it off
is free. HS-T is a true lower bound on any selector that uses these kernels. The rule reads HS. HS-T is reported beside it.
"""
import csv
import glob
import os
import sys

ARMS = ["R2", "WJ", "PR", "RC"]
STREAMS = ["CollegeMsg", "email-Eu-core-temporal", "sx-mathoverflow"]
VARIANTS = ["expiry", "insert"]
CUTS = [100, 1000]
LABEL = {"expiry": "real timestamps, synthetic expiry", "insert": "real timestamps, insert-only, no expiry"}


def load_summary(path):
    with open(path, newline="") as f:
        return next(csv.DictReader(f))


def load_batches(path):
    with open(path, newline="") as f:
        return list(csv.DictReader(f))


def fmt(x):
    if x is None:
        return "n/f"
    return f"{x:,.0f}" if x >= 100 else f"{x:.3g}"


def hs_with_table(b):
    """Two-state hindsight. Rows are dicts with R2, WJ, PR, RC, U (upkeep), W (wedges before the batch)."""
    off = on = None
    for r in b:
        plain = min(r["R2"], r["PR"], r["RC"])
        with_table = min(r["WJ"], plain + r["U"])
        if off is None:
            off, on = plain, with_table
            continue
        off, on = min(off, on) + plain, min(on, off + 3 * r["W"]) + with_table
    return min(off, on)


def analyse(out):
    cells = {}
    failures = []
    for s in STREAMS:
        for v in VARIANTS:
            for c in CUTS:
                key = (s, v, c)
                base = f"{out}/raw/{s}-{v}-{c}"
                cell = {"sum": {}, "rows": {}}
                for a in ["R2", "R2B", "WJ", "PR", "PW", "RC"]:
                    sp, rp = f"{base}-{a}.summary.csv", f"{base}-{a}.csv"
                    if os.path.exists(sp) and os.path.exists(rp):
                        cell["sum"][a] = load_summary(sp)
                        cell["rows"][a] = load_batches(rp)
                cells[key] = cell
                # Every arm against every other, on every batch where the arm knows its count.
                seen = {}
                compared = 0
                for a, rows in cell["rows"].items():
                    for r in rows:
                        if r["known"] != "1":
                            continue
                        i, t = int(r["batch"]), int(r["t"])
                        compared += 1
                        if i in seen and seen[i] != t:
                            failures.append((key, a, i, seen[i], t))
                        seen.setdefault(i, t)
                cell["compared"] = compared
                if cell["rows"]:
                    # R2 and PR run on every batch in a finished cell, so they anchor the comparison.
                    cell["batches"] = max(len(r) for r in cell["rows"].values())
    return cells, failures


def complete(cell, a):
    return a in cell["sum"] and cell["sum"][a]["status"] == "OK"


def score(cells, r2name):
    """Totals per cell for the rule, with r2name in the R2 place. RC with censored batches stays a lower bound."""
    res = {}
    for key, cell in cells.items():
        names = {"R2": r2name, "WJ": "WJ", "PR": "PR", "RC": "RC"}
        if any(n not in cell["rows"] for n in names.values()):
            res[key] = None
            continue
        if not all(complete(cell, names[a]) for a in ["R2", "WJ", "PR"]):
            res[key] = {"unfinished": [a for a in ARMS if not complete(cell, names[a])],
                        "tot": {a: int(cell["sum"][names[a]]["ops"]) for a in ARMS},
                        "done": {a: int(cell["sum"][names[a]]["batches_done"]) for a in ARMS}}
            continue
        rows = {a: cell["rows"][names[a]] for a in ARMS}
        n = len(rows["R2"])
        rc_done = len(rows["RC"])
        hs = 0
        table = []
        wedges_before = 0
        rc_tail = 0
        for i in range(n):
            vals = {a: int(rows[a][i]["ops"]) for a in ["R2", "WJ", "PR"]}
            # An RC batch past a time stop was not run: it counts as dearer than the others, as a censored batch does.
            vals["RC"] = int(rows["RC"][i]["ops"]) if i < rc_done else 4 * max(vals.values())
            if i >= rc_done:
                rc_tail += vals["RC"]
            hs += min(vals.values())
            table.append({**vals, "U": int(rows["WJ"][i]["upkeep"]), "W": wedges_before})
            wedges_before = int(rows["WJ"][i]["wedges"])
        tot = {a: sum(int(r["ops"]) for r in rows[a]) for a in ["R2", "WJ", "PR", "RC"]}
        tot["RC"] += rc_tail
        order = sorted(ARMS, key=lambda a: tot[a])
        best, second = order[0], order[1]
        res[key] = {
            "tot": tot, "best": best, "second": second, "margin": tot[second] / max(tot[best], 1), "hs": hs,
            "hs_ratio": hs / max(tot[best], 1), "hst": hs_with_table(table), "rc_lb": cell["sum"]["RC"]["rc_censored"] != "0" or rc_tail > 0,
        }
        res[key]["hst_ratio"] = res[key]["hst"] / max(tot[best], 1)
    return res


def verdict(res, cells, failures, ratio_key="hs_ratio"):
    lines = []
    finished = {k: r for k, r in res.items() if r and "best" in r}
    unfinished = [k for k in res if k not in finished]
    stop = []
    if failures:
        stop.append(f"exactness: {len(failures)} mismatches")
    wins = {}
    for r in finished.values():
        wins[r["best"]] = wins.get(r["best"], 0) + 1
    for a in ARMS:
        if wins.get(a, 0) == 12 and all(r["margin"] > 2 for r in finished.values()):
            stop.append(f"{a} beats every other by more than 2 on all 12")
    wj_all = wins.get("WJ", 0) == 12
    peak = {}
    for k, cell in cells.items():
        if "WJ" in cell["sum"]:
            peak[k[0]] = max(peak.get(k[0], 0), int(cell["sum"]["WJ"]["table_bytes"]))
    if wj_all and all(p < 10**9 for p in peak.values()):
        stop.append("WJ wins all 12 and its peak table is under 1 GB on every stream")
    top = max(wins.values()) if wins else 0
    low = sum(1 for r in finished.values() if r[ratio_key] <= 0.67)
    lines.append(f"cells scored {len(finished)} of 12, not scored {len(unfinished)}: {unfinished}")
    lines.append(f"wins {wins}, most by one kernel {top}, peak table bytes {peak}")
    lines.append(f"{ratio_key} at most 0.67 on {low} cells")
    if stop:
        lines.append("VERDICT Stop: " + "; ".join(stop))
    elif unfinished:
        lines.append("VERDICT undecided by the scored cells alone: see the unfinished cells")
    elif top <= 9 and low >= 3:
        lines.append("VERDICT Go")
    else:
        lines.append(f"VERDICT Pivot: top wins {top} (Go needs at most 9), low cells {low} (Go needs at least 3)")
    return lines


def main(out):
    cells, failures = analyse(out)
    res = score(cells, "R2")
    print("exactness: failures", len(failures), "comparisons", sum(c.get("compared", 0) for c in cells.values()))
    for f in failures[:10]:
        print("  ", f)

    rows = []
    fields = ["stream", "variant", "label", "cut", "arm", "status", "batches_done", "batches", "events", "net", "ops", "ops_per_net", "scan", "probe", "row",
              "deg", "tab_read", "tab_write", "upkeep", "graph", "net_ops", "group", "rc", "rc_censored", "rc_finished", "forced_checks", "peak_m", "final_t",
              "table_entries", "table_bytes", "table_transient", "max_update", "groups", "spokes", "peak_rss", "seconds"]
    for (s, v, c), cell in cells.items():
        for a, m in cell["sum"].items():
            r = {k: m.get(k, "") for k in fields}
            r["label"] = LABEL[v]
            r["ops_per_net"] = f"{int(m['ops']) / max(int(m['net']), 1):.1f}"
            rows.append(r)
        sc = res.get((s, v, c))
        if sc and "best" in sc:
            for name, val in (("HS", sc["hs"]), ("HS-T", sc["hst"])):
                rows.append({"stream": s, "variant": v, "label": LABEL[v], "cut": c, "arm": name, "status": "derived", "ops": val})
    with open(f"{out}/fivecycle.csv", "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        w.writerows(rows)

    print("\nper cell: operations, best, second / best, HS / best, HS-T / best")
    for k, r in res.items():
        if r is None:
            print(k, "missing")
        elif "best" not in r:
            print(k, "unfinished", r["unfinished"], r["tot"], r["done"])
        else:
            t = r["tot"]
            print(f"{k[0]:24s} {k[1]:6s} {k[2]:5d} R2 {t['R2']:>16,d} WJ {t['WJ']:>16,d} PR {t['PR']:>16,d} RC {t['RC']:>18,d}{'+' if r['rc_lb'] else ' '}"
                  f" best {r['best']:3s} margin {r['margin']:7.3f} HS {r['hs_ratio']:.3f} HS-T {r['hst_ratio']:.3f}")

    print("\nper update, operations per net update")
    for k, cell in cells.items():
        if not cell["sum"]:
            continue
        net = max(int(cell["sum"][a]["net"]) for a in cell["sum"])
        print(f"{k[0]:24s} {k[1]:6s} {k[2]:5d} net {net:8d} " + " ".join(f"{a} {int(m['ops']) / max(int(m['net']), 1):12,.1f}" for a, m in cell["sum"].items()))

    print("\nsharing: PR / R2, PW / WJ, spokes in stars of 2 or more / net")
    for k, cell in cells.items():
        s = cell["sum"]
        if all(a in s for a in ["R2", "PR", "PW", "WJ"]):
            print(f"{k[0]:24s} {k[1]:6s} {k[2]:5d} PR/R2 {int(s['PR']['ops']) / int(s['R2']['ops']):.3f} PW/WJ {int(s['PW']['ops']) / int(s['WJ']['ops']):.3f}"
                  f" spokes/net {int(s['PR']['spokes']) / max(int(s['PR']['net']), 1):.3f}")

    print("\nwall clock, seconds, indicative only, and peak resident bytes")
    for k, cell in cells.items():
        print(f"{k[0]:24s} {k[1]:6s} {k[2]:5d} " + " ".join(f"{a} {float(m['seconds']):7.1f}" for a, m in cell["sum"].items()))

    print("\nRULE, as written")
    print("\n".join(verdict(res, cells, failures)))
    print("\nSENSITIVITY: HS-T in the place of HS")
    print("\n".join(verdict(res, cells, failures, "hst_ratio")))
    if any("R2B" in c["rows"] for c in cells.values()):
        print("\nSENSITIVITY: R2B in the place of R2")
        print("\n".join(verdict(score(cells, "R2B"), cells, failures)))


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "results/screen-c-M1C02")
