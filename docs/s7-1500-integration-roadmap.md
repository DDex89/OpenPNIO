# Roadmap — first OpenPNIO IO Device connection to Siemens S7-1500

**Status: Planning / implementation not started for the PROFINET IO connection stack (2026-10-17).**

Target: Siemens **S7-1500 CPU 1511T-1 PN**, initially a **fixed 8DI + 8DO** lab model. Raspberry Pi 1 ARMv6 is the reference Linux platform for interoperability experiments; subsequent STM32 adaptation is a separate goal. First test uses virtual/process-buffer digital I/O rather than unsafe or electrically unprotected field circuits.

> **Finished:** DCP Identify, Get IP, Get Name, Set Name in RAM, readback, safe refusal of changed IP (plus unchanged-IP verification) were tested end-to-end over actual Ethernet. See [DCP hardware validation](hardware-validation-2026-10-17.md). **Not finished:** PLC connection, cyclic I/O, conformance or timing guarantees.

## Gate 0 — Prepare device configuration / safe test setup

- [ ] Reserve a dedicated isolated Ethernet segment for controller and Pi, a consistent station name/IP plan, Wireshark/tcpdump traces, and out-of-band recovery (UART or a separate management NIC) before allowing IP reconfiguration.
- [ ] Fix a lab-only DAP/device identity and module/submodule catalog. VendorID/DeviceID remain placeholders **0x0000/0x0000** and need an appropriate, honest identity policy; do not use a real manufacturer's VendorID without authorization.
- [ ] Define fixed application process image: one byte of DI, one byte of DO, explicit slot/subslot and module IDs, and a known safe DO state when disconnected.
- [ ] Produce a **minimal, internally consistent GSDML** describing that fixed configuration. TIA Portal acceptance of the XML, hardware catalog entry and actual runtime identity must match.
- [ ] Harden DCP: persistence and BlockQualifier semantics for station name; required DCP metadata and selectors (e.g. DeviceOptions) and any station reset/signal needed for the particular commissioning path. Keep the current safe IP lock until the explicit opt-in/backend and recovery tests in [ADR 0008](decisions/0008-dcp-ip-assignment-policy.md).
- [ ] **Early PLC smoke test**: check whether TIA Portal's *Accessible devices* sees the Pi and whether it can assign a station name. This is an exploratory compatibility test, **not** evidence of IO cyclic communication.

## Gate 1 — Portable process image / IO model

- [ ] Create platform-neutral `openpnio_device_model`: DeviceAccessPoint, slot/subslot descriptors, module/submodule identifiers, input/output lengths, and buffer callbacks.
- [ ] Implement fixed **8DI (1 byte)** + **8DO (1 byte)** synthetic IO, independent of GPIO pinout.
- [ ] Explicit ownership and validation of configured modules/submodules and their expected data lengths; reject an unsupported projected configuration with a useful protocol status.
- [ ] Define valid/invalid IO provider state; default outputs off on startup and controller loss.

## Gate 2 — Controller connection (acyclic communications)

- [ ] Linux UDP transport and platform-neutral **connectionless DCE/RPC/NDR** framing, endpoint handling and request/reply correlation (PROFINET RPC, commonly UDP port **34964**, verify exact stack/endpoint details against specification and real captures).
- [ ] PROFINET IO **Context Manager (CM)**: Connect/Release, AR/CR state, IOCR FrameIDs, expected modules/submodules and requested cycle constraints.
- [ ] Parameterization: necessary Read/Write, control sequences (`DControl`, `CControl` / application-ready), I&M0/required identification records, positive/negative error responses, and AR cleanup.
- [ ] End-to-end integration tests replaying captured valid and invalid exchanges with a deterministic synthetic IO model.
- [ ] Acceptance gate: controller establishes an AR; device reports consistent module/submodule layout and reaches application-ready without duplicate IDs/invalid transitions.

## Gate 3 — Cyclic PROFINET RT Class 1

- [ ] L2 send/receive of negotiated RTC1 IOCR Ethernet frames with proper FrameID, cycle counter, DataStatus and TransferStatus, process bytes and appropriate VLAN priority when applicable.
- [ ] State-machine cycle timing and watchdog for provider/consumer; update virtual DI, consume DO, and expose stable IOPS/IOCS.
- [ ] Define behavior when frames are lost or controller stops: timeout, AR cleanup and **safe DO state** (for demo, 0x00).
- [ ] Capture and measure actual cycle interval, jitter, missed frames and host CPU load on Pi 1; **do not claim deterministic RT or IRT** from stock Linux.
- [ ] Acceptance gate: S7-1500 input byte reflects synthetic PI data; S7-1500 output byte reaches the process buffer for a sustained test period; recovery after stop/restart and cable pull is verified.

## Gate 4 — Commissioning and maintainability

- [ ] LLDP and topology/port identity as needed for correct TIA diagnostics and the target conformance class. Identify the minimum interoperability requirements by measuring PLC behavior, not guessing.
- [ ] Diagnostics, alarms, IO module validation, acyclic record access, physical IO board or GPIO driver, persistence/reset behavior, and automated regression captures.
- [ ] Split core (MIT/C11), Linux BSP and sensor drivers; later move to STM32/Orange Pi without copying incompatible licensed source.
- [ ] Review protocol specification, safety, compliance and PNO/PI vendor identity requirements before any product claims.

## Testing strategy

Do not move the first PLC test to a production machine. First check simple TIA discovery, then controlled commissioning in an isolated lab, then RPC/AR, then cyclic bytes and failure recovery. Save simultaneous captures from Pi and controller link, including source/destination MAC and UDP packets, plus CPU diagnostics.

## Explicit nonclaims

The DCP milestone does **not** mean OpenPNIO can already be inserted into the Siemens hardware catalog, create an Application Relation, drive outputs, or implement a full IEC 61158 PROFINET IO stack. Future work must be validated against a **real Siemens S7-1500**.
