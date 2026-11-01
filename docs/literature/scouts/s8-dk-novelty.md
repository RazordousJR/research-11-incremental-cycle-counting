# Scout S8: hostile novelty check of D-k

Date: 2026-11-01. Goal: prove D-k is already published. Finding prior art counts as a success. No code was run. The
sources are stated with how far each was read. Every passage below is paraphrased, apart from one short quote.

Known ground from S4, S6 and the audit (interval indexes, delta-temporal work, Ghanem TKDE 2007, Xia 2025, DPTL+) is
not repeated unless the detail changes the reading.

## Verdict: partly taken

**D-k without refresh is a direct composition of published parts. A database reviewer can fairly call it an application
of them. No paper was found that does exact triangle counting this way, but the idea itself is not new.** What remains
is narrow: keeping one shared counter correct across k windows when a refresh breaks FIFO expiry.

| Component of D-k | Status | Earliest source read |
|---|---|---|
| A result expires with its oldest input | **Taken** | Golab, Özsu, SIGMOD 2005, Section 2.2. Also Schmidt et al. ICDE 2006, Barga et al. CIDR 2007 |
| Keep the count on the first input to expire, subtract at expiry, no re-probe | **Taken, for sequence patterns** | Qi et al., A-Seq, SIGMOD 2014, Section 3.2 |
| Expiry queue as a ring of time buckets | **Taken** | Golab, Özsu 2005, Section 5.3.2 |
| k windows over one shared join, with COUNT above it | **Taken** | Hammad et al., VLDB 2003, Figure 1 |
| Shared aggregates across window lengths | **Taken for plain streams** | Arasu, Widom, VLDB 2004. Cutty, CIKM 2016 |
| Last-event pair semantics | **Taken** | Jha et al. 2013. Gou, Zou 2021. Ghanem et al. 2006 |
| One carrier per triangle that moves on a refresh | **Partly taken, sampled and keyed to the newest edge** | Jha et al. 2013, Algorithm 1, Lemma 3.1 |
| Expiry and parent updated in place on a refresh | **Taken for paths** | Pacaci et al. 2021, S-PATH |
| Exact triangle count with a per-pair death counter | **Not found** | See limits |
| One counter per pair serving all k windows | **Not found, but a one-line consequence of the min rule when there is no refresh** | Nearest: GRETA keeps k counts per event |
| Re-homing a count between pairs, under k windows, with lazy handling in the smallest window | **Not found** | This is the only candidate claim |

## Question 1: is a per-edge "dies with this edge" counter published?

**Not for triangles, in anything I could read. Yes as a principle for aggregates over composite results.**

**A-Seq is the main find.** Qi, Cao, Ray, Rundensteiner, SIGMOD 2014, Section 3.2. It aggregates sequence patterns over a
sliding window without building matches. It first considers a count per event, so that "when ei expires, this count is
deducted from the aggregation result". It then rejects that, because a match of length l would be subtracted l times.
That is the same double-count as `tests/test_exact.cpp`. Its fix is Start Event Marking: only the START event's expiry
changes the answer, so the count lives on that event, and its Lemma 3 proves this. On expiry the counter is subtracted.
This is the D-k principle, stated and proved 12 years ago, in a different setting.

Sharon (Poppe et al., arXiv 2010.02989, Section 3.2) restates it: START expires first, so aggregates are kept per START
event. GRETA (Poppe et al., PVLDB 2017, arXiv 2010.02988, Section on sliding windows) shares events across k windows but
keeps a count per window on each event, so k counters per event.

**What differs from D-k.**
- In A-Seq the first-to-expire event is fixed by pattern order. In a triangle it depends on the data, so it must be
  found at birth, which is one comparison in the closing scan.
- A-Seq updates all active START counters on each arrival. D-k pays only per triangle.
- A-Seq's Lemma 3 rests on FIFO: earlier arrivals expire before later ones. A refresh breaks that. This is the exact
  point where the published argument stops and D-k has work to do.

**Other sources checked for Question 1.**

| Source | Read | Finding |
|---|---|---|
| Golab, Özsu, [Update-Pattern-Aware Modeling](https://cs.uwaterloo.ca/~tozsu/publications/stream/sigmod05golab.pdf), SIGMOD 2005 | Sections 2.2, 3, 5.3.2, text search of the rest | Join result expiry is the minimum of its inputs. State partitioned by expiry in a circular array, as a calendar queue, used as the materialized view of a join. Group-by decrements per expiring input tuple. So it stores results rather than a count per input |
| Schmidt, Jensen, Šaltenis, [Expiration Times for Data Management](https://homes.cs.aau.dk/~csj/Papers/Files/2006_schmidtICDEc.pdf), ICDE 2006 | Sections 1 to 2.6, text search of the rest | Same min rule in an algebra, with joins derived from it. COUNT is non-monotonic, so a materialized result needs recomputation, full or incremental. The paper leaves that choice open and describes no per-input counter. It assumes base data is never updated and lists that as future work |
| Wang, Li, Zhang, Guo, Processing Sliding Window Join Aggregate in Continuous Queries over Data Streams, ADBIS 2004, LNCS 3255, pp. 348-363, [record](https://rd.springer.com/chapter/10.1007/978-3-540-30204-9_24) | **Abstract only, through a search snippet. Springer redirected to a login** | **Unresolved risk.** Proposes join-aggregate operators that avoid storing join results, including multi-way joins. I could not see whether they use per-tuple counts. Later Zhong, Wang, Guo, DBTA 2009 and Wang, J. Software 17(4), 2006 share the topic. Neither was read |
| Li, Zou, Özsu, Zhao, [Timing](https://arxiv.org/pdf/1801.09240), ICDE 2019 | Definitions, text search | Exact, enumerates. Stores partial matches and deletes those containing the expired edge. No count |
| Min et al., [TCM](https://arxiv.org/pdf/2312.10486), ICDE 2024 | Definitions, Section on max-min timestamps | Finds embeddings. Its max-min timestamp is the largest minimum timestamp among weak embeddings, used as a filter. Same quantity family, different purpose. No count |
| Pacaci, Bonifati, Özsu, [SGA](https://arxiv.org/pdf/2101.12305), arXiv 2021 | Definitions, Section 6.2 | PATTERN uses pipelined hash joins, and results expire when a participant does. No count. Treats triangle-shaped queries as ordinary patterns |
| Liao, Liu, Huang, Xu, [Truss-Based Community Search over Streaming Directed Graphs](https://www.vldb.org/pvldb/vol17/p1816-xu.pdf), PVLDB 2024 | Definitions, text search | Exact, sliding window, cycle and flow support. Expiry as peeling or layer updates. No shared death counter seen in the search |
| Hu, Zhong, Li, [(k,δ)-truss](https://arxiv.org/pdf/2606.11582), arXiv 2026 | Maintenance passages by text search | A new timestamp on an edge can reduce a triangle's minimum span, and the paper assumes no edge deletion. This is a span measure, not expiry, and it is insert-only |
| Goodman, [Streaming Temporal Graphs](https://arxiv.org/pdf/2004.00215), arXiv 2020 | Text search | Temporal triangles, enumerated, with an expiry check on stored intermediate results |
| Le-Phuoc, [CQELS-TrieGS](https://arxiv.org/pdf/2608.15927), arXiv 2026 | Abstract, text search | Exact signed multiplicity for acyclic queries. Triangles go to a separate bag, and no window sharing was seen |
| Gou, Zou, [SWTC](https://www.wict.pku.edu.cn/docs/20221018150115260757.pdf), SIGMOD 2021 | Definitions, text search | Approximate. Confirms the problem is framed as sampling, with exact counting left to a reference tool |
| Hyper-Stream | **Not found** in two searches | I could not identify this work. Please give a citation if it is a specific paper |
| SymBi, TurboFlux | Search summary only | General matching with edge deletion handled as negative matches. Full text not read |
| Bursting subgraphs, persistent triangles | Search only | Bursting work seen is k-core based. "Persistent" returned only sources already covered in S4 |

No source used the phrases "weakest edge" or "oldest edge" for a triangle. That is weak evidence: the literature would
say "expiration time", "minimum timestamp" or "START event".

## Question 2: is the k-concurrent-window form published?

**For joins and for plain aggregates, yes. For a count over a three-way self-join with one shared counter, I found no
source.**

- **Hammad, Franklin, Aref, Elmagarmid, [Scheduling for Shared Window Joins](https://www.vldb.org/conf/2003/papers/S10P02.pdf), VLDB 2003**, read as page images,
  pages 1 to 4. Figure 1 is one shared join feeding a COUNT with a small window and a MAX with a large one. Largest
  Window Only runs the join at the largest window and routes each output using the gap between its minimum and maximum
  timestamps. **That is baseline B2's insertion side.** Binary joins, hash or nested loop. It notes in passing that
  multi-way joins can have windows per stream pair.
- **Wang, Rundensteiner et al., [State-Slice](https://davis.wpi.edu/~dsrg/PROJECTS/CAPE/publication/vldb06-slicejoin.pdf), VLDB 2006**, introduction, Section 3.1, definitions, conclusion, text search. Slices window state into a
  chain, so a tuple moves from one slice to the next as it ages. D-k's k tokens per pair do the same walk across window
  boundaries. Binary joins only. No aggregate maintained in the join.
- **Golab, Özsu, [Processing Sliding Window Multi-Joins](https://cs.uwaterloo.ca/~tozsu/publications/web/vldb03-178.pdf), VLDB 2003**, abstract, introduction, text search. Exact multi-way join, but
  results are treated as append-only and never retracted. No count or sharing across windows was seen.
- **Arasu, Widom, [Resource Sharing in Continuous Sliding-Window Aggregates](https://www.vldb.org/conf/2004/RS9P2.PDF), VLDB 2004**, introduction, Section 3.1 statements, text search. One structure serves many
  window lengths. B-INT uses dyadic intervals, and subtractable aggregates get a cheaper class. **This is close to W-F,
  not D-k.** It covers aggregates over one stream, not joins, and its input is append-only, whereas W moves units
  between slots.
- **Carbone et al., [Cutty](https://dcatkth.github.io/papers/cutty-cikm2016.pdf), CIKM 2016**, abstract and text search. Shares partial aggregates across windows and queries, on
  plain streams. The word "join" does not appear in the extracted text.
- **Sharon**, Section 7.2, says differing windows are an orthogonal problem and that existing segment-sharing methods can
  be plugged in. So its authors treat the k-window step as separate from their contribution, while noting that segment
  sharing may not always pay.

**Reader's composition, which a reviewer can write in one paragraph:** take the join with the min rule (Golab 2005), group
results by the oldest input as a partial aggregate (A-Seq 2014), and share across windows by summing those partials
(Arasu 2004, or Cutty). The identity is short. Without refresh, a triangle is in window i exactly when its oldest pair
is, so T[i] is the sum of d(e) over pairs in window i. **I found this stated nowhere, but it follows in one line.**

**Not reached.** Krishnamurthy, Wu, Franklin, SIGMOD 2006 (403). Panes, Li et al. 2005 (500). Scotty (403 and timeout).
MJoin and A-Greedy (unread; a guessed URL returned a different paper, so nothing here rests on it).

## Question 3: is refresh treated anywhere?

**The semantics are taken, several times over. A related mechanism exists for paths and for sampled triangles. Exact
count re-homing was not found.**

**Semantics.**
- Jha, Seshadhri, Pinar, [arXiv 1310.7665](https://arxiv.org/pdf/1310.7665), Sections 1 and 5. The window graph takes edges in the window and removes
  duplicates, so a pair is in the window if any event of it is. They keep only each edge's latest timestamp.
- Gou, Zou 2021, Definition 2.4, binary counting: distinct triangles over an edge multiset in the window.
- Ghanem, Aref, Elmagarmid, [Exploiting Predicate-Window Semantics](https://www.cerias.purdue.edu/assets/pdf/bibtex_archive/p3-ghanem.pdf), SIGMOD Record 35(1), 2006, Sections 2 to 5.
  A tuple with the same correlation attribute is an update over the earlier one, and only the latest tuple per key
  qualifies. A dynamic predicate on time is treated as a case of the model. With key = pair and predicate
  `ts > now - L`, this defines D-k's window, although the paper's own time example uses a different key. It leaves
  per-operator handling of update tuples as an open challenge, and sharing as future work.
- The same paper, Section 3.3, describes CQL `Partition By ... Rows 1`, where a new reading emits a negative tuple then a
  positive one. It notes that this produces a transiently wrong COUNT.
- Pacaci et al., Section 3.2, Definition 11: value-equivalent tuples with overlapping or adjacent validity are coalesced
  to the earliest start and latest expiry. Under a time window, I read that as the same last-event rule.
- Barga et al., [CEDR](https://www.cidrdb.org/cidr2007/papers/cidr07p42.pdf), CIDR 2007, Section 2 allows an event's validity to be changed by later tuples with the same
  ID. Section 6 defines join validity by max start and min end, then assumes no modifications and leaves their operator
  semantics as a challenge. This was read by text search.

**Mechanism.**
- **MG-Triangle (Jha et al.), Algorithm 1 and Lemma 3.1.** On a repeat of a wedge's edge, the wedge's closed flag resets to
  0. On a closing edge it is set to 1. The lemma shows exactly one wedge of a triangle carries the flag, the one opposite
  the edge with the latest timestamp. So each triangle has one carrier and a refresh moves it. **This is a re-home in a
  sampled estimator.** It is eager, keyed to the newest edge rather than the oldest, and the window is applied only when
  estimating, using a wedge formation time. S4 listed this paper from its abstract and introduction, so this detail
  is new. It raises its overlap with W more than with D-k. The authors add that small windows can simply be stored for
  exact answers, which is the exact case left unstudied.
- **S-PATH (Pacaci et al., Section 6.2.4, with Example 10).** For each node it keeps the path with the largest expiry. On a
  later arrival with larger expiry it updates the node's validity and parent pointer at once. Their direct approach
  avoids the traversals of the negative-tuple approach. **This is refresh with re-parenting**, for reachability, where
  one best path suffices. A count must move many units at once, so it does not transfer directly.

**Generic route, which a reviewer will raise.** A refresh can be handled as delete plus insert. That is plainly exact and
needs two scans per refresh. So exactness under refresh is not a contribution. Only the savings can be: no scan when
`d(e) = 0`, no scan while the pair stays in window 0, and the argument that this is safe.

**Not found.**
- Re-homing an exact count from a refreshed pair to the triple's new oldest pair.
- The split between lazy handling inside the smallest window and immediate handling outside it.
- The failure of pure lazy handling once k > 1, which never shows at k = 1. Invariants I1 to I3 in
  `results/screen-b-M1B01-B/README.md` carry this. The 79,725 wrong trials are the evidence that the point is not vacuous.

The stale-token re-file itself is a generic timer-queue pattern. A search showed cache libraries with wheel-driven lazy
expiry and rescheduling variants, but no academic source for it. Do not claim it.

## Where the speed actually comes from

This matters more than the citations. Using the 3,600 s table in `results/screen-b-M1B01-B/README.md`, and reading refresh
and stale-pop scans as part of total D-k scans (the README does not state this outright):

| Stream | D-k scans per event | Better B2 scans | Re-home scans per event | Re-home share of D-k scans |
|---|---:|---:|---:|---:|
| CollegeMsg | 0.271 | 1.429 | 0.039 | 14% |
| email-Eu-core-temporal | 0.404 | 1.313 | 0.332 | 82% |
| sx-mathoverflow | 0.649 | 3.169 | 0.087 | 13% |
| sx-askubuntu | 0.671 | 3.342 | 0.030 | 4% |
| sx-superuser | 0.697 | 3.467 | 0.031 | 4% |
| wiki-talk-temporal | 0.538 | 2.638 | 0.064 | 12% |

**So the gain comes almost entirely from removing expiry scans, which is the published principle. The refresh machinery,
the only unpublished part, is a cost paid for correctness.** On the one stream where refresh dominates,
email-Eu-core-temporal, D-k is at its weakest, 0.76. A paper must not present the 0.39 to 0.76 as the result of the new
part.

## Baseline gap this check exposes

M1B01-B has no arm for the database reader's obvious baseline. Before any novelty claim, I recommend adding:

1. **Materialized triangle view partitioned by expiry** (Golab, Özsu 2005, Section 5.3.2). Expiry needs no scan, it
   only pops. D-k's advantage over it is memory per triangle, and perhaps refresh cost. It is not the removal of the
   expiry scan. This could undercut D-k on operations.
2. **k counters per pair** (GRETA style). This measures what sharing one counter across windows is actually worth.
3. **Plain delete plus insert on refresh**, on the D-k counter. This measures what the re-home rules save.

## Question 4: what a paper can and cannot claim

**Can claim, if the citations below are made and the new arms hold up:**
- An exact counter for k concurrent sliding windows over last-event pairs, using one counter per pair and no per-triangle
  storage, with invariants and a proof of correctness under refresh.
- The re-home rule as the extension of first-to-expire counting to non-FIFO expiry, with the counterexample that pure lazy
  handling is wrong for k > 1.
- An operation-count study on labelled real streams against scan baselines, stated as an application of the principle.

**Cannot claim:**
- That the death counter, or storing the count on the oldest input, is new. Cite A-Seq and Golab 2005.
- That the min rule is new. S6 traced it to Ghanem TKDE 2007; it is at least as old as Golab 2005 in print.
- That k-window sharing for joins is new. Cite Hammad 2003 and State-Slice 2006.
- That last-event semantics are new. Cite Jha 2013 and Ghanem 2006.
- "First exact" or "first sliding-window exact triangle count". Exact window counters exist as reference tools
  (GoldenCounter, per S4), and the ADBIS 2004 question is open.
- That the speed gain comes from re-homing. The table above says otherwise.

**Honest size.** This is a small algorithm-engineering contribution with a narrow theory note. SEA or ALENEX fits, as a
graph algorithms result that cites the stream database line. A database venue is riskier, since a reviewer there will see
a known technique applied to a self-join, with one new case.

## How weak "not found" is

**Weak. Treat every "not found" here as "not found in what I could open".** The positive findings are solid, since they
were read in primary text. The negative ones are not. Most of the risk sits in database and CEP proceedings, which is
where this scout could reach least.

**Failed or unavailable.**
- **arXiv API: no usable screen.** Ten queries returned zero records, then 429. A control query also returned 429. Those
  zeros mean nothing. Individual arXiv PDFs did download.
- **OpenAlex and Semantic Scholar: 429** on the first test, so no citation screen. **No forward citation check was done on
  A-Seq, Golab 2005, Hammad 2003 or Jha 2013.** This is the largest single gap: a paper applying first-to-expire counting
  to graphs would most likely be found by citing A-Seq.
- **DBLP:** not used, under the bot-check rule.
- **Springer:** login redirect, so the ADBIS 2004 text and a window-aggregation survey (DOI 10.1007/s13222-022-00417-y) are
  unread.
- **IEEE Xplore:** empty page, so DBTA 2009 is title only.
- **typeset.io:** 403, for Krishnamurthy 2006 and Garcia-Molina 1998.
- **DFKI** 403 and **TU Delft** timeout, for Scotty. **SIGMOD Record** 500, for panes. **jos.org.cn** connection reset, for
  J. Software 2006.
- **Crossref** worked but is metadata only. Its relevance ranking was noisy, and one query's output stopped on an encoding
  error partway through its list.

**Read level.** Only A-Seq Section 3.2, Ghanem 2006 Sections 1 to 5, Schmidt Sections 1 to 2.6, and Hammad pages 1 to 4 (as
page images) were read closely. Everything else was read by selected passages and text search, so a remark buried elsewhere
could be missed. Text search on extracted PDF text is not a full read.

**What would change this verdict.**
1. The ADBIS 2004 paper or its J. Software 2006 version holding per-tuple result counts. That would push the no-refresh form
   from "composition" to "directly published".
2. A citing paper of A-Seq, or a CEP or IVM paper, handling updates to START-like inputs. That would close the last row.
3. A 2026 proceedings paper on sliding-window triangles, which none of these sources would show.

**Recommended manual checks for the PI, about an hour, library access needed:** ADBIS 2004 pp. 348-363 and J. Software
17(4) p. 740; Krishnamurthy et al. SIGMOD 2006; forward citations of A-Seq and of Jha et al. in Google Scholar.

## Corrections to earlier notes

- `results/screen-b-M1B01-B/README.md` says the principle traces to Ghanem et al. 2007 through S6. The count-on-oldest form is
  A-Seq 2014, and the min rule is in print by Golab 2005. Its warning that a database reader may call this a direct
  application is confirmed for the no-refresh case.
- S4 gives Jha et al. as closest in idea for many windows. That stands, and is stronger: it also has last-seen semantics and
  a single moving carrier per triangle.
- S4's "no exact work found" for many window lengths stands, with the same weakness.

## Search log

**Web search terms.** Exact triangle sliding window expiry counter; time-constrained continuous subgraph matching; Timing MS-tree;
sliding window k-truss, twice; oldest edge triangle; minimum timestamp, weakest edge, earliest edge triangle; shared sliding window
join; Golab Özsu multi-join; Ghanem negative tuples; SGA Pacaci; Scotty, Cutty, panes; update-pattern-aware; expiration times;
materialized view with expiring data; CQL partitioned window; predicate windows; on-the-fly sharing; Nile shared joins; temporal
butterfly and bitruss sliding window; persistent triangles; Hyper-Stream; TTL refresh on graph streams; multiple windows motif;
Flink state TTL; window join aggregate; A-Seq; GRETA, Sharon, Hamlet; CEP window sharing; AStream, AJoin; Materialize temporal
filters; timer wheel lazy expiry; StreamInsight lifetime operators; bursting triangle; SymBi, TurboFlux.

**Also seen, no change to the verdict.** AJoin (PVLDB 2020) and Hamlet (arXiv 2101.00361) from search and text search.
Materialize temporal filters (docs): expiry is a scheduled retraction, so a join would still process it. StreamInsight
docs: `ClipEventDuration` clips an event at the next event with the same key, which is a last-write-wins form in a shipped
product. Flink docs: state TTL covers regular joins, and window joins do not use it.

**Crossref queries, about 15.** Terms covered sliding window join aggregate, triangle, truss, bitruss, butterfly, sequence
prefix counters, expiration, tuple refresh, and multi-query matching.
