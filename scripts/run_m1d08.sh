#!/usr/bin/env bash
# M1D08: the cells near the pass line on a P-core and an E-core. Protocol: docs/experiments/m1d08-core-type.md. One process at a time.
# Cells, arms and repeats from results/gate1-M1D08/cells.txt, cheapest first. For repeat r, the cell's arms in Gate 1's order rotated by r, each on both
# cores back to back, P first when r plus the arm's position is even. After each cell, every run against the recorded first repeat and against the other
# arms, per batch, by scripts/m1d08_check.py. Hard rule 1. A finished file is never run again, so a claim cut by the line is completed by the next.
# Usage: LINE_HOURS=7 scripts/run_m1d08.sh. TOUT and CELLS may be set, and NOREF=1, only to dry-run on fixtures. A real run sets none of them.
set -u
export LC_ALL=C
tout="${TOUT:-results/gate1-M1D08}"
cells="${CELLS:-results/gate1-M1D08/cells.txt}"
pcpu=8
ecpu=16
timing=build/gate1/timing.exe
started=$(date +%s)
line=$((${LINE_HOURS:-7} * 3600))   # Checked before each process.

over_line() {
  if [ $(($(date +%s) - started)) -ge "$line" ]; then echo "M1D08-LINE stopped at the line before $*"; exit 2; fi
}

grep -q "$(sha256sum "$timing" | cut -d' ' -f1)" results/gate1-M1D04/hashes.txt || { echo "M1D08-STOP timing.exe differs from M1D04"; exit 1; }
mkdir -p "$tout/P/raw" "$tout/E/raw"
if [ ! -s "$tout/hashes.txt" ]; then
  (git rev-parse --short HEAD; sha256sum benchmarks/gate1.cpp src/r11/tc.hpp scripts/run_m1d08.sh scripts/m1d08_check.py "$cells" "$timing") > "$tout/hashes.txt"
fi

run() {  # stream variant cut file arm repeat core
  local s=$1 v=$2 c=$3 f=$4 a=$5 r=$6 core=$7 th="" sfx="" cap=1200 cpu=$pcpu
  [ "$core" = E ] && cpu=$ecpu
  [ "$a" = M5 ] && { th="--theta 128"; sfx=-th128; }
  [ "$a" = A5 ] && cap=300
  [ -s "$tout/$core/raw/$s-$v-$c-p5-$a$sfx.t$r.summary.csv" ] && return 0
  over_line "$s $v $c $a repeat $r core $core"
  timeout -k 5 $((cap + 20)) "$timing" "$f" "$s" "$v" "$c" 5 "$a" "$tout/$core" $th --rep "$r" --cpu "$cpu" --cap-seconds "$cap" --cap-bytes 16000000000
}

# Windows Python and git may leave a CR at the end of a line. This bash expands $'\r' to nothing, so the CR is given to tr in octal (log.md 0053)
while read -r s v c f ref reps margin arms; do
  [ -n "$s" ] || continue
  [ -s "$f" ] || { echo "M1D08-STOP no data file $f"; exit 1; }
  list=()
  for a in R2 PR WT WJ M5 A5; do case ",$arms," in *",$a,"*) list+=("$a") ;; esac; done
  n=${#list[@]}
  for ((r = 1; r <= reps; ++r)); do
    for ((i = 0; i < n; ++i)); do
      a=${list[$(((i + r) % n))]}
      if [ $(((r + i) % 2)) -eq 0 ]; then first=P; second=E; else first=E; second=P; fi
      run "$s" "$v" "$c" "$f" "$a" "$r" "$first" || true
      run "$s" "$v" "$c" "$f" "$a" "$r" "$second" || true
    done
  done
  python -I scripts/m1d08_check.py "$tout" "$ref" "$s" "$v" "$c" "$reps" "$arms" || { echo "M1D08-STOP $s $v $c"; exit 1; }
  echo "CELL-DONE $s $v $c $(date +%H:%M:%S)"
done < <(grep -v '^#' "$cells" | tr -d '\015')
echo "M1D08-DONE $tout"
