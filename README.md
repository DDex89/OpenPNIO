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
- Portable virtual **8DI + 8DO** process model (one byte each, DAP/PDev interface and port subslot descriptors, safe DO=0 on disconnect); **not yet connected to PROFINET AR/RTC1**
- **LLDP IEEE 802.1AB** frame codec/parser and optional Linux basic 5-second advertisement with `--lldp` (PROFINET organizational extensions and topology still missing)
- **DCE/RPC v4 connectionless** 80-byte header parser, endianness/body-length checks (**not** RPC endpoint/CM server)
- **RTC1** Ethernet/VLAN0 priority-tag frame and APDU-status codec (**not** cyclic I/O timing or negotiated IOCR)
- AR/watchdog **simulation core** that makes virtual DO safe when stopped/timed out; no CM Connect or real AR state yet
- Experimental **GSDML-V2.32 preview generator** with assigned-VendorID gating and a separately acknowledged unregistered isolated-lab mode (not yet PI-XSD-validated or imported into TIA Portal)
- Session-only lab DCP VendorID/DeviceID override via explicit CLI flags (no borrowed manufacturer identity hardcoded, persisted, or checked in; normal default remains `0x0000/0x0000`)
- Read-only six-operation batch DCP client mode (`--batch-read MAC`) plus Pi 1 regression script (`scripts/pi1-batch-validation.sh`)
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

**Real Ethernet laboratory testing on ARMv6 has verified DCP Identify All, Get IP, Get Name, Set Name, state readback and explicit refusal of an unauthorized IP change.** A follow-up Get IP confirmed that the interface still had its original IP/mask/gateway. Initial Pi 1 CTest: **7/7 passed** (later **8/8 passed** including virtual IO). GitHub Actions also passed C and offline Python regression suites. A subsequent Siemens TIA Portal V19 hardware session validated **permanent** DCP Set Name with storage across process restarts. Actual IP changes remain deliberately blocked.

Read the [hardware validation report](docs/hardware-validation-2026-10-17.md), the [S7-1500 interoperability roadmap](docs/s7-1500-integration-roadmap.md), and the [proposed IP assignment safety policy](docs/decisions/0008-dcp-ip-assignment-policy.md).

**This is a Siemens TIA Portal DCP discovery/configuration interoperability milestone, not a working PROFINET IO Device.** GSDML import, RPC/CM, AR establishment, cyclic RTC1 I/O and actual S7-1500 PLC connectivity remain future work.

## Documentation

- [Architecture](docs/architecture.md)
- [Reference audit](docs/reference-projects.md)
- [Linux DCP responder and Windows Python client](docs/linux-dcp-probe.md)
- [DCP Pi 1 hardware validation results](docs/hardware-validation-2026-10-17.md)
- [Siemens TIA Portal V19 DCP hardware interoperability](docs/siemens-tia-v19-dcp-validation-2026-10-17.md)
- [Consolidated Siemens + Pi 1 protocol batch test plan](docs/siemens-batch-test-plan.md)
- [Ephemeral lab identity override without committing borrowed VendorIDs](docs/lab-identity-override.md)
- [Siemens S7-1500 interoperability roadmap](docs/s7-1500-integration-roadmap.md)
- [Portable virtual 8DI/8DO module catalog](docs/virtual-io-model.md)
- [ADR 0008: Proposed guarded DCP IP assignment](docs/decisions/0008-dcp-ip-assignment-policy.md)
- [Modular lab development kit concept](docs/lab-development-kit.md)
- [Raspberry Pi 1 setup](docs/raspberry-pi-1.md)
- [Architecture decisions](docs/decisions/0001-ethernet-framing.md)

## Licensing

OpenPNIO's original implementation is licensed under [MIT](LICENSE). Reference projects and their licensing are recorded in [docs/reference-projects.md](docs/reference-projects.md). GPL implementation code is not copied into this MIT codebase.

## Limitations and safety

DCP implementation is incomplete, default DCP identity IDs in the Linux probe remain unassigned placeholders (opt-in session-only lab overrides exist), and station-name changes require explicit opt-in (with optional atomic file persistence). Working CM/AR, a functioning PNIO RPC server, cyclic RTC1 I/O, validated GSDML import and real S7-1500 PLC interoperability remain future work. The new protocol frame codecs and simulated watchdog do **not** establish a functioning IO Device. Never connect the experimental probe to a production industrial network.
