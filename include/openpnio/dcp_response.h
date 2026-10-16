#ifndef OPENPNIO_DCP_RESPONSE_H
#define OPENPNIO_DCP_RESPONSE_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct openpnio_dcp_identity {
    const char *station_name;
    const char *station_type;
    uint16_t vendor_id;
    uint16_t device_id;
    uint32_t device_instance; /* DCP DeviceInstance (high/low word); zero for lab prototype. */
    uint8_t ip[4];
    uint8_t netmask[4];
    uint8_t gateway[4];
} openpnio_dcp_identity_t;
/* Encode untagged DCP Identify response, including Ethernet header and FrameID.
 * Returns 0 on success; negative on invalid arguments or insufficient capacity.
 * No allocations. Source MAC is supplied by caller. */
int openpnio_dcp_identify_response(const uint8_t dst_mac[6],
                                   const uint8_t src_mac[6],
                                   uint32_t xid,
                                   const openpnio_dcp_identity_t *identity,
                                   uint8_t *out, size_t capacity,
                                   size_t *written);
#ifdef __cplusplus
}
#endif
#endif
