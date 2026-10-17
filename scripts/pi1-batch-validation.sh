#!/bin/sh
# One batch of portable protocol regression tests on Raspberry Pi 1 ARMv6.
# This script NEVER changes interface IP/hostname, starts network services,
# or sends state-changing DCP packets. Invoke while DCP daemon is stopped,
# then launch the responder separately as documented.
set -eu
cd "$(dirname "$0")/.."
printf '\n[1/4] OpenPNIO revision\n'
git rev-parse --short HEAD || true
printf '\n[2/4] C11 configure/build (single job for ARMv6)\n'
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j1
printf '\n[3/4] All CTest suites (DCP, LLDP, RTC1, RPC, safe AR)\n'
ctest --test-dir build --output-on-failure
printf '\n[4/4] Offline Python packet/GSDML tests\n'
python3 -m unittest discover -s tests -p 'test_*.py' -v
printf '\nPASS: offline protocol regression batch completed.\n'
printf 'Next: start Linux responder with --lldp; run DCP Windows --batch-read and TIA discovery.\n'
printf 'Reminder: NO S7-1500 AR/cyclic IO interoperability is claimed.\n'
