#!/bin/sh
# Raspberry Pi 1 (ARMv6) development setup. Run as a normal user.
set -eu
if [ "$(id -u)" -eq 0 ]; then
  echo "Run as a normal user (sudo is used only for apt)." >&2
  exit 1
fi
echo "Architecture: $(uname -m)"
echo "OS: $(uname -s)"
if [ "$(uname -s)" != Linux ]; then
  echo "Linux required" >&2
  exit 1
fi
sudo apt-get update
sudo apt-get install -y --no-install-recommends git cmake make gcc libc6-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build --parallel 1
ctest --test-dir build --output-on-failure
echo "Build and unit tests complete."
echo "DCP probe (isolated test LAN only): sudo ./build/openpnio_dcp_probe eth0"
