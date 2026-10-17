#include "openpnio/rtc1.h"
#include <string.h>
static uint16_t read16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0]<<8)|p[1]);
}
static void write16(uint8_t *p,uint16_t x)
{
    p[0]=(uint8_t)(x>>8);p[1]=(uint8_t)x;
}
static int is_rtc1(uint16_t id)
{
    return id>=0xc000 && id<=0xfaff;
}
int openpnio_rtc1_encode(const uint8_t dest[6],const uint8_t src[6],
                        uint16_t frame_id,const uint8_t *c_sdu,
                        size_t c_sdu_length,uint16_t cycle_counter,
                        uint8_t data_status,uint8_t transfer_status,
                        int vlan_priority,uint8_t *out,size_t capacity,
                        size_t *written)
{
    size_t base,padded,total;
    if (!dest || !src || !c_sdu || !out || !written ||
        !is_rtc1(frame_id) || c_sdu_length>1440 ||
        vlan_priority< -1 || vlan_priority>7) return -1;
    *written=0;
    /* Ethernet minimum without FCS: untagged 60, tagged 64 bytes. */
    padded=c_sdu_length<40?40:c_sdu_length;
    base=vlan_priority<0?14:18;
    total=base+2+padded+4;
    if (capacity<total) return -1;
    memcpy(out,dest,6);memcpy(out+6,src,6);
    if (vlan_priority>=0) {
        write16(out+12,0x8100);
        write16(out+14,(uint16_t)((unsigned)vlan_priority<<13));
        write16(out+16,0x8892);
    } else {
        write16(out+12,0x8892);
    }
    write16(out+base,frame_id);
    memcpy(out+base+2,c_sdu,c_sdu_length);
    if(padded>c_sdu_length)
        memset(out+base+2+c_sdu_length,0,padded-c_sdu_length);
    write16(out+base+2+padded,cycle_counter);
    out[base+2+padded+2]=data_status;
    out[base+2+padded+3]=transfer_status;
    *written=total;
    return 0;
}
int openpnio_rtc1_decode(const uint8_t *p,size_t n,
                         openpnio_rtc1_frame_t *out)
{
    size_t base=14,payload;
    uint16_t etype,vid;
    if(!p || !out || n<60) return -1;
    memset(out,0,sizeof *out);
    etype=read16(p+12);
    if (etype==0x8100) {
        if(n<64) return -1;
        vid=read16(p+14);
        if ((vid&0x1fffu)!=0) return -1; /* API: VLAN 0 priority tag only */
        out->tagged=1;
        out->vlan_priority=(uint8_t)(vid>>13);
        etype=read16(p+16);
        base=18;
    }
    if(etype!=0x8892 || n<base+2+40+4) return -1;
    out->frame_id=read16(p+base);
    if(!is_rtc1(out->frame_id)) return -1;
    memcpy(out->destination_mac,p,6);
    memcpy(out->source_mac,p+6,6);
    payload=n-base-2-4;
    if(payload<40 || payload>1440) return -1;
    out->c_sdu=p+base+2;
    out->c_sdu_length=payload;
    out->cycle_counter=read16(p+n-4);
    out->data_status=p[n-2];
    out->transfer_status=p[n-1];
    return 0;
}
