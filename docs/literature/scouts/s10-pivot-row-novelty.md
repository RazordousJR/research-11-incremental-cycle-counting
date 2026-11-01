# Scout S10: hostile novelty check of the pivot row kernel PR

Date: 2026-11-01. Goal: prove PR is already published. Finding prior art counts as a success. No code was run. Each source
states how far it was read. Everything is paraphrased. "Row" below means a per-vertex count vector built once. "New" means absent
from the audit, S8, S9 and the M1C02 reading files. S1 to S4, S6 and S7 were not read, see the gaps.

## Verdict

**Partly taken.** Sharing one per-vertex row across the updates of a batch is published for triangles (DPTL+, 2021) and for
butterflies (BFCB-IG, 2022). Telescoping across a batch is published for any pattern (delta subgraph queries).
**Not found:** an exact count-only batch kernel for 5-cycles, a mixed net batch, or a row adjusted in place per spoke of a star.
That remainder is a small engineering contribution, and a reviewer can derive it from the parts in one paragraph.

## Closest work

| Work | Read | What it shares | What it does not share, or where it stops |
|---|---|---|---|
| K. Wang et al., "Accelerated butterfly counting with vertex priority on bipartite graphs," The VLDB Journal, 2022, [DOI](https://doi.org/10.1007/s00778-022-00746-0), [PDF](https://d-nb.info/1263104584/34). **New to this project** | Section 7 from its second page to 7.3, Theorems 8 to 11, Algorithms 7 to 9, start of 8.3 | **Closest for a cycle pattern.** BFCB-IG builds hash rows `C_total[w]` and `C_old[w]` once per start vertex over the affected subgraph, so one 2-path count vector serves every batch update touching it. Count is new minus old. BFCB-IG+ splits old and new neighbours and touches old wedges only for end vertices of new wedges. BFCB-BS is the per-edge arm, and it picks the cheaper endpoint by degree sums: that is R2 for butterflies | Bipartite 4-cycles only: two wedges meet, so the row pairs with itself. An odd cycle cannot do that. Rows sit at vertices of the affected subgraph, not at batch pivots. Exactly-once comes from vertex priority, not peeling. Insertions and deletions are separate batches. No row is adjusted between updates |
| Yu, Qin, Zhang, Zhang, Lin, "DPTL+," ICDE 2021, [record](https://opus.lib.uts.edu.au/handle/10453/153684) | Cover letter, Sections I to IV as page images. The audit read the letter and searched the text | Bitmap of N(u) built once per pivot u in V(B), used for all new edges oriented out of u | Triangles, listing, 1-hop row. Every batch vertex is a pivot, no peeling by batch degree. Insert and delete run as two phases. The cover letter argues that its orientation and degree guarantee fail on larger patterns, because several pivot edges would enter the intersection |
| Kankanamge, "Multiple Continuous Subgraph Query Optimization Using Delta Subgraph Queries," Waterloo thesis, [record](https://uwspace.uwaterloo.ca/handle/10012/14051) | Chapter 1 | A batch of updates over any pattern, one delta query per query edge, with edges labelled new, delta or old. **That is the telescoping form.** Plans share equal intersections across delta queries | Sharing is between operators of a plan, not between data updates at one vertex |
| Mhedhbi, Salihoglu, [arXiv:1903.02076](https://arxiv.org/abs/1903.02076); Mhedhbi, Kankanamge, Salihoglu, TODS 46(2), 2021, [DOI](https://doi.org/10.1145/3446980) | arXiv text to 100,000 characters by page summary. TODS abstract only | Intersection cache: the extension operator reuses its last result when the next consecutive partial match needs the same lists | One-time queries in the text read. Keeps one set, by adjacency in the input order. **My inference:** delta tuples sorted by endpoint would hit this cache, so TODS must be read |
| Cheetah, TACO 2025, [DOI](https://doi.org/10.1145/3736173) | Abstract and search excerpts. Addendum read it by keyword | Groups updates into exploration domains and mines each group together. Reports that over 90% of updates share a vertex | **The mechanism below that sentence is unread by me.** General patterns, enumeration, no 5-cycle in its evaluation, per the addendum |
| Gopher, EuroSys 2026, [DOI](https://doi.org/10.1145/3767295.3769365) | Addendum and reading.md only | DAG of set operations, common subexpressions merged at compile time. Reports 5-MC | Whether merging crosses updates is not stated in what was read |
| Nikolic, ElSeidy, Koch, LINVIEW, SIGMOD 2014, [DOI](https://doi.org/10.1145/2588555.2610519), [PDF](https://www.cs.ox.ac.uk/files/9131/sigmod2014-linview.pdf). **New** | Sections 1 to 5.2.1 | A row or column change is rank 1. Deltas of `A^k` stay factored, so evaluation is matrix-vector only. Its running example is `A^4` | Materialises every power, dense. No cycle counts in the pages read. **My reading:** a star at x is a rank-2 change, row A is `A^2 e_x`, and the main term is `e_x^T A^4 d`. So PR is a low-rank delta with no materialised power |
| Kara, Nikolic, Olteanu, Zhang, F-IVM, [arXiv:2303.08583](https://arxiv.org/abs/2303.08583) | Sections 6, 7.4 by page summary, to Section 8.1 | Updates that factor into vectors, pushed past joins. Matrix row update written as a product of two vectors. Triangle and chorded 4-cycle view trees | Maintains views. Bulk updates across relations go one relation at a time. No grouping by variable seen, no cycle experiment in the part read |
| Hanauer, Henzinger, Hua, SAND 2022, [arXiv:2106.15524](https://arxiv.org/abs/2106.15524); Hanauer, Heck, Henzinger, Sidl, [arXiv:2610.29776](https://arxiv.org/abs/2610.29776) | First: pages 2 to 4. Second: pages 1 to 7, 10 to 11 | **Mixed kernel for 4-cycles.** Tables count paths through low-degree vertices, and subgraphs with a high-degree vertex are scanned. HHH\*-H keeps high-degree anchors only | Single updates. No 5-cycle upper bound: Table 1 of the first has only a lower bound for k at least 5. The second recommends tables for 4-cycles and scanning for triangles, and reports 26.2 times over Graphflow on 4-cycles |
| Yuan, Hao, Lin, Zhang, "Batch Hop-Constrained s-t Simple Path Query Processing in Large Graphs," [arXiv:2312.01424](https://arxiv.org/abs/2312.01424), [IEEE record](https://ieeexplore.ieee.org/document/10597957/). **New** | Pages 1 to 4 | 5-cycles through (u,v) are the simple 4-hop u-v paths. This paper shares materialised partial paths across a batch of s-t queries, and motivates it by a new transaction closing a cycle | Static graph, directed, enumeration. Shares paths, not count vectors. Optimal sharing is NP-hard there. The arXiv page gives no venue |
| Cannoodt et al., "IncGraph," PLOS ONE 13(4), 2018, [DOI](https://doi.org/10.1371/journal.pone.0195997), [code](https://github.com/rcannood/incgraph). **New** | Full article by page summary, README | Exact delta of graphlet and orbit counts up to 5 nodes, per edge, add or remove | Per edge, no batch or sharing. Enumerates to depth 3, on bit-packed adjacency for small networks. **Dedicated, exact and 5-node, but not count-only** |
| dynG `cycle_count`, [repo](https://github.com/dyng-dev/dyng), [doc](https://raw.githubusercontent.com/dyng-dev/dyng/main/docs/algorithms/cycle_count.md); Khanda, Shovan, Satpathy, Das, "TruCy," submitted to IEEE TC, 2026, unpublished. **New** | Doc page by page summary, README, release notes. Paper not found | Exact k-bounded directed cycle histograms under a net batch. A cycle through several changed edges belongs to the smallest change id. Apache-2.0 | Enumerates paths per changed edge, cost exponential in k. The doc says earlier searches are not reused. Directed. **My inference:** a symmetric digraph gives twice the undirected count at k = 5 |
| Hriday G, Sista, Das, [arXiv:2308.14493](https://arxiv.org/abs/2308.14493) | All 5 pages | Batch recount of the region around changes: `C' = PGD(G_a) - PGD(G_b)` | **Size 4 only.** A search summary claimed all 5-vertex subgraphs, which is false |
| Dhulipala et al., [arXiv:2003.13585](https://arxiv.org/abs/2003.13585) | Sections 4, 6 by page summary, to 100,000 characters | Wedge table keyed by high-degree pairs, five buckets per batch-edge count | Triangles and cliques. No grouping by endpoint stated |
| Burkhardt, Harris, [arXiv:2303.06090](https://arxiv.org/abs/2303.06090); Huang, Shi, Shun, SEA 2021, [PDF](https://jshun.csail.mit.edu/fivecycle.pdf); Pinar et al., ESCAPE, [arXiv:1610.09411](https://arxiv.org/abs/1610.09411) | Excerpts; pages 1, 2, 17; abstract. reading.md has the JEA text | The per-vertex row with reset, static: O(n) space for 4-cycles, `U_v` for 5-cycles | Static. These show the row itself is standard |
| Others, no overlap found | Below | Khan et al., [SC25 poster](https://sc25.supercomputing.org/proceedings/posters/poster_files/post308s2-file3.pdf), read in full: k at most 4, each edge independent. Censor-Hillel, Kolobov, Schwartzman, [arXiv:2009.08208](https://arxiv.org/abs/2009.08208), excerpt: distributed 5-cycle listing. Assadi, Shah, [arXiv:2504.10748](https://arxiv.org/abs/2504.10748), excerpt: 4-cycle theory. Zhang et al., ICDE 2023, [PDF](https://fanzhangcs.github.io/papers/2023_icde_simplepath.pdf), pages 1, 2: persistent k-st path index for fixed pairs | |

## Answers to the six questions

**Q1. Yes for 4-cycles and triangles, not found for 5-cycles.** BFCB-IG shares a 2-path count vector, DPTL+ a 1-hop row. For patterns of
5 or more vertices the nearest are Cheetah and Gopher, whose mechanism I could not read. No source read keeps a row and adjusts it per
spoke. The intersection cache is the nearest thing to reuse across consecutive inputs.

**Q2. Telescoping is taken. Star peeling as such is not found.** Delta subgraph queries are the telescoping form for any pattern. dynG's
smallest-id ownership is the same idea in code. Applying star groups one after another is sequential application with a coarser step, so
exactly-once needs no new proof. Family by family, none shows peeling by batch degree:

- **Delta and IVM.** Kankanamge read. Budiu et al., DBSP, PVLDB 16(7), 2023, [PDF](https://www.vldb.org/pvldb/vol16/p1601-budiu.pdf), search
  excerpts. Kara et al., [arXiv:2004.03716](https://arxiv.org/abs/2004.03716), abstract: single-tuple updates, triangle queries. DBToaster,
  [PVLDB 5](http://vldb.org/pvldb/vol5/p968_yanifahmad_vldb2012.pdf), title only. Olteanu and Zavodny not opened.
- **Dataflow.** McSherry et al., Shared Arrangements, [arXiv:1812.02639](https://arxiv.org/abs/1812.02639), excerpt. Whether a key is joined
  once per batch was not established.
- **Matrix.** LINVIEW and F-IVM read. Alon, Yuster, Zwick, [record](https://collaborate.princeton.edu/en/publications/finding-and-counting-given-length-cycles-2/),
  title only. My reading: the base formula is the edge form of their trace identity, from memory. GraphBLAST,
  [arXiv:1908.01407](https://arxiv.org/abs/1908.01407), and Milakovic et al., [arXiv:2111.09947](https://arxiv.org/abs/2111.09947), excerpts:
  static masked products. One search found no incremental or delta SpGEMM. Azad et al. not opened.
- **Butterflies.** Wang et al. read. Shi, Shun, [arXiv:1907.08607](https://arxiv.org/abs/1907.08607), excerpt: static grouping by end points.
  Sanei-Mehri et al., [KDD 2018](https://sariyuce.com/papers/sigkdd18.pdf), excerpt: static and estimators. ABACUS, [arXiv:2312.03435](https://arxiv.org/abs/2312.03435), excerpt: sampling.
- **Bera.** Bera, Pashanasangi, Seshadhri, [arXiv:1911.05896](https://arxiv.org/abs/1911.05896), excerpt: static. **Paramonov et al.:** no match in one
  search. Please give a citation.

**Q3. None that is count-only and formula-based.** Three dedicated or nearly dedicated exact routes exist and all enumerate: IncGraph (per edge,
all 5-node graphlets), dynG (bounded directed cycles, net batch), Gopher 5-MC (through a mining system). Theory: no upper bound found. OMv gives a
lower bound for k-cycle detection (Hanauer et al. 2022). Censor-Hillel et al. list 5-cycles in a distributed model.

**Q4. Published in substance for 4-cycles, not for butterflies or 5-cycles.** HHH and HHH\* table the paths through low-degree vertices and
scan high-degree ones, and Dhulipala et al. key a table the same way. The planned baseline is this idea. Cite HHH and do not call it new.
Butterflies: BE-Index ([arXiv:2001.06111](https://arxiv.org/abs/2001.06111), excerpt) is for bitruss work, not count maintenance, and Wang et al. use no index in the batch path. Unresolved
title: "Wedge-index based butterfly counting in bipartite graphs," Information Sciences, 2026, seen only in
[Semantic Scholar's citing list](https://api.semanticscholar.org/graph/v1/paper/DOI:10.1007/s00778-022-00746-0/citations). One search could not find it.

**Q5. No follow-up found, weak evidence.** The paper is 11 days old, v1 only. arXiv's API for Sidl or Heck returned 2 records, this paper and an
unrelated k-means paper. Its conclusion names neither batches nor 5 vertices. One web search found nothing newer.

**Q6. Few, and batching helps fewer.**

- **ERGM.** `cycle(k)` exists for any length ([docs](https://www.rdocumentation.org/packages/ergm/versions/4.8.1/topics/cycle-ergmTerm)). Butts,
  "Cycle Census Statistics for Exponential Random Graph Models," 2006, [record](https://escholarship.org/uc/item/4719h8d2), abstract through search,
  gives change-score algorithms. **But the ergm material I saw, as search excerpts, describes Metropolis proposals that toggle one dyad, so the batch is 1
  and PR becomes R2.** One search found no empirical fit using a 5-cycle term. I could not locate the C source of the term.
- **Gene networks.** IncGraph's use is real: greedy edge selection with a graphlet penalty, with F1 gains of 7.6% and 2.2%. It needs all 5-node
  graphlets, one edit at a time.
- **Fraud.** Huang et al. call 5-cycles important for fraud and cite Qiu et al., [PVLDB 11(12), 2018](https://www.vldb.org/pvldb/vol11/p1876-qiu.pdf),
  excerpt only. That work detects directed hop-constrained cycles through a new edge. It reports cycles. It does not count undirected 5-cycles. Kankanamge mentions an
  unnamed financial firm searching cycle patterns of up to eight edges. So fraud wants directed, local reports, not a global count.
- **Chemistry.** Chordless cycles are rings (Uno, Satoh, [arXiv:1309.1051](https://arxiv.org/abs/1309.1051), excerpt), on small static molecules. No dynamic need found.
- **Honest position:** I found no application that needs a global exact undirected 5-cycle count under batched updates.

## What can be claimed

If Gate 1 holds, and with the citations above:

1. An exact count-only batch kernel for undirected 5-cycles with mixed net batches, no persistent index and row-sized memory, stated as an
   adaptation of per-vertex batch rows from triangles and butterflies to an odd cycle. "To our knowledge", never "first".
2. The in-place row adjustment per spoke, as the mechanism that the even-cycle form does not need. This is the only unpublished detail I can name.
3. A measured study on labelled real streams: row kernels against tuned wedge tables, operations and clock together, with memory. Bounds on when
   sharing pays are worth more than the mechanism. Hanauer's 4-cycle result gives a useful contrast.
4. Possibly a short note on the low-rank reading of a star, once a person checks it against LINVIEW and Alon, Yuster, Zwick.

## What cannot be claimed

- That sharing a row across a batch is new. Cite DPTL+ and BFCB-IG. Ediger et al. 2010 grouped by vertex earlier, per the audit.
- That exactly-once by peeling is new. Cite delta subgraph queries and Ammar et al.
- That grouping by batch vertex, lower-degree choice, net batches or the per-vertex row are new. Cite DPTL+, BFCB-BS, Dhulipala et al., ESCAPE.
- "First exact dynamic 5-cycle counting". Gopher, IncGraph and dynG each block it in some form.
- That the table is beaten. Operations favour it on 16 of 22 cells, HHH-style mixing is unbuilt, and tables win for 4-cycles in Hanauer et al.
- Any speed ratio against Gopher, or that an application needs this kernel.

## Baselines this check exposes

1. **Affected-subgraph recount:** run RC on the region before and after, as IGC does with PGD and DPTL's AOT\* baseline does. For 5-cycles the
   region is the 2-hop ball of the changed vertices, my derivation. Large batches are where it may win. A reviewer of either paper will ask for it.
2. **BFCB-IG port:** per-vertex row on that region, new minus old, with a stream side for the odd cycle.
3. **HHH-style table** in place of the planned fallback: wedges through low-degree middles only, hubs scanned.
4. **Batch size 1:** report PR against R2 so the ERGM reader sees the tie.

## Gaps in the search

**"Not found" here is weak. Keyword search has already missed DPTL+ and Gopher's repository in this project, and none of BFCB-IG, IncGraph or dynG is in the
audit, S8, S9 or the M1C02 reading.** Wang et al. surfaced only because the query named butterflies. An unnamed 5-cycle kernel in 2026 proceedings would be invisible to me.

- **Unread and decisive:** Cheetah and Gopher mechanism sections, and TODS 46(2) on delta plans. ACM returned 403 to earlier scouts, so I did not retry.
- **S1 to S4, S6, S7 not read.** Their filenames appear in no file I read, and listing the folder is barred by the machine rule. S5's first 40 lines were read. A "New" tag or a
  correction below may repeat something those reports already hold. The coordinator should check this against them.
- **Rate limits, stopped at once:** OpenAlex 429 with Retry-After of about 14 hours after one success, so no forward citations for DPTL+, Huang et al. or HHH.
  Semantic Scholar 429 after one success. Wang's 26 citers were screened by title only: none exact and dynamic.
- **Not tried, by rule:** DBLP, IEEE full text. No bot check, login or paywall was met or passed.
- **Read level:** "page summary" means a fetch tool's summary, not my line reading. PDFs were read as page images on the pages named. Excerpts are search snippets.
- **Survey:** Yin et al., [arXiv:2503.19573](https://arxiv.org/abs/2503.19573), first 100,000 characters: dynamic counting is excluded from scope, so its silence means nothing.
- **Title only:** DyPamear, DAC 2026, seen as a citation entry at Gopher's DOI. PSMiner, [record](https://wrap.warwick.ac.uk/id/eprint/174543/), abstract through search.
- **Copies:** the Wang PDF is the German National Library copy, listed by search. Springer was not tried, so this was no route around a refusal. Fetched PDFs sit in the Claude
  tool-results folder, none in the project. These citations are not yet in a checked `.bib`.
- **Search volume:** 38 web searches, and 2 arXiv API queries, which returned usable lists.

## Corrections to earlier notes

- `results/screen-c-M1C02/reading-addendum.md` says the surviving gap is a dedicated count-only kernel. Narrow it further: dedicated exact routes exist (IncGraph, dynG), so the gap is a
  **formula-based kernel with no enumeration, for batches**.
- `results/screen-c-M1C03/README.md` calls the mixed kernel the obvious next baseline. It is HHH's idea for 4-cycles, so credit it.
- The audit's correction names DPTL+ as the sharing precedent. BFCB-IG is closer, and it is a cycle.
- Huang et al.'s fraud sentence should not be cited for 5-cycle demand. Its source is about directed cycle detection.

## Novelty score: 4 of 10

The mechanism is a composition: a static row (ESCAPE), a batch row at a vertex (DPTL+, BFCB-IG), telescoping (delta queries), and a low-rank
view (LINVIEW). Demand is thin, and the one clear dynamic user (ERGM) gets nothing from batches. It is not lower because no direct hit turned up after a hostile search, the nearest
works stop at triangles and even cycles, and DPTL's authors say their method stops there. The operations reading also leaves the performance case open.

## The falsifier

**One source, paper or code, showing an exact 5-cycle or 5-vertex pattern update that computes a per-vertex wedge or 2-path vector once and reuses it across several updates of one batch.**
Look first in Cheetah's group-based mining section, then Gopher's DAG merging, then TODS delta plans. A person with UTeM ACM access can settle these in about an hour. If Cheetah shares
set intersections across updates at a common vertex, this claim shrinks to the count-only kernel and its measurements.

## Search log

**Terms.** Exact dynamic 5-cycle; batch butterfly wedge sharing; dynamic 4-cycle batch; Hanauer follow-up; delta star batches; Graphflow delta plans; mixed butterfly index; 5-cycle update time; TruCy;
bounded cycle updates; rank-2 vertex update; DPTL+; ergm `cycle`, twice; Butts cycle census; 5-cycle applications; masked and incremental SpGEMM; count-only mining systems; bitruss index; k-cycle lower
bounds; 5-node orbits; Paramonov; two-hop reuse; Cheetah; PSMiner, DyPamear; shared arrangements; wedge-index paper; motif survey; Burkhardt, Harris; DBSP; batch s-t paths; path counting under updates;
highly dynamic listing; chordless cycles.
