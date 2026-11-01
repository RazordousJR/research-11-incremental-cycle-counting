# Screen C test M1C01: the D-k family against the database baselines, run 2026-11-01

Protocol: `docs/experiments/screen-b-protocol.md`, M1C01, unchanged. All six streams carry the label **real timestamps, window
query**. Operation counts, not time. Non-citable. k = 4, windows 1, 7, 30 and 90 days, slot widths 3,600 s and 1 s. Base commit
`3b6f414`, this round's edits uncommitted. Hashes, fixed before the first stream: `window_sweep.cpp` 740b63076039,
`test_window.cpp` 886add711e96, `tc.hpp` d3db4c7dec77 (not edited). Full hashes are in `source-hashes.txt`. g++ 16.2.0, same flags
as M1B01-B. Longest run 244 s. One CSV per stream and slot width, 10 arms in each.

## Verdict

**Go, by the written rule, and at 1 s by the smallest margin the rule allows.** Stop is read first and does not hold. Go holds at
both slot widths.

- **Stop:** some arm dominates D-k on all 6 streams. Not met at either width. The most any arm reaches is 0 streams at 3,600 s and
  1 stream at 1 s.
- **Go, first part:** D-k not dominated on at least 4 of 6. Met on **6 of 6 at 3,600 s** and on **exactly 4 of 6 at 1 s**. At 1 s,
  W dominates D-k on sx-mathoverflow and V dominates it on sx-askubuntu.
- **Go, second part:** DI / D-k at least 1.25 on at least 3 streams. Met on 6 of 6 at both widths, from 1.483 to 5.011.
- **Exactness:** 0 mismatches. V finished on all six streams at both widths, with no cap hit.

## What this does not establish

**That D-k is safe on the literal clock.** The 1 s Go rests on two things that this test does not settle, and they pull in
opposite directions.

- **V's charge per record.** V sits just above D-k on operations at 1 s on sx-superuser (1.041) and wiki-talk-temporal (1.050), and
  V is below D-k on words there. If V's list charges were 4.14 units per record lower, sx-superuser is dominated and the 1 s count
  falls to 3 of 6, so the verdict becomes **Pivot**. At 5.41 lower, wiki-talk-temporal goes as well. V's charge is 35 to 73 units per
  record, so that is 11% to 14% of it on those streams. For scale, the free-list charge alone is 4. I fixed these charges before
  the run and I think they are fair, but a reader could fairly argue for a slightly lower one, and the Go would not survive that.
- **The ring constants.** At 1 s the words column is mostly fixed arrays over slots: 33.18M words for D-k's four token rings
  against 31.10M for the one ring and count array of W and V. That gap of 2.07M words is what puts W and V under D-k. With the
  fixed arrays taken out of every arm, **no arm dominates D-k on any stream at either width**. That is a bound and not a design,
  since a hashed queue would cost something per distinct slot.

So the honest reading at 1 s is: D-k is not clearly beaten and not clearly clear of V. At 3,600 s the position is firmer, because V
is above D-k on words on all six streams (1.070 to 2.215).

**That D-k is the cheapest arm.** It is not. W has fewer operations on 4 of 6 at 3,600 s and 3 of 6 at 1 s. V has fewer on
sx-askubuntu at both widths, and it is within 1% at 3,600 s on wiki-talk-temporal. Undominated means D-k trades operations for
memory better than some arm on each stream. It does not mean it wins.

**That the re-home rule is worth 1.5 to 5 times in general.** DI is two full scans per refresh because the protocol says so. One
merged scan is the obvious better generic route. Estimated and not built, it would give DI / D-k of 1.174 to 2.597 at 3,600 s and
1.211 to 2.785 at 1 s. That clause would still pass, on 4 of 6 and 5 of 6.

**That sharing one counter is worth 1.5 to 2 times.** K as built is k independent counters, so its windows also lose D-k's shared
home. A K that keeps a shared home and only stores k copies would be at most 1.067 to 1.272. That form is estimated and not
built, and it adds nothing a designer would want. Neither form can dominate D-k, since both hold more words per pair on the same
store and tokens, so the verdict does not depend on this.

**Novelty.** The rule measures cost. S8 stands: this test says nothing about prior art, and it leaves the ADBIS 2004 risk and the
missing forward citation checks of A-Seq and Jha et al. open.

**V at its limit.** A walk-or-scan hybrid, a singly linked list with tombstones, and a per-pair skip test were not tried. The last
of these would need a per-pair count of weakest triples, which is D-k's own idea.

**Also not established:** time, since no wall-clock comparison was made; real process memory, since words are the model of M1B01;
per-second exactness at 3,600 s; other values of k; and any stream outside these six.

## Scores

### Operations, slot width 3,600 s

| Stream | D-k | W | W-F | V | K | DI | B1 | B2 plain | B2 recency |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| CollegeMsg | 1,141,106 | 1,233,620 | 2,664,536 | 1,232,790 | 2,213,245 | 3,035,095 | 2,901,931 | 3,747,584 | 3,713,088 |
| email-Eu-core-temporal | 9,828,895 | 12,632,231 | 27,615,749 | 13,470,223 | 20,962,843 | 46,338,983 | 12,861,701 | 24,108,280 | 20,896,344 |
| sx-mathoverflow | 17,843,794 | 15,846,783 | 35,393,677 | 25,432,259 | 30,687,627 | 33,616,572 | 33,189,663 | 49,951,780 | 37,594,899 |
| sx-askubuntu | 24,555,596 | 18,263,602 | 45,371,414 | 23,204,604 | 38,490,747 | 38,488,882 | 49,326,130 | 61,378,103 | 48,216,258 |
| sx-superuser | 39,772,473 | 30,271,800 | 73,145,108 | 40,839,573 | 61,518,089 | 59,000,349 | 82,716,117 | 104,517,378 | 82,033,601 |
| wiki-talk-temporal | 261,986,029 | 230,896,968 | 477,054,618 | 264,172,425 | 497,055,658 | 656,445,305 | 483,265,433 | 862,420,999 | 559,495,396 |

### Each arm over D-k, slot width 3,600 s, operations then peak words

| Stream | W | W-F | V | K | DI | B1 | B2 plain | B2 recency | Dominated by |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---|
| CollegeMsg | 1.081, 1.753 | 2.335, 1.792 | 1.080, 1.361 | 1.940, 1.367 | **2.660**, 0.918 | 2.543, 1.606 | 3.284, 0.878 | 3.254, 1.673 | none |
| email-Eu-core-temporal | 1.285, 4.178 | 2.810, 4.240 | 1.370, 2.215 | 2.133, 1.364 | **4.715**, 0.919 | 1.309, 1.598 | 2.453, 0.879 | 2.126, 2.447 | none |
| sx-mathoverflow | 0.888, 2.965 | 1.984, 2.991 | 1.425, 1.764 | 1.720, 1.259 | **1.884**, 0.942 | 1.860, 2.115 | 2.799, 0.914 | 2.107, 2.566 | none |
| sx-askubuntu | 0.744, 3.579 | 1.848, 3.587 | 0.945, 1.070 | 1.567, 1.168 | **1.567**, 0.963 | 2.009, 2.893 | 2.500, 0.944 | 1.964, 3.310 | none |
| sx-superuser | 0.761, 3.817 | 1.839, 3.823 | 1.027, 1.251 | 1.547, 1.170 | **1.483**, 0.962 | 2.080, 2.943 | 2.628, 0.943 | 2.063, 3.512 | none |
| wiki-talk-temporal | 0.881, 2.772 | 1.821, 2.773 | 1.008, 1.086 | 1.897, 1.247 | **2.506**, 0.945 | 1.845, 2.389 | 3.292, 0.918 | 2.136, 2.390 | none |

Not dominated: 6 of 6. DI / D-k at least 1.25: 6 of 6.

### Operations, slot width 1 s, the literal clock

| Stream | D-k | W | W-F | V | K | DI | B1 | B2 plain | B2 recency |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| CollegeMsg | 1,694,456 | 2,189,100 | 4,922,630 | 1,926,113 | 2,897,120 | 5,540,527 | 3,672,181 | 4,257,824 | 4,772,630 |
| email-Eu-core-temporal | 11,801,669 | 16,415,326 | 46,351,465 | 16,925,258 | 23,074,745 | 59,135,012 | 15,520,554 | 25,720,899 | 24,240,393 |
| sx-mathoverflow | 21,163,719 | 20,235,269 | 57,675,040 | 29,405,731 | 34,782,886 | 41,467,143 | 37,510,472 | 52,907,854 | 42,156,618 |
| sx-askubuntu | 31,256,568 | 26,713,973 | 76,475,641 | 30,640,982 | 46,370,753 | 51,939,934 | 58,108,258 | 67,352,341 | 56,710,767 |
| sx-superuser | 50,252,421 | 43,246,990 | 122,364,783 | 52,324,816 | 74,039,867 | 77,183,655 | 96,339,327 | 113,705,232 | 94,694,777 |
| wiki-talk-temporal | 321,587,301 | 327,097,820 | 784,249,428 | 337,798,977 | 578,557,445 | 997,867,740 | 559,236,029 | 904,740,062 | 689,648,316 |

### Each arm over D-k, slot width 1 s, operations then peak words

| Stream | W | W-F | V | K | DI | B1 | B2 plain | B2 recency | Dominated by |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---|
| CollegeMsg | 1.292, 0.944 | 2.905, 1.214 | 1.137, 0.940 | 1.710, 1.002 | **3.270**, 1.000 | 2.167, 1.004 | 2.513, 0.999 | 2.817, 1.005 | none |
| email-Eu-core-temporal | 1.391, 0.950 | 3.928, 1.220 | 1.434, 0.941 | 1.955, 1.001 | **5.011**, 1.000 | 1.315, 1.002 | 2.179, 1.000 | 2.054, 1.005 | none |
| sx-mathoverflow | **0.956, 0.952** | 2.725, 1.222 | 1.389, 0.943 | 1.644, 1.002 | **1.959**, 1.000 | 1.772, 1.008 | 2.500, 0.999 | 1.992, 1.011 | **W** |
| sx-askubuntu | 0.855, 1.006 | 2.447, 1.270 | **0.980, 0.941** | 1.484, 1.004 | **1.662**, 0.999 | 1.859, 1.045 | 2.155, 0.999 | 1.814, 1.059 | **V** |
| sx-superuser | 0.861, 1.024 | 2.435, 1.287 | 1.041, 0.946 | 1.473, 1.005 | **1.536**, 0.999 | 1.917, 1.054 | 2.263, 0.998 | 1.884, 1.075 | none |
| wiki-talk-temporal | 1.017, 1.359 | 2.439, 1.574 | 1.050, 0.966 | 1.799, 1.047 | **3.103**, 0.989 | 1.739, 1.275 | 2.813, 0.984 | 2.145, 1.309 | none |

Not dominated: 4 of 6. DI / D-k at least 1.25: 6 of 6. On sx-superuser at 1 s D-k stands only because W's log leaves W 2.4% over on
words and V is 4.1% over on operations.

### Peak words per arm

One word is 8 bytes, counted as in M1B01. Slot width 3,600 s:

| Stream | D-k | W | W-F | V | K | DI | B1 | B2 plain | B2 recency |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| CollegeMsg | 155,010 | 271,799 | 277,830 | 210,922 | 211,953 | 142,356 | 248,900 | 136,029 | 259,270 |
| email-Eu-core-temporal | 97,289 | 406,476 | 412,506 | 215,454 | 132,690 | 89,422 | 155,458 | 85,488 | 238,084 |
| sx-mathoverflow | 231,324 | 685,967 | 691,998 | 408,044 | 291,262 | 218,007 | 489,156 | 211,348 | 593,639 |
| sx-askubuntu | 795,790 | 2,848,260 | 2,854,290 | 851,713 | 929,485 | 766,080 | 2,302,292 | 751,225 | 2,634,394 |
| sx-superuser | 943,828 | 3,602,134 | 3,608,165 | 1,180,902 | 1,104,298 | 908,168 | 2,778,018 | 890,338 | 3,315,187 |
| wiki-talk-temporal | 8,066,744 | 22,363,374 | 22,369,406 | 8,763,973 | 10,055,578 | 7,624,781 | 19,270,526 | 7,403,800 | 19,278,766 |

Slot width 1 s:

| Stream | D-k | W | W-F | V | K | DI | B1 | B2 plain | B2 recency |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| CollegeMsg | 33,373,883 | 31,501,134 | 40,502,349 | 31,356,768 | 33,430,830 | 33,361,228 | 33,495,530 | 33,354,900 | 33,557,353 |
| email-Eu-core-temporal | 33,309,974 | 31,637,200 | 40,638,415 | 31,355,275 | 33,345,376 | 33,302,108 | 33,389,495 | 33,298,174 | 33,485,348 |
| sx-mathoverflow | 33,420,730 | 31,827,929 | 40,829,144 | 31,524,508 | 33,480,688 | 33,407,406 | 33,688,199 | 33,400,744 | 33,799,894 |
| sx-askubuntu | 34,010,186 | 34,206,872 | 43,208,086 | 31,993,287 | 34,143,894 | 33,980,474 | 35,536,611 | 33,965,618 | 36,007,646 |
| sx-superuser | 34,161,987 | 34,974,465 | 43,975,680 | 32,326,058 | 34,322,475 | 34,126,323 | 36,016,210 | 34,108,491 | 36,709,928 |
| wiki-talk-temporal | 42,037,154 | 57,145,832 | 66,147,047 | 40,623,122 | 44,026,026 | 41,595,185 | 53,580,886 | 41,374,200 | 55,014,804 |

**Memory check on V.** V holds exactly the triples live at the longest window, so its peak was known from the M1B01-B CSVs before
it ran: 233,166 records on wiki-talk-temporal, 1.28M words, about 10 MB. V's whole peak there is 70 MB at 3,600 s and 325 MB at 1 s.
The 6 GB limit was never near, so no cap was built.

## Share of D-k that is re-homing, measured

Counted inside D-k, not inferred. A unit share covers everything from the degree reads of a re-home scan to the store of d.

### Slot width 3,600 s

| Stream | Insertion scans | Re-home at refresh | Re-home at stale pop | Share of scans | Share of list elements | Share of all D-k units | Elements per insertion scan | Per re-home scan |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| CollegeMsg | 13,928 | 1,543 | 766 | 14.2% | 26.6% | 11.5% | 9.3 | 20.3 |
| email-Eu-core-temporal | 23,975 | 82,319 | 27,972 | 82.1% | 81.9% | 52.1% | 14.7 | 14.4 |
| sx-mathoverflow | 219,384 | 25,783 | 8,370 | 13.5% | 37.9% | 20.1% | 9.6 | 37.4 |
| sx-askubuntu | 465,775 | 15,084 | 6,485 | 4.4% | 41.7% | 15.8% | 4.7 | 72.0 |
| sx-superuser | 737,900 | 23,238 | 11,171 | 4.5% | 33.2% | 13.6% | 5.7 | 60.5 |
| wiki-talk-temporal | 2,893,837 | 303,766 | 85,846 | 11.9% | 44.4% | 28.1% | 13.0 | 77.4 |

### Slot width 1 s

| Stream | Insertion scans | Re-home at refresh | Re-home at stale pop | Share of scans | Share of list elements | Share of all D-k units | Elements per insertion scan | Per re-home scan |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| CollegeMsg | 13,928 | 1,523 | 1,123 | 16.0% | 28.8% | 8.6% | 9.3 | 19.8 |
| email-Eu-core-temporal | 23,970 | 81,157 | 32,120 | 82.5% | 82.1% | 44.0% | 14.7 | 14.3 |
| sx-mathoverflow | 219,382 | 25,690 | 10,828 | 14.3% | 38.6% | 17.4% | 9.6 | 36.2 |
| sx-askubuntu | 465,774 | 14,981 | 10,287 | 5.1% | 42.9% | 13.0% | 4.7 | 64.5 |
| sx-superuser | 737,899 | 23,197 | 18,278 | 5.3% | 34.5% | 11.3% | 5.7 | 53.2 |
| wiki-talk-temporal | 2,893,803 | 302,867 | 122,038 | 12.8% | 46.0% | 24.2% | 13.0 | 75.5 |

**S8's scan shares are confirmed, and they understate the cost.** S8 inferred 14%, 82%, 13%, 4%, 4% and 12%. Those match. But re-home
scans run on much longer lists than insertion scans, up to 15 times longer on sx-askubuntu, so the same scans are 27% to 82% of
list elements and 9% to 52% of all D-k units. A share of 4% of scans is 42% of list elements on sx-askubuntu. S8's claim that the
gain comes from removing expiry scans stands. The refresh machinery is a much larger line in the bill than 4% suggests, and
DI / D-k shows that the plain alternative costs more still.

## Arms as built

All three forms were fixed in the source header before any stream was read.

### V: materialized triangle view

One graph at the longest window, no log, one token per pair. Each live triple is one record on an intrusive doubly linked list per
pair. A record is 3 pair ids, 3 next links, 3 prev links and sigma: 44 bytes, 5.5 words, packed. Each pair adds a list head, 0.5
words. Free records are threaded through the record, so they add no words. Peak words count every record ever allocated, and records
are reused. The count is the expiry ring: `C[slot]` holds live triples with that sigma, and k readers subtract it as slots leave
each window, as W does with B. Sigma is always true, so no window needs a walk. C costs 1 word per slot, so this is W's structure with
the recency scan replaced by a stored view.

| Step | Charge |
|---|---|
| Birth | 2 to take a record, 1 sigma, then per list 1 head read, 3 record writes, 1 head write, 1 old head prev write when there is one: 18 to 21. `slot` 2 for C. The insertion scan is W's first scan |
| Refresh | 1 head read. Each record: 1 step, 1 sigma read. If sigma equals the old time: 2 id reads, 2 last reads. If sigma rises: 1 sigma write, `slot` 4. No adjacency scan |
| Expiry | 1 head read. Each record: 1 step, then per other list 2 link reads, 1 write to the previous node or 1 id read and 1 head write, 1 write to the next node when there is one: 3 to 5. 2 to give the record back |

Pair ids count as stored and read, which a reader might find strict. List heads are 32-bit. A 32-bit sigma would make a record 5 words.
That would not change a verdict: at 1 s V is already under D-k on words, and at 3,600 s on sx-askubuntu, the one stream where V is under
D-k on operations at that width, V would still be 1.060 on words.

### K: k death counters per pair, no shared counter

Each window runs its own single-window death counter, the rule of the M1B01 death counter, over one shared graph at the longest
window, with one shared scan per event. This is the GRETA form S8 names. There is no shared home, so there is no believed time to
share either: b and d per pair per window, 1.5 words each, and one token per window as in D-k. Scans read believed times and need no
last time, because a pair is in window i exactly when `b_i > now - L_i`. A triple is tried from the largest window down, and the first
failure ends it. A pair that re-enters a window always scans, as B2 does, filtered on `b_j`, because a counter that reached 0 at exit
says nothing about re-entry. Every window pays its own stale pops and re-home scans.

### DI: delete then insert on every refresh

Exit scan at the old time, entry scan at the new time, both always, whatever d holds. So b = last at all times, a believed time would
be a copy, and DI drops it: 0.5 words per pair. Three choices favour DI, so that DI / D-k is if anything low: the delete is logical
and pays no graph edit, tokens stay lazy as in B2 so a stale token is only re-filed, and d(e) is stored once rather than per triple.
Each scan reads both last times per triple and commits its own count writes.

### New in D-k

Statistics only: re-home scans at a refresh and at a stale pop, list elements by scan kind, and all units inside re-home scans. None
is a charge. The oracle checks that the parts add up to the totals on every trial.

## Exactness

- **Oracle:** `tests/test_window.cpp`, 200,000 trials at seed 7 and at seed 11, PASS both. Output in `oracle-seed7.txt` and
  `oracle-seed11.txt`. 14.9M events and 37.2M window checks at seed 7, the same trials as M1B01-B. New, after every event: V 0
  failures, K 0, DI 0. Against the definition: every V list and record, its links both ways, sigma, and the count ring slot by slot, 0
  failures. K per window d, b and the sum, 0 failures. DI d and the sum, 0 failures. D-k scan split, 0 failures. Every older line stays
  at 0.
- **Coverage, seed 7:** 2.8M V records, 1.38M moved, 2.58M K re-entry scans, 2.91M K re-homed, 6.43M DI exit scans.
- **Mutants, wrong trials of 200,000, seed 7, all caught, seed 11 within 1.1%:** V no sigma write 87,408; V new sigma from one other pair
  110,282; V no unlink 180,934; K no re-entry scan 70,626; K no reset at exit 62,200; K no id tie-break 110,862; DI no exit scan 180,004;
  DI no id tie-break 78,536; DI exit homes at the new time 149,703. V no unlink still counts correctly, so it is judged on structure. They
  are built into the test and it fails if any one passes. The older mutants give exactly the M1B01-B figures at seed 7, and the controls
  still fail as required.
- **Real streams:** every arm agreed on every window after every event on all six streams at both widths. Static counts: 0 mismatches
  on 24,112 checks at each width.
- **Independent check on V:** records born and units moved equal W's on every run, and peak records equal the peak count on every run,
  0 differences. V and W share no scan or update code.
- **Old arms:** W, W0, B1, both B2 forms, W-F and D-k against the M1B01-B CSVs, every column: **0 differences in 2,940 cells.**

## Corrections to earlier notes

- `results/screen-b-M1B01-B/README.md` says of W against D-k that neither dominates. That holds at 3,600 s. At 1 s W does dominate D-k
  on sx-mathoverflow, in that test's own numbers, which this run reproduces. I did not edit that file.
- S8's table is a scan count and reads as the cost of re-homing. Measured, the cost is several times larger, as above.

## Process notes

- The source was fixed and hashed before the first stream. Nothing in it changed after. Hashes rechecked after the last run.
- The summariser was extended after the runs for reporting only: the margin and other-forms tables. Rule lines were written before
  the runs. It cannot change a CSV.
- Binaries are `build/r11_window_c.exe` and `build/r11_test_window_c.exe`, so the M1B01-B binaries stay as they were.
- One process at a time: oracle seeds, then 3,600 s, then 1 s. Oracle 100 s and 101 s. Longest stream 171 s at 3,600 s and 244 s at 1 s.
- Other jobs were sharing the machine, so no timing here means anything beyond the ten-minute rule.
- Rebuild: `scripts/run_window_sweep_c.sh 3600 1`, then `python -I scripts/summarise_window_c.py`. Its header has the build lines.
