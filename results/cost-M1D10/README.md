# M1D10: the cost model fitted on the engineering streams

Protocol: `docs/experiments/m1d10-cost-engineering.md`, committed as 493f478 before any run. Run: M1D10C, `results/gate1-M1D10/`. Reading: `scripts/m1d10_cost.py`, run once after release and the overlap check, output in `read.txt`, coefficients in `fit.csv`, per evaluation arm-cell predictions in `cells.csv`. Log: `log.md` 0065.

**Outcome, by the rule: Carries.** Fitted on the 12 engineering cells, where hard rule 5 says parameters are chosen, pricing by kind of operation names the clock winner on 63 of the 64 cells of M1D07's 12 new streams, while unit cost names 21. The median error is 23.0%. M1D09 also read Carries, so the weaker of the two outcomes, which section 6 claims, is **Carries**. Neither fit shows a cache price, so the paper does not claim one. On M1D05's three streams, which M1D09 fitted on, this fit names only 22 of 36 winners. That is a limit, and it is read below.

## The run, M1D10C

| | |
|---|---|
| Claim | One `--exclusive --timing` tower claim, 22:19:12 to 22:49:27 on 2026-11-04, on ControlPC's GO and the PI's word (`log.md` 0064). Line 2 hours, not reached |
| Work | Timing build, 22:19:12 to 22:36:31: 824 timed processes on the 12 engineering cells, 1 thread on logical CPU 8. Count build of every arm, 22:36:31 to 22:49:24 |
| Binaries | M1D04's `timing.exe` and `count.exe` by hash, checked by the chain before the claim and by the runner after it. `results/gate1-M1D10/hashes.txt` |
| Exact | Arms agree per batch on every cell. `mcheck` 0 differences on every M arm. `gate1_score.py exact` against M1D02B: 36 groups, 1,208 files, 1,172 pairs, 2,524,286 batch comparisons, 0 files differ (hard rule 1) |
| Stopped at a limit | A5 on sx-mathoverflow insert-only, cut 100, at 300 s, in both the timing and the count build, as the protocol fixes. It is not scored and not fitted |
| Start | The chain needs at least 14 GiB of free RAM. A retry loop inside ControlPC's window of 22:10 to 23:25 started the chain only when free RAM read 14 GiB. The chain's own check then read 13 GiB on three attempts, at 22:10:03, 22:15:07 and 22:18:09, and claimed nothing. The fourth, at 22:19:11, passed. A refused attempt makes no claim. `results/gate1-M1D10/chain.log`, `ram-wait.txt` |

**Background load, read after release from ControlPC's run records** (`scripts/overlap_check.py M1D10C`). The 22:06 status mail ended at 22:07:34, before the claim. Research 1's `Status` and `FollowOnGate` tasks started at 22:30:01, and `FollowOnGate` again at 22:45:01. Their lengths are not recorded, so each is taken as 300 s. ControlPC ran nothing inside the claim, and Research 12, 9 and 1 were told to read and edit only. No process table was sampled during the claim, so other load cannot be ruled out. Two timed processes cover a known event. One is the A5 run stopped at 300 s, which is not scored. The other is one of R2's five repeats on sx-mathoverflow insert-only, cut 100, at +0.2% over its arm's median, and not its fastest. The 22:45:01 task fell during the count builds, which give no seconds to any reading. Of the 789 repeats that cover no event, 15 run more than 3% over their arm's median.

## Sets

| Set | Streams | Cells with an arm-cell | Arm-cells | Tables over L3 |
|---|---|---|---|---|
| Fitting | CollegeMsg, email-Eu-core-temporal, sx-mathoverflow, insert-only and expiry, cuts 100 and 1,000, p = 3, 4, 5 | 29, of which 24 have two or more | 102 | 10, all on sx-mathoverflow insert-only |
| Evaluation, the outcome | M1D07's 12 new streams, as in M1D09 | 66, of which 64 have two or more | 276 | 102 |
| Evaluation, beside it | M1D05's three streams, sx-askubuntu, sx-superuser, wiki-talk-temporal | 36 | 173 | 42 |

The protocol estimated about 104 fitting arm-cells and 14 over L3. The 0.05 s floor and the stopped A5 run left 102 and 10.

## The readings

| Model | Set | Median error | Winner agrees | PR's side of 0.67, p = 5 |
|---|---|---|---|---|
| U, unit cost | fitting | 67.1% | 10 of 24 | 5 of 6 |
| U | M1D07 | 71.3% | **21 of 64** | 12 of 32 |
| U | M1D05 | 64.1% | 23 of 36 | 7 of 12 |
| K, by kind | fitting | 19.5% | 18 of 24 | 6 of 6 |
| K | M1D07 | 23.5% | 63 of 64 | 30 of 32 |
| K | M1D05 | 34.1% | 23 of 36 | 10 of 12 |
| KC, by kind and cache | fitting | 16.2% | 18 of 24 | 6 of 6 |
| KC | M1D07 | **23.0%** | **63 of 64** | 30 of 32 |
| KC | M1D05 | 25.8% | 22 of 36 | 10 of 12 |

The outcome test is KC against U on M1D07. Winner agreement gains 65.6 points, against the 25 needed, and the median error is 23.0%, against a limit of 25%. Both hold. The one M1D07 cell KC gets wrong is soc-redditHyperlinks-title expiry 100 at p = 3, where the clock picks S3 and KC picks P3.

**Cache, by the rule fixed before the run.** KC against K on M1D07: 63 against 63 winners, 23.0% against 23.5% median error. That is neither 3 winners nor 5 points, so the cache price is not shown here, as it was not in M1D09. The paper does not claim it.

## Coefficients, ns per counted operation, beside M1D09's

| Counter | K, M1D10 | KC, M1D10 | KC, M1D09 |
|---|---|---|---|
| scan | 0.46 | 0.46 | 0.56 |
| row | 0.07 | 0.05 | 0.23 |
| probe | 11.38 | 10.49 | 17.64 |
| deg | 0.00 | 0.00 | 2.03 |
| tab_read | 2.16 | 2.18 | 0.96 |
| tab_write | 2.33 | 1.47 | 4.87 |
| graph | 25.64 | 21.29 | 0.00 |
| net_ops | 24.68 | 30.53 | 21.28 |
| group | 0.00 | 0.00 | 4.68 |
| rc | 0.59 | 0.58 | 0.78 |
| probe, extra over L3 | | 0.00 | 0.00 |
| tab_read, extra over L3 | | 0.00 | 1.71 |
| tab_write, extra over L3 | | 3.79 | 0.15 |

U's one price is 0.41 ns, against 0.50 in M1D09. As in M1D09, the counters are correlated across arms, so no single coefficient is a measured price. What holds across the two fits is the ratio: a table probe costs about 10 to 18 ns, and a row or scan step 0.05 to 0.6 ns. The extra price over L3 moves from table reads in M1D09 to table writes here, which is what a term fitted on one stream (here) or on few cells would do. Neither fit gains from it on unseen streams.

## Read beside it

The protocol fixes the M1D05 reading and the WJ ratio as readings beside the outcome. The breakdown of M1D05's misses below was not fixed, and it is labelled as unplanned.

- **M1D05: the fit misses 14 of 24 cells at p = 3 and p = 4, and none at p = 5.** On every miss the clock picks the scan arm, S3 or S4, or once M4, and KC picks P3 or P4. The runner-up is 0.3% to 28% slower. The engineering fit prices a row step at 0.05 ns, against 0.23 in M1D09, and a degree read at zero, which is consistent with P coming out too cheap on the larger streams. No test here separates those two counters. M1D07 has no p = 4 cell and P3 or M3 wins most of its p = 3 cells, so its outcome does not see this. Unplanned breakdown, from `cells.csv`.
- **WJ is still underpriced.** KC puts WJ at a median 0.15 of its clock on M1D07 and 0.30 on M1D05, against 0.18 in M1D09. Fitting on sx-mathoverflow insert-only, where WJ's table is 768 MB, does not price it. What separates WJ from WT is not in any counter, table size included.
- **Other arms, KC, prediction over clock on M1D07:** between 0.76 and 1.15, apart from A5 at 1.30.
- **The winners are not hard to name at p = 5.** PR wins all 32 M1D07 cells there, as in M1D09.
- **One machine, one P-core.** Every price is this machine's.

## What section 6 can say

Pricing each kind of operation, fitted on the small engineering streams and read on 12 larger ones never seen, names the clock winner on 63 of 64 cells, and unit cost names 21. Fitted instead on the first three evaluation streams, it names all 64. Either way a table probe costs about 10 to 18 ns, against 0.05 to 0.6 ns for a row or scan step. The section states two limits beside that. The engineering fit does not carry back to the first three streams at p = 3 and 4, where it prefers P to the scan on 14 of 24 cells. And no fit finds a cache-size price or explains WJ's time.
