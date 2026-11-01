"""M1C03 live check, run by scripts/run_m1c03.sh after each pass over a cell. Exit 1 stops the run.

Usage: python -I scripts/check_m1c03.py <outdir> <stream> <variant> <cut>

1. Exactness. Every raw file of the cell, count build and timing build, every arm and repeat, must give the same count on every batch that
   both files reached. A partial file from a stopped run is compared as far as it goes, leaving out the last row of a CAP record, the batch in which
   the table stopped accepting writes. One mismatch is exit 1. Hard rule 1.
2. Unchanged arms. On the 12 cells of M1C02, each finished arm of the count build must match results/screen-c-M1C02 on every field of the
   summary and on every per-batch row. Any difference is exit 1.
"""
import csv
import glob
import os
import re
import sys

M1C02 = "results/screen-c-M1C02"
OLD_ARMS = ["R2", "WJ", "PR", "PW"]
FIELDS = ["status", "batches_done", "batches", "events", "net", "ops", "scan", "probe", "row", "deg", "tab_read", "tab_write", "graph", "net_ops", "group",
          "upkeep", "peak_m", "final_t", "table_entries", "table_bytes", "table_transient", "max_update", "groups", "spokes"]


def load(path):
    with open(path, newline="") as f:
        return list(csv.DictReader(f))


def main(out, stream, variant, cut):
    base = f"{out}/raw/{stream}-{variant}-{cut}-"
    bad = 0
    seen = {}
    files = 0
    compared = 0
    for path in sorted(glob.glob(base + "*.csv")):
        if path.endswith(".summary.csv"):
            continue
        name = os.path.basename(path)
        files += 1
        rows = load(path)
        # A table that hits its cap drops writes for the rest of that batch, so the last row of a CAP record is not a count. See README, Process notes.
        if load(path[:-4] + ".summary.csv")[0]["status"] == "CAP":
            rows = rows[:-1]
        for r in rows:
            i, t = int(r["batch"]), r["t"]
            compared += 1
            if i in seen and seen[i][0] != t:
                print(f"EXACTNESS mismatch {name} batch {i}: {t} against {seen[i][0]} in {seen[i][1]}")
                bad += 1
                if bad > 10:
                    return 1
            seen.setdefault(i, (t, name))

    old = f"{M1C02}/raw/{stream}-{variant}-{cut}-"
    matched = 0
    for a in OLD_ARMS:
        new_sum = f"{base}{a}.summary.csv"
        if not (os.path.exists(new_sum) and os.path.exists(f"{old}{a}.summary.csv")):
            continue
        n, o = load(new_sum)[0], load(f"{old}{a}.summary.csv")[0]
        for k in FIELDS:
            if n[k] != o[k]:
                print(f"COUNT difference against M1C02: {stream} {variant} {cut} {a} {k}: {n[k]} against {o[k]}")
                bad += 1
        nr, or_ = load(f"{base}{a}.csv"), load(f"{old}{a}.csv")
        if nr != or_:
            print(f"COUNT difference against M1C02: {stream} {variant} {cut} {a}: per-batch rows")
            bad += 1
        matched += 1
    print(f"check {stream} {variant} {cut}: files {files}, comparisons {compared}, batches {len(seen)}, arms matched to M1C02 {matched}, problems {bad}")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]))
