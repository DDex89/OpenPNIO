# Reference implementation audit

Status: **initial inventory only; source-code audit pending**.

| Project | Repository | License / notes |
| --- | --- | --- |
| P-Net | https://github.com/rtlabs-com/p-net | GPLv3 or commercial licensing; do not copy into MIT-only core |
| p-net-stm32 | https://github.com/andre-lost-a-pig/p-net-stm32 | STM32H723 example based on P-Net; inspect full dependency and license tree |
| pnio-rs | https://github.com/core-engineering/pnio-rs | MIT OR Apache-2.0; check files and dependency notices |
| profi++ | https://github.com/langmo/profipp | GPLv3; reference only unless licensing requirements satisfied |

## Audit checklist
- DCP identify/set and station naming
- Ethernet framing, VLAN and EtherType dispatch
- RPC endpoint mapper, connect, release and parameterization
- AR state machine and watchdog
- Cyclic RT class 1 frames, timing, data status
- IO model, slots/subslots, IOPS/IOCS
- Diagnostics, alarms, I&M
- GSDML mapping and TIA Portal integration
- Portability, memory model, test coverage and license provenance

Document observed behavior separately from implementation details. No external source files have been copied into this repository.
