#include "openpnio/dcp_set.h"
#include "openpnio/dcp.h"
#include "openpnio/frame.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct store {char name[241];unsigned calls;uint16_t qualifier;} store_t;
static int save(void *context,const char *name,size_t length,uint16_t qualifier)
{
    store_t *s=(store_t*)context;
    memcpy(s->name,name,length+1);
    s->calls++;
    s->qualifier=qualifier;
    return 0;
}
static size_t make_request(uint8_t *p,const char *name,uint8_t option,uint8_t suboption)
{
    size_t n=strlen(name), body=2+n, padded=body+(body&1u);
    static const uint8_t mac[6]={2,0,0,0,0,1};
    memcpy(p,mac,6);memset(p+6,0xaa,6);
    p[12]=0x88;p[13]=0x92;p[14]=0xfe;p[15]=0xfd;
    p[16]=4;p[17]=0;p[18]=0x12;p[19]=0x34;p[20]=0x56;p[21]=0x78;
    p[22]=0;p[23]=0;p[24]=0;p[25]=(uint8_t)(4+padded);
    p[26]=option;p[27]=suboption;p[28]=0;p[29]=(uint8_t)body;
    p[30]=0;p[31]=1;
    memcpy(p+32,name,n);
    if(body&1u)p[32+n]=0;
    return 30+padded;
}
int main(void)
{
    uint8_t req[300],res[300];
    const uint8_t mac[6]={2,0,0,0,0,1};
    store_t store={{0},0,0};
    openpnio_frame_t frame;
    openpnio_dcp_header_t hdr;
    openpnio_dcp_cursor_t cursor;
    openpnio_dcp_block_t block;
    size_t n,w=0;
    n=make_request(req,"openpnio-1",2,2);
    assert(openpnio_dcp_handle_set(req,n,mac,save,&store,res,sizeof res,&w)==1);
    assert(store.calls==1 && strcmp(store.name,"openpnio-1")==0 && store.qualifier==1);
    assert(openpnio_parse_frame(res,w,&frame)==OPENPNIO_FRAME_OK);
    assert(frame.frame_id==0xfefd);
    assert(openpnio_dcp_parse_header(frame.payload,frame.payload_size,&hdr)==OPENPNIO_DCP_OK);
    assert(hdr.service_id==4 && hdr.service_type==1 && hdr.xid==0x12345678);
    openpnio_dcp_cursor_init(&cursor,hdr.blocks,hdr.blocks_length);
    assert(openpnio_dcp_next_block(&cursor,&block)==1);
    assert(block.option==5 && block.suboption==4 && block.data_length==3);
    assert(block.data[0]==2 && block.data[1]==2 && block.data[2]==0);
    assert(openpnio_dcp_next_block(&cursor,&block)==0);
    n=make_request(req,"UPPER",2,2);
    assert(openpnio_dcp_handle_set(req,n,mac,save,&store,res,sizeof res,&w)==1);
    assert(store.calls==1);
    n=make_request(req,"openpnio",2,3);
    assert(openpnio_dcp_handle_set(req,n,mac,save,&store,res,sizeof res,&w)==1);
    assert(store.calls==1);
    n=make_request(req,"openpnio",2,2);
    assert(openpnio_dcp_handle_set(req,n-1,mac,save,&store,res,sizeof res,&w)==-1);
    assert(store.calls==1);
    assert(openpnio_dcp_handle_set(req,n,mac,save,&store,res,30,&w)==-1);
    assert(store.calls==1);
    /* The temporary qualifier 0 is passed through to application storage. */
    n=make_request(req,"temp-name",2,2);
    req[31]=0;
    assert(openpnio_dcp_handle_set(req,n,mac,save,&store,res,sizeof res,&w)==1);
    assert(store.calls==2 && store.qualifier==0);
    /* Unsupported qualifiers must never call the application. */
    n=make_request(req,"openpnio",2,2);
    req[31]=2;
    assert(openpnio_dcp_handle_set(req,n,mac,save,&store,res,sizeof res,&w)==1);
    assert(res[32]==5 && store.calls==2);
    /* Two valid blocks are refused before either setter can run; no
     * partially applied station configuration is allowed. */
    n=make_request(req,"first-name",2,2);
    memcpy(req+n,req+26,n-26);
    req[25]=(uint8_t)(2*(n-26));
    n+=n-26;
    assert(openpnio_dcp_handle_set(req,n,mac,save,&store,res,sizeof res,&w)==-1);
    assert(store.calls==2);
    puts("DCP Set station-name tests passed");
    return 0;
}
