#!/usr/bin/env bash
set -e
set -o pipefail
set -u

cmd="./transpose 46400 Vec256Buf $((8*1024*1024*1024)) 1 0"

test -e /tmp/perfctl && unlink /tmp/perfctl
mkfifo /tmp/perfctl
perf stat \
  --delay -1 \
  -e cycles,resource_stalls.sb,l2_rqsts.rfo_miss,l2_rqsts.rfo_hit,mem_load_retired.l1_hit,mem_load_retired.l1_miss,mem_load_retired.l2_hit,mem_load_retired.l2_miss \
  -a \
  --control=fifo:/tmp/perfctl \
  -- $cmd 2>&1 | while IFS='' read line; do
    echo "XXX: $line"
    if [ "$line" == 'PERF-BEGIN' ]; then
      echo 'enable' > /tmp/perfctl
    elif [ "$line" == 'PERF-END' ]; then
      echo 'disable' > /tmp/perfctl
    fi
  done

