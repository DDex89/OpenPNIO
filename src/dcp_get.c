#include "openpnio/dcp_get.h"
#include "openpnio/dcp.h"
#include "openpnio/frame.h"
#include <string.h>

static void put16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)v;
}
static void put32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v >> 24); p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8); p[3] = (uint8_t)v;
}
static int append(uint8_t *out, size_t cap, size_t *pos, uint8_t option,
                  uint8_t suboption, uint16_t info, const uint8_t *value, size_t len)
{
    size_t pad = len & 1u;
    size_t bytes = 6u + len + pad;
    if (!out || !pos || len > 65533u || *pos < 26u || *pos > cap ||
        bytes > cap - *pos || bytes > 65535u ||
        *pos - 26u > 65535u - bytes)
        return -1;
    out[*pos] = option;
    out[*pos + 1] = suboption;
    put16(out + *pos + 2, (uint16_t)(len + 2u));
    put16(out + *pos + 4, info);
    if (len) memcpy(out + *pos + 6, value, len);
    if (pad) out[*pos + 6 + len] = 0;
    *pos += 6 + len + pad;
    return 0;
}
int openpnio_dcp_handle_get(const uint8_t *request, size_t request_size,
                            const uint8_t device_mac[6],
                            const openpnio_dcp_identity_t *identity,
                            uint8_t *response, size_t capacity, size_t *written)
{
    openpnio_frame_t frame;
    openpnio_dcp_header_t header;
    size_t pos = 26, i, count;
    uint8_t ids[4], role = 1, ip[12];
    if (!request || !device_mac || !identity || !identity->station_name ||
        !identity->station_type || !response || !written) return -1;
    *written = 0;
    if (openpnio_parse_frame(request, request_size, &frame) != OPENPNIO_FRAME_OK) return -1;
    if (frame.frame_id != 0xfefd || memcmp(frame.dst, device_mac, 6) != 0) return 0;
    if (openpnio_dcp_parse_header(frame.payload, frame.payload_size, &header) != OPENPNIO_DCP_OK)
        return -1;
    if (header.service_id != 3 || header.service_type != 0) return 0;
    if (header.blocks_length < 2 || (header.blocks_length & 1u) != 0) return -1;
    /* The Get service carries option/suboption pairs, without block headers. */
    count = header.blocks_length / 2;
    for (i = 0; i < count; ++i) {
        uint8_t option = header.blocks[2*i], suboption = header.blocks[2*i+1];
        if (!((option == 1 && suboption == 2) ||
              (option == 2 && suboption >= 1 && suboption <= 4))) return 0;
    }
    if (capacity < 26) return -2;
    memcpy(response, frame.src, 6);
    memcpy(response + 6, device_mac, 6);
    put16(response + 12, 0x8892);
    put16(response + 14, 0xfefd);
    response[16] = 3; response[17] = 1;
    put32(response + 18, header.xid);
    put16(response + 22, 0);
    put16(response + 24, 0);
    put16(ids, identity->vendor_id);
    put16(ids + 2, identity->device_id);
    memcpy(ip, identity->ip, 4);
    memcpy(ip + 4, identity->netmask, 4);
    memcpy(ip + 8, identity->gateway, 4);
    for (i = 0; i < count; ++i) {
        uint8_t option = header.blocks[2*i], suboption = header.blocks[2*i+1];
        const uint8_t *value = NULL;
        size_t len = 0;
        uint16_t info = 0;
        if (option == 1) {
            value = ip; len = sizeof ip;
            info = (identity->ip[0] || identity->ip[1] ||
                    identity->ip[2] || identity->ip[3]) ? 1 : 0;
        } else if (suboption == 1) {
            value = (const uint8_t *)identity->station_type;
            len = strlen(identity->station_type);
        } else if (suboption == 2) {
            value = (const uint8_t *)identity->station_name;
            len = strlen(identity->station_name);
        } else if (suboption == 3) {
            value = ids; len = sizeof ids;
        } else {
            value = &role; len = sizeof role;
        }
        if (append(response, capacity, &pos, option, suboption, info, value, len) != 0)
            return -2;
    }
    put16(response + 24, (uint16_t)(pos - 26u));
    *written = pos;
    return 1;
}
