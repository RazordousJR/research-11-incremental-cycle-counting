#!/usr/bin/env bash
# M1B01-B runs, k = 4, one process at a time. Usage: scripts/run_window_sweep_b.sh <slot seconds> [<slot seconds> ...]
set -u
cd "$(dirname "$0")/.."
out=results/screen-b-M1B01-B
mkdir -p "$out"
for w in "$@"; do
    for s in CollegeMsg email-Eu-core-temporal sx-mathoverflow sx-askubuntu sx-superuser wiki-talk-temporal; do
        start=$SECONDS
        ./build/r11_window.exe "data/derived/$s.events.bin" "$s" 4 "$w" "$out/$s-k4-w$w.csv" || exit 1
        echo "$s k=4 w=$w took $((SECONDS - start)) s"
    done
done
echo ALL_DONE
