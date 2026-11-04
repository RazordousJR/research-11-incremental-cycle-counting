# Figure captions

The text that used to be drawn inside each figure, verbatim, for the paper's LaTeX captions. Written by `scripts/figures.py` and `scripts/figures_results.py` from the same logged runs as the figures; do not edit by hand.

## Figure 1: `fig-worked-batch.svg`

Caption lines:

> sx-askubuntu, real timestamps, synthetic expiry, cut 1,000, p = 5. Batch 465 of 1,454: 1,000 raw events, 667 net edits.
> Count build, M1D03 rows for PR and R2; PR's batch reproduced exactly by build/worked_batch.exe. Operations, not time.

## Figure 2: `fig-stream-timeline.svg`

Caption lines:

> sx-askubuntu, real timestamps, synthetic expiry, cut 1,000, p = 5, 1,454 batches.
> Count builds: M1D03 for PR, R2, WT and WJ; M1D04 for M5 at theta 128. Operations, not time.
> Every arm's count agrees on every batch. The expiry rule empties the graph by the end.

## Figure 3: `fig-test-set.svg`

Title line:

> The test set: every stream, its role, label and size, and its p = 5 passes

Caption lines:

> Black: streams in a verdict. Grey: engineering streams and the snapshot. Events of the insert-only or real-deletion stream; an expiry stream has about twice as many.
> KONECT wiki streams: 2,000,000-event prefixes, real deletions at the share shown. Pooled p = 5, P-core: 28 of 50 scored cells pass (M1D07E). sx-stackoverflow is the held-out stream.

## Figure 4: `fig-score-by-p.svg`

Title line:

> Score against pattern size: sharing's median over the best other arm's, every scored cell

Caption lines:

> Squares: M1D04's 4 streams (p = 4 only there). Circles: M1D07's 12 new streams. Filled: passes Gate 1's test, slowest repeat at most 0.67 x best's fastest.
> Bar: median. Under 1.0 sharing is faster by median. Timing builds, CPU 8. Scored as gate1_score.score, reading M1D04-R1; the snapshot is left out.

## Figure 5: `fig-cost-model.svg`

Title line:

> The cost model on the 12 new streams: predicted seconds against measured

Caption lines:

> (a) and (b) fitted on the 3 engineering streams (M1D10), (c) on M1D05's 3 streams (M1D09); all drawn on M1D07's 12, which no coefficient saw.
> Clock winner named: (a) 21 of 64 cells, (b) 63, (c) 64. Median error 71.3%, 23.0% and 22.9%. The table-size term adds nothing over pricing by kind in either fit,
> and both put WJ at a fifth of its time or less. In (a), 38 predictions under 0.03 s sit on the floor.

## Figure 6: `fig-theta.svg`

Title line:

> M5 at every theta against PR and R2, on M1D05's six passes: time over R2's median

Caption lines:

> Solid black: PR. Grey: R2, at 1. Dashed with squares: M5 at theta 16 to 1,024 (x axis, log). Medians of the timing repeats, CPU 8, M1D06A; y log scale.
> No theta brings M5 under R2 by enough to turn a pass, so all six passes hold against M5 at its best theta (log.md 0035).
