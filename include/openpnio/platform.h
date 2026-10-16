#ifndef OPENPNIO_PLATFORM_H
#define OPENPNIO_PLATFORM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Portable platform boundary. All timestamps use monotonic microseconds. */
typedef struct openpnio_platform {
    void *context;
    uint64_t (*monotonic_us)(void *context);
    int (*send_ethernet_frame)(void *context, const uint8_t *frame, size_t size);
} openpnio_platform_t;

#ifdef __cplusplus
}
#endif

#endif /* OPENPNIO_PLATFORM_H */
