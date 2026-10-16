#include "openpnio/dcp.h"
static uint16_t be16(const uint8_t *p) { return (uint16_t)(((uint16_t)p[0] << 8) | p[1]); }
static uint32_t be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}
openpnio_dcp_result_t openpnio_dcp_parse_header(const uint8_t *data, size_t size,
                                                  openpnio_dcp_header_t *out)
{
    openpnio_dcp_header_t parsed;
    size_t n;
    if (!data || !out) return OPENPNIO_DCP_INVALID;
    if (size < 10) return OPENPNIO_DCP_TRUNCATED;
    if (data[0] < 3 || data[0] > 6) return OPENPNIO_DCP_UNSUPPORTED;
    if (data[1] != 0 && data[1] != 1 && data[1] != 5) return OPENPNIO_DCP_UNSUPPORTED;
    n = be16(data + 8);
    if (n > size - 10) return OPENPNIO_DCP_TRUNCATED;
    parsed.service_id = data[0];
    parsed.service_type = data[1];
    parsed.xid = be32(data + 2);
    parsed.response_delay = be16(data + 6);
    parsed.blocks = data + 10;
    parsed.blocks_length = n;
    *out = parsed;
    return OPENPNIO_DCP_OK;
}
void openpnio_dcp_cursor_init(openpnio_dcp_cursor_t *cursor, const uint8_t *data,
                               size_t length)
{
    if (!cursor) return;
    cursor->data = data;
    cursor->length = length;
    cursor->offset = 0;
}
int openpnio_dcp_next_block(openpnio_dcp_cursor_t *cursor, openpnio_dcp_block_t *out)
{
    size_t remaining, n, advance;
    openpnio_dcp_block_t parsed;
    if (!cursor || !out || !cursor->data || cursor->offset > cursor->length) return OPENPNIO_DCP_INVALID;
    remaining = cursor->length - cursor->offset;
    if (remaining == 0) return 0;
    if (remaining < 4) return OPENPNIO_DCP_TRUNCATED;
    n = be16(cursor->data + cursor->offset + 2);
    if (n > remaining - 4) return OPENPNIO_DCP_TRUNCATED;
    advance = 4 + n + (n & 1u);
    if (advance > remaining) return OPENPNIO_DCP_TRUNCATED;
    parsed.option = cursor->data[cursor->offset];
    parsed.suboption = cursor->data[cursor->offset + 1];
    parsed.data = cursor->data + cursor->offset + 4;
    parsed.data_length = n;
    cursor->offset += advance;
    *out = parsed;
    return 1;
}
