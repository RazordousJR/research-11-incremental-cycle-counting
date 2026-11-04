# M1D09: the cost model, operations against the clock

Protocol: `docs/experiments/m1d09-cost-model.md`, committed as a712ab3 before any fit was read. Script: `scripts/m1d09_cost.py`, run once. Output: `read.txt`, coefficients in `fit.csv`, per evaluation arm-cell predictions in `cells.csv`. Log: `log.md` 0063. No machine time: logged count builds and logged timing on CPU 8 only.

**Outcome, by the rule: Carries.** Pricing each kind of operation predicts the clock winner on new streams, where a unit cost does not. The reading beside it limits what that means. Pricing by kind does all the work, and the second price for tables over L3 adds nothing. The model does not price the table arms: it puts WJ at about a fifth of its measured time.

**Correction, 2026-11-05 (`log.md` 0068).** The `probe` counter is a lookup of an edge in the graph's global hash map of edges (`g.has` in `benchmarks/gate1.cpp` and `benchmarks/fivecycle.cpp`), which every kernel makes, not a probe into a wedge table. Where this README says "a table probe", read "an edge lookup". What separates the table kernels is the price of a wedge-table read, about 1 to 2 ns, and of a write, about 1.5 to 4.9 ns, against 0.05 to 0.6 ns for a row or scan step. The readings and the outcome are unchanged.

## Sets

| Set | Streams | Cells | Arm-cells | Tables over L3 |
|---|---|---|---|---|
| Fitting | sx-askubuntu, sx-superuser, wiki-talk-temporal, p = 3, 4, 5 | 36 | 173 | 42 |
| Evaluation | M1D07's 12 new streams, snapshot left out, p = 3 and 5 | 66, of which 64 have two arm-cells | 276 | 102 |

An arm-cell under 0.05 s is left out, which removes the four ia-reality-call cells at p = 5 and some p = 3 arm-cells.

**Correction, 2026-11-04 (`log.md` 0064).** The protocol says the engineering tables fit in L3. They do not on sx-mathoverflow insert-only, where T3, T4 and WT reach 409 MB and WJ 768 MB. M1D10 fits the same models on the engineering streams.

## The readings

| Model | Set | Median error | Winner agrees | PR's side of 0.67, p = 5 |
|---|---|---|---|---|
| U, unit cost | fitting | 63.7% | 23 of 36 | 7 of 12 |
| U | evaluation | 70.1% | **21 of 64** | 12 of 32 |
| K, by kind | fitting | 19.6% | 34 of 36 | 12 of 12 |
| K | evaluation | 23.4% | 63 of 64 | 29 of 32 |
| KC, by kind and cache | fitting | 18.2% | 33 of 36 | 12 of 12 |
| KC | evaluation | **22.9%** | **64 of 64** | 31 of 32 |

The outcome test is KC against U on the evaluation set. Winner agreement gains 67 points, against the 25 needed, and the median error is 22.9%, against a limit of 25%. Both hold.

The one p = 5 cell where KC puts PR on the wrong side of 0.67 is dewiki real 100, at 0.669 measured and 0.681 predicted. The protocol's reading is on medians, so this is not Gate 1's pass test.

## Coefficients, ns per counted operation

| Counter | K | KC | KC, extra when the table exceeds L3 |
|---|---|---|---|
| scan | 0.52 | 0.56 | |
| row | 0.26 | 0.23 | |
| probe | 18.17 | 17.64 | 0.00 |
| deg | 1.84 | 2.04 | |
| tab_read | 2.12 | 0.96 | 1.71 |
| tab_write | 4.30 | 4.87 | 0.15 |
| graph | 0.00 | 0.00 | |
| net_ops | 20.08 | 21.28 | |
| group | 5.78 | 4.68 | |
| rc | 0.78 | 0.78 | |

U's one price is 0.50 ns. The counters are correlated across arms, so a single coefficient is not a measured price. `graph` at zero and `net_ops` at 20 ns most likely share the cost of the graph update between them. What the coefficients show together is that a table probe or table write costs about 8 to 80 times a row or scan step.

## Read beside it

These readings were not fixed in the protocol, and they are labelled as such. They limit the outcome.

- **The cache price adds nothing.** K, with no cache term, already names 63 of 64 winners, at a median error of 23.4%. The second price is zero for probes, 0.15 ns for table writes, and 1.71 ns for table reads. So the data do not show that a table over L3 is what makes the table arms slow.
- **The model misses the table arms**, as the median of prediction over clock per arm on the evaluation set shows. WJ is at 0.18 (range 0.12 to 0.64), so WJ runs about 5.5 times slower than the model says. M5 is at 1.41 and A5 at 1.74, both overpredicted. By arm, the median error is 82% for WJ, 74% for A5 and 41% for M5. For PR, R2, WT, the p = 3 arms and M3 it is between 7% and 22%. WJ and WT share counters, and WT runs at 1.42 ns per operation against WJ's 9.55 (M1D07), so what separates them is not in any counter, table size included.
- **The winners are not hard to name.** By the clock, PR is fastest on all 32 evaluation cells at p = 5, and P3 or M3 on 31 of 32 at p = 3. The runner-up is within 20% on 18 of 64 cells. Unit cost fails because the count of operations favours the table arms, which is the fact the section explains.
- **One machine, one P-core.** Every price is this machine's.

## What section 6 can say

Pricing each kind of operation, fitted on the first three streams, names the clock winner on all 64 new cells, and unit cost names 21. A table probe costs about 18 ns against about 0.2 to 0.6 ns for a row or scan step. That is the measured form of "the price of a table probe rather than a count of operations." The section cannot say that the price comes from the table outgrowing the cache. The cache term adds nothing, and the model underprices WJ about fivefold, with no counter that explains it.
