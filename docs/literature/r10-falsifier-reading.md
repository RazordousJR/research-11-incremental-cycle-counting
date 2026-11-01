# R10 reading: Cheetah, Gopher and TODS 46(2), 2026-11-01

Read by the coordinator in the PI's Chrome, through the publisher's pages, on the PI's word (`log.md` 0018). Nothing was downloaded. This settles the S10 falsifier: does a 5-vertex
update reuse a per-vertex vector across a batch? It is a primary reading, unlike S10.

## Verdict

**Strict falsifier: not fired.** None of the three keeps a per-vertex count vector across a batch and adjusts it per spoke. None counts without enumerating.

**Broad falsifier: fired by Cheetah.** Star grouping of a batch, with per-group reuse, on 4-vertex and 5-vertex patterns, is published. So is the finding that triangles gain least. Candidate 1
loses more of its novelty than S10 expected.

## Cheetah, TACO 22(2), 2025, [DOI](https://doi.org/10.1145/3736173)

Read: sections 1 to 9 in full on the ACM page, open access.

- **Mechanism.** Updates are placed in exploration domains built from the previous batch's matches, then grouped into update-groups, each a star around a recurring vertex. This is the same grouping as
  the PR kernel. Each group keeps a neighbor cache of sorted adjacency lists. A canonical buffer holds one candidate vertex sequence, and that sequence extends many update-subgraphs after a single symmetry
  check.
- **What differs from PR.** The cache holds neighbor lists, and results are enumerated matches followed by isomorphism tests. There is no count vector, no formula and no in-place adjustment per spoke.
- **The result that matters here (Table 2, section 7.2).** Against Tesseract, mean speedup is 1.12× for TC, 2.53× for 4-CF, 2.66× for 4-MC, 2.72× for 5-CF and 2.75× for 4-FSM. The authors attribute the low TC
  figure to few vertices and little reuse. **That is the Gate 0 finding in one sentence, in a different setting.**
- **The gain does not grow from 4 to 5 vertices** in these figures: 4-CF 2.53×, 5-CF 2.72×.
- **Against a pattern-aware baseline the gain shrinks.** The authors note that GraphPi-D stays close to Cheetah on TC, 4-CF and 5-CF on MO and WK. My arithmetic on Table 2 for GraphPi-D / Cheetah: TC 1.02 to 1.11;
  5-CF 1.03 to 1.57; 4-CF 0.89 (GraphPi-D faster on MO) to 1.45, leaving out SO. The SO cell for GraphPi-D is 376.57, the same value as the Delta-BigJoin cell, so it may be a copying error. The headline 2.7× is
  against the weaker baseline.
- **Limits of its comparison.** Times exclude preprocessing, compilation and applying updates to the graph, which `CLAUDE.md` hard rule 7 forbids here. It runs on 16 nodes with 28 cores each, with MongoDB holding the
  graph, and the default batch is 100K. Per-update time falls with batch size and plateaus past 100K (section 7.6). The patterns are cliques, motifs and frequent subgraphs. No plain cycle is reported.

## Gopher, EuroSys 2026, [DOI](https://doi.org/10.1145/3767295.3769365)

Read: sections 2.3 to 4.3 in full, and a keyword scan of the whole text for batch and inter-update terms. Sections 5 to 7 are not read in full.

- **Mechanism.** Sharing is across **mapping instances of one updated edge**, meaning the positions that edge can take in the pattern. DAGs of set operations are built per instance and merged by fingerprint at
  compile time. Output is embeddings, restored through canonical expressions.
- **Across updates.** Nothing found. "Batch" appears only as the default batch size, 10K, in the setup.
- **Useful to us.** Redundant intersections are 74% and 81% of the work on WK and BC, and they rise with pattern complexity. The abstract reports speedups of up to 35.99× over Cheetah. So on the same authors' setup, sharing
  inside an update beats sharing across updates. The 5-cycle formula already removes most of that inner redundancy, which is why it starts from a far lower base.

## Mhedhbi, Kankanamge, Salihoglu, TODS 46(2), 2021, [DOI](https://doi.org/10.1145/3446980)

Read: sections 2.2, 4 to 4.3 and the start of 6.3 in the ACM HTML, and every use of cache and batch in the full text.

- **Mechanism.** Delta Generic Join splits a query into one delta query per query edge, which is telescoping. A combined plan shares operators across delta queries and across registered queries. Partial intersection
  sharing reuses part of an intersection across operators.
- **Across updates.** The text assumes the update set is small, a few edges. Experiments insert batches of 5. Sharing is between plans, not between updates of a batch.
- **Intersection cache (section 3.1).** An operator reuses its intersection for the last tuple it extended. My inference in S10 stands as an inference: delta tuples sorted by endpoint would hit this cache. The text does not
  say so. It would hold one list, not a count vector, and it would not adjust in place.
- **5-cycle.** The continuous query sets are cliques, diamonds and SEED queries. No 5-cycle in the continuous sections read.

## What this changes

| Item | Before | After |
|---|---|---|
| PR as a mechanism | Novelty 4 (S10) | **3 of 10.** Star grouping with per-group reuse is published. Only the count-only row adjusted per spoke remains unfound |
| Candidate 1, "triangles fail, larger patterns pay" | Offered as the study's finding | **Already published as a trend**, for a distributed mining system, in one sentence and one table |
| What the study can still add | | The gain against the strongest baseline with all costs inside; a closed-form kernel, not enumeration; one sequential code base across pattern sizes; and operations beside clock |

"Not found" remains weak. Three papers were read, and only by the sections named above.
