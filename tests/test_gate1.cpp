// Exactness oracle for Gate 1. After every batch of random small graphs, every arm of every pattern is compared with a brute-force count over vertex tuples.
// Mixed arms run at several values of theta, down to 1, so that vertices turn heavy and light on graphs this small. The batches have deletions, hubs, whole
// stars, repeated pairs inside one batch, no-op inserts and no-op deletes. The naive per-edge sum is a control and must fail. Each mutant below is a real way to
// get this wrong, and the test FAILS if any one passes.
//   20  M: a heavy centre's wedges still written to the table      27  P4: spoke not added to the row, insertions
//   21  M: a vertex that turns heavy leaves its wedges in place    28  P4: spoke not taken out of the row, deletions
//   22  M: hub lists not kept on edge edits                        29  B4: pairs with both ends in S read twice
//   23  M5: T[h] not subtracted                                    30  B4: paths through an old edge missed in the difference row
//   24  M4: hub term dropped                                       31  A: ball radius one too small
//   25  P3: spoke not added to the row, insertions                 32  A: deleted edges kept in the new subgraph
//   26  P3: spoke not taken out of the row, deletions              33  A4: wedge pairing without the rank test on w
// Usage: r11_test_gate1 [trials] [seed]
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <random>

#define GATE1_NO_MAIN
#include "../benchmarks/gate1.cpp"

using namespace g1;

namespace {

struct Mat {
    u32 n;
    std::vector<char> a;
    explicit Mat(u32 nv) : n(nv), a(size_t(nv) * nv, 0) {}
    bool has(u32 u, u32 v) const { return a[size_t(u) * n + v]; }
    void set(u32 u, u32 v, bool on) { a[size_t(u) * n + v] = on; a[size_t(v) * n + u] = on; }
};

i64 brute3(const Mat& m) {
    i64 c = 0;
    for (u32 a = 0; a < m.n; ++a)
        for (u32 b = a + 1; b < m.n; ++b)
            if (m.has(a, b))
                for (u32 d = b + 1; d < m.n; ++d) c += m.has(a, d) && m.has(b, d);
    return c;
}

// Smallest vertex first, and one of the two directions.
i64 brute4(const Mat& m) {
    i64 c = 0;
    for (u32 v0 = 0; v0 < m.n; ++v0)
        for (u32 v1 = v0 + 1; v1 < m.n; ++v1) {
            if (!m.has(v0, v1)) continue;
            for (u32 v2 = v0 + 1; v2 < m.n; ++v2) {
                if (v2 == v1 || !m.has(v1, v2)) continue;
                for (u32 v3 = v1 + 1; v3 < m.n; ++v3)
                    if (v3 != v2 && m.has(v2, v3) && m.has(v3, v0)) ++c;
            }
        }
    return c;
}

i64 brute5(const Mat& m) {
    i64 c = 0;
    for (u32 v0 = 0; v0 < m.n; ++v0)
        for (u32 v1 = v0 + 1; v1 < m.n; ++v1) {
            if (!m.has(v0, v1)) continue;
            for (u32 v2 = v0 + 1; v2 < m.n; ++v2) {
                if (v2 == v1 || !m.has(v1, v2)) continue;
                for (u32 v3 = v0 + 1; v3 < m.n; ++v3) {
                    if (v3 == v1 || v3 == v2 || !m.has(v2, v3)) continue;
                    for (u32 v4 = v1 + 1; v4 < m.n; ++v4)
                        if (v4 != v2 && v4 != v3 && m.has(v3, v4) && m.has(v4, v0)) ++c;
                }
            }
        }
    return c;
}

// Checks the two small brute forces from outside: tr A^3 = 6 c3, and tr A^4 = 8 c4 + 2 m + 4 sum C(d, 2), with m edges.
bool walks_agree(const Mat& m) {
    const u32 n = m.n;
    std::vector<i64> a2(size_t(n) * n, 0);
    for (u32 i = 0; i < n; ++i)
        for (u32 k = 0; k < n; ++k)
            if (m.has(i, k))
                for (u32 j = 0; j < n; ++j) a2[size_t(i) * n + j] += m.has(k, j);
    i64 tr3 = 0, tr4 = 0, edges = 0, wedges = 0;
    for (u32 i = 0; i < n; ++i) {
        i64 d = 0;
        for (u32 j = 0; j < n; ++j) {
            d += m.has(i, j);
            tr3 += a2[size_t(i) * n + j] * m.has(j, i);
            tr4 += a2[size_t(i) * n + j] * a2[size_t(j) * n + i];
        }
        edges += d;
        wedges += d * (d - 1) / 2;
    }
    edges /= 2;
    return tr3 == 6 * brute3(m) && tr4 == 8 * brute4(m) + 2 * edges + 4 * wedges;
}

i64 brute(int p, const Mat& m) { return p == 3 ? brute3(m) : p == 4 ? brute4(m) : brute5(m); }

// Inserts as a set, then every edge is summed on the same graph, so a cycle with two new edges is counted twice.
i64 naive_delta(int p, const Mat& m, const NetBatch& nb) {
    Graph g(m.n);
    for (u32 a = 0; a < m.n; ++a)
        for (u32 b = a + 1; b < m.n; ++b)
            if (m.has(a, b)) g.insert(a, b);
    Row row(m.n);
    Cost c;
    i64 d = 0;
    for (const Edge& e : nb.del) d -= p == 3 ? through3(g, e.a, e.b, c) : through4(g, e.a, e.b, row, c);
    for (const Edge& e : nb.del) g.erase(e.a, e.b);
    for (const Edge& e : nb.ins) g.insert(e.a, e.b);
    for (const Edge& e : nb.ins) d += p == 3 ? through3(g, e.a, e.b, c) : through4(g, e.a, e.b, row, c);
    return d;
}

struct Spec { int p; const char* arm; u32 theta; };

const std::vector<Spec>& specs() {
    static const std::vector<Spec> s = {
        {3, "S3", 0}, {3, "P3", 0}, {3, "T3", 0}, {3, "M3", 1}, {3, "M3", 2}, {3, "M3", 4},
        {4, "S4", 0}, {4, "P4", 0}, {4, "T4", 0}, {4, "M4", 1}, {4, "M4", 2}, {4, "M4", 4}, {4, "B4", 0}, {4, "A4", 0},
        {5, "R2", 0}, {5, "PR", 0}, {5, "WT", 0}, {5, "WJ", 0}, {5, "M5", 1}, {5, "M5", 2}, {5, "M5", 4}, {5, "A5", 0},
    };
    return s;
}

struct Stats { u64 batches = 0, updates = 0, naive_wrong[6] = {0, 0, 0, 0, 0, 0}, naive_seen[6] = {0, 0, 0, 0, 0, 0}, turns = 0, stars = 0, walks = 0; };

std::string one_trial(std::mt19937_64& r, bool full, Stats& st) {
    std::uniform_real_distribution<double> uni(0, 1);
    const u32 n = 5 + u32(r() % 9);
    const int family = int(r() % 6);
    const double p = family == 0 ? 0.15 : family == 1 ? 0.55 : family == 2 ? 0.3 : family == 3 ? 0.1 : family == 4 ? 0.8 : 0.4;
    const u32 hub = u32(r() % n), hub2 = u32(r() % n);
    auto rand_pair = [&](u32& a, u32& b) {
        do {
            if ((family == 2 || family == 3) && uni(r) < 0.55) { a = uni(r) < 0.6 ? hub : hub2; b = u32(r() % n); }
            else { a = u32(r() % n); b = u32(r() % n); }
        } while (a == b);
    };
    Mat m(n);
    std::vector<std::unique_ptr<Counter>> arms;
    for (const Spec& s : specs()) arms.push_back(std::make_unique<Counter>(s.p, s.arm, n, s.theta));
    Normaliser norm;
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
            const u32 size = 1 + u32(r() % (2 * n)), star = u32(r() % n);
            for (u32 i = 0; i < size; ++i) {
                u32 x, y;
                if (style == 1) { x = star; do y = u32(r() % n); while (y == x); }
                else rand_pair(x, y);
                bool ins = style == 2 ? true : style == 3 ? false : uni(r) < 0.5;
                if (style == 4 && !b.empty() && uni(r) < 0.5) {
                    const Update& o = b[r() % b.size()];
                    x = o.v; y = o.u;
                    ins = uni(r) < 0.5;
                }
                b.push_back({x, y, ins});
            }
        }
        const NetBatch nb = normalise(arms[0]->graph(), b);
        const i64 was3 = brute3(m), was4 = full ? brute4(m) : 0;
        if (full && !(nb.ins.empty() && nb.del.empty())) {
            Mat next = m;
            for (const Edge& e : nb.del) next.set(e.a, e.b, false);
            for (const Edge& e : nb.ins) next.set(e.a, e.b, true);
            ++st.naive_seen[3];
            ++st.naive_seen[4];
            st.naive_wrong[3] += naive_delta(3, m, nb) != brute3(next) - was3;
            st.naive_wrong[4] += naive_delta(4, m, nb) != brute4(next) - was4;
        }
        for (auto& e : arms) {
            const NetBatch& mine = norm.run(e->graph(), b);
            e->apply(b, mine);
        }
        for (const Edge& e : nb.del) m.set(e.a, e.b, false);
        for (const Edge& e : nb.ins) m.set(e.a, e.b, true);
        ++st.batches;
        st.updates += nb.ins.size() + nb.del.size();
        const i64 truth[6] = {0, 0, 0, brute3(m), brute4(m), brute5(m)};
        if (full && (st.batches & 15) == 0) {
            ++st.walks;
            if (!walks_agree(m)) return "closed walks";
        }
        for (size_t i = 0; i < arms.size(); ++i) {
            if (arms[i]->count() != truth[arms[i]->p]) return "p" + std::to_string(arms[i]->p) + " " + arms[i]->arm + " theta " + std::to_string(specs()[i].theta) + " count";
            if (arms[i]->graph().m() != arms[0]->graph().m()) return arms[i]->arm + " graph";
        }
    }
    for (const auto& e : arms) { st.turns += e->turns(); st.stars += e->groups; }
    return "";
}

// Tables of hundreds of slots and hubs that turn many times, against the definition.
std::string stress(u64 seed, u64& checks, u64& turns) {
    std::mt19937_64 q(seed);
    const u32 n = 60;
    Mat m(n);
    std::vector<std::unique_ptr<Counter>> arms;
    for (const Spec& s : std::vector<Spec>{{3, "M3", 6}, {4, "M4", 6}, {4, "T4", 0}, {5, "M5", 6}, {3, "T3", 0}})
        arms.push_back(std::make_unique<Counter>(s.p, s.arm, n, s.theta));
    // Two count bits, so that a count read goes through the side table.
    arms.push_back(std::make_unique<Counter>(3, "T3", n, 0, 2));
    arms.push_back(std::make_unique<Counter>(4, "T4", n, 0, 2));
    arms.push_back(std::make_unique<Counter>(5, "M5", n, 6, 2));
    Normaliser norm;
    for (int bi = 0; bi < 300; ++bi) {
        std::vector<Update> b;
        const u32 star = u32(q() % 5);
        const int pct = (bi / 40) % 2 ? 25 : 85;
        for (int i = 0; i < 12; ++i) {
            const u32 x = q() % 2 ? star : u32(q() % n), y = u32(q() % n);
            if (x != y) b.push_back({x, y, int(q() % 100) < pct});
        }
        const NetBatch nb = normalise(arms[0]->graph(), b);
        for (auto& e : arms) { const NetBatch& mine = norm.run(e->graph(), b); e->apply(b, mine); }
        for (const Edge& e : nb.del) m.set(e.a, e.b, false);
        for (const Edge& e : nb.ins) m.set(e.a, e.b, true);
        for (auto& e : arms) {
            ++checks;
            if (e->count() != brute(e->p, m)) return std::string("stress ") + e->arm;
        }
    }
    for (const auto& e : arms) turns += e->turns();
    if (arms[5]->saturated() == 0 || arms[6]->saturated() == 0) return "stress never saturated";
    return "";
}

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
    const u64 trials = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 4000;
    const u64 seed = argc > 2 ? std::strtoull(argv[2], nullptr, 10) : 7;
    std::mt19937_64 r(seed);
    Stats st;
    bool ok = true;
    for (u64 t = 0; t < trials && ok; ++t) {
        const std::string why = one_trial(r, true, st);
        if (!why.empty()) { std::printf("FAIL at trial %llu: %s\n", (unsigned long long)t, why.c_str()); ok = false; }
    }
    std::printf("trials %llu, batches %llu, updates %llu, stars %llu, heavy turns %llu, closed-walk checks %llu\n", (unsigned long long)trials, (unsigned long long)st.batches,
                (unsigned long long)st.updates, (unsigned long long)st.stars, (unsigned long long)st.turns, (unsigned long long)st.walks);
    if (st.turns == 0) { ok = false; std::puts("FAIL: no vertex ever turned heavy"); }

    u64 checks = 0, turns = 0;
    for (u64 i = 0; i < 12 && ok; ++i) {
        const std::string why = stress(seed + 100 + i, checks, turns);
        if (!why.empty()) { ok = false; std::printf("FAIL: %s, stress seed %llu\n", why.c_str(), (unsigned long long)i); }
    }
    std::printf("stress: %llu counts checked, %llu heavy turns\n", (unsigned long long)checks, (unsigned long long)turns);

    for (int p : {3, 4}) {
        std::printf("control p=%d, naive per-edge sum: wrong on %llu of %llu batches\n", p, (unsigned long long)st.naive_wrong[p], (unsigned long long)st.naive_seen[p]);
        if (st.naive_wrong[p] == 0) { ok = false; std::puts("FAIL: the naive control passed, so this test proves nothing"); }
    }

    const u64 mt = std::min<u64>(trials, 2000);
    for (int mutant = 20; mutant <= 33; ++mutant) {
        const u64 wrong = hunt(mutant, mt, seed);
        std::printf("mutant %2d: wrong on %llu of %llu trials\n", mutant, (unsigned long long)wrong, (unsigned long long)mt);
        if (wrong == 0) { ok = false; std::printf("FAIL: mutant %d passed\n", mutant); }
    }
    std::puts(ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
