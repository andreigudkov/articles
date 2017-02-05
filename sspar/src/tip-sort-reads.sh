#!/usr/bin/env bash
set -e
set -o pipefail

win=1
seed=$(date +%s)
while [ $win -le 1024 ]; do
  echo 3 > /proc/sys/vm/drop_caches
  src/tip-sort-reads /dev/sdc $seed $win
  win=$(($win * 2))
done

