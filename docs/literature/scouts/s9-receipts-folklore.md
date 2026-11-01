# Scout S9: hostile folklore check on count receipts

Date: 2026-11-01. Inputs read first: `docs/literature/scouts/s5-local-and-guarantees.md`, `results/screen-b-M1B02/README.md`.
Job: try to prove candidate C1 is already known. Theorem numbers below refer to the cited source unless marked M1B02.

## Verdict

**Folklore in substance, not published as a lemma in anything I could read. It is not an open problem.**

| Question | Answer | Confidence |
|---|---|---|
| 1. Sparse sum-check prover for triangles published? | **Not found as a statement.** The protocol is published. The dense prover analysis is published. A prover in terms of m, wedges or degeneracy is not, in any text I read. A weaker bound, O(ℓ·W + n) with W = Σ d(v)², follows in about three lines from the monograph's protocol. An expert would call that an exercise. The degree orientation that turns W into D is the only step beyond that, and it is a known trick from plain counting | Medium. The two likeliest homes stayed unread (see "Not reached") |
| 2. Incremental or streaming form published? | **Yes, including repeated audits.** A streaming verifier for the same trivariate polynomial is in Thaler 2016, footnote 5, and the O(log n) word sketch is in Cormode, Thaler, Yi 2011. Section 2 of Thaler 2016 describes answering again after further updates, with the secret state updated in a stream. That is an audit of a running count in all but name, at O(n) state and proof, and it survives repeats, which C1 does not | High |
| 3. Real user? | **None found.** Systems for verifiable SQL and graph queries exist. None targets a triangle count, and no deployment of any of them on graph analytics turned up | Medium-high |
| 4. What remains claimable | First measurements on real streams, the W to D refinement as a credited remark, and a few open questions listed in section 4. **Not** a new prover as a headline contribution | My judgement |

Effect on C1. M1B02's stop clause, "sparse prover lemma found published", does not fire on what I read, but only because the statement is missing, not because the idea is hard. S5 scored novelty 7/10. I would put it at 4/10. Caveat 3 of M1B02 predicted that a specialist would call it folklore. This scout supports that prediction and finds no evidence against it.

## 1. Is the sparse prover published?

### Closest works

| Work | How far read | What it has | Why it does not contain C1's lemma |
|---|---|---|---|
| Goldreich, Rothblum, "Simple Doubly-Efficient Interactive Proof Systems for Locally-Characterizable Sets," ITCS 2018, LIPIcs 94, 18:1-18:19, [DROPS](https://drops.dagstuhl.de/entities/document/10.4230/LIPIcs.ITCS.2018.18) | Sections 1.2 and 3 in full | **Closest text overall. Not in S5.** Section 3 defines the same polynomial for t-cliques, a product of multilinear extensions of the adjacency predicate over pairs of vertex variables. Verifier evaluates it in O(\|E\|·ℓ) from the edge list, which is M1B02's Fact 2. Prover analysis notes that folded arguments with a Boolean suffix are cheap, the dense form of M1B02's Lemma 1. Remark 4 extends to counting | Prover bound is Õ(n^t), so Õ(n³) for triangles. Section 1.2 compares against Thaler's Õ(\|E\|·n^(t-2)) with Õ(n) communication and records theirs as worse on this point. So in 2018 two experts looked at exactly this cost and printed no sparse polylog-communication prover |
| Thaler, *Proofs, Arguments, and Zero-Knowledge*, version of 18 July 2023, [PDF](https://people.cs.georgetown.edu/jthaler/ProofsArgsAndZK.pdf) | Sections 4.3, 4.5.1, Lemmas 3.7, 4.5, Exercises 4.1, 4.2 | Same protocol. Prover T(A²) + O(n²). The book stresses "it does not matter how P chooses to compute A2". That sentence is the door to the sparse version | States n² as the extra cost. Warns that powers of sparse matrices may be dense |
| Björklund, Kaski, "How Proofs are Prepared at Camelot," [arXiv:1602.01295](https://arxiv.org/abs/1602.01295), PODC 2016 per Akmal et al. | Theorems 3 to 5 and Section 6 | **Sparsity-aware triangle proof, earlier than everything else here.** Trace of a triple product of sparse matrices. Proof size O(n^(ω+ε)/m), prover Õ(m) per node, for m ≥ n^((ω+ε)/2) | Total work is Õ(n^(ω+ε)), not wedges. It rests on the tensor rank of matrix multiplication, not sum-check. The authors list matching the sparse Alon-Yuster-Zwick bound in this setting as open. Any write-up must cite this |
| Akmal, Chen, Jin, Raj, Williams, "Improved Merlin-Arthur Protocols for Central Problems in Fine-Grained Complexity," ITCS 2022, LIPIcs 215, 3:1-3:25, [DROPS](https://drops.dagstuhl.de/opus/volltexte/2022/15599/); journal version in Springer, [DSpace](https://dspace.mit.edu/handle/1721.1/148127) | Pages 1 to 5 and 10 to 16 | Theorem 5 at k = 3: proof length Õ(n), verification Õ(m), for zero-weight triangles, which covers plain triangles. Section 4 sums over edges and evaluates sparse interpolants in time \|N(b)\|·polylog | Arthur reads the whole graph. Merlin's time is not stated in what I read. A degree Õ(n) proof at Õ(m) per evaluation suggests Õ(mn), my inference. Optimises verifier and proof length, the opposite goal |
| Thaler, "Semi-Streaming Algorithms for Annotated Graph Streams," ICALP 2016, [arXiv:1407.3462](https://arxiv.org/abs/1407.3462) | Sections 1, 3.1, footnotes 3, 5 | Prover O(m·n), called comparable to a naive count. Footnote 5 is the three-factor polynomial | Non-interactive route, n proof. No wedge or degeneracy bound. S5 has this right |
| Cormode, Mitzenmacher, Thaler, "Practical Verified Computation with Streaming Interactive Proofs," ITCS 2012, [arXiv:1105.2003](https://arxiv.org/abs/1105.2003) | Searched full text | Corollary 2.2, item 4: triangle protocol with prover O(n³ log n) | One line, dense |
| Chakrabarti, Cormode, Goyal, Thaler, "Annotations for Sparse Data Streams," SODA 2014, [arXiv:1304.3816](https://arxiv.org/abs/1304.3816) | Theorem 6.2 and its discussion | Scheme with costs in m, through a derived stream of sparsity m(n-2) | Annotation and space, no prover time, imperfect completeness |
| Setty, Thaler, Wahby, Lasso, [author PDF](https://people.cs.georgetown.edu/jthaler/Lasso-paper.pdf) | F.1, Theorem 6 (via M1B02's text) | Sparse-dense sum-check for one inner product. Credits CMT12 for O(m·polylog N) | One sparse factor against a structured one. Not a product of three adjacency tables |
| ZK-Value, [arXiv:2605.03581](https://arxiv.org/abs/2605.03581) | Sparse path, Appendix C (via M1B02's text) | Generic rule: track a support mask per factor and restrict the round sum to the intersection. Transcript is unchanged | Says to skip zeros. Does not bound or enumerate the intersection for factors on overlapping variables |
| Thaler, "Sum-check Is All You Need," [ePrint 2025/2041](https://eprint.iacr.org/2025/2041) | **Abstract and search excerpts only** | Has a section titled "Sparse Sums (and Streaming Proving)". When T of N terms are nonzero, the prover can be linear in T | **Main remaining risk.** See below |
| Goldreich, Rothblum, "Counting t-Cliques," FOCS 2018, [ECCC TR18-046](https://eccc.weizmann.ac.il/report/2018/046/) | **Search summaries only** | t-2 rounds, verifier Õ(t²n²), prover Õ(t^O(1)·n²) given an oracle for (t-1)-clique counts. Hirahara and Shimizu ([arXiv:2010.05822](https://arxiv.org/abs/2010.05822)) describe it as a sum-check variant with a degree O(n) polynomial | For t = 3 that is one round with an Õ(n) proof, the Thaler 2016 shape, my inference. Not a polylog proof |
| Hirahara, Shimizu, [arXiv:2010.05822](https://arxiv.org/abs/2010.05822) | Theorems 1.5, 2.8 through a page summary, first 100,000 characters | O(log n) rounds for K(a,b) and colorful H counting, verifier n²·polylog | Prover is an oracle call. No sparse bound |
| Williams, "Strong ETH Breaks With Merlin and Arthur," CCC 2016, [arXiv:1601.04743](https://arxiv.org/abs/1601.04743) | Theorem 1.6, Appendix A.1 through a page summary | MA proof for k-cliques, Õ(n^(⌊k/2⌋+2)) | Bounds in n only, no prover time |

Named in the brief but with nothing to add: Ball, Rosen, Sabin, Vasudevan, "Proofs of Useful Work," [ePrint 2017/203](https://eprint.iacr.org/2017/203), abstract through search only: low-degree polynomials from fine-grained complexity, no sparse statement seen. Reingold, Rothblum, Rothblum, STOC 2016: not read, known here only through Goldreich and Rothblum's comparison. Spartan, Libra: abstracts plus the monograph's account, sparse matrix times dense vector, and a sparse-monomial prover per a search excerpt. **I could not find a paper titled "Doubly-efficient interactive proofs for graph problems", nor a Bläser paper on clique counting proofs, in one or two searches each. I do not claim they exist or do not.**

### Why generic "sparse sum-check" does not kill it outright

g(x,y,z) has exactly 6T nonzero terms on the Boolean cube, so "prover linear in T" looks like an instant kill. It is not, on my reading. That statement needs the terms to stay sparse as variables are bound. Here each factor is a cylinder in 3ℓ dimensions. Once x bits are bound, (b,y,z) survives whenever (y,z) is an edge and both ends touch block b, whether or not any single x closes a triangle. That is why M1B02 logged 1.9×10⁹ iterations for T = 8.1×10⁶. So the cost scales with wedges, not triangles. I found no text that treats this overlapping case. If the survey does, the stop clause fires.

A second generic route also fails. Restricting A² to edges costs one recount, which is the natural idea, but its multilinear extension is not the extension of A² and the verifier cannot reduce it to the three sketch words. So the prover really must handle folded rows.

### The expert's exercise (my derivation, not a citation)

Take the monograph's Section 4.5.1. Compute C = A² sparsely in O(W), where W = Σ_v d(v)² and nnz(C) ≤ W. Run sum-check on the product of the two extensions over 2ℓ variables with sparse folded tables, O(ℓ·(m + nnz C)). The second phase needs a = Ã(r₁,·) and b = Ã(·,r₂), which cost O(m + n). Total **O(ℓ·W + n)**. By M1B02's reading this is the same transcript as C1, up to variable order. It uses nothing beyond the published protocol and table folding. So a specialist does not need a new idea to get a sparse prover.

Theorem 1 of M1B02 improves W to D = Σ d(v)·d⁺(v), by never materialising C and evaluating the sibling quadratic forms over oriented edges. That is the degree orientation behind compact-forward counting, applied to one sum. Worth stating, since the gain is real: on a star W is about m² and D is about m.

**Scratch measurement, not a gate.** Set semantics on the end state of each insert-only stream, labelled **real timestamps**. Script `C:\Users\esport34\AppData\Local\Temp\claude\C--Develop-20261031---Research-11-Local-Incremental-Triangle-Counting-in-Dynamic-Graphs\ee45f6ca-1e1f-4c35-84d3-b548251876ff\scratchpad\s9\w.py`. D matches M1B02 on the two states it reports.

| Stream | n | m | Max degree | W | D | W/D |
|---|---|---|---|---|---|---|
| email-Eu-core | 986 | 16,064 | 345 | 2,398,560 | 714,393 | 3.4 |
| CollegeMsg | 1,899 | 13,838 | 255 | 1,539,440 | 340,392 | 4.5 |
| sx-mathoverflow | 24,759 | 187,986 | 2,172 | 110,375,278 | 14,003,905 | 7.9 |
| sx-askubuntu | 157,222 | 455,691 | 5,401 | 377,223,894 | 14,541,515 | 25.9 |
| wiki-talk-temporal | 1,094,018 | 2,787,967 | 141,951 | 33,158,853,004 | 204,774,739 | 161.9 |

These are bound ratios, not measured run times of a W-prover. W is a loose stand-in for nnz(C) and the true iterations of an unoriented prover. I did not build that prover.

### Evidence each way

For folklore: the monograph's remark above; the O(ℓ·W) route is mechanical; M1B02 itself found the key count takes two lines; Lasso credits a sparse sum-check prover to CMT12, a 2012 paper.

Against "everyone knows it": Goldreich and Rothblum's 2018 comparison, the monograph's dense-only statement, and no statement in Lasso, Jolt or CMT12. The likely explanation is a matter of goals. This field chases verifier time, proof length and rounds. A sparse prover for a toy problem had no reader. That is why it is unpublished, and it is not evidence that it is hard.

## 2. Incremental or streaming form

| Work | How far read | Passage |
|---|---|---|
| Cormode, Thaler, Yi, "Verifying Computations with Streaming Interactive Proofs," PVLDB 5(1), 2011, [arXiv:1109.6882](https://arxiv.org/abs/1109.6882) | Sections 1, 2 by search | Theorem 1: the low-degree extension is computable over a stream of updates in O(d) space. The model allows negative deltas |
| Thaler, "Time-Optimal Interactive Proofs for Circuit Evaluation," CRYPTO 2013, [arXiv:1304.3812](https://arxiv.org/abs/1304.3812) | Sections 1.1, 2.1, 5.3.2, Corollary 2 | States that all its protocols work with streaming verifiers. Streams with negative deltas may define a matrix, as in MATMULT |
| Thaler monograph | Lemma 3.7, Section 4.3 | One pass, O(log n) words, any order. Section 4.3 says the final check needs three evaluations of Ã. Together these are C1's verifier, on one page each |
| Thaler, ICALP 2016 | Discussion before Theorem 3.1, footnotes 3, 5 | **Direct hit.** Interactive streaming protocol for triangles by sum-check on a bivariate form. Footnote 5 uses the trivariate form with Õ(log n) space and Õ(n) communication over degree n extensions. The text gives non-interactivity and its reusability as the advantage of Theorem 3.1 over this simple route. Section 2 describes the verifier updating a secret state as the stream grows, checking fresh answers, with error growing linearly in the number of answers. It adds that many interactive protocols lack this, since they reveal information about r. That is C1's use case and M1B02's gap G2, both in print |
| Abdullah, Daruki, Dutta Roy, Venkatasubramanian, ISAAC 2016, [arXiv:1602.08162](https://arxiv.org/abs/1602.08162) | Section 6 through a page summary | Lemma 6.1: (log²n, log²n) cost, O(log n) rounds, through an inverse frequency moment. Lemma 6.2 brings update time to polylog n. No prover time |
| Zhang, Genkin, Katz, Papadopoulos, Papamanthou, vSQL, IEEE S&P 2017, [PDF](https://www.ieee-security.org/TC/SP2017/papers/581.pdf) | Introduction, updates section, by text search | **Same update rule in a system.** The client keeps a digest of the database's multilinear extension. A simple update changes it by the χ of the cell times the value, in one round. Public verifiability through a polynomial commitment |
| Papamanthou, Shi, Tamassia, Yi, "Streaming Authenticated Data Structures," EUROCRYPT 2013, [record](https://rd.springer.com/chapter/10.1007/978-3-642-38348-9_22) | Search summary | Verifier updates O(log n) state with no interaction during the stream. Lattice hash, cited by Thaler as the cryptographic strand |
| Wang et al., "Dynamic zk-SNARKs," [ePrint 2024/1566](https://eprint.iacr.org/2024/1566) | Search summary | Proofs updated after small changes. Argument setting, a different cost model |

So the verifier sketch, its O(log n) words, its update rule, its turnstile semantics, repeated answers on a growing stream and the reuse limit of interactive forms are all published. Against Thaler's non-interactive scheme, C1 trades O(n) state and proof, with reuse, for three words and a log-size proof, single use. That trade is the honest description of what C1 adds on the verifier side. I found no paper with a delta audit like S5's Conjecture 2. It remains untested and unsearched in depth.

## 3. Is there a real user?

| System | Source | Checks what | Triangle count? |
|---|---|---|---|
| vSQL | [S&P 2017](https://www.ieee-security.org/TC/SP2017/papers/581.pdf) | SQL on dynamic outsourced tables, TPC-H, interactive CMT plus commitments | Not as a target. The word "triangle" never appears, and "graph" appears only in citations. A triangle count is a three-way self join, so it could run through this at circuit cost, my inference |
| IntegriDB | [CCS 2015 record](https://unpaywall.org/10.1145%2F2810103.2813711) | Limited SQL with joins | Search summary only |
| zkSQL | [PVLDB 16](https://www.vldb.org/pvldb/vol16/p1804-li.pdf) | SQL in zero knowledge | No sum-check per M1B02's search |
| vRAM | [PDF](https://home.cse.ust.hk/~dipapado/docs/vram.pdf), search summary | RAM programs, S&P 2018 | No graph benchmark seen, not read |
| Pequin | [GitHub](https://github.com/pepper-project/pequin/), search summary | C to SNARK toolchain | No triangle example seen, app list not read |
| ALITHEIA, later "Verifiable Graph Processing" | Zhang, Papamanthou, Katz, CCS 2014, [PDF](https://www.cs.yale.edu/homes/cpap/published/ALITHEIA14.pdf), ACM TOPS 2018 | Shortest path, longest path and max flow by SNARKs, with edge updates | **No.** Word absent in my text search |
| ZKGraph | Wu et al., [arXiv:2507.00427](https://arxiv.org/abs/2507.00427) | Graph queries in zero knowledge, LDBC SNB. Authors include Ant Group staff. Owner proves to a data user | No. No sum-check in the text. Its motive is privacy, which C1 does not offer |
| VeriDKG, [PVLDB 17](https://www.vldb.org/pvldb/vol17/p912-zhou.pdf) | Search summary | SPARQL on decentralised knowledge graphs | No sign of it |
| OpenRank, Karma3 Labs | [SDK](https://github.com/Karma3Labs/openrank-sdk), [press release](https://decrypt.co/219892/karma3-labs-raises-a-4-5m-seed-round-led-by-galaxy-and-ideo-colab-to-build-openrank-a-decentralized-reputation-protocol?amp=1), search summary | EigenTrust on social graphs, with a press claim of zero-knowledge proving | **The nearest live product that wants a verifiable graph metric.** Ranking, not counts. Verifiability claim unconfirmed beyond the press text |
| Concerto, FastVer | Arasu et al., SIGMOD 2017, [PDF](https://vineetp13.github.io/publications/2017-SIGMOD-Concerto-A%20High%20Concurrency%20Key-Value%20Store%20with%20Integrity.pdf), search summary | Deferred integrity for key-value stores, within a factor of two of no integrity | Not counts. **Bears on the user case:** if the edge store has integrity, then little is left for a graph-blind audit to catch that a trusted recount would not |
| Cormode, Hickey, "Cheap Checking for Cloud Computing," AISTATS 2018, [PMLR](https://proceedings.mlr.press/v84/hickey18a.html) | Abstract | Annotated stream proofs for OLS, PCA, LDA, reported efficient in practice | Nearest academic model of an audit by a weak client. Not graphs |

Implementations of triangle sum-check: Kaski, ALENEX 2018 (S5) is the only engineering study in the family. [Sum-Check-Tri-Count](https://github.com/Yunfei-Ma-McMaster/Sum-Check-Tri-Count) is a three-commit Python repository, apparently coursework, whose contents I did not see. CMT12 and CRYPTO 2013 contain no triangle experiments.

**Conclusion.** M1B02's section 5 stands. All the systems above want either privacy or a general query language. A single-metric auditor who cannot hold the graph yet sees every change has no example in print. The one route with a pull is a public metric computed by a party nobody trusts, as in OpenRank, and there a ranking is wanted, not a triangle count.

## 4. What remains claimable

Ranked by how well each survives a hostile reviewer.

1. **First measurements of a stream-audited exact triangle count on real temporal graphs.** Survives. Prover 1.9 to 18.1 × `static_count` in medians on three states, 0 of 3,156 attacks accepted, soundness error measured against 6ℓ/p. I found no competing numbers. This is a measurement result, so its weight depends on the user question. Hard rule 4 labels apply.
2. **The oriented bound O(ℓ·D + n), deterministic, any orientation.** Claim it only as "to our knowledge not stated". Credit the protocol to the monograph's 4.3 and 4.5.1, the polynomial to Goldreich and Rothblum, the sketch to Cormode, Thaler, Yi, and note the O(ℓ·W + n) route as immediate. Do not call it a new prover. Its honest content is the W to D step, which the table above values at 3.4 to 161.9 on these graphs.
3. **Open questions that look genuinely open.**
   - Prover near R, the true recount cost, on all graphs, or a lower bound. D/R is unbounded on K(s,t), which M1B02 shows with a 392 × run. No source I read addresses it. This is the one real theory question here.
   - Same prover style for t-cliques in terms of degeneracy. Goldreich and Rothblum stop at Õ(n^t). I did not try it, and it may also be an exercise.
   - Reuse of r against adaptive provers. The limit is stated by Thaler 2016. A fix would be new.
4. **Set semantics on a raw stream, gap G1.** See the lead below. Thaler's schemes avoid the issue by counting with multiplicities (Theorem 3.1, footnote 3), so "raw feed with duplicates" is a known modelling choice, not a discovery. A proved lower bound for a graph-blind verifier under set semantics might still be small and new. His XOR-model lower bound is for connectivity and bipartiteness, not triangles.

**Not claimable:** the sketch, the protocol, 3ℓ rounds, 9ℓ field elements, turnstile support, Facts 1 to 4 of M1B02, soundness at 6ℓ/p.

## Lead for gap G1 (my reasoning, unproved, not built)

M1B02 says it has no design for G1. There is a published starting point. The DISTINCT protocol of Thaler, CRYPTO 2013, Section 5.3.2 and Corollary 2, maps each frequency a to a^(q-1) over a Mersenne prime q. That is exactly 1 on nonzero entries and 0 on zero entries, by Fermat's little theorem, with a streaming verifier of O(log n) words. The field already in use, 2^61 − 1, is such a prime. Let the verifier sketch the raw multiplicity table, which is linear and blind. Then Ẽ becomes the output layer of about 60 squaring and multiplying layers, proved by GKR before the triangle sum-check. Windows with expiry fit if the state is defined as the pairs with at least one live copy.

Open costs: Corollary 2 gives prover O(n log q) with n the universe, here n², so a sparse layer prover is needed. Each layer adds rounds, so the proof grows by roughly 60·2ℓ·3 words, my estimate: about 7,500 words or 60 KB at ℓ = 21, against 1.5 KB today. Soundness of the three correlated output claims needs writing. This might remove caveat 1 of M1B02 without a pair set at the source. It is a lead, not a result, and it does not help the user question.

## Corrections to earlier notes

- **S5 and M1B02 omitted Goldreich and Rothblum, ITCS 2018.** It is closer than the FOCS 2018 paper for C1: same polynomial, polylog proof, explicit prover analysis in Section 3. The brief called FOCS 2018 likely closest. On what I could read, ITCS 2018 is.
- **S5 listed Thaler 2016 footnote 5 as "without detail".** It names the exact trivariate polynomial. So C1's verifier protocol is in print, with degree n extensions in place of multilinear.
- **Thesis section number.** S5 and M1B02 point to Section 3.12. A search excerpt of the thesis contents gives 3.10.1, "Counting Triangles via Matrix Multiplication". The body is still unread.
- **Theorem 1 holds for any orientation.** The brief's "degeneracy-style" is a special case. The m term in the brief's bound is absorbed, since m ≤ D.

## Not reached

| Source | What happened | Consequence |
|---|---|---|
| Thaler, *Practical Verified Computation with Streaming Interactive Proofs*, Harvard thesis 2013, [DASH](https://dash.harvard.edu/handle/1/11169768) | HTTP 429 with Retry-After 900 on every attempt, including the handle page and one attempt after waiting out the full 900 seconds. Earlier runs got empty files. Not bypassed | **Unread.** Only the title and a contents line were seen |
| Thaler, "Sum-check Is All You Need," [PDF](https://eprint.iacr.org/2025/2041.pdf) | HTTP 403 by WebFetch. The first 100,000 characters of Thaler's home page do not list it. I did not route around the block | **Unread except excerpts.** Most likely place for a statement that kills the lemma |
| Goldreich, Rothblum FOCS 2018 | ECCC and all Weizmann URLs returned connection resets. Semantic Scholar API returned 429 | Summaries only. ITCS 2018 from DROPS read in its place |
| Goldreich's survey on doubly-efficient proofs | Same resets, Now Publishers 403 | Unread |
| Spartan, Libra, Ball et al., Nair et al. full text | ePrint PDFs blocked as above | Abstracts or excerpts only |
| Chakrabarti, Cormode, McGregor, Thaler, "Annotations in Data Streams," ICALP 2009, TALG 2014 | Author PDF refused on its certificate, ECCC reset | Known through CCGT's citations of Theorems 7.2 to 7.4 |

ePrint's site search returned 49 results for "sum-check", none for "triangle counting" or "sum-check graph sparse adjacency", and one off-topic hit for "triangles graph interactive proof". That index looks partial, so this is weak evidence. The arXiv API returned 503 once, then zero hits for `all:"sum-check" AND all:triangles`, `all:sumcheck AND all:triangles` and `abs:"sum-check" AND abs:"sparse" AND abs:graph`. `abs:"interactive proof" AND abs:triangle` gave three hits. I checked "Doubly-Efficient Pseudo-Deterministic Proofs" ([arXiv:1910.00994](https://arxiv.org/abs/1910.00994)) through a page summary: zero-weight triangle bounds in n only, not relevant. Jauregui and Miyamoto's paper, which has a triangle section, does not surface under these queries, so the API misses are weak evidence too. About 40 web searches ran, on sparse sum-check, matrix products, joins and FAQ, tensor networks, verifiable graph systems, dynamic SNARKs, and each named paper. Search excerpts are marked as such above and I relied on them only where stated.

**Reading aid.** Passages marked "page summary" came through a fetch tool's summary of the page, not my own line-by-line read. Passages marked "via M1B02's text" came from text extracted in that run and re-read here. Details of the ICALP 2016, CRYPTO 2013, ITCS 2018, Camelot, CCGT, CMT12, vSQL, ZKGraph, and monograph entries come from extracted full text. These citations are not yet in a checked `.bib`.
