// Gate 1: when batch sharing pays, across pattern size. Protocol: docs/experiments/gate1-protocol.md.
// Usage: r11_gate1 <events.bin> <stream> <insert|expiry> <cut> <3|4|5> <arm> <outdir> [--theta T] [--cap-seconds S] [--cap-bytes B] [--cpu N] [--rep N]
// One arm per process, as M1C03. Two binaries from this one source: the count build, and the timing build (-DFC_TIMING), which has no counters and no mutants.
//
// ARMS.
//   p = 3   S3 scan         P3 star row     T3 table    M3 mixed table
//   p = 4   S4 scan         P4 star row     T4 table    M4 mixed table    B4 BFCB-IG port    A4 affected recount
//   p = 5   R2 scan         PR star row     WT table    M5 mixed table    WJ flat table      A5 affected recount
// R2, PR, WT and WJ are the M1C03 arms, reached through Engine5, and their counts must match M1C03.
//
// COMMON TO ALL. N(u) excludes the pair (u, v) when a kernel joins, except where a kernel says otherwise. The formula for p = 5 is in fivecycle.cpp.
//   p = 3   cycles through (u, v) = W[u, v], the common neighbours of u and v.
//   p = 4   cycles through (u, v) = sum over a in N(u) of W[a, v]. With the pair absent, v is not in N(u) and u is not in N(v), so W[a, v] counts no b = u.
//
// M, THE MIXED TABLE, with hubs scanned. After the idea of HHH (Hanauer, Henzinger, Hua): the table keeps only wedges through light centres, so a hub pays
// no upkeep, and hubs are read at query time. A vertex turns heavy above 2 theta and light again below theta, so a turn costs at most (2 theta)^2 pairs and is
// paid for by at least theta updates. hn[v] lists v's heavy neighbours. Writing H for the heavy set and W_L for the table,
//   p = 3   W[u, v] = W_L[u, v] + |hn[u] and hn[v]|
//   p = 4   sum over a in N(s) of W_L[a, o], plus sum over a in N(s) of |hn[a] and hn[o]|, with s the endpoint of lower degree
//   p = 5   the WT join on W_L, plus sum over h in H of (A[h] C[h] - T[h]), with A[h] = |N(u) and N(h)|, C[h] = |N(v) and N(h)|, T[h] = |N(u) and N(v) and N(h)|.
//           The join's correction terms depend only on N(u) and N(v), so they stay correct when the table holds part of W. The sum over a != c of
//           [a ~ h][c ~ h] is A[h] C[h] - T[h], and h = u or h = v needs no special case.
// T is M with theta infinite: no hub lists, no extra rows. WT's table is shared, so T4 and M5 are as tuned as WT.
//
// B4, THE BFCB-IG PORT (K. Wang et al., VLDB J 2022), to general graphs. Mixed batches split into deletions then insertions, as they do. For a pass with edit set D, G0
// without D and G1 with it, 2 c4 = sum over unordered {x, w} of C(W[x, w], 2), and W[x, w] changes only if x or w is an endpoint of D. Let S be the endpoints.
// For each x in S: a row of paths x-a-w in G1, and a difference row of those paths that use an edit, and the pass adds C(new, 2) - C(new - diff, 2) for each touched w.
// A pair with both ends in S is read from its smaller end. The total is halved. Each start vertex builds one row, which every edit at it reads, as in BFCB-IG.
// This port applies to p = 4, where the formula is native. For p = 5 no such row form exists, and PR is the nearest thing.
//
// A, THE AFFECTED RECOUNT. Every vertex of a cycle through a changed edge is within r of an endpoint of that edge, r = 1 for p = 4 and r = 2 for p = 5, in the graph
// that holds the cycle. So with A the ball of radius r in the union of the old and new graphs, delta = count(new[A]) - count(old[A]), since cycles outside A are
// unchanged and cycles inside A with no changed edge cancel. Static counters: Algorithm 1 of Shi, Huang, Shun (fivecycle.cpp) for p = 5, and Chiba and Nishizeki's
// wedge pairing for p = 4. Not built for p = 3: Gate 0 prices a triangle update at 5.9 to 13.9 operations, so a recount cannot compete.
//
// CHARGES as fivecycle.cpp. Heavy-list elements are scans. A heavy flag is one row read. Extraction charges a scan per list element and a row read per mark.
#ifndef FIVECYCLE_NO_MAIN
#define FIVECYCLE_NO_MAIN
#endif
#include "fivecycle.cpp"

namespace g1 {

using namespace fc;

inline void edit(Graph& g, u32 u, u32 v, bool ins, Cost& c) {
    if (ins) g.insert(u, v); else g.erase(u, v);
    c.graph += 2;
}

// The pair may be present or absent.
inline i64 through3(const Graph& g, u32 u, u32 v, Cost& c) {
    const u32 s = g.deg(u) <= g.deg(v) ? u : v, o = s == u ? v : u;
    c.deg += 2;
    i64 n = 0;
    for (u32 w : g.nbr(s)) {
        ++c.scan;
        if (w == o) continue;
        ++c.probe;
        n += g.has(w, o);
    }
    return n;
}

// The pair may be present or absent. Mark N(o), then walk the paths from s.
inline i64 through4(const Graph& g, u32 u, u32 v, Row& row, Cost& c) {
    const u32 s = g.deg(u) <= g.deg(v) ? u : v, o = s == u ? v : u;
    c.deg += 2;
    row.next();
    for (u32 b : g.nbr(o)) {
        ++c.scan;
        if (b != s) { row.add(b, 1); c.row += 2; }
    }
    i64 n = 0;
    const std::vector<u32>& ns = g.nbr(s);
    for (size_t i = 0; i < ns.size(); ++i) {
        prefetch_lists(g, ns.data(), i, ns.size());
        const u32 a = ns[i];
        ++c.scan;
        if (a == o) continue;
        c.scan += g.deg(a);
        for (u32 b : g.nbr(a)) { n += row.get(b); ++c.row; }
    }
    return n;
}

// The guards below read degrees only, are charged, and use nothing beyond the current batch. M1D02 showed why they are needed: a star at a hub with a few small spokes
// cost P4 more than S4, because S4 walks from the lower-degree side and an unguarded row always walks from x. An arm must not lose to a careless form, and neither has a tuned parameter.

// Triangles: build the row only if writing it costs no more than the probes the spokes would make.
inline bool row_pays3(const Graph& g, const GroupV& gr, Cost& c) {
    const u64 dx = g.deg(gr.x);
    u64 probes = 0;
    for (u32 v : gr.c) probes += std::min<u64>(dx, g.deg(v));
    c.deg += gr.c.size() + 1;
    return dx <= probes;
}

// 4-cycles: build the row only if the row and its spokes cost less than the scans they replace, as S4 prices them.
inline bool row_pays4(const Graph& g, const GroupV& gr, Cost& c) {
    const u32 x = gr.x;
    const u64 dx = g.deg(x);
    u64 walk = 0;
    for (u32 a : g.nbr(x)) walk += g.deg(a);
    c.deg += dx;
    u64 star = walk, scans = 0;
    for (u32 v : gr.c) {
        const u64 dv = g.deg(v);
        star += 2 * dv;
        ++c.deg;
        if (dv >= dx) { scans += dv + walk; continue; }
        u64 w = 0;
        for (u32 a : g.nbr(v)) w += g.deg(a);
        c.deg += dv;
        scans += dx + w;
    }
    return star < scans;
}

// Star at x for triangles. Row holds N(x) as adjusted by the spokes so far. A spoke of higher degree than x is read from x's side, so sharing never makes a spoke dearer than S3.
inline i64 star3(Graph& g, const GroupV& gr, bool ins, Row& row, Cost& c) {
    const u32 x = gr.x;
    if (gr.c.size() == 1 || !row_pays3(g, gr, c)) {
        i64 delta = 0;
        for (u32 v : gr.c) {
            const i64 s = through3(g, x, v, c);
            edit(g, x, v, ins, c);
            delta += ins ? s : -s;
        }
        return delta;
    }
    row.next();
    for (u32 a : g.nbr(x)) { ++c.scan; row.add(a, 1); c.row += 2; }
    i64 delta = 0;
    for (u32 v : gr.c) {
        if (!ins && MUT != 26) { row.add(v, -1); c.row += 2; }
        i64 s = 0;
        c.deg += 2;
        if (g.deg(v) <= g.deg(x)) {
            for (u32 w : g.nbr(v)) { ++c.scan; ++c.row; s += row.get(w); }
        } else {
            s = through3(g, x, v, c);
        }
        delta += ins ? s : -s;
        if (ins && MUT != 25) { row.add(v, 1); c.row += 2; }
        edit(g, x, v, ins, c);
    }
    return delta;
}

// Star at x for 4-cycles. A[b] is the paths x-a-b with b != x, kept right as each spoke moves.
inline i64 star4(Graph& g, const GroupV& gr, bool ins, Row& row, Cost& c) {
    const u32 x = gr.x;
    if (gr.c.size() == 1 || !row_pays4(g, gr, c)) {
        i64 delta = 0;
        for (u32 v : gr.c) {
            const i64 s = through4(g, x, v, row, c);
            edit(g, x, v, ins, c);
            delta += ins ? s : -s;
        }
        return delta;
    }
    row.next();
    const std::vector<u32>& nx = g.nbr(x);
    for (size_t i = 0; i < nx.size(); ++i) {
        prefetch_lists(g, nx.data(), i, nx.size());
        const u32 a = nx[i];
        ++c.scan;
        c.scan += g.deg(a);
        for (u32 b : g.nbr(a))
            if (b != x) { row.add(b, 1); c.row += 2; }
    }
    i64 delta = 0;
    for (u32 v : gr.c) {
        if (!ins && MUT != 28) {
            c.scan += g.deg(v);
            for (u32 b : g.nbr(v))
                if (b != x) { row.add(b, -1); c.row += 2; }
        }
        i64 s = 0;
        c.scan += g.deg(v);
        for (u32 b : g.nbr(v)) {
            if (b == x) continue;
            s += row.get(b);
            ++c.row;
        }
        delta += ins ? s : -s;
        if (ins && MUT != 27) {
            c.scan += g.deg(v);
            for (u32 b : g.nbr(v))
                if (b != x) { row.add(b, 1); c.row += 2; }
        }
        edit(g, x, v, ins, c);
    }
    return delta;
}

// The table, and with a finite theta the mixed table. The header says what it holds.
class Mixed {
public:
    static constexpr u32 kNoHubs = 0xffffffffu;
    WTable wt;

    Mixed(u32 n, u32 theta, u32 count_bits = 0) : wt(n, count_bits), theta_(theta), hubs_(theta != kNoHubs), heavy_(hubs_ ? n : 0, 0), hn_(hubs_ ? n : 0),
                                                  mk_(hubs_ ? n : 0), ha_(hubs_ ? n : 0), hc_(hubs_ ? n : 0), ht_(hubs_ ? n : 0) {}

    u64 heavy_now() const { return heavy_count_; }
    u64 turns() const { return turns_; }

    // The pair must be absent from g when this runs.
    void upkeep(const Graph& g, u32 u, u32 v, bool ins, Cost& c) {
        const u64 before = c.total();
        if (!hubs_) {
            wt.change_list(u, g.nbr(v), ins, c);
            wt.change_list(v, g.nbr(u), ins, c);
        } else {
            c.row += 2;
            one_side(g, u, v, ins, c);
            one_side(g, v, u, ins, c);
        }
        c.upkeep += c.total() - before;
    }

    void reclass(const Graph& g, u32 v, Cost& c) {
        if (!hubs_) return;
        const u32 d = g.deg(v);
        ++c.deg;
        ++c.row;
        if (!heavy_[v] && u64(d) > 2 * u64(theta_)) turn(g, v, true, c);
        else if (heavy_[v] && d < theta_) turn(g, v, false, c);
    }

    // The pair must be absent from g and from the table.
    i64 through(int p, const Graph& g, u32 u, u32 v, Cost& c) {
        if (p == 3) return through_tri(u, v, c);
        if (p == 4) return through_c4(g, u, v, c);
        return through_c5(g, u, v, c);
    }

private:
    void one_side(const Graph& g, u32 u, u32 v, bool ins, Cost& c) {
        if (!heavy_[v] || MUT == 20) wt.change_list(u, g.nbr(v), ins, c);
        if (heavy_[v] && MUT != 22) hub_list(u, v, ins, c);
    }

    void hub_list(u32 a, u32 h, bool ins, Cost& c) {
        std::vector<u32>& l = hn_[a];
        if (ins) {
            l.push_back(h);
            ++c.row;
            return;
        }
        for (size_t i = 0; i < l.size(); ++i) {
            ++c.scan;
            if (l[i] == h) {
                l[i] = l.back();
                l.pop_back();
                c.row += 2;
                return;
            }
        }
    }

    void turn(const Graph& g, u32 v, bool to_heavy, Cost& c) {
        const u64 before = c.total();
        const std::vector<u32>& l = g.nbr(v);
        for (size_t i = 0; i + 1 < l.size(); ++i) {
            tmp_.assign(l.begin() + i + 1, l.end());
            if (MUT != 21) wt.change_list(l[i], tmp_, !to_heavy, c);
        }
        for (u32 a : l) hub_list(a, v, to_heavy, c);
        heavy_[v] = to_heavy;
        heavy_count_ += to_heavy ? 1 : -1;
        ++c.row;
        ++turns_;
        c.upkeep += c.total() - before;
    }

    i64 common_hubs(u32 s, u32 o, Cost& c) {
        mk_.next();
        for (u32 h : hn_[o]) { ++c.scan; mk_.add(h, 1); c.row += 2; }
        i64 n = 0;
        for (u32 h : hn_[s]) { ++c.scan; ++c.row; n += mk_.get(h); }
        return n;
    }

    i64 through_tri(u32 u, u32 v, Cost& c) {
        ++c.tab_read;
        i64 n = wt.get(u, v);
        return hubs_ ? n + common_hubs(u, v, c) : n;
    }

    i64 through_c4(const Graph& g, u32 u, u32 v, Cost& c) {
        const u32 s = g.deg(u) <= g.deg(v) ? u : v, o = s == u ? v : u;
        c.deg += 2;
        i64 n = 0;
        const std::vector<u32>& ns = g.nbr(s);
        c.scan += ns.size();
        c.tab_read += ns.size() + 1;
        for (u32 a : ns) n += wt.get(o, a);
        if (!hubs_ || MUT == 24) return n;
        mk_.next();
        for (u32 h : hn_[o]) { ++c.scan; mk_.add(h, 1); c.row += 2; }
        if (hn_[o].empty()) return n;
        for (u32 a : ns)
            for (u32 h : hn_[a]) { ++c.scan; ++c.row; n += mk_.get(h); }
        return n;
    }

    i64 through_c5(const Graph& g, u32 u, u32 v, Cost& c) {
        i64 n = wt.join(g, u, v, c);
        if (!hubs_) return n;
        mk_.next();
        ha_.next();
        hc_.next();
        ht_.next();
        seen_.clear();
        for (u32 z : g.nbr(v)) { ++c.scan; mk_.add(z, 1); c.row += 2; }
        for (u32 a : g.nbr(u)) {
            ++c.scan;
            ++c.row;
            const bool both = mk_.get(a) != 0;
            for (u32 h : hn_[a]) {
                ++c.scan;
                c.row += 3;
                if (ha_.get(h) == 0) seen_.push_back(h);
                ha_.add(h, 1);
                if (both) { ht_.add(h, 1); c.row += 2; }
            }
        }
        for (u32 z : g.nbr(v))
            for (u32 h : hn_[z]) { ++c.scan; hc_.add(h, 1); c.row += 2; }
        for (u32 h : seen_) {
            ++c.scan;
            c.row += 3;
            n += i64(ha_.get(h)) * hc_.get(h) - (MUT == 23 ? 0 : i64(ht_.get(h)));
        }
        return n;
    }

    u32 theta_;
    bool hubs_;
    std::vector<char> heavy_;
    std::vector<std::vector<u32>> hn_;
    Row mk_, ha_, hc_, ht_;
    std::vector<u32> seen_, tmp_;
    u64 heavy_count_ = 0, turns_ = 0;
};

// An extracted graph for the static counters.
struct Sub {
    std::vector<u32> off, adj;
    u32 deg(u32 v) const { return off[v + 1] - off[v]; }
    Spokes nbr(u32 v) const { return Spokes{adj.data() + off[v], adj.data() + off[v + 1]}; }
    void clear() { off.assign(1, 0); adj.clear(); }
    void end_vertex() { off.push_back(u32(adj.size())); }
};

// Chiba and Nishizeki: each 4-cycle once, at its vertex of top rank, by pairing the wedges to its opposite vertex.
class Count4 {
public:
    explicit Count4(u32 n) : rank_(n, 0), cnt_(n, 0), order_(n, 0) {}
    i64 count(const Sub& s, u32 A, u64& ops) {
        u32 maxd = 0;
        for (u32 v = 0; v < A; ++v) maxd = std::max(maxd, s.deg(v));
        bin_.assign(maxd + 2, 0);
        for (u32 v = 0; v < A; ++v) ++bin_[s.deg(v)];
        u32 st = 0;
        for (u32 d = maxd + 1; d-- > 0;) { const u32 t = bin_[d]; bin_[d] = st; st += t; }
        for (u32 v = 0; v < A; ++v) { const u32 r = bin_[s.deg(v)]++; rank_[v] = r; order_[r] = v; }
        ops += 7ull * A + 3ull * (maxd + 2);
        i64 total = 0;
        for (u32 r = 0; r < A; ++r) {
            const u32 v = order_[r];
            ops += 2;
            for (u32 a : s.nbr(v)) {
                ops += 2;
                if (rank_[a] <= r) continue;
                for (u32 w : s.nbr(a)) {
                    ops += 2;
                    if (rank_[w] <= r && MUT != 33) continue;
                    if (w == v) continue;
                    total += cnt_[w];
                    if (cnt_[w]++ == 0) touched_.push_back(w);
                    ops += 3;
                }
            }
            ops += touched_.size();
            for (u32 w : touched_) cnt_[w] = 0;
            touched_.clear();
        }
        return total;
    }

private:
    std::vector<u32> rank_, cnt_, order_, bin_, touched_;
};

// Marks that need no reset, as Row does, for the extraction and the port.
struct Tags {
    std::vector<u64> t;
    u32 ep = 0;
    explicit Tags(u32 n) : t(n, 0) {}
    void next() { ++ep; }
    bool on(u32 v) const { return u32(t[v] >> 32) == ep; }
    u32 val(u32 v) const { return u32(t[v]); }
    void set(u32 v, u32 x) { t[v] = (u64(ep) << 32) | x; }
};

// The edits of a batch by vertex: one flat store, read through a first-index tag.
class EditIndex {
public:
    explicit EditIndex(u32 n) : first_(n) {}
    struct Rec { u32 v, w, kind; };   // kind 1 inserted, 2 deleted
    void start() { rec_.clear(); }
    void add(const Edge& e, u32 kind) { rec_.push_back({e.a, e.b, kind}); rec_.push_back({e.b, e.a, kind}); }
    void finish(Cost& c) {
        std::sort(rec_.begin(), rec_.end(), [](const Rec& x, const Rec& y) { return x.v != y.v ? x.v < y.v : x.w < y.w; });
        c.row += 2 * rec_.size();
        first_.next();
        verts_.clear();
        for (u32 i = 0; i < u32(rec_.size()); ++i)
            if (i == 0 || rec_[i - 1].v != rec_[i].v) { first_.set(rec_[i].v, i); verts_.push_back(rec_[i].v); }
    }
    bool has(u32 v) const { return first_.on(v); }
    const std::vector<u32>& verts() const { return verts_; }
    template <class F> void each(u32 v, F f) const {
        for (u32 i = first_.val(v); i < rec_.size() && rec_[i].v == v; ++i) f(rec_[i].w, rec_[i].kind);
    }

private:
    Tags first_;
    std::vector<Rec> rec_;
    std::vector<u32> verts_;
};

class Local {
public:
    Local(u32 n, int p) : p_(p), ball_(n), kind_(n), ei_(n), rc_(p == 5 ? n : 0), c4_(p == 4 ? n : 0) {}

    // Applies the batch to g and returns the change in the count.
    i64 run(Graph& g, const NetBatch& nb, Cost& c) {
        if (nb.ins.empty() && nb.del.empty()) return 0;
        ei_.start();
        for (const Edge& e : nb.del) ei_.add(e, 2);
        for (const Edge& e : nb.ins) ei_.add(e, 1);
        ei_.finish(c);
        for (const Edge& e : nb.ins) { g.insert(e.a, e.b); c.graph += 2; }
        ball_.next();
        a_.clear();
        for (u32 v : ei_.verts()) { ball_.set(v, u32(a_.size())); a_.push_back(v); c.row += 2; }
        size_t from = 0;
        const int r = (p_ == 5 ? 2 : 1) - (MUT == 31 ? 1 : 0);
        for (int depth = 0; depth < r; ++depth) {
            const size_t to = a_.size();
            for (size_t i = from; i < to; ++i) {
                const std::vector<u32>& l = g.nbr(a_[i]);
                c.scan += l.size();
                c.row += l.size();
                for (u32 w : l)
                    if (!ball_.on(w)) { ball_.set(w, u32(a_.size())); a_.push_back(w); c.row += 2; }
            }
            from = to;
        }
        const u32 A = u32(a_.size());
        old_.clear();
        new_.clear();
        for (u32 i = 0; i < A; ++i) {
            const u32 v = a_[i];
            const bool edited = ei_.has(v);
            if (edited) {
                kind_.next();
                ei_.each(v, [&](u32 w, u32 k) { kind_.set(w, k); c.row += 2; });
            }
            const std::vector<u32>& l = g.nbr(v);
            c.scan += l.size();
            c.row += l.size();
            for (u32 w : l) {
                if (!ball_.on(w)) continue;
                const u32 k = edited && kind_.on(w) ? kind_.val(w) : 0;
                const u32 lw = ball_.val(w);
                if (k != 1) { old_.adj.push_back(lw); c.row += 1; }
                if (k != 2 || MUT == 32) { new_.adj.push_back(lw); c.row += 1; }
            }
            old_.end_vertex();
            new_.end_vertex();
        }
        for (const Edge& e : nb.del) { g.erase(e.a, e.b); c.graph += 2; }
        u64 ops = 0;
        i64 before = 0, after = 0;
        if (p_ == 5) {
            act_.resize(A);
            for (u32 i = 0; i < A; ++i) act_[i] = i;
            u64 o = 0;
            rc_.count(old_, act_, ~u64(0), before, o);
            ops += o;
            rc_.count(new_, act_, ~u64(0), after, o);
            ops += o;
        } else {
            before = c4_.count(old_, A, ops);
            after = c4_.count(new_, A, ops);
        }
        c.rc += ops;
        return after - before;
    }

private:
    int p_;
    Tags ball_, kind_;
    EditIndex ei_;
    Recount rc_;
    Count4 c4_;
    std::vector<u32> a_, act_;
    Sub old_, new_;
};

class Port4 {
public:
    explicit Port4(u32 n) : nw_(n), df_(n), x_(n), ei_(n) {}

    // c4(G1) - c4(G0), with g at G1 and es the edits.
    i64 pass(const Graph& g, const std::vector<Edge>& es, u32 kind, Cost& c) {
        ei_.start();
        for (const Edge& e : es) ei_.add(e, kind);
        ei_.finish(c);
        i64 twice = 0;
        for (u32 x : ei_.verts()) {
            nw_.next();
            df_.next();
            x_.next();
            seen_.clear();
            ei_.each(x, [&](u32 w, u32) { x_.set(w, 1); c.row += 2; });
            const std::vector<u32>& nx = g.nbr(x);
            for (size_t i = 0; i < nx.size(); ++i) {
                prefetch_lists(g, nx.data(), i, nx.size());
                const u32 a = nx[i];
                ++c.scan;
                c.scan += g.deg(a);
                ++c.row;
                const bool mine = x_.on(a);
                if (mine) {
                    for (u32 w : g.nbr(a))
                        if (w != x) { nw_.add(w, 1); note(w); c.row += 4; }
                } else {
                    for (u32 w : g.nbr(a))
                        if (w != x) { nw_.add(w, 1); c.row += 2; }
                    if (MUT != 30 && ei_.has(a))
                        ei_.each(a, [&](u32 w, u32) { if (w != x) { note(w); c.row += 2; } ++c.scan; });
                }
            }
            for (u32 w : seen_) {
                ++c.scan;
                ++c.row;
                if (MUT != 29 && ei_.has(w) && w < x) continue;
                const i64 k = nw_.get(w), d = df_.get(w);
                c.row += 2;
                twice += k * (k - 1) / 2 - (k - d) * (k - d - 1) / 2;
            }
        }
        return twice / 2;
    }

private:
    void note(u32 w) {
        if (df_.get(w) == 0) seen_.push_back(w);
        df_.add(w, 1);
    }
    Row nw_, df_;
    Tags x_;
    EditIndex ei_;
    std::vector<u32> seen_;
};

enum class Kind { Legacy, Scan, Star, Table, Recount, Port };

inline bool table_arm(const std::string& a) { return a == "T3" || a == "T4"; }

// One arm with its own graph, so that the oracle and the sweep run the same code.
class Counter {
public:
    const int p;
    const std::string arm;
    i64 t = 0;
    u64 groups = 0, spokes = 0;

    Counter(int pattern, const std::string& name, u32 n, u32 theta, u32 count_bits = 0)
        : p(pattern), arm(name), kind_(kind_of(name)), own_(kind_ == Kind::Legacy ? 0 : n), row_(kind_ == Kind::Scan || kind_ == Kind::Star ? n : 0),
          grouper_(kind_ == Kind::Star ? n : 0) {
        if (kind_ == Kind::Legacy) {
            Arm a = Arm::R2;
            for (Arm x : {Arm::R2, Arm::PR, Arm::WJ, Arm::WT})
                if (name == arm_name(x)) a = x;
            e5_ = std::make_unique<Engine5>(a, n, count_bits);
        }
        if (kind_ == Kind::Table) mix_ = std::make_unique<Mixed>(n, table_arm(name) ? Mixed::kNoHubs : theta, count_bits);
        if (kind_ == Kind::Recount) local_ = std::make_unique<Local>(n, p);
        if (kind_ == Kind::Port) port_ = std::make_unique<Port4>(n);
    }

    static bool valid(int p, const std::string& a) {
        if (p == 3) return a == "S3" || a == "P3" || a == "T3" || a == "M3";
        if (p == 4) return a == "S4" || a == "P4" || a == "T4" || a == "M4" || a == "B4" || a == "A4";
        return p == 5 && (a == "R2" || a == "PR" || a == "WT" || a == "WJ" || a == "M5" || a == "A5");
    }

    const Graph& graph() const { return e5_ ? e5_->g : own_; }
    const Cost& cost() const { return e5_ ? e5_->c : c_; }
    i64 count() const { return e5_ ? e5_->t : t; }
    u64 table_bytes() const {
        if (e5_) return e5_->table_bytes();
        return mix_ ? mix_->wt.peak_bytes() : 0;
    }
    bool capped() const {
        if (e5_) return e5_->table_capped();
        return mix_ && mix_->wt.capped;
    }
    // The tables carry their own limit, 6 GB as M1C03 left it, below the job limit. Gate 1 raises both.
    void set_table_cap(u64 bytes) {
        if (e5_) { e5_->wedge.cap_bytes = bytes; e5_->wt.cap_bytes = bytes; }
        if (mix_) mix_->wt.cap_bytes = bytes;
    }
    u64 heavy() const { return mix_ ? mix_->heavy_now() : 0; }
    u64 saturated() const { return mix_ ? mix_->wt.peak_saturated() : 0; }
    u64 turns() const { return mix_ ? mix_->turns() : 0; }

    u64 apply(const std::vector<Update>& b, const NetBatch& nb) {
        if (e5_) return e5_->apply(b, nb, ~u64(0));
        const u64 before = c_.total();
        const u64 net = nb.del.size() + nb.ins.size();
        c_.net += 2 * b.size();
        if (kind_ == Kind::Star) {
            c_.group += 3 * net;
            for (int pass = 0; pass < 2; ++pass) {
                const bool ins = pass == 1;
                for (const GroupV& gr : grouper_.run(ins ? nb.ins : nb.del)) {
                    if (gr.c.size() > 1) { ++groups; spokes += gr.c.size(); }
                    t += p == 3 ? star3(own_, gr, ins, row_, c_) : star4(own_, gr, ins, row_, c_);
                }
            }
        } else if (kind_ == Kind::Recount) {
            t += local_->run(own_, nb, c_);
        } else if (kind_ == Kind::Port) {
            if (!nb.del.empty()) t -= port_->pass(own_, nb.del, 2, c_);
            for (const Edge& e : nb.del) edit(own_, e.a, e.b, false, c_);
            for (const Edge& e : nb.ins) edit(own_, e.a, e.b, true, c_);
            if (!nb.ins.empty()) t += port_->pass(own_, nb.ins, 1, c_);
        } else {
            for (const Edge& e : nb.del) per_edge(e, false);
            for (const Edge& e : nb.ins) per_edge(e, true);
        }
        return c_.total() - before;
    }

private:
    static Kind kind_of(const std::string& a) {
        if (a == "S3" || a == "S4") return Kind::Scan;
        if (a == "P3" || a == "P4") return Kind::Star;
        if (a == "T3" || a == "T4" || a == "M3" || a == "M4" || a == "M5") return Kind::Table;
        if (a == "A4" || a == "A5") return Kind::Recount;
        if (a == "B4") return Kind::Port;
        return Kind::Legacy;
    }

    void per_edge(const Edge& e, bool ins) {
        i64 d = 0;
        if (kind_ == Kind::Scan) {
            d = p == 3 ? through3(own_, e.a, e.b, c_) : through4(own_, e.a, e.b, row_, c_);
            edit(own_, e.a, e.b, ins, c_);
        } else if (ins) {
            d = mix_->through(p, own_, e.a, e.b, c_);
            mix_->upkeep(own_, e.a, e.b, true, c_);
            edit(own_, e.a, e.b, true, c_);
            mix_->reclass(own_, e.a, c_);
            mix_->reclass(own_, e.b, c_);
        } else {
            edit(own_, e.a, e.b, false, c_);
            mix_->upkeep(own_, e.a, e.b, false, c_);
            d = mix_->through(p, own_, e.a, e.b, c_);
            mix_->reclass(own_, e.a, c_);
            mix_->reclass(own_, e.b, c_);
        }
        t += ins ? d : -d;
    }

    Kind kind_;
    Graph own_;
    Cost c_;
    Row row_;
    Grouper grouper_;
    std::unique_ptr<Engine5> e5_;
    std::unique_ptr<Mixed> mix_;
    std::unique_ptr<Local> local_;
    std::unique_ptr<Port4> port_;
};

}  // namespace g1

#ifndef GATE1_NO_MAIN

using namespace g1;

namespace {

#pragma pack(push, 1)
struct Event { std::int64_t t; u32 u, v, op; };
#pragma pack(pop)

u64 peak_rss() {
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof pmc)) return u64(pmc.PeakWorkingSetSize);
    return 0;
}

struct BatchRow { u64 raw = 0, net = 0, ops = 0; i64 t = 0; };

struct Run {
    std::string stream, variant, base, arm, raw_path, sum_path;
    i64 cut = 0;
    int pattern = 0, rep = 0, cpu = -1;
    u32 n = 0, theta = 0;
    u64 events = 0, net_total = 0, peak_m = 0;
    size_t batches = 0;
    double cap_seconds = 600.0;
    std::vector<BatchRow> rows;
    std::atomic<size_t> done{0};
    std::atomic<int> writing{0};
    Counter* s = nullptr;
    std::chrono::steady_clock::time_point t0;
    double seconds() const { return std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count(); }
};

Run R;

bool write_outputs(const char* status, double secs) {
    if (R.writing.exchange(1)) return false;
    const size_t done = R.done.load(std::memory_order_acquire);
    FILE* out = std::fopen(R.raw_path.c_str(), "w");
    if (!out) { std::puts("cannot open raw output"); return false; }
    std::fprintf(out, "batch,raw,net,ops,t\n");
    for (size_t i = 0; i < done; ++i) {
        const BatchRow& r = R.rows[i];
        std::fprintf(out, "%zu,%llu,%llu,%llu,%lld\n", i, (unsigned long long)r.raw, (unsigned long long)r.net, (unsigned long long)r.ops, (long long)r.t);
    }
    std::fclose(out);
    FILE* f = std::fopen(R.sum_path.c_str(), "w");
    if (!f) { std::puts("cannot open summary"); return false; }
    const Counter& s = *R.s;
    const Cost& c = s.cost();
#ifdef FC_TIMING
    const char* build = "timing";
#else
    const char* build = "count";
#endif
    std::fprintf(f, "stream,variant,cut,pattern,arm,theta,status,batches_done,batches,events,net,ops,scan,probe,row,deg,tab_read,tab_write,graph,net_ops,group,rc,upkeep,"
                    "peak_m,final_t,table_bytes,heavy,turns,groups,spokes,peak_rss,seconds,vertices,build,rep,cpu\n");
    std::fprintf(f, "%s,%s,%lld,%d,%s,%u,%s,%zu,%zu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%lld,%llu,%llu,%llu,%llu,%llu,%llu,%.3f,%u,%s,%d,%d\n",
                 R.stream.c_str(), R.variant.c_str(), (long long)R.cut, R.pattern, R.arm.c_str(), R.theta, status, done, R.batches, (unsigned long long)R.events,
                 (unsigned long long)R.net_total, (unsigned long long)c.total(), (unsigned long long)c.scan, (unsigned long long)c.probe, (unsigned long long)c.row,
                 (unsigned long long)c.deg, (unsigned long long)c.tab_read, (unsigned long long)c.tab_write, (unsigned long long)c.graph, (unsigned long long)c.net,
                 (unsigned long long)c.group, (unsigned long long)c.rc, (unsigned long long)c.upkeep, (unsigned long long)R.peak_m, (long long)(done ? R.rows[done - 1].t : 0),
                 (unsigned long long)s.table_bytes(), (unsigned long long)s.heavy(), (unsigned long long)s.turns(), (unsigned long long)s.groups, (unsigned long long)s.spokes,
                 (unsigned long long)peak_rss(), secs, R.n, build, R.rep, R.cpu);
    std::fclose(f);
    std::printf("%s p%d %s status=%s done=%zu/%zu net=%llu ops=%llu t=%lld table=%llu secs=%.3f\n", R.base.c_str(), R.pattern, R.arm.c_str(), status, done, R.batches,
                (unsigned long long)R.net_total, (unsigned long long)c.total(), (long long)(done ? R.rows[done - 1].t : 0), (unsigned long long)s.table_bytes(), secs);
    std::fflush(stdout);
    return true;
}

DWORD WINAPI watchdog(void*) {
    Sleep(DWORD(R.cap_seconds * 1000.0));
    if (R.s && write_outputs("TIME", R.seconds())) { std::fflush(nullptr); _exit(3); }
    if (!R.s) { std::puts("time limit before the engine started"); std::fflush(nullptr); _exit(3); }
    return 0;
}

bool limit_memory(u64 bytes) {
    HANDLE job = CreateJobObjectW(nullptr, nullptr);
    if (!job) return false;
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION li{};
    li.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_PROCESS_MEMORY;
    li.ProcessMemoryLimit = SIZE_T(bytes);
    if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &li, sizeof li)) return false;
    return AssignProcessToJobObject(job, GetCurrentProcess()) != 0;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 8) {
        std::puts("usage: r11_gate1 events.bin stream insert|expiry cut 3|4|5 arm outdir [--theta T] [--cap-seconds S] [--cap-bytes B] [--cpu N] [--rep N]");
        return 2;
    }
    const std::string path = argv[1], outdir = argv[7];
    R.stream = argv[2];
    R.variant = argv[3];
    R.cut = std::atoll(argv[4]);
    R.pattern = std::atoi(argv[5]);
    R.arm = argv[6];
    if (!Counter::valid(R.pattern, R.arm)) { std::puts("unknown arm for this pattern"); return 2; }
    u64 cap_bytes = 6000000000ull;
    for (int i = 8; i + 1 < argc; i += 2) {
        const std::string a = argv[i];
        if (a == "--cap-seconds") R.cap_seconds = std::atof(argv[i + 1]);
        else if (a == "--cap-bytes") cap_bytes = std::strtoull(argv[i + 1], nullptr, 10);
        else if (a == "--cpu") R.cpu = std::atoi(argv[i + 1]);
        else if (a == "--rep") R.rep = std::atoi(argv[i + 1]);
        else if (a == "--theta") R.theta = u32(std::strtoul(argv[i + 1], nullptr, 10));
        else { std::puts("unknown option"); return 2; }
    }
    if (R.arm[0] == 'M' && R.theta == 0) { std::puts("a mixed arm needs --theta"); return 2; }
    if (!limit_memory(cap_bytes)) { std::puts("cannot set the memory limit"); return 2; }
    if (R.cpu >= 0 && !SetProcessAffinityMask(GetCurrentProcess(), DWORD_PTR(1) << R.cpu)) { std::puts("cannot set affinity"); return 2; }
    HANDLE dog = CreateThread(nullptr, 0, watchdog, nullptr, 0, nullptr);
    if (!dog) { std::puts("cannot start the time limit"); return 2; }

    FILE* in = std::fopen(path.c_str(), "rb");
    if (!in) { std::puts("cannot open events"); return 2; }
    std::fseek(in, 0, SEEK_END);
    const long long bytes = _ftelli64(in);
    std::fseek(in, 0, SEEK_SET);
    std::vector<Event> ev(size_t(bytes) / sizeof(Event));
    if (std::fread(ev.data(), sizeof(Event), ev.size(), in) != ev.size()) { std::puts("short read"); return 2; }
    std::fclose(in);
    R.events = ev.size();
    for (const Event& x : ev) R.n = std::max({R.n, x.u + 1, x.v + 1});

    std::vector<size_t> starts{0};
    for (size_t i = 1; i < ev.size(); ++i)
        if (i % size_t(R.cut) == 0) starts.push_back(i);
    starts.push_back(ev.size());
    R.batches = starts.size() - 1;
    R.rows.resize(R.batches);

    R.base = R.stream + "-" + R.variant + "-" + std::to_string(R.cut);
#ifdef FC_TIMING
    const std::string suffix = ".t" + std::to_string(R.rep);
#else
    const std::string suffix;
#endif
    const std::string tag = R.base + "-p" + std::to_string(R.pattern) + "-" + R.arm + (R.theta ? "-th" + std::to_string(R.theta) : "") + suffix;
    R.raw_path = outdir + "/raw/" + tag + ".csv";
    R.sum_path = outdir + "/raw/" + tag + ".summary.csv";

    auto engine = std::make_unique<Counter>(R.pattern, R.arm, R.n, R.theta);
    Counter& s = *engine;
    s.set_table_cap(cap_bytes);
    R.s = engine.get();
    Normaliser norm;
    std::vector<Update> b;
    const char* status = "OK";
    R.t0 = std::chrono::steady_clock::now();
    try {
        for (size_t bi = 0; bi < R.batches; ++bi) {
            b.clear();
            for (size_t i = starts[bi]; i < starts[bi + 1]; ++i) b.push_back({ev[i].u, ev[i].v, ev[i].op == 1});
            const NetBatch& nb = norm.run(s.graph(), b);
            const u64 net = nb.ins.size() + nb.del.size();
            R.net_total += net;
            const u64 ops = s.apply(b, nb);
            BatchRow& row = R.rows[bi];
            R.peak_m = std::max(R.peak_m, s.graph().m());
            row.raw = b.size();
            row.net = net;
            row.ops = ops;
            row.t = s.count();
            R.done.store(bi + 1, std::memory_order_release);
            if (s.capped()) { status = "CAP"; break; }
        }
    } catch (const std::bad_alloc&) {
        status = "MEM";
    }
    const double secs = R.seconds();
    if (!write_outputs(status, secs)) Sleep(5000);
    return status[0] == 'O' ? 0 : 3;
}

#endif  // GATE1_NO_MAIN
