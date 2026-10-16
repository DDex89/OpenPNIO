# OpenPNIO

**Experimental portable PROFINET IO Device protocol core (C11, MIT).**

The long-term goal is a platform-independent PROFINET IO Device stack for Linux and microcontrollers, with initial interoperability experiments targeting Siemens S7-1500. **This is not yet a functional PROFINET IO Device**; no PLC interoperability, timing performance or certification is claimed.

## Implemented

- Ethernet/PROFINET frame parsing (EtherType 0x8892, single VLAN tag)
- DCP header and TLV decoding
- DCP Identify with standard device metadata (DeviceOptions, DeviceInstance, DeviceRole, live IPBlockInfo); Identify All plus name/type/ID selectors
- DCP Get for MAC, IP suite, name/type, device identity/role/options/instance
- DCP Set station name with qualifier-aware temporary/permanent storage and IP suite with safe callbacks (experimental)
- Linux AF_PACKET DCP Identify/Get/guarded Set responder: reports live IPv4 parameters, optional station-name persistence, never changes Linux network IP
- Windows/Linux Scapy DCP bench client for Identify, Get and explicitly confirmed Set
- Portable virtual **8DI + 8DO** process model (one byte each, slot/subslot validation, safe DO=0 on disconnect); **not yet connected to PROFINET AR/RTC1**
- CMake/CTest C tests, offline Python framing tests, and GitHub Actions CI

## Raspberry Pi 1 quick start (headless)

See [the full Raspberry Pi 1 guide](docs/raspberry-pi-1.md). On Raspberry Pi OS Lite **32-bit**, connect over SSH, then:

```sh
sudo apt-get update
sudo apt-get install -y git
git clone https://github.com/DDex89/OpenPNIO.git
cd OpenPNIO
sh scripts/pi1-setup.sh
sh scripts/pi1-diagnostics.sh
```

The setup script builds with one parallel job to conserve Pi 1 RAM and runs CTest. **Use a dedicated isolated network** before running the experimental DCP responder:

```sh
sudo ./build/openpnio_dcp_probe eth0
```

## Hardware milestone: DCP validated on Raspberry Pi 1 (2026-10-17)

**Real Ethernet laboratory testing on ARMv6 has verified DCP Identify All, Get IP, Get Name, Set Name, state readback and explicit refusal of an unauthorized IP change.** A follow-up Get IP confirmed that the interface still had its original IP/mask/gateway. Pi 1 CTest: **7/7 passed**; GitHub Actions also passed C and offline Python regression suites. The Set Name test used RAM only; persistence and accepting changed IP were **not** hardware-validated.

Read the [hardware validation report](docs/hardware-validation-2026-10-17.md), the [S7-1500 interoperability roadmap](docs/s7-1500-integration-roadmap.md), and the [proposed IP assignment safety policy](docs/decisions/0008-dcp-ip-assignment-policy.md).

**This is a DCP discovery/configuration milestone, not a working PROFINET IO Device.** GSDML, RPC/CM, AR establishment, cyclic RTC1 I/O and Siemens interoperability remain future work.

## Documentation

- [Architecture](docs/architecture.md)
- [Reference audit](docs/reference-projects.md)
- [Linux DCP responder and Windows Python client](docs/linux-dcp-probe.md)
- [DCP Pi 1 hardware validation results](docs/hardware-validation-2026-10-17.md)
- [Siemens S7-1500 interoperability roadmap](docs/s7-1500-integration-roadmap.md)
- [Portable virtual 8DI/8DO module catalog](docs/virtual-io-model.md)
- [ADR 0008: Proposed guarded DCP IP assignment](docs/decisions/0008-dcp-ip-assignment-policy.md)
- [Modular lab development kit concept](docs/lab-development-kit.md)
- [Raspberry Pi 1 setup](docs/raspberry-pi-1.md)
- [Architecture decisions](docs/decisions/0001-ethernet-framing.md)

## Licensing

OpenPNIO's original implementation is licensed under [MIT](LICENSE). Reference projects and their licensing are recorded in [docs/reference-projects.md](docs/reference-projects.md). GPL implementation code is not copied into this MIT codebase.

## Limitations and safety

DCP implementation is incomplete, identity IDs in the Linux probe are placeholders, and in-memory station changes require explicit opt-in (with optional atomic file persistence). AR, RPC/CM, RTC1, cyclic I/O, GSDML and real PLC interoperability remain future work. Never connect the experimental probe to a production industrial network.
