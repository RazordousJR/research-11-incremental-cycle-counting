# Paper outline, draft 1

Written on 2026-11-03 after M1D06A, on the PI's word (`log.md` 0038). Nothing here is prose for the manuscript. It fixes the claim, the figures and what stands between the evidence and a submission. Three decisions in it are the PI's, and they are marked.

## The claim, in one sentence

For exact counting of 5-cycles under batches of edge updates, sharing one row per star of the batch runs its slowest repeat in at most 0.67 times the fastest of a per-edge scan, a tuned wedge table and a mixed table on 28 of 50 scored cells over 14 of 16 real-timestamp streams, real deletions included, 19 of 25 at batches of 1,000 and 9 of 25 at 100, and on no cell of dewiki or the held-out sx-stackoverflow; for triangles it passes on no cell, though it beats the scan by median on 27 of 36 new cells. The reason is the price of a table probe rather than a count of operations: pricing each kind of operation, fitted on the small engineering streams, names the clock winner on 63 of 64 new cells, against 21 for unit cost, and on all 64 when fitted on the first three streams, though it does not explain the flat wedge table's time and finds no cache-size effect. The baseline it beats is mostly the unshared scan, on both core types of one machine, 28 of 50 on its P-core and 24 of 50 on its E-core, where frwiki and nlwiki drop out, by a margin that a scan 10% faster would bring under the Go clause on the P-core.

*M1D10 read **Carries** on 2026-11-04 (`log.md` 0065), and the cost-model clause now gives the engineering fit first, as its protocol fixed.*

*M1D08 read **Core-stable** on 2026-11-04 (`log.md` 0061), and the protocol adds "on both core types of one machine" and the count per core. The E-core loses only wiki real-deletion cells.*

*Rewritten on 2026-11-04 after M1D07 read **Narrows** (`log.md` 0058), as the protocol requires before any prose. The earlier sentence said "while for triangles it does not". The pooled p = 3 median is now 0.965, under the trend's 1.0, so that half no longer stands as written.*

## What a reader gets

1. **The star-row kernel at p = 5, engineered and measured.** PR: one row at the centre of each star, adjusted by each spoke, with all costs inside. The mechanism is not new (Cheetah, `log.md` 0019). What is new is its measured gain against the strongest baselines, sequential, exact, with the upkeep counted.
2. **A rule for when sharing pays.** Pattern size 5, from 50 scored cells on 16 streams (M1D05 and M1D07 pooled): 28 pass at the strict worst-of-repeats test, 19 of 25 at cut 1,000 and 9 of 25 at cut 100, under all three labels. None pass at p = 3 or p = 4. At p = 3 sharing is faster than the scan on the new streams, by a median of 13%, and never reaches the test.
3. **Why operations mislead here.** WJ does fewer charged operations than PR on most sx cells and runs 3 to 26 times slower. A unit-cost model does not price a random probe into a table of several GB. This is the cost-model section the protocol asks for.

## What it is compared against

R2, the same kernel without sharing; WT, the tuned wedge table; WJ, the flat wedge table; M5, the mixed table with hubs scanned, at its best theta per cell in M1D06A; A5, the affected recount; and at p = 4, B4, a port of BFCB-IG.

## Did it win, and where it did not

Go by the Gate 1 rule, with the trend, on the first 4 streams: median 1.098 at p = 3 and 0.686 at p = 5. All six passes hold against M5 at its best theta. On 12 new streams M1D07 reads **Narrows**: 22 of 36 pass at p = 5, and the pooled Go holds at 28 of 50, but the pooled p = 3 median of 0.965 breaks the trend. Stated up front, not in the conclusion: the held-out stream, sx-stackoverflow, gives no pass, and neither does dewiki. R2 is the best other arm on 43 of 50 cells, and with it 10% faster the pooled passes fall to 16 of 50. All of this is one machine (`results/gate1-M1D07T/README.md`). M1D08 timed the 24 cells near the pass line on a P-core and an E-core of it and reads **Core-stable**: the P-core reproduces the record, arm medians within 1%, and 28 of 50 pass; the E-core passes 24 of 50, on 12 streams, and every wiki real-deletion cell has a worse margin there (`results/gate1-M1D08/README.md`). The two cores share L3 and DRAM, so this is not a second machine.

## Figures, at least five

| # | Figure | Source | State |
|---|---|---|---|
| 1 | The worked batch | `docs/figures/fig-worked-batch.svg` | Done |
| 2 | The stream timeline | `docs/figures/fig-stream-timeline.svg` | Done |
| 3 | The test set in one figure: every stream with its role, label, size and p = 5 passes | `docs/figures/fig-test-set.svg` | Done. 20 streams, 16 of them in a verdict |
| 4 | Score against pattern size, every cell, both cuts | `docs/figures/fig-score-by-p.svg` | Done |
| 5 | The cost model: predicted against measured seconds, unit cost beside priced by kind and cache | `docs/figures/fig-cost-model.svg`, M1D09 | Done from M1D09's fit. Section 6 gives M1D10's coefficients, so whether to redraw it from M1D10's is open |
| 6 | M5 over theta against PR and R2 on the six passes | `docs/figures/fig-theta.svg` | Done |

## Sections

1. Introduction: the question, the answer, the held-out result in the same paragraph.
2. Problem and model: exact counts, batches, the two stream labels, all costs inside.
3. Kernels: R2 and PR at p = 3, 4, 5, and the baselines as built. Exactly-once is folklore and is cited, not claimed (hard rule 6).
4. Method of measurement: pinned, exclusive claims and what "exclusive" did not exclude, operations then clock, the rule fixed before the clock, M1D04-R1.
5. Results: the verdict, the six passes, the held-out stream, the near misses.
6. The cost model: operations against the clock. Claimed: **Carries**, the weaker of two fits that both read Carries. M1D10, fitted on the 12 engineering cells as hard rule 5 says, gives the coefficients (`results/cost-M1D10/README.md`): priced by kind and cache, 63 of 64 new winners at 23.0% median error, against 21 and 71.3% for unit cost. M1D09, fitted on M1D05's streams, is given beside it: 64 of 64 at 22.9% (`results/cost-M1D09/README.md`). A table probe costs about 10 to 18 ns in the two fits, against 0.05 to 0.6 ns for a row or scan step. Stated with it: neither fit shows a cache price, so none is claimed; WJ is underpriced five- to sevenfold in both (0.18 and 0.15 of its clock); and the engineering fit does not carry back to M1D05's streams at p = 3 and 4, where it prefers P to the scan on 14 of 24 cells, while all 12 p = 5 cells are right.
7. Sensitivity: M at its best theta; the scan margin; background load.
8. Limits: one machine, four streams, the novelty boundary with Cheetah.

## Readiness, checked now against bootstrap §5

| Test | State | Gap |
|---|---|---|
| 1 Claims trace to records | Yes | None |
| 2 Ablation per component | Partly | PR against R2 is the sharing ablation. No component beyond that is claimed |
| 3 Baselines tuned on equal terms | Mostly | Tuned table and mixed table at its best theta. A second machine is still open |
| 4 Novelty survived the literature | Narrowly | The gain is new, the mechanism is not |
| 5 Evaluation scope reaches the venue's bar | **Improved, not yet checked against the venue** | 16 real-timestamp streams: 3 evaluation, 1 held-out and 12 new, chosen from Hanauer et al.'s 56 by a rule fixed first, 6 with 16% to 28% real deletions. Their 38 Kronecker streams are left out as fully synthetic. JGAA's median scope is not counted |
| 6 Contribution type fits the venue | Open | See decision A |
| 7 Venue-fit record | Venue decided: JGAA (`log.md` 0064) | `docs/submissions/paper-1/venue-fit.md` predates the decision and is not yet checked against JGAA's scope and length |
| 8 Outside reader | Not started | After a draft |

## The PI's decisions

**A. Contribution type.** CLAUDE.md says Method. With the mechanism published, what the evidence supports is an experimental algorithmics paper: an engineered kernel, a rule for when it pays, and a cost model. That is between a method and an empirical study, and §3 makes the choice a §9 decision. My recommendation: algorithm engineering, with PR as the artifact a reader adopts, aimed at a venue that publishes experimental algorithmics. **Decided by the PI on 2026-11-03 as recommended (`log.md` 0043).**

**B. Scope before writing.** Test 5 likely fails as things stand. More streams means more `--exclusive` hours, about 4 to 8 per batch of new streams at p = 5 alone, with a fixed protocol extension written first. My recommendation: add streams before writing, at p = 5 and p = 3 only, including KONECT streams with real deletions as the hard case, because the held-out failure makes four streams too few to carry a rule. **Decided by the PI on 2026-11-03 (`log.md` 0040). Protocol: `docs/experiments/m1d07-streams.md`.**

**C. Venue.** M1A17 first, with the §3 record and verified indexing. The proposal named JGAA, unverified. The candidates note named SEA or ALENEX, which are conferences. My recommendation: run M1A17 now, as reading and no machine, and bring two journals with checked exemplars. **Approved by the PI on 2026-11-03, to run as reading during M1D07's count sweep (`log.md` 0043). The venue itself is still the PI's.** M1A17 read: `docs/submissions/paper-1/venue-fit.md` recommends JGAA, with KAIS if WoS is needed (`log.md` 0045). **Decided by the PI on 2026-11-04: JGAA, WoS not needed (`log.md` 0064).**
