"""M1C03: apply the written rule to the timing runs. Protocol: docs/experiments/screen-b-protocol.md, M1C03.

Usage: python -I scripts/summarise_m1c03.py [outdir]
Writes m1c03.csv and hubs.csv into outdir and prints the tables. Stop is read first, then Go, and Pivot is anything else.

READINGS FIXED BEFORE THE FIRST STREAM. The protocol leaves these open. Each other reading is printed beside the verdict.
- Finished: an arm is finished on a cell when at least 2 of its 3 timing runs finished. A run that does not finish counts as infinite in the
  median. An arm that does not finish its first run is not run again, and it is not finished.
- Scored: a cell is scored when R2 or PR finished and at least one of R2, WJ, WT finished. Score = PR / best, best = fastest of R2, WJ, WT.
  If PR did not finish on a scored cell, the score is infinite: not finished is not a win.
- "Fastest overall": fastest of all five timed arms, R2, PR, WJ, WT, PW. The other reading leaves PW out.
- "WJ or WT is fastest overall on more than half": the cells won by WJ and the cells won by WT, added. This is the reading that stops most easily.
  The other reading takes each arm alone.
- A tie at the printed precision does not arise, since the comparison is on the medians as measured, to 1 ms.
"""
import csv
import glob
import math
import os
import statistics
import struct
import sys

STREAMS = ["CollegeMsg", "email-Eu-core-temporal", "sx-mathoverflow", "sx-askubuntu", "sx-superuser", "wiki-talk-temporal"]
VARIANTS = ["expiry", "insert"]
CUTS = [100, 1000]
ARMS = ["R2", "PR", "WJ", "WT", "PW"]
LABEL = {"expiry": "real timestamps, synthetic expiry", "insert": "real timestamps, insert-only, no expiry"}
INF = math.inf


def load(path):
    with open(path, newline="") as f:
        return list(csv.DictReader(f))


def cell_data(out, s, v, c):
    base = f"{out}/raw/{s}-{v}-{c}-"
    arms = {}
    for a in ARMS:
        reps = []
        for r in (1, 2, 3):
            p = f"{base}{a}.t{r}.summary.csv"
            if os.path.exists(p):
                m = load(p)[0]
                reps.append({"status": m["status"], "secs": float(m["seconds"]), "rss": int(m["peak_rss"]), "done": int(m["batches_done"]),
                             "batches": int(m["batches"]), "bytes": int(m["table_bytes"]), "transient": int(m["table_transient"]),
                             "entries": int(m["table_entries"])})
        ok = [x["secs"] for x in reps if x["status"] == "OK"]
        vals = sorted(ok + [INF] * (3 - len(ok)))
        med = vals[1]
        info = {"reps": reps, "ok": ok, "median": med, "finished": med < INF}
        info["why"] = next((x["status"] for x in reps if x["status"] != "OK"), "none" if not reps else "OK")
        info["lo"] = min(ok) if ok else INF
        info["hi"] = max(ok) if ok else INF
        info["spread"] = (max(ok) - min(ok)) / med if len(ok) >= 2 and med < INF and med > 0 else None
        cp = f"{base}{a}.summary.csv"
        info["count"] = load(cp)[0] if os.path.exists(cp) else None
        arms[a] = info
    return arms


def exactness(out, cells):
    bad = comps = 0
    for (s, v, c) in cells:
        seen = {}
        for path in sorted(glob.glob(f"{out}/raw/{s}-{v}-{c}-*.csv")):
            if path.endswith(".summary.csv"):
                continue
            rows = load(path)
            # A table that hits its cap drops writes for the rest of that batch, so the last row of a CAP record is not a count. See README, Process notes.
            if load(path[:-4] + ".summary.csv")[0]["status"] == "CAP":
                rows = rows[:-1]
            for r in rows:
                i, t = int(r["batch"]), r["t"]
                comps += 1
                if i in seen and seen[i] != t:
                    bad += 1
                seen.setdefault(i, t)
    return bad, comps


def score_cell(arms, pick=lambda a: a["median"]):
    """Rule quantities for a cell, with pick giving the time of an arm. None when the cell is not scored."""
    t = {a: pick(arms[a]) if arms[a]["finished"] else INF for a in ARMS}
    if t["R2"] == INF and t["PR"] == INF:
        return None
    best = min(t["R2"], t["WJ"], t["WT"])
    if best == INF:
        return None
    return {"t": t, "best": best, "score": t["PR"] / best, "pw": t["PW"] / best, "win5": min(ARMS, key=lambda a: t[a]),
            "win4": min(["R2", "PR", "WJ", "WT"], key=lambda a: t[a]), "bestarm": min(["R2", "WJ", "WT"], key=lambda a: t[a])}


def verdict(res, failures, key="win5", combined=True):
    scored = {k: r for k, r in res.items() if r}
    n = len(scored)
    tab = {"n": n}
    wins = {}
    for r in scored.values():
        wins[r[key]] = wins.get(r[key], 0) + 1
    tab["wins"] = wins
    table_wins = wins.get("WJ", 0) + wins.get("WT", 0) if combined else max(wins.get("WJ", 0), wins.get("WT", 0))
    tab["table_wins"] = table_wins
    stop = []
    if failures:
        stop.append(f"exactness: {failures} mismatches")
    if n < 16:
        stop.append(f"only {n} cells scored")
    if n and table_wins * 2 > n:
        stop.append(f"WJ or WT fastest on {table_wins} of {n} scored cells")
    if n and all(r["score"] > 0.90 for r in scored.values()):
        stop.append("score above 0.90 on every scored cell")
    low = {k: r for k, r in scored.items() if r["score"] <= 0.67}
    streams = {k[0] for k in low}
    exp = sum(1 for k in low if k[1] == "expiry")
    tab.update(low=len(low), streams=len(streams), expiry=exp)
    go = n > 0 and 3 * len(low) >= n and len(streams) >= 3 and exp >= 2
    tab["word"] = "Stop" if stop else "Go" if go else "Pivot"
    tab["stop"] = stop
    return tab


def show(t):
    return f"{t['word']}: scored {t['n']}, wins {t['wins']}, table wins {t['table_wins']}, score <= 0.67 on {t['low']} cells, {t['streams']} streams, {t['expiry']} expiry" + (
        f"; stop clauses {t['stop']}" if t["stop"] else "")


def hubs(out):
    """Measured from the streams and not from any arm: at the moment of most wedges, the wedges per edge and the largest degree."""
    path = f"{out}/hubs.csv"
    rows = {}
    if os.path.exists(path):
        for r in load(path):
            rows[(r["stream"], r["variant"])] = r
        return rows
    for s in STREAMS:
        for v in VARIANTS:
            f = f"data/derived/{s}.ins.bin" if v == "insert" else f"data/derived/{s}.bin"
            if not os.path.exists(f):
                continue
            deg = {}
            live = set()
            wedges = peak = peak_m = maxd = 0
            maxd_at_peak = m_at_peak = 0
            with open(f, "rb") as h:
                data = h.read()
            for _, u, w, op in struct.iter_unpack("<qIII", data):
                if u == w:
                    continue
                k = (u, w) if u < w else (w, u)
                if op == 1:
                    if k in live:
                        continue
                    live.add(k)
                    for x in k:
                        d = deg.get(x, 0)
                        wedges += d
                        deg[x] = d + 1
                        if d + 1 > maxd:
                            maxd = d + 1
                else:
                    if k not in live:
                        continue
                    live.discard(k)
                    for x in k:
                        d = deg[x] - 1
                        wedges -= d
                        deg[x] = d
                if wedges > peak:
                    peak, m_at_peak = wedges, len(live)
                peak_m = max(peak_m, len(live))
            r = {"stream": s, "variant": v, "peak_wedges": peak, "edges_at_peak": m_at_peak, "peak_m": peak_m,
                 "wedges_per_edge": f"{peak / max(m_at_peak, 1):.1f}", "max_degree": maxd}
            rows[(s, v)] = r
    with open(path, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=["stream", "variant", "peak_wedges", "edges_at_peak", "peak_m", "wedges_per_edge", "max_degree"])
        w.writeheader()
        w.writerows(rows.values())
    return rows


def rank(xs):
    order = sorted(range(len(xs)), key=lambda i: xs[i])
    r = [0.0] * len(xs)
    i = 0
    while i < len(order):
        j = i
        while j + 1 < len(order) and xs[order[j + 1]] == xs[order[i]]:
            j += 1
        for k in range(i, j + 1):
            r[order[k]] = (i + j) / 2
        i = j + 1
    return r


def spearman(x, y):
    if len(x) < 3:
        return None
    a, b = rank(x), rank(y)
    ma, mb = statistics.mean(a), statistics.mean(b)
    num = sum((p - ma) * (q - mb) for p, q in zip(a, b))
    den = math.sqrt(sum((p - ma) ** 2 for p in a) * sum((q - mb) ** 2 for q in b))
    return num / den if den else None


def fs(x):
    return "n/f" if x == INF else f"{x:.3f}"


def fr(x):
    return "n/f" if x == INF else f"{x:.3f}"


def mb(x):
    return f"{x / 1e6:,.1f}"


def main(out):
    keys = [(s, v, c) for s in STREAMS for v in VARIANTS for c in CUTS]
    cells = {k: cell_data(out, *k) for k in keys}
    bad, comps = exactness(out, keys)
    print("exactness: mismatches", bad, "comparisons", comps)

    res = {k: score_cell(a) for k, a in cells.items()}
    hub = hubs(out)

    print("\nper cell, median seconds of 3, n/f = not finished. Score = PR / best of R2, WJ, WT")
    for k, a in cells.items():
        r = res[k]
        ts = " ".join(f"{x} {fs(a[x]['median']):>9s}" for x in ARMS)
        if r:
            print(f"{k[0]:24s} {k[1]:6s} {k[2]:5d} {ts} | win {r['win5']:3s} best {r['bestarm']:3s} score {fr(r['score']):>7s} PW/best {fr(r['pw']):>7s}")
        else:
            print(f"{k[0]:24s} {k[1]:6s} {k[2]:5d} {ts} | NOT SCORED")

    print("\nnot finished, by reason")
    for k, a in cells.items():
        for x in ARMS:
            if not a[x]["finished"]:
                reps = a[x]["reps"]
                d = f"{reps[0]['done']}/{reps[0]['batches']} batches" if reps else "not run"
                print(f"{k[0]:24s} {k[1]:6s} {k[2]:5d} {x:3s} {a[x]['why']:5s} {d}")

    print("\nnoise, (max - min) / median of the 3 runs, per arm")
    all_spread = {x: [a[x]["spread"] for a in cells.values() if a[x]["spread"] is not None] for x in ARMS}
    for x in ARMS:
        v = all_spread[x]
        if v:
            print(f"{x:3s} median {statistics.median(v) * 100:6.2f}%  max {max(v) * 100:6.2f}%  over 5% on {sum(1 for y in v if y > 0.05)} of {len(v)}")
    print("\nnoise against margins: winner, runner-up, second / first, and whether their ranges of 3 runs overlap")
    in_noise = []
    for k, a in cells.items():
        r = res[k]
        if not r:
            continue
        o = sorted(ARMS, key=lambda x: r["t"][x])
        f, g = o[0], o[1]
        ratio = r["t"][g] / r["t"][f] if r["t"][g] < INF else INF
        over = a[f]["hi"] >= a[g]["lo"]
        if over:
            in_noise.append(k)
        print(f"{k[0]:24s} {k[1]:6s} {k[2]:5d} {f:3s} {g:3s} {fr(ratio):>7s} {'OVERLAP' if over else ''}")
    print("winner rests inside the noise on", len(in_noise), "cells:", in_noise)

    print("\npeak table bytes, MB. WJ and PW flat, 12 byte slots, settled allocation. WT allocation, directory included. Peak resident of the first run")
    for k, a in cells.items():
        def b(x):
            cnt = a[x]["count"]
            if cnt:
                return mb(int(cnt["table_bytes"]))
            reps = a[x]["reps"]
            return mb(reps[0]["bytes"]) + "*" if reps else "-"
        def rss(x):
            reps = a[x]["reps"]
            return mb(reps[0]["rss"]) if reps else "-"
        print(f"{k[0]:24s} {k[1]:6s} {k[2]:5d} WJ {b('WJ'):>10s} WT {b('WT'):>10s} PW {b('PW'):>10s} | rss R2 {rss('R2'):>9s} PR {rss('PR'):>9s} WJ {rss('WJ'):>9s} WT {rss('WT'):>9s}")

    print("\ncounts beside the clock: table reads, row reads and writes, operations, from the count build")
    for k, a in cells.items():
        for x in ARMS:
            c = a[x]["count"]
            if c:
                print(f"{k[0]:24s} {k[1]:6s} {k[2]:5d} {x:3s} {c['status']:5s} ops {int(c['ops']):>18,d} tab_read {int(c['tab_read']):>18,d} tab_write {int(c['tab_write']):>16,d} row {int(c['row']):>18,d} scan {int(c['scan']):>18,d}")

    print("\nhub structure, measured from the streams, at the moment of most wedges")
    for (s, v), r in hub.items():
        print(f"{s:24s} {v:6s} wedges {int(r['peak_wedges']):>16,d} edges {int(r['edges_at_peak']):>11,d} wedges/edge {r['wedges_per_edge']:>10s} max degree {r['max_degree']}")
    xs, ys, zs = [], [], []
    for k, a in cells.items():
        r = res[k]
        h = hub.get((k[0], k[1]))
        if r and h and r["t"]["WT"] < INF and min(r["t"]["R2"], r["t"]["PR"]) < INF:
            xs.append(float(h["wedges_per_edge"]))
            ys.append(r["t"]["WT"] / min(r["t"]["R2"], r["t"]["PR"]))
            zs.append(r["score"])
    sp = spearman(xs, ys)
    print("rank correlation, wedges per edge against WT / min(R2, PR), on", len(xs), "cells:", None if sp is None else round(sp, 3))
    sz = spearman(xs, zs)
    print("rank correlation, wedges per edge against the score, on", len(xs), "cells:", None if sz is None else round(sz, 3))
    print("winner by stream and variant:")
    for s in STREAMS:
        for v in VARIANTS:
            w = [res[(s, v, c)]["win5"] if res[(s, v, c)] else "unscored" for c in CUTS]
            print(f"  {s:24s} {v:6s} cut 100 {w[0]:8s} cut 1000 {w[1]:8s}")

    print("\nRULE, as written, medians of 3")
    t = verdict(res, bad)
    print(show(t))
    print("not scored:", [k for k, r in res.items() if not r])
    print("cells with score at most 0.67:", [k for k, r in res.items() if r and r["score"] <= 0.67])
    print("\nREADINGS")
    print("PW left out of fastest overall:", show(verdict(res, bad, key="win4")))
    print("each table arm taken alone:", show(verdict(res, bad, combined=False)))
    print("PW left out and each alone:", show(verdict(res, bad, key="win4", combined=False)))
    print("\nNOISE ON THE VERDICT")
    for r in (0, 1, 2):
        one = {k: score_cell(a, lambda x, i=r: x["reps"][i]["secs"] if len(x["reps"]) > i and x["reps"][i]["status"] == "OK" else x["median"]) for k, a in cells.items()}
        print(f"run {r + 1} alone:", show(verdict(one, bad)))
    def against(arms):
        c = score_cell(arms, lambda x: x["median"])
        if not c:
            return None
        lo = {a: arms[a]["lo"] if arms[a]["finished"] else INF for a in ARMS}
        hi = {a: arms[a]["hi"] if arms[a]["finished"] else INF for a in ARMS}
        c = dict(c)
        c["score"] = hi["PR"] / min(lo["R2"], lo["WJ"], lo["WT"])
        c["t"] = dict(lo)
        c["t"]["PR"] = hi["PR"]
        c["win5"] = min(ARMS, key=lambda a: c["t"][a])
        return c
    def favour(arms):
        c = score_cell(arms, lambda x: x["median"])
        if not c:
            return None
        lo = {a: arms[a]["lo"] if arms[a]["finished"] else INF for a in ARMS}
        hi = {a: arms[a]["hi"] if arms[a]["finished"] else INF for a in ARMS}
        c = dict(c)
        c["score"] = lo["PR"] / min(hi["R2"], hi["WJ"], hi["WT"])
        c["t"] = dict(hi)
        c["t"]["PR"] = lo["PR"]
        c["win5"] = min(ARMS, key=lambda a: c["t"][a])
        return c
    print("every cell at the worst of its 3 runs for sharing:", show(verdict({k: against(a) for k, a in cells.items()}, bad)))
    print("every cell at the best of its 3 runs for sharing:", show(verdict({k: favour(a) for k, a in cells.items()}, bad)))

    fields = ["stream", "variant", "label", "cut", "arm", "status", "median_s", "run1_s", "run2_s", "run3_s", "spread", "score", "pw_over_best", "best_arm", "winner", "scored",
              "ops", "tab_read", "tab_write", "row", "scan", "table_entries", "table_bytes", "table_transient", "peak_rss", "batches_done", "batches"]
    rows = []
    for k, a in cells.items():
        r = res[k]
        for x in ARMS:
            i = a[x]
            c = i["count"] or {}
            reps = i["reps"]
            row = {"stream": k[0], "variant": k[1], "label": LABEL[k[1]], "cut": k[2], "arm": x, "status": "OK" if i["finished"] else i["why"],
                   "median_s": "" if not i["finished"] else i["median"], "spread": "" if i["spread"] is None else f"{i['spread']:.4f}",
                   "score": "" if not r else (f"{r['score']:.4f}" if r["score"] < INF else "inf"), "pw_over_best": "" if not r else (f"{r['pw']:.4f}" if r["pw"] < INF else "inf"),
                   "best_arm": r["bestarm"] if r else "", "winner": r["win5"] if r else "", "scored": int(bool(r)), "peak_rss": reps[0]["rss"] if reps else "",
                   "batches_done": reps[0]["done"] if reps else "", "batches": reps[0]["batches"] if reps else ""}
            for n in (1, 2, 3):
                row[f"run{n}_s"] = reps[n - 1]["secs"] if len(reps) >= n else ""
            for f in ["ops", "tab_read", "tab_write", "row", "scan", "table_entries", "table_bytes", "table_transient"]:
                row[f] = c.get(f, "")
            rows.append(row)
    with open(f"{out}/m1c03.csv", "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        w.writerows(rows)


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "results/screen-c-M1C03")
