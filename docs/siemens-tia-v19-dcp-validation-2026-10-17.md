# Siemens TIA Portal V19 — real DCP interoperability lab report

**Date:** 2026-10-17. **Status:** completed physical laboratory DCP discovery and permanent station-name assignment test on Raspberry Pi 1 (ARMv6). **Scope: DCP ONLY**; no PNIO AR or cyclic IO.

## Test bench

- Siemens **TIA Portal V19**, Windows PC with **Realtek PCIe GbE Family Controller**, MAC `d4:5d:64:64:ed:20`, IP `192.168.1.4`.
- Raspberry Pi **Model B Rev 2** (ARMv6), Raspberry Pi OS Lite, `eth0` MAC `b8:27:eb:bb:9e:7a`, IP `192.168.1.8/24`, gateway `192.168.1.1`.
- OpenPNIO portable C11 core running as root using a Linux AF_PACKET DCP responder; isolated *test* network is strongly recommended.
- Pi build and CTest **8/8 tests passed, 0 failures, 0.24 seconds** (including portable `virtual_8di_8do`).

## Actual observed sequence

1. The first TIA **Accessible devices / PN/IE** scan on the Realtek adapter found zero devices. A simultaneous Pi tcpdump of EtherType `0x8892` observed zero DCP packets. Python/Scapy DCP Identify from the **same Realtek** found OpenPNIO with the correct MAC, name, IP and netmask, so the DCP responder was operational.
2. Windows bindings showed `s7PnDiscoveryDriver`, `Siem_ISOTrans` and `SI_SNPNIO` enabled alongside Npcap and various virtualization filters. The Windows machine was then rebooted after Siemens installation. This is a **strongly correlated** recovery, not a proven sole root cause.
3. After reboot, Scapy capture during TIA scan observed **two** outbound multicast frames with EtherType `0x8892` and destination `01:0e:cf:00:00:00`; the Pi console logged `Identify response sent (142 bytes)`.
4. **TIA Portal V19 Accessible devices now found one device**, named `openpnio-test`, IP `192.168.1.8`, MAC `B8-27-EB-BB-9E-7A`, interface PN/IE and device type `OpenPNIO Linux...`. It reported scanning and information retrieval completed.
5. TIA **Online access / Online diagnostics** displayed the name and type. It also showed UI sections for Assign IP address, Assign PROFINET device name and Reset to factory settings. **UI availability is not evidence those operations are implemented**. The manufacturer description was empty: Linux DCP VendorID/DeviceID currently remain unassigned placeholders `0x0000/0x0000`.
6. TIA initiated Set Name. With `--allow-set-name` but **without** `--station-file`, Pi logged `Permanent DCP Set requires --station-file` and returned a DCP Set reply. This demonstrates correct refusal of an attempted permanent Set without configured storage; a reply transmission is not by itself proof of status OK.
7. TIA later assigned the **PROFINET NameOfStation `testnode`** while the Pi was running:
   ```sh
   sudo ./build/openpnio_dcp_probe eth0 --allow-set-name --station-file /var/tmp/openpnio-station-name
   ```
   Pi console logged `DCP Set station name -> testnode (persisted)` and `Set response sent (34 bytes)`.
8. The responder was stopped (Ctrl+C) and started again **with the same `--station-file`**. Startup reported **`Station: testnode, IPv4: 192.168.1.8`**, verifying persistence between process starts.
9. A second **TIA Accessible devices** scan found the same Pi with the new name **`testnode`**, IP **`192.168.1.8`**, and MAC **`B8-27-EB-BB-9E-7A`**. This closes the discovery → permanent Set → restart → independent readback loop.

## Validated vs not validated

**Validated:** Siemens TIA V19 DCP discovery, device identification/name readout, real Set Name Permanent, file persistence across process restart and Siemens rediscovery; Pi ARMv6 8/8 CTests.

**Not validated or not implemented:** name survives full *Pi reboot* (only program restart tested); GSDML install/recognition, vendor identity for productization, LLDP, DCP factory reset, DCP IP *changes*, DCE/RPC/CM, AR, RTC1 provider/consumer, S7-1500 PLC online/IO data, PNIO conformance and realtime guarantees. TIA Portal *can show* Assign IP and Factory Reset controls even when device cannot execute them.

**IP safety:** `192.168.1.8` is the same NIC used for SSH. DCP Set IP is intentionally refusal-only for changed addresses; no experimental auto-reconfiguration should be deployed without an independent management/recovery path.

## Next batch

Plan a **single captured test session** with one Pi build/CTest run plus TIA re-discovery, fixed 8DI/8DO metadata/GSDML import attempt (only once identity and GSDML are ready), and saved **non-SSH** packet capture. Do not claim unsupported CM/RTC1 as passing. The recipe and acceptance table live in [Siemens batch test plan](siemens-batch-test-plan.md).

Tools / prior docs: [DCP hardware report](hardware-validation-2026-10-17.md), [Linux DCP probe](linux-dcp-probe.md), [integration roadmap](s7-1500-integration-roadmap.md).
