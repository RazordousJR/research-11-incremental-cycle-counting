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
