#!/usr/bin/env bash
# Gate 0, all six streams at all six cuts. Protocol: docs/experiments/gate0-protocol.md.
set -u
out="results/gate0-$(date +%Y%m%d)-$(git rev-parse --short HEAD)"
mkdir -p "$out"
git rev-parse HEAD > "$out/commit.txt"
for s in CollegeMsg email-Eu-core-temporal sx-mathoverflow sx-askubuntu sx-superuser wiki-talk-temporal; do
  for c in "time 3600" "time 86400" "time 604800" "count 100" "count 1000" "count 10000"; do
    set -- $c
    ./build/r11_gate.exe "data/derived/$s.bin" "$s" "$1" "$2" "$out/$s-$1-$2.csv" || echo "FAILED $s $c"
  done
done
echo "GATE0-DONE $out"
