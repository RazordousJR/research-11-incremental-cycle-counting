// Non-citable probe P0: operation counts on a synthetic graph and synthetic update streams.
// Writes one CSV row per batch. Every policy is cross-checked against static_count.
// Usage: probe_ops <n> <m_per_vertex> <seed> <batches_per_cell> <out.csv>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>

#include "../src/r11/tc.hpp"

using namespace r11;

namespace {

using Rng = std::mt19937_64;

// Holme-Kim growth: preferential attachment with triad formation, so the base graph has
// hubs and clustering. Isolated spare vertices are kept at the end for arrivals.
Graph holme_kim(u32 n, u32 spare, u32 mv, double p_triad, Rng& r) {
    Graph g(n + spare);
    std::vector<u32> ends;
    for (u32 a = 0; a <= mv; ++a)
        for (u32 b = a + 1; b <= mv; ++b) { g.insert(a, b); ends.push_back(a); ends.push_back(b); }
    std::uniform_real_distribution<double> u(0, 1);
    for (u32 v = mv + 1; v < n; ++v) {
        u32 last = ~0u;
        for (u32 i = 0; i < mv; ++i) {
            u32 w = ~0u;
            if (last != ~0u && u(r) < p_triad && g.deg(last) > 0) {
                u32 c = g.nbr(last)[r() % g.deg(last)];
                if (c != v && !g.has(v, c)) w = c;
            }
            if (w == ~0u)
                for (int t = 0; t < 20 && w == ~0u; ++t) {
                    u32 c = ends[r() % ends.size()];
                    if (c != v && !g.has(v, c)) w = c;
                }
            if (w == ~0u) continue;
            g.insert(v, w);
            ends.push_back(v);
            ends.push_back(w);
            last = w;
        }
    }
    return g;
}

std::vector<i64> degree_ranks(const Graph& g, u32 base_n) {
    std::vector<u32> ord(base_n);
    std::iota(ord.begin(), ord.end(), 0u);
    std::sort(ord.begin(), ord.end(), [&](u32 a, u32 b) { return g.deg(a) != g.deg(b) ? g.deg(a) < g.deg(b) : a < b; });
    std::vector<i64> rk(g.n());
    for (u32 i = 0; i < base_n; ++i) rk[ord[i]] = i;
    for (u32 v = base_n; v < g.n(); ++v) rk[v] = -i64(v) - 1;   // arrivals rank below every old vertex
    return rk;
}

u32 degree_biased(const Graph& g, u32 limit, Rng& r) {
    for (;;) {
        u32 v = u32(r() % limit);
        if (g.deg(v)) return g.nbr(v)[r() % g.deg(v)];
    }
}

u32 pareto(Rng& r, double kmin, double kmax, double alpha) {
    std::uniform_real_distribution<double> u(1e-12, 1);
    double k = kmin / std::pow(u(r), 1.0 / alpha);
    return u32(std::min(k, kmax));
}

// Each generator returns one batch of about `size` updates against the current graph.
std::vector<Update> uniform_mix(const Graph& g, u32 base_n, u64 size, Rng& r) {
    std::vector<Update> b;
    for (u64 i = 0; i < size / 2; ++i) {
        u32 v = u32(r() % base_n);
        if (g.deg(v)) b.push_back({v, g.nbr(v)[r() % g.deg(v)], false});
        b.push_back({u32(r() % base_n), u32(r() % base_n), true});
    }
    return b;
}

std::vector<Update> biased_inserts(const Graph& g, u32 base_n, u64 size, Rng& r) {
    std::vector<Update> b;
    for (u64 i = 0; i < size; ++i) b.push_back({degree_biased(g, base_n, r), degree_biased(g, base_n, r), true});
    return b;
}

std::vector<Update> arrivals(const Graph& g, u32 base_n, u64 size, u32& next, Rng& r) {
    std::vector<Update> b;
    while (b.size() < size && next < g.n()) {
        u32 x = next++, k = pareto(r, 2, 5000, 1.2);
        for (u32 i = 0; i < k; ++i) b.push_back({x, degree_biased(g, base_n, r), true});
    }
    return b;
}

std::vector<Update> departures(const Graph& g, u32 base_n, u64 size, Rng& r) {
    std::vector<Update> b;
    while (b.size() < size) {
        u32 x = degree_biased(g, base_n, r);
        if (g.deg(x) > 4000) continue;
        for (u32 w : g.nbr(x)) b.push_back({x, w, false});
    }
    return b;
}

std::vector<Update> cliques(const Graph& g, u32 base_n, u64 size, Rng& r) {
    std::vector<Update> b;
    while (b.size() < size) {
        u32 s = pareto(r, 3, 60, 1.5);
        std::vector<u32> vs;
        for (u32 i = 0; i < s; ++i) vs.push_back(degree_biased(g, base_n, r));
        for (u32 i = 0; i < s; ++i)
            for (u32 j = i + 1; j < s; ++j) b.push_back({vs[i], vs[j], true});
    }
    return b;
}

struct Policy { const char* name; Pivot p; KernelMode k; };

}  // namespace

int main(int argc, char** argv) {
    if (argc < 6) { std::puts("usage: probe_ops n mv seed batches out.csv"); return 2; }
    u32 n = u32(std::atoi(argv[1])), mv = u32(std::atoi(argv[2]));
    u64 seed = std::strtoull(argv[3], nullptr, 10);
    u32 batches = u32(std::atoi(argv[4]));
    u32 spare = n / 4;
    Rng r(seed);
    Graph base = holme_kim(n, spare, mv, 0.5, r);
    base.set_ranks(degree_ranks(base, n));
    auto [t0, sops] = static_count(base);
    std::printf("n=%u m=%llu triangles=%lld static_ops=%llu\n", n, (unsigned long long)base.m(), (long long)t0, (unsigned long long)sops);

    const Policy ps[] = {
        {"seq_stream", Pivot::Single, KernelMode::EdgeOnly},
        {"grp_edge", Pivot::BatchDegree, KernelMode::EdgeOnly},
        {"grp_noorient", Pivot::BatchDegree, KernelMode::AdaptiveNoOrient},
        {"grp_full", Pivot::BatchDegree, KernelMode::Adaptive},
        {"lowdeg_full", Pivot::LowDegree, KernelMode::Adaptive},
    };
    const char* streams[] = {"uniform", "biased", "arrival", "departure", "clique"};
    const double fracs[] = {1e-5, 1e-4, 1e-3, 1e-2, 1e-1};

    FILE* f = std::fopen(argv[5], "w");
    std::fprintf(f, "seed,stream,frac,batch,updates,net,policy,ops,kernel_a,kernel_b,kernel_c,static_ops,delta_ok\n");
    u64 mismatches = 0;
    for (const char* sname : streams) {
        std::string st = sname;
        for (double fr : fracs) {
            u64 size = std::max<u64>(1, u64(fr * double(base.m())));
            std::vector<Graph> gs(std::size(ps), base);
            i64 tcur = t0;
            u32 next = n;
            Graph& view = gs[0];
            Rng gr(seed * 1000003 + u64(fr * 1e6) + st.size() * 7 + (u64)st[0]);
            for (u32 bi = 0; bi < batches; ++bi) {
                std::vector<Update> b;
                if (st == "uniform") b = uniform_mix(view, n, size, gr);
                else if (st == "biased") b = biased_inserts(view, n, size, gr);
                else if (st == "arrival") b = arrivals(view, n, size, next, gr);
                else if (st == "departure") b = departures(view, n, size, gr);
                else b = cliques(view, n, size, gr);
                if (b.empty()) break;
                NetBatch nb = normalise(view, b);
                i64 tnew = 0;
                std::vector<std::pair<u64, std::array<u64, 3>>> res;
                for (size_t i = 0; i < std::size(ps); ++i) {
                    Engine e(gs[i].n());
                    Ops o;
                    i64 d = e.apply(gs[i], nb, ps[i].p, ps[i].k, o);
                    if (i == 0) tnew = tcur + d;
                    else if (tcur + d != tnew) ++mismatches;
                    res.push_back({o.total(), {e.kernel_a, e.kernel_b, e.kernel_c}});
                }
                auto [tref, sop] = static_count(gs[0]);
                bool ok = tref == tnew;
                if (!ok) ++mismatches;
                for (size_t i = 0; i < std::size(ps); ++i)
                    std::fprintf(f, "%llu,%s,%g,%u,%zu,%zu,%s,%llu,%llu,%llu,%llu,%llu,%d\n", (unsigned long long)seed, sname, fr, bi,
                                 b.size(), nb.ins.size() + nb.del.size(), ps[i].name, (unsigned long long)res[i].first,
                                 (unsigned long long)res[i].second[0], (unsigned long long)res[i].second[1],
                                 (unsigned long long)res[i].second[2], (unsigned long long)sop, ok ? 1 : 0);
                tcur = tnew;
            }
            std::fflush(f);
        }
        std::printf("done %s mismatches=%llu\n", sname, (unsigned long long)mismatches);
        std::fflush(stdout);
    }
    std::fclose(f);
    std::printf("mismatches=%llu\n", (unsigned long long)mismatches);
    return mismatches ? 1 : 0;
}
