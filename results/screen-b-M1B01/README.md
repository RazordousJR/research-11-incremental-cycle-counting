# Screen B test M1B01: every window length at once, run 2026-11-01

Protocol: `docs/experiments/screen-b-protocol.md`, M1B01, unchanged. All six streams carry the label **real timestamps,
window query**. Operation counts, not time. Non-citable. Base commit `eb5472e`, new files uncommitted. Hashes:
`window_sweep.cpp` 69f674cc0ac9, `test_window.cpp` 18d881f1b175, `tc.hpp` d3db4c7dec77 (not edited). g++ 16.2.0, flags
`-std=c++20 -O2 -Wall -Wextra -static`. Longest run 179 s. One CSV per stream, k and slot width, named
`<stream>-k<k>-w<w>.csv`.

## Verdict

**The rule as written does not give one verdict. Its Go clause and its Stop clause are both met, and it states no
precedence.** It cannot be Pivot, which is "anything else".

- **Go clause: met.** Score at most 0.67 at k = 4 on 6 of 6 streams; 3 are needed.
- **Stop clause, k = 4 part: not met.** No stream is above 0.80.
- **Stop clause, k = 1 part: met.** Score above 1.0 at k = 1 on 6 of 6 streams, and the clause reads "or".

The same holds at slot widths 60 s and 1 s (at 1 s the Go clause is met on 5 of 6). I have not picked a reading, because
that would be changing the rule after results exist. The choice is the PI's, as a log entry.

What the two clauses say in plain words: W is 2.4 to 3.0 times cheaper than B2 on five streams when four windows are
kept, but with one window it costs 9% to 157% more than a simple baseline on every stream.

## Scores, primary slot width 3,600 s

Score k = 4 is W over the cheaper of the two B2 forms. Score k = 1 is W over the cheaper of plain and death counter.

| Stream | W k=4 | B2 plain | B2 recency | Score k=4 | W k=1 | Plain | Death | Score k=1 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| CollegeMsg | 1,233,620 | 3,747,584 | 3,713,088 | **0.332** | 975,536 | 738,427 | 691,878 | **1.410** |
| email-Eu-core-temporal | 12,632,231 | 24,108,280 | 20,896,344 | **0.605** | 8,277,059 | 3,222,936 | 4,555,571 | **2.568** |
| sx-mathoverflow | 15,846,783 | 49,951,780 | 37,594,899 | **0.422** | 8,105,676 | 7,421,295 | 7,118,794 | **1.139** |
| sx-askubuntu | 18,263,602 | 61,378,103 | 48,216,258 | **0.379** | 12,199,595 | 11,653,232 | 11,074,125 | **1.102** |
| sx-superuser | 30,271,800 | 104,517,378 | 82,033,601 | **0.369** | 19,257,488 | 19,104,000 | 17,727,731 | **1.086** |
| wiki-talk-temporal | 230,896,968 | 862,420,999 | 559,495,396 | **0.413** | 117,616,803 | 105,010,644 | 97,494,435 | **1.206** |

### Slot width

| Stream | k=4, 3,600 s | k=4, 60 s | k=4, 1 s | k=1, 3,600 s | k=1, 60 s | k=1, 1 s |
|---|---:|---:|---:|---:|---:|---:|
| CollegeMsg | 0.332 | 0.475 | 0.514 | 1.410 | 1.705 | 1.707 |
| email-Eu-core-temporal | 0.605 | 0.663 | 0.677 | 2.568 | 2.723 | 2.672 |
| sx-mathoverflow | 0.422 | 0.476 | 0.480 | 1.139 | 1.211 | 1.214 |
| sx-askubuntu | 0.379 | 0.460 | 0.471 | 1.102 | 1.195 | 1.201 |
| sx-superuser | 0.369 | 0.445 | 0.457 | 1.086 | 1.171 | 1.177 |
| wiki-talk-temporal | 0.413 | 0.435 | 0.474 | 1.206 | 1.342 | 1.367 |

A coarser clock helps W, because a repeat inside one slot moves nothing, while B2 pays almost the same. So 1 s is the hard
case for W, and 1 s is also the literal raw clock.

### How far the k = 4 score can be pushed against W

Slot width 3,600 s. Each column is one change, all against W.

| Stream | W / B1 | W without recency / best B2 | W charged 4 per unit, B2 unchanged | W / idealised B2 | W without recency, k=1 |
|---|---:|---:|---:|---:|---:|
| CollegeMsg | 0.425 | 0.416 | 0.351 | 0.576 | 1.546 |
| email-Eu-core-temporal | 0.982 | 0.748 | 0.671 | 1.434 | 2.613 |
| sx-mathoverflow | 0.477 | 0.466 | 0.477 | 0.647 | 1.062 |
| sx-askubuntu | 0.370 | 0.419 | 0.407 | 0.558 | 1.001 |
| sx-superuser | 0.366 | 0.392 | 0.399 | 0.534 | 0.982 |
| wiki-talk-temporal | 0.478 | 0.597 | 0.442 | 0.629 | 1.272 |

Idealised B2 is not a real method. It is one graph's upkeep (field, edit, queue, count), plus only the scan and probe units
of B1's per-window graphs, with every timestamp read free. W still clears 0.67 on 5 of 6 against it.

Two findings the rule does not ask for, and that matter:
- **B2 is not the strongest k = 4 baseline.** B1 is cheaper than the better B2 on CollegeMsg, email-Eu-core-temporal,
  sx-mathoverflow and wiki-talk-temporal. Against B1, email-Eu-core-temporal is 0.982.
- **The k = 1 Stop clause hangs on the recency form.** W without recency scores 0.982 on sx-superuser at 3,600 s, so
  that stream would fall under 1.0 and the clause would fail. At 1 s W without recency is 1.093 there, so at that width the
  clause holds either way. I fixed W as the recency form before any stream was read, as the protocol names it.

## Definitions fixed before any stream

- **Slot width:** 3,600 s, `slot(t) = floor(t / w)`. Windows are 24, 168, 720 and 2,160 slots, and 336 slots for 14 days, all
  exact multiples. 60 s and 1 s are sensitivity runs. All methods run on the same clock.
- **In-window test:** pair in window L at `now` iff `slot(last) > slot(now) - L`. This is the protocol's "above now - L" on
  the slot clock. At w = 1 it is the protocol's test on raw seconds.
- **Tie rule:** strict. A pair with `slot(last) = now - L` is out. Events in one slot are applied one at a time in stable
  stream order, and every count is the state after that prefix. A repeat in the same slot as the pair's last event
  changes nothing and costs one probe and one read.
- **Horizon:** a pair at or below `now - Lmax` has no record, so its next event is a first arrival. Boundaries move and
  expired records go before the event is applied.
- **Ties at expiry in B2:** a triple with several pairs leaving at one slot is subtracted at the smallest pair id.
- **Score baselines:** k = 4, cheaper of B2 plain and B2 recency per stream. B2 recency uses W's adjacency and pays the same
  upkeep, because a B2 without it would be handicapped on short windows. k = 1, cheaper of plain and death counter.

## Charges

Unit costs are those of the gate 0 protocol. Register compares are free for everyone, including comparing a unit or a
triangle against the k boundaries. Vector size and capacity bookkeeping is free for everyone.

| Item | Cost | W | B2 | B1, plain | Death |
|---|---:|---|---|---|---|
| Pair lookup | 1 | every event | every event | every event, per graph | every event |
| Read last time on a found pair | 1 | yes | yes | yes | yes |
| Degree reads to pick the lower side | 2 per scan | yes | yes | yes | yes |
| List or log element scanned | 1 | yes | yes | yes | yes |
| Probe of the other endpoint | 1 | yes | yes | yes | yes |
| Timestamp read in a scan | 1 each | both ends of each common neighbour | same, these are B2's filters | none needed | believed time, 2 per common neighbour |
| Inline log time read | 1 | each log element | each log element, recency form | no log | no log |
| Superseded-entry check | 1 | each common log element | same, recency form | no log | no log |
| Pair added or removed | 2 | yes | yes | yes, per graph | yes |
| Write last time | 1 | on add and refresh | on add and refresh | on add and refresh | on add and refresh |
| Extra fields | 1 each | none | none | none | believed time and counter on add, believed time on re-home |
| Recency log append | 1 | 2 per non-no-op event | 2, recency form | none | none |
| Log compaction | 1 per entry moved, log2(size) to search | yes | yes, recency form | none | none |
| Slot-log append, read | 1 each | k readers | k readers | one reader per graph | one reader |
| Bucket header read | 1 | longest window only | each window | yes | yes |
| Expiry token pop, push | 2, 1 | one token per pair | one token per pair per window | one per pair per graph | one per pair |
| B read at a boundary | 1 | k per slot | none | none | none |
| B clear at the horizon | 1 | when not zero | none | none | none |
| Unit added | 2 | yes | none | none | none |
| Unit moved | 4 | yes | none | none | none |
| Death counter update | 2 | none | none | none | per triangle born or re-homed; 1 to store what stays |
| Running count write | 1 | per window, per scan or boundary step, when the delta is not zero | same | same | same |

Running counts are written once per step on both sides, so neither side pays per triangle or per unit. The column "W charged 4 per unit" above shows the effect of dropping that choice for W only.

Expiry queue: a pair holds one token per window it is in, re-filed lazily at pop. No queue state is touched on a repeat. I
chose this over a per-event queue because a per-event queue would have charged B2 k reads on every repeat.

Recency order: a per-vertex time-ordered log beside the plain list, with superseded entries left in place. A repeat reads it
backwards to p. If the log is longer than the degree, the scan finishes on the plain list. First arrivals use the plain
list. A doubly linked list would cost about 18 per repeat against 2 here plus compaction.

Re-homing: as S6 Candidate 1 states, at the stale pop. Stale pops of a slot run before true ones.

## Memory, moved units, scans

Slot width 3,600 s. One word is 8 bytes. Counts hash slots at 1.5 words, records, list entries, log entries, tokens, B,
running counts, live slot-log entries and per-vertex and per-bucket vector headers.

| Stream | Peak words W k=4 | B2 plain | B2 recency | W k=1 | Moved per event | Added per event | Scans per event W | Scans per event B2 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| CollegeMsg | 271,799 | 136,029 | 259,270 | 139,134 | 0.056 | 0.228 | 0.621 | 1.429 |
| email-Eu-core-temporal | 406,476 | 85,488 | 238,084 | 112,045 | 0.735 | 0.308 | 0.787 | 1.313 |
| sx-mathoverflow | 685,967 | 211,348 | 593,639 | 468,260 | 0.328 | 1.003 | 0.898 | 3.169 |
| sx-askubuntu | 2,848,260 | 751,225 | 2,634,394 | 2,330,378 | 0.104 | 0.362 | 0.868 | 3.342 |
| sx-superuser | 3,602,134 | 890,338 | 3,315,187 | 2,970,326 | 0.108 | 0.452 | 0.883 | 3.467 |
| wiki-talk-temporal | 22,363,374 | 7,403,800 | 19,278,766 | 14,945,089 | 0.185 | 0.491 | 0.771 | 2.638 |

W uses 2.0 to 4.8 times the memory of B2 plain and 1.1 to 1.7 times B2 recency. The log causes this. At w = 1 on
wiki-talk-temporal, W peaks at 57.1M words, B2 plain at 41.4M and B2 recency at 55.0M, since the rings hold one entry per second.

Moved plus added units reach at most 1.33 per event, so the running-sums form is the honest one on these streams. Slot reads
and writes cost W 4.3% to 9.8% of its total.

## What recency order buys

k = 4, slot width 3,600 s.

| Stream | Events | First arrivals | Repeats | Same-slot no-ops | Elements per repeat, no recency | With recency | Cut | Ops W / W without recency | Plain-list fallbacks per repeat |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| CollegeMsg | 59,835 | 13,928 | 23,221 | 22,686 | 16.9 | 5.6 | 66.7% | 0.799 | 0.055 |
| email-Eu-core-temporal | 332,334 | 23,975 | 237,682 | 70,677 | 21.7 | 9.8 | 54.6% | 0.808 | 0.118 |
| sx-mathoverflow | 390,441 | 219,384 | 131,391 | 39,666 | 23.3 | 9.7 | 58.5% | 0.905 | 0.051 |
| sx-askubuntu | 726,661 | 465,775 | 165,205 | 95,681 | 20.1 | 6.7 | 66.4% | 0.904 | 0.018 |
| sx-superuser | 1,108,739 | 737,900 | 241,176 | 129,663 | 18.9 | 7.0 | 63.1% | 0.941 | 0.017 |
| wiki-talk-temporal | 6,100,538 | 2,893,837 | 1,808,220 | 1,398,481 | 50.2 | 14.0 | 72.2% | 0.691 | 0.048 |

Recency cuts list elements on repeats by 55% to 72%, but total operations by only 6% to 31%, since its upkeep is charged. At
k = 1 it does not pay on three streams: W costs more than W without recency on sx-mathoverflow, sx-askubuntu and sx-superuser.

## Exactness

- **Oracle:** `tests/test_window.cpp`, 200,000 random trials at seed 7 and again at seed 11, PASS both. Seed 7 ran 14.9M events
  and 37.2M window checks after every event, against a brute-force recount of all triples: 0 failures for W, W without
  recency, B2 both forms, B1, plain and death counter. Array B matched the sigma histogram slot by slot, 0 failures.
- **Coverage, seed 7:** 6.7M equal-slot events, 11.3M repeats, 3.5M re-entries after the horizon, 370K jumps past the horizon,
  1.4M units moved, 1.0M units re-homed, 163K log fallbacks in W. Half the trials are k = 1.
- **Controls that must fail, did:** plain add in place of the move rule was wrong on 157,017 of 200,000 trials. W read
  against the `>=` tie rule differed on 171,344 of 200,000.
- **Mutants, 3,000 trials each:** removing B2's expiry tie rule, running true pops before stale ones in the death counter,
  dropping the superseded check, a loose fallback bound and a loose move test each failed. A one-off boundary hung the oracle
  on a missing key, which counts as caught. Moving when m = p passed, correctly, because that move is a no-op.
- **Real streams:** W, W without recency and the baselines agreed on every window after every event on all six streams, at all
  three slot widths. Static counts with `r11::static_count`, on graphs rebuilt from an independent map of last times, gave 0
  mismatches at about 1,000 checkpoints per window: 24,112 checks at k = 4 and 6,028 at k = 1, per slot width.
- No cause of disagreement arose, so nothing had to be stopped.

## What this does not establish

- **That W beats the best baseline at k = 4.** B2 is what the rule names, and B1 already beats it on four streams. More
  important, the death counter alone beats W at k = 1 on all six streams. A death counter for k windows was
  not in the protocol and was not run. My unchecked expectation is that one counter per pair may serve all windows, since every
  triple on a pair has the same sigma. That baseline could erase the k = 4 gain, and it should be tested before any full gate.
- **Any window length.** Only k fixed running sums were measured. The Fenwick form, which is the "any L" capability, was not
  built, and its 2 log2(L) per unit would raise W.
- **Time.** No wall-clock comparison was made. The log may help or hurt cache behaviour.
- **Per-second exactness at the primary width.** At 3,600 s the counts are exact on the hour clock, and boundaries move on the
  hour. The 1 s runs are the literal case.
- **Other streams.** Six SNAP streams only. email-Eu-core-temporal, the densest in repeats, is the weakest for W.
- **Novelty or usefulness.** Nothing here tests either.

## Process notes

- The C++ was not changed after the first stream was read. The charges above are those of that first build.
- `data/derived/*.events.bin` come from `scripts/prepare_window_stream.py` run with `python -I`, and match the event order of
  the `.ins.bin` files on all six streams. Input hashes passed against `data/snap/SHA256SUMS`.
- One slip against the one-process rule: the long oracle job and the Python stream preparation overlapped, and a hung mutant
  ran for several minutes until I killed it. No score run overlapped another score run from this test.
- Rebuild: `scripts/run_window_sweep.sh 3600 60 1`, then `python -I scripts/summarise_window.py 3600`.
