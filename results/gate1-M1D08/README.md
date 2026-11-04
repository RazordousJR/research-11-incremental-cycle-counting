# M1D08: the cells near the pass line, on a P-core and an E-core

Protocol: `docs/experiments/m1d08-core-type.md`, committed as 3357061 before any run. Reading: `scripts/m1d08_score.py`, read once after release. Full output: `score.txt`. Overlap: `overlap.txt`. Log: `log.md` 0061.

**Outcome: Core-stable.** The pooled Go clause holds with the M1D08 P-core verdicts (28 of 50) and with the E-core verdicts (24 of 50). The E-core passes fewer cells, and all of the loss is on the wiki real-deletion streams.

## Run

| | |
|---|---|
| Claim | One `--exclusive --timing` claim, 12:10:37 to 18:15:24 on 2026-11-04, 6 h 05 min, inside the 7-hour line. PI's standing order (`log.md` 0056), ControlPC's GO |
| Work | 24 cells, 88 arm-cells, 5 repeats per arm per core: 440 runs on logical CPU 8 (P-core) and 440 on logical CPU 16 (E-core), every summary showing its CPU |
| Status | 880 of 880 OK. No CAP, no time or memory limit |
| Exactness | Every run equals, per batch, the recorded first repeat of its arm on its cell, and the longest run of its cell: 24 cells, 0 differ or missing, checked after each cell and again after release |
| Hashes | Every line of `hashes.txt` holds after the run: `timing.exe` is M1D04's |
| Unused | About 3.9 of the 10 hours. No second claim |

## Background load

ControlPC's run records give the mail at 14:06:30 to 14:07:37, 16:06:32 to 16:07:29 and 18:06:33 to 18:07:33. Research 1, Research 9 and Research 12 confirmed reading and editing only, and ControlPC ran no process. Research 1's scheduled tasks at :00, :15, :30 and :45 and BackupResults at 18:00 are taken from their schedules, with wide guessed lengths. Per core, 34 of 440 repeats overlap a window, at +0.01% (P) and +0.00% (E) against their arm's median, and the clean repeats at +0.00%. **No pass on either core has its best arm's fastest repeat inside a window.** Load can only add time, so no verdict here can come from it.

## Reading 1: the P-core reproduces the record

The P-core verdict equals the recorded one on 22 of 24 cells, and each arm's median is the recorded one to within 1%: PR 1.001, R2 1.000, WT 1.001, M5 0.999, A5 1.007, WJ 1.005. The machine did not drift between M1D04 and M1D07 and today.

The two cells that turned both sit at the line:

| Cell | Recorded | M1D08 P-core |
|---|---|---|
| soc-wiki-elec expiry 100 | pass, 0.628 | fail, 0.675, with a median score of 0.629: it fails on spread |
| plwiki real 100 | fail, 0.6702 | pass, 0.655 |

They cancel. The pooled P-core count is 28 of 50, as recorded, with 9 expiry passes in place of 10 and 8 real-deletion passes in place of 7. A cell counted only if it passes on both P-core measurements gives 27 of 50, and Go holds on that too.

## Reading 2: the cells whose verdict differs between the cores

Six of 24. The E-core is 1.55 to 1.60 times slower for PR and R2 by median, 1.80 for WT, 1.41 for M5, but not by the same factor on every stream:

| Cell | P-core | E-core | |
|---|---|---|---|
| frwiki real 1,000 | pass 0.666 | fail 0.729 | real deletions, worse on E |
| nlwiki real 1,000 | pass 0.648 | fail 0.701 | real deletions, worse on E |
| plwiki real 100 | pass 0.655 | fail 0.707 | real deletions, worse on E |
| soc-wiki-elec insert 100 | pass 0.654 | fail 0.673 | |
| tech-as-topology insert 100 | pass 0.656 | fail 0.727 | best is M5 on both |
| soc-wiki-elec expiry 100 | fail 0.675 | pass 0.649 | the only cell that turns the other way |

On every one of the ten wiki real-deletion cells, the margin is worse on the E-core than on the P-core, by 0.016 to 0.063. On the seven sx and reddit cells it is better on four, the expiry and insert cells at 1,000, for example sx-askubuntu expiry 1,000 from 0.592 to 0.541. It is worse on the other three, the two insert cells at 100 and soc-redditHyperlinks-body expiry 1,000. So the shared kernel's lead over the unshared scan depends on the core, and on the wiki graphs with real deletions it is smaller on the core with the smaller near caches.

## Reading 3: the pooled count per core

M1D07's pooled 50 cells, with M1D08's verdicts on the 24 tested cells and the 26 others kept at their recorded verdicts.

| | Pass | Streams | Expiry | Real deletions | Insert-only | Batch 1,000 | Batch 100 | Go, literal |
|---|---|---|---|---|---|---|---|---|
| Recorded (M1D07E) | 28 | 14 | 10 | 7 | 11 | 19 | 9 | holds |
| M1D08 P-core | 28 | 14 | 9 | 8 | 11 | 19 | 9 | **holds** |
| M1D08 E-core | 24 | 12 | 10 | 5 | 9 | 17 | 7 | **holds** |

The third of 50 is 16.67. On the E-core, frwiki and nlwiki drop out of the passing streams. Of the five E-core real-deletion passes, two are itwiki's, which M1D08 did not test because their margins of 0.482 and 0.426 lie outside the window.

## Read beside it

- **Untested.** 26 cells sit outside the window [0.558, 0.804] or under 0.05 s and keep their recorded verdicts. They are listed in `score.txt`. The four ia-reality-call cells among them run in milliseconds, and one passes at exactly 0.667 on clock ticks.
- **Not a second machine.** The two core types share L3 and DRAM. A verdict held on the E-core does not show that the result carries to another memory hierarchy. A verdict turned does show it depends on the core.
- **The best arm.** R2 is best on 22 of 24 cells on the P-core and 21 of 24 on the E-core. M5 is best on tech-as-topology insert 100 and sx-askubuntu insert 100 on both cores, and on sx-superuser insert 100 on the E-core only.
- **Remedies.** `log.md` 0060 fixed remedies for a Not reproduced reading before the run. The reading is Core-stable, so none is used.
