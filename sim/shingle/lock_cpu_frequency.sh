#!/usr/bin/env bash
set -e
set -o pipefail

#https://serverfault.com/questions/716317/linux-why-does-the-cpu-frequency-fluctuate-when-using-the-performance-governor

function usage() {
  echo "Usage: $0 lock|unlock|status"
  exit 1
}

function write() {
  value="$1"
  path="$2"
  sudo bash -c "echo ${value} > ${path}"
  if [ $(cat $path) != ${value} ]; then
    echo "Failed to set ${path} to ${value}"
    exit 1
  fi
}

if [ $# -ne 1 ]; then
  usage
fi
sudo true
case "$1" in
  'lock')
    for path in /sys/devices/system/cpu/cpufreq/policy*/scaling_governor; do
      write 'performance' ${path}
    done
    write 1 '/sys/devices/system/cpu/intel_pstate/no_turbo'
    ;;
  'unlock')
    for path in /sys/devices/system/cpu/cpufreq/policy*/scaling_governor; do
      write 'powersave' ${path}
    done
    write 0 '/sys/devices/system/cpu/intel_pstate/no_turbo'
    ;;
  'status')
    echo "CPU: $(cat /proc/cpuinfo | sed -rn '/^model name[ :\t]*(.*)$/s//\1/p' | head -1)"
    echo "ScalingDriver: $(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_driver)"
    echo "ScalingGovernor: $(cat /sys/devices/system/cpu/cpufreq/policy0/scaling_governor)"
    if [ -e /sys/devices/system/cpu/intel_pstate/no_turbo ]; then
      noturbo=$(cat /sys/devices/system/cpu/intel_pstate/no_turbo)
      if [ ${noturbo} == '1' ]; then
        echo "TurboBoost: disabled"
      elif [ ${noturbo} == '0' ]; then
        echo "TurboBoost: enabled"
      else
        echo "TurboBoost: N/A"
      fi
    else
      echo "TurboBoost: N/A"
    fi 
    echo "Frequency: $(sudo cat /sys/devices/system/cpu/cpu*/cpufreq/cpuinfo_cur_freq | tr '\n' ' ')"
    for path in /sys/class/thermal/thermal_zone*/; do
      if [ $(cat "${path}/type") == 'x86_pkg_temp' ]; then
        echo "Temp: $(cat ${path}/temp)"
      fi
    done
    ;;
  *)
    usage
    ;;
esac

# Set scaling driver
#  1. Boot with intel_pstate=disable
#  2. Check that /sys/devices/system/cpu/cpu*/cpufreq/scaling_driver is set to acpi-cpufreq
# Set scaling governor
#  1. echo performance > /sys/devices/system/cpu/cpufreq/policy*/scaling_governor
#  2. Check that scaling governor is set to performance
# (Optionally) lower frequency with /sys/devices/system/cpu/cpu*/cpufreq/cpuinfo_max_freq
# Check that frequency is locked with /sys/devices/system/cpu/cpu*/cpufreq/cpuinfo_cur_freq

