#include "openpnio/dcp_set.h"
#include "openpnio/dcp.h"
#include "openpnio/frame.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct {uint8_t ip[4],mask[4],gw[4];uint16_t qualifier;int calls;} state_t;
static int apply(void *ctx,const uint8_t ip[4],const uint8_t mask[4],
                 const uint8_t gw[4],uint16_t qualifier)
{
    state_t *s=(state_t*)ctx;
    memcpy(s->ip,ip,4);memcpy(s->mask,mask,4);memcpy(s->gw,gw,4);
    s->qualifier=qualifier;s->calls++;
    return 0;
}
int main(void)
{
    uint8_t req[128]={0},res[128];
    const uint8_t mac[6]={2,0,0,0,0,1};
    const uint8_t ip[12]={192,168,1,42,255,255,255,0,192,168,1,1};
    state_t state={{0},{0},{0},0,0};
    openpnio_dcp_set_handlers_t handlers={0,apply,&state};
    openpnio_frame_t frame;
    openpnio_dcp_header_t hdr;
    openpnio_dcp_cursor_t cursor;
    openpnio_dcp_block_t block;
    size_t written=0;
    memcpy(req,mac,6);memset(req+6,0xaa,6);
    req[12]=0x88;req[13]=0x92;req[14]=0xfe;req[15]=0xfd;
    req[16]=4;req[17]=0;req[21]=7;req[24]=0;req[25]=18;
    req[26]=1;req[27]=2;req[28]=0;req[29]=14;
    req[30]=0;req[31]=1;
    memcpy(req+32,ip,12);
    assert(openpnio_dcp_handle_set_ex(req,sizeof req,mac,&handlers,res,sizeof res,&written)==1);
    assert(state.calls==1 && state.qualifier==1);
    assert(memcmp(state.ip,ip,4)==0 && memcmp(state.mask,ip+4,4)==0 &&
           memcmp(state.gw,ip+8,4)==0);
    assert(openpnio_parse_frame(res,written,&frame)==OPENPNIO_FRAME_OK);
    assert(openpnio_dcp_parse_header(frame.payload,frame.payload_size,&hdr)==OPENPNIO_DCP_OK);
    openpnio_dcp_cursor_init(&cursor,hdr.blocks,hdr.blocks_length);
    assert(openpnio_dcp_next_block(&cursor,&block)==1);
    assert(block.option==5 && block.suboption==4 && block.data_length==3);
    assert(block.data[0]==1 && block.data[1]==2 && block.data[2]==0);
    assert(openpnio_dcp_next_block(&cursor,&block)==0);
    assert(openpnio_dcp_handle_set_ex(req,43,mac,&handlers,res,sizeof res,&written)==-1);
    assert(state.calls==1);
    assert(openpnio_dcp_handle_set_ex(req,sizeof req,mac,&handlers,res,30,&written)==-1);
    assert(state.calls==1);
    req[31]=2; /* unknown DCP qualifier */
    assert(openpnio_dcp_handle_set_ex(req,44,mac,&handlers,res,sizeof res,&written)==1);
    assert(res[32]==5 && state.calls==1);
    req[31]=1;
    /* Reject multi-block Set before callback; never apply partially. */
    memcpy(req+44,req+26,18);
    req[25]=36;
    assert(openpnio_dcp_handle_set_ex(req,62,mac,&handlers,res,sizeof res,&written)==-1);
    assert(state.calls==1);
    req[25]=18;
    handlers.set_ip_suite=0;
    assert(openpnio_dcp_handle_set_ex(req,sizeof req,mac,&handlers,res,sizeof res,&written)==1);
    assert(state.calls==1);
    puts("DCP IP Set tests passed");
    return 0;
}
