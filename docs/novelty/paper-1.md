# Novelty record, paper 1

Status: **refuted at Gate 0, 2026-11-01** (`log.md` 0008). On six real streams the sharing this delta offers is
worth at most 13% even in hindsight. What follows is the record as written before the gate. The candidates that
replaced it are ranked in `docs/novelty/candidates-2026-11-01.md`. None has earned a novelty record yet.

Status before the gate: **provisional and unearned**. The delta below survived a search on 2026-11-01. GAMMA has been read and does not cover it.
It has not yet survived the TODS text of Mhedhbi et al., EDTC or ParaDyMS, and it has no result on a real
stream.

## The delta, in one sentence

We share work between the updates of one batch in exact scanning-based triangle maintenance, by peeling star
groups and choosing a per-edge or an oriented kernel for each group, with a bound on overhead against
sequential scanning and a family of batches where the gain is asymptotic.

## Against the closest work

| Paper | It does | This paper adds |
|---|---|---|
| Hanauer et al. 2026 | Engineers single-update counters and finds scanning best for triangles | A batch model on top of that scanning baseline: sharing between updates |
| Dhulipala et al. 2021 | Batch work bound through heavy-light tables | No tables beyond one vertex order, a bound relative to scanning, and evaluation against tuned scanning |
| Makkar et al. 2017 | Batches through inclusion-exclusion and one intersection per edge | A kernel chosen per group, with no correction terms |
| Ediger et al. 2010 | Groups batch edges by source vertex for updates | Counting cost decides the group and the kernel |
| Mhedhbi et al. 2019, 2021 | Adaptive vertex orderings per partial match for general queries | A choice made per group of updates. For a triangle with its changed edge fixed, per-match adaptivity has nothing to choose. The TODS hybrid plans are still unread, so this row stays open |

## Not claimed

The exactly-once proof, net-batch semantics, heavy-light partitioning, wedge indexes, grouping by vertex in
itself, compact-forward, and worst-case update bounds.

## Falsifier

Any paper that selects a counting kernel per group of a batch for exact triangle maintenance and evaluates it
ends this delta. Tuned sequential scanning matching the method on real streams ends the claim.
