#!/usr/bin/env bash
set -e
set -o pipefail

declare -a xvalues
declare -a yvalues

function gen() {
  local min="$1"
  local max="$2"
  local cnt="$3"

  for i in $(seq 1 "$cnt"); do
    local val=$(( $min+($RANDOM%($max-$min)) ))
    yvalues[${#yvalues[@]}]=$val
  done
}

function dump() {
  local capacity=2000 #gigabytes
  local count=5

  for i in $(seq 0 $(($count-1))); do
    j=$((${#yvalues[@]}/($count-1)*$i))
    xvalues[$j]=$(($capacity/($count-1)*$i))
  done

  for i in $(seq 0 $((${#yvalues[@]}-1))); do
    echo -e "${xvalues[$i]}\t${yvalues[$i]}"
  done
}


gen 80 100 15
gen 0 5 150
gen 50 60 10
gen 0 5 110 
gen 20 40 15
gen 70 100 5
gen 20 40 10
gen 0 5 110
dump

