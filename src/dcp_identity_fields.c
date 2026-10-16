#include "dcp_identity_fields.h"
#include <string.h>

const uint8_t openpnio_dcp_device_options[] = {
    1, 1, /* IP/MAC */
    1, 2, /* IP/parameters */
    2, 1, /* Device/type */
    2, 2, /* Device/name */
    2, 3, /* Device/vendor and device identifiers */
    2, 4, /* Device/role */
    2, 5, /* Device/supported options */
    2, 7  /* Device/instance */
};
const size_t openpnio_dcp_device_options_size = sizeof openpnio_dcp_device_options;

static void put16(uint8_t *p, uint16_t n)
{
    p[0]=(uint8_t)(n>>8); p[1]=(uint8_t)n;
}
static void put32(uint8_t *p, uint32_t n)
{
    p[0]=(uint8_t)(n>>24); p[1]=(uint8_t)(n>>16);
    p[2]=(uint8_t)(n>>8); p[3]=(uint8_t)n;
}
int openpnio_dcp_identity_field(const openpnio_dcp_identity_t *identity,
                               const uint8_t device_mac[6],
                               uint8_t option, uint8_t suboption,
                               openpnio_dcp_field_t *out)
{
    if(!identity || !device_mac || !out ||
       !identity->station_name || !identity->station_type) return -1;
    memset(out,0,sizeof *out);
    if(option==1 && suboption==1) {
        out->data=device_mac; out->size=6;
    } else if(option==1 && suboption==2) {
        memcpy(out->scratch,identity->ip,4);
        memcpy(out->scratch+4,identity->netmask,4);
        memcpy(out->scratch+8,identity->gateway,4);
        out->data=out->scratch; out->size=12;
        out->block_info=(identity->ip[0] || identity->ip[1] ||
                         identity->ip[2] || identity->ip[3]) ? 1 : 0;
    } else if(option==2 && suboption==1) {
        out->data=(const uint8_t*)identity->station_type;
        out->size=strlen(identity->station_type);
    } else if(option==2 && suboption==2) {
        out->data=(const uint8_t*)identity->station_name;
        out->size=strlen(identity->station_name);
    } else if(option==2 && suboption==3) {
        put16(out->scratch,identity->vendor_id);
        put16(out->scratch+2,identity->device_id);
        out->data=out->scratch; out->size=4;
    } else if(option==2 && suboption==4) {
        out->scratch[0]=1; /* IO Device role */
        out->scratch[1]=0; /* reserved, required in this block */
        out->data=out->scratch; out->size=2;
    } else if(option==2 && suboption==5) {
        out->data=openpnio_dcp_device_options;
        out->size=openpnio_dcp_device_options_size;
    } else if(option==2 && suboption==7) {
        put32(out->scratch,identity->device_instance);
        out->data=out->scratch; out->size=4;
    } else {
        return 0;
    }
    return out->size<=240 ? 1 : -1;
}
