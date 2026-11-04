# M1D10: the cost model fitted on the engineering streams

Written on 2026-11-04, after M1D09 read Carries (`log.md` 0063) and before any M1D10 run. The PI accepted M1D09's reading of hard rule 5 and asked for this run as well (`log.md` 0064). Any change to what is fixed here gets a new ID and a `log.md` entry.

## The question

M1D09 fitted its prices on M1D05's streams, which had already been read, and judged them on M1D07's streams, which no coefficient saw. Hard rule 5 says parameters are chosen on engineering streams. M1D10 fits the same three models on the engineering streams, timed for every arm, and reads them on the same unseen streams. It asks: **do the prices hold when they are fitted where the rule says, on small streams, and read on large ones?**

**A correction to M1D09.** Its protocol says the engineering tables "fit in the 30 MB last-level cache". That is wrong for sx-mathoverflow insert-only. There the count build of M1D02B records 409 MB for T3, T4 and WT, 768 MB for WJ and 37.5 MB for M. The other five engineering streams' tables are under 13 MB. So the cache price can be fitted on the engineering streams, but only on one stream. M1D09 stands as run, because its protocol was fixed before its fit, and the correction goes in `log.md` 0064 and in its README.

## Run, fixed now

| | |
|---|---|
| Cells | The 12 engineering cells: CollegeMsg, email-Eu-core-temporal and sx-mathoverflow, insert-only and two-week expiry, cuts of 100 and 1,000. Labels: real timestamps, insert-only, and real timestamps, synthetic expiry |
| Arms | Every arm of Gate 1: S3, P3, T3, M3 at p = 3; S4, P4, T4, M4, B4, A4 at p = 4; R2, PR, WT, WJ, M5, A5 at p = 5. M at M1D04's theta, 16, 16 and 128. Nothing is chosen here |
| Timing | `scripts/run_m1d04.sh`'s steps 2 to 4, unchanged, with the engineering cells as its cells and M1D04's `theta.txt`. Timing build, M1D04's `timing.exe` by hash, one process at a time, pinned to logical CPU 8. Five repeats, an arm run once on a cell where it took more than 5 times the fastest, A4 and A5 stopped at 300 s, the rest at 1,200 s. Arms agree per batch after each cell, or the run stops (hard rule 1) |
| Count | Then the count build of every arm on every cell, M1D04's `count.exe` by hash, so the counters and the clock come from one binary. Every timing repeat against every count build and against M1D02B's count builds, per batch, by `gate1_score.py exact`. Any difference stops the run |
| Claim | One `--exclusive --timing` claim, on ControlPC's GO, with a line of 2 hours. Estimate from M1D02B's count builds, which run at about 1.05 times the timing build: about 20 minutes of timing and 15 of counting, plus process start |
| Runner | `scripts/run_m1d10.sh`, inside `scripts/chain_m1d10.sh`, which checks the tower, memory and both binaries' hashes before it claims, and releases whatever the exit |

Background load is read after release, as in M1D08, from ControlPC's run records.

## Reading, fixed now

`scripts/m1d10_cost.py`, run once, output in `results/cost-M1D10/`. It uses `scripts/m1d09_cost.py`'s arm-cells, models, fit and readings unchanged, so U, K and KC, the 10 counters with `upkeep` left out, the 30,720 KB line, and arm-cells under 0.05 s left out.

- **Fitting set.** The 12 engineering cells at p = 3, 4 and 5. From M1D02B's count builds, about 104 arm-cells remain over 0.05 s, 14 of them with a table over the last-level cache, all on sx-mathoverflow insert-only. If no arm-cell with a table over the cache remains, KC's extra prices are zero by construction, KC equals K, and the reading says so.
- **Evaluation set.** M1D07's 12 new streams, as in M1D09: the outcome is read here, by M1D09's rule. **Carries** if KC beats U on winner agreement by at least 25 points and KC's median error is at most 25%. **Partly** if one holds. **Does not carry** if neither does.
- **Second evaluation set, read beside it.** M1D05's three streams, which M1D09 fitted on. They are unseen by this fit.

## What the paper says, fixed now

| | |
|---|---|
| Coefficients | Section 6 gives M1D10's, because they are fitted where hard rule 5 says. M1D09's are given beside them |
| Outcome | Section 6 claims the weaker of M1D09's and M1D10's outcomes. If they differ, both are given and the section says which fit fails and where |
| Cache | The paper claims a cache price only if KC beats K on the evaluation set in both M1D09 and M1D10, by at least 3 of 64 winners or 5 points of median error. M1D09 does not show one, so M1D10 cannot add the claim. It can only show again that it is absent, now with tables over L3 inside the fit |
| WJ | M1D09 put WJ at a median 0.18 of its clock. M1D10's fit sees WJ's slowest engineering cells. The prediction over clock for WJ is read beside it and named in section 6, whatever it is |

## Order of work

| ID | Step | Machine |
|---|---|---|
| M1D10A | This file, `scripts/run_m1d10.sh`, `scripts/chain_m1d10.sh` and `scripts/m1d10_cost.py`, committed before any run | No claim |
| M1D10B | Dry run of `scripts/run_m1d10.sh` on CollegeMsg expiry, output in the scratchpad, seconds thrown away | Short ordinary claim, ControlPC's GO |
| M1D10C | The run | One `--exclusive --timing` claim, ControlPC's GO and the PI's word (`log.md` 0064) |
| M1D10D | `scripts/m1d10_cost.py`, once, after release and the overlap check | No claim |
