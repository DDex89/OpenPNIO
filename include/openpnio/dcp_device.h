#ifndef OPENPNIO_DCP_DEVICE_H
#define OPENPNIO_DCP_DEVICE_H
#include "openpnio/dcp_response.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Returns 1 when a reply is encoded, 0 when request does not match,
 * -1 for malformed input, -2 if output capacity is insufficient.
 * Only DCP Identify requests are currently supported. */
int openpnio_dcp_handle_identify(const uint8_t *request, size_t request_size,
                                const uint8_t device_mac[6],
                                const openpnio_dcp_identity_t *identity,
                                uint8_t *response, size_t response_capacity,
                                size_t *response_size);
#ifdef __cplusplus
}
#endif
#endif
