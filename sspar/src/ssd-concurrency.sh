#!/usr/bin/env bash
set -e
set -o pipefail

echo "cnc read_iops read_p90 write_iops write_p90"
for i in $(seq 1 40); do 
  output=$(./drvperf --probes $(($i * 10000)) --method kaio --concurrent $i rndread 'extent')
  read_iops=$(echo "$output" | grep overall | grep -v concurrent | awk '{print $2}')
  read_p90=$(echo "$output" | grep p90 | awk '{print $2}')

  output=$(./drvperf --probes $(($i * 10000)) --method kaio --concurrent $i --sync none rndwrite 'extent')
  write_iops=$(echo "$output" | grep overall | grep -v concurrent | awk '{print $2}')
  write_p90=$(echo "$output" | grep p90 | awk '{print $2}')

  echo "$i $read_iops $read_p90 $write_iops $write_p90"
done

