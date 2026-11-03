# M1D07: more streams for Gate 1, at p = 3 and p = 5

Written on 2026-11-03, on the PI's word (`log.md` 0040), before any new stream is downloaded or read. The selection rule below was committed before any size, count or page of a candidate stream was looked up. The stream list and the hours come after it, by that rule, in a later commit. Thresholds are Gate 1's and do not change (`docs/experiments/gate1-protocol.md`). A change is a new ID and a `log.md` entry.

## Why

The outline's readiness check fails §5 test 5: three evaluation streams and one held-out, against 56 instances in Hanauer et al., and no stream with real deletions. The one unseen stream gave no pass. Four streams are too few to carry a rule for when sharing pays. M1D07 asks whether M1D05's result holds on streams that no arm, no parameter and no reading of this project has touched.

## What it can and cannot do

M1D05's verdict was given by its rule and does not reopen, whatever M1D07 finds. M1D07 is read beside it, pooled, by outcomes fixed below. It can narrow the paper's claim or turn it. It cannot widen it past what the pooled cells show.

## Selection rule

Applied in this order. Each step is mechanical, and every dropped instance is listed with its step.

1. **Universe.** The 56 instances of Hanauer et al.'s full set, exactly as `exp/scripts/get_data.sh` of AlgorithmsInAction/DynamicSubgraphCounter lists them on its `main` branch, read on 2026-11-03: 38 DyReach Kronecker streams (19 graphs, constant and growing size), 6 KONECT Wikipedia link streams, 5 SNAP temporal files, 2 SNAP Reddit TSV files and 5 Network Repository files. No instance from outside that list.
2. **Label.** The 38 Kronecker streams are fully synthetic. Hard rule 4 says a synthetic stream never supports a claim on its own, so they are out of M1D07. They stay available as a reported hard case later, on the PI's word.
3. **Already used.** sx-askubuntu, sx-superuser and wiki-talk-temporal are evaluation streams here, sx-stackoverflow is the held-out stream, and sx-mathoverflow is an engineering stream. All five are out: they are not new.
4. **Source.** Each remaining instance is fetched only from the URL that script names, at konect.cc, snap.stanford.edu or nrvis.com, and only on the PI's word with file name, source and size. No mirror and no substitute. An instance whose URL fails, or whose file holds no stream the format check of step 6 accepts, is dropped and recorded, and nothing replaces it.
5. **Size.** A file over 2 GB as served is dropped, decided by the server's size header before download. Every derived stream is cut to its first 2,000,000 events if it is longer, which is Hanauer et al.'s own `2m` prefix. The cut is by event count, fixed now, and it reads nothing in the stream but its length. A stream shorter than that runs whole.
6. **Format check.** Before any arm reads it, each file goes through the loader and its check: the columns parse on every data line, the times are integers or the documented date format, the vertex count and event count are printed, and for KONECT every weight is +1 or -1. A file that fails is dropped by step 4. A Network Repository file whose weights carry its own deletions takes the KONECT label below instead of the synthetic expiry one, and that is decided by the check, not by any count.

## Streams, by the rule

Applied after the rule above was committed (ea5f5d9), on the servers' size headers read on 2026-11-03 and nothing else. Steps 2 and 3 drop 43 of 56. Step 5 drops none: the largest file is 437.6 MB. 13 remain, none downloaded.

| # | Stream | File | Source | Bytes as served | Label | Cells per p |
|---:|---|---|---|---:|---|---:|
| 1 | simplewiki | `download.tsv.link-dynamic-simplewiki.tar.bz2` | `http://konect.cc/files/` | 7,226,642 | real timestamps, real deletions | 2 |
| 2 | nlwiki | `download.tsv.link-dynamic-nlwiki.tar.bz2` | `http://konect.cc/files/` | 107,470,446 | real timestamps, real deletions | 2 |
| 3 | plwiki | `download.tsv.link-dynamic-plwiki.tar.bz2` | `http://konect.cc/files/` | 125,747,866 | real timestamps, real deletions | 2 |
| 4 | itwiki | `download.tsv.link-dynamic-itwiki.tar.bz2` | `http://konect.cc/files/` | 176,843,227 | real timestamps, real deletions | 2 |
| 5 | frwiki | `download.tsv.link-dynamic-frwiki.tar.bz2` | `http://konect.cc/files/` | 294,197,388 | real timestamps, real deletions | 2 |
| 6 | dewiki | `download.tsv.link-dynamic-dewiki.tar.bz2` | `http://konect.cc/files/` | 437,616,884 | real timestamps, real deletions | 2 |
| 7 | soc-redditHyperlinks-body | `soc-redditHyperlinks-body.tsv` | `https://snap.stanford.edu/data/` | 318,931,394 | real timestamps, synthetic expiry; insert-only | 4 |
| 8 | soc-redditHyperlinks-title | `soc-redditHyperlinks-title.tsv` | `https://snap.stanford.edu/data/` | 368,581,209 | real timestamps, synthetic expiry; insert-only | 4 |
| 9 | ia-enron-email-dynamic | `ia-enron-email-dynamic.zip` | `https://nrvis.com/download/data/dynamic/` | 4,415,186 | real timestamps, synthetic expiry; insert-only (M1D07-R1) | 4 |
| 10 | ia-reality-call | `ia-reality-call.zip` | `https://nrvis.com/download/data/dynamic/` | 389,600 | real timestamps, synthetic expiry; insert-only (M1D07-R1) | 4 |
| 11 | soc-wiki-elec | `soc-wiki-elec.zip` | `https://nrvis.com/download/data/dynamic/` | 633,985 | real timestamps, synthetic expiry; insert-only (M1D07-R1) | 4 |
| 12 | tech-as-topology | `tech-as-topology.zip` | `https://nrvis.com/download/data/dynamic/` | 1,093,293 | real timestamps, synthetic expiry; insert-only (M1D07-R1) | 4 |
| 13 | soc-youtube-growth | `soc-youtube-growth.zip` | `https://nrvis.com/download/data/dynamic/` | 59,019,201 | real timestamps, synthetic expiry; insert-only (M1D07-R1) | 4 |

1,902,166,321 bytes in all, 1.90 GB. The archives are read in place by the loader, so nothing is unpacked to disk. If every Network Repository file takes the synthetic labels, there are 40 cells at each of p = 3 and p = 5: 12 real-deletion cells, 14 synthetic expiry and 14 insert-only. With M1D05's, p = 5 then has 56 evaluation cells on 17 streams.

Dropped at step 2, fully synthetic: the 19 DyReach Kronecker graphs, constant and growing size, 38 instances. Dropped at step 3, already used: sx-askubuntu, sx-superuser, wiki-talk-temporal, sx-stackoverflow, sx-mathoverflow.

KONECT's page gives simplewiki 1,627,472 events before cleaning and nlwiki 20,070,561, so simplewiki runs whole and the other five are cut at 2,000,000. Both are directed with a top degree of 33,633 and 108,306.

### Reading M1D07-R1, fixed on 2026-11-03 after the format check and before any stream was derived or any arm ran (`log.md` 0044)

Step 6 assumed every four-column Network Repository file reads "u v w t", and that weights of +1 and -1 mean deletions. The check found neither true. `ia-reality-call` reads "u,v,t,duration", so the rule would have taken the call's duration as its time. `soc-wiki-elec` is headed `% asym signed`: its -1 is an oppose vote, not a removal, so the rule would have deleted an edge for every oppose vote. The readme in each zip documents no columns. So, mechanically:

- A Network Repository file is a real-deletion stream only if its first comment line is KONECT's `% <structure> dynamic`. None of the five is, so all five take the two synthetic labels, and their weight column is ignored, as Hanauer et al.'s `temporal_to_updates` ignores it.
- With four columns, the time is whichever of columns 3 and 4 has every value at least 10^8, Unix seconds after March 1973. If neither or both qualify, the file fails step 6 and is dropped.

**Accepted by the PI on 2026-11-03 (`log.md` 0048).** This changes which column a file is read from, not any threshold, cell, arm or reading of a result. Checked on the four small files: time in column 3 for `ia-reality-call` and in column 4 for the other three. `soc-youtube-growth` is checked when it is derived. `ia-enron-email-dynamic` has times from 1980 to 2024, the known bad dates of the Enron mail. No filter was fixed before this check, so none is added, and the stream runs as served.

### soc-youtube-growth, decided by the PI on 2026-11-03 (`log.md` 0047, 0048)

Its first 2,000,000 events all carry one time, 2006-12-10: the file opens with a dated snapshot. Its expiry and insert streams are then the same file byte for byte. One copy runs, the insert stream, labelled **static snapshot, file order**. It is reported, and never in a verdict or a pooled reading. The expiry copy is held out in `data/m1d07/held/`. Its p = 3 cells had already run when this was found, and they are kept and not used.

## Labels and variants

| Source | Label | Variants |
|---|---|---|
| KONECT `link-dynamic-*wiki` | **real timestamps, real deletions** | One: the stream as the file gives it |
| SNAP Reddit TSV, Network Repository | **real timestamps, synthetic expiry** and **real timestamps, insert-only** | Two, built exactly as `scripts/prepare_stream.py` builds them: two-week expiry, deletions before insertions at equal times |

The KONECT stream is cleaned as Hanauer et al.'s `graph_cleaner` cleans it: self-loops dropped, direction dropped, events stable-sorted by time with file order kept inside a time, and only events that change the undirected edge's state are kept. So its first 2,000,000 events are their `2m` instance up to vertex names. No expiry is added, and no event is reordered by kind, since its deletions are real.

**A stated difference.** Hanauer et al. give SNAP and Network Repository streams a lifetime of 14 days that a repeat interaction extends. This project's expiry, fixed at Gate 0, removes an edge 14 days after its first interaction inside its life and does not extend it. M1D07 keeps this project's rule so that its cells read with M1D05's. The paper states the difference.

## Cells and arms

Cuts of 1,000 and 100 events, as Gate 1. Per stream: two cells for a KONECT stream, four for any other.

| p | Arms | Parameter |
|---|---|---|
| 5 | R2, PR, WT, WJ, M5, A5 | M5 at theta 128, chosen on the engineering cells in M1D04 |
| 3 | S3, P3, T3, M3 | M3 at theta 16, chosen on the engineering cells in M1D04 |

p = 4 is not run, by the PI's word. Theta is not chosen again: every new stream is an evaluation stream, and none is used to choose anything. The binaries are M1D04's, `build/gate1/count.exe` and `build/gate1/timing.exe`, with the hashes in `results/gate1-M1D04/hashes.txt`. A rebuild that changes a hash stops M1D07 until the PI decides.

## Measure

As Gate 1, unchanged: timing build, one process at a time, pinned to logical CPU 8, an `--exclusive` claim, five repeats on a cell whose fastest arm takes under 60 s in repeat 1 and three otherwise, an arm past 5 times the fastest or not finished run once, arms interleaved in a rotated order. Limits 1,200 s and 16 GB per process, A5 at 300 s. Reading M1D04-R1 stands: a time-stopped arm enters "best" at its limit, and one stopped by memory is out of that cell.

Every arm's per-batch count must match every other arm's in that pattern and cell, in the count sweep and in every timing repeat. A single mismatch stops the sweep, the claim is released and the PI and ControlPC are told. No restart (hard rule 1).

## Order of work

| ID | Step | Machine |
|---|---|---|
| M1D07A | This protocol, the loader `scripts/prepare_m1d07_stream.py` and its oracle test `tests/test_m1d07_loader.py`, before any download | No claim |
| M1D07B | Downloads, on the PI's word with file, source and size, into `data/m1d07/raw/`, with `SHA256SUMS`. Then the format check and the derived streams in `data/m1d07/derived/` | Downloads need no claim. Deriving the largest archives may pass ten minutes, so it runs at the start of M1D07C's claim |
| M1D07C | Count sweep on every new cell, all arms, per-batch agreement, by `scripts/run_m1d07.sh`, written and dry-run on an engineering stream first. Operations first (hard rule 3), and the source of the timing plan. Its seconds are not timing and are never quoted as time | Ordinary claim, ControlPC's GO |
| M1D07D | Timing sweep, in phases of at most 8 hours each | `--exclusive`, the PI's hours, then ControlPC's GO |
| M1D07E | Reading, below | No claim |

**The timing phases, fixed now, by expected cost and not by any result.** D1: every p = 3 cell, and every p = 5 cell of the streams under 1,000,000 events. D2: simplewiki, nlwiki, plwiki at p = 5. D3: itwiki, frwiki, dewiki at p = 5. D4: the p = 5 cells of the other streams at or over 1,000,000 events. M1D07C's seconds may repack these into claims of at most 8 hours, keeping the order. The PI fixes how many phases run before D1. A phase is never dropped after a timing result of M1D07 is read, since that would be the free option `log.md` 0024 warned of.

The M count runs of M1D07C are the count build that `mcheck` compares with M's timing build, so no count build runs inside an exclusive claim.

## Hours, before any run

From M1D03's count seconds and M1D04's timing seconds on the same cells, the count build runs at 1.0 to 1.1 times the timing build, so M1D07C costs about one repeat of everything. Cost at p = 5 is in the streams that grow: sx-superuser insert, 1.1 million events, took about 22 minutes a cell for all arms and repeats, and wiki-talk insert, 6.1 million, about 2 hours. Time grew as about the 2.2 power of events between them. A 2,000,000-event growing prefix then costs about 1 hour a cell, and an expiry cell minutes. The limits bound the worst case at about 1.75 hours a cell. The growing streams are the 6 KONECT prefixes and about 2.5 of the others, insert-only.

| Step | Claim | Low | Central | High |
|---|---|---:|---:|---:|
| M1D07B, derive the streams | Ordinary, inside M1D07C's first claim | 0.2 h | 0.3 h | 0.5 h |
| M1D07C, count sweep | Ordinary, ControlPC's GO | 6 h | 10 h | 30 h |
| M1D07D, timing | `--exclusive`, the PI's hours and ControlPC's GO | 10 h | 19 h | 35 h |

The range is wide because the 5-cycle cost of a Wikipedia link prefix is not known before M1D07C. M1D07C fixes it: its seconds, through the repeat rules of `scripts/gate1_plan.py`, give the timing plan to within the clock's spread before any exclusive claim is asked for.

## Reading, fixed now

Every figure is reported by label. The pass test, "scored" and "best" are Gate 1's, unchanged: a cell passes when share's slowest repeat is at most 0.67 times best's fastest.

1. **M1D07 alone, at p = 5.** Scored, passed, on how many streams, and by label.
2. **Pooled, at p = 5.** M1D05's 14 scored cells and M1D07's scored cells together, under Gate 1's Go clause read literally: at least a third of the scored cells pass, on at least 2 streams, and at least 2 of the passes are synthetic expiry cells. The same clause is reported a second time with real-deletion cells counted beside expiry cells, marked as a second reading.
3. **Trend, pooled.** The median score at p = 3 at least 1.0 and at p = 5 at most 0.90, over M1D05's and M1D07's scored cells together.

What each outcome supports, fixed before any new stream is read:

| Outcome | When | What the paper says |
|---|---|---|
| **Holds** | The pooled Go clause and the pooled trend both hold | The claim sentence of `docs/paper/outline.md` stands, with the stream count and labels stated |
| **Narrows** | Either fails, and M1D07 has at least one pass at p = 5 | The claim is restricted to the labels and sizes where cells pass, by name. The outline's claim sentence is rewritten before any prose |
| **Does not carry** | M1D07 has no pass at p = 5 | M1D05's Go is reported as holding on 3 streams of this project and not on the new ones. Whether that is a paper, and what kind, goes to the PI |
| **Stop** | An exactness failure not traced to a test bug | As Gate 1 |

Read beside it, whatever the outcome: the real-deletion cells as their own row, which are the hard case; the share of deletions among each KONECT prefix's events; the median score against the scan alone; nanoseconds per charged operation per arm; every cell where operations and the clock name a different winner; and drift of PR and R2 against M1D04 on nothing, since no old cell is rerun.

## What M1D07 cannot show

One machine and one core still. A second machine is the test of the scan margin and the cache claim, and stays open (`log.md` 0038). A 2,000,000-event prefix is the early part of each stream, and a Wikipedia link stream's early part may hold few deletions. The prefix is fixed now, and the deletion share is reported so that a reader can see it.
