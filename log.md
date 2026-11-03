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

## 0025, 2026-11-02: PI accepts the five items; the reading of a stopped arm is fixed

The PI answered the decision table of the last report in full: "Approved". Each item stands as I recommended it. Nothing here has touched a clock.

| # | Item | Decision |
|---|---|---|
| 1 | Candidate | Continue to M1D04. D-k stays in reserve |
| 2 | A time-stopped arm in "best" | It enters at its limit |
| 3 | `--exclusive` M1D04 | Approved, 8 to 12 hours, in two claims: the other 12-cell work first, wiki-talk insert apart. ControlPC's GO is still needed |
| 4 | `scripts/run_m1d04.sh` | Fixed first: a line check, a check on the M count runs, and the hash of `build/gate1/timing.exe` |
| 5 | sx-stackoverflow | Download approved, and committed now: p = 5 only, in a separate claim after the 12 cells, in the verdict if scored |

**Protocol reading M1D04-R1.** It closes the gap in 0024 and changes no threshold. "Best" is the fastest arm of the other rows, finished or stopped by time, and a time-stopped arm enters at its time limit. An arm stopped by memory stays out of that cell. "Scored" still needs share and at least one other arm to have finished. This is harder on the sharing arm, and it is fixed after M1D03's operations and before any Gate 1 clock.

The PI asked for the download through the PI's own Chrome. It came from the link on SNAP's dataset page, 00:08 to 00:12:58 on 2026-11-02, and only that file was fetched. It sits at `data/snap/held-out/`, apart from the six inputs, with its own `SHA256SUMS`: 529,869,668 bytes, equal to the server's header, sha256 starting fc04264de165, gzip test passed. SNAP publishes no checksum, so that hash fixes what was received and does not verify it. No arm has read it, and I read only its first three lines.

`scripts/run_m1d04.sh` passed a dry run on CollegeMsg, an engineering stream, in the scratchpad: all three phases' paths, the line stop and `mcheck`, 0 differences. Its seconds were thrown away, and its theta values are not a choice.

## 0026, 2026-11-02: ControlPC's GO for main; held-out stream derived

ControlPC answered: derive GO, phase `main` GO straight after it, phases `wiki` and `held` WAIT. Its GO settles the queue only, and the PI's word is 0025. Its terms for `main`: `LINE_HOURS=8`, overrun of 20 minutes accepted, tower clear at the claim, clean tree, a report at 4 hours, at once on STOP or LINE, and at release. This session stays quiet while timing runs: no build, no test, no Chrome, no subagent. The run's README must say that "exclusive" here means no heavy work and not a silent machine, since the machine has other Claude sessions, Chrome, OneDrive and ControlPC's two-hourly status mail.

**A limit on the PI's hours.** ControlPC reads 0025's 8 to 12 hours as covering both claims. So `wiki` gets a line of 12 hours less `main`'s real hours, and if that is too little for its estimate it goes back to the PI first.

**Derive.** A short R9 `pytest` run with no claim read as busy at 00:16, so the derive waited for it. Claim 00:16:53 to 00:18:17, one process at a time, `scripts/prepare_stream.py` unchanged. Expiry 44 s, insert 40 s, peak working set 13.77 GB each, peak commit 14.62 and 14.61 GB.

| File | Label | Events | Bytes | sha256 starts |
|---|---|---|---|---|
| `sx-stackoverflow.bin` | real timestamps, synthetic expiry | 95,806,532 | 1,916,130,640 | 3b8720e44e1d |
| `sx-stackoverflow.ins.bin` | real timestamps, insert-only | 47,903,266 | 958,065,320 | f3df8cff0843 |

63,497,050 lines gave 47,903,266 kept, as self-loops drop, and 2,584,164 vertices. Times are sorted and ids are compact. 7.9 times wiki-talk's insert events, so the rough 6 to 8 hours for `held` was not too high, and its insert cells may finish no arm at all.

## 0027, 2026-11-02: M1D04 main finished clean; six of ten p = 5 cells pass

Phase `main` ran under the exclusive claim from 00:19:17 to 02:48:58, 2 h 29 min 41 s against an estimate of 3.5 to 6.5 hours and a line of 8. `M1D04-DONE`, no restart, tree clean at e23c597, all nine hashes the same after the run. Record: `results/gate1-M1D04/README.md`, scored by the new `scripts/gate1_score.py`.

**Exactness holds.** Every repeat, the M count build and M1D03's arms on the same cells: 13,101,889 batch comparisons, 0 differ. Hard rule 1 is not touched.

**Theta,** from the engineering cells only: 16, 16, 128 at p = 3, 4, 5.

| p | Scored | Median score | Median against scan | Pass |
|---:|---:|---:|---:|---:|
| 3 | 10 of 10 | 1.131 | 1.131 | 0 |
| 4 | 10 of 10 | 1.279 | 0.992 | 0 |
| 5 | 10 of 10 | 0.599 | 0.599 | 6, on 3 streams, 4 expiry |

Streams: real timestamps with synthetic expiry, and real timestamps insert-only. wiki-talk insert and sx-stackoverflow are not in this reading, and there is no verdict until M1D05.

**The rest cannot turn this but by an exactness failure.** With 16 scored and no further pass, p = 5 stands at 6 of 16, above a third, with its median at worst 0.796, and p = 3 at worst 1.098. That is Go with the trend, unless an exactness failure gives Stop. Phases `wiki` and `held` still run: the PI committed the held-out stream before this clock (0025), and dropping a committed stream once the outcome is seen would be the free option 0024 warned of. They are also the two largest exactness tests, and the only unseen stream.

**0024 expected wrongly.** It said Pivot or a thin Go, resting on M. M is best on 2 of 10 cells at p = 5, and the scan on 8. What moved was the price per operation: PR 0.61 ns, R2 0.72, M5 0.83, WT 1.69, WJ 8.26. WJ does fewer operations than PR on 6 of 8 sx cells at p = 5 and takes 3 to 26 times as long, so most of the sx result is a cache result on one machine. Two of the six passes would fall to a WJ about 2.6 times faster, and those are the ones to attack. Cut 1,000 passes on 5 of 5 cells, and cut 100 on 1 of 5.

**An ablation for M1D06.** p = 3 and 4 chose the end of the grid, and theta came from streams that run in fractions of a second. M5 over the grid on evaluation cells would favour the baseline, and tells how much of the result rests on that.

## 0028, 2026-11-02: M1D04 wiki finished clean; held needs the PI's hours

Phase `wiki` ran under the exclusive claim from 02:56:10 to 08:17:13, 5 h 21 min 03 s against an estimate of 5 to 7.4 hours and a line of 9, on ControlPC's GO. `M1D04-DONE`, no restart, all nine hashes the same. `main` and `wiki` come to 7 h 50 min 44 s, inside the PI's 12 hours. The record is `results/gate1-M1D04/README.md`.

**Exactness holds.** Over both phases: 16,141,789 batch comparisons, 0 differ.

| p | Scored | Median score | Median against scan | Pass |
|---:|---:|---:|---:|---:|
| 3 | 12 of 12 | 1.098 | 1.098 | 0 |
| 4 | 12 of 12 | 1.299 | 0.974 | 0 |
| 5 | 12 of 12 | 0.642 | 0.535 | 6, on 3 streams, 4 expiry |

Streams: real timestamps with synthetic expiry, and real timestamps insert-only. sx-stackoverflow is not in this reading, and there is no verdict until M1D05.

**M1D04-R1 cost two passes.** On wiki-talk insert A5 stopped at 300 s with 14.3% and 5.6% of the batches done, and it enters "best" at its limit, so PR scores 0.922 and 1.363. Against the arms that finished, PR's slowest over M5's fastest is 0.322 and 0.463. The reading was fixed before this clock to be harder on the sharing arm. It does not reopen, and the count stays 6. Reopening it now, with the clock read, would be the free option 0024 warned of, in the sharing arm's favour.

**A cell that does not fit.** On wiki-talk insert at p = 4, M4 takes 15.7 s against P4's 58 and 79 s, scores of 3.699 and 5.016, while at p = 5 PR beats M5 by more than two to one. The median trend holds, and this cell is why a single line through p = 3, 4, 5 would be a poor summary. M at p = 4 was also far from my planning guess of 65 to 106 s, so the wiki estimate was high for a reason I did not foresee.

**The outcome is still held by the arithmetic of 0027.** `held` is p = 5 only, so p = 3 is final at 1.098. If its 4 cells are all scored and none passes, p = 5 is 6 of 16 with its median at worst 0.796.

**Held needs the PI's word.** 0025 committed the stream and a separate claim, and its hours cover the two claims now done, as ControlPC read them in 0026. So no GO is asked for `held` until the PI gives hours. Estimate from the real costs: 4 to 6.5 hours. Both insert cells will almost surely finish no arm: wiki-talk insert took PR 277 to 409 s, and this stream has 7.9 times its events, so PR would need less than 4.4 times the time to finish inside 1,200 s, and on past streams time grew faster than events. Such cells cost about 1.4 hours each and are not scored. The expiry cells are the live part. The M count build adds about 0.8 hours.

## 0029, 2026-11-02: R9 ran inside the wiki claim

ControlPC passed on R9's report, unchecked by either: one R9 process at a time from 05:39:59 to about 05:47, unpinned, during the exclusive `wiki` claim. Two of this project's timed processes overlap it, PR repeat 3 on wiki-talk insert 1,000 (286.62 s, the others 273.56 and 276.63) and PR repeat 1 on wiki-talk insert 100 (414.00 s, the others 408.52 and 408.89). Labels: real timestamps, insert-only. Both are the slowest of their three. No other timed process overlaps. So hard rule 3's "pinned, exclusive" does not strictly hold for those two repeats.

Neither moves a result. Both cells fail with or without the repeat, the median scores could move by at most 0.010 and 0.001, and the p = 5 median rests on other cells. The README records the table. Rerunning them is the PI's call. My recommendation is to keep them, marked, since a rerun can change nothing that the rule reads and would add a repeat chosen after the clock was seen.

## 0030, 2026-11-02: PI approves held and keeps the two repeats

The PI answered the four items of the last report in this session, at about 11:20: "Okeh approved all beb". Each stands as I recommended it.

| # | Item | Decision |
|---|---|---|
| 1 | `--exclusive` hours for `held` | Approved, up to 8 hours, `LINE_HOURS=8`. Estimate 4 to 6.5 hours. ControlPC's GO is still needed |
| 2 | When | Tonight, so that it does not meet the PI's use of the machine by day |
| 3 | The 4 held-out cells | All 4 stay, though both insert cells will almost surely finish no arm and go unscored at about 1.4 hours each |
| 4 | The two PR repeats that overlap R9 | Not rerun. They stay, marked in the README, as 0029 states |

ControlPC had reported an approval recorded in R9's log, to a table it could not see. That record is not this project's, and it was not used. The word above is the PI's own, here. No threshold changes, and reading M1D04-R1 stands.

## 0031, 2026-11-02: PI moves held to now

The PI, in this session at about 16:05: "okeh mesin dah kosong. aku nak ko tutup mane2 yg tak diperlukan and mulakan sekarang berbanding malam nanti. start now". This replaces item 2 of 0030, tonight. Items 1, 3 and 4 stand: up to 8 hours, all 4 cells, the two PR repeats kept.

ControlPC answered GO for now, with a claim between now and 17:30 and `LINE_HOURS=8`, no one else in the queue. Its e-mails at 18:00, 20:00, 22:00 and 00:00 fall inside the claim and are its overlaps.

**Apps closed** at 16:3x on the PI's word, each gracefully first and then stopped: Edge in the background (7 processes, 494 MB), BingWallpaper (182 MB), Phone Link (151 MB), NVIDIA Overlay (5 processes, 151 MB, one came back by itself). ChatGPT was already closed. Left alone: Claude Desktop, which hosts every session, TightVNC, Tailscale, Codex, and the shells of other sessions. Available memory went from 16.39 to 16.87 GiB. Startup items were not touched, since they are a lasting system setting and do nothing tonight without a reboot.

## 0032, 2026-11-02: M1D04 held finished clean; M1D05 gives Go with the trend

Phase `held` ran under the exclusive claim from 16:40:52 to 23:26:23, 6 h 45 min 31 s against an estimate of 4 to 6.5 hours and a line of 8. `M1D04-DONE`, no restart, all nine hashes the same. M1D04 took 14 h 36 min 15 s of exclusive claims in all. Records: `results/gate1-M1D04/README.md` and `results/gate1-M1D05/README.md`.

**Exactness holds.** Over all three phases: 32,103,387 batch comparisons, 0 differ. Hard rule 1 is not touched.

**Held-out.** sx-stackoverflow expiry, real timestamps with synthetic expiry, scores 0.846 and 0.978, PR's slowest over R2's fastest 0.863 and 0.983, so neither passes. Its insert cells, real timestamps insert-only, finished no arm inside the limit and are not scored. PR got furthest in the same 1,200 s.

**Verdict by the rule.** p = 5: 14 of 16 scored, 6 pass on 3 streams with 4 expiry cells, which is above a third. No exactness failure. Go. Median score 1.098 at p = 3 and 0.686 at p = 5, so the trend clause holds too. The protocol's outcome is one experimental paper.

**My reading of its weight, for the PI.** The rule's word stands and does not reopen. But the only unseen stream gives no pass. Two of the six passes, the sx expiry 1,000 cells, decide Go, and a flat table about 2.6 times faster than WJ would remove them. Most passes beat PR's own unshared kernel. And the win over the tables is a cache win on one machine. A paper from this is a careful experimental paper with those limits up front, not a large contribution. The PI decides what to do next, and M1D06 waits on that.

## 0033, 2026-11-03: a correction to 0032 on what breaks the two sx expiry passes

0032, the M1D04 and M1D05 records and STATUS said the two sx expiry 1,000 passes "fall to a WJ about 2.6 times faster". The arithmetic is right and the aim is wrong. WJ is the plain flat table. WT, the tuned table, already exists and is in the sweep. The speed each other arm's fastest repeat would need, at p = 5, to break each pass:

| Stream | Label | Cut | R2 | WT | WJ | M5 | A5 |
|---|---|---:|---:|---:|---:|---:|---:|
| sx-askubuntu | real timestamps, synthetic expiry | 1,000 | 1.13 | 2.06 | 2.65 | 2.16 | 1.88 |
| sx-superuser | real timestamps, synthetic expiry | 1,000 | 1.09 | 1.95 | 2.61 | 1.98 | 2.07 |
| sx-askubuntu | real timestamps, insert-only | 1,000 | 1.63 | 2.83 | 17.17 | 1.61 | 24.6 |
| sx-superuser | real timestamps, insert-only | 1,000 | 1.59 | 2.66 | 16.79 | 1.69 | 15.05 |
| wiki-talk-temporal | real timestamps, synthetic expiry | 100 | 1.38 | 7.44 | | 1.73 | 11.37 |
| wiki-talk-temporal | real timestamps, synthetic expiry | 1,000 | 2.20 | 11.85 | | 2.76 | 5.90 |

So the two fragile passes are closest to the scan: R2 13% and 9% faster would remove them. R2 is PR's own kernel without sharing, so a faster kernel speeds both, and the margin is a property of this machine more than of a weak baseline. The tuned table would need about twice its speed, and M5 at theta 128 about twice too. A stronger flat table is the weakest attack, and M5 at another theta the cheapest real one.

On the PI's "Yes, attack first", M1D06A runs M5 over the whole theta grid on the evaluation cells at p = 5, with PR and R2 in the same claim, and `docs/experiments/m1d06a-theta.md` fixes it before any run. No new arm is built. A second machine is the test for the scan margin and the cache claim, and that is the PI's question.

## 0034, 2026-11-03: PI approves M1D06A's hours; dry run passed

The PI, in this session: "Yes, 5 hours", to the question "Approve up to 5 exclusive hours for M1D06A?". So M1D06A runs `--exclusive` with `LINE_HOURS=5` when ControlPC gives a slot. The PI did not answer the other two items of that table, no stronger flat table and a second machine, so neither is decided. No table is being built.

**Dry run.** On ControlPC's GO, an ordinary claim from 01:54:36 to 01:54:45, `scripts/run_m1d06a.sh` on CollegeMsg, an engineering stream, expiry and insert at both cuts, theta 16 and 128, output in the scratchpad: 80 summaries with full repeats, every cell's arms agree per batch, `mcheck` 0 differences on 8 runs, `M1D06A-DONE`. Its seconds were thrown away.

## 0035, 2026-11-03: M1D06A, all six passes hold against M5 at its best theta

M1D06A ran under the exclusive claim from 02:05:57 to 04:55:33, 2 h 49 min 36 s against a line of 5. `M1D06A-DONE`, no restart, the M1D04 binary. Record: `results/gate1-M1D06A/README.md`.

**Exact.** 70 `mcheck` runs and 14,830,901 batch comparisons against M1D04 and M1D03, 0 differ.

**Result.** At p = 5, on real timestamps with synthetic expiry and insert-only, all 6 passes of M1D05 hold when M5 gets the theta that suits each cell best, chosen on that cell. So the attack the PI chose did not move the verdict: Go stands at 6 of 14 under the harder reading too. The best theta is 16 on sx expiry and 64 elsewhere, against engineering's 128. M5 gains up to 29%, beats R2 only on sx-askubuntu insert, and never comes near breaking a pass.

**What is left.** The two sx expiry passes hold against R2 at 0.612 and 0.616. That margin is the scan's and this machine's, and only a second machine tests it. The claim was noisier than M1D04: 12 repeats up to 19% slow from 02:08 to 04:06, not explained by ControlPC's 04:00 mail. That noise works against PR in this test, and it is recorded with the near-miss margins kept apart from M1D04's.

## 0036, 2026-11-03: Research 1's scheduled tasks ran through every claim; no pass rests on them

After M1D06A, ControlPC read the event logs and Task Scheduler and reported that Research 1's PowerShell tasks, run from that project's folder and paused by no one, fire through every claim of this project, M1D03 and M1D04 included: a gate check every 15 minutes, a status every 30, and an hourly `git bundle --all` into OneDrive, which OneDrive then uploads. Their per-run CPU is not known, and their history before the last run is not kept. It also corrected its own mail time to about :06:15, not :00, on every claim, and listed one-off Windows and Office tasks and Defender status events inside M1D06A. Research 12 ran a web-only agent at an unknown time. These are ControlPC's readings, not checked here.

**Test.** `scripts/overlap_check.py` takes every timed process, from its summary's write time less its seconds, and marks it when it covers a quarter-hour mark, the :06:15 mail or a listed one-off task. Repeats that overlap run slower than their own arm's median more often: in `main` 8 of 19 over 3% against 8 of 531 clean, and in M1D06A 6 of 60 against 6 of 385. In `wiki` and `held` the effect does not show. So the load is real, small, and seen mostly on short runs.

**No pass can be its artefact.** Load only adds time. A pass needs PR's slowest repeat under 0.67 times the best arm's fastest, so a false pass would need the best arm's fastest repeat slowed. For all 6 passes that repeat overlaps nothing known, and at most 1 of the best arm's 5 repeats overlaps on any. M1D05 and M1D06A stand.

**Corrections made.** The `held` mail overlaps in the M1D04 record now use 18:06, 20:06 and 22:06. My guess of Windows maintenance in the M1D06A record is withdrawn. "Exclusive" in this project's records means no heavy work by a tower project, and not a machine without scheduled work. Whether Research 1's tasks are paused during claims is for the PI and ControlPC, since they belong to another project, and Task Scheduler is not mine to change.

## 0037, 2026-11-03: Research 12 inside the M1D06A claim

Research 12 reported, through ControlPC and unchecked by either, three concurrent ripgrep searches over the whole of C:\, 20 s each, somewhere from 04:50:07 to 04:53:50, and a 25 s probe at about 02:00:27 to 02:00:52, before the claim. M1D06A's last timed process ended at 04:28:44, from its summary's write time, and its count build ran from 04:28:50 to 04:55:33. So the searches fell in the count build, which carries no time, and every count there matched. No figure moves. Recorded in `results/gate1-M1D06A/README.md`.

## 0038, 2026-11-03: PI approves the next step after M1D06A

The PI answered the four items of the last report in this session: "Okeh approved beb". Each stands as I recommended it.

| # | Item | Decision |
|---|---|---|
| 1 | Next step | The two §16 figures and the paper outline start now |
| 2 | Second machine | Used for the scan margin and the cache claim if one exists. The paper does not wait for it, and whether one exists is still open |
| 3 | Research 1's tasks during timing claims | Paused for future timing claims. They belong to another project, so ControlPC and the PI carry it out. Not this project's to change |
| 4 | A stronger flat table | Not built |

## 0039, 2026-11-03: the two §16 figures and the outline

**Figures.** `docs/figures/`, from logged runs only, as plain SVG, since no plotting library is installed and none was fetched. Figure 1 opens batch 465 of sx-askubuntu expiry, cut 1,000, at p = 5, chosen by a rule fixed before any batch was read: the median of PR over R2 among batches where PR is cheaper. The new `benchmarks/worked_batch.cpp` replays the stream with Gate 1's PR engine and reproduces the logged row exactly, 177,236 operations and a count of 9,394. Figure 2 is the same cell's count and each arm's operations over all 1,454 batches. Both are operations, not time, and say so. Rendered for checking with headless Edge, outside the app.

**Outline.** `docs/paper/outline.md`. The check against bootstrap §5 finds test 5, the scope of the evaluation, likely failing: 3 evaluation streams and 1 held-out, against 56 instances in the closest prior study, and no stream with real deletions. Three decisions go to the PI: the contribution type, more streams before writing, and the venue task M1A17.

## 0040, 2026-11-03: PI chooses more streams before writing

The PI, in this session: "Yes, add streams", to "Add more streams before writing the paper?". So the evaluation grows before any manuscript, as the outline's decision B recommended: more streams at p = 5 and p = 3, with real deletions from KONECT as the hard case, under a protocol extension fixed before any new stream is read. Decisions A, the contribution type, and C, the venue task M1A17, were not answered and stay open.

## 0041, 2026-11-03: handover, where the stream search stands

The PI closed this session to start a new one. No claim is held, the tree is clean, and nothing is running.

**Stream search, begun and not finished.** Hanauer et al.'s experiment guide (`exp/README.md` of AlgorithmsInAction/DynamicSubgraphCounter, read by WebFetch) names four sources for its 56 instances: DyReach Kronecker streams of constant and growing size, KONECT dynamic Wikipedia streams, and timestamped interaction streams from SNAP and Network Repository, the latter given a 14-day lifetime to produce deletions. It cleans every stream as this project does, and builds 2,000,000-update and 150,000-update prefixes. The per-instance list is in `exp/scripts/get_data.sh` there, which was not read. The arXiv PDF of the paper could not be read as text by the fetch tool. Not yet done: read that script, choose streams by a rule written first, check KONECT's format and the loader it needs, and write the M1D07 protocol extension. Every download needs the PI's word with file, source and size first, and every timing run the PI's hours and ControlPC's GO.

**Open with the PI.** Decision A, the contribution type, and decision C, the venue task M1A17, in `docs/paper/outline.md`.

## 0042, 2026-11-03: M1D07 protocol, 13 streams by a rule fixed first, loader exact

**The 56 instances.** `exp/scripts/get_data.sh` of AlgorithmsInAction/DynamicSubgraphCounter, read as text on its `main` branch: 38 DyReach Kronecker streams (19 graphs, constant and growing size), 6 KONECT `link-dynamic-*wiki` streams, 5 SNAP temporal files, 2 SNAP Reddit TSV files and 5 Network Repository files. Its tools were read too. `graph_cleaner` drops self-loops and direction, stable-sorts by time and keeps only state changes, which is this project's set semantics, so a KONECT stream reads here as it does there. Their lifetime tool, despite its file name `unixTo24hLifetime.cpp`, writes 1-day, 7-day and 14-day versions and the script uses the 14-day one; a repeat interaction extends a life. This project's expiry does not extend it. M1D07 keeps this project's rule, and the paper states the difference.

**Rule first.** The selection rule of `docs/experiments/m1d07-streams.md` was committed at ea5f5d9 before any size, page or count of a candidate was looked up: the full set, drop the fully synthetic Kronecker streams (hard rule 4), drop the five streams already used, fetch only from the URL the script names, drop a file over 2 GB, cut every derived stream to Hanauer et al.'s own 2,000,000-event prefix. Then the servers' size headers, by HEAD request, and nothing else: 13 streams remain, 1,902,166,321 bytes, the largest dewiki at 437.6 MB. KONECT's page was read for simplewiki, 1,627,472 events, so it runs whole, and nlwiki, 20,070,561.

**Loader and oracle, no download.** `scripts/prepare_m1d07_stream.py` reads KONECT archives in place, SNAP triples, the Reddit TSV and Network Repository files. `tests/test_m1d07_loader.py`, run with seeds 7 and 11: the expiry and insert streams of CollegeMsg and email-Eu-core-temporal equal `data/derived` byte for byte, prefix included; 60 random KONECT-format archives equal a line-by-line transliteration of `graph_cleaner`; every Gate 1 arm at p = 3 and p = 5, M at two thetas, gives the brute-force count on every batch of 8 derived real-deletion streams at cuts 1, 7 and 40, 288 runs, 42 of 48 cells with a nonzero count. The three controls fail as they must: deletions first at equal times, direction kept, and a brute force that never deletes. `build/gate1/test.exe 2000 7` passes with every mutant caught. The hashes of `count.exe`, `timing.exe`, `gate1.cpp` and `tc.hpp` equal M1D04's, so no C++ changed. All single processes under a minute, no timing, run beside Research 12's non-exclusive claim of four single-thread processes, which `tower_check.py` showed at 0.4 hours old.

**Estimate.** Count sweep, ordinary claim: 6 to 30 hours, central 10. Timing, `--exclusive`: 10 to 35 hours, central 19, in phases of at most 8 hours whose order is fixed now. The range is the unknown 5-cycle cost of a Wikipedia link prefix, and the count sweep fixes it before any exclusive hours are asked for.

**Waiting on the PI.** The 13 downloads, with file, source and size in the protocol. The ordinary claim hours for M1D07C. Nothing is downloaded, no claim is held, and nothing is running.

## 0043, 2026-11-03: PI approves the seven items of 0042's table

The PI, in this session: "Okeh approved beb", to the table of the last report. Each stands as I recommended it.

| # | Item | Decision |
|---|---|---|
| 1 | The 13 downloads, 1,902,166,321 bytes, files and sources as in `docs/experiments/m1d07-streams.md` | Approved, all 13 |
| 2 | Ordinary claim for the count sweep M1D07C | Approved, up to 16 hours in two claims, after Research 12 releases, on ControlPC's GO |
| 3 | `--exclusive` hours for M1D07D | Deferred. A firm figure from M1D07C's seconds, and how many phases run, come to the PI before D1 |
| 4 | The 38 Kronecker streams | Out of M1D07 |
| 5 | This project's expiry rule against Hanauer et al.'s | Kept. The paper states the difference |
| 6 | Decision A, contribution type | Algorithm engineering, with PR as the artifact. `CLAUDE.md` updated |
| 7 | Decision C, M1A17 | Runs now as reading, during the count sweep, with no machine. The venue stays the PI's |

## 0044, 2026-11-03: downloads done, a format check that changed how two files are read, a dry run, and an unclaimed 20 s

**Downloads.** All 13, 08:20:21 to 08:25:50, each equal to the server's size header, into `data/m1d07/raw/` with `SHA256SUMS`. 1,902,166,321 bytes. None of the servers publishes a checksum, so the hashes fix what was received and do not verify it.

**Reading M1D07-R1.** The format check, by listing each zip and reading its first lines, found that step 6 of the protocol would misread two files. `ia-reality-call` reads "u,v,t,duration", so the time would have been the call's duration. `soc-wiki-elec` is headed `% asym signed`, so its oppose votes would have become edge deletions. The rule now reads deletions only under a `% <structure> dynamic` header, which none of the five has, and takes the time from whichever of columns 3 and 4 has every value at least 10^8. Written into `docs/experiments/m1d07-streams.md` before any stream was derived or any arm ran. It changes which column is read, not any threshold, cell or arm. `tests/test_m1d07_loader.py` now covers the four layouts of the five files, a refusal of real deletions without a dynamic header, and a refusal when both columns look like times: it passes. The Enron file carries dates from 1980 to 2024. No filter was fixed before the check, so none is added. **This is the PI's to accept or turn back.** The alternative is to drop the two files under step 6.

**Dry run.** On ControlPC's GO, `scripts/run_m1d07.sh` on fixtures and CollegeMsg, scratchpad output: every path ran, 140 arm runs OK, every cell's arms agree per batch, and CollegeMsg matches M1C03 on 4 arm-cells, 0 differences. Its seconds were thrown away.

**A slip.** The dry run's claim command used `--shared` without `--for`, `tower_check.py` refused it, and the chain did not test its exit status, so the dry run ran unclaimed from 08:23:48 to 08:24:08, 20 s, one pinned process at a time, a few MB, beside Research 12's non-timing claim. Reported to ControlPC, which logged it as harmless. `scripts/chain_m1d07c.sh` now does every check first and starts nothing unless the tower reads clear, RAM available is at least 14 GiB, commit free at least 40 GB, the checksums and `count.exe`'s hash hold, and the claim file exists. It refused correctly at 08:3x while Research 12 held the tower.

**Queue.** ControlPC: WAIT for M1D07C until Research 12 releases its W2 claim, expected about 09:40 to 10:40, then GO with a window. M1A17 runs as reading meanwhile.

## 0045, 2026-11-03: M1A17 read; JGAA first, KAIS second

`docs/submissions/paper-1/venue-fit.md`. A subagent read the venues' own pages. SCImago, ACM, Clarivate and DBLP refused the plain fetcher, so it read them in the PI's Chrome, by links and the Master Journal List's search box only, with no login, form or cookie banner. I checked JGAA's scope and fees again myself. ACM refused my own fetch, so JEA's closure is the subagent's reading only.

**JGAA** passes on scope, which names dynamic graph algorithms, experimentation and implementations, and on type, with three engineering papers in 2025 and 2026, none on counting. It is free. SJR 2025 gives Q2 in one category and Q3 in four, so the PI's list's "Q2" is its best category only, and the Master Journal List finds it in no WoS index. **KAIS** is free by the subscription route, Q1 and SCIE, with dynamic and streaming graph papers, but its readers are data-mining readers and it would need an applications framing. **J.UCS** fails: no exemplar, and its about page does not name algorithm engineering. **ACM JEA** fails: by its home page it closed after 2023, and its successor, TALG's experimental track, charges $950 to $1,450 unless the institution is in ACM Open, which is not checked.

The venue is the PI's decision. My recommendation: JGAA, unless this paper needs WoS, then KAIS.

## 0046, 2026-11-03: M1D07C's first claim, with two streams held out

**GO.** Research 12 released at 09:59:02. ControlPC gave GO for the first ordinary claim, to start by 10:30, `LINE_HOURS=8`, `--workers 1`, `--commit-gb 16`, on the checks of 0044 with its commit floor of 40 GB, and with a word at once if RAM available falls under 6 GiB while a stream is derived. The second claim needs its GO again.

**Before the claim.** The dewiki file has about 10^8 lines, and the loader as written would have held them as 64-bit columns and sorted copies of them, near 10 GB, against 15.5 GiB available on a machine the PI uses by day. So the KONECT reader now reads in chunks of 4,000,000 lines into narrow arrays, and the cleaning now runs over only as much of the time-sorted stream as yields 2,000,000 kept events, doubling from twice that. Whether an event changes its pair's state depends only on earlier events, so the output is the same, and the test now checks it: random prefixes and chunks of one to eight lines give the `graph_cleaner` stream, and the whole test passes. Each loader process also watches RAM available itself and stops, exit 5, under 6 GiB, and the sweep then stops and releases. Commit 8c2c92f.

**Held out.** The PI has not yet answered reading M1D07-R1 (0044). `ia-reality-call` and `soc-wiki-elec` are the two files it changes, so `SKIP` holds both out of this claim: no stream of theirs is derived and no arm reads them until the PI decides, so the decision cannot follow their numbers. Both are small and fit in the second claim. The other three Network Repository files read the same under R1 as under step 6's intent, with the weight ignored.

**Claim.** Taken 10:02:26, after every check: tower clear, RAM available 15 GiB, commit free 79 GiB, the 13 checksums, `count.exe` equal to M1D04's. The line is 18:02:26, plus at most one process. Release chained in the same background command. Output `results/gate1-M1D07/`.

## 0047, 2026-11-03: the derived streams; soc-youtube-growth's prefix is one snapshot

Derivation finished inside the claim with no drop and no RAM stop. From each stream's summary, structure only, no arm result:

| Stream | Label | Events | Deletions | Note |
|---|---|---:|---:|---|
| simplewiki | real timestamps, real deletions | 1,558,278, whole | 27.8% | |
| nlwiki, plwiki, itwiki, frwiki, dewiki | real timestamps, real deletions | 2,000,000 each, cut from 20 to 86 million lines | 15.8% to 20.7% | |
| soc-redditHyperlinks-body, -title | real timestamps, synthetic expiry; insert-only | 573,122 and 1,143,854 expiry, whole | | |
| tech-as-topology | real timestamps, synthetic expiry; insert-only | 342,806 expiry, whole | | |
| ia-enron-email-dynamic | real timestamps, synthetic expiry; insert-only | 2,000,000 expiry of 2,269,980; 1,134,990 insert | | Dates from 1980, as 0044 noted |
| soc-youtube-growth | see below | 2,000,000 of 24,447,548 and of 12,223,774 | none | **One time** |

So the KONECT prefixes hold 16% to 28% real deletions, and 0042's worry that an early prefix would hold few was wrong: the hard case is there.

**soc-youtube-growth.** Every one of its first 2,000,000 events carries the same time, 1165705200, 2006-12-10: the file opens with a snapshot of more than 2,000,000 edges, dated at once. So the prefix holds no expiry, and its expiry and insert streams are the same file byte for byte (`cmp`). Its cells are then a static graph inserted in file order, two copies of each, and the label "real timestamps" does not describe them. Hanauer et al.'s `2m` instance of it has the same property, since their lifetime also starts at that one time. Nothing has read its p = 5 cells yet. The sweep reaches them after the p = 5 cells of the four streams before it. **The PI's to decide**: keep one copy, labelled a static snapshot in file order, reported and never in a verdict; or drop the stream under step 6. Running both copies costs hours and adds nothing.

## 0048, 2026-11-03: PI accepts R1 and one youtube copy as a static snapshot

The PI, in this session: "Aku follow cadangan ko la senang. Proceed beb. Approved mane yg optimum", to the two open items. Each stands as I recommended it.

| # | Item | Decision |
|---|---|---|
| 1 | Reading M1D07-R1 for `ia-reality-call` and `soc-wiki-elec` | Accepted. Both are derived and swept in M1D07C's second claim, on ControlPC's GO |
| 2 | soc-youtube-growth | One copy, the insert stream, labelled static snapshot, file order, reported and never in a verdict. The expiry copy moved to `data/m1d07/held/` at 10:28, while the sweep was on Enron's p = 5 cells and before any p = 5 youtube cell. Its two p = 3 cells had run and are kept, unused |

With the expiry copy gone, the sweep's two p = 5 youtube expiry cells find no file, run no arm, and are recorded as not run. No result has been read in deciding either item: the p = 5 cells of the two NR files and of youtube had not run.

## 0049, 2026-11-03: Research 12 beside M1D07C from about 10:31

ControlPC reports, unchecked here, that Research 12 runs probe P4 beside this sweep from about 10:31 for up to an hour: four unpinned single-thread `r12_bench.exe` processes, 6 GB of commit, an ordinary claim. M1D07C is a count build, so its operations and counts do not move. Its seconds feed the estimate of exclusive hours, so every cell from 10:31 until ControlPC reports the release is marked in the M1D07C record as run beside a 4-worker neighbour, and the timing plan reads those seconds as high.
