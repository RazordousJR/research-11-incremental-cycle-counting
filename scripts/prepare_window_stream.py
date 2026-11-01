"""Turn one SNAP temporal file into the raw event stream of Screen B test M1B01.

Usage: python -I scripts/prepare_window_stream.py <in.txt.gz> <out.bin>
Record: little-endian int64 time, uint32 u, uint32 v, packed, 16 bytes.

Every repeat event is kept. Self-loops are dropped. Direction is dropped by the reader, which keys pairs on the
unordered pair. Vertex ids are compacted in first-seen order, and rows are stable-sorted by time. No expiry event is
written: under the window query, expiry is the definition of the query (docs/experiments/screen-b-protocol.md).
"""
import gzip
import sys

import numpy as np


def main(src, dst):
    with gzip.open(src, "rb") as f:
        raw = np.array(f.read().split(), dtype=np.int64)
    rows = raw.reshape(-1, 3)
    rows = rows[rows[:, 0] != rows[:, 1]]
    rows = rows[np.argsort(rows[:, 2], kind="stable")]

    flat = rows[:, :2].ravel()
    ids, first = np.unique(flat, return_index=True)
    seen_rank = np.empty(len(ids), dtype=np.int64)
    seen_rank[np.argsort(first, kind="stable")] = np.arange(len(ids))
    compact = seen_rank[np.searchsorted(ids, flat)].reshape(-1, 2)

    rec = np.zeros(len(rows), dtype=[("t", "<i8"), ("u", "<u4"), ("v", "<u4")])
    rec["t"] = rows[:, 2]
    rec["u"] = compact[:, 0]
    rec["v"] = compact[:, 1]
    rec.tofile(dst)
    pairs = len(np.unique(np.minimum(compact[:, 0], compact[:, 1]) * len(ids) + np.maximum(compact[:, 0], compact[:, 1])))
    print(f"{src}: lines={len(raw) // 3} kept={len(rows)} vertices={len(ids)} distinct_pairs={pairs} "
          f"t_min={rows[:, 2].min()} t_max={rows[:, 2].max()} distinct_times={len(np.unique(rows[:, 2]))}")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
