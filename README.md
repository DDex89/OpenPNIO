# OpenPNIO

**Experimental, hardware-independent PROFINET IO Device project.**

OpenPNIO aims to implement a portable PROFINET IO Device protocol core with Linux and embedded-platform adapters. Initial development focuses on protocol analysis, reproducible tests and interoperability with Siemens S7-1500 controllers.

> **Status:** early research and scaffolding. This repository does **not** yet contain a functional PROFINET IO Device stack. No interoperability, real-time performance or certification is claimed.

## Repository layout

- [Architecture](docs/architecture.md) — intended layers and development milestones.
- [Reference audit](docs/reference-projects.md) — candidate upstream projects and source-review checklist.
- `include/openpnio/platform.h` — preliminary C11 platform boundary.
- `include/openpnio/version.h` — development version.
- `CMakeLists.txt` — initial portable build configuration.

## Target platforms

The protocol core is intended to remain independent of hardware and operating system. Linux is the initial test target; STM32/RTOS ports will follow. Raspberry Pi 1 and Orange Pi 3 are potential Linux test hosts, subject to network and timing constraints.

## Licensing and attribution

Original OpenPNIO code is licensed under [MIT](LICENSE). Upstream projects are cited in [docs/reference-projects.md](docs/reference-projects.md). GPL-licensed implementation code must not be incorporated into this MIT-only codebase without an appropriate licensing decision. Source review is not a claim of clean-room independence.

## Next step

Audit source trees and test suites of the candidate implementations before choosing the protocol architecture. Then introduce packet parsers, tests and a Linux adapter incrementally.
