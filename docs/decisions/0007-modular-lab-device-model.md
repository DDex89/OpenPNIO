# ADR 0007 — Modular laboratory device model

Status: **Accepted for design**, 2026-10-16.

## Context

The initial target is a fixed 8DI/8DO experimental PROFINET IO Device. The primary user need is an inexpensive laboratory learning platform for Siemens S7-1500, sensors and peripheral experiments. Future sale as a development kit is optional.

## Decision

Keep the **first interoperable configuration fixed at 8DI/8DO** while designing an internal module/submodule abstraction that can later expose additional analog, RTD and serial modules. Separate protocol core, device model, platform BSP and peripheral drivers. Device-specific capabilities must not be hard-coded into DCP/CM/RTC1 logic. Keep GSDML consistent with the actual device catalog.

The initial Raspberry Pi 1 build remains a protocol and network test platform, not the final hardware design. Preserve MIT for original software and do not import incompatible third-party source.

## Consequences

Positive: one protocol implementation supports many sensor boards and platforms; bench hardware can be inexpensive; optional productization remains possible.

Tradeoffs: modularity adds configuration validation, slot/subslot identity and GSDML maintenance work. These are not reasons to delay basic Siemens interoperability.

## Validation

First verify fixed 8DI/8DO cyclic data with S7-1500 and explicit output-safe-state behavior. Only then add dynamically selectable module catalogs and hardware daughterboards.
