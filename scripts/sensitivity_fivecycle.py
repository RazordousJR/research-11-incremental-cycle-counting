"""M1C02: how thin is the verdict? Outside the rule, written after the runs, for reporting only. It changes no count and no threshold.

Usage: python -I scripts/sensitivity_fivecycle.py [outdir]
1. Table size in every form the phrase "peak table" could mean.
2. Hash weight: what if one table read or write costs w operations, not 1? Exact from the totals, per cell.
3. Which kernel is cheapest per batch, and how far HS gets.
4. Coverage of the real-stream checks, and agreement with the Gate 0 record.
5. Hindsight under a heavier table. Per-batch table operations were not logged, so they are estimated from the batch total: half of the join
   and two thirds of the upkeep, scaled in each cell so that they sum to the exact total. That makes this table approximate.
6. The indicative clock per cell.
"""
import csv
import glob
import sys

STREAMS = ["CollegeMsg", "email-Eu-core-temporal", "sx-mathoverflow"]
VARIANTS = ["expiry", "insert"]
CUTS = [100, 1000]
KERNELS = ["R2", "WJ", "PR", "RC"]


def summary(out, s, v, c, a):
    with open(f"{out}/raw/{s}-{v}-{c}-{a}.summary.csv", newline="") as f:
        return next(csv.DictReader(f))


def batches(out, s, v, c, a):
    with open(f"{out}/raw/{s}-{v}-{c}-{a}.csv", newline="") as f:
        return list(csv.DictReader(f))


def main(out):
    print("1. table size, WJ arm, per cell")
    print(f"{'cell':44s} {'entries':>12s} {'at 12 B':>14s} {'allocated':>14s} {'at 16 B':>14s} {'rehash peak':>14s} {'peak resident':>14s} {'wedges':>14s}")
    for s in STREAMS:
        for v in VARIANTS:
            for c in CUTS:
                m = summary(out, s, v, c, "WJ")
                e, slots = int(m["table_entries"]), int(m["table_bytes"]) // 12
                w = max(int(r["wedges"]) for r in batches(out, s, v, c, "WJ"))
                print(f"{s + ' ' + v + ' ' + str(c):44s} {e:>12,d} {e * 12:>14,d} {slots * 12:>14,d} {slots * 16:>14,d} {int(m['table_transient']):>14,d} {int(m['peak_rss']):>14,d} {w:>14,d}")

    print("\n2. hash weight: WJ total with table operations at w each. Best other = cheapest of R2, PR, RC")
    print("   break-even w is where WJ stops being cheapest. wins(w) counts cells WJ still wins")
    weights = [1, 1.25, 1.5, 2, 3, 4, 8, 16, 33]
    wins = {w: 0 for w in weights}
    for s in STREAMS:
        for v in VARIANTS:
            for c in CUTS:
                m = {a: summary(out, s, v, c, a) for a in KERNELS}
                ops = {a: int(m[a]["ops"]) for a in KERNELS}
                tab = int(m["WJ"]["tab_read"]) + int(m["WJ"]["tab_write"])
                rest = ops["WJ"] - tab
                other = min(ops[a] for a in ["R2", "PR", "RC"])
                who = min(["R2", "PR", "RC"], key=lambda a: ops[a])
                be = (other - rest) / tab
                for w in weights:
                    wins[w] += rest + w * tab < other
                print(f"{s + ' ' + v + ' ' + str(c):44s} table share of WJ {tab / ops['WJ']:.3f}  best other {who} {other:>16,d}  break-even w {be:6.2f}")
    print("   " + "  ".join(f"w={w}: {wins[w]}" for w in weights))
    print("   RC totals with censored batches are lower bounds, so break-even values against RC can only be higher")

    print("\n3. per-batch cheapest kernel, share of batches with any net update, and of operations under HS")
    for s in STREAMS:
        for v in VARIANTS:
            for c in CUTS:
                rows = {a: batches(out, s, v, c, a) for a in KERNELS}
                n = len(rows["R2"])
                won = {a: 0 for a in KERNELS}
                live = 0
                for i in range(n):
                    if int(rows["R2"][i]["net"]) == 0:
                        continue
                    live += 1
                    vals = {a: int(rows[a][i]["ops"]) for a in KERNELS}
                    won[min(KERNELS, key=lambda a: vals[a])] += 1
                print(f"{s + ' ' + v + ' ' + str(c):44s} batches {live:6d}  " + "  ".join(f"{a} {won[a] / max(live, 1):.3f}" for a in KERNELS))

    print("\n4. wall clock per million operations, indicative only, shared machine")
    for s, v, c in [("sx-mathoverflow", "insert", 100), ("sx-mathoverflow", "insert", 1000)]:
        line = []
        for a in ["R2", "R2B", "PR", "WJ", "PW", "RC"]:
            m = summary(out, s, v, c, a)
            line.append(f"{a} {float(m['seconds']):.1f} s, {1e6 * float(m['seconds']) / int(m['ops']):.4f} s/Mop")
        print(f"{s} {v} {c}: " + "; ".join(line))

    print("\n4b. hindsight at table weight w, approximate: HS(w) / best(w), and the kernel that is best at w")
    wset = [1, 2, 3, 4, 8]
    low = {w: 0 for w in wset}
    tops = {w: {} for w in wset}
    for s in STREAMS:
        for v in VARIANTS:
            for c in CUTS:
                rows = {a: batches(out, s, v, c, a) for a in KERNELS}
                m = summary(out, s, v, c, "WJ")
                exact = int(m["tab_read"]) + int(m["tab_write"])
                est = []
                for r in rows["WJ"]:
                    join = int(r["ops"]) - 2 * int(r["raw"]) - 2 * int(r["net"]) - int(r["upkeep"])
                    est.append(join / 2 + 2 * int(r["upkeep"]) / 3)
                k = exact / max(sum(est), 1)
                est = [e * k for e in est]
                parts = []
                for w in wset:
                    tot = {a: sum(int(r["ops"]) for r in rows[a]) for a in ["R2", "PR", "RC"]}
                    tot["WJ"] = sum(int(r["ops"]) + (w - 1) * e for r, e in zip(rows["WJ"], est))
                    hs = 0.0
                    for i in range(len(rows["R2"])):
                        hs += min(int(rows["R2"][i]["ops"]), int(rows["PR"][i]["ops"]), int(rows["RC"][i]["ops"]), int(rows["WJ"][i]["ops"]) + (w - 1) * est[i])
                    best = min(tot, key=tot.get)
                    ratio = hs / tot[best]
                    low[w] += ratio <= 0.67
                    tops[w][best] = tops[w].get(best, 0) + 1
                    parts.append(f"w={w} {best} {ratio:.3f}")
                print(f"{s + ' ' + v + ' ' + str(c):44s} " + "  ".join(parts))
    for w in wset:
        print(f"   w={w}: best-kernel wins {tops[w]}, cells with HS(w) / best(w) at most 0.67: {low[w]}")

    print("\n4c. indicative clock, seconds, R2, PR, WJ, and the fastest of them. RC is left out, because its budget stops recounts early")
    wins = {}
    for s in STREAMS:
        for v in VARIANTS:
            for c in CUTS:
                t = {a: float(summary(out, s, v, c, a)["seconds"]) for a in ["R2", "PR", "WJ"]}
                who = min(t, key=t.get)
                wins[who] = wins.get(who, 0) + 1
                print(f"{s + ' ' + v + ' ' + str(c):44s} R2 {t['R2']:7.3f} PR {t['PR']:7.3f} WJ {t['WJ']:7.3f}  fastest {who}")
    print("   fastest per cell:", wins)

    print("\n5. coverage of the real-stream checks")
    tot_known = 0
    for s in STREAMS:
        for v in VARIANTS:
            for c in CUTS:
                rows = batches(out, s, v, c, "RC")
                known = sum(r["known"] == "1" for r in rows)
                forced = sum(r["checked"] == "1" for r in rows)
                tot_known += known
                print(f"{s + ' ' + v + ' ' + str(c):44s} batches {len(rows):6d}  RC recount known on {known:6d}, of which forced {forced:3d}, last batch known {rows[-1]['known']}, final t {rows[-1]['t']}")
    print("batches with a static recount in agreement:", tot_known)

    print("\n6. against the Gate 0 record, same streams at the same cuts. Gate 0 is the expiry variant")
    for path in sorted(glob.glob("results/gate0-*/*-count-*.csv")):
        with open(path, newline="") as f:
            r = next(csv.DictReader(f))
        s, c = r["stream"], int(r["cut"])
        if s in STREAMS and c in CUTS:
            m = summary(out, s, "expiry", c, "R2")
            print(f"{s} cut {c}: Gate 0 net {r['net']} peak_m {r['peak_m']} batches {r['batches']}; here net {m['net']} peak_m {m['peak_m']} batches {m['batches']}")


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "results/screen-c-M1C02")
