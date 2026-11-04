# STATUS: Research 11

## Position, 2026-11-04

- **The original proposal is refuted.** Batch sharing for triangles failed Gate 0, 1.317 to 2.438 times sequential scanning, with a hindsight cap of 13%
  (`log.md` 0008, 0009).
- **The PI kept the project open** (`log.md` 0010). Screens B and C are done, all non-citable. The ranking is `docs/novelty/candidates-2026-11-01.md`.
- **The PI picked the study of when batch sharing pays across pattern size** (`log.md` 0018). D-k stays in reserve.
- **Reading weakened that pick.** Cheetah publishes star grouping with reuse, and the trend itself (`log.md` 0019). What is left is the gain against the strongest baseline.
- **Gate 1 is written and its arms are exact on the oracle.** `docs/experiments/gate1-protocol.md`. Series M1D in `docs/tasks/M1.md`.
- **M1D03 is done and exact.** 156 of 156 arm-cells, 0 mismatches, 19 stopped at a limit, claim released (`log.md` 0024). On operations the median against best other is 1.447, 0.916, 0.950 at p = 3, 4, 5, without M. Gate 1 is not dead, and M decides it.
- **M1D04 is done and exact, and M1D05 gives Go with the trend.** 216 arm-cells, 0 mismatches, all claims released (`log.md` 0032). At p = 5, 6 of 14 scored cells pass, median 0.686, against 1.098 at p = 3. The held-out stream gives no pass, two passes decide Go, and the win over the tables is a cache win on one machine.
- **M1D06A: all six passes hold against M5 at its best theta per cell** (`log.md` 0035). The attack the PI chose did not move Go. The two sx expiry passes still rest on a 9% to 13% margin over the scan, which only a second machine tests.
- **M1D07 reads Narrows** (`log.md` 0058). 12 new streams from Hanauer et al.'s set, 6 of them with real deletions, timed in two exclusive claims, exact. At p = 5, 22 of 36 new cells pass. Pooled with M1D05, 28 of 50 pass and Go holds, but the pooled p = 3 median of 0.965 breaks the trend: sharing now beats the scan at p = 3 on 27 of 36 new cells, without passing. The outline's claim sentence is rewritten. R2 is best on 43 of 50, and a scan 10% faster would take the pooled passes to 16 of 50.
- **M1D08 reads Core-stable** (`log.md` 0061). The 24 cells near the pass line, timed on a P-core and an E-core in one exclusive claim, exact. The P-core reproduces the record: arm medians within 1%, 22 of 24 verdicts the same, and 28 of 50 pooled. The E-core passes 24 of 50 on 12 streams, and Go holds on both. The loss is on the wiki real-deletion cells, which are worse on every one there. Not a second machine.
- **M1D09 reads Carries** (`log.md` 0063). A cost model priced by kind of operation, fitted on M1D05's streams, names the clock winner on all 64 new cells where unit cost names 21, at a median error of 22.9%. The cache-size term adds nothing, and WJ is underpriced about fivefold. Figures 3 to 6 are drawn, so the outline has its six.
- **M1D10 reads Carries** (`log.md` 0065). The same models fitted on the 12 engineering cells, every arm timed there in one exclusive claim, exact. KC names 63 of 64 new winners, unit cost 21, at 23.0% median error. Section 6 claims Carries with M1D10's coefficients. Neither fit shows a cache price. On M1D05's streams the engineering fit misses 14 of 24 cells at p = 3 and 4, always preferring P to the scan, and none at p = 5.

| Test | Verdict | Read with |
|---|---|---|
| M1B01-B, D-k | Go | "Any L" dropped in through-tree form |
| M1C01, D-k against database baselines | Go at both widths | Thin at 1 s: exactly 4 of 6, 11% cheaper V charges give Pivot |
| M1B02, receipts | Weak Go on one reading | Stop if an honest run means a graph-blind verifier. Novelty 4, no user |
| M1B03, ERGM index | Stop on the plain reading | Rule named no graph count. Theta tuned, not fitted |
| M1C02, 5-cycles on operations | Stop at clause 4 | Clock reverses the winner |
| M1C03, 5-cycles on the clock | Go by one cell | Shared clock. Worst-of-3 gives Pivot. Operations still favour tables on 16 of 22 |
| M1D03, Gate 1 operations | Clean, no verdict | M absent. At p = 5 only wiki-talk reaches 0.67, against a scan. WJ beats PR on 6 of 8 sx cells |
| M1D05, Gate 1 on the clock | Go, with the trend | 6 of 14 at p = 5. Held-out stream 0 of 2. Two sx expiry passes decide it, and fall to a scan 9% to 13% faster or a table twice as fast (`log.md` 0033). Best is the scan on 10 of 14. One machine, one core |
| M1D06A, M5 at its best theta | 6 of 6 passes hold | Best theta 16 or 64, not 128. M5 beats R2 on one stream. A noisier claim, against PR |
| M1D07E, 12 new streams, pooled | **Narrows** | Pooled Go 28 of 50 holds; pooled trend fails at p = 3, median 0.965. No pass on dewiki or sx-stackoverflow. 10% margin over the scan. One P-core, one machine |
| M1D08, P-core against E-core | **Core-stable** | Go holds on P 28 of 50 and E 24 of 50. 6 of 24 cells differ by core; frwiki and nlwiki pass on P only. Same L3 and DRAM, so not a second machine |
| M1D09, the cost model | **Carries** | Priced by kind: 64 of 64 new winners, unit cost 21. No cache-size effect found; WJ underpriced about 5x. Fitted on read streams, a reading of hard rule 5 |
| M1D10, the cost model on the engineering streams | **Carries** | 63 of 64 new winners, unit cost 21, 23.0% error. No cache price in either fit; WJ at 0.15. On M1D05's streams 22 of 36: P preferred to the scan on 14 of 24 at p = 3 and 4 |

The PI's readings of 2026-11-01 (`log.md` 0018): M1B02 Stop and parked, M1B03 Stop and parked, M1C02 Stop stands, M1C01 Go read with the V margin and the ring note.

## Open

**No run is open.** M1D10 is done (`log.md` 0065). On the PI's word (`log.md` 0066): figure 5 redrawn from both fits; JGAA's guidelines and two exemplars checked (`docs/submissions/paper-1/venue-fit.md`); sections 2 to 4 drafted in `docs/paper/jgaa/sec2-4.tex`, with seven citations still to read (`refs-todo.md`). Open for the PI: a remote for code and data release, which JGAA's exemplars all have; downloading the JGAA LaTeX package; the AI-use declaration JGAA requires. A second machine is still the open gap in readiness test 3. Nothing runs on the standing order of 0056.

**Venue decided:** JGAA, WoS not needed (PI, 2026-11-04, `log.md` 0064).

## Handover notes

- `scripts/chain_m1d10.sh` needs 14 GiB of free RAM, and with many desktop sessions loaded the machine sits near 13.8. M1D10C started only after a retry loop (`log.md` 0065). Close idle sessions before a claim.

- Research 1's scheduled tasks fire at every quarter hour, and ControlPC's mail at about :06, through every claim. They are not paused for a claim. `scripts/overlap_check.py` tests timed runs against them (`log.md` 0036).

- Closing a desktop session's window does not end its `claude.exe`. A shell it owns keeps running, and the session wakes when that shell ends, so check it before assuming either.

- `data/` is not in git. `data/snap/SHA256SUMS` lists the six inputs, and `data/snap/held-out/SHA256SUMS` the held-out stream. Rebuild `data/derived/` with `scripts/prepare_stream.py` and `scripts/prepare_window_stream.py`.
- Raw per-batch files are ignored by git, under `results/**/raw/`.
- Heavy work needs a tower claim, released between tests. ControlPC is the coordinator (PI, `log.md` 0018; rule in `CLAUDE.md`).
- After release, a process with the project folder name on its command line reads as R11 load. A watcher reads only lines written after it starts.
- DBLP sits behind a bot check and is not screened.
- Commits are under the PI's name, with the assistant as co-author.
- Research 11's monitoring and GO requests come from one session at a time. Since 2026-11-04 21:40 that is "Sambung Research 11 M1D10C" (`log.md` 0065).

## Environment read on 2026-11-01

Intel Core Ultra 7 265, 31 GB RAM, 206 GB free on C:. GCC 16.2 at `C:/Develop/tools/winlibs`. Python 3.12 with
numpy. WSL2 Ubuntu 24.04, stopped. The idea documents name a Ryzen 5 7500F, which is not this machine.

## Build

```bash
export PATH="/c/Develop/tools/winlibs/gcc-16.2.0-ucrt-r1/mingw64/bin:$PATH"
g++ -std=c++20 -O2 -Wall -Wextra -static -o build/r11_tests.exe tests/test_exact.cpp
./build/r11_tests.exe 200000 7
```

Screen binaries are built as each results README states.
