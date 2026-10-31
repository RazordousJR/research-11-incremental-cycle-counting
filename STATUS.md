# STATUS: Research 11

## Position, 2026-11-01

- The audit is finished, with limits listed in `docs/literature/audit-2026-11-01.md`. Hanauer et al.
  (arXiv:2610.29776, October 2026) rejected the indexing half of the idea for triangles.
- The project is narrowed to work-sharing between the updates of one batch: `docs/proposal.md`.
- The exactly-once proof is written and tested: 0 failures on 400,000 random mixed batches across two seeds.
  It is not a contribution.
- The prototype exists: `src/r11/tc.hpp`, `tests/test_exact.cpp`, `benchmarks/probe_ops.cpp`. It counts
  operations. It is not fit for timing, because it uses `std::unordered_map`.
- Probes P0 and P0B are synthetic and non-citable: gain on whole-vertex bursts, none on uniform updates.
- **No real stream has been read.** Gate 0 has not run. The claim is a hypothesis.
- Nothing is running. No claim is held. The project is not registered in the tower.

## Next step

Once the PI approves the table below: finish M1A05 (TODS text, EDTC, ParaDyMS), write M1A06 loaders, run M1A07
Gate 0 on six real streams.

## Open, waiting on the PI

1. Download of six SNAP files, about 72 MB in total.
2. Approval of the thresholds in `docs/proposal.md` §7, before any real stream is read.
3. Project number: Research 11, and the "Research 5" heading treated as a slip.
4. `CLAUDE_MAIN.md` not applied here.
5. Registering `research11` in the tower.
6. `git init` in this folder.
7. Funding statement, not given for this project.

## Environment read on 2026-11-01

Intel Core Ultra 7 265, 31 GB RAM, 206 GB free on C:. GCC 16.2 at `C:/Develop/tools/winlibs`. Python 3.12.
WSL2 Ubuntu 24.04, stopped. The idea documents name a Ryzen 5 7500F, which is not this machine. This folder
is not a git repository, so run IDs carry a source hash instead of a commit.

## Build

```bash
export PATH="/c/Develop/tools/winlibs/gcc-16.2.0-ucrt-r1/mingw64/bin:$PATH"
g++ -std=c++20 -O2 -Wall -Wextra -static -o build/test_exact.exe tests/test_exact.cpp
g++ -std=c++20 -O2 -Wall -Wextra -static -o build/probe_ops.exe benchmarks/probe_ops.cpp
./build/test_exact.exe 200000 7
```
