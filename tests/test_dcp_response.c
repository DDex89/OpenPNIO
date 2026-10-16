#include "openpnio/dcp_response.h"
#include "openpnio/frame.h"
#include "openpnio/dcp.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    static const uint8_t dst[6]={1,2,3,4,5,6};
    static const uint8_t src[6]={6,5,4,3,2,1};
    static const uint8_t expected_options[16]={
        1,1,1,2,2,1,2,2,2,3,2,4,2,5,2,7
    };
    const openpnio_dcp_identity_t id={
        .station_name="openpnio",.station_type="OpenPNIO",
        .vendor_id=0x1234,.device_id=0x5678,.device_instance=0x1234,
        .ip={192,168,1,42},.netmask={255,255,255,0},.gateway={192,168,1,1}
    };
    uint8_t buf[512];
    size_t size=0;
    openpnio_frame_t f;
    openpnio_dcp_header_t h;
    openpnio_dcp_cursor_t c;
    openpnio_dcp_block_t b;
    int count=0;
    assert(openpnio_dcp_identify_response(dst,src,0x11223344,&id,buf,sizeof buf,&size)==0);
    assert(openpnio_parse_frame(buf,size,&f)==OPENPNIO_FRAME_OK);
    assert(f.frame_id==0xfeff);
    assert(memcmp(f.dst,dst,6)==0 && memcmp(f.src,src,6)==0);
    assert(openpnio_dcp_parse_header(f.payload,f.payload_size,&h)==OPENPNIO_DCP_OK);
    assert(h.service_id==5 && h.service_type==1 && h.xid==0x11223344);
    openpnio_dcp_cursor_init(&c,h.blocks,h.blocks_length);
    while(openpnio_dcp_next_block(&c,&b)==1) {
        if (count==0) {
            assert(b.option==2 && b.suboption==2 && b.data_length==10);
            assert(b.data[0]==0 && b.data[1]==0);
            assert(memcmp(b.data+2,"openpnio",8)==0);
        } else if (count==1) {
            assert(b.option==2 && b.suboption==5 && b.data_length==18);
            assert(memcmp(b.data+2,expected_options,sizeof expected_options)==0);
        } else if (count==4) {
            assert(b.option==2 && b.suboption==4 && b.data_length==4);
            assert(b.data[2]==1 && b.data[3]==0);
        } else if (count==5) {
            assert(b.option==2 && b.suboption==7 && b.data_length==4);
            assert(b.data[2]==0x12 && b.data[3]==0x34);
        } else if (count==6) {
            assert(b.option==1 && b.suboption==2 && b.data_length==14);
            assert(b.data[0]==0 && b.data[1]==1);
        }
        ++count;
    }
    assert(count==7);
    assert(openpnio_dcp_identify_response(dst,src,0,&id,buf,30,&size)!=0);
    puts("DCP Identify response metadata tests passed");
    return 0;
}
