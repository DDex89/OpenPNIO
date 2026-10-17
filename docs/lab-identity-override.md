# Explicit, session-only PROFINET identity override for an isolated bench

**Status: implemented for local experiment, 2026-10-17.** This mode is intended solely to compare a GSDML-described device's identity with a physical DCP responder using caller-supplied test numbers. It does **not** establish ownership of an assigned PI VendorID, product identity, or PROFINET certification.

**Nothing in the tracked C source, tests, manifest, default runtime, README, or GitHub Issues selects a borrowed manufacturer ID.** The user chooses the values locally at runtime.

## Safety requirements

1. Use a physically separated dedicated lab Ethernet link or a test-only VLAN. **The existing 192.168.1.x network previously showed other equipment and a router**, so it must not automatically be considered isolated.
2. Do not join a production/shared industrial segment, broadcast forged device identities to plant systems, or deploy a GSDML containing an ID owned by another manufacturer.
3. The Linux runner accepts a third-party/unregistered identity only with all three flags: `--lab-vendor-id`, `--lab-device-id`, and `--acknowledge-isolated-lab`. Both IDs must be in range 1 through 65534. No ID is stored in the station-name file; the override disappears on process restart.
4. **DCP Set IP changes remain refused**; the process does not alter network IP, Linux hostname, gateway, or link properties.
5. GSDML generated with `--lab-unregistered-identity` must be written **outside the repository**, requires `--confirm-isolated-lab`, and keeps `VendorName` identifying OpenPNIO as experimental (it does NOT claim to be Siemens or the owner of the borrowed ID).
6. Borrowing a test identity does not fix incomplete GSDML or missing CM/AR/RTC1. GSDML preview has **not** passed an official PI GSDML XSD check and **not** been imported into TIA V19 as an IO Device. Perform PI schema validation before interpreting an import failure as a VendorID problem.

## Local-only values

Choose a publicly documented known PI VendorID **and a distinct experimental DeviceID**. The device number here is a laboratory selector, **not** an assigned product number; check for conflicts on the isolated segment.

Do NOT put either value in a repository config, tracked XML, issue, commit, release or a public screenshot. Input the values locally on the Pi:

```sh
export LAB_VENDOR_ID='REPLACE_WITH_HEX_VENDOR_ID'
export LAB_DEVICE_ID='REPLACE_WITH_HEX_DEVICE_ID'
```

The placeholders must be replaced with actual numeric values (`0x...`), not left literal. Shell expansion passes them to the executable; nothing in the application stores those values on disk.

Use the **same shell** that contains the two `export` commands. First generate a local XML for comparison with the runtime, then launch the responder (which blocks the terminal). If using two SSH shells instead, export the two ID values independently in **both** shells.

**First generate the XML outside the repository** (no network traffic is sent):

```sh
cd ~/OpenPNIO
python3 tools/generate_gsdml.py \
    --vendor-id "$LAB_VENDOR_ID" \
    --device-id "$LAB_DEVICE_ID" \
    --lab-unregistered-identity \
    --confirm-isolated-lab \
    --vendor-name 'OpenPNIO experimental' \
    --dns-name testnode \
    --output /tmp/GSDML-V2.32-OpenPNIO-LOCAL.lab.xml
```

The generator only verifies XML well-formedness and internal manifest consistency; this is **not yet official GSDML schema validation**.

**Then start the DCP responder**, only on a physically isolated Pi lab segment:

```sh
cd ~/OpenPNIO
sudo ./build/openpnio_dcp_probe eth0 \
    --allow-set-name \
    --station-file /var/tmp/openpnio-station-name \
    --lldp \
    --lab-vendor-id "$LAB_VENDOR_ID" \
    --lab-device-id "$LAB_DEVICE_ID" \
    --acknowledge-isolated-lab
```

It should report `Station: testnode`, actual eth0 IP, and an explicit **LAB-ONLY DCP IDENTITY** warning with the selected values. Stop and restart the process **without** these flags to restore `VendorID=0x0000` and `DeviceID=0x0000`.

To copy the local XML from the Pi onto the Windows TIA Portal machine, use Windows PowerShell:

```powershell
scp su@192.168.1.8:/tmp/GSDML-V2.32-OpenPNIO-LOCAL.lab.xml "$env:USERPROFILE\Downloads\"
```

Confirm the runtime DCP DeviceID field and the XML `DeviceIdentity` agree. Do not mistake an XML file's successful generation for schema validation, TIA import, AR negotiation or real I/O exchange.

## Future production identity

Use an assigned PI VendorID, choose a legitimate DeviceID, and use the generator's normal `--confirm-owned-vendor-id` mode. PI registration and certification are separate matters. Remove temporary third-party identities from the lab, then update DCP + GSDML together.

More context: [batch test plan](siemens-batch-test-plan.md) and [TIA DCP hardware validation](siemens-tia-v19-dcp-validation-2026-10-17.md).
