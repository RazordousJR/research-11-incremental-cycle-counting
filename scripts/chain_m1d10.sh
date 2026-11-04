#!/usr/bin/env bash
# M1D10C under one --exclusive --timing tower claim, on the PI's word (log.md 0064) and ControlPC's GO. Checks first, then the claim, then
# scripts/run_m1d10.sh, then the release, which runs whatever the run's exit. Nothing runs unless every check passes and the claim file exists.
# Usage: LINE_HOURS=2 scripts/chain_m1d10.sh. A second claim, only to finish a run cut by the line, is the same command on a new GO.
set -u
out=results/gate1-M1D10
tower=/c/Develop/tower
fail() { echo "M1D10-NOT-STARTED $*"; exit 4; }
mkdir -p "$out"

python -I "$tower/tower_check.py" --for research11 > "$out/tower-check.txt" 2>&1
verdict=$(grep "^VERDICT" "$out/tower-check.txt" | tr -d '\015')
[[ $verdict == "VERDICT: clear for "* ]] || fail "tower not clear: $verdict"
mem=$(powershell -NoProfile -Command '$o = Get-CimInstance Win32_OperatingSystem; "{0} {1}" -f [math]::Floor($o.FreePhysicalMemory / 1MB), [math]::Floor($o.FreeVirtualMemory / 1MB)' | tr -d '\015')
ram=${mem%% *}; commit=${mem##* }
[ "$ram" -ge 14 ] || fail "RAM available ${ram} GiB, under 14"
[ "$commit" -ge 40 ] || fail "commit free ${commit} GiB, under 40"
for exe in build/gate1/timing.exe build/gate1/count.exe; do
  grep -q "$(sha256sum "$exe" | cut -d' ' -f1)" results/gate1-M1D04/hashes.txt || fail "$exe differs from M1D04"
done
echo "checks: RAM ${ram} GiB, commit free ${commit} GiB, both binaries' hashes good"

python -I "$tower/tower_check.py" --claim research11 --what "R11 M1D10C timing, exclusive, 1 thread on CPU 8, line ${LINE_HOURS:-2} h, PI's word and ControlPC GO" --workers 1 --commit-gb 16 --exclusive --timing
[ -s "$tower/claim-research11.json" ] || fail "no claim file after --claim"
echo "M1D10-CLAIMED $(date +%T)"
bash scripts/run_m1d10.sh "$out" > "$out/run.log" 2>&1
rc=$?
python -I "$tower/tower_check.py" --release research11
echo "M1D10-RELEASED $(date +%T) exit $rc"
exit $rc
