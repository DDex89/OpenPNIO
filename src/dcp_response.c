#include "openpnio/dcp_response.h"
#include <string.h>
static void put16(uint8_t *p, uint16_t n) { p[0]=(uint8_t)(n>>8); p[1]=(uint8_t)n; }
static void put32(uint8_t *p, uint32_t n) {
    p[0]=(uint8_t)(n>>24); p[1]=(uint8_t)(n>>16);
    p[2]=(uint8_t)(n>>8); p[3]=(uint8_t)n;
}
static int add_block(uint8_t *out, size_t cap, size_t *pos,
                     uint8_t option, uint8_t suboption,
                     const uint8_t *value, size_t len)
{
    size_t pad = len & 1u;
    if (len > 65533u || *pos > cap || 6u + len + pad > cap - *pos) return -1;
    out[*pos] = option;
    out[*pos+1] = suboption;
    put16(out + *pos + 2, (uint16_t)(len+2u));
    put16(out + *pos + 4, 0); /* BlockInfo */
    if (len) memcpy(out + *pos + 6, value, len);
    if (pad) out[*pos + 6 + len] = 0;
    *pos += 6 + len + pad;
    return 0;
}
int openpnio_dcp_identify_response(const uint8_t dst_mac[6],
                                   const uint8_t src_mac[6],
                                   uint32_t xid,
                                   const openpnio_dcp_identity_t *identity,
                                   uint8_t *out, size_t capacity,
                                   size_t *written)
{
    size_t pos = 26, name_len, type_len;
    uint8_t ids[4], ip[12], role[1] = {1};
    if (!dst_mac || !src_mac || !identity || !out || !written ||
        !identity->station_name || !identity->station_type || capacity < 26)
        return -1;
    name_len = strlen(identity->station_name);
    type_len = strlen(identity->station_type);
    if (name_len > 240 || type_len > 240) return -1;
    /* Build into caller buffer; only publish written size on success. */
    memcpy(out, dst_mac, 6);
    memcpy(out+6, src_mac, 6);
    put16(out+12, 0x8892);
    put16(out+14, 0xfeff);
    out[16] = 5; /* Identify */
    out[17] = 1; /* ResponseSuccess */
    put32(out+18, xid);
    put16(out+22, 0); /* ResponseDelay */
    put16(out+24, 0); /* data length placeholder */
    put16(ids, identity->vendor_id);
    put16(ids+2, identity->device_id);
    memcpy(ip, identity->ip, 4);
    memcpy(ip+4, identity->netmask, 4);
    memcpy(ip+8, identity->gateway, 4);
    if (add_block(out,capacity,&pos,2,2,(const uint8_t*)identity->station_name,name_len) ||
        add_block(out,capacity,&pos,2,1,(const uint8_t*)identity->station_type,type_len) ||
        add_block(out,capacity,&pos,2,3,ids,sizeof ids) ||
        add_block(out,capacity,&pos,2,4,role,sizeof role) ||
        add_block(out,capacity,&pos,1,2,ip,sizeof ip))
        return -1;
    put16(out+24,(uint16_t)(pos-26));
    *written=pos;
    return 0;
}
