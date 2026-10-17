#ifndef OPENPNIO_LAB_IDENTITY_H
#define OPENPNIO_LAB_IDENTITY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Explicitly supplied session-only test identity. No assigned or borrowed
 * manufacturer values are compiled into OpenPNIO. */
int openpnio_lab_identity_parse(const char *text, uint16_t *value);

/* Enforce all-or-nothing selection: BOTH vendor and device IDs, with explicit
 * isolated-lab acknowledgement, or NEITHER (default zero placeholder). */
int openpnio_lab_identity_validate(int lab_acknowledged,
                                  int vendor_supplied, int device_supplied,
                                  uint16_t vendor, uint16_t device);

#ifdef __cplusplus
}
#endif
#endif
