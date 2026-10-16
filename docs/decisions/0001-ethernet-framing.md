# ADR-0001: Portable Ethernet frame parsing

Status: accepted for initial experimental implementation.

## Observation
PROFINET RT and DCP use Ethernet EtherType 0x8892, with a two-byte FrameID following the Ethernet header. An IEEE 802.1Q tag can precede the EtherType. Raw L2 access is therefore needed on the eventual platform.

## Decision
Implement a small, allocation-free C11 parser independent of sockets, OS, and hardware. Accept untagged and single-tagged 802.1Q frames; explicitly reject stacked/802.1ad tags. The caller owns input storage and the returned payload pointer remains valid only while that storage remains valid.

## Provenance
Behavior was checked against the publicly documented PROFINET framing and reviewed in context of:
- https://github.com/core-engineering/pnio-rs (eth/header.rs, eth/transport.rs)
- https://github.com/rtlabs-com/p-net (src/common/pf_eth.c and src/common/pf_dcp.c)

This module is an original implementation, not a translation of either codebase.

## Limitations
This is framing only, not DCP decoding, AR setup, cyclic data exchange, Ethernet FCS handling, or PROFINET certification. The Ethernet FCS is normally stripped by NIC hardware and must not be passed to this parser as payload.
