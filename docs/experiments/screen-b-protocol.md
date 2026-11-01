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

## M1B01-B: the same family against its strongest baseline

Added on 2026-11-01 after M1B01 ran, as a new screen ID. The M1B01 rule had a defect: its Go and Stop clauses can both
hold and it states no precedence, which is what happened. M1B01 stays recorded as undecided by its rule (`log.md` 0012).
This test is stricter than M1B01. It adds a baseline and removes no charge.

**New arms.**
- **D-k:** a per-pair death counter serving k windows. A triple's weakest pair is its oldest pair, so all triples that
  share a weakest pair leave any window together. One counter per pair may therefore serve all k windows, with one expiry
  token per pair per window. Refresh semantics and re-homing as in M1B01.
- **W-F:** method W with a Fenwick tree over B, so any window length up to the horizon can be read. This prices the
  "any L" capability that M1B01 did not build. All Fenwick reads and writes are charged.

**Run.** All six streams, k = 4 with the same windows, slot widths 3,600 s and 1 s. The 1 s width is the literal clock.

**Rule.** Let best = the cheaper of B1 and the better B2 per stream. Precedence is stated: Stop is read first, then Go, and
Pivot is anything else. Each clause is scored at both slot widths and must hold at both.
- Stop: `min(W-F, D-k) / best` above 0.80 on all 6 streams.
- Go: `min(W-F, D-k) / best` at most 0.67 on at least 3 streams.
- Capability check, reported beside the verdict: the "any L" claim survives only if `W-F / D-k` is at most 1.5 on at least
  3 streams. Otherwise the method is the fixed-k form and the capability is dropped.

Exactness as in M1B01, with D-k added to the oracle and to the mutants.

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

## M1C01: the D-k family against the database baselines

Added on 2026-11-01 after scout S8 (`docs/literature/scouts/s8-dk-novelty.md`), before any run of it. M1B01-B stays Go by its
own rule. This test asks a different question: whether D-k holds a place once the database reader's baselines are in, and
whether the only unpublished part, re-homing under refresh, earns anything.

**New arms, added to the M1B01-B harness. Every old arm stays and must reproduce its M1B01-B counts.**
- **V:** a materialized triangle view. Each live triple is stored once, with its current sigma, listed on its three pairs. No
  adjacency scan at expiry or on a refresh: a refresh walks the pair's list, and an expiry walks the list of the expiring pair
  and unlinks from the other two. Count per window through an expiry ring. Every list write, unlink and stored word is charged.
  A new pair still needs its insertion scan.
- **K:** k death counters per pair, no shared counter. Measures what sharing one counter is worth.
- **DI:** the D-k counter with every refresh handled as delete then insert: an exit that scans and an entry that scans, charged,
  with no lazy rule. Measures what the re-home rule saves.

**Run.** All six streams, k = 4 with the same windows, slot widths 3,600 s and 1 s.

**Rule.** Precedence is stated: Stop is read first, then Go, and Pivot is anything else. Each clause must hold at both widths.
An arm dominates D-k on a stream if it has at most the operations and at most the peak words of D-k, with one strictly lower.
The field is W, W-F, V, K, DI, B1 and both B2 forms.
- Stop: some arm dominates D-k on all 6 streams.
- Go: D-k is not dominated on at least 4 of 6 streams, and DI / D-k is at least 1.25 on at least 3 streams.
- Report beside the verdict: K / D-k, V / D-k, peak words per arm, and the share of D-k scans that are re-home scans, measured
  and not inferred.

Exactness as in M1B01-B, with V, K and DI added to the oracle after every event, and at least one mutant per new arm.

## M1C02: exact dynamic 5-cycles, kernels and a kill test

Source: `docs/literature/scouts/s1-expensive-patterns.md`, candidate 1. Added on 2026-11-01, before any run of it.

**Reading comes first.** Try to read Gopher (EuroSys 2026), Cheetah (TACO 2025) and the Hanauer et al. 2026 paper for any exact
dynamic 5-cycle count. If a source already reports exact dynamic 5-cycle counting with these kernels, the test stops there and
that is a Stop. If a source cannot be reached, say so and go on, with the gap recorded as a risk.

**Kernels.** Net cycles through a changed edge (u, v), with A[b] the paths u-a-b avoiding v and C[b] the paths v-c-b avoiding u:
sum over b of A[b] C[b], minus the sum over t in N(u) and N(v) of (d(t) - 2).
- **R2:** two-row scan, per update.
- **WJ:** persistent wedge table W[a, c], sum over N(u) x N(v), with table upkeep charged per update.
- **PR:** shared pivot row for a star group, built once. Exactly-once is telescoping and is not a contribution.
- **RC:** static recount by an O(m alpha^2) counter after Shi, Huang, Shun, charged per batch.
- **HS:** per-batch hindsight best of the above, with no selection cost, as an upper bound on any selector.

**Streams.** CollegeMsg, email-Eu-core-temporal and sx-mathoverflow, insert-only and the two-week expiry rule, at cuts of 100 and
1,000 events. Labels as in Gate 0. Unit costs as in Gate 0, plus 1 per row or table read and write. Wall clock is reported as
indicative only.

**Rule.** Precedence: Stop, then Go, and Pivot is anything else.
- Stop: any exactness failure not traced to a test bug; or a reading finding as above; or one kernel beats every other by more than a
  factor of 2 in operations on all 12 cells; or WJ wins all 12 cells and its peak table is under 1 GB on every stream, which reduces
  the paper to a one-line lemma.
- Go: no Stop, and no single kernel is best on more than 9 of the 12 cells, and HS / best single kernel is at most 0.67 on at least 3 cells.

**Exactness.** Brute-force check on small graphs after every batch, with deletions, repeated pairs in a batch and hubs. The naive
per-edge sum stays as a control and must fail. At least three mutants.

## M1C03: 5-cycle kernels priced on the clock, on all six streams

Added on 2026-11-01 after M1C02, as a new screen ID. M1C02 stays Stop by its own rule. That rule charged a hash probe the same as a
stamp read, while S1's kill test read wall clock. WJ won 12 of 12 in operations and lost 12 of 12 on the indicative clock, and the
three streams left out the hub-heavy case that S1's claim is about. This test asks one question: **once access cost is real, does
batch sharing on a row kernel beat the best non-sharing kernel, including a tuned table?** That is the original project idea on a
pattern where an update is dear.

**Arms.** R2, PR, WJ and PW unchanged from M1C02. New arm **WT**: the same table with a cache-aware layout, written the way a careful
engineer would, for example one small table per first vertex so that a join stays local. It must be exact and is scored on the same
footing. It exists so that sharing does not win against a slow table. RC is dropped: it lost every cell.

**Streams.** All six, insert-only and two-week expiry, cuts of 100 and 1,000 events: 24 cells. Labels as in Gate 0.

**Measure.** Wall clock, one process at a time, median of 3 runs per arm and cell, with counts of table reads and row reads reported beside
it. This is indicative, because the claim is shared, and it decides this screen only. A candidate that passes still needs a pinned
exclusive Gate 1 under hard rule 3. Any arm over 10 minutes or 6 GB on a cell is **not finished** on that cell, which is not a win.
A cell counts only if R2 or PR finished.

**Rule.** Let best = the fastest of R2, WJ and WT on a cell. Score = PR / best. If R2, WJ and WT are all not finished, the cell is not scored and is
listed. Precedence: Stop, then Go, and Pivot is anything else.
- Stop: an exactness failure not traced to a test bug; or fewer than 16 cells scored; or WJ or WT is fastest overall on more than half of the
  scored cells; or the score is above 0.90 on every scored cell.
- Go: the score is at most 0.67 on at least a third of the scored cells, those cells cover at least 3 streams, and at least 2 of them
  are expiry cells.

Report per cell: the winner, the score, PW / best, peak table bytes, and whether the winner changes with hub structure. Exactness as in M1C02,
with WT added to the oracle and at least two mutants for it. Operation counts must match M1C02 for the unchanged arms on its 12 cells.

## Machine rule

One process per test, each run under ten minutes. The coordinator holds the tower claim. Timing, where a test needs
it, is indicative only because the claim is shared.
