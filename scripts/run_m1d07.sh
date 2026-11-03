#!/usr/bin/env bash
# M1D07: more streams at p = 3 and p = 5. Protocol: docs/experiments/m1d07-streams.md. One process at a time, pinned.
# Phase derive: the format check and the derived streams, from data/m1d07/raw into data/m1d07/derived. A Network Repository file is tried
# as an interaction file first, and as a real-deletion file only if the loader says its weights carry deletions.
# Phase count: M1D07C, the count sweep, every arm at p = 3 and then p = 5, cheap streams first. M at M1D04's thetas. Each cell's arms
# must agree per batch, or the run stops. Hard rule 1. A finished file is never run again, so the script can be restarted under a new claim.
# Phase timing: M1D07D, the timing build under an --exclusive claim, on the cells of the protocol's phases named in PHASES, in order.
# Repeat 1 of every arm in a rotated order, then the further repeats by scripts/gate1_plan.py, then per-batch agreement among all runs
# of the cell and against M1D07C's count build. Output in TOUT, default results/gate1-M1D07T.
# Usage: scripts/run_m1d07.sh derive|count [outdir] [cpu]; PHASES="D1 D2" scripts/run_m1d07.sh timing [count outdir] [cpu]
# SKIP names streams held out of this run, by the PI's word or while the PI decides (log.md 0046).
# RAW, DATA and CELLS may be set, only to dry-run this script on fixtures and an engineering stream. A real run sets none of them. LINE_HOURS is ControlPC's line.
set -u
export LC_ALL=C
phase="${1:-}"
out="${2:-results/gate1-M1D07}"
cpu="${3:-8}"
count=build/gate1/count.exe
raw="${RAW:-data/m1d07/raw}"
data="${DATA:-data/m1d07/derived}"
started=$(date +%s)
line=$((${LINE_HOURS:-8} * 3600))   # Checked before each process.

over_line() {
  if [ $(($(date +%s) - started)) -ge "$line" ]; then echo "M1D07-LINE stopped at the line before $*"; exit 2; fi
}

# stream format archive, in the order the count sweep takes them: small and expiring first, the KONECT prefixes last.
STREAMS="
ia-reality-call nr ia-reality-call.zip
soc-wiki-elec nr soc-wiki-elec.zip
tech-as-topology nr tech-as-topology.zip
ia-enron-email-dynamic nr ia-enron-email-dynamic.zip
soc-redditHyperlinks-body reddit soc-redditHyperlinks-body.tsv
soc-redditHyperlinks-title reddit soc-redditHyperlinks-title.tsv
soc-youtube-growth nr soc-youtube-growth.zip
simplewiki konect download.tsv.link-dynamic-simplewiki.tar.bz2
nlwiki konect download.tsv.link-dynamic-nlwiki.tar.bz2
plwiki konect download.tsv.link-dynamic-plwiki.tar.bz2
itwiki konect download.tsv.link-dynamic-itwiki.tar.bz2
frwiki konect download.tsv.link-dynamic-frwiki.tar.bz2
dewiki konect download.tsv.link-dynamic-dewiki.tar.bz2
"

derive() {
  mkdir -p "$data"
  (cd "$raw" && sha256sum -c --quiet SHA256SUMS) || { echo "M1D07-STOP checksums"; exit 1; }
  echo "$STREAMS" | while read -r s fmt arc; do
    [ -n "$s" ] || continue
    [[ " ${SKIP:-} " == *" $s "* ]] && { echo "M1D07-SKIP $s"; continue; }
    if [ "$fmt" = konect ]; then variants=real; else variants="expiry insert"; fi
    for v in $variants; do
      [ -s "$data/$s.$v.bin" ] && continue
      msg=$(python -I scripts/prepare_m1d07_stream.py "$fmt" "$raw/$arc" "$data/$s.$v.bin" "$v")
      rc=$?
      echo "$msg"
      [ $rc -eq 5 ] && { echo "M1D07-STOP RAM during $s"; exit 1; }
      if [ $rc -eq 3 ] && [ "$fmt" = nr ] && [[ $msg == *real-deletion* ]]; then
        python -I scripts/prepare_m1d07_stream.py nr "$raw/$arc" "$data/$s.real.bin" real || echo "M1D07-DROP $s real"
        break
      fi
      [ $rc -eq 0 ] || echo "M1D07-DROP $s $v exit $rc"
    done
  done || return 1
  echo "M1D07-DERIVED $data"
}

cells() {   # stream:variant, in sweep order, for every derived stream
  if [ -n "${CELLS:-}" ]; then echo "$CELLS"; return; fi
  echo "$STREAMS" | while read -r s fmt arc; do
    [ -n "$s" ] || continue
    [[ " ${SKIP:-} " == *" $s "* ]] && continue
    for v in expiry insert real; do [ -s "$data/$s.$v.bin" ] && echo -n "$s:$v "; done
  done
}

run() {
  local s=$1 v=$2 c=$3 p=$4 a=$5 f=$6 th="" sfx=""
  case $a in M3) th="--theta 16"; sfx=-th16 ;; M5) th="--theta 128"; sfx=-th128 ;; esac
  local cap=1200
  [ "${a:0:1}" = A ] && cap=300
  [ -s "$out/raw/$s-$v-$c-p$p-$a$sfx.summary.csv" ] && return 0
  over_line "$s $v $c p$p $a"
  timeout -k 5 $((cap + 20)) "$count" "$f" "$s" "$v" "$c" "$p" "$a" "$out" $th --cap-seconds "$cap" --cap-bytes 16000000000 --cpu "$cpu"
}

sweep() {
  mkdir -p "$out/raw"
  if [ ! -s "$out/hashes.txt" ]; then
    (git rev-parse --short HEAD; sha256sum benchmarks/gate1.cpp src/r11/tc.hpp scripts/prepare_m1d07_stream.py scripts/run_m1d07.sh scripts/check_m1d02.py "$count") > "$out/hashes.txt"
  fi
  local list
  list=$(cells)
  for p in 3 5; do
    if [ "$p" = 3 ]; then arms="S3 P3 T3 M3"; else arms="R2 PR WT WJ M5 A5"; fi
    for cell in $list; do
      s="${cell%%:*}"; v="${cell##*:}"; f="$data/$s.$v.bin"
      for c in 1000 100; do
        for a in $arms; do run "$s" "$v" "$c" "$p" "$a" "$f"; done
        python -I scripts/check_m1d02.py "$out" "$s" "$v" "$c" || { echo "M1D07-STOP $s $v $c p$p"; exit 1; }
        echo "CELL-DONE p$p $s $v $c $(date +%H:%M:%S)"
      done
    done
  done
  echo "M1D07-DONE count $out"
}

arms_of() {  # rotated by repeat, as run_m1d04.sh
  local list r=$2
  case $1 in
    3) list=(S3 P3 T3 M3) ;;
    5) list=(R2 PR WT WJ M5 A5) ;;
  esac
  local n=${#list[@]} i
  for ((i = 0; i < n; ++i)); do echo -n "${list[$(((i + r) % n))]} "; done
}

run_timing() {
  local s=$1 v=$2 c=$3 p=$4 a=$5 r=$6 f=$7 th="" sfx=""
  case $a in M3) th="--theta 16"; sfx=-th16 ;; M5) th="--theta 128"; sfx=-th128 ;; esac
  local cap=1200
  [ "${a:0:1}" = A ] && cap=300
  [ -s "$tout/raw/$s-$v-$c-p$p-$a$sfx.t$r.summary.csv" ] && return 0
  over_line "$s $v $c p$p $a repeat $r"
  timeout -k 5 $((cap + 20)) "$timing" "$f" "$s" "$v" "$c" "$p" "$a" "$tout" $th --rep "$r" --cpu "$cpu" --cap-seconds "$cap" --cap-bytes 16000000000
}

timing_sweep() {
  timing=build/gate1/timing.exe
  tout="${TOUT:-results/gate1-M1D07T}"
  [ -n "${PHASES:-}" ] || { echo "timing needs PHASES"; exit 2; }
  mkdir -p "$tout/raw"
  if [ ! -s "$tout/hashes.txt" ]; then
    (git rev-parse --short HEAD; sha256sum benchmarks/gate1.cpp src/r11/tc.hpp scripts/run_m1d07.sh scripts/gate1_plan.py scripts/check_m1d02.py scripts/m1d07_tcheck.py scripts/m1d07_plan.py "$timing") > "$tout/hashes.txt"
  fi
  grep -q "$(sha256sum "$timing" | cut -d' ' -f1)" results/gate1-M1D04/hashes.txt || { echo "M1D07-STOP timing.exe differs from M1D04"; exit 1; }
  for cell in $(python -I scripts/m1d07_plan.py "$out" --cells $PHASES); do
    IFS=: read -r s v c p <<< "$cell"
    f="$data/$s.$v.bin"
    for a in $(arms_of "$p" 1); do run_timing "$s" "$v" "$c" "$p" "$a" 1 "$f"; done
    for spec in $(python -I scripts/gate1_plan.py repeats "$tout" "$s" "$v" "$c" "$p"); do
      run_timing "$s" "$v" "$c" "$p" "${spec%%:*}" "${spec##*:}" "$f"
    done
    python -I scripts/check_m1d02.py "$tout" "$s" "$v" "$c" || { echo "M1D07-STOP $s $v $c p$p"; exit 1; }
    python -I scripts/m1d07_tcheck.py "$tout" "$out" "$s" "$v" "$c" "$p" || { echo "M1D07-STOP count build $s $v $c p$p"; exit 1; }
    echo "CELL-DONE p$p $s $v $c $(date +%H:%M:%S)"
  done
  echo "M1D07-DONE timing $tout $PHASES"
}

case $phase in
  derive) derive || exit 1 ;;
  count) sweep ;;
  timing) timing_sweep ;;
  *) echo "usage: scripts/run_m1d07.sh derive|count|timing [outdir] [cpu]"; exit 2 ;;
esac
