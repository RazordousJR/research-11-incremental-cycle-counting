#!/usr/bin/env bash
# M1D02: regression against M1C03 and agreement of all arms, count build, engineering cells only. Protocol: docs/experiments/gate1-protocol.md.
# Theta is 64 here. It cannot change a count, and no timing is read. A process stopped by its limit leaves a partial record, and the checker compares what finished.
# Usage: scripts/run_m1d02.sh [outdir] [cap seconds]
set -u
export LC_ALL=C
out="${1:-results/gate1-M1D02}"
cap="${2:-120}"
bin=build/gate1/count.exe
mkdir -p "$out/raw"

run() {
  local s=$1 v=$2 c=$3 p=$4 a=$5 f=$6 th=""
  case "$a" in M*) th="--theta 64" ;; esac
  local tag="$s-$v-$c-p$p-$a"
  [ "${a:0:1}" = M ] && tag="$tag-th64"
  [ -s "$out/raw/$tag.summary.csv" ] && return 0
  timeout -k 5 $((cap + 20)) "$bin" "$f" "$s" "$v" "$c" "$p" "$a" "$out" $th --cap-seconds "$cap" --cap-bytes 6000000000 --cpu 8
}

for cell in CollegeMsg:expiry CollegeMsg:insert email-Eu-core-temporal:expiry email-Eu-core-temporal:insert sx-mathoverflow:expiry sx-mathoverflow:insert; do
  s="${cell%%:*}"
  v="${cell##*:}"
  if [ "$v" = insert ]; then f="data/derived/$s.ins.bin"; else f="data/derived/$s.bin"; fi
  for c in 1000 100; do
    for a in S3 P3 T3 M3; do run "$s" "$v" "$c" 3 "$a" "$f"; done
    for a in S4 P4 T4 M4 B4 A4; do run "$s" "$v" "$c" 4 "$a" "$f"; done
    for a in R2 PR WT WJ M5 A5; do run "$s" "$v" "$c" 5 "$a" "$f"; done
    python -I scripts/check_m1d02.py "$out" "$s" "$v" "$c" || { echo "M1D02-STOP $s $v $c"; exit 1; }
    echo "CELL-DONE $s $v $c"
  done
done
echo "M1D02-DONE"
