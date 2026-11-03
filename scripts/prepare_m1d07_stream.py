"""Turn one M1D07 input file into a binary event stream, as fixed in docs/experiments/m1d07-streams.md.

Usage: python -I scripts/prepare_m1d07_stream.py <format> <in> <out.bin> <expiry|insert|real> [--prefix N]
Formats:
  konect   a KONECT dynamic file, plain or the .tar.bz2 as served: '%' comments, then "u v w t" with w = +1 or -1. Variant real only
  triples  a SNAP temporal file, plain or .gz: "u v t". The same stream as scripts/prepare_stream.py builds
  reddit   a SNAP Reddit hyperlink TSV: a header, then SOURCE, TARGET, POST_ID, TIMESTAMP as YYYY-MM-DD HH:MM:SS (UTC), ...
  nr       a Network Repository file, plain or the .zip as served (one edge file inside): '%' or '#' comments, then "u v t" or "u v w t", by spaces, tabs or commas.
           With four columns, a first comment line '% <structure> dynamic' makes it a real-deletion stream (variant real) with the
           weight +1 or -1 in column 3. Otherwise t is whichever of columns 3 and 4 has every value at least 10^8, the other is
           ignored, and if neither or both qualify the file fails the check (reading M1D07-R1)
Record: little-endian int64 time, uint32 u, uint32 v, uint32 op (0 delete, 1 insert), packed, 20 bytes.
--prefix keeps the first N events of the derived stream, default 2,000,000. 0 keeps all.

expiry and insert are built exactly as scripts/prepare_stream.py builds them. real is built as Hanauer et al.'s graph_cleaner
builds a stream: self-loops dropped, direction dropped, a stable sort by time with file order kept inside a time, and only
the events that change the undirected edge's state kept. Vertex ids are compacted in first-seen order.
A summary line goes to stdout and to <out.bin>.txt.
"""
import bz2
import calendar
import gzip
import io
import sys
import tarfile
import time
import zipfile

import numpy as np

EXPIRY_SECONDS = 14 * 86400
DEFAULT_PREFIX = 2_000_000


class FormatError(Exception):
    pass


def open_text(path):
    if path.endswith(".tar.bz2"):
        tar = tarfile.open(path, "r:bz2")
        members = [m for m in tar.getmembers() if m.isfile() and m.name.rsplit("/", 1)[-1].startswith("out.")]
        if len(members) != 1:
            raise FormatError(f"expected one out.* member in {path}, found {[m.name for m in members]}")
        return io.TextIOWrapper(tar.extractfile(members[0]), encoding="ascii"), members[0].name
    if path.endswith(".zip"):
        z = zipfile.ZipFile(path)
        names = [n for n in z.namelist() if not n.endswith("/") and n.rsplit(".", 1)[-1].lower() in ("edges", "txt", "csv", "tsv", "mtx")
                 and not n.rsplit("/", 1)[-1].lower().startswith("readme")]
        if len(names) != 1:
            raise FormatError(f"expected one edge file in {path}, found {names}")
        return io.TextIOWrapper(z.open(names[0]), encoding="ascii"), names[0]
    if path.endswith(".gz"):
        return io.TextIOWrapper(gzip.open(path, "rb"), encoding="ascii"), path
    if path.endswith(".bz2"):
        return io.TextIOWrapper(bz2.open(path, "rb"), encoding="ascii"), path
    return open(path, encoding="ascii"), path


def load_int_table(f, comments, delimiter=None):
    try:
        a = np.loadtxt(f, dtype=np.int64, comments=comments, delimiter=delimiter, ndmin=2)
    except ValueError as e:
        raise FormatError(str(e)) from None
    if a.size == 0:
        raise FormatError("no data lines")
    return a


def read_konect(path):
    f, member = open_text(path)
    a = load_int_table(f, "%")
    if a.shape[1] != 4:
        raise FormatError(f"{member}: {a.shape[1]} columns, expected 4")
    w = a[:, 2]
    if not np.all((w == 1) | (w == -1)):
        raise FormatError(f"{member}: a weight other than +1 or -1")
    return a[:, 0], a[:, 1], a[:, 3], (w == 1).astype(np.uint32), member


def read_triples(path):
    f, member = open_text(path)
    a = load_int_table(f, None)
    if a.shape[1] != 3:
        raise FormatError(f"{member}: {a.shape[1]} columns, expected 3")
    return a[:, 0], a[:, 1], a[:, 2], None, member


def read_reddit(path):
    names, src, dst, ts = {}, [], [], []
    with open(path, encoding="utf-8") as f:
        head = f.readline().rstrip("\n").split("\t")
        if head[:4] != ["SOURCE_SUBREDDIT", "TARGET_SUBREDDIT", "POST_ID", "TIMESTAMP"]:
            raise FormatError(f"unexpected header {head[:4]}")
        for no, line in enumerate(f, 2):
            p = line.split("\t", 4)
            if len(p) < 4:
                raise FormatError(f"line {no}: {len(p)} fields")
            try:
                t = calendar.timegm(time.strptime(p[3], "%Y-%m-%d %H:%M:%S"))
            except ValueError:
                raise FormatError(f"line {no}: time {p[3]!r}") from None
            src.append(names.setdefault(p[0], len(names)))
            dst.append(names.setdefault(p[1], len(names)))
            ts.append(t)
    return np.array(src, np.int64), np.array(dst, np.int64), np.array(ts, np.int64), None, path


def read_nr(path):
    """Reading M1D07-R1 (log.md 0044): deletions only under a KONECT-style '% <structure> dynamic' header, and the time is the
    one of columns 3 and 4 whose every value is at least 10^8, Unix seconds after March 1973."""
    f, member = open_text(path)
    head, lines = None, []
    for ln in f:
        s = ln.strip()
        if not s:
            continue
        if s[0] in "%#":
            head = s if head is None else head
            continue
        lines.append(ln)
    if not lines:
        raise FormatError(f"{member}: no data lines")
    delimiter = "," if "," in lines[0] else None
    a = load_int_table(lines, None, delimiter)
    if a.shape[1] == 3:
        return a[:, 0], a[:, 1], a[:, 2], None, member
    if a.shape[1] != 4:
        raise FormatError(f"{member}: {a.shape[1]} columns, expected 3 or 4")
    words = head.split() if head else []
    if len(words) >= 3 and words[2] == "dynamic":
        w = a[:, 2]
        if not np.all((w == 1) | (w == -1)):
            raise FormatError(f"{member}: a dynamic header with a weight other than +1 or -1")
        return a[:, 0], a[:, 1], a[:, 3], (w == 1).astype(np.uint32), member
    times = [c for c in (2, 3) if np.all(a[:, c] >= 10**8)]
    if len(times) != 1:
        raise FormatError(f"{member}: the time column is not determined, columns {[c + 1 for c in times]} qualify")
    return a[:, 0], a[:, 1], a[:, times[0]], None, member


READERS = {"konect": read_konect, "triples": read_triples, "reddit": read_reddit, "nr": read_nr}


def compact(flat):
    """First-seen ranks of the values in flat, as scripts/prepare_stream.py computes them."""
    ids, first = np.unique(flat, return_index=True)
    seen_rank = np.empty(len(ids), dtype=np.int64)
    seen_rank[np.argsort(first, kind="stable")] = np.arange(len(ids))
    return seen_rank[np.searchsorted(ids, flat)], len(ids)


def synthetic(u, v, t, insert_only):
    """scripts/prepare_stream.py, unchanged in effect."""
    rows = np.stack([u, v, t], axis=1)
    rows = rows[rows[:, 0] != rows[:, 1]]
    rows = rows[np.argsort(rows[:, 2], kind="stable")]
    ends, nv = compact(rows[:, :2].ravel())
    ends = ends.reshape(-1, 2)
    n = len(rows)
    if insert_only:
        times, ops = rows[:, 2], np.ones(n, dtype=np.uint32)
    else:
        times = np.concatenate([rows[:, 2], rows[:, 2] + EXPIRY_SECONDS])
        ops = np.concatenate([np.ones(n, dtype=np.uint32), np.zeros(n, dtype=np.uint32)])
        ends = np.concatenate([ends, ends])
    order = np.lexsort((ops, times))
    return times[order], ends[order, 0], ends[order, 1], ops[order], n, nv


def real(u, v, t, op):
    """graph_cleaner: drop self-loops and direction, stable sort by time, keep only state changes."""
    keep = u != v
    u, v, t, op = u[keep], v[keep], t[keep], op[keep]
    order = np.argsort(t, kind="stable")
    u, v, t, op = u[order], v[order], t[order], op[order]
    a, b = np.minimum(u, v), np.maximum(u, v)
    # Within each pair, in stream order, an event changes the state when its op differs from the pair's previous op.
    # The state before a pair's first event is absent, as if its previous op were a deletion.
    pair_order = np.lexsort((np.arange(len(a)), b, a))
    pa, pb, pop = a[pair_order], b[pair_order], op[pair_order]
    first = np.ones(len(pa), dtype=bool)
    first[1:] = (pa[1:] != pa[:-1]) | (pb[1:] != pb[:-1])
    prev = np.empty_like(pop)
    prev[0] = 0
    prev[1:] = pop[:-1]
    prev[first] = 0
    changed = np.empty(len(a), dtype=bool)
    changed[pair_order] = pop != prev
    a, b, t, op = a[changed], b[changed], t[changed], op[changed]
    ends, nv = compact(np.stack([a, b], axis=1).ravel())
    ends = ends.reshape(-1, 2)
    return t, ends[:, 0], ends[:, 1], op, int(keep.sum()), nv


def main(argv):
    if len(argv) < 5 or argv[1] not in READERS or argv[4] not in ("expiry", "insert", "real"):
        print(__doc__)
        return 2
    fmt, src, dst, variant = argv[1:5]
    prefix = DEFAULT_PREFIX
    if len(argv) == 7 and argv[5] == "--prefix":
        prefix = int(argv[6])
    elif len(argv) != 5:
        print(__doc__)
        return 2
    try:
        u, v, t, op, member = READERS[fmt](src)
    except FormatError as e:
        print(f"FORMAT-FAIL {src}: {e}")
        return 3
    lines = len(u)
    has_real = op is not None
    if has_real != (variant == "real"):
        print(f"FORMAT-FAIL {src}: variant {variant} on a {'real-deletion' if has_real else 'timestamped interaction'} file")
        return 3
    if has_real:
        times, uu, vv, ops, kept, nv = real(u, v, t, op)
    else:
        times, uu, vv, ops, kept, nv = synthetic(u, v, t, variant == "insert")
    total = len(times)
    if prefix and total > prefix:
        times, uu, vv, ops = times[:prefix], uu[:prefix], vv[:prefix], ops[:prefix]
    rec = np.zeros(len(times), dtype=[("t", "<i8"), ("u", "<u4"), ("v", "<u4"), ("op", "<u4")])
    rec["t"], rec["u"], rec["v"], rec["op"] = times, uu, vv, ops
    rec.tofile(dst)
    used_v = int(max(uu.max(), vv.max()) + 1) if len(uu) else 0
    dels = int((ops == 0).sum())
    line = (f"{src} [{member}] format={fmt} variant={variant} lines={lines} kept={kept} vertices={nv} events_full={total} "
            f"events={len(rec)} prefix={'whole' if not prefix or total <= prefix else prefix} vertices_in_prefix={used_v} "
            f"deletions={dels} deletion_share={dels / max(len(rec), 1):.4f} t_min={int(times.min())} t_max={int(times.max())} "
            f"distinct_times={len(np.unique(times))}")
    print(line)
    with open(dst + ".txt", "w") as f:
        f.write(line + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
