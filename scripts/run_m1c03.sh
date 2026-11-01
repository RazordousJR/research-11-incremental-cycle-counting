#!/usr/bin/env bash
# M1C03, all 24 cells. Protocol: docs/experiments/screen-b-protocol.md, M1C03. One process at a time, each its own process.
# Cheap cells first. For each cell:
#   timing repeat 1 of every arm, then the count build of every arm that finished, then the live check, then timing repeats 2 and 3.
# Arms are interleaved across repeats in a rotated order, so that slow drift on a shared machine does not fall on one arm.
# An arm that does not finish repeat 1 is not finished on that cell, and it is not run again. The limits live in the binary: a job object
# refuses commit past 6e9 bytes and a watchdog thread stops the process 600 s after it started, leaving a partial record. timeout is a third
# stop outside it. A finished file is never run again, so the script can be restarted.
# Any exactness failure, or any count that differs from M1C02, stops the whole run. Hard rule 1.
# Usage: scripts/run_m1c03.sh [outdir] [cpu]
# DATA and CELLS may be set, only to dry-run this script on synthetic streams. A real run sets neither.
set -u
export LC_ALL=C
out="${1:-results/screen-c-M1C03}"
cpu="${2:-8}"
timing=build/m1c03/timing.exe
count=build/m1c03/count.exe
mkdir -p "$out/raw"

status_of() {  # the status column of a summary file, or nothing
  [ -s "$1" ] || return 0
  python -I -c "import csv,sys; print(next(csv.DictReader(open(sys.argv[1], newline='')))['status'])" "$1"
}

arms_for() {  # rotate the arm order by the repeat number
  case "$1" in
    1) echo "R2 PR WJ WT PW" ;;
    2) echo "PR WJ WT PW R2" ;;
    3) echo "WJ WT PW R2 PR" ;;
  esac
}

run_timing() {
  local s=$1 v=$2 c=$3 a=$4 r=$5 f=$6 tag="$1-$2-$3-$4"
  [ -s "$out/raw/$tag.t$r.summary.csv" ] && return 0
  timeout -k 5 620 "$timing" "$f" "$s" "$v" "$c" "$a" "$out" --rep "$r" --cpu "$cpu" --cap-seconds 600 --cap-bytes 6000000000
  local rc=$?
  if [ $rc -ne 0 ]; then echo "NOT-FINISHED timing $tag rep $r exit=$rc"; fi
}

run_count() {
  local s=$1 v=$2 c=$3 a=$4 f=$5 tag="$1-$2-$3-$4"
  [ -s "$out/raw/$tag.summary.csv" ] && return 0
  timeout -k 5 620 "$count" "$f" "$s" "$v" "$c" "$a" "$out" --cpu "$cpu" --cap-seconds 600 --cap-bytes 6000000000
  local rc=$?
  if [ $rc -ne 0 ]; then echo "NOT-FINISHED count $tag exit=$rc"; fi
}

check() {
  python -I scripts/check_m1c03.py "$out" "$1" "$2" "$3"
  if [ $? -ne 0 ]; then echo "M1C03-STOP exactness or count check failed at $1 $2 $3"; exit 1; fi
}

data="${DATA:-data/derived}"
cells="${CELLS:-CollegeMsg:expiry CollegeMsg:insert email-Eu-core-temporal:expiry email-Eu-core-temporal:insert sx-mathoverflow:expiry sx-askubuntu:expiry sx-superuser:expiry sx-mathoverflow:insert wiki-talk-temporal:expiry sx-askubuntu:insert sx-superuser:insert wiki-talk-temporal:insert}"
for cell in $cells; do
  s="${cell%%:*}"
  v="${cell##*:}"
  if [ "$v" = insert ]; then f="$data/$s.ins.bin"; else f="$data/$s.bin"; fi
  for c in 1000 100; do
    for a in $(arms_for 1); do run_timing "$s" "$v" "$c" "$a" 1 "$f"; done
    for a in $(arms_for 1); do
      if [ "$(status_of "$out/raw/$s-$v-$c-$a.t1.summary.csv")" = OK ]; then run_count "$s" "$v" "$c" "$a" "$f"; fi
    done
    check "$s" "$v" "$c"
    for r in 2 3; do
      for a in $(arms_for "$r"); do
        if [ "$(status_of "$out/raw/$s-$v-$c-$a.t1.summary.csv")" = OK ]; then run_timing "$s" "$v" "$c" "$a" "$r" "$f"; fi
      done
    done
    check "$s" "$v" "$c"
    echo "CELL-DONE $s $v $c $(date -u +%H:%M:%S)"
  done
done
echo "M1C03-DONE $out"
