# Linux PROFINET DCP laboratory responder

Experimental **DCP Identify / Get / guarded Set** over Linux AF_PACKET. This is **not** a complete PROFINET IO Device (no CM/AR, RTC1, cyclic I/O or certification).

## First real-hardware result (2026-10-17)

OpenPNIO built on Raspberry Pi **Model B Rev 2, BCM2835 ARMv6**, Raspberry Pi OS Trixie. Six initial CTests passed, and a Windows Scapy/Npcap client discovered the station over Ethernet.

Actual captured transaction (example):

- Windows MAC `d4:5d:64:64:ed:20` -> DCP multicast `01:0e:cf:00:00:00`, FrameID `0xFEFE`, XID `0x340D3590`.
- Raspberry MAC `b8:27:eb:bb:9e:7a` -> Windows, FrameID `0xFEFF`, same XID, name `openpnio-test`.
- Two timestamps in a single tcpdump capture were 343 microseconds apart. **Not** a guaranteed realtime latency measurement.
- The test uncovered and fixed a bug in the original Identify-All `FF/FF` handling.

**Update:** subsequent physical tests on 2026-10-17 also verified **DCP Get IP, DCP Get Name, Set Name (in-memory), successful Get readback, rejection of a changed IP and unchanged-IP readback**. All seven Pi ARMv6 CTests passed. The complete evidence, packet XIDs, commands, tested/non-tested distinctions are recorded in the [DCP hardware validation report](hardware-validation-2026-10-17.md). These successful DCP tests are **not** evidence of interoperability with Siemens S7-1500.

## Linux build and run

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j1
ctest --test-dir build --output-on-failure
sudo ./build/openpnio_dcp_probe eth0
```

Run **only on an isolated laboratory Ethernet network**, not on a production or industrial segment. Requires `CAP_NET_RAW` (typically `sudo`). Ctrl+C stops the responder.

The responder reads **IPv4 address, netmask and default gateway from Linux** for the selected interface (e.g. `eth0`). DCP Identify and DCP Get report these current values; Linux hostname is independent of PROFINET NameOfStation. Vendor ID / Device ID are still zero placeholders. No code changes the interface IP or routing table.

### Name Set, disabled by default

To enable *temporary*, in-memory station-name changes:

```sh
sudo ./build/openpnio_dcp_probe eth0 --allow-set-name
```

To **also persist** station-name changes across restarts, provide a writable file path. The parent directory must already exist:

```sh
sudo ./build/openpnio_dcp_probe eth0 --allow-set-name --station-file /var/tmp/openpnio-station-name
```

**BlockQualifier semantics (implemented, newly awaiting physical persistence validation):**
- Qualifier `0x0000`: **temporary**, modify active RAM name only; never touch `--station-file` even if one is supplied.
- Qualifier `0x0001`: **permanent**, requires both `--allow-set-name` **and** `--station-file`, writes atomically, then changes active name. If no file is configured or the write fails, returns `Set not possible` and keeps the old active name.
- Other qualifiers: rejected without invoking application callback.
- Multiple Set blocks in one request: currently rejected **before changing any setting**, pending transactional processing support.

The state file is a plain ASCII station name plus a newline, read on startup. Updates use a temporary file and atomic rename. Without `--allow-set-name`, all station-name Set requests return `Set not possible`; the existing saved name may still be read on startup. The Linux hostname is independent and remains unchanged.

DCP Set IP always refuses any **change** with status `5` (`Set not possible`). If the requested IP, netmask **and gateway** already match the live interface, it replies status `0` as a no-op. **No SSH-disrupting address changes are implemented.** A guarded `temporary`/`persistent` backend is **proposed**, not implemented; see [ADR 0008](decisions/0008-dcp-ip-assignment-policy.md) and [IP Set work item](https://github.com/DDex89/OpenPNIO/issues/3).

DCP Set is experimental and unauthenticated at Ethernet layer; enabling it on shared networks is inappropriate.

## Windows DCP client (no TIA Portal needed)

Install Npcap on Windows, then:

```powershell
py -m pip install scapy
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/DDex89/OpenPNIO/main/tools/dcp_client.py" -OutFile "$env:USERPROFILE\dcp_client.py"
py "$env:USERPROFILE\dcp_client.py" --list
```

Use your network card name as reported by `--list`. Example for the lab's Realtek controller:

```powershell
$iface = "Realtek PCIe GbE Family Controller"
$mac = "b8:27:eb:bb:9e:7a"
py "$env:USERPROFILE\dcp_client.py" --iface $iface
py "$env:USERPROFILE\dcp_client.py" --iface $iface --get-name $mac
py "$env:USERPROFILE\dcp_client.py" --iface $iface --get-ip $mac
py "$env:USERPROFILE\dcp_client.py" --iface $iface --get-options $mac
py "$env:USERPROFILE\dcp_client.py" --iface $iface --get-id $mac
py "$env:USERPROFILE\dcp_client.py" --iface $iface --get-instance $mac
```

**Optional state-changing test, only on an isolated test network and after starting the responder with `--allow-set-name`:**

```powershell
py "$env:USERPROFILE\dcp_client.py" --iface $iface --set-name $mac openpnio-lab --confirm-set
py "$env:USERPROFILE\dcp_client.py" --iface $iface --get-name $mac
```

To **test persistent station name** (only on isolated bench LAN), stop the responder and relaunch it with `--allow-set-name --station-file /var/tmp/openpnio-station-name`; then run:

```powershell
py "$env:USERPROFILE\dcp_client.py" --iface $iface --set-name $mac openpnio-saved --permanent-name --confirm-set
py "$env:USERPROFILE\dcp_client.py" --iface $iface --get-name $mac
```

Restart the responder with the **same** `--station-file` path and issue Get Name once more. It should still return `openpnio-saved`. `--permanent-name` sends the standard BlockQualifier `0x0001`; without it the client explicitly requests **temporary** Set (`0x0000`). **Do not confuse a successful RAM-only Set with persistent configuration.** This experiment has not yet been run on the user's hardware.

`--set-ip MAC IP MASK GATEWAY --confirm-set` exists for testing DCP Set status codes. It cannot reconfigure the Raspberry Pi interface. The CLI requires explicit `--confirm-set` for every Set operation.

On Linux you may install Scapy with your distro package manager or a virtual environment. No packet-capture drivers are needed beyond Linux raw-socket access.

## Observe packets on Pi

```sh
sudo tcpdump -i eth0 -e -nn 'ether proto 0x8892'
```

The `0x8892` EtherType may be shown as `Unknown` by tcpdump; this does not indicate a protocol failure.

## Limits

- Initial implementation supports only untagged outgoing replies. VLAN-tagged reception depends on Ethernet/socket handling and is not verified.
- Supported DCP Get fields: MAC address (01/01), IP suite (01/02), Type (02/01), Name (02/02), DeviceID (02/03), Role (02/04), DeviceOptions (02/05) and DeviceInstance (02/07). Other selectors receive no reply.
- Identify emits 7 corresponding metadata blocks (excluding the redundant Get-MAC block) and supports filter selectors by Name, Type and DeviceID plus FF/FF Identify All. VendorID/DeviceID remain clearly identified lab placeholders; this does not establish certified PROFINET identity.
- DCP response-delay scheduling, multiple network-interface lifecycles, GSDML and PROFINET IO-Controller association are not implemented.
- A device's PROFINET station name is **not** its Linux hostname.
- No PLC interoperability or PROFINET conformance has been established.
