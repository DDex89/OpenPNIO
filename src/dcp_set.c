#include "openpnio/dcp_set.h"
#include "openpnio/dcp.h"
#include "openpnio/frame.h"
#include <string.h>
static uint16_t be16(const uint8_t *p) { return (uint16_t)(((uint16_t)p[0]<<8)|p[1]); }
static void put16(uint8_t *p,uint16_t v) {p[0]=(uint8_t)(v>>8);p[1]=(uint8_t)v;}
static void put32(uint8_t *p,uint32_t v) {
    p[0]=(uint8_t)(v>>24);p[1]=(uint8_t)(v>>16);
    p[2]=(uint8_t)(v>>8);p[3]=(uint8_t)v;
}
static int valid_name(const uint8_t *p,size_t n)
{
    size_t i;
    if (n==0 || n>240 || p[0]=='-' || p[n-1]=='-') return 0;
    for(i=0;i<n;i++) {
        uint8_t c=p[i];
        if (!((c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='-'||c=='.'))
            return 0;
        if (c=='.' && (i==0 || i==n-1 || p[i-1]=='.' || p[i-1]=='-' ||
                       p[i+1]=='-')) return 0;
    }
    return 1;
}
int openpnio_dcp_handle_set(const uint8_t *request,size_t request_size,
                           const uint8_t device_mac[6],
                           openpnio_station_name_setter_t setter,void *context,
                           uint8_t *response,size_t capacity,size_t *written)
{
    openpnio_frame_t frame;
    openpnio_dcp_header_t hdr;
    openpnio_dcp_cursor_t cursor;
    openpnio_dcp_block_t block;
    size_t pos=26;
    int next;
    if(!request||!device_mac||!response||!written) return -1;
    *written=0;
    if(openpnio_parse_frame(request,request_size,&frame)!=OPENPNIO_FRAME_OK) return -1;
    if(frame.frame_id!=0xfefd || memcmp(frame.dst,device_mac,6)!=0) return 0;
    if(openpnio_dcp_parse_header(frame.payload,frame.payload_size,&hdr)!=OPENPNIO_DCP_OK) return -1;
    if(hdr.service_id!=4 || hdr.service_type!=0) return 0;
    /* Preflight all blocks before any side effects. Reject incomplete or oversized
       requests rather than applying only their first part. */
    openpnio_dcp_cursor_init(&cursor,hdr.blocks,hdr.blocks_length);
    while((next=openpnio_dcp_next_block(&cursor,&block))>0) {
        if(block.data_length<2 || pos>capacity || capacity-pos<10) return -1;
        pos+=10; /* Control/Response block: 4 header + 3 bytes + padding */
    }
    if(next<0 || pos-26>65535u || capacity<pos) return -1;
    memcpy(response,frame.src,6);
    memcpy(response+6,device_mac,6);
    put16(response+12,0x8892);
    put16(response+14,0xfefd);
    response[16]=4;response[17]=1;
    put32(response+18,hdr.xid);
    put16(response+22,0);
    put16(response+24,(uint16_t)(pos-26));
    pos=26;
    openpnio_dcp_cursor_init(&cursor,hdr.blocks,hdr.blocks_length);
    while((next=openpnio_dcp_next_block(&cursor,&block))>0) {
        uint8_t status=OPENPNIO_SET_OPTION_UNSUPPORTED;
        if(block.option==2) {
            status=OPENPNIO_SET_SUBOPTION_UNSUPPORTED;
            if(block.suboption==2) {
                const uint8_t *name=block.data+2;
                size_t n=block.data_length-2;
                uint16_t qualifier=be16(block.data);
                status=OPENPNIO_SET_NOT_POSSIBLE;
                if(setter && valid_name(name,n)) {
                    char temp[241];
                    memcpy(temp,name,n);
                    temp[n]='\0';
                    if(setter(context,temp,n,qualifier)==0) status=OPENPNIO_SET_OK;
                }
            }
        }
        response[pos]=5;response[pos+1]=4;
        put16(response+pos+2,3);
        response[pos+4]=block.option;
        response[pos+5]=block.suboption;
        response[pos+6]=status;
        response[pos+7]=0; /* even-byte padding */
        pos+=8;
    }
    /* Actual response block length is 8 bytes, not 10. */
    put16(response+24,(uint16_t)(pos-26));
    *written=pos;
    return 1;
}
