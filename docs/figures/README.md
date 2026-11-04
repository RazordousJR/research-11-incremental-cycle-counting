# Figures

The two §16 figures the PI fixed on 2026-11-01 (`log.md` 0018): one worked batch drawn from a logged run, and one stream timeline. Both are plain SVG from `scripts/figures.py`, built from logged runs only, safe in greyscale. No plotting library is installed on this machine, and none was fetched. PNG copies here were rendered with headless Edge, outside the Claude app (bootstrap §13), for checking only.

## Figure 1, the worked batch: `fig-worked-batch.svg`

**Cell.** sx-askubuntu, real timestamps with synthetic expiry, cut 1,000, p = 5. M1D05's first pass, PR against R2 at 0.592 on the clock.

**Which batch, by a rule fixed before any batch was opened.** Among the batches where PR's logged operations are below R2's, the one at the median of PR over R2, lower index on a tie. That is batch 465 of 1,454, at 0.435, and PR was cheaper on all 1,454. The rule picks a typical batch for sharing, not the best one. Panel (a) then draws the largest star of that batch, which is where sharing shows. Panel (b) shows the whole batch, so the reader sees that 223 of its 341 stars have a single edge and gain nothing.

**Provenance.** `build/worked_batch.exe data/derived/sx-askubuntu.bin 1000 465 docs/figures/worked-batch.csv`, built from `benchmarks/worked_batch.cpp` with the project flags (`-std=c++20 -O2 -Wall -Wextra -static -lpsapi`, count build). It replays the stream with Gate 1's PR engine and runs batch 465 star by star, as `Engine5::apply` does. It reproduces the logged row exactly: 177,236 operations and a count of 9,394, as M1D03's `sx-askubuntu-expiry-1000-p5-PR.csv` holds. Each star's spokes sum to PR's delta for it. R2's figure per edge is read in the same graph state, and R2's batch total is M1D03's logged row, 407,745.

**What it does not show.** Operations, not time. On the clock PR over R2 on this cell is 0.588 (M1D04), a smaller gain than the 0.435 in operations here.

## Figure 2, the stream timeline: `fig-stream-timeline.svg`

Same cell, all 1,454 batches. (a) The 5-cycle count after each batch, which every arm agrees on. (b) The operations each arm has spent so far: R2, PR, WT and WJ from M1D03's count builds, M5 at theta 128 from M1D04's. PR ends at 236 M against WJ's 349 M and R2's 556 M. On this cell PR leads in operations too, as `results/gate1-M1D04/README.md` notes. On the sx insert cells WJ does fewer operations than PR and loses on the clock, so this timeline is not the general case, and the paper will say so beside it.

## Figures 3 to 6: `scripts/figures_results.py`

Plain SVG in the same style, from logged runs only, drawn on 2026-11-04 after M1D08 and M1D09 (`log.md` 0063). PNG copies were rendered with headless Edge for checking only.

- **Figure 3, the test set: `fig-test-set.svg`.** All 20 streams: 3 engineering, 3 evaluation, the held-out sx-stackoverflow, M1D07's 12 new and the snapshot. Each row gives the role, the label (with the deletion share for the KONECT prefixes), vertices and events on log scales, and the pooled p = 5 passes on the P-core record (M1D07E), 28 of 50. Engineering sizes are taken from M1D02's summaries, evaluation sizes from M1D04's, and new sizes from the sidecar files `data/m1d07/derived/*.bin.txt`.
- **Figure 4, score against pattern size: `fig-score-by-p.svg`.** Every scored cell of M1D04 and M1D07, the snapshot left out, by `gate1_score.score` with reading M1D04-R1. Medians 0.965, 1.299 and 0.650 at p = 3, 4 and 5, with 0, 0 and 28 passes, as recorded.
- **Figure 5, the cost model: `fig-cost-model.svg`.** M1D09's 276 evaluation arm-cells, predicted against measured seconds, unit cost beside priced by kind and cache (`results/cost-M1D09/cells.csv`). The caption states that WJ is underpriced about fivefold, and that 33 unit-cost predictions under 0.03 s sit on the axis floor.
- **Figure 6, M5 over theta: `fig-theta.svg`.** M1D06A's theta grid on M1D05's six passes, M5's median over R2's median, with PR's. One panel per pass.

Figures 3 to 6 are plots, and plots carry the argument. The two figures of the system in operation are 1 and 2.

## For JGAA, 2026-11-05

All six figures were redrawn to a 425-unit viewBox, the 15 cm text width of the JGAA class, with no text under 10 pt, and the grey caption lines moved out of the figures into `captions.md`, which the scripts write. `scripts/figures_pdf.py` prints each SVG to a vector PDF in `pdf/` with headless Edge and checks size, fonts and the 10 pt rule; the PNGs here are now rendered from those PDFs. Internal run names were removed from the figure labels. The paper's captions are in `docs/paper/jgaa/figures.tex` (`log.md` 0067).
