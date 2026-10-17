"""Offline structure/identity regression tests for the experimental GSDML generator.
Do NOT treat these tests as official PI XSD or Siemens TIA Portal validation.
"""
import json
from pathlib import Path
import tempfile
import unittest
import xml.etree.ElementTree as ET

from tools import generate_gsdml as gsd


class GsdmlTests(unittest.TestCase):
    def setUp(self):
        self.cfg = gsd.load_catalog()
        self.ns = {"g": gsd.NS}

    def test_manifest_describes_virtual_process_image(self):
        self.assertEqual(self.cfg["digital_input"]["input_bytes"], 1)
        self.assertEqual(self.cfg["digital_output"]["output_bytes"], 1)
        self.assertEqual(self.cfg["digital_input"]["slot"], 1)
        self.assertEqual(self.cfg["digital_output"]["slot"], 2)
        self.assertEqual(self.cfg["pdev_interface"]["subslot"], 0x8000)
        self.assertEqual(self.cfg["pdev_port"]["subslot"], 0x8001)

    def test_gsdml_identity_and_module_lengths(self):
        root = gsd.build_gsdml(self.cfg, 0x1234, 0x5678, "TEST FIXTURE", "testnode")
        identity = root.find(".//g:DeviceIdentity", self.ns)
        self.assertEqual(identity.attrib["VendorID"], "0x1234")
        self.assertEqual(identity.attrib["DeviceID"], "0x5678")
        modules = root.findall(".//g:ModuleItem", self.ns)
        self.assertEqual(len(modules), 2)
        ids = {m.attrib["ID"]: m.attrib["ModuleIdentNumber"] for m in modules}
        self.assertEqual(ids["M_DI"], "0x00001001")
        self.assertEqual(ids["M_DO"], "0x00001002")
        self.assertEqual(root.find(".//g:Input/g:DataItem", self.ns).attrib["Length"], "1")
        self.assertEqual(root.find(".//g:Output/g:DataItem", self.ns).attrib["Length"], "1")
        iface = root.find(".//g:InterfaceSubmoduleItem", self.ns)
        port = root.find(".//g:PortSubmoduleItem", self.ns)
        self.assertEqual(iface.attrib["SubslotNumber"], "32768")
        self.assertEqual(port.attrib["SubslotNumber"], "32769")
        self.assertEqual(iface.attrib["SubmoduleIdentNumber"], "0x00000F00")
        self.assertEqual(port.attrib["SubmoduleIdentNumber"], "0x00000F01")

    def test_tia_v19_schema_regression_missing_attribute_and_wrong_child_order(self):
        """Regression for observed TIA V19 SchemaValidation errors at DAP."""
        root = gsd.build_gsdml(self.cfg, 0x1234, 0x5678,
                               "TEST FIXTURE", "testnode")
        dap = root.find(".//g:DeviceAccessPointItem", self.ns)
        self.assertIsNotNone(dap)
        # NameOfStationNotTransferable is mandatory for the chosen schema;
        # this prototype has no removable station-name memory carrier.
        self.assertEqual(dap.attrib["NameOfStationNotTransferable"], "true")
        direct_children = [x.tag.split("}")[-1] for x in dap]
        for expected in ("ModuleInfo", "SubslotList", "IOConfigData",
                         "UseableModules", "VirtualSubmoduleList",
                         "SystemDefinedSubmoduleList"):
            self.assertIn(expected, direct_children)
        positions = [direct_children.index(x) for x in
                     ("ModuleInfo", "SubslotList", "IOConfigData",
                      "UseableModules", "VirtualSubmoduleList",
                      "SystemDefinedSubmoduleList")]
        self.assertEqual(positions, sorted(positions))
        subslots = dap.find("g:SubslotList", self.ns)
        self.assertEqual([s.attrib["SubslotNumber"] for s in subslots],
                         ["32768", "32769"])

    def test_tia_v19_interface_mib_and_port_mau_schema_regressions(self):
        """Second TIA V19 install log: required SupportedMibs and port child."""
        root = gsd.build_gsdml(self.cfg, 0x1234, 0x5678,
                               "TEST FIXTURE", "testnode")
        interface = root.find(".//g:InterfaceSubmoduleItem", self.ns)
        port = root.find(".//g:PortSubmoduleItem", self.ns)
        self.assertEqual(interface.attrib["SupportedMibs"], "MIB2")
        self.assertEqual(port.attrib["MAUTypes"], "16")
        mau_items = port.findall("g:MAUTypeList/g:MAUTypeItem", self.ns)
        self.assertEqual(len(mau_items), 1)
        self.assertEqual(mau_items[0].attrib["Value"], "16")
        self.assertEqual(mau_items[0].attrib["AdjustSupported"], "false")

    def test_generator_xml_is_well_formed(self):
        root = gsd.build_gsdml(self.cfg, 0, 0, "UNASSIGNED", "openpnio-test")
        reloaded = ET.fromstring(ET.tostring(root))
        self.assertEqual(reloaded.tag, gsd.tag("ISO15745Profile"))

    def test_preview_and_owned_vendor_identity_gate(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "preview.xml"
            gsd.main(["--preview", "--output", str(path)])
            self.assertTrue(path.is_file())
            root = ET.parse(path)
            self.assertEqual(root.find(".//g:DeviceIdentity", self.ns).attrib["VendorID"],
                             "0x0000")
            with self.assertRaises(SystemExit):
                gsd.main(["--vendor-id", "0x1234", "--device-id", "0x5678",
                          "--output", str(path)])
            gsd.main(["--vendor-id", "0x1234", "--device-id", "0x5678",
                      "--confirm-owned-vendor-id", "--output", str(path)])
            root = ET.parse(path)
            self.assertEqual(root.find(".//g:DeviceIdentity", self.ns).attrib["DeviceID"],
                             "0x5678")

    def test_unregistered_identity_requires_isolated_lab_and_external_path(self):
        with tempfile.TemporaryDirectory() as directory:
            xml = Path(directory) / "GSDML-LAB.xml"
            args = ["--vendor-id", "0x1234", "--device-id", "0x5678",
                    "--lab-unregistered-identity", "--output", str(xml)]
            with self.assertRaises(SystemExit):
                gsd.main(args)  # --confirm-isolated-lab absent
            gsd.main(args + ["--confirm-isolated-lab"])
            root = ET.parse(xml)
            identity = root.find(".//g:DeviceIdentity", self.ns)
            self.assertEqual(identity.attrib["VendorID"], "0x1234")
            self.assertEqual(identity.attrib["DeviceID"], "0x5678")
            self.assertEqual(root.find(".//g:VendorName", self.ns).attrib["Value"],
                             "OpenPNIO experimental")
            with self.assertRaises(SystemExit):
                gsd.main(args + ["--confirm-isolated-lab",
                                 "--confirm-owned-vendor-id"])
            with self.assertRaises(SystemExit):
                gsd.main(["--vendor-id", "0x1234", "--device-id", "0x5678",
                          "--lab-unregistered-identity", "--confirm-isolated-lab",
                          "--output", str(gsd.ROOT / "GSDML-LAB-DO-NOT-COMMIT.xml")])
            self.assertFalse((gsd.ROOT / "GSDML-LAB-DO-NOT-COMMIT.xml").exists())

    def test_manifest_matches_c11_identifiers(self):
        header = (gsd.ROOT / "include/openpnio/device_model.h").read_text(encoding="utf-8")
        self.assertIn("OPENPNIO_LAB_IDENT_DAP = 0x00000001", header)
        self.assertIn("OPENPNIO_LAB_IDENT_DI = 0x00001001", header)
        self.assertIn("OPENPNIO_LAB_IDENT_DO = 0x00001002", header)
        self.assertIn("OPENPNIO_LAB_IDENT_PDEV_INTERFACE = 0x00000f00", header)
        self.assertIn("OPENPNIO_LAB_IDENT_PDEV_PORT_1 = 0x00000f01", header)


if __name__ == "__main__":
    unittest.main()
