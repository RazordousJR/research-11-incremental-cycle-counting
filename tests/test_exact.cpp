// Exactness oracle for the batch engine. Every policy is compared with an O(n^3) reference
// after every batch. The control at the end must fail, or this test proves nothing.
#include <cstdio>
#include <cstdlib>
#include <random>

#include "../src/r11/tc.hpp"

using namespace r11;

namespace {

struct Stats { u64 batches = 0, multi2 = 0, multi3 = 0, mixed = 0, noops = 0; };

// Triangles of the union graph classified by how many net changed edges they hold.
void classify(const Graph& g, const NetBatch& nb, Stats& s) {
    std::unordered_set<u64> ins, del;
    for (Edge e : nb.ins) ins.insert(key(e));
    for (Edge e : nb.del) del.insert(key(e));
    auto kind = [&](u32 a, u32 b) {
        u64 k = key(canon(a, b));
        return ins.count(k) ? 1 : del.count(k) ? 2 : g.has(a, b) ? 3 : 0;
    };
    u32 n = g.n();
    for (u32 a = 0; a < n; ++a)
        for (u32 b = a + 1; b < n; ++b) {
            int ab = kind(a, b);
            if (!ab) continue;
            for (u32 c = b + 1; c < n; ++c) {
                int ac = kind(a, c), bc = kind(b, c);
                if (!ac || !bc) continue;
                int i = (ab == 1) + (ac == 1) + (bc == 1), d = (ab == 2) + (ac == 2) + (bc == 2);
                if (i && d) ++s.mixed;
                if (i + d == 2 && !(i && d)) ++s.multi2;
                if (i + d == 3 && !(i && d)) ++s.multi3;
            }
        }
}

std::vector<Update> random_batch(std::mt19937_64& r, u32 n, u32 len, double p_ins) {
    std::vector<Update> b;
    std::bernoulli_distribution ins(p_ins);
    std::uniform_int_distribution<u32> v(0, n - 1);
    for (u32 i = 0; i < len; ++i) b.push_back({v(r), v(r), ins(r)});
    if (len && (r() & 1)) {                     // a star burst around one vertex
        u32 x = v(r);
        bool in = ins(r);
        for (u32 w = 0; w < n; ++w)
            if (r() % 2) b.push_back({x, w, in});
    }
    if (len > 1 && (r() & 1))                   // repeat earlier pairs to force toggles
        for (u32 i = 0; i < len / 2; ++i) {
            Update u = b[r() % b.size()];
            u.ins = !u.ins;
            b.push_back(u);
        }
    return b;
}

// The naive sum: per-edge common neighbours on the final graph with no peeling. A triangle
// with j changed edges is counted j times, so this must disagree with the reference.
i64 naive_delta(Graph& g, const NetBatch& nb) {
    i64 d = 0;
    for (Edge e : nb.del) g.erase(e.a, e.b);
    for (Edge e : nb.ins) g.insert(e.a, e.b);
    for (Edge e : nb.ins)
        for (u32 w : g.nbr(e.a)) if (g.has(w, e.b)) ++d;
    return d;
}

}  // namespace

int main(int argc, char** argv) {
    u64 trials = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 20000;
    u64 seed = argc > 2 ? std::strtoull(argv[2], nullptr, 10) : 1;
    std::mt19937_64 r(seed);
    const Pivot pivots[] = {Pivot::Single, Pivot::LowDegree, Pivot::BatchDegree};
    const KernelMode modes[] = {KernelMode::EdgeOnly, KernelMode::ReverseOnly, KernelMode::OrientedOnly,
                                KernelMode::AdaptiveNoOrient, KernelMode::Adaptive};
    Stats st;
    u64 fails = 0, bound_fail = 0, ratio_fail = 0, naive_wrong = 0, naive_runs = 0;
    u64 used_a = 0, used_b = 0, used_c = 0;

    for (u64 t = 0; t < trials; ++t) {
        u32 n = 4 + u32(r() % 21);
        std::vector<i64> rk(n);
        for (auto& x : rk) x = i64(r() % 1000);     // orders are arbitrary on purpose
        Graph base(n);
        if (r() & 1) base.set_ranks(rk);
        std::bernoulli_distribution p(0.1 + 0.6 * double(r() % 100) / 100.0);
        for (u32 a = 0; a < n; ++a)
            for (u32 b = a + 1; b < n; ++b)
                if (p(r)) base.insert(a, b);
        std::vector<Update> b = random_batch(r, n, 1 + u32(r() % 40), 0.5);
        NetBatch nb = normalise(base, b);
        classify(base, nb, st);
        if (nb.ins.empty() && nb.del.empty()) ++st.noops;
        ++st.batches;

        Graph refg = base;
        for (const Update& u : b) {     // sequential semantics, one update at a time
            if (u.u == u.v) continue;
            if (u.ins && !refg.has(u.u, u.v)) refg.insert(u.u, u.v);
            if (!u.ins && refg.has(u.u, u.v)) refg.erase(u.u, u.v);
        }
        i64 t0 = brute_triangles(base), t1 = brute_triangles(refg);

        for (Pivot pm : pivots) {
            u64 ops_edge = 0;
            for (KernelMode km : modes) {
                Graph g = base;
                Engine e(n);
                e.check_bound = true;
                Ops o;
                i64 got = t0 + e.apply(g, nb, pm, km, o);
                if (got != t1 || g.m() != refg.m()) ++fails;
                if (brute_triangles(g) != t1) ++fails;
                bound_fail += e.bound_violations;
                if (km == KernelMode::Adaptive) { used_a += e.kernel_a; used_b += e.kernel_b; used_c += e.kernel_c; }
                if (km == KernelMode::EdgeOnly) ops_edge = o.total();
                if (km == KernelMode::Adaptive && pm != Pivot::Single) {
                    u64 slack = 2 * (nb.ins.size() + nb.del.size());
                    if (o.total() > 2 * ops_edge + slack) ++ratio_fail;
                }
            }
        }
        if (!nb.ins.empty() && nb.del.empty()) {
            Graph g = base;
            ++naive_runs;
            if (t0 + naive_delta(g, nb) != t1) ++naive_wrong;
        }
    }

    std::printf("trials=%llu seed=%llu\n", (unsigned long long)trials, (unsigned long long)seed);
    std::printf("triangles with 2 same-type changed edges=%llu, 3=%llu, mixed insert+delete=%llu, no-op batches=%llu\n",
                (unsigned long long)st.multi2, (unsigned long long)st.multi3,
                (unsigned long long)st.mixed, (unsigned long long)st.noops);
    std::printf("adaptive groups by kernel: edge=%llu reverse=%llu oriented=%llu\n", (unsigned long long)used_a,
                (unsigned long long)used_b, (unsigned long long)used_c);
    std::printf("exactness failures=%llu\n", (unsigned long long)fails);
    std::printf("cost estimate violations=%llu\n", (unsigned long long)bound_fail);
    std::printf("adaptive above 2*edge+2|D| =%llu\n", (unsigned long long)ratio_fail);
    std::printf("control: naive sum wrong on %llu of %llu insert-only batches\n",
                (unsigned long long)naive_wrong, (unsigned long long)naive_runs);
    bool ok = fails == 0 && bound_fail == 0 && ratio_fail == 0 && naive_wrong > 0;
    std::puts(ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
