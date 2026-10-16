# ADR-0004: Conservative Identify request dispatch

Status: experimental.

The dispatcher accepts Ethernet DCP Identify requests (FrameID 0xFEFE, service 5, request type 0) addressed to the DCP multicast MAC or the device's own MAC. It supports an empty selector list (Identify All) or an exact NameOfStation selector (option 2/suboption 2). Unrecognized selectors are not treated as matches. Malformed TLVs are rejected. Responses preserve the request XID and address the original source MAC.

Source review: https://github.com/core-engineering/pnio-rs/tree/main/crates/pnio/src/dcp and https://github.com/rtlabs-com/p-net/blob/public/src/common/pf_dcp.c . The implementation is original, not a copy.

Outstanding: spec-accurate support for further selectors, DCP response delay scheduling, validation of station names, identity metadata, VLAN policies, and live packet-capture comparison. Do not claim TIA Portal discovery compatibility until tested.
