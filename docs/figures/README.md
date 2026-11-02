# Figures

The two §16 figures the PI fixed on 2026-11-01 (`log.md` 0018): one worked batch drawn from a logged run, and one stream timeline. Both are plain SVG from `scripts/figures.py`, built from logged runs only, safe in greyscale. No plotting library is installed on this machine, and none was fetched. PNG copies here were rendered with headless Edge, outside the Claude app (bootstrap §13), for checking only.

## Figure 1, the worked batch: `fig-worked-batch.svg`

**Cell.** sx-askubuntu, real timestamps with synthetic expiry, cut 1,000, p = 5. M1D05's first pass, PR against R2 at 0.592 on the clock.

**Which batch, by a rule fixed before any batch was opened.** Among the batches where PR's logged operations are below R2's, the one at the median of PR over R2, lower index on a tie. That is batch 465 of 1,454, at 0.435, and PR was cheaper on all 1,454. The rule picks a typical batch for sharing, not the best one. Panel (a) then draws the largest star of that batch, which is where sharing shows. Panel (b) shows the whole batch, so the reader sees that 223 of its 341 stars have a single edge and gain nothing.

**Provenance.** `build/worked_batch.exe data/derived/sx-askubuntu.bin 1000 465 docs/figures/worked-batch.csv`, built from `benchmarks/worked_batch.cpp` with the project flags (`-std=c++20 -O2 -Wall -Wextra -static -lpsapi`, count build). It replays the stream with Gate 1's PR engine and runs batch 465 star by star, as `Engine5::apply` does. It reproduces the logged row exactly: 177,236 operations and a count of 9,394, as M1D03's `sx-askubuntu-expiry-1000-p5-PR.csv` holds. Each star's spokes sum to PR's delta for it. R2's figure per edge is read in the same graph state, and R2's batch total is M1D03's logged row, 407,745.

**What it does not show.** Operations, not time. On the clock PR over R2 on this cell is 0.588 (M1D04), a smaller gain than the 0.435 in operations here.

## Figure 2, the stream timeline: `fig-stream-timeline.svg`

Same cell, all 1,454 batches. (a) The 5-cycle count after each batch, which every arm agrees on. (b) The operations each arm has spent so far: R2, PR, WT and WJ from M1D03's count builds, M5 at theta 128 from M1D04's. PR ends at 236 M against WJ's 349 M and R2's 556 M. On this cell PR leads in operations too, as `results/gate1-M1D04/README.md` notes. On the sx insert cells WJ does fewer operations than PR and loses on the clock, so this timeline is not the general case, and the paper will say so beside it.
