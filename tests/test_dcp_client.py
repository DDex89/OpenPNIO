"""Offline tests of DCP wire-format builders and response decoder (no Scapy needed)."""
import struct
import unittest

from tools import dcp_client as dcp


SRC = bytes.fromhex("b827ebbb9e7a")
DST = bytes.fromhex("d45d6464ed20")
XID = 0x340D3590


def ethernet_reply(service, frame_id, blocks, tagged=False):
    dcp_header = bytearray(dcp.make_dcp(frame_id, service, XID, blocks))
    dcp_header[3] = 1  # successful response
    ethernet = DST + SRC
    if tagged:
        ethernet += b"\x81\x00\x00\x00"
    ethernet += b"\x88\x92"
    return ethernet + dcp_header


class DcpClientTests(unittest.TestCase):
    def test_identify_all_matches_real_packet(self):
        data = dcp.make_identify(XID)
        self.assertEqual(data.hex(), "fefe0500340d359000000004ffff0000")

    def test_get_selector_is_two_bytes(self):
        data = dcp.make_get(XID, 2, 2)
        self.assertEqual(data.hex(), "fefd0300340d3590000000020202")
        self.assertEqual(dcp.make_get(XID, 1, 2)[-2:], b"\x01\x02")

    def test_set_name_length_padding_and_qualifier(self):
        data = dcp.make_set(XID, 2, 2, b"openpnio-test")
        self.assertEqual(int.from_bytes(data[10:12], "big"), 20)
        self.assertEqual(data[12:18], b"\x02\x02\x00\x0f\x00\x00")
        self.assertEqual(data[18:31], b"openpnio-test")
        self.assertEqual(data[-1:], b"\x00")

    def test_permanent_name_qualifier(self):
        data = dcp.make_set(XID, 2, 2, b"openpnio-lab", qualifier=1)
        self.assertEqual(data[16:18], b"\x00\x01")
        self.assertEqual(data[12:14], b"\x02\x02")

    def test_identity_option_pairs_role_and_instance(self):
        options = bytes((1,1,1,2,2,1,2,2,2,3,2,4,2,5,2,7))
        blocks = (bytes.fromhex("020500120000") + options +
                  bytes.fromhex("0204000400000100") +
                  bytes.fromhex("0207000400001234"))
        result = dcp.parse_ethernet_response(ethernet_reply(5,0xFEFF,blocks),
                                             XID,5,0xFEFF)
        self.assertEqual(dcp.find_value(result[1],2,5),options)
        self.assertEqual(dcp.find_value(result[1],2,4),b"\x01\x00")
        self.assertEqual(dcp.find_value(result[1],2,7),b"\x12\x34")

    def test_identify_reply_parse(self):
        b = bytes.fromhex("0202000f0000") + b"openpnio-test" + b"\x00"
        result = dcp.parse_ethernet_response(ethernet_reply(5, 0xFEFF, b), XID, 5, 0xFEFF)
        self.assertIsNotNone(result)
        mac, blocks = result
        self.assertEqual(mac, "b8:27:eb:bb:9e:7a")
        self.assertEqual(dcp.find_value(blocks,2,2), b"openpnio-test")
        self.assertEqual(blocks[0][2], 0)

    def test_ip_reply_and_vlan(self):
        ip = bytes([192,168,1,8,255,255,255,0,192,168,1,1])
        b = bytes.fromhex("0102000e0001") + ip
        frame = ethernet_reply(3, 0xFEFD, b, tagged=True)
        result = dcp.parse_ethernet_response(frame, XID, 3, 0xFEFD)
        self.assertEqual(dcp.find_value(result[1],1,2), ip)
        self.assertEqual(dcp.ip_text(ip[:4]), "192.168.1.8")

    def test_set_status_reply(self):
        b = bytes.fromhex("0504000301020500")
        result = dcp.parse_ethernet_response(ethernet_reply(4,0xFEFD,b), XID, 4, 0xFEFD)
        self.assertEqual(result[1][0][3], b"\x01\x02\x05")
        self.assertEqual(dcp.SET_ERRORS[5], "Set not possible")

    def test_malformed_and_other_xid_ignored(self):
        b = bytes.fromhex("0202000f0000") + b"openpnio-test" + b"\x00"
        frame = ethernet_reply(5,0xFEFF,b)
        self.assertIsNone(dcp.parse_ethernet_response(frame,XID+1,5,0xFEFF))
        self.assertIsNone(dcp.parse_ethernet_response(frame[:-4],XID,5,0xFEFF))
        corrupt = ethernet_reply(5,0xFEFF,bytes.fromhex("020200ff0000"))
        with self.assertRaises(ValueError):
            dcp.parse_ethernet_response(corrupt,XID,5,0xFEFF)

    def test_mac_and_station_name_guard(self):
        self.assertEqual(dcp.mac_bytes("B8:27:EB:BB:9E:7A"),SRC)
        with self.assertRaises(ValueError):
            dcp.mac_bytes("b8:27:eb")
        dcp.validate_station_name("openpnio-lab")
        for bad in ("OpenPNIO","a..b","a-.b","-abc","abc-","name space",""):
            with self.subTest(bad=bad):
                with self.assertRaises(ValueError):
                    dcp.validate_station_name(bad)


if __name__ == "__main__":
    unittest.main()
