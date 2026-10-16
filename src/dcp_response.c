#include "openpnio/dcp_response.h"
#include "dcp_identity_fields.h"
#include <string.h>

static void put16(uint8_t *p, uint16_t n)
{
    p[0]=(uint8_t)(n>>8); p[1]=(uint8_t)n;
}
static void put32(uint8_t *p, uint32_t n)
{
    p[0]=(uint8_t)(n>>24); p[1]=(uint8_t)(n>>16);
    p[2]=(uint8_t)(n>>8); p[3]=(uint8_t)n;
}
static int add_block(uint8_t *out, size_t cap, size_t *pos,
                     uint8_t option, uint8_t suboption,
                     const openpnio_dcp_field_t *field)
{
    size_t len=field->size, pad=len&1u, bytes=6u+len+pad;
    if (len>65533u || *pos<26 || *pos>cap ||
        bytes>cap-*pos || bytes>65535u || *pos-26u>65535u-bytes)
        return -1;
    out[*pos]=option;
    out[*pos+1]=suboption;
    put16(out+*pos+2,(uint16_t)(len+2u));
    put16(out+*pos+4,field->block_info);
    if (len) memcpy(out+*pos+6,field->data,len);
    if (pad) out[*pos+6+len]=0;
    *pos+=bytes;
    return 0;
}
int openpnio_dcp_identify_response(const uint8_t dst_mac[6],
                                   const uint8_t src_mac[6],
                                   uint32_t xid,
                                   const openpnio_dcp_identity_t *identity,
                                   uint8_t *out, size_t capacity,
                                   size_t *written)
{
    /* These standard Identify fields are selected from the same registry
     * as DCP Get. Avoid advertising unimplemented device options. */
    static const uint8_t fields[][2]={
        {2,2}, /* NameOfStation */
        {2,5}, /* DeviceOptions */
        {2,1}, /* TypeOfStation */
        {2,3}, /* DeviceID */
        {2,4}, /* DeviceRole */
        {2,7}, /* DeviceInstance */
        {1,2}  /* IP parameters */
    };
    size_t pos=26,i;
    if (!dst_mac || !src_mac || !identity || !out || !written ||
        !identity->station_name || !identity->station_type || capacity<26)
        return -1;
    *written=0;
    memcpy(out,dst_mac,6);
    memcpy(out+6,src_mac,6);
    put16(out+12,0x8892);
    put16(out+14,0xfeff);
    out[16]=5; out[17]=1;
    put32(out+18,xid);
    put16(out+22,0);
    put16(out+24,0);
    for (i=0;i<sizeof fields/sizeof fields[0];i++) {
        openpnio_dcp_field_t value;
        int rc=openpnio_dcp_identity_field(identity,src_mac,
                                          fields[i][0],fields[i][1],&value);
        if (rc!=1 || add_block(out,capacity,&pos,fields[i][0],fields[i][1],&value)!=0)
            return -1;
    }
    put16(out+24,(uint16_t)(pos-26));
    *written=pos;
    return 0;
}
