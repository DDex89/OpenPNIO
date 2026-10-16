#include "openpnio/dcp_device.h"
#include "openpnio/frame.h"
#include "openpnio/dcp.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static const uint8_t mac[6]={2,0,0,0,0,1};
static const openpnio_dcp_identity_t id={
    .station_name="openpnio",.station_type="OpenPNIO",
    .vendor_id=0x1234,.device_id=0x5678,
    .ip={192,168,1,42},.netmask={255,255,255,0},.gateway={192,168,1,1}
};
static size_t request(uint8_t *p,const char *name)
{
    size_t n = name ? strlen(name) : 0;
    size_t blocks = name ? 4+n+(n&1u) : 0;
    const uint8_t multicast[6]={1,0x0e,0xcf,0,0,0};
    memcpy(p,multicast,6);
    memset(p+6,0xaa,6);
    p[12]=0x88;p[13]=0x92;p[14]=0xfe;p[15]=0xfe;
    p[16]=5;p[17]=0;p[18]=0x12;p[19]=0x34;p[20]=0x56;p[21]=0x78;
    p[22]=0;p[23]=0;p[24]=(uint8_t)(blocks>>8);p[25]=(uint8_t)blocks;
    if(name) {
        p[26]=2;p[27]=2;p[28]=(uint8_t)(n>>8);p[29]=(uint8_t)n;
        memcpy(p+30,name,n);
        if(n&1u)p[30+n]=0;
    }
    return 26+blocks;
}
int main(void)
{
    uint8_t req[256],reply[512];
    size_t n,out=0;
    openpnio_frame_t frame;
    openpnio_dcp_header_t h;
    n=request(req,NULL);
    assert(openpnio_dcp_handle_identify(req,n,mac,&id,reply,sizeof reply,&out)==1);
    assert(out>26);
    assert(openpnio_parse_frame(reply,out,&frame)==OPENPNIO_FRAME_OK);
    assert(frame.frame_id==0xfeff && memcmp(frame.dst,req+6,6)==0);
    assert(openpnio_dcp_parse_header(frame.payload,frame.payload_size,&h)==OPENPNIO_DCP_OK);
    assert(h.xid==0x12345678);
    /* Real Windows/Scapy Identify All request: FF FF 00 00 followed
     * by zero Ethernet padding to a 60-byte frame (without FCS). */
    n=request(req,NULL);
    req[24]=0;req[25]=4;
    req[26]=0xff;req[27]=0xff;req[28]=0;req[29]=0;
    memset(req+30,0,30);
    n=60;
    assert(openpnio_dcp_handle_identify(req,n,mac,&id,reply,sizeof reply,&out)==1);
    assert(openpnio_parse_frame(reply,out,&frame)==OPENPNIO_FRAME_OK);
    assert(frame.frame_id==0xfeff && memcmp(frame.dst,req+6,6)==0);
    assert(openpnio_dcp_parse_header(frame.payload,frame.payload_size,&h)==OPENPNIO_DCP_OK);
    assert(h.xid==0x12345678);
    /* Nonempty Identify All is unsupported (not silently accepted). */
    req[28]=0;req[29]=2;req[24]=0;req[25]=6;
    assert(openpnio_dcp_handle_identify(req,n,mac,&id,reply,sizeof reply,&out)==0);
    assert(out==0);
    n=request(req,"openpnio");
    assert(openpnio_dcp_handle_identify(req,n,mac,&id,reply,sizeof reply,&out)==1);
    n=request(req,"another");
    assert(openpnio_dcp_handle_identify(req,n,mac,&id,reply,sizeof reply,&out)==0);
    assert(out==0);
    n=request(req,"OpenPNIO");
    req[27]=1; /* TypeOfStation selector */
    assert(openpnio_dcp_handle_identify(req,n,mac,&id,reply,sizeof reply,&out)==1);
    req[30]='X';
    assert(openpnio_dcp_handle_identify(req,n,mac,&id,reply,sizeof reply,&out)==0);
    n=request(req,NULL);
    req[24]=0;req[25]=8;req[26]=2;req[27]=3;
    req[28]=0;req[29]=4;
    req[30]=0x12;req[31]=0x34;req[32]=0x56;req[33]=0x78;
    n=34;
    assert(openpnio_dcp_handle_identify(req,n,mac,&id,reply,sizeof reply,&out)==1);
    req[33]=0x79;
    assert(openpnio_dcp_handle_identify(req,n,mac,&id,reply,sizeof reply,&out)==0);
    n=request(req,"openpnio");
    req[27]=3; /* Unsupported/mismatched DeviceID length */
    assert(openpnio_dcp_handle_identify(req,n,mac,&id,reply,sizeof reply,&out)==0);
    n=request(req,"openpnio");
    assert(openpnio_dcp_handle_identify(req,n-1,mac,&id,reply,sizeof reply,&out)==-1);
    n=request(req,NULL);
    assert(openpnio_dcp_handle_identify(req,n,mac,&id,reply,30,&out)==-2);
    req[14]=0xfe;req[15]=0xfd;
    assert(openpnio_dcp_handle_identify(req,n,mac,&id,reply,sizeof reply,&out)==0);
    puts("DCP Identify request handler tests passed");
    return 0;
}
