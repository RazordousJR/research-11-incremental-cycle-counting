# M1C02 step 1: reading, done on 2026-11-01 before any code

Protocol: `docs/experiments/screen-b-protocol.md`, M1C02. The question was narrow: does a reachable source already report exact
dynamic 5-cycle counting with these kernels? If yes, the test stops here.

## Outcome

**No reading Stop.** No source I could reach reports exact dynamic 5-cycle counting with these kernels. The test goes on.

**The gap stays open as a risk.** The two papers that matter most, Gopher and Cheetah, were not reached in full text. The public
Gopher code was reached and read, which is new since scout S1, and it narrows the risk but does not close it.

## What was reached and what was not

| Source | Route tried | Result |
|---|---|---|
| Gopher, EuroSys 2026, DOI 10.1145/3767295.3769365 | ACM landing page and ACM PDF URL | 403 on both. Stopped there. No full text |
| | Michigan Tech record `digitalcommons.mtu.edu/michigantech-p2/2645` | Reached. Abstract, authors, CC BY 4.0 notice, and a link to a publisher PDF |
| | That Michigan Tech PDF link | 403. S1 met a Cloudflare challenge on this host, so I stopped and did not try to pass it |
| | OpenAlex and Semantic Scholar | Metadata reached. Both list only the DOI and the Michigan Tech record as locations |
| | CORE API | 403 |
| | sysartifacts EuroSys 2026 results page | Reached. No Gopher entry |
| | Zenodo API | Reached. 0 records for the phrase "dynamic graph pattern mining" |
| | Jingling Xue's UNSW page and `papers.html` | Reached. No Gopher or Cheetah entry on the pages read. The by-year pages were not opened |
| | **GitHub `CGCL-codes/Gopher`** | **Reached and read.** See below. S1's two keyword searches missed it, and so did mine. Listing the HUST lab organisation found it |
| Cheetah, TACO 2025, DOI 10.1145/3736173 | ACM landing page | 403. Stopped there |
| | Michigan Tech record `.../michigantech-p2/1862` | Reached. Abstract and authors. No PDF offered, only a link to the DOI |
| | OpenAlex and Semantic Scholar | Metadata reached. Marked open at the DOI, no separate PDF URL |
| | GitHub | No Cheetah repository in the 281 repositories of `CGCL-codes`, and none in three searches |
| Hanauer, Heck, Henzinger, Sidl, arXiv:2610.29776 | arXiv abstract page | Reached. v1 only, 21 October 2026. Triangles and connected 4-vertex patterns. No mention of 5 vertices on that page. S1 read the main text and appendices |
| Follow-ups of it | Semantic Scholar forward citations | Reached. Empty. A paper ten days old can have no indexed citations, so this proves little |
| | arXiv author listing for Hanauer | Reached. 2610.29776 is the newest subgraph counting entry. No 5-vertex entry among the 15 shown of 20 |
| | arXiv listing for Sidl, and Heck with Henzinger | 429 twice. Not reached |
| | Web search | One search. No follow-up found |
| Other searches | arXiv query for 5-cycle in abstract or title, newest 30 | Reached. Nothing on exact dynamic counting |
| | DBLP search API | Anubis access page. Stopped, no attempt to pass it |
| | Web search, exact fully dynamic 5-cycle counting | Nearest results are 4-cycle theory. Nothing on 5-cycles |
| Forward citations of Gopher and Cheetah | Semantic Scholar | Reached. Titles only: DyPamear, Octopus, SVSIG, a dynamic graph processing survey, and Gopher citing Cheetah. All 2026. Apart from Gopher's abstract none was read, and DyPamear could not be found by search |

No login, proxy, mirror or changed client was used anywhere, and nothing was bought. Sci-hub style mirrors and web archives of the
blocked pages were not tried: going to a copy because the host refused me would be working around the refusal.

## Gopher: what the code shows

I read commit `6ab6eb27` (16 May 2025) of `CGCL-codes/Gopher`, in the files `README.md`, `src/baseline_test.cpp`, `src/mining.cpp`,
`include/mining.h`, `src/pattern.cpp`, `include/pattern.h`, `src/mining_5Clique.cpp`, and by search `src/graph_analysis.cpp` and four
more. The repository has no licence file. I read it, and copied and ran nothing.

- **A plain 5-cycle is defined but nothing uses it.** `PatternType::Pentagon` in `src/pattern.cpp` has the edges 0-1, 0-2, 1-3, 2-4,
  3-4, which is a 5-cycle. A search of the sources reached finds no `PatternType::` use outside `pattern.cpp`. The enum looks
  inherited from an earlier static system, but that is my guess.
- **The driver does not reach the pattern at all.** `baseline_test.cpp` takes a pattern from the command line and builds schedules,
  then hands `Mining` an empty DAG list. `Mining::mine_patterns` in `mining.cpp` is hand-written nested set intersection with no
  reference to the pattern or the DAG.
- **What `mine_patterns` finds.** Its README example pattern `0111010011100011100001100` has edges 0-1, 0-2, 0-3, 1-3, 1-4, 2-4. That
  is the `House` pattern, a 5-cycle plus one chord, and not a plain 5-cycle. All four branches of `mine_patterns` require that chord,
  so a plain 5-cycle is not what it finds.
- **Insert only, and enumeration.** `Mining::run` reads a file of new edges and calls `add_edge`. I found no deletion path. Each
  match calls `process`, which adds 1. There is no net-through-edge formula, no wedge table, and no shared row between updates.
- **Separate 4-clique and 5-clique miners** exist in `src/four_clique.cpp` and `src/five_clique.cpp`.

So the public code is not exact dynamic 5-cycle counting. It is also not the full system of the paper. It is thin: the DAG merging
that the abstract describes is not wired into the miner that runs. **That is why this does not settle what the paper reports.** The
paper could evaluate a pentagon pattern on code that is not in this repository.

## Gopher and Cheetah: what the abstracts say

- **Gopher** (Zhang, Huang, Liu, Liu, Chen, Yuan, Yue, Liao, Jin, Xue). Set operations are written as DAGs, and common subexpressions
  are merged at compile time. It reports speedups of up to 75.58, 35.99 and 11.76 times over Tesseract, Cheetah and PSMiner on a
  multi-core CPU. The pattern list is not in the abstract.
- **Cheetah** (Zhang, Yi, Huang, Yuan, Gui, Chen, Zheng, Yue, Liao, Jin, Xue). Updates are grouped by exploration domains built from
  community structure, with neighbour-centric expansion. It reports 2.63 times over leading systems on average on five datasets. The
  pattern list is not in the abstract.

## Tesseract, read because Gopher measures itself against it

Bindschaedler et al., EuroSys 2021, sections 1 to 8 read. Update-based exploration enumerates the change in the match set for each
update, and each update in a snapshot is still explored separately. Evaluation: k-clique, graph keyword search, motif counting at
size 4 and frequent subgraphs at size 2K. Its 5-GKS-3 is a keyword search and not a 5-cycle query. A general system of this kind
could express a 5-cycle by filter and match, but it would enumerate each one. Nothing in the text reports 5-cycle figures.

## Shi, Huang, Shun, read for RC

ACM JEA 27(4), article 4.1, 2022, DOI 10.1145/3556541, CC BY 4.0, pages 1 to 14 read. Static only. Algorithm 1 is their parallel form
of Kowalik's counter, and it takes O(m alpha^2) work and O(m alpha) space (Theorem 4.2). Vertices are relabelled in non-increasing
degree, and `G->` is an arboricity orientation. For each v, table `U_v` holds wedges `v-y-x` with y and x above v. For each wedge
`v-u-w`, table `T_{v,u}` removes `y = u`, and an adjacency test removes `y = w`. The third step walks `N->_v(w)`, which is the
out-neighbours of w above v. **That subscript matters:** without it x can equal v and the count goes wrong. RC is built from this
listing, run sequentially, and mutant 12 in `tests/test_fivecycle.cpp` checks that trap.

## The risk carried forward

| Risk | Status |
|---|---|
| The Gopher paper reports pentagon or 5-cycle figures | **Open.** The public code defines the pattern and does not run it. If the paper has it, it would be general enumeration, most likely on insertions, and that is still a real overlap for novelty |
| The Cheetah paper reports 5-cycle figures | **Open.** Abstract only |
| A follow-up from the Hanauer group exists | Not found, weak evidence. The paper is ten days old |
| A count-only dedicated dynamic 5-cycle kernel exists elsewhere | Not found. Keyword search is known to be weak here: it missed both Gopher's repository and, in S1, the papers themselves |

A reader with library access still has to read both Gopher and Cheetah before anyone relies on the gap. Gopher is CC BY, so the file
is free to read once a browser, not an automated client, reaches it.
