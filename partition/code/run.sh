#!/usr/bin/env bash
set -euo pipefail

make
for bench in ops layout kmeans balancer; do
  rm -f ${bench}.results
  for impl in PolyRbSet PolyHashSet PolyHopscotchSet ItemTwine ChunkTwine Carousel; do
    for repeat in $(seq 1 7); do
      echo ${bench} ${impl} ${repeat}
    done
  done
done | sort -R | while read bench impl repeat; do
  echo ${bench} ${impl} ${repeat}
  ./benchmark ${bench} ${impl} >> ${bench}.results
done

