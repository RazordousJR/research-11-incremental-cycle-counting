# M1D09: the cost model, operations against the clock

Written on 2026-11-04, after M1D08 read Core-stable (`log.md` 0061), before any coefficient was fitted. The PI said to go on with the cost model and the figures on 2026-11-04 (`log.md` 0062). It needs no machine time: every input is a logged count build and a logged timing run. Any change to what is fixed here gets a new ID and a `log.md` entry.

## The question

The outline's section 6 says the reason PR wins is "the price of a table probe rather than a count of operations" (`docs/paper/outline.md`). So far that rests on one number per arm, nanoseconds per charged operation. Operations and the clock name different winners on 60 of M1D07's 76 cells. M1D09 asks: **does a model that prices each kind of operation, and prices a table access by whether the table fits in the last-level cache, predict the clock winner on streams it was not fitted on?** If it does, the section has a cost model. If it does not, the section says operations mislead and stops there.

## Data, by rule

- **Arm-cell.** A (stream, variant, cut, p, arm) with status OK in its count build and in every timing repeat. Seconds are the median of the timing repeats on logical CPU 8, the P-core. Operations are the count build's 10 counters: `scan, probe, row, deg, tab_read, tab_write, graph, net_ops, group, rc`. Their sum is `ops`. The `upkeep` column is not an eleventh: `benchmarks/gate1.cpp` adds to it the share of the others spent on M's hub upkeep, so it is already inside them, and it is left out of every model. `table_bytes` is the count build's. An arm-cell whose median is under 0.05 s is left out, as in M1D08, because it is measured in clock ticks. A cell enters the winner reading when at least two of its arm-cells remain.
- **Fitting set.** M1D05's three evaluation streams, sx-askubuntu, sx-superuser and wiki-talk-temporal, at p = 3, 4 and 5. Count builds come from M1D03, and for M at its fixed theta from M1D04's `count/`. Timing comes from M1D04.
- **Evaluation set.** M1D07's 12 new streams at p = 3 and p = 5, with the snapshot left out. Count builds from M1D07, timing from M1D07T. No coefficient sees them.
- **Left out.** sx-stackoverflow, because only M has a count build there. M1D06A's theta grid, because the theta is fixed per p in M1D04 and M1D07. M1D08, because it has no count build of its own.
- **Hard rule 5, as read here.** The rule says parameters are chosen on engineering streams. The engineering streams have no timing for most arms. They are also small, and a table of a few MB fits in the 30 MB last-level cache there, so the cache term this model tests could not be fitted on them. The coefficients are fitted on streams already read, M1D05's, and judged only on streams none of them saw, M1D07's. That keeps the rule's purpose, no leak from the evaluation, but it departs from the rule's letter, so it goes in `log.md` and to the PI.

## Models, fixed now

Time is in seconds per arm-cell. Every coefficient is nonnegative. Each model is fitted by nonnegative least squares on relative error, minimising the sum over fitting arm-cells of (prediction / seconds − 1)².

| Model | Prediction | Coefficients |
|---|---|---|
| **U**, unit cost | c · ops | 1 |
| **K**, priced by kind | Σ over the 10 counters of c_k · counter_k | 10 |
| **KC**, by kind and cache | K, with `probe`, `tab_read` and `tab_write` each given a second coefficient that applies when `table_bytes` exceeds the last-level cache | 13 |

The last-level cache is 30,720 KB, by Windows' `Win32_CacheMemory` on this machine, read on 2026-11-04. A per-arm constant is not a model here: it cannot say why one arm costs more per operation than another.

## Reading, fixed now

On the evaluation set, for each model:

1. **Error.** The median of |prediction / seconds − 1| over arm-cells.
2. **Winner.** On each cell, the arm the model predicts fastest against the arm fastest by median seconds, over the cell's arm-cells. The count of cells where they agree.
3. **Pass, at p = 5.** PR's predicted median score (PR over the best other arm, by prediction) against its measured median score, per cell. The count of cells where both are at most 0.67, or both are above it. This reading is on medians, so it is not Gate 1's pass test, and it is never quoted as one.

| Outcome | When | What section 6 says |
|---|---|---|
| **Carries** | KC agrees on the winner on at least 25 percentage points more of the evaluation cells than U, and KC's median error is at most 25% | A cost model that prices a table access by whether the table fits in cache predicts the clock winner on new streams. The coefficients are given |
| **Partly** | One of the two conditions holds | The section gives the model with what it gets right and what it misses, by name |
| **Does not carry** | Neither holds | Operations mislead here, and the paper does not offer a model of why |

Read beside it, whatever the outcome: the fitted coefficients in nanoseconds; the same three readings on the fitting set; the cells where KC names the wrong winner; and M1D04's and M1D07's ns per charged operation per arm, which this replaces.

## Order of work

| ID | Step |
|---|---|
| M1D09A | This file and `scripts/m1d09_cost.py`, committed before any fit is read |
| M1D09B | One run of the script, output in `results/cost-M1D09/`, read once |
