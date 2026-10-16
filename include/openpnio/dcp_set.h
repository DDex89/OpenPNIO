#ifndef OPENPNIO_DCP_SET_H
#define OPENPNIO_DCP_SET_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum openpnio_set_status {
    OPENPNIO_SET_OK = 0,
    OPENPNIO_SET_OPTION_UNSUPPORTED = 1,
    OPENPNIO_SET_SUBOPTION_UNSUPPORTED = 2,
    OPENPNIO_SET_NOT_POSSIBLE = 5
} openpnio_set_status_t;
/* Called for a validated station name. Return 0 only after accepting the change.
 * Persistent storage and network reconfiguration are platform responsibilities. */
typedef int (*openpnio_station_name_setter_t)(void *context, const char *name,
                                               size_t name_length, uint16_t qualifier);
/* Returns 1 if a DCP Set response was produced, 0 if frame is not a Set request,
 * negative on invalid/truncated input or insufficient response capacity.
 * Does not implement IP Set, reset or other Set operations. */
int openpnio_dcp_handle_set(const uint8_t *request, size_t request_size,
                           const uint8_t device_mac[6],
                           openpnio_station_name_setter_t setter, void *context,
                           uint8_t *response, size_t capacity, size_t *written);
#ifdef __cplusplus
}
#endif
#endif
