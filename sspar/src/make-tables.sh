#!/usr/bin/env bash
set -e
set -o pipefail

# rpm table
echo '15000 10000 7200 5900 5400 4200' \
 | tr ' ' '\n' \
 | awk '{printf "%d|%.1f|%d|%.1f|%d\n", $1, 1000.0/($1/60.0*2.0), $1/60.0*2.0, 1000.0/($1/60.0), $1/60.0}'
echo

# time to copy table
caps="1 2 4 6 8 10"
echo -n "|max sust|avg sust"
for cap in $caps; do
  echo -n "|$cap TB"
done
echo
for spdmax in 50 100 200 400; do
  spdavg=$(echo "$spdmax / 1.3" | bc)
  printf "|%3d MB/s|%3d MB/s" $spdmax $spdavg
  for cap in $caps; do
    seconds=$(($cap * 1000 * 1000 / $spdavg))
    minutes=$(($seconds / 60)); seconds=$(($seconds - ($minutes * 60)))
    hours=$(($minutes / 60)); minutes=$(($minutes - ($hours * 60)))
    echo -n "|"
    if [ "$hours" -le 10 ]; then
      printf "%dh:%02dm" $hours $minutes
    fi
  done
  echo
done

