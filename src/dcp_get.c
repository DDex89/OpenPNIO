#include "openpnio/dcp_get.h"
#include "openpnio/dcp.h"
#include "openpnio/frame.h"
#include "dcp_identity_fields.h"
#include <string.h>

static void put16(uint8_t *p, uint16_t v)
{
    p[0]=(uint8_t)(v>>8);p[1]=(uint8_t)v;
}
static void put32(uint8_t *p, uint32_t v)
{
    p[0]=(uint8_t)(v>>24);p[1]=(uint8_t)(v>>16);
    p[2]=(uint8_t)(v>>8);p[3]=(uint8_t)v;
}
static int append(uint8_t *out, size_t cap, size_t *pos,
                  uint8_t option, uint8_t suboption,
                  const openpnio_dcp_field_t *field)
{
    size_t len=field->size,pad=len&1u,bytes=6u+len+pad;
    if (*pos<26 || *pos>cap || len>65533u || bytes>cap-*pos ||
        bytes>65535u || *pos-26u>65535u-bytes)
        return -1;
    out[*pos]=option; out[*pos+1]=suboption;
    put16(out+*pos+2,(uint16_t)(len+2u));
    put16(out+*pos+4,field->block_info);
    if (len) memcpy(out+*pos+6,field->data,len);
    if (pad) out[*pos+6+len]=0;
    *pos+=bytes;
    return 0;
}
int openpnio_dcp_handle_get(const uint8_t *request, size_t request_size,
                            const uint8_t device_mac[6],
                            const openpnio_dcp_identity_t *identity,
                            uint8_t *response, size_t capacity, size_t *written)
{
    openpnio_frame_t frame;
    openpnio_dcp_header_t header;
    size_t pos=26,i,count;
    if (!request || !device_mac || !identity || !identity->station_name ||
        !identity->station_type || !response || !written) return -1;
    *written=0;
    if(openpnio_parse_frame(request,request_size,&frame)!=OPENPNIO_FRAME_OK) return -1;
    if(frame.frame_id!=0xfefd || memcmp(frame.dst,device_mac,6)!=0) return 0;
    if(openpnio_dcp_parse_header(frame.payload,frame.payload_size,&header)!=OPENPNIO_DCP_OK)
        return -1;
    if(header.service_id!=3 || header.service_type!=0) return 0;
    /* Get requests contain two-byte option/suboption selectors, not TLVs. */
    if(header.blocks_length<2 || (header.blocks_length&1u)!=0) return -1;
    count=header.blocks_length/2;
    for(i=0;i<count;i++) {
        openpnio_dcp_field_t value;
        int rc=openpnio_dcp_identity_field(identity,device_mac,
                                          header.blocks[2*i],header.blocks[2*i+1],&value);
        if(rc<0) return -1;
        if(rc==0) return 0; /* All-or-nothing for unsupported selectors. */
    }
    if(capacity<26) return -2;
    memcpy(response,frame.src,6);
    memcpy(response+6,device_mac,6);
    put16(response+12,0x8892);
    put16(response+14,0xfefd);
    response[16]=3;response[17]=1;
    put32(response+18,header.xid);
    put16(response+22,0);
    put16(response+24,0);
    for(i=0;i<count;i++) {
        openpnio_dcp_field_t value;
        if(openpnio_dcp_identity_field(identity,device_mac,
                                      header.blocks[2*i],header.blocks[2*i+1],&value)!=1 ||
           append(response,capacity,&pos,
                  header.blocks[2*i],header.blocks[2*i+1],&value)!=0)
            return -2;
    }
    put16(response+24,(uint16_t)(pos-26));
    *written=pos;
    return 1;
}
