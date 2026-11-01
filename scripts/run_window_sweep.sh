#!/usr/bin/env bash
# M1B01 runs, one process at a time. Usage: scripts/run_window_sweep.sh <slot seconds> [<slot seconds> ...]
set -u
cd "$(dirname "$0")/.."
out=results/screen-b-M1B01
mkdir -p "$out"
for w in "$@"; do
    for s in CollegeMsg email-Eu-core-temporal sx-mathoverflow sx-askubuntu sx-superuser wiki-talk-temporal; do
        for k in 4 1; do
            start=$SECONDS
            ./build/r11_window.exe "data/derived/$s.events.bin" "$s" "$k" "$w" "$out/$s-k$k-w$w.csv" || exit 1
            echo "$s k=$k w=$w took $((SECONDS - start)) s"
        done
    done
done
echo ALL_DONE
