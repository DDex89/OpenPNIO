# ADR-0003: First DCP Identify response encoder

Status: experimental; interoperability unverified.

A portable, allocation-free encoder generates an untagged Ethernet DCP Identify response with a caller-supplied MAC address, XID, station name/type, vendor/device IDs, device role and IPv4 suite. A separate unit test decodes its output through our independent Ethernet and DCP parsers.

Sources studied:
- https://github.com/core-engineering/pnio-rs/blob/main/crates/pnio/src/dcp/frame.rs
- https://github.com/core-engineering/pnio-rs/blob/main/crates/pnio/src/dcp/block.rs
- https://github.com/rtlabs-com/p-net/blob/public/src/common/pf_dcp.c

This is original code. No upstream source has been copied.

Limitations: this encoder is not connected to a live Ethernet transport. DCP Identify request matching, timing/jitter, DeviceOptions and complete identity requirements still need review against official PROFINET specifications and real TIA Portal traffic. Vendor/device IDs used in unit tests are placeholders, not assigned product identifiers. No device should advertise them on a production network.
