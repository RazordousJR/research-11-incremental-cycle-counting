# Efficiency class per logical processor, from GetSystemCpuSetInformation. Read-only.
import ctypes, struct
k = ctypes.WinDLL("kernel32")
n = ctypes.c_ulong(0)
k.GetSystemCpuSetInformation(None, 0, ctypes.byref(n), None, 0)
buf = ctypes.create_string_buffer(n.value)
assert k.GetSystemCpuSetInformation(buf, n, ctypes.byref(n), None, 0)
off = 0
rows = []
while off < n.value:
    size, typ = struct.unpack_from("<II", buf, off)
    # SYSTEM_CPU_SET_INFORMATION.CpuSet: Id u32, Group u16, LogicalProcessorIndex u8, CoreIndex u8, LastLevelCacheIndex u8, NumaNodeIndex u8, EfficiencyClass u8
    cid, grp, lp, core, llc, numa, eff = struct.unpack_from("<IHBBBBB", buf, off + 8)
    rows.append((lp, core, llc, eff))
    off += size
for r in sorted(rows):
    print(f"logical {r[0]:2d} core {r[1]:2d} llc {r[2]} efficiency class {r[3]}")
