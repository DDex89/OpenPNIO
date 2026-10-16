#ifndef OPENPNIO_DCP_IDENTITY_FIELDS_H
#define OPENPNIO_DCP_IDENTITY_FIELDS_H
#include "openpnio/dcp_response.h"
#include <stddef.h>
#include <stdint.h>

/* Core-maintained registry of actual Get/Identify fields.
 * Lists only fields this implementation can answer. */
extern const uint8_t openpnio_dcp_device_options[];
extern const size_t openpnio_dcp_device_options_size;

typedef struct openpnio_dcp_field {
    const uint8_t *data;
    size_t size;
    uint16_t block_info;
    uint8_t scratch[16];
} openpnio_dcp_field_t;

/* Returns 1 for supported field, 0 if not implemented, -1 on invalid args.
 * The data pointer may refer to identity storage, device_options, or scratch.
 * The caller must use it before the field object goes out of scope. */
int openpnio_dcp_identity_field(const openpnio_dcp_identity_t *identity,
                               const uint8_t device_mac[6],
                               uint8_t option, uint8_t suboption,
                               openpnio_dcp_field_t *out);
#endif
