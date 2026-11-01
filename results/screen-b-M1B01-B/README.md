# Screen B test M1B01-B: the same family against its strongest baseline, run 2026-11-01

Protocol: `docs/experiments/screen-b-protocol.md`, M1B01-B, unchanged. All six streams carry the label **real timestamps,
window query**. Operation counts, not time. Non-citable. k = 4, windows 1, 7, 30 and 90 days, slot widths 3,600 s and 1 s. Base
commit `3b6f414`, this round's edits uncommitted. Hashes: `window_sweep.cpp` 543ba1e3d8de, `test_window.cpp` 708962c42188,
`tc.hpp` d3db4c7dec77 (not edited). g++ 16.2.0, same flags as M1B01. Longest run 199 s. One CSV per stream and slot width.

## Verdict

**Go, by the written rule.** Stop is read first and does not hold. Go holds at both slot widths.

- **Stop:** `min(W-F, D-k) / best` above 0.80 on all 6. Not met at either width. The highest score is 0.764.
- **Go:** at most 0.67 on at least 3 streams, at both widths. Met on 5 of 6 at 3,600 s and on 5 of 6 at 1 s.
- **Capability check: the "any L" claim does not survive.** `W-F / D-k` is at most 1.5 on 0 of 6 streams at both widths, in the
  form I fixed before the run. Read literally, the method is the fixed-k form and the capability is dropped. See the form
  question below, because the other form passes.

**Read this with the verdict: the Go was earned by D-k, not by W.** `min(W-F, D-k)` is D-k on every stream at both widths. W-F
alone would reach 0.67 on no stream. So this says a death counter beats the scan baselines at k = 4. It does not say that the
B array or the every-window idea does.

## Scores

`best` is the cheaper of B1 and the better B2, per stream. Bold columns are the two the rule reads.

### Slot width 3,600 s

| Stream | B1 | Better B2 | W | W-F | D-k | W-F / best | D-k / best | min / best | W-F / D-k |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| CollegeMsg | 2,901,931 | 3,713,088 | 1,233,620 | 2,664,536 | 1,141,106 | 0.918 | 0.393 | **0.393** | **2.335** |
| email-Eu-core-temporal | 12,861,701 | 20,896,344 | 12,632,231 | 27,615,749 | 9,828,895 | 2.147 | 0.764 | **0.764** | **2.810** |
| sx-mathoverflow | 33,189,663 | 37,594,899 | 15,846,783 | 35,393,677 | 17,843,794 | 1.066 | 0.538 | **0.538** | **1.984** |
| sx-askubuntu | 49,326,130 | 48,216,258 | 18,263,602 | 45,371,414 | 24,555,596 | 0.941 | 0.509 | **0.509** | **1.848** |
| sx-superuser | 82,716,117 | 82,033,601 | 30,271,800 | 73,145,108 | 39,772,473 | 0.892 | 0.485 | **0.485** | **1.839** |
| wiki-talk-temporal | 483,265,433 | 559,495,396 | 230,896,968 | 477,054,618 | 261,986,029 | 0.987 | 0.542 | **0.542** | **1.821** |

### Slot width 1 s, the literal clock

| Stream | B1 | Better B2 | W | W-F | D-k | W-F / best | D-k / best | min / best | W-F / D-k |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| CollegeMsg | 3,672,181 | 4,257,824 | 2,189,100 | 4,922,630 | 1,694,456 | 1.341 | 0.461 | **0.461** | **2.905** |
| email-Eu-core-temporal | 15,520,554 | 24,240,393 | 16,415,326 | 46,351,465 | 11,801,669 | 2.986 | 0.760 | **0.760** | **3.928** |
| sx-mathoverflow | 37,510,472 | 42,156,618 | 20,235,269 | 57,675,040 | 21,163,719 | 1.538 | 0.564 | **0.564** | **2.725** |
| sx-askubuntu | 58,108,258 | 56,710,767 | 26,713,973 | 76,475,641 | 31,256,568 | 1.349 | 0.551 | **0.551** | **2.447** |
| sx-superuser | 96,339,327 | 94,694,777 | 43,246,990 | 122,364,783 | 50,252,421 | 1.292 | 0.531 | **0.531** | **2.435** |
| wiki-talk-temporal | 559,236,029 | 689,648,316 | 327,097,820 | 784,249,428 | 321,587,301 | 1.402 | 0.575 | **0.575** | **2.439** |

## The form question on W-F, which decides the capability check

The protocol says "method W with a Fenwick tree over B", and that all Fenwick reads and writes are charged. It does not say how
W-F serves the k standing windows. There are two honest forms. I fixed the first in the source header before any M1B01-B
stream was read, because it is the one where the reads actually happen. The second is arguably the plainer reading of "W with".

| | Through the tree (primary) | Sums plus an idle tree |
|---|---|---|
| k windows after every event | read through the tree, charged | W's running sums, at W's charges |
| Ad hoc length | same read | charged per read, none in this cost |
| Total | W-F column above | W + `fen` |
| Streams with W-F / D-k at most 1.5, 3,600 s | **0 of 6** | 5 of 6 |
| Streams with W-F / D-k at most 1.5, 1 s | **0 of 6** | 3 of 6 |

| Stream | Sums form / D-k, 3,600 s | 1 s | Ad hoc reads per event before 1.5, 3,600 s | 1 s |
|---|---:|---:|---:|---:|
| CollegeMsg | 1.327 | 1.657 | 0.42 | past 1.5 |
| email-Eu-core-temporal | 1.856 | 2.482 | past 1.5 | past 1.5 |
| sx-mathoverflow | 1.361 | 1.791 | 0.53 | past 1.5 |
| sx-askubuntu | 0.969 | 1.238 | 1.47 | 0.48 |
| sx-superuser | 1.012 | 1.292 | 1.44 | 0.40 |
| wiki-talk-temporal | 1.120 | 1.447 | 1.34 | 0.12 |

So the sums form keeps the capability only under a light query load: on the literal clock, about one ad hoc read per 2 to 8
events, on exactly the three streams that pass, and with no margin to spare on wiki-talk-temporal. Under it the verdict is still
Go, since D-k still sets `min`. The break-even reads are estimates, from mean units per prefix. Which form the check means is
the PI's reading, not mine. I report 0 of 6 as the result because that form was declared first.

## Was one counter enough? The claim, tested

**The claim holds, but its stated reason is not sufficient, and taken at face value it gives wrong counts.**

- **Tested directly.** After every event of every oracle trial: T[i] equals the sum of d(e) over pairs with b(e) above
  now - L[i], for every window. 0 failures.
- **Taken at face value it fails.** One counter with re-homing only at the stale pop, as in M1B01, is wrong on 79,725 of about
  100,000 k = 4 trials at seed 7. It cannot fail at k = 1, which is why M1B01 never saw this. The cause: a pair that has left
  window 0 and then refreshes raises the sigma of its triples at once, so they can re-enter smaller windows at that event.
- **"All triples that share a weakest pair leave together" is true only of true weakest pairs.** Under lazy re-homing, a
  triple's home can be a stale pair that is not its weakest. That is harmless only because of invariant I2 below.

### D-k as built

State per pair: `last`, believed time `b <= last`, counter `d`. A triple's home is the pair with the smallest `(b, id)`. One token
per pair per window it is in, as in B2.

| Event | Action |
|---|---|
| New pair | Scan. Each common neighbour: `d[home] += 1`, and count in each window above `min(b(f), b(g))` |
| Refresh, pair still in window 0 | Write `last` only |
| Refresh, pair has left window 0 | Read `d`. If not zero, scan now: re-home those triples, count each in windows the pair had left and `min(b(f), b(g))` is inside. Set `b = last`, push tokens for those windows |
| Window 0 token, stale | If `d` not zero, scan and re-home. Set `b = last`, re-file |
| Larger window token, stale | Re-file. No re-homing |
| Any token, true exit | `T[i] -= d(e)`, no scan. At the longest window, remove the pair |

Order: windows ascend inside an advance, and within a slot of window 0 stale pops run before true ones.

Invariants after each advance:
- **I1.** `d(e)` is the number of live triples homed on e. Births and re-homes keep it, and a triple dies only with its home.
- **I2.** `b(e) < last(e)` only while `b(e)` is inside window 0. A refresh in window 0 is the only way to make a pair stale, and
  its window 0 token fires at `b + L0`, when `b` rises. A pair outside window 0 has `b = last`.
- **I3.** Let h be the home of T, and sigma its true minimum. All three pairs have `b >= b(h)` and `last >= b`, so
  `sigma >= b(h)`. If h is not stale, `sigma = b(h)`. If h is stale, both lie inside window 0 by I2. Either way T is in window i
  exactly when `b(h)` is, for every i. This is the identity above.
- **Why `d(e) = 0` needs no scan on refresh.** If T is not homed on e, then some other pair holds sigma, or a tie does, and raising
  `last(e)` leaves sigma alone. A pair outside window 0 has exact times, so nothing can be missed.
- **Why window 0 is enough for stale pops.** By I2 a larger window never meets a stale pair at its own exit, since `b` passes
  `now - L0` first and windows ascend.

## Charges

Every M1B01 charge stands, and the old arms match the M1B01 CSVs in every category, including peak words: 0 differences across
all stream, slot width and arm combinations. New items:

| Item | Cost | W-F | D-k |
|---|---:|---|---|
| Fenwick node on an update | 2, read and write | 1 per path node, per add | none |
| Fenwick move | 2 per node | two paths until they meet | none |
| Fenwick node on a read | 1 | k + 1 prefixes per event, shared newest prefix, 1 for the total on a wrap | none |
| Fenwick update at horizon clear | 2 per node | when the slot is not zero | none |
| Running sums, k boundary readers | | none; one reader for the horizon | none |
| Scans, units, log, tokens | as W | same | none |
| Believed time, on new pair and on setting | 1 write | none | yes |
| Death counter, on new pair | 1 write | none | yes |
| Believed time reads in a scan | 1 each | none | 2 per common neighbour; read first on re-home scans, so a neighbour that cannot hold the home skips the probe |
| Read `d` on an out-of-window-0 refresh, on a stale pop, on a true exit | 1 | none | yes |
| `d[home] += 1` | 2 | none | per triangle born or re-homed; 1 to store what stays on e |
| Tokens | as B2 | one per pair | one per pair per window |
| Running count write | 1 | none | per window, per scan or exit, when not zero |

D-k stops a re-home scan once `d(e)` triples are found. The M1B01 death counter was probe-first and had neither refinement, and I
left it unchanged, so its M1B01 numbers are not comparable with D-k at k = 1.

## Memory and work

One word is 8 bytes, counted as in M1B01. W-F adds B and a tree of size 2^12 at 3,600 s and 2^23 at 1 s. The sums form adds k more.

| Stream | Peak words W-F, 3,600 s | D-k | B2 plain | B2 recency | Fenwick update units per event | Read units per event | D-k scans per event | Better B2 | D-k refresh scans per event | D-k stale-pop scans per event | Re-homed per event |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| CollegeMsg | 277,830 | 155,010 | 136,029 | 259,270 | 4.7 | 19.9 | 0.271 | 1.429 | 0.026 | 0.013 | 0.064 |
| email-Eu-core-temporal | 412,506 | 97,289 | 85,488 | 238,084 | 16.9 | 28.8 | 0.404 | 1.313 | 0.248 | 0.084 | 0.867 |
| sx-mathoverflow | 691,998 | 231,324 | 211,348 | 593,639 | 21.6 | 30.2 | 0.649 | 3.169 | 0.066 | 0.021 | 0.342 |
| sx-askubuntu | 2,854,290 | 795,790 | 751,225 | 2,634,394 | 7.6 | 30.5 | 0.671 | 3.342 | 0.021 | 0.009 | 0.111 |
| sx-superuser | 3,608,165 | 943,828 | 890,338 | 3,315,187 | 9.0 | 30.5 | 0.697 | 3.467 | 0.021 | 0.010 | 0.116 |
| wiki-talk-temporal | 22,369,406 | 8,066,744 | 7,403,800 | 19,278,766 | 10.3 | 30.4 | 0.538 | 2.638 | 0.050 | 0.014 | 0.199 |

D-k needs 1.06 to 1.14 times the memory of B2 plain and has no log, while W-F needs 1.8 to 4.2 times D-k. At 1 s the rings dominate:
on wiki-talk-temporal W-F peaks at 66.1M words, D-k at 42.0M, B2 plain at 41.4M and B2 recency at 55.0M. Most of the D-k and B2
figure there is 3 words per bucket header across about 11M one-second slots, which a hashed queue would remove for all token arms.

Fenwick reads are 33% to 49% of W-F at 3,600 s, and updates 11% to 24%. Tokens are 16% to 30% of D-k, its largest overhead.

W against D-k, outside the rule, since W has no any-L capability and the rule does not read it: W is cheaper on 4 of 6 at 3,600 s
(0.744 to 0.888) and 3 of 6 at 1 s (0.855 to 0.956). D-k is cheaper on CollegeMsg, email-Eu-core-temporal and, narrowly at 1 s,
wiki-talk-temporal. Neither dominates. D-k uses much less memory.

## Exactness

- **Oracle:** `tests/test_window.cpp`, 200,000 trials at seed 7 and again at seed 11, PASS both. 14.9M events and 37.2M window checks
  at seed 7. Every arm of M1B01 stays at 0 failures. New, after every event: W-F 0 failures, 0 failures on 186.2M reads of every
  length from 1 to Lmax, every tree node equal to the sum of B over its range, D-k 0 failures, and 0 failures on I1, I2 and the
  identity. About half the trials are k = 1, where the refresh path of D-k does not arise.
- **Coverage, seed 7:** 1.9M D-k triples re-homed, 618K immediate re-home scans, plus the M1B01 coverage of repeats, equal slots and
  horizon re-entries.
- **Mutants, wrong trials of 200,000, seed 7, all caught:** W-F plain add 157,017; W-F no tree update at the clear 172,196; W-F
  move keeps the old unit 157,017; D-k lazy only 79,725; D-k true pops before stale 26,909; D-k no id tie-break 98,386. They are
  built into the test and it fails if any one passes. The M1B01 controls still fail as required.
- **Real streams:** W, W0, B2 both forms, B1, W-F and D-k agreed on every window after every event on all six streams at both
  widths. Static counts as in M1B01: 0 mismatches on 24,112 checks at each width.

## What this does not establish

- **That W or B is the contribution.** The Go rests on D-k, whose principle S6 traces to Ghanem et al. 2007. Novelty of the k-window
  form with immediate re-homing is untested, and a database reader may call it a direct application.
- **That "any L" is dead in every form.** Only the through-the-tree form fails outright. The sums form passes at both widths under
  no ad hoc reads. A D-k with a tree over believed slots might also serve lengths above L0 and was not built.
- **That D-k is tuned to its limit.** Eager re-homing at every refresh was not tried, and neither was a recency log or a coarser
  token scheme. Tuning would lower `min`, so it cannot undo the Go, but it could widen the gap to W.
- **That the baselines are at their limit.** The M1B01 finding stands: nothing here is a lower bound on a scan baseline.
- **Time.** No wall-clock comparison was made.
- **Unplanned windows for D-k.** Its k are fixed at the start, by the tokens. Changing a window needs a rebuild.
- **Per-second exactness at 3,600 s,** and any stream outside these six. email-Eu-core-temporal, the most repeat-heavy, stays
  above 0.67 at both widths.

## Process notes

- The source was fixed before the first stream of this test, and the form choice is in its header. The summariser was changed
  after the runs for reporting only: a redundant column dropped and the break-even table added.
- One process at a time this round: oracle seeds, then 3,600 s, then 1 s. Each run was under ten minutes.
- `results/screen-b-M1B01/` is untouched.
- Rebuild: `scripts/run_window_sweep_b.sh 3600 1`, then `python -I scripts/summarise_window_b.py`.
