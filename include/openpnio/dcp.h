#ifndef OPENPNIO_DCP_H
#define OPENPNIO_DCP_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum openpnio_dcp_result {
    OPENPNIO_DCP_OK = 0,
    OPENPNIO_DCP_INVALID = -1,
    OPENPNIO_DCP_TRUNCATED = -2,
    OPENPNIO_DCP_UNSUPPORTED = -3
} openpnio_dcp_result_t;
typedef struct openpnio_dcp_header {
    uint8_t service_id;
    uint8_t service_type;
    uint32_t xid;
    uint16_t response_delay;
    const uint8_t *blocks;
    size_t blocks_length;
} openpnio_dcp_header_t;
typedef struct openpnio_dcp_block {
    uint8_t option;
    uint8_t suboption;
    const uint8_t *data;
    size_t data_length;
} openpnio_dcp_block_t;
typedef struct openpnio_dcp_cursor {
    const uint8_t *data;
    size_t length;
    size_t offset;
} openpnio_dcp_cursor_t;
/* Input starts immediately after the two-byte PROFINET FrameID. */
openpnio_dcp_result_t openpnio_dcp_parse_header(const uint8_t *data, size_t size,
                                                  openpnio_dcp_header_t *out);
void openpnio_dcp_cursor_init(openpnio_dcp_cursor_t *cursor, const uint8_t *data,
                               size_t length);
/* Returns 1 for block, 0 for end, negative on malformed input. */
int openpnio_dcp_next_block(openpnio_dcp_cursor_t *cursor, openpnio_dcp_block_t *out);
#ifdef __cplusplus
}
#endif
#endif
