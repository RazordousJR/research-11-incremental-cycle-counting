# Template record, paper 1, JGAA (bootstrap §18)

| | |
|---|---|
| Source | https://jgaa.info/JGAA-LaTeX.zip, linked from the journal's own submissions page https://jgaa.info/index.php/jgaa/about/submissions |
| Read | 2026-11-05 |
| Archive | 311,119 bytes, SHA-256 `240d7964b5d8b79e75deb6ae3215a27f8a11e0a5e2d833c845ff80a45684f780` |
| Class | `jgaa-art.cls` with its art files, copied unmodified into `docs/paper/jgaa/`. The instructions give the package's last revision as 2021-01-07 |
| Bibliography style | `abbrvurl`, as the instructions require, with a `doi` field in every entry that has one |
| Added packages | `lineno` for line numbers, which the submission checklist asks for, and `array` for left-aligned table columns. `amsmath`, `amssymb` and `graphicx` are in the template |
| Text block | 15 cm by 20 cm, from the class |

## First page against a published article

Done on 2026-11-05: page 1 of `main.pdf` (commit f65792f) beside page 1 of Großmann et al., JGAA 30(1) 2026, both rendered at 55 dpi. They match in the journal header with the cube logo, the title and author block with ORCID marks, the submitted and accepted box, the abstract layout, the e-mail footnote and the CC BY footer, and in fonts (Computer Modern). One difference: the published paper puts its code and data links in the first-page footnote. Ours has a data and code section that says the repository is released on acceptance, which is the PI's decision. All fonts in `main.pdf` are embedded (`pdffonts`).

## Submission checklist, from the submissions page, read 2026-11-05

| Item | State |
|---|---|
| Not published or under review elsewhere | True: no preprint, no other submission |
| PDF | From `main.tex` |
| Page numbers and line numbers | Class gives page numbers; `lineno` gives line numbers |
| Stylistic and bibliographic requirements of the Author Guidelines | JGAA class unmodified; `abbrvurl`; title case in title and headings |
| DOIs for references where available | Checked per entry in `docs/paper/jgaa/refs-verified.md` |
| Title case for title and section headings | Checked by hand at the compile read |
| AI Policy agreed, use declared in Comments for the Editor | Statement drafted in `docs/paper/jgaa/ai-statement.tex`; wording is the PI's to approve (bootstrap §9, §21) |
| Optional: preferred Editorial Board member | Open, for the PI |

## Required statements in the manuscript

| Statement | Where |
|---|---|
| Funding | `\Ack` on the first page, wording from `AUTHORS.md` |
| Acknowledgement | Unnumbered section, wording from `AUTHORS.md` |
| AI use | Unnumbered section and the Comments for the Editor |
| Data and code availability | Unnumbered section. The repository is private on GitHub; releasing it is the PI's decision |
| Conflicts of interest | JGAA's page asks for none. None to declare |
