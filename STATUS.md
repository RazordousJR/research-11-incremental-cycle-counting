# STATUS: Research 11

## Position, 2026-11-01

- **Gate 0 stopped the proposal.** On six real streams the method cost 1.317 to 2.438 times sequential scanning,
  against a go line of 0.67 (`results/gate0-20261101-d2efe26/README.md`, `log.md` 0008).
- The stop is not an implementation artifact. With free selection and no overhead, the gain is at most 13%
  (`results/diag-oracle-20261101-e267b9f/README.md`, `log.md` 0009).
- The exactly-once proof stands and is tested. It is not a contribution.
- Not covered: Wikipedia link streams and sx-stackoverflow.
- Nothing is running. No claim is held. `research11` is registered in the tower.
- M1A09 to M1A18 wait on the PI. Do not start them.

## Open, waiting on the PI

1. Close the project, or run one last hindsight check on a Wikipedia stream.
2. Whether a short negative note is worth writing. Recommended: no.

## Handover notes

- `data/` is not in git. `data/snap/SHA256SUMS` lists the six inputs. Rebuild `data/derived/` with
  `scripts/prepare_stream.py`.
- `build/r11_probe.exe` is currently the hindsight diagnostic, built from `benchmarks/diag_oracle.cpp`. P0 and P0B
  came from `benchmarks/probe_ops.cpp` at earlier source hashes.
- DBLP sits behind a bot check and is not screened.
- Commits are under the PI's name, with the assistant as co-author.

## Environment read on 2026-11-01

Intel Core Ultra 7 265, 31 GB RAM, 206 GB free on C:. GCC 16.2 at `C:/Develop/tools/winlibs`. Python 3.12 with
numpy. WSL2 Ubuntu 24.04, stopped. The idea documents name a Ryzen 5 7500F, which is not this machine.

## Build

```bash
export PATH="/c/Develop/tools/winlibs/gcc-16.2.0-ucrt-r1/mingw64/bin:$PATH"
g++ -std=c++20 -O2 -Wall -Wextra -static -o build/r11_tests.exe tests/test_exact.cpp
g++ -std=c++20 -O2 -Wall -Wextra -static -o build/r11_gate.exe benchmarks/gate0.cpp
./build/r11_tests.exe 200000 7
```
