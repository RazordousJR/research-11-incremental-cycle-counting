# STATUS: Research 11

## Position, 2026-11-01

- **The original proposal is refuted.** Batch sharing for triangles failed Gate 0, 1.317 to 2.438 times sequential scanning, with a hindsight cap of 13%
  (`log.md` 0008, 0009).
- **The PI kept the project open** (`log.md` 0010). Screens B and C are done, all non-citable. The ranking is `docs/novelty/candidates-2026-11-01.md`.
- **The PI picked the study of when batch sharing pays across pattern size** (`log.md` 0018). D-k stays in reserve.
- **Reading weakened that pick.** Cheetah publishes star grouping with reuse, and the trend itself (`log.md` 0019). What is left is the gain against the strongest baseline.
- **Gate 1 is written and its arms are exact on the oracle.** `docs/experiments/gate1-protocol.md`. Series M1D in `docs/tasks/M1.md`.
- **M1D03 is done and exact.** 156 of 156 arm-cells, 0 mismatches, 19 stopped at a limit, claim released (`log.md` 0024). On operations the median against best other is 1.447, 0.916, 0.950 at p = 3, 4, 5, without M. Gate 1 is not dead, and M decides it.
- **M1D04 `main` and `wiki` are done and exact.** 192 arm-cells, 684 timing processes, 0 mismatches, claims released (`log.md` 0027, 0028). At p = 5, 6 of 12 cells pass, median 0.642, against a median of 1.098 at p = 3. No verdict until M1D05. Only an exactness failure can now move it off Go with the trend. Most of the sx gain is a cache result, not fewer operations.

| Test | Verdict | Read with |
|---|---|---|
| M1B01-B, D-k | Go | "Any L" dropped in through-tree form |
| M1C01, D-k against database baselines | Go at both widths | Thin at 1 s: exactly 4 of 6, 11% cheaper V charges give Pivot |
| M1B02, receipts | Weak Go on one reading | Stop if an honest run means a graph-blind verifier. Novelty 4, no user |
| M1B03, ERGM index | Stop on the plain reading | Rule named no graph count. Theta tuned, not fitted |
| M1C02, 5-cycles on operations | Stop at clause 4 | Clock reverses the winner |
| M1C03, 5-cycles on the clock | Go by one cell | Shared clock. Worst-of-3 gives Pivot. Operations still favour tables on 16 of 22 |
| M1D03, Gate 1 operations | Clean, no verdict | M absent. At p = 5 only wiki-talk reaches 0.67, against a scan. WJ beats PR on 6 of 8 sx cells |
| M1D04 main and wiki, Gate 1 clock | Clean, no verdict yet | 12 of 16 possible p = 5 cells. Best is the scan on 8. Reading R1 cost 2 passes on wiki-talk insert and stands. Two passes fall to a WJ 2.6 times faster. One machine, one core |

The PI's readings of 2026-11-01 (`log.md` 0018): M1B02 Stop and parked, M1B03 Stop and parked, M1C02 Stop stands, M1C01 Go read with the V margin and the ring note.

## Open, waiting on the PI

Nothing is waiting on the PI. The PI approved `held` at up to 8 hours, moved to now on 2026-11-02 afternoon (`log.md` 0031), with all 4 cells, and kept the two PR repeats that overlap R9 (`log.md` 0030). `held` now waits on ControlPC's GO. Run `scripts/run_m1d04.sh results/gate1-M1D04 8 held` under its own claim with the release chained, check `hashes.txt` first, and send the log to a new file so the tree stays clean. Then M1D05 with `scripts/gate1_score.py`, confirming that all 6 arms of all 4 cells left a summary.

## Handover notes

- Closing a desktop session's window does not end its `claude.exe`. A shell it owns keeps running, and the session wakes when that shell ends, so check it before assuming either.

- `data/` is not in git. `data/snap/SHA256SUMS` lists the six inputs, and `data/snap/held-out/SHA256SUMS` the held-out stream. Rebuild `data/derived/` with `scripts/prepare_stream.py` and `scripts/prepare_window_stream.py`.
- Raw per-batch files are ignored by git, under `results/**/raw/`.
- Heavy work needs a tower claim, released between tests. ControlPC is the coordinator (PI, `log.md` 0018; rule in `CLAUDE.md`).
- After release, a process with the project folder name on its command line reads as R11 load. A watcher reads only lines written after it starts.
- DBLP sits behind a bot check and is not screened.
- Commits are under the PI's name, with the assistant as co-author.

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
