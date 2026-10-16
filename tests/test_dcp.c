#include "openpnio/dcp.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
int main(void)
{
    static const uint8_t packet[] = {
        5,0, 0x12,0x34,0x56,0x78, 0,0, 0,8,
        2,2, 0,3, 0x61,0x62,0x63,0
    };
    openpnio_dcp_header_t h;
    openpnio_dcp_cursor_t cursor;
    openpnio_dcp_block_t b;
    assert(openpnio_dcp_parse_header(packet,sizeof packet,&h)==OPENPNIO_DCP_OK);
    assert(h.service_id==5 && h.xid==0x12345678 && h.blocks_length==8);
    openpnio_dcp_cursor_init(&cursor,h.blocks,h.blocks_length);
    assert(openpnio_dcp_next_block(&cursor,&b)==1);
    assert(b.option==2 && b.suboption==2 && b.data_length==3 && b.data[2]=='c');
    assert(openpnio_dcp_next_block(&cursor,&b)==0);
    assert(openpnio_dcp_parse_header(packet,12,&h)==OPENPNIO_DCP_TRUNCATED);
    {
        static const uint8_t bad_pad[] = {2,2,0,1,0x61};
        openpnio_dcp_cursor_init(&cursor,bad_pad,sizeof bad_pad);
        assert(openpnio_dcp_next_block(&cursor,&b)==OPENPNIO_DCP_TRUNCATED);
    }
    puts("DCP codec tests passed");
    return 0;
}
