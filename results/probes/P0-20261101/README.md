# Probes P0 and P0B

**Non-citable.** Synthetic graph, synthetic streams, operation counts only, no wall-clock time. These probes
test the prototype and sharpen Gate 0. They support no claim about real streams. The arrival, departure and
clique streams were written by us with heavy-tailed sizes, so they favour the mechanism by construction.

| Field | P0 | P0B |
|---|---|---|
| Directory | `results/probes/P0-20261101/` | `results/probes/P0B-20261101/` |
| `src/r11/tc.hpp` SHA-256 prefix | `e99fcfb05ae3` | `3dc3a6560635` |
| `benchmarks/probe_ops.cpp` prefix | `a90c94b566c0` | `a90c94b566c0` |
| Change | First gate | Floor raised by an estimate of the R term; adaptive policy no longer offers kernel R |

Compiler: GCC 16.2, `-std=c++20 -O2 -static`. Graph: Holme-Kim with n = 200,000, 8 edges per vertex, triad
probability 0.5, m = 1,599,964, seeds 1 to 3. Command: `probe_ops 200000 8 <seed> 5 <csv>`, then
`python scripts/summarise_p0.py <dir>`. Five batches per cell. Batch size is a fraction of m.

Every cell is summed operations divided by those of sequential scanning in stream order, median across seeds.
Below 1 means fewer operations. `static` is the same ratio for fresh full recomputation. All five policies
matched each other and a static count on every batch of both probes: 0 mismatches.

Results: `summary.md` in each directory. P0B, policy `grp_full`, range across the five batch sizes:

| Stream | Ratio to sequential scanning | Reading |
|---|---|---|
| uniform | 1.000 to 1.029 | No gain, as Lemma 5 predicts. Overhead of the gate only |
| biased inserts | 1.000 to 1.015 | No gain |
| vertex arrival | 0.457 to 0.817 | Gain, from whole-vertex stars |
| vertex departure | 0.590 to 0.765 | Gain |
| clique insertion | 0.979 to 1.050 | No useful gain |

In P0 the gate overhead reached 1.207 on biased inserts. That is why P0B exists. P0 is kept as evidence.

What this does not establish: any result on a real stream, any time saving, anything about the order going
stale, since the order here is fixed once and the streams are short.
