# Citation keys used in the draft, to be filled into a .bib

JGAA asks for complete BibTeX entries with page numbers and DOIs. No entry is filled from memory. Each must be read from the paper or publisher page before it goes into `refs.bib`.

| Key | What it cites | Where the project records it | State |
|---|---|---|---|
| `hanauer-dyncount` | Hanauer, Heck, Henzinger, Sidl, *Fully Dynamic Triangle and 4-Vertex Subgraph Counting: From Theory to Practice and Back Again*, arXiv:2610.29776, short version at ALENEX 2027. The 56-instance set and the stream cleaning | `docs/literature/audit-2026-11-01.md`, row 1 | arXiv only. Check whether the ALENEX version is out before submission |
| `cheetah` | Cheetah, ACM TACO 22(2), 2025, DOI 10.1145/3736173. Star grouping with reuse, and the trend | `docs/literature/r10-falsifier-reading.md` | Authors and pages to read |
| `hhh` | Hanauer, Henzinger, Hua. The heavy-light table with hubs scanned that M follows | `benchmarks/gate1.cpp` header names the idea only | Title, venue and year not on record. Must be found |
| `bfcb` | K. Wang et al., BFCB-IG, VLDB Journal 2022. Batch butterfly counting | `benchmarks/gate1.cpp` header | Title and pages to read |
| `shi-huang-shun` | Shi, Huang, Shun, static 5-cycle counting, Algorithm 1, JEA 2022 | `benchmarks/fivecycle.cpp` header | Title and pages to read |
| `chiba-nishizeki` | Chiba and Nishizeki, arboricity and subgraph listing, the wedge pairing used by A4 | `benchmarks/gate1.cpp` header names the authors only | Everything to read |
| `folklore-exactly-once` | That sequential application of the net updates counts every changed cycle once. Hard rule 6: cited, not claimed | `docs/novelty/paper-1.md`, Not claimed | Needs a source that states it, for example a dynamic counting paper's correctness lemma. If none is found, the sentence says "standard" with no citation |

The bootstrap §3 bibliography check wants at least five entries from JGAA and its neighbours. None of the keys above is a JGAA paper. Candidates from the venue-fit reading: the three exemplars in `docs/submissions/paper-1/venue-fit.md`.
