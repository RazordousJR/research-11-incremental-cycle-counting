# Proposal: peeling star groups for exact batch triangle maintenance

Status: hypothesis. The exactness results are tested. Nothing here has been measured on a real stream.
Task M1A02. Prior work is in `docs/literature/audit-2026-11-01.md`.

## 1. The narrowing

The idea as filed was "affected-neighbourhood indexing and a degree-aware intersection schedule". The audit
rejects the indexing half for triangles. Hanauer et al. (October 2026) measured plain scanning of the
lower-degree endpoint ahead of every auxiliary structure for triangles, on one core and single-edge updates.
The exactly-once proof is also not new, because three published forms exist. What stays open is how to share
work between the updates of one batch in a scanning algorithm. This proposal addresses that question only.

## 2. Problem and research question

**Problem.** Given a simple undirected graph and a sequence of update batches, maintain the exact global
triangle count at every batch boundary, on one CPU core, with no structure beyond the adjacency lists and one
total order on the vertices.

**Research question.** On real update streams, does grouping the updates of a batch around pivot vertices and
counting each group with the cheapest of two kernels reduce total adjacency operations and total CPU time,
including the upkeep of the vertex order, against the best tuned sequential scanning algorithm, while every
count stays equal to an exact reference?

**Win condition, written before any real stream is read.** See §7.

## 3. Mechanism

### 3.1 Graph model and batch semantics

- Graph: simple and undirected, with no self-loops, parallel edges or weights. `T(G)` is the number of
  unordered triangles.
- Input batch: a sequence `B = ((op_1, e_1), ..., (op_k, e_k))` with `op` in {insert, delete}. A self-loop
  is ignored. Inserting a present edge and deleting an absent edge are no-ops.
- The semantics of `B` is sequential application in order. The count is published only at the end of `B`.
- **Net batch.** For each pair in `B`, only its last operation matters. `D` holds pairs present in `G_old`
  whose last operation is delete. `I` holds pairs absent from `G_old` whose last operation is insert.
- `U = E_old \ D = E_new \ I` is the set of edges unchanged by the batch.

**Lemma 1.** `G_new = (U ∪ I)` equals the result of sequential application. For one pair, each operation sets
its presence to a fixed value, so the final presence is decided by the last operation alone. Pairs outside
`B` do not change.

### 3.2 Why every changed triangle is counted exactly once

For an edge set `X`, write `C(U, X)` for the number of triangles of `U ∪ X` with at least one edge in `X`.

**Lemma 2.** `T(G_new) = T(G_old) + C(U, I) - C(U, D)`.

*Proof.* Split the triangles of `G_new = U ∪ I` by whether they hold an edge of `I`. Those without one are
exactly the triangles of `U`. So `T(G_new) = T(U) + C(U, I)`. In the same way `T(G_old) = T(U) + C(U, D)`.
Subtract. ∎

A triangle with an edge in `I` and an edge in `D` lies in neither `U ∪ I` nor `U ∪ D`, so Lemma 2 never
touches it. That is the correct result, because such a triangle exists in neither graph.

**Peeling.** Assign each edge of `X` to one endpoint, its pivot. This splits `X` into groups `S_1, ..., S_q`,
where `S_i` is a star from pivot `x_i` to a set `C_i` of neighbours. Let `H_0 = U ∪ X` and
`H_i = H_{i-1} \ S_i`, so `H_q = U`. Let `L_i` be the number of triangles of `H_{i-1}` with at least one edge
in `S_i`.

**Lemma 3 (exactly once).** `C(U, X) = L_1 + ... + L_q`, and each triangle counted by `C(U, X)` is counted in
exactly one `L_i`.

*Proof.* Take a triangle `t` of `H_0` with an edge in `X`. Let `i` be the smallest index such that `S_i`
contains an edge of `t`. Such an `i` exists because `t` has an edge in `X`. For every `j < i`, `S_j` removes no
edge of `t`, so `t` is in `H_{j}` for all `j < i` and it is counted in `L_i`. For `j > i`, `t` has lost an edge
and is not in `H_{j-1}`, so it is not counted in `L_j`. For `j < i`, `t` has no edge in `S_j`, so it is not
counted in `L_j`. Conversely, everything counted in any `L_i` is a triangle of `H_0` with an edge in `X`. ∎

**Worked case.** Let triangle `{a, b, c}` have all three edges in `I`, with groups `S_1 = {ab, ac}` (pivot `a`)
and `S_2 = {bc}` (pivot `b`). `S_1` is peeled first. `{a, b, c}` lies in `H_0` and is counted in `L_1`. It is
missing from `H_1`, so `L_2` does not count it. Per-edge inclusion-exclusion would count it three times and
correct afterwards. Peeling never produces the duplicate.

In practice, deletions run first, starting from `G_old`: count a group, then remove it. Insertions then run
in the opposite direction, starting from `U`: add a group, then count it. Lemma 3 holds for insertions because
the sequence of graphs is the same sequence read backwards. The graph update and the count are the same pass.

### 3.3 The star kernels

For a group with pivot `x`, set `C`, working graph `H`, `N = N_H(x)`, `R = N \ C`, `k = |C|`, `d = |N|`:

    L(x, C) = number of unordered pairs {a, b} in N, adjacent in H, with a in C or b in C

**Lemma 4.** `L(x, C)` is the number of triangles of `H` with at least one spoke in `C`. A triangle holding a
spoke `xc` has the form `{x, c, w}` with `w` in `N` and `cw` an edge. The map to the pair `{c, w}` is a
bijection. ∎

**Kernel A (per edge).** This is sequential scanning, restricted to the group: for each `v` in `C`, count
common neighbours of `x` and `v` by scanning the smaller list and probing the other, then remove or add that
one spoke. Cost: `2 · sum min(deg x, deg v)` with degrees at that moment.

**Kernel O (oriented).** Fix a total order on vertices and let `out(y)` be the neighbours after `y`. This
needs no property of the order for exactness. Each adjacent pair in `N` has one earlier endpoint, so it is
visited once.

    stamp N and C
    l = 0
    for y in N:
        if y in C or |out(y)| <= k:
            for z in out(y):
                if z in N and (y in C or z in C): l += 1
        else:
            for v in C:
                if v != y and adjacent(y, v) and y before v: l += 1

Cost at most `d + k + 2 · sum_{y in C} |out(y)| + 2 · sum_{y in R} min(|out(y)|, k)`.

The prototype also holds **kernel R (reverse scan)**, which uses no order. It is kept for ablation A3 only and
the adaptive policy no longer offers it. In probe P0, with the batch-degree pivot, the selector never chose
it, which agrees with Lemma 5.

### 3.4 Batch algorithm

    PROCESS(G, B):
        (I, D) = NET(G, B)
        delta = 0
        for group (x, C) in GROUPS(D):
            delta -= STEP(G, x, C, delete)
        for group (x, C) in GROUPS(I):
            delta += STEP(G, x, C, insert)
        return delta

    GROUPS(X):
        bd[v] = number of edges of X at v
        pivot of edge uv = the endpoint with larger bd, ties to the smaller id
        return groups in order of first appearance

    STEP(G, x, C, op):
        k = |C|; ca = exact cost of kernel A, computed in O(k)
        if k == 1: run kernel A
        floor = d + k + 2 · sum_{y in C} |out(y)| + (d - k) · min(m / n, k)    # computed in O(k)
        if ca <= floor: run kernel A
        co = cost of kernel O, computed in O(d)
        if co < ca: run kernel O, applying the star before counting on insert and after on delete
        else: run kernel A

### 3.5 Correctness assumptions

1. Graph and batch semantics as in §3.1.
2. `adjacent` and the membership stamps answer exactly. There is no Bloom filter or sampling in the count path.
3. The counter does not overflow: 64-bit signed.
4. Sequential execution. Concurrent peeling would break the order that Lemma 3 relies on.
5. The vertex order is any total order. A stale order costs time and cannot cost correctness.

Tested by `tests/test_exact.cpp`, see §8.

### 3.6 Complexity

Let `Δ` be the net batch and `W_A` the cost of kernel A alone on the same groups in the same order.

**Theorem 1 (exactness).** Lemmas 1 to 4 give an exact count at every batch boundary.

**Theorem 2 (bounded overhead).** The total cost is at most `2 · W_A + 2|Δ|` operations. *Proof sketch.* A
singleton group pays `ca` only. Otherwise the gate pays at most `2k`. The gate passes only when `ca > floor >= d`,
so the `d` reads of the estimate are at most `ca`. The kernel then run costs at most `ca`, because the selector
runs kernel O only when its exact cost bound is below `ca`. Sum over groups. ∎ The last floor term is a
heuristic estimate of the `R` cost. It can only send more groups to kernel A, so it cannot break this bound.
This compares with kernel A in group order. The comparison with stream order is empirical (`grp_edge`
against `seq_stream` in §8).

**Theorem 3 (separation).** There is a family of batches on which every sequential order of single-edge
lower-degree scanning costs `Θ(k²)` and the mechanism costs `O(k)`, with `k = √m`. *Construction.* Take `k`
vertices `y_i`, each with `k` private leaves, ordered after their leaves. A new vertex `x`, ordered first, gains
an edge to every `y_i`. In any order the `i`-th update scans `min(i, k + 1) = i` elements, so the sum is
`Θ(k²)`. Kernel O reads `N`, `C` and the `out` lists of the `y_i`, which are empty. ∎

For a group with `R` empty, which is a whole-vertex arrival or departure, kernel O costs at most
`3k + 2k · max|out(y)|`, so a degeneracy order gives `O(kλ)`, where `λ` is the degeneracy.

**Lemma 5 (the null case).** If `C` is a uniform random `k`-subset of `N`, then in expectation kernel A is no
worse than kernel R. For a neighbour of degree `D`, the function `min(D, t) / t` does not increase with `t`, so
`k · min(D, d) <= d · min(D, k)` for `k <= d`. So uniform random updates give no sharing. Any gain must come
from structure in the batch: skew towards high-degree members, or a large share of `N` changing at once.

**Not improved.** The worst case stays. Toggling one edge between two hubs costs `Θ(min(deg u, deg v))`, which
can be `Θ(m)`. Heavy-light methods reach `O(√m)` there and this mechanism does not. Space is `O(n + m)`.
Upkeep of the order is `O(1)` expected per edge, plus `O(m + n log n)` for each re-ranking.

## 4. Baselines, datasets and metrics

**Baselines, strongest first.**

| ID | Baseline | Source |
|---|---|---|
| S1 | Sequential scanning, lower degree first, no auxiliary structure. The configuration recommended for triangles by Hanauer et al. | `DynamicSubgraphCounter`, and an in-house copy on the same adjacency structure for a fair container comparison |
| S2 | HHH*-H with tuned ε, EGST*-HD | `DynamicSubgraphCounter` |
| S3 | Inclusion-exclusion batches on sorted adjacency (Makkar et al.), one thread | In-house CPU version, labelled as such, because no dynamic code was found in Hornet |
| S4 | Heavy-light batches (Dhulipala et al.), one thread | GBBS `dynamiclocalalgs` branch if it holds the code, or an in-house version, labelled |
| S5 | Full recomputation by compact-forward on a fresh degree order | In-house, cross-checked with the GBBS benchmark in BYO |

BYO supplies container comparison for S5 and for an optional variant on other containers. It has no dynamic
triangle code.

**Datasets.** Start from the instances of Hanauer et al. so that results compare directly.

| Group | Inputs | Label | Note |
|---|---|---|---|
| Timestamped interactions | SNAP CollegeMsg, email-Eu-core-temporal, sx-mathoverflow, sx-askubuntu, sx-superuser, wiki-talk-temporal, sx-stackoverflow | Real timestamps, synthetic expiry | Insert at the timestamp, remove after two weeks, as Hanauer et al. do |
| Wikipedia hyperlinks | KONECT Wikipedia link streams, small languages first | Real insertions and deletions | Burst structure expected from page creation |
| Natural hyperedges | Co-authorship or email with recipient lists | Real, if a clean source exists | Clique and star bursts occur by construction |
| Synthetic | Kronecker sets from Hanauer et al.; Holme-Kim base with the five streams of P0 | Fully synthetic | Hard cases and null cases only |

Batches are cut two ways: by time window, which keeps real bursts, and by fixed count. Sizes run from 1 to 10^6
updates. Preprocessing removes self-loops, parallel edges, directions and deletions of absent edges.

**Metrics.** Scan, probe and mark operations. Total stream time including graph update. Latency per batch at
p50, p99 and maximum. Peak RSS. Order upkeep cost. Payback point, in updates, at which upkeep has been repaid.
Share of operations by kernel. All are reported per stream and as geometric means, with no single aggregate.

**Fairness.** One compiler and one flag set. The same hash container across in-house baselines. Every baseline
gets the same tuning effort, written in `docs/experiments/baseline-tuning.md`. Single core, pinned.

## 5. Ablations, failure cases, leakage and noise

| Ablation | Switches off | Answers |
|---|---|---|
| A1 | Grouping, so `seq_stream` | Whether sharing does anything |
| A2 | Kernel O, keeping groups (`grp_edge`) | Whether group order alone helps |
| A3 | Vertex order, so kernels A and R only (`grp_noorient`) | Whether the order pays for itself. This is the idea file's pivot condition |
| A4 | Estimator, always kernel O | What the selector is worth |
| A5 | Batch-degree pivot, replaced by lower-degree pivot | Whether the pivot rule matters |
| A6 | Re-ranking, order frozen at the first epoch | How fast the order goes stale |
| A7 | Netting, raw batch processed | What cancellation is worth on real streams |

**Failure cases, reported whichever way they go.** Hub-to-hub churn. Uniform random updates, where Lemma 5
predicts no gain. Graphs with very few triangles. Very small batches, where every group is a singleton. A
stream that creates new hubs after the order is fixed. Batches that are mostly no-ops. Graphs with a large
degeneracy, where `out` lists are long.

**Leakage.**
- The vertex order is computed only from the graph at the epoch start. Re-ranking never reads ahead.
- The selector sees only the current group.
- Thresholds are set on an engineering subset of streams and frozen before the evaluation streams run.
- Hanauer et al. publish a good ε for each pattern. Those values are starting points, and each baseline is
  re-tuned on the same engineering subset.

**Noise.**
- Gate 0 uses operation counts, which are deterministic for a fixed seed and stream.
- Timed runs use a pinned core, five repeats, median and IQR, and an exclusive claim on the machine.
- Peak RSS comes from a separate run, because measuring it perturbs timing.
- Exactness is checked by the O(n³) reference on small batches, and by a static count at intervals on large
  streams.

## 6. Prototype, budget and screening plan

**Prototype.** `src/r11/tc.hpp` holds the engine, kernels, net batch and references, about 350 lines of C++20
with no dependencies. `tests/test_exact.cpp` is the oracle. `benchmarks/probe_ops.cpp` is the operation probe.
It is written for counting operations. Its `std::unordered_map` adjacency must be replaced before any timing.

**Resources.** Machine as read on 2026-11-01: Intel Core Ultra 7 265, 31 GB RAM, 206 GB free on C:, GCC 16.2 in
`C:/Develop/tools/winlibs`, WSL2 Ubuntu 24.04. No GPU, cloud or paid API is needed. Note that the idea documents
name a Ryzen 5 7500F, which is not this machine. Data: about 620 MB compressed for the seven SNAP streams,
of which 530 MB is sx-stackoverflow. Runtime: minutes per stream for operation counts, hours for a full timed
sweep.

**Plan, by task rather than by date, sized at 2 to 4 weeks of part-time work.**

| Week | Tasks | Ends in |
|---|---|---|
| 1 | M1A05 finish the novelty reading. M1A06 stream loaders. M1A07 Gate 0 on six real streams, operation counts only | Gate 0 decision |
| 2 | M1A08 flat adjacency. M1A09 reproduce S1 on two instances from Hanauer et al. M1A10 Gate 1 timing | Gate 1 decision |
| 3 | M1A11 re-ranking policy. M1A12 ablations A1 to A7. M1A13 S2 and S3 | Component evidence |
| 4 | M1A14 fill the coverage matrix to about 20 streams. M1A15 hard cases. M1A16 go, pivot or stop | Decision |

## 7. Go, pivot and stop

The strongest simple baseline is S1, tuned. Thresholds below are fixed before any real stream is read.

| Gate | Evidence | Go | Pivot | Stop |
|---|---|---|---|---|
| Novelty (M1A05) | Full text of the TODS paper of Mhedhbi et al., EDTC, ParaDyMS | None of them selects a counting kernel per group of a batch for triangles | One does it in another setting: reposition against it and continue, CPU only | One does this exact thing for triangles and evaluates it |
| Gate 0, operations | Six real streams, batch sizes by time window and by count | Adaptive at most 0.67 of S1's operations on at least 3 streams, at 2 or more batch sizes, and at most 1.05 on every stream | Gain appears only on whole-vertex bursts: narrow to a short paper on vertex-level updates | Adaptive above 0.80 on every stream |
| Gate 1, time | Pinned runs on the same streams | Median speedup of at least 1.3 against tuned S1, no stream worse than 1.1 | Gain in operations but not in time: check whether stamps lose to hash probes in memory, redesign once | Second design also fails |
| Gate 2, payback | A3 and A6 | Order upkeep repaid within the first 10% of the stream on at least two thirds of winning streams | Repaid only with a frozen order: drop re-ranking from the method | Never repaid. This is the idea file's stated pivot condition, and kernel O goes with it |

**The stop case is a real possibility and is not a failure of process.** If Gate 0 stops the project, the
result is a refutation task. Whether a short negative note is worth writing goes to the PI under §3.

**Venue, unconfirmed.** Candidate: Journal of Graph Algorithms and Applications. It is free to authors, its
scope page lists dynamic graph algorithms and experimentation, and it published Eppstein and Spiro. The PI's
list gives it as Q2, which is not independently checked. No exemplar from the last three years has been
verified, so bootstrap §3 is not met yet. The method contribution is small enough that a short format may fit
better (bootstrap §19).

## 8. Evidence so far

All of this is non-citable. It tests the prototype, not the claim.

**Exactness (`tests/test_exact.cpp`, 200,000 random batches, seed 7).** Graphs of 4 to 24 vertices, arbitrary
vertex orders, batches with repeats, toggles, no-ops and stars. Each batch ran every combination of 3 pivot
rules and 5 kernel modes against the O(n³) reference. Result: 0 exactness failures. The batches held 610,614
triangles with two same-type changed edges, 62,426 with three, and 448,103 mixed insert and delete triangles.
The naive control was wrong on 12,950 of 26,929 insert-only batches, as it must be. The cost estimate was never
exceeded, and the bound of Theorem 2 was never exceeded.

A repeat after the gate change (seed 11) gave the same outcome: 0 failures on 200,000 batches.

**Probes P0 and P0B.** Synthetic only, see `results/probes/P0-20261101/README.md`. With the final gate, the
operation ratio to sequential scanning was 1.000 to 1.029 on uniform updates, 1.000 to 1.015 on degree-biased
inserts, 0.457 to 0.817 on vertex arrivals, 0.590 to 0.765 on vertex departures, and 0.979 to 1.050 on clique
insertion. The orientation is what produced the gain: with kernel O removed the ratio was above 1 on every
burst stream. The pattern agrees with Lemma 5. These streams were written by us and favour bursts, so they
say nothing about real data.
