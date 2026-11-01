# Gate 0, run 2026-11-01

Commit `d2efe26`. Protocol: `docs/experiments/gate0-protocol.md`, written before any stream was parsed. All
streams carry the label **real timestamps, synthetic expiry**. Operation counts, not time. `summary.md` holds
every table. One CSV per stream and cut.

**Verdict by the written rule: STOP.** All 36 cells are above 0.80. The range is 1.317 to 2.438, so the method
costs more than the baseline on every stream at every cut. No cell reaches the go threshold of 0.67.

Exactness: 0 mismatches across all policies, 7,232 static checks.

Check against Hanauer et al., Table 5.1, which prints rounded figures. A disagreement above 2% is reported:

| Stream | Applied updates, ours | Against theirs | Peak edges, ours | Against theirs |
|---|---|---|---|---|
| sx-mathoverflow | 489,520 | +6.9% | 2,620 | +4.8% |
| sx-askubuntu | 963,534 | +1.2% | 6,017 | -1.4% |
| sx-superuser | 1,524,966 | +1.7% | 7,165 | -1.8% |
| wiki-talk-temporal | 6,644,984 | +5.5% | 99,908 | -2.1% |

The cause of the gaps on sx-mathoverflow and wiki-talk-temporal was not found. Their text does not state every
preprocessing step. The streams are at the same scale, so the gaps do not bear on the verdict.

Why the method lost. Sequential scanning needs only 5.9 to 13.9 operations per net update at the 1,000-event
cut, so there is almost nothing to share. Fewer than 9% of net updates fell in groups where kernel O was chosen.
Batch overhead of a few operations per update therefore outweighs any saving. Even without that overhead, the
kernel-only ratio was 1.056 to 1.249.

What this does not establish: anything about Wikipedia link streams or sx-stackoverflow, which were not run.
No wall-clock time was measured.
