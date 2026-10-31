# CLAUDE.md — MAIN DIRECTORY
## Research Portfolio 2027: Five GPU Research Projects, One Machine, One Codebase Family

> **Instruction to Claude Code:** This is the ROOT context file for the entire research portfolio. In EVERY session, regardless of which project you are working in, follow the Session Boot Protocol below BEFORE doing anything else. Rules in this file apply to all projects. Project CLAUDE.md files may ADD rules but never override the rules here.

---

## 0. Session Boot Protocol (mandatory, every session)

1. Read THIS file (root `CLAUDE.md`) completely.
2. Read the `CLAUDE.md` of the project folder you are working in.
3. Read that project's `PLAN.md` (the PI's own research plan) if it exists. If `PLAN.md` conflicts with the project `CLAUDE.md`, **PLAN.md wins** — it is the PI's latest intent. Flag the conflict to the PI in your first message.
4. Check `STATUS.md` in the project folder (latest state, open problems, next steps). If it doesn't exist, create it at the end of your first session.
5. Only then start working.
6. At the end of every session: update the project's `STATUS.md` (date, what was done, results pointers, next steps).

## 1. Portfolio Overview

| Code | Project | Branch | Primary target venue | Submit target |
|------|---------|--------|----------------------|---------------|
| A | MP-PDHG | Mixed-precision first-order LP/QP solver on consumer GPU | Optimization & Engineering (Q2) / Math. Prog. Computation | Q1–Q2 2027 |
| B | Contact-PDHG | GPU first-order solver for large-scale frictional contact | OPTE (math version) + CGF/TVCG/SCA (visual version) | Q1–Q2 2027 |
| C | PhysGS | Physics-embedded 3D Gaussian Splatting, real-time on consumer RTX | TVCG (Q1) / CGF (Q1) / C&G (Q2) | Feb–Mar 2027 |
| D | NeuralBVH | Certified-conservative learned collision culling for deformables | CGF (Q1) / TVCG (Q1) / Graphical Models (Q2) | Dec 2026 – Jan 2027 |
| E | GenCrowd | Generative crowd steering + ORCA safety projection at scale | CGF (Q1) / TVCG (Q1) / SMPT (Q1) | Dec 2026 – Jan 2027 |

**Submission order of priority: D and E first (lowest risk, PI's home turf), then C, then A/B mature through the year.** If time pressure forces a choice, protect D and E's December deadlines.

**Synergy map (why these five live in one repo):**
- D's collision pipeline (LBVH, narrow phase) is the front-end of B's contact solver.
- B's spatial hashing and sparse formats are reused by C's MPM grid and E's neighbor queries.
- A's mixed-precision lessons (what tolerates FP16/FP32, what needs FP64) flow into B, C, and D's kernels.
- E's evacuation scenarios generate the huge network LPs that serve as A's engineering case study (OPTE requirement).
- All five share one benchmark discipline, one plotting style, one LaTeX setup.

## 2. The PI

- Lecturer in computer science: teaching, programming, research in computer graphics, animation, simulation; publishes WoS Q1/Q2 papers; builds apps; does consultation.
- Core expertise: computer graphics, collision detection (BVH), crowd simulation, CUDA/GPU programming. Newer to: optimization theory, learning-based methods — when developing math or ML components, explain the steps clearly in `docs/` notes.
- Communication: PI often writes casually (Malay/English mix). Interpret intent generously; confirm scope changes explicitly before acting on them.

## 3. Hard Hardware Constraint (applies to ALL projects)

- **Everything runs on the PI's single consumer RTX GPU + its CPU. No cloud. No A100/H100. Ever.**
- Consequences you must design around: limited VRAM, weak FP64 (consumer ratio), consumer memory bandwidth. Prefer FP32/FP16/mixed precision; document precision choices.
- This constraint is also a research ANGLE, not just a limitation — "works in real time on consumer hardware" is a selling point in every paper. Log GPU model, driver, and clocks with every experiment automatically.
- If an experiment cannot fit (VRAM/time), scale the problem down and say so; never silently switch to a different machine or fabricate scaled-up numbers.

## 4. Repository Structure & Conventions

```
research-2027/
├── CLAUDE.md                  # this file
├── shared/
│   ├── cuda/                  # shared kernels: LBVH, spatial hash, sparse formats, SpMV, reductions
│   ├── scripts/               # run_bench.py template, plotting style, GPU logger
│   ├── latex/                 # shared preamble, bib style, figure conventions
│   └── bib/                   # one shared references.bib (all projects cite from here)
├── projectA-mp-pdhg/
├── projectB-contact-pdhg/
├── projectC-physgs/
├── projectD-neuralbvh/
└── projectE-gencrowd/
```

Each project folder contains: `CLAUDE.md`, `PLAN.md` (PI's plan), `STATUS.md`, `src/`, `experiments/`, `scripts/`, `docs/`, `paper/`.

**Shared-code rules:**
- Anything used by ≥2 projects lives in `shared/` (sparse matrix format, LBVH, spatial hash, precision utilities, benchmark harness). Do not fork copies into project folders.
- `shared/` changes must not silently break another project: after editing `shared/`, run each dependent project's smoke test (`scripts/smoke.sh` in each project) before committing.
- One sparse matrix format across A/B/C (defined in `shared/cuda/sparse_format.md`); B's contact QPs must be exportable as A's benchmark instances.

**Git conventions:** one repo, one branch per experiment series if needed; commit before every benchmark run so `results.csv` rows always carry a valid commit hash; never commit large data blobs (use `.gitignore` + `data/README.md` describing how to regenerate/download).

## 5. Experiment Discipline (all projects — this is non-negotiable)

1. **Never fabricate numbers.** Every number reported anywhere (chat, STATUS.md, paper) must come from an actual logged run.
2. Every run goes through the project's `scripts/run_bench.py` (built from `shared/scripts/` template): logs config hash, git commit, GPU/driver/clocks, seed, timestamp → appends to `experiments/results.csv` (append-only, never hand-edited).
3. Stochastic methods: fixed reported seeds, ≥3 repeats (≥5 for learned policies), report median (+ IQR where relevant).
4. Baselines must be honestly tuned and documented (`docs/baseline_tuning.md`); a win over a crippled baseline is worthless and will be caught in review.
5. Each paper keeps `paper/claims_map.md` mapping every claim → experiment file. No claim without a row.
6. Negative results are reported honestly to the PI, with analysis and pivot options — never hidden or spun.
7. Safety-critical metrics (D's missed collisions, E's hybrid collision count) are sacred: nonzero = stop-the-line, fix or redesign, never average away.

## 6. Writing & Publication Conventions

- LaTeX from `shared/latex/`; venue templates per project (TVCG, CGF, OPTE/Springer).
- One shared `references.bib`; cite 2022–2026 literature prominently — every paper's related work must position against the latest wave, not just classics.
- Figures: consistent style from `shared/scripts/plotstyle.py`; every figure regenerable by one script.
- Paper drafts live in `paper/`; internal deadlines in each project's timeline. A full draft exists BEFORE polish experiments — writing reveals missing experiments.
- WoS Q1/Q2 target means: clear single contribution, strong baselines, honest limitations section. When in doubt, cut scope, not rigor.

## 7. Phase-0 Rule (all projects)

Before serious coding in any project: complete the gap check (arXiv 2024–2026 search as specified in each project CLAUDE.md), build `docs/related_work.md`, and report findings to the PI. If the exact contribution already exists, present the pivot options from the project file — do not start building anyway.

## 8. Scope Discipline

- Each project's CLAUDE.md defines scope guardrails and a GO/NO-GO gate. Respect them.
- Default answer to "should we also add X?" is **NO** until that project's core results table is complete.
- The portfolio succeeds by finishing papers, not by growing features. Two submitted papers beat five half-built systems.

## 9. Cross-Project Requests

If work in one project requires changing another project's code or shared code, state it explicitly to the PI first (one line: what, why, risk). Small `shared/` utility additions are fine without asking; behavior changes to existing shared code are not.
