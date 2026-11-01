# Scout S4: change the query model and the knowledge of the future

Date: 2026-11-01. Direction: interval and time-travel queries, many windows at once, delta-temporal triangles,
known expiry. No kill test below has been run. Every number from a stream is quoted from Gate 0 or from a
paper. Everything labelled **Conjecture** was derived for this note and has had no second reader.

## Verdict by direction

| Direction | Verdict | Deciding source |
|---|---|---|
| (a) Index for exact count in any window [t1, t2] | **Taken.** Simple graphs since 2019. Multigraphs, any δ and distinct triples since 2025 | Chanchary and Maheshwari, JGAA 2019, Thm 10; Xia, Fang, Luo, PACMMOD 2025, Sections 4 and 5 |
| (b) One pass, exact, many window lengths | **No exact work found.** The approximate version exists and already has the key idea | Jha, Seshadhri, Pinar, arXiv:1310.7665 |
| (c) Exact δ-temporal triangles | **Taken.** Query time carries δ, so "all δ" is also closed | Xia et al. 2025; Paranjape et al.; Pashanasangi and Seshadhri |
| (d) Known expiry against the OMv barrier | **Cannot beat it.** Insert-only is already hard. Only knowing future *insertions* helps | Bernstein et al., arXiv:2510.20341, Thm 1.4; Peng and Rubinstein |

### Direct answer to the theory question

Known expiry does not beat the barrier. Insertion-only is the special case with every expiry at infinity, and
also the sliding window with a window longer than the stream. Bernstein, Bhattacharya, Fischer, Kiss and
Saranurak prove under OMv that reporting YES once a triangle exists in an incremental graph needs n^(3-o(1))
total update time. Over at most n^2 insertions that is n^(1-o(1)) amortized per update, which is m^(1/2-o(1)) on
dense graphs. Kara et al. already give O(√m), so nothing is left to gain in the exponent. This part is a corollary
of a published theorem, not a new result.

The clean positive result is one level up. If the whole update sequence is known, including insertions, then
Conjecture O1 in Candidate 2 gives per-update time below m^(1/2). Its proposed bound is
Õ(min(m^((ω-1)/(ω+1)), n^((ω-1)/2))) per update, about m^0.407 on general graphs and m^0.343 on dense graphs,
against a lower bound of m^(1/3-o(1)) under 3SUM.

## What the Gate 0 cause means here

Gate 0 lost because one scan costs only 5.9 to 13.9 operations, so sharing work between updates in a batch
has almost nothing to save and any per-update overhead loses. A valid candidate must therefore do one of three
things: remove whole scans, avoid scanning at query time, or be a result about something other than cost per
update. Candidates below say which.

Two facts from this repo shape the candidates.

- The diagnostic reports at most 64.4 operations per update on insert-only streams. So one forward scan over
  the whole history is cheap. Index build by a single sweep is cheap on these streams.
- Gate 0 uses Hanauer's convention: edges are "inserted at their timestamp and removed two weeks later", under
  set semantics, so "an edge inserted twice in a window is removed at the first expiry"
  (`docs/experiments/gate0-protocol.md`, rule 4). That is not a true sliding window: a repeat does not extend the
  edge's life. A kill test for window semantics needs its own baseline on last-seen semantics, not Gate 0's numbers.

## Core observation used by Candidates 1 and 4

This was derived here. The second half overlaps Xia et al.; see Candidate 4.

For a vertex triple T, let σ_b(T) be the minimum, over its three pairs, of the last event time at or before b.
T is a triangle of the simple graph of events in [a, b] exactly when a ≤ σ_b(T).

So for one fixed right end b, the whole function a -> count is a one-dimensional step function, held in an
array B over left endpoints. When an event (u, v, t) arrives, each common neighbour w with
m = min(last(u,w), last(v,w)) moves one unit in B from p to m, where p is the previous last(u,v). If the pair is
new, the unit is added at m. Nothing moves when p ≥ m. Expiry needs no scan at all, because a triple falls out of
a window when the window's left boundary passes its slot in B.

Making that sweep persistent gives the two-dimensional version. A triple's minimal covering windows
[s_1,e_1], ..., [s_r,e_r], increasing in both ends, yield +1 at each (s_i, e_i) and -1 at each (s_i, e_(i+1)). A
query window containing runs i..j sums to (j-i+1) - (j-i) = 1, so each triple counts once.

## Candidate 1 (top): every window length at once, with no expiry scan

**Claim.** Exact triangle counts of the last L time units, for every L up to L_max at once, can be maintained
with one scan per event, no scan at expiry, and memory linear in the window, with no per-triangle storage.

**Type.** Method, with an empirical study. The known-expiry corollary above serves as its lower-bound section.

**Closest five.**

| Paper | Read | Overlap |
|---|---|---|
| Jha, Seshadhri, Pinar. [Counting Triangles in Real-World Graph Streams](https://arxiv.org/abs/1310.7665) | Abstract, introduction | **Closest in idea.** One pass, multigraph, "does not require committing to any window size(s) a priori". Wedge sampling, so an estimate only |
| Xia, Fang, Luo. [Efficiently Counting Triangles in Large Temporal Graphs](https://doi.org/10.1145/3709688), PACMMOD 2025; [authors' repository](https://github.com/xqbf/counting-triangles) | LaTeX source in the repository: Sections 2, 5, 7, 8 in full; Section 4 for definitions, bounds and maintenance; Section 6 setup. Publisher page returned 403 | **High.** WT-Index supports append, so t_e = now is a query it can answer. But it stores all C-points (651M on wiki-talk, 2.7B on stackoverflow), keeps all history, and its Lemma on insertion costs O(π log m). The binary KDT-Index is 3-dimensional |
| Gou et al. [Sliding window-based approximate triangle counting with bounded memory usage](https://mod.icst.pku.edu.cn/docs/20230529102821794405.pdf), VLDB Journal 2023; [code](https://github.com/StreamingTriangleCounting/TriangleCounting) | Text search of the PDF, repository README | **Medium.** Sliding window with duplicates, approximate, one window. Its GoldenCounter is an exact reference and the README calls it time-consuming |
| Zhang, Bonifati, Özsu. [Incremental Sliding Window Connectivity over Streaming Graphs](https://arxiv.org/abs/2406.06754), 2024 | Abstract, text search | **Medium, same motive.** Avoids physical edge deletion in sliding windows, for connectivity. Fixed window size and slide. Different mechanism (bidirectional incremental computation) |
| Hanauer et al. [Fully Dynamic Triangle and 4-Vertex Subgraph Counting](https://arxiv.org/abs/2610.29776), 2026 | Per audit; text search here for expiry, window, lifetime | **Baseline.** Removal is an ordinary deletion in its benchmark. No use of known removal times was found by that search |

**Gap.** I found no exact algorithm that keeps counts for many or all window lengths in one pass, and none that
drops the expiry scan. Search terms are in the log below. This is weak evidence: "exact sliding window" is often
a footnote or a ground-truth tool, as with GoldenCounter, so an unpublished or buried version is plausible.

**Mechanism.**
- State: the graph of pairs seen in the last L_max, with last(x, y) per pair; array B over time slots, as a ring.
- Event (u, v, t): scan N(x) for the lower-degree endpoint x, probe the other endpoint, and move units in B.
- Pruning on repeats: keep each adjacency list in recency order, so a repeat of (u, v) only scans neighbours newer
  than p. Without this a repeat costs a full scan that the baseline does not pay.
- Expiry: drop the pair from adjacency in O(1). No common-neighbour scan.
- Query: Fenwick tree over B gives any L in O(log L_max). For k fixed lengths, k running sums and k moving
  boundaries give O(1) reads and O(k) per moved unit.
- Index size: O(edges in L_max window + number of slots) words. Nothing is stored per triangle.
- Variant: any decay function f, as a sum of B[s] f(now - s), follows from the same array.

Exactness: each triple with all three pairs inside the horizon holds exactly one unit in B, at its σ slot.
Only an event on one of its pairs can change σ, and that event's scan finds it. This is a short proof and is not
the contribution, as with the exactly-once proof in the audit.

**Why it escapes Gate 0.** It does not try to beat a scan by sharing inside a batch. It deletes whole scans: all
expiry scans, and for k windows it replaces about 2k scans per edge with one. New overhead is per moved unit in
B, not per update.

**Honest risk.** Overhead per moved unit may be large: 2 log2(L) operations with a Fenwick tree against a scan of
6 to 14 operations. On streams with many triangles per event, use the k-sums form. Also, a single window may gain
little on repeat-heavy streams, since the baseline scans only on first arrival and last expiry while this method
scans per event, and the pruning has to recover that. The gain is a constant factor, so the paper rests on the new
capability (any L, exact, small memory), not on speed alone. A reviewer may call the single-window case folklore.

**Strongest simple baseline.** Two of them. (B1) k independent lower-degree scan counters on last-seen semantics.
(B2) One graph at L_max, one shared insertion scan with timestamp filters, and one expiry scan per window.
B2 is the fair comparison. Also compare memory and query time against Xia's WT-Index with t_e = now.

**Cheapest kill test, under one hour.** Add a standalone driver that uses `r11::Graph` and `FlatMap` from
`src/r11/tc.hpp` with a last-time value per pair, on all six streams in `data/snap/`, with
L in {1h, 1d, 7d, 14d, 30d, 90d}. Count operations with the same unit-cost `Ops`. Run with and without recency
pruning. Check exactness against `static_count` at 1,000 checkpoints per L.
Kill if the operations ratio against B2 at k = 4 exceeds 0.67 on four or more of six streams, or if a single window
exceeds 1.0 on all six. Use the unit-cost model so the result is comparable to Gate 0. Report moved units per event,
since that decides whether Fenwick or k-sums is the honest form.

**Venue.** SEA or ALENEX as an algorithm-engineering paper. PVLDB or EDBT only if extended to multi-query stream
processing.

**Scores.** Novelty 6/10. Feasibility solo 9/10.

## Candidate 2: what knowledge of the future actually helps

**Claim.** For exact triangle counting with edge updates, deletion look-ahead gives nothing in the exponent, while
the full update sequence gives a per-update rate equal to static all-edges counting.

**Type.** Theory.

**Closest five.**

| Paper | Read | Overlap |
|---|---|---|
| Bernstein et al. [Separations between Oblivious and Adaptive Adversaries](https://arxiv.org/abs/2510.20341), 2025 | Theorem 1.4, surrounding text, last lines of its proof | Gives the insert-only barrier. No offline result for counting |
| Henzinger, Saha, Seybold, Ye. [On the Complexity of Algorithms with Predictions for Dynamic Graph Problems](https://arxiv.org/abs/2307.16771), ITCS 2024 | Abstract, Table 1, Sections 5.2, 6.2, Theorem 8.5 statement | **Highest.** Studies #s-△, the count through a fixed vertex, with delayed predictions. Upper bounds allow polynomial preprocessing, so they do not bound total time. Offline-online gap stated for OMv (n^3 to n^ω), not for global count with edge updates |
| van den Brand, Forster, Nazari, Polak. [On Dynamic Graph Algorithms with Predictions](https://arxiv.org/abs/2307.09961), SODA 2024 | Introduction, Theorem 1.4, Section 5.1 | Triangle detection with predicted **vertex** updates in O(n^(ω-1) + n·η_i). Edge updates are not treated this way, and n^(ω-1) would lose to the trivial O(n) per edge update |
| Liu, Srinivas. [The Predicted-Updates Dynamic Model](https://arxiv.org/abs/2307.08890), 2023 | Text search, Appendix A.5, discussion of [PR23] | Transformations from incremental to fully dynamic under predicted deletions. Inherits the incremental barrier here |
| Peng, Rubinstein. [Fully-dynamic-to-incremental reductions with known deletion order](https://arxiv.org/abs/2211.05178), SOSA 2023 | Abstract | With known deletion order, amortized fully dynamic reduces to worst-case incremental with a log T factor. So known expiry sits at the incremental level |

Also relevant: [Dhulipala et al.](https://arxiv.org/abs/2003.13585) (Section 3.3 read). Its bound at k = 3, by my
substitution, is O(min(Δ m^(ω/(1+ω)), (m+Δ)^(2ω/(1+ω)))) work for a batch of Δ, which gives the count at batch ends,
not after each update. At Δ about m it matches the static rate. [van den Brand, Nanongkai, Saranurak](https://arxiv.org/abs/1905.05067)
(look-ahead section, Figure 2, Appendix C remark) treats matrix inverse with look-ahead and notes the edge-update
triangle bound is n^(1-ε) with a trivial match. [Chan, Xu](https://arxiv.org/abs/2310.11575) (Section 1) states the
All-Edges Sparse Triangle bounds used below. [Pant, Williams](https://arxiv.org/abs/2606.27727) (abstract, Section 1)
shows that bound's trivial algorithm can be slightly beaten, so "tight" here means up to subpolynomial factors.

**Gap.** I found no statement of a total-time offline bound for the global count with per-update output under edge
updates. One web search and the arXiv screen on "offline dynamic", "look-ahead" and "semi-online" found nothing. The
technique (phases, heavy-light, one matrix product per phase) is standard, so the most likely failure is that this is
folklore or sits in a paper on a different primitive.

**Conjecture O1 (offline upper bound).** Given all T ≥ m updates in advance, all c_i can be computed in time
T · Õ(θ), with θ = min(m^((ω-1)/(ω+1)), n^((ω-1)/2)).

Sketch. Work in epochs of m updates. H = vertices of degree at least θ at epoch start, or touched at least θ times in
the epoch, so |H| = h ≤ min(n, O(m/θ)). A pair with an endpoint outside H is answered by scanning O(θ). For u, v in H:
- Common neighbours outside H: table P over H × H, updated in O(θ) per toggle. Initialise in O(mθ + h^2) per epoch.
- Common neighbours inside H: split the epoch into phases of g = θh updates. Compute A_0^2 on H in h^ω per phase. For
  vertices touched at least θ times in a phase, snapshot their change vector every θ touches, giving at most g/θ = h
  rows R. Compute R·A_0 and R·R^T in h^ω. A query reads tables and fixes tails of at most θ entries, in O(θ).

Per update: h^(ω-1)/θ + θ, which balances at the stated θ.

Open steps: (1) a full write-up, including memory, which looks like Õ(h^2 + m); (2) confirm the bound
is not implied by a known result; (3) check the balance at intermediate m, where the two terms of the min cross;
(4) test the statement on a small instance against a brute-force counter.

**Claim LB (lower bound, short reduction, unchecked).** Insert all m edges, then delete and reinsert each edge once.
The change in count at each reinsertion is that edge's triangle count. So an offline algorithm with T = 3m solves
All-Edges Sparse Triangle, which needs m^(4/3-o(1)) time under 3SUM per Chan and Xu's summary of Pătraşcu. That gives
m^(1/3-o(1)) per update. If ω = 2, O1 and this match on general graphs.

**Conjecture O2 (partial look-ahead).** With exact knowledge of only the next ℓ updates on dense graphs, the same
phases give Õ(n^ω/ℓ + ℓ/n) per update, so the bound beats n only when ℓ > n^(ω-1), about m^0.69 known future
insertions. Open: whether rectangular multiplication lowers this threshold. It does on star-shaped OuMv rounds, but
the sketch does not cover updates spread over many vertices, where the phase reads about n·g entries.

Open question worth stating in any paper: dense graphs have a gap, upper n^((3+ω)/2) total against lower n^ω. From
memory, not from a source opened here, the upper exponent equals the dominance-product exponent. Any relation is
unestablished.

| Knowledge of the future | Per-update exact count | Status |
|---|---|---|
| None | m^(1/2±o(1)) | [Kara et al.](https://arxiv.org/abs/2004.03716) (abstract), [HKNS](https://arxiv.org/abs/1511.06773) (Lemma 3.3, Corollary 3.4) |
| Expiry or deletion order known, or insert-only | m^(1/2-o(1)) lower bound | Corollary of Bernstein et al. Thm 1.4 |
| Next ℓ updates known | Õ(n^ω/ℓ + ℓ/n), dense | Conjecture O2 |
| All updates known | Õ(θ) as above; m^(1/3-o(1)) lower bound | Conjecture O1 and Claim LB |

**Index size and query.** Not an index. Output is the array c_1..c_T. After that, any point in time is a binary search.

**Why it escapes Gate 0.** It makes no claim about practical cost per update. It is a statement of exponents on
worst-case inputs, where scans are not 6 to 64 operations. It has no practical speed-up on these streams, and that
must be said plainly.

**Strongest simple baseline.** Trivial O(n) per update, or O(√m) by Kara et al. Against O1 with practical
matrix multiplication the constant is hopeless, so no one would run it.

**Cheapest kill test, under one hour.** Not a stream test. First, 30 more minutes of search on "offline" with
"all-edges", in Dhulipala et al. and in Sankowski-Mucha style work. Second, code O1 with naive cubic multiplication and
check c_i against `r11::Graph` scanning on 1,000-vertex random dense update sequences, with a phase length sweep. It
would kill the sketch only if the counts disagree or if a hidden term appears. It cannot settle novelty.

**Venue.** SOSA or ESA Track A, or IPL if it stays short.

**Scores.** Novelty 6/10, with a real folklore risk. Feasibility solo 7/10.

## Candidate 3: windowed transitivity, where the denominator is the hard part

**Claim.** Under the strong set disjointness conjecture, exact wedge counting in a time window needs S·Q^2 = Ω̃(m^2),
which a simple grid matches, so windowed transitivity costs more for its denominator than its numerator on
triangle-sparse inputs.

**Type.** Theory plus method.

**Closest five.**

| Paper | Read | Overlap |
|---|---|---|
| Deng, Lu, Tao. [Space-Query Tradeoffs in Range Subgraph Counting and Listing](https://arxiv.org/abs/2301.03390), ICDT 2023 | Introduction, Table 1, theorem statements | **Highest in form.** Same result shape for ranges over **vertex** attributes: wedges Õ(m^2/λ^2) with matching conditional bound, cliques O(m). Clique compression does not carry to edge time, where the two extreme edges of a triangle are not a single edge |
| Chanchary, Maheshwari. [Time Windowed Data Structures for Graphs](https://jgaa.info/index.php/jgaa/article/view/paper489), JGAA 2019 | Introduction, Table 1, Sections 5.2, 5.3 | **High.** 2-paths in O(n) query with linear space, or K points. Triangles as K dominance points. Each event has a unique timestamp. The WALCOM 2016 version was not read; its Springer page redirected to a login |
| Mang et al. [Efficient Historical Butterfly Counting](https://www.vldb.org/pvldb/vol18/p1607-mang.pdf), PVLDB 2025 | Abstract, introduction, Section 4.1, index sections, Section 5 statements | **High in technique.** Edge-timestamp set disjointness reduction by 2D differencing for butterflies, then a structure-aware index for power laws. Not wedges or triangles |
| Xia et al. 2025 | As above | Supplies the numerator. No wedge or transitivity query was seen in the sections read, though its experiments list a case study I did not read |
| Bannister, DuBois, Eppstein, Smyth. [Windows into Relational Events](https://arxiv.org/abs/1209.5791), SODA 2013 | Abstract | Founding paper for contiguous-edge windows. Triad closure events, not wedges under this bound |

**Gap.** Wedge windows have no matching tradeoff in the sources read. JGAA gives two end points only. Deng et al. give
the vertex-range form, and Ouyang et al. ([PVLDB 19, 2026](https://www.vldb.org/pvldb/vol19/p1170-wen.pdf), abstract
and related work read) only remark that a temporal graph can be viewed as attributed. Mang's reduction is for
butterflies. A short reduction could be folklore to Tao's group, so this gap is narrow.

**Conjecture W1.** Take a set disjointness instance. For each i and each u in S_i, add event (u, x, i) with a fresh
vertex x. Then wedges in window [a, b] = Σ_u C(x_u, 2), where x_u counts sets in [a, b] holding u. Prefix sums recover
Σ_u x_u^2, and the mixed difference in a and b equals 2|S_a ∩ S_b|. So S and Q for wedges give S and 4Q for set
intersection size, with m = N.
Open steps: check the conjecture's exact form allows this; check that ties in timestamps fit the windows used.

**Conjecture W2.** The weighted problem of Xia et al. inherits the same bound. Add hubs p, q. For u in S_i add (u, p, i)
and (u, q, i). Add (p, q, i) for each i. Instances in [a, b] = (b - a + 1) Σ_u x_u^2. So polylog query needs about m^2
space in the worst case, which would make C-points worst-case optimal, since π = O(m^2).
Open steps: as W1, and whether any reduction exists for distinct triples, where this hub gadget gives only W ≤ m.

**Open problem.** For simple temporal graphs, is Õ(m) space with polylog query possible for triangles, or is storing
about τ points needed? The gadgets above need a central edge per set pair, which makes m about k^2 and the instance
easy. I found no answer.

**Mechanism.** Numerator: signed points, O(W) space, O(log W) query. Denominator: answers on a g × g grid of block
boundaries, plus per-vertex rank or distinct-partner counts. Query: start at the nearest grid window and adjust at most
2m/g events, each O(log m) with prev and next occurrence links. Index size O(g^2 + m + W). Query time Õ(m/g).
At g = √m that is about m words of grid and about √m adjustments.

**Why it escapes Gate 0.** Queries do not scan the graph, and the cost being reduced is per query over a whole window.

**Strongest simple baseline.** Mo's algorithm for a batch of q known windows: about m√q pointer moves, each one scan of
6 to 64 operations, with no index. Also recompute on the filtered window, and O(n) per query by rank on every vertex.

**Cheapest kill test, under one hour.** Build the grid at g = 1024 on sx-superuser and wiki-talk-temporal. Run 1,000
random windows. Compare operations and bytes to Mo's algorithm and to the O(n) rank scan, and read W from the sweep. Kill
if the rank scan restricted to the top 1,000 vertices plus a light-vertex table is within a factor of two, since then
the grid has no practical point and only W1 remains.

**Venue.** ICDT. EDBT short paper if W1 fails to carry weight alone.

**Scores.** Novelty 5/10. Feasibility solo 7/10.

## Candidate 4: two-dimensional index for distinct triangles in a window

**Claim.** For the plain question, the triangle count of the simple graph in [t1, t2], a 2D signed index with polylog
query and about 2W points replaces the 3D KD-tree, built by one sweep rather than per-triangle enumeration.

**Type.** Method, empirical. Weak.

**Closest five.**

| Paper | Read | Overlap |
|---|---|---|
| Xia et al. 2025, Section 5 | Full section in LaTeX | **Near total.** BC-points are the same signed construction in 3D with δ. Fix δ at infinity and this candidate is a projection of theirs |
| Chanchary, Maheshwari 2019, Thm 10 | As above | Same idea for unique timestamps |
| Mang et al. 2025 | As above | Same pattern for butterflies, including duplicate edges through active intervals |
| Christopoulos et al. [Triangle Counting in Large Historical Graphs](https://www.ceid.upatras.gr/webpages/faculty/ktsichlas/Tempo/files/10%20-%20SAC2025_Triangle%20Counting%20in%20Large%20Historical%20Graphs.pdf), SAC 2025 | Full text | No index, Spark, filter then count, synthetic intervals. Its claim of no prior work overlooks the windowed line from 2013 and 2019, though that line uses point events, not intervals |
| Ouyang et al., PVLDB 2026 | Abstract, related work | Linear interval index for retrieving a given set of subgraphs by window. It reports, it does not aggregate distinct triples |

**Gap.** Only constants and build cost. KDT-Index uses a 3D KD-tree whose build enumerates, per static triangle, windows
and durations over all timestamps of its three edges. For δ = ∞ neither is needed.

**Mechanism.** The sweep in the core observation emits at most 2W signed points, where W is the number of minimal
windows over all triples and W ≤ Σ_e mult(e)·tri(e), with mult and tri taken on the union graph. Size O(W) words with a
wavelet tree and sampled ranks, O(W log W) with a persistent Fenwick. Query O(log W) up to the structure's factor. Merged
by position, the positive points are a subset of the C-point positions, so at most π. How much smaller is unknown.

**Why it escapes Gate 0.** Query cost is the point, not update cost.

**Strongest simple baseline.** Xia's KDT-Index with δ = t_max, and static recount on the filtered window.

**Cheapest kill test, under one hour.** Run the sweep on six streams, print W and points per event, build a persistent
Fenwick on the smallest four, run 1,000 random windows, and check against `static_count`. Kill if W is above half of π on
wiki-talk (π = 651M from Xia's table) after merging by position, or if their KDT-Index is within a factor of ten at
δ = t_max.

**Venue.** GRADES-NDA or an EDBT short paper. A reviewer can fairly say this is one remark in Xia et al.

**Scores.** Novelty 3/10. Feasibility solo 9/10.

## Rejected ideas

| Idea | Killed by | Note |
|---|---|---|
| Index for exact triangle count in any window, simple graph | [Chanchary, Maheshwari 2019](https://jgaa.info/index.php/jgaa/article/view/paper489), Thm 10 | Points (high, low), one per triangle, O(log W / log log K) query |
| Same for δ-temporal triangles, with δ chosen at query time | [Xia et al. 2025](https://github.com/xqbf/counting-triangles) | WT-Index with C-points and transformed points. Query is (t_s, t_e, δ). Polylog. Also has append maintenance |
| Same for distinct vertex triples in a window | Xia et al. 2025, Section 5 | KDT-Index with BC-points, at most twice the C-points by their appendix lemma |
| Faster online δ-temporal counting | [Pashanasangi, Seshadhri](https://arxiv.org/abs/2106.02762) (abstract), [Paranjape et al.](https://arxiv.org/abs/1612.09259) (abstract), Xia's OTTC | O(mκ log m) exists, and Xia's abstract reports OTTC up to 70× faster than the prior best. Little room is left |
| Temporal motifs in general | [Kovanen et al.](https://arxiv.org/abs/1107.5646) and [Mackey et al.](https://arxiv.org/abs/1801.08098) (abstracts), [Gao et al.](https://arxiv.org/abs/2204.09236) (abstract) | Mature exact line, plus sampling and GPU systems seen in the arXiv screen |
| Known expiry beats OMv in theory | [Bernstein et al. 2025](https://arxiv.org/abs/2510.20341), Thm 1.4 | Insert-only already carries the barrier |
| Lift a good incremental counter to sliding windows | [Peng, Rubinstein](https://arxiv.org/abs/2211.05178) plus Kara et al. | Nothing to lift: O(√m) fully dynamic exists and scanning beats it in practice per the audit |
| Predictions for exact counting in practice | [Henzinger et al.](https://arxiv.org/abs/2307.16771), Thm 6.10 and 8.5; [Venturin et al.](https://arxiv.org/abs/2506.13173) (abstract) | Delay O(n) gives no bound below OuMv for #s-△. STEP already uses predictions for approximate counts |
| Time-travel point query, count at one instant | No paper needed | One forward pass stores c_t. Binary search answers it. Size O(events) |
| Index for "simultaneously active" triangles in interval-edge graphs | Own observation against [Christopoulos et al.](https://www.ceid.upatras.gr/webpages/faculty/ktsichlas/Tempo/files/10%20-%20SAC2025_Triangle%20Counting%20in%20Large%20Historical%20Graphs.pdf) | With one co-active interval per triple, overlap count = total - ended before t1 - started after t2. Two sorted arrays. Too easy to publish |
| Windowed cohesive subgraph with triangles: span-truss or (k,δ)-truss | [Hu, Zhong, Li](https://arxiv.org/abs/2606.11582) (abstract) | (k,δ)-truss indexes exist as of June 2026. Different output from a count |
| Historical or span k-core analogue | [Yu et al.](https://www.vldb.org/pvldb/volumes/14/paper/On%20Querying%20Historical%20K-Cores) (abstract), [Galimberti et al.](https://arxiv.org/abs/1808.09376), [Yang et al.](https://arxiv.org/abs/2301.03770), [ECB-forest](https://arxiv.org/abs/2508.18151) (abstracts) | Crowded, and these report subgraphs rather than counts. Not this project's strength |
| Repeated edges with forgetting | [Chakraborty et al.](https://arxiv.org/abs/2609.19943) (abstract) | Approximate, optimal space. Cites Xia et al. Not exact, so it does not close Candidate 1, but it shows active attention |

## Search log and limits

**Web search.** Terms included: historical triangle counting; exact triangle count arbitrary time window index; multiple
sliding windows exact triangle; exact sliding window no deletion; offline dynamic triangle matrix multiplication;
temporal triangle fine-grained dominance; deletions look-ahead OMv; clustering coefficient arbitrary time window;
historical k-truss; relational event graph subgraph counting.

**arXiv API.** Twelve abstract queries, sorted newest first, up to 60 records each, on temporal triangles, sliding
windows, temporal motifs, look-ahead, predictions, offline dynamic, historical graphs, span-cores, truss, time-windowed,
lifetimes, expiration. Two queries stopped on an encoding error partway through the listing, so they show fewer records.
Then HTTP 429 stopped three more: range subgraph, temporal triangle lower bounds, and exact counting with windows. Several
broad queries returned mostly noise, so relevance filtering was poor.

**OpenAlex.** One lookup worked, for Xia et al. The next returned 429 with a 23-hour retry. No OpenAlex screen was done.

**Other limits.**
- DBLP not used.
- ACM, ResearchGate and Springer pages returned 403 or a login redirect. The published Xia et al. paper was not opened;
  the authors' LaTeX source and technical report in their public repository were.
- Citation screen for Xia et al. came from Semantic Scholar and lists three citing works. That index is incomplete.
- I could not identify the "Sun" paper named in the brief for delta-temporal counting. One search found none.
- The user-named "Liu and Srinivas 2024" was read in its arXiv 2023 form.
- Kara et al. was read at abstract level only. The O(√m) figure comes from the audit and from statements in other papers.
- Proof sketches here were not checked by anyone else, and no code was run.
- "I found nothing" in any gap above is weak evidence, especially for proceedings of 2026.
