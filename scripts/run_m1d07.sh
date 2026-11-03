#!/usr/bin/env bash
# M1D07: more streams at p = 3 and p = 5. Protocol: docs/experiments/m1d07-streams.md. One process at a time, pinned.
# Phase derive: the format check and the derived streams, from data/m1d07/raw into data/m1d07/derived. A Network Repository file is tried
# as an interaction file first, and as a real-deletion file only if the loader says its weights carry deletions.
# Phase count: M1D07C, the count sweep, every arm at p = 3 and then p = 5, cheap streams first. M at M1D04's thetas. Each cell's arms
# must agree per batch, or the run stops. Hard rule 1. A finished file is never run again, so the script can be restarted under a new claim.
# Usage: scripts/run_m1d07.sh derive|count [outdir] [cpu]
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
    if [ "$fmt" = konect ]; then variants=real; else variants="expiry insert"; fi
    for v in $variants; do
      [ -s "$data/$s.$v.bin" ] && continue
      msg=$(python -I scripts/prepare_m1d07_stream.py "$fmt" "$raw/$arc" "$data/$s.$v.bin" "$v")
      rc=$?
      echo "$msg"
      if [ $rc -eq 3 ] && [ "$fmt" = nr ] && [[ $msg == *real-deletion* ]]; then
        python -I scripts/prepare_m1d07_stream.py nr "$raw/$arc" "$data/$s.real.bin" real || echo "M1D07-DROP $s real"
        break
      fi
      [ $rc -eq 0 ] || echo "M1D07-DROP $s $v exit $rc"
    done
  done
  echo "M1D07-DERIVED $data"
}

cells() {   # stream:variant, in sweep order, for every derived stream
  if [ -n "${CELLS:-}" ]; then echo "$CELLS"; return; fi
  echo "$STREAMS" | while read -r s fmt arc; do
    [ -n "$s" ] || continue
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

case $phase in
  derive) derive ;;
  count) sweep ;;
  *) echo "usage: scripts/run_m1d07.sh derive|count [outdir] [cpu]"; exit 2 ;;
esac
