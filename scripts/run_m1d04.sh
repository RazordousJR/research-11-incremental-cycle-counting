#!/usr/bin/env bash
# M1D04: theta selection, then the timing sweep, under an --exclusive tower claim. Protocol: docs/experiments/gate1-protocol.md. Timing build, pinned, one process at a time.
# Step 1 picks one theta per pattern on the engineering cells. Step 2 is one repeat of every arm on every evaluation cell. Step 3 is the further repeats, for the arms the
# protocol keeps. Step 4 is the count build of the M arms at the chosen theta. A finished file is never run again, so the script can be restarted.
# Any count that differs between arms stops the run. Hard rule 1.
# Usage: scripts/run_m1d04.sh [outdir] [cpu]
# DATA, ENG and EVAL may be set, only to dry-run this script on synthetic streams. A real run sets none of them.
set -u
export LC_ALL=C
out="${1:-results/gate1-M1D04}"
cpu="${2:-8}"
timing=build/gate1/timing.exe
count=build/gate1/count.exe
data="${DATA:-data/derived}"
eng="${ENG:-CollegeMsg:expiry CollegeMsg:insert email-Eu-core-temporal:expiry email-Eu-core-temporal:insert sx-mathoverflow:expiry sx-mathoverflow:insert}"
eval_cells="${EVAL:-sx-askubuntu:expiry sx-superuser:expiry wiki-talk-temporal:expiry sx-askubuntu:insert sx-superuser:insert wiki-talk-temporal:insert}"
grid="16 32 64 128 256 512 1024"
mkdir -p "$out/raw" "$out/theta/raw"

file_of() { if [ "$2" = insert ]; then echo "$data/$1.ins.bin"; else echo "$data/$1.bin"; fi; }

arms_of() {  # rotated by repeat
  local list r=$2
  case $1 in
    3) list=(S3 P3 T3 M3) ;;
    4) list=(S4 P4 T4 M4 B4 A4) ;;
    5) list=(R2 PR WT WJ M5 A5) ;;
  esac
  local n=${#list[@]} i
  for ((i = 0; i < n; ++i)); do echo -n "${list[$(((i + r) % n))]} "; done
}

theta_of() { python -I -c "import sys; print(dict(l.split() for l in open(sys.argv[1]))[sys.argv[2]])" "$out/theta.txt" "$1"; }

# Step 1.
if [ ! -s "$out/theta.txt" ]; then
  for p in 3 4 5; do
    for th in $grid; do
      for cell in $eng; do
        s="${cell%%:*}"; v="${cell##*:}"; f=$(file_of "$s" "$v")
        for c in 1000 100; do
          [ -s "$out/theta/raw/$s-$v-$c-p$p-M$p-th$th.t1.summary.csv" ] && continue
          timeout -k 5 620 "$timing" "$f" "$s" "$v" "$c" "$p" "M$p" "$out/theta" --theta "$th" --rep 1 --cpu "$cpu" --cap-seconds 600 --cap-bytes 16000000000
        done
      done
    done
  done
  python -I scripts/gate1_plan.py theta "$out" || { echo "M1D04-STOP theta"; exit 1; }
fi

run_timing() {
  local s=$1 v=$2 c=$3 p=$4 a=$5 r=$6 f=$7 th="" sfx=""
  if [ "${a:0:1}" = M ]; then th="--theta $(theta_of "$p")"; sfx="-th$(theta_of "$p")"; fi
  local cap=1200
  [ "${a:0:1}" = A ] && cap=300
  [ -s "$out/raw/$s-$v-$c-p$p-$a$sfx.t$r.summary.csv" ] && return 0
  timeout -k 5 $((cap + 20)) "$timing" "$f" "$s" "$v" "$c" "$p" "$a" "$out" $th --rep "$r" --cpu "$cpu" --cap-seconds "$cap" --cap-bytes 16000000000
}

for p in 3 4 5; do
  for cell in $eval_cells; do
    s="${cell%%:*}"; v="${cell##*:}"; f=$(file_of "$s" "$v")
    for c in 1000 100; do
      for a in $(arms_of "$p" 1); do run_timing "$s" "$v" "$c" "$p" "$a" 1 "$f"; done
      for spec in $(python -I scripts/gate1_plan.py repeats "$out" "$s" "$v" "$c" "$p"); do
        a="${spec%%:*}"; r="${spec##*:}"
        run_timing "$s" "$v" "$c" "$p" "$a" "$r" "$f"
      done
      python -I scripts/check_m1d02.py "$out" "$s" "$v" "$c" || { echo "M1D04-STOP $s $v $c p$p"; exit 1; }
      echo "CELL-DONE p$p $s $v $c $(date -u +%H:%M:%S)"
    done
  done
done

# Step 4.
for p in 3 4 5; do
  th=$(theta_of "$p")
  for cell in $eval_cells; do
    s="${cell%%:*}"; v="${cell##*:}"; f=$(file_of "$s" "$v")
    for c in 1000 100; do
      [ -s "$out/count/raw/$s-$v-$c-p$p-M$p-th$th.summary.csv" ] && continue
      mkdir -p "$out/count/raw"
      timeout -k 5 1220 "$count" "$f" "$s" "$v" "$c" "$p" "M$p" "$out/count" --theta "$th" --cpu "$cpu" --cap-seconds 1200 --cap-bytes 16000000000
    done
  done
done
echo "M1D04-DONE $out"
