# M1B02: count receipts, kill test

Date: 2026-11-01. Rule: `docs/experiments/screen-b-protocol.md`, M1B02, unchanged. Candidate: `docs/literature/scouts/s5-local-and-guarantees.md`, C1.
Code: `benchmarks/receipts.cpp`. Non-citable screen, like all of Screen B. Timing is indicative: the tower claim is shared.

All streams here are labelled **real timestamps**. The wiki-talk window has a **synthetic 14-day expiry rule**. The K(s,t) run is **fully synthetic**.

## 1. Verdict

**Go, by the written rule, on one reading the PI should confirm. It is a weak Go.**

| Clause of the rule | Result | Status |
|---|---|---|
| Stop: any honest run rejected | 414 of 414 accepted on real states, on a feed of applied edits | Pass, **see the first caveat** |
| Stop: wrong claim or flipped edge accepted | 0 of 3,156 attacks on real states at p = 2^61 − 1 | Pass |
| Stop: prover above 1,000 × `static_count` | Worst single run 23.3 ×. Medians 1.9, 9.6, 18.1 × | Pass |
| Stop: sketch above 3 × mean scan | Split-table sketch at most 0.28 × | Pass, **see the second caveat** |
| Stop: sparse prover lemma found published | Not found in what could be read. Thaler's thesis stayed unread | Pass, **see the third caveat** |
| Go: all checks pass, and a line-by-line proof of the prover bound with no open step | Written in section 3. No open step in the prover bound | Met |

Three things the rule does not price, and each could change the call.

1. **The verifier cannot read the raw stream.** 37% to 54% of raw events are no-ops under set semantics. A graph-blind sketch fed the raw file holds multiplicities, not the graph, and rejected the honest prover on 3 of 3 states. The accepted runs use a feed of applied edits from a separate source that keeps the whole pair set, at least 2m words (5.6M words on wiki-talk). I scored "honest run" as a run on a well-formed feed, because C1 says the sketch updates per stream *edit* and S5's own table counts applied updates. **If "honest run" means the raw event file against a verifier with no pair set, the verdict is Stop.** That is the PI's call.
2. **Only the split-table sketch passes the sketch clause.** It uses 9,284 words at ℓ = 21. The pure form, which is the claim's "few words" reading at 131 words, costs 4.5 × scan on the wiki-talk window and fails that clause there. C1 lists both forms, so I scored the split form.
3. **Go here is not a claim of novelty.** The bound took a page and its key counting step is two lines. I expect a sum-check specialist to call it folklore, and "not found" rests on a partial read (section 2).

## 2. Novelty

**Finding: I did not find the lemma.** No text I could read gives a prover for the triangle sum-check in terms of m, degeneracy or wedges. So there is no early stop. This is weak evidence.

| Source | How far | What it has |
|---|---|---|
| Thaler, *Proofs, Arguments, and Zero-Knowledge*, version of 18 July 2023, [PDF](https://people.cs.georgetown.edu/jthaler/ProofsArgsAndZK.pdf) | Read 4.3, 4.4 (Lemmas 4.3 to 4.5), 4.5.1, Exercises 4.1, 4.2. All 28 lines with "sparse" | Dense only: O(n³), or T(A²) + O(n²). Warns that powers of a sparse matrix may be dense. Sparse hits are wiring predicates, R1CS and sparse commitments |
| Thaler, "Time-Optimal Interactive Proofs for Circuit Evaluation," CRYPTO 2013, [arXiv:1304.3812](https://arxiv.org/abs/1304.3812) | Read 3.4, 8.2 opening, Theorem 3. Searched full text | MATMULT prover T(n) + O(n²). "Triangle" never appears. "Sparse" once, for wiring |
| Thaler, [matrix multiplication notes](https://people.cs.georgetown.edu/jthaler/OptimalMatMult.pdf) | Read the opening | Same protocol, same warning on dense powers |
| Setty, Thaler, Wahby, Lasso, [author PDF](https://people.cs.georgetown.edu/jthaler/Lasso-paper.pdf) | Read F.1, Theorem 6. Searched the rest | Sparse-dense sum-check for one inner product ⟨u, t⟩ with u sparse and t structured. Not a product of three matrices. It credits CMT12 for O(m polylog N) |
| Arun, Setty, Thaler, Jolt, [author PDF](https://people.cs.georgetown.edu/jthaler/Jolt-paper.pdf) | Searched text | Reuses the above. No triangle, no matrix product |
| ZK-Value, [arXiv:2605.03581](https://arxiv.org/abs/2605.03581) | Read the sparse path section | Support masks on factors over one variable set. Skips zero cells. No join between factors |
| Jauregui, Miyamoto, [arXiv:2605.14015](https://arxiv.org/abs/2605.14015) | Read the triangle section | Same polynomial, no sparse prover cost |
| ZKSQL, [PVLDB 16](https://www.vldb.org/pvldb/vol16/p1804-li.pdf) | Searched text | No sum-check |
| PoneglyphDB, [arXiv:2411.15031](https://arxiv.org/abs/2411.15031) | Searched text | No sum-check |
| Spartan [2019/550](https://eprint.iacr.org/2019/550), Libra [2019/317](https://eprint.iacr.org/2019/317), zk-vSQL [2017/1146](https://eprint.iacr.org/2017/1146), SpaGKR [2024/1018](https://eprint.iacr.org/2024/1018), matrix zkSNARKs [2025/1646](https://eprint.iacr.org/2025/1646) | **Abstract only**, plus the monograph's account of Spartan and Libra | Matrix-vector and circuit provers. Nothing in the abstracts on a sparse matrix-matrix trace. SpaGKR's body is unread |
| Thaler, "Sum-check Is All You Need," [2025/2041](https://eprint.iacr.org/2025/2041) | **Abstract and a search snippet only** | Lists "sparse sum-checks". A prefix-suffix prover, linear in sparsity when nonzero terms can be enumerated. **This is the most likely home for a related statement, and I could not read it** |
| Goldreich, Rothblum, "Counting t-cliques," FOCS 2018, [ECCC TR18-046](https://eccc.weizmann.ac.il/report/2018/046) | **Search summary only**. Download failed | Õ(n²) prover given an oracle for (t−1)-clique counts. Dense |

**Not read, stated plainly.**
- **Thaler's 2013 thesis, Section 3.12: still unread.** Two attempts here on two DASH URLs each returned 0 bytes, as S5's did. The handle page loads, the file does not. The gap S5 flagged stays open.
- **ePrint full text: blocked.** Direct download hit a Cloudflare challenge, and WebFetch on PDFs returned 403. I did not bypass either. Abstract pages and site search worked.
- vSQL's S&P paper, Twist and Shout, and Speeding Up Sum-Check Proving were not read. DBLP was not touched. OpenAlex was not used.

**Searches.** Twelve web searches, on sparse triangle sum-check, sparse matmul, proofs for joins and FAQ, tensor contraction and graph analytics, plus locating the sources above. Six arXiv API queries: only two sparse-Merkle GKR papers came back, off topic. Six ePrint site searches: few hits, the survey above the only relevant one. That search returned nothing at all for "triangle counting sum-check", which is weak evidence.

**Two corrections to S5.**
- Exercise 4.2 of the monograph is **not** this protocol. It applies sum-check to the multilinear extension of the triangle indicator, which has soundness (3 log n)/p and a final evaluation the verifier cannot do cheaply. The published dense bound for C1's polynomial is in 4.3 and 4.5.1.
- My reading: 4.3 and 4.5.1 are the same polynomial with a different variable order, since Σ_z Ã(x,z)Ã(z,y) is multilinear in x and y and so equals the extension of A². The new part is only that the prover need not build A², which S5 already suspected.

## 3. Proof

Author-checked and mechanically corroborated (section 4), **not independently reviewed**.

**Notation.** Vertices V ⊆ {0,1}^ℓ, ℓ = ⌈log₂ n⌉. E(u,v) ∈ {0,1} is symmetric with E(v,v) = 0, and the graph has m edges. Ẽ is the multilinear extension over F_p, and χ_a(u) = Π_j (u_j a_j + (1−u_j)(1−a_j)). Variables bind bit 0 first. Fix **any** orientation, giving each edge one tail, with out-lists N⁺(v) of size d⁺(v). Put D = Σ_v d(v)·d⁺(v) and g(x,y,z) = Ẽ(x,y)·Ẽ(y,z)·Ẽ(z,x).

**Fact 1.** Σ over the cube of g is tr(A³) = 6T.
**Fact 2.** Ẽ(a,b) = Σ_{E(u,v)=1} χ_a(u)χ_b(v).
**Fact 3.** Ẽ(a,b) = Ẽ(b,a), by Fact 2 and symmetry of E.
**Fact 4.** g has degree at most 2 in each variable and total degree at most 6ℓ.

**Lemma 1 (rows).** Take challenges ρ = (r_1..r_i). For b ∈ {0,1}^{ℓ−i} let B_i(b) be the vertices whose high ℓ−i bits equal b, and R_i(b,y) = Ẽ((ρ,b),y). Then:
(a) supp R_i(b) ⊆ N(B_i(b)).
(b) Σ_b |N(B_i(b))| ≤ 2m.
(c) Each y lies in at most min(d(y), 2^{ℓ−i}) of the sets N(B_i(b)).

*Proof.* By Fact 2, with trailing coordinates Boolean, R_i(b,y) = Σ_{x∈B_i(b)} χ_ρ(x_low)·E(x,y), which gives (a). The sets B_i(b) partition V and |N(B)| ≤ Σ_{x∈B} d(x), which gives (b). If y ∈ N(B_i(b)) then B_i(b) holds a neighbour of y. The sets are disjoint, so there are at most d(y) of them, and there are 2^{ℓ−i} values of b in all, which gives (c). ∎

**Lemma 2 (round form).** In x round i+1, for b' ∈ {0,1}^{ℓ−i−1} let a = R_i((0,b'),·) and a' = R_i((1,b'),·). Define ⟨u,v⟩ = Σ_{E(y,z)=1} u(y)v(z). The honest polynomial is

s(X) = Σ_{b'} [(1−X)²⟨a,a⟩ + 2X(1−X)⟨a,a'⟩ + X²⟨a',a'⟩].

*Proof.* s(X) = Σ_{b'} Σ_{y,z} g((ρ,X,b'),y,z). On Boolean y, z, Ẽ(y,z) = E(y,z), and Ẽ(z,x) = Ẽ(x,z) by Fact 3. Since Ẽ has degree 1 in X, Ẽ((ρ,X,b'),y) = (1−X)a(y) + Xa'(y). Expand, and use ⟨a,a'⟩ = ⟨a',a⟩ by symmetry of E. ∎

**Lemma 3 (oriented sum).** ⟨u,v⟩ = Σ_y Σ_{z∈N⁺(y)} (u(y)v(z) + u(z)v(y)), and the outer sum may be restricted to any S ⊇ supp u ∪ supp v.

*Proof.* Each edge is one (tail, head) pair and two ordered pairs. If y ∉ S then u(y) = v(y) = 0 and both summands vanish. ∎

So, with fa = Σ_{z∈N⁺(y)} a(z) and fb = Σ_{z∈N⁺(y)} a'(z), each y ∈ S adds 2a(y)fa to ⟨a,a⟩, 2a'(y)fb to ⟨a',a'⟩, and a(y)fb + a'(y)fa to ⟨a,a'⟩. That is at most 4 multiplications per y, however long N⁺(y) is.

**Theorem 1 (prover).** From adjacency lists and any orientation, the honest prover produces all 3ℓ messages using, in the worst case, deterministically:
- at most Σ_y d⁺(y) Σ_{k<ℓ} min(d(y), 2^k) ≤ ℓ·D list-element reads, each with two field additions,
- at most 12ℓm + 16·2^ℓ + 8ℓ field multiplications,
- O(ℓm + 2^ℓ) other word operations.

Since m ≤ D and 2^ℓ < 2n, the total is **O(ℓ·D + n) field operations, worst case.** This proves Conjecture 1's bound and removes "expected".

*Proof.*
1. *Setup.* A degree order needs no sort: compare (d(v),v) with (d(w),w) per edge. Out-lists and initial rows R_0 cost O(n+m).
2. *Access.* Keep one dense array of pairs (a(v), a'(v)), all zero between parents. Loading and clearing cost O(|supp a| + |supp a'|). So every read is O(1) in the worst case, and no hash table is used.
3. *x round, message.* Let S(b') = supp a ∪ supp a'. Then S(b') ⊆ N(B_{i+1}(b')), because B_i((0,b')) ∪ B_i((1,b')) = B_{i+1}(b'). By Lemma 1(c) at level i+1, y lies in at most min(d(y), 2^{ℓ−i−1}) sets S(b'). So reads in the round are at most Σ_y d⁺(y)·min(d(y), 2^{ℓ−i−1}). Summing over i gives the first bound with k = ℓ−1−i, and each term is at most d(y), so it is at most ℓ·D. By Lemma 1(b), Σ_{b'} |S(b')| ≤ 2m, so 4 per y gives at most 8m multiplications, plus 2 to form s(2) = ⟨a,a⟩ − 4⟨a,a'⟩ + 4⟨a',a'⟩.
4. *x round, fold.* R_{i+1}(b',y) = (1−r)a(y) + r·a'(y), since Ẽ has degree 1 in that variable. That is at most 2 multiplications per y, at most 4m per round. So phase 1 uses at most 12ℓm + 2ℓ.
5. *y rounds.* After round ℓ one row remains, a = Ẽ(r_x,·). Let h(y) = Σ_{z∈N(y)} a(z), at O(m + 2^ℓ) additions. For y ∈ F^ℓ, Σ_z g(r_x,y,z) = Ẽ(r_x,y)·Σ_z Ẽ(y,z)Ẽ(z,r_x). The inner sum is multilinear in y and equals h on the cube, using Fact 3. So it is h̃, and the phase is sum-check on a product of two multilinear tables. Round j has 2^{ℓ−j} index pairs, at 3 multiplications to compute and 4 to fold, as in the monograph's Lemma 4.5. Total at most 7·2^ℓ + 3ℓ.
6. *z rounds.* Let c* = Ẽ(r_x,r_y), the last entry of the folded table. The polynomial is c*·b̃(z)·ã(z), with b(z) = Ẽ(r_y,z) = Σ_{y∈N(z)} χ_{r_y}(y). The χ table costs 2·2^ℓ multiplications from the prover's own copy of r_y, and b costs O(m + 2^ℓ) additions. The rounds then cost as in step 5. Total at most 9·2^ℓ + 3ℓ.
7. *Total.* 12ℓm + 2ℓ + 7·2^ℓ + 3ℓ + 9·2^ℓ + 3ℓ = 12ℓm + 16·2^ℓ + 8ℓ. Each edge has a tail of degree at least 1, so m ≤ D. ∎

**Corollary 1.**
(a) Degree order: D ≤ 3√2·m^1.5. With τ = √(2m): vertices of degree at most τ contribute at most τ·Σd⁺ = τm. A vertex of degree above τ has out-neighbours of degree above τ, of which fewer than τ exist, so it contributes less than τ·d(v), at most 2τm in total.
(b) Degeneracy order: d⁺ ≤ δ, so D ≤ 2mδ, giving O(ℓmδ + n). This answers S5's open question. The order costs O(m+n), and Theorem 1 needs no property of it.
(c) D = R + Σ_v d⁺(v)², where R = Σ_z d⁻(z)d⁺(z) is the wedge count of one recount.

**What Theorem 1 does not say.** "Near one recount" is not a theorem. On K(s,t) with s < t and the degree order, R = 0 and D = t·s². A prover at ℓ·D on such a graph costs far more than a recount. See the synthetic row in section 4.

**Sketch lemma.** Under A1 below, the verifier's words equal Ẽ(r_x,r_y), Ẽ(r_y,r_z), Ẽ(r_z,r_x) at audit time, by Fact 2 and linearity. Each toggle adds ±(χ_a(u)χ_b(v) + χ_a(v)χ_b(u)).

**Assumptions.**
- **A1.** Every feed record is a true toggle of its pair.
- **A2.** r is independent of the stream, and of the prover's view before each round. It is used for one audit.
- **A3.** T < p. Here T ≤ C(n,3) < 2^61 − 1 for n < 2.4M. S5's "field must exceed 6T" is more than is needed, since 6 is invertible.

**Theorem 2 (soundness).** Under A1 to A3:
(a) With an honest prover on the true graph and claim T, the verifier accepts with probability 1.
(b) For any prover strategy and any claim T' ≠ T with T' < p, the verifier accepts with probability at most 6ℓ/p.
(c) An honest prover on any G' ≠ G, claiming T(G'), is accepted with probability at most 6ℓ/p, **even if T(G') = T(G)**.

*Proof of (a).* The true round polynomials pass every check, and the last value is g(r), which equals the three words by the sketch lemma.

*Proof of (b).* A randomised prover is an average of deterministic ones, so fix one. Let s_i be the true polynomial of round i and m_i the message, a function of r_1..r_{i−1} only. V rejects unless m_i(0) + m_i(1) equals the running value. If the running value is wrong at round i and V does not reject, then m_i ≠ s_i. If also m_i(r_i) ≠ s_i(r_i), the running value is wrong at round i+1. At the end a wrong value is rejected, because the three words give the true g(r). So acceptance needs some i with m_i ≠ s_i and m_i(r_i) = s_i(r_i). Both have degree at most 2 and r_i is uniform and independent of them, so each round has probability at most 2/p. The union over 3ℓ rounds gives 6ℓ/p. The claim starts wrong since 6 is invertible and T ≠ T' mod p by A3. ∎

*Proof of (c).* First, g_G ≠ g_{G'} as polynomials. Take (u,v) with E(u,v) = 1 and E'(u,v) = 0. Then g_{G'}(u,v,z) = 0 for all z, while g_G(u,v,z) = Ẽ(v,z)·Ẽ(z,u). Both factors are nonzero, the first being 1 at z = u and the second 1 at z = v, and F[z] is a domain. All rounds pass, so V compares g_{G'}(r) with g_G(r), and these agree with probability at most 6ℓ/p by Schwartz-Zippel and Fact 4. ∎

**Verifier cost.** 12 multiplications per edit with split tables, or 6ℓ + 6 in pure form. An audit takes 3ℓ rounds at O(1) each. The prover sends 9ℓ field elements, 1,512 bytes at ℓ = 21.

### Named gaps

- **G1, the feed (not in the prover bound, but it is the main result).** Theorem 2 needs A1. Under set semantics a verifier cannot tell a no-op from an edit without knowing whether the pair is present. That is a membership question, and by the standard INDEX argument it needs state of the order of the distinct pairs, unless a prover helps. I did not write that reduction out. If the prover labels the no-ops, a dropped edit is simply labelled a no-op, and the audit accepts (measured: 3 of 3). A repair might use annotated edits and offline memory checking. I have no design, no proof and no cost for that, and it would not stay at a few words of communication without new work.
- **G2, reuse of r.** A later audit against an adaptive prover needs a fresh sketch. K audits cost K sketches. No proof exists for reusing r.
- **G3, no lower bound.** Nothing here says ℓ·D is needed.
- **G4, unit cost.** Field operations are counted as unit cost on a word RAM.

## 4. Measurement

Compiled with g++ 16.2.0, `-std=c++20 -O2 -Wall -Wextra -static`, no warnings. One process at a time, each under ten minutes (longest 7 m 59 s). The verifier code precedes `#include tc.hpp`, so naming `Graph` there is a compile error. The prover gets each r only as the return value of `Audit::receive`. S5's prototype broke the graph rule at the sketch: `real()` filtered no-ops by reading adjacency, and its prover read the verifier's χ table. Neither carried over.

| State | n | ℓ | m | T | Raw events | No-op share |
|---|---|---|---|---|---|---|
| wiki-talk, window, half stream | 1,094,018 | 21 | 71,217 | 6,702 | 6,100,538 | 46.8% |
| sx-askubuntu, insert-only, end | 157,222 | 18 | 455,691 | 680,407 | 726,661 | 37.3% |
| wiki-talk, insert-only, end | 1,094,018 | 21 | 2,787,967 | 8,113,676 | 6,100,538 | 54.3% |

The running scan count equals `static_count` on all three.

**Prover against one recount.**

| State | Seeds | `static_count` s | Prover s, median (max) | Ratio, median (max) | D | R | D/R | Iterations | Iterations / R |
|---|---|---|---|---|---|---|---|---|---|
| wiki-talk window | 300 | 0.0784 | 0.151 (0.277) | **1.9** (3.5) | 373,206 | 196,593 | 1.90 | 5,474,682 | 27.8 |
| sx-askubuntu | 100 | 0.0426 | 0.411 (0.992) | **9.6** (23.3) | 14,541,515 | 10,168,082 | 1.43 | 126,118,442 | 12.4 |
| wiki-talk insert-only | 14 | 0.419 | 7.59 (8.59) | **18.1** (20.5) | 204,774,739 | 145,908,916 | 1.40 | 1,946,093,252 | 13.3 |
| K(1000,5000), **fully synthetic, not a protocol state** | 1 | 0.0189 | 7.39 | **392** | 5×10⁹ | 0 | none | 1.48×10¹⁰ | none |

S5 predicted 100 to 300 ×. The real figure is lower because the inner loop does additions only, not 4 hash lookups and 4 multiplications. The window ratio is low most likely because `static_count` is dominated there by sorting 1.09M vertices, with only 71,217 edges to count; I did not profile it. The synthetic row confirms the warning in section 3. By the formula its ratio grows with s, so a graph built for it should cross the 1,000 × line. I did not run such a graph.

Theorem 1 held on every graph: iterations were 72% to 82% of the refined bound, and multiplications 59% to 87% of the bound. It also held with no violations on all 5,000 random graphs.

**Sketch against scan.** Per applied update, in ns.

| State | Scan only | Scan + edit | Split sketch (ratio to scan) | Pure sketch (ratio to scan) | Source filter |
|---|---|---|---|---|---|
| wiki-talk window | 49.4 | 103.2 | 13.9 (**0.28**) | 223.1 (**4.51, above 3**) | 30.2 |
| sx-askubuntu | 174.5 | 317.5 | 10.5 (**0.06**) | 175.9 (1.01) | 44.9 |
| wiki-talk insert-only | 763.1 | 946.0 | 10.7 (**0.014**) | 219.4 (0.29) | 78.6 |

Scan only is the scan-plus-edit replay minus the edit-only replay, which is the smaller denominator and so the stricter test. Pure form was made branch-free before scoring, down from 345 ns on the window.

**Verifier state, in words.** Split: 9,284 at ℓ = 21, 3,131 at ℓ = 18. Pure: 131 at ℓ = 21, 113 at ℓ = 18. Source: at least 142,434 on the window, 911,382 on sx-askubuntu, 5,575,934 on wiki-talk insert-only. So the source, not the verifier, holds a graph's worth of state.

**Checks, real states, p = 2^61 − 1.** Recounted from the CSV files. Seed bases 1000, 2000, 3000.

| Scenario | Runs | Accepted | Rejected at the last check | Meaning |
|---|---|---|---|---|
| Honest | 414 | **414** | 0 | |
| Wrong claim, honest messages | 414 | 0 | 0 | Dies in round 1. Weak test on its own |
| Wrong claim, liar through every round | 414 | 0 | 414 | Only the sketch stops it |
| Fabricator, never reads the graph | 1,500 | 0 | 1,500 | |
| Flipped edge | 414 | 0 | 414 | 332 of these had the **true** count, so only the graph was wrong |
| Dropped update | 414 | 0 | 414 | Modelled as the XOR of one applied edit |
| **Control:** leaky verifier reads the prover's graph, flipped edge | 414 | **414** | 0 | The mistake you warned of. It must accept, and it did |
| **Control:** leaky verifier, dropped update | 414 | **414** | 0 | Same |
| Raw feed, no filter | 3 | **0** | 3 | Honest prover rejected. Caveat 1 |
| Prover labels no-ops, drops one edit | 3 | **3** | 0 | Accepted, outside the model. Gap G1 |

After each honest audit, once r was public, the three words were checked against the true graph: 0 mismatches.

Zero acceptances at p = 2^61 − 1 say little by themselves, since the bound is about 5×10⁻¹⁷. They catch bugs. So the error was measured directly in a small field.

**Soundness error, p = 8191, ℓ = 5, 500,000 trials.** Bound 6ℓ/p = 0.003663.

| Attack | Accepted | Rate |
|---|---|---|
| Honest | 500,000 | 1 |
| Liar planting two roots per round, the best strategy | 1,832 | **0.003664**, predicted 0.003656, sampling error about 0.00009 |
| Liar, random deviation | 892 | 0.001784 |
| Flipped edge | 61 | 0.000122 |

So 6ℓ/p is tight in practice and the implementation matches it.

**Selftest, 5,000 random mixed graphs.** Same outcomes as the table above, with 1,637 same-count flips. The prover matched an exhaustive sum over the definition of g on all 300 rounds compared. The count matched `brute_triangles` on all graphs.

## 5. Who would use this instead of a trusted recount?

**On the evidence here, almost nobody, and nobody on these streams.**

A recount costs 0.42 s on 2.8M edges, and the prover costs 18 times that. So a party that can run a recount, or trust one, should. S5's baseline of an edge-set fingerprint plus one trusted recount catches lost updates and corrupted graphs, at about one recount plus a fingerprint pass. I did not build that baseline, so its cost is an estimate from the recount and sketch times above. It loses only on what the auditor must hold: the graph, 22 MB here, against 1.5 KB of proof and 74 KB of state.

The remaining user must meet all of these at once:
1. It cannot hold or read the graph.
2. It does not trust the party that does.
3. It still sees every change.
4. Each change it sees is a true toggle.
5. The count matters enough to pay for an interactive audit.

Item 4 is the hard one. Whoever guarantees it usually holds the edge set, and that party can recount. The case that survives is a feed that is well-formed by construction, such as a change log of real state changes, read by a small independent auditor. I know of no one who needs that for a triangle count. C1 has no use case today beyond the delegation setting of the theory papers, which is the risk S5 named.

## 6. Reproduce

```
export PATH="/c/Develop/tools/winlibs/gcc-16.2.0-ucrt-r1/mingw64/bin:$PATH"
g++ -std=c++20 -O2 -Wall -Wextra -static benchmarks/receipts.cpp -o build/receipts.exe
build/receipts.exe selftest 5000 500000 results/screen-b-M1B02/selftest.txt
build/receipts.exe state data/derived/wiki-talk-temporal.bin wiki-talk-temporal-window-half 0.5 300 500 1000 results/screen-b-M1B02/wiki_window
build/receipts.exe state data/derived/sx-askubuntu.ins.bin sx-askubuntu-insert-only-end 1.0 100 500 2000 results/screen-b-M1B02/sx_askubuntu_ins
build/receipts.exe state data/derived/wiki-talk-temporal.ins.bin wiki-talk-temporal-insert-only-end 1.0 14 500 3000 results/screen-b-M1B02/wiki_ins
build/receipts.exe kst 1000 5000 11 results/screen-b-M1B02/kst_1000_5000.txt
```

Each run wrote `*_summary.txt` and `*_runs.csv`, or a `.txt`, in this folder. `src/r11/tc.hpp` was not edited, nothing was committed, and nothing under `C:/Develop/tower` was touched. PDFs and extracted text are in the session scratchpad, folder `k2`.
