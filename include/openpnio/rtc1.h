#ifndef OPENPNIO_RTC1_H
#define OPENPNIO_RTC1_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* PROFINET RT Class 1 generic framing ONLY. A real IOCR must negotiate
 * frame identifiers, C_SDU object offsets, IOPS/IOCS and cycle timing via
 * CM before application process data can be interpreted safely. */
typedef struct openpnio_rtc1_frame {
    uint8_t source_mac[6];
    uint8_t destination_mac[6];
    uint16_t frame_id;
    const uint8_t *c_sdu;  /* includes PN frame padding */
    size_t c_sdu_length;
    uint16_t cycle_counter;
    uint8_t data_status;
    uint8_t transfer_status;
    uint8_t tagged;
    uint8_t vlan_priority;
} openpnio_rtc1_frame_t;

int openpnio_rtc1_encode(const uint8_t dest[6],const uint8_t src[6],
                        uint16_t frame_id,const uint8_t *c_sdu,
                        size_t c_sdu_length,uint16_t cycle_counter,
                        uint8_t data_status,uint8_t transfer_status,
                        int vlan_priority, /* -1: untagged, 0..7: VLAN0 */
                        uint8_t *out,size_t capacity,size_t *written);
int openpnio_rtc1_decode(const uint8_t *frame,size_t size,
                        openpnio_rtc1_frame_t *out);
#ifdef __cplusplus
}
#endif
#endif
