#include "openpnio/lldp.h"
#include <string.h>

static const uint8_t lldp_dst[6]={0x01,0x80,0xc2,0x00,0x00,0x0e};
static void be16(uint8_t *p,uint16_t x)
{
    p[0]=(uint8_t)(x>>8);p[1]=(uint8_t)x;
}
static uint16_t read16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0]<<8)|p[1]);
}
static int append(uint8_t *p,size_t capacity,size_t *offset,
                  unsigned type,const uint8_t *value,size_t length)
{
    if (length>511u || type>127u || *offset>capacity ||
        capacity-*offset<2u+length) return -1;
    be16(p+*offset,(uint16_t)(type*512u+length));
    if (length) memcpy(p+*offset+2,value,length);
    *offset+=2u+length;
    return 0;
}
int openpnio_lldp_build(const uint8_t src_mac[6],const char *station_name,
                       uint16_t ttl_seconds,uint8_t *out,size_t capacity,
                       size_t *written)
{
    uint8_t chassis[242],port[256],ttl[2];
    size_t name_len,off=14;
    static const char prefix[]="port-001.";
    const size_t prefix_len=sizeof(prefix)-1;
    if (!src_mac || !station_name || !out || !written) return -1;
    *written=0;
    name_len=strlen(station_name);
    if (!name_len || name_len>240 || ttl_seconds==0 ||
        prefix_len+name_len>254 || capacity<60) return -1;
    /* Locally assigned chassis and port identifiers. The PN v2.3
     * topology extensions are intentionally NOT claimed here. */
    chassis[0]=7;
    memcpy(chassis+1,station_name,name_len);
    port[0]=7;
    memcpy(port+1,prefix,prefix_len);
    memcpy(port+1+prefix_len,station_name,name_len);
    be16(ttl,ttl_seconds);
    memcpy(out,lldp_dst,6);
    memcpy(out+6,src_mac,6);
    be16(out+12,0x88cc);
    if (append(out,capacity,&off,1,chassis,name_len+1)!=0 ||
        append(out,capacity,&off,2,port,1+prefix_len+name_len)!=0 ||
        append(out,capacity,&off,3,ttl,2)!=0 ||
        append(out,capacity,&off,5,(const uint8_t*)station_name,name_len)!=0 ||
        append(out,capacity,&off,0,0,0)!=0) return -1;
    if (off<60) {memset(out+off,0,60-off);off=60;}
    *written=off;
    return 0;
}
int openpnio_lldp_parse(const uint8_t *p,size_t size,
                       openpnio_lldp_frame_t *out)
{
    size_t off=14;
    unsigned expect=1;
    int ended=0;
    if (!p || !out || size<14 || read16(p+12)!=0x88cc) return -1;
    memset(out,0,sizeof *out);
    memcpy(out->src_mac,p+6,6);
    while (off+2<=size) {
        uint16_t hdr=read16(p+off);
        unsigned type=hdr>>9;
        size_t n=hdr&511u;
        const uint8_t *data;
        off+=2;
        if (n>size-off) return -1;
        data=p+off;
        if (type==0) {
            size_t i;
            if (n!=0 || expect!=4) return -1;
            for (i=off;i<size;i++) if (p[i]!=0) return -1;
            ended=1;
            break;
        }
        if (expect<=3 && type!=expect) return -1;
        if (type==1) {
            if (n<2 || data[0]>7 || data[0]==0) return -1;
            out->chassis_id=data+1;out->chassis_id_length=n-1;
            expect=2;
        } else if (type==2) {
            if (n<2 || data[0]>7 || data[0]==0) return -1;
            out->port_id=data+1;out->port_id_length=n-1;
            expect=3;
        } else if (type==3) {
            if (n!=2) return -1;
            out->ttl_seconds=read16(data);
            expect=4;
        } else if (type==1 || type==2 || type==3) {
            return -1;
        }
        off+=n;
    }
    return ended?0:-1;
}
