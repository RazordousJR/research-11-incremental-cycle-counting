# Scout S1: expensive patterns

Date: 2026-11-01. Direction: batch maintenance of patterns dearer than a triangle, exact, on CPU. Inputs read
first: `docs/literature/audit-2026-11-01.md`, the Gate 0 record and the hindsight diagnostic.

## Verdict

**Batch sharing for expensive patterns is mostly taken, and where it is not taken it is mostly not worth having.
One opening survives, and it does not rest on batch sharing.**

1. **Grouping updates to share work is published.** Wang et al. (VLDB Journal 2022) maintain butterflies exactly
   under batches. Cheetah (TACO 2025) groups updates for general patterns. Gopher (EuroSys 2026) merges repeated
   set operations in dynamic pattern mining. Hriday et al. (2023) and ParaDyMS (CCGrid 2025) cover 4-vertex
   census under batches.
2. **The Gate 0 cause comes back in a new form.** A 4-cycle update is expensive only for a scanner. A
   persistent wedge table, the simple O(n) algorithm in Assadi and Shah's Appendix A, brings a 4-cycle update
   back to about d(u)+d(v). In unit operations it beat scanning in all 18 insert-only cells. A 5-cycle update stays expensive with the
   table, but star sharing on top of it still leaves a ratio of 0.80 to 1.06. Hanauer et al.
   reach the same conclusion by experiment: 3-paths, paws, 4-cycles and diamonds are the patterns where auxiliary
   structures pay. Scanning stays best for triangles and 4-cliques.
3. **The one exception is hub-heavy streams.** On wiki-talk-temporal the wedge table is about as dear as scanning
   or dearer, and it needs up to 1.66e10 wedge pairs. Only there does the hindsight ratio against the best baseline
   fall under 0.67. That is one stream.
4. **The opening.** I found no exact dynamic 5-cycle counter, and Hanauer, Henzinger and Hua name patterns of five
   or more vertices as future work. Treat that as weak evidence: see the search log. The candidate is an
   engineering study of exact dynamic 5-cycle counting. Batch sharing is one arm of it and not the claim.

## What exists, by pattern

| Pattern | Exact, batch, CPU? | Source |
|---|---|---|
| Butterflies (bipartite 4-cycles) | **Yes**, one paper. BFCB-IG+, with mixed batches split into deletions then insertions | Wang et al. 2022 |
| 4-cycles in general graphs | As part of census: Hriday, ParaDyMS. No dedicated batch method found. Wang's algorithm does not appear to need bipartiteness beyond its two-layer setting, but that is my reading, not a result of theirs | See below |
| Diamonds, paws | Census only. Single-update: Hanauer 2026 | |
| 4-cliques | Census, and the k-clique theory below | |
| k-cliques | Theory: Dhulipala 2021, Liu et al. Implemented batch code: triangles only in Dhulipala. Tesseract enumerates cliques per update | |
| 5-vertex patterns, 5-cycles | **None dedicated found.** Static only: ESCAPE, Shi et al. General pattern systems (Tesseract, PSMiner, Gopher) can mine 5-vertex patterns by enumeration. PSMiner's figures include the 5-clique | |
| Motif census, k ≤ 4 | **Yes.** Hriday 2023, ParaDyMS 2025. SC'25 poster for local counts | |
| Continuous subgraph matching with batches | CPU: Graphflow and IEDyn (per Sun et al.), Mnemonic. GPU: GAMMA, GCSM-BU, DCSM. ParaCOSM adds parallelism with no sharing of work between updates | |

## Search log and limits

"I found nothing" below always means "these searches found nothing". DBLP was not used.

| Source | What was run | Limit |
|---|---|---|
| arXiv API | 44 queries over `abs:` and `ti:` fields: butterfly, batch-dynamic, 4-cycle, 5-cycle, k-clique, graphlet, motif, CSM, IVM, delta queries, graph pattern mining | Heavy 429s. 43 of 44 returned after retries. Several exact-phrase queries returned 0 and prove little. The 5-cycle queries returned nearly all noise, with no exact dynamic counter among them, which is thin evidence either way |
| OpenAlex API | 34 title-and-abstract filters, 30 relevance searches since 2017, 675 records, 134 kept by title filter | 3 relevance searches failed. Then the shared free daily budget ran out, so OpenAlex forward citations failed. It resets at midnight UTC. I did not request a key, since that would send your email to a third party |
| Semantic Scholar API | Forward citations of 15 seed papers, 385 citing works, 51 kept by title filter. **This is what found Cheetah, Gopher and DyPamear, which my keyword searches missed.** That is a direct warning about keyword coverage | Hriday, ParaDyMS and Hanauer 2026 returned 0 citing works here, so nothing downstream of them was screened. The title filter can drop relevant work |
| Web search | 23 queries | Snippets only unless a read level says otherwise |
| GitHub API | 2 repository searches. `bindsch/tesseract` and `TauferLab/ParaDyMS` are public. No Cheetah or Gopher repository returned | Two queries are weak evidence |

Full text I could not reach: ACM Digital Library (403) for Cheetah, ParaCOSM's ACM page and Lee et al.; Springer
(login redirect) for GCSM-BU; NSF PAR (no connection) for ParaDyMS. The Michigan Tech copy of Gopher returned a
Cloudflare challenge, so I stopped there and did not try to pass it. Mhedhbi TODS stays closed, as the audit said.

**Cheetah and Gopher are the two most important papers here and I read only their abstracts plus excerpts.** A
reader with library access should read both before trusting any "gap" below.

## Scout probe, not a gate

No paper gives a per-update scan cost for these patterns on these streams, and that number decides this direction.
So I wrote a small operation-count probe in my scratchpad. It includes `src/r11/tc.hpp` read-only, uses the Gate 0
streams, cuts, netting and batch-degree groups, and changes nothing in the project. Each pass of 36 expiry cells
took about two minutes. Sources and CSVs are in
`C:\Users\esport34\AppData\Local\Temp\claude\C--Develop-20261031---Research-11-Local-Incremental-Triangle-Counting-in-Dynamic-Graphs\ee45f6ca-1e1f-4c35-84d3-b548251876ff\scratchpad\s1_scripts\`
and the sibling `s1\` folder. Copy them if they are wanted, since the scratchpad is temporary.

**It is a cost model. It computes no counts and proves no exactness.** It charges the Gate 0 unit costs, with a
hash probe costing the same as a stamp read, which favours the wedge table. Per-update numbers are per net update.
All streams carry the label **real timestamps, synthetic expiry** (36 cells) or **real timestamps, insert-only, no
expiry** (18 cells).

Operations per net update:

| Pattern, per-edge cost | Expiry | Insert-only |
|---|---|---|
| Triangle scan (sanity) | 4.8 to 15.4 | 23 to 66 |
| 4-clique scan | 5.3 to 26.7 | 37 to 576 |
| Diamond scan | 13.4 to 82.7 | 176 to 6,467 |
| 4-cycle scan, cheaper role | 30.9 to 262 | 610 to 13,131 |
| 4-cycle, wedge table | 43.4 to 2,247 | 146 to 12,280 |
| 5-cycle, two pivot rows | 290 to 9,395 | 3,976 to 510,287 |
| 5-cycle, wedge table and join | 181 to 6,585 | 1,625 to 162,899 |

Sanity: at the 1,000-event cut the triangle column reads 6.2 to 14.6, against 5.9 to 13.9 in the Gate 0 record. This
probe reads degrees with the whole group applied, so it runs slightly high.

Hindsight ratio: for each batch, the cheapest of the method's arms and a static recount, with the Gate 0 charges of
2 per raw event, 2 per net update and 3 for grouping, divided by the same for the baseline. No selector cost is
charged, so this is a floor. The reference line is Gate 0's 0.67.

| Ratio | Expiry, range | Cells ≤ 0.67 | Insert-only, range | Cells ≤ 0.67 |
|---|---|---|---|---|
| 4-cycle, star sharing against scanning, static floor | 0.575 to 1.165 | 2 of 36, both wiki-talk | 0.496 to 0.957 | 8 of 18, on 5 streams |
| 4-cycle, same, memo kernel always, no hindsight | 0.844 to 1.506 | 0 | 0.811 to 1.280 | 0 |
| 4-cycle, star sharing against the better of scanning and wedge table | 0.575 to 2.006 | 2, wiki-talk | 0.554 to 8.523 | 2 of 18, wiki-talk |
| 5-cycle, star sharing against scanning, static floor | 0.635 to 1.183 | 1 | 0.460 to 1.017 | 3 |
| 5-cycle, star sharing against the better of scanning and wedge join (24 and 12 cells) | 0.814 to 1.532 | 0 | 0.988 to 3.217 | 0 |
| 5-cycle, star sharing on top of the wedge join, 3-path memo (12 and 12 cells) | 0.798 to 1.060 | 0 | 0.855 to 0.997 | 0 |

Without the static floor and without charges the star ratio looks far better, 0.20 to 0.98 for 4-cycles on expiry
streams. **That number misleads.** At coarse cuts a batch is as large as the two-week window (10,000 events against
about 7,000 peak edges on sx-superuser), so a recount is the honest competitor.

Memory: peak wedge pairs, summed as Σ C(d,2) and sampled, so an upper bound on table entries.

| Stream | Expiry | Insert-only |
|---|---|---|
| CollegeMsg, email-Eu-core, sx-* | 5.2e4 to 2.1e5 | 7.6e5 to 4.4e8 |
| wiki-talk-temporal | 4.8e8 | 1.66e10 |

At a nominal 16 bytes per entry that is about 7.6 GB and 265 GB if every pair were distinct, against 31 GB of RAM.
On expiry streams with small windows the table is trivially small, so it is a fair competitor there.

Known weaknesses of these numbers:

- The 5-cycle static floor is a lower bound of my own, `2·Σ d(u)·d⁺(u)`. The real static cost is O(mα²) (Shi et
  al.). The floor favours recount, so coarse-cut 5-cycle ratios near 1.0 are pessimistic for the method.
- The two-row 5-cycle cost is an upper estimate of what a careful scanner would pay.
- A real hash probe costs several stamp reads. Under a wall clock the wedge table loses some of its lead. By how
  much is unmeasured, and rule 3 of `CLAUDE.md` says operations decide first in any case.
- `HHH*-H`, the real best 4-cycle baseline, is not modelled. Reading Figure A.4(d) of Hanauer et al. by eye, it
  takes about 14 s for 4-cycles and about 2 s for triangles on their ES set, so about 7 µs per update if the plotted
  figure is per instance. That is a chart reading, on their hardware.
- Star groups use Gate 0's batch-degree pivots. A weighted vertex cover of the batch graph could do better. Not
  tested.

Separately, a 40-line Python brute-force check confirmed the shared-row formulas in candidate 1 on 3,887 random star
insertions, for both 4-cycles and 5-cycles, with no mismatch. Insertions only, on graphs of at most 11 vertices.

## Candidates, ranked

Scores are out of 10, for novelty and for solo feasibility on the Core Ultra 7 265 with 31 GB RAM. CPU only, so
the RTX 5070 Ti plays no part.

### 1. Exact dynamic 5-cycle counting on real streams

**Claim.** Exact fully dynamic and batch 5-cycle counting is practical on real streams, and the best of three
kernels (two-row scan, wedge-table join, shared pivot row) is decided by hub structure.

**Type.** Method plus empirical study. **Scores: novelty 7, feasibility 7.**

| Closest paper | Read | Overlap |
|---|---|---|
| Hanauer, Heck, Henzinger, Sidl. [arXiv:2610.29776](https://arxiv.org/abs/2610.29776), ALENEX 2027 short version | Main text, appendices A.3, A.4, A.17, Figures 5.5 and A.4 | Stops at 4 vertices, single updates. The obvious authors of the follow-up. Highest risk |
| Hanauer, Henzinger, Hua. [Fully Dynamic Four-Vertex Subgraph Counting, arXiv:2106.15524](https://arxiv.org/abs/2106.15524), SAND 2022 | Conclusion only, plus the 4-cycle bound | States that patterns of five or more vertices are future work. Theory of 4-vertex patterns |
| Shi, Huang, Shun. [Parallel Five-cycle Counting Algorithms](https://jshun.csail.mit.edu/fivecycle-jea.pdf), ACM JEA | Section 1 | Static, O(mα²). Supplies the recount baseline. JEA 27, 2022. Reports ESCAPE spending 25% to 58% of its runtime on 5-cycles, which is the motivation |
| Zhang et al. [Gopher, EuroSys 2026](https://doi.org/10.1145/3767295.3769365) | Abstract through the [Michigan Tech record](https://digitalcommons.mtu.edu/michigantech-p2/2645/) and the Semantic Scholar API | General patterns by enumeration, merging common set operations. Could already handle 5-cycles. Whether it is count-only, and how it treats batches, is unread. **Main unknown** |
| Yuan et al. [Batch Hop-Constrained s-t Simple Path Query Processing, arXiv:2312.01424](https://arxiv.org/abs/2312.01424), ICDE 2024 | Introduction | One 5-cycle through an update is a 4-hop path between its endpoints, and this paper shares work across a batch of such queries. But it enumerates on a static graph, with no update model |

Also relevant: [ESCAPE, arXiv:1610.09411](https://arxiv.org/abs/1610.09411) (abstract), static counts of all 5-vertex
patterns.

**Gap.** No exact dynamic counter for 5-cycles found, and no experimental comparison of how to build one. Search
terms and limits are above. Evidence strength: weak to moderate. Forward citations of ESCAPE (201), the five-cycle
paper (3) and Hanauer, Henzinger and Hua (15) were title-screened, so a paper with an unusual title could pass.

**Mechanism.** Maintain the count through the cycles on each changed edge. For an edge (u,v), with A[b] the number
of paths u-a-b avoiding v, and C[b] the number of paths v-c-b avoiding u:

`cycles through (u,v) = Σ_b A[b]·C[b] − Σ_{t ∈ N(u)∩N(v)} (d(t) − 2)`

The kernels differ in how they get A and C.

1. **Two-row scan.** Build both rows. No index and no choice of a cheap side: unlike a 4-cycle, the cost has both
   endpoints in it. That makes it the case where a star group has most to save.
2. **Wedge-table join.** With a persistent table W, sum W[a,c] over N(u)×N(v). Cost d(u)·d(v) reads plus the table
   writes.
3. **Shared pivot row.** For a star (x; v₁…v_k), build the row at x once. After each spoke, add that spoke's
   neighbours to the row. No correction term is needed, because each spoke sees the graph with the earlier ones
   already in. Exactly-once is ordinary telescoping and is not a contribution.

Add a recount fallback for coarse batches, and a selector over all four.

**Why it escapes the Gate 0 cause.** Partly, and only because the claim is not batch sharing. Per-update cost is 181
to 162,899 operations against Gate 0's 6 to 64, so kernel choice matters. Batch sharing alone does not carry it: it
is capped at 0.46 to 1.02 against scanning, at 0.81 or above against the wedge join, and leaves 0.80 to 1.06 when run on top of
the join. Report it as one arm.

**Strongest simple baseline.** Wedge-table join on hub-free streams, per-edge two-row scan where the table will not
fit, and static recount by the Shi et al. method at coarse cuts. Reviewers will also ask for Tesseract, which is
public, and for Graphflow, as Hanauer uses it.

**Kill test, under an hour.** Add the three kernels beside `Engine` in a copy of `tc.hpp`, with a real exact count
and a brute-force check on small graphs, including deletions, which my check did not cover. Add a true O(mα²) static
counter. Run CollegeMsg, email-Eu-core and sx-mathoverflow under both stream variants at 100 and 1,000 events.
Kill if: any mismatch; or one kernel wins every stream at every cut by more than a factor of 2 in wall-clock
time, since a plain wedge join then leaves nothing to study; or peak memory of the table is under 1 GB on every
stream and it wins, which reduces the paper to a one-line lemma. Before any of it, read Gopher's full text.

**Likely venue.** ALENEX or SEA first. For the PI's free list: *Journal of Graph Algorithms and Applications*, which
is on it. *ACM Journal of Experimental Algorithmics* fits best but was not found on the list.

**Main risks.** Few applications ask for 5-cycle counts alone, so the motivation leans on 5-vertex census. The
fallback story, a two-page wedge-join lemma, may be all there is. Hanauer's group can do this quickly.

### 2. Index-level reuse with scanning-level memory on hub-heavy streams

**Claim.** On hub-heavy streams, where a persistent wedge table does not fit and its writes cost as much as a scan,
per-group ephemeral rows maintain 4-cycles exactly at lower cost than scanning, the table or an affected-subgraph
recount.

**Type.** Method. **Scores: novelty 4, feasibility 8.**

| Closest paper | Read | Overlap |
|---|---|---|
| Wang, Lin, Qin, Zhang, Zhang. [Accelerated butterfly counting with vertex priority on bipartite graphs, VLDB J. 2022](https://d-nb.info/1263104584/34) ([DOI](https://doi.org/10.1007/s00778-022-00746-0)) | Sections 7, 8.3, 9 | **Highest.** BFCB-IG+ aggregates wedges by start vertex over an affected subgraph, so sharing between updates is already there. Exact and parallel, reported at least 10 times faster than per-edge on large graphs |
| Hanauer et al. 2026, as above | As above | `HHH*-H` is the real baseline. It restricts anchors, which is how it survives hubs |
| Assadi, Shah. [arXiv:2504.10748](https://arxiv.org/abs/2504.10748), PACMMOD 2025 | Introduction and Appendix A | Simple O(n) wedge table in Appendix A. Main result is theory with fast matrix multiplication |
| Burkhardt, Harris. [Simple and efficient four-cycle counting on sparse graphs, arXiv:2303.06090](https://doi.org/10.48550/arxiv.2303.06090) | Abstract through OpenAlex | Static, n words of space. Good recount fallback |
| Zhang et al. [Cheetah, TACO 2025](https://doi.org/10.1145/3736173) | Abstract through the [Michigan Tech record](https://digitalcommons.mtu.edu/michigantech-p2/1862/) and the Semantic Scholar API, plus search excerpts | Groups updates by shared exploration domains, for enumeration, on five graphs |

**Gap.** BFCB-IG+ builds the affected subgraph for every batch, and the authors report it slightly losing to the
per-edge method on small datasets for that reason. Its experiments use random batches of 0.01% to 2% of the edges,
plus timestamped MovieLens. Nothing found studies small real batches, general graphs and selection among per-edge,
pivot rows, affected subgraph and recount. That gap is narrow.

**Mechanism.** Rows as in candidate 1. Pivot on a weighted vertex cover of the batch graph, weight being the wedge
cost of the vertex. Fall back to per-edge for singletons, to recount when the batch is large against the graph.

**Why it escapes.** Against scanning, 4-cycle updates cost 31 to 13,131 operations. Against the table, only wiki-talk
escapes: ratios 0.554 to 0.815 in 8 of the 9 cells on that stream and 1.103 at the 7-day cut, while the table costs 1,654 to 12,280 per update and peaks at
4.8e8 to 1.66e10 pairs. On the other five streams the table wins in unit operations.

**Strongest simple baseline.** `HHH*-H|late.233` from Hanauer et al. Plain wedge table where it fits.

**Kill test, under an hour.** Implement BFCB-IG+ (Algorithm 9) and the row kernel on wiki-talk-temporal, insert-only
and expiry, at 1,000 and 10,000 events, and run `HHH*-H` from the public artifact as an external baseline. Kill if
the hindsight ratio against the best of these is above 0.80, or if the win is on this stream alone. Only one such
stream is in `data/snap/`, so this needs a second hub-heavy real stream before it can support any claim. A Wikipedia
hyperlink stream is the natural one, and fetching it needs PI approval.

**Likely venue.** *Journal of Parallel and Distributed Computing* or JGAA, both on the PI's list.

### 3. Do dedicated counters beat dynamic pattern-mining systems on real batches?

**Claim.** A head-to-head on real streams shows whether general batch-sharing systems beat lean per-edge counters,
which no paper read here measures.

**Type.** Empirical. **Scores: novelty 5, feasibility 5.**

| Closest paper | Read | Overlap |
|---|---|---|
| Hanauer et al. 2026, as above | As above | Compares dedicated counters with CSM systems and DNA. Does not cite Tesseract, ParaDyMS, Cheetah or Gopher (text search of the PDF) |
| Khan, Tan, Marquez, Taufer, Bhowmick. [ParaDyMS](https://doi.org/10.1109/ccgrid64434.2025.00046) | CCGrid text unreachable. [SC'25 poster](https://sc25.supercomputing.org/proceedings/posters/poster_files/post308s2-file3.pdf) in full, [repository](https://github.com/TauferLab/ParaDyMS) README and `dynamic_counting.hpp` in part | Per-edge in the outer loop, distance-2 enumeration, baseline is static PGD |
| Hriday, Sista, Das. [Efficient Batch Dynamic Graphlet Counting, arXiv:2308.14493](https://arxiv.org/abs/2308.14493) | Full text | Static PGD run on two affected subgraphs. Baseline is recount. Batches of 10 and 100 |
| Bindschaedler, Malicevic, Lepers, Goel, Zwaenepoel. [Tesseract, EuroSys 2021](https://www.eecg.utoronto.ca/~ashvin/publications/tesseract.pdf) | Sections 1 and 4 in part | Each update is still explored separately within a snapshot |
| Zhang et al. Gopher and Cheetah, as above | Abstracts | Baselines named are Tesseract, Delta-BigJoin, PSMiner. No dedicated counter named in what I could read |

**Gap.** The dedicated-counter papers and the mining-system papers do not appear to measure each other, on what I
could read. This is close to a replication study, and worth having only if the gap in speed is large.

**Mechanism.** No new algorithm. One harness, the six streams, one thread and then all cores, count-only.

**Why it escapes.** It is a measurement, so sharing need not win. One predictive number: ParaDyMS's per-edge loop is
close to the two-row cost above, about 690 operations per update on sx-superuser at the 1,000-event cut, against
about 87 for the three cheapest-role scans it would replace (4-cycle, diamond, 4-clique). That predicts a factor near
8, before constants. Not measured on ParaDyMS itself.

**Strongest simple baseline.** The sum of per-pattern cheapest-role scans.

**Kill test, under an hour.** Build ParaDyMS with Kokkos in WSL, run one thread on CollegeMsg and sx-mathoverflow
batches, and compare time with a scan census. Kill if within a factor of 2. Check for Cheetah and Gopher code first.
If neither is available the study loses its most interesting arm.

**Likely venue.** *Journal of Parallel and Distributed Computing*.

### 4. Batch local 4-cycle counts without enumerating cycles

**Claim.** Per-vertex and per-edge 4-cycle counts can be updated under batches from aggregated rows, without touching
each affected cycle, so sharing has real output-size work to remove.

**Type.** Method. **Scores: novelty 5, feasibility 5.** Nothing here was measured.

| Closest paper | Read | Overlap |
|---|---|---|
| Khan, Bhowmick, Taufer. [Parallel Local Motif Counting on Large-Scale Dynamic Graphs, SC'25 poster](https://sc25.supercomputing.org/proceedings/posters/poster_files/post308s2-file3.pdf) | Full | Local counts for k ≤ 4 under batches, per updated edge. **Closest** |
| Abidi, Chen, Zhou, Liu. [Searching Personalized k-wing, arXiv:2101.00810](https://arxiv.org/abs/2101.00810) | Definitions and maintenance passages | Butterfly support under single edge updates |
| [Distributed Approaches to Butterfly Analysis on Large Dynamic Bipartite Graphs, TPDS 2022](https://doi.org/10.1109/tpds.2022.3221821) | Abstract through OpenAlex | Tip maintenance per updated edge, distributed |
| Wang et al. 2022, as above | As above | Global count only, on my reading of Section 7 |
| [A Unified Framework for Dense Subgraph Maintenance over Dynamic Bipartite Graphs, PACMMOD 2026](https://doi.org/10.1145/3786618) | Abstract through OpenAlex | Maintains bi-core and links it to bitruss. Could subsume the use case |

**Gap.** Batch maintenance of butterfly support with aggregated rows was not found. My bitruss search was one query
plus a screen of titles, so this is weaker than candidate 1. [Dense subgraph mining in dynamic bipartite
graphs](https://doi.org/10.1016/j.ins.2025.122758) (Information Sciences 2025) returned 403 at ScienceDirect, so it is unread and may overlap.

**Mechanism.** For a star at x, the row gives per-vertex deltas in one pass. Per-edge deltas need wedge-level
aggregation, which is between row cost and cycle enumeration.

**Why it escapes.** Baseline cost is tied to output. Hanauer's Table A.4 lists about 1.7e6 4-cycles at about 7.3K
edges on superuser, if I read its unlabelled columns in the order of their pattern list. That is about 930 cycles
per edge at peak, against 56 operations for a count-only scan.

**Strongest simple baseline.** Per-edge enumeration of the cycles through each update.

**Kill test, under an hour.** Log the count of cycles through each update and the wedge-level work for the same
groups on three streams. Kill if the ratio is above 0.67, or if no application in the literature needs exact local
counts at batch granularity.

**Likely venue.** JGAA.

## Rejected ideas

| Idea | Killed by | Read | Why |
|---|---|---|---|
| Exact batch butterfly counting from an affected subgraph | [Wang et al. 2022](https://d-nb.info/1263104584/34) | Sections 7, 8.3 | Done, parallel, in a top journal |
| Group updates so they share exploration, general patterns | [Cheetah, TACO 2025](https://doi.org/10.1145/3736173) | Abstract, excerpts | The project's framing in another form. Excerpts say over 90% of updates share a vertex with another, and that groups share expansion paths. Average 2.63× over leading systems |
| Merge repeated set intersections in dynamic mining | [Gopher, EuroSys 2026](https://doi.org/10.1145/3767295.3769365) | Abstract | Up to 75.58× over Tesseract, on a multi-core CPU |
| Local recount of a changed region for 4-vertex graphlets | [Hriday et al. 2023](https://arxiv.org/abs/2308.14493) | Full text | Done, though its region is 3 hops and its baseline is recount |
| Parallel per-edge batch motif counts | [ParaDyMS](https://doi.org/10.1109/ccgrid64434.2025.00046) | Poster, code in part | Done. Classifies matches by number of updated edges |
| Exact batch k-clique counting | [Dhulipala et al. 2021](https://arxiv.org/abs/2003.13585), [Liu et al.](https://arxiv.org/abs/2106.03824) | Sections 1, 3.2 to 3.4; abstract | Bounds exist. Implementation gap remains, but see the next row |
| Practical batch 4-cliques | [Hanauer et al. 2026](https://arxiv.org/abs/2610.29776) plus this probe | Sections 5, 6 | Plain scanning is best for 4-cliques, and they cost 5.3 to 26.7 operations per update on expiry streams. The Gate 0 cause applies unchanged |
| Batch paws | Hanauer et al. 2026, section A.13 paw routine | Appendix | Paw counts follow from triangle counts on the edge plus degree terms and per-pair counts. Reasoned from their routine, not measured |
| Batch diamonds as a separate target | Probe and Hanauer | | Same 2-hop structure as 4-cycles with the same index answer. No separate case |
| 4-cycles by persistent wedge or heavy-light index | [Hanauer et al. 2026](https://arxiv.org/abs/2610.29776), [Assadi, Shah 2025](https://arxiv.org/abs/2504.10748) | As above | Taken, and excluded by the brief |
| Approximate batch butterflies | [Abacus, ICDE 2024](https://arxiv.org/abs/2312.03435) | Abstract, introduction | Sampling, out of scope. Includes a parallel mini-batch variant |
| Batch CSM on GPU | [GAMMA](https://arxiv.org/abs/2401.17018), [DCSM, ICS 2026](https://doi.org/10.1145/3797905.3800512), GCSM-BU (search excerpt only, page not reachable) | Introduction; abstract | Out of scope, CPU only here |
| Parallelise CSM across updates | [ParaCOSM, ICPP 2025](https://haibinlai.github.io/File.github.io/icpp25-5.pdf) | Abstract, batch passages | Done, and it is parallelism, not less work |
| Shared traversal frontier for batched matching | [Mnemonic, arXiv:2206.09983](https://arxiv.org/abs/2206.09983) | Section V | Names the redundancy between updates and uses a unified frontier. Enumeration with an index |
| Delta-query plans as the frame for batch matching | Lee et al., SIGMOD 2024, [PACMMOD](https://doi.org/10.1145/3654950) | Search excerpt only, ACM 403 and repository page empty | Common delta-query framework. Unread, so this is a flag, not a finding |

Set aside as not bearing on this direction: Sun et al. [PVLDB 15](https://www.vldb.org/pvldb/vol15/p1403-sun.pdf)
(Table 1 and section 4.8, confirms Graphflow and IEDyn as the batch-capable methods in their study);
[PSMiner](https://wrap.warwick.ac.uk/174543/1/WRAP-PSMiner-A-pattern-aware-accelerator-high-performance-23.pdf)
(introduction, hardware accelerator); [DyPamear](https://doi.org/10.1145/3770743.3804278) (title through Semantic Scholar, PIM hardware);
[AJOSC](https://doi.org/10.1145/3725263) (abstract through OpenAlex, join order for continuous queries);
[Change Propagation Without Joins](https://doi.org/10.14778/3579075.3579080) (abstract through OpenAlex).

## What I would do next

1. Get Gopher and Cheetah through a library login. If Gopher reports exact 5-cycle counts under updates at low cost,
   candidate 1 drops sharply.
2. Rerun the Semantic Scholar screen with OpenAlex after midnight UTC, seeded on Cheetah and Gopher, which were found
   too late to be used as seeds.
3. Only then run the candidate 1 kill test. The reading is cheaper and could settle it first.
