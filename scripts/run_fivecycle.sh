#!/usr/bin/env bash
# M1C02, all 12 cells. Protocol: docs/experiments/screen-b-protocol.md, M1C02. One process at a time.
# Each arm of each cell is its own process, under a hard stop of 590 s. The binary stops itself at 540 s and writes what it has.
# RC runs last in a cell, because its budget reads the other arms. A finished arm is not run again.
# Usage: scripts/run_fivecycle.sh [outdir] [stream ...]
set -u
out="${1:-results/screen-c-M1C02}"
shift || true
streams=("$@")
if [ ${#streams[@]} -eq 0 ]; then streams=(CollegeMsg email-Eu-core-temporal sx-mathoverflow); fi
mkdir -p "$out/raw"
for s in "${streams[@]}"; do
  for v in expiry insert; do
    if [ "$v" = insert ]; then f="data/derived/$s.ins.bin"; else f="data/derived/$s.bin"; fi
    for c in 1000 100; do
      for a in R2 PR WJ PW R2B RC; do
        tag="$s-$v-$c-$a"
        if [ -s "$out/raw/$tag.summary.csv" ]; then continue; fi
        timeout 590 ./build/r11_fivecycle.exe "$f" "$s" "$v" "$c" "$a" "$out"
        rc=$?
        if [ $rc -ne 0 ]; then echo "NOT-FINISHED $tag exit=$rc"; fi
      done
    done
  done
done
echo "M1C02-DONE $out"
