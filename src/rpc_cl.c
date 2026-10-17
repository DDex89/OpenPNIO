#include "openpnio/rpc_cl.h"
#include <string.h>

uint16_t openpnio_rpc_cl_u16(const uint8_t *p,int little)
{
    if(little) return (uint16_t)(((uint16_t)p[1]<<8)|p[0]);
    return (uint16_t)(((uint16_t)p[0]<<8)|p[1]);
}
uint32_t openpnio_rpc_cl_u32(const uint8_t *p,int little)
{
    if(little)
        return ((uint32_t)p[3]<<24)|((uint32_t)p[2]<<16)|
               ((uint32_t)p[1]<<8)|p[0];
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|
           ((uint32_t)p[2]<<8)|p[3];
}
int openpnio_rpc_cl_parse(const uint8_t *p,size_t n,
                         openpnio_rpc_cl_header_t *out)
{
    int little;
    uint16_t len;
    if(!p || !out || n<OPENPNIO_RPC_CL_HEADER_SIZE || p[0]!=4 ||
       (p[4]!=0x10 && p[4]!=0x00) || p[5]!=0 || p[6]!=0)
        return -1;
    little=p[4]==0x10;
    len=openpnio_rpc_cl_u16(p+74,little);
    /* No trailing UDP bytes may be silently interpreted as another PDU;
     * unlike Ethernet capture, UDP preserves precise datagram length. */
    if(len != n-OPENPNIO_RPC_CL_HEADER_SIZE) return -1;
    memset(out,0,sizeof *out);
    out->packet_type=p[1];
    out->flags1=p[2];out->flags2=p[3];
    out->little_endian=(uint8_t)little;
    memcpy(out->object_uuid,p+8,16);
    memcpy(out->interface_uuid,p+24,16);
    memcpy(out->activity_uuid,p+40,16);
    out->server_boot=openpnio_rpc_cl_u32(p+56,little);
    out->interface_version=openpnio_rpc_cl_u32(p+60,little);
    out->sequence=openpnio_rpc_cl_u32(p+64,little);
    out->operation=openpnio_rpc_cl_u16(p+68,little);
    out->fragment_number=openpnio_rpc_cl_u16(p+76,little);
    out->body_length=len;
    out->serial=(uint16_t)(((uint16_t)p[7]<<8)|p[79]);
    out->auth_protocol=p[78];
    out->body=p+OPENPNIO_RPC_CL_HEADER_SIZE;
    return 0;
}
