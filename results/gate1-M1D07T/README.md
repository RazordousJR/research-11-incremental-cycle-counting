# M1D07D and M1D07E: timing on the new streams, and the reading

Protocol: `docs/experiments/m1d07-streams.md`, with reading M1D07-R1 and the youtube decision (`log.md` 0044, 0048). Timing build, M1D04's `build/gate1/timing.exe`, checked by hash. One process at a time, pinned to logical CPU 8, under two `--exclusive --timing` tower claims, on the PI's hours (`log.md` 0052) and ControlPC's GO. Gate 1's repeat rule, limits and reading M1D04-R1 are unchanged. The reading is `scripts/m1d07_score.py`. It was written during claim 2, before any score was read, and it calls `gate1_score.score` without change. Its full output is in `score.txt`.

Labels: **real timestamps, real deletions** (KONECT), **real timestamps, synthetic expiry** and **real timestamps, insert-only** (SNAP Reddit, Network Repository), and **static snapshot, file order** (soc-youtube-growth, reported and never in a verdict or a pooled reading).

## Runs

| Claim | Phases | Ran | Line | Cells | End |
|---|---|---|---:|---:|---|
| 1 | D1, D2 | 2026-11-03 22:10:08 to 2026-11-04 02:30:10, 4 h 20 min | 7 h | 62 | `M1D07-DONE`, exit 0, released by the chain |
| 2 | D3, D4 | 2026-11-04 02:35:01 to 06:32:19, 3 h 57 min | 7 h | 14 | `M1D07-DONE`, exit 0, released by the chain |

8 h 17 min in all, against a plan of 8.55 to 9.40 hours and 14 approved hours. There was no restart, no STOP and no LINE.

- **Exact.** Every timing repeat equals its arm's count build in M1D07C, batch by batch. The chain checked this per cell, and after release `scripts/m1d07_tcheck.py` was run again over all 76 cells: 1,448 timing runs, 0 differ or missing. The count build's arms agree with each other (M1D07C), so every repeat agrees with every other arm of its cell. Hard rule 1 is not touched.
- **Plan.** 76 cells, 380 arm-cells, 1,448 processes. Every arm has the repeats Gate 1's rule asks of it, 0 wrong.
- **Hashes.** Every line of `hashes.txt` holds after release: `gate1.cpp`, `tc.hpp`, `timing.exe` and the scripts that ran.
- **Core.** Logical CPU 8 is a P-core: efficiency class 1, as are CPUs 0, 1, 6 to 9, 18 and 19. The other 12 are E-cores, class 0. Source: `scripts/cpu_sets.py`, output in `cpu-sets.txt`. Every Gate 1 time in this project ran on that P-core.
- **Arms short of the stream,** as in M1D07C: A5 on 16 arm-cells by its 300 s limit, T3 on 4 and WT on 4 by the 16 GB cap, WJ on 6 by its table cap. None came from the machine.

## Background load

Research 1's scheduled tasks and ControlPC's mail ran through both claims, as the PI chose (`log.md` 0054). Research 12 made one empty echo a few minutes after 05:23:56, inside claim 2, as ControlPC reported it. `scripts/overlap_check.py M1D07D` marks a repeat as overlapping when it touches any of these. The windows are guesses and are set wide: 90 s at :00, 300 s at 00:00 and 06:00, 30 s at :15, :30 and :45, 120 s from :05:30, and 05:23:56 to 05:34:00. Output: `overlap.txt`.

| Claim | Overlapping repeats, median against their arm's median | Clean repeats |
|---|---|---|
| 1 | 33, +0.13%, 1 over 3% | 1,037, +0.00%, 20 over 3% |
| 2 | 87, +0.00%, 1 over 3% | 178, +0.00%, 9 over 3% |

Two passes have the best arm's fastest repeat inside a window. Every repeat of each falls between 06:00 and 06:06:

- ia-enron-email-dynamic insert 100, margin 0.571;
- soc-redditHyperlinks-title expiry 1,000, margin 0.544.

Load only adds time. So for either pass to be false, the background load would have had to slow R2 by 15% to 19% in every one of its five repeats, and overlapping repeats elsewhere run +0.00% to +0.13%. Both are kept as the rule scores them. Dropping them is shown below.

## The reading, as fixed

### 1. M1D07 alone, p = 5

36 cells are scored, without the snapshot, and **22 pass**. They fall on 11 of the 12 new streams. dewiki is the exception, at margins 0.710 and 0.783.

| Label | Scored | Pass | Median score |
|---|---:|---:|---:|
| real timestamps, real deletions | 12 | 7 | 0.646 |
| real timestamps, synthetic expiry | 12 | 6 | 0.667 |
| real timestamps, insert-only | 12 | 9 | 0.637 |

### 2. Pooled, p = 5, with M1D05's 14 scored cells

50 scored and 28 pass, against a third of 16.67. The passes fall on 14 of 16 streams: every stream except dewiki and the held-out sx-stackoverflow. 10 passes are synthetic expiry cells and 7 are real-deletion cells.

- **Go, literal:** holds. 28 ≥ 16.67, 14 streams ≥ 2, 10 expiry passes ≥ 2.
- **Go, second reading,** with real-deletion cells beside expiry: holds. 17 ≥ 2.

### 3. Trend, pooled

- p = 3: 48 scored, median **0.965**. It needs at least 1.0, so this clause **fails**.
- p = 5: 50 scored, median 0.650. It needs at most 0.90, so this clause holds.

The trend fails at p = 3, and it fails because sharing got faster there, not slower. On the new streams P3 beats the scan S3 by a median of 13% (0.870). It passes on none of 36 cells. Its best margin is 0.700, on soc-wiki-elec expiry 1,000.

### Outcome: **Narrows**

The pooled Go clause holds and the pooled trend fails, and M1D07 has passes at p = 5. By the protocol's table, the claim is restricted to the labels and sizes where cells pass, by name, and the outline's claim sentence is rewritten before any prose. The rewrite is in `docs/paper/outline.md`. There is no Stop, because exactness holds.

## The p = 5 cells

Share is PR. "Best" is the other arm with the lowest median under M1D04-R1. The test is PR's slowest repeat over best's fastest, which must be at most 0.67.

| Stream | Label | Cut | PR median s | Best | Best median s | Score | Slowest/fastest | Pass |
|---|---|---:|---:|---|---:|---:|---:|---|
| dewiki | real deletions | 100 | 20.00 | R2 | 29.89 | 0.669 | 0.710 | no |
| dewiki | real deletions | 1,000 | 18.21 | R2 | 27.15 | 0.671 | 0.783 | no |
| frwiki | real deletions | 100 | 33.93 | R2 | 47.74 | 0.711 | 0.725 | no |
| frwiki | real deletions | 1,000 | 29.04 | R2 | 43.84 | 0.662 | 0.665 | yes |
| itwiki | real deletions | 100 | 47.97 | R2 | 103.82 | 0.462 | 0.482 | yes |
| itwiki | real deletions | 1,000 | 38.12 | R2 | 96.73 | 0.394 | 0.426 | yes |
| nlwiki | real deletions | 100 | 30.49 | R2 | 44.36 | 0.687 | 0.692 | no |
| nlwiki | real deletions | 1,000 | 25.37 | R2 | 39.57 | 0.641 | 0.643 | yes |
| plwiki | real deletions | 100 | 41.10 | R2 | 63.23 | 0.650 | 0.670 | no |
| plwiki | real deletions | 1,000 | 35.21 | R2 | 59.16 | 0.595 | 0.596 | yes |
| simplewiki | real deletions | 100 | 14.14 | R2 | 24.36 | 0.581 | 0.591 | yes |
| simplewiki | real deletions | 1,000 | 9.99 | R2 | 17.24 | 0.580 | 0.614 | yes |
| ia-enron-email-dynamic | synthetic expiry | 100 | 0.55 | R2 | 0.97 | 0.568 | 0.577 | yes |
| ia-enron-email-dynamic | synthetic expiry | 1,000 | 0.47 | R2 | 0.89 | 0.527 | 0.533 | yes |
| ia-enron-email-dynamic | insert-only | 100 | 1.26 | R2 | 2.23 | 0.567 | 0.571 | yes |
| ia-enron-email-dynamic | insert-only | 1,000 | 1.16 | R2 | 2.22 | 0.523 | 0.531 | yes |
| ia-reality-call | synthetic expiry | 100 | 0.01 | R2 | 0.01 | 1.000 | 1.000 | no |
| ia-reality-call | synthetic expiry | 1,000 | 0.00 | R2 | 0.01 | 0.800 | 0.800 | no |
| ia-reality-call | insert-only | 100 | 0.00 | R2 | 0.00 | 1.000 | 1.000 | no |
| ia-reality-call | insert-only | 1,000 | 0.00 | R2 | 0.00 | 0.667 | 0.667 | yes |
| soc-redditHyperlinks-body | synthetic expiry | 100 | 0.14 | R2 | 0.15 | 0.934 | 0.940 | no |
| soc-redditHyperlinks-body | synthetic expiry | 1,000 | 0.09 | R2 | 0.14 | 0.686 | 0.691 | no |
| soc-redditHyperlinks-body | insert-only | 100 | 0.98 | R2 | 1.10 | 0.888 | 0.889 | no |
| soc-redditHyperlinks-body | insert-only | 1,000 | 0.68 | R2 | 1.10 | 0.618 | 0.625 | yes |
| soc-redditHyperlinks-title | synthetic expiry | 100 | 0.73 | R2 | 0.83 | 0.884 | 0.889 | no |
| soc-redditHyperlinks-title | synthetic expiry | 1,000 | 0.41 | R2 | 0.77 | 0.539 | 0.544 | yes |
| soc-redditHyperlinks-title | insert-only | 100 | 5.22 | R2 | 5.77 | 0.904 | 0.905 | no |
| soc-redditHyperlinks-title | insert-only | 1,000 | 3.59 | R2 | 5.77 | 0.622 | 0.633 | yes |
| soc-wiki-elec | synthetic expiry | 100 | 0.12 | R2 | 0.20 | 0.624 | 0.628 | yes |
| soc-wiki-elec | synthetic expiry | 1,000 | 0.08 | A5 | 0.10 | 0.827 | 0.845 | no |
| soc-wiki-elec | insert-only | 100 | 0.55 | R2 | 0.84 | 0.657 | 0.657 | yes |
| soc-wiki-elec | insert-only | 1,000 | 0.51 | R2 | 0.83 | 0.611 | 0.614 | yes |
| tech-as-topology | synthetic expiry | 100 | 1.51 | R2 | 2.33 | 0.647 | 0.649 | yes |
| tech-as-topology | synthetic expiry | 1,000 | 0.90 | R2 | 2.33 | 0.384 | 0.385 | yes |
| tech-as-topology | insert-only | 100 | 0.86 | M5 | 1.32 | 0.651 | 0.657 | yes |
| tech-as-topology | insert-only | 1,000 | 0.53 | M5 | 1.33 | 0.397 | 0.400 | yes |

soc-youtube-growth, static snapshot, file order, reported only: PR over M5 is 0.476 at cut 100 and 0.431 at cut 1,000, both under the test. It is not in any count above.

## Read beside it, as the protocol asks

- **Real-deletion row.** At p = 3, 12 scored, 0 pass, median 0.864. At p = 5, 12 scored, 7 pass, median 0.646. The KONECT prefixes carry 15.8% to 27.8% real deletions (`log.md` 0047). The hard case passes at about the rate of the others.
- **Against the scan alone.** M1D07 median 0.870 at p = 3 and 0.633 at p = 5. Pooled 0.965 and 0.625.
- **Best arm at p = 5.** In M1D07 it is R2 on 33 of 36 cells, M5 on 2 and A5 on 1. Pooled, R2 is best on 43 of 50. The baseline that loses is PR's own unshared kernel, as M1D05 item 4 said, and the tuned tables are never close.
- **Operations against the clock.** They name different winners on 60 of M1D07's 76 cells, the snapshot's 3 included. On 35 at p = 3 the scan does the fewest operations and P3 is fastest. On 25 at p = 5 WJ, WT, M5 or A5 does the fewest and PR is fastest. Nanoseconds per charged operation, by median: PR 0.35, R2 0.36, A5 0.45, M5 0.77, WT 1.42, M3 2.06, T3 3.11, P3 5.15, S3 7.40, WJ 9.55. So the cost-model reading of M1D05 carries to the new streams.
- **Spread.** The median slowest-over-fastest of one arm on one cell is 1.012 at p = 3 and 1.005 at p = 5. The largest is 1.155, R2 on dewiki real 1,000.
- **Drift against M1D04.** It cannot be read, because no old cell was rerun.

## How much weight the passes carry

These figures are not part of the verdict. They show how much the 28 pooled passes depend on margin.

| If the best arm's fastest were | Pooled passes | Over a third (16.67) | Expiry passes | Streams |
|---|---:|---|---:|---:|
| as measured | 28 of 50 | yes | 10 | 14 |
| 5% faster | 22 | yes | 9 | 11 |
| 10% faster | 16 | **no** | 7 | 9 |
| 15% faster | 13 | no | 6 | 7 |

- **The margin is about 10%.** A scan 10% faster, on another core or another machine, would take the pooled Go clause from holds to fails. This is the M1D05 margin again, now spread over many cells. It is why the core-type test M1D08 matters (`log.md` 0056).
- **Clock resolution.** The four ia-reality-call cells run in 2 to 7 ms. One of them passes at exactly 0.667 on clock ticks. It passes by the rule, and its weight is nil.
- **Robustness.** Without that cell and the two passes that overlap background windows, 25 of 47 pass, and Go still holds.

## What M1D07 cannot show

It ran on one machine, on one P-core. A 2,000,000-event prefix is the early part of each stream. Every p = 3 cell of the new streams runs under a second, and every p = 5 cell outside the Wikipedia prefixes and the snapshot runs under 6 s. The longest cells are the Wikipedia prefixes, and they decide most of the real-deletion row.
