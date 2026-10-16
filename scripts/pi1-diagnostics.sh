#!/bin/sh
# Read-only diagnostics for Raspberry Pi 1 and its network interface.
set -eu
printf '\n== Kernel and architecture ==\n'
uname -a
printf '\n== CPU ==\n'
grep -E '^(model name|Hardware|Revision|Model)' /proc/cpuinfo || true
printf '\n== Memory ==\n'
grep -E '^(MemTotal|MemAvailable):' /proc/meminfo || true
printf '\n== Network interfaces ==\n'
ip -brief link show
printf '\n== IPv4 addresses ==\n'
ip -brief -4 address show
printf '\n== Repository commit ==\n'
git rev-parse --short HEAD
printf '\n== Compiler ==\n'
cc --version | head -n 1
printf '\n== CMake ==\n'
cmake --version | head -n 1
