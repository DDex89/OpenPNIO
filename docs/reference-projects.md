# Reference implementation audit

Status: **source-level inspection in progress**, not a complete audit.

## Repositories and licensing

| Project | Repository | Initial findings |
| --- | --- | --- |
| P-Net | https://github.com/rtlabs-com/p-net | Public branch is an evaluation snapshot; lacks platform ports/build entrypoint. GPLv3/commercial licensing. |
| p-net-stm32 | https://github.com/andre-lost-a-pig/p-net-stm32 | STM32H723-oriented firmware tree with large vendor HAL/CMSIS/FreeRTOS dependencies. License provenance needs per-component review. |
| pnio-rs | https://github.com/core-engineering/pnio-rs | MIT OR Apache-2.0. Standalone Rust implementation with Linux-specific raw Ethernet transport and RTC1 runner. |
| profi++ | https://github.com/langmo/profipp | GPLv3 wrapper/project tree with bundled P-Net and OSAL code; contains STM32F769 FreeRTOS platform material. |

## Findings from actual source inspection

### pnio-rs
- `crates/pnio/src/cm/ar.rs` models AR as a pure event/action state machine with explicit Idle, Connected, AppReadySent, Data phases; timeout and watchdog abort reasons are distinct.
- `crates/pnio/src/eth/transport.rs` abstracts complete Layer-2 frames behind a transport trait and supplies a mock transport for tests.
- `crates/pnio/src/rt/mod.rs` separates frame codec, IO layout, engine and Linux-only runner/scheduler.
- **Gap:** `crates/pnio/src/dcp/set.rs` refuses IP-suite changes unless they already match configured values; other options, including station-name setting, are not applied. This is relevant to TIA Portal commissioning and must be addressed in OpenPNIO.

### P-Net
- `src/common/pf_dcp.c` implements L2 DCP services including Get, Set, Identify, Hello; comments describe delayed Identify replies and client hold-off timing.
- Public branch `public` has protocol source and tests but is not a standalone buildable port. The README on branch `public` explicitly labels it an evaluation version.
- Do not transfer GPL source into MIT-only modules.

### profi++ / STM32 port
- Repository tree inspected, but code-level feature and dependency comparison is pending. No completeness claim.

## Next audit items
1. Compare DCP Set station name persistence and IP commissioning.
2. Compare Connect/PrmEnd/ApplicationReady, AR reconnect and watchdog.
3. Inspect RTC1 frame layout, IOPS/IOCS and VLAN handling.
4. Inspect GSDML generators, I&M, alarm handling, unit tests and captured fixtures.
5. Record exact upstream commit SHAs for reproducible review.

## Method
Document protocol behavior, evidence and engineering decisions. Independently implement original modules with tests. Do not copy GPL-licensed implementation code into the MIT-only core.
