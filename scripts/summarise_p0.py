"""Summarise probe P0. Each cell is the ratio of summed operations to sequential scanning in stream order,
summed over the batches of each seed, then the median across seeds. Non-citable: synthetic graph only."""
import collections
import csv
import pathlib
import statistics
import sys

root = pathlib.Path(sys.argv[1])
ops = collections.defaultdict(int)
static = collections.defaultdict(int)
oriented = collections.defaultdict(int)
groups = collections.defaultdict(int)
seen_static = set()
bad = 0
for f in sorted(root.glob("seed*.csv")):
    for r in csv.DictReader(f.open()):
        k = (r["seed"], r["stream"], float(r["frac"]))
        ops[k + (r["policy"],)] += int(r["ops"])
        bad += r["delta_ok"] != "1"
        b = k + (r["batch"],)
        if b not in seen_static:
            seen_static.add(b)
            static[k] += int(r["static_ops"])
        if r["policy"] == "grp_full":
            oriented[k] += int(r["kernel_c"])
            groups[k] += int(r["kernel_a"]) + int(r["kernel_b"]) + int(r["kernel_c"])

seeds = sorted({k[0] for k in ops})
streams = ["uniform", "biased", "arrival", "departure", "clique"]
fracs = sorted({k[2] for k in ops})
pol = ["grp_edge", "grp_noorient", "grp_full", "lowdeg_full"]
print("seeds", seeds, "delta mismatches", bad)
print("| stream | frac | grp_edge | grp_noorient | grp_full | lowdeg_full | static | oriented groups |")
print("|---|---|---|---|---|---|---|---|")
for s in streams:
    for fr in fracs:
        cell = []
        for p in pol:
            v = []
            for sd in seeds:
                base = ops.get((sd, s, fr, "seq_stream"), 0)
                if base:
                    v.append(ops[(sd, s, fr, p)] / base)
            cell.append(statistics.median(v) if v else float("nan"))
        st = statistics.median([static[(sd, s, fr)] / ops[(sd, s, fr, "seq_stream")] for sd in seeds if ops.get((sd, s, fr, "seq_stream"), 0)])
        oc = sum(oriented[(sd, s, fr)] for sd in seeds)
        gc = sum(groups[(sd, s, fr)] for sd in seeds)
        print(f"| {s} | {fr:g} | {cell[0]:.3f} | {cell[1]:.3f} | {cell[2]:.3f} | {cell[3]:.3f} | {st:.1f} | {oc} of {gc} |")
