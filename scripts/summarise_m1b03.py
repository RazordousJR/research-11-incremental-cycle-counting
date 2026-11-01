"""Summarise results/screen-b-M1B03/production.csv against the written M1B03 rule.

Usage: python -I scripts/summarise_m1b03.py [results dir]

Comparison rule, fixed before the production runs:
  - Scan baseline for a given split: the cheapest scan row, memo or no memo, whose index memory is at most that split's.
  - Eager is the real table where it fits and the formula row where it does not. Formula rows are marked.
  - A Stop line is tested at each |H| separately. A split that escapes both lines on a graph can carry that graph.
"""
import csv
import os
import sys
from collections import defaultdict

REAL = ("sx-superuser-peak", "wiki-talk-temporal-peak", "sx-superuser-ins-end", "wiki-talk-temporal-ins-end")


def num(x):
    return float(x) if x not in ("", "nan") else float("nan")


def main(rdir):
    rows = list(csv.DictReader(open(os.path.join(rdir, "production.csv"), newline="")))
    runs = defaultdict(list)
    for r in rows:
        if r["tag"] == "main":
            runs[(r["graph"], float(r["target"]))].append(r)

    out = []
    for (graph, target), rs in sorted(runs.items(), key=lambda kv: (kv[0][0], -kv[0][1])):
        scans = [r for r in rs if r["backend"] == "scan"]
        splits = [r for r in rs if r["backend"].startswith("split")]
        eager = [r for r in rs if r["backend"].startswith("eager")]
        e = eager[0] if eager else None
        for s in splits:
            allowed = [x for x in scans if num(x["index_bytes"]) <= num(s["index_bytes"])]
            best = min(allowed or scans, key=lambda x: num(x["ops_total"]))
            best_any = min(scans, key=lambda x: num(x["ops_total"]))
            so, sa = num(s["ops_total"]), num(s["ops_alt"])
            out.append({
                "graph": graph, "label": s["label"], "target": target, "acc": s["acc_rate"], "acc_tail": s["acc_tail"],
                "theta_e": s["theta_e"], "a": s["a"], "theta_t": s["theta_t"], "m_start": s["m_start"], "m_end": s["m_end"],
                "hubs": s["hubs"], "split_kind": s["backend"], "split_ops_per_prop": so / num(s["N"]),
                "scan_ops_per_prop": num(best["ops_total"]) / num(s["N"]),
                "scan_memo_slots": best["memo_slots"], "scan_memo_min": best["memo_min"],
                "eager_ops_per_prop": num(e["ops_total"]) / num(s["N"]) if e else float("nan"),
                "eager_kind": e["backend"] if e else "none",
                "scan_over_split": num(best["ops_total"]) / so,
                "scan_any_over_split": num(best_any["ops_total"]) / so,
                "scan_over_split_lenient": num(best["ops_alt"]) / sa,
                "split_over_eager": so / num(e["ops_total"]) if e else float("nan"),
                "split_over_eager_lenient": sa / num(e["ops_alt"]) if e else float("nan"),
                "split_bytes": num(s["index_bytes"]), "eager_bytes": num(e["index_bytes"]) if e else float("nan"),
                "split_entries_peak": s["entries_peak"], "eager_entries_peak": e["entries_peak"] if e else "",
                "mem_ratio": num(s["index_bytes"]) / num(e["index_bytes"]) if e else float("nan"),
                "split_build": s["build_ops"], "eager_build": e["build_ops"] if e else "",
                "scan_with_build_over_split": num(best["ops_total"]) / (so + num(s["build_ops"])),
                "split_with_build_over_eager": (so + num(s["build_ops"])) / (num(e["ops_total"]) + num(e["build_ops"])) if e else float("nan"),
            })
    keys = list(out[0].keys())
    with open(os.path.join(rdir, "step2_summary.csv"), "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=keys)
        w.writeheader()
        w.writerows(out)

    print(f"{'graph':28}{'tgt':>5}{'acc':>7}{'H':>6}{'split':>9}{'scan':>9}{'eager':>9}{'sc/sp':>7}{'sp/eg':>7}{'mem':>7}")
    for r in out:
        print(f"{r['graph']:28}{r['target']:>5}{float(r['acc']):7.3f}{r['hubs']:>6}{r['split_ops_per_prop']:9.2f}{r['scan_ops_per_prop']:9.2f}"
              f"{r['eager_ops_per_prop']:9.2f}{r['scan_over_split']:7.2f}{r['split_over_eager']:7.2f}{r['mem_ratio']:7.3f}")

    print("\nrule lines, per graph, per |H|")
    by = defaultdict(dict)
    for r in out:
        by[(r["graph"], r["hubs"])][r["target"]] = r
    for (graph, h), t in sorted(by.items()):
        a, b = t.get(0.05), t.get(0.01)
        if not a or not b:
            print(graph, h, "missing a target")
            continue
        stop1 = a["scan_over_split"] <= 1.5
        stop2 = b["split_over_eager"] > 2.0
        memok = min(x["mem_ratio"] for x in t.values()) <= 0.5
        print(f"{graph:28} H={h:>5} scan/split@5%={a['scan_over_split']:.3f} stop1={stop1}  split/eager@1%={b['split_over_eager']:.3f} "
              f"stop2={stop2}  mem<=0.5={memok} real={graph in REAL}")


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "results/screen-b-M1B03")
