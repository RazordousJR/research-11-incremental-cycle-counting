# Gate 1 protocol: when batch sharing pays, across pattern size

Written on 2026-11-01, before any run of its new arms on a protocol stream. Serves M1D02 to M1D06. Thresholds never change after results: a change is a new ID and a `log.md` entry.

## The question

Cheetah already reports the trend: batch sharing gains least on triangles and more on larger patterns (`docs/literature/r10-falsifier-reading.md`). So the trend alone is not a contribution. This
gate asks the narrower thing that the literature read so far leaves open:

**For exact sequential counting by closed-form kernels, with every cost inside the total, does sharing inside a batch beat the strongest non-sharing baseline as the pattern grows from 3 to 5 vertices, and why do
operation counts and the clock disagree on the winner?**

The baseline set is the point. Cheetah's 2.7× is against Tesseract, and against GraphPi-D its gain falls to 1.03 to 1.57 on 5-CF by my arithmetic on its Table 2. M1C03's Go had no finished table arm on 4 of its 9 passing cells.

## Arms

Source: `benchmarks/gate1.cpp`. The header states every formula and charge. Exactness: `tests/test_gate1.cpp`.

| Role | p = 3 | p = 4 | p = 5 |
|---|---|---|---|
| Per-update scan | S3 | S4 | R2 |
| **Sharing, star row** | **P3** | **P4** | **PR** |
| Table, tuned layout | T3 | T4 | WT |
| Flat table | | | WJ |
| Mixed table, hubs scanned (idea of HHH) | M3 | M4 | M5 |
| Affected-subgraph recount | | A4 | A5 |
| Published sharing, BFCB-IG port | | B4 | |

P3 and P4 carry a guard, which reads degrees, is charged, and has no tuned parameter: a star keeps its row only if the row costs less than what it replaces, otherwise its spokes run as scans. PR stays as in M1C03, with no guard, so its counts and operations can be
checked against that run, and an unguarded PR can only understate sharing.

Not built, with reasons, in the header: A3, because Gate 0 prices a triangle update at 5.9 to 13.9 operations; a BFCB-IG port at p = 5, because no row form of that kind exists there, and PR is the nearest.
Batch size 1 is not an arm: PR, P3 and P4 reduce to their scan on a group of one, by construction, and the cut of 100 against 1,000 shows the batch axis.

## Streams

Hard rule 5 applies. Labels appear in every table (hard rule 4). Insert-only and two-week expiry, cuts of 100 and 1,000 events.

| Set | Streams | Use |
|---|---|---|
| Engineering | CollegeMsg, email-Eu-core-temporal, sx-mathoverflow | Choosing theta, debugging, the scripts' dry run. Reported, and never in a verdict |
| Evaluation | sx-askubuntu, sx-superuser, wiki-talk-temporal | The verdict |
| Held out | sx-stackoverflow, 529.9 MB gz from SNAP | The only stream no arm has seen. Used only if the PI approves the download. 4 more cells, in the verdict if they are scored |

**Weakness, stated.** The evaluation streams were read by R2, PR, WT and WJ in M1C03. No parameter came from them, and none of the arms has a parameter on them here except theta, which is chosen elsewhere. They are still not
unseen. KONECT Wikipedia streams stay out of this gate: their format and size are not checked, and they would need a new loader.

## Theta

One value per pattern, chosen on the engineering cells, from {16, 32, 64, 128, 256, 512, 1024}, as the value with the lowest geometric mean of M's seconds over the 12 engineering cells, one run each. It runs inside the
exclusive claim, because a theta picked on a noisy clock would hurt M, and M is a baseline: an error there favours the sharing arm. A tie goes to the smaller value. PR, P3 and P4 have no parameter.

## Order of work

| ID | Step | Machine |
|---|---|---|
| M1D01 | This protocol, the arms and the oracle, before any run on a protocol stream | No claim |
| M1D02 | Regression: R2, PR, WT and WJ match M1C03 per batch on 4 engineering cells. New arms agree per batch with each other on all 12 engineering cells, count build | Single processes, each under a minute |
| M1D02B | The guarded P3 and P4 against all other arms on the 12 engineering cells | Inside the M1D03 claim |
| M1D03 | Operation sweep on the evaluation cells, count build, all arms but M, whose theta is not chosen yet. Operations are the first reading (hard rule 3) | Ordinary claim, ControlPC's GO |
| M1D04 | Theta selection, then the timing sweep, pinned to one P-core, then the count build of M at the chosen theta | `--exclusive` claim, ControlPC's GO and the PI's word |
| M1D05 | Verdict by the rule below, read together with M1D03 | No claim |
| M1D06 | Ablations and the §16 figures, if the verdict allows | Decided after M1D05 |

## Measure

Timing build, one process at a time, pinned to logical CPU 8, under an exclusive claim. Five repeats on a cell whose fastest arm takes under 60 s, and three otherwise. Arms are interleaved across repeats in a rotated order. An arm is
run once only, on a cell where it took more than 5 times the fastest arm. Limits per process: 1,200 s and 16 GB of commit, which closes the 6 GB bound left open on wiki-talk in M1C03. A4 and A5 stop at 300 s, set after M1D02 and before any evaluation run (`log.md` 0020). Past a limit an arm is **not finished**, which is not a win. Its time
enters as the limit, if the limit was time, so a lower limit makes the test harder for the sharing arm, never easier. An arm stopped by memory is out of that cell, and the table says so.

Every arm's per-batch count must match every other arm's in that pattern and cell. A single mismatch stops the sweep (hard rule 1).

## Rule

Let **share** be P3, P4 or PR, or at p = 4 the faster of P4 and B4. Let **best** be the fastest finished arm of the other rows, B4 aside.

A cell **passes** when share's slowest repeat is at most 0.67 times best's fastest repeat. This is stricter than M1C03, on purpose: its Go rested on one clock tick and turned to Pivot on the worst of three.
The cell's **score** is the ratio of medians. A cell is scored if share finished and at least one other arm finished.

Verdict on p = 5, over the evaluation cells, with precedence Stop, then Go, and Pivot is anything else:

- **Stop:** an exactness failure not traced to a test bug; or fewer than 8 cells scored; or no cell passes.
- **Go:** at least a third of the scored cells pass, on at least 2 streams, and at least 2 of them are expiry cells.

Read beside the verdict, not part of it, and reported whatever the outcome: the median score at each p; the same score against a scan alone; the ratio of the row kernel to BFCB-IG at p = 4; nanoseconds per charged operation per arm;
peak memory; and every cell where operations and the clock name a different winner.

| Outcome | What it supports |
|---|---|
| Go, and median score at p = 3 at least 1.0, and median at p = 5 at most 0.90 | One experimental paper: sharing beats the strongest baseline at 5 vertices and not at 3, with the cost model explained |
| Go without that trend | The p = 5 kernel only, short format |
| Pivot | A negative study at most: sharing beats a scan and not a tuned baseline. PI decides whether it is worth writing |
| Stop | Candidate 1 closes, and D-k is next in line |

## What this gate cannot show

A win here is on one machine and one core, on three streams plus perhaps one. It does not give a user: S9 and S10 found none. It does not establish novelty: R10's strict falsifier did not fire on three papers read, and that is all.
