"""M1D05: apply the written rule to the timing runs. Protocol: docs/experiments/gate1-protocol.md.

Usage: python -I scripts/summarise_gate1.py [timing outdir] [count outdir]
Writes gate1.csv into the timing outdir and prints the tables. Stop is read first, then Go, and Pivot is anything else.

READINGS FIXED BEFORE THE FIRST STREAM.
- Finished: an arm's first repeat finished. An arm that ran once only has that repeat as its median, slowest and fastest.
- An arm stopped by time enters at the limit, 1,200 s, as a lower bound, for the score and the pass test alike. An arm stopped by memory is out of the cell.
- Share: P3, P4 or PR. At p = 4, the faster of P4 and B4 by median, and both are printed.
- Best: the fastest of the other rows, B4 aside, by median for the score and by fastest repeat for the pass test.
- Scored: share finished and at least one other arm finished. If share did not finish, the cell is listed and not scored.
"""
import csv
import glob
import math
import os
import statistics
import sys

STREAMS = ["sx-askubuntu", "sx-superuser", "wiki-talk-temporal", "sx-stackoverflow"]
VARIANTS = ["expiry", "insert"]
CUTS = [100, 1000]
LABEL = {"expiry": "real timestamps, synthetic expiry", "insert": "real timestamps, insert-only, no expiry"}
SHARE = {3: ["P3"], 4: ["P4", "B4"], 5: ["PR"]}
OTHERS = {3: ["S3", "T3", "M3"], 4: ["S4", "T4", "M4", "A4"], 5: ["R2", "WT", "WJ", "M5", "A5"]}
SCAN = {3: "S3", 4: "S4", 5: "R2"}
LIMIT = 1200.0


def one(path):
    with open(path, newline="") as f:
        return next(csv.DictReader(f))


def arm(out, s, v, c, p, a):
    runs = []
    for path in sorted(glob.glob(f"{out}/raw/{s}-{v}-{c}-p{p}-{a}*.t*.summary.csv")):
        runs.append(one(path))
    if not runs:
        return None
    ok = [float(r["seconds"]) for r in runs if r["status"] == "OK"]
    status = next((r["status"] for r in runs if r["status"] != "OK"), "OK")
    if status == "OK":
        return {"status": "OK", "med": statistics.median(ok), "lo": min(ok), "hi": max(ok), "n": len(ok), "bytes": int(runs[0]["table_bytes"])}
    if status == "TIME":
        return {"status": "TIME", "med": LIMIT, "lo": LIMIT, "hi": LIMIT, "n": 0, "bytes": int(runs[0]["table_bytes"])}
    return {"status": status, "med": math.inf, "lo": math.inf, "hi": math.inf, "n": 0, "bytes": int(runs[0]["table_bytes"])}


def ops(out, s, v, c, p, a):
    for d in (out, f"{out}/count", out.replace("M1D04", "M1D03")):
        hits = sorted(glob.glob(f"{d}/raw/{s}-{v}-{c}-p{p}-{a}*.summary.csv"))
        hits = [h for h in hits if ".t" not in os.path.basename(h)]
        if hits:
            m = one(hits[0])
            return int(m["ops"]) if m["status"] == "OK" else None
    return None


def cell(out, s, v, c, p):
    all_arms = {a: arm(out, s, v, c, p, a) for a in SHARE[p] + OTHERS[p]}
    if not any(all_arms.values()):
        return None
    shares = [a for a in SHARE[p] if all_arms[a] and all_arms[a]["status"] == "OK"]
    r = {"stream": s, "variant": v, "cut": c, "p": p, "arms": all_arms, "scored": False, "pass": False, "score": None, "share": None, "best": None}
    others = {a: all_arms[a] for a in OTHERS[p] if all_arms[a] and all_arms[a]["status"] in ("OK", "TIME")}
    if not shares or not any(x["status"] == "OK" for x in others.values()):
        return r
    sh = min(shares, key=lambda a: all_arms[a]["med"])
    best = min(others, key=lambda a: others[a]["med"])
    fast = min(x["lo"] for x in others.values())
    r.update(scored=True, share=sh, best=best, score=all_arms[sh]["med"] / others[best]["med"], tight=all_arms[sh]["hi"] / fast)
    r["pass"] = r["tight"] <= 0.67
    scan = all_arms.get(SCAN[p])
    r["vs_scan"] = all_arms[sh]["med"] / scan["med"] if scan and scan["status"] in ("OK", "TIME") else None
    named = {a: ops(out, s, v, c, p, a) for a in SHARE[p] + OTHERS[p]}
    known = {a: n for a, n in named.items() if n is not None}
    r["ops_winner"] = min(known, key=known.get) if known else None
    r["clock_winner"] = min((a for a in all_arms if all_arms[a] and all_arms[a]["status"] == "OK"), key=lambda a: all_arms[a]["med"])
    r["ns_per_op"] = {a: all_arms[a]["med"] * 1e9 / known[a] for a in known if all_arms[a] and all_arms[a]["status"] == "OK" and known[a]}
    return r


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else "results/gate1-M1D04"
    cells = [cell(out, s, v, c, p) for p in (3, 4, 5) for s in STREAMS for v in VARIANTS for c in CUTS]
    cells = [x for x in cells if x]
    with open(f"{out}/gate1.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["pattern", "stream", "label", "variant", "cut", "share", "best", "score", "tight", "pass", "vs_scan", "ops_winner", "clock_winner"])
        for x in cells:
            if x["scored"]:
                w.writerow([x["p"], x["stream"], LABEL[x["variant"]], x["variant"], x["cut"], x["share"], x["best"], f"{x['score']:.3f}", f"{x['tight']:.3f}", int(x["pass"]),
                            f"{x['vs_scan']:.3f}" if x["vs_scan"] else "", x["ops_winner"], x["clock_winner"]])
            else:
                w.writerow([x["p"], x["stream"], LABEL[x["variant"]], x["variant"], x["cut"], "not scored", "", "", "", 0, "", "", ""])
    for p in (3, 4, 5):
        here = [x for x in cells if x["p"] == p]
        sc = [x for x in here if x["scored"]]
        print(f"\np = {p}: {len(sc)} of {len(here)} cells scored, {sum(x['pass'] for x in sc)} pass")
        for x in here:
            if not x["scored"]:
                print(f"  {x['stream']} {x['variant']} {x['cut']}: not scored")
                continue
            print(f"  {x['stream']} {x['variant']} {x['cut']}: {x['share']} / {x['best']} = {x['score']:.3f}, tight {x['tight']:.3f}, {'PASS' if x['pass'] else 'no'}"
                  f", vs scan {x['vs_scan'] and round(x['vs_scan'], 3)}, ops winner {x['ops_winner']}, clock winner {x['clock_winner']}")
        if sc:
            print(f"  median score {statistics.median(x['score'] for x in sc):.3f}")
    sc = [x for x in cells if x["p"] == 5 and x["scored"]]
    passed = [x for x in sc if x["pass"]]
    if len(sc) < 8 or not passed:
        verdict = "STOP"
    elif 3 * len(passed) >= len(sc) and len({x["stream"] for x in passed}) >= 2 and sum(x["variant"] == "expiry" for x in passed) >= 2:
        verdict = "GO"
    else:
        verdict = "PIVOT"
    print(f"\nVerdict on p = 5, exactness read separately: {verdict}. {len(passed)} of {len(sc)} cells pass.")


if __name__ == "__main__":
    main()
