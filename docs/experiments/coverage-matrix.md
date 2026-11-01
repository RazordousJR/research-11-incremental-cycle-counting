# Coverage matrix, draft

Filled before the experiments and updated as streams are added. Nothing below has been run on the real
inputs. The floor for the count of inputs is set by comparison: Hanauer et al. use 56 instances, and the
target venue's own median is not yet counted (M1A17).

| Axis | Plan | Status |
|---|---|---|
| Number of distinct inputs | About 20 real streams plus synthetic hard cases | 6 real streams run at Gate 0, then stopped |
| Size | From 6K to 60M updates, from small messaging logs to sx-stackoverflow | Not run |
| Static and dynamic | Every input is a stream. Figure: batch cost over time on one logged run | Not run |
| Standard inputs | The SNAP and KONECT instances used by Hanauer et al. | Not run |
| Hard case | Hub-to-hub churn, uniform random updates, late-forming hubs | Synthetic generators for uniform exist |

| Input | Source | Label | Size | Batch cuts | Done |
|---|---|---|---|---|---|
| CollegeMsg | SNAP | Real timestamps, synthetic expiry | 0.3 MB gz | time, count | Gate 0 only |
| email-Eu-core-temporal | SNAP | Real timestamps, synthetic expiry | 1.7 MB gz | time, count | Gate 0 only |
| sx-mathoverflow | SNAP | Real timestamps, synthetic expiry | 3.6 MB gz | time, count | Gate 0 only |
| sx-askubuntu | SNAP | Real timestamps, synthetic expiry | 7.4 MB gz | time, count | Gate 0 only |
| sx-superuser | SNAP | Real timestamps, synthetic expiry | 11.2 MB gz | time, count | Gate 0 only |
| wiki-talk-temporal | SNAP | Real timestamps, synthetic expiry | 48.1 MB gz | time, count | Gate 0 only |
| sx-stackoverflow | SNAP | Real timestamps, synthetic expiry | 529.9 MB gz | time, count | no |
| Wikipedia link streams | KONECT | Real insertions and deletions | Not checked | time, count | no |
