#!/usr/bin/env bash
# M1D10: every arm timed on the 12 engineering cells, then counted. Protocol: docs/experiments/m1d10-cost-engineering.md. One process at a time, CPU 8.
# The timing is scripts/run_m1d04.sh's steps 2 to 4, unchanged, with M1D04's theta.txt and the engineering cells as its cells, so step 1 is skipped. Then the
# count build of every other arm, so the counters and the clock come from one binary, and every file against every other per batch. Hard rule 1.
# A finished file is never run again, so a claim cut by the line is completed by the next.
# Usage: LINE_HOURS=2 scripts/run_m1d10.sh [outdir]. ENG may be set, only to dry-run on one engineering stream. A real run sets none of it.
set -u
export LC_ALL=C
out="${1:-results/gate1-M1D10}"
cpu=8
timing=build/gate1/timing.exe
count=build/gate1/count.exe
data=data/derived
eng="${ENG:-CollegeMsg:expiry CollegeMsg:insert email-Eu-core-temporal:expiry email-Eu-core-temporal:insert sx-mathoverflow:expiry sx-mathoverflow:insert}"
started=$(date +%s)
line=$((${LINE_HOURS:-2} * 3600))   # Checked before each count process. run_m1d04.sh checks its own, from a later start.

over_line() {
  if [ $(($(date +%s) - started)) -ge "$line" ]; then echo "M1D10-LINE stopped at the line before $*"; exit 2; fi
}

for exe in "$timing" "$count"; do
  grep -q "$(sha256sum "$exe" | cut -d' ' -f1)" results/gate1-M1D04/hashes.txt || { echo "M1D10-STOP $exe differs from M1D04"; exit 1; }
done
mkdir -p "$out/raw" "$out/count/raw"
[ -s "$out/theta.txt" ] || cp results/gate1-M1D04/theta.txt "$out/theta.txt"
if [ ! -s "$out/hashes.txt" ]; then   # run_m1d04.sh writes its own only when this is missing
  (git rev-parse --short HEAD; sha256sum benchmarks/gate1.cpp src/r11/tc.hpp scripts/run_m1d10.sh scripts/run_m1d04.sh scripts/gate1_plan.py scripts/check_m1d02.py scripts/gate1_score.py \
    "$timing" "$count" "$out/theta.txt" "$data"/CollegeMsg*.bin "$data"/email-Eu-core-temporal*.bin "$data"/sx-mathoverflow*.bin) > "$out/hashes.txt"
fi

EVAL="$eng" LINE_HOURS="${LINE_HOURS:-2}" bash scripts/run_m1d04.sh "$out" "$cpu" main
rc=$?
[ $rc -eq 0 ] || { echo "M1D10-STOP run_m1d04.sh exit $rc"; exit $rc; }

file_of() { if [ "$2" = insert ]; then echo "$data/$1.ins.bin"; else echo "$data/$1.bin"; fi; }

for p in 3 4 5; do
  case $p in
    3) arms="S3 P3 T3" ;;
    4) arms="S4 P4 T4 B4 A4" ;;
    5) arms="R2 PR WT WJ A5" ;;
  esac
  for cell in $eng; do
    s="${cell%%:*}"; v="${cell##*:}"; f=$(file_of "$s" "$v")
    for c in 1000 100; do
      for a in $arms; do
        cap=1200
        [ "${a:0:1}" = A ] && cap=300
        [ -s "$out/count/raw/$s-$v-$c-p$p-$a.summary.csv" ] && continue
        over_line "count $s $v $c p$p $a"
        timeout -k 5 $((cap + 20)) "$count" "$f" "$s" "$v" "$c" "$p" "$a" "$out/count" --cpu "$cpu" --cap-seconds "$cap" --cap-bytes 16000000000
      done
    done
  done
done

python -I scripts/gate1_score.py exact "$out" results/gate1-M1D02B || { echo "M1D10-STOP exact"; exit 1; }
echo "M1D10-DONE $out"
