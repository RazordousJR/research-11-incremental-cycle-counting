"""Figures 3 to 6 of docs/paper/outline.md, as plain SVG in the style of scripts/figures.py, from logged runs only. Read-only on results.
Usage: figures_results.py
Figure 3, the test set: every stream with its role, label, size and p = 5 passes. Figure 4, PR's or P3's or the faster of P4 and B4's median score against
pattern size, every scored cell. Figure 5, the cost model of M1D09 on the evaluation set, unit cost against priced by kind and cache. Figure 6, M5 over theta
against PR and R2 on M1D05's six passes (M1D06A). Seconds are the timing builds' on CPU 8; operations are never drawn as time."""
import csv
import glob
import math
import os
import statistics
import sys

sys.path.insert(0, "scripts")
import gate1_score as g
import m1d07_score as m
from figures import fmt, svg, text

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

    L, top, rh = 12, 58, 19
    xv, xe, bw = 330, 520, 150
    lo, hi = 3, 8  # log10 axis from 1,000 to 100 million
    X = lambda x0, n: x0 + bw * (math.log10(max(n, 10 ** lo)) - lo) / (hi - lo)
    b = [text(L, 22, "The test set: every stream, its role, label and size, and its p = 5 passes", 12, weight="bold"),
         text(L, 46, "Stream", 10, weight="bold"), text(178, 46, "Role", 10, weight="bold"), text(250, 46, "Label", 10, weight="bold"),
         text(xv, 46, "Vertices, log scale", 10, weight="bold"), text(xe, 46, "Events, log scale", 10, weight="bold"), text(720, 46, "p = 5 passes", 10, weight="bold")]
    y = top
    last = None
    for s, role, lab, nv, ne, dele in rows:
        if last is not None and role != last:
            b.append(f'<line x1="{L}" y1="{y - 6}" x2="820" y2="{y - 6}" stroke="#bbb" stroke-width="0.6"/>')
        last = role
        b.append(text(L, y + 8, s, 10))
        b.append(text(178, y + 8, role, 10, fill="#333"))
        b.append(text(250, y + 8, lab + (f", {dele:.0%}" if dele is not None else ""), 9, fill="#333"))
        for x0, n in ((xv, nv), (xe, ne)):
            if n:
                b.append(f'<rect x="{x0}" y="{y}" width="{X(x0, n) - x0:.1f}" height="11" fill="{"black" if role in ("evaluation", "held-out", "new") else "#bbb"}"/>')
                b.append(text(X(x0, n) + 3, y + 9, f"{n / 1e6:.2f} M" if n >= 1e6 else f"{n / 1e3:.0f} k", 8, fill="#333"))
        cells = [r for c, r in p5.items() if c[0] == s]
        if role in ("engineering", "snapshot"):
            b.append(text(720, y + 8, "never in a verdict", 9, fill="#555"))
        elif cells:
            b.append(text(720, y + 8, f"{sum(r['passed'] for r in cells)} of {len(cells)}", 10))
        y += rh
    for x0 in (xv, xe):
        for k in range(lo, hi + 1):
            xx = x0 + bw * (k - lo) / (hi - lo)
            b.append(f'<line x1="{xx:.1f}" y1="{y}" x2="{xx:.1f}" y2="{y + 4}" stroke="black"/>')
            b.append(text(xx, y + 15, f"1e{k}", 8, "middle"))
        b.append(f'<line x1="{x0}" y1="{y}" x2="{x0 + bw}" y2="{y}" stroke="black" stroke-width="0.6"/>')
    total = sum(r["passed"] for r in p5.values())
    b.append(text(L, y + 34, f"Black: streams in a verdict. Grey: engineering streams and the snapshot. Events of the insert-only or real-deletion stream; an expiry stream has about twice as many.", 9, fill="#333"))
    b.append(text(L, y + 48, f"KONECT wiki streams: 2,000,000-event prefixes, real deletions at the share shown. Pooled p = 5, P-core: {total} of {len(p5)} scored cells pass (M1D07E). sx-stackoverflow is the held-out stream.", 9, fill="#333"))
    open("docs/figures/fig-test-set.svg", "w", encoding="utf-8", newline="\n").write(svg(830, int(y + 60), b))
    return len(rows), total, len(p5)


def score_by_p():
    pts = []
    for src, cells in (("M1D04", g.load("results/gate1-M1D04")), ("M1D07", {c: a for c, a in m.load("results/gate1-M1D07T").items() if c[0] != m.SNAPSHOT})):
        for c in cells:
            r = g.score(c, cells[c])
            if r:
                pts.append((c, src, r))
    L, T, W, H = 70, 40, 520, 300
    lo, hi = math.log10(0.25), math.log10(8.0)
    Y = lambda v: T + H * (hi - math.log10(v)) / (hi - lo)
    xs = {3: L + 90, 4: L + 260, 5: L + 430}
    b = [text(12, 22, "Score against pattern size: sharing's median over the best other arm's, every scored cell", 12, weight="bold")]
    for v in (0.25, 0.5, 0.67, 1.0, 2.0, 4.0, 8.0):
        b.append(f'<line x1="{L}" y1="{Y(v):.1f}" x2="{L + W}" y2="{Y(v):.1f}" stroke="{"black" if v in (0.67, 1.0) else "#ddd"}" stroke-width="{0.9 if v in (0.67, 1.0) else 0.6}" stroke-dasharray="{"4 3" if v == 0.67 else ""}"/>')
        b.append(text(L - 6, Y(v) + 4, f"{v:g}", 10, "end"))
    b.append(text(L + W + 4, Y(0.67) + 4, "0.67", 9))
    b.append(text(L + W + 4, Y(1.0) + 4, "1.0", 9))
    meds = {}
    for p, x0 in xs.items():
        mine = sorted((r["score"], c, src, r) for c, src, r in pts if c[3] == p)
        meds[p] = (statistics.median(s for s, *_ in mine), len(mine), sum(r["passed"] for *_, r in mine))
        for i, (sc, c, src, r) in enumerate(mine):
            dx = (-1 if c[2] == 100 else 1) * (14 + 26 * ((i * 7919) % 13) / 13)
            x, y = x0 + dx, Y(sc)
            fill = "black" if r["passed"] else "white"
            if src == "M1D04":
                b.append(f'<rect x="{x - 3:.1f}" y="{y - 3:.1f}" width="6" height="6" fill="{fill}" stroke="black" stroke-width="0.8"/>')
            else:
                b.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="3.2" fill="{fill}" stroke="black" stroke-width="0.8"/>')
        md = meds[p][0]
        b.append(f'<line x1="{x0 - 44}" y1="{Y(md):.1f}" x2="{x0 + 44}" y2="{Y(md):.1f}" stroke="black" stroke-width="2.2"/>')
        b.append(text(x0, T + H + 18, f"p = {p}", 11, "middle", "bold"))
        b.append(text(x0, T + H + 32, f"median {md:.3f}, {meds[p][2]} of {meds[p][1]} pass", 9, "middle"))
        b.append(text(x0 - 30, T + H + 44, "cut 100", 8, "middle", fill="#555"))
        b.append(text(x0 + 30, T + H + 44, "cut 1,000", 8, "middle", fill="#555"))
    b.append(f'<line x1="{L}" y1="{T}" x2="{L}" y2="{T + H}" stroke="black" stroke-width="0.8"/>')
    b.append(f'<text x="22" y="{T + H / 2:.1f}" font-size="10" text-anchor="middle" transform="rotate(-90 22 {T + H / 2:.1f})">score, log scale</text>')
    b.append(text(12, T + H + 66, "Squares: M1D04's 4 streams (p = 4 only there). Circles: M1D07's 12 new streams. Filled: passes Gate 1's test, slowest repeat at most 0.67 x best's fastest.", 9, fill="#333"))
    b.append(text(12, T + H + 80, "Bar: median. Under 1.0 sharing is faster by median. Timing builds, CPU 8. Scored as gate1_score.score, reading M1D04-R1; the snapshot is left out.", 9, fill="#333"))
    open("docs/figures/fig-score-by-p.svg", "w", encoding="utf-8", newline="\n").write(svg(640, T + H + 92, b))
    return meds


def cost_model():
    rows = list(csv.DictReader(open("results/cost-M1D09/cells.csv", newline="")))
    lo, hi = -1.5, 3.5  # log10 seconds
    S, gap, L, T = 250, 70, 60, 46
    b = [text(12, 22, "The cost model on the 12 new streams: predicted seconds against measured", 12, weight="bold")]
    for k, (model, title) in enumerate((("U", "(a) unit cost, one price per operation"), ("KC", "(b) priced by kind, and by table size against L3"))):
        x0 = L + k * (S + gap)
        P = lambda v: (x0 + S * (math.log10(max(v, 10 ** lo)) - lo) / (hi - lo))
        Q = lambda v: (T + S - S * (math.log10(max(v, 10 ** lo)) - lo) / (hi - lo))
        b.append(text(x0, T - 8, title, 10, weight="bold"))
        b.append(f'<rect x="{x0}" y="{T}" width="{S}" height="{S}" fill="none" stroke="black" stroke-width="0.8"/>')
        b.append(f'<line x1="{x0}" y1="{T + S}" x2="{x0 + S}" y2="{T}" stroke="{GREY}" stroke-width="0.8"/>')
        for e in range(int(lo + 0.5), int(hi) + 1):
            b.append(text(P(10 ** e), T + S + 14, f"{10 ** e:g}", 9, "middle"))
            b.append(text(x0 - 4, Q(10 ** e) + 3, f"{10 ** e:g}", 9, "end"))
        for r in rows:
            x, y, a = P(float(r["seconds"])), Q(float(r[model])), r["arm"]
            if a in ("PR", "P3"):
                b.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="2.6" fill="black"/>')
            elif a in ("R2", "S3"):
                b.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="2.6" fill="{GREY}"/>')
            elif a == "WJ":
                b.append(f'<path d="M{x:.1f},{y - 3.4:.1f} L{x + 3.2:.1f},{y + 2.4:.1f} L{x - 3.2:.1f},{y + 2.4:.1f} Z" fill="white" stroke="black" stroke-width="0.9"/>')
            else:
                b.append(f'<rect x="{x - 2.4:.1f}" y="{y - 2.4:.1f}" width="4.8" height="4.8" fill="white" stroke="black" stroke-width="0.8"/>')
        b.append(text(x0 + S / 2, T + S + 30, "measured seconds, median, log", 10, "middle"))
    b.append(f'<text x="18" y="{T + S / 2:.1f}" font-size="10" text-anchor="middle" transform="rotate(-90 18 {T + S / 2:.1f})">predicted seconds, log</text>')
    y = T + S + 52
    b.append(f'<circle cx="18" cy="{y - 3}" r="2.6" fill="black"/>' + text(26, y, "sharing (PR, P3)", 9))
    b.append(f'<circle cx="128" cy="{y - 3}" r="2.6" fill="{GREY}"/>' + text(136, y, "scan (R2, S3)", 9))
    b.append(f'<rect x="216" y="{y - 6}" width="4.8" height="4.8" fill="white" stroke="black" stroke-width="0.8"/>' + text(226, y, "tables (WT, M5, A5, T3, M3)", 9))
    b.append(f'<path d="M378,{y - 7} L381,{y - 1} L375,{y - 1} Z" fill="white" stroke="black" stroke-width="0.9"/>' + text(386, y, "WJ", 9))
    b.append(text(12, y + 18, "Fitted on M1D05's 3 streams, drawn on M1D07's 12, which no coefficient saw (M1D09). Clock winner named: unit cost 21 of 64 cells, (b) 64 of 64.", 9, fill="#333"))
    b.append(text(12, y + 32, "Median error 70.1% and 22.9%. The table-size term adds nothing over pricing by kind, and (b) puts WJ at about a fifth of its time. In (a), 33 predictions under 0.03 s sit on the floor.", 9, fill="#333"))
    open("docs/figures/fig-cost-model.svg", "w", encoding="utf-8", newline="\n").write(svg(2 * S + gap + L + 20, int(y + 44), b))
    return len(rows)


def theta():
    cells = g.load("results/gate1-M1D06A")
    m4 = g.load("results/gate1-M1D04")
    passes = sorted(c for c, r in m.scored(m4, 5) if r["passed"])
    grid = [16, 32, 64, 128, 256, 512, 1024]
    med = lambda reps: statistics.median(float(x["seconds"]) for x in reps.values())
    W, H, L, T, gx, gy = 210, 130, 52, 44, 40, 64
    lo, hi = math.log10(0.25), math.log10(2.0)
    b = [text(12, 22, "M5 at every theta against PR and R2, on M1D05's six passes: time over R2's median", 12, weight="bold")]
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
        b.append(text(x0, y0 - 8, f"{c[0]} {m.label(c).replace('real timestamps, ', '')}, {fmt(c[2])}", 9, weight="bold"))
        b.append(f'<rect x="{x0}" y="{y0}" width="{W}" height="{H}" fill="none" stroke="black" stroke-width="0.7"/>')
        for v in (0.25, 0.5, 1.0, 2.0):
            b.append(f'<line x1="{x0}" y1="{Y(v):.1f}" x2="{x0 + W}" y2="{Y(v):.1f}" stroke="#ddd" stroke-width="0.5"/>')
            if k % 3 == 0:
                b.append(text(x0 - 4, Y(v) + 3, f"{v:g}", 8, "end"))
        b.append(f'<line x1="{x0}" y1="{Y(1.0):.1f}" x2="{x0 + W}" y2="{Y(1.0):.1f}" stroke="{GREY}" stroke-width="1.6"/>')
        b.append(f'<line x1="{x0}" y1="{Y(pr / r2):.1f}" x2="{x0 + W}" y2="{Y(pr / r2):.1f}" stroke="black" stroke-width="1.8"/>')
        pts = [(th, statistics.median(mt[th]) / r2) for th in grid if th in mt]
        b.append('<polyline fill="none" stroke="black" stroke-width="1" stroke-dasharray="4 2" points="' + " ".join(f"{X(t):.1f},{Y(v):.1f}" for t, v in pts) + '"/>')
        for t, v in pts:
            b.append(f'<rect x="{X(t) - 2.2:.1f}" y="{Y(v) - 2.2:.1f}" width="4.4" height="4.4" fill="white" stroke="black" stroke-width="0.8"/>')
        for t in (16, 128, 1024):
            b.append(text(X(t), y0 + H + 12, str(t), 8, "middle"))
        b.append(text(x0 + W - 2, Y(pr / r2) - 3, f"PR {pr / r2:.2f}", 8, "end"))
    y = T + 2 * H + gy + 34
    b.append(text(12, y, "Solid black: PR. Grey: R2, at 1. Dashed with squares: M5 at theta 16 to 1,024 (x axis, log). Medians of the timing repeats, CPU 8, M1D06A; y log scale.", 9, fill="#333"))
    b.append(text(12, y + 14, "No theta brings M5 under R2 by enough to turn a pass, so all six passes hold against M5 at its best theta (log.md 0035).", 9, fill="#333"))
    open("docs/figures/fig-theta.svg", "w", encoding="utf-8", newline="\n").write(svg(L + 3 * W + 2 * gx + 20, int(y + 26), b))
    return passes


if __name__ == "__main__":
    print("test set", test_set())
    print("score by p", score_by_p())
    print("cost model", cost_model())
    print("theta", theta())
