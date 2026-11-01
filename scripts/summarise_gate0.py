"""Apply the Gate 0 decision of docs/experiments/gate0-protocol.md to one results directory."""
import collections
import csv
import pathlib
import sys

root = pathlib.Path(sys.argv[1])
rows = collections.defaultdict(dict)
for f in sorted(root.glob("*.csv")):
    for r in csv.DictReader(f.open()):
        rows[(r["stream"], r["kind"], int(r["cut"]))][r["policy"]] = r

streams = ["CollegeMsg", "email-Eu-core-temporal", "sx-mathoverflow", "sx-askubuntu", "sx-superuser", "wiki-talk-temporal"]
cuts = [("time", 3600), ("time", 86400), ("time", 604800), ("count", 100), ("count", 1000), ("count", 10000)]
label = {("time", 3600): "1h", ("time", 86400): "1d", ("time", 604800): "7d",
         ("count", 100): "100", ("count", 1000): "1k", ("count", 10000): "10k"}

print("Runs:", len(rows), "of", len(streams) * len(cuts))
print("\n## Score: grp_full operations / seq_net operations\n")
print("| Stream | " + " | ".join(label[c] for c in cuts) + " |")
print("|---|" + "---|" * len(cuts))
score = {}
for s in streams:
    cells = []
    for c in cuts:
        r = rows.get((s,) + c)
        if not r:
            cells.append("-")
            continue
        v = int(r["grp_full"]["ops"]) / int(r["seq_net"]["ops"])
        score[(s,) + c] = v
        cells.append(f"{v:.3f}")
    print(f"| {s} | " + " | ".join(cells) + " |")


def table(title, num, den):
    print(f"\n## {title}\n")
    print("| Stream | " + " | ".join(label[c] for c in cuts) + " |")
    print("|---|" + "---|" * len(cuts))
    for s in streams:
        cells = []
        for c in cuts:
            r = rows.get((s,) + c)
            cells.append("-" if not r else f"{num(r) / max(den(r), 1):.3f}")
        print(f"| {s} | " + " | ".join(cells) + " |")


kops = lambda p: (lambda r: int(r[p]["kernel_ops"]))
table("seq_net operations / seq_raw operations (what netting gives)", lambda r: int(r["seq_net"]["ops"]), lambda r: int(r["seq_raw"]["ops"]))
table("Kernel-only ratio: grp_full kernel / seq_net kernel (excludes all batch overhead, not a gate figure)", kops("grp_full"), kops("seq_net"))
table("grp_noorient / seq_net (order removed)", lambda r: int(r["grp_noorient"]["ops"]), lambda r: int(r["seq_net"]["ops"]))

print("\n## Streams\n")
print("| Stream | Events | Net updates | Peak m | Vertices | Kernel ops, seq_net at 1k | ops per net update |")
print("|---|---|---|---|---|---|---|")
for s in streams:
    r = rows.get((s, "count", 1000))
    if r:
        q = r["seq_net"]
        print(f"| {s} | {q['events']} | {q['net']} | {q['peak_m']} | {q['vertices']} | {q['kernel_ops']} | {int(q['kernel_ops']) / int(q['net']):.1f} |")

print("\n## Kernel O use, grp_full at 1k\n")
print("| Stream | groups A | groups O | whole-vertex O groups | spokes in O groups | re-ranks |")
print("|---|---|---|---|---|---|")
for s in streams:
    r = rows.get((s, "count", 1000))
    if r:
        q = r["grp_full"]
        print(f"| {s} | {q['groups_a']} | {q['groups_o']} | {q['star_whole']} | {q['star_spokes']} | {q['reranks']} |")

mis = sum(int(p["mismatches"]) for r in rows.values() for p in r.values())
checks = sum(int(next(iter(r.values()))["static_checks"]) for r in rows.values())
print(f"\nExactness: mismatches={mis}, static checks={checks}")

win = {s: sum(1 for c in cuts if score.get((s,) + c, 9) <= 0.67) for s in streams}
go_a = sum(1 for s in streams if win[s] >= 2)
go_b = all(v <= 1.05 for v in score.values())
stop = all(v > 0.80 for v in score.values())
verdict = "GO" if go_a >= 3 and go_b else "STOP" if stop else "PIVOT"
print(f"\nStreams with 2+ cuts at <=0.67: {go_a} (need 3). All cells <=1.05: {go_b}. All cells >0.80: {stop}.")
print(f"Cells: {len(score)}. Minimum {min(score.values()):.3f}. Maximum {max(score.values()):.3f}.")
print("Verdict by the written rule:", verdict)
