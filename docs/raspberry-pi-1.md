# Raspberry Pi 1: headless first run

Target: original Raspberry Pi 1 (ARMv6), Raspberry Pi OS Lite **32-bit**. No monitor or keyboard needed.

## Prepare SD card on a PC

Use Raspberry Pi Imager to write a compatible Raspberry Pi OS Lite 32-bit image. In its OS customization settings, configure a username and password, enable SSH password authentication, and set a hostname such as `openpnio-pi`. Connect the Pi to a DHCP router using Ethernet and power it on.

From Windows PowerShell:

```powershell
ssh YOUR_USERNAME@openpnio-pi.local
```

If mDNS does not resolve, obtain the IP from the router's DHCP lease table and use `ssh YOUR_USERNAME@IP_ADDRESS`.

## Install and test

On the Pi, over SSH:

```sh
sudo apt-get update
sudo apt-get install -y git
git clone https://github.com/DDex89/OpenPNIO.git
cd OpenPNIO
sh scripts/pi1-setup.sh
sh scripts/pi1-diagnostics.sh
```

The setup script installs the C compiler and CMake, builds with one job to conserve RAM, and runs all CTest tests.

## DCP bench test

Use **a dedicated, isolated test network** for DCP. Do not connect this experimental responder to an industrial/production PROFINET network. It is not yet a functional PROFINET IO Device.

```sh
sudo ./build/openpnio_dcp_probe eth0
```

The process responds to selected DCP Identify requests only. It does not implement DCP Set, RT IO, AR, GSDML or an S7-1500 connection. Stop with Ctrl+C.

For a packet capture (optional):

```sh
sudo apt-get install -y tcpdump
sudo tcpdump -i eth0 -e -nn ether proto 0x8892
```

**Important:** If the same Ethernet port is used for SSH and for an isolated DCP bench network, disconnecting it from the router will terminate the SSH session. Use a serial console, a second network interface, or a test switch with appropriate management access if uninterrupted console access is required.
