# OpenPNIO — DCP hardware validation on Raspberry Pi 1

**Status: DCP experimental lab features validated on hardware.**  
Date: **2026-10-17** (device local capture timestamps).  
Platform: Raspberry Pi **Model B Rev 2**, BCM2835 **ARMv6**, Raspberry Pi OS Lite / Linux, Ethernet **eth0**.  
Host: Windows PC + Scapy/Npcap on a Realtek Ethernet adapter.  
Device MAC observed: `b8:27:eb:bb:9e:7a`.  
Repository: OpenPNIO (portable original C11 core, MIT).

This records **observed tests**, not a PROFINET certification, timing guarantee, or evidence of PLC interoperability.

## Hardware evidence

1. **ARMv6 build and C unit tests**: `ctest --test-dir build --output-on-failure` passed **7/7** on the physical Pi in **0.22 seconds**. Tests: `frame_parser`, `dcp_codec`, `dcp_identify_response`, `dcp_identify_request`, `dcp_get`, `dcp_set_station_name`, `dcp_set_ip_suite`.
2. **DCP Identify All** from Windows: one response received, with station name `openpnio-test` and Pi Ethernet MAC. A `tcpdump` capture on Pi recorded request FrameID **FEFE**, reply FrameID **FEFF**, and matching XID **340D3590**. The two timestamps were 343 µs apart *within this one packet capture*, not a guaranteed device response latency.
3. **DCP Get IP**: Windows sent request XID **77083F05**; Pi returned a single response carrying IP **192.168.1.8**, mask **255.255.255.0**, gateway **192.168.1.1**. Linux adapter reported the same live IP rather than prototype zero-address placeholders.
4. **DCP Get Name**: Windows sent XID **1E17426D**; Pi replied `openpnio-test`.
5. **DCP Set Name**, explicitly enabled on device with `--allow-set-name`: Windows sent XID **FBA7D51E**, device responded `Set 02/02: OK`. Pi console printed `DCP Set station name -> openpnio-lab (RAM only)` and `Set response sent (34 bytes)`.
6. **Readback after Set**: Windows DCP Get Name XID **83CDA38D** returned **`openpnio-lab`**. This independently verified that the application's active state changed, not only the response status.
7. **DCP Set IP refusal**: Windows requested **192.168.1.99** with XID **8F5D9DB0**; Pi returned **`Set 01/02: Set not possible`** as designed.
8. **Post-refusal Get IP**: XID **8FD2E59A** returned the original **192.168.1.8 / 255.255.255.0 / 192.168.1.1**. This independently verified that the rejected request did not alter the advertised network parameters.

Each Windows invocation reported **`Found 1 DCP response(s)`**.

## What these tests establish

- Physical two-way Ethernet communication between Windows/Npcap and the Pi ARMv6 DCP responder.
- Functional DCP Identify All, Get Name, Get IP, Set Name and Get-after-Set readback.
- Explicit refusal of an unauthorized Set IP and unchanged network parameters after refusal.
- A working incremental C11 build/test path on the Pi 1.

## What they **do not** establish

- DCP station-name persistence after reboot (the test explicitly used RAM-only mode).
- Acceptance of unchanged IP suite (no-op), or any actual IP reconfiguration.
- DCP reset/flash, additional selectors, LLDP interoperability, VLAN handling, GSDML acceptance, DCE/RPC/CM, AR, RTC1, cycle jitter, process data, real-time guarantees or conformity.
- Siemens TIA Portal / S7-1500 discovery, configuration or connectivity; all external stimuli in this report came from the local Windows Python test client.

## Root cause discovered during bring-up

Initial DCP Identify All requests (`FF FF 00 00`) were received by Linux but discarded by the station-name-only DCP Identify selector decoder. The code was corrected to accept `FF/FF` with empty selector payload, and a regression test was added against an actual padded 60-byte Ethernet request. Following the fix, Windows discovered the Pi.

## Next gates

See [S7-1500 integration roadmap](s7-1500-integration-roadmap.md) and [proposed IP assignment policy](decisions/0008-dcp-ip-assignment-policy.md). The following must be kept distinct: **DCP hardware milestone achieved** versus **PROFINET IO Device / PLC interoperability not yet achieved**.

## Repeat tests

On Pi (dedicated laboratory network only):

```sh
cd ~/OpenPNIO
cmake --build build -j1
ctest --test-dir build --output-on-failure
sudo ./build/openpnio_dcp_probe eth0 --allow-set-name
```

On Windows (Npcap + Scapy installed, using `tools/dcp_client.py`):

```powershell
py dcp_client.py --iface "Realtek PCIe GbE Family Controller" --get-ip b8:27:eb:bb:9e:7a
py dcp_client.py --iface "Realtek PCIe GbE Family Controller" --get-name b8:27:eb:bb:9e:7a
py dcp_client.py --iface "Realtek PCIe GbE Family Controller" --set-name b8:27:eb:bb:9e:7a openpnio-lab --confirm-set
py dcp_client.py --iface "Realtek PCIe GbE Family Controller" --get-name b8:27:eb:bb:9e:7a
```

All Set operations are explicitly gated in the client; the test should be performed on an **isolated lab Ethernet link**.
