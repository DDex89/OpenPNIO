# OpenPNIO

**Experimental portable PROFINET IO Device protocol core (C11, MIT).**

The long-term goal is a platform-independent PROFINET IO Device stack for Linux and microcontrollers, with initial interoperability experiments targeting Siemens S7-1500. **This is not yet a functional PROFINET IO Device**; no PLC interoperability, timing performance or certification is claimed.

## Implemented

- Ethernet/PROFINET frame parsing (EtherType 0x8892, single VLAN tag)
- DCP header and TLV decoding
- DCP Identify response encoding and request dispatch (limited selectors)
- DCP Set station name and IP suite with application callbacks (experimental)
- Linux AF_PACKET DCP Identify responder (read-only; no Set)
- CMake/CTest unit tests and GitHub Actions CI

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

## Documentation

- [Architecture](docs/architecture.md)
- [Reference audit](docs/reference-projects.md)
- [Linux DCP responder](docs/linux-dcp-probe.md)
- [Raspberry Pi 1 setup](docs/raspberry-pi-1.md)
- [Architecture decisions](docs/decisions/0001-ethernet-framing.md)

## Licensing

OpenPNIO's original implementation is licensed under [MIT](LICENSE). Reference projects and their licensing are recorded in [docs/reference-projects.md](docs/reference-projects.md). GPL implementation code is not copied into this MIT codebase.

## Limitations and safety

DCP implementation is incomplete, identity IDs in the Linux probe are placeholders, and DCP Set callbacks do not provide transactional persistence. AR, RPC/CM, RTC1, cyclic I/O, GSDML and real PLC interoperability remain future work. Never connect the experimental probe to a production industrial network.
