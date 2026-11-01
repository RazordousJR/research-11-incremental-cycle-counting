#!/usr/bin/env bash
# M1D03: operation sweep, count build, evaluation cells. Protocol: docs/experiments/gate1-protocol.md. One process at a time. Cheap patterns and cheap cells first.
# M arms are left out: their theta comes from M1D04, and their count runs follow it there. A finished file is never run again, so the script can be restarted.
# Any count that differs between arms stops the run. Hard rule 1.
# Usage: scripts/run_m1d03.sh [outdir] [cpu]
# DATA and CELLS may be set, only to dry-run this script on synthetic streams. A real run sets neither.
set -u
export LC_ALL=C
out="${1:-results/gate1-M1D03}"
cpu="${2:-8}"
bin=build/gate1/count.exe
data="${DATA:-data/derived}"
cells="${CELLS:-sx-askubuntu:expiry sx-superuser:expiry wiki-talk-temporal:expiry sx-askubuntu:insert sx-superuser:insert wiki-talk-temporal:insert}"
mkdir -p "$out/raw"
started=$(date +%s)
line=$((8 * 3600))   # ControlPC's line for this claim. Checked at each cell boundary. A rerun skips what finished.

run() {
  local s=$1 v=$2 c=$3 p=$4 a=$5 f=$6
  local cap=1200
  [ "${a:0:1}" = A ] && cap=300
  [ -s "$out/raw/$s-$v-$c-p$p-$a.summary.csv" ] && return 0
  timeout -k 5 $((cap + 20)) "$bin" "$f" "$s" "$v" "$c" "$p" "$a" "$out" --cap-seconds "$cap" --cap-bytes 16000000000 --cpu "$cpu"
}

for p in 3 4 5; do
  case $p in
    3) arms="S3 P3 T3" ;;
    4) arms="S4 P4 T4 B4 A4" ;;
    5) arms="R2 PR WT WJ A5" ;;
  esac
  for cell in $cells; do
    s="${cell%%:*}"
    v="${cell##*:}"
    if [ "$v" = insert ]; then f="$data/$s.ins.bin"; else f="$data/$s.bin"; fi
    for c in 1000 100; do
      for a in $arms; do run "$s" "$v" "$c" "$p" "$a" "$f"; done
      python -I scripts/check_m1d02.py "$out" "$s" "$v" "$c" || { echo "M1D03-STOP $s $v $c"; exit 1; }
      echo "CELL-DONE p$p $s $v $c $(date -u +%H:%M:%S)"
      if [ $(($(date +%s) - started)) -ge "$line" ]; then echo "M1D03-LINE stopped at the 8-hour line after p$p $s $v $c"; exit 2; fi
    done
  done
done
echo "M1D03-DONE $out"
