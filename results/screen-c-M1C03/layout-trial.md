# M1C03: how WT was chosen

**Correction note, added after the runs.** Parts of this page are wrong, including its next sentence: it was written while a mistaken run was reading real streams, though every layout
decision had been made. Four corrections are listed under Process notes in `README.md`, which governs. The text below is left as first written.

Written before the source was fixed and before any protocol stream was read by this test. Non-citable. Label of every stream on this page: **fully synthetic**.
Nothing here supports a claim (hard rule 4), and the rule does not read it.

## Why synthetic streams

All six protocol streams are scored in M1C03, so no stream is left over as an engineering stream. Choosing a layout on any of them would break hard rule 5. The layouts were
therefore tried on R-MAT streams generated in the scratchpad. They exist only to rank layouts, and they say nothing about which arm wins on real data.

| Name | Scale | Events, insert form | Skew a | Window for the expiry form, events | Vertices touched |
|---|---:|---:|---:|---:|---|
| s | 12 | 40,000 | 0.57 | 4,000 | up to 4,096 |
| m | 15 | 200,000 | 0.57 | 20,000 | up to 32,768 |
| f | 17 | 400,000 | 0.45 | 40,000 | up to 131,072 |

Every variant gave the same final count as WJ on every stream, and every run was pinned to one P-core. The trial driver was a scratch program with a pair of clock reads around each
kernel call, so its times are slightly above those of the fixed source.

## The layouts

| Variant | Storage | Slot | Join |
|---|---|---|---|
| WJ | flat table, one key per pair | 12 bytes | d(u) x d(v) probes, prefetch 12 ahead |
| half8 | one table per first vertex, each pair once | 8 bytes | sorted lists, each row local, the other half hops between tables |
| sym8 | one table per vertex, each pair in both tables | 8 bytes | outer loop on the shorter list, probes stay in one table |
| sym4 | as sym8 | 4 bytes, packed key and count, side table for saturated counts | as sym8 |
| sym4s | as sym4 | as sym4 | SSE2 window of four slots, no branch per probe |
| sym4sp, the choice | as sym4 | as sym4 | as sym4s, with prefetch, and a scan of the table against a bitset when 4 q >= capacity |

## Results, seconds and peak table MB, cut of 1,000

| Stream | WJ | half8 | sym8 | sym4s | sym4sp, first form | Fixed source |
|---|---:|---:|---:|---:|---:|---:|
| m insert | 47 s | 25.2 s, 346 MB | 28.9 s, 689 MB | 15.0 s, 345 MB | 13.4 s | **11.5 s, 365 MB** |
| m expiry | | 3.68 s, 57 MB | 3.78 s, 112 MB | 2.00 s, 28 MB | 1.63 s | **1.74 s, 32 MB** |
| f insert | 11.8 s | | | | 4.07 s | **3.57 s, 393 MB** |
| f expiry | 0.60 s | | | | 0.68 s | **0.55 s, 11 MB** |

For scale, and not as a result: PR took 1.9 s on m insert and 1.05 s on f insert, and R2 3.6 s and 1.25 s. WT did not catch either row kernel on these streams, which is the reason
the rest of the page exists: I kept tuning until the gains ran out, and did not stop at the first layout that beat WJ.

## What each step gave

| Step | Effect | Kept |
|---|---|---|
| Packed 4 byte slot, in place of 8 | Half the memory. About a third off the time at the same layout, from denser cache lines | Yes |
| SSE2 window on reads | 15.0 s against about 25 s on m insert: the probe branch mispredicts once per read | Yes |
| Each pair in both tables, against half storage | With an 8 byte slot about equal in time at twice the memory, but only symmetric storage keeps every join inside one table, which is what the later steps need | Yes |
| Scan of the table against a bitset, 4 q >= capacity | 2% to 5%. Ratios 1, 4 and 16 tried, 4 best | Yes |
| Join prefetch distance 16, tables of 2^12 slots or more | 12.05 s to 11.10 s on m insert. Thresholds 2^10, 2^14 and none tried | Yes |
| Hash-sorted inner list, so each table is walked front to back | None. 3% worse on the streams tried | **No** |
| Size-class pool | 11% on f insert, none elsewhere | Yes |
| Shrink below 1/16 load, against 1/8 and 1/32 | Within noise on time, less rehash churn | Yes |
| Pre-sizing a table for a whole list | 2% to 4% | Yes |
| SSE2 window on writes | 6% on f insert | Yes |
| Smallest table 8 slots, against 4, 16, 32 | Within noise | Yes |
| Upkeep in blocks: prefetch directory, then slots, then write | 9% on f expiry, 3% worse on m expiry, none elsewhere. Kept, since short lists are where the plain form could not prefetch at all | Yes |

## What was not tried

- **A table that also falls back to a row scan on one update.** That is a different kernel, a mix of WT and R2, and not a layout of the same table. The protocol asks for the same table with a
  cache-aware layout, so it stays out of WT.
- **AVX2.** It would need a flag beyond the project's. SSE2 is in the x86-64 baseline.
- **A filter in front of the table.** Hard rule 1 rules out a Bloom filter in the count path.

## Row kernels, measured the same way

Prefetching list headers and list data helps both R2 and PR, so it stays. Medians of 3, seconds, cut of 1,000.

| Stream | R2 with | R2 without | PR with | PR without |
|---|---:|---:|---:|---:|
| m insert | 3.67 | 3.93 | 1.91 | 2.07 |
| f insert | 1.25 | 1.73 | 1.05 | 1.42 |
| m expiry | 1.22 | 1.34 | 0.70 | 0.76 |
| f expiry | 0.37 | 0.43 | 0.40 | 0.44 |

## Why a separate timing binary

Medians of 3, m insert, cut of 1,000, seconds.

| Arm | Count build | Timing build | Counters cost |
|---|---:|---:|---:|
| R2 | 4.16 | 3.64 | 14% |
| PR | 2.22 | 1.91 | 16% |
| WJ | 47.4 | 46.9 | 1% |
| PW | 44.6 | 42.2 | 6% |

Counters would have charged the row arms far more than the table arms, so the distortion is real and it is one-sided. All clock figures in this test come from the timing build.
