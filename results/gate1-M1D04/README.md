# M1D04: timing sweep, phase main

Protocol: `docs/experiments/gate1-protocol.md`, thresholds unchanged, reading M1D04-R1 fixed before this clock (`log.md` 0025). Phase `main` of three. Phases `wiki` and `held` have not run, so there is **no verdict here**. M1D05 gives it.
Timing build, one process at a time, pinned to logical CPU 8, `--exclusive` tower claim on the PI's word and ControlPC's GO, taken 00:19:17 and released 02:48:58 on 2026-11-02, 2 h 29 min 41 s, under the line of 8 hours. `LINE_HOURS=8`. No restart.
Every stream here is labelled **real timestamps, synthetic expiry** or **real timestamps, insert-only**. None is fully synthetic. wiki-talk insert is not in this phase, and sx-stackoverflow has not been read by any arm.

## Result

- **Complete.** `M1D04-DONE`. 30 cell records of the 30 in this phase, 160 arm-cells, 600 timing processes, 0 away from the repeats the protocol asks. 504 theta runs and 30 count runs of M.
- **Exact.** 0 differences. The sweep's own check stopped nothing. A separate check after the run took every repeat of every arm, the M count build and every M1D03 arm on the same cells: 30 groups, 760 files, 13,101,889 batch comparisons, 0 files differ. `mcheck` compared M's two builds on 584,355 batches, 0 differ.
- **The binary did not change.** All nine hashes in `hashes.txt` still match after the run, at commit e23c597.
- **Theta,** by the geometric mean over the engineering cells: 16 at p = 3, 16 at p = 4, 128 at p = 5. See `theta.txt` and the first lines of `run.log`.
- **The clock was steady.** Median slowest over fastest of one arm on one cell: 1.020, 1.009, 1.008 at p = 3, 4, 5. The worst is 1.144, M3 on sx-superuser insert 1,000, a sub-second cell. PR's and R2's first repeats sit within 0.7% of their medians, so nothing drifted.
- **Cost.** p = 3 took about 8 minutes, p = 4 about 26, p = 5 about 1 h 47. 8,375 s of timing, 137 s of theta, 217 s of the M count build. Peak RSS was 13.97 GB, on 6 arm-cells past 12 GB, all T3, T4 or WT on wiki-talk expiry.
- **Stopped arms,** 7 of 160: A5 by time on 4 cells, A4 by time on 1, WJ by memory on 2, both wiki-talk expiry. No T arm stopped here. They stopped on wiki-talk insert in M1D03, and that cell is phase `wiki`.

**"Exclusive" here means no heavy work, not a silent machine.** ControlPC reports that other Claude sessions, Chrome, OneDrive and its 2-hourly status mail stay on the machine during a claim. The spread above is the measured effect, and it is small.

## Reading against the rule, over these 10 cells only

| p | Scored | Median score | Median against scan | Pass |
|---:|---:|---:|---:|---:|
| 3 | 10 of 10 | 1.131 | 1.131 | 0 |
| 4 | 10 of 10 | 1.279 | 0.992 | 0 |
| 5 | 10 of 10 | 0.599 | 0.599 | 6, on all 3 streams, 4 of them expiry |

The share of passes does not depend on how "best" is read: taking the lowest fastest repeat of any other arm gives the same 6. Share at p = 4 is P4 on every cell.

**What the rest can change.** If wiki-talk insert and all 4 held-out cells are scored and none passes, p = 5 stands at 6 of 16, which is above a third, on 3 streams, with 4 expiry cells, and its median at worst 0.796. p = 3 at worst stands at 1.098 over 12 cells. So this is Go with the trend on every outcome of the other two phases but one: an exactness failure not traced to a test bug, which is Stop. This is arithmetic on the rule, not the verdict.

## What this reading does not say

1. **M did not decide it, and the reading before this run was wrong.** `log.md` 0024 expected Pivot or a thin Go, resting on M. M5 is the best arm on 2 of 10 p = 5 cells, and R2, the scan, on 8. What moved was the clock's price per operation: PR 0.61 ns, R2 0.72, M5 0.83, WT 1.69, WJ 8.26. M1D03's count seconds pointed this way and could not be cited.
2. **On most sx cells PR wins by cache cost, not by work.** WJ does fewer charged operations than PR on 6 of the 8 sx cells at p = 5, listed below, and still takes about 3 to 26 times as long. The unit-cost model does not price a random probe into a table of several GB. So the result is tied to this memory hierarchy, on one machine and one core. PR leads on operations as well only on the two sx expiry 1,000 cells and on wiki-talk expiry, among the arms that finished.
3. **WJ's margin is not uniform.** To undo the two sx insert passes it would have to run about 17 times faster. To undo the two sx expiry passes, about 2.6 times faster, where a better flat table is not out of the question. With those two gone, and none of the other 6 cells passing, 4 of 16 would be Pivot.
4. **The gain needs a large batch.** Cut 1,000 passes on all 5 cells and cut 100 on 1, wiki-talk expiry. Cut 1,000 batches are about ten times the size of cut 100. sx-superuser insert 100 missed at 0.678 against 0.67.
5. **Most of the passes are against a scan.** R2 is PR's own kernel without sharing. That is the honest baseline on this clock, since the tuned tables are slower, but it reads as "sharing pays against its unshared self, and the tables lost on memory".
6. **M was tuned on small streams.** The engineering cells run in fractions of a second. p = 3 and p = 4 chose 16, the end of the grid, with the curve rising from there, so M might do better lower still. That cannot change a pass at p = 3 or 4, where none exists. p = 5 chose 128 inside the grid, and the curve is flat, 0.147 to 0.185. Running M5 over the grid on evaluation cells would favour the baseline, and it belongs in M1D06.
7. **The three streams are not unseen,** as the protocol says. Only sx-stackoverflow is.
8. **Novelty does not move.** The trend itself is published by Cheetah (`log.md` 0019). A clean result here is a gain against the strongest baseline, closed-form kernels, sequential, all costs inside, and no more.

## Scores, cell by cell

The slowest and fastest columns are the pass test. A cell passes at 0.67 or less. Seconds are the timing build under the exclusive claim.

### p = 5

| Stream | Label | Cut | Share | Median s | Slowest s | Best | Median s | Fastest s | Score | Slowest over fastest | Pass | Against scan |
|---|---|---:|---|---:|---:|---|---:|---:|---:|---:|---|---:|
| sx-askubuntu | real timestamps, synthetic expiry | 1,000 | PR | 0.268 | 0.270 | R2 | 0.456 | 0.456 | 0.588 | 0.592 | yes | 0.588 |
| sx-askubuntu | real timestamps, synthetic expiry | 100 | PR | 0.429 | 0.430 | R2 | 0.480 | 0.478 | 0.894 | 0.900 | no | 0.894 |
| sx-askubuntu | real timestamps, insert-only | 1,000 | PR | 4.866 | 4.870 | M5 | 11.760 | 11.684 | 0.414 | 0.417 | yes | 0.411 |
| sx-askubuntu | real timestamps, insert-only | 100 | PR | 8.183 | 8.274 | M5 | 11.717 | 11.693 | 0.698 | 0.708 | no | 0.691 |
| sx-superuser | real timestamps, synthetic expiry | 1,000 | PR | 0.469 | 0.473 | R2 | 0.768 | 0.768 | 0.611 | 0.616 | yes | 0.611 |
| sx-superuser | real timestamps, synthetic expiry | 100 | PR | 0.719 | 0.741 | R2 | 0.799 | 0.798 | 0.900 | 0.929 | no | 0.900 |
| sx-superuser | real timestamps, insert-only | 1,000 | PR | 13.254 | 13.357 | R2 | 31.793 | 31.760 | 0.417 | 0.421 | yes | 0.417 |
| sx-superuser | real timestamps, insert-only | 100 | PR | 21.417 | 21.561 | R2 | 31.815 | 31.781 | 0.673 | 0.678 | no | 0.673 |
| wiki-talk-temporal | real timestamps, synthetic expiry | 1,000 | PR | 10.921 | 11.008 | R2 | 36.261 | 36.151 | 0.301 | 0.305 | yes | 0.301 |
| wiki-talk-temporal | real timestamps, synthetic expiry | 100 | PR | 17.519 | 17.674 | R2 | 36.386 | 36.293 | 0.481 | 0.487 | yes | 0.481 |

### p = 4

| Stream | Label | Cut | Share | Median s | Slowest s | Best | Median s | Fastest s | Score | Slowest over fastest | Pass | Against scan |
|---|---|---:|---|---:|---:|---|---:|---:|---:|---:|---|---:|
| sx-askubuntu | real timestamps, synthetic expiry | 1,000 | P4 | 0.146 | 0.147 | S4 | 0.124 | 0.124 | 1.177 | 1.185 | no | 1.177 |
| sx-askubuntu | real timestamps, synthetic expiry | 100 | P4 | 0.166 | 0.168 | S4 | 0.130 | 0.129 | 1.277 | 1.302 | no | 1.277 |
| sx-askubuntu | real timestamps, insert-only | 1,000 | P4 | 0.747 | 0.773 | M4 | 0.590 | 0.586 | 1.266 | 1.319 | no | 0.898 |
| sx-askubuntu | real timestamps, insert-only | 100 | P4 | 0.841 | 0.844 | M4 | 0.591 | 0.586 | 1.423 | 1.440 | no | 1.006 |
| sx-superuser | real timestamps, synthetic expiry | 1,000 | P4 | 0.235 | 0.238 | S4 | 0.203 | 0.202 | 1.158 | 1.178 | no | 1.158 |
| sx-superuser | real timestamps, synthetic expiry | 100 | P4 | 0.265 | 0.266 | S4 | 0.207 | 0.207 | 1.280 | 1.285 | no | 1.280 |
| sx-superuser | real timestamps, insert-only | 1,000 | P4 | 1.967 | 1.993 | M4 | 1.493 | 1.490 | 1.317 | 1.338 | no | 0.884 |
| sx-superuser | real timestamps, insert-only | 100 | P4 | 2.172 | 2.184 | M4 | 1.489 | 1.481 | 1.459 | 1.475 | no | 0.977 |
| wiki-talk-temporal | real timestamps, synthetic expiry | 1,000 | P4 | 4.048 | 4.057 | M4 | 3.292 | 3.287 | 1.230 | 1.234 | no | 0.562 |
| wiki-talk-temporal | real timestamps, synthetic expiry | 100 | P4 | 6.093 | 6.100 | M4 | 3.353 | 3.347 | 1.817 | 1.823 | no | 0.835 |

### p = 3

| Stream | Label | Cut | Share | Median s | Slowest s | Best | Median s | Fastest s | Score | Slowest over fastest | Pass | Against scan |
|---|---|---:|---|---:|---:|---|---:|---:|---:|---:|---|---:|
| sx-askubuntu | real timestamps, synthetic expiry | 1,000 | P3 | 0.090 | 0.093 | S3 | 0.080 | 0.079 | 1.125 | 1.177 | no | 1.125 |
| sx-askubuntu | real timestamps, synthetic expiry | 100 | P3 | 0.097 | 0.099 | S3 | 0.081 | 0.080 | 1.198 | 1.238 | no | 1.198 |
| sx-askubuntu | real timestamps, insert-only | 1,000 | P3 | 0.141 | 0.143 | S3 | 0.141 | 0.139 | 1.000 | 1.029 | no | 1.000 |
| sx-askubuntu | real timestamps, insert-only | 100 | P3 | 0.149 | 0.152 | S3 | 0.139 | 0.139 | 1.072 | 1.094 | no | 1.072 |
| sx-superuser | real timestamps, synthetic expiry | 1,000 | P3 | 0.138 | 0.140 | S3 | 0.119 | 0.119 | 1.160 | 1.176 | no | 1.160 |
| sx-superuser | real timestamps, synthetic expiry | 100 | P3 | 0.148 | 0.149 | S3 | 0.120 | 0.120 | 1.233 | 1.242 | no | 1.233 |
| sx-superuser | real timestamps, insert-only | 1,000 | P3 | 0.256 | 0.259 | S3 | 0.264 | 0.263 | 0.970 | 0.985 | no | 0.970 |
| sx-superuser | real timestamps, insert-only | 100 | P3 | 0.274 | 0.279 | S3 | 0.266 | 0.264 | 1.030 | 1.057 | no | 1.030 |
| wiki-talk-temporal | real timestamps, synthetic expiry | 1,000 | P3 | 1.008 | 1.033 | S3 | 0.887 | 0.880 | 1.136 | 1.174 | no | 1.136 |
| wiki-talk-temporal | real timestamps, synthetic expiry | 100 | P3 | 1.062 | 1.068 | S3 | 0.899 | 0.896 | 1.181 | 1.192 | no | 1.181 |

## Every arm at p = 5

Median seconds, with the repeats in brackets. TIME is the 300 s stop of A5. CAP is WJ's stop by memory.

| Stream | Label | Cut | PR | R2 | M5 | WT | WJ | A5 |
|---|---|---:|---:|---:|---:|---:|---:|---:|
| sx-askubuntu | real timestamps, synthetic expiry | 1,000 | 0.27 (5) | 0.46 (5) | 0.87 (5) | 0.83 (5) | 1.07 (5) | 0.76 (5) |
| sx-askubuntu | real timestamps, synthetic expiry | 100 | 0.43 (5) | 0.48 (5) | 0.92 (5) | 0.88 (5) | 1.22 (5) | 4.94 (1) |
| sx-askubuntu | real timestamps, insert-only | 1,000 | 4.87 (5) | 11.83 (5) | 11.76 (5) | 20.56 (5) | 124.79 (1) | 178.81 (1) |
| sx-askubuntu | real timestamps, insert-only | 100 | 8.18 (5) | 11.84 (5) | 11.72 (5) | 20.60 (5) | 125.09 (1) | TIME |
| sx-superuser | real timestamps, synthetic expiry | 1,000 | 0.47 (5) | 0.77 (5) | 1.40 (5) | 1.38 (5) | 1.85 (5) | 1.47 (5) |
| sx-superuser | real timestamps, synthetic expiry | 100 | 0.72 (5) | 0.80 (5) | 1.44 (5) | 1.45 (5) | 2.03 (5) | 10.00 (1) |
| sx-superuser | real timestamps, insert-only | 1,000 | 13.25 (5) | 31.79 (5) | 33.93 (5) | 52.96 (5) | 334.81 (1) | TIME |
| sx-superuser | real timestamps, insert-only | 100 | 21.42 (5) | 31.82 (5) | 34.00 (5) | 52.98 (5) | 334.71 (1) | TIME |
| wiki-talk-temporal | real timestamps, synthetic expiry | 1,000 | 10.92 (5) | 36.26 (5) | 45.45 (5) | 194.75 (1) | CAP | 96.93 (1) |
| wiki-talk-temporal | real timestamps, synthetic expiry | 100 | 17.52 (5) | 36.39 (5) | 45.75 (5) | 196.21 (1) | CAP | TIME |

## Row kernel against BFCB-IG at p = 4

Median seconds of P4 over B4. Read beside the verdict.

| Stream | Label | Cut | P4 | B4 | P4 over B4 |
|---|---|---:|---:|---:|---:|
| sx-askubuntu | real timestamps, synthetic expiry | 1,000 | 0.146 | 0.452 | 0.323 |
| sx-askubuntu | real timestamps, synthetic expiry | 100 | 0.166 | 0.549 | 0.302 |
| sx-askubuntu | real timestamps, insert-only | 1,000 | 0.747 | 5.037 | 0.148 |
| sx-askubuntu | real timestamps, insert-only | 100 | 0.841 | 8.976 | 0.094 |
| sx-superuser | real timestamps, synthetic expiry | 1,000 | 0.235 | 0.757 | 0.310 |
| sx-superuser | real timestamps, synthetic expiry | 100 | 0.265 | 0.907 | 0.292 |
| sx-superuser | real timestamps, insert-only | 1,000 | 1.967 | 13.474 | 0.146 |
| sx-superuser | real timestamps, insert-only | 100 | 2.172 | 22.995 | 0.094 |
| wiki-talk-temporal | real timestamps, synthetic expiry | 1,000 | 4.048 | 39.076 | 0.104 |
| wiki-talk-temporal | real timestamps, synthetic expiry | 100 | 6.093 | 46.986 | 0.130 |

## Operations against the clock

12 of 30 cells name a different winner. M's operations come from `count/` at the chosen theta, the other arms' from M1D03.

| Stream | Label | Cut | p | Fewest operations | Fastest on the clock |
|---|---|---:|---:|---|---|
| sx-askubuntu | real timestamps, insert-only | 1,000 | 3 | S3 | P3 |
| sx-superuser | real timestamps, insert-only | 1,000 | 3 | S3 | P3 |
| sx-askubuntu | real timestamps, synthetic expiry | 1,000 | 4 | M4 | S4 |
| sx-askubuntu | real timestamps, synthetic expiry | 100 | 4 | M4 | S4 |
| sx-superuser | real timestamps, synthetic expiry | 1,000 | 4 | M4 | S4 |
| sx-superuser | real timestamps, synthetic expiry | 100 | 4 | M4 | S4 |
| sx-askubuntu | real timestamps, synthetic expiry | 100 | 5 | WJ | PR |
| sx-askubuntu | real timestamps, insert-only | 1,000 | 5 | WJ | PR |
| sx-askubuntu | real timestamps, insert-only | 100 | 5 | WJ | PR |
| sx-superuser | real timestamps, synthetic expiry | 100 | 5 | WJ | PR |
| sx-superuser | real timestamps, insert-only | 1,000 | 5 | WJ | PR |
| sx-superuser | real timestamps, insert-only | 100 | 5 | WJ | PR |

Nanoseconds per charged operation, median over the cells where the arm finished in both builds. Streams: all ten evaluation cells of this phase, both labels.

| Arm | Cells | Median | Lowest | Highest |
|---|---:|---:|---:|---:|
| S3 | 10 | 7.89 | 6.59 | 9.79 |
| P3 | 10 | 6.23 | 4.68 | 7.19 |
| T3 | 10 | 3.32 | 2.68 | 3.55 |
| M3 | 10 | 3.45 | 1.57 | 3.61 |
| S4 | 10 | 0.32 | 0.27 | 0.80 |
| P4 | 10 | 0.70 | 0.28 | 1.36 |
| T4 | 10 | 3.27 | 2.67 | 3.54 |
| M4 | 10 | 2.57 | 0.50 | 3.26 |
| B4 | 10 | 0.55 | 0.29 | 0.96 |
| A4 | 9 | 0.95 | 0.75 | 1.41 |
| R2 | 10 | 0.72 | 0.36 | 0.82 |
| PR | 10 | 0.61 | 0.31 | 1.14 |
| WT | 10 | 1.69 | 1.33 | 2.65 |
| WJ | 8 | 8.26 | 3.06 | 15.35 |
| M5 | 10 | 0.83 | 0.73 | 1.28 |
| A5 | 6 | 0.66 | 0.57 | 0.81 |
