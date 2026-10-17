#ifndef OPENPNIO_LLDP_H
#define OPENPNIO_LLDP_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Minimal IEEE 802.1AB LLDPDU framing, not a complete PROFINET LLDP
 * topology manager (no PI organization-specific TLVs or neighbor DB). */
typedef struct openpnio_lldp_frame {
    uint8_t src_mac[6];
    const uint8_t *chassis_id;
    size_t chassis_id_length;
    const uint8_t *port_id;
    size_t port_id_length;
    uint16_t ttl_seconds;
} openpnio_lldp_frame_t;

/* Builds a standard untagged LLDP advertisement, including Ethernet header
 * and minimum frame padding (without Ethernet FCS). Not sent by itself. */
int openpnio_lldp_build(const uint8_t src_mac[6], const char *station_name,
                       uint16_t ttl_seconds, uint8_t *out, size_t capacity,
                       size_t *written);
/* Parse an untagged LLDP frame, with strict mandatory TLV ordering and bounds.
 * Returned slices refer into caller-owned frame bytes. */
int openpnio_lldp_parse(const uint8_t *frame, size_t size,
                       openpnio_lldp_frame_t *out);
#ifdef __cplusplus
}
#endif
#endif
