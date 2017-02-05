#!/usr/bin/env bash
set -e
set -o pipefail

seq_speed='200000000' # 200 MB/s
rnd_times='0.0001 0.001 0.01'

# header
echo -en "size_unit\tsize"
for rnd_time in $rnd_times; do
  printf "\t%4.1f ms" $(echo "$rnd_time*1000" | bc -l)
done
echo 

for pow in $(seq 10 32); do
  size=$(echo "2 ^ $pow" | bc -l)
  if [ $pow -lt 20 ]; then
    size_unit=$(echo "2 ^ ($pow - 10)" | bc -l)" KiB"
  elif [ $pow -lt 30 ]; then
    size_unit=$(echo "2 ^ ($pow - 20)" | bc -l)" MiB"
  else
    exit 1
  fi
  if [ $(($pow/2*2)) -eq $pow ]; then
    echo -en "$size_unit"
  fi
  echo -en "\t$size"

  for rnd_time in $rnd_times; do
    seq_time=$(echo "$size/$seq_speed" | bc -l)
    total_time=$(echo "$rnd_time + $seq_time" | bc -l)

    expected_throughput="$seq_speed"
    actual_throughput=$(echo "$size / $total_time" | bc -l)
    efficiency=$(echo "$actual_throughput / $expected_throughput * 100.0" | bc -l)

    echo -en "\t$(echo "$actual_throughput/1000000.0" | bc -l)"
  done

  echo 
done

