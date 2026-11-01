# Candidate ranking after Screen B and Screen C, 2026-11-01

**Update, same day, after the PI picked candidate 1 (`log.md` 0018, 0019).** The reading in `docs/literature/r10-falsifier-reading.md` weakens candidate 1. Cheetah already publishes star grouping with per-group reuse, and the trend of low gain on
triangles and more on larger patterns. PR as a mechanism falls to 3 of 10. The table below stands as written on the evidence of that morning, and its "Why candidate 1 leads" must be read with this note: what is left is the gain against the
strongest baseline with closed-form kernels and all costs inside. The honest ceiling does not rise, and the chance of reaching it falls.

Serves M1B04. This ranks candidates for a full gate. It claims no contribution. `docs/novelty/paper-1.md` stays the record of the proposal
refuted at Gate 0. The PI decides the pick. Screen results are non-citable. Literature scores are scout-grade, and every "not found" is weak.

## Ranking

| Rank | Candidate | Screen evidence | Prior art | Real user found | Honest size |
|---|---|---|---|---|---|
| 1 | **When batch sharing pays, across pattern size.** Triangles fail, 5-cycles pay on the clock, tables lead on unit costs | Gate 0 Stop, M1C02 Stop on operations, M1C03 Go on the clock by one cell | Partly taken, novelty 4 of 10 (S10). Row sharing is published for triangles and butterflies, telescoping for any pattern | None for a global 5-cycle count under batches. ERGM toggles one dyad, so PR equals R2 there | One experimental paper, SEA or ALENEX level, if Gate 1 holds |
| 2 | **D-k, one death counter across k windows under refresh** | M1B01-B Go, M1C01 Go, thin at 1 s against V and W | Partly taken (S8). Only re-homing under refresh with k windows was not found, and ADBIS 2004 is unread | Not established | Short paper. Its speed gain comes from a published principle |
| 3 | **Sum-check count receipts** | M1B02 weak Go on one reading, Stop on the other | Folklore in substance, novelty 4 of 10 (S9) | None | Note, with open questions. Parked |
| 4 | **Bounded-memory shared-partner index for ERGM** | M1B03 Stop on the plain reading | Not scored by a scout here. Theta was tuned and not fitted, so this is not ERGM practice | Exists in principle | Needs a new screen ID. Parked |

Still parked from the scouts: the I/O bound (S2), the exponent map (S6), the Euler signature (S7). The 4-cycle hub-heavy case (S1) and dedicated counters against mining
systems (S1) fold into candidate 1 if it runs.

## Why candidate 1 leads, and what is weak about it

**For.** It is the only candidate with a question a reader would still care about if the mechanism were already known: at what pattern size does sharing inside a batch start to
pay, and why does the unit-cost winner lose on the clock? Gate 0 gives the failing end of that axis, with 5.9 to 13.9 operations per triangle update. M1C03 gives the other end, with
about 7,660 operations per 5-cycle update on wiki-talk expiry and 123,800 on sx-superuser insert, on the same streams, the same model and the same code base. Negative and positive results are both usable, which a mechanism
paper cannot say. WJ costs 3 to 13 ns per charged operation against 0.3 to 1 ns for the row arms, and that gap is itself a finding about unit-cost models.

**Against.** Novelty 4. No application. The Go has a margin of one cell, and it is on a shared clock. Operations favour tables on 16 of 22 cells, so a hostile reader can say the clock was
chosen after operations stopped. M1C03's protocol predates its run, but M1C02's Stop stands and must be reported beside it. Three baselines are unbuilt: an HHH-style table with hubs
scanned, affected-subgraph recount, and a BFCB-IG port. Any of them could erase the gain. Cheetah, Gopher and TODS mechanisms are unread, and one of them could fire the falsifier.

## What must happen before a full gate

1. A person, or the PI's Chrome with the PI's word, reads the mechanism sections of Cheetah, Gopher and TODS 46(2). S10 puts this at about an hour. A hit shrinks the claim to the study.
2. A new protocol, written before any run, that adds the three unbuilt baselines and the 4-cycle row, with engineering streams kept apart under hard rule 5.
3. ControlPC's GO, and the PI's word for `--exclusive` timing.

## What no candidate offers

No candidate here is a large contribution. The honest ceiling on current evidence is one solid experimental paper at an algorithm engineering venue. A stronger venue would need a
real user, which S9 and S10 did not find for candidates 3 and 1.
