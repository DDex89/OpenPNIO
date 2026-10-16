# OpenPNIO virtual 8DI / 8DO process model

**Status:** portable C11 core implemented and tested with CTest. **No PROFINET IO cyclic exchange or GSDML is implemented yet.**

This intentionally small lab model is a step between hardware-validated DCP and a Siemens S7-1500 IO-Controller connection. The first application data is **simulated in memory**: no Raspberry Pi GPIO, 24 V I/O, relay or actuator is attached to these bytes.

## Fixed initial module catalog (provisional)

| Slot | Subslot | ModuleIdent | SubmoduleIdent | Input bytes | Output bytes |
| ---: | ---: | ---: | ---: | ---: | ---: |
| 0 (DAP placeholder) | 1 | `0x00000001` | `0x00000001` | 0 | 0 |
| 1 (8DI) | 1 | `0x00001001` | `0x00001001` | 1 | 0 |
| 2 (8DO) | 1 | `0x00001002` | `0x00001002` | 0 | 1 |

**Important:** These IDs are original provisional *internal laboratory values*, **not** registered PI VendorIDs/DeviceIDs, and **not** a certified GSDML. Actual DAP/slot/subslot layout must be reviewed against the applicable PNIO/GSDML requirements before generating an importable description or claiming Siemens compatibility. Port subslots, I&M records and topology descriptions are not implemented.

## Behavior and API

Header: `include/openpnio/device_model.h`, implementation: `src/device_model.c`.

- `openpnio_lab_submodule_count()`, `openpnio_lab_submodule_at()` and `openpnio_lab_find_submodule()` expose deterministic metadata.
- `openpnio_device_model_set_di()` simulates the **8 DI bits** (one byte). `openpnio_device_model_read_di()` requires slot 1/subslot 1 and exactly one byte.
- `openpnio_device_model_write_do()` consumes **one byte** of virtual DO data only for slot 2/subslot 1 and only when the process model is explicitly marked operational.
- `openpnio_device_model_init()` and every transition to `operational=false` reset DO to **0x00**.
- Illegal slot/subslot, incorrect payload lengths and attempts to write outputs while disconnected return a distinct error, without overwriting DO.
- The state is single-threaded. A future CM/AR/RTC1 integration must provide synchronization, legal connection state transitions, proper IOPS/IOCS and its own watchdog deadline; setting `operational=true` manually for a unit test is not an AR implementation.

## Run unit tests

```sh
cmake -S . -B build
cmake --build build -j1
ctest --test-dir build --output-on-failure
```

CTest entry `virtual_8di_8do` validates the catalog, byte sizes, slot/subslot checks, read/write directions, disconnected-write refusal and fail-safe output reset on controller loss.

## Path to a real controller

1. Finalize lab device identity and DAP/module catalog; implement a minimal compatible GSDML and validate the same IDs in the runtime.
2. Add acyclic UDP/DCE-RPC/CM handlers and Application Relation (AR) state machine.
3. Feed cyclic consumer data (controller -> virtual DO) and publish provider data (virtual DI -> controller) at negotiated IOCR frame IDs and timing.
4. Tie the watchdog/AR teardown and S7 STOP states to `openpnio_device_model_set_operational(...,0)`; verify real device diagnostics and IO state.

See [S7-1500 integration roadmap](s7-1500-integration-roadmap.md) and [GitHub issue #4](https://github.com/DDex89/OpenPNIO/issues/4).
