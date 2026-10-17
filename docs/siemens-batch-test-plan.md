# OpenPNIO — consolidated protocol validation batch (Pi 1 + TIA Portal V19)

**Status 2026-10-17:** DCP discovery/name assignment validated on real Siemens software. **Newly implemented since then:** independent LLDP, DCE/RPC v4 connectionless-header and RTC1 frame codecs, provisional AR/watchdog state machine, GSDML preview generator. The new features are **CI-validated only**, unless separately marked with actual captures below. No real PNIO RPC/CM/AR handshake or live RTC1 exchange has been implemented.

## Boundaries and current environment

- Windows Realtek MAC `d4:5d:64:64:ed:20`, Pi `eth0` MAC `b8:27:eb:bb:9e:7a`, Pi `192.168.1.8/24`.
- Pi is accessed over **SSH on the SAME interface**. Leave DCP Set IP in guarded/refusal-only mode. Run only on an isolated lab LAN, never a plant network.
- Existing persisted station name is `testnode`, file `/var/tmp/openpnio-station-name`. Keep the same file when starting the responder.
- The network is not an S7-1500 cyclic I/O test without a **real PLC and correct GSDML identity**.

## Single batch: Raspberry Pi build / all offline regressions

First stop an older instance of `openpnio_dcp_probe` with **Ctrl+C** in its SSH terminal. In a separate SSH window (or after stopping):

```sh
cd ~/OpenPNIO
git pull
sh scripts/pi1-batch-validation.sh
```

This compiles with `-j1` suitable for Pi 1 ARMv6, executes CTest **12 checks** and Python **15 checks** (as of this plan) and leaves the interface untouched. A green GitHub Actions build does not replace this specific hardware run.

For reference, unit test groups cover DCP parser/Identify/Get/guarded Set, IEEE LLDP frame build/parse, DCE/RPC v4 UDP header/drep/body bounds, generic RTC1 L2 tagged/untagged frame/APDU fields, provisional virtual 8DI/8DO device catalog, simulated AR connection watchdog/safe outputs, and GSDML XML structure and ID controls. None of these offline tests alone establishes Siemens PLC interoperability.

## Single batch: Ethernet with OpenPNIO + Siemens

**Pi window A** (run continuously):

```sh
cd ~/OpenPNIO
sudo ./build/openpnio_dcp_probe eth0 --allow-set-name --station-file /var/tmp/openpnio-station-name --lldp
```

Expected: station `testnode`, IP `192.168.1.8`, `Set IP: changes refused`. `--lldp` additionally sends **experimental basic IEEE 802.1AB LLDP advertisements** approximately every five seconds. LLDP topology interoperability and PROFINET-specific organizational TLVs are **not complete**.

**Pi window B**, new unique capture file (SSH excluded from capture):

```sh
sudo tcpdump -i eth0 -n -e -s 0 -w "/tmp/openpnio-batch-$(date +%s).pcap" 'not tcp port 22'
```

Once it prints `listening on eth0`, run tests below. Stop with Ctrl+C. The timestamped filename avoids the old `/tmp/tia-all.pcap: Permission denied` overwrite problem.

**Windows PowerShell:** update the packet client, then execute the whole read-only suite with **one command**:

```powershell
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/DDex89/OpenPNIO/main/tools/dcp_client.py" -OutFile "$env:USERPROFILE\dcp_client.py"
py "$env:USERPROFILE\dcp_client.py" --iface "Realtek PCIe GbE Family Controller" --batch-read b8:27:eb:bb:9e:7a --timeout 1.5
```

Expected **6/6**: DCP Identify All, Get Name, Get IP, Get ID, Get DeviceOptions and Get DeviceInstance. This is **read-only**; it does not Set name, Set IP or reset the device.

**TIA Portal V19:** Online access → Realtek PCIe GbE Family Controller → Update accessible devices / Accessible devices → Start search. Record screenshot confirming `testnode` and `192.168.1.8`. Optional diagnostic inspection only; do not press Assign IP or Reset.

**Pi window B**, summary for the saved capture (replace `ls` if it returns no files):

```sh
sudo tcpdump -nn -e -r "$(ls -t /tmp/openpnio-batch-*.pcap | head -n 1)" 'ether proto 0x8892 or ether proto 0x88cc or udp port 34964'
```

This reveals DCP frames and **optional LLDP advertisements**. UDP port 34964 is included for future DCE/RPC/PNIO CM attempts; it may legitimately be absent when scanning from TIA without a PLC. All SSH (TCP/22) is already excluded from the saved capture.

## GSDML preview (structural only)

```sh
cd ~/OpenPNIO
python3 tools/generate_gsdml.py --preview --output /tmp/OpenPNIO-PREVIEW.xml
```

This generates XML with **UNASSIGNED** `VendorID=0x0000` / `DeviceID=0x0000` to verify manifest/schema structure. **Do not import the preview as an actual device** or use it to initiate S7-1500 connections. Before a real GSDML import, request an assigned VendorID from **PROFIBUS & PROFINET International (PI)** and choose your DeviceID, then validate against the official GSDML V2.32 XSD. An importable candidate can be generated via:

```sh
python3 tools/generate_gsdml.py --vendor-id 0xYOUR_PI_ASSIGNED_ID --device-id 0xYOUR_DEVICE_ID --confirm-owned-vendor-id --output /tmp/GSDML-V2.32-OpenPNIO-Lab.xml
```

The symbolic placeholders in the example are **not valid arguments**: supply actual numeric hex values. An additional **unregistered lab-only** experimental identity mode now exists in the GSDML generator and DCP responder; it requires explicit isolated-lab confirmations, takes a pair of IDs strictly as local parameters, and does NOT insert any borrowed identity into tracked GitHub files. See [lab identity override instructions](lab-identity-override.md). This mode must not be used on an ordinary/shared network or taken as proof of PI VendorID ownership. Even successful GSDML import would only validate an XML/catalog description, not that a PROFINET controller can establish AR or exchange IO.

## Acceptance matrix

| Feature | Previous hardware | Current newly written code | Next real check |
| --- | --- | --- | --- |
| DCP Identify / Get IP/Name/ID | **TIA V19 PASS** for discovery and identification | Extended/retained | `--batch-read` and TIA rescan |
| DCP Set Name persistent | **TIA V19 PASS**: `testnode` saved and restored after process restart | Retained | Optional repeat (not required) |
| DCP Set IP | Change deliberately refused on real Pi | **Still refusal-only** | Recheck IP remains 192.168.1.8 |
| Basic LLDP transmit | Not yet physically tested | Codec plus opt-in `--lldp` live transmit | `tcpdump` showing 0x88cc sent from Pi |
| DCE/RPC connectionless | Not yet PLC tested | Header parser only | Unit test now; live protocol only with controller |
| PNIO CM/AR Connect | **NOT implemented** | Simulated AR state/watchdog only | Requires actual Connect/Release and RPC server |
| RTC1 cyclic IO | **NOT implemented** | Frame codec only | Requires negotiated IOCR and actual PLC |
| GSDML | **NOT imported/validated** | Structural generator with identity guard | Obtain valid VendorID; official XSD/TIA import |
| 8DI/8DO application | Physical armv6 unit tests PASS | Five catalog descriptors and safe DO reset | Integrate with CM/RTC1 after actual handshake |

## Safety and logs

By default we do **not** enable production outputs, borrow anyone's VendorID, imply PI conformance or deploy on a plant network. A separately acknowledged **isolated bench identity override** is explicitly experimental and never indicates manufacturer ownership; do not enable it on shared or plant Ethernet. Capture `0x8892`, `0x88cc` and `UDP 34964` while excluding TCP/22. A default PSK for SIMATIC NET Remote Communication is unrelated to PROFINET DCP. When sharing captures, redact unrelated private LAN endpoints if required.
