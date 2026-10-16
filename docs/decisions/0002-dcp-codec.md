# ADR-0002: DCP wire decoder before commissioning logic

Status: initial implementation.

DCP header fields are parsed independently of transport and device configuration. The decoder validates the 10-byte header, service ID/type, XID, and declared block-data length. The TLV cursor checks 4-byte block headers and even-byte padding without allocation. BlockInfo and BlockQualifier remain part of the block data and must be interpreted by the relevant service handler.

Source observations:
- https://github.com/core-engineering/pnio-rs/blob/main/crates/pnio/src/dcp/block.rs
- https://github.com/core-engineering/pnio-rs/blob/main/crates/pnio/src/dcp/set.rs
- https://github.com/rtlabs-com/p-net/blob/public/src/device/pf_cmina.c

The pnio-rs DCP Set implementation does not apply changes; P-Net's CM-Ina component explicitly handles station name and IP assignment, including persistent storage. Our commissioning layer will therefore need distinct validation, apply, persistence and reply stages.

No upstream source code was copied into the implementation. Decoder is not yet a functional DCP responder.
