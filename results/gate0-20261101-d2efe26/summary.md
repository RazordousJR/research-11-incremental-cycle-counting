Runs: 36 of 36

## Score: grp_full operations / seq_net operations

| Stream | 1h | 1d | 7d | 100 | 1k | 10k |
|---|---|---|---|---|---|---|
| CollegeMsg | 1.387 | 1.391 | 1.358 | 1.387 | 1.394 | 1.317 |
| email-Eu-core-temporal | 1.490 | 1.462 | 1.372 | 1.491 | 1.475 | 1.379 |
| sx-mathoverflow | 1.869 | 1.900 | 1.806 | 1.898 | 1.868 | 1.575 |
| sx-askubuntu | 2.426 | 2.420 | 2.168 | 2.438 | 2.409 | 2.211 |
| sx-superuser | 2.368 | 2.356 | 2.139 | 2.378 | 2.357 | 2.217 |
| wiki-talk-temporal | 2.010 | 2.033 | 1.890 | 2.013 | 2.016 | 2.025 |

## seq_net operations / seq_raw operations (what netting gives)

| Stream | 1h | 1d | 7d | 100 | 1k | 10k |
|---|---|---|---|---|---|---|
| CollegeMsg | 1.461 | 1.417 | 1.303 | 1.458 | 1.430 | 1.274 |
| email-Eu-core-temporal | 1.300 | 1.054 | 0.793 | 1.318 | 1.137 | 0.773 |
| sx-mathoverflow | 1.299 | 1.273 | 1.154 | 1.292 | 1.237 | 0.658 |
| sx-askubuntu | 1.367 | 1.346 | 1.261 | 1.364 | 1.338 | 1.118 |
| sx-superuser | 1.339 | 1.322 | 1.240 | 1.337 | 1.320 | 1.170 |
| wiki-talk-temporal | 1.255 | 1.193 | 1.018 | 1.267 | 1.249 | 1.191 |

## Kernel-only ratio: grp_full kernel / seq_net kernel (excludes all batch overhead, not a gate figure)

| Stream | 1h | 1d | 7d | 100 | 1k | 10k |
|---|---|---|---|---|---|---|
| CollegeMsg | 1.056 | 1.085 | 1.085 | 1.057 | 1.086 | 1.068 |
| email-Eu-core-temporal | 1.085 | 1.120 | 1.142 | 1.083 | 1.110 | 1.140 |
| sx-mathoverflow | 1.065 | 1.149 | 1.188 | 1.121 | 1.171 | 1.137 |
| sx-askubuntu | 1.113 | 1.188 | 1.249 | 1.141 | 1.196 | 1.231 |
| sx-superuser | 1.114 | 1.181 | 1.227 | 1.137 | 1.184 | 1.204 |
| wiki-talk-temporal | 1.072 | 1.121 | 1.117 | 1.082 | 1.081 | 1.113 |

## grp_noorient / seq_net (order removed)

| Stream | 1h | 1d | 7d | 100 | 1k | 10k |
|---|---|---|---|---|---|---|
| CollegeMsg | 1.235 | 1.272 | 1.265 | 1.234 | 1.270 | 1.241 |
| email-Eu-core-temporal | 1.270 | 1.289 | 1.263 | 1.266 | 1.289 | 1.259 |
| sx-mathoverflow | 1.302 | 1.395 | 1.436 | 1.362 | 1.417 | 1.334 |
| sx-askubuntu | 1.358 | 1.416 | 1.480 | 1.381 | 1.424 | 1.480 |
| sx-superuser | 1.364 | 1.418 | 1.473 | 1.383 | 1.420 | 1.479 |
| wiki-talk-temporal | 1.279 | 1.340 | 1.387 | 1.254 | 1.291 | 1.335 |

## Streams

| Stream | Events | Net updates | Peak m | Vertices | Kernel ops, seq_net at 1k | ops per net update |
|---|---|---|---|---|---|---|
| CollegeMsg | 119670 | 31024 | 4561 | 1899 | 430640 | 13.9 |
| email-Eu-core-temporal | 664668 | 187712 | 2941 | 986 | 2318218 | 12.3 |
| sx-mathoverflow | 780882 | 485368 | 2613 | 24759 | 3886838 | 8.0 |
| sx-askubuntu | 1453322 | 961178 | 5976 | 157222 | 5696746 | 5.9 |
| sx-superuser | 2217478 | 1521360 | 7143 | 192409 | 9732216 | 6.4 |
| wiki-talk-temporal | 12201076 | 6598268 | 99908 | 1094018 | 71511624 | 10.8 |

## Kernel O use, grp_full at 1k

| Stream | groups A | groups O | whole-vertex O groups | spokes in O groups | re-ranks |
|---|---|---|---|---|---|
| CollegeMsg | 12185 | 121 | 27 | 1768 | 13 |
| email-Eu-core-temporal | 80187 | 872 | 60 | 14573 | 101 |
| sx-mathoverflow | 190510 | 4848 | 1272 | 42634 | 274 |
| sx-askubuntu | 448440 | 3530 | 860 | 27080 | 261 |
| sx-superuser | 719277 | 7371 | 2077 | 47372 | 324 |
| wiki-talk-temporal | 2765209 | 6793 | 580 | 240300 | 240 |

Exactness: mismatches=0, static checks=7232

Streams with 2+ cuts at <=0.67: 0 (need 3). All cells <=1.05: False. All cells >0.80: True.
Cells: 36. Minimum 1.317. Maximum 2.438.
Verdict by the written rule: STOP
