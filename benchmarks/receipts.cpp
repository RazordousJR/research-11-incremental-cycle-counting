// Screen B test M1B02: count receipts. Protocol: docs/experiments/screen-b-protocol.md.
// Sum-check audit of a running triangle count against three field elements.
//
// Usage: receipts selftest <graphs> <small_field_trials> <out.txt>
//        receipts state <events.bin> <label> <frac> <full_seeds> <cheap_seeds> <seed_base> <out_prefix>
//        receipts kst <s> <t> <seed> <out.txt>
//
// LAYOUT IS PART OF THE TEST. Part A holds the field, the sketch and the verifier, and it comes
// BEFORE tc.hpp is included, so the verifier cannot name r11::Graph: reading the graph from there
// is a compile error. The verifier sees edits, then 3 field elements per round, and nothing else.
// The prover (Part B) gets a challenge only as the return value of Audit::receive.

#include <algorithm>
#include <array>
#include <chrono>
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <random>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------------------------
// Part A: field, sketch, verifier. No graph type exists in this part of the file.
// ---------------------------------------------------------------------------------------------
namespace rec {

using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i64 = std::int64_t;
using u128 = unsigned __int128;

// Mersenne field of modulus 2^Q - 1. Q = 61 is the protocol field. Q = 13 exists only to measure
// the soundness error, which cannot be observed at Q = 61.
template <int Q>
struct Fp {
    static constexpr u64 P = (u64(1) << Q) - 1;
    static constexpr u64 INV2 = u64(1) << (Q - 1);
    static u64 add(u64 a, u64 b) { u64 c = a + b; return c >= P ? c - P : c; }
    static u64 sub(u64 a, u64 b) { return a >= b ? a - b : a + P - b; }
    static u64 mul(u64 a, u64 b) {
        u128 t = u128(a) * b;
        u64 c = (u64(t) & P) + u64(t >> Q);
        return c >= P ? c - P : c;
    }
    static u64 reduce(u128 t) {
        while (t >> Q) t = (t & P) + (t >> Q);
        u64 c = u64(t);
        return c >= P ? c - P : c;
    }
    static u64 pow(u64 a, u64 e) {
        u64 r = 1;
        for (; e; e >>= 1, a = mul(a, a))
            if (e & 1) r = mul(r, a);
        return r;
    }
    static u64 inv(u64 a) { return pow(a, P - 2); }
    static u64 from_i64(i64 v) {
        i64 m = v % i64(P);
        return u64(m < 0 ? m + i64(P) : m);
    }
    template <class R> static u64 random(R& rng) {
        for (;;) {
            u64 x = rng() & P;
            if (x != P) return x;
        }
    }
};

// One message: the round polynomial at 0, 1, 2.
struct Msg { u64 s0, s1, s2; };

// Degree-2 interpolation through X = 0, 1, 2, evaluated at r.
template <int Q>
inline u64 interp(const Msg& m, u64 r) {
    using F = Fp<Q>;
    u64 r1 = F::sub(r, 1), r2 = F::sub(r, 2);
    u64 t0 = F::mul(F::mul(m.s0, F::mul(r1, r2)), F::INV2);
    u64 t1 = F::mul(m.s1, F::mul(r, F::sub(2 % F::P, r)));
    u64 t2 = F::mul(F::mul(m.s2, F::mul(r, r1)), F::INV2);
    return F::add(F::add(t0, t1), t2);
}

// One entry of a well-formed turnstile feed: a true toggle of pair {u, v}.
struct Edit { u32 u, v; int sign; };

// The verifier's stream state: 3 words of sketch and 3L words of randomness, plus either split
// tables of O(sqrt n) words (fast form) or the 3L complements of the randomness (pure form).
template <int Q>
class Sketch {
    using F = Fp<Q>;

public:
    Sketch(int L, u64 seed) : L_(L), l1_(L / 2), l2_(L - L / 2) {
        std::mt19937_64 rng(seed);
        for (auto& r : r_) {
            r.resize(size_t(L));
            for (auto& x : r) x = F::random(rng);
        }
        for (int i = 0; i < 3; ++i) {
            lo_[i] = table(i, 0, l1_);
            hi_[i] = table(i, l1_, l2_);
            both_[i].resize(2 * size_t(L));
            for (int j = 0; j < L; ++j) {
                both_[i][size_t(2 * j)] = F::sub(1, r_[i][size_t(j)]);
                both_[i][size_t(2 * j + 1)] = r_[i][size_t(j)];
            }
        }
    }

    // Fast form: two table reads per chi value.
    void update_split(const Edit& e) { apply(e, chi_split(0, e.u), chi_split(0, e.v), chi_split(1, e.u), chi_split(1, e.v), chi_split(2, e.u), chi_split(2, e.v)); }
    // Pure form: L multiplications per chi value, no tables, the state is 3L + 3 words.
    void update_pure(const Edit& e) { apply(e, chi_pure(0, e.u), chi_pure(0, e.v), chi_pure(1, e.u), chi_pure(1, e.v), chi_pure(2, e.u), chi_pure(2, e.v)); }

    u64 words_pure() const { return u64(6 * L_ + 3 + 2); }  // +2: claim register and round counter
    u64 words_split() const { return u64(3 * L_ + 3 + 2) + 3 * ((u64(1) << l1_) + (u64(1) << l2_)); }
    bool same_words(const Sketch& o) const { return xy_ == o.xy_ && yz_ == o.yz_ && zx_ == o.zx_; }
    int L() const { return L_; }

    template <int> friend class Audit;

private:
    std::vector<u64> table(int which, int from, int bits) const {
        std::vector<u64> t{1};
        for (int j = 0; j < bits; ++j) {
            u64 b = r_[which][size_t(from + j)], a = F::sub(1, b);
            size_t s = t.size();
            t.resize(2 * s);
            for (size_t i = 0; i < s; ++i) { t[i + s] = F::mul(t[i], b); t[i] = F::mul(t[i], a); }
        }
        return t;
    }
    u64 chi_split(int w, u32 v) const { return F::mul(lo_[w][v & ((u32(1) << l1_) - 1)], hi_[w][v >> l1_]); }
    // Branch-free: 2L words per point hold r_j and 1 - r_j, so a random id costs no mispredicts.
    // The state is 6L + 3 words. 1 - r_j could be recomputed from 3L words at a small extra cost.
    u64 chi_pure(int w, u32 v) const {
        const u64* t = both_[w].data();
        u64 c = 1;
        for (int j = 0; j < L_; ++j) c = F::mul(c, t[2 * j + ((v >> j) & 1)]);
        return c;
    }
    void apply(const Edit& e, u64 xu, u64 xv, u64 yu, u64 yv, u64 zu, u64 zv) {
        u64 a = F::add(F::mul(xu, yv), F::mul(xv, yu));
        u64 b = F::add(F::mul(yu, zv), F::mul(yv, zu));
        u64 c = F::add(F::mul(zu, xv), F::mul(zv, xu));
        if (e.sign > 0) { xy_ = F::add(xy_, a); yz_ = F::add(yz_, b); zx_ = F::add(zx_, c); }
        else            { xy_ = F::sub(xy_, a); yz_ = F::sub(yz_, b); zx_ = F::sub(zx_, c); }
    }

    int L_, l1_, l2_;
    std::array<std::vector<u64>, 3> r_, lo_, hi_, both_;
    u64 xy_ = 0, yz_ = 0, zx_ = 0;
};

// The verifier during an audit. Takes 3L rounds of 3 field elements. A challenge leaves this
// class only after the message of its round has been received.
template <int Q>
class Audit {
    using F = Fp<Q>;

public:
    Audit(const Sketch<Q>& s, u64 claim) : s_(s), cur_(claim) {}

    // Returns false on rejection. On success *challenge holds this round's r.
    bool receive(const Msg& m, u64* challenge) {
        if (F::add(m.s0, m.s1) != cur_) return false;
        u64 r = s_.r_[size_t(round_ / s_.L_)][size_t(round_ % s_.L_)];
        cur_ = interp<Q>(m, r);
        ++round_;
        *challenge = r;
        return true;
    }
    // The last check reads only the verifier's own three words.
    bool finish() const { return F::mul(F::mul(s_.xy_, s_.yz_), s_.zx_) == cur_; }
    u64 final_value() const { return cur_; }
    int round() const { return round_; }

private:
    const Sketch<Q>& s_;
    u64 cur_;
    int round_ = 0;
};

}  // namespace rec

// ---------------------------------------------------------------------------------------------
// Part B: everything that may touch a graph.
// ---------------------------------------------------------------------------------------------
#include "../src/r11/tc.hpp"

namespace {

using namespace rec;
using r11::Graph;
using Clock = std::chrono::steady_clock;

double since(Clock::time_point t0) { return std::chrono::duration<double>(Clock::now() - t0).count(); }

template <class T> T median(std::vector<T> v) {
    std::sort(v.begin(), v.end());
    return v[v.size() / 2];
}

#pragma pack(push, 1)
struct Event { std::int64_t t; u32 u, v, op; };
#pragma pack(pop)
static_assert(sizeof(Event) == 20);

volatile u64 g_sink = 0;

// The data source. It turns raw set-semantics events into a well-formed turnstile feed. It holds
// its own pair set, O(distinct pairs) words, separate from the prover's graph. It is NOT the
// verifier, and its state is reported, not hidden: the verifier's small state rests on it.
struct Source {
    r11::FlatMap<char> present;
    bool accept(const Event& e, Edit* out) {
        if (e.u == e.v) return false;
        u64 k = r11::key(r11::canon(e.u, e.v));
        bool has = present.find(k) != nullptr;
        if (e.op == 1) {
            if (has) return false;
            present.emplace(k, 1);
            *out = Edit{e.u, e.v, 1};
            return true;
        }
        if (!has) return false;
        present.erase(k);
        *out = Edit{e.u, e.v, -1};
        return true;
    }
};

struct Cell { u64 a, b; };

// Honest prover. Reads only the current adjacency. Round order: x bits, y bits, z bits, LSB first.
// Phase 1 holds the folded rows of E as sparse rows. For a sibling pair (a, b) it needs the forms
// a'Eb, and it computes them from the out-lists of the union support. See the README proof.
template <int Q>
class Prover {
    using F = Fp<Q>;

public:
    u64 iters = 0;   // phase 1 inner iterations: list elements read, one field addition pair each
    u64 mults = 0;   // field multiplications, all phases

    Prover(const Graph& g, int L) : g_(g), L_(L), N_(size_t(1) << L), d_(N_, Cell{0, 0}) {
        u32 n = g.n();
        optr_.assign(size_t(n) + 1, 0);
        for (u32 v = 0; v < n; ++v) {
            u64 c = 0;
            for (u32 w : g.nbr(v)) c += before(v, w);
            optr_[size_t(v) + 1] = optr_[v] + c;
        }
        oadj_.resize(optr_[n]);
        for (u32 v = 0; v < n; ++v) {
            u64 p = optr_[v];
            for (u32 w : g.nbr(v))
                if (before(v, w)) oadj_[p++] = w;
        }
        ey_.reserve(2 * g.m());
        ev_.reserve(2 * g.m());
        ptr_.push_back(0);
        for (u32 v = 0; v < n; ++v) {
            if (!g.deg(v)) continue;
            key_.push_back(v);
            for (u32 w : g.nbr(v)) { ey_.push_back(w); ev_.push_back(1); }
            ptr_.push_back(ey_.size());
        }
    }

    Msg message() {
        if (round_ < L_) return msg_rows();
        if (round_ < 2 * L_) return msg_pair(1);
        return msg_pair(cstar_);
    }

    void bind(u64 r) {
        if (round_ < L_) {
            fold_rows(r);
            if (round_ == L_ - 1) start_y();
        } else if (round_ < 2 * L_) {
            ry_.push_back(r);
            fold_pair(r);
            if (round_ == 2 * L_ - 1) start_z();
        } else {
            fold_pair(r);
        }
        ++round_;
    }

private:
    bool before(u32 v, u32 w) const {
        u32 dv = g_.deg(v), dw = g_.deg(w);
        return dv != dw ? dv < dw : v < w;
    }

    // Pairs up sibling rows. Calls f(index of even row or -1, index of odd row or -1).
    template <class Fn> void each_parent(Fn f) const {
        size_t nr = key_.size();
        const size_t none = ~size_t(0);
        for (size_t i = 0; i < nr;) {
            u32 k = key_[i];
            if (k & 1) { f(none, i, k >> 1); ++i; }
            else if (i + 1 < nr && key_[i + 1] == k + 1) { f(i, i + 1, k >> 1); i += 2; }
            else { f(i, none, k >> 1); ++i; }
        }
    }

    Msg msg_rows() {
        const size_t none = ~size_t(0);
        u128 s00 = 0, s11 = 0, s01 = 0;
        const u64* optr = optr_.data();
        const u32* oadj = oadj_.data();
        Cell* d = d_.data();
        each_parent([&](size_t ia, size_t ib, u32) {
            touched_.clear();
            if (ia != none)
                for (u64 e = ptr_[ia]; e < ptr_[ia + 1]; ++e) { d[ey_[e]].a = ev_[e]; touched_.push_back(ey_[e]); }
            if (ib != none)
                for (u64 e = ptr_[ib]; e < ptr_[ib + 1]; ++e) {
                    u32 y = ey_[e];
                    if (d[y].a == 0) touched_.push_back(y);
                    d[y].b = ev_[e];
                }
            for (u32 y : touched_) {
                u64 lo = optr[y], hi = optr[size_t(y) + 1];
                if (lo == hi) continue;
                iters += hi - lo;
                u128 sa = 0, sb = 0;
                for (u64 p = lo; p < hi; ++p) { const Cell& c = d[oadj[p]]; sa += c.a; sb += c.b; }
                u64 fa = F::reduce(sa), fb = F::reduce(sb);
                u64 ay = d[y].a, by = d[y].b;
                s00 += F::mul(ay, fa);
                s11 += F::mul(by, fb);
                s01 += F::add(F::mul(ay, fb), F::mul(by, fa));
                mults += 4;
            }
            for (u32 y : touched_) d[y] = Cell{0, 0};
        });
        u64 b00 = F::add(F::reduce(s00), F::reduce(s00));
        u64 b11 = F::add(F::reduce(s11), F::reduce(s11));
        u64 b01 = F::reduce(s01);
        u64 four = 4 % F::P;
        u64 s2 = F::add(F::sub(b00, F::mul(four, b01)), F::mul(four, b11));
        return Msg{b00, b11, s2};
    }

    void fold_rows(u64 r) {
        const size_t none = ~size_t(0);
        u64 ra = F::sub(1, r);
        size_t out = 0, nk = 0;
        nptr_.assign(1, 0);
        Cell* d = d_.data();
        // Writes never pass reads: a parent's output is at most its input, and a pair is
        // scattered into d_ in full before any of its output is written.
        each_parent([&](size_t ia, size_t ib, u32 par) {
            size_t start = out;
            if (ia != none && ib != none) {
                touched_.clear();
                for (u64 e = ptr_[ia]; e < ptr_[ia + 1]; ++e) { d[ey_[e]].a = ev_[e]; touched_.push_back(ey_[e]); }
                for (u64 e = ptr_[ib]; e < ptr_[ib + 1]; ++e) {
                    u32 y = ey_[e];
                    if (d[y].a == 0) touched_.push_back(y);
                    d[y].b = ev_[e];
                }
                for (u32 y : touched_) {
                    u64 v = F::add(F::mul(d[y].a, ra), F::mul(d[y].b, r));
                    mults += 2;
                    d[y] = Cell{0, 0};
                    if (v) { ey_[out] = y; ev_[out] = v; ++out; }
                }
            } else {
                size_t i = ia != none ? ia : ib;
                u64 w = ia != none ? ra : r;
                for (u64 e = ptr_[i]; e < ptr_[i + 1]; ++e) {
                    u64 v = F::mul(ev_[e], w);
                    u32 y = ey_[e];
                    ++mults;
                    if (v) { ey_[out] = y; ev_[out] = v; ++out; }
                }
            }
            if (out > start) { key_[nk++] = par; nptr_.push_back(out); }
        });
        key_.resize(nk);
        ptr_.swap(nptr_);
    }

    // Phase 2: a(y) = E~(r_x, y) is ready. u = a, v = h with h(y) = sum over z in N(y) of a(z).
    void start_y() {
        a_.assign(N_, 0);
        if (!key_.empty())
            for (u64 e = ptr_[0]; e < ptr_[1]; ++e) a_[ey_[e]] = ev_[e];
        u_ = a_;
        v_.assign(N_, 0);
        for (u32 y = 0; y < g_.n(); ++y) {
            u128 t = 0;
            for (u32 z : g_.nbr(y)) t += a_[z];
            v_[y] = F::reduce(t);
        }
    }

    // Phase 3: u = E~(r_y, z), v = a(z). chi of r_y is built here from the challenges received.
    void start_z() {
        cstar_ = u_[0];
        std::vector<u64> chi{1};
        for (int j = 0; j < L_; ++j) {
            u64 b = ry_[size_t(j)], c = F::sub(1, b);
            size_t s = chi.size();
            chi.resize(2 * s);
            for (size_t i = 0; i < s; ++i) { chi[i + s] = F::mul(chi[i], b); chi[i] = F::mul(chi[i], c); }
            mults += 2 * s;
        }
        u_.assign(N_, 0);
        for (u32 z = 0; z < g_.n(); ++z) {
            u128 t = 0;
            for (u32 y : g_.nbr(z)) t += chi[y];
            u_[z] = F::reduce(t);
        }
        v_ = a_;
    }

    // Sum-check round for the product of two multilinear tables, scaled by c.
    Msg msg_pair(u64 c) {
        size_t half = u_.size() / 2;
        u128 s0 = 0, s1 = 0, s2 = 0;
        for (size_t k = 0; k < half; ++k) {
            u64 u0 = u_[2 * k], u1 = u_[2 * k + 1], v0 = v_[2 * k], v1 = v_[2 * k + 1];
            s0 += F::mul(u0, v0);
            s1 += F::mul(u1, v1);
            s2 += F::mul(F::sub(F::add(u1, u1), u0), F::sub(F::add(v1, v1), v0));
        }
        mults += 3 * half + 3;
        return Msg{F::mul(F::reduce(s0), c), F::mul(F::reduce(s1), c), F::mul(F::reduce(s2), c)};
    }

    void fold_pair(u64 r) {
        size_t half = u_.size() / 2;
        u64 ra = F::sub(1, r);
        for (size_t k = 0; k < half; ++k) {
            u_[k] = F::add(F::mul(u_[2 * k], ra), F::mul(u_[2 * k + 1], r));
            v_[k] = F::add(F::mul(v_[2 * k], ra), F::mul(v_[2 * k + 1], r));
        }
        mults += 4 * half;
        u_.resize(half);
        v_.resize(half);
    }

    const Graph& g_;
    int L_;
    size_t N_;
    int round_ = 0;
    std::vector<u64> optr_;
    std::vector<u32> oadj_;
    std::vector<u32> key_, ey_, touched_;
    std::vector<u64> ptr_, nptr_, ev_;
    std::vector<Cell> d_;
    std::vector<u64> a_, u_, v_, ry_;
    u64 cstar_ = 0;
};

// Cheating provers. Each sends only messages that pass the round check, so it is caught, if at
// all, by the last check against the sketch. An honest message with a wrong claim dies in round 1
// and never reaches the sketch, so that test alone would prove little.
enum class Lie { Random, Roots, Fabricate };

template <int Q>
class Liar {
    using F = Fp<Q>;

public:
    Liar(Prover<Q>* honest, Lie mode, u64 claim, u64 seed) : h_(honest), mode_(mode), cur_(claim), rng_(seed) {}
    Msg message() {
        Msg m{0, 0, 0};
        if (h_) m = h_->message();
        u64 delta = F::sub(cur_, F::add(m.s0, m.s1));
        if (delta != 0 || !h_) {
            if (mode_ == Lie::Roots && h_) {
                // d(X) = c (X - p)(X - q) with d(0) + d(1) = delta. If r hits p or q, the claim
                // is true again and the prover is back on the honest track. This is the best
                // strategy the soundness bound allows, so it should nearly reach 6L/p.
                u64 p, q, den;
                do {
                    p = F::random(rng_); q = F::random(rng_);
                    den = F::add(F::mul(p, q), F::mul(F::sub(1, p), F::sub(1, q)));
                } while (p == q || den == 0);
                u64 c = F::mul(delta, F::inv(den));
                m.s0 = F::add(m.s0, F::mul(c, F::mul(p, q)));
                m.s1 = F::add(m.s1, F::mul(c, F::mul(F::sub(1, p), F::sub(1, q))));
                m.s2 = F::add(m.s2, F::mul(c, F::mul(F::sub(2 % F::P, p), F::sub(2 % F::P, q))));
            } else {
                u64 e0 = F::random(rng_), e2 = F::random(rng_);
                m.s0 = F::add(m.s0, e0);
                m.s1 = F::add(m.s1, F::sub(delta, e0));
                m.s2 = F::add(m.s2, e2);
            }
        }
        last_ = m;
        return m;
    }
    void bind(u64 r) {
        cur_ = interp<Q>(last_, r);
        if (h_) h_->bind(r);
    }

private:
    Prover<Q>* h_;
    Lie mode_;
    u64 cur_;
    std::mt19937_64 rng_;
    Msg last_{0, 0, 0};
};

struct Outcome { bool accepted; int reject_round; };  // reject_round: 3L means the last check

template <int Q, class P>
Outcome run(Audit<Q>& v, P& p, int L, std::vector<u64>* transcript = nullptr) {
    for (int i = 0; i < 3 * L; ++i) {
        u64 r = 0;
        if (!v.receive(p.message(), &r)) return {false, i};
        if (transcript) transcript->push_back(r);
        p.bind(r);
    }
    return {v.finish(), 3 * L};
}

i64 common(const Graph& g, u32 u, u32 v) {
    u32 s = g.deg(u) <= g.deg(v) ? u : v, t = s == u ? v : u;
    i64 c = 0;
    for (u32 w : g.nbr(s)) c += g.has(w, t);
    return c;
}

// Toggles {u, v} in g and returns the new triangle count.
i64 flip(Graph& g, u32 u, u32 v, i64 t) {
    if (g.has(u, v)) { g.erase(u, v); return t - common(g, u, v); }
    i64 c = common(g, u, v);
    g.insert(u, v);
    return t + c;
}

template <int Q>
std::vector<u64> chi_table(const u64* r, int L) {
    using F = Fp<Q>;
    std::vector<u64> t{1};
    for (int j = 0; j < L; ++j) {
        u64 b = r[j], a = F::sub(1, b);
        size_t s = t.size();
        t.resize(2 * s);
        for (size_t i = 0; i < s; ++i) { t[i + s] = F::mul(t[i], b); t[i] = F::mul(t[i], a); }
    }
    return t;
}

// E~(a, b) of graph g, by direct evaluation. TEST ORACLE ONLY.
template <int Q>
u64 eval_mle(const Graph& g, const std::vector<u64>& ca, const std::vector<u64>& cb) {
    using F = Fp<Q>;
    u128 s = 0;
    for (u32 v = 0; v < g.n(); ++v) {
        u128 t = 0;
        for (u32 w : g.nbr(v)) t += cb[w];
        s += F::mul(ca[v], F::reduce(t));
    }
    return F::reduce(s);
}

// g(r) read from a graph. Used in two places only: the positive control, and to check that the
// three sketch words match the true graph after an audit has revealed r.
template <int Q>
u64 product_on_graph(const Graph& g, const std::vector<u64>& ch, int L) {
    using F = Fp<Q>;
    auto cx = chi_table<Q>(ch.data(), L), cy = chi_table<Q>(ch.data() + L, L), cz = chi_table<Q>(ch.data() + 2 * L, L);
    return F::mul(F::mul(eval_mle<Q>(g, cx, cy), eval_mle<Q>(g, cy, cz)), eval_mle<Q>(g, cz, cx));
}

// POSITIVE CONTROL. The mistake this test is built to catch: a verifier whose last check reads
// the prover's graph. Same transcript, wrong check. It must ACCEPT flipped-edge and dropped-update
// runs. If it rejected them, the rejections of the real verifier would mean nothing.
template <int Q>
bool leaky_accepts(const Graph& prover_graph, const std::vector<u64>& ch, int L, u64 final_value) {
    return product_on_graph<Q>(prover_graph, ch, L) == final_value;
}

int ceil_log2(u32 n) {
    int L = 1;
    while ((u64(1) << L) < n) ++L;
    return L;
}

// The quantities of Theorem 1. D = sum d(v) d+(v), R = wedge checks of one recount on the same
// order, S2 = sum d+(v)^2, refined = sum over y of d+(y) sum over k < L of min(d(y), 2^k).
struct Bounds { u64 D = 0, R = 0, S2 = 0, refined = 0; };
Bounds bounds(const Graph& g, int L) {
    u32 n = g.n();
    auto before = [&](u32 v, u32 w) { u32 a = g.deg(v), b = g.deg(w); return a != b ? a < b : v < w; };
    std::vector<u32> outd(n, 0);
    for (u32 v = 0; v < n; ++v)
        for (u32 w : g.nbr(v)) outd[v] += before(v, w);
    Bounds b;
    for (u32 v = 0; v < n; ++v) {
        b.D += u64(g.deg(v)) * outd[v];
        b.S2 += u64(outd[v]) * outd[v];
        u64 row = 0;
        for (int k = 0; k < L; ++k) row += std::min<u64>(g.deg(v), u64(1) << k);
        b.refined += row * outd[v];
        for (u32 w : g.nbr(v))
            if (before(v, w)) b.R += outd[w];
    }
    return b;
}
u64 mult_bound(const Graph& g, int L) { return 12 * u64(L) * g.m() + 16 * (u64(1) << L) + 8 * u64(L); }

struct Tally { u64 n = 0, accepted = 0, at_last = 0; };
void tally(Tally& t, const Outcome& o, int L) {
    ++t.n;
    t.accepted += o.accepted;
    t.at_last += !o.accepted && o.reject_round == 3 * L;
}

template <int Q>
Msg brute_msg(const std::vector<std::pair<u32, u32>>& ordered, int L, const std::vector<u64>& bound) {
    // Sums g over Boolean completions of (bound, X, ...), directly from the definition.
    using F = Fp<Q>;
    int total = 3 * L, i = int(bound.size());
    u64 out[3] = {0, 0, 0};
    u64 nrest = u64(1) << (total - i - 1);
    for (int e = 0; e < 3; ++e)
        for (u64 b = 0; b < nrest; ++b) {
            std::vector<u64> pt = bound;
            pt.push_back(u64(e));
            for (int j = 0; j < total - i - 1; ++j) pt.push_back((b >> j) & 1);
            auto cx = chi_table<Q>(pt.data(), L), cy = chi_table<Q>(pt.data() + L, L), cz = chi_table<Q>(pt.data() + 2 * L, L);
            auto ev = [&](const std::vector<u64>& p, const std::vector<u64>& q) {
                u128 s = 0;
                for (auto [u, v] : ordered) s += F::mul(p[u], q[v]);
                return F::reduce(s);
            };
            u64 val = F::mul(F::mul(ev(cx, cy), ev(cy, cz)), ev(cz, cx));
            out[e] = F::add(out[e], val);
        }
    return Msg{out[0], out[1], out[2]};
}

struct Toy {
    Graph g;
    std::vector<Edit> edits;
    i64 t;
};

Toy random_graph(u32 n, u32 toggles, std::mt19937_64& rng) {
    Toy y{Graph(n), {}, 0};
    for (u32 i = 0; i < toggles; ++i) {
        u32 u = u32(rng() % n), v = u32(rng() % n);
        if (u == v) continue;
        bool had = y.g.has(u, v);
        y.t = flip(y.g, u, v, y.t);
        y.edits.push_back(Edit{u, v, had ? -1 : 1});
    }
    return y;
}

int selftest(int graphs, u64 small_trials, const char* out_path) {
    FILE* out = std::fopen(out_path, "w");
    if (!out) { std::puts("cannot open output"); return 2; }
    auto say = [&](const char* fmt, auto... a) {
        std::printf(fmt, a...);
        std::fprintf(out, fmt, a...);
        std::fflush(stdout);
    };
    std::mt19937_64 rng(20261101);
    u64 failures = 0;

    // 1. Prover messages against the definition of g, by exhaustive sum.
    {
        u64 rounds = 0, wrong = 0;
        for (int tr = 0; tr < 40; ++tr) {
            u32 n = u32(2 + rng() % 7);
            int L = ceil_log2(n);
            if (L > 3) L = 3;
            Toy y = random_graph(n, 30, rng);
            std::vector<std::pair<u32, u32>> ordered;
            for (u32 v = 0; v < n; ++v)
                for (u32 w : y.g.nbr(v)) ordered.push_back({v, w});
            Sketch<61> sk(L, rng());
            Audit<61> au(sk, Fp<61>::from_i64(6 * y.t));
            Prover<61> p(y.g, L);
            std::vector<u64> bound;
            for (int i = 0; i < 3 * L; ++i) {
                Msg m = p.message(), b = brute_msg<61>(ordered, L, bound);
                ++rounds;
                wrong += m.s0 != b.s0 || m.s1 != b.s1 || m.s2 != b.s2;
                u64 r = 0;
                if (!au.receive(m, &r)) { ++wrong; break; }
                p.bind(r);
                bound.push_back(r);
            }
        }
        say("brute_messages rounds=%llu mismatches=%llu\n", (unsigned long long)rounds, (unsigned long long)wrong);
        failures += wrong;
    }

    // 2. Random mixed graphs in the protocol field.
    Tally honest, wrong, rnd, roots, fab, flipped, dropped, leak_flip, leak_drop;
    u64 same_count_flips = 0, count_mismatch = 0, bound_violations = 0;
    for (int tr = 0; tr < graphs; ++tr) {
        u32 n = u32(4 + rng() % 61);
        int L = ceil_log2(n);
        u32 toggles = u32(5 + rng() % 400);
        Toy y = random_graph(n, toggles, rng);
        count_mismatch += y.t != r11::brute_triangles(y.g);
        Sketch<61> sk(L, rng());
        for (const Edit& e : y.edits) sk.update_split(e);
        using F = Fp<61>;
        u64 claim = F::from_i64(6 * y.t);
        {
            Audit<61> a(sk, claim);
            Prover<61> p(y.g, L);
            tally(honest, run<61>(a, p, L), L);
            Bounds b = bounds(y.g, L);
            bound_violations += p.iters > b.refined || b.refined > u64(L) * b.D || b.D != b.R + b.S2 || p.mults > mult_bound(y.g, L);
        }
        { Audit<61> a(sk, F::add(claim, 6)); Prover<61> p(y.g, L); tally(wrong, run<61>(a, p, L), L); }
        for (int mode = 0; mode < 2; ++mode) {
            u64 bad = F::add(claim, 6 * (1 + rng() % 3));
            Audit<61> a(sk, bad);
            Prover<61> p(y.g, L);
            Liar<61> liar(&p, mode ? Lie::Roots : Lie::Random, bad, rng());
            tally(mode ? roots : rnd, run<61>(a, liar, L), L);
        }
        {
            u64 bad = F::add(claim, 6);
            Audit<61> a(sk, bad);
            Liar<61> liar(nullptr, Lie::Fabricate, bad, rng());
            tally(fab, run<61>(a, liar, L), L);
        }
        // Flipped edge: honest prover on G xor e. Half of these keep the count, so the claim is
        // the true count of G and only the graph is wrong.
        {
            u32 u = u32(rng() % n), v = u32((u + 1 + rng() % (n - 1)) % n);
            i64 t2 = flip(y.g, u, v, y.t);
            same_count_flips += t2 == y.t;
            Audit<61> a(sk, F::from_i64(6 * t2));
            Prover<61> p(y.g, L);
            std::vector<u64> ch;
            Outcome o = run<61>(a, p, L, &ch);
            tally(flipped, o, L);
            bool lk = leaky_accepts<61>(y.g, ch, L, a.final_value());
            ++leak_flip.n; leak_flip.accepted += lk;
            flip(y.g, u, v, t2);
        }
        if (!y.edits.empty()) {
            const Edit& e = y.edits[rng() % y.edits.size()];
            i64 t2 = flip(y.g, e.u, e.v, y.t);
            Audit<61> a(sk, F::from_i64(6 * t2));
            Prover<61> p(y.g, L);
            std::vector<u64> ch;
            Outcome o = run<61>(a, p, L, &ch);
            tally(dropped, o, L);
            bool lk = leaky_accepts<61>(y.g, ch, L, a.final_value());
            ++leak_drop.n; leak_drop.accepted += lk;
            flip(y.g, e.u, e.v, t2);
        }
    }
    auto line = [&](const char* name, const Tally& t) {
        say("%-20s runs=%llu accepted=%llu rejected_at_last_check=%llu\n", name, (unsigned long long)t.n,
            (unsigned long long)t.accepted, (unsigned long long)t.at_last);
    };
    line("honest", honest); line("wrong_claim", wrong); line("liar_random", rnd); line("liar_roots", roots);
    line("fabricator", fab); line("flipped_edge", flipped); line("dropped_update", dropped);
    line("CONTROL_leaky_flip", leak_flip); line("CONTROL_leaky_drop", leak_drop);
    say("same_count_flips=%llu count_mismatch_vs_brute=%llu theorem1_violations=%llu\n", (unsigned long long)same_count_flips, (unsigned long long)count_mismatch, (unsigned long long)bound_violations);
    failures += bound_violations;
    failures += honest.n - honest.accepted;
    failures += wrong.accepted + rnd.accepted + roots.accepted + fab.accepted + flipped.accepted + dropped.accepted;
    failures += (leak_flip.n - leak_flip.accepted) + (leak_drop.n - leak_drop.accepted);
    failures += count_mismatch;

    // 3. The soundness error itself, in a field small enough to see it. p = 8191, L = 5.
    {
        constexpr int Q = 13;
        using F = Fp<Q>;
        u64 acc_roots = 0, acc_random = 0, acc_flip = 0, acc_flip_samecount = 0, flips_samecount = 0, hon = 0;
        const int L = 5;
        const u32 n = 32;
        for (u64 tr = 0; tr < small_trials; ++tr) {
            Toy y = random_graph(n, 60, rng);
            Sketch<Q> sk(L, rng());
            for (const Edit& e : y.edits) sk.update_pure(e);
            u64 claim = F::from_i64(6 * y.t);
            { Audit<Q> a(sk, claim); Prover<Q> p(y.g, L); hon += run<Q>(a, p, L).accepted; }
            u64 bad = F::add(claim, 6);
            { Audit<Q> a(sk, bad); Prover<Q> p(y.g, L); Liar<Q> l(&p, Lie::Roots, bad, rng()); acc_roots += run<Q>(a, l, L).accepted; }
            { Audit<Q> a(sk, bad); Prover<Q> p(y.g, L); Liar<Q> l(&p, Lie::Random, bad, rng()); acc_random += run<Q>(a, l, L).accepted; }
            u32 u = u32(rng() % n), v = u32((u + 1 + rng() % (n - 1)) % n);
            i64 t2 = flip(y.g, u, v, y.t);
            { Audit<Q> a(sk, F::from_i64(6 * t2)); Prover<Q> p(y.g, L);
              bool ok = run<Q>(a, p, L).accepted; acc_flip += ok;
              if (t2 == y.t) { ++flips_samecount; acc_flip_samecount += ok; } }
        }
        double p = double(F::P), bound = 6.0 * L / p, tight = 1 - std::pow(1 - 2 / p, 3.0 * L);
        double N = double(small_trials);
        say("small_field p=%llu L=%d trials=%llu honest_accepted=%llu\n", (unsigned long long)F::P, L, (unsigned long long)small_trials, (unsigned long long)hon);
        say("  bound_6L_over_p=%.6f  predicted_roots_liar=%.6f\n", bound, tight);
        say("  liar_roots   accepted=%llu rate=%.6f\n", (unsigned long long)acc_roots, double(acc_roots) / N);
        say("  liar_random  accepted=%llu rate=%.6f\n", (unsigned long long)acc_random, double(acc_random) / N);
        say("  flipped_edge accepted=%llu rate=%.6f (same-count flips %llu, accepted %llu)\n", (unsigned long long)acc_flip, double(acc_flip) / N,
            (unsigned long long)flips_samecount, (unsigned long long)acc_flip_samecount);
        failures += small_trials - hon;
    }
    say("selftest_failures=%llu\n", (unsigned long long)failures);
    std::fclose(out);
    return failures ? 1 : 0;
}

int state_mode(const char* path, const std::string& label, double frac, int full_seeds, int cheap_seeds, u64 seed_base, const std::string& prefix) {
    using F = Fp<61>;
    FILE* in = std::fopen(path, "rb");
    if (!in) { std::puts("cannot open events"); return 2; }
    std::fseek(in, 0, SEEK_END);
    long long bytes = _ftelli64(in);
    std::fseek(in, 0, SEEK_SET);
    std::vector<Event> ev(size_t(bytes) / sizeof(Event));
    if (std::fread(ev.data(), sizeof(Event), ev.size(), in) != ev.size()) { std::puts("short read"); return 2; }
    std::fclose(in);

    u32 n = 0;
    for (const Event& e : ev) n = std::max({n, e.u + 1, e.v + 1});
    size_t stop = size_t(double(ev.size()) * frac);
    int L = ceil_log2(n);

    // The source pass. Nothing here touches the prover's graph.
    std::vector<Edit> edits;
    edits.reserve(stop);
    double filter_s;
    u64 source_pairs;
    {
        Source src;
        auto t0 = Clock::now();
        for (size_t i = 0; i < stop; ++i) {
            Edit e{0, 0, 0};
            if (src.accept(ev[i], &e)) edits.push_back(e);
        }
        filter_s = since(t0);
        source_pairs = src.present.size();
    }
    std::printf("[%s] n=%u L=%d raw=%zu applied=%zu noop_share=%.4f\n", label.c_str(), n, L, stop, edits.size(), 1.0 - double(edits.size()) / double(stop));
    std::fflush(stdout);

    // Mean scan time. A replay with scans, and one with graph edits alone. The scan is the
    // difference, the smaller denominator, so the ratio test below is the stricter one.
    std::vector<double> t_scan, t_edit;
    i64 running = 0;
    u64 list_scanned = 0;
    for (int rep = 0; rep < 3; ++rep) {
        {
            Graph g(n);
            auto t0 = Clock::now();
            for (const Edit& e : edits) {
                if (e.sign > 0) g.insert(e.u, e.v); else g.erase(e.u, e.v);
            }
            t_edit.push_back(since(t0));
            g_sink = g.m();
        }
        {
            Graph g(n);
            i64 t = 0;
            u64 scanned = 0;
            auto t0 = Clock::now();
            for (const Edit& e : edits) {
                if (e.sign < 0) g.erase(e.u, e.v);
                u32 s = g.deg(e.u) <= g.deg(e.v) ? e.u : e.v, o = s == e.u ? e.v : e.u;
                i64 c = 0;
                for (u32 w : g.nbr(s)) c += g.has(w, o);
                scanned += g.deg(s);
                t += e.sign * c;
                if (e.sign > 0) g.insert(e.u, e.v);
            }
            t_scan.push_back(since(t0));
            running = t;
            list_scanned = scanned;
        }
    }
    double edit_s = median(t_edit), scan_edit_s = median(t_scan), scan_only_s = std::max(scan_edit_s - edit_s, 1e-12);
    double upd = double(edits.size());

    // The prover's graph, built from the same feed.
    Graph g(n);
    for (const Edit& e : edits) {
        if (e.sign > 0) g.insert(e.u, e.v); else g.erase(e.u, e.v);
    }
    std::vector<double> t_static;
    i64 t_true = 0;
    u64 static_ops = 0;
    for (int rep = 0; rep < 7; ++rep) {
        auto t0 = Clock::now();
        auto [c, ops] = r11::static_count(g);
        t_static.push_back(since(t0));
        t_true = c;
        static_ops = ops;
    }
    double static_s = median(t_static);
    if (running != t_true) { std::printf("EXACTNESS FAILURE running=%lld static=%lld\n", (long long)running, (long long)t_true); return 3; }

    // Structure of the bound.
    Bounds bd = bounds(g, L);
    u64 D = bd.D, R = bd.R, S2 = bd.S2;
    if (D != R + S2) { std::puts("IDENTITY FAILURE D != R + sum d+^2"); return 3; }
    u64 m = g.m(), T = u64(t_true);
    std::printf("[%s] m=%llu T=%llu D=%llu R=%llu edit_s=%.3f scan+edit_s=%.3f static_s=%.4f\n", label.c_str(), (unsigned long long)m,
                (unsigned long long)T, (unsigned long long)D, (unsigned long long)R, edit_s, scan_edit_s, static_s);
    std::fflush(stdout);

    // Sketch time, both forms. They must end on the same three words.
    std::vector<double> t_split, t_pure;
    bool forms_agree = true;
    for (int rep = 0; rep < 3; ++rep) {
        Sketch<61> a(L, seed_base + 900 + u64(rep)), b(L, seed_base + 900 + u64(rep));
        auto t0 = Clock::now();
        for (const Edit& e : edits) a.update_split(e);
        t_split.push_back(since(t0));
        auto t1 = Clock::now();
        for (const Edit& e : edits) b.update_pure(e);
        t_pure.push_back(since(t1));
        forms_agree = forms_agree && a.same_words(b);
    }
    double split_s = median(t_split), pure_s = median(t_pure);
    u64 words_pure = Sketch<61>(L, 1).words_pure(), words_split = Sketch<61>(L, 1).words_split();

    FILE* csv = std::fopen((prefix + "_runs.csv").c_str(), "w");
    if (!csv) { std::puts("cannot open csv"); return 2; }
    std::fprintf(csv, "seed,scenario,accepted,reject_round,last_check_round,seconds,iters,mults\n");
    auto rec_run = [&](u64 seed, const char* sc, const Outcome& o, double s, u64 it, u64 mu) {
        std::fprintf(csv, "%llu,%s,%d,%d,%d,%.6f,%llu,%llu\n", (unsigned long long)seed, sc, int(o.accepted), o.reject_round, 3 * L, s,
                     (unsigned long long)it, (unsigned long long)mu);
        std::fflush(csv);
    };

    u64 claim = F::from_i64(6 * t_true);
    Tally honest, wrong, rnd, fab, flipped, dropped, leak_flip, leak_drop;
    std::vector<double> prover_s;
    u64 iters = 0, mults = 0, same_count = 0, oracle_bad = 0;
    std::mt19937_64 rng(seed_base * 7919 + 13);
    std::vector<u32> active;
    for (u32 v = 0; v < n; ++v)
        if (g.deg(v)) active.push_back(v);

    for (int s = 0; s < full_seeds; ++s) {
        u64 seed = seed_base + u64(s);
        Sketch<61> sk(L, seed);
        for (const Edit& e : edits) sk.update_split(e);
        // Honest.
        {
            Audit<61> a(sk, claim);
            std::vector<u64> ch;
            auto t0 = Clock::now();
            Prover<61> p(g, L);
            Outcome o = run<61>(a, p, L, &ch);
            double t = since(t0);
            prover_s.push_back(t);
            iters = p.iters; mults = p.mults;
            tally(honest, o, L);
            rec_run(seed, "honest", o, t, p.iters, p.mults);
            // After the audit r is public, so the three words can be set against the true graph.
            oracle_bad += product_on_graph<61>(g, ch, L) != a.final_value();
            std::printf("  seed %llu honest accepted=%d %.3fs\n", (unsigned long long)seed, int(o.accepted), t);
            std::fflush(stdout);
        }
        // Wrong claim with honest messages: dies in round 1.
        {
            u64 bad = F::add(claim, 6);
            Audit<61> a(sk, bad);
            Prover<61> p(g, L);
            auto t0 = Clock::now();
            Outcome o = run<61>(a, p, L);
            tally(wrong, o, L);
            rec_run(seed, "wrong_claim", o, since(t0), p.iters, p.mults);
        }
        // Wrong claim, lying through every round. Only the sketch can stop this one.
        {
            u64 bad = F::add(claim, 6 * (1 + rng() % 3));
            Audit<61> a(sk, bad);
            Prover<61> p(g, L);
            Liar<61> liar(&p, Lie::Random, bad, rng());
            auto t0 = Clock::now();
            Outcome o = run<61>(a, liar, L);
            tally(rnd, o, L);
            rec_run(seed, "liar_random", o, since(t0), p.iters, p.mults);
        }
        // Flipped edge: half existing edges, half random pairs of active vertices.
        {
            u32 u, v;
            if ((s & 1) && m) {
                const Edit& e = edits[rng() % edits.size()];
                u = e.u; v = e.v;
                for (int tries = 0; tries < 64 && !g.has(u, v); ++tries) {
                    const Edit& f = edits[rng() % edits.size()];
                    u = f.u; v = f.v;
                }
            } else {
                u = active[rng() % active.size()];
                do { v = active[rng() % active.size()]; } while (v == u);
            }
            i64 t2 = flip(g, u, v, t_true);
            same_count += t2 == t_true;
            Audit<61> a(sk, F::from_i64(6 * t2));
            Prover<61> p(g, L);
            std::vector<u64> ch;
            auto t0 = Clock::now();
            Outcome o = run<61>(a, p, L, &ch);
            tally(flipped, o, L);
            rec_run(seed, "flipped_edge", o, since(t0), p.iters, p.mults);
            bool lk = leaky_accepts<61>(g, ch, L, a.final_value());
            ++leak_flip.n; leak_flip.accepted += lk;
            flip(g, u, v, t2);
        }
        // Dropped update: one applied edit lost by the prover. V saw it.
        {
            const Edit& e = edits[rng() % edits.size()];
            i64 t2 = flip(g, e.u, e.v, t_true);
            Audit<61> a(sk, F::from_i64(6 * t2));
            Prover<61> p(g, L);
            std::vector<u64> ch;
            auto t0 = Clock::now();
            Outcome o = run<61>(a, p, L, &ch);
            tally(dropped, o, L);
            rec_run(seed, "dropped_update", o, since(t0), p.iters, p.mults);
            bool lk = leaky_accepts<61>(g, ch, L, a.final_value());
            ++leak_drop.n; leak_drop.accepted += lk;
            flip(g, e.u, e.v, t2);
        }
        std::printf("  seed %llu done: wrong_acc=%llu liar_acc=%llu flip_acc=%llu drop_acc=%llu\n", (unsigned long long)seed,
                    (unsigned long long)wrong.accepted, (unsigned long long)rnd.accepted, (unsigned long long)flipped.accepted, (unsigned long long)dropped.accepted);
        std::fflush(stdout);
    }

    // Seeds that need no prover pass: a fabricator never touches the graph.
    for (int s = 0; s < cheap_seeds; ++s) {
        u64 seed = seed_base + 100000 + u64(s);
        Sketch<61> sk(L, seed);
        for (const Edit& e : edits) sk.update_split(e);
        u64 bad = F::add(claim, 6);
        Audit<61> a(sk, bad);
        Liar<61> liar(nullptr, Lie::Fabricate, bad, seed ^ 0x5555);
        Outcome o = run<61>(a, liar, L);
        tally(fab, o, L);
        rec_run(seed, "fabricator", o, 0.0, 0, 0);
    }

    // Feed boundary 1: the raw event file, no filter. The sketch then holds multiplicities.
    Tally rawfeed;
    {
        Sketch<61> sk(L, seed_base + 777);
        for (size_t i = 0; i < stop; ++i) {
            if (ev[i].u == ev[i].v) continue;
            sk.update_split(Edit{ev[i].u, ev[i].v, ev[i].op == 1 ? 1 : -1});
        }
        Audit<61> a(sk, claim);
        Prover<61> p(g, L);
        Outcome o = run<61>(a, p, L);
        tally(rawfeed, o, L);
        rec_run(seed_base + 777, "raw_feed_no_filter", o, 0.0, 0, 0);
    }
    // Feed boundary 2: the prover labels the no-ops, and labels one real edit a no-op. The sketch
    // never sees that edit, and the prover's graph lacks it too.
    Tally labelled;
    {
        size_t idx = size_t(rng() % edits.size());
        Sketch<61> sk(L, seed_base + 778);
        for (size_t i = 0; i < edits.size(); ++i)
            if (i != idx) sk.update_split(edits[i]);
        i64 t2 = flip(g, edits[idx].u, edits[idx].v, t_true);
        Audit<61> a(sk, F::from_i64(6 * t2));
        Prover<61> p(g, L);
        Outcome o = run<61>(a, p, L);
        tally(labelled, o, L);
        rec_run(seed_base + 778, "prover_labels_noops_drops_one", o, 0.0, 0, 0);
        flip(g, edits[idx].u, edits[idx].v, t2);
    }
    std::fclose(csv);

    double med_prover = prover_s.empty() ? 0 : median(prover_s);
    double min_p = prover_s.empty() ? 0 : *std::min_element(prover_s.begin(), prover_s.end());
    double max_p = prover_s.empty() ? 0 : *std::max_element(prover_s.begin(), prover_s.end());

    FILE* out = std::fopen((prefix + "_summary.txt").c_str(), "w");
    if (!out) { std::puts("cannot open summary"); return 2; }
    auto say = [&](const char* fmt, auto... a) {
        std::printf(fmt, a...);
        std::fprintf(out, fmt, a...);
    };
    auto U = [](u64 x) { return (unsigned long long)x; };
    say("state=%s file=%s frac=%.3f seed_base=%llu\n", label.c_str(), path, frac, U(seed_base));
    say("n=%u L=%d raw=%zu applied=%zu noop_share=%.4f m=%llu T=%lld source_pairs=%llu\n", n, L, stop, edits.size(), 1.0 - upd / double(stop), U(m), (long long)t_true, U(source_pairs));
    say("running_count_equals_static_count=1 static_ops=%llu\n", U(static_ops));
    say("D=%llu R=%llu sum_dplus2=%llu D_over_R=%.3f iters=%llu iters_over_LD=%.4f iters_over_R=%.2f mults=%llu\n", U(D), U(R), U(S2), double(D) / double(std::max<u64>(R, 1)), U(iters),
        double(iters) / (double(L) * double(D)), double(iters) / double(std::max<u64>(R, 1)), U(mults));
    // Theorem 1 of the README: iters <= refined <= L D, and mults <= 12 L m + 16 2^L + 8 L.
    u64 mb = mult_bound(g, L);
    say("theorem_check iters_le_refined=%d refined=%llu iters_over_refined=%.4f mults_le_bound=%d mult_bound=%llu mults_over_bound=%.3f\n", int(iters <= bd.refined), U(bd.refined),
        double(iters) / double(std::max<u64>(bd.refined, 1)), int(mults <= mb), U(mb), double(mults) / double(mb));
    say("static_s=%.5f prover_s_median=%.4f min=%.4f max=%.4f ratio_median=%.1f ratio_max=%.1f\n", static_s, med_prover, min_p, max_p, med_prover / static_s, max_p / static_s);
    say("edit_ns=%.1f scan_plus_edit_ns=%.1f scan_ns=%.1f list_elements_per_update=%.2f\n", 1e9 * edit_s / upd, 1e9 * scan_edit_s / upd, 1e9 * scan_only_s / upd, double(list_scanned) / upd);
    say("sketch_split_ns=%.1f sketch_pure_ns=%.1f ratio_split_vs_scan=%.3f ratio_pure_vs_scan=%.3f forms_agree=%d\n", 1e9 * split_s / upd, 1e9 * pure_s / upd, split_s / scan_only_s, pure_s / scan_only_s, int(forms_agree));
    say("source_filter_ns_per_raw_event=%.1f source_filter_ns_per_applied=%.1f\n", 1e9 * filter_s / double(stop), 1e9 * filter_s / upd);
    say("verifier_words pure=%llu split=%llu source_words_at_least=%llu\n", U(words_pure), U(words_split), U(2 * source_pairs));
    auto line = [&](const char* name, const Tally& t) {
        say("%-22s runs=%llu accepted=%llu rejected_at_last_check=%llu\n", name, U(t.n), U(t.accepted), U(t.at_last));
    };
    line("honest", honest); line("wrong_claim", wrong); line("liar_random", rnd); line("fabricator", fab);
    line("flipped_edge", flipped); line("dropped_update", dropped);
    line("CONTROL_leaky_flip", leak_flip); line("CONTROL_leaky_drop", leak_drop);
    line("raw_feed_no_filter", rawfeed); line("prover_labels_noops", labelled);
    say("flips_with_same_count=%llu sketch_vs_graph_mismatches=%llu\n", U(same_count), U(oracle_bad));
    std::fclose(out);
    return 0;
}

// Fully synthetic stress, outside the protocol's states: K(s, t) with shuffled ids. It has no
// triangle, R = 0 on the degree order, and D = t s^2 for s < t. It shows what Theorem 1 does not say.
int kst_mode(u32 s, u32 t, u64 seed, const char* out_path) {
    u32 n = s + t;
    int L = ceil_log2(n);
    std::mt19937_64 rng(seed);
    std::vector<u32> id(n);
    for (u32 i = 0; i < n; ++i) id[i] = i;
    std::shuffle(id.begin(), id.end(), rng);
    Graph g(n);
    Sketch<61> sk(L, seed + 1);
    for (u32 a = 0; a < s; ++a)
        for (u32 b = 0; b < t; ++b) {
            g.insert(id[a], id[s + b]);
            sk.update_split(Edit{id[a], id[s + b], 1});
        }
    std::vector<double> ts;
    i64 count = 0;
    for (int rep = 0; rep < 5; ++rep) {
        auto t0 = Clock::now();
        count = r11::static_count(g).first;
        ts.push_back(since(t0));
    }
    Bounds b = bounds(g, L);
    Audit<61> a(sk, Fp<61>::from_i64(6 * count));
    auto t0 = Clock::now();
    Prover<61> p(g, L);
    Outcome o = run<61>(a, p, L);
    double ps = since(t0), st = median(ts);
    FILE* out = std::fopen(out_path, "w");
    if (!out) { std::puts("cannot open output"); return 2; }
    auto say = [&](const char* fmt, auto... x) { std::printf(fmt, x...); std::fprintf(out, fmt, x...); };
    say("label=fully_synthetic K(%u,%u) shuffled ids n=%u m=%llu L=%d T=%lld\n", s, t, n, (unsigned long long)g.m(), L, (long long)count);
    say("D=%llu R=%llu refined=%llu iters=%llu accepted=%d static_s=%.4f prover_s=%.2f ratio=%.0f\n", (unsigned long long)b.D, (unsigned long long)b.R, (unsigned long long)b.refined,
        (unsigned long long)p.iters, int(o.accepted), st, ps, ps / st);
    std::fclose(out);
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc >= 6 && std::string(argv[1]) == "kst") return kst_mode(u32(std::atoi(argv[2])), u32(std::atoi(argv[3])), std::strtoull(argv[4], nullptr, 10), argv[5]);
    if (argc >= 5 && std::string(argv[1]) == "selftest") return selftest(std::atoi(argv[2]), std::strtoull(argv[3], nullptr, 10), argv[4]);
    if (argc >= 9 && std::string(argv[1]) == "state")
        return state_mode(argv[2], argv[3], std::atof(argv[4]), std::atoi(argv[5]), std::atoi(argv[6]), std::strtoull(argv[7], nullptr, 10), argv[8]);
    std::puts("usage: receipts selftest graphs small_trials out.txt | receipts state events.bin label frac full_seeds cheap_seeds seed_base out_prefix");
    return 2;
}
