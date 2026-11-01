# M1D03: operation sweep, evaluation cells

Protocol: `docs/experiments/gate1-protocol.md`, thresholds unchanged. Non-citable. Count build, one process at a time, pinned to logical CPU 8, ordinary tower claim on ControlPC's GO, taken 20:33:01 and released at about 23:49:30 local time on 2026-11-01.
Operations are the reading here (hard rule 3). The seconds in `run.log` come from a count build on a shared machine. They are a record of cost, and nothing in this file uses them as timing.
Every stream here is labelled **real timestamps, synthetic expiry** (expiry) or **real timestamps, insert-only, no expiry** (insert). None is fully synthetic.

## Result

- **Complete.** `M1D03-DONE`, 36 cell-and-cut records, 156 of 156 arm-cells, each with a non-empty summary. 137 finished and 19 stopped at a limit. The M arms are not in this sweep, since their theta comes from M1D04.
- **Exact.** The sweep's own check found no mismatch. A separate check after the run compared every pair of arms on every batch both reached: 276 pairs, 4,723,486 batch comparisons, 0 differ. The last row of each of the 4 CAP records is left out, as in M1C03, since the table stopped inside that batch.
- **Regression holds.** R2, PR, WT and WJ match M1C03 on count and operations, per batch, on all 34 arm-cells where M1C03 has a raw file: 0 differ.
- **The binary did not change.** All seven hashes in `hashes.txt` still match after the run, at commit 6b20ece.
- **The session that started the sweep was closed while it ran, and neither the sweep nor the chained release died.** No restart happened, so `run.log` is one unbroken run.

## Operations, in millions

Finished arms only. A stopped arm has no total, because its partial sum is not comparable.

**p = 3**

| Stream | Label | Cut | S3 | P3 | T3 |
|---|---|---|---|---|---|
| sx-askubuntu | expiry | 1,000 | 11 | 18 | 258 |
| sx-askubuntu | expiry | 100 | 11 | 18 | 267 |
| sx-askubuntu | insert | 1,000 | 14 | 21 | 2,086 |
| sx-askubuntu | insert | 100 | 14 | 21 | 2,086 |
| sx-superuser | expiry | 1,000 | 18 | 29 | 411 |
| sx-superuser | expiry | 100 | 18 | 29 | 422 |
| sx-superuser | insert | 1,000 | 28 | 41 | 4,680 |
| sx-superuser | insert | 100 | 28 | 40 | 4,679 |
| wiki-talk-temporal | expiry | 1,000 | 112 | 162 | 50,521 |
| wiki-talk-temporal | expiry | 100 | 114 | 156 | 50,631 |
| wiki-talk-temporal | insert | 1,000 | 197 | 270 | MEM |
| wiki-talk-temporal | insert | 100 | 197 | 250 | MEM |

**p = 4**

| Stream | Label | Cut | S4 | P4 | T4 | B4 | A4 |
|---|---|---|---|---|---|---|---|
| sx-askubuntu | expiry | 1,000 | 154 | 107 | 264 | 472 | 272 |
| sx-askubuntu | expiry | 100 | 163 | 165 | 273 | 732 | 1,234 |
| sx-askubuntu | insert | 1,000 | 2,942 | 2,477 | 2,098 | 16,539 | 38,281 |
| sx-askubuntu | insert | 100 | 2,936 | 2,829 | 2,098 | 30,678 | 200,003 |
| sx-superuser | expiry | 1,000 | 270 | 195 | 420 | 798 | 516 |
| sx-superuser | expiry | 100 | 281 | 284 | 432 | 1,237 | 2,346 |
| sx-superuser | insert | 1,000 | 8,204 | 6,905 | 4,705 | 45,308 | 134,123 |
| sx-superuser | insert | 100 | 8,216 | 7,722 | 4,704 | 80,546 | TIME |
| wiki-talk-temporal | expiry | 1,000 | 22,565 | 5,614 | 50,592 | 71,840 | 26,737 |
| wiki-talk-temporal | expiry | 100 | 22,700 | 8,993 | 50,704 | 83,759 | 85,711 |
| wiki-talk-temporal | insert | 1,000 | 192,690 | 112,394 | MEM | 825,959 | TIME |
| wiki-talk-temporal | insert | 100 | 191,446 | 157,100 | MEM | 1,132,893 | TIME |

**p = 5**

| Stream | Label | Cut | R2 | PR | WT | WJ | A5 |
|---|---|---|---|---|---|---|---|
| sx-askubuntu | expiry | 1,000 | 556 | 236 | 611 | 349 | 937 |
| sx-askubuntu | expiry | 100 | 597 | 428 | 661 | 380 | 7,624 |
| sx-askubuntu | insert | 1,000 | 33,019 | 15,032 | 12,185 | 9,434 | 315,916 |
| sx-askubuntu | insert | 100 | 33,006 | 24,296 | 12,206 | 9,444 | TIME |
| sx-superuser | expiry | 1,000 | 990 | 451 | 1,014 | 582 | 1,823 |
| sx-superuser | expiry | 100 | 1,046 | 766 | 1,069 | 617 | 14,995 |
| sx-superuser | insert | 1,000 | 88,471 | 42,085 | 27,495 | 21,819 | TIME |
| sx-superuser | insert | 100 | 88,485 | 64,998 | 27,498 | 21,811 | TIME |
| wiki-talk-temporal | expiry | 1,000 | 50,542 | 18,782 | 73,418 | CAP | 150,905 |
| wiki-talk-temporal | expiry | 100 | 51,026 | 27,376 | 74,040 | CAP | TIME |
| wiki-talk-temporal | insert | 1,000 | 1,444,280 | 552,709 | MEM | CAP | TIME |
| wiki-talk-temporal | insert | 100 | 1,442,344 | 764,786 | MEM | CAP | TIME |

## What the operations show

This is not the verdict. The rule of the protocol is on the clock, the M arms are missing, and M is the baseline this gate exists to test. "Best other" below is the cheapest finished arm of the other rows, B4 aside.

| p | Sharing arm against best other, median | Range | Cells at or under 0.67 | Note |
|---|---|---|---|---|
| 3 | 1.447 | 1.264 to 1.640 | 0 of 12 | S3 is cheapest on all 12. Gate 0 again, on the evaluation cells |
| 4 | 0.916 | 0.249 to 1.642 | 3 of 12, all wiki-talk-temporal | T4 beats P4 on all 4 sx insert cells. P4 / B4 is 0.078 to 0.244 |
| 5 | 0.950 | 0.372 to 2.980 | 4 of 12, all wiki-talk-temporal | WJ beats PR on 6 of the 8 sx cells. On the other 4 cells R2 is the rival |

Read the p = 5 row with care. On wiki-talk-temporal PR's only rival on operations is R2, as WJ never finished there and WT did not finish on insert. Its 4 cells therefore show sharing against a scan, and not against a table. The fifth-closest cell, sx-askubuntu expiry cut 1,000, misses at 0.675. If the protocol's pass line were applied to operations, which it is not, the 4 cells would sit on one stream and fail the two-stream clause.

## Arms stopped at a limit

| Arm | Stream | Label | Cuts | Stop | Reached | Why |
|---|---|---|---|---|---|---|
| T3, T4 | wiki-talk-temporal | insert | 1,000, 100 | MEM | 1,473 of 6,101, 14,730 of 61,006 | Job limit, table at 14.6 GB |
| WT | wiki-talk-temporal | insert | 1,000, 100 | MEM | 1,473 of 6,101, 14,730 of 61,006 | Job limit, table at 14.6 GB |
| WJ | wiki-talk-temporal | expiry | 1,000, 100 | CAP | 12,010 of 12,202, 120,094 of 122,011 | Next table doubling needs 18 GiB during the rehash, over the 16 GB limit |
| WJ | wiki-talk-temporal | insert | 1,000, 100 | CAP | 988 of 6,101, 9,880 of 61,006 | As above |
| A4 | sx-superuser | insert | 100 | TIME | 7,957 of 11,088 | 300 s |
| A4 | wiki-talk-temporal | insert | 1,000, 100 | TIME | 2,186 of 6,101, 11,193 of 61,006 | 300 s |
| A5 | sx-askubuntu | insert | 100 | TIME | 3,113 of 7,267 | 300 s |
| A5 | sx-superuser | insert | 1,000, 100 | TIME | 687 of 1,109, 2,527 of 11,088 | 300 s |
| A5 | wiki-talk-temporal | expiry | 100 | TIME | 63,027 of 122,011 | 300 s |
| A5 | wiki-talk-temporal | insert | 1,000, 100 | TIME | 836 of 6,101, 3,316 of 61,006 | 300 s |

19 arm-cells in all: 6 MEM, 4 CAP, 9 TIME.

## Memory

12 arm-cells passed 12 GB of peak working set, all on wiki-talk-temporal:

| Arms | Label | Cuts | Peak | Status |
|---|---|---|---|---|
| T3, T4, WT | expiry | 1,000, 100 | 13.97 GB | Finished |
| T3, T4 | insert | 1,000, 100 | 15.75 to 15.76 GB | MEM |
| WT | insert | 1,000, 100 | 15.40 and 15.71 GB | MEM |

WJ's peak was 9.73 to 9.99 GB, so its CAP is the rehash bound and not resident memory. Paging is not recorded, so "capped while paging" cannot be ruled out outright. Against it: the MEM stops came from the 16 GB job limit with 15.4 to 15.76 GB resident, so almost all the commit was in RAM at the stop, and the limit that stopped them was the job's and not the machine's.

## Limits of this record

- The M arms are absent, so every "best other" above can only fall.
- These evaluation cells have now been read by every arm but M. They are no less seen than the protocol's weakness section says, and now more so.
- A table arm's stop on memory is partly a limit of this run's 16 GB, as the protocol states. It is out of those cells, and it is not a loss on the merits.
- Raw per-batch files are under `raw/`, ignored by git, 83 MB.
