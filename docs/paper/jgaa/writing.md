# Writing record, bootstrap §22, steps 1 to 3

Written on 2026-11-05 after the PI's word "Okeh aku approved all. Ko buatkan semua tu beb" (`log.md` 0067). That word is read as approving this sentence outline (step 3) so that drafting can go on. The PI can still change any line here, and the draft follows.

## Step 1: the eight questions, answered from the records

| # | Question | Answer | Record |
|---|---|---|---|
| 1 | What is the paper trying to do? | Find out, for exact sequential counting of triangles, 4-cycles and 5-cycles under batches of edge insertions and deletions, whether sharing work between the updates of one batch beats the strongest baseline that does not share, and explain why operation counts and the clock disagree on the winner | `docs/experiments/gate1-protocol.md`, The question |
| 2 | Why is it worth doing, and for whom? | Batch sharing is published (Cheetah) with gains reported against baselines that are not the strongest. An engineer choosing a counter for a temporal stream needs to know when sharing pays against a well-implemented scan or table, and an algorithm designer needs to know that unit-cost operation counts pick the wrong winner here | `log.md` 0019; `docs/literature/r10-falsifier-reading.md`; outline, What a reader gets |
| 3 | What does it claim? | (1) A star-row kernel, PR, engineered and measured with all costs inside. (2) A rule: sharing pays at p = 5 and not at p = 3 or p = 4, with the cells by name. (3) A cost model priced by kind of operation that names the clock winner where unit cost does not. Not claimed: the sharing mechanism, the exactly-once argument, heavy-light partitioning, wedge indexes | outline, What a reader gets; `CLAUDE.md` hard rule 6 |
| 4 | Main result, with its number | At p = 5, PR's slowest repeat is at most 0.67 times the fastest repeat of every other arm on 28 of 50 scored cells over 14 of 16 real-timestamp streams; no cell passes at p = 3 (0 of 48) or p = 4 (0 of 12). The cost model fitted on three small streams names the clock winner on 63 of 64 cells of 12 unseen streams, against 21 for unit cost | `results/gate1-M1D07T/README.md`; `results/gate1-M1D05/README.md`; `results/cost-M1D10/README.md` |
| 5 | Compared against what, and did it win? | The per-update scan, a tuned and a flat wedge table, a mixed heavy-light table at its best theta, an affected recount, and at p = 4 a port of BFCB-IG. At p = 5 it wins on 28 of 50 by the strict test; the arm it beats is the scan on 43 of 50 cells; the tables are never close. At p = 3 it beats the scan by median on 27 of 36 new cells and passes on none | M1D07T README, Read beside it; M1D05 README |
| 6 | What is new against the closest work? | Cheetah publishes star grouping with reuse and the trend; Hanauer et al. engineer single-update counters and find the scan best for triangles. New here: the measured gain of batch sharing against the strongest sequential baselines, exact, with upkeep counted, on 16 real streams with real deletions, a held-out stream, and a cost model that explains why operation counts mislead | `log.md` 0019; audit row 1; outline |
| 7 | Where does it not hold? | No pass on the held-out sx-stackoverflow or on dewiki. A scan 10% faster takes the pooled passes from 28 to 16 of 50. One machine: on an E-core of it the passes fall to 24 of 50, all the loss on Wikipedia real-deletion cells. The cost model does not explain the flat table's time (WJ predicted at 0.15 of its clock), finds no cache-size effect, and the engineering fit picks the wrong p = 3 and 4 winner on 14 of 24 cells of the first three streams | M1D07T; M1D08; M1D10 READMEs |
| 8 | For each objective, which result answers it? | Objective 1, does sharing pay: Section 6 (Results), the pass counts by p. Objective 2, why operations mislead: Section 7 (Cost model). The conclusion states both with their numbers | this outline |

## Step 2: figure storyboard

1. **A worked batch:** one batch of a real stream, its stars, and the row that PR builds once for a star and adjusts per spoke.
2. **The stream timeline:** the 5-cycle count after each batch and the cumulative operations of five kernels over a whole stream, so the reader sees a dynamic run. Operations, not time.
3. **The test set:** every stream, with its role, label, size and p = 5 passes, so the breadth of the evaluation is visible at once.
4. **Score against pattern size:** sharing's score falls from about 1 at p = 3 to 0.65 at p = 5, and only p = 5 cells cross the 0.67 line.
5. **The cost model:** unit cost scatters the table arms far below the diagonal, while pricing by kind puts all arms but the flat table near it, under either fit.
6. **The mixed table over theta:** no theta brings the mixed table near PR on the six first passes.

Read in order: what the kernel does, that it runs on real streams, what it was tested on, where it pays, why the clock and the counts disagree, and that the strongest table baseline does not rescue the comparison.

## Step 3: sentence outline, one sentence per planned paragraph

**Abstract.** One paragraph: the problem, the kernel, the test set, the pass counts by p, the held-out failure, the cost-model result, the limit to one machine.

**1 Introduction.**
1. Counting triangles and short cycles in a graph that changes in batches is a basic task in streaming graph analysis, and exact sequential counters for single updates have been engineered carefully.
2. When a batch arrives, several updates often share an endpoint, so work done for one update can be reused for the next, and systems such as Cheetah report gains from this.
3. Those gains were measured against baselines that do not represent the best a sequential exact counter can do, so it is open whether sharing pays against a well-tuned scan or table.
4. We ask that question for triangles, 4-cycles and 5-cycles, with every cost inside the time and a pass test fixed before any clock.
5. The answer depends on the pattern size: at five vertices sharing passes on 28 of 50 cells, and at three and four vertices on none, and the one stream held out to the end gives no pass.
6. Unit-cost operation counts name the wrong winner because a random table probe costs about twenty to two hundred times a sequential scan step, and a model priced by kind of operation recovers the clock winner on unseen streams.
7. Contributions, as a list of three, and what is not claimed.
8. Organisation of the paper.

**2 Related Work.** Paragraphs on exact dynamic counting of triangles and four-vertex subgraphs; batch-dynamic counting; continuous subgraph matching and batch sharing systems; static cycle counting used here; and a table of the closest work with what each does and what this paper adds. (Written from the verified bibliography.)

**3 to 5.** Drafted in `sec2-4.tex`.

**6 Results.**
1. The verdict by the rule fixed first is Go at p = 5 on the first three streams, with 6 of 14 cells passing.
2. On twelve new streams, 22 of 36 p = 5 cells pass, and pooled with the first set 28 of 50 pass on 14 of 16 streams.
3. Passes depend on the batch size: 19 of 25 at batches of 1,000 and 9 of 25 at 100.
4. The held-out stream, sx-stackoverflow, and dewiki give no pass.
5. At p = 3, sharing beats the scan by a median of 13% on the new streams and never reaches the test, and at p = 4 no cell passes either.
6. The arm that sharing beats is almost always the scan, R2 on 43 of 50 cells, and the tables are never close.

**7 Why Operation Counts Mislead: a Cost Model.**
1. On 60 of 76 new cells, operation counts and the clock name different winners.
2. We fit three linear models of the clock on the operation counts: unit cost, a price per kind, and a price per kind with extra prices for tables larger than the last-level cache.
3. Fitted on the three engineering streams and read on the twelve new streams, the price-by-kind model names 63 of 64 winners, and unit cost 21.
4. A table probe costs about 10 to 18 ns in the two fits, and a row or scan step 0.05 to 0.6 ns.
5. The cache-size term adds nothing in either fit, so the data do not show that tables outgrowing the cache make them slow.
6. The model does not explain the flat table, which runs five to seven times slower than predicted, and the engineering fit picks the wrong winner at p = 3 and 4 on 14 of 24 cells of the first three streams.

**8 Sensitivity.**
1. The mixed table at its best theta per cell, chosen after the fact, does not remove any of the six first passes.
2. A scan 10% faster would take the pooled passes from 28 to 16 of 50.
3. Background load measured after each claim moves overlapping repeats by at most a fraction of a percent.
4. On an efficiency core of the same processor, 24 of 50 pass, and every loss is a Wikipedia real-deletion cell.

**9 Limits and Conclusion.**
1. All timing comes from one machine, and the efficiency-core run does not replace a second machine.
2. Sixteen real streams is few beside static benchmark sets, because timestamped streams with deletions are scarce.
3. The mechanism is not new, and the contribution is its measured gain and the cost model.
4. Sharing pays for exact 5-cycle counting under batches, against the strongest sequential baselines, on most of the streams tested, and not for triangles or 4-cycles.
5. An engineer should price a table probe, not count operations, when choosing among these kernels.
