# Submission package, paper 1, JGAA

Prepared on 2026-11-05 (`log.md` 0069); the PI's approval to proceed is `log.md` 0070. **Submitted on 2026-11-05 as JGAA submission 3330** (`log.md` 0071). JGAA's portal is OJS at https://jgaa.info, and "Login or Register to make a submission" is on its submissions page. The PI logs in or registers; Claude never enters a password and never creates the account. Every form step that submits or confirms waits for the PI's word in chat.

## Gates before the portal (bootstrap §9, §21, §22)

| Gate | State | Who |
|---|---|---|
| §22 step 6, red-team read from the paper alone | Done, and the corrections made (`log.md` 0068) | Claude |
| §22 step 7, outside human read: a person fluent in English who does not know the paper, who returns the objective and result in their own words, every sentence read twice, and every unknown term | **Not done as written.** The PI reports that the authors read the manuscript; no separate outside reader was used. The PI decided to proceed (§9, `log.md` 0070) | PI |
| §22 step 8, the PI reads the paper as main author and changes what they would not say | Done: the PI reports the authors read it and found it correct, with no changes (`log.md` 0070) | PI |
| §21 AI statement, wording approved by the PI | Approved, with "revised" replaced by "approved" because no text was changed (`log.md` 0070) | PI |
| Co-authors: all six named with their affiliations and ORCIDs from `AUTHORS.md`, which records the co-authors' standing approval | Recorded | PI |
| Repository release: the data statement says "to be released on acceptance" | The PI made the repository public right after submission (`log.md` 0071). The URL goes into the data statement at the first revision | PI |

## What JGAA's own pages say, read 2026-11-05

From https://jgaa.info/index.php/jgaa/about/submissions:

- **AI Policy.** Generative AI may be used "where it genuinely assists", for example "for proofreading and polishing text written by the authors or as programming assistants in experimental work". Use must be disclosed in the Comments for the Editor, and "failure to correctly declare the use of generative AI can result in immediate rejection". The Author Guidelines add that "the unattributed use of any such tools in the creation of the paper's scientific content is forbidden and reason for rejection".
- **Selectivity.** "JGAA is a highly selective journal ... Presenting results that are new and correct is not in itself sufficient for publication."
- **Editor request.** Authors may name a member of the Editorial Board in the Comments for the Editor.

**What this means here.** In this project the assistant wrote the code, ran the experiments under protocols the PI approved, and drafted the manuscript. That goes beyond the two uses JGAA names as examples. The use is attributed, which the policy requires. Whether JGAA accepts a paper produced this way is the editor's decision, and the declaration must not understate it. The PI's own revision of the text in step 8 is what lets the statement say truthfully that the authors revised it.

## Files to upload

| File | Source |
|---|---|
| Manuscript PDF | `docs/paper/jgaa/main.pdf`, built by `docs/paper/jgaa/build.sh` from the commit named at upload. The PDF has page and line numbers, as required |

Source files are only asked for after acceptance.

## Metadata for the form

**Title.** Sharing Work Within Update Batches for Exact Dynamic Counting of Triangles, 4-Cycles and 5-Cycles: An Experimental Study

**Abstract.** Use the abstract of `main.tex` as plain text, with `\%` as `%`.

**Keywords.** dynamic graphs; batch updates; cycle counting; triangle counting; algorithm engineering; cost model

**Paper type.** Regular paper

**Authors, in order** (from `AUTHORS.md`; the PI sets the order):

| # | Name | Affiliation | Email | ORCID |
|---|---|---|---|---|
| 1 | Hamzah Asyrani Sulaiman (corresponding) | Department of Interactive Media, Faculty of Information and Communication Technology, Universiti Teknikal Malaysia Melaka | asyrani@utem.edu.my | 0000-0002-7714-5967 |
| 2 | Shafina binti Abd Karim Ishigaki | Department of Interactive Media, FTMK, UTeM | shafina@utem.edu.my | 0000-0003-4551-0824 |
| 3 | Asniyani Nur Haidar binti Abdullah | Department of Software Engineering, FTMK, UTeM | asniyani@utem.edu.my | 0009-0008-5148-2341 |
| 4 | Nazreen Abdullasim | Department of Interactive Media, FTMK, UTeM | nazreen.abdullasim@utem.edu.my | 0000-0001-9316-6277 |
| 5 | Mohamad Lutfi Dolhalit | Department of Interactive Media, FTMK, UTeM | lutfi@utem.edu.my | 0009-0006-0967-330X |
| 6 | Abdullah Bade | School of Science and Technology, Universiti Malaysia Sabah | abb@ums.edu.my | 0000-0002-4054-3064 |

## Comments for the Editor, approved by the PI on 2026-11-05

> Dear Editors,
>
> We submit "Sharing Work Within Update Batches for Exact Dynamic Counting of Triangles, 4-Cycles and 5-Cycles: An Experimental Study" as a regular paper. It has not been published and is not under consideration elsewhere.
>
> If possible, we would ask that H. Meyerhenke handle the paper, as an experimental study in algorithm engineering for dynamic graphs.
>
> Use of generative AI. This study was carried out with an AI assistant, Claude (Anthropic), under the direction of the corresponding author. The assistant wrote the source code of the kernels, the exactness tests and the analysis scripts; wrote the experimental protocols, each fixed in version control before its run; ran the measurements; searched the literature; and drafted the manuscript and its figures. Every count was checked against a brute-force count and between all kernels on every batch; every number was traced to a committed result file; every reference was checked against its publisher or DOI record. The authors read and approved the full manuscript and take full responsibility for its content.
>
> Yours sincerely,
> Hamzah Asyrani Sulaiman, on behalf of all authors

The editor request is a recommendation. The board, read on 2026-11-05 at https://jgaa.info/index.php/jgaa/about/editorialTeam, lists H. Meyerhenke (KIT Karlsruhe), C. Tsourakakis (Boston U) and F. Vandin (U Padova). Vandin co-authored a 2026 paper on triangle counting in graph streams (`venue-fit.md`, KAIS exemplar 1). That Meyerhenke works on algorithm engineering for large graphs and Tsourakakis on triangle counting is from memory and was not checked this session. Leaving the field empty is allowed.

## Portal steps, when the gates are passed

1. Claude opens https://jgaa.info/index.php/jgaa/login in the browser pane. The PI logs in, or registers, there.
2. Claude starts a new submission and fills the form from this file, step by step: checklist ticks, the Comments for the Editor, the PDF, the metadata and the authors. Each tick is a statement the PI makes, so Claude shows the list before ticking.
3. Before the final "Finish Submission" step, Claude shows the filled form and the PI confirms in chat. Then the PI or Claude, on that word, submits.
4. Afterwards: the submission ID and date go into `log.md`, STATUS and this file.
5. Done on 2026-11-05: submission 3330 (`log.md` 0071).
