#!/bin/bash
# M1B03 production: 10^7 counted proposals per graph and target, all backends in lockstep, one process at a time.
# Usage: bash scripts/run_m1b03.sh <small|su-ins|rmat|wiki-ins|extras>
R=results/screen-b-M1B03
OUT=$R/production.csv
LOG=$R/logs
mkdir -p $LOG
param() { grep "^$1,$2," $R/theta.csv | cut -d, -f3,4; }
main() {  # graph burn target extra-args
    local g=$1 b=$2 t=$3; shift 3
    local p=$(param $g $t); local tt=${p%,*} a=${p#*,}
    local name=$(echo "$g-$t" | tr ':' '_')
    timeout 590 ./build/ergm_chain.exe run graph=$g a=$a tt=$tt target=$t burn=$b N=1e7 maxm=3 out=$OUT "$@" > $LOG/$name$TAGSUFFIX.log 2>&1
    echo "$name$TAGSUFFIX exit=$?"; tail -1 $LOG/$name$TAGSUFFIX.log
}
ALL="hubs=64,256,1024,4096 memos=1:3,4:3,16:3,4:16"
case $1 in
small)
    for t in 0.15 0.05 0.01; do main su-peak 2e6 $t tag=main $ALL; main wiki-peak 2e6 $t tag=main $ALL; done ;;
su-ins)
    for t in 0.15 0.05 0.01; do main su-ins 7.2e6 $t tag=main eager=formula verify=5e7 $ALL; done ;;
rmat)
    for t in 0.15 0.05 0.01; do main rmat:16:16:20261101 9.1e6 $t tag=main eager=formula hubs=1024,4096 fsplit=64,256 memos=1:3,4:3,16:3,4:16 checks=10 verify=5e7; done ;;
rmat2)
    # 0.15 and 0.05 hit the 590 s limit with all backends in one process, because scan costs near 400 per proposal on
    # this graph. These repeats split them into a scan process and a table process. The decision hash ties the two.
    for t in 0.15 0.05; do
        TAGSUFFIX=-scan main rmat:16:16:20261101 9.1e6 $t tag=main hubs= memos=4:3,16:3 nomemo=0 eager=0 dyads=0 tables=0 checks=3
        TAGSUFFIX=-tab main rmat:16:16:20261101 9.1e6 $t tag=main eager=formula hubs=1024 fsplit=64,256 memos= nomemo=0 checks=3 verify=5e7
    done ;;
wiki-ins)
    for t in 0.15 0.05 0.01; do main wiki-ins 2.8e7 $t tag=main eager=formula hubs=1024,4096 fsplit=64,256 memos=1:3,4:3,16:3,4:16 checks=10 verify=5e7; done ;;
warm)
    TAGSUFFIX=-warm main su-ins 7.2e6 0.05 tag=warm eager=formula hubs=64,256 memos=1:3,4:3,16:3,4:16 verify=5e7 warm=1e7
    TAGSUFFIX=-warm main wiki-ins 2.8e7 0.05 tag=warm eager=formula hubs=1024 fsplit=64,256 memos=1:3,4:3,16:3,4:16 checks=5 verify=5e7 warm=1e7 ;;
extras)
    for t in 0.15 0.05 0.01; do
        TAGSUFFIX=-early0 main su-peak 2e6 $t tag=early0 early=0 $ALL
        TAGSUFFIX=-early0 main wiki-peak 2e6 $t tag=early0 early=0 $ALL
    done
    TAGSUFFIX=-early0 main su-ins 7.2e6 0.05 tag=early0 early=0 eager=formula verify=5e7 hubs=64,256 memos=1:3,4:3,16:3,4:16
    for t in 0.05 0.01; do
        TAGSUFFIX=-seed2 main su-peak 2e6 $t tag=seed2 seed=2 $ALL
        TAGSUFFIX=-seed2 main wiki-peak 2e6 $t tag=seed2 seed=2 $ALL
        TAGSUFFIX=-seed2 main su-ins 7.2e6 $t tag=seed2 seed=2 eager=formula verify=5e7 hubs=64,256 memos=1:3,4:3,16:3,4:16
    done
    # memo keeps only tie-proposal dyads at the top level, since a random non-edge almost never comes back
    for g in su-peak wiki-peak; do TAGSUFFIX=-strong main $g 2e6 0.05 tag=strongscan memoadd=0 hubs=64 memos=1:3,4:3,16:3,4:16 eager=0; done
    TAGSUFFIX=-strong main su-ins 7.2e6 0.05 tag=strongscan memoadd=0 hubs=64 memos=1:3,4:3,16:3,4:16 eager=0 verify=5e7
    # one backend alone, separate processes: the decision hash must equal the lockstep hash
    for g in su-peak wiki-peak; do
        TAGSUFFIX=-solo-scan main $g 2e6 0.05 tag=solo-scan hubs= memos=4:3 nomemo=0 eager=0
        TAGSUFFIX=-solo-split main $g 2e6 0.05 tag=solo-split hubs=64 memos= nomemo=0 eager=0
        TAGSUFFIX=-solo-eager main $g 2e6 0.05 tag=solo-eager hubs= memos= nomemo=0
    done ;;
esac
