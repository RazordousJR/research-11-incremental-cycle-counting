# M1C02 reading addendum, 2026-11-01, after the runs

Written by the coordinator after the M1C02 verdict. `reading.md` stands as written. This adds what a browser with the PI's UTeM
ACM access could read. It changes no threshold and does not rerun the reading Stop. No file was downloaded: pages were read
in the ACM viewer.

## Gopher, EuroSys 2026, full text read through the ACM viewer, by keyword and section

- Single-pattern tests P1 to P6: tailed triangle, diamond, house, winged square, winged pentagon, triangular pyramid. None is the plain
  5-cycle.
- **It does report 5-motif counting (5-MC) on mathoverflow, superuser and wiki-talk (Table 3).** 5-MC finds all 5-vertex patterns, so a
  count of plain 5-cycles is inside that output. Gopher 5-MC: 5,921 s, 6,924 s and 30,292 s. Cheetah, PSMiner and Tesseract are slower
  or over 48 hours.
- Setup: 28 hardware threads, dual Xeon E5-2680v4, default batch size 10K. Enumeration through set operations.
- No mention of a count-only 5-cycle kernel, a net-through-edge formula, a wedge table or a pivot row in what I read.

## Cheetah, TACO 2025, read the same way

- Evaluated on 4-CF, 4-MC, 5-CF and 4-FSM. No 5-cycle and no 5-MC found by keyword. Default batch 100K. Additions and deletions are
  handled separately.

## What this does to the gap

**"No exact dynamic 5-cycle counter" must be narrowed.** A general mining system does produce exact 5-vertex motif counts under updates,
at thousands of seconds on 28 threads. The gap that survives is a **dedicated count-only** exact dynamic 5-cycle kernel, and no
comparison between such a kernel and these systems. The two are not measured on the same task or the same update stream, so no
speed ratio may be quoted from these figures.

## Forward citations and other checks, for S8

- A-Seq is DOI 10.1145/2588555.2593684. ACM showed 14 citing works on its page. Their titles were screened: event processing and
  message ordering work seen, nothing on graphs or triangles found. ACM's list is partial, so this is weak evidence.
- ACM full-text search on exact triangle counting with sliding windows and multiple windows returned 9 hits, screened by title. One
  needs reading: *MWP: Multi-Window Parallel Evaluation of Regular Path Queries on Streaming Graphs*. It is multi-window sharing on
  streaming graphs, for path queries.
- Wang et al., ADBIS 2004: Springer shows the abstract and a purchase box in this browser, so UTeM access does not cover it. Still unread.
