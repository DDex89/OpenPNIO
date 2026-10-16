#include "openpnio/frame.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
int main(void)
{
    static const uint8_t plain[] = {
        0,1,2,3,4,5, 6,7,8,9,10,11, 0x88,0x92, 0xfe,0xfe, 0x11,0x22
    };
    static const uint8_t tagged[] = {
        0,1,2,3,4,5, 6,7,8,9,10,11,
        0x81,0x00, 0x60,0x01, 0x88,0x92, 0xfe,0xfd, 0x33
    };
    static const uint8_t non_pn[] = {
        0,1,2,3,4,5, 6,7,8,9,10,11, 0x08,0x00, 0x00,0x00
    };
    openpnio_frame_t f = {0};
    assert(openpnio_parse_frame(plain, sizeof plain, &f) == OPENPNIO_FRAME_OK);
    assert(f.frame_id == 0xfefe && f.has_vlan == 0 && f.payload_size == 2);
    assert(f.payload[0] == 0x11 && f.payload[1] == 0x22);
    assert(openpnio_parse_frame(tagged, sizeof tagged, &f) == OPENPNIO_FRAME_OK);
    assert(f.frame_id == 0xfefd && f.has_vlan == 1 && f.vlan_tci == 0x6001);
    assert(f.payload_size == 1 && f.payload[0] == 0x33);
    assert(openpnio_parse_frame(non_pn, sizeof non_pn, &f) == OPENPNIO_FRAME_NOT_PROFINET);
    assert(openpnio_parse_frame(plain, 13, &f) == OPENPNIO_FRAME_SHORT);
    assert(openpnio_parse_frame(plain, 15, &f) == OPENPNIO_FRAME_SHORT);
    assert(openpnio_parse_frame(NULL, 0, &f) == OPENPNIO_FRAME_SHORT);
    puts("frame parser tests passed");
    return 0;
}
