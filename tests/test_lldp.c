#include "openpnio/lldp.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    const uint8_t mac[6]={0xb8,0x27,0xeb,0xbb,0x9e,0x7a};
    uint8_t buffer[1024],broken[1024];
    size_t n=0;
    openpnio_lldp_frame_t ll;
    assert(openpnio_lldp_build(mac,"testnode",120,buffer,sizeof buffer,&n)==0);
    assert(n>=60);
    assert(buffer[12]==0x88 && buffer[13]==0xcc);
    assert(openpnio_lldp_parse(buffer,n,&ll)==0);
    assert(memcmp(ll.src_mac,mac,6)==0 && ll.ttl_seconds==120);
    assert(ll.chassis_id_length==8);
    assert(memcmp(ll.chassis_id,"testnode",8)==0);
    assert(ll.port_id_length==17);
    assert(memcmp(ll.port_id,"port-001.testnode",17)==0);
    assert(openpnio_lldp_parse(buffer,15,&ll)==-1);
    memcpy(broken,buffer,n);
    broken[14]=0xff;broken[15]=0xff;
    assert(openpnio_lldp_parse(broken,n,&ll)==-1);
    memcpy(broken,buffer,n);
    broken[n-1]=1; /* nonzero bytes following End TLV */
    assert(openpnio_lldp_parse(broken,n,&ll)==-1);
    assert(openpnio_lldp_build(mac,"testnode",0,buffer,sizeof buffer,&n)==-1);
    assert(openpnio_lldp_build(mac,"testnode",120,buffer,30,&n)==-1);
    puts("LLDP frame codec tests passed");
    return 0;
}
