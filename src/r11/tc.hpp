// Exact batch triangle maintenance by peeling star groups. See docs/proposal.md section 3.
// Prototype for operation counting. It is not tuned for wall-clock time.
#pragma once
#include <algorithm>
#include <cstdint>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace r11 {

using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i64 = std::int64_t;

struct Edge { u32 a, b; };                  // a < b
struct Update { u32 u, v; bool ins; };      // one element of an input batch
struct NetBatch { std::vector<Edge> del, ins; };

inline Edge canon(u32 u, u32 v) { return u < v ? Edge{u, v} : Edge{v, u}; }
inline u64 key(Edge e) { return (u64(e.a) << 32) | e.b; }

// Unit-cost operation counter. One scan is one list element read, one probe is one
// adjacency test, one mark is one read or write of a stamp array.
struct Ops {
    u64 scan = 0, probe = 0, mark = 0;
    u64 total() const { return scan + probe + mark; }
};

// Open-addressing map from an edge key to a value, linear probing, backward-shift deletion.
template <class V>
class FlatMap {
public:
    FlatMap() { rehash(1u << 10); }
    u64 size() const { return size_; }
    V* find(u64 k) {
        for (u64 i = slot(k);; i = (i + 1) & mask_) {
            if (keys_[i] == kEmpty) return nullptr;
            if (keys_[i] == k) return &vals_[i];
        }
    }
    const V* find(u64 k) const { return const_cast<FlatMap*>(this)->find(k); }
    void emplace(u64 k, V v) {
        if ((size_ + 1) * 10 > (mask_ + 1) * 7) rehash((mask_ + 1) * 2);
        u64 i = slot(k);
        while (keys_[i] != kEmpty) i = (i + 1) & mask_;
        keys_[i] = k;
        vals_[i] = v;
        ++size_;
    }
    void erase(u64 k) {
        u64 i = slot(k);
        while (keys_[i] != k) i = (i + 1) & mask_;
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
    template <class F> void for_each(F f) {
        for (u64 i = 0; i <= mask_; ++i)
            if (keys_[i] != kEmpty) f(keys_[i], vals_[i]);
    }

private:
    static constexpr u64 kEmpty = ~u64(0);
    u64 slot(u64 k) const { return ((k ^ (k >> 29)) * 0x9E3779B97F4A7C15ull) >> shift_; }
    void rehash(u64 cap) {
        std::vector<u64> ok = std::move(keys_);
        std::vector<V> ov = std::move(vals_);
        keys_.assign(cap, kEmpty);
        vals_.resize(cap);
        mask_ = cap - 1;
        shift_ = 64;
        for (u64 c = cap; c > 1; c >>= 1) --shift_;
        size_ = 0;
        for (size_t i = 0; i < ok.size(); ++i)
            if (ok[i] != kEmpty) emplace(ok[i], ov[i]);
    }
    std::vector<u64> keys_;
    std::vector<V> vals_;
    u64 mask_ = 0, size_ = 0;
    int shift_ = 64;
};

class Graph {
public:
    explicit Graph(u32 n) : adj_(n), out_(n), rank_(n) {
        for (u32 v = 0; v < n; ++v) rank_[v] = v;
    }
    u32 n() const { return u32(adj_.size()); }
    u64 m() const { return pos_.size(); }
    u64 active() const { return active_; }      // vertices with at least one edge
    u32 deg(u32 v) const { return u32(adj_[v].size()); }
    u32 outdeg(u32 v) const { return u32(out_[v].size()); }
    const std::vector<u32>& nbr(u32 v) const { return adj_[v]; }
    const std::vector<u32>& out(u32 v) const { return out_[v]; }
    i64 rank(u32 v) const { return rank_[v]; }
    bool has(u32 u, u32 v) const { return pos_.find(key(canon(u, v))) != nullptr; }

    void insert(u32 u, u32 v) {
        Edge e = canon(u, v);
        Pos p{u32(adj_[e.a].size()), u32(adj_[e.b].size()), 0};
        active_ += (p.a == 0) + (p.b == 0);
        adj_[e.a].push_back(e.b);
        adj_[e.b].push_back(e.a);
        u32 lo = low(e), hi = lo == e.a ? e.b : e.a;
        p.o = u32(out_[lo].size());
        out_[lo].push_back(hi);
        pos_.emplace(key(e), p);
    }

    void erase(u32 u, u32 v) {
        Edge e = canon(u, v);
        Pos p = *pos_.find(key(e));
        u32 lo = low(e);
        pos_.erase(key(e));
        swap_out_adj(e.a, p.a);
        swap_out_adj(e.b, p.b);
        swap_out_out(lo, p.o);
        active_ -= (adj_[e.a].empty() ? 1 : 0) + (adj_[e.b].empty() ? 1 : 0);
    }

    // Replace the vertex order. Exactness never depends on the order, only speed does.
    void set_ranks(const std::vector<i64>& r) {
        rank_ = r;
        for (auto& l : out_) l.clear();
        pos_.for_each([&](u64 k, Pos& p) {
            Edge e{u32(k >> 32), u32(k)};
            u32 lo = low(e), hi = lo == e.a ? e.b : e.a;
            p.o = u32(out_[lo].size());
            out_[lo].push_back(hi);
        });
    }

private:
    struct Pos { u32 a, b, o; };

    u32 low(Edge e) const {
        i64 ra = rank_[e.a], rb = rank_[e.b];
        return (ra < rb || (ra == rb && e.a < e.b)) ? e.a : e.b;
    }
    void swap_out_adj(u32 v, u32 i) {
        u32 w = adj_[v].back();
        adj_[v][i] = w;
        adj_[v].pop_back();
        if (i < adj_[v].size()) {
            Pos* q = pos_.find(key(canon(v, w)));
            if (v < w) q->a = i; else q->b = i;
        }
    }
    void swap_out_out(u32 v, u32 i) {
        u32 w = out_[v].back();
        out_[v][i] = w;
        out_[v].pop_back();
        if (i < out_[v].size()) pos_.find(key(canon(v, w)))->o = i;
    }

    std::vector<std::vector<u32>> adj_, out_;
    std::vector<i64> rank_;
    FlatMap<Pos> pos_;
    u64 active_ = 0;
};

// Reference counters. Neither shares a code path with the batch engine.
inline i64 brute_triangles(const Graph& g) {
    i64 t = 0;
    for (u32 a = 0; a < g.n(); ++a)
        for (u32 b = a + 1; b < g.n(); ++b)
            if (g.has(a, b))
                for (u32 c = b + 1; c < g.n(); ++c)
                    if (g.has(a, c) && g.has(b, c)) ++t;
    return t;
}

// Static compact-forward on a fresh degree order. Returns the count and its operations,
// which include rebuilding the oriented lists from scratch.
inline std::pair<i64, u64> static_count(const Graph& g) {
    u32 n = g.n();
    std::vector<u32> ord(n), rk(n);
    std::iota(ord.begin(), ord.end(), 0u);
    std::sort(ord.begin(), ord.end(), [&](u32 x, u32 y) {
        return g.deg(x) != g.deg(y) ? g.deg(x) < g.deg(y) : x < y;
    });
    for (u32 i = 0; i < n; ++i) rk[ord[i]] = i;
    std::vector<std::vector<u32>> o(n);
    u64 ops = n;
    for (u32 v = 0; v < n; ++v)
        for (u32 w : g.nbr(v)) {
            ++ops;
            if (rk[v] < rk[w]) o[v].push_back(w);
        }
    std::vector<u32> stamp(n, 0xffffffffu);
    i64 t = 0;
    for (u32 y = 0; y < n; ++y) {
        for (u32 z : o[y]) { stamp[z] = y; ++ops; }
        for (u32 z : o[y])
            for (u32 w : o[z]) {
                ops += 2;
                if (stamp[w] == y) ++t;
            }
    }
    return {t, ops};
}

// Reduce an input batch to its net effect under set semantics, last write wins.
inline NetBatch normalise(const Graph& g, const std::vector<Update>& b) {
    std::unordered_set<u64> seen;
    NetBatch out;
    for (auto it = b.rbegin(); it != b.rend(); ++it) {
        if (it->u == it->v) continue;
        Edge e = canon(it->u, it->v);
        if (!seen.insert(key(e)).second) continue;
        bool present = g.has(e.a, e.b);
        if (it->ins && !present) out.ins.push_back(e);
        if (!it->ins && present) out.del.push_back(e);
    }
    return out;
}

struct Group { u32 x; std::vector<u32> c; };

enum class Pivot { Single, LowDegree, BatchDegree };
enum class KernelMode { EdgeOnly, ReverseOnly, OrientedOnly, AdaptiveNoOrient, Adaptive };

inline std::vector<Group> make_groups(const Graph& g, const std::vector<Edge>& es, Pivot mode) {
    std::vector<Group> gs;
    if (mode == Pivot::Single) {
        gs.reserve(es.size());
        for (Edge e : es) gs.push_back(Group{e.a, {e.b}});
        return gs;
    }
    std::unordered_map<u32, u32> bd;
    if (mode == Pivot::BatchDegree)
        for (Edge e : es) { ++bd[e.a]; ++bd[e.b]; }
    auto prefer = [&](u32 p, u32 q) {   // true when p is the better pivot
        if (mode == Pivot::BatchDegree) {
            u32 bp = bd[p], bq = bd[q];
            if (bp != bq) return bp > bq;
        } else if (g.deg(p) != g.deg(q)) {
            return g.deg(p) < g.deg(q);
        }
        return p < q;
    };
    std::unordered_map<u32, u32> at;
    for (Edge e : es) {
        u32 x = prefer(e.a, e.b) ? e.a : e.b, v = x == e.a ? e.b : e.a;
        auto f = at.find(x);
        if (f == at.end()) {
            at.emplace(x, u32(gs.size()));
            gs.push_back(Group{x, {v}});
        } else {
            gs[f->second].c.push_back(v);
        }
    }
    return gs;
}

// Peeling engine. For deletions the working graph goes from G_old down to U; for
// insertions from U up to G_new. Each group is counted and applied as one step.
class Engine {
public:
    explicit Engine(u32 n) : in_n_(n, 0), in_c_(n, 0) {}

    u64 kernel_a = 0, kernel_b = 0, kernel_c = 0;   // groups run per kernel
    u64 star_whole = 0, star_spokes = 0;            // kernel O groups with R empty; spokes in kernel O groups
    bool check_bound = false;                       // verify real cost <= estimate

    i64 apply(Graph& g, const NetBatch& nb, Pivot pm, KernelMode km, Ops& ops) {
        i64 delta = 0;
        for (const Group& gr : make_groups(g, nb.del, pm)) delta -= step(g, gr, false, km, ops);
        for (const Group& gr : make_groups(g, nb.ins, pm)) delta += step(g, gr, true, km, ops);
        return delta;
    }

private:
    enum class K { A, B, C };

    static u64 common(const Graph& g, u32 u, u32 v, Ops& ops) {
        u32 s = g.deg(u) <= g.deg(v) ? u : v, t = s == u ? v : u;
        u64 c = 0;
        for (u32 w : g.nbr(s)) {
            ++ops.scan; ++ops.probe;
            if (g.has(w, t)) ++c;
        }
        return c;
    }

    void stamp_group(const Graph& g, const Group& gr, Ops& ops, bool all_n) {
        ++epoch_;
        for (u32 v : gr.c) { in_c_[v] = epoch_; ++ops.mark; }
        if (all_n)
            for (u32 w : g.nbr(gr.x)) { in_n_[w] = epoch_; ++ops.mark; }
    }

    // Counts triangles through x with at least one spoke in C. Every adjacent pair
    // {a, b} inside N(x) with a or b in C is one triangle.
    u64 reverse_kernel(const Graph& g, const Group& gr, Ops& ops) {
        stamp_group(g, gr, ops, false);
        u64 k = gr.c.size(), cross = 0, inside = 0;
        for (u32 w : g.nbr(gr.x)) {
            u64 hits = 0;
            if (g.deg(w) <= k) {
                for (u32 z : g.nbr(w)) {
                    ++ops.scan; ++ops.mark;
                    if (in_c_[z] == epoch_) ++hits;
                }
            } else {
                for (u32 v : gr.c) {
                    ++ops.scan; ++ops.probe;
                    if (v != w && g.has(w, v)) ++hits;
                }
            }
            if (in_c_[w] == epoch_) inside += hits; else cross += hits;
        }
        return cross + inside / 2;
    }

    u64 oriented_kernel(const Graph& g, const Group& gr, Ops& ops) {
        stamp_group(g, gr, ops, true);
        u64 k = gr.c.size(), l = 0;
        for (u32 y : g.nbr(gr.x)) {
            bool yc = in_c_[y] == epoch_;
            if (yc || g.outdeg(y) <= k) {
                for (u32 z : g.out(y)) {
                    ++ops.scan; ++ops.mark;
                    if (in_n_[z] == epoch_ && (yc || in_c_[z] == epoch_)) ++l;
                }
            } else {
                for (u32 v : gr.c) {
                    ++ops.scan; ++ops.probe;
                    if (v != y && g.has(y, v) && before(g, y, v)) ++l;
                }
            }
        }
        return l;
    }

    static bool before(const Graph& g, u32 p, u32 q) {
        i64 a = g.rank(p), b = g.rank(q);
        return a < b || (a == b && p < q);
    }

    u64 estimate_a(const Graph& g, const Group& gr, bool ins) const {
        u64 d = g.deg(gr.x), c = 0;
        for (size_t i = 0; i < gr.c.size(); ++i) {
            u64 dv = g.deg(gr.c[i]);
            c += ins ? 2 * std::min(d + i + 1, dv + 1) : 2 * std::min(d - i, dv);
        }
        return c;
    }

    i64 step(Graph& g, const Group& gr, bool ins, KernelMode km, Ops& ops) {
        K k = choose(g, gr, ins, km, ops);
        u64 before_ops = ops.total(), est = est_chosen_, l = 0;
        if (k == K::A) {
            ++kernel_a;
            for (u32 v : gr.c) {
                if (ins) g.insert(gr.x, v);
                l += common(g, gr.x, v, ops);
                if (!ins) g.erase(gr.x, v);
            }
        } else {
            if (ins) for (u32 v : gr.c) g.insert(gr.x, v);
            if (k == K::B) { ++kernel_b; l = reverse_kernel(g, gr, ops); }
            else {
                ++kernel_c;
                star_spokes += gr.c.size();
                star_whole += g.deg(gr.x) == gr.c.size();
                l = oriented_kernel(g, gr, ops);
            }
            if (!ins) for (u32 v : gr.c) g.erase(gr.x, v);
        }
        if (check_bound && km == KernelMode::Adaptive && ops.total() - before_ops > est) ++bound_violations;
        return i64(l);
    }

    K choose(const Graph& g, const Group& gr, bool ins, KernelMode km, Ops& ops) {
        est_chosen_ = ~u64(0);
        if (km == KernelMode::EdgeOnly) return K::A;
        if (km == KernelMode::ReverseOnly) return K::B;
        if (km == KernelMode::OrientedOnly) return K::C;
        u64 k = gr.c.size(), ca = estimate_a(g, gr, ins), d = g.deg(gr.x) + (ins ? k : 0);
        est_chosen_ = ca;
        if (k == 1) return K::A;        // one spoke never repays marking N(x)
        ops.mark += k;                  // reading the degrees of C
        // The floor adds half the expected R term, taken from the mean out-degree. It only
        // ever sends more groups to kernel A, so the overhead bound is unaffected.
        u64 floor_b = k + 2 * d, floor_c = d + k + (d - k) * std::min<u64>(g.m() / std::max<u64>(g.active(), 1), k);
        for (u32 v : gr.c) floor_c += 2 * g.outdeg(v);
        if (ca <= (km == KernelMode::AdaptiveNoOrient ? floor_b : floor_c)) return K::A;
        ops.mark += d + k;              // stamping C, then reading degrees in N(x)
        u64 cb = k, cc = d + k;
        ++epoch_;
        for (u32 v : gr.c) in_c_[v] = epoch_;
        auto cost = [&](u32 y, bool yc) {
            u64 dy = g.deg(y) + (ins && yc ? 1 : 0), oy = g.outdeg(y) + (ins && yc ? 1 : 0);
            cb += 2 * std::min(dy, k);
            cc += yc ? 2 * oy : 2 * std::min(oy, k);
        };
        for (u32 y : g.nbr(gr.x)) cost(y, in_c_[y] == epoch_);
        if (ins) for (u32 v : gr.c) cost(v, true);
        if (km == KernelMode::AdaptiveNoOrient) {
            if (cb < ca) { est_chosen_ = cb; return K::B; }
            return K::A;
        }
        if (cc < ca) { est_chosen_ = cc; return K::C; }
        return K::A;
    }

    std::vector<u32> in_n_, in_c_;
    u32 epoch_ = 0;
    u64 est_chosen_ = 0;

public:
    u64 bound_violations = 0;
};

}  // namespace r11
