#include "openpnio/frame.h"
#include <string.h>
static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}
openpnio_frame_result_t openpnio_parse_frame(const uint8_t *bytes, size_t length,
                                               openpnio_frame_t *out)
{
    size_t offset = 12;
    uint16_t ethertype;
    openpnio_frame_t parsed = {0};
    if (bytes == NULL || out == NULL || length < 14) {
        return OPENPNIO_FRAME_SHORT;
    }
    memcpy(parsed.dst, bytes, 6);
    memcpy(parsed.src, bytes + 6, 6);
    ethertype = read_be16(bytes + offset);
    offset += 2;
    if (ethertype == OPENPNIO_VLAN_TPID) {
        if (length < offset + 4) {
            return OPENPNIO_FRAME_SHORT;
        }
        parsed.has_vlan = 1;
        parsed.vlan_tci = read_be16(bytes + offset);
        ethertype = read_be16(bytes + offset + 2);
        offset += 4;
        if (ethertype == OPENPNIO_VLAN_TPID || ethertype == 0x88a8) {
            return OPENPNIO_FRAME_UNSUPPORTED_VLAN;
        }
    } else if (ethertype == 0x88a8) {
        return OPENPNIO_FRAME_UNSUPPORTED_VLAN;
    }
    if (ethertype != OPENPNIO_ETHERTYPE) {
        return OPENPNIO_FRAME_NOT_PROFINET;
    }
    if (length < offset + 2) {
        return OPENPNIO_FRAME_SHORT;
    }
    parsed.frame_id = read_be16(bytes + offset);
    parsed.payload = bytes + offset + 2;
    parsed.payload_size = length - offset - 2;
    *out = parsed;
    return OPENPNIO_FRAME_OK;
}
