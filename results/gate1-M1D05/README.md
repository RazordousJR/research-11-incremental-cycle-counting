# M1D05: Gate 1 verdict

Protocol: `docs/experiments/gate1-protocol.md`, rule unchanged, reading M1D04-R1 fixed before the clock (`log.md` 0025). Data: `results/gate1-M1D04/`, timing build, pinned, exclusive claims, read with M1D03's operations. No claim and no run: this is a reading of committed files with `scripts/gate1_score.py`.
Streams: real timestamps with synthetic expiry, and real timestamps insert-only. None is fully synthetic.

## Verdict by the rule: Go, with the trend

| Clause | Needs | Has | Holds |
|---|---|---|---|
| Stop: exactness | No failure not traced to a test bug | 32,103,387 batch comparisons, 0 differ | Not met, so no Stop |
| Stop: scored | At least 8 cells scored at p = 5 | 14 of 16 | Not met, so no Stop |
| Stop: no pass | At least 1 pass | 6 | Not met, so no Stop |
| Go: share | At least a third of scored cells pass | 6 of 14, against 4.67 | Yes |
| Go: streams | Passes on at least 2 streams | sx-askubuntu, sx-superuser, wiki-talk-temporal | Yes |
| Go: expiry | At least 2 passing expiry cells | 4 | Yes |
| Trend: p = 3 | Median score at least 1.0 | 1.098 | Yes |
| Trend: p = 5 | Median score at most 0.90 | 0.686 | Yes |

The protocol's outcome for Go with that trend is one experimental paper: sharing beats the strongest baseline at 5 vertices and not at 3, with the cost model explained.

## The six passes

| Stream | Label | Cut | PR slowest over best fastest | Best |
|---|---|---:|---:|---|
| wiki-talk-temporal | real timestamps, synthetic expiry | 1,000 | 0.305 | R2 |
| sx-askubuntu | real timestamps, insert-only | 1,000 | 0.417 | M5 |
| sx-superuser | real timestamps, insert-only | 1,000 | 0.421 | R2 |
| wiki-talk-temporal | real timestamps, synthetic expiry | 100 | 0.487 | R2 |
| sx-askubuntu | real timestamps, synthetic expiry | 1,000 | 0.592 | R2 |
| sx-superuser | real timestamps, synthetic expiry | 1,000 | 0.616 | R2 |

The 8 scored cells that fail, from 0.678 to 1.380: the four sx-askubuntu and sx-superuser cells at cut 100, wiki-talk insert at both cuts against a stopped A5, and both sx-stackoverflow expiry cells. Every cell, with all its numbers, is in `results/gate1-M1D04/README.md`.

## Read beside the verdict, as the protocol asks

| p | Median score | Median against scan | Pass |
|---:|---:|---:|---:|
| 3 | 1.098 | 1.098 | 0 of 12 |
| 4 | 1.299 | 0.974 | 0 of 12 |
| 5 | 0.686 | 0.599 | 6 of 14 |

- Row kernel against BFCB-IG at p = 4: P4 takes 0.094 to 0.323 of B4's time.
- Nanoseconds per charged operation: PR 0.56, R2 0.69, M5 0.90, WT 1.69, WJ 8.26. The full table is in the M1D04 record.
- Peak memory: 14 arm-cells past 12 GB, all tables, the insert ones stopped at 15.7 to 15.9 GB.
- Operations and the clock name different winners on 13 of 40 cells, 6 of them at p = 5, where WJ does the fewest operations and PR is fastest.
- **With M1D03.** On operations alone the median against the best other arm at p = 5 was 0.950, without M. On the clock it is 0.686. The verdict rests on the clock, as hard rule 3 sets it, and the gap between the two is the cost model this paper would have to explain.

## How much weight it carries

The rule gives Go, and that does not reopen. The weight is less than the word suggests.

1. **The held-out stream gives no pass.** sx-stackoverflow, the one stream no arm had seen, scores 0.846 and 0.978 on expiry, and its insert cells go unscored because no arm finished.
2. **Two passes decide Go.** Without the two sx expiry 1,000 passes, 4 of 14 is under a third. They sit closest to the scan: R2 about 13% and 9% faster would remove them. The tuned table WT would need about twice its speed, and M5 at theta 128 about twice too. The other four need at least 1.6 times from any arm. This item first named a WJ 2.6 times faster, and `log.md` 0033 corrects it.
3. **The win is a cache win on one machine.** Where PR beats the tables, it mostly does more charged work in less time. Another memory hierarchy could move it either way.
4. **The baseline that loses most often is PR's own unshared kernel.** That is a fair baseline here, and a narrower claim than "beats tuned tables".
5. **The trend is not new.** Cheetah publishes it (`log.md` 0019).

A paper from this is a careful experimental paper on when sharing pays at p = 5 on a CPU, with the held-out failure and the cache explanation up front. It is not a large contribution.
