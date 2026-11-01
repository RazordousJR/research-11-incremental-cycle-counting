# STATUS: Research 11

## Position, 2026-11-01

- **The original proposal is refuted.** Batch sharing for triangles failed Gate 0, 1.317 to 2.438 times sequential scanning, with a hindsight cap of 13%
  (`log.md` 0008, 0009).
- **The PI kept the project open** (`log.md` 0010). Screens B and C are done, all non-citable. The ranking is `docs/novelty/candidates-2026-11-01.md`.
- **The PI picked the study of when batch sharing pays across pattern size** (`log.md` 0018). D-k stays in reserve.
- **Reading weakened that pick.** Cheetah publishes star grouping with reuse, and the trend itself (`log.md` 0019). What is left is the gain against the strongest baseline.
- **Gate 1 is written and its arms are exact on the oracle.** `docs/experiments/gate1-protocol.md`. Series M1D in `docs/tasks/M1.md`.
- **Nothing is running. No claim is held.**

| Test | Verdict | Read with |
|---|---|---|
| M1B01-B, D-k | Go | "Any L" dropped in through-tree form |
| M1C01, D-k against database baselines | Go at both widths | Thin at 1 s: exactly 4 of 6, 11% cheaper V charges give Pivot |
| M1B02, receipts | Weak Go on one reading | Stop if an honest run means a graph-blind verifier. Novelty 4, no user |
| M1B03, ERGM index | Stop on the plain reading | Rule named no graph count. Theta tuned, not fitted |
| M1C02, 5-cycles on operations | Stop at clause 4 | Clock reverses the winner |
| M1C03, 5-cycles on the clock | Go by one cell | Shared clock. Worst-of-3 gives Pivot. Operations still favour tables on 16 of 22 |

The PI's readings of 2026-11-01 (`log.md` 0018): M1B02 Stop and parked, M1B03 Stop and parked, M1C02 Stop stands, M1C01 Go read with the V margin and the ring note.

## Open, waiting on the PI

The live list is the final table of the last report. In short: the P3 and P4 guard and the 300 s recount limit, which hold M1D03 (`log.md` 0021), then the word for an `--exclusive` Gate 1 and the sx-stackoverflow download.

## Handover notes

- `data/` is not in git. `data/snap/SHA256SUMS` lists the six inputs. Rebuild `data/derived/` with `scripts/prepare_stream.py` and `scripts/prepare_window_stream.py`.
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
