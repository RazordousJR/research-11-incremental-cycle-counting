# M1D06A: M5 over the theta grid

Protocol: `docs/experiments/m1d06a-theta.md`, fixed before any run (`log.md` 0033). A sensitivity on the M1D05 verdict, not a second verdict.
Timing build, the same binary as M1D04, one process at a time, pinned to logical CPU 8, `--exclusive` claim on the PI's word (`log.md` 0034) and ControlPC's GO, from 02:05:57 to 04:55:33 on 2026-11-03, 2 h 49 min 36 s against a line of 5 hours. `M1D06A-DONE`, no restart. Every stream is labelled **real timestamps, synthetic expiry** or **real timestamps, insert-only**. None is fully synthetic.

## Result

- **All six passes hold.** With M5 at the theta that suits it best on each cell, chosen after the fact on the cell itself, every p = 5 pass of M1D05 still has PR's slowest repeat at most 0.67 times the fastest repeat of R2 and of M5 at any theta. 6 of 6 hold, so the M1D05 rule with fallen passes counted as fails still gives 6 of 14, Go.
- **Exact.** `mcheck` on all 70 M5 count runs, 0 differences. Every file of this claim against M1D04's and M1D03's files on the same cells: 802 pairs, 14,830,901 batch comparisons, 0 differ.
- **The binary did not change.** Hashes match after the run, and `timing.exe` and `count.exe` are the ones of M1D04.
- **Repeats** as the Gate 1 rule, 5 on every cell, and M5 at 1,024 once on wiki-talk expiry 1,000, where it passed 5 times the fastest arm.

## Theta

Engineering chose 128 for M1D04. On these cells the best is 16 on the sx expiry cells and 64 on the rest. The gain over 128 is 23% to 29% on sx expiry, 9% on sx-askubuntu insert, 3% on sx-superuser insert and 1% on wiki-talk expiry. M5 beats R2 on one stream only, sx-askubuntu insert, by 9%.

16 is the end of the grid, and on sx expiry M5 still falls as theta falls. That does not threaten the two expiry passes. As theta falls more vertices are scanned as hubs, so M5 tends to R2 with the hub lists on top, and R2 is already the best other arm there. As theta rises M5 tends to the table. On these cells the mix beat both of its ends only on sx-askubuntu insert.

## The clock in this claim

This claim was noisier than M1D04's. 12 repeats ran more than 3% over their own arm's median, up to 19%, scattered from 02:08 to 04:06, and on sx-superuser insert 100 R2's median rose 10.9% with its fastest repeat unchanged. ControlPC found afterwards (`log.md` 0036) that Research 1's scheduled tasks fire at every quarter hour through every claim, that its own mail runs at about :06:15, and listed one-off Windows and Office tasks. The 02:15, 02:45 and 04:06 slow repeats cover one of those, and 03:13 follows an Office task by 23 s. `scripts/overlap_check.py` finds 60 repeats overlapping a known event, median +0.07% and 6 over 3%, against 385 clean, median 0.00% and 6 over 3%. My guess of Windows maintenance is withdrawn. Research 12 reported, through ControlPC and unchecked, three concurrent ripgrep searches over C:\ for 20 s each, somewhere from 04:50:07 to 04:53:50. The last timed process ended at 04:28:44, so that fell in the count build, which carries no time, and its counts all matched. Noise of this kind only adds time, and the test takes PR's slowest repeat against the others' fastest, so it works against PR and cannot make a pass hold that should fall. It does make the near-miss margins at cut 100 worse than in M1D04, so those are not compared across the two claims. PR's medians sit within 1.6% of M1D04's on every cell.

## What this does not show

1. **The scan margin is untouched.** The two sx expiry passes hold against R2 at 0.612 and 0.616, about where M1D04 had them. R2 13% and 9% faster would still remove them, and only another machine tests that.
2. **No stronger table was built** (`log.md` 0033), and the tuned table would need about twice its speed.
3. **sx-stackoverflow and wiki-talk insert were left out,** as the protocol says. Neither has a pass to lose.

## Every cell

Fastest repeat of each M5 theta, in seconds. (1) is an arm run once by the repeat rule. A cell holds at a margin of 0.67 or less.

| Stream | Label | Cut | PR slowest | R2 fastest | M5 16 | M5 32 | M5 64 | M5 128 | M5 256 | M5 512 | M5 1024 | Best other | Margin | Holds |
|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|---:|---|
| sx-askubuntu | real timestamps, synthetic expiry | 1,000 | 0.279 | 0.456 | 0.624 | 0.680 | 0.764 | 0.871 | 0.889 | 0.889 | 0.889 | R2 | 0.612 | yes |
| sx-askubuntu | real timestamps, synthetic expiry | 100 | 0.433 | 0.479 | 0.652 | 0.709 | 0.796 | 0.917 | 0.939 | 0.937 | 0.937 | R2 | 0.904 | no |
| sx-askubuntu | real timestamps, insert-only | 1,000 | 5.761 | 11.849 | 13.048 | 11.257 | 10.739 | 11.784 | 14.019 | 17.342 | 20.481 | M5 at 64 | 0.536 | yes |
| sx-askubuntu | real timestamps, insert-only | 100 | 9.224 | 11.858 | 13.046 | 11.208 | 10.737 | 11.776 | 13.940 | 17.293 | 20.478 | M5 at 64 | 0.859 | no |
| sx-superuser | real timestamps, synthetic expiry | 1,000 | 0.474 | 0.770 | 1.079 | 1.157 | 1.279 | 1.396 | 1.476 | 1.480 | 1.480 | R2 | 0.616 | yes |
| sx-superuser | real timestamps, synthetic expiry | 100 | 0.723 | 0.799 | 1.119 | 1.196 | 1.327 | 1.441 | 1.541 | 1.542 | 1.543 | R2 | 0.905 | no |
| sx-superuser | real timestamps, insert-only | 1,000 | 13.415 | 31.769 | 41.785 | 35.016 | 32.518 | 33.628 | 37.020 | 41.844 | 47.823 | R2 | 0.422 | yes |
| sx-superuser | real timestamps, insert-only | 100 | 25.474 | 31.767 | 41.561 | 34.923 | 32.504 | 33.587 | 36.981 | 41.803 | 47.706 | R2 | 0.802 | no |
| wiki-talk-temporal | real timestamps, synthetic expiry | 1,000 | 11.214 | 35.927 | 50.659 | 46.728 | 44.836 | 45.328 | 48.050 | 52.856 | 63.542 (1) | R2 | 0.312 | yes |
| wiki-talk-temporal | real timestamps, synthetic expiry | 100 | 19.123 | 36.428 | 51.421 | 47.524 | 45.396 | 45.702 | 48.618 | 53.715 | 62.038 | R2 | 0.525 | yes |

| Stream | Label | Cut | PR median here | PR median M1D04 | R2 median here | R2 median M1D04 | Margin here | Margin with M1D04's PR and R2 |
|---|---|---:|---:|---:|---:|---:|---:|---:|
| sx-askubuntu | real timestamps, synthetic expiry | 1,000 | 0.270 | 0.268 | 0.457 | 0.456 | 0.612 | 0.592 |
| sx-askubuntu | real timestamps, synthetic expiry | 100 | 0.430 | 0.429 | 0.481 | 0.480 | 0.904 | 0.900 |
| sx-askubuntu | real timestamps, insert-only | 1,000 | 4.944 | 4.866 | 11.859 | 11.834 | 0.536 | 0.453 |
| sx-askubuntu | real timestamps, insert-only | 100 | 8.212 | 8.183 | 11.875 | 11.844 | 0.859 | 0.771 |
| sx-superuser | real timestamps, synthetic expiry | 1,000 | 0.472 | 0.469 | 0.770 | 0.768 | 0.616 | 0.616 |
| sx-superuser | real timestamps, synthetic expiry | 100 | 0.721 | 0.719 | 0.802 | 0.799 | 0.905 | 0.929 |
| sx-superuser | real timestamps, insert-only | 1,000 | 13.252 | 13.254 | 31.787 | 31.793 | 0.422 | 0.421 |
| sx-superuser | real timestamps, insert-only | 100 | 21.415 | 21.417 | 35.273 | 31.815 | 0.802 | 0.678 |
| wiki-talk-temporal | real timestamps, synthetic expiry | 1,000 | 11.086 | 10.921 | 36.275 | 36.261 | 0.312 | 0.305 |
| wiki-talk-temporal | real timestamps, synthetic expiry | 100 | 17.618 | 17.519 | 36.724 | 36.386 | 0.525 | 0.487 |
