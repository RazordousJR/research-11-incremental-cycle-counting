# Screen B protocol

Written on 2026-11-01 before any Screen B run. Series M1B replaces the plan of `docs/tasks/M1.md` M1A09 to M1A18
(`log.md` 0011). These tests only decide which candidate gets a full gate. They are non-citable. Changing a
threshold after results exist is a new log entry and a new screen ID.

Every test is scored as **Go**, **Pivot** or **Stop** by the rule below. An exactness failure is a Stop until its
cause is found. Unit costs are those of `docs/experiments/gate0-protocol.md`: one list element scanned, one adjacency
probe, one stamp or slot read or write. Graph edits cost 2 each, plus 1 for any extra per-edge field written.

## M1B01: every window length at once, no expiry scan

Source: `docs/literature/scouts/s4-time-and-queries.md`, Candidate 1. Same core as S6 Candidate 1 and S5 M3.

**Semantics.** True sliding window. A pair is in the window of length L at time `now` when its last event time is
above `now - L`. A repeat refreshes the pair. The count is the number of vertex triples with all three pairs in the
window. Expiry here is the definition of the query, so these streams are labelled **real timestamps, window query**.

**Method W.** For a triple, `sigma` is the minimum of its three last-event times. An array B over time slots holds how
many triples have each `sigma`. An event on pair (u, v) with previous time p scans the common neighbours w and moves one
unit to `m = min(last(u,w), last(v,w))`, from `min(p, m)` when p is inside the horizon. Nothing moves when `p >= m`.
Adjacency in recency order lets a repeat scan only entries newer than p. Expiry removes the pair with no intersection.

**Windows.** k = 4: 1 day, 7 days, 30 days, 90 days. k = 1: 14 days.

**Baselines.** B1: k independent lower-degree scan counters. B2: one graph at the longest window, one shared insertion
scan with timestamp filters, one expiry scan per window. **B2 is the score baseline for k = 4.** For k = 1 the score
baseline is the better of a plain scan counter and scan-at-insert with a stored death counter (S6 Candidate 1) under
refresh semantics, with the cost of re-homing charged.

**Rule.** Score = W operations / baseline operations, summed over the stream, all costs inside.
- Go: score at most 0.67 at k = 4 on at least 3 of the 6 streams.
- Stop: score above 0.80 at k = 4 on all 6 streams, or above 1.0 at k = 1 on all 6 streams.
- Pivot: anything else.

Report too: peak memory in words, moved units per event, and how far recency order cuts scans.

## M1B02: count receipts

Source: `docs/literature/scouts/s5-local-and-guarantees.md`, candidate C1. A sum-check audit of a running count.

**States.** Half-stream window of wiki-talk-temporal, end state of sx-askubuntu insert-only, end state of
wiki-talk-temporal insert-only.

**Rule.**
- Stop: any honest run rejected, or any wrong claim or flipped edge accepted, or prover wall clock above 1,000 times
  `static_count` on any state, or sketch time per update above 3 times mean scan time, or the sparse prover lemma
  found published.
- Go: all checks pass, and a line-by-line proof of the prover bound is written with no open step.
- Pivot: all measured checks pass and the proof is not finished.

## M1B03: bounded-memory shared-partner index

Source: `docs/literature/scouts/s3-structured-workloads.md`, Candidate 1.

**Step 1.** On the peak snapshots of sx-superuser and wiki-talk-temporal and one rMAT graph, compute
`E(tau) = sum over d < tau of C(d,2)` against the total. Stop if `E(tau) > 0.5 E(inf)` at `|H| <= 4096`.

**Step 2.** 10^7 TNT proposals on edges plus alternating k-triangle, lambda 2, theta tuned to acceptance 1%, 5% and 15%.
Backends: scan with a bounded memo, eager table, hub split.
- Stop: scan within 1.5 times split at 5% acceptance, or split above 2 times eager at 1% acceptance.
- Go: neither, and split memory at most half of eager on at least one real graph.
- Pivot: anything else.

## Machine rule

One process per test, each run under ten minutes. The coordinator holds the tower claim. Timing, where a test needs
it, is indicative only because the claim is shared.
