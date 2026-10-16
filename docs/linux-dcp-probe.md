# Linux DCP Identify probe

This is an experimental **read-only DCP discovery responder**, not a PROFINET IO Device. It listens for untagged EtherType 0x8892 on a specified Ethernet interface, parses Identify requests and sends Identify responses.

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

## Run on an isolated bench network

```sh
sudo ./build/openpnio_dcp_probe eth0
```

Replace eth0 with the **dedicated test** Ethernet interface. Linux CAP_NET_RAW (typically root) is required. Stop with Ctrl+C.

The demo identity is `openpnio-test`. Vendor ID and Device ID are both zero placeholders; this prototype must **not** be used as a production PROFINET device or connected to a production plant network. DCP Set is deliberately disabled; the tool never changes network settings.

Capture on another terminal with `sudo tcpdump -i eth0 -e -nn ether proto 0x8892`.

Limitations: VLAN-tagged DCP traffic may not be received by this EtherType-bound socket; DCP Identify response delay scheduling, additional Identify selectors and full device identity metadata are not implemented. Network compatibility with Siemens engineering software has not been established.
