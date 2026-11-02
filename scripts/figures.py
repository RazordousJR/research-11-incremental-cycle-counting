"""The two §16 figures, as plain SVG, from logged runs only. No plotting library: the machine has none, and SVG is safe in greyscale and at column width.
Usage: figures.py
Figure 1, the worked batch: docs/figures/worked-batch.csv from build/worked_batch.exe on sx-askubuntu expiry, cut 1,000, batch 465, chosen by the rule in
docs/figures/README.md. Figure 2, the stream timeline: per-batch counts and operations of the count builds, M1D03 for R2, PR, WT and WJ, M1D04 for M5 at theta 128.
Operations, not seconds: the count build carries counters and is never timed."""
import csv
import math

CELL = "sx-askubuntu-expiry-1000-p5"
LABEL = "sx-askubuntu, real timestamps, synthetic expiry, cut 1,000, p = 5"
PICK = 465
FONT = 'font-family="Helvetica, Arial, sans-serif"'


def svg(w, h, body):
    return (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {w} {h}" width="{w}" height="{h}" {FONT}>\n'
            f'<rect width="{w}" height="{h}" fill="white"/>\n' + "\n".join(body) + "\n</svg>\n")


def text(x, y, s, size=11, anchor="start", weight="normal", fill="black"):
    return f'<text x="{x:.1f}" y="{y:.1f}" font-size="{size}" text-anchor="{anchor}" font-weight="{weight}" fill="{fill}">{s}</text>'


def fmt(n):
    return f"{n:,}"


def worked_batch():
    rows = list(csv.reader(open("docs/figures/worked-batch.csv", newline="")))
    head, last = rows[0], rows[-1]
    spokes = [dict(zip(head, r)) for r in rows[1:-1]]
    tail = dict(zip(last[0::2], last[1::2]))
    assert int(tail["batch"]) == PICK
    groups = {}
    for r in spokes:
        groups.setdefault((r["pass"], r["group"]), []).append(r)
    big = min(groups.values(), key=lambda v: (-len(v), int(v[0]["group"])))
    logged_pr = next(r for i, r in enumerate(csv.DictReader(open(f"results/gate1-M1D03/raw/{CELL}-PR.csv", newline=""))) if i == PICK)
    logged_r2 = next(r for i, r in enumerate(csv.DictReader(open(f"results/gate1-M1D03/raw/{CELL}-R2.csv", newline=""))) if i == PICK)
    assert int(logged_pr["ops"]) == int(tail["pr_ops"]) and int(logged_pr["t"]) == int(tail["t"]), "the tool must reproduce the logged row"

    b = []
    b.append(text(12, 22, "(a) The largest star of the batch", 12, weight="bold"))
    cx, cy, R = 165, 190, 120
    k = len(big)
    dmax = max(int(r["delta"]) for r in big)
    for i, r in enumerate(big):
        ang = -math.pi / 2 + 2 * math.pi * i / k
        x, y = cx + R * math.cos(ang), cy + R * math.sin(ang)
        d = int(r["delta"])
        if d:
            wdt = 1.2 + 5.0 * math.sqrt(d / dmax)
            b.append(f'<line x1="{cx}" y1="{cy}" x2="{x:.1f}" y2="{y:.1f}" stroke="black" stroke-width="{wdt:.2f}"/>')
            lx, ly = cx + (R + 16) * math.cos(ang), cy + (R + 16) * math.sin(ang) + 4
            b.append(text(lx, ly, str(d), 10, "middle"))
        else:
            b.append(f'<line x1="{cx}" y1="{cy}" x2="{x:.1f}" y2="{y:.1f}" stroke="#777" stroke-width="0.8" stroke-dasharray="3 2"/>')
        b.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="3.2" fill="white" stroke="black" stroke-width="1"/>')
    b.append(f'<circle cx="{cx}" cy="{cy}" r="17" fill="white" stroke="black" stroke-width="1.6"/>')
    b.append(text(cx, cy + 4, "x", 12, "middle", "bold"))
    pr_g = int(big[0]["pr_group_ops"])
    r2_g = sum(int(r["r2_ops"]) for r in big)
    kind = "inserted" if big[0]["pass"] == "1" else "deleted"
    b.append(text(12, 348, f"Centre x = vertex {big[0]['centre']}, degree {big[0]['centre_degree']}, {k} edges {kind}.", 10))
    b.append(text(12, 362, f"Edge labels: 5-cycles each edge adds, {fmt(int(big[0]['pr_group_delta']))} in all. Dashed: adds none.", 10))
    b.append(text(12, 376, f"PR builds one row at x: {fmt(pr_g)} operations.", 10))
    b.append(text(12, 390, f"R2, one row per edge: {fmt(r2_g)} operations.", 10))

    ox = 345
    b.append(text(ox, 22, "(b) The whole batch", 12, weight="bold"))
    classes = [("1", 1, 1), ("2", 2, 2), ("3-4", 3, 4), ("5-8", 5, 8), ("9+", 9, 10 ** 9)]
    cnt = [sum(1 for v in groups.values() if lo <= len(v) <= hi) for _, lo, hi in classes]
    sp = [sum(len(v) for v in groups.values() if lo <= len(v) <= hi) for _, lo, hi in classes]
    top, base, bw, gap = 48, 200, 34, 12
    m = max(cnt)
    b.append(text(ox, 40, f"Stars by number of edges, {len(groups)} stars, {len(spokes)} net edits", 10))
    for i, ((name, _, _), c, s) in enumerate(zip(classes, cnt, sp)):
        h = (base - top - 14) * c / m
        x = ox + 8 + i * (bw + gap)
        b.append(f'<rect x="{x}" y="{base - h:.1f}" width="{bw}" height="{h:.1f}" fill="#bbb" stroke="black" stroke-width="0.8"/>')
        b.append(text(x + bw / 2, base - h - 4, str(c), 10, "middle"))
        b.append(text(x + bw / 2, base + 13, name, 10, "middle"))
        b.append(text(x + bw / 2, base + 25, f"{s} e", 9, "middle", fill="#333"))
    b.append(f'<line x1="{ox}" y1="{base}" x2="{ox + 5 * (bw + gap) + 8}" y2="{base}" stroke="black" stroke-width="0.8"/>')
    b.append(text(ox, base + 38, "Edges per star (e: edits in that class)", 10))

    pr, r2 = int(logged_pr["ops"]), int(logged_r2["ops"])
    y0, full = 272, 160
    b.append(text(ox, y0 - 12, "Operations for the batch, as logged", 10))
    for j, (name, v, fill) in enumerate((("PR", pr, "black"), ("R2", r2, "#999"))):
        wv = full * v / max(pr, r2)
        b.append(f'<rect x="{ox + 26}" y="{y0 + j * 26}" width="{wv:.1f}" height="16" fill="{fill}"/>')
        b.append(text(ox, y0 + j * 26 + 12, name, 10))
        b.append(text(ox + 30 + wv, y0 + j * 26 + 12, fmt(v), 10))
    b.append(text(ox, y0 + 70, f"PR over R2: {pr / r2:.3f}. Count after the batch: {fmt(int(tail['t']))}.", 10))
    b.append(text(12, 414, f"{LABEL}. Batch {PICK} of 1,454: {fmt(int(logged_pr['raw']))} raw events, {fmt(int(logged_pr['net']))} net edits.", 10, fill="#333"))
    b.append(text(12, 428, "Count build, M1D03 rows for PR and R2; PR's batch reproduced exactly by build/worked_batch.exe. Operations, not time.", 10, fill="#333"))
    open("docs/figures/fig-worked-batch.svg", "w", encoding="utf-8", newline="\n").write(svg(600, 440, b))
    return dict(groups=len(groups), big=k, pr_g=pr_g, r2_g=r2_g, pr=pr, r2=r2)


def series(path):
    return [(int(r["ops"]), int(r["t"]), int(r["net"])) for r in csv.DictReader(open(path, newline=""))]


def timeline():
    arms = [("PR", f"results/gate1-M1D03/raw/{CELL}-PR.csv", "black", "", 2.2),
            ("R2", f"results/gate1-M1D03/raw/{CELL}-R2.csv", "#888", "", 2.0),
            ("M5", f"results/gate1-M1D04/count/raw/{CELL}-M5-th128.csv", "black", "6 3", 1.4),
            ("WT", f"results/gate1-M1D03/raw/{CELL}-WT.csv", "black", "1.5 2.5", 1.6),
            ("WJ", f"results/gate1-M1D03/raw/{CELL}-WJ.csv", "#555", "8 3 2 3", 1.4)]
    data = {a: series(p) for a, p, *_ in arms}
    n = len(data["PR"])
    assert all(len(v) == n for v in data.values())
    assert all([x[1] for x in v] == [x[1] for x in data["PR"]] for v in data.values()), "every arm's count must agree per batch"
    L, Rr, W = 62, 92, 600
    pw = W - L - Rr
    b = []
    X = lambda i: L + pw * i / (n - 1)

    t = [x[1] for x in data["PR"]]
    tmax = max(t)
    t0, t1 = 36, 150
    Y = lambda v: t1 - (t1 - t0) * v / tmax
    b.append(text(12, 22, "(a) 5-cycles in the graph after each batch", 12, weight="bold"))
    b.append('<polyline fill="none" stroke="black" stroke-width="1.2" points="' + " ".join(f"{X(i):.1f},{Y(v):.1f}" for i, v in enumerate(t)) + '"/>')
    for v in (0, tmax // 2, tmax):
        b.append(f'<line x1="{L - 4}" y1="{Y(v):.1f}" x2="{L}" y2="{Y(v):.1f}" stroke="black"/>')
        b.append(text(L - 7, Y(v) + 4, fmt(v), 10, "end"))
    b.append(f'<line x1="{L}" y1="{t0}" x2="{L}" y2="{t1}" stroke="black" stroke-width="0.8"/>')

    cum = {a: [] for a in data}
    for a, v in data.items():
        s = 0
        for x in v:
            s += x[0]
            cum[a].append(s)
    cmax = max(c[-1] for c in cum.values())
    c0, c1 = 196, 372
    Yc = lambda v: c1 - (c1 - c0) * v / cmax
    b.append(text(12, 182, "(b) Operations so far, each arm", 12, weight="bold"))
    ends = []
    for a, _, col, dash, wdt in arms:
        pts = " ".join(f"{X(i):.1f},{Yc(v):.1f}" for i, v in enumerate(cum[a]) if i % 4 == 0 or i == n - 1)
        b.append(f'<polyline fill="none" stroke="{col}" stroke-width="{wdt}" stroke-dasharray="{dash}" points="{pts}"/>')
        ends.append((Yc(cum[a][-1]), a, cum[a][-1]))
    ends.sort()
    last = -99
    for y, a, v in ends:
        y = max(y, last + 13)
        last = y
        b.append(text(L + pw + 6, y + 4, f"{a} {v / 1e6:.0f} M", 10))
    for v in (0, cmax // 2, cmax):
        b.append(f'<line x1="{L - 4}" y1="{Yc(v):.1f}" x2="{L}" y2="{Yc(v):.1f}" stroke="black"/>')
        b.append(text(L - 7, Yc(v) + 4, f"{v / 1e6:.0f} M", 10, "end"))
    b.append(f'<line x1="{L}" y1="{c0}" x2="{L}" y2="{c1}" stroke="black" stroke-width="0.8"/>')
    b.append(f'<line x1="{L}" y1="{c1}" x2="{L + pw}" y2="{c1}" stroke="black" stroke-width="0.8"/>')
    for i in (0, 500, 1000, n - 1):
        b.append(f'<line x1="{X(i):.1f}" y1="{c1}" x2="{X(i):.1f}" y2="{c1 + 4}" stroke="black"/>')
        b.append(text(X(i), c1 + 16, fmt(i), 10, "middle"))
    b.append(text(L + pw / 2, c1 + 30, "Batch", 10, "middle"))
    b.append(f'<line x1="{X(PICK):.1f}" y1="{t0 - 4}" x2="{X(PICK):.1f}" y2="{c1}" stroke="black" stroke-width="0.6" stroke-dasharray="2 2"/>')
    b.append(text(X(PICK) + 4, t0 + 10, f"batch {PICK}, Figure 1", 9))
    b.append(text(12, 418, f"{LABEL}, {fmt(n)} batches.", 10, fill="#333"))
    b.append(text(12, 432, "Count builds: M1D03 for PR, R2, WT and WJ; M1D04 for M5 at theta 128. Operations, not time.", 10, fill="#333"))
    b.append(text(12, 446, "Every arm's count agrees on every batch. The expiry rule empties the graph by the end.", 10, fill="#333"))
    open("docs/figures/fig-stream-timeline.svg", "w", encoding="utf-8", newline="\n").write(svg(600, 456, b))
    return {a: c[-1] for a, c in cum.items()}, tmax, n


if __name__ == "__main__":
    print("worked batch", worked_batch())
    print("timeline", timeline())
