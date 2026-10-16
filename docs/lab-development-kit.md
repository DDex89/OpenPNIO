# OpenPNIO Lab Development Kit — product direction

Status: **design proposal**, not a tested product specification.

## Purpose

An affordable, hackable PROFINET IO Device **learning and sensor-integration bench** for Siemens S7-1500 / TIA Portal. Industrial field deployment is out of scope for the first release. A commercial kit is a possible later outcome, not a current claim.

## Architectural boundaries

1. **PROFINET core (portable C11):** Ethernet frame handling, DCP, RPC/CM, AR, RTC1, watchdog, diagnosis. No Linux GPIO, MCU HAL, sensor driver or persistent storage calls inside the protocol core.
2. **Device model:** modules/submodules, slot/subslot identity, input/output byte lengths, process-image buffers, status and diagnostic events. Configuration must eventually be validated against the controller's expected module layout.
3. **Board support package:** Ethernet transport, monotonic clock, persistent identity/configuration, GPIO/ADC/DAC access, safe-output control.
4. **Sensor and expansion drivers:** digital I/O, analog front ends, PT100/RTD, NTC, I2C, SPI and UART. Drivers expose values through the device model rather than changing the network stack.
5. **Description and tooling:** GSDML generated or maintained in lockstep with the supported module catalog; example TIA Portal project and Wireshark capture instructions.

## First milestone (MVP)

- One **fixed 8DI + 8DO** module, with a defined byte-oriented process image.
- Raspberry Pi 1/Linux for protocol bring-up; STM32 after behavior is validated.
- On a safe bench, software-generated DI values and LED/debug DO outputs are sufficient. No 24 V hardware is required for the initial protocol test.
- DCP Identify/Set, RPC/CM AR establishment, RTC1 cyclic exchange, watchdog and minimal GSDML must work together before claiming S7-1500 interoperability.
- Defined fail-safe DO behavior when the controller stops or AR expires.

## Expansion candidates (later)

| Module type | Typical interface | Note |
| --- | --- | --- |
| 8DI / 8DO | GPIO; optoisolated 24 V daughterboard | 24 V circuits remain electrically separate from logic |
| 4AI / 2AO | ADC/DAC + protected signal conditioning | 0–10 V and 4–20 mA need dedicated front ends |
| 4RTD | RTD converter / precision analog front end | PT100 2/3/4-wire modes need explicit support |
| Environmental sensors | I2C / SPI | Expose calibrated engineering units |
| Serial bridge | UART / RS-485 | Requires a documented cyclic or acyclic data mapping |

The expansion connector pinout, power budget, voltage levels, protection, GSDML module identifiers and pricing are **not fixed yet**.

## Development-kit release gates

- Reproducible BOM, schematics, firmware source, build/test instructions and documented connector pinout.
- Test fixture and automated protocol regression tests.
- Isolation, EMC, power and electrical safety design appropriate to the intended market.
- Review applicable PROFINET trademark, conformance/certification and market-access requirements before product claims or sale.
- Hardware design license to be chosen separately; MIT currently covers original software only.

## Non-goals

Not a substitute for safety-rated I/O, certified industrial equipment or a production controller. Do not connect prototype outputs directly to actuators in a live machine.
