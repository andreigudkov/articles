#!/usr/bin/env bash
set -e
set -o pipefail

sectors=1
while true; do
  length=$(($sectors*4096))

  probes=$((134217728/$length))
  if [ $probes -gt 1024 ]; then
    probes=1024
  fi
  if [ $probes -lt 16 ]; then
    probes=16
  fi

  output=$(./drvperf --probe-length $length --align 4096 --probes $probes rndread /dev/sda)
  p90=$(echo "$output" | grep p90 | awk '{print $2}')
  iops=$(echo "1000.0 / $p90" | bc -l)
  mbps=$(echo "$length/1000.0/$p90" | bc -l)
  echo "$length $mbps $iops"

  sectors=$((sectors*2))
done

