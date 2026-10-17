#!/usr/bin/env python3
"""Generate an *experimental* GSDML-V2.32 device description from the OpenPNIO
fixed lab model. This does NOT create a working PNIO IO Device.

A genuine PI-assigned VendorID and chosen DeviceID are required before the
user can prepare an import candidate. --preview makes a structural-only
document with deliberately UNASSIGNED identity 0x0000; don't deploy it.
GSDML schema validation and TIA Portal V19 import remain separate gates.
"""
import argparse
import json
from pathlib import Path
import re
import sys
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
NS = "http://www.profibus.com/GSDML/2003/11/DeviceProfile"
XSI = "http://www.w3.org/2001/XMLSchema-instance"
ET.register_namespace("", NS)
ET.register_namespace("xsi", XSI)


def tag(name):
    return "{" + NS + "}" + name


def elem(parent, name, **attributes):
    return ET.SubElement(parent, tag(name), {k: str(v) for k, v in attributes.items()})


def hex32(value):
    return f"0x{int(value, 16):08X}"


def load_catalog(path=ROOT / "config/lab-device.json"):
    cfg = json.loads(Path(path).read_text(encoding="utf-8"))
    d, i, o = cfg["dap"], cfg["digital_input"], cfg["digital_output"]
    iface, port = cfg["pdev_interface"], cfg["pdev_port"]
    assert (d["slot"], d["subslot"]) == (0, 1)
    assert (iface["slot"], iface["subslot"]) == (0, 0x8000)
    assert (port["slot"], port["subslot"]) == (0, 0x8001)
    assert (i["slot"], i["subslot"], i["input_bytes"], i["output_bytes"]) == (1, 1, 1, 0)
    assert (o["slot"], o["subslot"], o["input_bytes"], o["output_bytes"]) == (2, 1, 0, 1)
    ids = [int(v["module_ident"], 16) for v in (d, i, o)]
    assert len(set(ids)) == 3
    return cfg


def build_gsdml(cfg, vendor_id, device_id, vendor_name, dns_name):
    if not re.fullmatch(r"[a-z][a-z0-9-]{0,239}", dns_name) or dns_name.endswith("-"):
        raise ValueError("DNS_CompatibleName must be lowercase and DNS-compatible")
    root = ET.Element(tag("ISO15745Profile"),
                      {"{" + XSI + "}schemaLocation": NS + " GSDML-DeviceProfile-V2.32.xsd"})
    head = elem(root, "ProfileHeader")
    for k, v in [
        ("ProfileIdentification", "PROFINET Device Profile"),
        ("ProfileRevision", "1.00"),
        ("ProfileName", "Device Profile for PROFINET Devices"),
        ("ProfileSource", "PROFIBUS Nutzerorganisation e. V. (PNO)"),
        ("ProfileClassID", "Device"),
    ]:
        elem(head, k).text = v
    iso = elem(head, "ISO15745Reference")
    for k, v in [("ISO15745Part", "4"), ("ISO15745Edition", "1"),
                 ("ProfileTechnology", "GSDML")]:
        elem(iso, k).text = v
    body = elem(root, "ProfileBody")
    identity = elem(body, "DeviceIdentity",
                    VendorID=f"0x{vendor_id:04X}", DeviceID=f"0x{device_id:04X}")
    elem(identity, "InfoText", TextId="DEV_INFO")
    elem(identity, "VendorName", Value=vendor_name)
    family = elem(elem(body, "DeviceFunction"), "Family",
                  MainFamily="I/O", ProductFamily=cfg["product_family"])
    assert family is not None
    ap = elem(body, "ApplicationProcess")
    daps = elem(ap, "DeviceAccessPointList")
    d = cfg["dap"]
    dap = elem(daps, "DeviceAccessPointItem",
               ID="DAP", PNIO_Version="V2.32", PhysicalSlots="0..2",
               FixedInSlots="0", ModuleIdentNumber=hex32(d["module_ident"]),
               MinDeviceInterval="32", DNS_CompatibleName=dns_name,
               ObjectUUID_LocalIndex="1", DeviceAccessSupported="true",
               CheckDeviceID_Allowed="true")
    mi = elem(dap, "ModuleInfo")
    elem(mi, "Name", TextId="DAP_NAME")
    elem(mi, "InfoText", TextId="DAP_INFO")
    elem(mi, "OrderNumber", Value="OpenPNIO-8DI8DO-LAB")
    elem(mi, "SoftwareRelease", Value="V0.0.1")
    elem(dap, "IOConfigData", MaxInputLength="1", MaxOutputLength="1",
         MaxDataLength="2")
    use = elem(dap, "UseableModules")
    elem(use, "ModuleItemRef", ModuleItemTarget="M_DI", AllowedInSlots="1")
    elem(use, "ModuleItemRef", ModuleItemTarget="M_DO", AllowedInSlots="2")
    subslots = elem(dap, "SubslotList")
    elem(subslots, "SubslotItem", SubslotNumber="32768", TextId="IFACE_TEXT")
    elem(subslots, "SubslotItem", SubslotNumber="32769", TextId="PORT_TEXT")
    virtual = elem(dap, "VirtualSubmoduleList")
    dsm = elem(virtual, "VirtualSubmoduleItem", ID="V_DAP",
               SubmoduleIdentNumber=hex32(d["submodule_ident"]),
               FixedInSubslots="1", MayIssueProcessAlarm="false")
    elem(dsm, "IOData")
    info = elem(dsm, "ModuleInfo")
    elem(info, "Name", TextId="DAP_NAME")
    elem(info, "InfoText", TextId="DAP_INFO")
    system = elem(dap, "SystemDefinedSubmoduleList")
    iface = elem(system, "InterfaceSubmoduleItem",
                 ID="PDEV_IFACE", SubslotNumber="32768",
                 SubmoduleIdentNumber=hex32(cfg["pdev_interface"]["submodule_ident"]),
                 TextId="IFACE_TEXT", SupportedRT_Classes="RT_CLASS_1",
                 SupportedProtocols="LLDP")
    ar = elem(iface, "ApplicationRelations", NumberOfAR="1")
    elem(ar, "TimingProperties", SendClock="32",
         ReductionRatio="1 2 4 8 16 32 64 128")
    elem(system, "PortSubmoduleItem",
         ID="PDEV_PORT1", SubslotNumber="32769",
         SubmoduleIdentNumber=hex32(cfg["pdev_port"]["submodule_ident"]),
         TextId="PORT_TEXT", MAUTypes="16")
    modules = elem(ap, "ModuleList")
    for k, name, label, direction in (
        ("digital_input", "M_DI", "8_DI_NAME", "Input"),
        ("digital_output", "M_DO", "8_DO_NAME", "Output"),
    ):
        item = cfg[k]
        module = elem(modules, "ModuleItem", ID=name,
                      ModuleIdentNumber=hex32(item["module_ident"]))
        info = elem(module, "ModuleInfo")
        elem(info, "Name", TextId=label)
        elem(info, "InfoText", TextId=label)
        vs = elem(module, "VirtualSubmoduleList")
        sub = elem(vs, "VirtualSubmoduleItem", ID=name + "_SUB",
                   SubmoduleIdentNumber=hex32(item["submodule_ident"]),
                   FixedInSubslots=str(item["subslot"]),
                   MayIssueProcessAlarm="false")
        iod = elem(sub, "IOData", IOPS_Length="1", IOCS_Length="1")
        data = elem(iod, direction)
        elem(data, "DataItem", DataType="OctetString",
             Length=str(item["input_bytes"] or item["output_bytes"]),
             TextId=label)
        info = elem(sub, "ModuleInfo")
        elem(info, "Name", TextId=label)
        elem(info, "InfoText", TextId=label)
    primary = elem(elem(ap, "ExternalTextList"), "PrimaryLanguage")
    for key, value in (
        ("DEV_INFO", "OpenPNIO EXPERIMENTAL development device"),
        ("DAP_NAME", "OpenPNIO virtual device access point"),
        ("DAP_INFO", "Experimental fixed 8DI / 8DO demo; no operating CM/AR yet"),
        ("IFACE_TEXT", "Ethernet interface X1"),
        ("PORT_TEXT", "Ethernet port X1 P1"),
        ("8_DI_NAME", "8 virtual digital inputs (1 byte)"),
        ("8_DO_NAME", "8 virtual digital outputs (1 byte)"),
    ):
        elem(primary, "Text", TextId=key, Value=value)
    return root


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--vendor-id", type=lambda s: int(s, 0))
    parser.add_argument("--device-id", type=lambda s: int(s, 0))
    parser.add_argument("--vendor-name", default="OpenPNIO experimental")
    parser.add_argument("--dns-name", default="openpnio-test")
    parser.add_argument("--preview", action="store_true",
                        help="structural preview with 0x0000 identity; NOT usable for a real device")
    parser.add_argument("--confirm-owned-vendor-id", action="store_true",
                        help="affirm that the supplied VendorID was assigned by PI to your organization")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    if args.preview:
        if args.vendor_id is not None or args.device_id is not None:
            parser.error("--preview uses deliberately unassigned 0x0000/0x0000 only")
        vendor, device = 0, 0
        print("WARNING: PREVIEW ONLY: identity 0x0000/0x0000 is not assigned.",
              file=sys.stderr)
    else:
        if not args.confirm_owned_vendor_id:
            parser.error("real candidates require --confirm-owned-vendor-id")
        vendor, device = args.vendor_id, args.device_id
        if vendor is None or device is None or not (1 <= vendor < 0xFFFF and
                                                    1 <= device <= 0xFFFF):
            parser.error("valid --vendor-id and --device-id required")
    try:
        tree = ET.ElementTree(build_gsdml(load_catalog(),vendor,device,
                                          args.vendor_name,args.dns_name))
    except (ValueError, AssertionError, KeyError) as exc:
        parser.error(f"invalid catalog / identity: {exc}")
    ET.indent(tree, space="  ")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    tree.write(args.output, encoding="utf-8", xml_declaration=True)
    print(f"Generated {args.output} — XML well-formedness only.")
    print("Must validate against official PI GSDML V2.32 XSD and test TIA Portal import.")
    print("Do not claim IO operation until RPC/CM/AR and RTC1 are actually implemented.")


if __name__ == "__main__":
    main()
