# Screen C test M1C02: exact dynamic 5-cycles, kernels and a kill test, run 2026-11-01

Protocol: `docs/experiments/screen-b-protocol.md`, M1C02, unchanged. Operation counts, not time. Non-citable. Streams carry the label
**real timestamps, synthetic expiry** or **real timestamps, insert-only, no expiry**, shown per row. Three streams, both variants, cuts of 100
and 1,000 events: 12 cells, all finished. Base commit `3b6f414`, this round uncommitted. Hashes, recorded before the first stream was read, are
in `hashes.txt`: `fivecycle.cpp` f7bff0195d28, `test_fivecycle.cpp` 01e0e7a426f4, `tc.hpp` d3db4c7dec77 (not edited), `run_fivecycle.sh`
9e2403e61ec9, `summarise_fivecycle.py` e169fbc2f6f8. All five match after the runs. g++ 16.2.0, the given flags plus `-lpsapi` for the peak
resident size call. 72 processes, one at a time, 284 s of run time in all, longest 42.7 s. No cap hit, no time stop.

## Verdict

**Stop, by the written rule.** Stop is read first. One clause holds.

| Stop clause | Result |
|---|---|
| Exactness failure not traced to a test bug | No. 0 mismatches |
| Reading finding | No. See `reading.md`. Gopher and Cheetah full text were not reached, recorded as a risk |
| One kernel beats every other by more than 2 on all 12 cells | No. WJ does so on 3 of 12. Margins run from 1.04 to 3.61 |
| **WJ wins all 12 cells and its peak table is under 1 GB on every stream** | **Yes.** 12 of 12. Peaks 12.6 MB, 6.3 MB and 805 MB |

Go would have failed on both of its counts: WJ is best on 12 cells, above the limit of 9, and HS / best is at most 0.67 on 0 cells, where it needed
3. HS / best runs from 0.910 to 0.996.

The verdict is the same with HS-T in the place of HS and with R2B in the place of R2. Both forms are explained below.

**Read this with the verdict.** Two things are true at once.

1. **Stop against Pivot is thin.** Two margins carry it, and either one going the other way gives Pivot.
   - sx-mathoverflow expiry at the 1,000 cut is won by 4%, 101.5M against PR's 105.6M. A table operation at 1.08 units in place of 1 loses that cell.
   - The 805 MB is the settled table. During its last doubling that table reaches 1.21 GB for a moment, and the process peaked at 1.23 GB resident.
     The summariser, hashed before the runs, reads the settled figure, so that is the result. Read as the transient, the clause fails.
2. **Go is out of reach either way, and that part is firm.** Hindsight over these kernels never gets near 0.67. It stays above 0.80 at every table
   weight tried. A selector over these kernels has almost nothing to win on these streams.

## What this does not establish

- **That WJ is the fastest kernel. On the clock it is the slowest of the three.** This is the largest gap in the result. WJ wins 12 of 12 in operations and
  loses 12 of 12 on the indicative clock, by 1.7 to 12.8 times against the fastest of R2 and PR. On the largest cell PR took 3.3 s and WJ 42.7 s. Rule 3
  says operations decide, and they did. But the Stop rests on a hash probe costing the same as a stamp read, which S1 had flagged as favouring the
  table. WJ wins 7 cells at weight 2, 3 at weight 3, and none at 8. The clock here implies a weight far above 8 on the largest cell, on a shared machine
  with a table not tuned for cache, so that figure is a rough sign and not a measurement of the weight. S1's own kill test read wall-clock time. The
  protocol changed that to operations, and it is the protocol that binds.
- **That the paper is a one-line lemma in every sense.** That is what the Stop clause says, and it holds under unit costs. If a table probe is really
  dear, PR is the best kernel on all 12 cells, and that would be a different and scan-based story. Neither this test nor any pinned run has priced it.
- **That the table fits in general.** The three streams are the protocol's, and sx-mathoverflow is the smallest of the sx streams. S1's wedge sums reach
  4.4e8 among the sx streams and 1.66e10 on wiki-talk-temporal, against 5.5e7 measured here. Those streams were not run. "Under 1 GB on every stream" means these three.
- **The 1 GB line is layout-dependent.** At 16 bytes a slot, the 12-byte table's 2^26 slots would be 1.07 GB. Packed with no empty slots it would be 308 MB.
- **That nobody has done this.** Gopher and Cheetah remain unread in full. Public Gopher code defines a pentagon pattern and does not run it. See `reading.md`.
- **That hub structure does not matter.** S1's claim was that hub structure decides the kernel. These streams never made the table the loser under unit
  costs, so the claim was not tested. wiki-talk-temporal, the hub-heavy stream, is outside this protocol.
- **RC at its best.** Every batch re-sorts, re-peels and rebuilds all lists. An RC that kept its order between batches would be cheaper. RC totals are lower
  bounds on 9 of 12 cells (see Budget), which cannot change a winner, because each of those bounds is above the second-best kernel.
- **A selector.** HS has no selection cost and none was built.
- **Exact hindsight at heavier table weights.** Those figures are approximate, since per-batch table operations were estimated and not logged.
- **Worst-case update time.** Only totals are scored. Per-cell means are below.

## Scores

Operations summed over the stream, all costs inside. `+` marks an RC lower bound. HS is the rule's form.

**Label: real timestamps, synthetic expiry.**

| Stream | Cut | R2 | WJ | PR | RC | Best | Second / best | HS / best | HS-T / best |
|---|---:|---:|---:|---:|---:|---|---:|---:|---:|
| CollegeMsg | 100 | 42,546,062 | 19,305,796 | 32,209,918 | 187,337,156+ | WJ | 1.668 | 0.934 | 0.970 |
| CollegeMsg | 1,000 | 41,152,131 | 18,753,032 | 24,861,811 | 66,884,535 | WJ | 1.326 | 0.969 | 0.989 |
| email-Eu-core-temporal | 100 | 132,956,253 | 70,329,075 | 98,826,681 | 606,754,604+ | WJ | 1.405 | 0.975 | 0.994 |
| email-Eu-core-temporal | 1,000 | 105,096,884 | 55,603,699 | 68,289,445 | 144,385,647 | WJ | 1.228 | 0.993 | 0.999 |
| sx-mathoverflow | 100 | 215,211,824 | 117,047,588 | 174,745,786 | 884,204,920+ | WJ | 1.493 | 0.995 | 0.999 |
| sx-mathoverflow | 1,000 | 186,440,955 | 101,468,233 | 105,637,222 | 137,786,209 | WJ | **1.041** | 0.937 | 0.952 |

**Label: real timestamps, insert-only, no expiry.**

| Stream | Cut | R2 | WJ | PR | RC | Best | Second / best | HS / best | HS-T / best |
|---|---:|---:|---:|---:|---:|---|---:|---:|---:|
| CollegeMsg | 100 | 56,920,853 | 21,562,177 | 43,101,334 | 253,723,183+ | WJ | 1.999 | 0.910 | 0.949 |
| CollegeMsg | 1,000 | 56,867,773 | 21,600,519 | 34,594,704 | 186,201,099+ | WJ | 1.602 | 0.966 | 0.976 |
| email-Eu-core-temporal | 100 | 140,762,283 | 57,230,586 | 120,197,562 | 956,733,880+ | WJ | 2.100 | 0.969 | 0.984 |
| email-Eu-core-temporal | 1,000 | 140,735,647 | 57,259,533 | 108,229,110 | 561,472,535+ | WJ | 1.890 | 0.995 | 0.999 |
| sx-mathoverflow | 100 | 16,223,520,812 | 3,916,258,142 | 14,132,006,471 | 65,266,203,673+ | WJ | 3.609 | 0.980 | 1.000 |
| sx-mathoverflow | 1,000 | 16,215,015,360 | 3,923,024,123 | 10,557,516,968 | 63,576,189,821+ | WJ | 2.691 | 0.996 | 1.000 |

## Per-update cost

Operations per net update. Range across the cells of each variant.

| Kernel | Expiry | Insert-only |
|---|---:|---:|
| WJ | 209 to 606 | 1,558 to 20,869 |
| PR | 218 to 1,011 | 2,500 to 75,176 |
| R2 | 384 to 1,335 | 4,110 to 86,302 |
| R2B (both rows stored) | 681 to 2,344 | 6,846 to 134,622 |
| RC | 284 to 5,878 | 13,456 to 347,187 |
| PW | 253 to 648 | 1,428 to 20,515 |

RC figures are lower bounds except on the three expiry cells at the 1,000 cut. Per stream, WJ reads 606, 315 and 240 at the 100 cut on expiry, and 1,558, 3,563 and
20,833 on insert-only, against the 6 to 64 of a triangle update at Gate 0. S1's model gave 181 to 6,585 and 1,625 to 162,899 over its six streams, so this is broadly in
line. The dearest single WJ update, 47M operations, is almost all a table rehash: 2 per entry over 23.5M entries. R2's dearest is 688K.

## Peak table

| Stream | Variant | Entries | Allocated, the rule's figure | Last rehash | Peak resident | Wedges, sum of C(d, 2) |
|---|---|---:|---:|---:|---:|---:|
| CollegeMsg | expiry | 86,598 | 1.6 MB | 2.4 MB | 10 MB | 137,522 |
| | insert | 367,064 | 12.6 MB | 18.9 MB | 26 MB | 755,882 |
| email-Eu-core-temporal | expiry | 31,844 | 0.8 MB | 1.2 MB | 20 MB | 51,866 |
| | insert | 223,377 | 6.3 MB | 9.4 MB | 22 MB | 1,183,216 |
| sx-mathoverflow | expiry | 35,654 | 0.8 MB | 1.2 MB | 27 MB | 64,290 |
| | insert | 25,674,972 | **805 MB** | **1.21 GB** | **1.23 GB** | 54,999,653 |

One slot is 12 bytes, load at most 0.7, doubling. On expiry streams the table is about 1 MB, so it is no burden at all. Table upkeep is 7% to 28% of WJ's
operations, highest on expiry streams, where edges churn.

## How far batch sharing gets

| Stream | Variant | Cut | PR / R2 | **PW / WJ, sharing on the best kernel** | Spokes in stars of 2 or more, share of net |
|---|---|---:|---:|---:|---:|
| CollegeMsg | expiry | 100 | 0.757 | 1.070 | 0.579 |
| | | 1,000 | 0.604 | 1.069 | 0.790 |
| | insert | 100 | 0.757 | 0.978 | 0.632 |
| | | 1,000 | 0.608 | 0.915 | 0.837 |
| email-Eu-core-temporal | expiry | 100 | 0.743 | 1.098 | 0.598 |
| | | 1,000 | 0.650 | 1.135 | 0.757 |
| | insert | 100 | 0.854 | 0.931 | 0.425 |
| | | 1,000 | 0.769 | 0.903 | 0.615 |
| sx-mathoverflow | expiry | 100 | 0.812 | 1.210 | 0.586 |
| | | 1,000 | 0.567 | 1.210 | 0.805 |
| | insert | 100 | 0.871 | 0.985 | 0.621 |
| | | 1,000 | 0.651 | 0.919 | 0.815 |

**On top of the best kernel, sharing gets at best 0.903, and on all six expiry cells it costs more than it saves.** That is 0.90 to 0.99 on insert-only and 1.07 to 1.21
on expiry. Stars are common, up to 84% of net updates, but a join gains only when spokes share neighbours, and the memo costs operations whether they do or not.
S1's model gave 0.80 to 1.06 on its own cells, so this is somewhat worse than that estimate. On top of a scan, sharing is real: PR / R2 is 0.57 to 0.87.
PR measures sharing alone, because R2 and PR use the same streamed spoke side.

## If a table operation costs more than 1

Outside the rule, written after the runs (`sensitivity.txt`). Winners by weight are exact, from the totals. Hindsight by weight is approximate.

| Table operation weight | 1 | 1.25 | 1.5 | 2 | 3 | 4 | 8 |
|---|---:|---:|---:|---:|---:|---:|---:|
| Cells WJ wins | 12 | 11 | 10 | 7 | 3 | 2 | 0 |
| Cells with HS / best at most 0.67 | 0 | | | 0 | 0 | 0 | 0 |

Break-even weight per cell runs from 1.08 to 6.05, lowest on expiry at the 1,000 cut and highest on sx-mathoverflow insert-only. The kernel that takes over is PR in all 12
cells, never R2 or RC. Lowest approximate HS / best at any weight: 0.808. So a rule read at a weight near 2 would not be Stop, and would still not be Go.

Per batch at weight 1: WJ is cheapest on 64% to 99.7% of batches, PR on most of the rest, R2 on at most 0.3%, RC on at most 9.5%.

Indicative clock, R2, PR and WJ only, since the budget stops RC early. The machine was shared with other jobs. Fastest per cell: PR on 9, R2 on 3, WJ on 0.

| Stream | Variant | Cut | R2 s | PR s | WJ s |
|---|---|---:|---:|---:|---:|
| CollegeMsg | expiry | 1,000 | 0.031 | 0.027 | 0.102 |
| email-Eu-core-temporal | expiry | 1,000 | 0.125 | 0.119 | 0.277 |
| sx-mathoverflow | expiry | 1,000 | 0.229 | 0.218 | 0.376 |
| sx-mathoverflow | insert | 100 | 5.315 | 4.625 | 42.088 |
| sx-mathoverflow | insert | 1,000 | 5.466 | 3.328 | 42.706 |

## Forms I fixed before the first stream

The protocol leaves these open. Each is in the header of `fivecycle.cpp`, which is hashed. None changes the verdict.

| Open point | Form scored | Form reported beside it | Effect |
|---|---|---|---|
| "Two-row scan" | **R2:** row A stored, C streamed through it, no join term. That is how S1 priced it, and PR uses the same spoke side | R2B: both rows stored and joined | Same verdict. R2B costs 1.48 to 1.77 times R2 |
| "Per-batch hindsight best" | **HS:** per-batch minimum, as written. A batch not using WJ pays no table upkeep, so no real selector can reach it | HS-T: two-state hindsight, table on or off. Upkeep paid while on, 3 per wedge to turn on. A true bound | Same verdict. HS-T / best is 0.949 to 1.000 |
| "Peak table" | Allocated slots times 12 bytes, read by the hashed summariser | Rehash peak and peak resident, shown above | **This one could change the verdict, to Pivot** |

## Budget on RC

A full recount at every batch on insert-only streams cannot meet the machine rule at the 100 cut. Fixed before the first stream: RC stops a recount once it passes 4 times the dearest of
R2, WJ and PR on that batch, and the batch is charged what was spent. A censored batch cannot be the hindsight minimum, and it cannot decide a factor of 2. RC is exact on the
three expiry cells at the 1,000 cut and a lower bound on the other nine, with 32% to 99.6% of batches censored. In all nine the bound is above the second-best kernel, so no winner or margin
depends on it. A forced recount, not charged, checks counts at about 20 batches per cell.

## The formula

Derived in the header of `fivecycle.cpp` and tested, not assumed. **The protocol's formula is right, provided that b is held out of {u, v} in both rows.** It reads no fact about the pair
itself, so it holds with the edge present or absent, and d(t) is unaffected, since t is neither endpoint. Mutant 3 shows the exclusion matters. The WJ form is

`sum over a in N(u), c in N(v), a != c of W[a, c], minus tau (d(u) - 1), minus tau (d(v) - 1)`, with the pair out of the graph and the table, and tau = |N(u) and N(v)|.

W counts b = u exactly when c is also in N(u), and b = v exactly when a is also in N(v). tau comes from the a = c cases of the loop, so WJ reads no degree. That sum is the one-line
lemma the Stop clause refers to.

RC is Algorithm 1 of Shi, Huang, Shun, sequential, after Kowalik: relabel by non-increasing degree, orient by a degeneracy order. Its O(m alpha^2) shape is asserted on every recount of the oracle: wedges
visited at most the sum over edges of min(d(u), d(v)), and inner steps at most wedges times (degeneracy + 1). The third walk must stop at x above v, the subscript on `N->_v(w)` in line 14 of their
listing. Without it the count is wrong (mutant 12).

## Charges

As Gate 0: 1 per list element scanned, 1 per adjacency probe, 2 per raw event, 2 per applied edit, 3 per net update for the group arms. Added by the protocol: 1 per row or table read and 1 per write, so an
increment is 2. Degree reads 1. A created or removed table key is 1 more write. A rehash is 2 per live entry moved. A row holds its epoch in the same word, so there is no reset cost. RC is charged for its sort,
peel, relabel and list building on each batch, and for keeping its list of active vertices. A batch with no net update does not recount. Hash probes cost 1, which favours WJ, as above.

## Exactness

- **Oracle:** `tests/test_fivecycle.cpp`, 200,000 trials at seed 7 and again at seed 11, PASS both. After every batch, each of R2, R2B, WJ, PR, PW and RC against a brute count over vertex tuples. About 1.5M batches and
  7.4M net updates per seed, 1.9M of them deletions, plus 7.9M no-ops and 1.1M repeated pairs in one batch. Graphs of 5 to 13 vertices with hub, dense and sparse families, and a stream of 48 vertices run 600 batches.
- **The brute force was itself checked** 93,862 times at seed 7 against the closed-walk formula of Alon, Yuster and Zwick.
- **Every kernel on every pair,** present or absent, against a brute path count: 7.4M checks per seed. The wedge table entry by entry after every batch. Operation counts identical between repeat runs. A stopped recount
  leaves later recounts exact. The memory cap stops the table and the run ends.
- **Control:** the naive per-edge sum fails, wrong on 777,448 of 1,501,792 batches at seed 7 and 774,905 of 1,498,390 at seed 11.
- **Mutants, wrong trials of 4,000, seed 7, all caught, seed 11 likewise:** 1 no degenerate term 3,715; 2 d(t) for d(t) - 2 3,811; 3 b = other endpoint kept 3,809; 4 WJ drops tau (p - 1) 3,715; 5 WJ upkeep one side
  3,782; 6 PR no row add on insert 3,497; 7 PR no row removal on delete 2,910; 8 RC no T subtraction 3,323; 9 RC no adjacency term 3,343; 10 PW memo stale 3,573; 11 PW drops tau (q - 1) 3,568; 12 RC x not above v
  3,805. They are built into the test and it fails if any one passes.
- **Real streams:** all arms agree on every batch of all 12 cells, 131,783 comparisons, 0 mismatches. The static recount agrees on 2,628 batches, including the last of every cell. Every expiry cell ends at exactly 0. Each
  insert-only stream ends at the same count at both cuts: 18,543,001, 245,585,096 and 27,396,207,208.
- **Against the Gate 0 record:** net updates, peak edges and batch counts match exactly on all six expiry cells.

## Process notes

- The reading came first and is in `reading.md`.
- Bugs caught before the source was fixed, so neither reached a stream: RC stamps that could carry over between recounts, found on reading the code back, and a double-counting table builder in the test itself.
- One smoke test used a synthetic stream in the scratchpad before the source was fixed. It is not reported anywhere here, and no protocol stream was read before the hashes were written.
- `scripts/sensitivity_fivecycle.py` was written after the runs, for reporting only. It changes no count and no threshold, and the rule does not read it.
- `results/screen-c-M1C02/raw/` holds per-batch files and is ignored by git. `fivecycle.csv` has one row per cell and arm, plus HS and HS-T.
- Nothing in `tc.hpp`, `window_sweep.cpp`, `test_window.cpp`, `log.md`, `STATUS.md`, `docs/tasks` or `C:/Develop/tower` was touched. Nothing was committed, and no dataset was downloaded.
- Rebuild: compile as in `hashes.txt`, `scripts/run_fivecycle.sh`, then `python -I scripts/summarise_fivecycle.py` and `python -I scripts/sensitivity_fivecycle.py`.
