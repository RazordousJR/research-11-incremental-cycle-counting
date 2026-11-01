// M1C02: exact dynamic 5-cycle counting, kernels and a kill test. Protocol: docs/experiments/screen-b-protocol.md, M1C02.
// Usage: r11_fivecycle <events.bin> <stream> <insert|expiry> <cut> <arm> <outdir> [budget multiple]
// Each arm is its own process, so that one process runs at a time and each stays under ten minutes. Per-batch rows go to
// <outdir>/raw and scripts/summarise_fivecycle.py joins them, compares the counts on every batch and applies the rule.
//
// THE FORMULA. A 5-cycle through the pair (u, v) is a path u-a-b-c-v on five distinct vertices that does not use the pair, closed by
// it. Fix A[b], the paths u-a-b with a != v and b not in {u, v}, and C[b], the paths v-c-b with c != u and b not in {u, v}.
// Then sum over b of A[b] C[b] counts every triple (a, b, c) with a != b and c != b, and the only triples that are not paths are those
// with a = c. Such a t = a = c lies in N(u) and N(v), and any neighbour of t other than u and v gives one such triple, so the total to
// remove is the sum over t in N(u) and N(v) of (d(t) - 2). Neither term reads whether the pair is present. d(t) is not changed by the
// pair, since t is neither endpoint. The protocol's formula stands, with b kept out of {u, v} in both rows.
//
// FORMS, FIXED BEFORE THE FIRST STREAM.
// R2  Row A built at the endpoint of lower degree. Side C streamed through A, so sum A[b] C[b] is read without storing C. This is how
//     scout S1 priced two rows: both sides at about 2 per element, no join term. PR uses the same streamed spoke side, so PR / R2
//     measures sharing alone. The rule reads this R2.
// R2B Both rows stored, then joined over the shorter touched list, which is the plainest reading of "two-row scan". Outside the rule.
//     The summariser reports the verdict with R2B in the place of R2, as a sensitivity.
// WJ  With W[a, c] the common neighbours of a and c in the graph without (u, v), and tau = |N(u) and N(v)|, p = d(u), q = d(v):
//     cycles = sum over a in N(u), c in N(v), a != c of W[a, c], minus tau (p - 1), minus tau (q - 1). W counts b = u exactly when c is
//     also in N(u), and b = v exactly when a is also in N(v). tau falls out of the a = c cases of the loop, so no degree is read.
// PR  Star groups as in Gate 0, batch-degree pivots. Row at x built once, and adjusted by each spoke. A group of one is R2.
// PW  PR's idea on top of WJ, outside the rule, to price sharing on top of the join. The table is frozen for the group. W without
//     centre x does not change across a star, since only x's edges change. A memo Q[c] holds the sum of that over the current N(x), and
//     moves to a later spoke by reading only the spokes it has not seen. Table upkeep runs per edge afterwards at full charge.
// RC  Algorithm 1 of Shi, Huang, Shun (JEA 2022), sequential: relabel by non-increasing degree, orient by a degeneracy order, then
//     O(m alpha^2). Sort, peel, relabel and list building are all charged per batch. A batch with no net update does not recount.
// HS  Computed by the summariser: per-batch minimum of R2, WJ, PR, RC. The written form leaves table upkeep out of batches that do not
//     pick WJ. A second form with table state is reported beside it.
//
// BUDGET ON RC. A recount on a large insert-only graph at a cut of 100 cannot finish inside the machine rule. RC therefore stops a
// recount when it passes M times the dearest of R2, WJ, PR on that batch, M = 4 unless a run states otherwise. The batch is then
// marked censored and charged what was spent, which is a lower bound on its true cost. A censored batch can never be the hindsight
// minimum, and the rule's factor of 2 cannot be decided by one. Cells with censored batches report RC as "at least". A batch with no
// such file falls back to the arms that have one. Forced full recounts check the counts at up to 20 batches and are not charged.
//
// CHARGES. As Gate 0: 1 per list element scanned, 1 per adjacency probe, 2 per raw event, 2 per applied edit, 3 per net update for the
// group arms. Added by the protocol: 1 per row or table read and 1 per write, so an increment is 2. Rows hold an epoch in the same word,
// so there is no reset. Degree reads 1. A created or removed table key is 1 more write, and a table rehash is 2 per live entry moved.
#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../src/r11/tc.hpp"

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

namespace fc {

using namespace r11;

inline int MUT = 0;  // 0 in every real run. tests/test_fivecycle.cpp sets the mutants, listed there.

struct Cost {
    u64 scan = 0, probe = 0, row = 0, deg = 0, tab_read = 0, tab_write = 0, graph = 0, net = 0, group = 0, rc = 0;
    u64 upkeep = 0;     // a tally inside the total: everything spent keeping the table, rehash included
    u64 total() const { return scan + probe + row + deg + tab_read + tab_write + graph + net + group + rc; }
};

// A sparse row: value and epoch in one word, so reading or writing it is one operation and no reset is needed.
struct Row {
    std::vector<u64> w;
    u32 ep = 0;
    explicit Row(u32 n) : w(n, 0) {}
    void next() { ++ep; }
    u32 get(u32 i) const { u64 x = w[i]; return u32(x >> 32) == ep ? u32(x) : 0; }
    void add(u32 i, int s) {
        u64 x = w[i];
        u32 c = u32(x >> 32) == ep ? u32(x) : 0;
        w[i] = (u64(ep) << 32) | u32(i64(c) + s);
    }
};

// sum over t in N(u) and N(v) of (d(t) - 2). The pair may be present or absent.
inline i64 degenerate(const Graph& g, u32 u, u32 v, Cost& c, u64* tau = nullptr) {
    u32 s = g.deg(u) <= g.deg(v) ? u : v, o = s == u ? v : u;
    c.deg += 2;
    i64 sum = 0;
    u64 t = 0;
    for (u32 w : g.nbr(s)) {
        ++c.scan;
        if (w == o) continue;
        ++c.probe;
        if (g.has(w, o)) {
            ++c.deg;
            ++t;
            sum += i64(g.deg(w)) - (MUT == 2 ? 0 : 2);
        }
    }
    if (tau) *tau = t;
    return MUT == 1 ? 0 : sum;
}

// Build the row of paths p-a-b, a != q, b not in {p, q}.
inline void build_row(const Graph& g, u32 p, u32 q, Row& row, Cost& c) {
    bool keep_q = MUT == 3;
    for (u32 a : g.nbr(p)) {
        ++c.scan;
        if (a == q) continue;
        c.scan += g.deg(a);
        for (u32 b : g.nbr(a))
            if (b != p && (b != q || keep_q)) { row.add(b, 1); c.row += 2; }
    }
}

// Stream the paths q-c-b, c != p, b not in {p, q}, through a row built at p. Returns sum over b of A[b] C[b].
inline i64 stream_row(const Graph& g, u32 q, u32 p, const Row& row, Cost& c) {
    bool keep_q = MUT == 3;
    i64 s = 0;
    for (u32 m : g.nbr(q)) {
        ++c.scan;
        if (m == p) continue;
        c.scan += g.deg(m);
        for (u32 b : g.nbr(m))
            if (b != p && (b != q || keep_q)) { s += row.get(b); ++c.row; }
    }
    return s;
}

inline i64 through_r2(const Graph& g, u32 u, u32 v, Row& row, Cost& c) {
    u32 p = g.deg(u) <= g.deg(v) ? u : v, q = p == u ? v : u;
    c.deg += 2;
    row.next();
    build_row(g, p, q, row, c);
    return stream_row(g, q, p, row, c) - degenerate(g, u, v, c);
}

// The literal form: both rows stored, then joined over the shorter touched list.
struct TwoRows {
    Row a, b;
    std::vector<u32> ta, tb;
    explicit TwoRows(u32 n) : a(n), b(n) {}
};

inline void build_row_touched(const Graph& g, u32 p, u32 q, Row& row, std::vector<u32>& t, Cost& c) {
    t.clear();
    for (u32 m : g.nbr(p)) {
        ++c.scan;
        if (m == q) continue;
        c.scan += g.deg(m);
        for (u32 b : g.nbr(m))
            if (b != p && b != q) {
                ++c.row;
                if (row.get(b) == 0) { t.push_back(b); ++c.row; }
                row.add(b, 1);
                ++c.row;
            }
    }
}

inline i64 through_r2b(const Graph& g, u32 u, u32 v, TwoRows& r, Cost& c) {
    r.a.next();
    r.b.next();
    build_row_touched(g, u, v, r.a, r.ta, c);
    build_row_touched(g, v, u, r.b, r.tb, c);
    const bool use_a = r.ta.size() <= r.tb.size();
    const std::vector<u32>& t = use_a ? r.ta : r.tb;
    const Row& mine = use_a ? r.a : r.b;
    const Row& other = use_a ? r.b : r.a;
    i64 s = 0;
    c.scan += t.size();
    c.row += 2 * t.size();
    for (u32 b : t) s += i64(mine.get(b)) * other.get(b);
    return s - degenerate(g, u, v, c);
}

// Shared pivot row for a star. Telescoping: each spoke sees the graph with the earlier spokes already in or already out.
inline i64 star_pr(Graph& g, const Group& gr, bool ins, Row& row, Cost& c) {
    const u32 x = gr.x;
    if (gr.c.size() == 1) {
        u32 v = gr.c[0];
        i64 s = through_r2(g, x, v, row, c);
        if (ins) g.insert(x, v); else g.erase(x, v);
        c.graph += 2;
        return ins ? s : -s;
    }
    row.next();
    for (u32 a : g.nbr(x)) {
        ++c.scan;
        c.scan += g.deg(a);
        for (u32 b : g.nbr(a))
            if (b != x) { row.add(b, 1); c.row += 2; }
    }
    i64 delta = 0;
    for (u32 v : gr.c) {
        if (!ins && MUT != 7) {
            c.scan += g.deg(v);
            for (u32 b : g.nbr(v))
                if (b != x) { row.add(b, -1); c.row += 2; }
        }
        i64 s = stream_row(g, v, x, row, c) - degenerate(g, x, v, c);
        delta += ins ? s : -s;
        if (ins) {
            if (MUT != 6) {
                c.scan += g.deg(v);
                for (u32 b : g.nbr(v)) { row.add(b, 1); c.row += 2; }
            }
            g.insert(x, v);
        } else {
            g.erase(x, v);
        }
        c.graph += 2;
    }
    return delta;
}

// Persistent wedge table. Open addressing, linear probing, backward-shift deletion, one slot is 12 bytes.
class Wedge {
public:
    static constexpr u64 kSlotBytes = 12;
    u64 cap_bytes = 6000000000ull;
    bool capped = false;
    u64 peak_entries = 0, peak_slots = 0, peak_transient = 0;

    Wedge() { alloc(1u << 8); peak_slots = slots(); peak_transient = bytes(); }
    u64 size() const { return size_; }
    u64 slots() const { return mask_ + 1; }
    u64 bytes() const { return slots() * kSlotBytes; }
    static u64 pair_key(u32 a, u32 b) { return a < b ? (u64(a) << 32) | b : (u64(b) << 32) | a; }

    void prefetch(u64 k) const { __builtin_prefetch(&keys_[slot(k)]); }
    u32 get(u64 k) const {
        for (u64 i = slot(k);; i = (i + 1) & mask_) {
            if (keys_[i] == kEmpty) return 0;
            if (keys_[i] == k) return vals_[i];
        }
    }
    // Returns true when a key was created.
    bool add(u64 k, Cost& c) {
        if ((size_ + 1) * 10 > slots() * 7 && !grow(c)) return false;
        u64 i = slot(k);
        while (keys_[i] != kEmpty && keys_[i] != k) i = (i + 1) & mask_;
        c.tab_read += 1;
        c.tab_write += 1;
        if (keys_[i] == k) { ++vals_[i]; return false; }
        keys_[i] = k;
        vals_[i] = 1;
        ++size_;
        c.tab_write += 1;
        peak_entries = std::max(peak_entries, size_);
        return true;
    }
    void sub(u64 k, Cost& c) {
        u64 i = slot(k);
        while (keys_[i] != k) {
            if (keys_[i] == kEmpty) return;     // only after a cap hit, when the run is already void
            i = (i + 1) & mask_;
        }
        c.tab_read += 1;
        c.tab_write += 1;
        if (--vals_[i] != 0) return;
        c.tab_write += 1;
        for (u64 j = (i + 1) & mask_; keys_[j] != kEmpty; j = (j + 1) & mask_) {
            u64 home = slot(keys_[j]);
            if (((j - home) & mask_) >= ((j - i) & mask_)) {
                keys_[i] = keys_[j];
                vals_[i] = vals_[j];
                i = j;
            }
        }
        keys_[i] = kEmpty;
        --size_;
    }
    template <class F> void for_each(F f) const {
        for (u64 i = 0; i <= mask_; ++i)
            if (keys_[i] != kEmpty) f(keys_[i], vals_[i]);
    }

private:
    static constexpr u64 kEmpty = ~u64(0);
    u64 slot(u64 k) const { return ((k ^ (k >> 29)) * 0x9E3779B97F4A7C15ull) >> shift_; }
    void alloc(u64 cap) {
        keys_.assign(cap, kEmpty);
        vals_.assign(cap, 0);
        mask_ = cap - 1;
        shift_ = 64;
        for (u64 x = cap; x > 1; x >>= 1) --shift_;
    }
    bool grow(Cost& c) {
        u64 cap = slots() * 2;
        if ((slots() + cap) * kSlotBytes > cap_bytes) { capped = true; return false; }
        std::vector<u64> ok = std::move(keys_);
        std::vector<u32> ov = std::move(vals_);
        alloc(cap);
        for (size_t i = 0; i < ok.size(); ++i)
            if (ok[i] != kEmpty) {
                u64 j = slot(ok[i]);
                while (keys_[j] != kEmpty) j = (j + 1) & mask_;
                keys_[j] = ok[i];
                vals_[j] = ov[i];
            }
        c.tab_read += size_;
        c.tab_write += size_;
        peak_slots = std::max(peak_slots, slots());
        peak_transient = std::max(peak_transient, (u64(ok.size()) + cap) * kSlotBytes);
        return true;
    }
    std::vector<u64> keys_;
    std::vector<u32> vals_;
    u64 mask_ = 0, size_ = 0;
    int shift_ = 64;
};

// W keeps the graph's wedges. The pair (u, v) must be absent from g when this runs.
inline void wedge_upkeep(const Graph& g, u32 u, u32 v, bool ins, Wedge& w, Cost& c) {
    const u64 before = c.total();
    c.scan += g.deg(v);
    for (u32 t : g.nbr(v)) {
        u64 k = Wedge::pair_key(u, t);
        if (ins) w.add(k, c); else w.sub(k, c);
    }
    if (MUT != 5) {
        c.scan += g.deg(u);
        for (u32 t : g.nbr(u)) {
            u64 k = Wedge::pair_key(v, t);
            if (ins) w.add(k, c); else w.sub(k, c);
        }
    }
    c.upkeep += c.total() - before;
}

// The pair (u, v) must be absent from g and from W.
inline i64 through_wj(const Graph& g, u32 u, u32 v, const Wedge& w, Cost& c) {
    const std::vector<u32>& nu = g.nbr(u);
    const std::vector<u32>& nv = g.nbr(v);
    const size_t q = nv.size();
    i64 s = 0;
    u64 tau = 0;
    for (u32 a : nu) {
        ++c.scan;
        c.scan += q;
        for (size_t j = 0; j < q; ++j) {
            if (j + 12 < q) w.prefetch(Wedge::pair_key(a, nv[j + 12]));
            u32 m = nv[j];
            if (m == a) { ++tau; continue; }
            s += w.get(Wedge::pair_key(a, m));
            ++c.tab_read;
        }
    }
    i64 p = i64(nu.size()), q2 = i64(q), t = i64(tau);
    if (MUT != 4) s -= t * (p - 1);
    s -= t * (q2 - 1);
    return s;
}

inline i64 edge_wj(Graph& g, u32 u, u32 v, bool ins, Wedge& w, Cost& c) {
    c.graph += 2;
    if (ins) {
        i64 s = through_wj(g, u, v, w, c);
        wedge_upkeep(g, u, v, true, w, c);
        g.insert(u, v);
        return s;
    }
    g.erase(u, v);
    wedge_upkeep(g, u, v, false, w, c);
    return -through_wj(g, u, v, w, c);
}

// PW: the working state of a star on top of the table.
struct StarMemo {
    std::vector<u64> st;       // epoch, then bit 31 set for N0(x), low bits spoke position + 1
    std::vector<i64> q;        // the memo
    std::vector<u64> qv;       // epoch, then version
    u32 ep = 0;
    explicit StarMemo(u32 n) : st(n, 0), q(n, 0), qv(n, 0) {}
};

inline i64 star_pw(Graph& g, const Group& gr, bool ins, Wedge& w, StarMemo& m, Cost& c) {
    const u32 x = gr.x;
    if (gr.c.size() == 1) return edge_wj(g, x, gr.c[0], ins, w, c);
    ++m.ep;
    const u64 tag = u64(m.ep) << 32;
    const std::vector<u32>& n0 = g.nbr(x);
    c.scan += n0.size();
    c.row += n0.size();
    for (u32 a : n0) m.st[a] = tag | 0x80000000u;
    for (size_t i = 0; i < gr.c.size(); ++i) {
        u32 v = gr.c[i];
        c.row += 2;
        u64 base = (m.st[v] >> 32) == m.ep ? m.st[v] : tag;
        m.st[v] = base | u32(i + 1);
    }
    i64 delta = 0;
    for (size_t i = 0; i < gr.c.size(); ++i) {
        const u32 v = gr.c[i];
        const u64 target = ins ? i : i + 1;       // spokes the current N(x) has already taken in or removed
        i64 sum = 0;
        u64 q = 0, tau = 0;
        for (u32 z : g.nbr(v)) {
            ++c.scan;
            if (z == x) continue;
            ++q;
            ++c.row;
            u64 sz = m.st[z];
            const bool live = (sz >> 32) == m.ep;
            const bool in0 = live && (sz & 0x80000000u);
            const u32 pos = live ? u32(sz & 0x7fffffffu) : 0;
            const bool member = ins ? (in0 || (pos != 0 && pos - 1 < i)) : (in0 && !(pos != 0 && pos - 1 <= i));
            if (member) ++tau;
            c.row += 2;     // read the version word and the memo
            u64 ver = 0;
            if ((m.qv[z] >> 32) == m.ep) {
                ver = u32(m.qv[z]);
            } else {
                i64 b = 0;
                c.scan += n0.size();
                for (size_t j = 0; j < n0.size(); ++j) {
                    if (j + 12 < n0.size()) w.prefetch(Wedge::pair_key(n0[j + 12], z));
                    u32 a = n0[j];
                    if (a == z) continue;
                    b += i64(w.get(Wedge::pair_key(a, z))) - (in0 ? 1 : 0);
                    ++c.tab_read;
                }
                m.q[z] = b;
                ++c.row;
            }
            const u64 goal = MUT == 10 ? std::min<u64>(ver, target) : target;
            bool moved = (m.qv[z] >> 32) != m.ep;
            for (; ver < goal; ++ver) {
                u32 a = gr.c[ver];
                ++c.scan;
                if (a == z) continue;
                i64 t = i64(w.get(Wedge::pair_key(a, z)));
                ++c.tab_read;
                m.q[z] += ins ? t : -(t - (in0 ? 1 : 0));
                moved = true;
            }
            if (moved) {
                c.row += 2;
                m.qv[z] = tag | ver;
            }
            sum += m.q[z];
        }
        if (MUT != 11) sum -= i64(tau) * (i64(q) - 1);
        delta += ins ? sum : -sum;
    }
    for (u32 v : gr.c) {
        c.graph += 2;
        if (ins) {
            wedge_upkeep(g, x, v, true, w, c);
            g.insert(x, v);
        } else {
            g.erase(x, v);
            wedge_upkeep(g, x, v, false, w, c);
        }
    }
    return delta;
}

// Vertices with at least one edge, kept so that RC does not walk the whole id space.
struct Active {
    std::vector<u32> list, pos;
    explicit Active(u32 n) : pos(n, UINT_MAX) {}
    void touch(const Graph& g, u32 v, Cost& c) {
        c.rc += 2;  // read the degree, read the position
        bool on = g.deg(v) > 0, in = pos[v] != UINT_MAX;
        if (on == in) return;
        c.rc += 2;
        if (on) {
            pos[v] = u32(list.size());
            list.push_back(v);
        } else {
            u32 p = pos[v], l = list.back();
            list[p] = l;
            pos[l] = p;
            list.pop_back();
            pos[v] = UINT_MAX;
        }
    }
};

// Static exact counter: Algorithm 1 of Shi, Huang, Shun (JEA 2022), after Kowalik, run sequentially.
class Recount {
public:
    u64 wedges = 0, inner = 0, sum_min = 0;
    u32 degeneracy = 0;

    explicit Recount(u32 n) : u_(n), order_(n), rank_(n), off_(n + 2), oof_(n + 2), fill_(n + 2), d_(n + 1), vert_(n + 1),
                              pos_(n + 1), adjv_(n + 1, 0), t_(n + 1, 0) {}

    // True and the count when it finished. False when it passed the budget, and ops then holds what was spent.
    bool count(const Graph& g, const std::vector<u32>& active, u64 budget, i64& result, u64& ops) {
        const u32 A = u32(active.size());
        ops = 0;
        wedges = inner = sum_min = 0;
        degeneracy = 0;
        result = 0;
        if (A == 0) return true;
        u32 maxd = 0;
        for (u32 v : active) maxd = std::max(maxd, g.deg(v));
        // Counting sort, non-increasing degree.
        std::vector<u32>& cnt = bin_;
        cnt.assign(maxd + 2, 0);
        for (u32 v : active) ++cnt[g.deg(v)];
        {
            u32 start = 0;
            for (u32 d = maxd + 1; d-- > 0;) { u32 t = cnt[d]; cnt[d] = start; start += t; }
        }
        for (u32 v : active) { u32 r = cnt[g.deg(v)]++; order_[r] = v; rank_[v] = r; }
        ops += 7ull * A + 3ull * (maxd + 2);
        // Relabelled adjacency, each list ascending by rank.
        u64 M2 = 0;
        for (u32 r = 0; r < A; ++r) { off_[r] = u32(M2); fill_[r] = u32(M2); M2 += g.deg(order_[r]); }
        off_[A] = u32(M2);
        ops += 2ull * A;
        adj_.resize(M2);
        for (u32 r = 0; r < A; ++r)
            for (u32 w : g.nbr(order_[r])) adj_[fill_[rank_[w]]++] = r;
        ops += 5 * M2;
        // Degeneracy order by bucket peeling, on the relabelled graph. vert_ ascends by remaining degree.
        cnt.assign(maxd + 2, 0);
        for (u32 r = 0; r < A; ++r) { d_[r] = off_[r + 1] - off_[r]; ++cnt[d_[r]]; }
        {
            u32 start = 0;
            for (u32 d = 0; d <= maxd + 1; ++d) { u32 t = cnt[d]; cnt[d] = start; start += t; }
        }
        for (u32 r = A; r-- > 0;) { u32 i = A - 1 - r; vert_[i] = r; pos_[r] = i; }
        ops += 6ull * A + 3ull * (maxd + 2);
        for (u32 i = 0; i < A; ++i) {
            u32 v = vert_[i], dv = d_[v];
            degeneracy = std::max(degeneracy, dv);
            ops += 2;
            for (u32 k = off_[v]; k < off_[v + 1]; ++k) {
                u32 x = adj_[k];
                ops += 2;
                if (d_[x] > dv) {
                    u32 dx = d_[x], px = pos_[x], pw = cnt[dx], w = vert_[pw];
                    if (x != w) {
                        pos_[x] = pw; pos_[w] = px;
                        vert_[px] = w; vert_[pw] = x;
                    }
                    ++cnt[dx];
                    --d_[x];
                    ops += 10;
                }
            }
        }
        if (ops > budget) return false;
        // Out-lists in the degeneracy orientation, each ascending by rank.
        for (u32 r = 0; r <= A; ++r) fill_[r] = 0;
        ops += A + 1;
        for (u32 r = 0; r < A; ++r)
            for (u32 k = off_[r]; k < off_[r + 1]; ++k)
                if (pos_[adj_[k]] < pos_[r]) ++fill_[adj_[k]];
        ops += 4 * M2;
        {
            u32 s = 0;
            for (u32 r = 0; r < A; ++r) { oof_[r] = s; s += fill_[r]; fill_[r] = oof_[r]; }
            oof_[A] = s;
            out_.resize(s);
            ops += 3ull * A;
        }
        for (u32 r = 0; r < A; ++r)
            for (u32 k = off_[r]; k < off_[r + 1]; ++k)
                if (pos_[adj_[k]] < pos_[r]) out_[fill_[adj_[k]]++] = r;
        ops += 4 * M2 + 2 * M2;
        if (ops > budget) return false;
        // Algorithm 1. Lists ascend, so N_v(u) is a suffix, walked from the end. The element that stops a walk is charged.
        i64 total = 0;
        const bool mut_t = MUT == 8, mut_a = MUT == 9, mut_v = MUT == 12;
        for (u32 v = 0; v < A; ++v) {
            u_.next();
            const u64 vtag = ++stamp_;      // stamps are unique across recounts, because a recount can stop part way
            const u32* ab = adj_.data() + off_[v];
            const u32* ae = adj_.data() + off_[v + 1];
            ops += 2 + 2;       // two offset reads, and the element that ends each of the two outer walks
            u64 w1 = 0;
            for (const u32* p = ae; p != ab && p[-1] > v; --p) {
                const u32 m = p[-1];
                adjv_[m] = vtag;
                ops += 3;       // the list element, the stamp write, the element that ends the inner walk
                const u32* mb = adj_.data() + off_[m];
                for (const u32* q = adj_.data() + off_[m + 1]; q != mb && q[-1] > v; --q) { u_.add(q[-1], 1); ++w1; }
            }
            ops += 3 * w1;      // one list element, then an increment of U
            wedges += w1;
            for (const u32* p = ae; p != ab && p[-1] > v; --p) {
                const u32 m = p[-1];
                ops += 1;
                const u32* mb = adj_.data() + off_[m];
                const u32* me = adj_.data() + off_[m + 1];
                u64 k = 0, x_seen = 0, x_read = 0;
                const u64 tag = ++stamp_;
                for (const u32* q = me; q != mb && q[-1] > v; --q) { t_[q[-1]] = tag; ++k; }
                for (const u32* q = me; q != mb && q[-1] > v; --q) {
                    const u32 w = q[-1];
                    const int minus = (!mut_a && adjv_[w] == vtag) ? 1 : 0;
                    const u32* ob = out_.data() + oof_[w];
                    const u32* oe = out_.data() + oof_[w + 1];
                    for (const u32* s = oe; s != ob && (mut_v || s[-1] > v); --s) {
                        const u32 x = s[-1];
                        ++x_seen;
                        if (x == m) continue;
                        ++x_read;
                        total += i64(u_.get(x)) - ((!mut_t && t_[x] == tag) ? 1 : 0) - minus;
                    }
                    ++x_seen;  // the element that ended the walk
                }
                // k walks twice at 2 each (list element, stamp), two ending elements, then one list element per x and two reads
                ops += 2 * k + 1 + 2 * k + 1 + x_seen + 2 * x_read;
                inner += x_seen;
                if (ops > budget) return false;
            }
        }
        for (u32 r = 0; r < A; ++r)
            for (u32 k = off_[r]; k < off_[r + 1]; ++k)
                if (adj_[k] > r) sum_min += std::min(off_[r + 1] - off_[r], off_[adj_[k] + 1] - off_[adj_[k]]);
        result = total;
        return true;
    }

private:
    Row u_;
    std::vector<u32> order_, rank_, off_, oof_, fill_, d_, vert_, pos_, adj_, out_, bin_;
    std::vector<u64> adjv_, t_;
    u64 stamp_ = 0;
};

// One arm with its own graph. All of the arms' state lives here, so the oracle and the sweep run the same code.
enum class Arm { R2, R2B, WJ, PR, PW, RC };

inline const char* arm_name(Arm a) {
    switch (a) {
        case Arm::R2: return "R2";
        case Arm::R2B: return "R2B";
        case Arm::WJ: return "WJ";
        case Arm::PR: return "PR";
        case Arm::PW: return "PW";
        default: return "RC";
    }
}

struct Engine5 {
    Arm arm;
    Graph g;
    Cost c;
    Row row;
    TwoRows two;
    Wedge wedge;
    StarMemo memo;
    Active active;
    Recount rc;
    i64 t = 0;                  // the running count, or the last finished recount for RC
    bool t_known = true;        // false for RC after a censored batch, until the next finished recount
    u64 upkeep = 0, wedges = 0, max_update = 0, groups = 0, spokes = 0;
    u64 censored = 0, finished = 0;

    Engine5(Arm a, u32 n) : arm(a), g(n), row(n), two(n), memo(n), active(n), rc(n) {}

    // Wedges in the graph, sum of C(d, 2). Only what a rebuild of the table would cost, for the summariser. du and dv are read with the
    // pair absent. The degrees are free: this tally is not an operation of any arm.
    void track_wedges(u64 du, u64 dv, bool ins) {
        if (ins) wedges += du + dv; else wedges -= du + dv;
    }

    void per_edge(const Edge& e, bool ins) {
        u64 before = c.total();
        const u64 lose = ins ? 0 : 1;
        track_wedges(g.deg(e.a) - lose, g.deg(e.b) - lose, ins);
        i64 d = 0;
        if (arm == Arm::R2) {
            d = through_r2(g, e.a, e.b, row, c);
            if (ins) g.insert(e.a, e.b); else g.erase(e.a, e.b);
            c.graph += 2;
            d = ins ? d : -d;
        } else if (arm == Arm::R2B) {
            d = through_r2b(g, e.a, e.b, two, c);
            if (ins) g.insert(e.a, e.b); else g.erase(e.a, e.b);
            c.graph += 2;
            d = ins ? d : -d;
        } else {
            d = edge_wj(g, e.a, e.b, ins, wedge, c);
        }
        t += d;
        max_update = std::max(max_update, c.total() - before);
    }

    // The batch cost in operations, with a recount budget for RC only.
    u64 apply(const std::vector<Update>& b, const NetBatch& nb, u64 budget) {
        u64 before = c.total();
        const u64 net = nb.del.size() + nb.ins.size();
        c.net += 2 * b.size();
        if (arm == Arm::RC) {
            c.graph += 2 * net;
            for (const Edge& e : nb.del) g.erase(e.a, e.b);
            for (const Edge& e : nb.ins) g.insert(e.a, e.b);
            if (net == 0) return c.total() - before;
            for (const Edge& e : nb.del) { active.touch(g, e.a, c); active.touch(g, e.b, c); }
            for (const Edge& e : nb.ins) { active.touch(g, e.a, c); active.touch(g, e.b, c); }
            u64 ops = 0;
            i64 r = 0;
            if (rc.count(g, active.list, budget, r, ops)) {
                t = r;
                t_known = true;
                ++finished;
            } else {
                t_known = false;
                ++censored;
            }
            c.rc += ops;
            return c.total() - before;
        }
        if (arm == Arm::PR || arm == Arm::PW) {
            c.group += 3 * net;
            for (int pass = 0; pass < 2; ++pass) {
                const bool ins = pass == 1;
                for (const Group& gr : make_groups(g, ins ? nb.ins : nb.del, Pivot::BatchDegree)) {
                    if (gr.c.size() > 1) { ++groups; spokes += gr.c.size(); }
                    u64 g0 = c.total();
                    u64 dx = g.deg(gr.x);
                    for (u32 v : gr.c) {
                        if (ins) { track_wedges(dx, g.deg(v), true); ++dx; }
                        else { --dx; track_wedges(dx, u64(g.deg(v)) - 1, false); }
                    }
                    t += arm == Arm::PR ? star_pr(g, gr, ins, row, c) : star_pw(g, gr, ins, wedge, memo, c);
                    max_update = std::max(max_update, (c.total() - g0) / gr.c.size());
                }
            }
            return c.total() - before;
        }
        for (const Edge& e : nb.del) per_edge(e, false);
        for (const Edge& e : nb.ins) per_edge(e, true);
        return c.total() - before;
    }
};

}  // namespace fc

#ifndef FIVECYCLE_NO_MAIN

using namespace fc;

namespace {

#pragma pack(push, 1)
struct Event { std::int64_t t; u32 u, v, op; };
#pragma pack(pop)

constexpr double kTimeGuard = 540.0;  // seconds. The run script holds a harder stop at 590 s.

u64 peak_rss() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof pmc)) return u64(pmc.PeakWorkingSetSize);
#endif
    return 0;
}

bool parse_arm(const std::string& s, Arm& a) {
    for (Arm x : {Arm::R2, Arm::R2B, Arm::WJ, Arm::PR, Arm::PW, Arm::RC})
        if (s == arm_name(x)) { a = x; return true; }
    return false;
}

// Per-batch operations of a finished arm, if its file exists. Column 3 of the raw file is the batch total.
std::vector<u64> read_batch_ops(const std::string& path) {
    std::vector<u64> v;
    FILE* f = std::fopen(path.c_str(), "r");
    if (!f) return v;
    char line[512];
    if (!std::fgets(line, sizeof line, f)) { std::fclose(f); return v; }
    while (std::fgets(line, sizeof line, f)) {
        unsigned long long batch, raw, net, ops;
        if (std::sscanf(line, "%llu,%llu,%llu,%llu", &batch, &raw, &net, &ops) == 4) v.push_back(ops);
    }
    std::fclose(f);
    return v;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 7) { std::puts("usage: r11_fivecycle events.bin stream insert|expiry cut arm outdir [budget multiple]"); return 2; }
    const std::string path = argv[1], stream = argv[2], variant = argv[3], outdir = argv[6];
    const i64 cut = std::atoll(argv[4]);
    Arm arm;
    if (!parse_arm(argv[5], arm)) { std::puts("unknown arm"); return 2; }
    const u64 mult = argc > 7 ? std::strtoull(argv[7], nullptr, 10) : 4;

    FILE* in = std::fopen(path.c_str(), "rb");
    if (!in) { std::puts("cannot open events"); return 2; }
    std::fseek(in, 0, SEEK_END);
    long long bytes = _ftelli64(in);
    std::fseek(in, 0, SEEK_SET);
    std::vector<Event> ev(size_t(bytes) / sizeof(Event));
    if (std::fread(ev.data(), sizeof(Event), ev.size(), in) != ev.size()) { std::puts("short read"); return 2; }
    std::fclose(in);

    u32 n = 0;
    for (const Event& x : ev) n = std::max({n, x.u + 1, x.v + 1});

    std::vector<size_t> starts{0};
    for (size_t i = 1; i < ev.size(); ++i)
        if (i % size_t(cut) == 0) starts.push_back(i);
    starts.push_back(ev.size());
    const size_t batches = starts.size() - 1;

    const std::string base = stream + "-" + variant + "-" + std::to_string(cut);
    const std::string raw_path = outdir + "/raw/" + base + "-" + arm_name(arm) + ".csv";
    std::vector<std::vector<u64>> prior;
    if (arm == Arm::RC)
        for (const char* name : {"R2", "WJ", "PR"}) prior.push_back(read_batch_ops(outdir + "/raw/" + base + "-" + name + ".csv"));

    auto t0 = std::chrono::steady_clock::now();
    auto elapsed = [&] { return std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count(); };

    auto engine = std::make_unique<Engine5>(arm, n);
    Engine5& s = *engine;
    FILE* out = std::fopen(raw_path.c_str(), "w");
    if (!out) { std::puts("cannot open raw output"); return 2; }
    std::fprintf(out, "batch,raw,net,ops,upkeep,wedges,t,known,checked\n");

    const size_t check_every = std::max<size_t>(1, batches / 20);
    std::vector<Update> b;
    u64 net_total = 0, peak_m = 0, checks = 0, done = 0, peak_batch = 0;
    i64 running = 0;
    bool timed_out = false;
    for (size_t bi = 0; bi < batches; ++bi) {
        b.clear();
        for (size_t i = starts[bi]; i < starts[bi + 1]; ++i) b.push_back({ev[i].u, ev[i].v, ev[i].op == 1});
        peak_batch = std::max<u64>(peak_batch, b.size());
        NetBatch nb = normalise(s.g, b);
        const u64 net = nb.ins.size() + nb.del.size();
        net_total += net;
        u64 budget = ~u64(0);
        if (arm == Arm::RC) {
            u64 top = 0;
            bool any = false;
            for (const auto& p : prior)
                if (bi < p.size()) { top = std::max(top, p[bi]); any = true; }
            if (any) budget = mult * top;
        }
        const u64 up0 = s.c.upkeep;
        const u64 ops = s.apply(b, nb, budget);
        const u64 upkeep = s.c.upkeep - up0;
        running = s.t;
        peak_m = std::max(peak_m, s.g.m());
        int checked = 0;
        if (arm == Arm::RC && (bi % check_every == 0 || bi + 1 == batches) && !s.t_known && elapsed() < 400) {
            u64 sink = 0;
            i64 r = 0;
            if (s.rc.count(s.g, s.active.list, ~u64(0), r, sink)) {
                s.t = r;
                s.t_known = true;
                running = r;
                checked = 1;
            }
        }
        if (arm == Arm::RC && checked) ++checks;
        std::fprintf(out, "%zu,%zu,%llu,%llu,%llu,%llu,%lld,%d,%d\n", bi, b.size(), (unsigned long long)net, (unsigned long long)ops,
                     (unsigned long long)upkeep, (unsigned long long)s.wedges, (long long)running, int(s.t_known), checked);
        ++done;
        if (s.wedge.capped) break;
        if (elapsed() > kTimeGuard) { timed_out = true; break; }
    }
    std::fclose(out);

    const double secs = elapsed();
    const char* status = s.wedge.capped ? "CAP" : timed_out ? "TIME" : "OK";
    const std::string sum_path = outdir + "/raw/" + base + "-" + arm_name(arm) + ".summary.csv";
    FILE* f = std::fopen(sum_path.c_str(), "w");
    if (!f) { std::puts("cannot open summary"); return 2; }
    const Cost& c = s.c;
    std::fprintf(f, "stream,variant,cut,arm,status,batches_done,batches,events,net,ops,scan,probe,row,deg,tab_read,tab_write,graph,net_ops,group,rc,"
                    "rc_censored,rc_finished,forced_checks,upkeep,peak_m,final_t,table_entries,table_slots,table_bytes,table_transient,"
                    "max_update,groups,spokes,peak_rss,seconds,budget_mult,vertices\n");
    std::fprintf(f, "%s,%s,%lld,%s,%s,%llu,%zu,%zu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%lld,%llu,%llu,%llu,%llu,"
                    "%llu,%llu,%llu,%llu,%.3f,%llu,%u\n",
                 stream.c_str(), variant.c_str(), (long long)cut, arm_name(arm), status, (unsigned long long)done, batches, ev.size(),
                 (unsigned long long)net_total, (unsigned long long)c.total(), (unsigned long long)c.scan, (unsigned long long)c.probe,
                 (unsigned long long)c.row, (unsigned long long)c.deg, (unsigned long long)c.tab_read, (unsigned long long)c.tab_write,
                 (unsigned long long)c.graph, (unsigned long long)c.net, (unsigned long long)c.group, (unsigned long long)c.rc,
                 (unsigned long long)s.censored, (unsigned long long)s.finished, (unsigned long long)checks, (unsigned long long)c.upkeep,
                 (unsigned long long)peak_m, (long long)s.t, (unsigned long long)s.wedge.peak_entries, (unsigned long long)s.wedge.peak_slots,
                 (unsigned long long)(s.wedge.peak_slots * Wedge::kSlotBytes), (unsigned long long)s.wedge.peak_transient,
                 (unsigned long long)s.max_update, (unsigned long long)s.groups, (unsigned long long)s.spokes, (unsigned long long)peak_rss(),
                 secs, (unsigned long long)mult, n);
    std::fclose(f);
    std::printf("%s %s %s status=%s done=%llu/%zu net=%llu ops=%llu t=%lld censored=%llu table=%llu secs=%.1f\n", base.c_str(), arm_name(arm), status,
                status, (unsigned long long)done, batches, (unsigned long long)net_total, (unsigned long long)c.total(), (long long)s.t,
                (unsigned long long)s.censored, (unsigned long long)(s.wedge.peak_slots * Wedge::kSlotBytes), secs);
    return 0;
}

#endif  // FIVECYCLE_NO_MAIN
