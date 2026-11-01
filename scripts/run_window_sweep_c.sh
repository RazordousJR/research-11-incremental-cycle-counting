#!/usr/bin/env bash
# M1C01 runs, k = 4, one process at a time, each under ten minutes. Usage: scripts/run_window_sweep_c.sh <slot seconds> [...]
# Build first:
#   g++ -std=c++20 -O2 -Wall -Wextra -static benchmarks/window_sweep.cpp -o build/r11_window_c.exe
#   g++ -std=c++20 -O2 -Wall -Wextra -static tests/test_window.cpp -o build/r11_test_window_c.exe
set -u
cd "$(dirname "$0")/.."
out=results/screen-c-M1C01
mkdir -p "$out"
for w in "$@"; do
    for s in CollegeMsg email-Eu-core-temporal sx-mathoverflow sx-askubuntu sx-superuser wiki-talk-temporal; do
        start=$SECONDS
        timeout 590 ./build/r11_window_c.exe "data/derived/$s.events.bin" "$s" 4 "$w" "$out/$s-k4-w$w.csv"
        rc=$?
        echo "$s k=4 w=$w rc=$rc took $((SECONDS - start)) s"
        [ "$rc" -eq 0 ] || { echo STOPPED; exit 1; }
    done
done
echo ALL_DONE
