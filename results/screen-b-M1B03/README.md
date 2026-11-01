# M1B03: bounded-memory shared-partner index

Run on 2026-11-01. Rule: `docs/experiments/screen-b-protocol.md`, section M1B03, not changed. Source: scout S3, Candidate 1.
Non-citable. Code: `benchmarks/ergm_chain.cpp`, with `src/r11/tc.hpp` not edited (SHA-256 prefix `d3db4c7dec77167c`), built with g++ 16.2.0
and `-std=c++20 -O2 -Wall -Wextra -static`. Nothing is committed. The tower was not touched.

## Verdict: Stop

Step 1 passes on every graph. Step 2 crosses the first Stop line on one real graph. On wiki-talk peak at 5% acceptance, scan costs
1.15 times split at the best |H| (64), against a line of 1.5. Every larger |H| is lower still. A scan with no memo at all gets 1.33.

**The rule has a gap, and it decides the word.** The M1B03 Stop line names no graph count, while M1B01 says "on all 6 streams" and
M1B02 says "on any state". The result is mixed, so the reading matters.

| Reading of the Stop line | Verdict | Status |
|---|---|---|
| It fires if any listed graph crosses it | **Stop** | Adopted: the plain wording, and the one that does not favour the candidate |
| Stop needs all graphs, and a mixed result is "anything else", as in M1B01 and Gate 0 | Pivot | Defensible, not adopted |
| Stop needs all graphs, and "neither" means "not on all" | Go | Not adopted: it stretches "neither" and would be chosen after seeing the numbers |

Choosing a reading after the results is itself a choice, so it is stated here for the PI. Changing the rule needs a new screen ID.

The second Stop line does not fire at |H| = 64: split / eager at 1% is 0.84 to 1.87. The memory clause holds on all four real graphs at
|H| = 64, at 0.012 to 0.278 of eager.

The Stop is not the whole picture. The two large real graphs are far from the line, at 6.10 and 10.84, and those are the sizes the
scout's users work at. The crossing comes from a star-shaped graph where scanning is already cheap: see "Why the graphs differ". Counted
over only 10^7 proposals, the one-off table build reverses part of this: see "Build cost".

## Step 1

`E(tau)` as a share of the total, with tau the largest degree in L. The true L sum differs slightly (0.159 against 0.149 on sx-superuser
peak at |H| = 64), and both are in `step1.csv`.

| Graph | Label | n | m | E(inf) | Share at |H| = 64 | Share at |H| = 4096 |
|---|---|---:|---:|---:|---:|---:|
| sx-superuser peak | real timestamps, synthetic expiry | 4,348 | 7,172 | 201,909 | 0.149 | 0.000 |
| wiki-talk-temporal peak | real timestamps, synthetic expiry | 73,932 | 99,914 | 278,098,974 | 0.004 | 0.000 |
| sx-superuser end state | real timestamps, insert-only | 192,409 | 714,570 | 436,408,980 | 0.187 | 0.009 |
| wiki-talk-temporal end state | real timestamps, insert-only | 1,094,018 | 2,787,967 | 16,576,638,535 | 0.069 | 0.002 |
| rMAT scale 16, factor 16 (Step 2 graph) | fully synthetic | 46,733 | 910,197 | 622,111,823 | 0.521 | 0.016 |
| rMAT scale 20, factor 16 | fully synthetic | 646,451 | 15,701,849 | 35,037,527,200 | 0.647 | 0.101 |

**Pass: no Stop.** Every graph is below 0.5 by |H| = 4096. I read "at |H| <= 4096" as "no |H| in range gets below half". A reading that
requires every |H| in range to pass would fire on both rMAT graphs at |H| = 64, and on scale 20 at 128 as well. The real graphs pass on
either reading. On sx-superuser peak, |H| = 4096 is 94% of all vertices, so that cell is trivial.

Peak edge counts agree with Gate 0: 7,172 against 7,165 and 99,914 against 99,908, since Gate 0 reads at batch boundaries. rMAT uses
Graph500 parameters, seed 20261101. Scale 20 was picked first. Its wedge count rules out a table in RAM, so scale 16 was picked for Step 2
on wedge count alone, and both rows are shown.

## Step 2 setup

- **Model and chain.** Edges plus alternating k-triangle, lambda 2. EstimNetDirected's `changeAltKTriangles` and its add-or-delete TNT move
  were read through raw files. The Hastings ratio here is the exact one for that move, where EstimNet reuses the statnet expression.
- **Delete moves.** EstimNet removes the edge to score a delete and reinserts it on rejection, so its table pays a commit per delete
  proposal. Here a delete uses `cn(i,v) - 1`, for all backends. That makes eager stronger than the tool.
- **Units, strict.** One list element, one probe, one table read, one table write, one stamp, one mask word. A read-modify-write is 2 and
  an AND of two mask words is 2. The lenient count, 1 each, is in the CSVs. Graph edit 3, edge pick 1, edge test 1, charged to every backend.
- **Scan.** Lower-degree endpoint, `common` copied from `tc.hpp`. Direct-mapped memo keyed by dyad with per-vertex stamps, four sizes
  and gates, plus no memo. **The score uses the cheapest scan whose memory is at most that split's.** Uncapped figures are in the CSV.
- **Early rejection, all backends.** A rejection may stop as soon as bounds on delta prove it. This helps scan most.
- **Split.** As the scout's table, with |H| in 64, 256, 1024, 4096. Hubs and tables are built from the graph at the start of the count.

## Departures you should know about

1. **Burn-in, which the rule does not mention.** Each run first takes max(10 m0, 2 x 10^6) proposals under the same theta, uncounted. From
   a snapshot, edges in no triangle are deleted with probability near 1, so low acceptance cannot be reached: a wiki-talk end-state probe ran
   at 26% in the first tenth of burn-in and settled at 10%. The tables therefore see the burned-in graph, not the snapshot. Edge counts at
   the start of the count are 3% to 86% below the snapshot, by graph and level.
2. **Theta has two parameters and the rule names one.** I tuned (theta_e, theta_t) for acceptance within 10% of target, drift at most 20%
   per 10^7 proposals, and the largest edge count, which is kindest to the tables. The same pair goes to every backend, since they run on
   one chain.
3. **The tuning method changed before production.** A solve on theta_e failed on sx-superuser peak, so that graph used a grid and then a
   walk along the narrow ridge between melting and blow-up, in steps of 0.05. The limits were not changed. `tuning_trials.csv` holds every
   trial. Early rows there use the status `left_band`, later ones `dense` or `melted`.
4. **rMAT 1% misses the drift limit** (0.23) and falls to about 103,000 edges. It is the closest pair found, and I would not lean on it.
5. **Formula rows**, marked * below. Free RAM was about 12 GB of 31. Eager on both end states and rMAT, and split at |H| 64 and 256 on
   wiki-talk end state and rMAT, charge the real backend's costs call by call and take values from a real split table. On the two peaks,
   formula and real rows are **equal to the operation** (`formula_check.csv`). Entry counts on those rows are start counts, and real split
   peaks there ran up to 56% higher. Eager was never built on the end states or rMAT, so those decisions are not independent evidence for it.
6. **rMAT 15% and 5%** hit the 590 s limit with all backends together and left no row. They were repeated as a scan process and a table
   process, tied by an equal decision hash. rMAT |H| = 4096 was run at 1% only. The early-off run on sx-superuser end state also timed out
   and left no row.
7. **The build changed during the campaign**, by additions only. Decision hashes and split operation counts were re-checked and are equal.

## Step 2 results

Operations per proposal, shared costs included. * marks a formula row. Scan is the cheapest within the split-64 memory cap.

| Graph | Label | Target | Reached | theta_e | theta_t | m start to end | Scan | Eager | Split 64 | Split 256 | Split 1024 | Split 4096 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| sx-superuser-peak | synthetic expiry | 15% | 16.21% | -9.934 | 3.475 | 2990 to 2815 | 10.17 | 4.39 | 5.89 | 11.78 | 36.81 | 137.81 |
| sx-superuser-peak | synthetic expiry | 5% | 5.47% | -11.184 | 4.150 | 2589 to 2644 | 9.21 | 3.14 | 4.91 | 11.06 | 36.43 | 138.29 |
| sx-superuser-peak | synthetic expiry | 1% | 0.97% | -13.434 | 5.275 | 2366 to 2499 | 8.31 | 2.74 | 4.75 | 11.29 | 37.87 | 144.32 |
| wiki-talk-temporal-peak | synthetic expiry | 15% | 14.04% | -13.217 | 5.000 | 32923 to 37719 | 6.37 | 3.64 | 5.53 | 11.67 | 36.38 | 135.37 |
| wiki-talk-temporal-peak | synthetic expiry | 5% | 4.57% | -14.717 | 5.000 | 26925 to 27382 | 5.67 | 2.93 | 4.94 | 11.54 | 38.02 | 144.03 |
| wiki-talk-temporal-peak | synthetic expiry | 1% | 0.99% | -16.717 | 8.000 | 27603 to 32954 | 5.46 | 2.38 | 4.45 | 10.73 | 35.89 | 136.60 |
| sx-superuser-ins-end | insert-only | 15% | 14.21% | -12.346 | 8.000 | 553916 to 621420 | 43.21 | 9.20* | 10.18 | 15.55 | 39.07 | 135.83 |
| sx-superuser-ins-end | insert-only | 5% | 4.71% | -13.683 | 8.000 | 493720 to 556670 | 42.22 | 5.69* | 6.92 | 12.51 | 36.42 | 133.83 |
| sx-superuser-ins-end | insert-only | 1% | 1.06% | -18.490 | 10.000 | 454121 to 486271 | 47.01 | 5.60* | 6.64 | 12.27 | 36.90 | 137.54 |
| wiki-talk-temporal-ins-end | insert-only | 15% | 14.25% | -14.645 | 8.000 | 1852975 to 1897326 | 64.53 | 6.59* | 7.78* | 13.52* | 37.24 | 133.71 |
| wiki-talk-temporal-ins-end | insert-only | 5% | 4.93% | -15.879 | 8.000 | 1664957 to 1705669 | 62.34 | 4.49* | 5.75* | 11.60* | 35.56 | 132.44 |
| wiki-talk-temporal-ins-end | insert-only | 1% | 0.96% | -18.099 | 10.000 | 1595647 to 1636137 | 58.61 | 3.58* | 4.99* | 10.89* | 34.95 | 131.98 |
| rmat-s16-f16 | synthetic | 15% | 15.21% | -10.786 | 3.000 | 882373 to 895228 | 1697.65 | 128.83* | 120.87* | 120.40* | 170.97 | not run |
| rmat-s16-f16 | synthetic | 5% | 4.42% | -32.509 | 14.000 | 739418 to 712470 | 1815.97 | 78.50* | 74.29* | 78.89* | 138.05 | not run |
| rmat-s16-f16 | synthetic | 1% | 0.90% | -27.676 | 10.000 | 123151 to 103050 | 373.65 | 28.76* | 24.22* | 39.47* | 109.59 | 391.67 |

Memory, index only, at |H| = 64. The graph itself is common to all and left out. Bytes: 12 per entry, plus 8 per mask word and 1 per
vertex for split. Scan memo cap is the largest memo inside the split-64 memory.

| Graph | Target | Scan memo cap | Split 64 entries | Split 64 MB | Eager entries | Eager MB | Split / eager memory |
|---|---|---|---|---|---|---|---|
| sx-superuser-peak | 15% | 0.1 MB | 8,977 | 0.1 | 45,431 | 0.5 | 0.269 |
| sx-superuser-peak | 5% | 0.1 MB | 8,759 | 0.1 | 43,238 | 0.5 | 0.278 |
| sx-superuser-peak | 1% | 0.1 MB | 7,473 | 0.1 | 40,449 | 0.5 | 0.265 |
| wiki-talk-temporal-peak | 15% | 1.9 MB | 130,143 | 2.2 | 16,241,627 | 194.9 | 0.011 |
| wiki-talk-temporal-peak | 5% | 1.1 MB | 79,791 | 1.6 | 15,336,637 | 184.0 | 0.009 |
| wiki-talk-temporal-peak | 1% | 1.1 MB | 124,583 | 2.2 | 15,445,272 | 185.3 | 0.012 |
| sx-superuser-ins-end | 15% | 403.4 MB | 42,499,263 | 511.7 | 252,832,428* | 3034.0 | 0.169 |
| sx-superuser-ins-end | 5% | 202.1 MB | 41,437,444 | 499.0 | 251,063,865* | 3012.8 | 0.166 |
| sx-superuser-ins-end | 1% | 202.1 MB | 37,091,569 | 446.8 | 248,866,679* | 2986.4 | 0.150 |
| wiki-talk-temporal-ins-end | 15% | 809.7 MB | 263,479,672* | 3171.6 | 5,257,187,435* | 63086.2 | 0.050 |
| wiki-talk-temporal-ins-end | 5% | 809.7 MB | 261,542,492* | 3148.4 | 5,191,845,710* | 62302.1 | 0.051 |
| wiki-talk-temporal-ins-end | 1% | 809.7 MB | 261,076,272* | 3142.8 | 5,169,492,282* | 62033.9 | 0.051 |
| rmat-s16-f16 | 15% | 402.8 MB | 96,837,982* | 1162.5 | 178,177,234* | 2138.1 | 0.544 |
| rmat-s16-f16 | 5% | 402.8 MB | 86,866,011* | 1042.8 | 166,735,403* | 2000.8 | 0.521 |
| rmat-s16-f16 | 1% | 50.5 MB | 7,051,961* | 85.0 | 66,773,907* | 801.3 | 0.106 |

## The rule, line by line

| Graph | |H| | Scan / split at 5% | Crosses 1.5? | Split / eager at 1% | Above 2? | Split / eager memory, worst level |
|---|---|---|---|---|---|---|
| sx-superuser-peak | 64 | 1.88 | no | 1.73 | no | 0.278 |
| sx-superuser-peak | 256 | 0.83 | yes | 4.12 | yes | 0.363 |
| sx-superuser-peak | 1024 | 0.22 | yes | 13.82 | yes | 1.158 |
| sx-superuser-peak | 4096 | 0.06 | yes | 52.65 | yes | 4.595 |
| wiki-talk-temporal-peak | 64 | 1.15 | yes | 1.87 | no | 0.012 |
| wiki-talk-temporal-peak | 256 | 0.49 | yes | 4.50 | yes | 0.019 |
| wiki-talk-temporal-peak | 1024 | 0.13 | yes | 15.07 | yes | 0.055 |
| wiki-talk-temporal-peak | 4096 | 0.03 | yes | 57.35 | yes | 0.207 |
| sx-superuser-ins-end | 64 | 6.10 | no | 1.19 | no | 0.169 |
| sx-superuser-ins-end | 256 | 3.37 | no | 2.19 | yes | 0.074 |
| sx-superuser-ins-end | 1024 | 1.16 | yes | 6.59 | yes | 0.037 |
| sx-superuser-ins-end | 4096 | 0.32 | yes | 24.55 | yes | 0.044 |
| wiki-talk-temporal-ins-end | 64 | 10.84 | no | 1.40 | no | 0.051 |
| wiki-talk-temporal-ins-end | 256 | 5.37 | no | 3.04 | yes | 0.028 |
| wiki-talk-temporal-ins-end | 1024 | 1.76 | no | 9.77 | yes | 0.012 |
| wiki-talk-temporal-ins-end | 4096 | 0.47 | yes | 36.90 | yes | 0.011 |
| rmat-s16-f16 | 64 | 24.44 | no | 0.84 | no | 0.544 |
| rmat-s16-f16 | 256 | 23.02 | no | 1.37 | no | 0.325 |
| rmat-s16-f16 | 1024 | 13.18 | no | 3.81 | yes | 0.166 |
| rmat-s16-f16 | 4096 | not run | | | | |

| Clause at the best |H| (64) | sx-superuser peak | wiki-talk peak | sx-superuser end | wiki-talk end | rMAT |
|---|---:|---:|---:|---:|---:|
| Scan / split at 5%, Stop at 1.5 or below | 1.88 | **1.15** | 6.10 | 10.84 | 24.44 |
| Split / eager at 1%, Stop above 2 | 1.73 | 1.87 | 1.19 | 1.40 | 0.84 |
| Split / eager memory, worst level, Go at 0.5 or below | 0.278 | 0.012 | 0.169 | 0.051 | 0.544 |

rMAT is synthetic and supports no claim alone. Its memory figure misses 0.5 at 15% and 5%, which does not matter, since the clause asks for
one real graph.

## Why the graphs differ

The plainest number is the scan with no memo: 6.6 operations per proposal on wiki-talk peak, 14.1 on sx-superuser peak, about 52 on
sx-superuser end state and about 91 on wiki-talk end state, all at 5%. The wiki-talk peak is star-shaped, so most dyads have a low-degree
end and a lower-degree scan is short with or without a memo. This is the Gate 0 picture again.

The memo adds little on the large graphs. On sx-superuser end state, misses outnumber hits about 3 to 1, and warming the memo for a further
10^7 proposals changes the ratio by under 10%, so the misses are not cold starts. A likely cause, which I did not test separately: an endpoint
of degree d sees about `2 alpha d` edits between two returns of the same edge, so at 5% acceptance a stamp on a degree-20 vertex has usually moved.

Split at |H| > 64 loses because every cn read costs `1 + 2 |H| / 64`, and almost every proposal makes one. At |H| = 4096 that is about 130
operations before anything else happens. Only |H| = 64 clears both lines anywhere.

## Sensitivity

| Scan / split at 5%, |H| 64 | sx-superuser peak | wiki-talk peak | sx-superuser end | wiki-talk end |
|---|---:|---:|---:|---:|
| Base | 1.88 | 1.15 | 6.10 | 10.84 |
| Lenient units | 2.44 | 1.49 | 8.38 | 14.67 |
| Memo uncapped | 1.56 | 0.97 | 6.10 | 10.84 |
| Seed 2 | 1.95 | 1.15 | 6.11 | not run |
| Memo keeps tie dyads only at top level | 1.87 | 1.14 | 6.09 | not run |
| Memo warmed for 10^7 proposals | not run | not run | 5.94 | 9.81 |
| No early rejection | 2.24 | 1.40 | timed out | not run |

The wiki-talk peak crossing holds in every row, and the closest is 1.49 under lenient units. With the memo uncapped, scan is cheaper than
split outright. Sx-superuser peak does not cross in any row here. With early rejection off and the memo uncapped it would, at 1.21.

**Build cost.** Scored operations leave out the one-off table build, because the rule counts proposals. Counted against only 10^7 proposals
at 5% and |H| 64, scan / split becomes 1.87, 1.14, 1.79 and **0.46** on the four real graphs in the order above. So wiki-talk end state
crosses as well, and sx-superuser end state stays clear. Split with build is 0.075 to 1.56 of eager with build. A chain long enough to repay
the build changes this, and I did not measure one.

## Exactness evidence

- **Change statistic against brute force.** Seeds 11 and 12: 552,655 and 554,035 delta evaluations against a full recompute of the statistic,
  on graphs of 5 to 28 vertices, lambda 1.5, 2 and 3, add and delete, every backend. 0 failures, worst error 1.6e-12.
- **The control fails as it must.** Dropping the delete correction is wrong on 49,772 of 54,974 delete dyads, seed 11.
- **Exact target.** On all 1,024 graphs of 5 vertices, chain frequencies match the exact ERGM at total variation 0.0075 and 0.0073 over
  6 x 10^6 steps. The pass limit of 0.01 is mine, and the sampling noise at that length was not computed.
- **Short lockstep chains.** 6,000 brute checks of triangles and statistic per seed, with tables rebuilt and compared.
- **Identical decisions.** Asserted on every proposal in every run, with bitwise delta and cn wherever both sides finished scoring. 0
  mismatches in 43 logs, no FATAL.
- **Separate processes.** On both peaks at 5%, scan alone, split alone and eager alone each gave the lockstep decision hash. Early rejection
  off gave the same hash too. On rMAT the scan and table processes agree.
- **Full recounts.** 757 across the finished runs: graph structures, running statistic, and triangle count against `static_count`. 0 failures,
  largest statistic drift 8.8e-7 between recounts. Three logs ended at the time limit, with no failure up to that point.
- **Tables.** Rebuilt and compared at every interval on the peaks, and at start and end elsewhere up to 5 x 10^7 entries. Larger tables rely on
  the per-proposal check.

## Does real ERGM practice use chains like this?

In form, yes. `tntSampler.c` describes TNT as usually used in statnet, EstimNetDirected ships it as a sampler, and the scout read shared-partner
change statistics in both tools. Metropolis chains at fixed theta are the core of simulation and of estimation loops. In setting, no:

- Theta comes from a fit. Nobody tunes it to an acceptance rate. Here acceptance was the input, so a theta_e of -32.5 on rMAT is an artefact,
  not a model.
- Fitted models usually carry more terms, such as degree and covariates. Edges plus alternating k-triangle alone does not hold a tree-like
  periphery, which is why edge counts fell, and why sx-superuser peak has only a thin ridge between melting and blow-up.
- **These snapshots are not datasets that ERGM users fit.** This test is about costs only, and its theta values mean nothing as estimates.
- The only acceptance figures the scout found are 15% and 0.4% on a 3,061-node network, so whether 1% to 15% is typical at scale is open.

## What this does not establish

- Anything about a fitted model, a real ERGM dataset, or the physician network the scout names as the failure in print.
- Wall-clock speed. Everything here is unit-cost operations, and a hash read is priced like a list element, which flatters tables on real
  hardware.
- Long chains. 10^7 proposals is about 3 to 10 sweeps of the edges on the end states, so neither build repayment nor long-run memo behaviour
  was measured.
- Mixing or statistical quality. Exactness here means the same chain and correct statistics, not that a chain has converged.
- That eager fails. No eager table above about 200 MB was built. The 62 GB figure for wiki-talk end state is an entry count times 12 bytes,
  without load factor.
- A finer scan baseline. Per-vertex stamps are as specified. A memo with sharper invalidation might do better on large graphs, and it was not
  tried.
- A variant of split that skips the mask scan for low-degree pairs. It might lift the |H| limit. It is outside the scout's table, so it was
  not run.
- Novelty. Wedge indexes and heavy-light partitioning are not claimed as contributions, by project rule 6.

## Files

- `step1.csv`: Step 1, all |H| from 64 to 4096.
- `theta.csv`, `tuning_trials.csv`: chosen pairs and every trial.
- `production.csv`: every backend row, with the tag `main`, `early0`, `seed2`, `warm`, `strongscan` or `solo-*`.
- `step2_summary.csv`, `summary.txt`: scored rows and ratios, from `scripts/summarise_m1b03.py`.
- `formula_check.csv`, `selftest_seed11.txt`, `selftest_seed12.txt`, `logs/`: evidence.
- Runner and tuner: `scripts/run_m1b03.sh`, `scripts/tune_m1b03.py`.
