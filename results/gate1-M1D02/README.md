# M1D02: regression and agreement, engineering cells

Protocol: `docs/experiments/gate1-protocol.md`. Non-citable. Count build, one process at a time, pinned to logical CPU 8, ordinary tower claim on ControlPC's GO, released 19:52:12 local time.
All streams here are labelled real timestamps with a synthetic expiry rule (expiry), or real timestamps, insert-only, no expiry (insert). Theta 64 in the M arms, for agreement only: no theta is chosen here.

## Result

- **Regression holds.** R2, PR, WT and WJ match M1C03 on count and operations, per batch, on all 12 engineering cells: 48 arm-cells, 0 batches differ. The two additive edits to `benchmarks/fivecycle.cpp` changed no count and no charge.
- **All arms agree.** On each cell and pattern, every arm gives the same count on every batch both finished. 192 processes, 0 mismatches.
- **Agreement is partial on two arm-cells.** A5 hit the 120 s limit on sx-mathoverflow insert: cut 1,000 at 382 of 391 batches, cut 100 at 1,484 of 3,905 batches. Those batches agree, and the rest are uncompared.
- **Past a minute:** the two A5 runs above, and A4 on sx-mathoverflow insert cut 100, which finished at 114 s. Every other process stayed under a minute. Largest resident size 1.23 GB. Total process time 574 s.

## What the operations show

Operation totals are in `run.log`. These are engineering cells and enter no verdict.

- p = 3: S3 has the lowest operations on all 12 cells, and P3 is 1.2 to 2.9 times S3. Gate 0 repeated on a separate kernel.
- A4 and A5 win no cell. On sx-mathoverflow insert, A5 had already spent about 20 and 27 times PR's whole run when it was stopped. The nearest case is sx-mathoverflow expiry cut 1,000, where A4 is 2.07 times P4.
- B4 is dearer than P4 on all 12 cells, by 3.0 to 5.7 times.
- **P4 lost to S4 on sx-mathoverflow insert and on sx-mathoverflow expiry cut 100, on operations.** An unguarded row at a hub always walks from x, while S4 walks from the lower-degree side. That is a careless form of the sharing arm, so P3 and P4 took a guard after this run (`log.md` 0020). The P3 and P4 figures in `run.log` are the unguarded forms.
- M at theta 64 stays close to T on CollegeMsg and email-Eu-core. On sx-mathoverflow insert it moves both ways: M3 is 0.28 times T3, and M5 is 3.3 times WT. Theta 64 is not a chosen value, so this is not a test of M.

## Limits of this record

The binary that made these files had sha256 starting b8712ff15d58, built from commit 895d70f. The source changed after this run, in two ways (`log.md` 0020): the tables' own 6 GB limit is raised for Gate 1, and P3 and P4 take a guard. Five arm-cells rerun after the first change, before the
guard, gave identical raw files. The theta grid, the evaluation cells and the clock are untested here.
