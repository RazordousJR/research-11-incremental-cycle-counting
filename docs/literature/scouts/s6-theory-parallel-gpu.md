# Scout S6: theory, multicore and GPU

Date: 2026-11-01. Coverage target: work up to 1 November 2026. Everything labelled "probe" is mine, non-citable,
and ran in the scratch folder, not in project code. Everything labelled "conjecture" is my derivation. It has had no
second reader and is not a theorem.

## 1. Bottom line

1. **The multicore question is taken.** DPTL and DPTL+ (ICDE 2021) give exact, exactly-once batch processing with no
   duplicate removal and no per-triangle division, in parallel, by pivot vertex. **The audit missed this paper, and it
   also damages audit decision 3.** DPTL+ builds one bitmap per pivot vertex and reuses it across that pivot's batch
   edges. That is sharing between the updates of a batch in a scanning algorithm, which the audit recorded as unstudied.
   DPTL+ also tested time-cut batches on sx-stackoverflow.
2. **Parameterised batch bounds are taken.** Liu et al. (SPAA 2022) give `O(|B| α log² n)` amortized for triangles
   (`k = 3` in their Theorem 3.7). Fan and Lu give `O(bh)`. DPTL+ gives `Θ(Σ min deg + m_adj)`.
3. **The batch-size theory is not fully settled.** The DLSY bound is matched for combinatorial algorithms at every
   batch size, by their own reduction. It is not matched in general. Candidate 2 sets out what I could derive.
4. **Raw GPU or multicore throughput is not a real target on our streams.** The largest stream needs about 61M kernel
   operations in total under the 14-day rule (probe, §6), which should take seconds on one core. The GPU case exists
   only on far larger base graphs, which are not in `data/snap/`.
5. **Best lead found: paying for a triangle's death at its birth.** When an edge's removal time is known at insertion,
   the delete scan can be dropped. A probe on all six streams gave 0 count mismatches per event and a kernel-only
   ratio of 0.518 to 0.630 against `seq_raw`. This came out of the update-sequence theory rather than from parallelism.
   It depends on the synthetic expiry rule, so read the caveats in Candidate 1.

## 2. Answers to the questions asked

**(a) Does O(Δ√(Δ+m)) have a matching batch lower bound for every Δ?**
Combinatorial algorithms: yes, for all Δ ≤ m. DLSY §3.1 splits a static triangle-detection instance into batches and
appeals to the Strong Triangle conjecture. General algorithms: no such statement was found. DLSY cite no OMv bound.
The term OMv does not occur in their text. Their bound is also beaten at very large Δ by static fast matrix
multiplication. Kara et al. describe the single-tuple versus bulk gap and leave a characterisation of the difference
open (arXiv version, related-work and discussion passages). Hu and Wang (PODS 2025)
list the trade-off with batch size as open. Those two passages are direct hooks. I found no paper resolving either.
This is weak evidence, see §8.

**Is a clean new bound open?** Narrowly. See Candidate 2.

**(b) Multicore.** Peeling needs no colouring. Its order can be replaced by a read-only priority test: edge `f` counts
as present for edge `e` only if `f` is unchanged or has a higher rank. Then every step runs on one frozen graph. This
is the telescoping delta rule of Ammar et al. GAMMA states the same rule for general patterns on GPU (a total order on
updates, matches formed from lower to higher order). DPTL reaches the same end by a case split on orientation.
So proposal §3.5 item 4 is too strong: peeling itself is order-bound, but its count is not. Reported speedups are in §6.

**(c) GPU.** Open items are few. No paper found compares GPU dynamic counting against tuned sequential or multicore
scanning on real streams. The GPU papers read compare mainly against static recount or other GPU systems. No GPU
version of heavy-light tables was found. EDTC's full text could not be reached, so its overlap remains partly unknown.
Two premises in the brief need correcting. **HyKernel appears to be static counting**: its abstract page uses "dynamic"
for run-time kernel selection, and its comparison is H-Index, a static counter. **gSWORD is sampling**, so it is
approximate ([record](https://api.crossref.org/works/10.1145/3639288), title and search summary only).

## 3. Candidate 1: deaths paid at birth (method, small theory note)

**Claim.** If each edge's removal time is known when it is inserted, exact triangle maintenance needs scanning only at
insertion, and the delete scan disappears, at a cost of one counter write per triangle.

**Closest papers.**

| Paper | Read | Overlap |
|---|---|---|
| Ghanem et al., [TKDE 2007](https://www-users.cse.umn.edu/~mokbel/papers/TKDE2007.pdf) | Abstract, intro, join passages | **Highest.** States that a join output expires at the minimum expiry of its inputs. General stream operators, no triangle kernel or graph scan comparison. **The principle is 19 years old** |
| Peng, Rubinstein, [arXiv:2211.05178](https://arxiv.org/abs/2211.05178) | Abstract | Black-box reduction of known-deletion-order to incremental with a log factor. The trick below needs no rewinding because a count is invertible |
| Hu, Wang, [PODS 2025](https://cs.uwaterloo.ca/~xiaohu/papers/pods25-update.pdf) | Intro, FIFO definition, Theorem 1.11, conclusion | FIFO dichotomy for free-connex queries. The triangle is cyclic, so this does not cover it. No constant-factor scan result |
| Abo Khamis et al., [arXiv:2312.09331](https://arxiv.org/abs/2312.09331) | Intro | Encodes tuple lifespans as intervals. General worst-case framework with unknown deletion times, not scan elimination |
| Hanauer et al., [arXiv:2610.29776](https://arxiv.org/abs/2610.29776) | Stream setup line and related work; the audit read the main text | Defines the benchmark. Removes edges two weeks later. Per the audit its best triangle method is plain scanning. The word "expiry" does not occur in its text. I did not confirm how it handles deletions in detail |

Also near: [Liu, Srinivas](https://arxiv.org/abs/2307.08890) (abstract) and
[Henzinger et al.](https://arxiv.org/abs/2307.16771) (abstract) on predicted updates. Approximate sliding-window work
(TRIEST-FD, SWTC) is sampling, seen in search results only.

**Precise gap.** I found no exact triangle counter that uses known expiry to skip delete scans. Two web searches and an
arXiv query on sliding window, triangle and exact returned nothing relevant, which is weak evidence. A database reader may call it a direct application of 2007 work.

**Mechanism.** Store expiry `x(e)` beside each adjacency entry, and a counter `d(e)`. On inserting `e`, scan as usual.
For each common neighbour `w`, take the edge among `e, pw, qw` with the smallest expiry (ties by delete sequence) and
do `d(that edge) += 1`. On removing `f`, do `T -= d(f)`, remove it, and do not scan. A triangle dies exactly when the
first of its edges goes, which was known at its birth. So `d(f)` is exact at `f`'s removal.

Batch form: a deletion batch is `O(Δ)` with no scan and is trivially parallel. An insertion batch runs read-only under
the priority test, with fetch-and-add on `d`. No sort and no division.

**Why it escapes the Gate 0 failure.** Gate 0 failed because one scan costs too little for updates in a batch to share.
This does not share within a batch. It shares across time, between a triangle's birth and death, so it removes a whole
class of scans rather than a fraction of one. Batch overhead is close to zero.

**Probe.** All six streams, Gate 0 rules, stream order, no batches. Scan cost 2 per element, as in the protocol.

| Stream | Triangles born | Count mismatches per event | Kernel ratio | With graph update charged | Incarnations ended early |
|---|---|---|---|---|---|
| CollegeMsg | 8,312 | 0 | 0.518 | 0.623 | 6.7% |
| email-Eu-core-temporal | 173,362 | 0 | 0.630 | 0.725 | 54.1% |
| sx-mathoverflow | 99,600 | 0 | 0.550 | 0.712 | 1.1% |
| sx-askubuntu | 89,701 | 0 | 0.539 | 0.770 | 0.4% |
| sx-superuser | 149,628 | 0 | 0.539 | 0.756 | 0.3% |
| wiki-talk-temporal | 978,136 | 0 | 0.532 | 0.661 | 4.8% |

Ratios are against `seq_raw`, the true single-update S1, not the gate's `seq_net`. Kernel ratio is `(insert scans +
2 × triangles) / (insert scans + delete scans)`. The last ratio adds 2 per applied edit on both sides and 1 per delete on
the new side. Update counts match the Gate 0 README on the four streams checked there, and every run ends at count 0.

**Caveats, all serious.**
- **The expiry is synthetic.** Real fully dynamic streams do not announce deletions. The method only applies to
  time-to-live or sliding-window semantics. That is a genuine use, but not the proposal's problem statement.
- **The streams are not strictly FIFO.** Under the set rule, a stale delete can end a later incarnation early, which
  the last column measures. Death is still known at birth, but only with per-pair pending-delete state. The probe
  read this from a first pass. Its online cost is not charged. Expect the ratios to rise.
- **Refresh semantics may break it.** If a repeat contact extends an edge's life, `d` must be re-homed. Not measured.
- It gains a factor of at most 2 on scans. It does not change the worst case, and hub-hub wedge tables cannot use it,
  because they count without enumerating.
- Operation counts, not time. The extra expiry field may cost cache misses.
- Only 2 of 6 streams reach 0.67 with update cost charged, so read against `seq_raw` it sits in the pivot band of the
  gate, not the go band. All six are below the stop line of 0.80.

**Strongest simple baseline.** `seq_raw`, tuned. I also tested per-edge support counters, which need no knowledge of
the future: skip the delete scan when support is 0, stop early otherwise. They lost. Ratio with update cost 0.984 to
1.401, even though 41% to 90% of delete scans were skipped. The skipped scans are the cheap ones. 0 mismatches.

**Cheapest kill test, under 1 hour.** Add a refresh variant of the probe, with expiry at last contact plus 14 days and
re-homing of `d` at the stale pop, and charge online pending-delete state to the strict variant. Then time both on
`FlatMap` adjacency from `src/r11/tc.hpp`, on a copy. **Kill if** the with-update ratio exceeds 0.85 on 4 or more
streams under refresh semantics, or if wall-clock gain is below 1.15 on the strict rule.

**Likely venue.** JGAA or a short paper at SEA or ALENEX, unconfirmed. Alone it is a note. It becomes a paper only with
the batch and parallel form plus time measurements.

**Scores. Novelty 5/10, feasibility solo 9/10.**

## 4. Candidate 2: exponent map of batch size (theory)

**Claim.** The optimal batch work for exact global triangle counting is pinned down for `Δ ≤ √m`, can be improved over
published bounds for `Δ` above about `m^0.686`, and stays open for `√m < Δ < m`, with a stated barrier for OMv reductions.

**Closest papers.**

| Paper | Read | Overlap |
|---|---|---|
| Dhulipala, Liu, Shun, Yu, [arXiv:2003.13585](https://arxiv.org/abs/2003.13585) | §1 to 3, parts of 6, §7 | **Highest.** Upper bound, combinatorial optimality, FMM route. Excludes `k = 3` from the FMM algorithm and remarks that a fixed Δ allows a better threshold, without stating a bound |
| Kara et al., [arXiv:2004.03716](https://arxiv.org/abs/2004.03716) | §9, related work, future work | OMv bound for Δ = 1. Names the bulk versus single-tuple gap and leaves it open |
| Henzinger et al., [arXiv:1511.06773](https://arxiv.org/abs/1511.06773) | §1, Theorem 2.2, triangle reductions | OMv, unbalanced form, source of the reduction |
| Hu, Wang, [PODS 2025](https://cs.uwaterloo.ca/~xiaohu/papers/pods25-update.pdf) | As above | Lists the batch-size trade-off as open |
| Dhulipala et al., [arXiv:1908.01956](https://arxiv.org/abs/1908.01956) | Abstract, lower bound sections | Batch-dynamic lower bounds, but MPC and adaptive connectivity. No triangles or OMv |

Also: [Kopelowitz, Pettie, Porat](https://arxiv.org/abs/1407.6756) (§1.5): 3SUM-based `Ω(mα^(1-o(1)))` for triangle
*enumeration*. It binds listing, including DPTL, and not counting.
[Alman et al.](https://arxiv.org/abs/2404.16349) (abstract): ω < 2.371177.

**Everything below is conjecture, derived here. Let `Δ = m^δ`, `δ ≤ 1`, and let W be amortized work per batch.**

- **Conjecture 2.1 (OMv, small batches).** For `δ ≤ 1/2`, no polynomial-preprocessing algorithm has
  `W = O(Δ m^(1/2-ε))`. Sketch: take OuMv of dimension `n = √m`, as in the standard triangle reduction. One query
  changes `O(n)` spokes at a fixed vertex. Cut those into `O(n/Δ)` batches and read the count after the last. So
  `n · (n/Δ) · W ≥ n^(3-ε)`. *Open: check that padding and mixed batches meet the problem definition.*
- **Conjecture 2.2 (OMv, large batches).** For `δ ≥ 1/2`, `W = Ω(m^(1-ε))`. One query per batch, padded with changes
  on isolated edges.
- **Conjecture 2.3 (upper bound).** For fixed Δ, `W = O(min(Δ√m, (Δm)^(ω/(ω+1))))` plus rebuild terms. Sketch: threshold
  `t`. Changed edges touching a light vertex enumerate in `O(t)` under the priority test. Heavy-heavy changed edges read
  a Kara-style wedge table for a light third vertex, which also solves the case DLSY excluded for `k = 3`, since only one
  heavy-heavy edge can occur in such a triangle. All-heavy triangles come from `Tr(A³)` per batch at `(m/t)^ω`. Balance
  at `t = (m^ω/Δ)^(1/(ω+1))`. *Open: amortization of reclassification and table rebuild under this `t`; the mixed
  insert and delete case; whether the DLSY remark already implies it in the eyes of a reviewer.*
- **Conjecture 2.4 (star-structured bound).** With the batch assigned to pivots with `k_x` spokes,
  `W = O(Σ_x min(k_x √m, m))`, using per-edge heavy-light for narrow stars and one sweep over all edges for wide ones.
  OMv's hard instance is a single star of about `√m` spokes, so this is tight there. *Open: picking pivots well,
  and tightness for several stars.*
- **Barrier 2.5.** A batch reveals one count, so `O(log n)` bits. An OuMv reduction needs about one batch per query, and
  the matrix must sit in the graph, so such a reduction cannot prove more than about `Ω(m)` per batch. Per-vertex counts
  escape the bit argument, but batched OMv has rectangular FMM algorithms. *Open: making "reduction" precise.*

| δ | Published upper | Conjectured upper | Lower, with 2.1, 2.2 and the FMM form of Strong Triangle |
|---|---|---|---|
| 0.25 | 0.75 | 0.75 | 0.75 |
| 0.50 | 1.00 | 1.00 | 1.00 |
| 0.60 | 1.10 | 1.10 | 1.01 |
| 0.75 | 1.25 | 1.23 | 1.16 |
| 0.90 | 1.40 | 1.34 | 1.31 |
| 1.00 | 1.41 | 1.41 | 1.41 |

Exponents of m, using ω = 2.371177. "Published upper" takes DLSY's formula at `k = 3`, although they exclude that case.
The FMM-sensitive form of Strong Triangle is an assumption here. I read only the weaker statement in HKNS Appendix A.

**Precise gap.** No source I read states a lower bound in OMv for general Δ, or a triangle FMM bound tuned to Δ. If ω = 2,
the middle gap is still open, with an upper exponent of `2(1+δ)/3` against a lower exponent of `max(1, δ + 1/3)`.

**Why it escapes Gate 0.** It does not depend on real streams. Conjecture 2.4 explains Gate 0 instead: grouping helps
worst-case work only for stars wider than `√m`, and real batches rarely have them.

**Strongest simple baseline.** Corollaries of existing work. A reviewer may hold that 2.1 and 2.2 are re-cuts of HKNS
and 2.3 is the DLSY remark plus AYZ. The new content would be 2.4, 2.5 and the map.

**Cheapest kill test, under 1 hour.** One reading pass: the OMv and batch sections of Olteanu's IVM survey, Abo Khamis
et al. in full, and the journal version of DLSY if one exists, looking for a Δ-indexed statement. Then a line-by-line
proof attempt for 2.3 for `k = 3`. **Kill if** any source states the map or if amortization fails. It needs no code.
For contact with the data, count stars with `k > √m` on each stream using the Gate 0 batches. Near zero is a support for
the Gate 0 explanation, not a kill.

**Likely venue.** SOSA or Information Processing Letters, unconfirmed.

**Scores. Novelty 6/10, feasibility solo 6/10.** Solo risk: this sits outside the PI's CUDA expertise, and subtle
errors in amortization are likely. It needs a theory reader before any submission.

## 5. Candidate 3 and Candidate 4

### Candidate 3: same-machine bake-off with parallel scanning (empirical)

**Claim.** On one consumer machine, a plain parallel lower-degree scan under a read-only priority test is the strongest
baseline for exact batches, and the batch size at which each of DPTL+, DLSY, inclusion-exclusion and a GPU overtakes it
can be stated as a crossover map.

| Paper | Read | Overlap |
|---|---|---|
| Yu et al., DPTL+, [ICDE 2021](https://opus.lib.uts.edu.au/handle/10453/153684), [PDF](https://opus.lib.uts.edu.au/bitstream/10453/153684/3/DPTL%20Efficient%20Parallel%20Triangle%20Listing.pdf) | Cover letter, §I to III, experiment excerpts | **Highest.** Listing, not counting. Baselines were sort-merge and static listing on the influenced graph. It reports no clear gain from conventional hashing with collisions, which sits oddly with hash-based scanning being best in Hanauer et al. and needs testing |
| DLSY, [arXiv:2003.13585](https://arxiv.org/abs/2003.13585) | As above | Compared against its own CPU port of Makkar, which merges sorted lists, not against a hash-probing scan. Randomly permuted batches |
| Hanauer et al., [arXiv:2610.29776](https://arxiv.org/abs/2610.29776) | As above | The result this extends. One core, single updates |
| Makkar et al., [HiPC 2017](https://davidbader.net/publication/2017-mbg/2017-mbg.pdf) | Abstract, batches, §V | K40. Batches drawn from the graph, not from real time order |
| Wang et al., EDTC, [TPDS 2026](https://doi.org/10.1109/TPDS.2025.3627974) | Abstract only, through Semantic Scholar API | Exact GPU system. Evaluation unseen. **Could already contain part of this** |

**Precise gap.** No same-machine comparison of these methods was found, including graph update time. DPTL+ does not
compare with DLSY (listing against counting). The GPU papers read do not compare with Hanauer-style scanning.

**Mechanism.** Merge the batch into adjacency with a rank on each changed entry, keep one bit per touched vertex so that
rank lookups are skipped for untouched `w`, then loop read-only in parallel. Port the same loop to CUDA with a flat edge
hash. No Makkar three-pass inclusion-exclusion, which Meerkat also uses, and no sort.

**Why it escapes Gate 0.** It seeks throughput, not sharing. But see the kill test: on our streams there may be no
throughput to win.

**Strongest simple baseline.** One-core `seq_raw`. Then the same scan on 20 threads.

**Cheapest kill test, under 1 hour.** Time `seq_raw` single-thread on `FlatMap`, on wiki-talk-temporal and its
no-expiry variant, and compute mean batch time at the 10k cut. **Kill for SNAP streams** if a full replay is under about
10 s or the mean 10k batch is under about 1 ms, because a parallel fork-join then barely pays. RTIndeX reports GPU
lookup throughput saturating near 2^21 lookups per batch on an RTX 4090. At about 10 lookups per update, that suggests
200,000 or more updates per batch for a GPU to be fully used. If killed here, the study moves to larger graphs, which
need new data and a new protocol.

**Likely venue.** SEA, or GrAPL at IPDPS, unconfirmed. **Scores. Novelty 4/10, feasibility solo 7/10.**

### Candidate 4: dense heavy core in VRAM (method, GPU)

**Claim.** On a 16 GB consumer GPU, holding the top `h` vertices as a bit matrix plus a dense wedge-count matrix makes
hub-hub updates constant-cost, while a CPU scans everything else, with `h` chosen by batch size as in Conjecture 2.3.

| Paper | Read | Overlap |
|---|---|---|
| DLSY, [arXiv:2003.13585](https://arxiv.org/abs/2003.13585) | As above | Heavy-light on multicore with hash tables. Beat Makkar on Twitter and dense rMAT |
| Wang et al., EDTC, [TPDS 2026](https://doi.org/10.1109/TPDS.2025.3627974) | Abstract only | Addresses hub memory with a compressed hash table and loads the affected subgraph. **Unknown overlap on hub handling** |
| Yaşar et al., [arXiv:2009.12457](https://arxiv.org/abs/2009.12457) | Abstract, parts | Static CPU plus GPU by blocks. Friendster 2.1 s, or 3.2 s with copy time |
| Pandey et al., TRUST, [arXiv:2103.08053](https://arxiv.org/abs/2103.08053) | Abstract | Static hash-based GPU counting. Over a trillion TEPS |
| Hanauer et al., [arXiv:2610.29776](https://arxiv.org/abs/2610.29776) | As above | Per the audit, scanning beats auxiliary structures for triangles on one core |

**Precise gap.** No GPU heavy-light batch triangle maintenance was found. Search here was thin, and IEEE Xplore pages
gave no text.

**Mechanism.** Memory arithmetic: 16 GB holds a 32-bit `h × h` matrix up to about `h = 63,000`, or a bit matrix up to about
`h = 358,000`. Batch: hub-hub edges read the wedge matrix and AND two bit rows. Light-vertex edges emit wedge
increments for a sort-reduce or atomic pass. CPU scans light edges.

**Why it escapes Gate 0.** It targets the expensive tail of updates, not the cheap mean.

**Cheapest kill test, half run.** Probe on cost concentration, using `2 × min(deg)` per update:

| Stream | 14 days: top 1% share | 14 days: share with min deg ≥ 1000 | No expiry: ops per update | No expiry: top 1% | No expiry: share with min deg ≥ 1000 | No expiry: largest update |
|---|---|---|---|---|---|---|
| CollegeMsg | 7.5% | 0 | 19.5 | 8.0% | 0 | 370 |
| email-Eu-core-temporal | 5.3% | 0 | 40.3 | 5.4% | 0 | 416 |
| sx-mathoverflow | 10.4% | 0 | 54.7 | 12.7% | 0 | 1,866 |
| sx-askubuntu | 23.7% | 0 | 24.4 | 30.9% | 3.3% | 6,012 |
| sx-superuser | 21.1% | 0 | 32.4 | 24.5% | 2.8% | 6,014 |
| wiki-talk-temporal | 23.6% | 0.3% | 62.5 | 29.7% | 12.9% | 44,716 |

**On these streams it is dead.** The expensive tail barely exists under the 14-day rule and is small even with no
expiry. It could only live on graphs at Twitter scale. Remaining test: edge share of the top `h` vertices at no expiry.
**Kill for good** if the tail share stays under 20% on the largest graph available.

**Strongest simple baseline.** 20-thread scanning, and DLSY in GBBS. **Likely venue.** ICPP or TPDS, unconfirmed.
**Scores. Novelty 7/10, feasibility solo 3/10.** High build cost, and new data is needed before it can even be tested.

## 6. Numbers reported

Not comparable across rows: different hardware, graphs and years.

| Source | Setting | Number |
|---|---|---|
| DLSY | 72 cores, 144 threads, batch 2M | 36.54x to 74.73x self-relative. Orkut 1.05 to 16.2M edges/s. Twitter 0.935 to 5.46M. rMAT 3.08 to 32.4M |
| DLSY | Against Makkar port | Up to an order of magnitude on Twitter. Makkar port ahead on Orkut |
| DPTL+ | 32 threads, 4 graphs | Total 18, 6, 12, 7. Listing only 21, 10, 16, 11. Update phase is the brake |
| DPTL | Same | Total 25, 17, 18, 16 |
| Makkar | K40, 2017 | Count only 32M updates/s; with graph update 11M. Below about 1,000 per batch, overheads dominate and prefix sums take milliseconds |
| Hornet | [HPEC 2018](https://davidbader.net/publication/2018-bgbb/2018-bgbb.pdf), parts | Peak about 48M updates/s with deletes and dynamic counts. Structure alone over 200M |
| faimGraph | [SC18](https://hpdgp.mpi-inf.mpg.de/pappe/faimGraph.pdf), abstract | Up to 200M edge updates/s, structure only |
| Meerkat | [arXiv:2305.17813](https://arxiv.org/abs/2305.17813) §6.3, appendix | Static counting 31.12x slower than Hornet on average. Dynamic speedup over its own static recount of hundreds to about 16,000x at 10K batches. Hash-based. No updates/s figure |
| DYTC | [ICACS 2024](https://doi.org/10.1145/3708597.3708610), abstract | Up to 7993x over the prior algorithm. Baseline not stated in abstract |
| EDTC | Abstract | No number in the abstract |
| WarpCore | [arXiv:2009.07914](https://arxiv.org/abs/2009.07914), abstract | Up to 1.6B inserts, 4.3B retrievals/s, GV100 |
| HyKernel | [HPEC 2021](https://researchconnect.buffalo.edu/en/publications/hykernel-a-hybrid-selection-of-onetwo-phase-kernels-for-triangle-/), abstract page | Appears static. Geomean 7.2x over H-Index, up to 43.6x |
| Our streams | Gate 0 and probe | 61M kernel ops total on wiki-talk under the 14-day rule, 174M with no expiry |

No RTX 50-series result was found for any of these workloads.

## 7. Rejected ideas

| Idea | Killed by |
|---|---|
| New parallel exactly-once rule with no sort and no division | **DPTL, DPTL+** ([ICDE 2021](https://opus.lib.uts.edu.au/handle/10453/153684)). Also the delta rule of [Ammar et al.](https://arxiv.org/abs/1802.03760) (audit row 7) and the total-order rule in [GAMMA](https://arxiv.org/abs/2401.17018) (§IV), which credits Guo et al. |
| Parallel peeling by colouring the batch graph | Needless, for the same reason. A read-only priority test replaces the order |
| Grouping around pivots as a new idea for batches | **DPTL+**: bitmap once per pivot, with degree guarantee. Proposal's star grouping is close to it, in listing form. Reposition any surviving claim against it |
| Batch bound in arboricity or degeneracy | [Liu et al., SPAA 2022](https://arxiv.org/abs/2106.03824), Theorem 3.7 (statements read): `O(|B| α^(k-2) log² n)`. Space `O(mα^(k-2) + n log² n)` |
| Batch bound in h-index | [Fan, Lu](https://math.mit.edu/research/highschool/primes/materials/2020/Fan-Lu.pdf), abstract: `O(bh)`, not peer reviewed as far as seen |
| Presenting the `O(kλ)` star bound of proposal §3.6 as new | [Lu, Tao, ICDT 2021](https://drops.dagstuhl.de/entities/document/10.4230/LIPIcs.ICDT.2021.6), abstract in part, with the arboricity-sensitive bound as recorded in audit row 10, and Liu et al. above |
| Better low-orientation route to batches | Already available: [Blelloch et al.](https://arxiv.org/abs/2602.17811) improve orientation. No triangle application in its text (full-text search), so a plug-in corollary may be open but is minor |
| Offline replay by lifespan intervals | [Abo Khamis et al.](https://arxiv.org/abs/2312.09331) own the reduction. Replay is also seconds on one core, so nothing to gain |
| Ray-tracing cores as hardware set intersection | [RTIndeX](https://arxiv.org/abs/2303.01139) (§4): a GPU hash table clearly beats it on point lookups. [cgRX](https://arxiv.org/abs/2406.03965) (abstract) names poor updateability |
| Edge support counters to skip delete scans | My probe (§3): ratio 0.984 to 1.401 |
| GPU batch kernel on these streams | Gate 0 operation totals, Makkar's small-batch overheads and RTIndeX's saturation point |
| Instance-optimal join theory for this problem | Not pursued. [Survey](https://arxiv.org/abs/1310.3314) abstract only. Certificate results concern one query, and Gate 0 shows the instance gain is small. **Not a proper rejection** |

## 8. Search record and limits

- **arXiv API.** Heavy 429 throttling, shared with parallel scouts. Completed: title queries on triangle plus
  dynamic, batch, streaming, incremental; "batch-dynamic" with triangle, clique, subgraph; "batch-dynamic" with "lower
  bound"; GPU with dynamic, title on triangle, subgraph, motif, clique, truss; title GPU with dynamic graph; "triangle
  counting" with GPU or heterogeneous; ray tracing; continuous subgraph matching with parallel or GPU; orientation with
  "batch-dynamic". Two returned **no records**: "online matrix-vector" with batch or sensitivity, and "sliding window" with
  triangle and exact. Set intersection on GPU failed three times.
- **OpenAlex.** Daily budget exhausted, 429 on every call. **Nothing was screened there.** I did not make a key.
- **DBLP.** Not used, as instructed. **IEEE Xplore** returned empty pages. So **EDTC and DYTC are abstract only**, and
  their evaluations, and DPTL+'s forward citations, are unscreened. A web search for its citers found none, and that
  search cannot show absence.
- **Web search.** Roughly 20 queries, on the exact topics in §2.
- **Not read:** Mhedhbi TODS full text, Kara ICDT version, Olteanu survey, GPU work behind paywalls, faimGraph and Hornet
  evaluation sections beyond the lines cited.

Any claim here that something is missing rests on these searches, and should be treated as weak evidence.

Read levels for items in tables reflect what I opened. Where a table says "abstract", no method claim should be drawn.

## 9. Probe files

Scratch only, not project code.
- `C:\Users\esport34\AppData\Local\Temp\claude\C--Develop-20261031---Research-11-Local-Incremental-Triangle-Counting-in-Dynamic-Graphs\ee45f6ca-1e1f-4c35-84d3-b548251876ff\scratchpad\s6\scripts\probe_birth.cpp`
- `C:\Users\esport34\AppData\Local\Temp\claude\C--Develop-20261031---Research-11-Local-Incremental-Triangle-Counting-in-Dynamic-Graphs\ee45f6ca-1e1f-4c35-84d3-b548251876ff\scratchpad\s6\scripts\probe_sup.cpp`
- `C:\Users\esport34\AppData\Local\Temp\claude\C--Develop-20261031---Research-11-Local-Incremental-Triangle-Counting-in-Dynamic-Graphs\ee45f6ca-1e1f-4c35-84d3-b548251876ff\scratchpad\s6\scripts\probe_fifo.cpp`
