// Screen B test M1B01: every window length at once, no expiry scan. Protocol: docs/experiments/screen-b-protocol.md.
// Usage: r11_window <events.bin> <stream> <k: 4|1> <slot seconds> <out.csv> [checkpoints]
//
// Fixed before any stream was read:
//  - Clock. Time is cut into slots of width w, slot(t) = floor(t / w). Primary w = 3,600 s, so 1, 7, 14, 30 and 90 days
//    are 24, 168, 336, 720 and 2,160 slots. w = 60 and w = 1 are sensitivity runs. w = 1 is the literal raw clock.
//  - In-window test, every method: pair in window L at now iff slot(last) > slot(now) - L. Strict, as the protocol says
//    "above". So a pair whose last slot equals now - L is out. Events in one slot tie, are applied one at a time in
//    stream order, and each count below is the state after that prefix of the stream.
//  - A pair with slot(last) <= now - Lmax does not exist: its record is gone before the next event is applied.
//  - Score baseline at k = 4 is the cheaper of B2 plain and B2 recency on each stream. At k = 1 it is the cheaper of the
//    plain counter and the death counter. W is always the recency form the protocol names. W0 is a diagnostic.
//
// Charges, all methods alike. One unit is one list element scanned, one adjacency probe, or one stamp, slot or field
// read or write (gate0-protocol.md). A read-modify-write of a counter is 2. Comparing values already held is free.
//   scan     one element of an adjacency list or recency log
//   probe    one pair lookup (u, v) -> record
//   read     one last-time, inline log time, believed time, death counter or degree read
//   field    one write of an extra per-edge field (last, believed time, death counter)
//   edit     2 per pair added or removed (protocol flat rate)
//   slot     2 per B[x] or d[e] read-modify-write; 1 per B read at a boundary; 1 per B clear
//   recency  1 per log append, 1 per entry moved at compaction, log2(size) per compaction search
//   queue    1 per slot-log append or read, 1 per bucket header read, 2 per token popped (id, last), 1 per token pushed
//   count    1 per running count written, committed once per scan or boundary step when the delta is not zero
#include <algorithm>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <string>
#include <vector>

#include "../src/r11/tc.hpp"

namespace ws {

using r11::canon;
using r11::Edge;
using r11::FlatMap;
using r11::i64;
using r11::key;
using r11::u32;
using r11::u64;

constexpr u32 NONE = 0xffffffffu;
constexpr i64 INF = std::numeric_limits<i64>::max();
constexpr size_t MAXK = 8;

struct Cost {
    u64 scan = 0, probe = 0, read = 0, field = 0, edit = 0, slot = 0, recency = 0, queue = 0, count = 0;
    u64 total() const { return scan + probe + read + field + edit + slot + recency + queue + count; }
    Cost& operator+=(const Cost& o) {
        scan += o.scan; probe += o.probe; read += o.read; field += o.field; edit += o.edit;
        slot += o.slot; recency += o.recency; queue += o.queue; count += o.count;
        return *this;
    }
};

struct Stats {
    u64 events = 0, firsts = 0, repeats = 0, noops = 0;     // pair absent, pair live, pair already at this slot
    u64 scans_event = 0, scans_expiry = 0;                  // whole scans started
    u64 elems_repeat = 0;                                   // list elements read by scans on repeats
    u64 moved = 0, added = 0, fallbacks = 0, rehomed = 0;
    Stats& operator+=(const Stats& o) {
        events = o.events; firsts += o.firsts; repeats += o.repeats; noops += o.noops;
        scans_event += o.scans_event; scans_expiry += o.scans_expiry; elems_repeat += o.elems_repeat;
        moved += o.moved; added += o.added; fallbacks += o.fallbacks; rehomed += o.rehomed;
        return *this;
    }
};

// One graph of live pairs. Each pair has one record, reached by hash from (u, v) or directly from a list element.
// Plain lists use swap removal, as r11::Graph does. The optional log holds, per vertex, one entry per refresh in time
// order, newest last. A repeat reads the log backwards. Superseded entries stay until compaction and are told apart by
// comparing their time with the record.
class Store {
public:
    Store(u32 n, bool use_log) : adj_(n), log_(use_log ? n : 0), lim_(use_log ? n : 0, 16), n_(n) {}

    u32 find(u32 u, u32 v, Cost& c) {
        ++c.probe;
        u32* p = map_.find(key(canon(u, v)));
        return p ? *p : NONE;
    }
    u32 low(u32 u, u32 v, Cost& c) const {          // lower degree first
        c.read += 2;
        return adj_[u].size() <= adj_[v].size() ? u : v;
    }
    u32 deg(u32 v) const { return u32(adj_[v].size()); }
    i64 last(u32 e) const { return rec_[e].last; }
    u32 end_a(u32 e) const { return rec_[e].a; }
    u32 end_b(u32 e) const { return rec_[e].b; }
    u64 pairs() const { return map_.size(); }
    u64 log_entries() const { return log_entries_; }
    u64 pool() const { return rec_.size(); }

    u32 alloc() {
        if (!free_.empty()) { u32 e = free_.back(); free_.pop_back(); return e; }
        rec_.push_back({});
        return u32(rec_.size() - 1);
    }
    void link(u32 e, u32 u, u32 v, i64 s, Cost& c) {
        c.edit += 2;
        c.field += 1;
        Edge d = canon(u, v);
        rec_[e] = Rec{d.a, d.b, u32(adj_[d.a].size()), u32(adj_[d.b].size()), s};
        adj_[d.a].push_back({d.b, e});
        adj_[d.b].push_back({d.a, e});
        if ((map_.size() + 1) * 10 > hash_cap_ * 7) hash_cap_ *= 2;
        map_.emplace(key(d), e);
    }
    u32 add(u32 u, u32 v, i64 s, Cost& c) { u32 e = alloc(); link(e, u, v, s, c); return e; }
    void remove(u32 e, Cost& c) {
        c.edit += 2;
        Rec r = rec_[e];
        map_.erase(key(Edge{r.a, r.b}));
        unlink(r.a, r.pa);
        unlink(r.b, r.pb);
        free_.push_back(e);
    }
    void set_last(u32 e, i64 s, Cost& c) { rec_[e].last = s; ++c.field; }

    // Recency upkeep: one entry on each side. Entries at or below `horizon` can never be read again.
    void touch(u32 e, i64 s, i64 horizon, Cost& c) {
        const Rec& r = rec_[e];
        push_log(r.a, r.b, e, s, horizon, c);
        push_log(r.b, r.a, e, s, horizon, c);
    }

    // Every common neighbour of x and y, with no time filter possible, so probing first is cheapest.
    template <class F> void common_all(u32 x, u32 y, Cost& c, F&& f) {
        for (const Adj& a : adj_[x]) {
            ++c.scan;
            if (a.nbr == y) continue;
            u32 g = find(a.nbr, y, c);
            if (g == NONE) continue;
            c.read += 2;
            f(a.eid, g, rec_[a.eid].last, rec_[g].last);
        }
    }
    // Common neighbours with cut < last(x, w) < upper, reading the time first and probing only the survivors.
    template <class F> void common_filtered(u32 x, u32 y, i64 cut, i64 upper, Cost& c, F&& f) {
        for (const Adj& a : adj_[x]) {
            ++c.scan;
            ++c.read;
            i64 lx = rec_[a.eid].last;
            if (lx <= cut || lx >= upper || a.nbr == y) continue;
            u32 g = find(a.nbr, y, c);
            if (g == NONE) continue;
            ++c.read;
            f(a.eid, g, lx, rec_[g].last);
        }
    }
    // Same set by reading the log backwards. If the log turns out longer than the plain list, finish on the plain list.
    template <class F> void common_recent(u32 x, u32 y, i64 cut, Cost& c, Stats& st, F&& f) {
        const std::vector<LogE>& lg = log_[x];
        u32 budget = deg(x), seen = 0;
        i64 prev = INF;                             // every entry with time >= prev has been visited
        for (size_t i = lg.size(); i-- > 0;) {
            const LogE& e = lg[i];
            ++c.scan;
            ++c.read;
            if (e.s <= cut) return;
            if (e.s != prev) {
                if (seen >= budget) {
                    ++st.fallbacks;
                    common_filtered(x, y, cut, prev, c, f);
                    return;
                }
                prev = e.s;
            }
            ++seen;
            if (e.nbr == y) continue;
            u32 g = find(e.nbr, y, c);
            if (g == NONE) continue;
            ++c.read;
            if (rec_[e.eid].last != e.s) continue;  // superseded entry
            ++c.read;
            f(e.eid, g, e.s, rec_[g].last);
        }
    }
    u64 count_common(u32 x, u32 y, Cost& c) {       // plain counter: no times needed
        u64 t = 0;
        for (const Adj& a : adj_[x]) {
            ++c.scan;
            if (a.nbr != y && find(a.nbr, y, c) != NONE) ++t;
        }
        return t;
    }
    template <class F> bool for_nbr(u32 x, F&& f) const {   // early exit when f returns false
        for (const Adj& a : adj_[x])
            if (!f(a.nbr, a.eid)) return false;
        return true;
    }

    double words() const {                          // one word is 8 bytes
        double w = hash_cap_ * 1.5 + 3.0 * double(rec_.size()) + 2.0 * double(map_.size()) + 3.0 * n_;
        if (!log_.empty()) w += 2.0 * double(log_entries_) + 4.0 * n_;
        return w;
    }

private:
    struct Rec { u32 a, b, pa, pb; i64 last; };
    struct Adj { u32 nbr, eid; };
    struct LogE { u32 nbr, eid; i64 s; };

    void unlink(u32 v, u32 i) {
        Adj back = adj_[v].back();
        adj_[v][i] = back;
        adj_[v].pop_back();
        if (i < adj_[v].size()) {
            Rec& q = rec_[back.eid];
            if (q.a == v) q.pa = i; else q.pb = i;
        }
    }
    void push_log(u32 x, u32 nbr, u32 e, i64 s, i64 horizon, Cost& c) {
        std::vector<LogE>& lg = log_[x];
        if (lg.size() >= lim_[x]) {
            auto it = std::partition_point(lg.begin(), lg.end(), [&](const LogE& g) { return g.s <= horizon; });
            size_t drop = size_t(it - lg.begin());
            u64 searched = 1;
            for (size_t m = lg.size(); m > 1; m >>= 1) ++searched;
            c.recency += searched + (lg.size() - drop);
            lg.erase(lg.begin(), it);
            log_entries_ -= drop;
            lim_[x] = std::max<u32>(16, u32(2 * lg.size()));
        }
        lg.push_back({nbr, e, s});
        ++log_entries_;
        ++c.recency;
    }

    std::vector<std::vector<Adj>> adj_;
    std::vector<std::vector<LogE>> log_;
    std::vector<u32> lim_, free_;
    std::vector<Rec> rec_;
    FlatMap<u32> map_;
    u64 log_entries_ = 0, hash_cap_ = 1u << 10;
    u32 n_;
};

// The distinct slots that hold at least one event, oldest first, with one reader per window. Units and tokens only ever
// sit at such slots, so a boundary visits these and never walks an empty stretch of time.
struct SlotLog {
    std::vector<i64> sl;
    std::vector<size_t> cur;
    explicit SlotLog(size_t k) : cur(k, 0) {}
    bool due(size_t i, i64 len, i64 s) const { return cur[i] < sl.size() && sl[cur[i]] + len <= s; }
    i64 next(size_t i, Cost& c) { ++c.queue; return sl[cur[i]++]; }
    void stamp(i64 s, Cost& c) {
        if (sl.empty() || sl.back() != s) { sl.push_back(s); ++c.queue; }
    }
    double live(size_t i) const { return double(sl.size() - cur[i]); }
};

// Expiry queue, lazy. A pair holds one token per window it is in, filed under some slot at or before its last time. When
// the boundary reaches the token, either the pair really leaves or the token is re-filed under its current last time. A
// repeat touches no queue state, which is why this beats a per-event queue on repeat-heavy streams.
class Buckets {
public:
    explicit Buckets(i64 len) : ring_(size_t(len + 1)), r_(len + 1) {}
    void push(i64 a, u32 e, Cost& c) { ring_[size_t(a % r_)].push_back(e); ++c.queue; ++tokens_; }
    void take(i64 a, std::vector<u32>& out, Cost& c) {
        ++c.queue;
        out.clear();
        out.swap(ring_[size_t(a % r_)]);
        tokens_ -= out.size();
    }
    double words() const { return 0.5 * double(tokens_) + 3.0 * double(r_); }
private:
    std::vector<std::vector<u32>> ring_;
    i64 r_;
    u64 tokens_ = 0;
};

// Method W. Array B holds one unit per triple with all three pairs live, at the slot of its sigma. S[i] is the sum of B
// over slots inside window i. With control_plain_add the move rule becomes a plain add, which must fail the oracle.
class MethodW {
public:
    Cost cost;
    Stats st;
    MethodW(u32 n, std::vector<i64> len, bool recency, bool control_plain_add = false)
        : g_(n, recency), L_(std::move(len)), k_(L_.size()), lmax_(L_.back()), r_(lmax_ + 1), rec_(recency),
          plain_add_(control_plain_add), B_(size_t(r_), 0), S_(k_, 0), clock_(k_), bk_(lmax_) {}

    i64 count(size_t i) const { return S_[i]; }
    i64 slot(i64 s) const { return B_[size_t(s % r_)]; }
    double words() const {
        return g_.words() + bk_.words() + double(r_) + 2.0 * double(k_) + clock_.live(k_ - 1);
    }
    const Store& graph() const { return g_; }

    void event(u32 u, u32 v, i64 s) {
        advance(s);
        ++st.events;
        now_ = s;
        std::fill(d_, d_ + k_, 0);
        u32 e = g_.find(u, v, cost);
        if (e == NONE) {
            ++st.firsts;
            ++st.scans_event;
            u32 x = g_.low(u, v, cost), y = x == u ? v : u;
            g_.common_all(x, y, cost, [&](u32, u32, i64 lx, i64 ly) { add_unit(std::min(lx, ly)); });
            e = g_.add(u, v, s, cost);
            bk_.push(s, e, cost);
            if (rec_) g_.touch(e, s, s - lmax_, cost);
        } else {
            ++cost.read;
            i64 p = g_.last(e);
            if (p == s) { ++st.noops; return; }
            ++st.repeats;
            ++st.scans_event;
            u32 x = g_.low(u, v, cost), y = x == u ? v : u;
            auto f = [&](u32, u32, i64 lx, i64 ly) {
                i64 m = std::min(lx, ly);
                if (m > p) { if (plain_add_) add_unit(m); else move_unit(p, m); }
            };
            u64 before = cost.scan;
            if (rec_) g_.common_recent(x, y, p, cost, st, f);
            else g_.common_filtered(x, y, p, INF, cost, f);
            st.elems_repeat += cost.scan - before;
            g_.set_last(e, s, cost);
            if (rec_) g_.touch(e, s, s - lmax_, cost);
        }
        for (size_t i = 0; i < k_; ++i)
            if (d_[i]) { S_[i] += d_[i]; ++cost.count; }
    }

private:
    void add_unit(i64 m) {
        ++B_[size_t(m % r_)];
        cost.slot += 2;
        ++st.added;
        for (size_t i = 0; i < k_; ++i) if (m > now_ - L_[i]) ++d_[i];
    }
    void move_unit(i64 p, i64 m) {
        --B_[size_t(p % r_)];
        ++B_[size_t(m % r_)];
        cost.slot += 4;
        ++st.moved;
        for (size_t i = 0; i < k_; ++i) if (p <= now_ - L_[i] && m > now_ - L_[i]) ++d_[i];
    }
    void advance(i64 s) {
        for (size_t i = 0; i < k_; ++i)
            while (clock_.due(i, L_[i], s)) {
                i64 a = clock_.next(i, cost);
                i64& b = B_[size_t(a % r_)];
                ++cost.slot;
                if (b) { S_[i] -= b; ++cost.count; }
                if (i + 1 == k_) {
                    if (b) { b = 0; ++cost.slot; }
                    bk_.take(a, tmp_, cost);
                    for (u32 e : tmp_) {        // no intersection here: every triple on e already left B
                        cost.queue += 2;
                        i64 l = g_.last(e);
                        if (l == a) g_.remove(e, cost); else bk_.push(l, e, cost);
                    }
                }
            }
        clock_.stamp(s, cost);
    }

    Store g_;
    std::vector<i64> L_;
    size_t k_;
    i64 lmax_, r_, now_ = 0;
    bool rec_, plain_add_;
    std::vector<i64> B_, S_;
    SlotLog clock_;
    Buckets bk_;
    std::vector<u32> tmp_;
    i64 d_[MAXK] = {};
};

// Baseline B2: one graph at the longest window, one shared insertion scan with timestamp filters, one expiry scan per
// window. Two forms. Plain uses the swap-removal lists. Recency uses the same log as W, pays the same upkeep, and scans
// only entries inside the window in question.
class MethodB2 {
public:
    Cost cost;
    Stats st;
    MethodB2(u32 n, std::vector<i64> len, bool recency)
        : g_(n, recency), L_(std::move(len)), k_(L_.size()), lmax_(L_.back()), rec_(recency), T_(k_, 0), clock_(k_) {
        for (size_t i = 0; i < k_; ++i) bk_.emplace_back(L_[i]);
    }
    i64 count(size_t i) const { return T_[i]; }
    double words() const {
        double w = g_.words() + double(k_) * 2.0 + clock_.live(k_ - 1);
        for (const Buckets& b : bk_) w += b.words();
        return w;
    }
    const Store& graph() const { return g_; }

    void event(u32 u, u32 v, i64 s) {
        advance(s);
        ++st.events;
        std::fill(d_, d_ + k_, 0);
        u32 e = g_.find(u, v, cost);
        if (e == NONE) {
            ++st.firsts;
            ++st.scans_event;
            u32 x = g_.low(u, v, cost), y = x == u ? v : u;
            g_.common_all(x, y, cost, [&](u32, u32, i64 lx, i64 ly) {
                i64 m = std::min(lx, ly);
                for (size_t i = 0; i < k_; ++i) if (m > s - L_[i]) ++d_[i];
            });
            e = g_.add(u, v, s, cost);
            for (size_t i = 0; i < k_; ++i) bk_[i].push(s, e, cost);
            if (rec_) g_.touch(e, s, log_horizon(s), cost);
        } else {
            ++cost.read;
            i64 p = g_.last(e);
            if (p == s) { ++st.noops; return; }
            ++st.repeats;
            size_t j = k_;                  // largest window the pair had left
            for (size_t i = k_; i-- > 0;) if (p <= s - L_[i]) { j = i; break; }
            if (j < k_) {
                ++st.scans_event;
                u32 x = g_.low(u, v, cost), y = x == u ? v : u;
                i64 cut = s - L_[j];
                auto f = [&](u32, u32, i64 lx, i64 ly) {
                    i64 m = std::min(lx, ly);
                    for (size_t i = 0; i <= j; ++i) if (m > s - L_[i]) ++d_[i];
                };
                u64 before = cost.scan;
                if (rec_) g_.common_recent(x, y, cut, cost, st, f);
                else g_.common_filtered(x, y, cut, INF, cost, f);
                st.elems_repeat += cost.scan - before;
                for (size_t i = 0; i <= j; ++i) bk_[i].push(s, e, cost);
            }
            g_.set_last(e, s, cost);
            if (rec_) g_.touch(e, s, log_horizon(s), cost);
        }
        for (size_t i = 0; i < k_; ++i)
            if (d_[i]) { T_[i] += d_[i]; ++cost.count; }
    }

private:
    i64 log_horizon(i64 s) const { return k_ > 1 ? s - L_[k_ - 2] : s - L_[0]; }   // the longest window never reads the log

    void advance(i64 s) {
        for (size_t i = 0; i < k_; ++i)
            while (clock_.due(i, L_[i], s)) {
                i64 a = clock_.next(i, cost);
                bk_[i].take(a, tmp_, cost);
                gone_.clear();
                for (u32 e : tmp_) {
                    cost.queue += 2;
                    i64 l = g_.last(e);
                    if (l == a) gone_.push_back(e); else bk_[i].push(l, e, cost);
                }
                for (u32 e : gone_) expiry_scan(e, i, a);
                if (i + 1 == k_) for (u32 e : gone_) g_.remove(e, cost);
            }
        clock_.stamp(s, cost);
    }
    // Every pair with last == a leaves window i now. A triple with several such pairs is charged to the smallest id, so
    // it leaves exactly once. Pairs with last < a have already left.
    void expiry_scan(u32 e, size_t i, i64 a) {
        ++st.scans_expiry;
        u32 u = g_.end_a(e), v = g_.end_b(e);
        u32 x = g_.low(u, v, cost), y = x == u ? v : u;
        i64 lost = 0;
        auto f = [&](u32 f1, u32 f2, i64 lx, i64 ly) {
            if (lx < a || ly < a || (lx == a && f1 < e) || (ly == a && f2 < e)) return;
            ++lost;
        };
        if (i + 1 == k_) g_.common_all(x, y, cost, f);
        else if (rec_) g_.common_recent(x, y, a - 1, cost, st, f);
        else g_.common_filtered(x, y, a - 1, INF, cost, f);
        if (lost) { T_[i] -= lost; ++cost.count; }
    }

    Store g_;
    std::vector<i64> L_;
    size_t k_;
    i64 lmax_;
    bool rec_;
    std::vector<i64> T_;
    SlotLog clock_;
    std::vector<Buckets> bk_;
    std::vector<u32> tmp_, gone_;
    i64 d_[MAXK] = {};
};

// Plain lower-degree scan counter for one window, on its own graph. No time filters: every list element is in the window.
// This is the k = 1 plain baseline, and four of these are B1.
class MethodP {
public:
    Cost cost;
    Stats st;
    MethodP(u32 n, i64 len) : g_(n, false), L_(len), clock_(1), bk_(len) {}
    i64 count() const { return T_; }
    double words() const { return g_.words() + bk_.words() + 2.0 + clock_.live(0); }

    void event(u32 u, u32 v, i64 s) {
        advance(s);
        ++st.events;
        u32 e = g_.find(u, v, cost);
        if (e == NONE) {
            ++st.firsts;
            ++st.scans_event;
            u32 x = g_.low(u, v, cost), y = x == u ? v : u;
            u64 t = g_.count_common(x, y, cost);
            if (t) { T_ += i64(t); ++cost.count; }
            e = g_.add(u, v, s, cost);
            bk_.push(s, e, cost);
        } else {
            ++cost.read;
            if (g_.last(e) == s) { ++st.noops; return; }
            ++st.repeats;
            g_.set_last(e, s, cost);
        }
    }

private:
    void advance(i64 s) {
        while (clock_.due(0, L_, s)) {
            i64 a = clock_.next(0, cost);
            bk_.take(a, tmp_, cost);
            for (u32 e : tmp_) {
                cost.queue += 2;
                i64 l = g_.last(e);
                if (l != a) { bk_.push(l, e, cost); continue; }
                ++st.scans_expiry;
                u32 u = g_.end_a(e), v = g_.end_b(e);
                u32 x = g_.low(u, v, cost), y = x == u ? v : u;
                u64 t = g_.count_common(x, y, cost);
                if (t) { T_ -= i64(t); ++cost.count; }
                g_.remove(e, cost);             // removal in sequence gives each triple exactly once
            }
        }
        clock_.stamp(s, cost);
    }
    Store g_;
    i64 L_, T_ = 0;
    SlotLog clock_;
    Buckets bk_;
    std::vector<u32> tmp_;
};

class MethodB1 {
public:
    MethodB1(u32 n, const std::vector<i64>& len) { for (i64 l : len) p_.emplace_back(n, l); }
    void event(u32 u, u32 v, i64 s) { for (MethodP& m : p_) m.event(u, v, s); }
    i64 count(size_t i) const { return p_[i].count(); }
    double words() const { double w = 0; for (const MethodP& m : p_) w += m.words(); return w; }
    Cost total_cost() const { Cost c; for (const MethodP& m : p_) c += m.cost; return c; }
    Stats total_stats() const { Stats s; for (const MethodP& m : p_) s += m.st; return s; }
private:
    std::vector<MethodP> p_;
};

// Scan at insert with a stored death counter (S6 Candidate 1), under refresh semantics, re-homing at the stale pop.
// Each pair has a believed time b <= last, the time of its token. A triple is homed on the pair with the smallest (b, id)
// and d[e] counts triples homed on e. On a true expiry the count falls by d[e], with no scan. On a stale pop, b rises to
// last, and the triples homed there are found by a scan and re-homed. Stale pops of one slot run before true ones.
class MethodD {
public:
    Cost cost;
    Stats st;
    MethodD(u32 n, i64 len) : g_(n, false), L_(len), clock_(1), bk_(len) {}
    i64 count() const { return T_; }
    double words() const { return g_.words() + bk_.words() + 2.0 + clock_.live(0) + 1.5 * double(g_.pool()); }

    void event(u32 u, u32 v, i64 s) {
        advance(s);
        ++st.events;
        u32 e = g_.find(u, v, cost);
        if (e == NONE) {
            ++st.firsts;
            ++st.scans_event;
            u32 x = g_.low(u, v, cost), y = x == u ? v : u;
            e = g_.alloc();
            grow(e);
            b_[e] = s;
            d_[e] = 0;
            cost.field += 2;
            u64 born = 0;
            scan(x, y, [&](u32 f1, u32 f2) {
                cost.read += 2;
                u32 h = home(e, s, f1, f2);
                ++d_[h];
                cost.slot += 2;
                ++born;
                return true;
            });
            if (born) { T_ += i64(born); ++cost.count; }
            g_.link(e, u, v, s, cost);
            bk_.push(s, e, cost);
        } else {
            ++cost.read;
            if (g_.last(e) == s) { ++st.noops; return; }
            ++st.repeats;
            g_.set_last(e, s, cost);
        }
    }

private:
    template <class F> void scan(u32 x, u32 y, F&& f) {
        g_.for_nbr(x, [&](u32 w, u32 f1) {
            ++cost.scan;
            if (w == y) return true;
            u32 f2 = g_.find(w, y, cost);
            if (f2 == NONE) return true;
            return f(f1, f2);
        });
    }
    void grow(u32 e) { if (e >= b_.size()) { b_.resize(e + 1); d_.resize(e + 1); } }
    bool before(u32 p, u32 q) const { return b_[p] != b_[q] ? b_[p] < b_[q] : p < q; }
    u32 home(u32 e, i64, u32 f1, u32 f2) const {
        u32 h = before(f1, f2) ? f1 : f2;
        return before(e, h) ? e : h;
    }
    void rehome(u32 e, i64 a) {
        ++cost.read;
        i64 l = g_.last(e);
        u32 left = d_[e];
        if (left) {
            ++st.scans_expiry;
            u32 u = g_.end_a(e), v = g_.end_b(e);
            u32 x = g_.low(u, v, cost), y = x == u ? v : u;
            u32 keep = 0;
            b_[e] = a;
            scan(x, y, [&](u32 f1, u32 f2) {
                cost.read += 2;
                if (!before(e, f1) || !before(e, f2)) return true;   // homed elsewhere
                b_[e] = l;
                u32 h = home(e, l, f1, f2);
                b_[e] = a;
                if (h == e) ++keep;
                else { ++d_[h]; cost.slot += 2; ++st.rehomed; }
                return --left != 0;
            });
            d_[e] = keep;
            ++cost.slot;
        }
        b_[e] = l;
        ++cost.field;
        bk_.push(l, e, cost);
    }
    void advance(i64 s) {
        while (clock_.due(0, L_, s)) {
            i64 a = clock_.next(0, cost);
            bk_.take(a, tmp_, cost);
            gone_.clear();
            for (u32 e : tmp_) {
                cost.queue += 2;
                if (g_.last(e) == a) gone_.push_back(e); else rehome(e, a);
            }
            for (u32 e : gone_) {
                ++cost.read;
                if (d_[e]) { T_ -= i64(d_[e]); ++cost.count; }
                g_.remove(e, cost);
            }
        }
        clock_.stamp(s, cost);
    }
    Store g_;
    i64 L_, T_ = 0;
    SlotLog clock_;
    Buckets bk_;
    std::vector<i64> b_;
    std::vector<u32> d_;
    std::vector<u32> tmp_, gone_;
};

}  // namespace ws

#ifndef WINDOW_SWEEP_NO_MAIN

using namespace ws;

namespace {

#pragma pack(push, 1)
struct Raw { i64 t; u32 u, v; };
#pragma pack(pop)

struct Row { std::string name; Cost c; Stats st; double words; u64 peak_pairs, peak_log; };

// Reference for the static checks. It shares no code with the methods above: a map of last slots, thinned by walking the
// event array, and r11::static_count on a graph rebuilt from scratch for each window.
struct Reference {
    FlatMap<i64> last;
    std::vector<u32> label, used;
    explicit Reference(u32 n) : label(n, NONE) {}
    i64 count(i64 s, i64 len) {
        struct P { u32 a, b; };
        std::vector<P> ps;
        used.clear();
        auto id = [&](u32 v) {
            if (label[v] == NONE) { label[v] = u32(used.size()); used.push_back(v); }
            return label[v];
        };
        last.for_each([&](u64 k, i64& l) {
            if (l > s - len) ps.push_back({id(u32(k >> 32)), id(u32(k))});
        });
        r11::Graph g(u32(used.size()));
        for (const P& p : ps) g.insert(p.a, p.b);
        for (u32 v : used) label[v] = NONE;
        return r11::static_count(g).first;
    }
};

}  // namespace

int main(int argc, char** argv) {
    if (argc < 6) { std::puts("usage: r11_window events.bin stream k slot_seconds out.csv [checkpoints]"); return 2; }
    FILE* in = std::fopen(argv[1], "rb");
    if (!in) { std::puts("cannot open events"); return 2; }
    _fseeki64(in, 0, SEEK_END);
    long long bytes = _ftelli64(in);
    _fseeki64(in, 0, SEEK_SET);
    std::vector<Raw> ev(size_t(bytes) / sizeof(Raw));
    if (std::fread(ev.data(), sizeof(Raw), ev.size(), in) != ev.size()) { std::puts("short read"); return 2; }
    std::fclose(in);

    std::string stream = argv[2];
    int k = std::atoi(argv[3]);
    i64 w = std::atoll(argv[4]);
    size_t checkpoints = argc > 6 ? size_t(std::atoll(argv[6])) : 1000;
    std::vector<i64> days = k == 4 ? std::vector<i64>{1, 7, 30, 90} : std::vector<i64>{14};
    std::vector<i64> len;
    for (i64 d : days) {
        if (d * 86400 % w != 0) { std::puts("windows must be exact multiples of the slot width"); return 2; }
        len.push_back(d * 86400 / w);
    }
    u32 n = 0;
    for (const Raw& x : ev) n = std::max({n, x.u + 1, x.v + 1});
    i64 base = ev.front().t / w;
    i64 lmax = len.back();

    MethodW W(n, len, true), W0(n, len, false);
    MethodB2 B2p(n, len, false), B2r(n, len, true);
    MethodB1 B1(n, len);
    MethodP P(n, len.back());
    MethodD D(n, len.back());

    Reference ref(n);
    size_t head = 0, every = std::max<size_t>(1, ev.size() / checkpoints), checks = 0;
    double pw = 0, pw0 = 0, pb2p = 0, pb2r = 0, pb1 = 0, pp = 0, pd = 0;
    u64 pair_w = 0, pair_b2 = 0, log_w = 0, log_b2 = 0, peak_tri = 0;

    for (size_t idx = 0; idx < ev.size(); ++idx) {
        u32 u = ev[idx].u, v = ev[idx].v;
        i64 s = ev[idx].t / w - base;
        W.event(u, v, s);
        W0.event(u, v, s);
        if (k == 4) { B2p.event(u, v, s); B2r.event(u, v, s); B1.event(u, v, s); }
        else { P.event(u, v, s); D.event(u, v, s); }

        // every method, every window, every event
        for (int i = 0; i < k; ++i) {
            i64 c = W.count(size_t(i));
            bool ok = W0.count(size_t(i)) == c;
            if (k == 4) ok = ok && B2p.count(size_t(i)) == c && B2r.count(size_t(i)) == c && B1.count(size_t(i)) == c;
            else ok = ok && P.count() == c && D.count() == c;
            if (!ok) {
                std::printf("DISAGREE stream=%s event=%zu slot=%lld window=%d W=%lld W0=%lld B2p=%lld B2r=%lld B1=%lld P=%lld D=%lld\n",
                            stream.c_str(), idx, (long long)s, i, (long long)c, (long long)W0.count(size_t(i)),
                            (long long)(k == 4 ? B2p.count(size_t(i)) : 0), (long long)(k == 4 ? B2r.count(size_t(i)) : 0),
                            (long long)(k == 4 ? B1.count(size_t(i)) : 0), (long long)(k == 1 ? P.count() : 0),
                            (long long)(k == 1 ? D.count() : 0));
                return 1;
            }
        }
        peak_tri = std::max<u64>(peak_tri, u64(W.count(size_t(k - 1))));

        u64 kk = key(canon(u, v));
        if (i64* l = ref.last.find(kk)) *l = s; else ref.last.emplace(kk, s);
        for (; head <= idx && ev[head].t / w - base <= s - lmax; ++head) {
            u64 hk = key(canon(ev[head].u, ev[head].v));
            i64* l = ref.last.find(hk);
            if (l && *l <= s - lmax) ref.last.erase(hk);
        }
        if (idx % every == 0 || idx + 1 == ev.size()) {
            for (int i = 0; i < k; ++i) {
                i64 t = ref.count(s, len[size_t(i)]);
                ++checks;
                if (t != W.count(size_t(i))) {
                    std::printf("STATIC MISMATCH stream=%s event=%zu window=%d static=%lld W=%lld\n", stream.c_str(), idx, i,
                                (long long)t, (long long)W.count(size_t(i)));
                    return 1;
                }
            }
        }

        pw = std::max(pw, W.words());
        pw0 = std::max(pw0, W0.words());
        pair_w = std::max(pair_w, W.graph().pairs());
        log_w = std::max(log_w, W.graph().log_entries());
        if (k == 4) {
            pb2p = std::max(pb2p, B2p.words());
            pb2r = std::max(pb2r, B2r.words());
            pb1 = std::max(pb1, B1.words());
            pair_b2 = std::max(pair_b2, B2r.graph().pairs());
            log_b2 = std::max(log_b2, B2r.graph().log_entries());
        } else {
            pp = std::max(pp, P.words());
            pd = std::max(pd, D.words());
        }
    }

    std::vector<Row> rows;
    rows.push_back({"W", W.cost, W.st, pw, pair_w, log_w});
    rows.push_back({"W0", W0.cost, W0.st, pw0, pair_w, 0});
    if (k == 4) {
        rows.push_back({"B2_plain", B2p.cost, B2p.st, pb2p, pair_b2, 0});
        rows.push_back({"B2_recency", B2r.cost, B2r.st, pb2r, pair_b2, log_b2});
        rows.push_back({"B1", B1.total_cost(), B1.total_stats(), pb1, 0, 0});
    } else {
        rows.push_back({"P", P.cost, P.st, pp, pair_w, 0});
        rows.push_back({"D", D.cost, D.st, pd, pair_w, 0});
    }

    FILE* f = std::fopen(argv[5], "w");
    if (!f) { std::puts("cannot open output"); return 2; }
    std::fprintf(f, "stream,label,k,slot_seconds,method,ops,scan,probe,read,field,edit,slot,recency,queue,count,events,firsts,repeats,noops,"
                    "scans_event,scans_expiry,elems_repeat,moved,added,fallbacks,rehomed,peak_words,peak_pairs,peak_log,"
                    "peak_count_longest,static_checks,mismatches,vertices\n");
    for (const Row& r : rows) {
        const Cost& c = r.c;
        const Stats& t = r.st;
        std::fprintf(f, "%s,real timestamps window query,%d,%lld,%s,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,"
                        "%llu,%llu,%llu,%llu,%llu,%llu,%llu,%.0f,%llu,%llu,%llu,%zu,0,%u\n",
                     stream.c_str(), k, (long long)w, r.name.c_str(), (unsigned long long)c.total(), (unsigned long long)c.scan,
                     (unsigned long long)c.probe, (unsigned long long)c.read, (unsigned long long)c.field, (unsigned long long)c.edit,
                     (unsigned long long)c.slot, (unsigned long long)c.recency, (unsigned long long)c.queue, (unsigned long long)c.count,
                     (unsigned long long)t.events, (unsigned long long)t.firsts, (unsigned long long)t.repeats, (unsigned long long)t.noops,
                     (unsigned long long)t.scans_event, (unsigned long long)t.scans_expiry, (unsigned long long)t.elems_repeat,
                     (unsigned long long)t.moved, (unsigned long long)t.added, (unsigned long long)t.fallbacks, (unsigned long long)t.rehomed,
                     r.words, (unsigned long long)r.peak_pairs, (unsigned long long)r.peak_log, (unsigned long long)peak_tri, checks, n);
    }
    std::fclose(f);
    std::printf("%s k=%d w=%lld events=%zu checks=%zu", stream.c_str(), k, (long long)w, ev.size(), checks);
    for (const Row& r : rows) std::printf(" %s=%llu", r.name.c_str(), (unsigned long long)r.c.total());
    std::puts("");
    return 0;
}

#endif  // WINDOW_SWEEP_NO_MAIN
