# log.md: append-only decision log

A correction is a new entry that references the old one. Entries are never edited.

## 0001, 2026-11-01: project opened

The PI supplied the idea file, the prompt and six authors, held in `AUTHORS.md`, with Hamzah Asyrani Sulaiman
as corresponding author. The idea file is headed "Research 5" and the folder is Research 11. Treated as the
same numbering slip as Research 10, pending the PI's word. The starter `STATUS.md` said to replace itself, and
it was replaced. Audit cut-off set to 1 November 2026, later than the prompt's 7 October, because that covers
the earlier date in full.

## 0002, 2026-11-01: M1A01 literature audit finished, idea narrowed

Ended in evidence: `docs/literature/audit-2026-11-01.md`. Closest work is Hanauer et al., arXiv:2610.29776,
posted 21 October 2026, after the prompt's cut-off. It reports plain scanning ahead of every auxiliary
structure for triangles on single-edge updates.

Decision: reject affected-neighbourhood indexing as the contribution. Reject the exactly-once proof as a
contribution, since Makkar 2017, Dhulipala 2021 and delta queries all have it, but keep it because the PI asked
for it. Narrow to work-sharing between the updates of a batch. Sequential scanning replaces full recomputation
as the strongest simple baseline.

Rejected: building a wedge or heavy-light index anyway. That would rebuild HHH and EGST against a paper with
56 instances and public code.

DBLP was not screened, because its API sits behind a bot check. EDTC and DYTC were read as abstracts only.

## 0003, 2026-11-01: M1A02 constitution, proposal and ledger written

Records: `CLAUDE.md`, `docs/proposal.md`, `docs/novelty/paper-1.md`, `docs/tasks/M1.md`, `docs/risks.md`,
`docs/experiments/coverage-matrix.md`. Gate thresholds in proposal §7 were written before any real stream was
read. They wait for the PI's approval.

No technical run on real data was started, because bootstrap §11 asks for the PI's answers first and because
downloading datasets needs the PI's word. The small test and the synthetic probe below were run because the
PI asked for a proof and for a verdict on narrowing, and neither touches external data.

## 0004, 2026-11-01: M1A03 and M1A04 done; prototype, proof test and probe P0

Code: `src/r11/tc.hpp`, `tests/test_exact.cpp`, `benchmarks/probe_ops.cpp`. Exactness on 200,000 random mixed
batches, seed 7: 0 failures. The control double-counts as it must: wrong on 12,950 of 26,929 insert-only
batches. `tc.hpp` prefix `e99fcfb05ae3`.

Probe P0, three seeds, non-citable: gain on arrivals and departures, none on uniform or biased updates, and
gate overhead up to 1.207 on biased inserts. Kernel R was never chosen under the batch-degree pivot.

Defect in my own first code: the first run printed FAIL because I had required the adaptive policy to use
kernel R and it never does. The counts were correct. The check was wrong, and it was removed.

## 0005, 2026-11-01: gate tightened, probe P0B

Changed the gate in `tc.hpp`: the floor now adds an estimate of the R term, and the adaptive policy offers
only kernels A and O. Overhead on uniform and biased updates fell to at most 1.029. Gain on bursts held:
arrivals 0.457 to 0.817, departures 0.590 to 0.765. Clique insertion stayed near 1. Exactness repeated at seed
11: 0 failures. `tc.hpp` prefix `3dc3a6560635`. P0 is kept and not overwritten.

What this does not establish: anything about a real stream. Records: `results/probes/P0-20261101/README.md`.

## 0006, 2026-11-01: PI approvals

The PI wrote "Okeh approved beb. Ko download semua gune chrome aku. Proceed." to the open table. Approved: the
six SNAP files, downloaded through the PI's Chrome; the thresholds in `docs/proposal.md` §7 as written, before
any real stream was read; the name Research 11; `CLAUDE_MAIN.md` not applied; registering `research11` in the
tower; `git init` here; self-funded as in Research 9.

Rejected: `sx-stackoverflow` at this stage, which was not in the approved list of six.

## 0007, 2026-11-01: M1A19 and M1A20 done; tower, data and git

Six SNAP files were fetched through the PI's Chrome, one tab, closed afterwards. Sizes match the server's headers
to the byte. They were moved from Downloads to `data/snap/` and hashed in `SHA256SUMS`. `data/` is not committed.

`research11` is registered in the tower: one `PROJECTS` entry, no logic touched, 33 of 33 tests before and after,
notice filed. `git init` done, with commits under the PI's name, because run IDs need a revision. Starter
spreadsheets, documents and idea files are ignored.

## 0008, 2026-11-01: M1A07 Gate 0 run; verdict STOP by the written rule

Protocol written and committed before any stream was parsed. Results: `results/gate0-20261101-d2efe26/`, commit
`d2efe26`. Tower queue respected: run started when `--shared` allowed it, one process, claimed and released.

All 36 cells between 1.317 and 2.438 against a stop line of 0.80 and a go line of 0.67. Exactness held: 0
mismatches, 7,232 static checks. Cause: scanning costs 5.9 to 13.9 operations per net update at the 1,000-event
cut, and fewer than 9% of net updates fell in kernel O groups. Per bootstrap §4, this is a completed task.

## 0009, 2026-11-01: M1A21 hindsight diagnostic; the stop is not an implementation artifact

Run after the verdict, to test whether my accounting or gate caused the stop. With free selection and no overhead
the best ratio was 0.872 with expiry and 0.946 insert-only. Record: `results/diag-oracle-20261101-e267b9f/`.

Rejected: tuning the gate, dropping the charges or changing the thresholds. Any of those would rewrite a verdict
that was fixed in advance, and the 13% hindsight cap shows none of them could reach 0.67.

M1A05 was left partial. ParaDyMS works per updated edge according to a poster by its authors. TODS and EDTC are
closed access. Not pursued, because the stop makes the question moot.

Not covered: Wikipedia link streams and sx-stackoverflow. Whether to test them or close is the PI's decision.

## 0010, 2026-11-01: PI keeps the project open and widens the search

The PI wrote "Jgn tutup. Okeh kaji dari segi aspek sumbangan lain. Anta agents. Luaskan skop and cari and gali
lagi. Proceed. Jgn terhadkan" to the closing table. My recommendation to close was rejected.

The proposal of `docs/proposal.md` stays refuted on the six streams. Its Gate 0 record is not reopened. The search
now covers other contribution types, patterns, cost models, query models and application fields. Seven literature
scouts were sent, writing to `docs/literature/scouts/`. Every candidate must say why it escapes the cause of the
Gate 0 failure, which was that updates cost too little for sharing to matter.

A candidate that replaces the plan starts a new task series, per bootstrap §4.

## 0011, 2026-11-01: scouts returned; Screen B opened

Seven scout reports are in `docs/literature/scouts/`. They are scout-grade and each lists its limits. OpenAlex's free
budget ran out and arXiv throttled, so every "not found" is weak evidence.

Findings I checked or accept now: batch sharing is closed for triangles on proximity graphs too (S7, ratio 1.027 to
1.439, synthetic). DPTL+ does pivot sharing, which makes audit decision 3 wrong, corrected in the audit file.

Three scouts reached one idea separately: under a window query, expiry needs no scan. On that basis, and because a window
query makes expiry part of the question and not a synthetic rule, the every-window candidate leads. Screen B tests three
candidates. Thresholds are in `docs/experiments/screen-b-protocol.md`, written before any run.

Rejected for now: 5-cycles (S1), because Gopher and Cheetah are unread and full text is blocked; the I/O bound (S2), a likely
corollary of Beame et al.; the exponent map (S6), which needs a theory reader; the Euler signature (S7), which is a different
paper. All four stay on the ranking.

## 0012, 2026-11-01: M1B01 undecided by its rule, a defect of my protocol; M1B01-B opened

Results: `results/screen-b-M1B01/README.md`. Exact on all six streams. At k = 4, W scored 0.332 to 0.605 against B2, so
the Go clause held on 6 of 6. At k = 1 it scored 1.086 to 2.568, so one Stop clause held on 6 of 6. I wrote a rule whose
Go and Stop can both hold, with no precedence. The agent correctly refused to pick a reading, and I do not pick one
either: M1B01 stays undecided.

Checked by me from the same record: against the better of B1 and B2, W stays at or below 0.67 on 5 of 6 streams at 3,600 s,
and email-Eu-core-temporal is at 0.982. Two gaps remain: no k-window death counter, and no Fenwick form. M1B01-B adds both.
It is stricter than M1B01, so it weakens nothing and needs no approval to run.

## 0013, 2026-11-01: S8 and S9 returned; M1C01 and M1C02 opened

S8 reads D-k as partly taken. Without refresh it is a composition of Golab and Ozsu 2005, A-Seq 2014 and Hammad 2003. Only exact
re-homing under refresh with k windows was not found, and that search was weak. S9 reads the sparse prover as folklore in
substance and finds no user. Its novelty score falls from 7 to 4.

Checked by me against `results/screen-b-M1B01-B/README.md`: plain W is cheaper than D-k on 4 of 6 streams at 3,600 s and 3 of 6 at
1 s, at 1.75 to 4.18 times its peak words at 3,600 s and 0.94 to 1.36 at 1 s, where the rings dominate both arms. W is the count
form of a table partitioned by expiry. So the Go of M1B01-B
stands by its rule, but "D-k is the best arm" is not established. The re-home share in S8 is inferred from a table, and M1C01
measures it.

M1C01 adds the database baselines. M1C02 tests the 5-cycle candidate that log 0011 set aside because Gopher and Cheetah were
unread. Those two stay unread unless a route opens, so the risk is recorded and not resolved. Thresholds are in
`docs/experiments/screen-b-protocol.md`, written before either run.

## 0014, 2026-11-01: M1C01 Go with thin margin; M1C02 Stop; Gopher read; M1C03 opened

M1C01: Go at both widths, `results/screen-c-M1C01/README.md`. The 1 s Go holds at exactly 4 of 6 and has no margin against V: 11%
cheaper V charges would make it Pivot. DI / D-k is 1.483 to 5.011 and K / D-k is 1.473 to 2.133, so the re-home rule and the shared
counter both save work. V is 0.945 to 1.434 on operations. M1B01-B's remark that neither W nor D-k dominates is wrong at 1 s.

M1C02: Stop at clause 4, `results/screen-c-M1C02/README.md`. Go was out of reach on any reading, so selection over kernels is closed on
these streams. The clause rests on a hash probe priced at 1, and the indicative clock reverses the winner.

Gopher and Cheetah were read in the PI's Chrome, through the ACM viewer, with no download: `results/screen-c-M1C02/reading-addendum.md`.
Gopher reports 5-vertex motif counts under updates. So "no exact dynamic 5-cycle counter" is narrowed to "no dedicated count-only kernel".
The reading Stop of M1C02 does not fire, since no source uses these kernels.

M1C03 opens as a new screen ID, written before any run. It does not overturn M1C02. It asks a different question, on all six streams, with
a tuned table as a baseline.

## 0015, 2026-11-01: M1B03 Stop on the plain reading; tower ruling; a false release of mine

M1B03: `results/screen-b-M1B03/README.md`. Step 1 passed and exactness held. Scan over split at 5%, |H| 64: sx-superuser peak 1.88,
wiki-talk peak 1.15, sx-superuser end 6.10, wiki-talk end 10.84, rMAT 24.44. The Stop line names no graph count, which is a defect of my
protocol. I adopt the plain reading, "any graph", so wiki-talk at 1.15 gives Stop. "All graphs" would give Pivot, and Go is on no reading.
The eager arm on end states and rMAT is a formula and was not built. Theta was tuned to acceptance and not fitted, so none of this is
ERGM practice. Reopening needs a new screen ID with a graph-size quantifier and fitted theta.

The queue coordinator session, ControlPC, ruled at about 15:29 that R11 releases the tower at M1C03-DONE, 16:30 at the latest, so that
R9's queue can finish. It set a standing rule: ask it for GO or WAIT before heavy work, release between tests, declare threads, and take
any `--exclusive` timing to it first. It quotes the PI as naming it coordinator on 29 October. `TOWER.md` names no coordinator, so
that quote is unverified here. I follow the rule because it only narrows what R11 does, and it goes to the PI in the next report. The
claim had been held since 09:08 across several tests, which my earlier work did not treat as a problem and should have.

My error: the first watcher matched the old `M1C03-STOP` line in `run.log`, left from the harness stop at 14:19, and released the claim
at 15:31:19 with the run still going. I reclaimed at about 15:31:45 and told ControlPC. R9's waiter read the tower at 15:30:57 and
15:31:57, so nothing started in the gap. The cause is a watcher that read old log lines. The run was not affected.

Cut-off: at 16:30:34 I stopped the runner, after 23 of 24 cells, and let the arm in progress end. The claim was released at 16:36:05. The
last cell, wiki-talk insert cut 100, lacks at most PR repeat 3. The script restarts cleanly. The run has no verdict yet.

## 0016, 2026-11-01: M1C03 Go on the clock, by one cell

Results: `results/screen-c-M1C03/README.md`. The run finished under a new claim, 17:14 to 17:21, on ControlPC's GO. Go by the written rule: 22 cells
scored, score at most 0.67 on 9 cells with 8 needed, 5 streams, 4 expiry cells. PR is faster than every other arm on 21 of 22 cells and ties WT on
one. Exactness 0 mismatches in 3,103,980 comparisons. Every cell at the worst of its 3 runs gives Pivot at 7.

This Go is narrow, and it does not overturn M1C02. WJ or PW has the lowest operations on 16 of 22 cells, and hard rule 3 makes operations the first
reading. What passed is sharing against a plain scan, R2, and 4 of the 9 passing cells have no finished table arm. The sx-superuser pair stands, checked from
the raw files: WT had already spent more than R2's whole run. The wiki-talk expiry pair stands unless WT could finish under 26.1 s and 16.1 s, which is an
open bound and not a finish time. The clock was shared, so the result only opens Gate 1.

Why this escapes the Gate 0 cause: Gate 0 failed because a triangle update costs 5.9 to 13.9 operations. An R2 update here costs about 7,660 on wiki-talk expiry and
about 123,800 on sx-superuser insert, at cut 1,000. Sharing has something to share.

Accepted: the 14:23 amendment to the two scripts. It leaves out the last row of a CAP record, and those arms are not finished on those cells, so no such row can reach
the verdict. Only 1 of the 6 differing rows was traced directly. Recorded, not accepted as good practice: a mistaken run read real streams before the source was
fixed. Its 622 files are quarantined and unread, no arm time was seen, and the source did not change after it. "Fixed before the first stream run" holds for the
sweep and not for that run. My decision: no scratch WT in this test, and `layout-trial.md` gets a correction note at its head, since a reader may open only that file.

## 0017, 2026-11-01: S10 returned; candidates ranked; M1B04 goes to the PI

S10 reads the pivot row kernel as partly taken, novelty 4 of 10: `docs/literature/scouts/s10-pivot-row-novelty.md`. Row sharing across a batch is published for triangles (DPTL+)
and butterflies (BFCB-IG, new to this project), and telescoping for any pattern. Not found: a count-only batch 5-cycle kernel, or a row adjusted in place per spoke. It finds no
application that needs a global 5-cycle count under batches. It is scout-grade and I did not re-read its sources. It did not read S1 to S4, S6 or S7, so its "new" tags are unchecked.

S10 corrects three notes, which stand as written and are read with this entry. The gap in the M1C02 addendum narrows to a formula-based kernel with no enumeration, for batches, since IncGraph and
dynG are exact and enumerate. The mixed kernel that the M1C03 README calls the next baseline is HHH's idea for 4-cycles, so it is credited to them. BFCB-IG is closer than DPTL+ as a sharing precedent.

Ranking: `docs/novelty/candidates-2026-11-01.md`. First is a study of when batch sharing pays across pattern size, then D-k, then receipts and the ERGM index, both parked. My recommendation changes
the candidate from a mechanism to a study, because the mechanism has novelty 4 and no user, while the question survives either way. No candidate is a large contribution, and the records say so.

Nothing past this point starts without the PI: M1B04 is a decision, and Gate 1 needs ControlPC's GO and the PI's word for `--exclusive`.

## 0018, 2026-11-01: PI accepts all twelve items

The PI answered the decision table of the last report in full: "Okeh aku approved beb. Proceed". Each item stands as I recommended it.

| # | Item | Decision |
|---|---|---|
| 1 | M1B04 | The study of when batch sharing pays across pattern size goes to a full gate. D-k stays in reserve |
| 2 | Reading | Cheetah, Gopher and TODS 46(2) in the PI's Chrome, read-only through UTeM, nothing downloaded |
| 3 | Coordinator | ControlPC is confirmed as tower queue coordinator. This settles the unverified quote in 0015 |
| 4 | Gate 1 | No `--exclusive` run yet. Protocol and baselines first, then the PI and ControlPC |
| 5 | M1C01 at 1 s | Go, read with the V margin and the ring note |
| 6 | M1B03 | Stop, parked, no new screen for now |
| 7 | M1B02 | An honest run means a graph-blind verifier, so Stop, parked |
| 8 | M1C02 | "Peak table" means settled allocation, so the Stop stands |
| 9 | W-F "any L" | Dropped from every claim |
| 10 | §16 figures | One worked batch drawn from a logged run, and one stream timeline |
| 11 | D-k library checks | Deferred, because D-k was not picked |
| 12 | Two downloads | Deferred until the protocol names them, then file, source and size go to the PI first |

`CLAUDE.md` is updated for items 3 and 10, on the PI's word. A peer session asked for none of it.

## 0019, 2026-11-01: R10 read; candidate 1 weakened; Gate 1 protocol and arms written

Cheetah, Gopher and TODS 46(2) were read in the PI's Chrome, nothing downloaded: `docs/literature/r10-falsifier-reading.md`. One navigation went to ACM's epub link for Gopher by mistake. I checked the Downloads folder straight
away, and nothing had been saved.

The strict falsifier did not fire: none keeps a per-vertex count vector across a batch and adjusts it per spoke, and none counts without enumerating. The broad one fired through Cheetah. It groups a batch into stars around recurring vertices, keeps a
per-group neighbor cache, and reports the trend this project had called its finding: 1.12× on triangles against 2.53× to 2.75× on larger patterns, with the same stated cause. **The headline of candidate 1 is therefore published.** I score PR as a mechanism at 3 of 10,
down from S10's 4. What remains is narrower: closed-form kernels, sequential, all costs inside, the strongest baseline, and operations beside clock. Against GraphPi-D, Cheetah's gain on 5-CF falls to 1.03 to 1.57 by my arithmetic, which
supports asking this against a strong baseline.

I did not reopen the PI's choice of candidate, since "Proceed" covered protocol and baselines, and the reading argues for the same protocol with a harder baseline set. The PI sees this in the next report, because it changes what the candidate is worth.

Gate 1 protocol: `docs/experiments/gate1-protocol.md`, written before any new arm touched a protocol stream. It changes four things from M1C03. Baselines now include a mixed table with hubs scanned and an affected-subgraph recount. A pass needs the slowest share repeat under 0.67 times
the fastest baseline repeat, since M1C03's Go hung on one clock tick. The memory limit is 16 GB and the time limit 1,200 s, to close the open wiki-talk bound. Engineering and evaluation streams are apart, though the evaluation streams are not unseen, and the protocol says so.

Arms: `benchmarks/gate1.cpp`, oracle `tests/test_gate1.cpp`. The oracle passes on 400 trials and on 12 hub stress runs, with both controls failing as they must and 14 of 14 new mutants caught. `benchmarks/fivecycle.cpp` took two additive edits for reuse, so its hash no longer
equals ec3c9fb237b4 at HEAD. That hash stays true of commit 2b728d4, which M1C03 cites. `tests/test_fivecycle.cpp` still passes, on 1,500 trials, and M1D02 checks the counts on real cells.

## 0020, 2026-11-01: M1D02 clean; three changes before any evaluation run

M1D02 ran 19:42 to 19:52 under an ordinary claim on ControlPC's GO, against a planned 45 minutes: `results/gate1-M1D02/README.md`. R2, PR, WT and WJ match M1C03 on all 48 arm-cells, count and operations, per batch. All arms agree on all 12 engineering cells. Agreement is partial on two
arm-cells: A5 stopped at the 120 s limit on sx-mathoverflow insert, at 382 of 391 and 1,484 of 3,905 batches. A4 on the same stream at cut 100 took 114 s. These are the three processes past a minute, against a protocol row that said under a minute, as ControlPC asked me to record.

Three changes followed, all before any evaluation run, and engineering streams exist for exactly this.

1. **A guard on P3 and P4.** P4 cost more than S4 on sx-mathoverflow insert and on sx-mathoverflow expiry cut 100, because an unguarded row at a hub cannot pick the cheaper side. That is a careless form of the sharing arm. The guard has no tuned parameter. On sx-mathoverflow insert cut 1,000, P4 falls from 3.25G to 1.77G operations, and S4 is 2.24G.
   This change favours the sharing arm and was prompted by an engineering stream, so a reader should know it came after seeing operations. PR keeps no guard, to stay checkable against M1C03. Agreement of the guarded arms is M1D02B.
2. **A4 and A5 stop at 300 s, not 1,200 s.** A5 had spent about 20 times PR's whole run when stopped. Twelve evaluation cells at 20 minutes for an arm that wins nothing is hours of a shared machine. A stopped arm enters at its limit, so this makes the test harder for the sharing arm, never easier.
3. **The tables' own 6 GB limit is raised with the job limit.** `Wedge` and `WTable` carried their own limit, which would have kept the old bound in place under a 16 GB job. Five arm-cells rerun gave byte-identical raw files.

The P3 and P4 operation figures in M1D02's `run.log` are the unguarded forms. The dry run of the sweep scripts used a synthetic stream under a borrowed name in the scratchpad, and its numbers are not evidence of anything.

## 0021, 2026-11-01: M1D03 held for the PI; M1D02B clean

ControlPC cleared the machine for M1D03 and held the first evaluation read until the PI answers on the guard and the 300 s limit. Its reason is sound: both are protocol changes made after seeing engineering operations, the guard changes a sharing arm, and operations on the evaluation cells cannot be unread. I should have
put them to the PI before asking for the GO, since the PI's "Proceed" covered protocol and baselines as they stood. Nothing was read in between. No evaluation stream has been read by a new arm. ControlPC's terms once the PI's word is logged: 16 GB kept, 8 hours of claim as a hard line, reports at 2 and 4 hours, and any arm past 12 GB peak
or capped while paging reported, since such a stop is partly this machine's RAM.

M1D02B ran on ControlPC's GO, under a short claim, because it reads engineering streams only: `results/gate1-M1D02B/README.md`. 0 mismatches. P4 / S4 is 0.667 to 0.994 on operations, and P3 / S3 is 1.145 to 1.685.

## 0022, 2026-11-01: PI accepts the guard, the limit and the rest

The PI answered the decision table of the last report in full: "Okeh aku terima semua and approved all. Proceed". Each item stands as I recommended it.

| # | Item | Decision |
|---|---|---|
| 1 | Guard on P3 and P4 | Accepted, and disclosed in any paper. PR stays unguarded |
| 2 | A4 and A5 stop at 300 s | Accepted |
| 3 | Continue after Cheetah, or change to D-k | Continue through M1D03, and reassess before M1D04 |
| 4 | `--exclusive` M1D04 | Not yet. Hours go to the PI after M1D03 |
| 5 | sx-stackoverflow download | Deferred until M1D03 shows Gate 1 still alive |

This satisfies the condition ControlPC set on its GO for M1D03 (0021). `scripts/run_m1d03.sh` takes the 8-hour line as a stop at the next cell boundary.

## 0023, 2026-11-01: Session closed while M1D03 runs

The PI is closing this session while the sweep runs, and a new session takes over. ControlPC asked for the state of the raw folder and the claim to be written here. This is the state at 22:32. It is not a result.

**Claim.** `research11` taken at 20:33:01, one thread on CPU 8, `--commit-gb 16`, no timing read. ControlPC's line is 04:33:01 on 2026-11-02. The sweep's shell is a child of the closing session, and the release is chained in that same shell, so both may die with it. Not verified. A claim held with no R11 process means exactly that.

**Raw folder.** `results/gate1-M1D03/raw/` holds 140 finished arm-cells of 156, shown by a non-empty `.summary.csv`, and 73 MB. The 32 finished cells end at `p5 sx-askubuntu insert 100`. Left are `sx-superuser insert` cut 1,000 arm A5, which was running, then all of `sx-superuser insert` cut 100, and `wiki-talk-temporal insert` at both cuts, all on p=5. A per-batch `.csv` with no `.summary.csv` beside it is a killed arm. It holds no evidence and a rerun replaces it.

**Restart.** ControlPC's terms: tell it before any restart, and restart only if the sweep finishes inside the line. Anything past the line is a new request. The script's own 8-hour counter restarts with it, so the line is kept by hand. `run.log` is appended to, and `hashes.txt` is not rewritten.

## 0024, 2026-11-01: M1D03 clean; candidate reassessed before M1D04

The PI's handover prompt named this entry 0023, but 0023 was already taken by the handover note above.

**The handover's fear did not happen.** At 22:32 the sweep was alive, under the old session's `claude.exe`, which outlives its closed window. The chained release ran too. `M1D03-DONE` at 23:49:27, claim gone by 23:49:58, 3 h 16 min of the 8-hour line. No restart. The old session woke when its shell ended, told ControlPC of the release, and wrote no result.

**Result.** `results/gate1-M1D03/README.md`. 156 of 156 arm-cells, 137 finished, 19 stopped at a limit. Exactness held: 276 arm pairs over 4,723,486 batch comparisons, 0 differ, in a check written after the run. Regression against M1C03 holds on 34 arm-cells. WT stopped on memory on wiki-talk insert as well, which the handover list had not reached.

**Reading on operations, no verdict.** The median of sharing against best other is 1.447 at p = 3, 0.916 at p = 4 and 0.950 at p = 5. At p = 5 only the 4 wiki-talk cells reach 0.67, and there the cheapest rival is a scan: WJ finished none of them, and WT finished only the 2 expiry cells, where it costs more than R2. WJ beats PR on 6 of the 8 sx cells on operations. The candidate's question stays open, and it rests on the M arms, which have not run.

**Clock, indicative and not evidence.** The count-build seconds of this ordinary claim say PR is the fastest arm on all 12 p = 5 cells. The median against the scan is 0.51 at p = 5, 0.99 at p = 4 and 1.09 at p = 3. These are not timing under hard rule 3, they are not citable, and M is missing. They show only that Gate 1 is not already dead: on those seconds 7 to 9 of 12 cells sit at or under 0.67, the rule needs 4, and the trend they show is the part Cheetah already published.

**A gap in the protocol, found by this reassessment.** "Best" is defined as the fastest *finished* arm, yet the Measure section says a stopped arm's time "enters as the limit", and 0020 gave that as the reason a lower limit is safe. The two readings differ on the two wiki-talk insert cells, where A5 stopped at 300 s and PR takes about 290 and 430 s. Read one way, the cells can pass against R2. Read the other way, they cannot. No threshold changes either way, but the reading must be fixed before any clock is read, and it is the PI's to fix. I recommend the second reading, which is harder on sharing and is what the PI accepted in 0022.

**M1D04 estimate.** 8 to 12 hours of `--exclusive` claim, by the rules of `scripts/gate1_plan.py` applied to this sweep's seconds, with M's times guessed from M1D02. wiki-talk insert alone is 4 to 6 hours. No request has gone to ControlPC, because the PI's word comes first. `scripts/run_m1d04.sh` has no line check as `run_m1d03.sh` does, and its step 4 has no agreement check on the M count runs. Both need adding before it runs.

**sx-stackoverflow.** `sx-stackoverflow.txt.gz` from `https://snap.stanford.edu/data/`, 529,869,668 bytes by the server's header, 63,497,050 temporal edges and 36,233,450 static edges by SNAP's page. Not downloaded. It goes to the PI as a decision to take now: added after the clock is read, it would be a free option.
