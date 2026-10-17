# Roadmap — first OpenPNIO IO Device connection to Siemens S7-1500

**Status: DCP physically verified in Siemens TIA Portal V19; low-level LLDP/RPC/RTC1 codecs and offline simulated watchdog implemented, but the actual PROFINET IO connection stack remains unfinished (2026-10-17).**

Target: Siemens **S7-1500 CPU 1511T-1 PN**, initially a **fixed 8DI + 8DO** lab model. Raspberry Pi 1 ARMv6 is the reference Linux platform for interoperability experiments; subsequent STM32 adaptation is a separate goal. First test uses virtual/process-buffer digital I/O rather than unsafe or electrically unprotected field circuits.

> **Finished:** DCP Identify, Get IP, Get Name, Set Name in RAM, readback, safe refusal of changed IP (plus unchanged-IP verification) were tested end-to-end over actual Ethernet. Siemens TIA V19 subsequently discovered the Pi and performed **Permanent Set Name `testnode`**, recovered across a program restart and rediscovered by Siemens. See [TIA evidence](siemens-tia-v19-dcp-validation-2026-10-17.md). **Not finished:** PLC CM/AR, cyclic I/O, PI conformance or timing guarantees.

## Gate 0 — Prepare device configuration / safe test setup

- [ ] Reserve a dedicated isolated Ethernet segment for controller and Pi, a consistent station name/IP plan, Wireshark/tcpdump traces, and out-of-band recovery (UART or a separate management NIC) before allowing IP reconfiguration.
- [ ] Fix a lab-only DAP/device identity and module/submodule catalog. VendorID/DeviceID remain placeholders **0x0000/0x0000** and need an appropriate, honest identity policy; do not use a real manufacturer's VendorID without authorization.
- [x] Define fixed laboratory process image: one byte of DI, one byte of DO, explicit DAP/PDev interface/port and I/O module descriptors, safe DO=0 on disconnect. [ ] Validate exact submodule identifiers against adopted GSDML schema and CM requests.
- [x] Generate a **preview GSDML XML** from the manifest, with structurally checked module identifiers and 8DI/8DO data sizes. [ ] Obtain legitimately assigned PI VendorID, validate official V2.32 XSD, confirm GSDML import in TIA Portal and match a runtime identity. Preview uses unassigned 0x0000/0x0000 and must not be imported as a production device.
- [x] Implement qualifier-aware temporary/permanent station-name Set, DeviceOptions, DeviceInstance, correct DeviceRole/IPBlockInfo and additional Identify filters; [x] verify real TIA V19 permanent Set Name and Pi process restart/re-discovery. [ ] Implement factory reset and remaining required DCP options with controlled test coverage. Keep the current safe IP lock until the explicit opt-in/backend and recovery tests in [ADR 0008](decisions/0008-dcp-ip-assignment-policy.md).
- [x] **TIA Portal smoke test**: Accessible devices detects the Pi and TIA can assign and read back a persistent PROFINET station name. **Not** a PLC IO connection test.

## Gate 1 — Portable process image / IO model

- [x] Introduce an initial platform-neutral model with a provisional DAP, slot/subslot descriptors, module/submodule identifiers and process byte lengths. See [virtual IO model](virtual-io-model.md). **IDs and DAP layout still need standards/GSDML validation.**
- [x] Implement fixed **8DI (1 byte)** + **8DO (1 byte)** synthetic in-memory IO, independent of GPIO pinout (no RTC1 integration yet).
- [ ] Explicit ownership and validation of configured modules/submodules and their expected data lengths; reject an unsupported projected configuration with a useful protocol status.
- [x] Initialize virtual DO to zero and clear it on explicit transition to non-operational state. [ ] Wire that transition to future CM/AR watchdog, controller STOP and RT provider-status semantics.

## Gate 2 — Controller connection (acyclic communications)

- [x] Portable defensive **DCE/RPC v4 connectionless UDP header parser** with NDR endian handling; [ ] implement real Linux UDP transport and PNIO endpoint registration, request/reply correlation, fragmentation, RPC interface UUID validation and actual CM dispatch (port **34964** to be verified against real captures).
- [ ] PROFINET IO **Context Manager (CM)**: Connect/Release, AR/CR state, IOCR FrameIDs, expected modules/submodules and requested cycle constraints.
- [ ] Parameterization: necessary Read/Write, control sequences (`DControl`, `CControl` / application-ready), I&M0/required identification records, positive/negative error responses, and AR cleanup.
- [ ] End-to-end integration tests replaying captured valid and invalid exchanges with a deterministic synthetic IO model.
- [ ] Acceptance gate: controller establishes an AR; device reports consistent module/submodule layout and reaches application-ready without duplicate IDs/invalid transitions.

## Gate 3 — Cyclic PROFINET RT Class 1

- [x] Portable **RTC1 generic Ethernet/VLAN0 frame and APDU status codec**; [ ] implement negotiated IOCR C_SDU object offsets, live L2 TX/RX and FrameID handling, provider/consumer scheduling, data validity and priority when applicable.
- [ ] State-machine cycle timing and watchdog for provider/consumer; update virtual DI, consume DO, and expose stable IOPS/IOCS.
- [x] Add a portable **simulated AR watchdog and safe-output state machine** with tests; [ ] connect to real CM/AR state and controller STOP/RT frames so the fail-safe transition is triggered by hardware.
- [ ] Capture and measure actual cycle interval, jitter, missed frames and host CPU load on Pi 1; **do not claim deterministic RT or IRT** from stock Linux.
- [ ] Acceptance gate: S7-1500 input byte reflects synthetic PI data; S7-1500 output byte reaches the process buffer for a sustained test period; recovery after stop/restart and cable pull is verified.

## Gate 4 — Commissioning and maintainability

- [x] Implement an IEEE **LLDP core frame codec and optional basic Linux live transmitter**; [ ] add PROFINET organization-specific TLVs, topology/neighbor management and real Wireshark/TIA/PLC validation; verify required options against target CC-A profile.
- [ ] Diagnostics, alarms, IO module validation, acyclic record access, physical IO board or GPIO driver, persistence/reset behavior, and automated regression captures.
- [ ] Split core (MIT/C11), Linux BSP and sensor drivers; later move to STM32/Orange Pi without copying incompatible licensed source.
- [ ] Review protocol specification, safety, compliance and PNO/PI vendor identity requirements before any product claims.

## Testing strategy

Do not move the first PLC test to a production machine. First check simple TIA discovery, then controlled commissioning in an isolated lab, then RPC/AR, then cyclic bytes and failure recovery. Save simultaneous captures from Pi and controller link, including source/destination MAC and UDP packets, plus CPU diagnostics. Prefer [consolidated batch test procedure](siemens-batch-test-plan.md), using tcpdump capture filter `not tcp port 22` on Pi to avoid SSH flood.

## Explicit nonclaims

The DCP milestone does **not** mean OpenPNIO can already be inserted into the Siemens hardware catalog, create an Application Relation, drive outputs, or implement a full IEC 61158 PROFINET IO stack. Future work must be validated against a **real Siemens S7-1500**.
