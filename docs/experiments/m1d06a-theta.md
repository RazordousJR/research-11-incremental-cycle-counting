# M1D06A: M5 over the theta grid, a sensitivity on the Gate 1 passes

Fixed on 2026-11-03, after M1D05 and before any M1D06 run. The PI chose to attack the fragile passes first. `log.md` 0033 says why this is the attack and a stronger flat table is not.

## What it can and cannot do

M1D05's verdict stands whatever this finds: Go with the trend, by the rule as written. This is a sensitivity reported beside it. It asks one question: does any p = 5 pass fall when M5 gets the theta that suits it best on that very cell? That is an oracle choice on evaluation streams, which favours the baseline. So it can only make the test harder for the sharing arm, and hard rule 5 is not at stake: no parameter of the sharing arm is chosen here.

## Cells

At p = 5, real timestamps with synthetic expiry and real timestamps insert-only, cuts 1,000 and 100:

| Stream | Variants | Why |
|---|---|---|
| sx-askubuntu | expiry, insert | Holds 2 passes and 2 near misses |
| sx-superuser | expiry, insert | Holds 2 passes and 2 near misses |
| wiki-talk-temporal | expiry | Holds 2 passes |

10 cells, all 6 passes among them. Left out: wiki-talk insert, where PR already fails against a stopped A5 and M5 takes about 900 s a run, and sx-stackoverflow, where PR passes nothing and the scored cells take about 290 s a run of M5. Neither can lose a pass.

## Arms and measure

PR and R2 as in M1D04, and M5 at each theta of {16, 32, 64, 128, 256, 512, 1024}, 128 included so that every theta runs in the same claim. Same timing build, same binary, same hashes as M1D04, pinned to logical CPU 8, one process at a time, an `--exclusive` claim. Limits 1,200 s and 16 GB. Repeats as the Gate 1 protocol: 5 in all on a cell whose fastest arm took under 60 s in repeat 1, 3 otherwise, and an arm past 5 times the fastest, or not finished, runs once. Arms are interleaved in a rotated order across repeats. Then the count build of M5 at each theta, checked per batch against its own timing build. Every arm's per-batch count must match every other's in the cell. A single mismatch stops the run (hard rule 1).

## Reading, fixed now

For each passing cell, with all figures from this claim: the cell **holds** if PR's slowest repeat is at most 0.67 times the lowest fastest repeat of R2 and of M5 at any theta. Otherwise it **falls**. The result is the count of the 6 that hold, and what the M1D05 rule would give with the fallen ones counted as fails: at least a third of 14 is 5. This is a sensitivity and not a second verdict.

Read beside it: the best theta per cell and its margin; whether one theta suits every cell or each needs its own; the near misses at cut 100, which can only grow; and drift, PR's and R2's medians here against M1D04's. A drift past 3% on a cell is reported, and that cell is read with both claims.

## Cost, before any run

From M1D04's seconds at theta 128: M5 about 190 s a repeat over the 10 cells, PR about 80 s, R2 about 160 s. With 7 thetas at up to 5 repeats each, plus the count build, the central estimate is 2.5 hours. The high one is 5 hours, where an extreme theta runs near the table's speed. A theta that slow on a cell runs once, by the repeat rule. `LINE_HOURS=5`, checked before each process.

## Machine

`--exclusive` needs the PI's word on the hours and ControlPC's GO. Script `scripts/run_m1d06a.sh`, helper `scripts/m1d06a.py`, output `results/gate1-M1D06A/`.
