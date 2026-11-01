"""Tune (a, theta_t) for M1B03, one graph at a time, one chain process at a time.

Usage: python -I scripts/tune_m1b03.py <graph> <burn> <pilot_N> <coarse|fine> <trials.csv> <chosen.csv>

The model has two parameters. a is the edge parameter measured from the density value: theta_e = a - log(D / m0).
theta_t is the alternating k-triangle parameter. The tuning rule was fixed before any production run:

  A pair passes when the counted acceptance is within 10% of the target, the run stays in the band [0.03, 1.5] m0, and
  the edge count drifts by at most 20% per 10^7 proposals over the counted phase. Among passing pairs, keep the one
  with the largest edge count at the end, which keeps the most structure and is the setting kindest to the tables.

  Mode "grid" (the two peak snapshots, about 2 s a run) walks a full (a, theta_t) grid. Acceptance there is not
  monotone in a, because the chain has a melted state and a dense state, so a solve on a alone does not converge.
  Mode "coarse" (large graphs, 30 to 90 s a run) walks theta_t and solves a by a <- a + log(target / acc), at most
  three tries each. Stationary probes on those graphs showed acceptance close to 2 exp(a) for this reason.

The same (a, theta_t) then goes to every backend, because they run in lockstep on one chain.
"""
import csv
import math
import os
import re
import subprocess
import sys

EXE = os.path.abspath("build/ergm_chain.exe")
TARGETS = [0.15, 0.05, 0.01]
GRID = {
    "grid": [3, 3.25, 3.5, 3.75, 4, 4.25, 4.5, 4.75, 5, 5.5, 6, 7, 8, 10, 14],
    "coarse": [3, 4, 5, 6, 8],
}


def pilot(graph, a, tt, burn, n):
    args = [EXE, "run", f"graph={graph}", f"a={a:.6f}", f"tt={tt}", f"N={n}", f"burn={burn}", "hubs=", "memos=4:3",
            "nomemo=0", "eager=0", "checks=1", "maxm=1.5", "minm=0.03", "dyads=0", "tables=0"]
    out = subprocess.run(args, capture_output=True, text=True, timeout=590).stdout
    left = re.search(r"LEFT_BAND (\w+) step=(\d+) m=(\d+)", out)
    if left:
        m0 = float(re.search(r"m0=(\d+)", out).group(1))
        status = "dense" if int(left.group(3)) > m0 else "melted"
        return {"status": status, "acc": float("nan"), "m_start": 0, "m_end": int(left.group(3)), "drift": float("nan")}
    start = re.search(r"start of count: m=(\d+)", out)
    end = re.search(r"acc=([0-9.]+) tail=([0-9.]+) m_end=(\d+)", out)
    if not start or not end:
        return {"status": "failed", "acc": float("nan"), "m_start": 0, "m_end": 0, "drift": float("nan")}
    m_start, m_end = int(start.group(1)), int(end.group(3))
    drift = abs(m_end - m_start) / max(m_start, 1) * (1e7 / n)
    return {"status": "ok", "acc": float(end.group(1)), "m_start": m_start, "m_end": m_end, "drift": drift}


def passes(r, target):
    return r["status"] == "ok" and abs(r["acc"] / target - 1) <= 0.10 and r["drift"] <= 0.20


def main(graph, burn, n, mode, trials_path, chosen_path):
    trials = open(trials_path, "a", newline="")
    chosen = open(chosen_path, "a", newline="")
    tw, cw = csv.writer(trials), csv.writer(chosen)

    def record(target, tt, a, i, r):
        tw.writerow([graph, target, tt, f"{a:.6f}", i, r["status"], r["acc"], r["m_start"], r["m_end"], r["drift"]])
        trials.flush()
        print(graph, target, tt, f"{a:.4f}", r, flush=True)

    found = {t: None for t in TARGETS}

    def consider(target, tt, a, r):
        if passes(r, target) and (found[target] is None or r["m_end"] > found[target][2]["m_end"]):
            found[target] = (tt, a, r)

    if mode == "ridge":
        # sx-superuser peak: the grid found no pair inside the drift limit, because the chain falls below a ridge
        # theta_t = 2.1 - 0.5 a and blows up above it. This walks the ridge itself in steps of 0.05.
        for k in range(25):
            a = -2 - 0.25 * k
            for j in range(-4, 5):
                tt = round(2.1 - 0.5 * a + 0.05 * j, 4)
                r = pilot(graph, a, tt, burn, n)
                record(0, tt, a, 0, r)
                for t in TARGETS:
                    consider(t, tt, a, r)
    elif mode in ("grid", "grid-low"):
        for a in [-2 - 0.5 * k for k in range(11)] if mode == "grid" else [-5 - 0.5 * k for k in range(8)]:
            for tt in GRID["grid"]:
                r = pilot(graph, a, tt, burn, n)
                record(0, tt, a, 0, r)
                for t in TARGETS:
                    consider(t, tt, a, r)
                if r["status"] == "dense":
                    break       # a larger theta_t leaves the band on the dense side as well
    elif mode.startswith("walk:"):
        # walk:target:a0:tt1;tt2;... for a target the coarse pass left open. A band exit moves to the next theta_t.
        _, tgt, a0, tts = mode.split(":")
        target, a = float(tgt), float(a0)
        for tt in [float(x) for x in tts.split(";")]:
            for i in range(4):
                r = pilot(graph, a, tt, burn, n)
                record(target, tt, a, i, r)
                consider(target, tt, a, r)
                if r["status"] != "ok" or abs(r["acc"] / target - 1) <= 0.05:
                    break
                a += math.log(target / r["acc"])
    else:
        for target in TARGETS:
            a = math.log(target) - 0.7
            for tt in GRID[mode]:
                for i in range(3):
                    r = pilot(graph, a, tt, burn, n)
                    record(target, tt, a, i, r)
                    consider(target, tt, a, r)
                    if r["status"] != "ok" or abs(r["acc"] / target - 1) <= 0.05:
                        break
                    a += math.log(target / r["acc"])
                if r["status"] != "ok":
                    break
    for t in TARGETS:
        if mode.startswith("walk:") and t != float(mode.split(":")[1]):
            continue
        if found[t]:
            tt, a, r = found[t]
            cw.writerow([graph, t, tt, f"{a:.6f}", r["acc"], r["m_start"], r["m_end"], r["drift"], "pass"])
        else:
            cw.writerow([graph, t, "", "", "", "", "", "", "no pair passed"])
    chosen.flush()


if __name__ == "__main__":
    main(sys.argv[1], int(float(sys.argv[2])), int(float(sys.argv[3])), sys.argv[4], sys.argv[5], sys.argv[6])
