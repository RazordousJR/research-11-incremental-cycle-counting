# Hindsight diagnostic after Gate 0

Commit `e267b9f`. Not a gate. It was run after the verdict to test whether the STOP came from the implementation.
For every group it takes the cheaper of kernel A and kernel O with free selection, no batch overhead and no order
upkeep. No real selector can do better than this, under the unit-cost model and this grouping.

`oracle_ratio` is that hindsight cost divided by kernel A cost.

| Variant | Best ratio | Where |
|---|---|---|
| Two-week expiry | 0.872 | wiki-talk-temporal, 7-day batches |
| Insert-only, no expiry | 0.946 | CollegeMsg, 10,000-event batches |

So the cap on the gain is 13% on these streams, against the 33% that Gate 0 needed, before any overhead.
Insert-only streams raise kernel A to at most 64.4 operations per update, and the ratio moves closer to 1, not
further from it. All 60 runs ended exact against a static count.
