// Diagnostic after Gate 0, not a gate. Hindsight bound: for every group of a batch, the cheaper of kernel A and
// kernel O is taken with no selection cost, no batch overhead and no order upkeep. This favours the method.
// Usage: r11_gate <events.bin> <stream> <time|count> <cut>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "../src/r11/tc.hpp"

using namespace r11;

#pragma pack(push, 1)
struct Event { std::int64_t t; u32 u, v, op; };
#pragma pack(pop)

int main(int, char** argv) {
    FILE* in = std::fopen(argv[1], "rb");
    std::fseek(in, 0, SEEK_END);
    long long bytes = _ftelli64(in);
    std::fseek(in, 0, SEEK_SET);
    std::vector<Event> ev(size_t(bytes) / sizeof(Event));
    if (std::fread(ev.data(), sizeof(Event), ev.size(), in) != ev.size()) return 2;
    std::fclose(in);
    bool by_time = std::string(argv[3]) == "time";
    i64 cut = std::atoll(argv[4]);
    u32 n = 0;
    for (const Event& x : ev) n = std::max({n, x.u + 1, x.v + 1});
    Graph g(n);
    Engine e(n);
    e.collect_oracle = true;
    Ops ops;
    u64 net = 0, since = 0, at = 1024;
    i64 t = 0;
    std::vector<Update> b;
    for (size_t i = 0; i < ev.size();) {
        size_t j = i + 1;
        while (j < ev.size() && (by_time ? ev[j].t / cut == ev[i].t / cut : j % size_t(cut) != 0)) ++j;
        b.clear();
        for (size_t q = i; q < j; ++q) b.push_back({ev[q].u, ev[q].v, ev[q].op == 1});
        NetBatch nb = normalise(g, b);
        net += nb.ins.size() + nb.del.size();
        since += nb.ins.size() + nb.del.size();
        t += e.apply(g, nb, Pivot::BatchDegree, KernelMode::EdgeOnly, ops);
        if (since >= at) {
            std::vector<u32> live;
            for (u32 v = 0; v < n; ++v) if (g.deg(v)) live.push_back(v);
            std::sort(live.begin(), live.end(), [&](u32 a, u32 c) { return g.deg(a) != g.deg(c) ? g.deg(a) < g.deg(c) : a < c; });
            std::vector<i64> rk(n);
            for (u32 v = 0; v < n; ++v) rk[v] = -i64(v) - 1;
            for (size_t q = 0; q < live.size(); ++q) rk[live[q]] = i64(q);
            g.set_ranks(rk);
            since = 0;
            at = std::max<u64>(1024, g.m());
        }
        i = j;
    }
    auto [ref, o] = static_count(g);
    (void)o;
    std::printf("%s,%s,%lld,%llu,%llu,%.2f,%.4f,%llu,%lld,%d\n", argv[2], argv[3], (long long)cut, (unsigned long long)net,
                (unsigned long long)g.m(), double(e.oracle_a) / double(std::max<u64>(net, 1)),
                double(e.oracle_min) / double(std::max<u64>(e.oracle_a, 1)), (unsigned long long)e.oracle_a, (long long)t, ref == t);
    return ref == t ? 0 : 1;
}
