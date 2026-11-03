# M1D08: the cells near the pass line, on a P-core and an E-core

Written on 2026-11-04, after M1D07 read Narrows (`log.md` 0058) and before any M1D08 run. The PI approved the test on 2026-11-04 and gave a standing order of up to 10 `--exclusive` hours (`log.md` 0056). The thresholds are Gate 1's (`docs/experiments/gate1-protocol.md`) and do not change. Any change gets a new ID and a `log.md` entry.

## Why

R2, PR's own unshared kernel, is the best other arm on 43 of 50 pooled p = 5 cells. With R2's fastest repeat 10% faster, the pooled passes fall from 28 to 16 of 50, under a third (`results/gate1-M1D07T/README.md`). M1D05 called the win a cache win on one machine. Every Gate 1 time so far ran on logical CPU 8, a P-core (`results/gate1-M1D07T/cpu-sets.txt`). The PI has no second machine (`log.md` 0056).

The Core Ultra 7 265 has 8 P-cores and 12 E-cores. The two kinds differ in core design and in L1 and L2: an E-core shares its L2 with its cluster. They share L3 and DRAM. So M1D08 asks a narrower question than a second machine would: **does a cell's verdict hold when the core and its near caches change?** A held verdict does not show that the result carries to another memory hierarchy. A turned verdict does show that it depends on the core.

## What it can and cannot do

M1D05's verdict and M1D07's reading were given by their rules and do not reopen. M1D08 can only narrow the claim further, or state its scope. It cannot widen it. It is never reported as a second machine.

## Cells and arms, by rule

The rule is `scripts/m1d08_cells.py`, and its output is `results/gate1-M1D08/cells.txt`, committed with this file.

- **Cells.** Every pooled p = 5 scored evaluation cell of M1D05 and M1D07, excluding the snapshot, that meets two conditions. First, its recorded margin (PR's slowest repeat over best's fastest) lies within a factor 1.2 of the pass line, in [0.558, 0.804]. These are the cells whose verdict a 20% shift could turn. Cells further out are taken as fixed and stated as untested. Second, its best fastest repeat is at least 0.05 s, which leaves out the four ia-reality-call cells measured in clock ticks. **24 cells**: 4 from M1D04 and 20 from M1D07, 16 passing and 8 failing on the P-core record. plwiki real 100 fails at 0.6702, which `cells.txt` prints as 0.670.
- **Arms.** PR, and every other arm that finished with a recorded median at most twice best's median. That gives 88 arm-cells. R2 runs on every cell, M5 on 22, WT on 14, A5 on 3 and WJ on 1. An arm more than twice best's time on the P-core is taken as unable to become best on the E-core, and the paper says so.
- **Why the window and the factor 2.** With every arm on both cores the plan was 13 to 20 hours, over the 10-hour order. The window and the factor were set to fit the order. They were chosen from the recorded margins, which are already known, and the rule and the full list are published.

## Measure

- **Build and limits.** M1D04's `build/gate1/timing.exe`, checked by hash. Limits 1,200 s and 16 GB, A5 300 s. One process at a time under an `--exclusive --timing` claim.
- **Two cores.** The P-core is logical CPU 8, as before. The E-core is logical CPU 16, efficiency class 0, fixed now.
- **Five repeats per arm per core,** in the same claim. The recorded fastest arm of every listed cell takes under 60 s, so Gate 1's rule gives five, and the factor-2 arm rule leaves no arm that it would run once.
- **Order.** For repeat r = 1 to 5, arms in Gate 1's order rotated by r. Each arm runs on both cores back to back, P first when r plus the arm's position is even and E first otherwise. The P-core runs pair with the E-core runs in time, so drift since M1D04 and M1D07 does not enter the comparison.
- **Exactness.** Every run must equal, per batch, the recorded first timing repeat of its arm on that cell (`results/gate1-M1D04/raw` or `results/gate1-M1D07T/raw`, both checked against count builds). The last row of a CAP record is left out. A single difference stops the run, the claim is released, and ControlPC and the PI are told. No restart (hard rule 1).
- **Line.** ControlPC's line is checked before each process. Cells run cheapest first. A cell cut by the line is completed in the next claim, inside the 10 hours, and no cell is dropped once any M1D08 result is read.

## Reading, fixed now

Per cell and per core, Gate 1's score and pass test (`gate1_score.score`) over the arms run. "Best" is the fastest of the arms run, by median.

1. **P-core reproduction.** M1D08's P-core verdict against the recorded verdict, per cell. Drift of PR and R2 against their recorded medians. This is the drift reading that M1D07 could not give.
2. **E-core verdicts.** Per cell, pass or fail on the E-core. Cells whose verdict differs between the two cores are listed by name.
3. **Pooled count per core.** M1D07's pooled 50 cells, with M1D08's verdicts in place of the 24 listed cells and the 26 others kept at their recorded verdicts. This is computed twice, once with the M1D08 P-core verdicts and once with the E-core verdicts. Each count is then put to Gate 1's Go clause read literally: at least a third pass, on at least 2 streams, with at least 2 expiry passes.

| Outcome | When | What the paper says |
|---|---|---|
| **Core-stable** | The pooled Go clause holds with the P-core and with the E-core verdicts | The claim sentence stands, with "on both core types of one machine" and the count per core |
| **Core-dependent** | It holds on one core type and fails on the other | The claim is restricted to that core type by name, and the dependence is stated up front |
| **Not reproduced** | It fails with M1D08's own P-core verdicts | Drift since M1D07 or noise is larger than the margin. The claim's count is not stated without this. Goes to the PI |
| **Stop** | An exactness failure not traced to a test bug | As Gate 1 |

Read beside it, whatever the outcome: the E-over-P time ratio per arm, by median; PR's score on each core, cell by cell; the cells the window leaves out, named as untested; and the clause that a held verdict is not a second machine.

## Order of work

| ID | Step | Machine |
|---|---|---|
| M1D08A | This protocol, `scripts/m1d08_cells.py` and `cells.txt`, `scripts/run_m1d08.sh`, `scripts/m1d08_check.py` and `scripts/m1d08_score.py`, committed before any run | No claim |
| M1D08B | Dry run on CollegeMsg and a fixture, output in the scratchpad, seconds thrown away | Ordinary shared claim of at most 15 minutes, ControlPC's GO |
| M1D08C | The timing run | `--exclusive --timing`, at most 7 hours a claim and 10 in all, the PI's standing order and ControlPC's GO |
| M1D08D | The reading above | No claim |

## Hours

From `cells.txt`: 2.33 hours on one core at the recorded P-core seconds. On both cores, with the E-core 1.0, 1.5 and 2.0 times slower: **4.66, 5.82 and 6.99 hours**. That is one claim with a 7-hour line. If the E-core is slower than 2.0 times, the line stops the claim and a second one completes it, inside the 10 hours.
