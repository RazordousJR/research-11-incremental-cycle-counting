# CLAUDE.md: Research 11, Local Incremental Triangle Counting in Dynamic Graphs

This is the project constitution. `NEW-RESEARCH-BOOTSTRAP.md` in this folder supplies every rule not
restated here. Where the two conflict, this file wins and the conflict goes in `log.md`.

## Identity

- **Project:** Research 11. The source idea file is headed "Research 5", and the PI confirmed on 2026-11-01
  that the project is Research 11 (`log.md` 0006).
- **Field:** graph algorithms. Exact, sequential, CPU only.
- **Contribution type (bootstrap §3):** Method, with one small theory result. Not yet earned: see
  `docs/proposal.md` §7 and `docs/novelty/paper-1.md`.
- **Corresponding author:** Hamzah Asyrani Sulaiman (the PI). Six authors, listed in `AUTHORS.md`. Working
  mode is the PI plus automated roles. Decisions go to the PI alone, and co-authors are not a review step.

## Working language

Chat is informal Malay with *aku* and *ko*. Repository files are English. Every open item closes a turn in
the §1 decision table with a recommendation.

## Which bootstrap rows apply

| Rule | Status here | Reason |
|---|---|---|
| §3 win condition, strongest simple baseline, §5 test 3 | Applies | Method paper |
| §5 coverage matrix, static and dynamic row | Applies | The input is a time-varying graph by definition |
| §6 rule 9 portability | Not used | No portability constraint |
| §13 host rendering | Applies | No in-app Browser pane and no HTML Artifact on this machine |
| §14 playable builds | Exempt | The output is a command-line counter with nothing to play |
| §16 two figures "rendered in operation" | Applies, as read here | One worked batch drawn from a logged run, and one stream timeline (PI decision, 2026-11-01, `log.md` 0018) |

`CLAUDE_MAIN.md` belongs to the GPU portfolio and does **not** apply here (PI decision, 2026-11-01, `log.md`
0006). Its experiment discipline is already covered by bootstrap §6.

## Hard rules specific to this project

1. **Exact means exact.** Every engine policy matches the O(n^3) reference on every small test batch. A
   single mismatch stops all timing work. No sampling and no Bloom filter in the count path.
2. **The control must fail.** `tests/test_exact.cpp` carries a naive per-edge sum that double-counts. If
   that control ever passes, the test is broken.
3. **Operations first, time second.** Unit-cost operation counts are deterministic and machine-independent,
   so they decide Gate 0. Wall-clock time decides Gate 1 only, and only from a pinned, exclusive run.
4. **Every stream is labelled** real timestamps, real timestamps with a synthetic expiry rule, or fully
   synthetic. The label appears in every table. A synthetic stream never supports a claim on its own.
5. **No future leakage.** The vertex order uses only the graph at the epoch start. The batch policy sees
   only the current batch. Parameters are chosen on engineering streams and evaluated on other streams.
6. **The exactly-once proof, heavy-light partitioning and wedge indexes are not claimed as contributions.**
   See `docs/novelty/paper-1.md`.
7. **All costs count:** graph update, vertex-order upkeep and estimator reads are inside every total.

## Machine sharing

Registered in `C:/Develop/tower` as `research11` (`log.md` 0007). Heavy work runs only under a tower claim, and
the claim is released between tests. Heavy means worker processes, more than ten minutes, anything under Task
Scheduler, or any chain, sweep or overnight queue.

ControlPC is the tower queue coordinator (PI decision, 2026-11-01, `log.md` 0018). Before heavy work, ask it for GO or
WAIT with the planned claim hours, and declare real threads. Any `--exclusive` timing goes to ControlPC first and needs
the PI's word as well. A single process under a minute with no timing can run directly. ControlPC's GO settles the queue
only: it is not a PI decision.
