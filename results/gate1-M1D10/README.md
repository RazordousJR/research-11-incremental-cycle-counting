# M1D10C: every arm timed and counted on the 12 engineering cells

Protocol: `docs/experiments/m1d10-cost-engineering.md` (493f478). Runner: `scripts/chain_m1d10.sh` and `scripts/run_m1d10.sh`. The reading of this run is `results/cost-M1D10/README.md`. Log: `log.md` 0065.

One `--exclusive --timing` tower claim, 22:19:12 to 22:49:27 on 2026-11-04, logical CPU 8, M1D04's binaries by hash. Exact: 0 differences per batch, by `mcheck` and by `gate1_score.py exact` against M1D02B. Stream labels: real timestamps, insert-only, and real timestamps, synthetic expiry.

| File | What it is |
|---|---|
| `chain.log` | Every chain attempt, the claim and the release |
| `ram-wait.txt` | The retry loop's RAM reads before the claim |
| `tower-check.txt` | The tower check of the last attempt |
| `run.log` | The runner's output: every timing and count process, `mcheck`, `gate1_score.py exact` |
| `hashes.txt` | Commit, sources, binaries, `theta.txt` and inputs, by SHA-256 |
| `theta.txt` | M1D04's theta, copied |
| `raw/`, `count/raw/` | Per-batch and summary files, not in git |
