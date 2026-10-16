# Architecture (draft)

OpenPNIO targets a hardware-independent PROFINET IO Device implementation.

## Layers
- Protocol core: DCP, RPC/CM, AR, cyclic RT IO, diagnostics, alarms.
- Platform API: Ethernet frame RX/TX, monotonic time, storage, locking.
- IO model: slots, subslots, input/output process data, safe output states.
- Adapters: Linux first; STM32 and RTOS later.
- Tools: Python GSDML generation and packet-level conformance tests.

The protocol core must not depend on Linux, STM32 HAL, or a particular RTOS.

## First milestones
1. Source-level audit and license inventory of reference implementations.
2. Define testable behavior and platform-neutral interfaces.
3. Implement frame parsing and unit tests without network privileges.
4. Add Linux Ethernet adapter and virtual IO.
5. Validate DCP discovery, AR establishment, cyclic IO and fault recovery against Siemens S7-1500.

No PROFINET compatibility or real-time guarantees are claimed yet.
