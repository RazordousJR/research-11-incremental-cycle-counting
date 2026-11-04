# Venue fit, paper 1 (M1A17)

Read on 2026-11-03, on the PI's word (`log.md` 0043), as reading and no machine. The contribution type is algorithm engineering (`CLAUDE.md`, `log.md` 0043). The venue is the PI's decision, and submission is too (bootstrap §3, §9).

**How this was read.** A subagent read the venues' own pages. SCImago, ACM, Clarivate and DBLP refused the plain fetcher, so it read them in the PI's Chrome, clicking links and searching the Master Journal List by title and ISSN. It did not log in, submit a form or answer a cookie banner. I checked again myself, by fetch, JGAA's scope and fees. Everything else below is the subagent's reading, marked with its URL, and not checked a second time. Indexing claims are as read on those pages on this date, never remembered.

Candidates came from the PI's free-journal list (`OFFICIAL List_Jurnal_FREE_Q2_Q3_Q4_University_Overall_2026_UPDATED.xlsx`): JGAA and J.UCS. ACM JEA was added as the natural home of experimental algorithmics, though it is not on the list. KAIS was added when two of the three failed.

## Summary

| Venue | Scope | Exemplars, 2023 to 2026 | Scopus, SJR 2025 | WoS | Cost to authors | Verdict |
|---|---|---|---|---|---|---|
| **JGAA**, Journal of Graph Algorithms and Applications | Passes | 3 found, none on counting | Indexed. Q2 in Computer Science (miscellaneous) only, Q3 in four other categories. SJR 0.352 | Not found in SCIE, SSCI, AHCI or ESCI | None | **First choice** |
| **KAIS**, Knowledge and Information Systems (Springer) | Weak: data and information systems, not graph algorithms | 3 found on dynamic or streaming graphs, abstracts only | Indexed. Q1 in Information Systems, Q2 in four others. SJR 0.869 | SCIE | None by the subscription route. Open access optional, $3,290 | **Second choice**, if WoS is required |
| J.UCS, Journal of Universal Computer Science | General CS. "Algorithm engineering" is not on its about page | None found | Indexed. Q3 best, SJR 0.308 | SCIE | None | Fails: no exemplar, wrong readers |
| ACM JEA | Was a perfect fit | 2023 only | Coverage ends 2023 | | | Fails: by its home page, closed to submissions after 2023, and authors are sent to a TALG experimental track |

The PI's list gives JGAA as Q2. That is its best category only, and it is Q3 in the other four.

## JGAA

- **Scope.** https://jgaa.info/index.php/jgaa/about, read 2026-11-03 and checked twice. The scope names "static and dynamic graph algorithms" and, under experience with graph algorithms, experimentation and implementations. It does not name counting.
- **Exemplars,** each seen on its article page by the subagent:
  1. Großmann, Joos, Reinstädtler, Schulz, *Engineering Hypergraph b-Matching Algorithms*, 30(1):1–24, 2026. DOI 10.7155/jgaa.v30i1.3166.
  2. Borowitz, Großmann, Schulz, Schweisgut, *Scalable Algorithms for 2-Packing Sets on Arbitrary Graphs*, 29(1):159–186, 2025. DOI 10.7155/jgaa.v29i1.3064.
  3. Fink, Rutter, *Constrained Planarity in Practice: Engineering the Synchronized Planarity Algorithm*, 29(1):91–123, 2025. DOI 10.7155/jgaa.v29i1.2923.
  No paper on dynamic subgraph or triangle counting in 2025 and 2026.
- **Indexing.** Scopus coverage 1997 to 2026 and the quartiles above, from https://www.scimagojr.com/journalsearch.php?q=28347&tip=sid&clean=0. WoS: the Master Journal List returned no exact match by title or by ISSN 1526-1719. JGAA's own page names DBLP, Scopus and DOAJ.
- **Cost.** It "charges no author fees", checked twice. DOAJ shows no APC, CC BY.
- **Format.** No page limit for regular papers, and the exemplars run 24 to 33 pages. LaTeX with the JGAA style. Generative AI use must be disclosed. First round about eight weeks by its page, and DOAJ lists 50 weeks to publication.
- **Fit.** The Heidelberg algorithm-engineering group publishes engineering papers here, so the readers match. Risks: the journal leans to graph drawing and theory; with the mechanism published by Cheetah, the paper must claim the measured gain, the rule for when sharing pays and the cost model, and nothing more; a paper can be returned if no referee is found.

## KAIS

- **Scope.** https://link.springer.com/journal/10115/aims-and-scope. Knowledge and advanced information systems: data engineering, data mining and high-performance computing. Graph algorithms and algorithm engineering are not named.
- **Exemplars,** each seen on its DOI page, abstracts only, so timing experiments are not confirmed:
  1. Boldrin, Bulat, Vandin, *Fast and accurate triangle counting in graph streams using predictions*, 68(1), 2026. DOI 10.1007/s10115-026-02702-8. An estimator, not exact counting.
  2. Zhao, Chen, Ye, Guan, Liu, *Maximal Δ-biclique enumeration in temporal bipartite graphs*, 67(11), 2025. DOI 10.1007/s10115-025-02545-9.
  3. Shakiba, *Correlation clustering algorithm for dynamic complete signed graphs: an index-based approach*, 67(7), 2025. DOI 10.1007/s10115-025-02397-3.
- **Indexing.** https://www.scimagojr.com/journalsearch.php?q=15703&tip=sid&clean=0 and the Master Journal List, SCIE. Springer's page gives a 2025 impact factor of 3.6.
- **Cost.** No APC by the subscription route, by https://link.springer.com/journal/10115/how-to-publish-with-us.
- **Fit.** Moderate. Data-mining and database readers will value exact counts on real temporal streams more than the cache cost model. It needs an applications framing, or it risks a desk rejection as pure algorithmics.

## J.UCS and JEA

- **J.UCS.** https://lib.jucs.org/about: all of computer science, by ACM classification. Three Crossref searches on ISSN 0948-6968 from 2023 found only applied machine learning among graph papers. SJR from https://www.scimagojr.com/journalsearch.php?q=145537&tip=sid&clean=0. The PI's note that its scope names algorithm engineering is not borne out by the about page.
- **JEA.** https://dl.acm.org/journal/jea, as read by the subagent in Chrome, and refused to my fetch with a 403, so not checked twice. Its 2023 volume is its last. TALG's experimental track, its successor, is under ACM's all open access model from 2026, with an APC of $950 to $1,450 unless the institution is in ACM Open (https://www.acm.org/publications/policies/policy-on-geographic-apc-waivers-and-discounts). TALG's scope, indexing and exemplars were not read.

## The four checks of bootstrap §3, for JGAA

| Check | State |
|---|---|
| Scope sentence | Passes, quoted above with URL and date |
| Exemplars | Passes on type, three engineering papers in 2025 and 2026. None on counting |
| Bibliography, five entries from the venue and its neighbours | Not yet: no draft. The Heidelberg engineering papers and Hanauer et al. are the nearest |
| Home field | Passes: a graph-algorithms result submitted to a graph-algorithms venue |

## Not verified

Whether UTeM is in ACM Open, and whether any ACM waiver applies to Malaysia. TALG's experimental track. JGAA's WoS status beyond the public Master Journal List. Whether the KAIS exemplars time their algorithms, and KAIS page limits. Scopus status from Scopus itself, rather than SCImago's coverage years. What UTeM's Springer open access funding prompt covers.

## Recommendation, for the PI

JGAA first: free, the right readers, and engineering papers of this type in the last two years. Its weakness is indexing: Q2 in one SJR category and not in WoS. If the PI's institution needs WoS for this paper, KAIS second, with the applications framing written in from the start. The choice is the PI's.

## Checked against JGAA, 2026-11-05

After the PI chose JGAA (`log.md` 0064), with no need for WoS. Read by fetch on 2026-11-05: the submission page, and the experimental sections of two of the three exemplars from their PDFs.

**Author guidelines,** https://jgaa.info/index.php/jgaa/about/submissions:

| Requirement | What it means for this paper |
|---|---|
| PDF with page and line numbers for review. LaTeX and BibTeX with the unmodified JGAA package (revision of 2021-01-07) for the final version | Write the draft in the JGAA class from the start |
| No length limit for regular papers. Concise papers run 5 to 10 pages | Regular paper. The exemplars run 24 to 33 pages |
| Figures as EPS or PDF vector drawings, with **text at least 10 pt**, inside the margins | The six SVG figures use 8 to 12 pt text and figure 5 is 970 px wide. All six need redrawing at the column width with 10 pt text, then converting to PDF |
| DOIs for references where they exist. Title case for the title and section headings | Bibliography work |
| Generative AI: allowed for things such as programming help in experimental work, **must be declared in the Comments for the Editor**, and undeclared use "can result in immediate rejection" | The declaration is the PI's to write. This project's code, runs and records were made with an AI assistant, and the declaration has to say so |
| No data or code policy | See the exemplars |

**Exemplars' methods, from the PDFs:**

| Paper | Instances | Machine | Repeats | Release |
|---|---|---|---|---|
| Großmann et al., hypergraph b-matching, JGAA 30(1) 2026 | 488 hypergraphs from four sources, and 10 graphs | One cluster machine type, Intel Xeon Gold 6230, runs in parallel, one core each | 10, arithmetic mean, performance profiles | GitHub and Zenodo |
| Fink and Rutter, synchronized planarity, JGAA 29(1) 2025 | 1,643 instances from an existing set, plus 13,834 and 6,300 generated | One machine type, Intel Xeon E5-2690v2, single-threaded, runs in parallel, one core each | Not read | GitHub, dataset on Zenodo |

**What follows for this paper.**
- **One machine is the norm here.** Both exemplars time on one machine type. The readiness gap of a second machine (outline, test 3) is weaker for this venue than it was written for. It is still a limit to state.
- **Instance count is the weak point.** The exemplars use hundreds to thousands of instances. This paper has 16 real-timestamp streams and 50 scored cells at p = 5. The answer is that real temporal streams with timestamps are scarce: they are Hanauer et al.'s real set, chosen by a rule fixed first. Synthetic streams would add count without adding evidence (hard rule 4). The introduction has to say this before a referee does.
- **Code and data release is expected in practice.** Both exemplars give GitHub and Zenodo. The repository has no remote. Releasing it is the PI's decision.
- **Exclusive pinned runs are stricter than the exemplars', not weaker.** Both run many experiments in parallel on one machine. Section 4 can say so without overclaiming.

**Bootstrap §3 checks, updated.** Scope passes. Exemplars pass, on type and now on method. The bibliography check (five entries from the venue and its neighbours) is still not done and comes with the draft. Home field passes.
