#!/usr/bin/env bash
# M1D06A: M5 over the theta grid, with PR and R2, on 10 evaluation cells at p = 5. Protocol: docs/experiments/m1d06a-theta.md. Under an --exclusive tower claim.
# Step 1 is one repeat of every arm on every cell, then the further repeats the protocol keeps. Step 2 is the count build of M5 at each theta.
# A finished file is never run again, so the script can be restarted. Any count that differs between arms stops the run. Hard rule 1.
# Usage: scripts/run_m1d06a.sh [outdir] [cpu]
# DATA, EVAL and THETAS may be set, only to dry-run this script on engineering streams. A real run sets none of them. LINE_HOURS is ControlPC's line.
set -u
export LC_ALL=C
out="${1:-results/gate1-M1D06A}"
cpu="${2:-8}"
timing=build/gate1/timing.exe
count=build/gate1/count.exe
data="${DATA:-data/derived}"
eval_cells="${EVAL:-sx-askubuntu:expiry sx-askubuntu:insert sx-superuser:expiry sx-superuser:insert wiki-talk-temporal:expiry}"
thetas="${THETAS:-16 32 64 128 256 512 1024}"
mkdir -p "$out/raw" "$out/count/raw"
started=$(date +%s)
line=$((${LINE_HOURS:-5} * 3600))   # Checked before each process. A rerun skips what finished.

over_line() {
  if [ $(($(date +%s) - started)) -ge "$line" ]; then echo "M1D06A-LINE stopped at the line before $*"; exit 2; fi
}

if [ ! -s "$out/hashes.txt" ]; then
  (git rev-parse --short HEAD; sha256sum benchmarks/gate1.cpp benchmarks/fivecycle.cpp tests/test_gate1.cpp src/r11/tc.hpp scripts/run_m1d06a.sh scripts/m1d06a.py scripts/check_m1d02.py scripts/gate1_plan.py "$count" "$timing") > "$out/hashes.txt"
fi

file_of() { if [ "$2" = insert ]; then echo "$data/$1.ins.bin"; else echo "$data/$1.bin"; fi; }

arms_of() {  # arm:theta, rotated by repeat
  local list=(PR:0 R2:0) th n i r=$1
  for th in $thetas; do list+=("M5:$th"); done
  n=${#list[@]}
  for ((i = 0; i < n; ++i)); do echo -n "${list[$(((i + r) % n))]} "; done
}

run_timing() {
  local s=$1 v=$2 c=$3 a=$4 th=$5 r=$6 f=$7 opt="" sfx=""
  if [ "$a" = M5 ]; then opt="--theta $th"; sfx="-th$th"; fi
  [ -s "$out/raw/$s-$v-$c-p5-$a$sfx.t$r.summary.csv" ] && return 0
  over_line "$s $v $c $a$sfx repeat $r"
  timeout -k 5 1220 "$timing" "$f" "$s" "$v" "$c" 5 "$a" "$out" $opt --rep "$r" --cpu "$cpu" --cap-seconds 1200 --cap-bytes 16000000000
}

# Step 1.
for cell in $eval_cells; do
  s="${cell%%:*}"; v="${cell##*:}"; f=$(file_of "$s" "$v")
  for c in 1000 100; do
    for spec in $(arms_of 1); do run_timing "$s" "$v" "$c" "${spec%%:*}" "${spec##*:}" 1 "$f"; done
    for spec in $(python -I scripts/m1d06a.py repeats "$out" "$s" "$v" "$c"); do
      a="${spec%%:*}"; rest="${spec#*:}"; th="${rest%%:*}"; r="${rest##*:}"
      run_timing "$s" "$v" "$c" "$a" "$th" "$r" "$f"
    done
    python -I scripts/check_m1d02.py "$out" "$s" "$v" "$c" || { echo "M1D06A-STOP $s $v $c"; exit 1; }
    echo "CELL-DONE $s $v $c $(date +%H:%M:%S)"
  done
done

# Step 2.
for th in $thetas; do
  for cell in $eval_cells; do
    s="${cell%%:*}"; v="${cell##*:}"; f=$(file_of "$s" "$v")
    for c in 1000 100; do
      if [ ! -s "$out/count/raw/$s-$v-$c-p5-M5-th$th.summary.csv" ]; then
        over_line "count $s $v $c theta $th"
        timeout -k 5 1220 "$count" "$f" "$s" "$v" "$c" 5 M5 "$out/count" --theta "$th" --cpu "$cpu" --cap-seconds 1200 --cap-bytes 16000000000
      fi
      python -I scripts/gate1_plan.py mcheck "$out" "$s" "$v" "$c" 5 "$th" || { echo "M1D06A-STOP count $s $v $c theta $th"; exit 1; }
    done
  done
done
echo "M1D06A-DONE $out"
