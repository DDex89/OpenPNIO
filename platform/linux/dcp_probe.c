/* Experimental Linux AF_PACKET DCP Identify responder.
 * Run only on an isolated test network with a dedicated Ethernet interface.
 * Does not process DCP Set or change any network configuration.
 */
#define _POSIX_C_SOURCE 200809L
#include "openpnio/dcp_device.h"
#include <errno.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
static volatile sig_atomic_t running=1;
static void stop(int sig) { (void)sig; running=0; }
int main(int argc,char **argv)
{
    int fd,ifindex,rc;
    struct ifreq ifr;
    struct sockaddr_ll bind_addr;
    struct pollfd pfd;
    uint8_t rx[2048],tx[2048],mac[6];
    size_t written;
    openpnio_dcp_identity_t identity={
        .station_name="openpnio-test",.station_type="OpenPNIO Linux prototype",
        .vendor_id=0,.device_id=0,
        .ip={0,0,0,0},.netmask={0,0,0,0},.gateway={0,0,0,0}
    };
    if(argc!=2) {
        fprintf(stderr,"Usage: %s <ethernet-interface>\n",argv[0]);
        return 2;
    }
    if(strlen(argv[1])>=IFNAMSIZ) {
        fprintf(stderr,"Interface name too long\n");
        return 2;
    }
    fd=socket(AF_PACKET,SOCK_RAW,htons(0x8892));
    if(fd<0) {perror("socket(AF_PACKET)");return 1;}
    memset(&ifr,0,sizeof ifr);
    memcpy(ifr.ifr_name,argv[1],strlen(argv[1])+1);
    if(ioctl(fd,SIOCGIFINDEX,&ifr)<0) {perror("SIOCGIFINDEX");close(fd);return 1;}
    ifindex=ifr.ifr_ifindex;
    if(ioctl(fd,SIOCGIFHWADDR,&ifr)<0) {perror("SIOCGIFHWADDR");close(fd);return 1;}
    memcpy(mac,ifr.ifr_hwaddr.sa_data,6);
    memset(&bind_addr,0,sizeof bind_addr);
    bind_addr.sll_family=AF_PACKET;
    bind_addr.sll_protocol=htons(0x8892);
    bind_addr.sll_ifindex=ifindex;
    if(bind(fd,(struct sockaddr*)&bind_addr,sizeof bind_addr)<0) {
        perror("bind");close(fd);return 1;
    }
    signal(SIGINT,stop);
    signal(SIGTERM,stop);
    pfd.fd=fd;pfd.events=POLLIN;
    printf("OpenPNIO DCP Identify test on %s, MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
           argv[1],mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    while(running) {
        struct sockaddr_ll peer;
        socklen_t peer_len=sizeof peer;
        ssize_t n;
        rc=poll(&pfd,1,250);
        if(rc<0) {if(errno==EINTR)continue;perror("poll");break;}
        if(rc==0 || !(pfd.revents&POLLIN))continue;
        memset(&peer,0,sizeof peer);
        n=recvfrom(fd,rx,sizeof rx,0,(struct sockaddr*)&peer,&peer_len);
        if(n<0) {if(errno==EINTR)continue;perror("recvfrom");break;}
        if(peer.sll_pkttype==PACKET_OUTGOING)continue;
        rc=openpnio_dcp_handle_identify(rx,(size_t)n,mac,&identity,
                                       tx,sizeof tx,&written);
        if(rc==1) {
            struct sockaddr_ll dest;
            ssize_t sent;
            memset(&dest,0,sizeof dest);
            dest.sll_family=AF_PACKET;
            dest.sll_protocol=htons(0x8892);
            dest.sll_ifindex=ifindex;
            dest.sll_halen=6;
            memcpy(dest.sll_addr,tx,6);
            sent=sendto(fd,tx,written,0,(struct sockaddr*)&dest,sizeof dest);
            if(sent<0 || (size_t)sent!=written) perror("sendto");
            else printf("Identify response sent (%zu bytes)\n",written);
        }
    }
    close(fd);
    return 0;
}
