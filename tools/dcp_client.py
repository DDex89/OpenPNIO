#!/usr/bin/env python3
"""OpenPNIO DCP bench client for Windows/Npcap or Linux/Scapy.

Read-only by default. DCP Set needs --confirm-set and addresses a specific MAC.
Runs at Ethernet layer 2: ordinary IP sockets cannot perform DCP discovery.
"""
import argparse
import ipaddress
import re
import secrets
import struct
import time

DCP_MULTICAST = "01:0e:cf:00:00:00"
ETHERTYPE = 0x8892
SET_ERRORS = {
    0: "OK", 1: "Option unsupported", 2: "Suboption unsupported",
    3: "Suboption not set", 4: "Resource error", 5: "Set not possible",
}


def mac_bytes(value):
    parts = value.split(":")
    if len(parts) != 6 or any(not re.fullmatch(r"[0-9a-fA-F]{2}", p) for p in parts):
        raise ValueError("Expected MAC address aa:bb:cc:dd:ee:ff")
    return bytes(int(p, 16) for p in parts)


def make_dcp(frame_id, service, xid, data):
    if len(data) > 65535:
        raise ValueError("DCP data too large")
    return struct.pack("!HBBIHH", frame_id, service, 0, xid, 0, len(data)) + data


def make_identify(xid):
    return make_dcp(0xFEFE, 5, xid, b"\xff\xff\x00\x00")


def make_get(xid, option, suboption):
    # DCP Get uses option/suboption pairs; these are NOT ordinary Set TLVs.
    return make_dcp(0xFEFD, 3, xid, bytes((option, suboption)))


def make_set(xid, option, suboption, value, qualifier=0):
    body = struct.pack("!H", qualifier) + value
    block = struct.pack("!BBH", option, suboption, len(body)) + body
    if len(body) & 1:
        block += b"\x00"
    return make_dcp(0xFEFD, 4, xid, block)


def parse_blocks(blocks):
    decoded = []
    pos = 0
    while pos < len(blocks):
        if len(blocks) - pos < 4:
            raise ValueError("Truncated DCP block header")
        option, suboption, size = struct.unpack_from("!BBH", blocks, pos)
        start = pos + 4
        end = start + size
        if end > len(blocks) or end + (size & 1) > len(blocks):
            raise ValueError("Malformed DCP response block")
        if (option, suboption) == (5, 4):
            # Set Control/Response has three status bytes, NOT a BlockInfo.
            info, value = None, blocks[start:end]
        else:
            if size < 2:
                raise ValueError("DCP response block lacks BlockInfo")
            info = int.from_bytes(blocks[start:start+2], "big")
            value = blocks[start+2:end]
        decoded.append((option, suboption, info, value)
        pos = end + (size & 1)
    return decoded


def parse_ethernet_response(raw, xid, service, frame_id):
    if len(raw) < 26:
        return None
    ether_type = int.from_bytes(raw[12:14], "big")
    offset = 14
    if ether_type == 0x8100:
        if len(raw) < 30:
            return None
        ether_type = int.from_bytes(raw[16:18], "big")
        offset = 18
    if ether_type != ETHERTYPE or len(raw) < offset + 12:
        return None
    dcp = raw[offset:]
    if (int.from_bytes(dcp[0:2], "big") != frame_id or
        dcp[2] != service or dcp[3] != 1 or
        int.from_bytes(dcp[4:8], "big") != xid):
        return None
    length = int.from_bytes(dcp[10:12], "big")
    if len(dcp) < 12 + length:
        return None
    blocks = parse_blocks(dcp[12:12+length])
    mac = ":".join(f"{b:02x}" for b in raw[6:12])
    return mac, blocks


def find_value(blocks, option, suboption):
    for opt, sub, _info, value in blocks:
        if opt == option and sub == suboption:
            return value
    return None


def ip_text(data):
    return str(ipaddress.IPv4Address(data))


def execute(iface, dst, payload, xid, service, frame_id, timeout):
    # Lazy import: packet builders and parser can be tested without Scapy/Npcap.
    from scapy.all import AsyncSniffer, Ether, Raw, get_if_hwaddr, sendp
    request = Ether(src=get_if_hwaddr(iface), dst=dst, type=ETHERTYPE) / Raw(payload)
    sniffer = AsyncSniffer(iface=iface, filter="ether proto 0x8892", store=True)
    sniffer.start()
    try:
        time.sleep(0.5)
        sendp(request, iface=iface, verbose=False)
        print(f"Sent DCP request: XID 0x{xid:08X}, destination {dst}")
        time.sleep(timeout)
    finally:
        packets = sniffer.stop()
    results = []
    for packet in packets:
        try:
            result = parse_ethernet_response(bytes(packet), xid, service, frame_id)
        except ValueError as exc:
            print(f"Malformed DCP response ignored: {exc}")
            continue
        if result is not None:
            if dst != DCP_MULTICAST and result[0].lower() != dst.lower():
                continue
            results.append(result)
    return results


def validate_station_name(name):
    if (not 1 <= len(name) <= 240 or
        not re.fullmatch(r"[a-z0-9.-]+", name) or
        name.startswith((".", "-")) or name.endswith((".", "-")) or
        ".." in name or ".-" in name or "-." in name):
        raise ValueError("Invalid station name: use lowercase a-z, 0-9, '-' and '.'")


def main():
    p = argparse.ArgumentParser(description="OpenPNIO PROFINET DCP bench client")
    p.add_argument("--list", action="store_true", help="List Scapy/Npcap interfaces")
    p.add_argument("--iface", help="Scapy interface name or Npcap index")
    p.add_argument("--timeout", type=float, default=3.0)
    group = p.add_mutually_exclusive_group()
    group.add_argument("--get-name", metavar="MAC", help="DCP Get station name")
    group.add_argument("--get-ip", metavar="MAC", help="DCP Get IP, mask, gateway")
    group.add_argument("--set-name", nargs=2, metavar=("MAC", "NAME"),
                       help="DCP Set station name (opt-in)")
    group.add_argument("--set-ip", nargs=4, metavar=("MAC", "IP", "MASK", "GATEWAY"),
                       help="DCP Set IP suite (opt-in; the Pi will refuse changes)")
    p.add_argument("--confirm-set", action="store_true",
                   help="Confirm sending a network configuration request")
    args = p.parse_args()
    from scapy.all import show_interfaces
    if args.list:
        show_interfaces()
        return
    if not args.iface:
        p.error("Provide --iface or use --list")
    if args.timeout < 0.1 or args.timeout > 30:
        p.error("--timeout must be between 0.1 and 30 seconds")
    xid = secrets.randbits(32)
    dst = DCP_MULTICAST
    response_frame = 0xFEFF
    service = 5
    requested = None
    if args.get_name or args.get_ip:
        dst = args.get_name or args.get_ip
        requested = (2,2) if args.get_name else (1,2)
        payload = make_get(xid, *requested)
        response_frame, service = 0xFEFD, 3
    elif args.set_name or args.set_ip:
        if not args.confirm_set:
            p.error("Set requests require --confirm-set; use only on isolated test LAN")
        if args.set_name:
            dst, name = args.set_name
            try:
                validate_station_name(name)
            except ValueError as exc:
                p.error(str(exc))
            requested = (2,2)
            value = name.encode("ascii")
        else:
            dst, ip, mask, gw = args.set_ip
            requested = (1,2)
            try:
                value = b"".join(ipaddress.IPv4Address(x).packed for x in (ip,mask,gw))
            except ipaddress.AddressValueError as exc:
                p.error(str(exc))
        payload = make_set(xid, *requested, value)
        response_frame, service = 0xFEFD, 4
    else:
        payload = make_identify(xid)
    if dst != DCP_MULTICAST:
        try:
            mac_bytes(dst)
        except ValueError as exc:
            p.error(str(exc))
        dst = dst.lower()
    replies = execute(args.iface, dst, payload, xid, service, response_frame, args.timeout)
    for mac, blocks in replies:
        print(f"Device: {mac}")
        if service == 4:
            statuses = [value for opt, sub, _info, value in blocks if (opt,sub)==(5,4)]
            for value in statuses:
                if len(value) >= 3:
                    status = value[2]
                    print(f"  Set {value[0]:02X}/{value[1]:02X}: {SET_ERRORS.get(status, str(status))}")
        else:
            name = find_value(blocks, 2, 2)
            if name is not None:
                print("  Name:", name.decode("utf-8", errors="replace"))
            ip = find_value(blocks, 1, 2)
            if ip is not None and len(ip) == 12:
                print("  IP:", ip_text(ip[0:4]))
                print("  Mask:", ip_text(ip[4:8]))
                print("  Gateway:", ip_text(ip[8:12]))
            if requested and find_value(blocks, *requested) is None:
                print("  Requested DCP field was not provided")
    print(f"Found {len(replies)} DCP response(s).")


if __name__ == "__main__":
    main()
