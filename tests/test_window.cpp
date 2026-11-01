// Exactness oracle for M1B01. After every event of random small temporal graphs, every method and every window is
// compared with a brute-force recount over all vertex triples, and array B is compared slot by slot with the histogram of
// sigma. The graphs have repeats, equal timestamps and horizon expiry. Two controls must fail, or this test proves nothing.
// M1B01-B adds W-F and D-k, M1C01 adds V, K and DI. Every mutant must fail, or the test fails.
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

// D-k against the definition. Triples come from the live pairs, homes from the method's own believed times.
// Returns the number of broken statements: I1 per pair, I2, and the one-counter identity for each window.
u64 check_dk(MethodDK& dk, u32 n, i64 s, const std::vector<i64>& len, const std::vector<i64>& counts) {
    u64 bad = 0;
    Store& g = dk.graph();
    Cost c;
    std::map<u32, i64> tally;
    for (u32 a = 0; a < n; ++a)
        for (u32 b = a + 1; b < n; ++b) {
            u32 ab = g.find(a, b, c);
            if (ab == NONE) continue;
            for (u32 w = b + 1; w < n; ++w) {
                u32 aw = g.find(a, w, c), bw = g.find(b, w, c);
                if (aw == NONE || bw == NONE) continue;
                u32 h = ab;
                if (dk.earlier(aw, h)) h = aw;
                if (dk.earlier(bw, h)) h = bw;
                ++tally[h];
            }
        }
    std::vector<i64> sum(len.size(), 0);
    g.for_pairs([&](u32 e) {
        auto it = tally.find(e);
        bad += i64(dk.deaths(e)) != (it == tally.end() ? 0 : it->second);                  // I1
        i64 b = dk.believed(e), l = g.last(e);
        if (b > l || (b < l && b <= s - len[0])) ++bad;                                    // I2
        for (size_t i = 0; i < len.size(); ++i) if (b > s - len[i]) sum[i] += dk.deaths(e);
    });
    for (size_t i = 0; i < len.size(); ++i) bad += sum[i] != counts[i];                    // one counter serves window i
    return bad;
}

// V against the definition. Every record sits on the list of each of its pairs, every list holds exactly the triples on
// its pair, sigma is the true minimum, the links agree both ways, and the live total is the number of triples.
u64 check_v(MethodV& vw, u32 n) {
    u64 bad = 0, total = 0;
    Store& g = vw.graph();
    Cost c;
    std::map<u32, i64> tally;
    for (u32 a = 0; a < n; ++a)
        for (u32 b = a + 1; b < n; ++b) {
            u32 ab = g.find(a, b, c);
            if (ab == NONE) continue;
            for (u32 w = b + 1; w < n; ++w) {
                u32 aw = g.find(a, w, c), bw = g.find(b, w, c);
                if (aw == NONE || bw == NONE) continue;
                ++tally[ab]; ++tally[aw]; ++tally[bw];
                ++total;
            }
        }
    g.for_pairs([&](u32 e) {
        i64 on = 0;
        bool links = vw.walk(e, [&](u32 p0, u32 p1, u32 p2, i64 sigma) {
            ++on;
            bad += sigma != std::min({g.last(p0), g.last(p1), g.last(p2)});
        });
        bad += !links;
        auto it = tally.find(e);
        bad += on != (it == tally.end() ? 0 : it->second);
    });
    bad += vw.live() != total;
    return bad;
}

// K against the definition, window by window. d_i(e) counts the triples in window i homed on e, a pair is in window i exactly
// when b_i(e) says so, b_i never passes last, and the sum of d_i is the count of window i.
u64 check_k(MethodK& kc, u32 n, i64 s, const std::vector<i64>& len, const std::vector<i64>& counts) {
    u64 bad = 0;
    Store& g = kc.graph();
    Cost c;
    std::vector<std::map<u32, i64>> tally(len.size());
    for (u32 a = 0; a < n; ++a)
        for (u32 b = a + 1; b < n; ++b) {
            u32 ab = g.find(a, b, c);
            if (ab == NONE) continue;
            for (u32 w = b + 1; w < n; ++w) {
                u32 aw = g.find(a, w, c), bw = g.find(b, w, c);
                if (aw == NONE || bw == NONE) continue;
                i64 m = std::min({g.last(ab), g.last(aw), g.last(bw)});
                for (size_t i = 0; i < len.size(); ++i) {
                    if (m <= s - len[i]) continue;
                    u32 h = ab;
                    if (kc.earlier(aw, h, i)) h = aw;
                    if (kc.earlier(bw, h, i)) h = bw;
                    ++tally[i][h];
                }
            }
        }
    std::vector<i64> sum(len.size(), 0);
    g.for_pairs([&](u32 e) {
        for (size_t i = 0; i < len.size(); ++i) {
            auto it = tally[i].find(e);
            bad += i64(kc.deaths(e, i)) != (it == tally[i].end() ? 0 : it->second);
            i64 b = kc.believed(e, i), l = g.last(e);
            bad += b > l;
            bad += (b > s - len[i]) != (l > s - len[i]);
            sum[i] += kc.deaths(e, i);
        }
    });
    for (size_t i = 0; i < len.size(); ++i) bad += sum[i] != counts[i];
    return bad;
}

// DI against the definition. b = last at all times, so homes come from the last times alone, and the count of window i is the
// sum of d(e) over pairs in it.
u64 check_di(MethodDI& di, u32 n, i64 s, const std::vector<i64>& len, const std::vector<i64>& counts) {
    u64 bad = 0;
    Store& g = di.graph();
    Cost c;
    std::map<u32, i64> tally;
    for (u32 a = 0; a < n; ++a)
        for (u32 b = a + 1; b < n; ++b) {
            u32 ab = g.find(a, b, c);
            if (ab == NONE) continue;
            for (u32 w = b + 1; w < n; ++w) {
                u32 aw = g.find(a, w, c), bw = g.find(b, w, c);
                if (aw == NONE || bw == NONE) continue;
                u32 h = ab;
                if (di.earlier(aw, h)) h = aw;
                if (di.earlier(bw, h)) h = bw;
                ++tally[h];
            }
        }
    std::vector<i64> sum(len.size(), 0);
    g.for_pairs([&](u32 e) {
        auto it = tally.find(e);
        bad += i64(di.deaths(e)) != (it == tally.end() ? 0 : it->second);
        for (size_t i = 0; i < len.size(); ++i) if (g.last(e) > s - len[i]) sum[i] += di.deaths(e);
    });
    for (size_t i = 0; i < len.size(); ++i) bad += sum[i] != counts[i];
    return bad;
}

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
    u64 fail_wf = 0, fail_any = 0, fail_tree = 0, fail_dk = 0, fail_dk_state = 0, any_checks = 0;
    u64 wrong_wf[4] = {}, wrong_dk[4] = {}, rehomed_dk = 0, immediate_dk = 0;
    u64 fail_v = 0, fail_k = 0, fail_di = 0, fail_v_state = 0, fail_k_state = 0, fail_di_state = 0, fail_c = 0;
    u64 wrong_v[4] = {}, wrong_k[4] = {}, wrong_di[4] = {};
    u64 moved_v = 0, records_v = 0, reentry_k = 0, rehomed_k = 0, exit_scans_di = 0;
    u64 dk_split_bad = 0;
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
        MethodWF WF(t.n, t.len);
        MethodDK DK(t.n, t.len);
        std::vector<MethodWF> wf_mut;
        std::vector<MethodDK> dk_mut;
        for (int mu = 1; mu <= 3; ++mu) { wf_mut.emplace_back(t.n, t.len, mu); dk_mut.emplace_back(t.n, t.len, mu); }
        bool wf_bad[4] = {}, dk_bad[4] = {};
        MethodV V(t.n, t.len);
        MethodK K(t.n, t.len);
        MethodDI DI(t.n, t.len);
        std::vector<MethodV> v_mut;
        std::vector<MethodK> k_mut;
        std::vector<MethodDI> di_mut;
        for (int mu = 1; mu <= 3; ++mu) {
            v_mut.emplace_back(t.n, t.len, mu);
            k_mut.emplace_back(t.n, t.len, mu);
            di_mut.emplace_back(t.n, t.len, mu);
        }
        bool v_bad[4] = {}, k_bad[4] = {}, di_bad[4] = {};
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
            WF.event(u, v, s);
            WF.read_windows();
            DK.event(u, v, s);
            br.recount(s, t.len, counts, tied, sigma);
            for (size_t i = 0; i < k; ++i) { fail_wf += WF.count(i) != counts[i]; fail_dk += DK.count(i) != counts[i]; }
            fail_dk_state += check_dk(DK, t.n, s, t.len, counts);
            fail_tree += !WF.tree_ok();
            V.event(u, v, s);
            K.event(u, v, s);
            DI.event(u, v, s);
            for (size_t i = 0; i < k; ++i) {
                fail_v += V.count(i) != counts[i];
                fail_k += K.count(i) != counts[i];
                fail_di += DI.count(i) != counts[i];
            }
            fail_v_state += check_v(V, t.n);
            fail_k_state += check_k(K, t.n, s, t.len, counts);
            fail_di_state += check_di(DI, t.n, s, t.len, counts);
            for (i64 x = std::max<i64>(0, s - t.len.back() + 1); x <= s; ++x) {
                auto it = sigma.find(x);
                fail_c += V.slot(x) != (it == sigma.end() ? 0 : it->second);
            }
            for (int mu = 0; mu < 3; ++mu) {
                v_mut[size_t(mu)].event(u, v, s);
                k_mut[size_t(mu)].event(u, v, s);
                di_mut[size_t(mu)].event(u, v, s);
                for (size_t i = 0; i < k; ++i) {
                    if (v_mut[size_t(mu)].count(i) != counts[i]) v_bad[mu + 1] = true;
                    if (k_mut[size_t(mu)].count(i) != counts[i]) k_bad[mu + 1] = true;
                    if (di_mut[size_t(mu)].count(i) != counts[i]) di_bad[mu + 1] = true;
                }
            }
            // A view that never unlinks still counts right, so its mutant is judged on the structure.
            if (!v_bad[3] && check_v(v_mut[2], t.n)) v_bad[3] = true;
            for (int mu = 0; mu < 3; ++mu) {
                wf_mut[size_t(mu)].event(u, v, s);
                wf_mut[size_t(mu)].read_windows();
                dk_mut[size_t(mu)].event(u, v, s);
                for (size_t i = 0; i < k; ++i) {
                    if (wf_mut[size_t(mu)].count(i) != counts[i]) wf_bad[mu + 1] = true;
                    if (dk_mut[size_t(mu)].count(i) != counts[i]) dk_bad[mu + 1] = true;
                }
            }
            // any length up to the horizon, not only the k fixed ones
            {
                i64 above = 0;
                auto it = sigma.rbegin();
                for (i64 len = 1; len <= t.len.back(); ++len) {
                    while (it != sigma.rend() && it->first > s - len) { above += it->second; ++it; }
                    ++any_checks;
                    fail_any += WF.any(len) != above;
                }
            }
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
        rehomed_dk += DK.st.rehomed;
        immediate_dk += DK.st.scans_event - DK.st.firsts;
        for (int mu = 1; mu <= 3; ++mu) { wrong_wf[mu] += wf_bad[mu]; wrong_dk[mu] += dk_bad[mu]; }
        for (int mu = 1; mu <= 3; ++mu) { wrong_v[mu] += v_bad[mu]; wrong_k[mu] += k_bad[mu]; wrong_di[mu] += di_bad[mu]; }
        moved_v += V.st.moved;
        records_v += V.st.added;
        reentry_k += K.st.scans_event - K.st.firsts;
        rehomed_k += K.st.rehomed;
        exit_scans_di += DI.st.scans_expiry;
        // The split of D-k's scans must add up to the scans it already counted, and every element scanned must be in one part.
        dk_split_bad += DK.st.scans_rehome_now != DK.st.scans_event - DK.st.firsts;
        dk_split_bad += DK.st.scans_rehome_stale != DK.st.scans_expiry;
        dk_split_bad += DK.st.elems_insert + DK.st.elems_rehome != DK.cost.scan;
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
    std::printf("M1B01-B failures: W-F=%llu W-F any length=%llu (of %llu) tree nodes=%llu D-k=%llu D-k invariants and identity=%llu\n",
                (unsigned long long)fail_wf, (unsigned long long)fail_any, (unsigned long long)any_checks,
                (unsigned long long)fail_tree, (unsigned long long)fail_dk, (unsigned long long)fail_dk_state);
    std::printf("M1B01-B coverage: D-k re-homed=%llu immediate re-home scans=%llu\n", (unsigned long long)rehomed_dk,
                (unsigned long long)immediate_dk);
    std::printf("mutants wrong, of %llu trials: W-F plain add=%llu no clear=%llu move keeps old unit=%llu; "
                "D-k lazy only=%llu true before stale=%llu no id tie-break=%llu\n", (unsigned long long)trials,
                (unsigned long long)wrong_wf[1], (unsigned long long)wrong_wf[2], (unsigned long long)wrong_wf[3],
                (unsigned long long)wrong_dk[1], (unsigned long long)wrong_dk[2], (unsigned long long)wrong_dk[3]);
    std::printf("M1C01 failures: V=%llu V structure=%llu V count ring=%llu K=%llu K state=%llu DI=%llu DI state=%llu D-k scan split=%llu\n",
                (unsigned long long)fail_v, (unsigned long long)fail_v_state, (unsigned long long)fail_c, (unsigned long long)fail_k,
                (unsigned long long)fail_k_state, (unsigned long long)fail_di, (unsigned long long)fail_di_state,
                (unsigned long long)dk_split_bad);
    std::printf("M1C01 coverage: V records=%llu moved=%llu K re-entry scans=%llu K re-homed=%llu DI exit scans=%llu\n",
                (unsigned long long)records_v, (unsigned long long)moved_v, (unsigned long long)reentry_k,
                (unsigned long long)rehomed_k, (unsigned long long)exit_scans_di);
    std::printf("M1C01 mutants wrong, of %llu trials: V no sigma write=%llu V one other pair=%llu V no unlink=%llu; "
                "K no re-entry scan=%llu K no reset=%llu K no id tie-break=%llu; "
                "DI no exit scan=%llu DI no id tie-break=%llu DI exit at new time=%llu\n", (unsigned long long)trials,
                (unsigned long long)wrong_v[1], (unsigned long long)wrong_v[2], (unsigned long long)wrong_v[3],
                (unsigned long long)wrong_k[1], (unsigned long long)wrong_k[2], (unsigned long long)wrong_k[3],
                (unsigned long long)wrong_di[1], (unsigned long long)wrong_di[2], (unsigned long long)wrong_di[3]);
    bool ok = !(fail_w | fail_w0 | fail_b2p | fail_b2r | fail_b1 | fail_p | fail_d | fail_b) && control_add_wrong > 0 &&
              control_tie_wrong > 0 && fallbacks_w > 0 && fallbacks_b2 > 0 && rehomed > 0 && cov.reentries > 0 &&
              !(fail_wf | fail_any | fail_tree | fail_dk | fail_dk_state) && immediate_dk > 0 && rehomed_dk > 0;
    ok = ok && !(fail_v | fail_v_state | fail_c | fail_k | fail_k_state | fail_di | fail_di_state | dk_split_bad);
    ok = ok && moved_v > 0 && records_v > 0 && reentry_k > 0 && rehomed_k > 0 && exit_scans_di > 0;
    for (int mu = 1; mu <= 3; ++mu) ok = ok && wrong_wf[mu] > 0 && wrong_dk[mu] > 0;
    for (int mu = 1; mu <= 3; ++mu) ok = ok && wrong_v[mu] > 0 && wrong_k[mu] > 0 && wrong_di[mu] > 0;
    std::puts(ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
