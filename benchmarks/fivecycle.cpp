// M1C02 and M1C03: exact dynamic 5-cycle counting. Protocol: docs/experiments/screen-b-protocol.md, sections M1C02 and M1C03.
// Usage: r11_fivecycle <events.bin> <stream> <insert|expiry> <cut> <arm> <outdir> [budget multiple] [--cap-seconds S] [--cap-bytes B] [--cpu N] [--rep N]
// Each arm is its own process, so that one process runs at a time and each stays under ten minutes. Per-batch rows go to
// <outdir>/raw. scripts/summarise_fivecycle.py (M1C02) and scripts/summarise_m1c03.py (M1C03) join them, compare the counts and apply the rule.
//
// M1C03, WHAT CHANGED FROM THE M1C02 SOURCE. The M1C02 source is kept in results/screen-c-M1C03/source-M1C02.
// Arm WT is new. R2, PR, WJ, PW, R2B and RC keep their algorithms and their charges, and their counts must match M1C02.
// Written so that no arm loses to a careless implementation, with no change to any count:
// - Two binaries from this one source. The count build has the counters. The timing build (-DFC_TIMING) has none, and it has no mutants: a counter
//   in a hot loop is a store the compiler cannot move, since it may alias a row word. Measured on a synthetic stream, counters cost R2 and PR
//   about 15% and WJ about 1%, so they would favour the table. All timing comes from the timing build, and the counts come from the count build.
// - normalise and make_groups are replaced by Normaliser and Grouper: same output in the same order, no hash node per event, no vector per group.
//   The old forms cost PR most on small cells, and a constant added to every arm pulls every score towards 1.
// - The row kernels prefetch list headers and list data ahead of the walk.
// - An engine allocates only what its arm uses.
// - Per-batch rows are written after the clock stops. Limits: a job object refuses commit past --cap-bytes, and a sleeping watchdog thread stops the
//   process --cap-seconds after it started and leaves a partial record, status TIME. An allocation that fails ends the run, status MEM.
//
// WT, THE TUNED TABLE. Same entries and same formula as WJ. Chosen by trial on synthetic R-MAT streams in the scratchpad, so that no protocol stream
// shaped it (hard rule 5). See results/screen-c-M1C03/layout-trial.md.
// - One open-addressing table per vertex, and every pair stored in both tables. A join then reads only tables of the shorter list's vertices, and the
//   inner loop stays inside one small table. This costs two writes per wedge, one of them remote.
// - A slot is 4 bytes: key in the high bits, count in the low bits, as many bits as the vertex count leaves. Slot 0 is empty. A count that reaches the
//   top of its bits saturates, and its true value lives in a side table keyed by pair. This halves the memory of an 8 byte slot.
// - Reads and writes search four slots at a time with SSE2, which is in the x86-64 baseline and so inside the project flags. The first three slots are
//   mirrored past the end, so a window never wraps. This removes the branch misprediction per probe.
// - Each row of the join takes the cheaper of q probes and one pass over the table against a bitset of the inner list, when 4 q >= capacity.
// - Join probes into a table of 2^12 slots or more prefetch 16 ahead. Upkeep runs in blocks of 16: prefetch directory entries, then slots, then write.
// - Tables start at 8 slots, shrink below 1/16 load, and come from a size-class pool, since expiry streams rehash many small tables.
// CHARGES FOR WT, on the footing of WJ. A probe or a slot read is one table read whatever its probe length. One table read per directory entry, which
// WJ does not have. An add or a removal is 1 read and 1 write, with 1 more write for a created or removed key, on each of the two tables. A rehash is 2
// per live entry moved. Bitset writes and clears are row writes, bitset reads are row reads. Lists are charged per element per pass. Degrees 1.
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
#include <atomic>
#include <chrono>
#include <cinttypes>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>
#include <string>
#include <utility>
#include <vector>

#include <emmintrin.h>

#include "../src/r11/tc.hpp"

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

namespace fc {

using namespace r11;

#ifdef FC_TIMING
constexpr int MUT = 0;  // the timing binary has no mutants: every branch on MUT folds away
// A counter that is not there. It stays in the source of the hot loops, and the compiler removes it. See the M1C03 note above.
struct Tick {
    constexpr Tick& operator++() { return *this; }
    template <class T> constexpr Tick& operator+=(T) { return *this; }
    constexpr operator u64() const { return 0; }
};
struct Cost {
    Tick scan, probe, row, deg, tab_read, tab_write, graph, net, group, rc, upkeep;
    constexpr u64 total() const { return 0; }
};
#else
inline int MUT = 0;  // 0 in every real run. tests/test_fivecycle.cpp sets the mutants, listed there.

struct Cost {
    u64 scan = 0, probe = 0, row = 0, deg = 0, tab_read = 0, tab_write = 0, graph = 0, net = 0, group = 0, rc = 0;
    u64 upkeep = 0;     // a tally inside the total: everything spent keeping the table, rehash included
    u64 total() const { return scan + probe + row + deg + tab_read + tab_write + graph + net + group + rc; }
};
#endif

// r11::normalise with the same output in the same order, and no node allocated per event. All arms pay it.
class Normaliser {
public:
    const NetBatch& run(const Graph& g, const std::vector<Update>& b) {
        out_.del.clear();
        out_.ins.clear();
        size_t cap = 16;
        while (cap < 2 * b.size()) cap *= 2;
        if (cap > keys_.size()) {
            keys_.assign(cap, 0);
            tags_.assign(cap, 0);
            ep_ = 0;
            shift_ = 64;
            for (size_t x = cap; x > 1; x >>= 1) --shift_;
        }
        ++ep_;
        const size_t mask = keys_.size() - 1;
        for (auto it = b.rbegin(); it != b.rend(); ++it) {
            if (it->u == it->v) continue;
            const Edge e = canon(it->u, it->v);
            const u64 k = key(e);
            size_t i = size_t((k * 0x9E3779B97F4A7C15ull) >> shift_);
            bool seen = false;
            while (tags_[i] == ep_) {
                if (keys_[i] == k) { seen = true; break; }
                i = (i + 1) & mask;
            }
            if (seen) continue;
            tags_[i] = ep_;
            keys_[i] = k;
            const bool present = g.has(e.a, e.b);
            if (it->ins && !present) out_.ins.push_back(e);
            if (!it->ins && present) out_.del.push_back(e);
        }
        return out_;
    }

private:
    NetBatch out_;
    std::vector<u64> keys_;
    std::vector<u32> tags_;
    u32 ep_ = 0;
    int shift_ = 64;
};

// A star as PR and PW read it. Grouper gives the groups of make_groups(.., Pivot::BatchDegree) in the same order, with the spokes in the
// same order, from epoch-tagged arrays and one flat store, so a batch costs no heap allocation. tests/test_fivecycle.cpp asserts the match.
struct Spokes {
    const u32* b = nullptr;
    const u32* e = nullptr;
    size_t size() const { return size_t(e - b); }
    u32 operator[](size_t i) const { return b[i]; }
    const u32* begin() const { return b; }
    const u32* end() const { return e; }
};
struct GroupV { u32 x; Spokes c; };

class Grouper {
public:
    explicit Grouper(u32 n) : bd_(n, 0), at_(n, 0) {}
    const std::vector<GroupV>& run(const std::vector<Edge>& es) {
        ++ep_;
        const u64 tag = u64(ep_) << 32;
        const u64 hi = ~u64(0xffffffffu);
        for (const Edge& e : es) {
            for (u32 v : {e.a, e.b}) bd_[v] = ((bd_[v] & hi) == tag ? bd_[v] : tag) + 1;
        }
        gs_.clear();
        off_.clear();
        idx_.resize(es.size());
        spoke_.resize(es.size());
        store_.resize(es.size());
        for (size_t i = 0; i < es.size(); ++i) {
            const u32 a = es[i].a, b = es[i].b, ba = u32(bd_[a]), bb = u32(bd_[b]);
            const u32 x = (ba != bb ? ba > bb : a < b) ? a : b;
            spoke_[i] = x == a ? b : a;
            if ((at_[x] & hi) != tag) {
                at_[x] = tag | u32(gs_.size());
                gs_.push_back(GroupV{x, {}});
                off_.push_back(0);
            }
            idx_[i] = u32(at_[x]);
            ++off_[idx_[i]];
        }
        u32 sum = 0;
        for (size_t g = 0; g < gs_.size(); ++g) {
            const u32 k = off_[g];
            gs_[g].c.b = store_.data() + sum;
            gs_[g].c.e = store_.data() + sum + k;
            off_[g] = sum;
            sum += k;
        }
        for (size_t i = 0; i < es.size(); ++i) store_[off_[idx_[i]]++] = spoke_[i];
        return gs_;
    }

private:
    std::vector<u64> bd_, at_;      // epoch, then batch degree or group index
    std::vector<GroupV> gs_;
    std::vector<u32> off_, idx_, spoke_, store_;
    u32 ep_ = 0;
};

// The row kernels walk a list of lists. Each list is two misses, its header and its data, and both can be fetched ahead.
inline void prefetch_lists(const Graph& g, const u32* list, size_t i, size_t n) {
    if (i + 10 < n) __builtin_prefetch(&g.nbr(list[i + 10]));
    if (i + 5 < n) __builtin_prefetch(g.nbr(list[i + 5]).data());
}

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
    const std::vector<u32>& np = g.nbr(p);
    for (size_t i = 0; i < np.size(); ++i) {
        prefetch_lists(g, np.data(), i, np.size());
        const u32 a = np[i];
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
    const std::vector<u32>& nq = g.nbr(q);
    for (size_t i = 0; i < nq.size(); ++i) {
        prefetch_lists(g, nq.data(), i, nq.size());
        const u32 m = nq[i];
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
inline i64 star_pr(Graph& g, const GroupV& gr, bool ins, Row& row, Cost& c) {
    const u32 x = gr.x;
    if (gr.c.size() == 1) {
        u32 v = gr.c[0];
        i64 s = through_r2(g, x, v, row, c);
        if (ins) g.insert(x, v); else g.erase(x, v);
        c.graph += 2;
        return ins ? s : -s;
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

// WT: the wedge table of M1C03. The header note above says why it is laid out this way.
class WTable {
public:
    static constexpr u64 kSlotBytes = 4;
    u64 cap_bytes = 6000000000ull;
    bool capped = false;

    // count_limit, used only by the test, forces a small saturation point, so that the overflow path runs on small graphs.
    explicit WTable(u32 n, u32 count_bits = 0) : d_(n, Dir{zero_, 0, 2}), bits_((n >> 6) + 2, 0) {
        u32 kb = 1;
        while ((u64(1) << kb) < n) ++kb;
        cb_ = count_bits ? count_bits : 32 - kb;
        cm_ = (cb_ >= 31 ? 0x7fffffffu : (1u << cb_) - 1);
        bytes_ = fixed_bytes();
        peak_bytes_ = peak_transient_ = bytes_;
    }
    ~WTable() {
        for (Dir& d : d_)
            if (d.s != zero_) pool_.put(d.s, d.lg);
    }
    WTable(const WTable&) = delete;
    WTable& operator=(const WTable&) = delete;

    u64 size() const { return directed_ / 2; }                 // pairs
    u64 peak_entries() const { return peak_directed_ / 2; }
    u64 peak_slots() const { return peak_slots_; }
    u64 peak_bytes() const { return peak_bytes_; }
    u64 peak_transient() const { return peak_transient_; }
    u64 saturated() const { return over_.size(); }
    u64 peak_saturated() const { return peak_over_; }

    // The wedge between x and each t of l changes. Each pair is written in both tables.
    void change_list(u32 x, const std::vector<u32>& l, bool ins, Cost& c) {
        const size_t k = l.size();
        c.scan += k;
        ++c.tab_read;
        __builtin_prefetch(&d_[x]);
        if (ins && k > 8) reserve(x, k, c);
        for (size_t st = 0; st < k; st += 16) {
            const size_t en = std::min(k, st + 16);
            for (size_t i = st; i < en; ++i) __builtin_prefetch(&d_[l[i]]);
            for (size_t i = st; i < en; ++i) {
                const Dir& f = d_[l[i]];
                __builtin_prefetch(f.s + home(x, f.lg));
            }
            c.tab_read += en - st;
            for (size_t i = st; i < en; ++i) {
                if (ins) add_pair(x, l[i], c); else sub_pair(x, l[i], c);
            }
        }
    }

    // sum over a in N(u), c in N(v), a != c of W[a, c], minus tau (p - 1), minus tau (q - 1). The pair must be absent from g and from the table.
    i64 join(const Graph& g, u32 u, u32 v, Cost& c) {
        c.deg += 2;
        const bool fl = g.deg(u) <= g.deg(v);
        const std::vector<u32>& outer = g.nbr(fl ? u : v);
        const std::vector<u32>& inner = g.nbr(fl ? v : u);
        const size_t p = outer.size(), q = inner.size();
        if (p == 0 || q == 0) return 0;
        const u32* ip = inner.data();
        for (size_t j = 0; j < q; ++j) bits_[ip[j] >> 6] |= u64(1) << (ip[j] & 63);
        c.scan += q;
        c.row += q;
        const i64 s = over_.size() ? sum_rows<true>(outer, ip, q, c) : sum_rows<false>(outer, ip, q, c);
        if (MUT != 15)
            for (size_t j = 0; j < q; ++j) bits_[ip[j] >> 6] = 0;
        c.scan += q;
        c.row += q;
        return s;
    }

    // Gate 1 only: one entry, read in a's table. No arm of M1C02 or M1C03 calls this.
    u32 get(u32 a, u32 k) const {
        const Dir& d = d_[a];
        if (d.s == zero_) return 0;
        const u32 x = d.s[find(d, k)];
        if (!x) return 0;
        const u32 n = x & cm_;
        return n == cm_ ? side_count(a, k) : n;
    }

    template <class F> void for_each(F f) const {
        for (u32 a = 0; a < u32(d_.size()); ++a) {
            const Dir& d = d_[a];
            if (d.s == zero_) continue;
            for (u32 i = 0; i < (1u << d.lg); ++i) {
                const u32 x = d.s[i];
                if (!x) continue;
                const u32 k = x >> cb_, n = x & cm_;
                f(a, k, n == cm_ ? side_count(a, k) : n);
            }
        }
    }

    // Size-class allocator, so that the many small tables of an expiry stream do not go through the heap on every rehash.
    class Pool {
    public:
        u32* get(u32 lg) {
            const size_t words = (size_t(1) << lg) + 3;
            u32* b;
            if (lg >= kPoolMax) {
                b = static_cast<u32*>(std::calloc(words, 4));
                if (!b) throw std::bad_alloc();
                return b;
            }
            auto& f = free_[lg];
            if (!f.empty()) {
                b = f.back();
                f.pop_back();
            } else {
                if (left_ < words) {
                    left_ = size_t(1) << 20;
                    cur_ = static_cast<u32*>(std::malloc(left_ * 4));
                    if (!cur_) { left_ = 0; throw std::bad_alloc(); }
                    chunks_.push_back(cur_);
                }
                b = cur_;
                cur_ += words;
                left_ -= words;
            }
            std::memset(b, 0, words * 4);
            return b;
        }
        void put(u32* b, u32 lg) {
            if (lg >= kPoolMax) std::free(b);
            else free_[lg].push_back(b);
        }
        ~Pool() { for (u32* c : chunks_) std::free(c); }

    private:
        static constexpr u32 kPoolMax = 16;
        std::vector<u32*> free_[kPoolMax], chunks_;
        u32* cur_ = nullptr;
        size_t left_ = 0;
    };

private:
    struct Dir { u32* s; u32 size, lg; };   // 16 bytes
    static constexpr u32 kHash = 0x9E3779B1u, kMinLg = 3, kShrink = 16;

    static u64 pair_key(u32 a, u32 c) { return a < c ? (u64(a) << 32) | c : (u64(c) << 32) | a; }
    static u32 home(u32 c, u32 lg) { return (c * kHash) >> (32 - lg); }
    // The true value of a saturated count. The entry is always there unless a mutant has broken the table, and a mutant must fail by a wrong count.
    u32 side_count(u32 a, u32 c) const {
        const u32* e = over_.find(pair_key(a, c));
        return e ? *e : cm_;
    }
    u64 fixed_bytes() const { return d_.size() * sizeof(Dir) + bits_.size() * 8; }
    void note_bytes() {
        const u64 b = bytes_ + over_.size() * 24;
        peak_bytes_ = std::max(peak_bytes_, b);
        peak_transient_ = std::max(peak_transient_, b);
    }

    inline void put(Dir& d, u32 i, u32 x) {
        d.s[i] = x;
        if (i < 3 && MUT != 14) d.s[(1u << d.lg) + i] = x;
    }

    // The first slot holding key c or empty, reading four slots at a time, with no branch on where in the window it falls.
    inline u32 find(const Dir& d, u32 c) const {
        const u32 m = (1u << d.lg) - 1;
        const __m128i sh = _mm_cvtsi32_si128(int(cb_)), ky = _mm_set1_epi32(int(c)), zero = _mm_setzero_si128();
        u32 i = home(c, d.lg);
        for (;;) {
            const __m128i w = _mm_loadu_si128(reinterpret_cast<const __m128i*>(d.s + i));
            const int h = _mm_movemask_epi8(_mm_or_si128(_mm_cmpeq_epi32(_mm_srl_epi32(w, sh), ky), _mm_cmpeq_epi32(w, zero)));
            if (h) return (i + (u32(__builtin_ctz(u32(h))) >> 2)) & m;
            i = (i + 4) & m;
        }
    }

    bool grow(Dir& d, u32 lg, Cost& c) {
        const bool first = d.s == zero_;
        const u64 cap = u64(1) << lg, add = (cap + 3) * 4;
        if (bytes_ + add > cap_bytes) { capped = true; return false; }
        u32* ns = pool_.get(lg);
        Dir nd{ns, d.size, lg};
        bytes_ += add;
        slots_ += cap;
        peak_slots_ = std::max(peak_slots_, slots_);
        peak_transient_ = std::max(peak_transient_, bytes_ + over_.size() * 24);
        if (!first) {
            const u32 ocap = 1u << d.lg;
            for (u32 i = 0; i < ocap; ++i) {
                const u32 x = d.s[i];
                if (!x) continue;
                u32 j = home(x >> cb_, lg);
                while (ns[j]) j = (j + 1) & u32(cap - 1);
                put(nd, j, x);
            }
            c.tab_read += d.size;
            c.tab_write += d.size;
            pool_.put(d.s, d.lg);
            bytes_ -= (u64(ocap) + 3) * 4;
            slots_ -= ocap;
        }
        d = nd;
        note_bytes();
        return true;
    }

    void reserve(u32 x, size_t k, Cost& c) {
        Dir& d = d_[x];
        u32 lg = d.s == zero_ ? kMinLg : d.lg;
        while ((u64(d.size) + k) * 10 > (u64(1) << lg) * 7) ++lg;
        if (lg != d.lg || d.s == zero_) grow(d, lg, c);
    }

    // Returns the count seen before, as held in the slot. A saturated slot is left alone.
    inline u32 add_one(u32 a, u32 t, Cost& c) {
        Dir& d = d_[a];
        if (d.s == zero_ || (d.size + 1) * 10 > (1u << d.lg) * 7) {
            if (!grow(d, d.s == zero_ ? kMinLg : d.lg + 1, c)) return 0;
        }
        const u32 i = find(d, t);
        const u32 x = d.s[i];
        c.tab_read += 1;
        c.tab_write += 1;
        if (x == 0) {
            put(d, i, (t << cb_) | 1);
            ++d.size;
            ++directed_;
            c.tab_write += 1;
            peak_directed_ = std::max(peak_directed_, directed_);
            return 0;
        }
        const u32 n = x & cm_;
        if (n != cm_) put(d, i, x + 1);
        return n;
    }

    inline void remove_at(Dir& d, u32 i, Cost& c) {
        const u32 m = (1u << d.lg) - 1;
        c.tab_write += 1;
        if (MUT != 16) {
            for (u32 j = (i + 1) & m; d.s[j]; j = (j + 1) & m) {
                const u32 h = home(d.s[j] >> cb_, d.lg);
                if (((j - h) & m) >= ((j - i) & m)) {
                    put(d, i, d.s[j]);
                    i = j;
                }
            }
        }
        put(d, i, 0);
        --d.size;
        --directed_;
        if (d.lg > kMinLg && u64(d.size) * kShrink < (u64(1) << d.lg)) grow(d, d.lg - 1, c);
    }

    inline void add_pair(u32 x, u32 t, Cost& c) {
        if (capped) return;
        const u32 n = add_one(x, t, c);
        if (n == cm_) {
            if (u32* e = over_.find(pair_key(x, t))) ++*e;
            c.tab_read += 1;
            c.tab_write += 1;
            return;
        }
        if (MUT != 13) add_one(t, x, c);
        if (n + 1 == cm_) {
            over_.emplace(pair_key(x, t), cm_);
            c.tab_write += 2;
            peak_over_ = std::max<u64>(peak_over_, over_.size());
            note_bytes();
        }
    }

    inline void sub_pair(u32 x, u32 t, Cost& c) {
        Dir& d = d_[x];
        const u32 i = find(d, t);
        c.tab_read += 1;
        c.tab_write += 1;
        const u32 w = d.s[i];
        if (w == 0) return;     // only after a cap hit, when the run is already void
        const u32 n = w & cm_;
        if (n == cm_) {
            u32* e = over_.find(pair_key(x, t));
            c.tab_read += 1;
            c.tab_write += 1;
            if (!e) return;
            if (--*e >= cm_) return;
            over_.erase(pair_key(x, t));
            c.tab_write += 1;
        }
        if (n > 1) put(d, i, w - 1); else remove_at(d, i, c);
        if (MUT == 13) return;
        Dir& e = d_[t];
        const u32 j = find(e, x);
        c.tab_read += 1;
        c.tab_write += 1;
        const u32 y = e.s[j];
        if (y == 0) return;     // as above
        if ((y & cm_) > 1) put(e, j, y - 1); else remove_at(e, j, c);
    }

    static i64 hsum(__m128i a) {
        alignas(16) u32 x[4];
        _mm_store_si128(reinterpret_cast<__m128i*>(x), a);
        return i64(x[0]) + x[1] + x[2] + x[3];
    }

    // Over: some count is saturated, so a read of the saturation value goes to the side table. The other form has no such test.
    template <bool Over> i64 sum_rows(const std::vector<u32>& outer, const u32* ip, size_t q, Cost& c) const {
        const size_t p = outer.size();
        const __m128i cm = _mm_set1_epi32(int(cm_)), zero = _mm_setzero_si128(), sh = _mm_cvtsi32_si128(int(cb_));
        i64 s = 0;
        u64 tau = 0;
        const u64* bits = bits_.data();
        for (size_t i = 0; i < p; ++i) {
            if (i + 4 < p) __builtin_prefetch(&d_[outer[i + 4]]);
            const u32 a = outer[i];
            tau += (bits[a >> 6] >> (a & 63)) & 1;
            const Dir d = d_[a];
            c.scan += 1;
            c.row += 1;
            c.tab_read += 1;
            const u32 cap = 1u << d.lg;
            if (u64(q) * 4 >= cap) {
                // Reading the whole table costs less than q probes, so walk it against the bitset of the inner list.
                u64 acc = 0;
                for (u32 j = 0; j < cap; ++j) {
                    const u32 w = d.s[j], k = w >> cb_, n = w & cm_;
                    const u32 on = u32((bits[k >> 6] >> (k & 63)) & 1);
                    acc += n & (0u - on);
                    if (Over && n == cm_ && on && MUT != 17) acc += side_count(a, k) - cm_;
                }
                c.tab_read += cap;
                c.row += cap;
                s += i64(acc);
                continue;
            }
            const int sft = 32 - int(d.lg);
            const u32 mask = cap - 1;
            const bool fetch = d.lg >= 12;
            __m128i acc = zero;
            c.scan += q;
            c.tab_read += q;
            for (size_t j = 0; j < q; ++j) {
                if (fetch && j + 16 < q) __builtin_prefetch(d.s + ((ip[j + 16] * kHash) >> sft));
                const u32 y = ip[j];
                const __m128i ky = _mm_set1_epi32(int(y));
                u32 pos = (y * kHash) >> sft;
                for (;;) {
                    const __m128i w = _mm_loadu_si128(reinterpret_cast<const __m128i*>(d.s + pos));
                    const __m128i hit = _mm_cmpeq_epi32(_mm_srl_epi32(w, sh), ky);
                    const __m128i cnt = _mm_and_si128(_mm_and_si128(w, cm), hit);
                    acc = _mm_add_epi32(acc, cnt);
                    if constexpr (Over) {
                        if (_mm_movemask_epi8(_mm_cmpeq_epi32(cnt, cm)) && MUT != 17) s += i64(side_count(a, y)) - cm_;
                    }
                    if (_mm_movemask_epi8(_mm_or_si128(hit, _mm_cmpeq_epi32(w, zero)))) break;
                    pos = (pos + 4) & mask;
                }
                if ((j & 8191) == 8191) { s += hsum(acc); acc = zero; }
            }
            s += hsum(acc);
        }
        const i64 t = i64(tau);
        if (MUT != 18) s -= t * (i64(p) - 1);
        s -= t * (i64(q) - 1);
        return s;
    }

    static inline u32 zero_[8] = {};
    std::vector<Dir> d_;
    std::vector<u64> bits_;
    FlatMap<u32> over_;
    Pool pool_;
    u32 cb_ = 0, cm_ = 0;
    u64 directed_ = 0, peak_directed_ = 0, slots_ = 0, peak_slots_ = 0, bytes_ = 0, peak_bytes_ = 0, peak_transient_ = 0, peak_over_ = 0;
};

// The table keeps the graph's wedges. The pair (u, v) must be absent from g when this runs.
inline void wt_upkeep(const Graph& g, u32 u, u32 v, bool ins, WTable& w, Cost& c) {
    const u64 before = c.total();
    w.change_list(u, g.nbr(v), ins, c);
    w.change_list(v, g.nbr(u), ins, c);
    c.upkeep += c.total() - before;
}

inline i64 edge_wt(Graph& g, u32 u, u32 v, bool ins, WTable& w, Cost& c) {
    c.graph += 2;
    if (ins) {
        i64 s = w.join(g, u, v, c);
        wt_upkeep(g, u, v, true, w, c);
        g.insert(u, v);
        return s;
    }
    g.erase(u, v);
    wt_upkeep(g, u, v, false, w, c);
    return -w.join(g, u, v, c);
}

// PW: the working state of a star on top of the table.
struct StarMemo {
    std::vector<u64> st;       // epoch, then bit 31 set for N0(x), low bits spoke position + 1
    std::vector<i64> q;        // the memo
    std::vector<u64> qv;       // epoch, then version
    u32 ep = 0;
    explicit StarMemo(u32 n) : st(n, 0), q(n, 0), qv(n, 0) {}
};

inline i64 star_pw(Graph& g, const GroupV& gr, bool ins, Wedge& w, StarMemo& m, Cost& c) {
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
    // A template for Gate 1, which recounts an extracted subgraph. It reads only deg and nbr.
    template <class G>
    bool count(const G& g, const std::vector<u32>& active, u64 budget, i64& result, u64& ops) {
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
enum class Arm { R2, R2B, WJ, PR, PW, RC, WT };

inline const char* arm_name(Arm a) {
    switch (a) {
        case Arm::R2: return "R2";
        case Arm::R2B: return "R2B";
        case Arm::WJ: return "WJ";
        case Arm::PR: return "PR";
        case Arm::PW: return "PW";
        case Arm::WT: return "WT";
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
    WTable wt;
    StarMemo memo;
    Active active;
    Recount rc;
    Grouper grouper;
    i64 t = 0;                  // the running count, or the last finished recount for RC
    bool t_known = true;        // false for RC after a censored batch, until the next finished recount
    u64 upkeep = 0, wedges = 0, max_update = 0, groups = 0, spokes = 0;
    u64 censored = 0, finished = 0;

    // Only what this arm uses is allocated, so that peak resident size is the arm's own.
    Engine5(Arm a, u32 n, u32 wt_count_bits = 0) : arm(a), g(n), row(a == Arm::R2 || a == Arm::PR ? n : 0), two(a == Arm::R2B ? n : 0),
                            wt(a == Arm::WT ? n : 0, wt_count_bits), memo(a == Arm::PW ? n : 0), active(a == Arm::RC ? n : 0), rc(a == Arm::RC ? n : 0),
                            grouper(a == Arm::PR || a == Arm::PW ? n : 0) {}

    // Wedges in the graph, sum of C(d, 2). Only what a rebuild of the table would cost, for the summariser. du and dv are read with the
    // pair absent. The degrees are free: this tally is not an operation of any arm.
    void track_wedges(u64 du, u64 dv, bool ins) {
#ifndef FC_TIMING
        if (ins) wedges += du + dv; else wedges -= du + dv;
#else
        (void)du; (void)dv; (void)ins;
#endif
    }

    void per_edge(const Edge& e, bool ins) {
        u64 before = c.total();
#ifndef FC_TIMING
        const u64 lose = ins ? 0 : 1;
        track_wedges(g.deg(e.a) - lose, g.deg(e.b) - lose, ins);
#endif
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
        } else if (arm == Arm::WT) {
            d = edge_wt(g, e.a, e.b, ins, wt, c);
        } else {
            d = edge_wj(g, e.a, e.b, ins, wedge, c);
        }
        t += d;
        max_update = std::max(max_update, c.total() - before);
    }

    u64 table_entries() const { return arm == Arm::WT ? wt.peak_entries() : wedge.peak_entries; }
    u64 table_slots() const { return arm == Arm::WT ? wt.peak_slots() : wedge.peak_slots; }
    u64 table_bytes() const { return arm == Arm::WT ? wt.peak_bytes() : wedge.peak_slots * Wedge::kSlotBytes; }
    u64 table_transient() const { return arm == Arm::WT ? wt.peak_transient() : wedge.peak_transient; }
    bool table_capped() const { return wedge.capped || wt.capped; }

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
                for (const GroupV& gr : grouper.run(ins ? nb.ins : nb.del)) {
                    if (gr.c.size() > 1) { ++groups; spokes += gr.c.size(); }
                    u64 g0 = c.total();
#ifndef FC_TIMING
                    u64 dx = g.deg(gr.x);
                    for (u32 v : gr.c) {
                        if (ins) { track_wedges(dx, g.deg(v), true); ++dx; }
                        else { --dx; track_wedges(dx, u64(g.deg(v)) - 1, false); }
                    }
#endif
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

u64 peak_rss() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof pmc)) return u64(pmc.PeakWorkingSetSize);
#endif
    return 0;
}

bool parse_arm(const std::string& s, Arm& a) {
    for (Arm x : {Arm::R2, Arm::R2B, Arm::WJ, Arm::PR, Arm::PW, Arm::RC, Arm::WT})
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

// One row per batch, held in memory and written after the clock stops, so that no file output sits in the timed path of any arm.
struct BatchRow {
    u64 raw = 0, net = 0, ops = 0, upkeep = 0, wedges = 0;
    i64 t = 0;
    int known = 0, checked = 0;
};

// Everything the writer reads. The watchdog thread uses it too, so that a run stopped by the clock still leaves its partial record.
struct Run {
    std::string stream, variant, base, arm, raw_path, sum_path;
    i64 cut = 0;
    u64 mult = 4, events = 0, net_total = 0, peak_m = 0, checks = 0;
    u32 n = 0;
    int rep = 0, cpu = -1;
    size_t batches = 0;
    double cap_seconds = 600.0;
    std::vector<BatchRow> rows;
    std::atomic<size_t> done{0};
    std::atomic<int> writing{0};
    Engine5* s = nullptr;
    std::chrono::steady_clock::time_point start, t0;
    double seconds() const { return std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count(); }
};

Run R;

bool write_outputs(const char* status, double secs) {
    if (R.writing.exchange(1)) return false;
    const size_t done = R.done.load(std::memory_order_acquire);
    FILE* out = std::fopen(R.raw_path.c_str(), "w");
    if (!out) { std::puts("cannot open raw output"); return false; }
    std::fprintf(out, "batch,raw,net,ops,upkeep,wedges,t,known,checked\n");
    for (size_t i = 0; i < done; ++i) {
        const BatchRow& r = R.rows[i];
        std::fprintf(out, "%zu,%llu,%llu,%llu,%llu,%llu,%lld,%d,%d\n", i, (unsigned long long)r.raw, (unsigned long long)r.net, (unsigned long long)r.ops,
                     (unsigned long long)r.upkeep, (unsigned long long)r.wedges, (long long)r.t, r.known, r.checked);
    }
    std::fclose(out);
    Engine5& s = *R.s;
    FILE* f = std::fopen(R.sum_path.c_str(), "w");
    if (!f) { std::puts("cannot open summary"); return false; }
    const Cost& c = s.c;
#ifdef FC_TIMING
    const char* build = "timing";
#else
    const char* build = "count";
#endif
    std::fprintf(f, "stream,variant,cut,arm,status,batches_done,batches,events,net,ops,scan,probe,row,deg,tab_read,tab_write,graph,net_ops,group,rc,"
                    "rc_censored,rc_finished,forced_checks,upkeep,peak_m,final_t,table_entries,table_slots,table_bytes,table_transient,"
                    "max_update,groups,spokes,peak_rss,seconds,budget_mult,vertices,build,rep,cpu\n");
    std::fprintf(f, "%s,%s,%lld,%s,%s,%zu,%zu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%lld,%llu,%llu,%llu,%llu,"
                    "%llu,%llu,%llu,%llu,%.3f,%llu,%u,%s,%d,%d\n",
                 R.stream.c_str(), R.variant.c_str(), (long long)R.cut, R.arm.c_str(), status, done, R.batches, (unsigned long long)R.events,
                 (unsigned long long)R.net_total, (unsigned long long)c.total(), (unsigned long long)c.scan, (unsigned long long)c.probe,
                 (unsigned long long)c.row, (unsigned long long)c.deg, (unsigned long long)c.tab_read, (unsigned long long)c.tab_write,
                 (unsigned long long)c.graph, (unsigned long long)c.net, (unsigned long long)c.group, (unsigned long long)c.rc,
                 (unsigned long long)s.censored, (unsigned long long)s.finished, (unsigned long long)R.checks, (unsigned long long)c.upkeep,
                 (unsigned long long)R.peak_m, (long long)(done ? R.rows[done - 1].t : 0), (unsigned long long)s.table_entries(), (unsigned long long)s.table_slots(),
                 (unsigned long long)s.table_bytes(), (unsigned long long)s.table_transient(), (unsigned long long)s.max_update, (unsigned long long)s.groups,
                 (unsigned long long)s.spokes, (unsigned long long)peak_rss(), secs, (unsigned long long)R.mult, R.n, build, R.rep, R.cpu);
    std::fclose(f);
    std::printf("%s %s status=%s done=%zu/%zu net=%llu ops=%llu t=%lld censored=%llu table=%llu secs=%.3f\n", R.base.c_str(), R.arm.c_str(), status, done, R.batches,
                (unsigned long long)R.net_total, (unsigned long long)c.total(), (long long)(done ? R.rows[done - 1].t : 0), (unsigned long long)s.censored,
                (unsigned long long)s.table_bytes(), secs);
    std::fflush(stdout);
    return true;
}

#ifdef _WIN32
// The clock limit. This thread sleeps through the run and never wakes before the limit, so it costs the run nothing.
DWORD WINAPI watchdog(void*) {
    Sleep(DWORD(R.cap_seconds * 1000.0));
    if (R.s && write_outputs("TIME", R.seconds())) { std::fflush(nullptr); _exit(3); }
    if (!R.s) { std::puts("time limit before the engine started"); std::fflush(nullptr); _exit(3); }
    return 0;
}

// The memory limit. The operating system refuses commit past this line and the next allocation throws, which the run loop catches.
bool limit_memory(u64 bytes) {
    HANDLE job = CreateJobObjectW(nullptr, nullptr);
    if (!job) return false;
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION li{};
    li.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_PROCESS_MEMORY;
    li.ProcessMemoryLimit = SIZE_T(bytes);
    if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &li, sizeof li)) return false;
    return AssignProcessToJobObject(job, GetCurrentProcess()) != 0;
}
#endif

}  // namespace

int main(int argc, char** argv) {
    if (argc < 7) {
        std::puts("usage: r11_fivecycle events.bin stream insert|expiry cut arm outdir [budget multiple] [--cap-seconds S] [--cap-bytes B] [--cpu N] [--rep N]");
        return 2;
    }
    R.start = std::chrono::steady_clock::now();
    const std::string path = argv[1], outdir = argv[6];
    R.stream = argv[2];
    R.variant = argv[3];
    R.cut = std::atoll(argv[4]);
    R.arm = argv[5];
    Arm arm;
    if (!parse_arm(argv[5], arm)) { std::puts("unknown arm"); return 2; }
    u64 cap_bytes = 6000000000ull;
    for (int i = 7; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--cap-seconds" && i + 1 < argc) R.cap_seconds = std::atof(argv[++i]);
        else if (a == "--cap-bytes" && i + 1 < argc) cap_bytes = std::strtoull(argv[++i], nullptr, 10);
        else if (a == "--cpu" && i + 1 < argc) R.cpu = std::atoi(argv[++i]);
        else if (a == "--rep" && i + 1 < argc) R.rep = std::atoi(argv[++i]);
        else R.mult = std::strtoull(argv[i], nullptr, 10);
    }
#ifdef _WIN32
    if (!limit_memory(cap_bytes)) { std::puts("cannot set the memory limit"); return 2; }
    if (R.cpu >= 0 && !SetProcessAffinityMask(GetCurrentProcess(), DWORD_PTR(1) << R.cpu)) { std::puts("cannot set affinity"); return 2; }
    HANDLE dog = CreateThread(nullptr, 0, watchdog, nullptr, 0, nullptr);
    if (!dog) { std::puts("cannot start the time limit"); return 2; }
#endif

    FILE* in = std::fopen(path.c_str(), "rb");
    if (!in) { std::puts("cannot open events"); return 2; }
    std::fseek(in, 0, SEEK_END);
    long long bytes = _ftelli64(in);
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
    R.raw_path = outdir + "/raw/" + R.base + "-" + R.arm + suffix + ".csv";
    R.sum_path = outdir + "/raw/" + R.base + "-" + R.arm + suffix + ".summary.csv";
    std::vector<std::vector<u64>> prior;
    if (arm == Arm::RC)
        for (const char* name : {"R2", "WJ", "PR"}) prior.push_back(read_batch_ops(outdir + "/raw/" + R.base + "-" + name + ".csv"));

    auto engine = std::make_unique<Engine5>(arm, R.n);
    Engine5& s = *engine;
    R.s = engine.get();
    Normaliser norm;

    const size_t check_every = std::max<size_t>(1, R.batches / 20);
    std::vector<Update> b;
    const char* status = "OK";
    R.t0 = std::chrono::steady_clock::now();
    try {
        for (size_t bi = 0; bi < R.batches; ++bi) {
            b.clear();
            for (size_t i = starts[bi]; i < starts[bi + 1]; ++i) b.push_back({ev[i].u, ev[i].v, ev[i].op == 1});
            const NetBatch& nb = norm.run(s.g, b);
            const u64 net = nb.ins.size() + nb.del.size();
            R.net_total += net;
            u64 budget = ~u64(0);
            if (arm == Arm::RC) {
                u64 top = 0;
                bool any = false;
                for (const auto& p : prior)
                    if (bi < p.size()) { top = std::max(top, p[bi]); any = true; }
                if (any) budget = R.mult * top;
            }
            const u64 up0 = s.c.upkeep;
            const u64 ops = s.apply(b, nb, budget);
            BatchRow& row = R.rows[bi];
            row.upkeep = u64(s.c.upkeep) - up0;
            R.peak_m = std::max(R.peak_m, s.g.m());
            if (arm == Arm::RC && (bi % check_every == 0 || bi + 1 == R.batches) && !s.t_known && R.seconds() < 400) {
                u64 sink = 0;
                i64 r = 0;
                if (s.rc.count(s.g, s.active.list, ~u64(0), r, sink)) {
                    s.t = r;
                    s.t_known = true;
                    row.checked = 1;
                    ++R.checks;
                }
            }
            row.raw = b.size();
            row.net = net;
            row.ops = ops;
            row.wedges = s.wedges;
            row.t = s.t;
            row.known = int(s.t_known);
            R.done.store(bi + 1, std::memory_order_release);
            if (s.table_capped()) { status = "CAP"; break; }
        }
    } catch (const std::bad_alloc&) {
        status = "MEM";
    }
    const double secs = R.seconds();
    if (!write_outputs(status, secs)) Sleep(5000);  // the watchdog holds the writer, and it exits the process
    return status[0] == 'O' ? 0 : 3;
}

#endif  // FIVECYCLE_NO_MAIN
