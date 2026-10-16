#include "openpnio/dcp_get.h"
#include "openpnio/dcp.h"
#include "openpnio/frame.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static const uint8_t mac[6] = {2,0,0,0,0,1};
static const openpnio_dcp_identity_t id = {
    .station_name="openpnio-test", .station_type="OpenPNIO Linux prototype",
    .vendor_id=0x1234, .device_id=0x5678,
    .ip={192,168,1,8},.netmask={255,255,255,0},.gateway={192,168,1,1}
};
static size_t get_request(uint8_t *p, uint8_t option, uint8_t suboption)
{
    memset(p, 0, 60);
    memcpy(p, mac, 6);
    memset(p+6, 0xaa, 6);
    p[12]=0x88; p[13]=0x92; p[14]=0xfe; p[15]=0xfd;
    p[16]=3; p[17]=0; p[18]=0x34; p[19]=0x0d;
    p[20]=0x35; p[21]=0x90;
    p[24]=0; p[25]=2;
    p[26]=option; p[27]=suboption;
    return 60;
}
static void check_value(const uint8_t *reply, size_t written,
                        uint8_t option, uint8_t suboption,
                        const uint8_t *expected, size_t length)
{
    openpnio_frame_t frame;
    openpnio_dcp_header_t h;
    openpnio_dcp_cursor_t cursor;
    openpnio_dcp_block_t b;
    assert(openpnio_parse_frame(reply,written,&frame)==OPENPNIO_FRAME_OK);
    assert(frame.frame_id==0xfefd);
    assert(memcmp(frame.src,mac,6)==0);
    assert(openpnio_dcp_parse_header(frame.payload,frame.payload_size,&h)==OPENPNIO_DCP_OK);
    assert(h.service_id==3 && h.service_type==1 && h.xid==0x340d3590);
    openpnio_dcp_cursor_init(&cursor,h.blocks,h.blocks_length);
    assert(openpnio_dcp_next_block(&cursor,&b)==1);
    assert(b.option==option && b.suboption==suboption);
    assert(b.data_length==length+2);
    assert(memcmp(b.data+2,expected,length)==0);
    assert(openpnio_dcp_next_block(&cursor,&b)==0);
}
int main(void)
{
    uint8_t request[60], reply[512], saved[512];
    const uint8_t ip[12]={192,168,1,8,255,255,255,0,192,168,1,1};
    const uint8_t ids[4]={0x12,0x34,0x56,0x78};
    size_t n,w=0;
    n=get_request(request,2,2);
    assert(openpnio_dcp_handle_get(request,n,mac,&id,reply,sizeof reply,&w)==1);
    check_value(reply,w,2,2,(const uint8_t *)"openpnio-test",12);
    n=get_request(request,1,2);
    assert(openpnio_dcp_handle_get(request,n,mac,&id,reply,sizeof reply,&w)==1);
    check_value(reply,w,1,2,ip,12);
    assert(reply[30]==0 && reply[31]==1); /* IPBlockInfo: configured */
    n=get_request(request,2,3);
    assert(openpnio_dcp_handle_get(request,n,mac,&id,reply,sizeof reply,&w)==1);
    check_value(reply,w,2,3,ids,4);
    n=get_request(request,2,1);
    assert(openpnio_dcp_handle_get(request,n,mac,&id,reply,sizeof reply,&w)==1);
    check_value(reply,w,2,1,(const uint8_t *)"OpenPNIO Linux prototype",24);
    n=get_request(request,2,4);
    assert(openpnio_dcp_handle_get(request,n,mac,&id,reply,sizeof reply,&w)==1);
    assert(reply[32]==1); /* IO Device */
    n=get_request(request,2,2);
    request[27]=9;
    assert(openpnio_dcp_handle_get(request,n,mac,&id,reply,sizeof reply,&w)==0);
    assert(w==0);
    request[27]=2;
    assert(openpnio_dcp_handle_get(request,n,mac,&id,reply,29,&w)==-2);
    assert(w==0);
    /* Truncated Get, irrespective of trailing Ethernet padding. */
    request[25]=61;
    assert(openpnio_dcp_handle_get(request,n,mac,&id,reply,sizeof reply,&w)==-1);
    n=get_request(request,2,2);
    request[14]=0xfe;request[15]=0xfe;
    assert(openpnio_dcp_handle_get(request,n,mac,&id,reply,sizeof reply,&w)==0);
    /* Multiple Get selectors share one XID, and their response is ordered. */
    n=get_request(request,2,2);
    request[25]=4;
    request[28]=1;request[29]=2;
    assert(openpnio_dcp_handle_get(request,n,mac,&id,reply,sizeof reply,&w)==1);
    memcpy(saved,reply,w);
    {
        openpnio_frame_t f;
        openpnio_dcp_header_t h;
        openpnio_dcp_cursor_t cursor;
        openpnio_dcp_block_t b;
        assert(openpnio_parse_frame(saved,w,&f)==OPENPNIO_FRAME_OK);
        assert(openpnio_dcp_parse_header(f.payload,f.payload_size,&h)==OPENPNIO_DCP_OK);
        openpnio_dcp_cursor_init(&cursor,h.blocks,h.blocks_length);
        assert(openpnio_dcp_next_block(&cursor,&b)==1 && b.option==2 && b.suboption==2);
        assert(openpnio_dcp_next_block(&cursor,&b)==1 && b.option==1 && b.suboption==2);
        assert(openpnio_dcp_next_block(&cursor,&b)==0);
    }
    puts("DCP Get tests passed");
    return 0;
}
