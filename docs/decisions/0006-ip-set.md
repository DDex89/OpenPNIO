# ADR-0006: IP suite assignment via platform callback

Status: experimental.

The DCP Set handler now supports IPParameter (option 1/suboption 2), decoding a 2-byte qualifier and 12-byte IPv4 address/mask/gateway tuple. An application-provided callback decides whether the platform can apply or persist the settings. The existing station-name-only API remains available as a compatibility wrapper.

The protocol layer deliberately does not change operating-system network settings. Linux and MCU ports must implement their own policy, validation, persistence and recovery. Callback failure produces a SetNotPossible response.

Caveats: IP address/netmask/gateway semantic validation is not yet performed in the core; it must not be exposed to an untrusted network before a platform policy is implemented. Multi-block changes are not transactional. Live Siemens/TIA interoperability remains untested.

References: https://github.com/core-engineering/pnio-rs/blob/main/crates/pnio/src/dcp/set.rs and https://github.com/rtlabs-com/p-net/blob/public/src/device/pf_cmina.c . Implementation is original.
