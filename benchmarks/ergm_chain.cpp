// M1B03 kill test: Metropolis TNT chain on edges plus alternating k-triangle, with three cn backends.
// Protocol: docs/experiments/screen-b-protocol.md, section M1B03. Source idea: docs/literature/scouts/
// s3-structured-workloads.md, Candidate 1. Results: results/screen-b-M1B03/. Non-citable.
//
//   ergm_chain step1 <out.csv>
//   ergm_chain selftest [seed]
//   ergm_chain run key=value ...       see parse_run()
//
// Change statistic: EstimNetDirected src/changeStatisticsUndirected.c, changeAltKTriangles, master branch read on
// 2026-11-01. For adding i-j to a graph that lacks it, with p = 1 - 1/lambda:
//   delta = sum_{v in N(i) & N(j)} (p^cn(i,v) + p^cn(j,v)) + lambda (1 - p^cn(i,j))
// The full statistic behind it is lambda * sum_{edges ab} (1 - p^cn(a,b)). selftest checks one against the other.
//
// Proposal: EstimNetDirected src/tntSampler.c. Add or delete with probability 1/2 each, a uniform edge for a delete,
// a uniform non-edge by rejection for an add. The Hastings ratio here is the exact one for that proposal, including
// the empty and full graph; EstimNet reuses the statnet expression, which is a close approximation when sparse.
//
// EstimNet removes the edge before scoring a delete and reinserts it on rejection, so its table pays a commit for
// every delete proposal. Here a delete is scored on the graph as it stands with cn(i,v) - 1 for each common
// neighbour v, because j is a common neighbour of i and v. All three backends get that benefit.
#include <bit>
#include <chrono>
#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <memory>
#include <random>
#include <string>

#include "../src/r11/tc.hpp"

using namespace r11;

namespace {

[[noreturn]] void die(const std::string& why) {
    std::fprintf(stderr, "FATAL: %s\n", why.c_str());
    std::fflush(stderr);
    std::exit(3);
}

#pragma pack(push, 1)
struct Event { std::int64_t t; u32 u, v, op; };
#pragma pack(pop)
static_assert(sizeof(Event) == 20);

// xoshiro256** seeded by splitmix64, so the stream does not depend on the C++ library.
class Rng {
public:
    explicit Rng(u64 seed) {
        for (auto& w : s_) {
            seed += 0x9E3779B97F4A7C15ull;
            u64 z = seed;
            z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
            z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
            w = z ^ (z >> 31);
        }
    }
    u64 next() {
        u64 r = std::rotl(s_[1] * 5, 7) * 9, t = s_[1] << 17;
        s_[2] ^= s_[0]; s_[3] ^= s_[1]; s_[1] ^= s_[2]; s_[0] ^= s_[3]; s_[2] ^= t;
        s_[3] = std::rotl(s_[3], 45);
        return r;
    }
    double unif() { return (double(next() >> 11) + 0.5) * (1.0 / 9007199254740992.0); }   // open (0, 1)
    u64 below(u64 n) { return u64((static_cast<unsigned __int128>(next()) * n) >> 64); }

private:
    u64 s_[4];
};

// ---------------------------------------------------------------------------------------------------------------
// Graphs
// ---------------------------------------------------------------------------------------------------------------

struct Snapshot {
    std::string name, label;
    u32 n = 0;                  // active vertices, compacted
    std::vector<Edge> edges;
    i64 peak_event = -1, peak_time = 0;
};

std::vector<Event> read_events(const std::string& path) {
    FILE* in = std::fopen(path.c_str(), "rb");
    if (!in) die("cannot open " + path);
    _fseeki64(in, 0, SEEK_END);
    long long bytes = _ftelli64(in);
    _fseeki64(in, 0, SEEK_SET);
    if (bytes % sizeof(Event)) die("size of " + path + " is not a multiple of 20");
    std::vector<Event> ev(size_t(bytes) / sizeof(Event));
    if (std::fread(ev.data(), sizeof(Event), ev.size(), in) != ev.size()) die("short read " + path);
    std::fclose(in);
    return ev;
}

void compact(Snapshot& s, std::vector<u64>& keys) {
    std::sort(keys.begin(), keys.end());
    keys.erase(std::unique(keys.begin(), keys.end()), keys.end());
    std::vector<u32> ids;
    ids.reserve(keys.size() * 2);
    for (u64 k : keys) { ids.push_back(u32(k >> 32)); ids.push_back(u32(k)); }
    std::sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
    s.n = u32(ids.size());
    s.edges.clear();
    s.edges.reserve(keys.size());
    for (u64 k : keys) {
        u32 a = u32(std::lower_bound(ids.begin(), ids.end(), u32(k >> 32)) - ids.begin());
        u32 b = u32(std::lower_bound(ids.begin(), ids.end(), u32(k)) - ids.begin());
        s.edges.push_back(canon(a, b));
    }
}

// Set semantics, as in docs/experiments/gate0-protocol.md: a duplicate insert and a delete of an absent edge are
// no-ops. The peak is the first event at which the edge count is largest.
Snapshot load_window(const std::string& path, const std::string& name) {
    std::vector<Event> ev = read_events(path);
    FlatMap<u32> live;
    u64 best = 0;
    size_t best_at = 0;
    for (size_t i = 0; i < ev.size(); ++i) {
        const Event& x = ev[i];
        if (x.u == x.v) continue;
        u64 k = key(canon(x.u, x.v));
        bool has = live.find(k) != nullptr;
        if (x.op == 1 && !has) live.emplace(k, 1);
        if (x.op == 0 && has) live.erase(k);
        if (live.size() > best) { best = live.size(); best_at = i; }
    }
    FlatMap<u32> at;
    for (size_t i = 0; i <= best_at; ++i) {
        const Event& x = ev[i];
        if (x.u == x.v) continue;
        u64 k = key(canon(x.u, x.v));
        bool has = at.find(k) != nullptr;
        if (x.op == 1 && !has) at.emplace(k, 1);
        if (x.op == 0 && has) at.erase(k);
    }
    if (at.size() != best) die("peak replay disagrees for " + name);
    std::vector<u64> keys;
    keys.reserve(best);
    at.for_each([&](u64 k, u32&) { keys.push_back(k); });
    Snapshot s;
    s.name = name + "-peak";
    s.label = "real timestamps; synthetic expiry";
    s.peak_event = i64(best_at);
    s.peak_time = ev[best_at].t;
    compact(s, keys);
    return s;
}

Snapshot load_ins(const std::string& path, const std::string& name) {
    std::vector<Event> ev = read_events(path);
    std::vector<u64> keys;
    keys.reserve(ev.size());
    for (const Event& x : ev) {
        if (x.op != 1) die("insert-only file holds a delete: " + path);
        if (x.u != x.v) keys.push_back(key(canon(x.u, x.v)));
    }
    Snapshot s;
    s.name = name + "-ins-end";
    s.label = "real timestamps; insert-only";
    compact(s, keys);
    return s;
}

// Graph500 parameters. Self-loops and repeats are dropped, direction is dropped.
Snapshot gen_rmat(int scale, int factor, u64 seed) {
    constexpr double A = 0.57, B = 0.19, C = 0.19;
    Rng r(seed);
    u64 samples = u64(factor) << scale;
    std::vector<u64> keys;
    keys.reserve(samples);
    for (u64 i = 0; i < samples; ++i) {
        u32 u = 0, v = 0;
        for (int b = 0; b < scale; ++b) {
            double x = r.unif();
            u <<= 1; v <<= 1;
            if (x < A) {}
            else if (x < A + B) v |= 1;
            else if (x < A + B + C) u |= 1;
            else { u |= 1; v |= 1; }
        }
        if (u != v) keys.push_back(key(canon(u, v)));
    }
    Snapshot s;
    s.name = "rmat-s" + std::to_string(scale) + "-f" + std::to_string(factor);
    s.label = "fully synthetic";
    compact(s, keys);
    return s;
}

Snapshot load_spec(const std::string& spec) {
    const std::string dir = "data/derived/";
    if (spec == "su-peak") return load_window(dir + "sx-superuser.bin", "sx-superuser");
    if (spec == "wiki-peak") return load_window(dir + "wiki-talk-temporal.bin", "wiki-talk-temporal");
    if (spec == "su-ins") return load_ins(dir + "sx-superuser.ins.bin", "sx-superuser");
    if (spec == "wiki-ins") return load_ins(dir + "wiki-talk-temporal.ins.bin", "wiki-talk-temporal");
    if (spec.rfind("rmat:", 0) == 0) {
        int scale = 0, factor = 0;
        unsigned long long seed = 0;
        if (std::sscanf(spec.c_str(), "rmat:%d:%d:%llu", &scale, &factor, &seed) != 3) die("bad rmat spec");
        return gen_rmat(scale, factor, seed);
    }
    die("unknown graph " + spec);
}

// ---------------------------------------------------------------------------------------------------------------
// Step 1
// ---------------------------------------------------------------------------------------------------------------

void step1_row(FILE* f, const Snapshot& s) {
    std::vector<u32> deg(s.n, 0);
    for (Edge e : s.edges) { ++deg[e.a]; ++deg[e.b]; }
    std::sort(deg.begin(), deg.end(), std::greater<u32>());
    auto c2 = [](u64 d) { return d * (d - 1) / 2; };
    u64 total = 0;
    for (u32 d : deg) total += c2(d);
    u32 dmax = deg.empty() ? 0 : deg[0];
    for (u64 h = 64; h <= 4096; h *= 2) {
        u32 tau = h < s.n ? deg[h] : 0;     // largest degree in L
        u64 strict = 0, in_l = 0;
        for (size_t i = 0; i < deg.size(); ++i) {
            if (deg[i] < tau) strict += c2(deg[i]);
            if (i >= h) in_l += c2(deg[i]);
        }
        std::fprintf(f, "%s,%s,%u,%zu,%u,%llu,%llu,%u,%llu,%llu,%.6f,%.6f\n", s.name.c_str(), s.label.c_str(), s.n,
                     s.edges.size(), dmax, (unsigned long long)total, (unsigned long long)h, tau,
                     (unsigned long long)strict, (unsigned long long)in_l, double(strict) / double(total),
                     double(in_l) / double(total));
    }
    std::printf("%s n=%u m=%zu dmax=%u E_inf=%llu peak_event=%lld peak_time=%lld\n", s.name.c_str(), s.n,
                s.edges.size(), dmax, (unsigned long long)total, (long long)s.peak_event, (long long)s.peak_time);
    std::fflush(stdout);
}

// ---------------------------------------------------------------------------------------------------------------
// Costs
// ---------------------------------------------------------------------------------------------------------------

// total() is the strict protocol count: a read-modify-write is one read plus one write, and an AND of two mask words
// is two word reads. alt() is the lenient count: each of those is one access. Both are reported.
struct Cost {
    u64 list = 0, probe = 0, tread = 0, twrite = 0, stamp = 0, word = 0, edit = 0;
    u64 rmw = 0, maskpair = 0;
    u64 total() const { return list + probe + tread + twrite + stamp + word + edit; }
    u64 alt() const { return total() - rmw - maskpair; }
};

// ---------------------------------------------------------------------------------------------------------------
// Backends
// ---------------------------------------------------------------------------------------------------------------

struct Params {
    double theta_e = 0, theta_t = 0, lambda = 2;
    bool early = true;
    const std::vector<double>* pw = nullptr;    // pw[k] = (1 - 1/lambda)^k
};

struct Outcome { bool accept; double delta; u32 c; bool full; };

class Backend {
public:
    virtual ~Backend() = default;
    std::string name;
    u32 hsize = 0, memo_min = 0;
    u64 memo_slots = 0;
    Cost cost;
    u64 build_ops = 0, peak_entries = 0;
    bool skip_add_dyad = false;

    virtual i64 known_cn(const Graph& g, u32 i, u32 j) = 0;     // value, or -1 when not known without a scan
    virtual void store_cn(const Graph&, u32, u32, u32) {}
    virtual u32 cn(const Graph& g, u32 a, u32 b) = 0;
    virtual void commit(const Graph& g, u32 i, u32 j, bool add) = 0;    // g is still the graph before the edit
    virtual u64 entries() const { return 0; }
    virtual double index_bytes() const { return 0; }
    virtual void verify(const Graph&) {}

    // Scores one proposal. `need` is what sgn * theta_t * delta must exceed for acceptance. A rejection may stop as
    // soon as the bound on delta proves it. An acceptance always finishes, since the running statistic needs delta.
    Outcome decide(const Graph& g, u32 i, u32 j, bool present, const Params& p, double need) {
        const std::vector<double>& pw = *p.pw;
        const double sg = (present ? -1.0 : 1.0) * p.theta_t;
        const double margin = 1e-9 * (1.0 + std::fabs(need));
        auto rejected = [&](double lo, double hi) {
            double best = sg > 0 ? sg * hi : sg * lo;
            return p.early && best <= need - margin;
        };
        u32 s = g.deg(i) <= g.deg(j) ? i : j, t = s == i ? j : i;
        const bool use_dyad = present || !skip_add_dyad;   // a random non-edge almost never comes back
        i64 kn = use_dyad ? known_cn(g, i, j) : -1;
        double base = 0;
        if (kn == 0) return {0.0 > need, 0.0, 0, true};
        if (kn > 0) {
            base = p.lambda * (1.0 - pw[size_t(kn)]);
            if (rejected(base, base + 2.0 * double(kn))) return {false, 0.0, 0, false};
        }
        common_.clear();
        for (u32 w : g.nbr(s)) {
            ++cost.list;
            if (w == t) continue;
            ++cost.probe;
            if (g.has(w, t)) {
                common_.push_back(w);
                if (kn > 0 && i64(common_.size()) == kn) break;
            }
        }
        u32 c = u32(common_.size());
        if (kn < 0) {
            if (use_dyad) store_cn(g, i, j, c);
            if (c == 0) return {0.0 > need, 0.0, 0, true};
            base = p.lambda * (1.0 - pw[c]);
            if (rejected(base, base + 2.0 * double(c))) return {false, 0.0, 0, false};
        } else if (i64(c) != kn) {
            die(name + ": table says cn " + std::to_string(kn) + ", scan found " + std::to_string(c));
        }
        double sum = 0;
        u32 sub = present ? 1 : 0, rem = c;
        for (u32 v : common_) {
            u32 ci = cn(g, i, v), cj = cn(g, j, v);
            if (ci < sub || cj < sub) die(name + ": cn below the correction");
            sum += pw[ci - sub] + pw[cj - sub];
            --rem;
            if (rejected(base + sum, base + sum + 2.0 * double(rem))) return {false, 0.0, c, false};
        }
        double delta = sum + base;
        return {sg * delta > need, delta, c, true};
    }

private:
    std::vector<u32> common_;
};

// Backend (a). Lower-degree scan, copy of Engine::common in tc.hpp, plus a direct-mapped memo. A slot holds the dyad,
// cn and the version of each endpoint at store time. An edit a-b changes cn only for pairs that hold a or b, so
// bumping those two versions is exact. Pairs with min degree below memo_min skip the memo: a scan of one or two
// elements costs no more than the lookup.
class ScanBackend : public Backend {
public:
    ScanBackend(const Graph& g, u64 slots, u32 min_deg) : ver_(g.n(), 0) {
        name = "scan";
        memo_min = min_deg;
        memo_slots = slots;
        if (slots) {
            u64 cap = 1;
            int bits = 0;
            while (cap < slots) { cap <<= 1; ++bits; }
            memo_slots = cap;
            shift_ = 64 - bits;
            memo_.assign(cap, Slot{~u64(0), 0, 0, 0});
        }
    }
    u64 hits = 0, misses = 0, bypass = 0;

    i64 known_cn(const Graph& g, u32 i, u32 j) override {
        if (memo_.empty() || std::min(g.deg(i), g.deg(j)) < memo_min) return -1;
        return lookup(i, j);
    }
    void store_cn(const Graph& g, u32 i, u32 j, u32 c) override {
        if (memo_.empty() || std::min(g.deg(i), g.deg(j)) < memo_min) return;
        put(i, j, c);
    }
    u32 cn(const Graph& g, u32 a, u32 b) override {
        if (memo_.empty() || std::min(g.deg(a), g.deg(b)) < memo_min) { ++bypass; return common(g, a, b); }
        i64 r = lookup(a, b);
        if (r >= 0) return u32(r);
        u32 c = common(g, a, b);
        put(a, b, c);
        return c;
    }
    void commit(const Graph&, u32 i, u32 j, bool) override {
        if (memo_.empty()) return;
        ++ver_[i]; ++ver_[j];
        cost.stamp += 4;
        cost.rmw += 2;
    }
    double index_bytes() const override { return memo_.empty() ? 0.0 : double(memo_slots) * 24.0 + double(ver_.size()) * 4.0; }
    u64 entries() const override { return memo_slots; }

private:
    struct Slot { u64 k; u32 c, va, vb; };

    u32 common(const Graph& g, u32 u, u32 v) {
        u32 s = g.deg(u) <= g.deg(v) ? u : v, t = s == u ? v : u;
        u32 c = 0;
        for (u32 w : g.nbr(s)) {
            ++cost.list; ++cost.probe;
            if (g.has(w, t)) ++c;
        }
        return c;
    }
    u64 slot(u64 k) const { return ((k ^ (k >> 29)) * 0x9E3779B97F4A7C15ull) >> shift_; }
    i64 lookup(u32 a, u32 b) {
        Edge e = canon(a, b);
        u64 k = key(e);
        ++cost.tread;
        const Slot& s = memo_[slot(k)];
        if (s.k != k) { ++misses; return -1; }
        cost.stamp += 2;
        if (s.va != ver_[e.a] || s.vb != ver_[e.b]) { ++misses; return -1; }
        ++hits;
        return s.c;
    }
    void put(u32 a, u32 b, u32 c) {
        Edge e = canon(a, b);
        u64 k = key(e);
        ++cost.twrite;
        cost.stamp += 2;
        memo_[slot(k)] = Slot{k, c, ver_[e.a], ver_[e.b]};
    }
    std::vector<Slot> memo_;
    std::vector<u32> ver_;
    int shift_ = 64;
};

// Shared by (b) and (c): the wedge table, zero entries erased.
class WedgeTable {
public:
    u32 get(u32 a, u32 b) const {
        const u32* p = map_.find(key(canon(a, b)));
        return p ? *p : 0;
    }
    void bump(u32 a, u32 b, bool add) {
        u64 k = key(canon(a, b));
        u32* p = map_.find(k);
        if (add) {
            if (p) ++*p; else map_.emplace(k, 1);
        } else {
            if (!p) die("wedge table underflow");
            if (--*p == 0) map_.erase(k);
        }
    }
    u64 size() const { return map_.size(); }
    FlatMap<u32>& raw() { return map_; }

private:
    FlatMap<u32> map_;
};

// Backend (b). All wedges. Read cn in one access, commit over both neighbourhoods.
class EagerBackend : public Backend {
public:
    explicit EagerBackend(const Graph& g) {
        name = "eager";
        for (u32 x = 0; x < g.n(); ++x) {
            const auto& nb = g.nbr(x);
            for (size_t a = 0; a < nb.size(); ++a)
                for (size_t b = a + 1; b < nb.size(); ++b) { t_.bump(nb[a], nb[b], true); build_ops += 3; }
        }
        peak_entries = t_.size();
    }
    i64 known_cn(const Graph&, u32 i, u32 j) override { ++cost.tread; return t_.get(i, j); }
    u32 cn(const Graph&, u32 a, u32 b) override { ++cost.tread; return t_.get(a, b); }
    void commit(const Graph& g, u32 i, u32 j, bool add) override {
        for (u32 w : g.nbr(j)) { ++cost.list; if (w != i) up(i, w, add); }
        for (u32 w : g.nbr(i)) { ++cost.list; if (w != j) up(j, w, add); }
        peak_entries = std::max(peak_entries, t_.size());
    }
    u64 entries() const override { return t_.size(); }
    double index_bytes() const override { return double(peak_entries) * 12.0; }
    void verify(const Graph& g) override {
        WedgeTable r;
        for (u32 x = 0; x < g.n(); ++x) {
            const auto& nb = g.nbr(x);
            for (size_t a = 0; a < nb.size(); ++a)
                for (size_t b = a + 1; b < nb.size(); ++b) r.bump(nb[a], nb[b], true);
        }
        if (r.size() != t_.size()) die("eager table size differs from rebuild");
        r.raw().for_each([&](u64 k, u32& v) {
            if (t_.get(u32(k >> 32), u32(k)) != v) die("eager table value differs from rebuild");
        });
    }

private:
    void up(u32 a, u32 b, bool add) { ++cost.tread; ++cost.twrite; ++cost.rmw; t_.bump(a, b, add); }
    WedgeTable t_;
};

// Backend (c), the scout's table. Hubs are the top hsize vertices by degree when the backend is built, ties by id.
class SplitBackend : public Backend {
public:
    SplitBackend(const Graph& g, u32 h) : hid_(g.n(), -1) {
        name = "split";
        hsize = std::min(h, g.n());
        words_ = std::max<u32>(1, (hsize + 63) / 64);
        std::vector<u32> ord(g.n());
        std::iota(ord.begin(), ord.end(), 0u);
        std::sort(ord.begin(), ord.end(), [&](u32 a, u32 b) { return g.deg(a) != g.deg(b) ? g.deg(a) > g.deg(b) : a < b; });
        for (u32 i = 0; i < hsize; ++i) hid_[ord[i]] = i32(i);
        bits_.assign(u64(g.n()) * words_, 0);
        for (u32 v = 0; v < g.n(); ++v)
            for (u32 w : g.nbr(v)) {
                build_ops += 1;
                if (hid_[w] >= 0) { flip(v, w); build_ops += 2; }
            }
        for (u32 x = 0; x < g.n(); ++x) {
            if (hid_[x] >= 0) continue;
            const auto& nb = g.nbr(x);
            for (size_t a = 0; a < nb.size(); ++a)
                for (size_t b = a + 1; b < nb.size(); ++b) { t_.bump(nb[a], nb[b], true); build_ops += 3; }
        }
        peak_entries = t_.size();
    }
    i64 known_cn(const Graph&, u32 i, u32 j) override { return read(i, j); }
    u32 cn(const Graph&, u32 a, u32 b) override { return read(a, b); }
    void commit(const Graph& g, u32 i, u32 j, bool add) override {
        if (hid_[j] < 0)
            for (u32 w : g.nbr(j)) { ++cost.list; if (w != i) up(i, w, add); }
        if (hid_[i] < 0)
            for (u32 w : g.nbr(i)) { ++cost.list; if (w != j) up(j, w, add); }
        if (hid_[j] >= 0) { flip(i, j); cost.word += 2; ++cost.rmw; }
        if (hid_[i] >= 0) { flip(j, i); cost.word += 2; ++cost.rmw; }
        peak_entries = std::max(peak_entries, t_.size());
    }
    u32 peek(u32 a, u32 b) const { return raw(a, b); }
    u64 entries() const override { return t_.size(); }
    double index_bytes() const override {
        return double(peak_entries) * 12.0 + double(hid_.size()) * double(words_) * 8.0 + double(hid_.size());
    }
    void verify(const Graph& g) override {
        WedgeTable r;
        for (u32 x = 0; x < g.n(); ++x) {
            if (hid_[x] >= 0) continue;
            const auto& nb = g.nbr(x);
            for (size_t a = 0; a < nb.size(); ++a)
                for (size_t b = a + 1; b < nb.size(); ++b) r.bump(nb[a], nb[b], true);
        }
        if (r.size() != t_.size()) die("split table size differs from rebuild");
        r.raw().for_each([&](u64 k, u32& v) {
            if (t_.get(u32(k >> 32), u32(k)) != v) die("split table value differs from rebuild");
        });
        for (u32 v = 0; v < g.n(); ++v) {
            u64 pop = 0, want = 0;
            for (u32 w = 0; w < words_; ++w) pop += u64(std::popcount(bits_[u64(v) * words_ + w]));
            for (u32 w : g.nbr(v)) want += hid_[w] >= 0;
            if (pop != want) die("split mask differs from adjacency");
        }
    }

private:
    using i32 = std::int32_t;
    void flip(u32 v, u32 hub) {
        u32 b = u32(hid_[hub]);
        bits_[u64(v) * words_ + b / 64] ^= u64(1) << (b % 64);
    }
    u32 raw(u32 a, u32 b) const {
        u32 r = t_.get(a, b);
        const u64* x = &bits_[u64(a) * words_];
        const u64* y = &bits_[u64(b) * words_];
        for (u32 w = 0; w < words_; ++w) r += u32(std::popcount(x[w] & y[w]));
        return r;
    }
    u32 read(u32 a, u32 b) {
        ++cost.tread;
        cost.word += 2 * words_;
        cost.maskpair += words_;
        return raw(a, b);
    }
    void up(u32 a, u32 b, bool add) { ++cost.tread; ++cost.twrite; ++cost.rmw; t_.bump(a, b, add); }

    std::vector<i32> hid_;
    std::vector<u64> bits_;
    WedgeTable t_;
    u32 words_ = 1;
};

// Backend (b) for a state whose table does not fit in RAM. It charges what EagerBackend charges, call by call, and
// takes the values from a split table without charge. Its decisions agree by construction, so a row from it is
// evidence about cost only, and its table size is the start count from count_dyads().
class FormulaEager : public Backend {
public:
    FormulaEager(const SplitBackend* truth, u64 start_entries, u64 wedges) : truth_(truth) {
        name = "eager-formula";
        peak_entries = start_entries;
        build_ops = 3 * wedges;
    }
    i64 known_cn(const Graph&, u32 i, u32 j) override { ++cost.tread; return truth_->peek(i, j); }
    u32 cn(const Graph&, u32 a, u32 b) override { ++cost.tread; return truth_->peek(a, b); }
    void commit(const Graph& g, u32 i, u32 j, bool) override {
        u64 di = g.nbr(i).size(), dj = g.nbr(j).size();
        u64 touched = di + dj;
        u64 up = touched - (g.has(i, j) ? 2 : 0);
        cost.list += touched;
        cost.tread += up; cost.twrite += up; cost.rmw += up;
    }
    double index_bytes() const override { return double(peak_entries) * 12.0; }
    u64 entries() const override { return peak_entries; }

private:
    const SplitBackend* truth_;
};

// Backend (c) for an |H| whose table does not fit in free RAM. Same charges as SplitBackend call by call, with values
// from a real split table of another |H|, without charge. Its entry count is the start count from a stamped row pass.
class FormulaSplit : public Backend {
public:
    FormulaSplit(const Graph& g, u32 h, const SplitBackend* truth) : truth_(truth), hub_(g.n(), 0) {
        name = "split-formula";
        hsize = std::min(h, g.n());
        words_ = std::max<u32>(1, (hsize + 63) / 64);
        std::vector<u32> ord(g.n());
        std::iota(ord.begin(), ord.end(), 0u);
        std::sort(ord.begin(), ord.end(), [&](u32 a, u32 b) { return g.deg(a) != g.deg(b) ? g.deg(a) > g.deg(b) : a < b; });
        for (u32 i = 0; i < hsize; ++i) hub_[ord[i]] = 1;
        std::vector<u32> stamp(g.n(), 0xffffffffu);
        u64 wedges = 0, dyads = 0;
        for (u32 u = 0; u < g.n(); ++u) {
            build_ops += g.deg(u);
            for (u32 x : g.nbr(u)) {
                if (hub_[x]) { build_ops += 2; continue; }
                wedges += g.deg(x) - 1;
                for (u32 w : g.nbr(x))
                    if (w > u && stamp[w] != u) { stamp[w] = u; ++dyads; }
            }
        }
        build_ops += 3 * (wedges / 2);
        peak_entries = dyads;
    }
    i64 known_cn(const Graph&, u32 i, u32 j) override { charge(); return truth_->peek(i, j); }
    u32 cn(const Graph&, u32 a, u32 b) override { charge(); return truth_->peek(a, b); }
    void commit(const Graph& g, u32 i, u32 j, bool add) override {
        u64 skip = add ? 0 : 1;
        if (!hub_[j]) { u64 d = g.nbr(j).size(), up = d - skip; cost.list += d; cost.tread += up; cost.twrite += up; cost.rmw += up; }
        if (!hub_[i]) { u64 d = g.nbr(i).size(), up = d - skip; cost.list += d; cost.tread += up; cost.twrite += up; cost.rmw += up; }
        if (hub_[j]) { cost.word += 2; ++cost.rmw; }
        if (hub_[i]) { cost.word += 2; ++cost.rmw; }
    }
    u64 entries() const override { return peak_entries; }
    double index_bytes() const override {
        return double(peak_entries) * 12.0 + double(hub_.size()) * double(words_) * 8.0 + double(hub_.size());
    }

private:
    void charge() { ++cost.tread; cost.word += 2 * words_; cost.maskpair += words_; }
    const SplitBackend* truth_;
    std::vector<unsigned char> hub_;
    u32 words_ = 1;
};

// Distinct dyads with at least one shared partner, by one stamped row per vertex. Uses no table.
u64 count_dyads(const Graph& g, u64& wedges) {
    std::vector<u32> stamp(g.n(), 0xffffffffu);
    u64 d = 0;
    wedges = 0;
    for (u32 u = 0; u < g.n(); ++u) {
        for (u32 x : g.nbr(u)) {
            wedges += g.deg(x) - 1;
            for (u32 w : g.nbr(x))
                if (w > u && stamp[w] != u) { stamp[w] = u; ++d; }
        }
    }
    wedges /= 2;
    return d;
}

// ---------------------------------------------------------------------------------------------------------------
// Chain
// ---------------------------------------------------------------------------------------------------------------

class Chain {
public:
    Chain(const Snapshot& s, double te, double tt, double lambda, bool early, u64 seed)
        : g(s.n), rng(seed), n_(s.n), dyads_(double(s.n) * (double(s.n) - 1.0) / 2.0) {
        for (Edge e : s.edges) add_edge(e);
        pw_.resize(size_t(s.n) + 3);
        double q = 1.0 - 1.0 / lambda, x = 1.0;
        for (auto& v : pw_) { v = x; x *= q; }
        par.theta_e = te;
        par.theta_t = tt;
        par.lambda = lambda;
        par.early = early;
        par.pw = &pw_;
        recount(stat, tri);
    }

    Graph g;
    Rng rng;
    Params par;
    Cost shared;
    double stat = 0;        // alternating k-triangle statistic, running
    i64 tri = 0;            // triangles, running
    u64 accepted = 0, add_acc = 0, del_acc = 0, add_prop = 0, del_prop = 0, steps = 0;
    u64 checks = 0, static_checks = 0, verify_limit = ~u64(0);   // rebuild-and-compare only tables up to this size
    double max_stat_err = 0;
    u64 hash = 0xcbf29ce484222325ull;   // FNV-1a over the accept bits and dyads
    u64 c_sum = 0, c_seen = 0, c_pos = 0;

    double log_ratio(bool del) const {
        double m = double(el_.size()), d = dyads_;
        auto pdel = [&](double e) { return e <= 0 ? 0.0 : e >= d ? 1.0 : 0.5; };
        if (del) {
            double fwd = pdel(m) / m, back = (1.0 - pdel(m - 1)) / (d - m + 1);
            return std::log(back / fwd);
        }
        double fwd = (1.0 - pdel(m)) / (d - m), back = pdel(m + 1) / (m + 1);
        return std::log(back / fwd);
    }

    void step(const std::vector<Backend*>& bs) {
        u64 m = el_.size();
        double r = rng.unif();
        bool del = m > 0 && (double(m) >= dyads_ || r < 0.5);
        u32 i, j;
        if (del) {
            Edge e = el_[size_t(rng.below(m))];
            ++shared.tread;
            i = e.a; j = e.b;
            ++del_prop;
        } else {
            for (;;) {
                i = u32(rng.below(n_));
                j = u32(rng.below(n_));
                if (i == j) continue;
                ++shared.probe;
                if (!g.has(i, j)) break;
            }
            ++add_prop;
        }
        double sgn = del ? -1.0 : 1.0;
        double need = std::log(rng.unif()) - log_ratio(del) - sgn * par.theta_e;
        Outcome o0 = bs[0]->decide(g, i, j, del, par, need);
        for (size_t b = 1; b < bs.size(); ++b) {
            Outcome o = bs[b]->decide(g, i, j, del, par, need);
            if (o.accept != o0.accept) die("backends disagree on a decision: " + bs[0]->name + " vs " + bs[b]->name);
            if (o.full && o0.full && (std::memcmp(&o.delta, &o0.delta, sizeof(double)) != 0 || o.c != o0.c))
                die("backends disagree on delta: " + bs[0]->name + " vs " + bs[b]->name);
        }
        hash = (hash ^ (u64(i) << 33) ^ (u64(j) << 1) ^ u64(o0.accept)) * 0x100000001b3ull;
        ++steps;
        if (!o0.accept) return;
        if (!o0.full) die("accepted without a full evaluation");
        for (Backend* b : bs) b->commit(g, i, j, !del);
        Edge e = canon(i, j);
        if (del) { g.erase(i, j); drop_edge(e); ++del_acc; }
        else { g.insert(i, j); add_edge(e); ++add_acc; }
        shared.edit += 3;
        stat += sgn * o0.delta;
        tri += del ? -i64(o0.c) : i64(o0.c);
        ++accepted;
    }

    void recount(double& s, i64& t) const {
        double total = 0;
        double lam = par.lambda;
        for (u32 a = 0; a < n_; ++a)
            for (u32 b : g.nbr(a)) {
                if (b < a) continue;
                u32 x = g.deg(a) <= g.deg(b) ? a : b, y = x == a ? b : a, c = 0;
                for (u32 w : g.nbr(x)) c += g.has(w, y) ? 1 : 0;
                total += lam * (1.0 - pw_[c]);
            }
        s = total;
        t = static_count(g).first;
    }

    // Full recount: graph structures, the running statistic, the triangle count, and the tables when asked.
    void check(const std::vector<Backend*>& bs, bool tables) {
        u64 ds = 0;
        for (u32 v = 0; v < n_; ++v) ds += g.deg(v);
        if (ds != 2 * g.m() || g.m() != el_.size() || pos_.size() != el_.size()) die("graph and edge list differ");
        for (Edge e : el_) if (!g.has(e.a, e.b)) die("edge list holds an absent edge");
        double s; i64 t;
        recount(s, t);
        if (t != tri) die("triangle count differs from static_count");
        double err = std::fabs(s - stat);
        max_stat_err = std::max(max_stat_err, err);
        if (err > 1e-6 * (1.0 + std::fabs(s))) die("running statistic differs from recount");
        stat = s;
        ++checks; ++static_checks;
        if (tables) for (Backend* b : bs) if (b->entries() <= verify_limit) b->verify(g);
    }

    u32 n() const { return n_; }

private:
    void add_edge(Edge e) {
        if (g.has(e.a, e.b)) { /* insertion path of a step: already in g */ }
        else g.insert(e.a, e.b);
        pos_.emplace(key(e), u32(el_.size()));
        el_.push_back(e);
    }
    void drop_edge(Edge e) {
        u32 p = *pos_.find(key(e));
        Edge last = el_.back();
        el_[p] = last;
        el_.pop_back();
        if (key(last) != key(e)) *pos_.find(key(last)) = p;
        pos_.erase(key(e));
    }
    u32 n_;
    double dyads_;
    std::vector<Edge> el_;
    FlatMap<u32> pos_;
    std::vector<double> pw_;
};

// ---------------------------------------------------------------------------------------------------------------
// Self-test: brute force on small graphs, a control that must fail, and the exact target on a tiny state space
// ---------------------------------------------------------------------------------------------------------------

double brute_stat(const Graph& g, double lambda) {
    double p = 1.0 - 1.0 / lambda, s = 0;
    for (u32 a = 0; a < g.n(); ++a)
        for (u32 b = a + 1; b < g.n(); ++b) {
            if (!g.has(a, b)) continue;
            int c = 0;
            for (u32 w = 0; w < g.n(); ++w)
                if (w != a && w != b && g.has(a, w) && g.has(b, w)) ++c;
            s += lambda * (1.0 - std::pow(p, c));
        }
    return s;
}

int selftest(u64 seed) {
    Rng r(seed);
    u64 trials = 0, bad = 0, control_wrong = 0, control_tries = 0, tri_checks = 0;
    double worst = 0;
    for (int round = 0; round < 3000; ++round) {
        u32 n = 5 + u32(r.below(24));
        double dens = 0.05 + 0.9 * r.unif();
        double lambda = round % 3 == 0 ? 2.0 : round % 3 == 1 ? 1.5 : 3.0;
        Snapshot s;
        s.n = n;
        for (u32 a = 0; a < n; ++a)
            for (u32 b = a + 1; b < n; ++b)
                if (r.unif() < dens) s.edges.push_back({a, b});
        Chain ch(s, 0, 0, lambda, false, seed + u64(round));
        ScanBackend scan0(ch.g, 0, 0), scan1(ch.g, 64, 1);
        EagerBackend eager(ch.g);
        SplitBackend split(ch.g, 64u * u32(1 + r.below(2)) > n ? n / 2 : 64), split2(ch.g, n / 3);
        std::vector<Backend*> bs{&scan0, &scan1, &eager, &split, &split2};
        for (int t = 0; t < 40; ++t) {
            u32 i = u32(r.below(n)), j = u32(r.below(n));
            if (i == j) continue;
            bool present = ch.g.has(i, j);
            double before = brute_stat(ch.g, lambda);
            Graph h = ch.g;
            if (present) h.erase(i, j); else h.insert(i, j);
            double after = brute_stat(h, lambda);
            double truth = present ? before - after : after - before;
            for (Backend* b : bs) {
                Outcome o = b->decide(ch.g, i, j, present, ch.par, 0.0);
                double e = std::fabs(o.delta - truth);
                worst = std::max(worst, e);
                if (e > 1e-9) ++bad;
                ++trials;
            }
            if (present) {  // the control drops the -1 of a delete move, and it must be wrong
                double wrong = 0, p = 1.0 - 1.0 / lambda;
                int c = 0;
                for (u32 v = 0; v < n; ++v) {
                    if (v == i || v == j || !ch.g.has(i, v) || !ch.g.has(j, v)) continue;
                    ++c;
                    int ci = 0, cj = 0;
                    for (u32 w = 0; w < n; ++w) {
                        ci += w != i && w != v && ch.g.has(i, w) && ch.g.has(v, w);
                        cj += w != j && w != v && ch.g.has(j, w) && ch.g.has(v, w);
                    }
                    wrong += std::pow(p, ci) + std::pow(p, cj);
                }
                wrong += lambda * (1.0 - std::pow(p, c));
                ++control_tries;
                if (std::fabs(wrong - truth) > 1e-9) ++control_wrong;
            }
        }
        // a short live chain with every backend in lockstep, with and without early rejection
        for (int mode = 0; mode < 2; ++mode) {
            ch.par.early = mode == 1;
            ch.par.theta_e = -1.0 + 2.0 * r.unif();
            ch.par.theta_t = 1.5 * r.unif();
            for (int t = 0; t < 300; ++t) ch.step(bs);
            ch.check(bs, true);
            if (brute_triangles(ch.g) != ch.tri) die("brute triangles differ");
            if (std::fabs(brute_stat(ch.g, lambda) - ch.stat) > 1e-7) die("brute statistic differs");
            ++tri_checks;
        }
    }
    std::printf("delta vs brute: trials=%llu bad=%llu worst=%.2e\n", (unsigned long long)trials,
                (unsigned long long)bad, worst);
    std::printf("control without the delete correction: wrong on %llu of %llu delete dyads (must be > 0)\n",
                (unsigned long long)control_wrong, (unsigned long long)control_tries);
    std::printf("live lockstep chains: brute triangle and statistic checks=%llu\n", (unsigned long long)tri_checks);

    // Exact target on 5 vertices: all 1024 graphs, with empirical frequencies from the chain.
    constexpr u32 N = 5;
    std::vector<std::pair<u32, u32>> dy;
    for (u32 a = 0; a < N; ++a) for (u32 b = a + 1; b < N; ++b) dy.push_back({a, b});
    const double te = -0.7, tt = 0.45, lambda = 2.0;
    std::vector<double> w(1u << dy.size());
    double z = 0;
    for (u32 mask = 0; mask < w.size(); ++mask) {
        Graph h(N);
        for (size_t k = 0; k < dy.size(); ++k) if (mask >> k & 1) h.insert(dy[k].first, dy[k].second);
        w[mask] = std::exp(te * double(h.m()) + tt * brute_stat(h, lambda));
        z += w[mask];
    }
    Snapshot s;
    s.n = N;
    Chain ch(s, te, tt, lambda, true, seed ^ 0xabcdef);
    ScanBackend sc(ch.g, 16, 1);
    EagerBackend ea(ch.g);
    SplitBackend sp(ch.g, 2);
    std::vector<Backend*> bs{&sc, &ea, &sp};
    std::vector<u64> freq(w.size(), 0);
    const u64 steps = 6000000;
    for (u64 t = 0; t < steps; ++t) {
        ch.step(bs);
        u32 mask = 0;
        for (size_t k = 0; k < dy.size(); ++k) if (ch.g.has(dy[k].first, dy[k].second)) mask |= 1u << k;
        ++freq[mask];
    }
    double tv = 0;
    for (u32 mask = 0; mask < w.size(); ++mask) tv += std::fabs(double(freq[mask]) / double(steps) - w[mask] / z);
    tv /= 2;
    std::printf("exact target on 5 vertices: total variation=%.5f over %llu steps, acceptance=%.3f\n", tv,
                (unsigned long long)steps, double(ch.accepted) / double(steps));
    if (bad || control_wrong == 0 || tv > 0.01) { std::puts("SELFTEST FAILED"); return 1; }
    std::puts("SELFTEST OK");
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// run
// ---------------------------------------------------------------------------------------------------------------

struct RunCfg {
    std::string graph, out, tag = "";
    double te = 0, a = NAN, tt = 0, target = 0;
    u64 N = 10000000, burn = 0, seed = 1, checks = 20;
    u64 verify_limit = ~u64(0), warm = 0;
    bool memo_add = true;
    std::vector<u32> fsplit;
    bool both_eager = false;
    double maxm = 3.0, minm = 0.0;      // stop a run whose edge count passes maxm * m0: the model has left the sparse phase
    bool early = true, no_eager = false, formula_eager = false, no_tables = false, dyads = true;
    std::vector<u32> hubs{64, 256, 1024, 4096};
    std::vector<std::pair<double, u32>> memos{{1.0, 3}, {4.0, 3}, {4.0, 16}};    // slots per edge, min degree
    bool scan_nomemo = true;
};

std::vector<std::string> split_list(const std::string& s, char sep) {
    std::vector<std::string> out;
    size_t at = 0;
    while (at <= s.size()) {
        size_t e = s.find(sep, at);
        if (e == std::string::npos) e = s.size();
        if (e > at) out.push_back(s.substr(at, e - at));
        at = e + 1;
    }
    return out;
}

RunCfg parse_run(int argc, char** argv) {
    RunCfg c;
    for (int i = 2; i < argc; ++i) {
        std::string a = argv[i];
        size_t eq = a.find('=');
        if (eq == std::string::npos) die("expected key=value: " + a);
        std::string k = a.substr(0, eq), v = a.substr(eq + 1);
        if (k == "graph") c.graph = v;
        else if (k == "out") c.out = v;
        else if (k == "tag") c.tag = v;
        else if (k == "te") c.te = std::atof(v.c_str());
        else if (k == "a") c.a = std::atof(v.c_str());          // te = a - log(D / m0), the offset from the density value
        else if (k == "tt") c.tt = std::atof(v.c_str());
        else if (k == "target") c.target = std::atof(v.c_str());
        else if (k == "N") c.N = u64(std::atof(v.c_str()));
        else if (k == "burn") c.burn = u64(std::atof(v.c_str()));
        else if (k == "seed") c.seed = u64(std::atof(v.c_str()));
        else if (k == "checks") c.checks = u64(std::atof(v.c_str()));
        else if (k == "maxm") c.maxm = std::atof(v.c_str());
        else if (k == "minm") c.minm = std::atof(v.c_str());
        else if (k == "verify") c.verify_limit = u64(std::atof(v.c_str()));
        else if (k == "memoadd") c.memo_add = v != "0";
        else if (k == "warm") c.warm = u64(std::atof(v.c_str()));   // lockstep steps before the count, so memos start warm
        else if (k == "fsplit") { for (auto& x : split_list(v, ',')) c.fsplit.push_back(u32(std::atoi(x.c_str()))); }
        else if (k == "early") c.early = v != "0";
        else if (k == "eager") { c.no_eager = v == "0"; c.formula_eager = v == "formula" || v == "both"; c.both_eager = v == "both"; }
        else if (k == "tables") c.no_tables = v == "0";
        else if (k == "dyads") c.dyads = v != "0";
        else if (k == "hubs") { c.hubs.clear(); for (auto& x : split_list(v, ',')) c.hubs.push_back(u32(std::atoi(x.c_str()))); }
        else if (k == "memos") {
            c.memos.clear();
            for (auto& x : split_list(v, ',')) {
                auto parts = split_list(x, ':');
                if (parts.size() != 2) die("memos takes slots_per_edge:min_degree");
                c.memos.push_back({std::atof(parts[0].c_str()), u32(std::atoi(parts[1].c_str()))});
            }
        }
        else if (k == "nomemo") c.scan_nomemo = v != "0";
        else die("unknown key " + k);
    }
    if (c.graph.empty()) die("graph= is required");
    return c;
}

int run(const RunCfg& cfg) {
    auto t0 = std::chrono::steady_clock::now();
    auto secs = [&] { return std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count(); };
    Snapshot snap = load_spec(cfg.graph);
    double d = double(snap.n) * (double(snap.n) - 1.0) / 2.0, m0 = double(snap.edges.size());
    double te = std::isnan(cfg.a) ? cfg.te : cfg.a - std::log(d / m0);
    Chain ch(snap, te, cfg.tt, 2.0, cfg.early, cfg.seed);
    std::printf("%s n=%u m0=%.0f te=%.6f (a=%.4f) tt=%.6f burn=%llu N=%llu load=%.1fs\n", snap.name.c_str(), snap.n, m0,
                te, te + std::log(d / m0), cfg.tt, (unsigned long long)cfg.burn, (unsigned long long)cfg.N, secs());
    std::fflush(stdout);

    u64 burn_acc = 0;
    if (cfg.burn) {
        ScanBackend bk(ch.g, std::max<u64>(1024, 4 * u64(m0)), 3);
        std::vector<Backend*> bs{&bk};
        u64 every = std::max<u64>(1, cfg.burn / 10), last = 0;
        for (u64 k = 1; k <= cfg.burn; ++k) {
            ch.step(bs);
            if (double(ch.g.m()) > cfg.maxm * m0 || double(ch.g.m()) < cfg.minm * m0) {
                std::printf("LEFT_BAND burn step=%llu m=%llu\n", (unsigned long long)k, (unsigned long long)ch.g.m());
                return 4;
            }
            if (k % every == 0) {
                std::printf("  burn %llu acc=%.4f m=%llu T=%lld %.1fs\n", (unsigned long long)k,
                            double(ch.accepted - last) / double(every), (unsigned long long)ch.g.m(), (long long)ch.tri, secs());
                std::fflush(stdout);
                last = ch.accepted;
            }
        }
        ch.check(bs, false);
        burn_acc = ch.accepted;
        ch.accepted = ch.add_acc = ch.del_acc = ch.add_prop = ch.del_prop = ch.steps = 0;
        ch.shared = Cost{};
    }

    u64 start_m = ch.g.m();
    i64 start_t = ch.tri;
    u64 wedges = 0, dyads = 0;
    if (cfg.dyads) dyads = count_dyads(ch.g, wedges);
    std::printf("  start of count: m=%llu T=%lld wedges=%llu dyads=%llu %.1fs\n", (unsigned long long)start_m,
                (long long)start_t, (unsigned long long)wedges, (unsigned long long)dyads, secs());
    std::fflush(stdout);

    std::vector<std::unique_ptr<Backend>> own;
    if (cfg.scan_nomemo) own.push_back(std::make_unique<ScanBackend>(ch.g, 0, 0));
    for (auto [per_edge, md] : cfg.memos) {
        own.push_back(std::make_unique<ScanBackend>(ch.g, std::max<u64>(1024, u64(per_edge * double(start_m))), md));
        own.back()->skip_add_dyad = !cfg.memo_add;
    }
    const SplitBackend* truth = nullptr;
    for (u32 h : cfg.hubs) {
        auto sp = std::make_unique<SplitBackend>(ch.g, h);
        if (!truth) truth = sp.get();
        own.push_back(std::move(sp));
        std::printf("  split h=%u entries=%llu %.1fs\n", h, (unsigned long long)own.back()->entries(), secs());
        std::fflush(stdout);
    }
    for (u32 h : cfg.fsplit) {
        if (!truth) die("fsplit needs a real split backend");
        own.push_back(std::make_unique<FormulaSplit>(ch.g, h, truth));
        std::printf("  split-formula h=%u entries=%llu %.1fs\n", h, (unsigned long long)own.back()->entries(), secs());
        std::fflush(stdout);
    }
    if (cfg.formula_eager) {
        if (!truth) die("eager=formula needs a split backend");
        own.push_back(std::make_unique<FormulaEager>(truth, dyads, wedges));
    }
    if (!cfg.no_eager && (!cfg.formula_eager || cfg.both_eager)) {
        own.push_back(std::make_unique<EagerBackend>(ch.g));
        std::printf("  eager entries=%llu %.1fs\n", (unsigned long long)own.back()->entries(), secs());
        std::fflush(stdout);
    }
    std::vector<Backend*> bs;
    for (auto& b : own) bs.push_back(b.get());
    ch.verify_limit = cfg.verify_limit;
    if (!cfg.no_tables) ch.check(bs, true);

    if (cfg.warm) {     // same chain, every backend attached, nothing counted: this takes the cold start off the memos
        for (u64 k = 0; k < cfg.warm; ++k) ch.step(bs);
        ch.check(bs, false);
        for (Backend* b : bs) {
            b->cost = Cost{};
            if (auto* sc = dynamic_cast<ScanBackend*>(b)) sc->hits = sc->misses = sc->bypass = 0;
        }
        ch.shared = Cost{};
        ch.accepted = ch.add_acc = ch.del_acc = ch.add_prop = ch.del_prop = ch.steps = 0;
        std::printf("  warm done %.1fs\n", secs());
        std::fflush(stdout);
    }
    u64 every = std::max<u64>(1, cfg.N / 10), chk = std::max<u64>(1, cfg.N / std::max<u64>(1, cfg.checks));
    u64 last = 0, tail_acc = 0, tail_from = cfg.N / 2, m_at_half = 0;
    for (u64 k = 1; k <= cfg.N; ++k) {
        ch.step(bs);
        if (double(ch.g.m()) > cfg.maxm * m0 || double(ch.g.m()) < cfg.minm * m0) {
            std::printf("LEFT_BAND count step=%llu m=%llu\n", (unsigned long long)k, (unsigned long long)ch.g.m());
            return 4;
        }
        if (k == tail_from) { tail_acc = ch.accepted; m_at_half = ch.g.m(); }
        if (k % every == 0) {
            std::printf("  step %llu acc=%.4f m=%llu T=%lld add=%llu del=%llu %.1fs\n", (unsigned long long)k,
                        double(ch.accepted - last) / double(every), (unsigned long long)ch.g.m(), (long long)ch.tri,
                        (unsigned long long)ch.add_acc, (unsigned long long)ch.del_acc, secs());
            std::fflush(stdout);
            last = ch.accepted;
        }
        if (k % chk == 0) ch.check(bs, !cfg.no_tables && (k == cfg.N || wedges < 20000000));
    }

    double acc = double(ch.accepted) / double(cfg.N);
    double tail = double(ch.accepted - tail_acc) / double(cfg.N - tail_from);
    bool head = false;
    FILE* probe = std::fopen(cfg.out.c_str(), "rb");
    if (probe) std::fclose(probe); else head = true;
    FILE* f = cfg.out.empty() ? nullptr : std::fopen(cfg.out.c_str(), "ab");
    if (f && head)
        std::fprintf(f, "graph,label,tag,target,seed,N,burn,burn_accepted,n,m0,m_start,m_half,m_end,T_start,T_end,theta_e,a,theta_t,"
                        "early,accepted,acc_rate,acc_tail,add_acc,del_acc,decision_hash,backend,hubs,memo_slots,memo_min,"
                        "ops_total,ops_alt,ops_per_prop,list,probe,table_read,table_write,stamp,word,edit,shared,build_ops,"
                        "entries_start_or_slots,entries_peak,index_bytes,wedges_start,dyads_start,memo_hits,memo_misses,"
                        "checks,static_checks,max_stat_err\n");
    for (Backend* b : bs) {
        Cost c = b->cost;
        u64 tot = c.total() + ch.shared.total(), alt = c.alt() + ch.shared.alt();
        u64 hits = 0, misses = 0;
        if (auto* sc = dynamic_cast<ScanBackend*>(b)) { hits = sc->hits; misses = sc->misses; }
        std::printf("  %-14s h=%-5u memo=%-9llu md=%-3u ops=%14llu per=%10.2f entries=%llu bytes=%.3e\n", b->name.c_str(), b->hsize,
                    (unsigned long long)b->memo_slots, b->memo_min, (unsigned long long)tot, double(tot) / double(cfg.N),
                    (unsigned long long)b->peak_entries, b->index_bytes());
        if (f)
            std::fprintf(f, "%s,%s,%s,%g,%llu,%llu,%llu,%llu,%u,%.0f,%llu,%llu,%llu,%lld,%lld,%.6f,%.6f,%.6f,"
                            "%d,%llu,%.6f,%.6f,%llu,%llu,%016llx,%s,%u,%llu,%u,"
                            "%llu,%llu,%.3f,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,"
                            "%llu,%llu,%.0f,%llu,%llu,%llu,%llu,%llu,%llu,%.3e\n",
                         snap.name.c_str(), snap.label.c_str(), cfg.tag.c_str(), cfg.target, (unsigned long long)cfg.seed,
                         (unsigned long long)cfg.N, (unsigned long long)cfg.burn, (unsigned long long)burn_acc, snap.n, m0,
                         (unsigned long long)start_m, (unsigned long long)m_at_half, (unsigned long long)ch.g.m(), (long long)start_t,
                         (long long)ch.tri, ch.par.theta_e, ch.par.theta_e + std::log(d / m0), ch.par.theta_t, cfg.early ? 1 : 0,
                         (unsigned long long)ch.accepted, acc, tail, (unsigned long long)ch.add_acc, (unsigned long long)ch.del_acc,
                         (unsigned long long)ch.hash, b->name.c_str(), b->hsize, (unsigned long long)b->memo_slots, b->memo_min,
                         (unsigned long long)tot, (unsigned long long)alt, double(tot) / double(cfg.N),
                         (unsigned long long)c.list, (unsigned long long)c.probe, (unsigned long long)c.tread,
                         (unsigned long long)c.twrite, (unsigned long long)c.stamp, (unsigned long long)c.word,
                         (unsigned long long)c.edit, (unsigned long long)ch.shared.total(), (unsigned long long)b->build_ops,
                         (unsigned long long)b->entries(), (unsigned long long)b->peak_entries, b->index_bytes(),
                         (unsigned long long)wedges, (unsigned long long)dyads, (unsigned long long)hits, (unsigned long long)misses,
                         (unsigned long long)ch.checks, (unsigned long long)ch.static_checks, ch.max_stat_err);
    }
    if (f) std::fclose(f);
    std::printf("  acc=%.5f tail=%.5f m_end=%llu T_end=%lld checks=%llu hash=%016llx total=%.1fs\n", acc, tail,
                (unsigned long long)ch.g.m(), (long long)ch.tri, (unsigned long long)ch.checks,
                (unsigned long long)ch.hash, secs());
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) { std::puts("usage: ergm_chain step1|selftest|run ..."); return 2; }
    std::string mode = argv[1];
    if (mode == "selftest") return selftest(argc > 2 ? u64(std::atoll(argv[2])) : 11);
    if (mode == "step1") {
        if (argc < 3) { std::puts("usage: ergm_chain step1 out.csv [rmat specs]"); return 2; }
        FILE* f = std::fopen(argv[2], "wb");
        if (!f) die("cannot open output");
        std::fprintf(f, "graph,label,n,m,dmax,E_inf,H,tau,E_tau,E_L,E_tau_share,E_L_share\n");
        std::vector<std::string> specs{"su-peak", "wiki-peak", "su-ins", "wiki-ins"};
        for (int i = 3; i < argc; ++i) specs.push_back(argv[i]);
        for (const std::string& sp : specs) step1_row(f, load_spec(sp));
        std::fclose(f);
        return 0;
    }
    if (mode == "run") return run(parse_run(argc, argv));
    std::puts("unknown mode");
    return 2;
}
