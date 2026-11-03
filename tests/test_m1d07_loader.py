"""Oracle test for the M1D07 loader, and for Gate 1's arms on streams with real deletions. Protocol: docs/experiments/m1d07-streams.md.

1. Regression: on the SNAP files in data/snap, the loader's expiry and insert streams equal data/derived byte for byte.
2. Real deletions: on random KONECT-format files, packed as KONECT serves them, the loader equals an independent, line-by-line
   transliteration of Hanauer et al.'s graph_cleaner.
3. Engine: on derived real-deletion streams, every Gate 1 arm at p = 3 and p = 5, M1D07's own binary build/gate1/count.exe, gives
   per batch the count of a brute force over vertex tuples.
Controls, each of which must FAIL, or the test is broken:
   C1 a loader that puts deletions before insertions at equal times, as the synthetic rule does
   C2 a loader that keeps direction when it cleans
   C3 a brute force that never deletes, so that the streams are known to exercise deletions
4. Network Repository, reading M1D07-R1: a zip as served, read in place, gives the triples stream for each layout of the five files,
   is refused as real deletions without a dynamic header, fails with two time-like columns, and gives the graph_cleaner stream under one.
Usage: python -I tests/test_m1d07_loader.py [trials] [seed] [workdir]
"""
import io
import itertools
import os
import random
import subprocess
import sys
import tarfile
import tempfile
import zipfile

import numpy as np

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "scripts"))
import prepare_m1d07_stream as L  # noqa: E402

COUNT = os.path.join(ROOT, "build", "gate1", "count.exe")
ARMS = {3: ["S3", "P3", "T3", "M3"], 5: ["R2", "PR", "WT", "WJ", "M5", "A5"]}
THETAS = {"M3": [16, 1], "M5": [128, 1]}
EOL = chr(10)
REC = np.dtype([("t", "<i8"), ("u", "<u4"), ("v", "<u4"), ("op", "<u4")])


# ---- the reference: graph_cleaner, line by line, then first-seen compaction

def cleaner(lines):
    ev = []
    for ln in lines:
        if not ln.strip() or ln.startswith("%"):
            continue
        u, v, w, t = ln.split()
        ev.append((int(u), int(v), int(w), int(t)))
    ev.sort(key=lambda e: e[3])               # Python's sort is stable, as std::stable_sort
    state, out = {}, []
    for u, v, w, t in ev:
        if u == v:
            continue
        e = (min(u, v), max(u, v))
        if w == 1 and not state.get(e):
            state[e] = 1
            out.append((t, e[0], e[1], 1))
        elif w == -1 and state.get(e):
            state[e] = 0
            out.append((t, e[0], e[1], 0))
    rank, rec = {}, []
    for t, a, b, op in out:
        ra = rank.setdefault(a, len(rank))
        rb = rank.setdefault(b, len(rank))
        rec.append((t, ra, rb, op))
    return rec


def konect_text(rng, n, m):
    lines = ["% sym positive", f"% {m} {n} {n}"]
    t = 1000
    for _ in range(m):
        t += rng.choice([0, 0, 1, 3])                     # many equal times
        u, v = rng.randrange(1, n + 1), rng.randrange(1, n + 1)
        w = "+1" if rng.random() < 0.6 else "-1"
        lines.append(f"{u} {v} {w} {t}")
    body = lines[2:]
    for _ in range(m // 10):                             # a file not sorted by time
        i, j = rng.randrange(len(body)), rng.randrange(len(body))
        body[i], body[j] = body[j], body[i]
    return lines[:2] + body


def pack(lines, path, name):
    data = ("\n".join(lines) + "\n").encode("ascii")
    with tarfile.open(path, "w:bz2") as tar:
        info = tarfile.TarInfo(f"{name}/out.{name}")
        info.size = len(data)
        tar.addfile(info, io.BytesIO(data))
        meta = b"name: test\n"
        info = tarfile.TarInfo(f"{name}/meta.{name}")
        info.size = len(meta)
        tar.addfile(info, io.BytesIO(meta))


def derive(fmt, src, dst, variant, prefix="0"):
    rc = L.main(["x", fmt, src, dst, variant, "--prefix", prefix])
    if rc != 0:
        raise RuntimeError(f"loader exit {rc} on {src}")
    return np.fromfile(dst, dtype=REC)


def derive_chunked(arc, dst, chunk):
    """The KONECT reader with chunks of a few lines, so that a fixture crosses many chunk boundaries."""
    orig = L.READERS["konect"]
    L.READERS["konect"] = lambda p: L.read_konect(p, chunk)
    try:
        return derive("konect", arc, dst, "real")
    finally:
        L.READERS["konect"] = orig


def as_tuples(rec):
    return [(int(r["t"]), int(r["u"]), int(r["v"]), int(r["op"])) for r in rec]


# ---- brute force over vertex tuples, after every batch, set semantics, last write wins

def tri(adj, n):
    return sum(1 for a, b, c in itertools.combinations(range(n), 3) if adj[a][b] and adj[a][c] and adj[b][c])


def c5(adj, n):
    c = 0
    for s in itertools.combinations(range(n), 5):
        v0 = s[0]
        for p in itertools.permutations(s[1:]):
            if p[0] < p[3]:                             # one direction of each cycle through v0
                cyc = (v0,) + p
                if all(adj[cyc[i]][cyc[(i + 1) % 5]] for i in range(5)):
                    c += 1
    return c


def brute(rec, cut, p, deletes=True):
    n = int(max(rec["u"].max(), rec["v"].max())) + 1
    adj = [[0] * n for _ in range(n)]
    out = []
    for s in range(0, len(rec), cut):
        for r in rec[s:s + cut]:
            u, v = int(r["u"]), int(r["v"])
            if u == v:
                continue
            on = 1 if r["op"] == 1 else (0 if deletes else adj[u][v])
            adj[u][v] = adj[v][u] = on
        out.append(tri(adj, n) if p == 3 else c5(adj, n))
    return out


def engine(binpath, cut, p, arm, theta, outdir):
    cmd = [COUNT, binpath, "t", "real", str(cut), str(p), arm, outdir]
    if theta:
        cmd += ["--theta", str(theta)]
    r = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
    if r.returncode != 0:
        raise RuntimeError(f"{' '.join(cmd)}: exit {r.returncode} {r.stdout} {r.stderr}")
    tag = f"t-real-{cut}-p{p}-{arm}" + (f"-th{theta}" if theta else "")
    with open(os.path.join(outdir, "raw", tag + ".csv")) as f:
        next(f)
        return [int(line.split(",")[4]) for line in f]


def main():
    trials = int(sys.argv[1]) if len(sys.argv) > 1 else 60
    seed = int(sys.argv[2]) if len(sys.argv) > 2 else 7
    work = sys.argv[3] if len(sys.argv) > 3 else tempfile.mkdtemp(prefix="r11-m1d07-")
    os.makedirs(os.path.join(work, "raw"), exist_ok=True)
    rng = random.Random(seed)
    fails = 0

    # 1. Regression on the SNAP inputs this project already derived.
    for name in ("CollegeMsg", "email-Eu-core-temporal"):
        src = os.path.join(ROOT, "data", "snap", name + ".txt.gz")
        for variant, suffix in (("expiry", ".bin"), ("insert", ".ins.bin")):
            ref = os.path.join(ROOT, "data", "derived", name + suffix)
            got = derive("triples", src, os.path.join(work, f"{name}.{variant}.bin"), variant)
            same = got.tobytes() == open(ref, "rb").read()
            pre = derive("triples", src, os.path.join(work, f"{name}.{variant}.pre.bin"), variant, "1000")
            same_pre = pre.tobytes() == got[:1000].tobytes()
            print(f"regression {name} {variant}: {'same' if same else 'DIFFERS'}, prefix 1000 {'same' if same_pre else 'DIFFERS'}")
            fails += (not same) + (not same_pre)

    # 2. The loader against graph_cleaner, and controls C1 and C2.
    c1 = c2 = 0
    streams = []
    for i in range(trials):
        n, m = rng.randrange(4, 11), rng.randrange(20, 160)
        lines = konect_text(rng, n, m)
        name = f"link-dynamic-t{i}wiki"
        arc = os.path.join(work, f"download.tsv.{name}.tar.bz2")
        pack(lines, arc, name)
        got = derive("konect", arc, os.path.join(work, f"k{i}.bin"), "real")
        ref = cleaner(lines)
        if as_tuples(got) != ref:
            print(f"loader DIFFERS from graph_cleaner on trial {i}")
            fails += 1
        cut = rng.randrange(1, 40)                       # the prefix path cleans only part of the stream
        if as_tuples(derive("konect", arc, os.path.join(work, "kp.bin"), "real", str(cut))) != ref[:cut]:
            print(f"loader prefix {cut} DIFFERS from graph_cleaner on trial {i}")
            fails += 1
        small = derive_chunked(arc, os.path.join(work, "kc.bin"), rng.randrange(1, 9))
        if as_tuples(small) != ref:
            print(f"loader with small chunks DIFFERS from graph_cleaner on trial {i}")
            fails += 1
        # C1: deletions first at equal times
        ev = [ln.split() for ln in lines[2:]]
        ev.sort(key=lambda e: (int(e[3]), 0 if e[2] == "-1" else 1))
        if cleaner([" ".join(e) for e in ev]) != ref:
            c1 += 1
        # C2: direction kept
        u, v, t, op, _ = L.read_konect(arc)
        if len(u):
            keep = u != v
            order = np.argsort(t[keep], kind="stable")
            uu, vv, tt, oo = u[keep][order], v[keep][order], t[keep][order], op[keep][order]
            state, kept = {}, 0
            for a, b, o in zip(uu.tolist(), vv.tolist(), oo.tolist()):
                if state.get((a, b), 0) != o:
                    state[(a, b)] = o
                    kept += 1
            if kept != len(ref):
                c2 += 1
        if len(got) >= 5:
            streams.append((i, os.path.join(work, f"k{i}.bin"), got))
    print(f"loader against graph_cleaner: {trials} files, {fails} failures so far")
    print(f"control C1 (deletions first at equal times) differs on {c1} of {trials}: {'FAILS as it must' if c1 else 'PASSED, test broken'}")
    print(f"control C2 (direction kept) differs on {c2} of {trials}: {'FAILS as it must' if c2 else 'PASSED, test broken'}")
    fails += (c1 == 0) + (c2 == 0)

    # 4. Network Repository zips.
    for i in range(10):
        n, m = rng.randrange(4, 11), rng.randrange(20, 160)
        tri_lines = [f"{rng.randrange(1, n + 1)} {rng.randrange(1, n + 1)} {1_100_000_000 + rng.randrange(0, 40) * 3600 * 24}" for _ in range(m)]
        plain = os.path.join(work, f"nr{i}.txt")
        with open(plain, "w") as f:
            f.write(EOL.join(tri_lines) + EOL)
        # The layouts seen in M1D07's five files: "u v t", "u v 1 t", "u,v,t,duration" and a signed "u v +-1 t", whose -1 is a sign
        layouts = [(" ", "% sym unweighted", lambda a, b, t: [a, b, t]),
                   (" ", "% asym positive", lambda a, b, t: [a, b, "1", t]),
                   (",", None, lambda a, b, t: [a, b, t, str(rng.randrange(1, 3000))]),
                   (chr(9), "% asym signed", lambda a, b, t: [a, b, rng.choice(["1", "-1"]), t])]
        for k, (sep, head, row) in enumerate(layouts):
            body = [sep.join(row(*x.split())) for x in tri_lines]
            arc = os.path.join(work, f"nr{i}-{k}.zip")
            with zipfile.ZipFile(arc, "w") as z:
                z.writestr("readme.html", "x")
                z.writestr(f"nr{i}/nr{i}.edges", (head + EOL if head else "") + EOL.join(body) + EOL)
            if L.main(["x", "nr", arc, os.path.join(work, "nr.bin"), "real"]) != 3:
                print(f"NR zip without a dynamic header was accepted as real deletions, trial {i}, layout {k}")
                fails += 1
            for variant in ("expiry", "insert"):
                a = derive("nr", arc, os.path.join(work, "nr.bin"), variant)
                b = derive("triples", plain, os.path.join(work, "tr.bin"), variant)
                if a.tobytes() != b.tobytes():
                    print(f"NR zip DIFFERS from triples, trial {i}, layout {k}, {variant}")
                    fails += 1
        # Both of columns 3 and 4 look like times: the file must fail the check
        arc = os.path.join(work, f"nr{i}-both.zip")
        with zipfile.ZipFile(arc, "w") as z:
            z.writestr("x.edges", EOL.join(f"{x} {x.split()[2]}" for x in tri_lines) + EOL)
        if L.main(["x", "nr", arc, os.path.join(work, "nr.bin"), "insert"]) != 3:
            print(f"NR zip with two time columns was accepted, trial {i}")
            fails += 1
        lines = konect_text(rng, n, m)
        lines[0] = "% sym dynamic"
        arc = os.path.join(work, f"nrk{i}.zip")
        with zipfile.ZipFile(arc, "w") as z:
            z.writestr(f"nrk{i}.edges", EOL.join(lines) + EOL)
        if as_tuples(derive("nr", arc, os.path.join(work, "nrk.bin"), "real")) != cleaner(lines):
            print(f"NR zip with deletions DIFFERS from graph_cleaner, trial {i}")
            fails += 1
        if L.main(["x", "nr", arc, os.path.join(work, "nrk.bin"), "insert"]) != 3:
            print(f"NR zip with deletions was accepted as insert-only, trial {i}")
            fails += 1
    print(f"Network Repository zips: 10 trials, {fails} failures so far")

    # 3. Every arm against the brute force, and control C3.
    checked = c3 = nz = cells = 0
    dels = sum(int((s[2]["op"] == 0).sum()) for s in streams)
    for i, path, rec in streams[:8]:
        for p in (3, 5):
            for cut in (1, 7, 40):
                truth = brute(rec, cut, p)
                cells += 1
                nz += any(truth)
                if brute(rec, cut, p, deletes=False) != truth:
                    c3 += 1
                for arm in ARMS[p]:
                    for theta in THETAS.get(arm, [0]):
                        got = engine(path, cut, p, arm, theta, work)
                        checked += 1
                        if got != truth:
                            print(f"MISMATCH trial {i} p{p} cut {cut} {arm} theta {theta}: {got[:8]} against {truth[:8]}")
                            fails += 1
    print(f"engine against brute force: {checked} arm runs, {cells} cells, {nz} with a nonzero count, deletions in the streams {dels}")
    print(f"control C3 (no deletions) differs on {c3} cells: {'FAILS as it must' if c3 else 'PASSED, test broken'}")
    fails += c3 == 0
    print("M1D07-LOADER-TEST", "PASS" if fails == 0 else f"FAIL ({fails})")
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
