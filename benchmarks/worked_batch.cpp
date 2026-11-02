// The worked batch for the §16 figure: one batch of a logged run, opened up. Count build only, never timed.
// Usage: worked_batch <events.bin> <cut> <batch> <out.csv>
// Replays the stream with the PR engine of Gate 1 (Engine5, Arm::PR) up to the batch, then runs that batch group by group exactly as Engine5::apply does,
// and writes one row per spoke: the pass, the group's centre and size, the centre's degree before the group, the spoke, the 5-cycles that spoke adds or
// removes, and what R2 would charge for that edge in the same graph state. R2's charge is read on the graph as PR leaves it after each earlier spoke, and the
// edits are undone before PR runs, so PR's own run is the logged one. The last line holds the batch's PR operations and the count after it, which must equal
// the logged run's row for that batch.
#ifndef GATE1_NO_MAIN
#define GATE1_NO_MAIN
#endif
#include "gate1.cpp"

#include <fstream>

using namespace fc;

namespace {

#pragma pack(push, 1)
struct Ev { std::int64_t t; u32 u, v, op; };
#pragma pack(pop)

}  // namespace

int main(int argc, char** argv) {
    if (argc != 5) { std::puts("usage: worked_batch events.bin cut batch out.csv"); return 2; }
    FILE* in = std::fopen(argv[1], "rb");
    if (!in) { std::puts("cannot open events"); return 2; }
    std::fseek(in, 0, SEEK_END);
    const long long bytes = _ftelli64(in);
    std::fseek(in, 0, SEEK_SET);
    std::vector<Ev> ev(size_t(bytes) / sizeof(Ev));
    if (std::fread(ev.data(), sizeof(Ev), ev.size(), in) != ev.size()) { std::puts("short read"); return 2; }
    std::fclose(in);
    const size_t cut = size_t(std::atoll(argv[2])), want = size_t(std::atoll(argv[3]));
    u32 n = 0;
    for (const Ev& x : ev) n = std::max({n, x.u + 1, x.v + 1});

    // Batch bounds exactly as gate1.cpp's main.
    std::vector<size_t> starts{0};
    for (size_t i = 1; i < ev.size(); ++i)
        if (i % cut == 0) starts.push_back(i);
    starts.push_back(ev.size());
    if (want + 1 >= starts.size()) { std::puts("no such batch"); return 2; }

    Engine5 e(Arm::PR, n);
    Normaliser norm;
    std::vector<Update> b;
    for (size_t bi = 0; bi < want; ++bi) {
        b.clear();
        for (size_t i = starts[bi]; i < starts[bi + 1]; ++i) b.push_back({ev[i].u, ev[i].v, ev[i].op == 1});
        e.apply(b, norm.run(e.g, b), ~u64(0));
    }

    b.clear();
    for (size_t i = starts[want]; i < starts[want + 1]; ++i) b.push_back({ev[i].u, ev[i].v, ev[i].op == 1});
    const NetBatch& nb = norm.run(e.g, b);
    std::ofstream out(argv[4]);
    out << "pass,group,centre,size,centre_degree,spoke,delta,r2_ops,pr_group_ops,pr_group_delta\n";

    // Engine5::apply for Arm::PR, opened up.
    const u64 before = e.c.total();
    const u64 net = nb.del.size() + nb.ins.size();
    e.c.net += 2 * b.size();
    e.c.group += 3 * net;
    Row scratch(n);
    size_t gi = 0;
    for (int pass = 0; pass < 2; ++pass) {
        const bool ins = pass == 1;
        // Copied, since the next run of the grouper reuses its store.
        std::vector<std::pair<u32, std::vector<u32>>> groups;
        for (const GroupV& gr : e.grouper.run(ins ? nb.ins : nb.del)) groups.push_back({gr.x, std::vector<u32>(gr.c.begin(), gr.c.end())});
        for (auto& [x, spokes] : groups) {
            const u32 dx = e.g.deg(x);
            std::vector<i64> delta;
            std::vector<u64> r2;
            for (u32 v : spokes) {
                Cost c;
                if (!ins) e.g.erase(x, v);
                const i64 s = through_r2(e.g, x, v, scratch, c);
                if (ins) e.g.insert(x, v);
                delta.push_back(ins ? s : -s);
                r2.push_back(c.total() + 2);    // R2 also pays the graph edit, 2
            }
            for (auto it = spokes.rbegin(); it != spokes.rend(); ++it) {
                if (ins) e.g.erase(x, *it); else e.g.insert(x, *it);
            }
            GroupV gr{x, Spokes{spokes.data(), spokes.data() + spokes.size()}};
            const u64 g0 = e.c.total();
            const i64 d = star_pr(e.g, gr, ins, e.row, e.c);
            e.t += d;
            i64 sum = 0;
            for (i64 s : delta) sum += s;
            if (sum != d) { std::printf("MISMATCH group %zu: spokes %lld, PR %lld\n", gi, (long long)sum, (long long)d); return 1; }
            for (size_t i = 0; i < spokes.size(); ++i)
                out << pass << ',' << gi << ',' << x << ',' << spokes.size() << ',' << dx << ',' << spokes[i] << ',' << delta[i] << ',' << r2[i] << ','
                    << (e.c.total() - g0) << ',' << d << '\n';
            ++gi;
        }
    }
    out << "batch," << want << ",raw," << b.size() << ",net," << net << ",pr_ops," << (e.c.total() - before) << ",t," << e.t << '\n';
    std::printf("batch %zu: %zu raw, %llu net, %zu groups, PR %llu operations, count %lld\n", want, b.size(), (unsigned long long)net, gi,
                (unsigned long long)(e.c.total() - before), (long long)e.t);
    return 0;
}
