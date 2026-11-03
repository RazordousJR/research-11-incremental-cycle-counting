# Paper outline, draft 1

Written on 2026-11-03 after M1D06A, on the PI's word (`log.md` 0038). Nothing here is prose for the manuscript. It fixes the claim, the figures and what stands between the evidence and a submission. Three decisions in it are the PI's, and they are marked.

## The claim, in one sentence

For exact counting of 5-cycles under batches of edge updates, sharing one row per star of the batch beats the best of a per-edge scan, a tuned wedge table and a mixed table on the clock, on large batches, while for triangles it does not, and the reason is the price of a table probe rather than a count of operations.

## What a reader gets

1. **The star-row kernel at p = 5, engineered and measured.** PR: one row at the centre of each star, adjusted by each spoke, with all costs inside. The mechanism is not new (Cheetah, `log.md` 0019). What is new is its measured gain against the strongest baselines, sequential, exact, with the upkeep counted.
2. **A rule for when sharing pays.** Pattern size 5 and large batches, from 14 scored cells: 6 pass at the strict worst-of-repeats test, 5 of 7 at cut 1,000 and 1 of 7 at cut 100, and none at p = 3 or p = 4.
3. **Why operations mislead here.** WJ does fewer charged operations than PR on most sx cells and runs 3 to 26 times slower. A unit-cost model does not price a random probe into a table of several GB. This is the cost-model section the protocol asks for.

## What it is compared against

R2, the same kernel without sharing; WT, the tuned wedge table; WJ, the flat wedge table; M5, the mixed table with hubs scanned, at its best theta per cell in M1D06A; A5, the affected recount; and at p = 4, B4, a port of BFCB-IG.

## Did it win, and where it did not

Go by the Gate 1 rule, with the trend: median 1.098 at p = 3 and 0.686 at p = 5. All six passes hold against M5 at its best theta. Stated up front, not in the conclusion: the held-out stream, sx-stackoverflow, gives no pass. Two of the six passes rest on a 9% to 13% margin over the scan, on one machine.

## Figures, at least five

| # | Figure | Source | State |
|---|---|---|---|
| 1 | The worked batch | `docs/figures/fig-worked-batch.svg` | Done |
| 2 | The stream timeline | `docs/figures/fig-stream-timeline.svg` | Done |
| 3 | The test set in one figure: every stream with its size, label and batch counts | `data/derived`, M1D04 | To draw. It will show how few streams there are, and that is the point of the readiness check below |
| 4 | Score against pattern size, every cell, both cuts | `scripts/gate1_score.py` | To draw |
| 5 | Nanoseconds per charged operation per arm, and the cells where operations and the clock disagree | M1D04 README | To draw |
| 6 | M5 over theta against PR and R2 on the six passes | M1D06A README | To draw |

## Sections

1. Introduction: the question, the answer, the held-out result in the same paragraph.
2. Problem and model: exact counts, batches, the two stream labels, all costs inside.
3. Kernels: R2 and PR at p = 3, 4, 5, and the baselines as built. Exactly-once is folklore and is cited, not claimed (hard rule 6).
4. Method of measurement: pinned, exclusive claims and what "exclusive" did not exclude, operations then clock, the rule fixed before the clock, M1D04-R1.
5. Results: the verdict, the six passes, the held-out stream, the near misses.
6. The cost model: operations against the clock.
7. Sensitivity: M at its best theta; the scan margin; background load.
8. Limits: one machine, four streams, the novelty boundary with Cheetah.

## Readiness, checked now against bootstrap §5

| Test | State | Gap |
|---|---|---|
| 1 Claims trace to records | Yes | None |
| 2 Ablation per component | Partly | PR against R2 is the sharing ablation. No component beyond that is claimed |
| 3 Baselines tuned on equal terms | Mostly | Tuned table and mixed table at its best theta. A second machine is still open |
| 4 Novelty survived the literature | Narrowly | The gain is new, the mechanism is not |
| 5 Evaluation scope reaches the venue's bar | **Likely fails** | 3 evaluation streams and 1 held-out, against 56 instances in Hanauer et al. The coverage matrix plans about 20 streams and a hard case. Real deletions, from KONECT, are not in at all. The venue's median is not counted |
| 6 Contribution type fits the venue | Open | See decision A |
| 7 Venue-fit record | Not started | M1A17 |
| 8 Outside reader | Not started | After a draft |

## The PI's decisions

**A. Contribution type.** CLAUDE.md says Method. With the mechanism published, what the evidence supports is an experimental algorithmics paper: an engineered kernel, a rule for when it pays, and a cost model. That is between a method and an empirical study, and §3 makes the choice a §9 decision. My recommendation: algorithm engineering, with PR as the artifact a reader adopts, aimed at a venue that publishes experimental algorithmics. **Decided by the PI on 2026-11-03 as recommended (`log.md` 0043).**

**B. Scope before writing.** Test 5 likely fails as things stand. More streams means more `--exclusive` hours, about 4 to 8 per batch of new streams at p = 5 alone, with a fixed protocol extension written first. My recommendation: add streams before writing, at p = 5 and p = 3 only, including KONECT streams with real deletions as the hard case, because the held-out failure makes four streams too few to carry a rule. **Decided by the PI on 2026-11-03 (`log.md` 0040). Protocol: `docs/experiments/m1d07-streams.md`.**

**C. Venue.** M1A17 first, with the §3 record and verified indexing. The proposal named JGAA, unverified. The candidates note named SEA or ALENEX, which are conferences. My recommendation: run M1A17 now, as reading and no machine, and bring two journals with checked exemplars. **Approved by the PI on 2026-11-03, to run as reading during M1D07's count sweep (`log.md` 0043). The venue itself is still the PI's.**
