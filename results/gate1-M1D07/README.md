# M1D07C: count sweep on the new streams

Protocol: `docs/experiments/m1d07-streams.md`, readings M1D07-R1 and the youtube decision included (`log.md` 0044, 0048). Count build, M1D04's `build/gate1/count.exe`, one process at a time pinned to logical CPU 8, two ordinary tower claims on ControlPC's GO: 10:02:26 to 15:03:26 and 15:04:28 to 15:08:02 on 2026-11-03, 5 h 05 min of the PI's 16 hours. **Operations are the reading here (hard rule 3). No second in this file or in `run.log` is timing, and none is quoted as time.** The seconds feed `plan.txt`, the timing plan, only.

Labels: **real timestamps, real deletions** (KONECT), **real timestamps, synthetic expiry** and **real timestamps, insert-only** (SNAP Reddit, Network Repository), and **static snapshot, file order** (soc-youtube-growth, reported and never in a verdict).

## Result

- **Complete.** `M1D07-DONE` in both claims. 13 streams derived, none dropped. 70 cells: 38 at p = 3 and 32 at p = 5, the two p = 3 cells of youtube's held expiry copy among the p = 3 ones and not used.
- **Exact.** The sweep's check passed on every cell. A separate check after the first claim took every raw file: 62 cells, 308 arm-cells, 2,053,393 batch comparisons, 0 differ, and the second claim's 8 cells passed the same check in the run. Hard rule 1 is not touched. The hashes in `hashes.txt` hold.
- **Arms short of the stream,** all at their own limits: A5 by its 300 s limit, T3 and WT by the 16 GB cap at a peak resident near 15.75 GB, WJ by its 6.44 GB table cap. None came from the machine's memory.
- **Neighbours.** Research 12 ran beside the first claim from 10:31:05 to 10:57:01 and from 11:51:17 to 13:04:03, four unpinned single-thread processes each time. Counts and operations do not move. The seconds of those cells read high, and `plan.txt` marks them.

## Operations, share against the fewest other finished arm

Share is P3 or PR. "Fewest other" is the finished arm of the other rows with the fewest charged operations. At p = 5 that arm is WJ on 26 of 30 cells, R2 on 5, M5 on 4, WT on 2, A5 on 1, counting the snapshot. M1D04 found WJ 3 to 26 times slower than PR on the clock on cells where it does fewer operations, so this table does not predict the clock, and Gate 1's rule reads the clock only.

### p = 3
| Stream | Label | Cut | Share ops, millions | Fewest other | Its ops | Share over fewest other | Share over scan |
|---|---|---:|---:|---|---:|---:|---:|
| dewiki | real timestamps, real deletions | 100 | 121 | S3 | 92 | 1.319 | 1.319 |
| dewiki | real timestamps, real deletions | 1,000 | 107 | S3 | 83 | 1.301 | 1.301 |
| frwiki | real timestamps, real deletions | 100 | 120 | S3 | 92 | 1.308 | 1.308 |
| frwiki | real timestamps, real deletions | 1,000 | 106 | S3 | 83 | 1.273 | 1.273 |
| ia-enron-email-dynamic | real timestamps, synthetic expiry | 100 | 30 | S3 | 20 | 1.534 | 1.534 |
| ia-enron-email-dynamic | real timestamps, synthetic expiry | 1,000 | 28 | S3 | 18 | 1.530 | 1.530 |
| ia-enron-email-dynamic | real timestamps, insert-only | 100 | 17 | S3 | 13 | 1.296 | 1.296 |
| ia-enron-email-dynamic | real timestamps, insert-only | 1,000 | 17 | S3 | 13 | 1.268 | 1.268 |
| ia-reality-call | real timestamps, synthetic expiry | 100 | 1 | S3 | 0 | 1.363 | 1.363 |
| ia-reality-call | real timestamps, synthetic expiry | 1,000 | 1 | S3 | 0 | 1.382 | 1.382 |
| ia-reality-call | real timestamps, insert-only | 100 | 0 | S3 | 0 | 1.222 | 1.222 |
| ia-reality-call | real timestamps, insert-only | 1,000 | 0 | S3 | 0 | 1.256 | 1.256 |
| itwiki | real timestamps, real deletions | 100 | 104 | S3 | 79 | 1.314 | 1.314 |
| itwiki | real timestamps, real deletions | 1,000 | 95 | S3 | 74 | 1.287 | 1.287 |
| nlwiki | real timestamps, real deletions | 100 | 113 | S3 | 84 | 1.348 | 1.348 |
| nlwiki | real timestamps, real deletions | 1,000 | 95 | S3 | 72 | 1.323 | 1.323 |
| plwiki | real timestamps, real deletions | 100 | 142 | S3 | 112 | 1.270 | 1.270 |
| plwiki | real timestamps, real deletions | 1,000 | 125 | S3 | 101 | 1.244 | 1.244 |
| simplewiki | real timestamps, real deletions | 100 | 100 | S3 | 78 | 1.275 | 1.275 |
| simplewiki | real timestamps, real deletions | 1,000 | 63 | S3 | 50 | 1.246 | 1.246 |
| soc-redditHyperlinks-body | real timestamps, synthetic expiry | 100 | 8 | S3 | 5 | 1.480 | 1.480 |
| soc-redditHyperlinks-body | real timestamps, synthetic expiry | 1,000 | 8 | S3 | 5 | 1.651 | 1.651 |
| soc-redditHyperlinks-body | real timestamps, insert-only | 100 | 6 | S3 | 5 | 1.230 | 1.230 |
| soc-redditHyperlinks-body | real timestamps, insert-only | 1,000 | 6 | S3 | 5 | 1.362 | 1.362 |
| soc-redditHyperlinks-title | real timestamps, synthetic expiry | 100 | 23 | S3 | 16 | 1.411 | 1.411 |
| soc-redditHyperlinks-title | real timestamps, synthetic expiry | 1,000 | 24 | S3 | 15 | 1.633 | 1.633 |
| soc-redditHyperlinks-title | real timestamps, insert-only | 100 | 14 | S3 | 12 | 1.186 | 1.186 |
| soc-redditHyperlinks-title | real timestamps, insert-only | 1,000 | 16 | S3 | 12 | 1.322 | 1.322 |
| soc-wiki-elec | real timestamps, synthetic expiry | 100 | 7 | S3 | 4 | 1.637 | 1.637 |
| soc-wiki-elec | real timestamps, synthetic expiry | 1,000 | 6 | S3 | 4 | 1.509 | 1.509 |
| soc-wiki-elec | real timestamps, insert-only | 100 | 9 | S3 | 6 | 1.353 | 1.353 |
| soc-wiki-elec | real timestamps, insert-only | 1,000 | 8 | S3 | 7 | 1.156 | 1.156 |
| soc-youtube-growth | static snapshot, file order | 100 | 57 | S3 | 41 | 1.385 | 1.385 |
| soc-youtube-growth | static snapshot, file order | 1,000 | 56 | S3 | 42 | 1.342 | 1.342 |
| tech-as-topology | real timestamps, synthetic expiry | 100 | 11 | S3 | 8 | 1.326 | 1.326 |
| tech-as-topology | real timestamps, synthetic expiry | 1,000 | 11 | S3 | 8 | 1.390 | 1.390 |
| tech-as-topology | real timestamps, insert-only | 100 | 5 | S3 | 4 | 1.319 | 1.319 |
| tech-as-topology | real timestamps, insert-only | 1,000 | 5 | S3 | 4 | 1.358 | 1.358 |

Median share over fewest other, all labels but the snapshot: 1.320 over 36 cells; real timestamps, real deletions 1.294 (12); real timestamps, synthetic expiry 1.495 (12); real timestamps, insert-only 1.282 (12)
Median share over the scan: 1.320

### p = 5
| Stream | Label | Cut | Share ops, millions | Fewest other | Its ops | Share over fewest other | Share over scan |
|---|---|---:|---:|---|---:|---:|---:|
| dewiki | real timestamps, real deletions | 100 | 49,769 | WJ | 31,264 | 1.592 | 0.658 |
| dewiki | real timestamps, real deletions | 1,000 | 45,168 | WJ | 25,411 | 1.777 | 0.654 |
| frwiki | real timestamps, real deletions | 100 | 89,584 | WJ | 51,906 | 1.726 | 0.676 |
| frwiki | real timestamps, real deletions | 1,000 | 77,585 | WJ | 45,696 | 1.698 | 0.638 |
| ia-enron-email-dynamic | real timestamps, synthetic expiry | 100 | 949 | WJ | 1,101 | 0.862 | 0.449 |
| ia-enron-email-dynamic | real timestamps, synthetic expiry | 1,000 | 818 | WJ | 1,006 | 0.813 | 0.416 |
| ia-enron-email-dynamic | real timestamps, insert-only | 100 | 4,067 | WJ | 2,716 | 1.498 | 0.519 |
| ia-enron-email-dynamic | real timestamps, insert-only | 1,000 | 3,766 | WJ | 2,775 | 1.357 | 0.484 |
| ia-reality-call | real timestamps, synthetic expiry | 100 | 4 | R2 | 6 | 0.784 | 0.784 |
| ia-reality-call | real timestamps, synthetic expiry | 1,000 | 2 | R2 | 4 | 0.526 | 0.526 |
| ia-reality-call | real timestamps, insert-only | 100 | 3 | R2 | 4 | 0.764 | 0.764 |
| ia-reality-call | real timestamps, insert-only | 1,000 | 2 | R2 | 4 | 0.465 | 0.465 |
| itwiki | real timestamps, real deletions | 100 | 135,268 | M5 | 212,735 | 0.636 | 0.486 |
| itwiki | real timestamps, real deletions | 1,000 | 112,596 | M5 | 204,241 | 0.551 | 0.423 |
| nlwiki | real timestamps, real deletions | 100 | 79,404 | WJ | 63,518 | 1.250 | 0.668 |
| nlwiki | real timestamps, real deletions | 1,000 | 67,285 | WJ | 47,921 | 1.404 | 0.631 |
| plwiki | real timestamps, real deletions | 100 | 120,488 | WT | 94,079 | 1.281 | 0.637 |
| plwiki | real timestamps, real deletions | 1,000 | 102,465 | WT | 76,517 | 1.339 | 0.576 |
| simplewiki | real timestamps, real deletions | 100 | 41,111 | R2 | 75,876 | 0.542 | 0.542 |
| simplewiki | real timestamps, real deletions | 1,000 | 29,784 | WJ | 29,509 | 1.009 | 0.550 |
| soc-redditHyperlinks-body | real timestamps, synthetic expiry | 100 | 104 | WJ | 86 | 1.216 | 0.814 |
| soc-redditHyperlinks-body | real timestamps, synthetic expiry | 1,000 | 61 | WJ | 73 | 0.828 | 0.551 |
| soc-redditHyperlinks-body | real timestamps, insert-only | 100 | 3,349 | WJ | 1,068 | 3.137 | 0.897 |
| soc-redditHyperlinks-body | real timestamps, insert-only | 1,000 | 2,469 | WJ | 1,068 | 2.312 | 0.661 |
| soc-redditHyperlinks-title | real timestamps, synthetic expiry | 100 | 1,082 | WJ | 918 | 1.178 | 0.805 |
| soc-redditHyperlinks-title | real timestamps, synthetic expiry | 1,000 | 571 | WJ | 799 | 0.714 | 0.475 |
| soc-redditHyperlinks-title | real timestamps, insert-only | 100 | 19,004 | WJ | 4,796 | 3.962 | 0.918 |
| soc-redditHyperlinks-title | real timestamps, insert-only | 1,000 | 14,019 | WJ | 4,797 | 2.922 | 0.677 |
| soc-wiki-elec | real timestamps, synthetic expiry | 100 | 232 | WJ | 162 | 1.430 | 0.551 |
| soc-wiki-elec | real timestamps, synthetic expiry | 1,000 | 154 | A5 | 139 | 1.106 | 0.436 |
| soc-wiki-elec | real timestamps, insert-only | 100 | 1,978 | WJ | 1,018 | 1.943 | 0.619 |
| soc-wiki-elec | real timestamps, insert-only | 1,000 | 1,769 | WJ | 1,065 | 1.661 | 0.562 |
| soc-youtube-growth | static snapshot, file order | 100 | 47,036 | M5 | 46,956 | 1.002 | 0.222 |
| soc-youtube-growth | static snapshot, file order | 1,000 | 44,991 | M5 | 47,363 | 0.950 | 0.214 |
| tech-as-topology | real timestamps, synthetic expiry | 100 | 4,903 | WJ | 3,215 | 1.525 | 0.670 |
| tech-as-topology | real timestamps, synthetic expiry | 1,000 | 3,090 | WJ | 3,216 | 0.961 | 0.422 |
| tech-as-topology | real timestamps, insert-only | 100 | 2,887 | WJ | 1,157 | 2.496 | 0.632 |
| tech-as-topology | real timestamps, insert-only | 1,000 | 1,866 | WJ | 1,160 | 1.609 | 0.409 |

Median share over fewest other, all labels but the snapshot: 1.310 over 36 cells; real timestamps, real deletions 1.310 (12); real timestamps, synthetic expiry 0.911 (12); real timestamps, insert-only 1.802 (12)
Median share over the scan: 0.598

