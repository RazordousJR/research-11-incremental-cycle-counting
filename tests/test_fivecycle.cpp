// Exactness oracle for M1C02 and M1C03. After every batch of random small graphs, every arm is compared with a brute-force recount over vertex
// tuples. The brute force is itself checked against the closed-walk formula of Alon, Yuster and Zwick. Every per-edge kernel is compared
// on every pair, present or absent, with a brute count of the paths. The wedge table is compared entry by entry. RC is compared directly,
// and its operation bound is asserted on every recount.
//
// The batches have deletions, hubs, whole stars, repeated pairs inside one batch, no-op inserts and no-op deletes.
//
// The naive per-edge sum is a control and must fail. Each mutant below is a real way to get this wrong. The test FAILS if any one passes.
//   1  R2, PR: no degenerate term                       7  PR: spoke not taken out of the row before its join, deletions
//   2  R2, PR: degenerate term d(t), not d(t) - 2       8  RC: T_{v,u} not subtracted
//   3  R2, PR: b = other endpoint kept in both rows     9  RC: the adjacency test of line 16 dropped
//   4  WJ: tau (p - 1) dropped                         10  PW: memo does not advance over later spokes
//   5  WJ: upkeep on one side only                     11  PW: tau (q - 1) dropped
//   6  PR: spoke not added to the row, insertions      12  RC: x not held above v, line 14 read without its subscript
//  13  WT: a pair written in one table only            16  WT: removal without the backward shift
//  14  WT: the mirrored slots not kept                 17  WT: a saturated count read from the slot, side table ignored
//  15  WT: the bitset not cleared after a join         18  WT: tau (p - 1) dropped
//
// WT runs twice: once as built, and once with its counts forced to saturate at 3, since small graphs never reach the real saturation point.
// Normaliser and Grouper must give exactly what normalise and make_groups give, on every batch.
//
// Usage: r11_test_fivecycle [trials] [seed]
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <random>
#include <set>

#define FIVECYCLE_NO_MAIN
#include "../benchmarks/fivecycle.cpp"

using namespace fc;

namespace {

// The definition, nothing else. Adjacency matrix, n small.
struct Mat {
    u32 n;
    std::vector<char> a;
    explicit Mat(u32 nv) : n(nv), a(size_t(nv) * nv, 0) {}
    bool has(u32 u, u32 v) const { return a[size_t(u) * n + v]; }
    void set(u32 u, u32 v, bool on) { a[size_t(u) * n + v] = on; a[size_t(v) * n + u] = on; }
};

// Every 5-cycle has one smallest vertex v0 and two directions, and v1 < v4 keeps one of them.
i64 brute5(const Mat& m) {
    const u32 n = m.n;
    i64 c = 0;
    for (u32 v0 = 0; v0 < n; ++v0)
        for (u32 v1 = v0 + 1; v1 < n; ++v1) {
            if (!m.has(v0, v1)) continue;
            for (u32 v2 = v0 + 1; v2 < n; ++v2) {
                if (v2 == v1 || !m.has(v1, v2)) continue;
                for (u32 v3 = v0 + 1; v3 < n; ++v3) {
                    if (v3 == v1 || v3 == v2 || !m.has(v2, v3)) continue;
                    for (u32 v4 = v1 + 1; v4 < n; ++v4) {
                        if (v4 == v2 || v4 == v3) continue;
                        if (m.has(v3, v4) && m.has(v4, v0)) ++c;
                    }
                }
            }
        }
    return c;
}

// Alon, Yuster, Zwick: c5 = (tr A^5 - 5 tr A^3 - 5 sum_i (d_i - 2) (A^3)_ii) / 10. Checks the brute force from outside.
i64 closed_walks5(const Mat& m) {
    const u32 n = m.n;
    using V = std::vector<i64>;
    V a(size_t(n) * n), a2(size_t(n) * n, 0), a3(size_t(n) * n, 0);
    for (size_t i = 0; i < a.size(); ++i) a[i] = m.a[i];
    auto mul = [&](const V& x, const V& y, V& z) {
        std::fill(z.begin(), z.end(), 0);
        for (u32 i = 0; i < n; ++i)
            for (u32 k = 0; k < n; ++k)
                for (u32 j = 0; j < n; ++j) z[size_t(i) * n + j] += x[size_t(i) * n + k] * y[size_t(k) * n + j];
    };
    mul(a, a, a2);
    mul(a2, a, a3);
    i64 tr3 = 0, tr5 = 0, weighted = 0;
    for (u32 i = 0; i < n; ++i) {
        i64 d = 0;
        for (u32 j = 0; j < n; ++j) { d += a[size_t(i) * n + j]; tr5 += a3[size_t(i) * n + j] * a2[size_t(j) * n + i]; }
        tr3 += a3[size_t(i) * n + i];
        weighted += (d - 2) * a3[size_t(i) * n + i];
    }
    return (tr5 - 5 * tr3 - 5 * weighted) / 10;
}

// Paths u-a-b-c-v on five distinct vertices, which is the number of cycles the pair closes. Does not read the pair.
i64 brute_through(const Mat& m, u32 u, u32 v) {
    i64 c = 0;
    for (u32 a = 0; a < m.n; ++a) {
        if (a == u || a == v || !m.has(u, a)) continue;
        for (u32 b = 0; b < m.n; ++b) {
            if (b == u || b == v || b == a || !m.has(a, b)) continue;
            for (u32 z = 0; z < m.n; ++z) {
                if (z == u || z == v || z == a || z == b) continue;
                if (m.has(b, z) && m.has(z, v)) ++c;
            }
        }
    }
    return c;
}

Graph graph_of(const Mat& m) {
    Graph g(m.n);
    for (u32 a = 0; a < m.n; ++a)
        for (u32 b = a + 1; b < m.n; ++b)
            if (m.has(a, b)) g.insert(a, b);
    return g;
}

// Common neighbours, the true value of the wedge table.
std::map<u64, u32> true_wedges(const Mat& m) {
    std::map<u64, u32> w;
    for (u32 a = 0; a < m.n; ++a)
        for (u32 b = a + 1; b < m.n; ++b) {
            u32 c = 0;
            for (u32 t = 0; t < m.n; ++t) c += m.has(a, t) && m.has(b, t);
            if (c) w[Wedge::pair_key(a, b)] = c;
        }
    return w;
}

// Inserts first as a set, then every edge is summed on the same graph: cycles with two new edges are counted twice.
i64 naive_delta(Graph& g, const NetBatch& nb) {
    Row row(g.n());
    Cost c;
    i64 d = 0;
    for (const Edge& e : nb.del) d -= through_r2(g, e.a, e.b, row, c);
    for (const Edge& e : nb.del) g.erase(e.a, e.b);
    for (const Edge& e : nb.ins) g.insert(e.a, e.b);
    for (const Edge& e : nb.ins) d += through_r2(g, e.a, e.b, row, c);
    for (const Edge& e : nb.ins) g.erase(e.a, e.b);
    for (const Edge& e : nb.del) g.insert(e.a, e.b);
    return d;
}

constexpr Arm kArms[] = {Arm::R2, Arm::R2B, Arm::WJ, Arm::PR, Arm::PW, Arm::RC, Arm::WT};

// The saturated form of WT: two count bits, so the side table is in use on small graphs.
std::unique_ptr<Engine5> wt_saturated(u32 n) { return std::make_unique<Engine5>(Arm::WT, n, 2); }

bool same_edges(const std::vector<Edge>& a, const std::vector<Edge>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (a[i].a != b[i].a || a[i].b != b[i].b) return false;
    return true;
}

bool same_groups(const Graph& g, const std::vector<Edge>& es, Grouper& gr) {
    const std::vector<Group> want = make_groups(g, es, Pivot::BatchDegree);
    const std::vector<GroupV>& got = gr.run(es);
    if (want.size() != got.size()) return false;
    for (size_t i = 0; i < want.size(); ++i) {
        if (want[i].x != got[i].x || want[i].c.size() != got[i].c.size()) return false;
        for (size_t j = 0; j < want[i].c.size(); ++j)
            if (want[i].c[j] != got[i].c[j]) return false;
    }
    return true;
}

// WT against the true wedges: both tables, every entry, and the count of pairs.
bool wt_matches(const WTable& w, const std::map<u64, u32>& tw) {
    if (w.size() != tw.size()) return false;
    u64 seen = 0;
    bool bad = false;
    w.for_each([&](u32 a, u32 b, u32 v) {
        auto f = tw.find(Wedge::pair_key(a, b));
        if (f == tw.end() || f->second != v) bad = true;
        ++seen;
    });
    return !bad && seen == 2 * tw.size();
}

struct Stats {
    u64 batches = 0, updates = 0, ins = 0, del = 0, stars = 0, spokes = 0, noop = 0, repeated = 0, bound_checks = 0;
    u64 pair_checks = 0, table_checks = 0, static_checks = 0, walk_checks = 0, max_t = 0, naive_wrong = 0, saturated = 0;
};

// One random stream. mut_arm limits work when hunting a mutant. Returns the first mismatch as text, empty when clean.
std::string one_trial(std::mt19937_64& r, bool full, Stats& st, i64* opcheck = nullptr) {
    std::uniform_real_distribution<double> uni(0, 1);
    const u32 n = 5 + u32(r() % 9);
    const int family = int(r() % 6);
    const double p = family == 0 ? 0.15 : family == 1 ? 0.55 : family == 2 ? 0.3 : family == 3 ? 0.1 : family == 4 ? 0.8 : 0.4;
    const u32 hub = u32(r() % n), hub2 = u32(r() % n);

    auto rand_pair = [&](u32& a, u32& b) {
        do {
            if ((family == 2 || family == 3) && uni(r) < 0.55) {
                a = uni(r) < 0.6 ? hub : hub2;
                b = u32(r() % n);
            } else {
                a = u32(r() % n);
                b = u32(r() % n);
            }
        } while (a == b);
    };

    Mat m(n);
    std::vector<std::unique_ptr<Engine5>> arms;
    for (Arm a : kArms) arms.push_back(std::make_unique<Engine5>(a, n));
    arms.push_back(wt_saturated(n));
    Normaliser norm;
    Grouper grouper(n);
    // Seed the graph as one big insert batch, so that everything below starts from a real state.
    std::vector<Update> seed;
    for (u32 a = 0; a < n; ++a)
        for (u32 b = a + 1; b < n; ++b)
            if (uni(r) < p) seed.push_back({a, b, true});

    const int rounds = 2 + int(r() % 10);
    for (int round = -1; round < rounds; ++round) {
        std::vector<Update> b;
        if (round < 0) {
            b = seed;
        } else {
            const int style = int(r() % 5);
            const u32 size = 1 + u32(r() % (2 * n));
            const u32 star = u32(r() % n);
            for (u32 i = 0; i < size; ++i) {
                u32 x, y;
                if (style == 1) {               // a whole star at one vertex
                    x = star;
                    do y = u32(r() % n); while (y == x);
                } else {
                    rand_pair(x, y);
                }
                bool ins;
                if (style == 2) ins = true;
                else if (style == 3) ins = false;
                else ins = uni(r) < 0.5;
                if (style == 4 && !b.empty() && uni(r) < 0.5) {  // the same pair again in this batch, either way round
                    const Update& o = b[r() % b.size()];
                    x = o.v; y = o.u;
                    ins = uni(r) < 0.5;
                    ++st.repeated;
                }
                b.push_back({x, y, ins});
            }
        }
        NetBatch nb = normalise(arms[0]->g, b);
        st.noop += b.size() - nb.ins.size() - nb.del.size();
        {
            const NetBatch& fast = norm.run(arms[0]->g, b);
            if (!same_edges(fast.ins, nb.ins) || !same_edges(fast.del, nb.del)) return "Normaliser";
            if (!same_groups(arms[0]->g, nb.del, grouper) || !same_groups(arms[0]->g, nb.ins, grouper)) return "Grouper";
        }

        if (full && !nb.ins.empty() + !nb.del.empty() > 0) {
            Graph probe = graph_of(m);
            i64 was = brute5(m);
            Mat next = m;
            for (const Edge& e : nb.del) next.set(e.a, e.b, false);
            for (const Edge& e : nb.ins) next.set(e.a, e.b, true);
            if (naive_delta(probe, nb) != brute5(next) - was) ++st.naive_wrong;
        }

        for (auto& e : arms) {
            NetBatch mine = normalise(e->g, b);
            e->apply(b, mine, ~u64(0));
        }
        for (const Edge& e : nb.del) m.set(e.a, e.b, false);
        for (const Edge& e : nb.ins) m.set(e.a, e.b, true);
        ++st.batches;
        st.updates += nb.ins.size() + nb.del.size();
        st.ins += nb.ins.size();
        st.del += nb.del.size();

        const i64 truth = brute5(m);
        st.max_t = std::max<u64>(st.max_t, u64(truth));
        for (auto& e : arms) {
            if (e->t != truth) return std::string(arm_name(e->arm)) + " count";
            if (e->g.m() != arms[0]->g.m()) return std::string(arm_name(e->arm)) + " graph";
        }
        // The wedge tally that prices a table rebuild in HS-T.
        u64 wsum = 0;
        for (u32 v = 0; v < n; ++v) {
            u64 d = 0;
            for (u32 t = 0; t < n; ++t) d += m.has(v, t);
            wsum += d * (d - (d > 0 ? 1 : 0)) / 2;
        }
        for (size_t i = 0; i < 5; ++i)
            if (arms[i]->wedges != wsum) return std::string(arm_name(arms[i]->arm)) + " wedge tally";
        Engine5& rc = *arms[5];
        if (rc.rc.wedges > rc.rc.sum_min) return "RC wedge bound";
        if (rc.rc.inner > rc.rc.wedges * (u64(rc.rc.degeneracy) + 1)) return "RC inner bound";
        ++st.bound_checks;

        if (full) {
            ++st.static_checks;
            if ((st.batches & 15) == 0) {
                ++st.walk_checks;
                if (closed_walks5(m) != truth) return "closed walks";
            }
            // The wedge table, entry by entry, in both table arms.
            auto tw = true_wedges(m);
            for (int i : {2, 4}) {
                Wedge& w = arms[size_t(i)]->wedge;
                if (w.size() != tw.size()) return "table size";
                bool bad = false;
                w.for_each([&](u64 k, u32 v) { auto f = tw.find(k); if (f == tw.end() || f->second != v) bad = true; });
                if (bad) return "table entry";
            }
            if (!wt_matches(arms[6]->wt, tw)) return "WT table";
            if (!wt_matches(arms[7]->wt, tw)) return "WT table, saturated form";
            st.saturated = std::max(st.saturated, arms[7]->wt.peak_saturated());
            ++st.table_checks;
            // Every kernel on every pair, present or absent.
            if ((st.batches & 7) == 0) {
                Graph g(n);
                Row row(n);
                TwoRows two(n);
                Wedge w;
                WTable wt(n), wts(n, 2);
                Cost c;
                for (u32 u = 0; u < n; ++u)
                    for (u32 v = u + 1; v < n; ++v)
                        if (m.has(u, v)) {
                            wedge_upkeep(g, u, v, true, w, c);  // builds graph and table up together, pair by pair
                            wt_upkeep(g, u, v, true, wt, c);
                            wt_upkeep(g, u, v, true, wts, c);
                            g.insert(u, v);
                        }
                for (u32 u = 0; u < n; ++u)
                    for (u32 v = u + 1; v < n; ++v) {
                        const i64 want = brute_through(m, u, v);
                        if (through_r2(g, u, v, row, c) != want) return "through R2";
                        if (through_r2b(g, u, v, two, c) != want) return "through R2B";
                        const bool had = g.has(u, v);
                        if (had) {
                            g.erase(u, v);
                            wedge_upkeep(g, u, v, false, w, c);
                            wt_upkeep(g, u, v, false, wt, c);
                            wt_upkeep(g, u, v, false, wts, c);
                        }
                        if (through_wj(g, u, v, w, c) != want) return "through WJ";
                        if (wt.join(g, u, v, c) != want) return "through WT";
                        if (wts.join(g, u, v, c) != want) return "through WT, saturated form";
                        if (had) {
                            wedge_upkeep(g, u, v, true, w, c);
                            wt_upkeep(g, u, v, true, wt, c);
                            wt_upkeep(g, u, v, true, wts, c);
                            g.insert(u, v);
                        }
                        ++st.pair_checks;
                    }
            }
        }
    }
    st.stars += arms[3]->groups;
    st.spokes += arms[3]->spokes;
    if (opcheck) {
        u64 h = 0;
        for (auto& e : arms) h = h * 1000003 + e->c.total();
        *opcheck = i64(h);
    }
    return "";
}

// Graphs of 13 vertices give tables of a few slots, where a wrap at the end of a table or a long probe chain is rare, and those are what WT
// depends on at scale. This drives tables of hundreds of slots through growth and shrink, in both forms, against the definition. The density
// swings between phases, so tables fill and empty again. Empty text means clean.
std::string stress_wt(u64 seed, u64& checks) {
    std::mt19937_64 q(seed);
    const u32 n = 140;
    Mat m(n);
    Graph g(n);
    Cost c;
    WTable a(n), b(n, 2);
    u64 hi = 0;
    for (int i = 0; i < 24000; ++i) {
        const u32 x = q() % 3 ? u32(q() % 9) : u32(q() % n), y = u32(q() % n);
        if (x == y) continue;
        const int pct = (i / 4000) % 2 ? 22 : 88;
        const bool on = int(q() % 100) < pct;
        const bool had = m.has(x, y);
        if (on == had) continue;
        if (on) {
            wt_upkeep(g, x, y, true, a, c);
            wt_upkeep(g, x, y, true, b, c);
            g.insert(x, y);
        } else {
            g.erase(x, y);
            wt_upkeep(g, x, y, false, a, c);
            wt_upkeep(g, x, y, false, b, c);
        }
        m.set(x, y, on);
        hi = std::max(hi, b.peak_saturated());
        if (i % 1500 != 0) continue;
        auto tw = true_wedges(m);
        if (!wt_matches(a, tw)) return "WT stress table";
        if (!wt_matches(b, tw)) return "WT stress table, saturated form";
        for (int k = 0; k < 12; ++k) {
            const u32 u = q() % 2 ? u32(q() % 9) : u32(q() % n), v = u32(q() % n);
            if (u == v) continue;
            const i64 want = brute_through(m, u, v);
            const bool present = m.has(u, v);
            if (present) {
                g.erase(u, v);
                wt_upkeep(g, u, v, false, a, c);
                wt_upkeep(g, u, v, false, b, c);
            }
            if (a.join(g, u, v, c) != want) return "WT stress join";
            if (b.join(g, u, v, c) != want) return "WT stress join, saturated form";
            if (present) {
                wt_upkeep(g, u, v, true, a, c);
                wt_upkeep(g, u, v, true, b, c);
                g.insert(u, v);
            }
            ++checks;
        }
    }
    if (hi == 0) return "WT stress never saturated";
    return "";
}

// Mutants fail on the first mismatch of any arm, so this counts wrong trials out of a fixed number.
u64 hunt(int mutant, u64 trials, u64 seed) {
    MUT = mutant;
    std::mt19937_64 r(seed + u64(mutant) * 7919);
    u64 wrong = 0;
    Stats st;
    for (u64 t = 0; t < trials; ++t)
        if (!one_trial(r, false, st).empty()) ++wrong;
    MUT = 0;
    return wrong;
}

}  // namespace

int main(int argc, char** argv) {
    const u64 trials = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 20000;
    const u64 seed = argc > 2 ? std::strtoull(argv[2], nullptr, 10) : 7;
    std::mt19937_64 r(seed);
    Stats st;
    bool ok = true;
    for (u64 t = 0; t < trials && ok; ++t) {
        std::string why = one_trial(r, true, st);
        if (!why.empty()) {
            std::printf("FAIL at trial %llu: %s\n", (unsigned long long)t, why.c_str());
            ok = false;
        }
    }
    std::printf("trials %llu, batches %llu, updates %llu (ins %llu, del %llu), no-ops %llu, repeated pairs %llu\n", (unsigned long long)trials,
                (unsigned long long)st.batches, (unsigned long long)st.updates, (unsigned long long)st.ins, (unsigned long long)st.del,
                (unsigned long long)st.noop, (unsigned long long)st.repeated);
    std::printf("stars %llu, spokes %llu, pair checks %llu, table checks %llu, closed-walk checks %llu, RC bound checks %llu, largest count %llu, most saturated pairs %llu\n",
                (unsigned long long)st.stars, (unsigned long long)st.spokes, (unsigned long long)st.pair_checks, (unsigned long long)st.table_checks,
                (unsigned long long)st.walk_checks, (unsigned long long)st.bound_checks, (unsigned long long)st.max_t, (unsigned long long)st.saturated);
    if (st.saturated == 0) { ok = false; std::puts("FAIL: the saturated form never saturated"); }

    // Operation counts are deterministic.
    {
        std::mt19937_64 a(seed + 1), b(seed + 1);
        Stats s1, s2;
        i64 h1 = 0, h2 = 0;
        for (int i = 0; i < 200; ++i) { one_trial(a, false, s1, &h1); one_trial(b, false, s2, &h2); if (h1 != h2) { ok = false; std::puts("FAIL: operation counts differ between runs"); } }
    }

    // A recount cut off by its budget must leave later recounts exact.
    {
        std::mt19937_64 q(seed + 2);
        Mat m(40);
        Graph g(40);
        Active act(40);
        Recount rc(40);
        Cost c;
        for (int i = 0; i < 400; ++i) {
            u32 a = u32(q() % 40), b = u32(q() % 40);
            if (a == b) continue;
            bool on = !m.has(a, b);
            m.set(a, b, on);
            if (on) g.insert(a, b); else g.erase(a, b);
            act.touch(g, a, c);
            act.touch(g, b, c);
            i64 res = 0;
            u64 ops = 0;
            if (i % 3 == 0) rc.count(g, act.list, q() % 4000, res, ops);
            if (i % 3 == 1 && (!rc.count(g, act.list, ~u64(0), res, ops) || res != brute5(m))) { ok = false; std::puts("FAIL: RC after a stopped recount"); break; }
        }
        std::puts("RC after stopped recounts: done");
    }

    // The memory cap must stop the table, and a run past the cap must end, not hang.
    {
        Wedge w;
        w.cap_bytes = 200000;
        Cost c;
        for (u32 a = 0; a < 2000 && !w.capped; ++a)
            for (u32 b = a + 1; b < 2000 && !w.capped; ++b) w.add(Wedge::pair_key(a, b), c);
        for (u32 a = 0; a < 2000; ++a) w.sub(Wedge::pair_key(a, 1999), c);
        if (!w.capped || w.bytes() > w.cap_bytes) { ok = false; std::puts("FAIL: table cap"); }
        std::printf("table cap: stopped at %llu entries, %llu bytes\n", (unsigned long long)w.size(), (unsigned long long)w.bytes());
        WTable t(2000);
        t.cap_bytes = 200000;
        std::vector<u32> list;
        for (u32 b = 1; b < 2000; ++b) list.push_back(b);
        for (u32 a = 2000; a-- > 1000 && !t.capped;) t.change_list(a, list, true, c);
        t.change_list(1999, list, false, c);
        if (!t.capped || t.peak_bytes() > t.cap_bytes) { ok = false; std::puts("FAIL: WT cap"); }
        std::printf("WT cap: stopped at %llu pairs, %llu bytes\n", (unsigned long long)t.size(), (unsigned long long)t.peak_bytes());
    }

    // A larger hub-heavy stream, with the same oracle at lower frequency, to reach epochs and table growth that small graphs do not.
    {
        std::mt19937_64 q(seed + 3);
        const u32 n = 48;
        Mat m(n);
        std::vector<std::unique_ptr<Engine5>> arms;
        for (Arm a : kArms) arms.push_back(std::make_unique<Engine5>(a, n));
        arms.push_back(wt_saturated(n));
        u64 checks = 0;
        for (int bi = 0; bi < 600 && ok; ++bi) {
            std::vector<Update> b;
            const u32 star = u32(q() % 4);
            for (int i = 0; i < 16; ++i) {
                u32 x = q() % 2 ? star : u32(q() % n), y = u32(q() % n);
                if (x == y) continue;
                b.push_back({x, y, q() % 3 != 0});
            }
            NetBatch nb = normalise(arms[0]->g, b);
            for (auto& e : arms) { NetBatch mine = normalise(e->g, b); e->apply(b, mine, ~u64(0)); }
            for (const Edge& e : nb.del) m.set(e.a, e.b, false);
            for (const Edge& e : nb.ins) m.set(e.a, e.b, true);
            const i64 truth = brute5(m);
            ++checks;
            for (auto& e : arms)
                if (e->t != truth) { ok = false; std::printf("FAIL: %s on the large stream, batch %d\n", arm_name(e->arm), bi); }
        }
        std::printf("large stream: %llu batches, final count %lld, edges %llu\n", (unsigned long long)checks, (long long)arms[0]->t, (unsigned long long)arms[0]->g.m());
        auto tw = true_wedges(m);
        if (!wt_matches(arms[6]->wt, tw) || !wt_matches(arms[7]->wt, tw)) { ok = false; std::puts("FAIL: WT table on the large stream"); }
        std::printf("large stream, WT: %llu pairs, %llu saturated in the forced form\n", (unsigned long long)arms[6]->wt.size(), (unsigned long long)arms[7]->wt.peak_saturated());
    }

    // WT on tables far larger than the small graphs reach.
    {
        u64 checks = 0;
        for (u64 i = 0; i < 40 && ok; ++i) {
            const std::string why = stress_wt(seed + 100 + i, checks);
            if (!why.empty()) { ok = false; std::printf("FAIL: %s, stress seed %llu\n", why.c_str(), (unsigned long long)i); }
        }
        std::printf("WT stress: 40 runs, %llu joins checked against the definition\n", (unsigned long long)checks);
    }

    // The control must fail.
    std::printf("control, naive per-edge sum: wrong on %llu of %llu batches\n", (unsigned long long)st.naive_wrong, (unsigned long long)st.static_checks);
    if (st.naive_wrong == 0) { ok = false; std::puts("FAIL: the naive control passed, so this test proves nothing"); }

    // Every mutant must fail.
    const u64 mt = std::min<u64>(trials, 4000);
    for (int mutant = 1; mutant <= 18; ++mutant) {
        u64 wrong = hunt(mutant, mt, seed);
        std::printf("mutant %2d: wrong on %llu of %llu trials", mutant, (unsigned long long)wrong, (unsigned long long)mt);
        if (mutant >= 13) {             // the WT mutants must also fall to the stress test, which is where they matter
            MUT = mutant;
            u64 fails = 0, checks = 0;
            for (u64 i = 0; i < 12; ++i) fails += !stress_wt(seed + 500 + i, checks).empty();
            MUT = 0;
            std::printf(", stress runs failed %llu of 12", (unsigned long long)fails);
            if (fails == 0) { ok = false; std::printf("\nFAIL: mutant %d passed the stress test", mutant); }
        }
        std::puts("");
        if (wrong == 0) { ok = false; std::printf("FAIL: mutant %d passed\n", mutant); }
    }
    std::puts(ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
