#include "openpnio/rpc_cl.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    uint8_t packet[85]={0};
    openpnio_rpc_cl_header_t hdr;
    packet[0]=4;
    packet[1]=0; /* request */
    packet[2]=0x02;
    packet[4]=0x10; /* little-endian drep */
    packet[7]=0x12;
    packet[24]=0xab; /* interface uuid start */
    packet[40]=0xcd; /* activity uuid start */
    packet[60]=2; /* interface version */
    packet[64]=0x78;packet[65]=0x56;packet[66]=0x34;packet[67]=0x12;
    packet[68]=0x01;packet[69]=0x00; /* operation */
    packet[74]=5; packet[79]=0x34;
    memcpy(packet+80,"hello",5);
    assert(openpnio_rpc_cl_parse(packet,sizeof packet,&hdr)==0);
    assert(hdr.packet_type==0 && hdr.sequence==0x12345678);
    assert(hdr.little_endian && hdr.interface_version==2 && hdr.operation==1);
    assert(hdr.body_length==5 && memcmp(hdr.body,"hello",5)==0);
    assert(hdr.interface_uuid[0]==0xab && hdr.activity_uuid[0]==0xcd);
    assert(hdr.serial==0x1234);
    assert(openpnio_rpc_cl_parse(packet,84,&hdr)==-1);
    packet[0]=5;
    assert(openpnio_rpc_cl_parse(packet,sizeof packet,&hdr)==-1);
    packet[0]=4;
    packet[4]=0; /* big-endian */
    packet[60]=0;packet[61]=0;packet[62]=0;packet[63]=2;
    packet[64]=0x12;packet[65]=0x34;packet[66]=0x56;packet[67]=0x78;
    packet[68]=0;packet[69]=1;
    packet[74]=0;packet[75]=5;
    assert(openpnio_rpc_cl_parse(packet,sizeof packet,&hdr)==0);
    assert(!hdr.little_endian && hdr.operation==1);
    assert(hdr.sequence==0x12345678 && hdr.interface_version==2);
    packet[75]=6;
    assert(openpnio_rpc_cl_parse(packet,sizeof packet,&hdr)==-1);
    packet[4]=0x11; /* unsupported drep */
    assert(openpnio_rpc_cl_parse(packet,sizeof packet,&hdr)==-1);
    puts("DCE/RPC connectionless header parser tests passed");
    return 0;
}
