#include "openpnio/rtc1.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    const uint8_t src[6]={0xb8,0x27,0xeb,0xbb,0x9e,0x7a};
    const uint8_t dst[6]={1,2,3,4,5,6};
    const uint8_t c_sdu[3]={0xa5,0x80,0x80};
    openpnio_rtc1_frame_t r;
    uint8_t out[128];
    size_t n=0;
    assert(openpnio_rtc1_encode(dst,src,0xc000,c_sdu,3,0x1234,
                               0x35,0,-1,out,sizeof out,&n)==0);
    assert(n==60); /* 40 byte padded C_SDU, APDU status at end */
    assert(openpnio_rtc1_decode(out,n,&r)==0);
    assert(r.frame_id==0xc000 && r.tagged==0);
    assert(r.c_sdu_length==40 && memcmp(r.c_sdu,c_sdu,3)==0);
    assert(r.c_sdu[39]==0 && r.cycle_counter==0x1234);
    assert(r.data_status==0x35 && r.transfer_status==0);
    assert(openpnio_rtc1_encode(dst,src,0xfaff,c_sdu,3,0xffff,
                               0x35,0,6,out,sizeof out,&n)==0);
    assert(n==64);
    assert(openpnio_rtc1_decode(out,n,&r)==0);
    assert(r.tagged==1 && r.vlan_priority==6 && r.cycle_counter==0xffff);
    out[15]=1; /* VLAN ID 1 is not our negotiated priority-tag mode. */
    assert(openpnio_rtc1_decode(out,n,&r)==-1);
    out[15]=0;
    assert(openpnio_rtc1_decode(out,n,&r)==0);
    assert(openpnio_rtc1_encode(dst,src,0xfefe,c_sdu,3,0,0,0,-1,
                               out,sizeof out,&n)==-1);
    assert(openpnio_rtc1_encode(dst,src,0xc000,c_sdu,3,0,0,0,8,
                               out,sizeof out,&n)==-1);
    assert(openpnio_rtc1_encode(dst,src,0xc000,c_sdu,3,0,0,0,-1,
                               out,59,&n)==-1);
    out[18]=0xfe;out[19]=0xfe;
    assert(openpnio_rtc1_decode(out,64,&r)==-1);
    puts("RTC1 Ethernet framing tests passed");
    return 0;
}
