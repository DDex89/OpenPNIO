# ADR-0005: DCP Set station-name prototype

Status: experimental; **not yet validated against real controller traffic**.

Implements DCP Set (FrameID FEFD, service 4, request type 0) addressed to the device's unicast MAC. A station-name block (option 2, suboption 2) contains a two-byte qualifier and name. The handler validates basic lowercase ASCII station-name syntax, invokes an application-supplied setter, and emits a Control/Response block (option 5, suboption 4) with per-block status. It checks all TLV bounds and output capacity before invoking callbacks.

Source-level design review:
- https://github.com/core-engineering/pnio-rs/blob/main/crates/pnio/src/dcp/set.rs
- https://github.com/rtlabs-com/p-net/blob/public/src/device/pf_cmina.c

The source is original and does not incorporate GPL code.

Important limitations:
- The setter must apply and/or persist the new name; this module itself provides no persistence, rollback or reboot handling.
- IP-suite Set, reset, factory settings, DCP Set access control and controller ownership rules remain unimplemented.
- Validation is intentionally narrower than full station-name rules; no claim of full spec compliance.
- Multiple Set blocks are not atomic across callbacks. Platform integration must address transactional behavior.
- Do not deploy on a production network.
