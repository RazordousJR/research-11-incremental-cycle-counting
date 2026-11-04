# Verification record for refs.bib

Built on 2026-11-05 for the JGAA draft (`sec2-4.tex`). It has 42 entries. Every field in `refs.bib` was read from the URL in the third column: Crossref for most DOIs, DataCite for the LIPIcs and arXiv DOIs (Crossref does not register them), the arXiv API, the publisher's OJS page, or the paper's own first page. No field was filled in from memory. DBLP could not be used: its API returned 500 and 429 errors and then a non-JSON page (the bot check that `audit-2026-11-01.md` reports). Crossref's search endpoint rate-limited many queries, so every query was retried until it answered or was dropped.

The "What it does, and how it relates" column comes from what was read this session (abstracts, and the full text where named) and from the project's primary readings in `docs/literature/`. A scout-grade reading is marked as one.

**Closest work** is marked ★. There are five such rows.

## Entries

### Keys already in the draft

| Key | Short citation | Verified against | What it does, and how it relates |
|---|---|---|---|
| `hanauer-dyncount` ★ | Hanauer, Heck, Henzinger, Sidl, arXiv:2610.29776, 2026 | https://export.arxiv.org/api/query?id_list=2610.29776 and https://api.datacite.org/dois/10.48550/arXiv.2610.29776 | It engineers single-update exact counters for triangles and all 4-vertex patterns on 56 instances, and recommends plain scanning for triangles and heavy-light tables for 4-cycles. It only handles single updates and goes up to 4 vertices, and our streams, cleaning rule and 2,000,000-event prefix come from it. We add batches, sharing inside a batch, and 5-cycles. |
| `cheetah` ★ | Zhang et al., ACM TACO 22(2):1-26, 2025 | https://api.crossref.org/works/10.1145/3736173 (sections 1 to 9 read in `r10-falsifier-reading.md`) | It groups each batch of updates into stars around a recurring vertex, caches sorted neighbour lists per group, and enumerates matches with symmetry checks on a 16-node cluster. It reports a mean 1.12 times gain for triangles against 2.5 to 2.75 times for larger patterns, with times that leave out graph update and preprocessing. It publishes the star grouping and the trend. We add a count-only closed-form kernel that runs sequentially, includes every cost, is compared with tuned scanning, wedge tables and heavy-light tables, and covers plain 4-cycles and 5-cycles, which Cheetah does not report. |
| `hhh` | Hanauer, Henzinger, Hua, SAND 2022, LIPIcs 221, 18:1-18:17 | https://api.datacite.org/dois/10.4230/LIPIcs.SAND.2022.18 and https://arxiv.org/abs/2106.15524 (abstract and section 2 read) | It maintains counts of every connected 4-vertex subgraph under single updates. Tables hold paths through low-degree vertices, high-degree vertices are counted from scratch, and it proves OMv lower bounds. Our M3, M4 and M5 arms follow its heavy-light rule with hubs scanned. |
| `bfcb` ★ | K. Wang, Lin, Qin, W. Zhang, Y. Zhang, VLDB J. 32(2):257-281, 2023 (online 2022) | https://api.crossref.org/works/10.1007/s00778-022-00746-0 (section 7 read in `scouts/s10-pivot-row-novelty.md`) | BFCB-IG counts butterflies under a batch by building one hash row of 2-path counts per start vertex over the affected subgraph and taking new minus old. That is batch row sharing for even cycles in bipartite graphs. Our B4 arm ports it to general graphs. We add odd cycles, which a row that pairs with itself cannot do, and the star row adjusted spoke by spoke. |
| `shi-huang-shun` | Shi, Huang, Shun, ACM JEA 27:1-23, 2022 | https://api.crossref.org/works/10.1145/3556541 | Static parallel 5-cycle counting in O(m α²) work. Its Algorithm 1 is the static counter inside our A5 recount. |
| `chiba-nishizeki` | Chiba, Nishizeki, SIAM J. Comput. 14(1):210-223, 1985 | https://api.crossref.org/works/10.1137/0214017 and its abstract via https://api.openalex.org/works/doi:10.1137/0214017 | It lists triangles and quadrangles in O(a(G) m) time by an edge-searching strategy. This is the wedge pairing that our A4 recount uses for 4-cycles. |

### Exact dynamic and batch-dynamic counting

| Key | Short citation | Verified against | What it does, and how it relates |
|---|---|---|---|
| `dhulipala-batch-kclique` ★ | Dhulipala, Liu, Shun, Yu, APOCS 2021, 129-143 | https://api.crossref.org/works/10.1137/1.9781611976489.10 and https://arxiv.org/pdf/2003.13585 (sections 1, 4.4 read) | It gives parallel batch-dynamic triangle and k-clique counting with heavy-light tables and removal of nullifying updates. Its proof makes each triangle count once by dividing by the triangle's number of updated edges. It works on cliques with tables and parallel work. We work sequentially on cycles with star rows, and compare with tables on real timestamped streams with every cost counted. |
| `makkar-dynamic-tc` | Makkar, Bader, Green, HiPC 2017, 2-12 | https://api.crossref.org/works?query.bibliographic=Makkar+Bader+Green... (DOI 10.1109/HiPC.2017.00011) and https://davidbader.net/publication/2017-mbg/2017-mbg.pdf | It counts triangles exactly on the GPU under insertion and deletion batches, using an update graph, sorted adjacency and inclusion-exclusion over per-edge intersections. It is a batch triangle method with no sharing between the edges of a batch. |
| `ediger-streaming` | Ediger, Jiang, Riedy, Bader, IPDPSW 2010, 1-8 | https://api.crossref.org/works/10.1109/IPDPSW.2010.5470687 | It maintains clustering coefficients under streamed batches and groups each batch by source vertex for the data-structure update. It is the early precedent for grouping a batch by vertex, and the grouping there serves the graph update, not the count. |
| `dptl` ★ | Yu, Qin, Y. Zhang, W. Zhang, Lin, ICDE 2021, 1332-1343 | https://api.crossref.org/works/10.1109/ICDE51399.2021.00119 and its abstract via OpenAlex (cover letter and sections I to IV read in `scouts/s6`, `s10`) | DPTL and DPTL+ list the triangles a batch adds or removes. They use an orientation that reports each one without duplicates, and DPTL+ builds one bitmap of N(u) per pivot u and reuses it for every batch edge at u. Row sharing within a batch is therefore published for triangles, as listing and in parallel. We add counting with no listing, stars peeled by batch degree with a degree-only guard, and patterns up to 5-cycles. |
| `liu-batch-kcore` | Liu, Shi, Yu, Dhulipala, Shun, SPAA 2022, 191-204 | https://api.crossref.org/works/10.1145/3490148.3538569 and https://arxiv.org/pdf/2106.03824v4 (Theorem 3.7 read) | It maintains a low out-degree orientation under batches and, through it, k-clique counts in O(\|B\| α^(k−2) log² n) amortized work. It is the arboricity batch bound that rules out claiming a star bound as new. |
| `eppstein-spiro` | Eppstein, Spiro, JGAA 16(2):543-567, 2012 | https://api.crossref.org/works/10.7155/jgaa.00273 | It maintains counts of all 3-vertex subgraphs in O(h) amortized time per update using the graph's h-index. It is the wedge-count index behind later dynamic counters, and a JGAA paper on our problem. |
| `eppstein-extended` | Eppstein, Goodrich, Strash, Trott, TCS 447:44-52, 2012 | https://api.crossref.org/works/10.1016/j.tcs.2011.11.034 and its abstract via https://export.arxiv.org/api/query?id_list=1009.0783 | It extends h-index structures to 4-vertex subgraphs in O(h²) amortized time per update and to directed triads. This is the EGST line that Hanauer et al. engineer. |
| `kara-tods` | Kara, Ngo, Nikolic, Olteanu, Zhang, ACM TODS 45(3):1-46, 2020 | https://api.crossref.org/works/10.1145/3396375 | It maintains triangle counts in O(√m) amortized time per single-tuple update, which is worst-case optimal under the OMv conjecture. It settles the single-update worst case, which we do not improve. |
| `lu-tao` | Lu, Tao, ICDT 2021, LIPIcs 186, 6:1-6:23 | https://api.datacite.org/dois/10.4230/LIPIcs.ICDT.2021.6 | It gives arboricity-sensitive dynamic indexes for exact and approximate triangle counting under single updates. It is the theory bound that any claim about vertex orientation must cite. |
| `hanauer-survey` | Hanauer, Henzinger, Schulz, ACM JEA 27:1-45, 2022 | https://api.crossref.org/works/10.1145/3555806 | It is a quick reference guide to engineering and theory results for fully dynamic graph algorithms. It sets our paper in the algorithm-engineering line for dynamic graphs. |
| `incgraph` | Cannoodt et al., PLOS ONE 13(4):e0195997, 2018 | https://api.crossref.org/works/10.1371/journal.pone.0195997 (full article read by summary in `scouts/s10`) | It computes the exact change in graphlet and orbit counts up to 5 nodes for each added or removed edge by local enumeration. It is the nearest dedicated exact 5-node dynamic counter. It handles one edge at a time with no batch and no sharing, and it enumerates instead of counting in closed form. |
| `edtc` | Z. Wang et al., IEEE TPDS 37(1):247-259, 2026 | https://api.crossref.org/works/10.1109/TPDS.2025.3627974 and its abstract via OpenAlex | It counts triangles exactly on dynamic graphs on the GPU, with a hash-based counter built to save memory on high-degree vertices. It is a recent exact dynamic triangle system. Only its abstract was read, so how it handles batches is not confirmed. |
| `paradyms` | Khan, Tan, Marquez, Taufer, Bhowmick, CCGrid 2025, 1-10 | https://api.crossref.org/works/10.1109/CCGRID64434.2025.00046 and its abstract via OpenAlex | It updates motif counts in parallel under batched updates by recounting only the modified parts of the network. It is a published batch recount over the region of change, the idea behind our A4 and A5 arms. Only its abstract was read. |

### Continuous subgraph matching, dynamic mining and delta queries

| Key | Short citation | Verified against | What it does, and how it relates |
|---|---|---|---|
| `gopher` | Zhang et al., EuroSys 2026, 1722-1737 | https://api.crossref.org/works/10.1145/3767295.3769365 (sections 2.3 to 4.3 read in `r10-falsifier-reading.md`) | It merges the set operations of one updated edge's mapping instances into a DAG at compile time. Its abstract reports up to 35.99 times over Cheetah. It shares work inside one update, not across the updates of a batch, and on the same authors' setup that beats Cheetah's sharing across updates. |
| `tesseract` | Bindschaedler et al., EuroSys 2021, 458-473 | https://api.crossref.org/works/10.1145/3447786.3456253 and its abstract via OpenAlex | It is a distributed system that mines evolving graphs by splitting the update stream into per-update tasks over a multiversioned store, with no duplicates. It is Cheetah's baseline, and each update is explored separately. |
| `sun-csm-study` | X. Sun, S. Sun, Luo, He, PVLDB 15(7):1403-1416, 2022 | https://www.vldb.org/pvldb/vol15/p1403-sun.pdf (first page, reference format) | It models continuous subgraph matching as incremental view maintenance and compares six algorithms in one framework. It finds that index updates can dominate query time and that no algorithm wins everywhere. It is the benchmark study of general continuous matching, which Hanauer et al. report dedicated counters beat on triangles. |
| `rapidflow` | S. Sun, X. Sun, He, Luo, PVLDB 15(11):2415-2427, 2022 | https://www.vldb.org/pvldb/vol15/p2415-sun.pdf (first page) | It reduces continuous matching to batch matching over the region an update affects, so that matching need not start at the new edge. It also removes automorphism redundancy. It is a general-pattern system for single edge arrivals. |
| `gamma` | Qiu et al., ICDE 2024, 3204-3216 | https://api.crossref.org/works/10.1109/ICDE60146.2024.00248 and https://arxiv.org/pdf/2401.17018 (section IV read) | It matches subgraphs on the GPU under batch updates. It removes redundancy that comes from query automorphisms, and it removes duplicates by a total order on updates. Its sharing comes from query symmetry, not from updates meeting at one data vertex. |
| `mhedhbi-tods` | Mhedhbi, Kankanamge, Salihoglu, ACM TODS 46(2):1-45, 2021 | https://api.crossref.org/works/10.1145/3446980 (sections 2.2, 4 to 4.3 read in `r10-falsifier-reading.md`) | It optimises one-time and continuous subgraph queries with worst-case optimal joins and delta generic join, and shares operators across delta queries and registered queries. It shares between plans, not between the updates of a batch, and its experiments insert batches of 5. |
| `ammar-delta` | Ammar, McSherry, Salihoglu, Joglekar, PVLDB 11(6):691-704, 2018 | https://www.vldb.org/pvldb/vol11/p691-ammar.pdf (first page, for the DOI) and https://arxiv.org/pdf/1802.03760 (section 3 read) | It evaluates subgraph queries in distributed dataflows. For updates it uses n delta queries, each reading new versions of the earlier relations and old versions of the later ones, whose union is exactly the change in the output. It is the published telescoping delta rule, stated over the query's relations rather than over the sequence of updates. |

### Static cycle and subgraph counting

| Key | Short citation | Verified against | What it does, and how it relates |
|---|---|---|---|
| `alon-yuster-zwick` | Alon, Yuster, Zwick, Algorithmica 17(3):209-223, 1997 | https://api.crossref.org/works?query.bibliographic=Alon+Yuster+Zwick... (DOI 10.1007/BF02523189) and the abstract at https://link.springer.com/article/10.1007/BF02523189 | It gives methods for finding and counting simple cycles of a given length, with bounds that depend on the number of edges. It is the classic reference for counting short cycles. |
| `escape` | Pinar, Seshadhri, Vishal, WWW 2017, 1431-1440 | https://api.crossref.org/works/10.1145/3038912.3052597 | It counts all 5-vertex subgraphs exactly through degree orientation and a per-vertex row with reset. It is the static source of the row technique, and Hanauer et al. check their counts against it. |

### Streams and data sources

| Key | Short citation | Verified against | What it does, and how it relates |
|---|---|---|---|
| `snap` | Leskovec, Krevl, SNAP Datasets, 2014 | https://snap.stanford.edu/data/index.html (the citation it asks for) | It is the Stanford collection that serves our sx-*, wiki-talk-temporal, CollegeMsg, email-Eu-core-temporal and Reddit streams. |
| `konect` | Kunegis, WWW 2013 Companion, 1343-1350 | https://api.crossref.org/works/10.1145/2487788.2488173 | It describes the Koblenz network collection, the source of our six Wikipedia link streams with real deletions. |
| `network-repository` | Rossi, Ahmed, AAAI 2015, 4292-4293 | https://ojs.aaai.org/index.php/AAAI/article/view/9277 and the article PDF for the page numbers | It describes the Network Repository, the source of our five temporal files such as ia-enron-email-dynamic and soc-wiki-elec. |
| `paranjape-motifs` | Paranjape, Benson, Leskovec, WSDM 2017, 601-610 | https://api.crossref.org/works/10.1145/3018661.3018731 and the SNAP pages for sx-mathoverflow, email-Eu-core-temporal and wiki-talk-temporal | It defines and counts temporal motifs, and it is the source that SNAP names for the sx-*, email-Eu-core-temporal and wiki-talk-temporal streams. |
| `panzarasa-collegemsg` | Panzarasa, Opsahl, Carley, JASIST 60(5):911-932, 2009 | https://api.crossref.org/works/10.1002/asi.21015 and https://snap.stanford.edu/data/CollegeMsg.html | It analyses an online student community's messaging network, and SNAP names it as the source of CollegeMsg, one of our engineering streams. |
| `leskovec-wiki-governance` | Leskovec, Huttenlocher, Kleinberg, ICWSM 2010, 98-105 | https://ojs.aaai.org/index.php/ICWSM/article/view/14013 and https://snap.stanford.edu/data/wiki-talk-temporal.html | It studies the Wikipedia promotion process, and SNAP names it as the second source of wiki-talk-temporal. |
| `kumar-reddit` | Kumar, Hamilton, Leskovec, Jurafsky, WWW 2018, 933-943 | https://api.crossref.org/works?query.bibliographic=Kumar+Hamilton+Leskovec+Jurafsky... (DOI 10.1145/3178876.3186141) and https://snap.stanford.edu/data/soc-RedditHyperlinks.html | It studies conflict between subreddits, and SNAP names it as the source of the two Reddit hyperlink streams. |
| `preusse-wiki` | Preusse, Kunegis, Thimm, Staab, Gottron, ICWSM 2013, 506-515 | https://ojs.aaai.org/index.php/ICWSM/article/view/14402 (metadata and abstract) | It studies how links appear and disappear in Wikipedia's link network. It is cited by Hanauer et al. for the Wikipedia hyperlink streams with real insertions and deletions. |
| `chen-expiry` | Chen, Lachish, Helmer, Böhlen, PVLDB 15(11):3263-3276, 2022 | https://api.crossref.org/works/10.14778/3551793.3551868 | It maintains spanning trees for connectivity queries on fully dynamic graphs. Hanauer et al. cite it for the rule that a timestamped edge is removed two weeks after insertion, which is our synthetic expiry. |
| `rmat` | Chakrabarti, Zhan, Faloutsos, SDM 2004, 442-446 | https://api.crossref.org/works?query.bibliographic=Chakrabarti+Zhan+Faloutsos+R-MAT... (DOI 10.1137/1.9781611972740.43) | It is the recursive matrix graph generator. The WT table layout was tuned on synthetic R-MAT streams, so the reader needs to know where those came from. |

### JGAA conversation

| Key | Short citation | Verified against | What it does, and how it relates |
|---|---|---|---|
| `grossmann-bmatching` | Großmann, Joos, Reinstädtler, Schulz, JGAA 30(1):1-24, 2026 | https://api.crossref.org/works/10.7155/jgaa.v30i1.3166 | It engineers hypergraph b-matching algorithms on 488 hypergraphs. It is a JGAA exemplar of an algorithm-engineering paper, and `venue-fit.md` also uses its methods. |
| `borowitz-2packing` | Borowitz, Großmann, Schulz, Schweisgut, JGAA 29(1):159-186, 2025 | https://api.crossref.org/works/10.7155/jgaa.v29i1.3064 | It engineers scalable 2-packing set algorithms on arbitrary graphs. It is a JGAA engineering exemplar from the same Heidelberg group. |
| `fink-rutter` | Fink, Rutter, JGAA 29(1):91-123, 2025 | https://api.crossref.org/works/10.7155/jgaa.v29i1.2923 | It engineers the synchronized planarity algorithm and evaluates it on existing and generated instances. It is a JGAA exemplar for the engineering of a published algorithm, which is our paper's type. |
| `chanchary-windowed` | Chanchary, Maheshwari, JGAA 23(2):191-226, 2019 | https://api.crossref.org/works/10.7155/jgaa.00489 and its abstract via OpenAlex | It builds data structures for time-windowed queries on timestamped relational event graphs, including counts of 2-paths, quadrangles and cliques in a window. It is the JGAA paper nearest to counting on timestamped streams. |

With `eppstein-spiro`, the JGAA entries come to five, which passes the bootstrap §3 check of five entries from the venue and its neighbours.

## Keys left out

### `folklore-exactly-once`: no source found, left out of refs.bib

The search covered the full or relevant text of Dhulipala et al. 2021, Makkar et al. 2017, Ammar et al. 2018, GAMMA (ICDE 2024) and Mhedhbi and Salihoglu 2019, keyword scans of Hanauer et al. 2026 and Hanauer, Henzinger and Hua 2022, and one web search. None of them states the lemma in our form: that applying the net updates one at a time, with each delta counted in the graph that already holds the earlier updates, counts every changed subgraph exactly once.

- Ammar et al. (`ammar-delta`, section 3.1.3) come closest. They state that the union of the n delta queries, each reading new versions of the earlier relations and old versions of the later ones, is exactly the change in the output, and that this "logically sequences" simultaneous updates. Their order runs over query relations, not over the updates in a batch.
- GAMMA (section IV) assigns a total order to the updates and lets a match form only from lower-order to higher-order update edges, to remove duplicates. That is an ownership rule, not sequential application. It credits Guo et al., SIGMOD 2020, which was not read.
- Dhulipala et al. (Theorem 4.2) reach exactly-once by dividing by the triangle's number of updated edges, which is a different argument.
- dynG's documentation states smallest-change-id ownership, but it is not a publication.

**Recommendation.** Rewrite the sentence in `sec2-4.tex` so that it does not need the key. For example: "Applying the updates in sequence counts every cycle created or destroyed by the batch exactly once, since the deltas telescope. This is the delta-query rule of Ammar et al.~\cite{ammar-delta} with one update per step, and we do not claim it." The other option is to drop the citation and call it standard.

### Candidates rejected or not verified

| Candidate | Reason |
|---|---|
| Kara et al., ICDT 2019, LIPIcs 127, 4:1-4:18 (DOI 10.4230/LIPIcs.ICDT.2019.4, checked on DataCite) | Verified, but superseded by the TODS 2020 journal version (`kara-tods`) |
| Hriday G, Sista, Das, "Efficient Batch Dynamic Graphlet Counting," arXiv:2308.14493 | It exists on arXiv only, with no venue in the arXiv record. It handles size 4 only. `paradyms` is the peer-reviewed example of a batch recount |
| Huang, Shi, Shun, SEA 2021 five-cycle paper | Superseded by the JEA 2022 version (`shi-huang-shun`) |
| Mhedhbi, Salihoglu, PVLDB 12(11), 2019 (DOI 10.14778/3342263.3342643) | Verified through arXiv's journal reference, but the TODS 2021 version holds the continuous-query part the paper needs |
| DYTC, ICACS 2024 (DOI 10.1145/3708597.3708610) | It is the earlier conference work in the EDTC line. Only the abstract has been read, and `edtc` covers it |
| Kankanamge, Waterloo thesis on delta subgraph queries | Not fetched this session. `mhedhbi-tods` and `ammar-delta` cover the delta-query rule |
| Fan, Lu, MIT PRIMES 2020 | Not peer reviewed |
| Blelloch et al., arXiv:2602.17811 | It exists on arXiv only, and it covers orientation, not counting |
| dynG `cycle_count` and TruCy | Unpublished: a repository document and a paper under review |
| Bera, Pashanasangi, Seshadhri, ITCS 2020; Burkhardt, Harris, arXiv:2303.06090 | Not needed for any sentence in the current draft, and not verified |
| LINVIEW (SIGMOD 2014) and F-IVM (arXiv:2303.08583) | Matrix and factorised view maintenance. They would be cited only if the paper discusses the low-rank reading of a star, which the draft does not |
| DCSM, ICS 2026 (DOI 10.1145/3797905.3800512) | Seen in Crossref results. Not read, so its relation to batch sharing is unknown |
| "Wedge-index based butterfly counting in bipartite graphs," Information Sciences, 2026 | Unresolved title from `scouts/s10`. It could not be found |
| Cost-model sources in `scouts/s2` (Haas and Leis, PVLDB 2023; Lemire and Boytsov; Dementiev's thesis; Pagh and Silvestri) | They cover SSD I/O, integer decoding and external-memory triangle bounds, not the in-memory gap between random probes and sequential scans that our cost model measures. The project records hold no suitable source for that gap. If the paper wants one, it has to be found and verified first |
| Leskovec et al., Kronecker graphs; Hanauer, Henzinger, Schulz, ALENEX 2020 (DyReach) | Their streams are the 38 Kronecker instances that the paper leaves out as fully synthetic, so citing Hanauer et al. 2026 is enough |
| ALENEX 2027 version of `hanauer-dyncount` | No archival version found. The arXiv record says "Short version to appear at ALENEX 2027", so the entry is the arXiv version, with that note and the arXiv DataCite DOI. Check again before submission |

## Notes for the draft

1. **DPTL+ should be cited next to Cheetah.** Section 3 says that Cheetah publishes star grouping with reuse. DPTL+ (2021) already reuses one row per pivot across a batch, for triangles, and BFCB-IG (2022) does the same for butterflies. Both are older than Cheetah, and only BFCB-IG is cited, as the B4 port. The sentence on the mechanism should cite `dptl`, `bfcb` and `cheetah` together.
2. **The two-week expiry rule comes from Chen et al.** Hanauer et al. remove each timestamped edge two weeks after insertion and cite Chen et al., PVLDB 2022, for it. The Three Labels paragraph can cite `chen-expiry` beside `hanauer-dyncount`.
3. **The dates differ from the project's records.** BFCB-IG is in the VLDB Journal 32(2), 2023, published online in 2022. EDTC is in TPDS 37(1), 2026, not 2025. Ammar et al. have two DOIs in Crossref, and the entry uses the one printed on the paper's first page (10.14778/3184470.3184473).
4. **The `hhh` key is now resolved.** It is Hanauer, Henzinger and Hua, "Fully Dynamic Four-Vertex Subgraph Counting," SAND 2022, LIPIcs 221, 18:1-18:17, DOI 10.4230/LIPIcs.SAND.2022.18.
