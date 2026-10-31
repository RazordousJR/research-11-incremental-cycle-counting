# Gate 0 protocol

Written on 2026-11-01 before any stream was parsed. The thresholds are those of `docs/proposal.md` §7, approved
by the PI (`log.md` 0006). This file fixes the details the proposal left open. Changing it after results exist
is a new log entry and a new gate ID.

## Inputs

Six SNAP files, hashes in `data/snap/SHA256SUMS`: CollegeMsg, email-Eu-core-temporal, sx-mathoverflow,
sx-askubuntu, sx-superuser, wiki-talk-temporal. All carry the label **real timestamps, synthetic expiry**.

## Stream construction

1. Each line `u v t` gives `+uv` at time `t` and `-uv` at time `t + 14 days`.
2. Self-loops are dropped. Direction is dropped. Vertex ids are compacted in first-seen order.
3. Events are sorted by time, stable, with deletions before insertions at equal times.
4. The stream runs under set semantics. Duplicate insertions and deletions of absent edges are no-ops. So an
   edge inserted twice in a window is removed at the first expiry. This is the rule of Hanauer et al. as far as
   their text states it. Check: the update count and peak edge count must agree with their Table 5.1 on
   sx-mathoverflow, sx-askubuntu, sx-superuser and wiki-talk-temporal. A disagreement above 2% is reported.

## Batch cuts, six in all

By time window: 1 hour, 1 day, 7 days. By event count: 100, 1,000, 10,000 events.

## Policies

| Name | Meaning |
|---|---|
| `seq_raw` | Raw events one at a time, lower degree first. True single-update S1 |
| `seq_net` | Net batch, then per-edge scanning. **The gate baseline.** It is `seq_raw` with netting, so it is stricter than S1 |
| `grp_edge` | Batch-degree pivots, per-edge kernel only |
| `grp_noorient` | Batch-degree pivots, kernels A and R, no vertex order |
| `grp_full` | Batch-degree pivots, kernels A and O, with re-ranking. **The gate subject** |

## Operation accounting

Unit costs: one list element scanned, one adjacency probe, one stamp read or write. On top of the kernel costs:

| Cost | Charged to |
|---|---|
| Net batch: 2 per raw event | All except `seq_raw` |
| Grouping: 3 per net update | All group policies |
| Graph update: 2 per applied edit | All policies |
| Oriented list update: 1 per applied edit | `grp_full` |
| Re-rank: `2m + n·log2(n)` | `grp_full` |

## Re-ranking rule

The streams start from an empty graph, so `grp_full` needs an order that follows the graph. Re-rank to the degree
order after every `max(1024, m)` applied edits since the last re-rank, with `m` read at the last re-rank. A vertex
first seen after a re-rank sits below all ranked vertices. No re-rank reads ahead.

## Decision

The score of a stream at one cut is summed `grp_full` operations divided by summed `seq_net` operations.

- **Go:** score at most 0.67 on at least 3 streams, at 2 or more cuts each, and at most 1.05 on every stream at
  every cut.
- **Stop:** score above 0.80 on every stream at every cut.
- **Pivot:** anything else. If the low scores concentrate on batches made of whole-vertex stars, the pivot is the
  short paper on vertex-level updates.

## Exactness

All policies must agree on every batch boundary. A static count checks the result about 200 times per run.
One mismatch stops the gate.
