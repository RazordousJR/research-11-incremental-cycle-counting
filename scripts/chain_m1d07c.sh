#!/usr/bin/env bash
# M1D07C under one ordinary tower claim, on ControlPC's GO. Checks first, then the claim, then derive and the count sweep, then the
# release, which runs whatever the sweep's exit. Nothing runs unless every check passes and the claim file exists (log.md 0044).
# Usage: scripts/chain_m1d07c.sh [outdir] [cpu]. LINE_HOURS is ControlPC's line, 8 by the PI's word (log.md 0043).
# MODE=timing PHASES="D1 D2" runs M1D07D instead, under an --exclusive --timing claim, which needs the PI's hours and ControlPC's GO.
set -u
out="${1:-results/gate1-M1D07}"
cpu="${2:-8}"
tower=/c/Develop/tower
fail() { echo "M1D07C-NOT-STARTED $*"; exit 4; }

mkdir -p "$out"
python -I "$tower/tower_check.py" --for research11 > "$out/tower-check.txt" 2>&1
verdict=$(grep "^VERDICT" "$out/tower-check.txt")
[[ $verdict == "VERDICT: clear for "* ]] || fail "tower not clear: $verdict"
mem=$(powershell -NoProfile -Command '$o = Get-CimInstance Win32_OperatingSystem; "{0} {1}" -f [math]::Floor($o.FreePhysicalMemory / 1MB), [math]::Floor($o.FreeVirtualMemory / 1MB)' | tr -d '\r')
ram=${mem%% *}; commit=${mem##* }
[ "$ram" -ge 14 ] || fail "RAM available ${ram} GiB, under 14"
[ "$commit" -ge 40 ] || fail "commit free ${commit} GiB, under 40"
(cd data/m1d07/raw && sha256sum -c --quiet SHA256SUMS) || fail "checksums"
grep -q "$(sha256sum build/gate1/count.exe | cut -d' ' -f1)" results/gate1-M1D04/hashes.txt || fail "count.exe differs from M1D04"
echo "checks: RAM ${ram} GiB, commit free ${commit} GiB, checksums and count.exe hash good"

if [ "${MODE:-count}" = timing ]; then
  [ -n "${PHASES:-}" ] || fail "MODE=timing needs PHASES"
  python -I "$tower/tower_check.py" --claim research11 --what "R11 M1D07D timing $PHASES, exclusive, 1 thread CPU $cpu, line ${LINE_HOURS:-8} h, PI hours and ControlPC GO" --workers 1 --commit-gb 16 --exclusive --timing
else
  python -I "$tower/tower_check.py" --claim research11 --what "R11 M1D07C count sweep, ordinary, 1 thread CPU $cpu, line ${LINE_HOURS:-8} h, ControlPC GO" --workers 1 --commit-gb 16
fi
[ -s "$tower/claim-research11.json" ] || fail "no claim file after --claim"
echo "M1D07C-CLAIMED $(date +%T)"
started=$(date +%s)
if [ "${MODE:-count}" = timing ]; then
  bash scripts/run_m1d07.sh timing "$out" "$cpu"
else
  bash scripts/run_m1d07.sh derive "$out" "$cpu" && LINE_HOURS=$(( ${LINE_HOURS:-8} - ($(date +%s) - started) / 3600 )) bash scripts/run_m1d07.sh count "$out" "$cpu"
fi
rc=$?
python -I "$tower/tower_check.py" --release research11
echo "M1D07C-RELEASED $(date +%T) exit $rc"
exit $rc
