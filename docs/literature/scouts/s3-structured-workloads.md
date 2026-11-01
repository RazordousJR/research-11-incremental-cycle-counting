# Scout S3: workloads where structure is forced

Date: 2026-11-01. Direction: vertex-attribute updates, ERGM and MCMC change statistics, edge-switch chains,
vertex updates and generators. Web sources run to 1 November 2026, subject to the limits below.

## Verdict

**The peel-and-kernel mechanism does not find a home in this direction.** Forced whole-vertex structure is real,
but each place it appears has a blocker:

| Where structure is forced | Why the star kernels do not apply |
|---|---|
| Attribute flips (ALAAM, opinion models) | The graph is fixed, so a stored table beats any kernel. ALAAMEE says so in print |
| Actor-oriented rows (SAOM, LOLOG) | The step needs one score per alter, not a sum. RSiena already runs that two-hop scan |
| Curveball trades | Each side moves at most `min(a, b)` edges (my calculation, candidate 4), so hub stars stay small |

**What is open is something else.** Network MCMC splits one event into many *probes* and a few *commits*, and the
eager two-path tables in the tools hit a memory wall that Stivala et al. (2020) state as unsolved. The structure that
addresses it has been in print since 2009, and Eppstein and Spiro designed it for ERGMs. It appears not to have
been deployed in the tools I read. That makes the top candidate an engineering and empirical contribution, not a
new algorithm, and it gives up star peeling as the main idea.

Plain triangle terms do not escape the Gate 0 cause. Shared-partner terms (GWESP, alternating k-triangles) might.

## Method and limits

- **WebSearch and WebFetch:** 31 web searches across the four sub-areas, plus direct fetches. The queries are not
  all listed here; the areas are visible in the ledger.
- **arXiv API:** throttled hard (HTTP 429) during this session.
  - Returned: equilibrium expectation; autologistic actor attribute; ERGM scalable; stochastic actor computational
    (1 hit, irrelevant); Curveball (2 queries); edge switching with triangle or clustering; double edge swap;
    targeted rewiring; signed balance triangle; balanced triangle; triad census; coloured triangles; vertex updates;
    vertex insertions; dynamic set intersection; informed proposal; triangle densest; rejection-free network.
  - Returned zero hits, which I treat as uninformative: "change statistic" with ERGM; relational event triadic;
    `dK` rewiring; vertex label in continuous subgraph matching.
  - **Never returned, despite retries:** "change statistics" with triangle; "common neighbors" with dynamic set
    intersection; "vertex deletion" with triangle counting; graph generator with triangle target. These topics
    rest on web search alone.
- **OpenAlex: no query succeeded.** All 17 prepared queries returned HTTP 429. The response reported the shared
  daily budget as spent (retry after about 23 hours). I did not request a key.
- **DBLP:** not used, as instructed.
- **No Browser pane or Chrome tools** were used. No project code was edited.
- **Code:** read at the current default branch through raw GitHub files for ergm, EstimNetDirected, ALAAMEE,
  RSiena, lolog, RandNetGen, NetworkX and NetworKit. igraph was not checked. Gjoka's 2.5K code was not found at the
  repository path I tried.
- **Not scouted in depth:** relational event models (one web search only; each event changes a star of triadic
  statistics, so they may deserve their own look) and community-detection local moves with triangle objectives
  (not searched).

**How to read "not found" below.** For a named tool, "not in the code" is strong evidence, because I read the
code path. For the literature, "I found nothing" is weak evidence, and it is weaker still here because OpenAlex
was unavailable and arXiv was partly throttled. Conference proceedings of 2025 and 2026 are the likeliest place
for a miss.

## Corrections to the brief

1. **Hanauer et al. on vertex updates.** The passage I found (§4, "O-D, Direct Partition Swap") is about a vertex
   changing partition, not about inserting or deleting a vertex. It says this can be done edge by edge. It then
   says the swap can be done directly by scanning the 2-hop neighbourhood, and A.11 gives that algorithm. §5.3
   reports the combined optimisation EGST*-HD as 4.3× faster in total time and says high-degree anchors contribute
   most of the gain. So a dedicated whole-vertex path already exists there, for auxiliary tables. A text search of
   the PDF found no treatment of vertex insertion or deletion as an update type. That search is not a full read.
2. **"Penschuck and Hamann global edge switching."** The paper is by Allendorf, Meyer, Penschuck and Tran (IPDPS
   2022). Hamann is a co-author of the Global Curveball paper.
3. **The audit's row 5 undersells Eppstein and Spiro.** The paper's stated application is ERGMs, and §6 gives
   weighted triangles (Theorem 4) and coloured triangles (Theorem 5). That matters for rejected ideas 1 and 2.

## State of the art by sub-area

| Sub-area | Exact, incremental methods | Dedicated kernel published? |
|---|---|---|
| (a) Coloured, typed triangles | Eppstein-Spiro Theorem 5: `O(h + k)` per edge update. ergm `triangle(attr)` filters inside a scan | Colour *change* as an update: none found. Colours are exogenous in ERGM |
| (a) Signed, balance | Eppstein-Spiro Theorem 4 covers signs (my derivation, rejected idea 2). ITC is described as incremental (secondary source; primary not read) | Sign-flip kernel: none found, and none needed |
| (a) Triad census | Eppstein et al.: all directed 3-vertex counts in `O(h)` amortized per update. Coloured census is static (Lienert et al.) | No |
| (a) Attribute chains | ALAAMEE: exogenous network, static two-path lookup, flip delta from it | Static table, yes |
| (b) ERGM | Scan (ergm `triangle`), eager hash (ergm `.spcache.net`, EstimNetDirected), array table, on-the-fly | Per-dyad only. `SPDyad` is a proposal, not a kernel |
| (b) SAOM, LOLOG (adjacent, not in the brief) | RSiena two-hop `TwoPathTable` per ego; lolog per-dyad merge | RSiena's table is the row kernel |
| (c) Switching | NetworkX, NetworKit, graph-tool: no triangle tracking in the code or docs read. RandNetGen, Gjoka, Ferenczi and Grigoriev: exact per-proposal delta | No |
| (d) Vertex primitive | Chiba-Nishizeki-style mark, scan, delete. ARB-PEEL for peeling. Hanauer's direct swap | Not as a dynamic counting kernel, as far as I found |
| (d) Generators with targets | BTER, Darwini, NetworkX `random_clustered_graph`: constructive | No maintenance needed |

Worst case for vertex updates: for online vertex updates on directed graphs, van den Brand et al. state no
`O(n^(2-ε))` update algorithm exists for triangle detection, conditional on OMv.

---

## Candidate 1: bounded-memory shared-partner index for rejection-heavy network MCMC

**Claim.** For ERGM chains with shared-partner terms, a two-path index that stores only wedges through low-degree
midpoints and keeps hub adjacency in per-vertex bitmasks gives exact change statistics under a chosen memory cap,
with commits cheaper than an eager table, at a probe cost of a few machine words.

**Type.** Method and empirical, with a short cost analysis. Not a new algorithm in the theory sense.

**Closest work.**

| Work | Read | Overlap |
|---|---|---|
| [Eppstein, Spiro 2012](https://arxiv.org/abs/0904.3741) | Abstract, §1, §6; other sections not read | **Highest.** `P[u,v]` counts 2-paths through low-degree vertices; high-degree `H` is handled separately. Built for ERGM. Partition is maintained per update; no bitmask, no memory budget, no acceptance rate |
| [ergm](https://github.com/statnet/ergm) `src/changestats_spcache.c`, `changestats_dgw_sp.h`, `R/ergm-options.R` | Code and docs read | **High.** Eager khash of all dyads with a shared partner. `cache.sp` defaults to TRUE. Commit does one hash update per neighbour |
| [Stivala, Robins, Lomi 2020](https://arxiv.org/abs/1904.08063), [EstimNetDirected](https://github.com/stivalaa/EstimNetDirected) | Methods, discussion; `graph.c`, `graph.h`, `Makefile`, `changeStatisticsUndirected.c` | **High.** Eager uthash plus Bloom filter, or `n²` array, or no table. States the memory wall |
| [Hanauer et al. 2026](https://arxiv.org/abs/2610.29776) | §4 to §5 passages, A.11 | **Medium.** Heavy-light on streams with one count per update. Reports that auxiliary storage can stop paying. That is the case `ρ = 1` below |
| [Kopelowitz, Pettie, Porat 2015](https://arxiv.org/abs/1407.6755) | Abstract, §1 | **Medium.** Space-time trade-offs for dynamic set intersection, for reporting and witness queries. Theory, no Metropolis workload |
| [Stivala et al., ALAAMEE](https://arxiv.org/abs/2404.03116) | Methods passages | Low. Same table on a fixed network, where the problem vanishes |

**Precise gap.** Nothing I read does any of the following.
- Prices a triangle-type structure by probes per commit (call it `ρ`). The dynamic counting literature fixes
  `ρ = 1`.
- Caps memory for shared-partner terms. Stivala et al. report that a roughly one-million-node physician referral
  network did not fit in 512 GB, and they call the problem open: "Further work is required". Current
  EstimNetDirected still builds only no-table, array and hash variants. Current ergm is still eager.
- Uses a partition fixed for the run. An equilibrium chain has nearly stationary degrees, so the rebalancing that
  dominates Hanauer's §4 should mostly disappear. This is my reasoning and is untested.

"Not deployed" is strong for ergm and EstimNetDirected, where I read the code. It is weak for other tools.

**Mechanism sketch.** Fix `H` as the top vertices by degree, with `|H|` a multiple of 64, and let `L` be the rest.
Let `τ` be the largest degree in `L` at the start. Exactness never depends on the partition.

    P[u, v]   = wedges u-x-v with x in L        hash table
    B[u]      = bitmask of N(u) ∩ H             |H|/8 bytes per vertex
    cn(u, v)  = P[u, v] + popcount(B[u] & B[v])

    commit edge (u, x):
        if x in L: for w in N(x): P[u, w] += ±1
        if u in L: for w in N(u): P[x, w] += ±1
        flip the H bits for whichever endpoints are in H

| Backend | Probe `cn` | Commit | Memory |
|---|---|---|---|
| Scan, lower degree | `min(d_u, d_v)` | `O(1)` | `O(m)` |
| Eager table | `O(1)` | `d_u + d_v` | up to `Σ_v C(d_v, 2)` |
| Hub split | `O(1 + |H|/64)` | `O(d_x)` over endpoints in `L` | `Σ_{x in L} C(d_x, 2) + n|H|/8` |

Total work per commit is about `C_commit + ρ · C_probe`. An alternating k-triangle or GWESP proposal needs
`1 + 2c` shared-partner values, with `c = |N(i) ∩ N(j)|`, as `changeAltKTriangles` and `dsp_change` show. So
`ρ ≈ (1 + 2c̄) / α` for acceptance rate `α`.

The only numbers I found for `α` are one data point in Byshkin et al. 2016: 15% (IFD) and 0.4% (basic) on a
3,061-node hospital network. No source read gives `α` for a million-node run. The first step of the kill test
must measure it.

The star kernel survives only as an option. The `c` values `sp(i, v)` for `v` in the common neighbourhood are a
star of probes at pivot `i`, so kernel O with per-spoke output could serve them in `O(m)` memory. Expect this to
matter only for hub pivots, as in Gate 0.

**Why it escapes the Gate 0 cause.** Gate 0 failed because updates cost 5.9 to 13.9 operations and few fell in
large stars. Here no stars are needed. Commits are rare and probes multiply by `1 + 2c`. The eager structure also
fails on memory, which Gate 0 never measured.

**Where the escape is not secured.** For the plain `triangle` term, a random non-edge usually has `c = 0` and a
cheap scan, which is the Gate 0 situation again.

**Strongest simple baseline.** Lower-degree scan with `O(1)` adjacency and no table, plus a bounded memo of `cn`
keyed by dyad, invalidated by per-vertex version stamps. Note that ergm's `triangle` and its uncached `count_utp`
scan `neighbors(head)` without choosing the lower-degree endpoint, and EstimNetDirected's undirected `twoPaths`
is a nested loop. So any comparison with those tools must first fix those, or the gain is a one-line change.

**Cheapest kill test, under one hour.**
1. *From the degree sequence alone (one minute).* On the sx-superuser and wiki-talk-temporal peak snapshots and
   one rMAT graph, compute `E(τ) = Σ_{d<τ} C(d, 2)` against the total. **Kill if `E(τ) > 0.5 · E(∞)` at
   `|H| ≤ 4096`:** wedge mass is then not concentrated at hubs.
2. *Chain.* Write a driver on `Graph` and `Ops` from `src/r11/tc.hpp`, with its own copy of `common`, which is a
   private member of `Engine`. Run 10^7 TNT proposals on an
   edges plus alternating k-triangle model (`λ = 2`), with `θ` tuned to acceptance 1%, 5% and 15%. Count operations
   and table entries for scan, eager and split. Check every run against `static_count`.
   - **Kill if scan is within 1.5× of split at 5% acceptance:** tables are not needed at this scale.
   - **Kill if split costs over 2× eager at 1% acceptance.**

**Who needs it and what it costs them.** ERGM modellers working above about 10^5 nodes.
- Pokec, with hubs removed (1,632,783 nodes): about 22 hours on two parallel tasks with 512 GB each.
- The physician referral network could not run at all.
- ergm turns the cache on by default and says it usually improves performance significantly.
- Rough estimate of mine, not from a source: 0.06% non-zero on Pokec implies about 1.6 × 10^9 entries, on the
  order of 100 GB per table.

**Risks.**
- The physician network's wedges may come from many medium-degree vertices. If so, this design does not fix the
  one failure named in print. I cannot test that without the data, and I do not know it is available.
- Neural and block-approximate estimators ([Mele 2025](https://arxiv.org/abs/2502.01810),
  [Adriaens et al. 2020](https://arxiv.org/abs/2002.07076)) try to sidestep MCMC, which could shrink demand. Their
  abstracts show limited evidence so far, and the second changes the model.
- The paper would leave the project's batch mechanism behind.

**Likely venue.** SEA or ALENEX for index and benchmark. Social Networks or JCGS if integrated into
EstimNetDirected.

**Score.** Novelty 6/10. Feasibility solo 8/10.

---

## Candidate 2: sparse exact evaluation of whole-row steps

**Claim.** In models whose step scores every dyad of one vertex (the SAOM ministep, LOLOG vertex arrival), the same
chain can run in time proportional to the vertex's two-hop wedges plus its attribute classes, instead of `Θ(n)`,
because alters with no triadic contact share one score.

**Type.** Method with an exact-equivalence proof; empirical.

**Closest work.**

| Work | Read | Overlap |
|---|---|---|
| [RSiena](https://github.com/stocnet/rsiena) `TwoPathTable.cpp`, `ConfigurationTable.cpp`, `NetworkVariable.cpp` | Code read | **Highest.** One `n`-int table per configuration, zeroed by an `n`-step loop, recomputed for each ego. `calculateTieFlipContributions` loops over all alters and effects |
| RSiena `PrimarySetting.cpp`, `PrimaryLayer.cpp`, same repository | Code read, partly | **High.** A maintained 2-path layer restricts the choice set. This changes the model, so it is not an exact equivalent |
| [lolog](https://github.com/statnet/lolog) `Stats.h`, `LatentOrderLikelihood.h` | Code read | **High.** Nested loop over all earlier vertices. Undirected shared neighbours by sorted merge |
| [Fellows 2018](https://arxiv.org/abs/1804.04583) | Abstract, passages | **Medium.** States `O(n²)` to generate a network. A vertex order is the generative process |
| [Penschuck et al. 2020](https://arxiv.org/abs/2003.00736) | Sections on Bernoulli sampling and clustering generators | Low to medium. Geometric-skip sampling for independent dyads, which is the skip trick needed here |

**Precise gap.** No source I read evaluates an unrestricted row sparsely. This is weak evidence: the arXiv API
returned nothing useful and web search found no benchmark either way.

**Mechanism sketch.** Keep a stamped counter array and a touched list, so no `Θ(n)` reset is needed. For ego `x`,
walk `N(x)` then `N(y)`, counting in `cnt`. Score touched alters directly. Group untouched alters by attribute class
and draw a class by total mass, then an alter uniformly, rejecting touched ones. For LOLOG, draw geometric skips
over each class within the row, and add the neighbours of each new tie to the touched list. Exactness follows
because alters with equal class and `cnt = 0` have equal probability.

This is kernel R with `C` set to all alters. Kernel O does not apply, since it enumerates pairs inside `N(x)` and
most alters are not neighbours.

**Why it escapes the Gate 0 cause.** The row is the step, so nothing is hoped for. The cost floor is `Θ(n)` per step,
not about 10 operations. The graph changes between steps, so static precomputation is unavailable.

**Strongest simple baseline.** Dense row with a touched-list reset, which removes the zeroing cost only. For
RSiena the practical rival is the settings model.

**Cheapest kill test, under one hour.** Write a driver on `Graph` for Holme-Kim graphs at `n = 10^4` and `10^5`, plus
a snapshot. Run 10^5 ministeps with outdegree, transitive triplets and a two-class covariate. Count operations for
dense and sparse rows. Confirm equal distribution by chi-square against the dense sampler at small `n`. **Kill if
mean two-hop wedges exceed `n/5`,** or if the target models need dense dyadic covariates, which restore `Θ(n)`.

**Who needs it and what it costs them.** SAOM users, though few run large networks. The RSiena manual advises
representing meeting opportunities for large networks and warns that some effects built on dyadic covariates, held
as square arrays, are slow when `n` is large. Demand is the weak point: I found no benchmark of RSiena at `n > 10^4`,
and the model's full-choice assumption may be why. LOLOG users face `O(n²)` per simulated graph, across many
simulated graphs. That community looks small.

**Likely venue.** Social Networks, Network Science, or JSS. A CS venue is unlikely.

**Score.** Novelty 5/10. Feasibility solo 6/10, since each RSiena effect needs review or a separate prototype.

---

## Candidate 3: fast clustering-targeted rewiring

**Claim.** Degree-preserving rewiring towards a triangle or clustering target can scale well past current tools by
pairing cheap exact four-probe deltas with wedge-closing proposals, starting from the 86-hour data point below.

**Type.** Empirical, engineering.

**Closest work.**

| Work | Read | Overlap |
|---|---|---|
| [RandNetGen](https://github.com/polcolomer/RandNetGen) `annealingTRI.c`, readme | Code read | **Highest.** `calc_AH_TRI` computes an exact delta without applying it, by nested loops over both adjacency lists for each of four pairs |
| [Orsini et al. 2015](https://arxiv.org/abs/1505.07503) | Rewiring passages | **High.** `dk` Metropolis rewiring with `min(1, e^(-βΔH))` |
| [Gjoka et al. 2.5K](https://arxiv.org/abs/1208.3667) | Relevant sections | **High.** Naive swap chains stall, and their improved chain still faced scaling problems. Fix: start triangle-rich, then destroy |
| [Ferenczi, Grigoriev 2026](https://arxiv.org/abs/2602.19595) | Abstract, §3 passages | **Medium.** Local delta from common neighbours in `O(δ_max)`, inside an ACO and MCMC hybrid at small scale |
| [Nishimura 2017](https://arxiv.org/abs/1704.01951) | Abstract | **Medium.** Gives degree and triangle constraints whose state spaces stay disconnected under all swaps of up to 8 edges. Bounds what a *hard* constraint can do. Soft targets are unaffected |

**Precise gap.** Between RandNetGen's nested loops and a large-scale tool, I found nothing. Most of the first-order
gain is a plain data-structure fix, so that alone is not a paper. A paper needs either a chain that closes wedges by
construction with a correct Hastings ratio, or a scale result nobody has reached.

**Mechanism sketch.** Compute `Δ` for a swap of `(a, b), (c, d)` into `(a, d), (c, b)` from four lower-degree scans,
with corrections for triangles that hold two changed edges. Use stamps so `N(a)` and `N(c)` are each marked once.
That sharing is a star of size 2, so it is small. For a `C(k)` target, the delta needs the common neighbours
listed, not just counted, so count tables help less here than in candidate 1.

**Why it escapes the Gate 0 cause.** Only partly. Proposals are rejected at high rates near the target, and hub pairs
recur, so cost is in repeated probes. There are no large stars. Each RandNetGen proposal is quadratic in degree only
because of an implementation choice.

**Strongest simple baseline.** Gjoka's triangle-rich start plus swaps with lower-degree scanning.

**Cheapest kill test, under one hour.** On a synthetic scale-free graph of about 170,000 edges, with a copy of
`common` from `tc.hpp`, run a triangle-target chain, replay RandNetGen's nested loop by counting its reads, and
compare operations per accepted swap. Check against `static_count`. **Kill if the improvement is below 10×,** or if
the number of proposals needed grows so fast that wedge-closing proposals are required and cannot get a valid ratio.

**Who needs it and what it costs them.** Authors of `dk` null models. RandNetGen's readme reports about 86 hours for
a flat `C(k)` target with joint degrees preserved on 100,000 nodes and 169,518 edges, and 11 minutes on 24,000 edges.
Gjoka et al. report 118K to 381K seconds to reach a `c̄(k)` target on the New Orleans Facebook network from a plain
2K start, against 1K to 5K seconds from their triangle-rich start.

**Likely venue.** Journal of Complex Networks, Network Science, or a short SEA paper.

**Score.** Novelty 4/10. Feasibility solo 8/10.

---

## Candidate 4: Curveball trades as two-star batches

**Claim.** A Curveball trade is two stars by construction, so the project's mechanism applies with no change and
yields a per-trade triangle delta.

**Type.** Theory and empirical. Included because it is the only place the mechanism fits as it stands. **I expect
it to die quickly,** for the reason under "Why it escapes".

**Closest work.**

| Work | Read | Overlap |
|---|---|---|
| [Carstens et al. 2018](https://arxiv.org/abs/1804.08487) | Trade definition, grep for triangle | **High.** Defines the trade and the global trade. Triangles only as a mixing property |
| [Ajwani et al., SEA 2026](https://drops.dagstuhl.de/storage/00lipics/lipics-vol371-sea2026/html/LIPIcs.SEA.2026.2/LIPIcs.SEA.2026.2.html) | Read through a WebFetch summary, not directly | **High.** Tracks edges and wedges for autocorrelation; wedge tracking was costly for small gain. No triangle maintenance |
| [Allendorf et al. 2022](https://arxiv.org/abs/2111.03005) | Skim, grep | Medium. At most four edges per switch; triangles only as a less sensitive measure |
| [NetworKit](https://github.com/networkit/networkit) `randomization/` | File list; `EdgeSwitching.cpp` grepped | Low. No triangle code in `EdgeSwitching.cpp`. Curveball files not read |
| [Dhulipala et al. 2021](https://arxiv.org/abs/2003.13585) | Per audit row 2; not re-read | Low. Batch maintenance with no notion of a trade |

**Precise gap.** No source I read tracks triangles per trade, probably because nobody needs to.

**Mechanism sketch.** For a trade of `u` and `v`, peel four star groups: deletions at `u` and `v`, then insertions.
Run the existing selector per group. No code change beyond generating the groups.

**Why it escapes the Gate 0 cause: it probably does not.** Let `a` and `b` be the private neighbours of `u` and `v`.
Under a uniform reshuffle, each private neighbour stays with its side with probability `a/(a+b)` or `b/(a+b)`, so
each side moves about `ab/(a+b)` edges, which is at most `min(a, b)` (my calculation). A hub trading with a low-degree
vertex therefore moves few edges and `R` is large. Uniform trades mostly pair low-degree vertices, where updates are
cheap. This is the Gate 0 picture again. Only hub-hub trades give large stars, and they are rare.

**Strongest simple baseline.** Static recount at the sampling gap. A chain needs the count only at thinned samples,
and a global round touches every vertex anyway.

**Cheapest kill test, under one hour.** Ten minutes on `Engine` in `tc.hpp`: 10^5 uniform trades on a Holme-Kim graph
and a snapshot, with hindsight ratio from `collect_oracle`. **Kill if the oracle ratio is above 0.67,** which is the
Gate 0 go threshold. Separately, kill if per-trade tracking costs more than recounting at the usual gap.

**Who needs it.** No one I can identify today. The nearest demand is cheap mixing diagnostics, and SEA 2026 found
wedge tracking poor value.

**Likely venue.** At best an ALENEX note.

**Score.** Novelty 6/10. Feasibility solo 8/10. The real limit is demand, not either score.

---

## Rejected ideas

| # | Idea | Killed by | How |
|---|---|---|---|
| 1 | Kernel for coloured or typed triangles under edge updates | [Eppstein, Spiro](https://arxiv.org/abs/0904.3741) Theorem 5 | `O(h + k)` per update for `k` colours. Residual gap: colour *change* on a changing graph. No demand found, since colours are exogenous in ERGM and the graph is exogenous in ALAAM |
| 2 | Signed or balanced triangle maintenance under sign flips | Same paper, Theorem 4 | Weight edges `±1`. Total weight `W = B − U`, and unsigned count `T = B + U`, so `B = (T + W)/2`. A flip is a delete and an insert. Derivation is mine. [Secondary source](https://arxiv.org/abs/2602.14084) calls Arya et al. ITC incremental; I could not open the primary |
| 3 | Kernel O for vertex flips on a fixed graph (ALAAM, opinion or spin models) | [ALAAMEE](https://arxiv.org/abs/2404.03116) | The network is exogenous, so the paper notes that the two-path lookup never has to be updated. A stored per-vertex triangle list then costs `t(v)`, and kernel O costs at least that. Its `changeTriangleT3` is an `O(d²)` pair loop: a patch, not a paper. [Malarz, Wołoszyn](https://arxiv.org/abs/2007.02128) is a lattice example of the same setting (abstract only) |
| 4 | Dynamic triad census | [Eppstein et al.](https://arxiv.org/abs/1009.0783) | All directed 3-vertex counts in `O(h)` amortized per update. [Lienert et al.](https://arxiv.org/abs/1802.01481) covers the coloured static census (abstract and intro) |
| 5 | Vertex deletion kernel as a new algorithm | Chiba-Nishizeki, as implemented in [NetworKit](https://github.com/networkit/networkit) `ChibaNishizekiTriangleEdgeScore.cpp` | Mark `N(u)`, scan each neighbour's list, delete `u`. That is kernel O with `R` empty, without the orientation. 1985 original not read |
| 6 | Vertex peeling as the forced workload | [Shi, Dhulipala, Shun](https://arxiv.org/abs/2002.10047) (abstract and grep) | ARB-PEEL removes lowest-count vertices by rounds, with parallel work tied to Chiba-Nishizeki |
| 7 | Beating the worst case for vertex updates | [van den Brand et al.](https://arxiv.org/abs/2307.09961) §1 | Conditional on OMv, no `O(n^(2-ε))` online vertex update on directed graphs |
| 8 | A theory paper on the probe-commit trade-off | [Kopelowitz et al.](https://arxiv.org/abs/1407.6755), Eppstein-Spiro | Balancing `C_commit + ρ · C_probe` with a threshold is one line. Keep it as a lemma in candidate 1 |
| 9 | Star peeling on ERGM proposals | [ergm](https://github.com/statnet/ergm), [Byshkin et al. 2016](https://stivalaa.github.io/AcademicWebsite/preprints/IFDsampler_preprint.pdf) | Proposals are one dyad, or a pair of dyads, at unrelated vertices. There is nothing to group |
| 10 | Batch kernels for global switching or Global Curveball rounds | [Carstens et al.](https://arxiv.org/abs/1804.08487), [Allendorf et al.](https://arxiv.org/abs/2111.03005) | A round touches nearly every vertex, so a static recount is the baseline |
| 11 | Exact triangle-preserving swap chains | [Nishimura 2017](https://arxiv.org/abs/1704.01951) | Disconnected spaces for swaps up to 8 edges (examples given in the abstract) |
| 12 | Generators that hit triangle targets by maintaining counts | [Penschuck et al.](https://arxiv.org/abs/2003.00736) §8, NetworkX `random_clustered.py` | BTER, Darwini and the triangle-degree model hit targets by construction, with no count maintained |
| 13 | Incremental views for labelled triangles | [Olteanu 2024](https://arxiv.org/abs/2404.17679) (abstract) | Not killed outright. Single-tuple delta queries cover an attribute change as a delete and an insert. I did not read far enough to say more. Parked |

## Parked, not ranked

**Rejection-free triangle ERGM.** With a wedge table, dyads can be bucketed by edge state and `c`, giving an exact
jump chain for edges plus triangles. ergm's `SPDyad` already samples uniformly from dyads with a shared partner,
inside Metropolis, and [Zanella](https://arxiv.org/abs/1711.07424) (abstract) gives the general framework. A web
search and an arXiv abstract query found no informed or rejection-free ERGM sampler, which is weak evidence. The
nearest hit is the [Snake algorithm](https://arxiv.org/abs/2608.17531) (abstract, August 2026), a rejection-free
sampler for fixed margins with no triangle term. It inherits candidate 1's memory wall and needs a statistical
evaluation of mixing, so run it only if candidate 1 survives.

## Source ledger

Links opened, and how far. Sources in the tables above carry their read level there and are not repeated.

| Source | Read | Used for |
|---|---|---|
| [Krivitsky et al., ergm 4 computational](https://arxiv.org/abs/2203.08198) | Grep only | No shared-partner cache content found there; ergm `inst/NEWS.Rd` records the cache |
| [ergm NEWS](https://github.com/statnet/ergm/blob/master/inst/NEWS.Rd) | Grep | Reports a large speedup from the cache; `triadic` hint |
| ergm `src/cpp_changestats.cpp`, `src/MHproposals_triadic.cpp`, `R/InitErgmProposal.R` | Code read | `triangle` scan, `SPDyad` |
| [Byshkin et al. 2016](https://stivalaa.github.io/AcademicWebsite/preprints/IFDsampler_preprint.pdf) | Sampler and results passages | Acceptance 15% and 0.4% |
| [Byshkin et al. 2018](https://arxiv.org/abs/1802.10311) | Grep | EE background |
| [RSiena manual](https://www.stats.ox.ac.uk/~snijders/siena/RSiena_Manual.pdf) | Grep | Large-network advice, dyadic covariate cost |
| NetworkX [`swap.py`](https://github.com/networkx/networkx/blob/main/networkx/algorithms/swap.py), [`triads.py`](https://github.com/networkx/networkx/blob/main/networkx/algorithms/triads.py) | Grep | No triangle tracking; static census |
| [graph-tool `random_rewire`](https://graph-tool.skewed.de/static/docs/stable/autosummary/graph_tool.generation.random_rewire.html) | Doc page through WebFetch | No clustering target mentioned |
| [Bannink, van der Hofstad, Stegehuis 2017](https://arxiv.org/abs/1711.06137) | Title page only | Switch-chain mixing; not relied on |
| [Shang, Zhang, Tang 2026](https://arxiv.org/abs/2606.08786) | Abstract | Inference for balance; abstract does not mention counting |
| [Kashyap, Ambika 2017](https://arxiv.org/abs/1705.01689) | Abstract | Clustering rewiring; whether the delta is exact is not stated there |

Not opened, so not relied on: the original Chiba-Nishizeki paper, Arya et al. (the fetched page was unreadable),
Clark and Handcock 2022, the Stivala bipartite ERGM full text, and An et al. on the physician network.
