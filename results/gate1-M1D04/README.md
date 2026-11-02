# M1D04: timing sweep, phases main and wiki

Protocol: `docs/experiments/gate1-protocol.md`, thresholds unchanged, reading M1D04-R1 fixed before this clock (`log.md` 0025). Phases `main` and `wiki` of three. Phase `held` has not run, so there is **no verdict here**. M1D05 gives it.
Timing build, one process at a time, pinned to logical CPU 8, `--exclusive` tower claims on the PI's word and ControlPC's GO, on 2026-11-02:

| Phase | Claim | Length | Line | Record |
|---|---|---|---|---|
| `main` | 00:19:17 to 02:48:58 | 2 h 29 min 41 s | 8 h | `run.log` |
| `wiki` | 02:56:10 to 08:17:13 | 5 h 21 min 03 s | 9 h | `run-wiki.log` |

No restart in either. Every stream here is labelled **real timestamps, synthetic expiry** or **real timestamps, insert-only**. None is fully synthetic. sx-stackoverflow has not been read by any arm.

## Result

- **Complete.** `M1D04-DONE` on both phases. All 36 cell records of the 12 evaluation cells at 3 patterns, 192 arm-cells, 684 timing processes, 0 away from the repeats the protocol asks. 504 theta runs and 36 count runs of M.
- **Exact.** 0 differences. The sweep's own check stopped nothing. A separate check after each phase took every repeat of every arm, the M count build and every M1D03 arm on the same cells: 36 groups, 876 files, 16,141,789 batch comparisons, 0 files differ. `mcheck` compared M's two builds on every batch, 0 differ.
- **The binary did not change.** All nine hashes in `hashes.txt` still match after both phases, at commit e23c597's values.
- **Theta,** by the geometric mean over the engineering cells: 16 at p = 3, 16 at p = 4, 128 at p = 5. See `theta.txt` and the first lines of `run.log`.
- **The clock was steady.** Median slowest over fastest of one arm on one cell: 1.021, 1.009, 1.008 at p = 3, 4, 5. The worst is 1.144, M3 on sx-superuser insert 1,000, a sub-second cell. PR on wiki-talk insert 1,000 is the largest long-run spread that bears on a score, 273.6, 276.6 and 286.6 s. That cell does not pass on any of them, and its slowest repeat is one of the two that overlap R9's work, below.
- **Cost.** `main`: p = 3 about 8 minutes, p = 4 about 26, p = 5 about 1 h 47, 8,375 s of timing, 137 s of theta, 217 s of the M count build. `wiki`: p = 3 about 3 minutes, p = 4 about 45, p = 5 about 4 h, 17,187 s of timing, 1,913 s of the M count build. Timing ended at 07:45, and the last 32 minutes were count runs.
- **Memory.** 12 arm-cells past 12 GB, all T3, T4 or WT on wiki-talk. On expiry they finish at 13.97 GB. On insert they stop by memory at 15.76 GB, as in M1D03.
- **Stopped arms,** 17 of 192: A5 by time on 6 cells, A4 on 3, T3, T4 and WT by memory on 2 each, WJ by memory on 4, all 4 on wiki-talk.

**"Exclusive" here means no heavy work, not a silent machine.** ControlPC reports that other Claude sessions, Chrome, OneDrive and its 2-hourly status mail stay on the machine during a claim. From 02:55 that mail, format v4, reads the process list, `nvidia-smi` and the Windows logs once per run and takes a few seconds. R9 reported through ControlPC that one R9 LaTeX build hung from 00:21 to 02:56, across all of `main`, using 0.5 s of CPU and about 130 MB, and that two R9 rebuilds at 02:57 took 7 s and 4 s, inside the 120 s pause before `wiki`'s first process. Those R9 figures are not checked here. The spread above is the measured effect, and it is small.

**R9 ran inside the `wiki` claim, and two PR repeats were not exclusive.** R9 reported through ControlPC, whose figures these are not, and neither of us has checked them, that it ran one process at a time from 05:39:59 to about 05:47, unpinned, so any could have landed on CPU 8: `tower_check.py`, about six short Python reads, a listing script, `pdftotext`, `pytest` on 22 tests in 0.82 s, paper scripts under 1 s each, `figures.py` twice at about 3 s, two `latexmk` builds at about 6 s, git commits, `pdfinfo` and `pdffonts`, and one `nvidia-smi`. Most of those times are R9's estimates. Start times below are each summary file's write time less its seconds.

| Stream | Label | Cut | p | Arm, repeat | Ran | Overlap | Seconds | The other repeats |
|---|---|---:|---:|---|---|---|---:|---|
| wiki-talk-temporal | real timestamps, insert-only | 1,000 | 5 | PR, 3 | 05:37:28 to 05:42:15 | 05:39:59 to 05:42:15 | 286.62 | 273.56, 276.63 |
| wiki-talk-temporal | real timestamps, insert-only | 100 | 5 | PR, 1 | 05:42:16 to 05:49:10 | 05:42:16 to about 05:47 | 414.00 | 408.52, 408.89 |

No other timed process overlaps. Both are the slowest of their three, 3.6% and 1.3% above the median, so they sit apart, though a raw file has no per-batch clock to place the excess. The first overlap holds only R9's short reads, so its 10 s excess is not obviously explained by what R9 reports. Neither changes a result: both cells fail by a wide margin with or without the repeat, 0.955 or 0.922 and 1.380 or 1.363 of best's fastest. A repeat lower than its peers would move the median score by at most 0.005 and 0.001, and the p = 5 median of 0.642 rests on other cells. Nothing else in this file uses those two repeats.

## Reading against the rule, over the 12 evaluation cells

| p | Scored | Median score | Median against scan | Pass |
|---:|---:|---:|---:|---:|
| 3 | 12 of 12 | 1.098 | 1.098 | 0 |
| 4 | 12 of 12 | 1.299 | 0.974 | 0 |
| 5 | 12 of 12 | 0.642 | 0.535 | 6, on all 3 streams, 4 of them expiry |

Share at p = 4 is P4 on every cell.

**What the rest can change.** Phase `held` is p = 5 only, so p = 3 stands at 1.098. If all 4 held-out cells are scored and none passes, p = 5 stands at 6 of 16, which is above a third, on 3 streams, with 4 expiry cells, and its median at worst 0.796. So this is Go with the trend on every outcome of `held` but one: an exactness failure not traced to a test bug, which is Stop. This is arithmetic on the rule, not the verdict.

## What this reading does not say

1. **M1D04-R1 cost two passes, and it stands.** On wiki-talk insert the best arm is A5, stopped at 300 s with 14.3% and 5.6% of the batches done. It enters at its limit, so PR scores 0.922 and 1.363 and neither cell passes. Against the arms that finished, PR's slowest over M5's fastest is 0.322 and 0.463. The reading was fixed before this clock to be harder on the sharing arm, and that is what it did. It does not reopen, and 6 is the count.
2. **M did not decide it, and the reading before this run was wrong.** `log.md` 0024 expected Pivot or a thin Go, resting on M. Of 12 p = 5 cells R2, the scan, is best on 8, M5 on 2, stopped A5 on 2. What moved was the clock's price per operation: PR 0.56 ns, R2 0.69, M5 0.90, WT 1.69, WJ 8.26. M1D03's count seconds pointed this way and could not be cited.
3. **On most sx cells PR wins by cache cost, not by work.** WJ does fewer charged operations than PR on 6 of the 8 sx cells at p = 5, listed below, and still takes about 3 to 26 times as long. The unit-cost model does not price a random probe into a table of several GB. So the result is tied to this memory hierarchy, on one machine and one core. PR leads on operations as well only on the two sx expiry 1,000 cells and on wiki-talk expiry, among the arms that finished.
4. **WJ's margin is not uniform.** To undo the two sx insert passes it would have to run about 17 times faster. To undo the two sx expiry passes, about 2.6 times faster, where a better flat table is not out of the question. With those two gone, and no held-out cell passing, 4 of 16 would be Pivot.
5. **The gain needs a large batch.** Cut 1,000 passes on 5 of 6 cells and cut 100 on 1 of 6. Cut 1,000 batches are about ten times the size of cut 100. sx-superuser insert 100 missed at 0.678 against 0.67.
6. **Most of the passes are against a scan.** R2 is PR's own kernel without sharing. That is the honest baseline on this clock, since the tuned tables are slower, but it reads as "sharing pays against its unshared self, and the tables lost on memory".
7. **p = 4 is not on a smooth way from p = 3 to p = 5.** On wiki-talk insert M4 takes 15.7 s against P4's 58 and 79 s and S4's 81 s, a score of 3.699 and 5.016, the worst in this sweep, while M5 at 890 s loses to PR at 277 and 409 s. The median trend holds, and this cell does not fit a single line through it.
8. **M was tuned on small streams.** The engineering cells run in fractions of a second. p = 3 and p = 4 chose 16, the end of the grid, with the curve rising from there, so M might do better lower still. That cannot change a pass at p = 3 or 4, where none exists. p = 5 chose 128 inside the grid, and the curve is flat, 0.147 to 0.185. Running M5 over the grid on evaluation cells would favour the baseline, and it belongs in M1D06.
9. **The three streams are not unseen,** as the protocol says. Only sx-stackoverflow is.
10. **Novelty does not move.** The trend itself is published by Cheetah (`log.md` 0019). A clean result here is a gain against the strongest baseline, closed-form kernels, sequential, all costs inside, and no more.

## Scores, cell by cell

The slowest and fastest columns are the pass test. A cell passes at 0.67 or less. Seconds are the timing build under the exclusive claims. A stopped best enters at its limit, by reading M1D04-R1.

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
| wiki-talk-temporal | real timestamps, insert-only | 1,000 | PR | 276.627 | 286.620 | A5 (stopped) | 300.000 | 300.000 | 0.922 | 0.955 | no | 0.284 |
| wiki-talk-temporal | real timestamps, insert-only | 100 | PR | 408.891 | 414.004 | A5 (stopped) | 300.000 | 300.000 | 1.363 | 1.380 | no | 0.420 |

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
| wiki-talk-temporal | real timestamps, insert-only | 1,000 | P4 | 58.154 | 59.041 | M4 | 15.723 | 15.702 | 3.699 | 3.760 | no | 0.712 |
| wiki-talk-temporal | real timestamps, insert-only | 100 | P4 | 78.726 | 78.871 | M4 | 15.696 | 15.682 | 5.016 | 5.029 | no | 0.970 |

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
| wiki-talk-temporal | real timestamps, insert-only | 1,000 | P3 | 2.172 | 2.274 | S3 | 2.268 | 2.250 | 0.958 | 1.011 | no | 0.958 |
| wiki-talk-temporal | real timestamps, insert-only | 100 | P3 | 2.299 | 2.316 | S3 | 2.291 | 2.261 | 1.003 | 1.024 | no | 1.003 |

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
| wiki-talk-temporal | real timestamps, insert-only | 1,000 | 276.63 (3) | 975.37 (3) | 891.53 (3) | MEM | CAP | TIME |
| wiki-talk-temporal | real timestamps, insert-only | 100 | 408.89 (3) | 972.42 (3) | 895.01 (3) | MEM | CAP | TIME |

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
| wiki-talk-temporal | real timestamps, insert-only | 1,000 | 58.154 | 412.821 | 0.141 |
| wiki-talk-temporal | real timestamps, insert-only | 100 | 78.726 | 600.401 | 0.131 |

## Operations against the clock

13 of 36 cells name a different winner. M's operations come from `count/` at the chosen theta, the other arms' from M1D03.

| Stream | Label | Cut | p | Fewest operations | Fastest on the clock |
|---|---|---:|---:|---|---|
| sx-askubuntu | real timestamps, insert-only | 1,000 | 3 | S3 | P3 |
| sx-superuser | real timestamps, insert-only | 1,000 | 3 | S3 | P3 |
| wiki-talk-temporal | real timestamps, insert-only | 1,000 | 3 | S3 | P3 |
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

Nanoseconds per charged operation, median over the cells where the arm finished in both builds. Streams: the twelve evaluation cells, both labels.

| Arm | Cells | Median | Lowest | Highest |
|---|---:|---:|---:|---:|
| S3 | 12 | 8.63 | 6.59 | 11.60 |
| P3 | 12 | 6.45 | 4.68 | 9.21 |
| T3 | 10 | 3.32 | 2.68 | 3.55 |
| M3 | 12 | 2.72 | 0.91 | 3.61 |
| S4 | 12 | 0.37 | 0.27 | 0.80 |
| P4 | 12 | 0.60 | 0.28 | 1.36 |
| T4 | 10 | 3.27 | 2.67 | 3.54 |
| M4 | 12 | 1.63 | 0.49 | 3.26 |
| B4 | 12 | 0.54 | 0.29 | 0.96 |
| A4 | 9 | 0.95 | 0.75 | 1.41 |
| R2 | 12 | 0.69 | 0.36 | 0.82 |
| PR | 12 | 0.56 | 0.31 | 1.14 |
| WT | 10 | 1.69 | 1.33 | 2.65 |
| WJ | 8 | 8.26 | 3.06 | 15.35 |
| M5 | 12 | 0.90 | 0.73 | 1.28 |
| A5 | 6 | 0.66 | 0.57 | 0.81 |
