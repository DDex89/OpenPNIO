#ifndef OPENPNIO_FRAME_H
#define OPENPNIO_FRAME_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
enum { OPENPNIO_ETHERTYPE = 0x8892, OPENPNIO_VLAN_TPID = 0x8100 };
typedef enum openpnio_frame_result {
    OPENPNIO_FRAME_OK = 0,
    OPENPNIO_FRAME_SHORT = -1,
    OPENPNIO_FRAME_NOT_PROFINET = -2,
    OPENPNIO_FRAME_UNSUPPORTED_VLAN = -3
} openpnio_frame_result_t;
typedef struct openpnio_frame {
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t frame_id;
    uint16_t vlan_tci;
    uint8_t has_vlan;
    const uint8_t *payload;
    size_t payload_size;
} openpnio_frame_t;
/* Zero-copy parsing: payload points into caller-owned frame storage. */
openpnio_frame_result_t openpnio_parse_frame(const uint8_t *bytes, size_t length,
                                               openpnio_frame_t *out);
#ifdef __cplusplus
}
#endif
#endif
