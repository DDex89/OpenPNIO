#ifndef OPENPNIO_DCP_GET_H
#define OPENPNIO_DCP_GET_H
#include <stddef.h>
#include <stdint.h>
#include "openpnio/dcp_response.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Handle unicast DCP Get for station name/type, device ID/role, and IP suite.
 * Get requests contain Option/Suboption selectors (2 bytes each), not Set TLVs.
 * Returns 1 if a response was encoded, 0 if not a supported Get request,
 * negative on malformed input or insufficient output capacity. */
int openpnio_dcp_handle_get(const uint8_t *request, size_t request_size,
                            const uint8_t device_mac[6],
                            const openpnio_dcp_identity_t *identity,
                            uint8_t *response, size_t capacity, size_t *written);
#ifdef __cplusplus
}
#endif
#endif
