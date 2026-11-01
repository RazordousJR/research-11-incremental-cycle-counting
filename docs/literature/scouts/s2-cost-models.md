# Scout S2: non-unit cost models for exact batch triangle maintenance

Date: 2026-11-01. Direction: drop the unit-cost assumption of Gate 0 and find where one adjacency access is
expensive enough that grouping updates saves real money.

**Status of everything below.** No kill test here has been run. Nothing in this file is a measurement of ours.
Sections marked *my derivation* are mine, come from no paper I read, and have had no independent check.

## 1. Bottom line

1. **The cost gap is real.** One page read at queue depth 1 costs about 100 µs, which is 15 to 170 times the
   whole CPU cost of a Gate 0 update even if every counted operation were a full DRAM miss (§2). Batch
   overhead of a few operations per update stops mattering. So the Gate 0 cause does not carry over.
2. **But grouping updates into stars is not what wins, and I expect it to stay dead.** Once the buffer holds a
   batch's footprint, sequential scanning with plain LRU fetches exactly the same distinct pages as any
   batch-grouped schedule (*my derivation*, §4 C1). Without a cache the list-level saving is capped at
   2b/|V(Δ)|, the mean degree of the batch graph. What can beat LRU is one sequential sweep, once the batch
   reaches at least about 0.1% of the edges (more if the LRU hit rate is high), and asynchronous submission, which
   improves throughput without saving any I/O. Neither needs peeling.
3. **The opening I found is theoretical, and it is modest.** I found no I/O bound for batch-dynamic triangle
   counting. The bound Θ((m/B)·max(1, √(b/M))) looks reachable and tight in the witnessing model, and it gives a
   worst-case gain of min(√b, √M) over sequential processing. However, its shape follows from a known
   unequal-size load formula (Beame et al.), so the risk is that a referee calls it a corollary.
4. **The project's streams cannot show any of this on real hardware.** The active graph peaks at 99,908 edges,
   about 195 pages of 4 KiB. Insert-only wiki-talk-temporal is about 26 MB. All of it fits in RAM and inside
   LSMGraph's default 64 MB memtable. Any test here is a model test with an artificially small M.

## 2. What an access costs

Gate 0 reference: 5.9 to 13.9 operations per net update at the 1,000-event cut, and at most 64.4 in the
insert-only diagnostic. All figures below come from the source named. Most are from other hardware.

| Access | Cost | Source, how far read |
|---|---|---|
| DRAM load | Flash is "1000× slower than DRAM", so about 100 ns by implication | Haas & Leis §2.3 |
| Optane read | 2× to 3× DRAM | Yang et al. §3 |
| NVMe 4 KiB random read, synchronous | About 100 µs, so 10k IOPS, 40 MB/s | Haas & Leis §2.3 |
| NVMe 4 KiB random read, high queue depth | 1.56 M IOPS per drive, 12.5 M across 8 | Haas & Leis §1 to §2 |
| NVMe random against sequential bandwidth | 6 GB/s against 6.5 GB/s, a gap of 8% | Haas & Leis §2.2 |
| LSM neighbour read, Poly-LSM model | 2 + (d(u)+1)·I/B block I/Os | Aster §3.3 |
| LSM neighbour read, plain LSM worst case | O(L) disk I/Os, one per level | LSMGraph §2.2 |
| Chance of a delta entry at the top three levels, Wikipedia | 0.964, 0.284, 0.033 | Aster §3.3 |
| Get-neighbours on Twitter, measured | Aster 20.67 µs, NebulaGraph 844.17 µs, Neo4j 1302.51 µs | Aster Table 4. 128 GB RAM, NVMe, so much of this is likely cache |
| Decode one integer, best case | 1.5 cycles, SIMD-BP128, 2,300 M ints/s on 3.4 GHz | Lemire & Boytsov Table I |
| Compressed block size | Aspen chunks of expected size 2^8; PaC-trees B = 128 | Aspen §3 and §7.1, PaC-trees Fig. 1 |
| Compression, whole-algorithm slowdown | Sequential about 25% as quoted; parallel Ligra+ from 2.2× faster to 1.1× slower | Ligra+ §1 |

Arithmetic, mine:

- Gate 0 update if every operation were a DRAM miss: 0.6 to 6.4 µs. One synchronous page read: 100 µs, a factor
  of 15 to 170. At a nominal 5 ns per operation the factor is 300 to 3,000.
- **Caveat that weakens the headline.** At high queue depth one page costs about 0.64 µs of device throughput, so
  two pages per update is about 1.3 µs. Then I/O is roughly the same size as CPU work, and some of the Gate 0
  problem returns. The large ratio exists only for synchronous reads.
- Decoding half a block is 64 to 128 integers, or 96 to 192 cycles at the best-case rate. That is less than one
  DRAM miss. Byte codes are slower, but I found no figure for them. Decoding is expensive relative to one
  Gate 0 operation, about 50×, and not relative to memory.

## 3. Direct answers

| Question | Answer | Strength |
|---|---|---|
| Has anyone done batched exact triangle maintenance with I/O or decoding as the cost? | Not found. Closest are Mnemonic (batches, disk, triangle queries, no I/O metric seen) and Jiang et al. 2024 (semi-external, single-edge, kmax-truss only) | Weak. See §3.1 |
| Is there a provable I/O bound for batch-dynamic triangle counting? | Not found | Weak. A corollary-level result could exist unnamed |
| Does any external-memory triangle paper handle updates? | No, among Hu et al., Pagh & Silvestri, Cui et al.; in Dementiev §4.6 no update handling in the part read. GraphChi ingests updates but recounts in P static iterations | Medium: text searched for update terms, except that Menegola was seen only through Hu et al. and Dementiev only through §4.6 |
| Does batching cut I/O by a large factor over sequential scanning? | Worst case yes, by min(√b, √M). Real streams unknown; I expect little beyond LRU for b below about 0.1% of m | Model only, unmeasured |
| Does batching cut decoding by a large factor? | Unknown. Kill test C2 is the cheapest in this file | Unmeasured |

### 3.1 Search record and limits

Searched: 32 web search queries on external-memory, semi-external, SSD, LSM, compressed, persistent-memory,
out-of-core continuous matching and I/O-model batch-dynamic work, some in extended mode; 20 OpenAlex title
queries (from 2010, 2012 or 2013 to date); PDFs read as listed in §4, §5 and §7.

**Limits, stated plainly.**

- **Forward citations were not run.** OpenAlex began returning 429 partway through, with `Retry-After: 83717`
  seconds, about 23 hours, before the scan could start. So the papers that cite Hu et al. or Pagh & Silvestri were
  not screened. **This is the largest hole in this file.** The script is ready:
  `C:\Users\esport34\AppData\Local\Temp\claude\C--Develop-20261031---Research-11-Local-Incremental-Triangle-Counting-in-Dynamic-Graphs\ee45f6ca-1e1f-4c35-84d3-b548251876ff\scratchpad\s2tools\fwd2.py`.
- The arXiv API was throttled almost at once: three of nine early queries returned records, then came 429s. Three
  later queries returned zero results right after a run of 429s. One of them, `abs:"dynamic graph" AND abs:SSD`,
  cannot plausibly be empty, so I discarded all three, and the early empties too. **The arXiv API is effectively
  unscreened here.** arXiv papers below were found through web search.
- DBLP was not used, by rule. IEEE and ACM full text was read only where a free copy existed.
- OpenAlex search returned much noise on terms such as "disk" and "SSD".
- A lead I could not open: a paper on "page connectivity graph" page-fetch scheduling, seen only as a search
  snippet behind a firewall page. It suggests that minimum-fetch scheduling is NP-hard. I do not cite it.

Every "not found" in this file means "not found by these searches". Treat it as weak.

## 4. Candidates, ranked

### C1. Tight I/O bound and crossover for batch triangle maintenance

**Claim.** An exact batch of b updates on an m-edge disk-resident graph costs Θ((m/B)·max(1, √(b/M))) I/Os in the
witnessing model, which beats sequential processing by up to min(√b, √M), and on real streams the only regime
where batching beats sequential scanning with LRU is one sweep at b ≳ m/(Bμ).

**Type.** Theory first, with a small empirical part.

**Closest papers.**

| Paper | Read | Overlap |
|---|---|---|
| [Hu, Tao, Chung, TODS 2014](https://www.cse.cuhk.edu.hk/~taoyf/paper/tods14-triangle.pdf) | §1, §4.1, §6 opening | **High.** MGT Step 2 with Emem set to the batch *is* the one-sweep algorithm, one pass in O(|E|/B), with no degree assumption. No updates |
| [Pagh, Silvestri, PODS 2014](https://arxiv.org/abs/1312.0723) | §1, Theorem 3 | **High.** Colour coding and the bound Ω(t/(√M·B)) supply both proof tools. Static only |
| [Beame, Koutris, Suciu, JACM 2017](https://homes.cs.washington.edu/~beame/papers/mpc-final.pdf) | Passage on unequal triangle sizes in §3 | **Highest novelty risk.** For sizes M1 < M2 = M3 it states load M/p when p ≤ M/M1, then (M1M2M3)^(1/3)/p^(2/3). That is the same regime change. Parallel model, skew-free load, no updates |
| [Beame, Koutris, Suciu, arXiv:1604.01848](https://arxiv.org/abs/1604.01848) | §5 | **High.** Theorem 5.1 turns tuple-based MPC algorithms into external-memory ones, and Example 5.2 does triangles. The example is for equal sizes only, with polylog loss and W ≥ m^(2/5) |
| [Dhulipala, Liu, Shun, Yu, APOCS 2021](https://arxiv.org/abs/2003.13585) | Audit row 2 | Medium. Same problem, RAM work bound, no block cost |

Also [Koutris's open-problem notes](https://databasetheory.org/sites/default/files/blog_attach/open-problems-koutris-1_0.pdf)
(read the passage on page 6) list the constant-round MPC lower bound for different relation sizes as open. That is a
different model from the witnessing bound here, so it is a pointer and not a gap in this project's favour.

**Precise gap.** Not found: an external-memory statement for batch-dynamic triangles that (a) has one relation of
size b and two of size m, (b) handles skew in both batch and graph, (c) has no polylog loss, (d) handles mixed
inserts and deletes exactly, and (e) compares against sequential processing with a buffer pool.

**Mechanism sketch (*my derivation*, unchecked).** Let M be in words, and assume the usual semi-external
allowance of n bits in memory, as in Wen et al. and Jiang et al.

- *One sweep, b ≤ cM.* Hold Δ in memory. For each vertex u, stream N(u), keep N(u) ∩ V(Δ), and test Δ edges among
  those kept. This is O(scan(m) + sort(b)). Mixed updates: T(G′) = T(G) − D + A, where D counts triangles of G with
  an edge in Δ⁻ and A counts triangles of G′ with an edge in Δ⁺. Exactly-once by a canonical first Δ edge.
- *b > M.* Colour vertices with c ≈ √(b/M) colours. Each colour triple (i, j, k) sweeps the k-to-i and k-to-j edges
  against batch part Δij, of expected size b/c². Each edge is swept c times, so the total is O(sort(m) + c·m/B).
- *Batch hubs.* Vertices with batch degree above √(bM) break the colour balance. There are at most 2√(b/M) of them.
  Mark N(h) in the n-bit array and sweep once each, which is the same order. This is the one place where a star
  group reappears, as a proof device.
- *Lower bound.* Take A, B of size √b and C of size m/(2√b). Δ = A×B and G = (A∪B)×C give m√b/2 triangles, so
  Pagh & Silvestri Theorem 3 gives Ω(m√b/(B√M)). Reading the input gives Ω(m/B).
- *Separation.* On that instance sequential processing reads two lists of |C| words for each of b updates, about
  m√b/B I/Os, and LRU cannot rescue it when M < m.
- *LRU lemma.* If the pages touched by a batch fit in M, sequential LRU incurs one miss per distinct page, the same
  as a sorted batch schedule. So grouping can only help when the footprint exceeds M.
- *Crossover.* For 4-byte ids and 4 KiB pages a sweep reads 2m/1024 pages. Per-update reads cost about 2bμ pages,
  where μ is the LRU miss rate per list read. They tie at b/m ≈ 1/(1024μ).

Worked number: Twitter-size m = 1.4 G gives a sweep of 11.2 GB, 1.7 s at 6.5 GB/s. The tie is near 1.4 M/μ updates
with asynchronous reads, and near 8.6 k/μ against synchronous reads at 10k IOPS.

**Why it escapes Gate 0.** Gate 0 failed because scanning was cheap. Here the cost unit is a page, about 100 µs
synchronously, so a sweep that replaces 2bμ reads saves real time. The batch overhead is a sort and is negligible
next to the sweep itself. The escape is partial: at low b/m the LRU lemma says nothing is gained.

**Strongest simple baseline.** Sequential lower-degree scan, the project's kernel A, over a page LRU with the same M,
with synchronous reads and with asynchronous lookahead reported separately.

**Cheapest kill test, under an hour.**

1. Spend 30 minutes of proof attempt on the batch-hub case. If it needs a sort(m) per hub, the log returns and
   claim (c) falls.
2. Rerun forward citations of Hu et al. and Pagh & Silvestri once OpenAlex resets. If any paper states the bound,
   stop.
3. New file, for example `benchmarks/s2_pagesim.cpp`, including `src/r11/tc.hpp` without editing it. `Engine::common`
   is private, so re-implement the same scan with `Graph::deg`, `nbr` and `has`. Pack sorted lists at 1,024 ids per
   page in vertex order, rebuilt at batch start, with fence keys in memory. Probes touch one page each. Replay all
   six streams, expiry and insert-only, at 1k and 10k event cuts, with M at 5%, 10% and 33% of the graph.
   33% is about the low end of the memory ratio in LSMGraph, 16 GB for graphs of 18 to 50 GB. Report S (sequential LRU
   misses), D (distinct pages per batch), W (sweep pages), the mean 2b/|V(Δ)|, and μ.

**Kill rule for the empirical part.** Stop if min(D, W)/S > 0.67 in every cell with b·1024/m < 1. Cells at or above 1
count separately, since there a sweep wins merely because the graph is small: with m = 99,908 edges and b = 1,000,
the sweep is about 195 pages.

Before running: write a protocol first, as Gate 0 did. Keep each run under ten minutes, because `CLAUDE.md` treats
longer runs as heavy until the machine is registered. Label all output as a model test.

**My prediction.** μ is low on these bursty streams, and the empirical part is thin. The paper then stands on the
theorem.

**Likely venue.** SIAM ACDA or ALENEX as conferences. JGAA as a journal: it is in the project's list as Q2 with APC 0,
and the audit already noted the fit. A short note if the proof turns out to be short.

**Scores.** Novelty **5/10**: no paper found, but the shape is one substitution away from Beame et al. and MGT.
Feasibility solo **8/10**: a proof plus a simulator, with no hardware dependence. Note `CLAUDE.md` names the type as
method with one small theory result, so a theory-led paper is a PI decision.

### C2. Compression-aware batch kernels

**Claim.** On block-compressed adjacency, where a membership probe costs a block decode, batch memoisation of decoded
blocks cuts decoded integers by at least 3× against sequential scanning with a small decoded-block cache.

**Type.** Method, empirical. This is the idea file's own extension.

**Closest papers.**

| Paper | Read | Overlap |
|---|---|---|
| [Aspen, Dhulipala, Blelloch, Shun, arXiv:1904.08380](https://arxiv.org/abs/1904.08380) | §1 to §3, chunk parameter | High on the container. Batch updates, difference coding. No triangle work in the text |
| [PaC-trees / CPAM, arXiv:2204.06077](https://arxiv.org/abs/2204.06077) | Abstract, §1 | High on the container. No triangle work in the text |
| [CPMA, Wheatman et al., arXiv:2305.05055](https://arxiv.org/abs/2305.05055) | Abstract | Medium. Compressed batch-parallel set, 3× faster inserts and 4× faster range queries than compressed PaC-trees |
| [BYO, Wheatman et al., arXiv:2405.11671](https://arxiv.org/abs/2405.11671) | §1, §4 passage on TC, §5 list | **High on the finding.** Static TC on dynamic containers ran in similar time, because intersections dominate. The audit found no batch-dynamic TC in its tree |
| [GBBS, Dhulipala et al., arXiv:1805.05208](https://arxiv.org/abs/1805.05208) | §1, compression passages | Medium. Static TC on compressed graphs at under 1.5 bytes per edge |

**Precise gap.** Not found: batch-dynamic exact triangle counting on a compressed container, or a cost account that
charges decoding. One search on compressed intersection returned none. Weak.

**Mechanism sketch.** `common` scans the lower-degree list, then probes the other. A hash probe is unavailable on a
compressed list, so each candidate needs a block location plus decode. Within a batch, sort probes per target list,
decode each touched block once, and reuse it across the star. The kernel choice flips by block count, not by list
length.

**Why it escapes Gate 0.** A probe moves from one operation to about 64 to 128 decoded integers, so a 6 to 64
operation update becomes hundreds to thousands of decoded integers, and batch overhead is now a small share.
Weakness: in absolute terms one block decode is cheaper than one DRAM miss (§2), and whole-algorithm slowdown from
compression is reported near 25%.

**Strongest simple baseline.** Sequential scanning plus an LRU of decoded blocks of about 1 MiB, or a plain merge of
two decoded lists, taking the cheaper per edge. A fixed uncompressed sidecar for the top hubs is the next rival.

**Cheapest kill test, under an hour.** Copy the Gate 0 bench and charge one operation per decoded integer, with blocks
of 128 and 256 in sorted lists. Compare kernel A with no cache, kernel A with the decoded LRU, and batch distinct
blocks with kernel selection. **Kill if the batch total over the LRU total exceeds 0.67** on any non-engineering
stream, the Gate 0 threshold. Same streams and cuts. A hindsight cap as in `benchmarks/diag_oracle.cpp` should be added.

**Likely venue.** JGAA, or ALENEX or SEA. JEA did not appear in the journal list when I searched it.

**Scores.** Novelty **5/10**. Feasibility **9/10**, and the kill test is the cheapest here. I expect the decoded LRU to
absorb most of the gain, and a referee to ask why anyone compresses a graph of under 26 MB.

### C3. Triangle maintenance coupled to LSM flush and compaction

**Claim.** In an LSM graph store, charging the triangle delta to reads that compaction already pays for gives exact
counts at flush or compaction boundaries at a fraction of the neighbour reads of per-update scanning.

**Type.** Method, systems.

**Closest papers.**

| Paper | Read | Overlap |
|---|---|---|
| [Aster, Mo et al., arXiv:2501.06570](https://arxiv.org/abs/2501.06570) | §2, §3.3, Table 4 | High on cost model. Adaptive edge update by cost. No triangle maintenance |
| [LSMGraph, arXiv:2411.06392](https://arxiv.org/abs/2411.06392) | §1, §2, §5.1, §5.3 | High on storage. Multi-level CSR, 64 MB MemGraph, 16 GB memory cap. Analytics are SSSP, BFS, CC and similar, and no triangle results were found by search of the text |
| [BACH, Huang et al., PVLDB 18(5) 2025](https://www.vldb.org/pvldb/vol18/p1509-miao.pdf) | Abstract, §8.1 | Medium. Static triangle counting is among its algorithms on an LSM store, 16 GB cap, SATA SSD, 400 MB/s. Not incremental maintenance, as far as read |
| [Shared Arrangements, McSherry et al., arXiv:1812.02639](https://arxiv.org/abs/1812.02639) | §3 passages, conclusion | **High on idea.** Differential dataflow keeps batches in immutable LSM layers, as in-memory state. The conclusion leaves persistence undiscussed and calls the layers suitable for it |
| [Feldera storage design](https://github.com/feldera/storage-design) | Whole README, via a summary tool | **High on idea.** A spine of merged batches, moving to disk with io_uring and a buffer pool. Design note, not a paper. Nothing on triangles |

**Precise gap.** Not found: measured I/O for exact triangle delta queries on an LSM store, or a count tied to
compaction. The generic pattern of batched delta joins over LSM state exists, so only the triangle-specific I/O
account is new.

**Mechanism sketch.** Treat the memtable as Δ. Run the delta at the last-level compaction, reusing its sequential read
as the C1 sweep. Serve an exact count as of that version, or add a small top-up for edges in higher levels.

**Why it escapes Gate 0.** A neighbour read is 2 + (d+1)·I/B block I/Os or O(L), and the read is already paid for.

**Strongest simple baseline.** Sequential scanning over the store's own block cache, plus a periodic count.

**Cheapest kill test, under an hour.** Trace simulation only, with levels at ratio T = 10 and L = 4, and the memtable
scaled to 0.1% of m, since the streams would never leave a real 64 MB one. Measure the share of lists needed by each
flush's Δ that are read by compaction in the same interval. **Kill if it is under 20%.** I expect a kill, because
upper-level compaction touches little and last-level compaction is rare, so fresh counts need extra reads anyway.

**Likely venue.** PVLDB or SIGMOD if it held, which is a high bar solo. Big Data Mining and Analytics is APC 0 in the
list, quartile unconfirmed there.

**Scores.** Novelty **6/10**: the triangle-specific account is new as far as I found, but the pattern is not.
Feasibility **3/10**: it needs an LSM engine and graphs that exceed memory.

### C4. Asynchronous lookahead over a batch

**Claim.** Knowing the batch lets scanning submit its page reads at high queue depth, raising updates per second by
about 100× over synchronous reads with no change in I/O count.

**Type.** Empirical, engineering.

**Closest papers.**

| Paper | Read | Overlap |
|---|---|---|
| [Haas & Leis, PVLDB 16(9) 2023](https://www.vldb.org/pvldb/vol16/p2090-haas.pdf) | §1, §2 | **Highest.** The 150× gap between 10k and 1.56 M IOPS is their point |
| [ACGraph, Chen et al., arXiv:2511.07886](https://arxiv.org/abs/2511.07886) | Abstract, §1 | High. Asynchronous, io_uring, block-centric out-of-core system. BFS, WCC, PageRank, k-core; static |
| [Mnemonic, Bhattarai & Huang, arXiv:2206.09983](https://arxiv.org/abs/2206.09983) | §1, §3 storage passage, Table II | **High.** Batches, transactional edge logs on disk, evaluates triangle queries. No I/O metric found in the text |
| [GastCoCo, Li et al., arXiv:2312.14396](https://arxiv.org/abs/2312.14396) | Abstract, §1 | Medium. Same idea one level up: coroutine prefetch for cache misses, 1.4× to 41.1× on computation |
| [GraphZeppelin, Tench et al., arXiv:2203.14927](https://arxiv.org/abs/2203.14927) | Abstract, §4 | Medium. A gutter tree groups stream updates by vertex group for SSD, for connectivity sketches |

**Precise gap.** Not found: this applied to exact triangle maintenance with a measured I/O account. This is a thin gap,
because it follows from a device fact.

**Mechanism sketch.** Run S from C1 as a trace, then submit the page set for the batch asynchronously and run the
kernel on arrival. No sharing is needed.

**Why it escapes Gate 0.** It changes latency per I/O, not operation count, so it sidesteps the unit-cost argument
altogether.

**Strongest simple baseline.** Synchronous sequential scanning with LRU and OS read-ahead.

**Cheapest kill test, under an hour.** Replay the page trace from C1 on the NVMe with buffering off, once synchronously
and once at queue depth 64. Kill if the gain is under 10×. This needs the Windows asynchronous API, which I did not
check. Wall-clock results need the pinned exclusive run of `CLAUDE.md` rule 3. Passing proves little, as the
result is a property of the drive.

**Likely venue.** A workshop at most. It works better as a section of C1.

**Scores.** Novelty **3/10**. Feasibility **8/10**.

## 5. Rejected ideas

| Idea | Killed by | Reason |
|---|---|---|
| Star peeling as an I/O scheduler, as the contribution | LRU lemma (*my derivation*); [Kyrola et al., OSDI 2012](https://cs.ubc.ca/~andy/538w-2016/papers/osdi12-final-126.pdf) §3.5; GraphZeppelin §4 | Once the footprint fits in M, grouping saves no fetches. Below that, the cap is 2b/|V(Δ)|. Buffering updates by vertex group for disk is old, as with GraphChi's edge buffers from "I/O efficient buffer trees" |
| Sorting reads to gain sequentiality on NVMe | Haas & Leis §2.2 | Random 4 KiB bandwidth is within 8% of sequential at high queue depth. Page count matters, order much less |
| A new static external-memory bound | Pagh & Silvestri | Matching upper and lower bounds for all E and M |
| The one-sweep algorithm as new | Hu et al. §4.1 | It is one MGT iteration. Only the bound around it can be new |
| Persistent memory | [Yang et al., FAST 2020](https://www.usenix.org/system/files/fast20-yang.pdf) §3; [Tom's Hardware](https://www.tomshardware.com/news/intel-kills-optane-memory-business-for-good), headline only | 2× to 3× DRAM leaves too little to share, against a 13% hindsight cap. The product line is closed, and the Core Ultra 7 265 machine has no such memory. [DGAP](https://arxiv.org/abs/2403.02665) (abstract) shows the area, with no triangle maintenance in the abstract |
| NUMA placement | No paper. Machine fact | A single consumer CPU, so there is no NUMA effect to measure here |
| Picking a better container | BYO §4 | Static TC ran in similar time on the containers tested |
| Cache-line sharing between updates | GastCoCo, and scale | Prefetching gets the gain without grouping, and an 800 KB active graph has few misses to save |
| Bloom filter edge pre-check, then verify | `CLAUDE.md` rule 1, no paper | It saves at most one list read per update, a factor below 2, and the rule bars it from the count path unless the PI rules otherwise |
| External sketches | [Bender et al., arXiv:2504.17563](https://arxiv.org/abs/2504.17563), abstract | Sketch-based, for connectivity-class problems. Not exact counting, and triangles are outside its list |
| Claim of "first disk-aware batch subgraph maintenance" | Mnemonic; Shared Arrangements | Mnemonic batches over edges on disk. Arrangements batch over LSM layers in memory. Only a triangle-specific I/O result could be new |
| Index-or-scan choice for a delta as new | [Labio et al., VLDB 2000](https://www.vldb.org/conf/2000/P461.pdf), §4 skimmed | Warehouse maintenance already studies choices against update ratio with experiments. The principle is 26 years old, so a planner alone is not a contribution |
| Other maintenance in external memory as overlap | [Jiang et al., ICDE 2024](https://ronghuali.github.io/PaperFiles/IO%20Efficient%20Max-Truss%20Computation%20in%20Large%20Static%20and%20Dynamic%20Graphs.pdf) §1, §IV; [Wen et al.](https://arxiv.org/abs/1511.00367) abstract; [Meyer, STACS 2008](https://arxiv.org/abs/0802.2847) abstract | These set the style for semi-external updates. Jiang et al. is single-edge and kmax-truss only. Wen et al. is k-core. Meyer is BFS. None gives global triangle counts over a batch, but each shows updates in this model are an accepted topic |

## 6. Next steps, in order

1. PI decision: is a theory-led C1 acceptable under the project's stated contribution type?
2. Rerun forward citations after the OpenAlex limit resets (about 23 hours from this session). If the bound appears
   there, C1 falls to a section of a comparative note.
3. Run the C2 kill test first, since it is the cheapest, then the C1 simulator. Both need a written protocol.

If both fail, the honest result for this direction is: no large practical factor over LRU below a 0.1% batch ratio,
which is itself worth one paragraph in any paper on this topic.

## 7. Reading ledger

Links in §4 and §5 give the source. "Read" levels there are the claim. Extra entries for material cited only in passing:

| Source | Read |
|---|---|
| [Haas & Leis](https://www.vldb.org/pvldb/vol16/p2090-haas.pdf) | §1 to §2.3 |
| [Lemire & Boytsov, arXiv:1209.2137](https://arxiv.org/abs/1209.2137) | Table I, §1 |
| [Ligra+, Shun, Dhulipala, Blelloch](https://people.csail.mit.edu/jshun/ligra+.pdf) | §1 |
| [Cui, Xiao, Loguinov, TKDE 31(8), 2019](https://s1.irl.cs.tamu.edu/people/yi/papers/tkde2019.pdf) | Abstract, text searched for update terms: none relevant |
| [Dementiev, thesis, Saarbrücken 2006](https://ae.iti.kit.edu/documents/theses/DementievDiss.pdf) | §4.6 opening only |
| [Jiang, Huang, Cheng, VLDBJ 2021, manuscript](https://www.comp.hkbu.edu.hk/~xinhuang/publications/pdfs/VLDBJ21-TrussIO.pdf) | Abstract, §4 to §5 passages. The baseline Jiang et al. 2024 uses |
| [Christopoulos et al., SAC 2025](https://www.ceid.upatras.gr/webpages/faculty/ktsichlas/Tempo/files/10%20-%20SAC2025_Triangle%20Counting%20in%20Large%20Historical%20Graphs.pdf) | Abstract. Distributed counting over a time interval. Not relevant |
| [Hanauer et al., arXiv:2610.29776](https://arxiv.org/abs/2610.29776) | Text searched for storage terms: no I/O or compression treatment. Audit row 1 for the rest |
| [SNAP wiki-talk-temporal](https://snap.stanford.edu/data/wiki-talk-temporal.html) | Statistics table only: 7,833,140 temporal edges, 3,309,592 static edges |
| Menegola 2010 | Not opened. Known here only as quoted in Hu et al. and as reference 19 of Jiang et al. 2024 |
