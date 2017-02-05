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

  avg=$(sudo ./drvperf --probe-length $length --align 4096 --cache drop --probes $probes rndwrite /dev/sdb | grep avg | awk '{print $2}')
  echo "$length $probes $avg" | awk '{print $1, $2, $3, ($1/1000.0/$3)}'

  sectors=$((sectors*2))
done

