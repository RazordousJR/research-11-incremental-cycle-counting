// Exactness oracle for M1B01. After every event of random small temporal graphs, every method and every window is
// compared with a brute-force recount over all vertex triples, and array B is compared slot by slot with the histogram of
// sigma. The graphs have repeats, equal timestamps and horizon expiry. Two controls must fail, or this test proves nothing.
// Usage: r11_test_window [trials] [seed]
#include <cstdio>
#include <cstdlib>
#include <map>
#include <random>

#define WINDOW_SWEEP_NO_MAIN
#include "../benchmarks/window_sweep.cpp"

using namespace ws;

namespace {

constexpr i64 ABSENT = std::numeric_limits<i64>::min() / 2;

// The definition, nothing else: last event slot per pair, and the in-window test read straight off the protocol.
struct Brute {
    u32 n;
    std::vector<i64> last;
    explicit Brute(u32 nv) : n(nv), last(size_t(nv) * nv, ABSENT) {}
    void event(u32 u, u32 v, i64 s) { last[size_t(u) * n + v] = s; last[size_t(v) * n + u] = s; }
    // counts[i]: triples with all three pairs strictly above s - len[i]. tied[i]: same with >=, for the tie control.
    void recount(i64 s, const std::vector<i64>& len, std::vector<i64>& counts, std::vector<i64>& tied,
                 std::map<i64, i64>& sigma) const {
        std::fill(counts.begin(), counts.end(), 0);
        std::fill(tied.begin(), tied.end(), 0);
        sigma.clear();
        for (u32 a = 0; a < n; ++a)
            for (u32 b = a + 1; b < n; ++b) {
                i64 ab = last[size_t(a) * n + b];
                if (ab == ABSENT) continue;
                for (u32 c = b + 1; c < n; ++c) {
                    i64 m = std::min({ab, last[size_t(a) * n + c], last[size_t(b) * n + c]});
                    if (m == ABSENT) continue;
                    for (size_t i = 0; i < len.size(); ++i) {
                        if (m > s - len[i]) ++counts[i];
                        if (m >= s - len[i]) ++tied[i];
                    }
                    if (m > s - len.back()) ++sigma[m];
                }
            }
    }
};

struct Coverage { u64 events = 0, equal = 0, repeats = 0, reentries = 0, jumps = 0; };

struct Trial { u32 n; i64 width; std::vector<i64> len; std::vector<std::vector<i64>> ev; };

Trial make_trial(std::mt19937_64& r, Coverage& cov, bool single_window) {
    Trial t;
    t.n = 4 + u32(r() % 9);
    t.width = i64(1 + r() % 4);                     // raw times are quantised by this width
    i64 a = i64(1 + r() % 3), b = a + i64(r() % 4), c = b + 1 + i64(r() % 6), d = c + 1 + i64(r() % 10);
    if (single_window) t.len = {d}; else t.len = {a, b, c, d};
    u32 m = 30 + u32(r() % 90);
    double p_zero = 0.1 + 0.6 * double(r() % 100) / 100.0, p_repeat = 0.2 + 0.6 * double(r() % 100) / 100.0;
    double p_jump = double(r() % 100) / 2000.0;
    std::bernoulli_distribution zero(p_zero), rep(p_repeat), jump(p_jump);
    std::vector<std::pair<u32, u32>> seen;
    std::vector<i64> last(size_t(t.n) * t.n, ABSENT);
    i64 raw = i64(r() % 1000);
    for (u32 i = 0; i < m; ++i) {
        i64 before = raw / t.width;
        if (jump(r)) { raw += (d + 1 + i64(r() % d)) * t.width; ++cov.jumps; }
        else if (!zero(r)) raw += i64(1 + r() % (2 * t.width));
        i64 s = raw / t.width;
        if (s == before && i) ++cov.equal;
        u32 u, v;
        if (!seen.empty() && rep(r)) {
            auto q = seen[r() % seen.size()];
            u = q.first; v = q.second;
        } else {
            do { u = u32(r() % t.n); v = u32(r() % t.n); } while (u == v);
            seen.push_back({u, v});
        }
        i64& l = last[size_t(std::min(u, v)) * t.n + std::max(u, v)];
        if (l != ABSENT) { ++cov.repeats; if (l <= s - d) ++cov.reentries; }
        l = s;
        t.ev.push_back({i64(u), i64(v), s});
        ++cov.events;
    }
    return t;
}

}  // namespace

int main(int argc, char** argv) {
    u64 trials = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 20000;
    u64 seed = argc > 2 ? std::strtoull(argv[2], nullptr, 10) : 1;
    std::mt19937_64 r(seed);

    u64 fail_w = 0, fail_w0 = 0, fail_b2p = 0, fail_b2r = 0, fail_b1 = 0, fail_p = 0, fail_d = 0, fail_b = 0, checks = 0;
    u64 control_add_wrong = 0, control_add_runs = 0, control_tie_wrong = 0, control_tie_runs = 0;
    u64 moved = 0, added = 0, fallbacks_w = 0, fallbacks_b2 = 0, rehomed = 0, noops = 0;
    Coverage cov;

    for (u64 trial = 0; trial < trials; ++trial) {
        bool single = trial % 2 == 1;               // half the trials are k = 1 with the death counter
        Trial t = make_trial(r, cov, single);
        size_t k = t.len.size();
        Brute br(t.n);
        MethodW W(t.n, t.len, true), W0(t.n, t.len, false), control(t.n, t.len, true, true);
        MethodB2 B2p(t.n, t.len, false), B2r(t.n, t.len, true);
        MethodB1 B1(t.n, t.len);
        MethodP P(t.n, t.len.back());
        MethodD D(t.n, t.len.back());
        std::vector<i64> counts(k), tied(k);
        std::map<i64, i64> sigma;
        bool add_wrong = false, tie_wrong = false;
        for (const auto& e : t.ev) {
            u32 u = u32(e[0]), v = u32(e[1]);
            i64 s = e[2];
            br.event(u, v, s);
            W.event(u, v, s);
            W0.event(u, v, s);
            control.event(u, v, s);
            B2p.event(u, v, s);
            B2r.event(u, v, s);
            B1.event(u, v, s);
            if (single) { P.event(u, v, s); D.event(u, v, s); }
            br.recount(s, t.len, counts, tied, sigma);
            for (size_t i = 0; i < k; ++i) {
                ++checks;
                fail_w += W.count(i) != counts[i];
                fail_w0 += W0.count(i) != counts[i];
                fail_b2p += B2p.count(i) != counts[i];
                fail_b2r += B2r.count(i) != counts[i];
                fail_b1 += B1.count(i) != counts[i];
                if (control.count(i) != counts[i]) add_wrong = true;
                if (W.count(i) != tied[i]) tie_wrong = true;
            }
            if (single) { fail_p += P.count() != counts[0]; fail_d += D.count() != counts[0]; }
            // B holds exactly one unit per live triple, at the slot of its sigma
            i64 lo = s - t.len.back();
            for (i64 x = std::max<i64>(0, lo + 1); x <= s; ++x) {
                auto it = sigma.find(x);
                i64 want = it == sigma.end() ? 0 : it->second;
                fail_b += W.slot(x) != want;
                fail_b += W0.slot(x) != want;
            }
        }
        ++control_add_runs;
        ++control_tie_runs;
        control_add_wrong += add_wrong;
        control_tie_wrong += tie_wrong;
        moved += W.st.moved;
        added += W.st.added;
        noops += W.st.noops;
        fallbacks_w += W.st.fallbacks;
        fallbacks_b2 += B2r.st.fallbacks;
        rehomed += D.st.rehomed;
    }

    std::printf("trials=%llu seed=%llu events=%llu window_checks=%llu\n", (unsigned long long)trials, (unsigned long long)seed,
                (unsigned long long)cov.events, (unsigned long long)checks);
    std::printf("coverage: equal-slot events=%llu repeats=%llu re-entries after horizon=%llu time jumps past horizon=%llu same-slot no-ops=%llu\n",
                (unsigned long long)cov.equal, (unsigned long long)cov.repeats, (unsigned long long)cov.reentries,
                (unsigned long long)cov.jumps, (unsigned long long)noops);
    std::printf("coverage: W moved=%llu added=%llu log fallbacks W=%llu B2r=%llu death re-homed=%llu\n", (unsigned long long)moved,
                (unsigned long long)added, (unsigned long long)fallbacks_w, (unsigned long long)fallbacks_b2, (unsigned long long)rehomed);
    std::printf("failures: W=%llu W0=%llu B2_plain=%llu B2_recency=%llu B1=%llu P=%llu D=%llu B_histogram=%llu\n",
                (unsigned long long)fail_w, (unsigned long long)fail_w0, (unsigned long long)fail_b2p, (unsigned long long)fail_b2r,
                (unsigned long long)fail_b1, (unsigned long long)fail_p, (unsigned long long)fail_d, (unsigned long long)fail_b);
    std::printf("control, plain add for the move rule: wrong on %llu of %llu trials\n", (unsigned long long)control_add_wrong,
                (unsigned long long)control_add_runs);
    std::printf("control, W against the >= tie rule: differs on %llu of %llu trials\n", (unsigned long long)control_tie_wrong,
                (unsigned long long)control_tie_runs);
    bool ok = !(fail_w | fail_w0 | fail_b2p | fail_b2r | fail_b1 | fail_p | fail_d | fail_b) && control_add_wrong > 0 &&
              control_tie_wrong > 0 && fallbacks_w > 0 && fallbacks_b2 > 0 && rehomed > 0 && cov.reentries > 0;
    std::puts(ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
