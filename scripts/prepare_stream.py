"""Turn one SNAP temporal file into a binary event stream, as fixed in docs/experiments/gate0-protocol.md.

Usage: python -I scripts/prepare_stream.py <in.txt.gz> <out.bin> [insert-only]
Record: little-endian int64 time, uint32 u, uint32 v, uint32 op (0 delete, 1 insert), packed, 20 bytes.
"""
import gzip
import sys

import numpy as np

EXPIRY_SECONDS = 14 * 86400


def main(src, dst, insert_only=False):
    with gzip.open(src, "rb") as f:
        raw = np.array(f.read().split(), dtype=np.int64)
    rows = raw.reshape(-1, 3)
    rows = rows[rows[:, 0] != rows[:, 1]]               # self-loops are dropped
    order = np.argsort(rows[:, 2], kind="stable")
    rows = rows[order]

    flat = rows[:, :2].ravel()
    ids, first = np.unique(flat, return_index=True)
    seen_rank = np.empty(len(ids), dtype=np.int64)
    seen_rank[np.argsort(first, kind="stable")] = np.arange(len(ids))
    compact = seen_rank[np.searchsorted(ids, flat)].reshape(-1, 2)

    n = len(rows)
    if insert_only:
        times, ops, ends = rows[:, 2], np.ones(n, dtype=np.uint32), compact
    else:
        times = np.concatenate([rows[:, 2], rows[:, 2] + EXPIRY_SECONDS])
        ops = np.concatenate([np.ones(n, dtype=np.uint32), np.zeros(n, dtype=np.uint32)])
        ends = np.concatenate([compact, compact])
    # lexsort is stable: time first, then deletions before insertions at equal times
    order = np.lexsort((ops, times))
    rec = np.zeros(len(times), dtype=[("t", "<i8"), ("u", "<u4"), ("v", "<u4"), ("op", "<u4")])
    rec["t"] = times[order]
    rec["u"] = ends[order, 0]
    rec["v"] = ends[order, 1]
    rec["op"] = ops[order]
    rec.tofile(dst)
    print(f"{src}: lines={len(raw) // 3} kept={n} vertices={len(ids)} events={len(times)}")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2], len(sys.argv) > 3)
