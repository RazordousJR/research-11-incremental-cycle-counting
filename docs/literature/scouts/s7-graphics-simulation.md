# Scout S7: exact triangle and clique counts in graphics and simulation

Date: 2026-11-01. Cut-off: 1 November 2026. Direction: the PI's home field. Scout-grade evidence, not a gate.

## Bottom line

1. **Batch sharing does not survive the move to proximity graphs.** On 31 synthetic moving-point configurations,
   the adaptive method lost to plain scanning in all 31 (ratio 1.027 to 1.439). With free hindsight selection and no
   overhead it could save at most 2.3% (ratio 0.977 to 1.000), against 13% on the Gate 0 streams and the 33% that
   Gate 0 needed. All 31 ended exact against a static count.
2. **The hopeful premise in the brief is mostly false at simulation time steps.** Few edges change per step, and
   whole vertices rarely change. The published and measured fractions are below. Whole vertices change only at
   coarse frame spacing, and there a static recount wins anyway.
3. **Temporal coherence is real, but plain per-edge scanning already collects it.** Scanning beat a static recount by
   up to 167 times on operations at stride 1, or 26 times once the pair-list diff is charged. No new method is needed
   for that, so it is not a contribution.
4. **No consumer found needs exact triangle counts per frame in graphics or simulation.** The one honest use is
   topological description of collective motion, where published point counts are 20 to 500 and the bottleneck is
   persistence reduction, not clique enumeration.
5. **Best remaining candidate:** a spatially resolved Euler signature for crowd regimes (C1), novelty 6, feasibility 8.
   It is a topology-for-crowds paper and drops incremental maintenance. It would not be a Research 11 method paper.

## Method and limits

Sources: web search, direct arXiv PDFs through `pdftotext`, the C++ prototype. The "Read" columns give the depth
reached for each source.

- **The arXiv API returned HTTP 429 on every attempt** and OpenAlex returned 429 after its first test call. The
  structured screening the brief asked for did **not** happen. Absence claims below rest on web search only, so read
  "not found" as weaker than a screened result.
- DBLP was not used. No Browser or Chrome tool was used.
- Elsevier and Wiley scope pages returned 403. Venue scopes come from mirrors, each named where used. Quartiles come
  from the PI's spreadsheet, which itself marks them as not re-verified.
- Blocked and not cited: Tordesillas et al. on 3-cycles (HAL, bot check, not bypassed) and Ferrez and Liebling on
  dynamic Delaunay contact (EPFL 429, the CORE copy would not parse).
- Need statements are paraphrased with a pointer to the location, apart from one short quotation, to stay inside
  quoting limits. The brief asked for quotes, so this is a deliberate departure.

## Kill test run for the whole direction

This goes beyond a literature scan because the fraction of edges changing was the deciding number and could be
measured in minutes. **Label: fully synthetic.** It cannot support a claim on its own.

- Driver: `C:\Users\esport34\AppData\Local\Temp\claude\C--Develop-20261031---Research-11-Local-Incremental-Triangle-Counting-in-Dynamic-Graphs\ee45f6ca-1e1f-4c35-84d3-b548251876ff\scratchpad\s7\kill\mp.cpp`.
  Includes `src/r11/tc.hpp` read-only. No project code changed. This folder is session scratch, so copy the driver
  into the repository if it is wanted later.
- Each frame builds a radius graph, takes the net difference, and applies it with `EdgeOnly` and `Adaptive`. Each
  result is checked against `static_count`. 30 frames per configuration (16 for stride sweeps, 40 for n = 300).
- Charges follow Gate 0: 2 per net update, plus 3 per net update for grouping. "Diff" adds 2m per frame, the cost of
  comparing a freshly emitted pair list with the previous one.
- Crowd rows are straight-line walkers with speed spread and heading jitter, **no collision response and so no real
  lanes**. MD is ballistic, an upper bound on a caged liquid. SPH is advection in a frozen smooth field at CFL-size
  displacement, every particle at full field speed. Vicsek uses the Xian et al. noise and speed, with vector-mean
  headings and a minimum-image metric on all coordinates, both kinder than or different from their text.
- Timings are one unpinned run on a shared machine with the prototype's hash adjacency. They are indicative only.

An earlier pass of the Vicsek and Xian rows used noise π times too large. Those rows were rerun with U(−η/2, η/2) and
the values below are the corrected ones.

| Config | n | Mean degree | f | dT/T | Whole-vertex share | f, 16 scales | Ops per net update | Adaptive / edge | Hindsight / edge | Edge / static | Edge + diff / static | Search ms | Static ms | Edge ms |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| crowd2way dt=0.10 r=2 | 20000 | 12.6 | 0.102 | 0.201 | 0.037 | 0.564 | 25 | 1.156 | 1.000 | 0.207 | 0.368 | 7.36 | 8.44 | 6.64 |
| crowd2way dt=0.10 r=5 | 20000 | 78.6 | 0.041 | 0.081 | 0.000 | 0.324 | 152 | 1.043 | 1.000 | 0.098 | 0.130 | 29.81 | 106.77 | 58.20 |
| crowd2way dt=0.25 r=2 | 20000 | 12.6 | 0.263 | 0.503 | 0.339 | 0.801 | 24 | 1.177 | 1.000 | 0.514 | 0.675 | 8.25 | 10.38 | 18.35 |
| crowd2way dt=0.25 r=5 | 20000 | 78.6 | 0.105 | 0.207 | 0.000 | 0.603 | 149 | 1.048 | 1.000 | 0.251 | 0.283 | 29.95 | 112.20 | 144.95 |
| crowd1way dt=0.10 r=2 | 20000 | 12.6 | 0.036 | 0.071 | 0.005 | 0.284 | 26 | 1.137 | 1.000 | 0.074 | 0.234 | 8.30 | 10.45 | 3.30 |
| crowd2way r=2 stride=2 | 20000 | 12.6 | 0.200 | 0.388 | 0.185 | 0.699 | 25 | 1.172 | 1.000 | 0.396 | 0.556 | 8.27 | 10.59 | 14.09 |
| crowd2way r=2 stride=5 | 20000 | 12.6 | 0.508 | 0.929 | 0.893 | 0.881 | 23 | 1.209 | 0.999 | 0.928 | 1.089 | 8.44 | 11.10 | 32.33 |
| crowd2way r=2 stride=10 | 20000 | 12.6 | 0.993 | 1.572 | 1.000 | 0.946 | 19 | 1.263 | 0.989 | 1.550 | 1.711 | 8.55 | 10.72 | 49.95 |
| crowd2way r=2 stride=20 | 20000 | 12.6 | 1.534 | 1.904 | 1.000 | 0.980 | 15 | 1.330 | 0.977 | 1.912 | 2.072 | 8.26 | 10.54 | 59.41 |
| crowd2way r=2 stride=50 | 20000 | 12.6 | 1.873 | 1.993 | 1.000 | 0.995 | 13 | 1.392 | 0.979 | 1.962 | 2.122 | 8.18 | 10.35 | 64.22 |
| crowd r=2 spawn q=0.01 | 20000 | 12.6 | 0.138 | 0.252 | 0.328 | 0.559 | 23 | 1.179 | 0.995 | 0.251 | 0.411 | 8.21 | 10.36 | 9.84 |
| crowd r=2 spawn q=0.05 | 20000 | 12.6 | 0.285 | 0.455 | 0.735 | 0.593 | 19 | 1.226 | 0.986 | 0.427 | 0.586 | 8.23 | 10.49 | 15.84 |
| crowd r=2 spawn q=0.20 | 20000 | 12.5 | 0.784 | 1.077 | 0.984 | 0.711 | 16 | 1.287 | 0.979 | 0.995 | 1.156 | 8.11 | 10.46 | 31.23 |
| md-ballistic r=2.5 dt=.005 | 20000 | 52.3 | 0.007 | 0.013 | 0.000 | 0.036 | 101 | 1.038 | 1.000 | 0.016 | 0.063 | 45.23 | 60.58 | 6.88 |
| sph-field r=2 cfl=.4 | 20000 | 33.6 | 0.116 | 0.226 | 0.006 | 0.407 | 63 | 1.077 | 1.000 | 0.261 | 0.331 | 33.58 | 32.79 | 42.69 |
| vicsek pos eta=0.1 r=1 | 20000 | 4.0 | 0.004 | 0.008 | 0.309 | 0.019 | 11 | 1.273 | 1.000 | 0.006 | 0.301 | 4.32 | 3.83 | 0.13 |
| vicsek pos eta=0.1 r=3 | 20000 | 24.6 | 0.004 | 0.008 | 0.005 | 0.024 | 50 | 1.068 | 1.000 | 0.009 | 0.088 | 10.44 | 17.41 | 1.15 |
| vicsek pos eta=1.0 r=1 | 20000 | 3.6 | 0.016 | 0.036 | 0.346 | 0.107 | 10 | 1.299 | 1.000 | 0.025 | 0.333 | 4.35 | 4.50 | 0.44 |
| vicsek pos eta=1.0 r=3 | 20000 | 21.4 | 0.008 | 0.014 | 0.008 | 0.058 | 43 | 1.080 | 1.000 | 0.015 | 0.104 | 10.17 | 15.81 | 1.60 |
| xian eta=0.1 e=0.10 s=1 | 300 | 22.2 | 0.020 | 0.036 | 0.032 | 0.199 | 43 | 1.086 | 1.000 | 0.036 | 0.123 | 0.35 | 0.20 | 0.03 |
| xian eta=0.1 e=0.10 s=10 | 300 | 26.2 | 0.019 | 0.033 | 0.045 | 0.161 | 45 | 1.082 | 1.000 | 0.033 | 0.110 | 0.28 | 0.24 | 0.05 |
| xian eta=0.1 e=0.35 s=1 | 300 | 121.7 | 0.009 | 0.016 | 0.006 | 0.092 | 195 | 1.027 | 1.000 | 0.018 | 0.039 | 0.51 | 2.04 | 0.27 |
| xian eta=0.1 e=0.35 s=10 | 300 | 138.6 | 0.013 | 0.023 | 0.018 | 0.101 | 207 | 1.030 | 1.000 | 0.026 | 0.045 | 0.47 | 2.28 | 0.47 |
| xian eta=1.0 e=0.10 s=1 | 300 | 8.9 | 0.604 | 1.102 | 0.959 | 0.870 | 18 | 1.275 | 0.994 | 1.003 | 1.189 | 0.36 | 0.07 | 0.17 |
| xian eta=1.0 e=0.10 s=10 | 300 | 11.5 | 0.668 | 1.147 | 0.973 | 0.882 | 22 | 1.219 | 0.990 | 1.081 | 1.227 | 0.39 | 0.11 | 0.29 |
| xian eta=1.0 e=0.35 s=1 | 300 | 80.2 | 0.193 | 0.344 | 0.171 | 0.683 | 136 | 1.040 | 0.999 | 0.421 | 0.453 | 0.58 | 1.19 | 2.17 |
| xian eta=1.0 e=0.35 s=10 | 300 | 92.2 | 0.213 | 0.345 | 0.410 | 0.711 | 135 | 1.035 | 0.997 | 0.398 | 0.426 | 0.51 | 1.10 | 2.56 |
| xian eta=2.0 e=0.10 s=1 | 300 | 4.7 | 1.164 | 1.709 | 1.000 | 0.936 | 11 | 1.439 | 0.983 | 1.547 | 1.786 | 0.45 | 0.06 | 0.15 |
| xian eta=2.0 e=0.10 s=10 | 300 | 5.6 | 1.227 | 1.744 | 1.000 | 0.944 | 12 | 1.409 | 0.984 | 1.612 | 1.830 | 0.48 | 0.06 | 0.20 |
| xian eta=2.0 e=0.35 s=1 | 300 | 59.5 | 0.461 | 0.744 | 0.811 | 0.819 | 93 | 1.051 | 0.996 | 0.869 | 0.909 | 0.55 | 0.73 | 2.62 |
| xian eta=2.0 e=0.35 s=10 | 300 | 61.1 | 0.607 | 0.938 | 0.958 | 0.872 | 90 | 1.037 | 0.978 | 1.061 | 1.099 | 0.59 | 0.76 | 3.52 |

Columns. *f*: net edge changes over m. *Whole-vertex share*: net updates touching a vertex with at least half its
edges changed. *f, 16 scales*: edges whose bin changes on a 16-scale grid, the coherence that a multi-scale curve sees.
*Edge*: sequential scanning. *Hindsight*: the cheaper of kernels A and O per group, free, as in the post-Gate 0
diagnostic.

### What the table says

**Premise check.** At stride 1, f runs 0.004 to 0.26 on the crowd, MD, SPH and position-only Vicsek rows. Published
figures agree on direction: Ihmsen et al. report that about 2% of particles change cell per step in SPH (CFL argument,
section on compact hashing), and GROMACS rebuilds pair lists only every 10 to 250 steps (Páll et al., pipeline figure
and dual pair list section). Large fractions appear only at coarse spacing (stride 10 gives f = 0.99) or with a heading
coordinate (Xian metric at ε = 0.35: 0.19 to 0.21 at η = 1, 0.46 to 0.61 at η = 2). The whole-vertex share at stride 1
is at most 0.35 outside the noisy heading rows, and those low-degree cases are near-trivial. It reaches 0.89 at
stride 5, where static recount is about to win.

**Change-rate law (this scout's derivation; probably folklore, no source found).** For uniform density and f well
below 1, with d the dimension:

    f ≈ 2 d c_d E|v_rel| Δt / r,   c_2 = 1/π,   c_3 = 1/4

Predicted against measured: 0.095 vs 0.102, 0.038 vs 0.041, 0.237 vs 0.263, 0.095 vs 0.105 (crowd), 0.0068 vs 0.0068
(MD). It underpredicts the one-way crowd (0.019 vs 0.036), where heading spread breaks its assumptions. Because
T scales as r^(2d) and m as r^d, also ΔT/T ≈ 2f, measured at 1.95 to 1.99. Applied to Kern and Köster's 0.2 s
snapshots with balanced counterflow, it gives f ≈ 0.12 at ε = 3.2 and 0.38 at ε = 1.0 (estimate, their data not run).

**Why batch sharing fails.** Gate 0 failed because updates were few and cheap. Here scans cost 24 to 152 operations
per net update on the crowd, MD and SPH rows at stride 1 (10 to 207 over all rows) against 5.9 to 13.9 on Gate 0 streams
at the 1,000-event cut, so "cheap" no longer holds. It fails
anyway. The earlier synthetic win ran on Holme-Kim graphs (`benchmarks/probe_ops.cpp`), which have hubs by
construction. Proximity graphs have nearly uniform degree, so marking N(x) once saves nothing against scanning the
same-size lists. Even with whole-vertex changes (spawn q = 0.20, stride 10 to 50) the hindsight ratio stays above 0.977.

**The coherence window.** Scanning beats a static recount on operations up to about f = 0.5 to 0.8, then loses (stride 5:
0.928; spawn q = 0.20: 0.995; stride 10: 1.550). On wall clock the crossover is much earlier, near f = 0.1 on this prototype: 58 vs 107 ms at
f = 0.04, 145 vs 112 ms at f = 0.105. Xian et al. subsample by 10. The same stride puts the crowd rows past both
crossovers, while ordered Vicsek rows stay coherent (f = 0.013 to 0.019), so this depends on the system.

**Neighbour search caps the gain.** A radius search costs about as much as the count at low degree. If the pair list
is rebuilt each frame, a free incremental count could speed the frame by only 2.0 to 4.6 times. A Verlet skin lowers
the search cost but leaves a per-frame distance filter.

**A multi-scale curve loses coherence.** With 16 scales, 0.28 to 0.80 of crowd edges change bin per frame at stride 1. Only
position-only Vicsek flocks stay low (0.02 to 0.11).

**Is the count a bottleneck per frame?** No, for the published consumers. At n = 300, one scale costs 0.06 to 2.3 ms on
this prototype. At n = 20,000 and degree 12.6, search plus static count is about 16 ms on one thread. At degree 78 it
is about 137 ms, which is over a 60 Hz budget but far denser than any published consumer needs.

## Exact numbers for the 2-skeleton

| Quantity | Value | Source |
|---|---:|---|
| Changed simplices per frame, crowd, n = 20,000, f = 0.10 | 12,814 edges, 61,914 triangles | Run above |
| Maintaining the 2-skeleton, hash adjacency, counting only | 60 to 119 ns per changed triangle | Run above, 4 rows |
| Static enumeration | 8.8 to 27 ns per triangle | Run above, 4 rows |
| FastZigzag, oscillating Rips, rows 9 to 11 | 1.7 to 4.1 µs per simplex | Dey and Hou, Table 1, my division |
| FastZigzag, edge-toggle clique filtrations, rows 3 to 8 | 36 to 341 µs per simplex | Same |
| Ripser, dragon, 2,000 points, dimension 1 | 2.4 s, 96.7 MB | Bauer, Table 1 |

**So the 2-skeleton step is at most about 7% of the zigzag pipeline** (119 ns over 1.7 µs) and under 1% on the harder
rows. The ratio crosses machines and my figure counts rather than emits simplices, so read it as an order of
magnitude. Dey and Hou do not report generation time. Clique maintenance is therefore not where a persistence
pipeline spends its time. At 1.7 to 4.1 µs per simplex, the crowd frame above would take 0.13 to 0.31 s of reduction,
a projection from a different machine and filtration shape.

## Field by field

| Field | Real use of the count | Bottleneck per frame? | Anyone incremental? | Coherence helps? |
|---|---|---|---|---|
| Crowd neighbour graphs | Crowd regime description through topology (Kern and Köster). Group detection uses pairwise affinity, not cliques (Solera et al.) | No. n is tens to hundreds | Not found | Single scale yes, multi-scale no |
| Cloth and rigid contact | None found. Solvers need colouring, not counts (VBD; Box2D) | No | Not applicable | Not applicable |
| Granular force chains | Real science: 3-cycles as stabilising structures (Papadopoulos et al.) | No. A statistic on chosen snapshots in what was read | Not found | Moot |
| Rips and persistence of moving points | Real, the strongest line. The filled triangles are exactly proximity-graph triangles | Yes, but in reduction, not clique listing | Reduction is warm-started or zigzagged. Clique listing is not studied separately | Yes for reduction |
| Kinetic data structures | Kinetic Rips not found. The kinetic hourglass maintains a matching | Not applicable | No | An event per changed edge is costly at f = 0.1 |
| Remeshing and flips | None. The mesh is explicit, so the count is a by-product of each edit (Mahmoud et al.) | No | Trivially | Moot |
| GPU neighbour queries | Upstream of all of this | Search is the comparable cost | Yes, for the search itself (Xing et al.; Ihmsen et al.) | Already exploited |

## Candidates, ranked

### C1. Spatially resolved exact Euler signature for crowd regimes

**Claim.** Exact clique counts per cell of a crowd proximity graph separate crowd regimes about as well as CROCKER
plots of Betti numbers, and unlike CROCKER plots they show where a regime occurs.

**Type.** Empirical, with a small method.

**Who needs the output.** Kern and Köster (§1, §4) want to classify crowd phenomena without hand-crafted metrics and
plan to move to harder topographies and real data. Their Vadere plus `ripser` pipeline would take this output in
place of, or beside, its CROCKERs. Xian et al. (§5.2.4) report "subsampling to every 10 time steps to speed up
computations" at n = 300. Hacquard and Lebovici (abstract) report that Euler profiles reach state-of-the-art
supervised performance at small cost.

| Closest paper | Read | Overlap |
|---|---|---|
| [Kern, Köster, A study of holes, arXiv:2607.06086, 2026](https://arxiv.org/abs/2607.06086) | §1, §2, conclusion; keyword scan of results | **Highest.** Same problem with Rips and CROCKERs, global per frame, Euler not used. Preprint, a follow-up could close the gap |
| [Roy, Mitra, Dutta, Euler Characteristic Surfaces, arXiv:2408.09400](https://arxiv.org/abs/2408.09400) | Passages of §1, §2.3.3, §3.2 | High. Time-by-scale Euler surface on Vicsek with N = 500. Alpha complex, global, no crowds |
| [Dłotko, Gurnari, Euler Characteristic Curves and Profiles, arXiv:2212.01666](https://arxiv.org/abs/2212.01666) | Passages of §1, §4, App. A | High for method. Per-vertex local contributions for Rips, static, summed to one global curve |
| [Xian, Adams, Topaz, Ziegelmeier, Capturing dynamics of time-varying data via topology, arXiv:2010.05780](https://arxiv.org/abs/2010.05780) | Passages of §5.2, §5.3 | Medium. CROCKER stacks on Vicsek, n = 300, 50 scales, global |
| [Hacquard, Lebovici, Euler Characteristic Tools, JMLR 25, 2024](https://jmlr.org/papers/volume25/23-0353/23-0353.pdf) | Abstract | Medium. Evidence the descriptor works, no dynamics |

Also seen: [tda4j PR 249](https://github.com/appliedtopology/tda4j/pull/249) (description only, merged 29 Oct 2026),
static Rips Euler curve from per-edge clique counts, not peer reviewed. A strong baseline implementation.

**Precise gap.** No source found applies an Euler signature to pedestrians, and none read gives a location: CROCKERs
and Euler surfaces are global per frame. Dłotko and Gurnari already split the Euler sum by vertex, but only to
distribute the work. Using that split as a spatial field for crowd regimes is the unclaimed step. Web search only.

**Mechanism.** At each snapshot and each of S scales, assign every clique to its lowest-ranked vertex (the Dłotko and
Gurnari rule), bin vertices into grid cells, and store V − E + T − K4 per cell. The tensor cell × scale × time goes
to PCA for regimes and to a heatmap for location. Run on 2-D positions and on 4-D time-delayed positions.

**Truncation caveat.** Stopping at K4 is not the Euler characteristic of the flag complex. Report the dimension and
check against the full count at small n.

**Gate 0.** Escapes by not relying on sharing, and the count is the final output, so no reduction stage dominates.
But f on 16 scales is 0.28 to 0.80 for crowds, so per-frame recount is the right tool. Incremental applies only to
one or two scales.

**Strongest simple baseline.** In 2-D, the alpha complex: exact, small, and all scales from one Delaunay pass (Roy et
al. take this route). Rips earns its place mainly on the 4-D delayed positions, where Kern and Köster report clear
separation. Also Ripser per frame, plus density and a lane order parameter, which any crowd reviewer will ask for.

**Kill test, under an hour.** Add a repulsive force to the generator so that lanes form, because the current one
cannot. Run 21 inflow scenarios with 5 seeds at 0.2 s snapshots. Compute per-cell V, E, T at 16 scales with a
per-vertex copy of `static_count` in a scratch file. **Kill if** nearest-centroid accuracy for flow type is below 0.9,
or if it fails to beat density plus lane order, or if the per-cell map correlates below 0.5 with local lane order.

**Venue.** Computer Animation and Virtual Worlds, Q2 on the PI's list. Scope per [myhuiban mirror](https://www.myhuiban.com/journal/155.md):
lists modelling of groups and crowds, and visualization of physical phenomena. Second:
[Computers & Graphics](https://www.myhuiban.com/journal/154.md), applications of CG to problem domains.

**Solution looking for a problem?** Partly. Speed is not pressing at Kern and Köster's scale, about 18 to 37 people in
the observation area by my flow estimate. The claim must rest on location and scale-out, not on speed. Reviewers may
also call this a known descriptor applied to a new field, which matches the PI's rejection history.

**Scores.** Novelty 6/10. Feasibility solo 8/10.

### C2. Where zigzag and warm starts beat per-frame recompute on moving points

**Claim.** For moving Rips complexes there is a measurable change fraction below which zigzag or warm-started
persistence beats per-frame recompute, and the law above predicts it from velocity, radius and frame step.

**Type.** Empirical, with one small lemma.

**Who needs the output.** Piekenbrock and Perea (§1) say practitioners have no recourse for time-varying complexes
with millions of simplices. Their flocking test has m = 20 vertices (§4). Flammer and Hüper (§5) need n·m barcodes
over a time-by-scale grid. Gamble et al. use zigzag on moving sensor coverage (abstract).

| Closest paper | Read | Overlap |
|---|---|---|
| [Dey, Hou, Fast Computation of Zigzag Persistence, ESA 2022](https://arxiv.org/abs/2204.11080) | Experimental section, Tables 1, 2 | High. The engine. Its Rips rows are oscillating Rips on static samples of 2,000 to 4,000 points, others are random edge toggles. No moving points |
| [Piekenbrock, Perea, Move Schedules, arXiv:2104.12285](https://arxiv.org/abs/2104.12285) | §1, §2.2, §4.2 | High. Moves beat vineyards and recompute, on 20 flocking vertices. Column operations, not time at scale |
| [Luo, Nelson, Warm Starts, arXiv:2108.05022](https://arxiv.org/abs/2108.05022) | Abstract, §1, algorithm sections skimmed | High. Batched insert and delete in factorisations, cost scales with changes |
| [Dey, Hou, Zigzag Vineyard, SoCG 2024](https://arxiv.org/abs/2307.07462) | Abstract, §1 | Medium. Theory, no experiments seen in the text searched |
| [Flammer, Hüper, Spatiotemporal Persistence Landscapes, arXiv:2412.11925](https://arxiv.org/abs/2412.11925) | Abstract, construction to §5, not §6 | Medium. Rips on each X_i and on X_i ∪ X_{i+1}, an application of the same delta |

Tooling: [GUDHI zigzag module](https://gudhi.inria.fr/doc/latest/group__zigzag__persistence.html) (page read) takes
one cell at a time, and [Ripser](https://arxiv.org/abs/1908.02518) (§1, keyword scan, results table) avoids storing the
coboundary, so a warm start looks less direct there (my inference, not tested).

**Precise gap.** No test found at n of 1,000 or more on actual moving points, and no reported crossover in f.

**Mechanism.** The prototype generates the simplex delta per frame. Feed it to FastZigzag, compare with Ripser per
frame at the same threshold, sweep Δt, and fit the crossover to the law.

**Gate 0.** Irrelevant, because the sweep sets f directly, 0.01 to 0.2. Be plain about the limit: triangle work
is at most about 7% of this pipeline, so this is a persistence benchmark, not a triangle paper.

**Strongest simple baseline.** Ripser per frame with a threshold. Note the outputs differ: zigzag tracks features
through time at one scale, Ripser gives all scales in one frame. A fair comparison must fix what is being asked.

**Kill test, under an hour.** n = 2,000, 200 frames, f from 0.01 to 0.2. **Kill if** generation plus FastZigzag exceeds
the summed Ripser time at every f. Needs two external builds, so it needs the PI's approval to download them.

**Venue.** Computational Geometry (Elsevier), on the PI's `ScienceDirect-CG.txt` but not in the Q1 and Q2 table. The
official scope page returned 403. Luo and Nelson's paper is listed there (DOI 10.1016/j.comgeo.2024.102089), which is
indirect evidence of fit. Quartile unverified.

**Solution looking for a problem?** No, the bottleneck is real. But it is outside the PI's field, which is the
rejection pattern to avoid.

**Scores.** Novelty 6/10. Feasibility solo 5/10.

### C3. Frame-coherent exact counting on unit-ball graphs

**Claim.** On moving unit-ball graphs, per-edge deltas with a distance test in place of a hash probe and half-space
pruning beat static recount up to a predicted f.

**Type.** Method.

**Who needs the output.** Nobody published needs this per frame at scale. It would serve C1 at large n if C1 survives.
**This is a solution looking for a problem today.**

| Closest paper | Read | Overlap |
|---|---|---|
| [Hanauer et al., arXiv:2610.29776](https://arxiv.org/abs/2610.29776) | Abstract here; main text per the audit | High. Scanning wins for triangles on general graphs. Geometric graphs not addressed in the abstract |
| [Dhulipala et al., arXiv:2003.13585](https://arxiv.org/abs/2003.13585) | Abstract here; sections per the audit | Medium. Batch counting, no geometry |
| [Xing, Wang, Hui, Coherent Spherical Range-Search](https://mewangcl.github.io/pubs/CADDistQuery.pdf) | Abstract, §1 | Medium. Coherence in the query step, not in counts. Venue not confirmed from the text read |
| [Ihmsen et al., A Parallel SPH Implementation on Multi-Core CPUs, CGF 2011](https://cg.informatik.uni-freiburg.de/publications/2011_CGF_dataStructuresSPH.pdf) | Compact hashing section | Medium. Same coherence idea on cells |
| [Chan, Finding Triangles in Geometric Intersection Graphs, SODA 2023](https://arxiv.org/abs/2211.05345) | Introduction | Low. Names counting on unit disk graphs as useful, then treats detection only. Not dynamic |

**Precise gap.** No source found maintains exact counts on a moving unit-ball graph. The gap is narrow and shallow.

**Mechanism.** Changed edges have length near r, so their lens is near minimal: 0.391 of the disk in 2-D, 5/16 of the
ball in 3-D. Measured hit rates are 0.42 and 0.335. Candidates can be cut to a half-space and membership needs no hash.

**Gate 0.** Escapes the "cheap" half (24 to 152 ops per update on the main rows), and uses no sharing. f: 0.004 to 0.26.

**Limit already measured.** The half-space scans 0.515 to 0.55 of the list on uniform rows, so the operation gain is
capped near 0.52 before sector upkeep, and it worsens to 0.76 on clustered and heading rows. Gate 0's bar was 0.67, so a realistic
sector structure may not clear it.

**Strongest simple baseline.** Plain scanning, then static compact-forward on the fresh list.

**Kill test, under an hour.** Add 8 sectors per vertex, charge their upkeep, rerun the table. **Kill if** total
operations over plain scanning exceed 0.67, or if wall clock does not beat static at f = 0.1.

**Venue.** Computers & Graphics, same mirror as above, weak fit without a graphics use.

**Scores.** Novelty 3/10. Feasibility solo 8/10.

## Rejected ideas

| Idea | Killed by | Why |
|---|---|---|
| Batch sharing on proximity graphs | This scout's run, with [Hanauer et al.](https://arxiv.org/abs/2610.29776) as the audit reports them | Hindsight cap 2.3% in 31 of 31 cells. No hubs, so nothing to share |
| 2-skeleton maintenance as a persistence speed-up | [Dey, Hou, Tables 1, 2](https://arxiv.org/abs/2204.11080); [Bauer](https://arxiv.org/abs/1908.02518) | At most about 7% of the pipeline. Ripser does not store it at all |
| Kinetic Rips with certificates | [Russel et al.](https://geometry.stanford.edu/lgl_2024/papers/rkg-pekdssa-07/rkg-pekdssa-07.pdf) (§1, framework, no rebuild comparison in the parts read); [Munch et al.](https://arxiv.org/abs/2505.04048) (abstract) | Russel et al. report stability and efficiency problems in earlier kinetic code. At f = 0.1 every tenth edge is an event per frame. Not refuted by timing: no head-to-head found. Kinetic hourglass is about matchings |
| Triplet lists for three-body and angular potentials | [Fan et al., GPUMD](https://arxiv.org/abs/1610.03343) (abstract, keyword scan); [Fan et al. 2013](https://arxiv.org/abs/1211.3892) (introduction passage); [Páll et al.](https://arxiv.org/abs/2006.09167) (pipeline figure, dual pair list section) | Force evaluation touches every triplet anyway, so listing is a minority share. Lists are reused for many steps, and in solids they need not change. A logical bound, not a timing |
| Triangles in cloth and rigid contact graphs | [VBD](https://arxiv.org/abs/2403.06321) (§3.9); [Box2D source](https://docs.rs/crate/box2d_sys/latest/source/box2d/src/constraint_graph.c) (header) | Solvers need colouring. VBD avoids recolouring for collisions altogether. No use of triangle counts found |
| Mesh connectivity under remeshing | [Mahmoud et al., Dynamic Mesh Processing on the GPU, TOG 2025](https://dspace.mit.edu/handle/1721.1/164757) (abstract) | Triangles are explicit, so a count is arithmetic on each cavity edit. Nothing to compute |
| Force-chain 3-cycles in granular media | [Papadopoulos et al., Network Analysis of Particles and Grains](https://arxiv.org/abs/1708.08080) (abstract, keyword scan of §2 and §3) | Real use, but a statistic on snapshots in what was read. Sparsity of contact graphs is my reasoning, not measured here, and would make static recount cheap |
| Clique-based crowd group detection | [Solera et al., Socially Constrained Structural Learning, TPAMI 2015](https://aimagelab-legacy.ing.unimore.it/imagelab/pubblicazioni/TPAMI2015.pdf) (abstract, §1) | The method is correlation clustering on learned pairwise affinity, not clique counts |
| Subquadratic exact counting on unit disk graphs | [Chan](https://arxiv.org/abs/2211.05345) (introduction) | Open and not solo work, and it would not help: practical degrees are bounded |

## What this does not establish

- Nothing about real crowds. The crowd rows have no collision response and form no lanes.
- The op-count wins for scanning say nothing about a tuned GPU implementation. Wall-clock figures are one unpinned run.
- "Not found" means web search, because both structured APIs were rate-limited. M1A-style screening should be rerun
  before any novelty claim, especially for C1, whose closest paper is four months old.
- C1's scores are unverified: its kill test has **not** been run.

## Recommendation

Close batch sharing for proximity graphs. If the PI wants to stay in the home field, run the C1 kill test next,
knowing it produces a crowd-topology paper and leaves Research 11's method behind. If the project must stay a
triangle-maintenance paper, this direction does not offer one.
