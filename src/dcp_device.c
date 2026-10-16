#include "openpnio/dcp_device.h"
#include "openpnio/dcp.h"
#include "openpnio/frame.h"
#include <string.h>
static const uint8_t identify_multicast[6] = {0x01,0x0e,0xcf,0x00,0x00,0x00};
int openpnio_dcp_handle_identify(const uint8_t *request, size_t request_size,
                                const uint8_t device_mac[6],
                                const openpnio_dcp_identity_t *identity,
                                uint8_t *response, size_t response_capacity,
                                size_t *response_size)
{
    openpnio_frame_t frame;
    openpnio_dcp_header_t header;
    openpnio_dcp_cursor_t cursor;
    openpnio_dcp_block_t block;
    size_t station_len;
    int next;
    if (!request || !device_mac || !identity || !identity->station_name ||
        !response || !response_size) return -1;
    *response_size = 0;
    if (openpnio_parse_frame(request, request_size, &frame) != OPENPNIO_FRAME_OK)
        return -1;
    if (frame.frame_id != 0xfefe) return 0;
    if (memcmp(frame.dst, identify_multicast, 6) != 0 &&
        memcmp(frame.dst, device_mac, 6) != 0) return 0;
    if (openpnio_dcp_parse_header(frame.payload, frame.payload_size, &header) != OPENPNIO_DCP_OK)
        return -1;
    if (header.service_id != 5 || header.service_type != 0) return 0;
    station_len = strlen(identity->station_name);
    openpnio_dcp_cursor_init(&cursor, header.blocks, header.blocks_length);
    while ((next = openpnio_dcp_next_block(&cursor, &block)) > 0) {
        /* NameOfStation selector: exact, case-sensitive byte comparison.
         * Other selectors are intentionally unsupported, not ignored. */
        if (block.option != 2 || block.suboption != 2) return 0;
        if (block.data_length != station_len ||
            memcmp(block.data, identity->station_name, station_len) != 0) return 0;
    }
    if (next < 0) return -1;
    if (openpnio_dcp_identify_response(frame.src, device_mac, header.xid,
                                      identity, response, response_capacity,
                                      response_size) != 0) return -2;
    return 1;
}
