# ADR 0008 — Guarded DCP IP assignment on Linux bench adapters

**Status: Proposed (2026-10-17).** No IP-reconfiguration implementation or runtime opt-in exists yet.

## Context

OpenPNIO's Pi 1 `eth0` currently serves **both** the experimental PROFINET endpoint and the SSH management session. The DCP Set IP core already exposes a callback, but the Linux bench callback only accepts an *exact match* with current IP, mask and gateway as a no-op. Any change returns DCP Set `Not possible` (code 5).

On 2026-10-17, the user verified a request to change `192.168.1.8` to `192.168.1.99` was refused and subsequent Get IP still returned the original address. This is the correct **current safety default**, but **not** sufficient for normal PROFINET commissioning: an IO Controller may assign the configured IP to an IO Device when it starts. Permanent refusal can therefore block automatic commissioning.

## Proposed policy

The portable C11 DCP core continues to **parse and validate** Set, but does **not** make OS/network changes. A Linux-specific backend implements three opt-in, explicit policies:

| Mode | Action | Default |
| --- | --- | --- |
| `locked` | Only an identical IP/mask/gateway request returns OK (no-op). A change returns `Set not possible`. | **Yes** |
| `temporary` | Apply validated changes only for the active session, with recovery/rollback safeguards and a separate management strategy. | No |
| `persistent` | Permit validated changes and explicitly persist them according to confirmed DCP BlockQualifier and the target network manager, with atomic configuration handling. | No |

**Temporary/persistent are design goals only.** Do not expose an `--allow-set-ip` flag until there is a real backend, tests, and a credible recovery method.

### Preconditions before enabling actual IP changes

1. Test on an **isolated laboratory PROFINET segment**, with **UART/serial console or an independent management interface** if changing the active management NIC. A single SSH session over the target eth0 is **not** a recovery channel. For Raspberry Pi 1, a USB-Ethernet adapter can be used for a separate management network, subject to interface availability.
2. Parse and validate IP, prefix/netmask, gateway, suboption, DCP qualifier, conflicting/duplicate IPv4 assignments, reserved/multicast/broadcast and zero-address handling. Define policy on unset IP and conflicts (ARP probe) before implementation.
3. Serialize Set operations with current device identity state and network updates; handle multi-block Set requests and partial failures deliberately. Do not reply with a false success when a requested new IP has not been applied.
4. Use a platform-specific network backend (Linux netlink / configured network manager), **not** shell commands assembled from untrusted packet content. Route and prefix updates should be coordinated.
5. For `temporary`, arrange an independently controlled rollback path, with an explicit confirmation method. A timer alone cannot guarantee recovery if the networking process or device crashes.
6. For `persistent`, preserve the configuration consistently across boots, implement qualifier semantics based on actual PROFINET specification, and exercise reset-to-factory behavior later.
7. Document whether an in-progress AR permits address changes and how disconnection is handled.

### Initial acceptance tests

- On the existing Pi 1 + SSH setup, default `locked` still refuses an unauthorized IP change, reports code 5, and retains 192.168.1.8.
- A repeated identical IP suite may return code 0 without changing Linux.
- Under explicitly enabled `temporary` mode **with out-of-band recovery**, a new IP can be assigned, DCP Get reflects the new IP, and the host can reach the new address; after rollback or restart, the original configuration is restored.
- Invalid/conflicting requests are refused without side effects; malformed/multi-block inputs cannot partially apply a change or lead to false success.
- `persistent` is exercised independently after `temporary`, including power cycle, qualifier handling, and recovery.

## Why not unlock immediately?

DCP runs at Layer 2 and is not an authenticated remote-management API. Unrestricted address reassignment over a shared Ethernet segment can disconnect the Pi and any active debugging session, even when the implementation is otherwise correct.

## Review gate

Before Siemens PLC commissioning, either:
- assign a matching station name and IP manually on the **isolated lab** and retain `locked` for initial controlled tests, **or**
- enable an explicitly tested temporary policy and let the controller perform its normal address assignment.

Neither path substitutes for GSDML, RPC/CM, AR, and RT cyclic data.
