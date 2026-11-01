# Scout S5: change the output or the guarantee, not the mean

Date: 2026-11-01. Cut-off: 1 November 2026. Inputs read first: `docs/literature/audit-2026-11-01.md`,
`results/gate0-20261101-d2efe26/README.md`, `results/diag-oracle-20261101-e267b9f/README.md`.

## Verdict

| Direction | Finding |
|---|---|
| (a) Local outputs | Exact per-vertex and per-edge counts are taken or trivial. Truss maintenance is crowded, and its theory is closed by lower bounds. No candidate survives on its own. |
| (b) Tail latency | No published work found on p99 of exact counting. On our six streams the tail is too small to need a fix. The worst scan is 22,358 list elements. A competitive ratio for index admission already follows from database tuning results. |
| (c) De-amortisation | Taken for triangles by the `late` partition of Hanauer et al. A budgeted scan with a certified interval is the only opening, and it is weak. |
| (d) Certified counts | **The verifier side is published.** Thaler (ICALP 2016) has O(1) verifier work per update. **The prover side looks open.** Published prover bounds are O(L·n), O(n³), or the cost of A² plus O(n²). I found no sparse prover near the cost of one recount, and no implementation on real streams. This is the top candidate. |

Direct answers to the questions in the brief.

- **Is an incremental low-overhead certificate for a dynamic triangle count published?** Yes for the verifier. Thaler's verifier
  takes O(1) time per update, O(n) space and O(n) proof, on turnstile streams. The prover takes O(mn), where m is stream length.
  Low overhead for the prover is not published in anything I read.
- **Does any work give a competitive ratio for index admission in dynamic counting?** None found. The general result exists:
  Schnaitter and Polyzotis prove a ratio of 2^(c_max+1) − 1 for online index tuning with non-interacting parts, which gives 3 for
  single independent indices. Bruno and Chaudhuri had 3 for the one-index case, as reported by those authors. A hub-pair cache is
  close to a direct instance, so a ratio here is a corollary, not a result. See Rejected idea R5.
- **What are Hanauer et al.'s maximum update numbers for triangles?** They print no table of these. Read from log-scale plots in
  Figures A.4(a) and A.5(a), dataset ES, geometric mean: about 10⁻² s for HHH*-H once ε is large, about 7×10⁻³ s for EGST*-HD, and
  about 0.3 to 0.4 s for `orig`. Reduction factors they do state: `late` beats `orig` by 9.6 to 42.6 (all patterns), and HHH*-H beats
  HHH* by 116.1×. Their appendix names "expensive capacity increases in hash maps" as a driver of peaks.

## Measurements made by this scout

These are scratch runs, not gates. They were made to rank candidates honestly and need a proper protocol before anyone cites them.
Python, stream rule copied from `docs/experiments/gate0-protocol.md`. No wall-clock claims except where marked. Scripts:
`C:\Users\esport34\AppData\Local\Temp\claude\C--Develop-20261031---Research-11-Local-Incremental-Triangle-Counting-in-Dynamic-Graphs\ee45f6ca-1e1f-4c35-84d3-b548251876ff\scratchpad\s5_tools\`
(`tail.py`, `sumcheck.py`, `local_sc.py`, `expiry.py`, with `.out` files).

**M1. Scan cost per applied update.** Cost is min(d_u, d_v), so these units are roughly half of Gate 0's unit-cost figures.
"Repeat" is the share of the top 0.1% tail cost that falls on a repeat of a pair already seen in that tail. It is the most a reactive
pair cache could serve. On expiry streams many repeats are the paired insertion and expiry of one edge. I did not separate those.

| Stream | Variant | Updates | Mean | p99 | p99.9 | Max | Cost in top 0.1% | Repeat |
|---|---|---|---|---|---|---|---|---|
| CollegeMsg | 14-day expiry | 32,226 | 5.9 | 35 | 54 | 105 | 1.1% | 0.00 |
| CollegeMsg | insert-only | 13,838 | 9.8 | 59 | 98 | 185 | 1.3% | 0.00 |
| email-Eu-core | 14-day expiry | 235,090 | 5.5 | 25 | 35 | 72 | 0.7% | 0.60 |
| email-Eu-core | insert-only | 16,064 | 20.2 | 91 | 129 | 208 | 0.7% | 0.00 |
| sx-mathoverflow | 14-day expiry | 489,520 | 3.3 | 26 | 46 | 101 | 1.7% | 0.62 |
| sx-mathoverflow | insert-only | 187,986 | 27.4 | 249 | 482 | 933 | 2.1% | 0.00 |
| sx-askubuntu | 14-day expiry | 963,534 | 2.1 | 29 | 78 | 221 | 5.2% | 0.72 |
| sx-askubuntu | insert-only | 455,691 | 12.2 | 193 | 643 | 3,006 | 7.7% | 0.00 |
| sx-superuser | 14-day expiry | 1,524,966 | 2.3 | 30 | 74 | 259 | 4.3% | 0.62 |
| sx-superuser | insert-only | 714,570 | 16.2 | 221 | 647 | 3,007 | 6.1% | 0.00 |
| wiki-talk-temporal | 14-day expiry | 6,644,984 | 4.6 | 64 | 162 | 2,073 | 5.6% | 0.63 |
| wiki-talk-temporal | insert-only | 2,787,967 | 31.2 | 561 | 1,403 | 22,358 | 6.3% | 0.00 |

Mean triangles per update were 0.19 to 1.47 on expiry streams and 1.03 to 7.47 on insert-only streams. The tail is light, and at most
7.7% of scan cost sits in the top 0.1% of updates. These streams do not contain the hub-to-hub toggle of the Θ(m) worst case in any
form that hurts. Wikipedia link streams were not run.

**M2. Sum-check audit, sanity run.** See candidate C1. Random mixed insert and delete graphs, n ≤ 40, 300 trials: honest runs accepted
300 of 300, a wrong claim rejected 300 of 300, a flipped edge in the prover's graph rejected 300 of 300. Weighted variant, C2, 200
trials: all five checks 200 of 200. Real states follow. R is wedge checks in one compact-forward recount on the same degree order.
"Phase 1" is the inner loop of the prover. Each iteration does up to 4 hash lookups and up to 4 modular multiplications.

| State | n | m | T | R | Phase 1 | Phase 1 / R | Accepted | Python prover |
|---|---|---|---|---|---|---|---|---|
| sx-mathoverflow, expiry, half stream | 24,759 | 1,394 | 190 | 3,053 | 72,486 | 23.7 | yes | under 0.1 s |
| sx-askubuntu, expiry, half stream | 157,222 | 3,365 | 135 | 3,704 | 140,955 | 38.1 | yes | 0.4 s |
| sx-superuser, expiry, half stream | 192,409 | 4,953 | 360 | 7,373 | 240,826 | 32.7 | yes | 0.5 s |
| wiki-talk-temporal, expiry, half stream | 1,094,018 | 71,217 | 6,702 | 196,651 | 4,696,493 | 23.9 | yes | 5.5 s |
| CollegeMsg, insert-only, end | 1,899 | 13,838 | 14,319 | 160,840 | 1,902,960 | 11.8 | yes | 0.5 s |
| email-Eu-core, insert-only, end | 986 | 16,064 | 105,461 | 332,407 | 2,699,928 | 8.1 | yes | 0.7 s |
| sx-mathoverflow, insert-only, end | 24,759 | 187,986 | 1,403,919 | 8,336,743 | 88,142,388 | 10.6 | yes | 21.5 s |
| sx-askubuntu, insert-only, end | 157,222 | 455,691 | 680,407 | 10,168,368 | 126,108,256 | 12.4 | yes | 35.1 s |

Verifier sketch in CPython: 1.0 to 1.8 µs per update, with full χ tables of n words. That table is a prototype convenience. See C1.

**M3. Known-expiry scanning, outside this remit.** See the note after the rejected ideas.

## Candidates, ranked

### C1. Count receipts: a recount-cost prover for an audit of the running count

**Claim.** A running exact triangle count can be audited at any moment against three field elements updated per stream edit, with a
prover that needs only the current adjacency and pays about log n times one static recount, instead of cost proportional to stream
length times n.

**Type.** Theory (one prover lemma) plus empirical (first measurements on real streams).

**Closest five.**

| Paper | Read | Overlap |
|---|---|---|
| Thaler, "Semi-Streaming Algorithms for Annotated Graph Streams," ICALP 2016, [arXiv:1407.3462](https://arxiv.org/abs/1407.3462) | Sections 1, 3.1, full proof of Theorem 3.1 | **Highest.** Telescoped form, turnstile, verifier O(1) per update, O(n) space, O(n) proof. Non-interactive, which C1 is not. Prover O(m·n) with m the stream length, stated as comparable to a naive count. Footnote 5 notes a five-message form with Õ(n) communication and Õ(log n) space, without detail |
| Abdullah, Daruki, Dutta Roy, Venkatasubramanian, "Streaming Verification of Graph Properties," ISAAC 2016, [arXiv:1602.08162](https://arxiv.org/abs/1602.08162) | Section 6 | **High.** Polylog cost streaming interactive proof for the count through an inverse frequency moment. Prover time not given. Warm-up only |
| Thaler, *Proofs, Arguments, and Zero-Knowledge*, [monograph](https://people.cs.georgetown.edu/jthaler/ProofsArgsAndZK.pdf) | Sections 4.3, 4.4 opening, 4.5.1, Exercise 4.2 | **High.** Same polynomial. Dense costs: prover O(n³) with care, or compute A² then O(n²). Exercise 4.2 leaves the fastest runtime for this exact protocol to the reader. It also warns that powers of a sparse adjacency matrix may be dense |
| Chakrabarti, Ghosh, Thaler, "Streaming Verification for Graph Problems," FSTTCS 2020, [arXiv:2007.03039](https://arxiv.org/abs/2007.03039) | Sections 1, 2 | **Medium.** Optimal [t, ns] tradeoff. Counts help and verifier space, not prover time |
| Kaski, "Engineering a Delegatable and Error-Tolerant Algorithm for Counting Small Subgraphs," ALENEX 2018, [PDF](https://acris.aalto.fi/ws/portalfiles/portal/55026313/1.9781611975055.16.pdf) | Abstract, introduction | **Medium.** The only engineering study found in this family: six-vertex patterns, up to 512 vertices, on GPU. Reports at least one order of magnitude of overhead for proof preparation |

Also read: Chakrabarti, Ghosh, APPROX 2019, [DROPS](https://drops.dagstuhl.de/entities/document/10.4230/LIPIcs.APPROX-RANDOM.2019.70), Section 3.
Chakrabarti, Cormode, Goyal, Thaler, "Annotations for Sparse Data Streams," [arXiv:1304.3816](https://arxiv.org/abs/1304.3816), abstract and
Theorem 6.2: scheme costs depend on m, prover time not given. Cormode, Mitzenmacher, Thaler, ITCS 2012,
[arXiv:1105.2003](https://arxiv.org/abs/1105.2003), results list: triangle protocol with O(n³ log n) prover. Björklund, Kaski, "How proofs
are prepared at Camelot," [arXiv:1602.01295](https://arxiv.org/abs/1602.01295), Theorems 3 to 5 and Section 6: sparse triangle proofs with
matrix multiplication exponents, proof cost in terms of n^ω. Cormode, Thaler, Yi, PVLDB 5(1), 2011, [arXiv:1109.6882](https://arxiv.org/abs/1109.6882),
searched text: implementations exist, the word "triangle" does not appear. Jauregui, Miyamoto, [arXiv:2605.14015](https://arxiv.org/abs/2605.14015),
triangle section: the same polynomial in a distributed zero-knowledge setting, no sparse prover cost. ZKGraph,
[arXiv:2507.00427](https://arxiv.org/abs/2507.00427), ICDE 2026 per OpenAlex, experiments sections: general graph queries through PLONKish
circuits, 21 to 383 s proving for operators on small tables, with zero knowledge, which C1 does not offer.

**Precise gap.** Every prover bound I read for exact triangle sum-check is dense (O(n³), O(n²) plus A²) or history-long (O(L·n)). I found
no bound in terms of m and degeneracy, and no measured run of a stream-verified triangle count on a real graph. "I found nothing" is weak
evidence here: this lemma may be folklore in the sum-check community, it may sit in Thaler's 2013 thesis, whose PDF came back empty, and it
may sit in cryptography venues I did not search. Check those before any write-up.

**Mechanism sketch.** Let Ẽ be the multilinear extension of the adjacency matrix over ℓ = ⌈log₂ n⌉ bits per vertex.

1. Verifier draws r_x, r_y, r_z before the stream and keeps them secret. It holds s₁ = Ẽ(r_x, r_y), s₂ = Ẽ(r_y, r_z), s₃ = Ẽ(r_z, r_x).
   These are linear in the edge set, so inserts and deletes commute and shards can add. An edit costs 6 modular multiplications given
   the χ values. Those take 6 more from split tables of O(√n) words, or O(ℓ) multiplications from the 3ℓ words of randomness alone.
2. At audit, sum-check on Ẽ(x,y)Ẽ(y,z)Ẽ(z,x) over 3ℓ variables, degree 2 in each, 9ℓ field elements in total. The claim is 6T.
   This is the protocol of the monograph's Section 4.3. The new part is the prover.
3. x rounds. Fold rows into a sparse table A(x̂, y). Round j needs B(a,b) = Σ_{y,z} A(a,y)E(y,z)A(b,z) for sibling rows a, b. Orient
   edges by degree. Each nonzero term comes from an oriented edge y→z with y or z in a row's support, so enumerating from y over N⁺(y)
   finds it, with hash lookups in the two rows.
4. y and z rounds. Only vectors remain: O(m + n) in total.
5. Verifier's last check is s₁·s₂·s₃ against the last round value. It never sees the graph.

**Conjecture 1, not proved.** The honest prover runs in expected O(ℓ · Σ_v d(v)·d⁺(v) + m + n) field operations from the current adjacency,
so O(ℓ·m^1.5) in the worst case and near ℓ times a recount on real graphs. Steps open: a written proof that folded supports keep each round
under Σ_v d(v)·d⁺(v); the expected cost of the hash tables; whether a degeneracy order gives O(ℓ·m·δ). Evidence: M2. Phase 1 stayed
below ℓ·Σ_v d(v)·d⁺(v) on every state run, and the ratio to R was 8.1 to 38.1.

**Conjecture 2, not proved, not tested.** A delta audit of one epoch can run in about ℓ times that epoch's scan work, by sum-check on
3tr(DE²) + 3tr(D²E) + tr(D³) with D the net change. It needs sketches of E and D at the same point, so each planned audit needs a sketch
carried from the start of the run.

**Limits to state up front.** The protocol is interactive, one round trip per variable, and r is revealed during an audit. A later audit against an
adversarial prover needs a fresh sketch, so K planned audits cost K sketches. Against non-adaptive faults a reused r may be enough, but that
needs a proof. Field must exceed 6T. p = 2⁶¹ − 1 is enough for these streams, with soundness error about 6ℓ/p.

**Why it escapes the Gate 0 failure cause.** Gate 0 failed because the mean scan is already 6 to 64 operations, so nothing is left to save. C1
leaves the update path alone and measures something else: verifier state and audit cost against a recount, on an output the baseline cannot
give. Cheap scanning helps it, since a scanner already holds the adjacency the prover needs.

**Strongest simple baseline.** Keep a set fingerprint of the edge stream, and have a trusted machine with O(m) memory read the graph, check the
fingerprint and recount once. This costs one R plus m. The prover costs roughly 8 to 38 times R in loop iterations, each heavier. So on CPU,
the baseline wins by two orders of magnitude or more (my estimate, unmeasured). C1 wins only when the auditor cannot hold the graph or spend R
of trusted compute. That is the delegation setting of the annotated-stream papers and of ZKGraph. **This is the main risk: a reviewer may ask
who needs it.** Secondary baselines: Thaler's scheme, about 3.5×10¹² field operations on the wiki-talk state in M2 against 4.7×10⁶ phase 1
iterations, though it is non-interactive with an O(n) proof; compacting n to active vertices would narrow that gap, and on a sparse window an
O(n) verifier state is no smaller than the graph, 1.09M vertices against 71,217 edges.

**Cheapest kill test, under an hour.** Port the scratch prover to C++ against `r11::Graph` from `src/r11/tc.hpp`, read-only, in a new benchmark file.
Run on the wiki-talk window, on sx-askubuntu insert-only, and on wiki-talk insert-only (m = 2.79M). Fix thresholds before running. Kill if prover
wall-clock exceeds 1,000× `static_count` on any state, or sketch time per update exceeds 3× mean scan time, or one honest run is rejected.
My prediction is 100× to 300×, unmeasured. A second check takes one hour of reading: Thaler's thesis Section 3.12 and any "sparse" prover in
Spartan-line papers. If either has the lemma, novelty drops by 3.

**Venue.** ALENEX or SEA for the lemma plus measurements. ACM JEA as the journal route. PVLDB only if a real delegation use case comes with it.

**Scores.** Novelty 7/10. Feasibility solo 8/10.

### C2. Certified local queries from the same sketch

**Claim.** The three words of C1 also certify any per-vertex count t(v) chosen at audit time, any weighted sum of them, and the whole vector, in
a single pass over an arbitrary stream.

**Type.** Theory (corollary) plus empirical. It would ship inside the C1 paper, and it is what keeps "local" in the title.

**Closest five.**

| Paper | Read | Overlap |
|---|---|---|
| Chakrabarti, Ghosh, APPROX 2019, [DROPS](https://drops.dagstuhl.de/entities/document/10.4230/LIPIcs.APPROX-RANDOM.2019.70) | Section 3, Appendix A.2 and A.3 | **Highest.** Corollary 11: RootedTriangleCount in a two-pass [t², s] scheme. Corollary 12: one pass only if the edges at v_r arrive first. So v_r is fixed early. C2 fixes it at audit time, in an interactive model |
| Thaler, ICALP 2016, [arXiv:1407.3462](https://arxiv.org/abs/1407.3462) | Section 3.1 | **High.** Weights fit the same sum. My observation: his g(z) on [n] is the signed count of triangles whose last edit had apex z, so a scanning counter gets those n values for free. That covers half the proof, and the other half remains the cost |
| Kara, Nikolic, Ngo, Olteanu, Zhang, "Maintaining Triangle Queries under Updates," TODS 2020, [arXiv:2004.03716](https://arxiv.org/abs/2004.03716) | Sections 1 to 2, Theorem 3, rebalancing section | **Medium.** Unary and binary triangle queries with free variables, amortized. Maintains local answers, certifies nothing |
| Thaler monograph, [PDF](https://people.cs.georgetown.edu/jthaler/ProofsArgsAndZK.pdf) | Sections 4.3, 4.5 | **Medium.** Reducing several evaluations to one covers related needs, but not local counts |
| McConnell, Mehlhorn, Näher, Schweitzer, "Certifying Algorithms," Computer Science Review 2011, [preprint](https://people.mpi-inf.mpg.de/~mehlhorn/ftp/master-final.pdf) | Searched text, definitions | **Low.** Framework. The word triangle appears only in geometric senses, no counting certificate |

**Precise gap.** No source read certifies a local triangle count for a vertex named only at audit time, in one pass over an unrestricted stream.
Theory readers may call this a one-line change, so its value is the working system and the measurements, not the proof.

**Mechanism sketch.** Sum Σ_{x,y,z} w(x)Ẽ(x,y)Ẽ(y,z)Ẽ(z,x) for a low-degree w. This equals Σ_x w(x)(A³)_xx = 2Σ_x w(x)t(x). With w = eq(v,·) the
answer is t(v). With w = eq(ρ,·), ρ drawn after the prover commits to a vector t, the verifier computes the multilinear extension of t at ρ in
O(n) and so checks every entry. Rounds on x get degree 3. The final check multiplies by w̃(r_x), which the verifier computes alone. Per-edge
support follows with eq on (x, y), and from a certified vector the top-k by clustering is certified too.

**Conjecture 3, not proved.** Same sketch, degree 3 in x variables, soundness error about 7ℓ/p, prover no worse than C1. Steps open: a written
soundness proof, including that ρ is independent of r; the per-edge case, not implemented. Evidence: M2, 200 of 200 for true point query, off-by-one
rejected, flipped edge rejected, true vector, one-entry-wrong vector rejected.

**Why it escapes Gate 0.** Same reason as C1. The output differs: a local value with a proof.

**Strongest simple baseline.** For a vertex known in advance, the verifier keeps N(v) and counts edges inside it, exactly and with no prover, at memory
up to min(d(v)², m). C2 only beats this when v is chosen at audit time or the watch list is large.

**Cheapest kill test.** Extend the C1 port with weights. On the wiki-talk window, run 100 point queries, sampled by degree. Kill if the proof cost for a
point query exceeds 2× the global audit, or if one honest run fails. Then read Corollary 12 again for a one-pass route that covers audit-time vertices.

**Venue.** Same paper as C1.

**Scores.** Novelty 6/10. Feasibility solo 7/10. Depends on C1.

### C3. Budgeted exact counting with a certified interval

**Claim.** Bound work per update to B, report a count interval that always contains the truth and shrinks to a point when the backlog of parked
hub toggles drains.

**Type.** Method, with a small theory statement.

**Closest five.**

| Paper | Read | Overlap |
|---|---|---|
| Hanauer, Heck, Henzinger, Sidl, [arXiv:2610.29776](https://arxiv.org/abs/2610.29776) | Main text, Tables 4.1 and A.2, Figures 5.2, A.4, A.5, Appendix A.16 | **High.** `late` limits partition moves per update. Worst-case bound for direct scanning stays O(m) in Table 4.1. Exact at every update, so no interval |
| Kara et al., TODS 2020, [arXiv:2004.03716](https://arxiv.org/abs/2004.03716) | Sections 1 to 2, rebalancing | **High.** OMv lower bound. Rebalancing is amortized by design, so bounded work with exactness at each step needs a change of guarantee |
| Ghanem, Hammad, Mokbel, Aref, Elmagarmid, TKDE 2007, [record](https://docs.lib.purdue.edu/ccpubs/241) | Abstract | **Medium.** Input-triggered against negative tuples for window queries, with output delay as the trade-off |
| Abo-Khamis, Chmielewski, Draghici, Kara, Olteanu, [arXiv:2605.08397](https://arxiv.org/abs/2605.08397) | Abstract, introduction | **Low.** General heavy-light IVM, update bounds, no budget or interval |
| Zhang, Bonifati, Özsu, "Low-Latency Sliding Window Connectivity," [arXiv:2410.00884](https://arxiv.org/abs/2410.00884) | Abstract | **Low.** Latency aim on another problem |

**Precise gap.** I found no exact counter that trades per-update exactness for a certified interval. Searched web terms: deferred, bounded staleness,
anytime, lag, interval. Weak evidence: "bounded staleness" work in streaming systems was not read in depth.

**Mechanism sketch.** If min(d_u, d_v) > B, park the toggle: keep the edge invisible to other updates and widen the interval by that bound. Spend up to B
per update advancing the oldest parked scan. A later update touching u or v adjusts the partial result in O(1) per parked toggle. Exactness then comes from
telescoping on the reordered stream, with parked toggles placed after the updates that overtook them.

**Conjecture 4, not proved.** Work per update is O(B + q) with q parked toggles. The interval width is at most the sum of parked bounds, and the count is exact
when q = 0. Steps open: the exactness proof under reordering; a cursor rule for lists that change during a scan; bounding q, which cannot be small
on adversarial input given the OMv bound.

**Why it escapes Gate 0.** It targets the maximum. **Only partly**: M1 shows the maximum is small on our streams too, so the argument has to rest on adversarial
input or on Wikipedia-type streams. Hanauer et al. also report that basic versions occasionally exceed a static recomputation on one update. They link that to auxiliary map
maintenance and major rebalancing, which scanning does not have.

**Strongest simple baseline.** Plain scanning with a hash table that resizes incrementally. Hanauer et al. name hash map capacity increases as a driver of peaks,
and the flat 7 ms line across partitions in Figure A.5(a) suggests a floor unrelated to counting. That is my inference from a plot. `FlatMap::rehash` in
`tc.hpp` stops the world in the same way.

**Cheapest kill test.** Time each update with `tc.hpp` on wiki-talk insert-only. Classify the 100 slowest as rehash, vector growth or scan. Then build one synthetic
K(2,n) toggle stream as the positive control. Kill if on all six streams fewer than 10 of the 100 slowest are scans. I expect that outcome.

**Venue.** GRADES-NDA or a section of another paper.

**Scores.** Novelty 5/10. Feasibility solo 6/10.

## Rejected ideas

| # | Idea | Killed by | Why |
|---|---|---|---|
| R1 | Exact per-vertex and per-edge counts as a contribution | Kara et al., [arXiv:2004.03716](https://arxiv.org/abs/2004.03716), Theorem 3, read | Unary and binary queries already maintained. Scanning gets both at scan cost plus O(s), with mean s 0.19 to 7.47 in M1. So a support update is not much dearer than a count here, and the premise of the brief does not hold on these streams |
| R2 | Top-k by local clustering or support | Same theorem, plus M1 | Only u, v and common neighbours change, so s + 2 heap operations per update. I could not open the one close paper, a weighted variant in World Wide Web 2017, so I do not rely on it. Own reasoning, not a published kill |
| R3 | Bounded truss maintenance | Couto, Fernandes, WAOA 2025, [arXiv:2509.13584](https://arxiv.org/abs/2509.13584), abstract page; Zhang, Yu, SIGMOD 2019, [record](https://dbgroup.se.cuhk.edu.hk/node/177), metadata, and a [summary](https://rmarcus.info/dbscholar/papers/hafe0fbb3504f7c95) that is machine-generated | Under OMv no bounded algorithm exists, and under SETH Ω(m^(1−ε)) per update or query. Zhang and Yu are reported to show insertions unbounded without an AFF measure |
| R4 | Batch truss maintenance | Luo et al., [TCSS 2020](https://doi.org/10.1109/tcss.2020.3026574); Luo et al., [IEEE TC 2022](https://doi.org/10.1109/tc.2022.3174594); Sun et al., [SIGMOD 2023](https://doi.org/10.1145/3589278); Bai, Wang, Zheng, Dong, Wu, [IEEE TBD 2026](https://doi.org/10.1109/tbdata.2026.3702362). All abstract only, through OpenAlex. Also read: Huang et al., SIGMOD 2014, [PDF](https://cs.ubc.ca/~xin0/paper/SIGMOD14.pdf), Section 5 and results; Zhou et al., [arXiv:1402.2807](https://arxiv.org/abs/1402.2807), algorithms and complexity | Batch, mixed vertex and edge, star, and parallel with index are all covered. Luo reports large gains over single-edge work on static graphs with big batches. On our windows the mean update closes under 1.5 triangles, so most edges have low trussness and I expect the Gate 0 cause to repeat. **Weak spot:** no truss test was run, IEEE full texts were not read, and exact truss under sliding-window expiry came up empty in one search |
| R5 | Hub-pair cache by ski rental, with a competitive ratio | Schnaitter, Polyzotis, "Semi-Automatic Index Tuning," [arXiv:1004.1249](https://arxiv.org/abs/1004.1249), Theorems 4.1 to 4.3, read; M1 | Ratio 2^(c_max+1) − 1 with non-interacting parts, and they handle a non-symmetric transition cost. Pairs do not interact, so ratio 3 per pair should follow. A fixed candidate set and fixed build cost are needed, so a small adaptation note may remain, but that is a remark and not a paper. M1: insert-only streams have no repeated pairs at all, since set semantics forbids them. So a reactive cache serves nothing there |
| R6 | De-amortised heavy-light for triangles | Hanauer et al., [arXiv:2610.29776](https://arxiv.org/abs/2610.29776), Lemma 4.6, Table 4.1, Section 5.2.3 | `late` improves worst-case update by factors in [√m, m], at a preserved amortized bound. For triangles their advice is still no partition |
| R7 | Freivalds on trace(A³) | Thaler monograph, [PDF](https://people.cs.georgetown.edu/jthaler/ProofsArgsAndZK.pdf), Section 4.4 opening; Bennett et al., RANDOM 2024, [arXiv:2309.16176](https://arxiv.org/abs/2309.16176), abstract | Freivalds checks AB = C and so needs C. Sending A² takes Ω(n²), and it may be dense for sparse A. A random quadratic form estimates a trace and does not fix it exactly. Incremental form is not the issue. The missing C is |
| R8 | Per-update certificate of one set intersection | Own reasoning | A disjointness proof for N(u) and N(v) is as large as the smaller list, so checking one update costs what the scan costs. Aggregating across updates is why Thaler's sum exists |
| R9 | Generic proof systems around the scanner | ZKGraph, [arXiv:2507.00427](https://arxiv.org/abs/2507.00427), experiments sections | 21 to 383 s to prove expansion operators in its experiments, with scale tests at 60k to 180k rows. Useful for zero knowledge, heavy for a plain audit. Not a like-for-like comparison |
| R10 | Formal proof of the maintainer, instead of a run-time certificate | Bonifati et al., TPLP 2018, [arXiv:1804.10565](https://arxiv.org/abs/1804.10565), abstract | Coq route for graph views exists. It proves code, not a run, so it cannot catch memory faults or a lost update. Different goal, no gap found |
| R11 | Predictions or lookahead to avoid OMv | van den Brand, Forster, Nazari, Polak, SODA 2024, [arXiv:2307.09961](https://arxiv.org/abs/2307.09961), Theorem 1.4, Section 5.1 | Triangle detection with predicted vertex updates through fast matrix multiplication, about n^(ω−1) per update. Dense, far above 6 to 64 operations |

## Outside the remit, flagged for the PI

**M3. Known-expiry scanning.** On a window stream an edge's next expiry is known when it is inserted, so a triangle's death time is the minimum of three known
times. Scan on insertion, write that time to a calendar, and a deletion needs no intersection. Exact on all six streams: 0 mismatches over 16.1M checks, one at
each timestamp change. Scratch cost ratio against plain scanning: 0.662, 0.912, 0.749, 0.749, 0.740, 0.668, charging 3 per triangle and 1 per update. This is a
mean result, so it is not in my direction, and these units differ from Gate 0's. It is also not new as an idea: input-triggered expiry is classic in window
queries (Ghanem et al., [TKDE 2007](https://docs.lib.purdue.edu/ccpubs/241), abstract). **Consequence:** on streams labelled "synthetic expiry," plain scanning is no
longer the strongest baseline, and any mean claim on those streams must beat this one. All of Hanauer et al.'s TMP group has this structure, by their description.

## What was searched, and limits

- **OpenAlex API:** about 23 title and abstract queries on certifying, verifiable, Freivalds, annotated streams, Merlin-Arthur, authenticated structures, truss maintenance,
  top-k clustering. **It then returned HTTP 429 for the rest of the session.** So the queries on tail latency, deamortization, competitive ratio, online view selection and
  ski rental did not run there, and those topics rest on web search alone. Part (b) and (c) gap statements are the weakest in this file.
- **arXiv API:** four queries before 429, later seven more with pacing. `abs:"triangle" AND abs:"interactive proof"` never completed. `abs:"verifiable" AND abs:"dynamic graph"`
  returned nothing relevant. `abs:"sum-check" AND abs:"sparse" AND abs:"prover"` returned nothing, and that is weak evidence.
- **Web search:** verifiable and interactive triangle counting, incremental certificates, competitive index admission, sum-check sparse prover, verifiable graph databases and join
  proofs, worst-case dynamic triangle counting, predictions, truss maintenance, Bai et al., top-k clustering, exact sliding-window counting, bounded staleness.
- **Not reached.** DBLP, by rule. Cryptography venues such as ePrint, not searched: a sparse sum-check prover could well sit there. Thaler's 2013 thesis: the download was empty, so unread.
  Bruno and Chaudhuri, ICDE 2007: not opened, reported through Schnaitter and Polyzotis. Borodin, Linial, Saks: PDF refused the connection, ratio 2σ − 1 taken from
  Schnaitter and Polyzotis. IEEE full texts in R4: abstract only. A weighted clustering paper in World Wide Web 2017 was behind a login and is not used as evidence.
- **Scope of the data.** Six SNAP streams, two with rules this project chose. Nothing here speaks for Wikipedia link streams, Kronecker graphs or adversarial input.
- **Citations.** Details come from the linked source or OpenAlex record. They have not been turned into a checked `.bib`. Venue of ZKGraph comes from OpenAlex. Web search could not confirm it.
