// Gate 0: operation counts on one real stream at one batch cut. Protocol: docs/experiments/gate0-protocol.md.
// Usage: r11_gate <events.bin> <stream> <time|count> <cut> <out.csv>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "../src/r11/tc.hpp"

using namespace r11;

namespace {

#pragma pack(push, 1)
struct Event { std::int64_t t; u32 u, v, op; };
#pragma pack(pop)

struct Run {
    const char* name;
    Pivot pivot;
    KernelMode mode;
    bool raw, group, oriented;
};

struct State {
    Graph g;
    Engine e;
    Ops kernel;
    u64 extra = 0, applied = 0, since_rank = 0, rank_at = 1024, reranks = 0;
    i64 t = 0;
    State(u32 n) : g(n), e(n) {}
    u64 total() const { return kernel.total() + extra; }
};

void rerank(State& s) {
    Graph& g = s.g;
    std::vector<u32> live;
    for (u32 v = 0; v < g.n(); ++v)
        if (g.deg(v)) live.push_back(v);
    std::sort(live.begin(), live.end(), [&](u32 a, u32 b) {
        return g.deg(a) != g.deg(b) ? g.deg(a) < g.deg(b) : a < b;
    });
    std::vector<i64> rk(g.n());
    for (u32 v = 0; v < g.n(); ++v) rk[v] = -i64(v) - 1;
    for (size_t i = 0; i < live.size(); ++i) rk[live[i]] = i64(i);
    g.set_ranks(rk);
    double a = double(std::max<size_t>(live.size(), 2));
    s.extra += 2 * g.m() + u64(a * std::log2(a));
    ++s.reranks;
    s.since_rank = 0;
    s.rank_at = std::max<u64>(1024, g.m());
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 6) { std::puts("usage: r11_gate events.bin stream time|count cut out.csv"); return 2; }
    FILE* in = std::fopen(argv[1], "rb");
    if (!in) { std::puts("cannot open events"); return 2; }
    std::fseek(in, 0, SEEK_END);
    long long bytes = _ftelli64(in);
    std::fseek(in, 0, SEEK_SET);
    std::vector<Event> ev(size_t(bytes) / sizeof(Event));
    if (std::fread(ev.data(), sizeof(Event), ev.size(), in) != ev.size()) { std::puts("short read"); return 2; }
    std::fclose(in);
    std::string stream = argv[2], kind = argv[3];
    i64 cut = std::atoll(argv[4]);
    bool by_time = kind == "time";

    u32 n = 0;
    for (const Event& x : ev) n = std::max({n, x.u + 1, x.v + 1});

    const Run runs[] = {
        {"seq_raw", Pivot::Single, KernelMode::EdgeOnly, true, false, false},
        {"seq_net", Pivot::Single, KernelMode::EdgeOnly, false, false, false},
        {"grp_edge", Pivot::BatchDegree, KernelMode::EdgeOnly, false, true, false},
        {"grp_noorient", Pivot::BatchDegree, KernelMode::AdaptiveNoOrient, false, true, false},
        {"grp_full", Pivot::BatchDegree, KernelMode::Adaptive, false, true, true},
    };
    constexpr size_t P = std::size(runs);
    std::vector<State> st;
    st.reserve(P);
    for (size_t i = 0; i < P; ++i) st.emplace_back(n);

    // batch boundaries
    std::vector<size_t> starts{0};
    for (size_t i = 1; i < ev.size(); ++i) {
        bool cutnow = by_time ? (ev[i].t / cut != ev[i - 1].t / cut) : (i % size_t(cut) == 0);
        if (cutnow) starts.push_back(i);
    }
    starts.push_back(ev.size());
    size_t batches = starts.size() - 1, every = std::max<size_t>(1, batches / 200);

    u64 mismatches = 0, net_total = 0, peak_m = 0, static_checks = 0, peak_batch = 0;
    std::vector<Update> b;
    for (size_t bi = 0; bi < batches; ++bi) {
        b.clear();
        for (size_t i = starts[bi]; i < starts[bi + 1]; ++i) b.push_back({ev[i].u, ev[i].v, ev[i].op == 1});
        peak_batch = std::max<u64>(peak_batch, b.size());
        NetBatch nb = normalise(st[1].g, b);
        u64 net = nb.ins.size() + nb.del.size();
        net_total += net;
        for (size_t p = 0; p < P; ++p) {
            State& s = st[p];
            if (runs[p].raw) {
                for (const Update& u : b) {
                    NetBatch one;
                    Edge e = canon(u.u, u.v);
                    bool present = s.g.has(e.a, e.b);
                    if (u.ins && !present) one.ins.push_back(e);
                    else if (!u.ins && present) one.del.push_back(e);
                    else continue;
                    s.t += s.e.apply(s.g, one, Pivot::Single, KernelMode::EdgeOnly, s.kernel);
                    s.extra += 2;
                    ++s.applied;
                }
            } else {
                s.extra += 2 * b.size() + 2 * net;
                if (runs[p].group) s.extra += 3 * net;
                if (runs[p].oriented) s.extra += net;
                s.t += s.e.apply(s.g, nb, runs[p].pivot, runs[p].mode, s.kernel);
                s.applied += net;
                if (runs[p].oriented) {
                    s.since_rank += net;
                    if (s.since_rank >= s.rank_at) rerank(s);
                }
            }
        }
        for (size_t p = 1; p < P; ++p) mismatches += st[p].t != st[0].t;
        peak_m = std::max(peak_m, st[1].g.m());
        if (bi % every == 0 || bi + 1 == batches) {
            auto [t, ops] = static_count(st[4].g);
            (void)ops;
            mismatches += t != st[0].t;
            ++static_checks;
        }
        if (mismatches) {
            std::printf("MISMATCH at batch %zu of %s\n", bi, stream.c_str());
            return 1;
        }
    }

    FILE* f = std::fopen(argv[5], "w");
    std::fprintf(f, "stream,kind,cut,policy,ops,kernel_ops,extra_ops,scan,probe,mark,groups_a,groups_r,groups_o,star_whole,star_spokes,"
                    "reranks,events,net,batches,peak_batch,peak_m,final_t,vertices,static_checks,mismatches\n");
    for (size_t p = 0; p < P; ++p) {
        const State& s = st[p];
        std::fprintf(f, "%s,%s,%" PRId64 ",%s,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%zu,%llu,%zu,%llu,%llu,%lld,%u,%llu,%llu\n",
                     stream.c_str(), kind.c_str(), std::int64_t(cut), runs[p].name, (unsigned long long)s.total(),
                     (unsigned long long)s.kernel.total(), (unsigned long long)s.extra, (unsigned long long)s.kernel.scan,
                     (unsigned long long)s.kernel.probe, (unsigned long long)s.kernel.mark, (unsigned long long)s.e.kernel_a,
                     (unsigned long long)s.e.kernel_b, (unsigned long long)s.e.kernel_c, (unsigned long long)s.e.star_whole,
                     (unsigned long long)s.e.star_spokes, (unsigned long long)s.reranks, ev.size(), (unsigned long long)net_total,
                     batches, (unsigned long long)peak_batch, (unsigned long long)peak_m, (long long)st[0].t, n,
                     (unsigned long long)static_checks, (unsigned long long)mismatches);
    }
    std::fclose(f);
    std::printf("%s %s %lld batches=%zu net=%llu peak_m=%llu T=%lld seq_net=%llu grp_full=%llu\n", stream.c_str(), kind.c_str(), (long long)cut,
                batches, (unsigned long long)net_total, (unsigned long long)peak_m, (long long)st[0].t,
                (unsigned long long)st[1].total(), (unsigned long long)st[4].total());
    return 0;
}
