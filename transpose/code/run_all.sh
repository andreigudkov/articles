#!/usr/bin/env bash
set -e
set -u

algos="Naive Reverse Blocks BlocksPrf Vec32 Vec64 Vec256 Vec256Buf"
sizes="320 576 704 1088 1472 2112 2880 4160 5824 8256 11584 16448 23232 30784 46400 65600 77888 92736 111808"

for algo in $algos; do
  for size in $sizes; do
    echo "$algo $size" >&2
    ./transpose $size $algo $((8*1024*1024*1024)) 0 1
  done > results/${algo}.result
done


