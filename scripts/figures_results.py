"""Figures 3 to 6 of docs/paper/outline.md, as plain SVG in the style of scripts/figures.py, from logged runs only. Read-only on results.
Usage: figures_results.py
Figure 3, the test set: every stream with its role, label, size and p = 5 passes. Figure 4, PR's or P3's or the faster of P4 and B4's median score against
pattern size, every scored cell. Figure 5, the cost model on M1D07's streams, unit cost and priced by kind and cache fitted on the engineering cells (M1D10), and priced by kind and cache fitted on M1D05's (M1D09). Figure 6, M5 over theta
against PR and R2 on M1D05's six passes (M1D06A). Seconds are the timing builds' on CPU 8; operations are never drawn as time.
Sized for JGAA as in scripts/figures.py: WIDTH units wide, printed at the text width, no text under 10 pt. Titles and captions go to docs/figures/captions.md."""
import csv
import glob
import math
import os
import statistics
import sys

sys.path.insert(0, "scripts")
import gate1_score as g
import m1d07_score as m
from figures import WIDTH, caption, fmt, svg, text

GREY = "#888"


def one_summary(pattern):
    paths = sorted(glob.glob(pattern))
    return g.one(paths[0]) if paths else None


def pooled_p5():
    """Pooled p = 5 verdicts, as M1D07E reads them: M1D04's cells and M1D07's, snapshot aside."""
    return dict(m.scored(g.load("results/gate1-M1D04"), 5) + m.scored({c: a for c, a in m.load("results/gate1-M1D07T").items() if c[0] != m.SNAPSHOT}, 5))


def test_set():
    p5 = pooled_p5()
    eng = [("CollegeMsg", "data/derived"), ("email-Eu-core-temporal", "data/derived"), ("sx-mathoverflow", "data/derived")]
    rows = []
    for s, _ in eng:
        r = one_summary(f"results/gate1-M1D02/raw/{s}-insert-1000-p5-R2*.summary.csv")
        rows.append((s, "engineering", "expiry, insert", int(r["vertices"]) if r and r.get("vertices") else None, int(r["events"]) if r else None, None))
    for s in ("sx-askubuntu", "sx-superuser", "wiki-talk-temporal", "sx-stackoverflow"):
        r = one_summary(f"results/gate1-M1D04/raw/{s}-insert-1000-p5-*.t1.summary.csv")
        rows.append((s, "held-out" if s == "sx-stackoverflow" else "evaluation", "expiry, insert", int(r["vertices"]), int(r["events"]), None))
    for path in sorted(glob.glob("data/m1d07/derived/*.bin.txt")):
        s, v = os.path.basename(path).split(".")[:2]
        if v == "expiry":
            continue
        kv = dict(x.split("=", 1) for x in open(path, encoding="utf-8").read().split() if "=" in x)
        role = "snapshot" if s == m.SNAPSHOT else "new"
        lab = "real deletions" if v == "real" else "static, file order" if role == "snapshot" else "expiry, insert"
        rows.append((s, role, lab, int(kv["vertices_in_prefix"]), int(kv["events"]), float(kv["deletion_share"]) if v == "real" else None))
    order = {"engineering": 0, "evaluation": 1, "held-out": 2, "new": 3, "snapshot": 4}
    rows.sort(key=lambda r: (order[r[1]], r[2] != "real deletions", r[4] or 0))

    # Rows are grouped by role and label: each group gets a header row, so the role and label columns go and the table fits the text width.
    L, top, rh = 2, 42, 14
    xv, xe, bw, xp = 134, 240, 80, WIDTH - 2
    lo, hi = 3, 8  # log10 axis from 1,000 to 100 million
    X = lambda x0, n: x0 + bw * (math.log10(max(n, 10 ** lo)) - lo) / (hi - lo)
    b = [text(L, 12, "Stream", 10, weight="bold"),
         text(xv, 12, "Vertices,", 10, weight="bold"), text(xv, 24, "log scale", 10, weight="bold"),
         text(xe, 12, "Events,", 10, weight="bold"), text(xe, 24, "log scale", 10, weight="bold"),
         text(xp, 12, "p = 5", 10, "end", "bold"), text(xp, 24, "passes", 10, "end", "bold")]
    y = top
    last = None
    for s, role, lab, nv, ne, dele in rows:
        if (role, lab) != last:
            if last is not None:
                b.append(f'<line x1="{L}" y1="{y + 1}" x2="{WIDTH - 2}" y2="{y + 1}" stroke="#bbb" stroke-width="0.6"/>')
                y += 4
            head = f"{role}: {lab}" + (", share deleted in brackets" if dele is not None else "") + ("; never in a verdict" if role in ("engineering", "snapshot") else "")
            b.append(text(L, y + 10, head, 10, weight="bold", fill="#333"))
            y += rh
        last = (role, lab)
        b.append(text(L + 8, y + 10, s + (f" ({dele:.0%})" if dele is not None else ""), 10))
        for x0, n in ((xv, nv), (xe, ne)):
            if n:
                b.append(f'<rect x="{x0}" y="{y + 2}" width="{X(x0, n) - x0:.1f}" height="9" fill="{"black" if role in ("evaluation", "held-out", "new") else "#bbb"}"/>')
                b.append(text(X(x0, n) + 3, y + 10, f"{n / 1e6:.2f} M" if n >= 1e6 else f"{n / 1e3:.0f} k", 10, fill="#333"))
        cells = [r for c, r in p5.items() if c[0] == s]
        if role in ("engineering", "snapshot"):
            b.append(text(xp, y + 10, "&#8211;", 10, "end", fill="#555"))
        elif cells:
            b.append(text(xp, y + 10, f"{sum(r['passed'] for r in cells)} of {len(cells)}", 10, "end"))
        y += rh
    y += 3
    for x0 in (xv, xe):
        for k in range(lo, hi + 1):
            xx = x0 + bw * (k - lo) / (hi - lo)
            b.append(f'<line x1="{xx:.1f}" y1="{y}" x2="{xx:.1f}" y2="{y + 4}" stroke="black" stroke-width="0.7"/>')
            if k % 2:
                b.append(text(xx, y + 15, f"1e{k}", 10, "middle"))
        b.append(f'<line x1="{x0}" y1="{y}" x2="{x0 + bw}" y2="{y}" stroke="black" stroke-width="0.6"/>')
    ly = y + 34
    b.append(f'<rect x="{L}" y="{ly - 8}" width="14" height="9" fill="black"/>' + text(L + 19, ly, "in a verdict", 10))
    b.append(f'<rect x="{L + 90}" y="{ly - 8}" width="14" height="9" fill="#bbb"/>' + text(L + 109, ly, "never in a verdict", 10))
    open("docs/figures/fig-test-set.svg", "w", encoding="utf-8", newline="\n").write(svg(WIDTH, int(ly + 8), b))
    total = sum(r["passed"] for r in p5.values())
    caption(3, "fig-test-set.svg", [
        f"Black: streams in a verdict. Grey: engineering streams and the snapshot. Events of the insert-only or real-deletion stream; an expiry stream has about twice as many.",
        f"KONECT wiki streams: 2,000,000-event prefixes, real deletions at the share shown. Pooled p = 5, P-core: {total} of {len(p5)} scored cells pass (M1D07E). sx-stackoverflow is the held-out stream."],
        "The test set: every stream, its role, label and size, and its p = 5 passes")
    return len(rows), total, len(p5)


def score_by_p():
    pts = []
    for src, cells in (("M1D04", g.load("results/gate1-M1D04")), ("M1D07", {c: a for c, a in m.load("results/gate1-M1D07T").items() if c[0] != m.SNAPSHOT})):
        for c in cells:
            r = g.score(c, cells[c])
            if r:
                pts.append((c, src, r))
    L, T, W, H = 42, 8, 355, 270
    lo, hi = math.log10(0.25), math.log10(8.0)
    Y = lambda v: T + H * (hi - math.log10(v)) / (hi - lo)
    xs = {3: L + 61, 4: L + 178, 5: L + 295}
    b = []
    for v in (0.25, 0.5, 0.67, 1.0, 2.0, 4.0, 8.0):
        b.append(f'<line x1="{L}" y1="{Y(v):.1f}" x2="{L + W}" y2="{Y(v):.1f}" stroke="{"black" if v in (0.67, 1.0) else "#ddd"}" stroke-width="{0.8 if v in (0.67, 1.0) else 0.5}" stroke-dasharray="{"3 2.5" if v == 0.67 else ""}"/>')
        b.append(text(L - 5, Y(v) + 3.5, f"{v:g}", 10, "end"))
    b.append(text(L + W + 4, Y(0.67) + 3.5, "0.67", 10))
    b.append(text(L + W + 4, Y(1.0) + 3.5, "1.0", 10))
    meds = {}
    for p, x0 in xs.items():
        mine = sorted((r["score"], c, src, r) for c, src, r in pts if c[3] == p)
        meds[p] = (statistics.median(s for s, *_ in mine), len(mine), sum(r["passed"] for *_, r in mine))
        for i, (sc, c, src, r) in enumerate(mine):
            dx = (-1 if c[2] == 100 else 1) * (9 + 19 * ((i * 7919) % 13) / 13)
            x, y = x0 + dx, Y(sc)
            fill = "black" if r["passed"] else "white"
            if src == "M1D04":
                b.append(f'<rect x="{x - 2.6:.1f}" y="{y - 2.6:.1f}" width="5.2" height="5.2" fill="{fill}" stroke="black" stroke-width="0.7"/>')
            else:
                b.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="2.8" fill="{fill}" stroke="black" stroke-width="0.7"/>')
        md = meds[p][0]
        b.append(f'<line x1="{x0 - 32}" y1="{Y(md):.1f}" x2="{x0 + 32}" y2="{Y(md):.1f}" stroke="black" stroke-width="1.8"/>')
        b.append(text(x0, T + H + 16, f"p = {p}", 11, "middle", "bold"))
        b.append(text(x0, T + H + 29, f"median {md:.3f}", 10, "middle"))
        b.append(text(x0, T + H + 42, f"{meds[p][2]} of {meds[p][1]} pass", 10, "middle"))
        b.append(text(x0 - 22, T + H + 56, "cut 100", 10, "middle", fill="#555"))
        b.append(text(x0 + 22, T + H + 56, "cut 1,000", 10, "middle", fill="#555"))
    b.append(f'<line x1="{L}" y1="{T}" x2="{L}" y2="{T + H}" stroke="black" stroke-width="0.7"/>')
    b.append(f'<text x="11" y="{T + H / 2:.1f}" font-size="10" text-anchor="middle" transform="rotate(-90 11 {T + H / 2:.1f})">score, log scale</text>')
    ly = T + H + 78
    b.append(f'<rect x="{L - 2.6:.1f}" y="{ly - 6.6:.1f}" width="5.2" height="5.2" fill="white" stroke="black" stroke-width="0.7"/>' + text(L + 6, ly, "first 4 streams", 10))
    b.append(f'<circle cx="{L + 108}" cy="{ly - 4}" r="2.8" fill="white" stroke="black" stroke-width="0.7"/>' + text(L + 114, ly, "12 new streams", 10))
    b.append(f'<rect x="{L + 237.4:.1f}" y="{ly - 6.6:.1f}" width="5.2" height="5.2" fill="black" stroke="black" stroke-width="0.7"/>'
             f'<circle cx="{L + 248}" cy="{ly - 4}" r="2.8" fill="black" stroke="black" stroke-width="0.7"/>' + text(L + 254, ly, "passes", 10))
    b.append(f'<line x1="{L + 300}" y1="{ly - 4}" x2="{L + 318}" y2="{ly - 4}" stroke="black" stroke-width="1.8"/>' + text(L + 322, ly, "median", 10))
    open("docs/figures/fig-score-by-p.svg", "w", encoding="utf-8", newline="\n").write(svg(WIDTH, ly + 6, b))
    caption(4, "fig-score-by-p.svg", [
        "Squares: M1D04's 4 streams (p = 4 only there). Circles: M1D07's 12 new streams. Filled: passes Gate 1's test, slowest repeat at most 0.67 x best's fastest.",
        "Bar: median. Under 1.0 sharing is faster by median. Timing builds, CPU 8. Scored as gate1_score.score, reading M1D04-R1; the snapshot is left out."],
        "Score against pattern size: sharing's median over the best other arm's, every scored cell")
    return meds


def cost_model():
    """Three panels on M1D07's 12 new streams, which no coefficient saw: unit cost and priced by kind and cache, both fitted on the engineering cells (M1D10),
    and priced by kind and cache fitted on M1D05's streams (M1D09), beside it."""
    r10 = [r for r in csv.DictReader(open("results/cost-M1D10/cells.csv", newline="")) if r["set"] == "M1D07"]
    r09 = list(csv.DictReader(open("results/cost-M1D09/cells.csv", newline="")))
    lo, hi = -1.5, 3.5  # log10 seconds
    S, gap, rgap, L, T = 168, 36, 62, 46, 32  # a 2 x 2 grid: (a) (b) on top, (c) and the legend below
    b = []
    panels = ((r10, "U", "(a) unit cost, engineering fit"), (r10, "KC", "(b) by kind and cache, engineering fit"), (r09, "KC", "(c) by kind and cache, evaluation fit"))
    for k, (rows, model, title) in enumerate(panels):
        x0, y0 = L + (k % 2) * (S + gap), T + (k // 2) * (S + rgap)
        P = lambda v: (x0 + S * (math.log10(max(v, 10 ** lo)) - lo) / (hi - lo))
        Q = lambda v: (y0 + S - S * (math.log10(max(v, 10 ** lo)) - lo) / (hi - lo))
        head, tail = title.rsplit(", ", 1)
        b.append(text(x0, y0 - 19, head + ",", 10, weight="bold"))
        b.append(text(x0, y0 - 7, tail, 10, weight="bold"))
        b.append(f'<rect x="{x0}" y="{y0}" width="{S}" height="{S}" fill="none" stroke="black" stroke-width="0.7"/>')
        b.append(f'<line x1="{x0}" y1="{y0 + S}" x2="{x0 + S}" y2="{y0}" stroke="{GREY}" stroke-width="0.7"/>')
        for e in range(int(lo + 0.5), int(hi) + 1):
            b.append(text(P(10 ** e), y0 + S + 13, f"{10 ** e:g}", 10, "middle"))
            b.append(text(x0 - 4, Q(10 ** e) + 3.5, f"{10 ** e:g}", 10, "end"))
        for r in rows:
            x, y, a = P(float(r["seconds"])), Q(float(r[model])), r["arm"]
            if a in ("PR", "P3"):
                b.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="2.2" fill="black"/>')
            elif a in ("R2", "S3"):
                b.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="2.2" fill="{GREY}"/>')
            elif a == "WJ":
                b.append(f'<path d="M{x:.1f},{y - 2.9:.1f} L{x + 2.7:.1f},{y + 2.0:.1f} L{x - 2.7:.1f},{y + 2.0:.1f} Z" fill="white" stroke="black" stroke-width="0.7"/>')
            else:
                b.append(f'<rect x="{x - 2.0:.1f}" y="{y - 2.0:.1f}" width="4.0" height="4.0" fill="white" stroke="black" stroke-width="0.7"/>')
        b.append(text(x0 + S / 2, y0 + S + 27, "measured seconds, median, log", 10, "middle"))
        if k % 2 == 0:
            b.append(f'<text x="11" y="{y0 + S / 2:.1f}" font-size="10" text-anchor="middle" transform="rotate(-90 11 {y0 + S / 2:.1f})">predicted seconds, log</text>')
    floor = sum(1 for r in r10 if float(r["U"]) < 10 ** lo)
    lx, y = L + S + gap + 8, T + S + rgap + S / 2 - 22  # the legend sits in the fourth cell
    b.append(f'<circle cx="{lx + 3}" cy="{y - 3.5}" r="2.2" fill="black"/>' + text(lx + 12, y, "sharing (PR, P3)", 10))
    b.append(f'<circle cx="{lx + 3}" cy="{y + 14.5}" r="2.2" fill="{GREY}"/>' + text(lx + 12, y + 18, "scan (R2, S3)", 10))
    b.append(f'<rect x="{lx + 1}" y="{y + 30.5}" width="4.0" height="4.0" fill="white" stroke="black" stroke-width="0.7"/>' + text(lx + 12, y + 36, "tables, recount (WT, M5, A5, T3, M3)", 10))
    b.append(f'<path d="M{lx + 3},{y + 47.6} L{lx + 5.7},{y + 52.5} L{lx + 0.3},{y + 52.5} Z" fill="white" stroke="black" stroke-width="0.7"/>' + text(lx + 12, y + 54, "WJ", 10))
    open("docs/figures/fig-cost-model.svg", "w", encoding="utf-8", newline="\n").write(svg(WIDTH, T + 2 * S + rgap + 32, b))
    caption(5, "fig-cost-model.svg", [
        "(a) and (b) fitted on the 3 engineering streams (M1D10), (c) on M1D05's 3 streams (M1D09); all drawn on M1D07's 12, which no coefficient saw.",
        "Clock winner named: (a) 21 of 64 cells, (b) 63, (c) 64. Median error 71.3%, 23.0% and 22.9%. The table-size term adds nothing over pricing by kind in either fit,",
        f"and both put WJ at a fifth of its time or less. In (a), {floor} predictions under 0.03 s sit on the floor."],
        "The cost model on the 12 new streams: predicted seconds against measured")
    return len(r10), len(r09)


def theta():
    cells = g.load("results/gate1-M1D06A")
    m4 = g.load("results/gate1-M1D04")
    passes = sorted(c for c, r in m.scored(m4, 5) if r["passed"])
    grid = [16, 32, 64, 128, 256, 512, 1024]
    med = lambda reps: statistics.median(float(x["seconds"]) for x in reps.values())
    W, H, L, T, gx, gy = 104, 92, 40, 30, 28, 52
    lo, hi = math.log10(0.25), math.log10(2.0)
    b = []
    for k, c in enumerate(passes):
        mt = {}
        for path in glob.glob(f"results/gate1-M1D06A/raw/{c[0]}-{c[1]}-{c[2]}-p5-M5-th*.t*.summary.csv"):
            th = int(os.path.basename(path).split("-th")[1].split(".")[0])
            r = g.one(path)
            if r["status"] == "OK":
                mt.setdefault(th, []).append(float(r["seconds"]))
        arms = cells[c]
        r2, pr = med(arms["R2"]), med(arms["PR"])
        x0, y0 = L + (k % 3) * (W + gx), T + (k // 3) * (H + gy)
        X = lambda th: x0 + W * math.log2(th / 16) / 6
        Y = lambda v: y0 + H * (hi - math.log10(v)) / (hi - lo)
        b.append(text(x0, y0 - 19, c[0], 10, weight="bold"))
        b.append(text(x0, y0 - 7, f"{m.label(c).replace('real timestamps, ', '')}, {fmt(c[2])}", 10))
        b.append(f'<rect x="{x0}" y="{y0}" width="{W}" height="{H}" fill="none" stroke="black" stroke-width="0.7"/>')
        for v in (0.25, 0.5, 1.0, 2.0):
            b.append(f'<line x1="{x0}" y1="{Y(v):.1f}" x2="{x0 + W}" y2="{Y(v):.1f}" stroke="#ddd" stroke-width="0.5"/>')
            if k % 3 == 0:
                b.append(text(x0 - 4, Y(v) + 3.5, f"{v:g}", 10, "end"))
        b.append(f'<line x1="{x0}" y1="{Y(1.0):.1f}" x2="{x0 + W}" y2="{Y(1.0):.1f}" stroke="{GREY}" stroke-width="1.4"/>')
        b.append(f'<line x1="{x0}" y1="{Y(pr / r2):.1f}" x2="{x0 + W}" y2="{Y(pr / r2):.1f}" stroke="black" stroke-width="1.5"/>')
        pts = [(th, statistics.median(mt[th]) / r2) for th in grid if th in mt]
        b.append('<polyline fill="none" stroke="black" stroke-width="0.9" stroke-dasharray="3.5 2" points="' + " ".join(f"{X(t):.1f},{Y(v):.1f}" for t, v in pts) + '"/>')
        for t, v in pts:
            b.append(f'<rect x="{X(t) - 2.0:.1f}" y="{Y(v) - 2.0:.1f}" width="4.0" height="4.0" fill="white" stroke="black" stroke-width="0.7"/>')
        for t in (16, 128, 1024):
            b.append(text(X(t), y0 + H + 12, str(t), 10, "middle"))
        b.append(text(x0 + W - 2, Y(pr / r2) - 3, f"PR {pr / r2:.2f}", 10, "end"))
    gb = T + 2 * H + gy  # bottom of the grid
    b.append(text(L + (3 * W + 2 * gx) / 2, gb + 27, "theta, log scale", 10, "middle"))
    b.append(f'<text x="11" y="{T + (2 * H + gy) / 2:.1f}" font-size="10" text-anchor="middle" transform="rotate(-90 11 {T + (2 * H + gy) / 2:.1f})">time over R2\'s median, log scale</text>')
    ly = gb + 46
    b.append(f'<line x1="{L}" y1="{ly - 3.5}" x2="{L + 18}" y2="{ly - 3.5}" stroke="black" stroke-width="1.5"/>' + text(L + 22, ly, "PR", 10))
    b.append(f'<line x1="{L + 50}" y1="{ly - 3.5}" x2="{L + 68}" y2="{ly - 3.5}" stroke="{GREY}" stroke-width="1.4"/>' + text(L + 72, ly, "R2, at 1", 10))
    b.append(f'<line x1="{L + 125}" y1="{ly - 3.5}" x2="{L + 149}" y2="{ly - 3.5}" stroke="black" stroke-width="0.9" stroke-dasharray="3.5 2"/>'
             f'<rect x="{L + 135}" y="{ly - 5.5}" width="4.0" height="4.0" fill="white" stroke="black" stroke-width="0.7"/>' + text(L + 153, ly, "M5 at theta 16 to 1,024", 10))
    open("docs/figures/fig-theta.svg", "w", encoding="utf-8", newline="\n").write(svg(WIDTH, int(ly + 6), b))
    caption(6, "fig-theta.svg", [
        "Solid black: PR. Grey: R2, at 1. Dashed with squares: M5 at theta 16 to 1,024 (x axis, log). Medians of the timing repeats, CPU 8, M1D06A; y log scale.",
        "No theta brings M5 under R2 by enough to turn a pass, so all six passes hold against M5 at its best theta (log.md 0035)."],
        "M5 at every theta against PR and R2, on M1D05's six passes: time over R2's median")
    return passes


if __name__ == "__main__":
    print("test set", test_set())
    print("score by p", score_by_p())
    print("cost model", cost_model())
    print("theta", theta())
