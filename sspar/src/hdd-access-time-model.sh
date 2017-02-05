#!/usr/bin/env bash
set -e
set -o pipefail

echo 'pct max avg min'
for x in $(seq 0 100); do
  seek=$(echo "(sqrt(2.25*$x)+0.5)" | bc -l)
  total_min=$seek
  total_avg=$(echo "$seek + 4.2" | bc -l)
  total_max=$(echo "$seek + 8.3" | bc -l)
  echo "$x $total_max $total_avg $total_min"
done
