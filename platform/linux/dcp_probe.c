/* Experimental Linux AF_PACKET DCP Identify/Get/Set bench device.
 * No network-interface reconfiguration is performed by this process.
 * Use only on an isolated bench network, never an industrial installation.
 */
#define _POSIX_C_SOURCE 200809L
#include "openpnio/dcp_device.h"
#include "openpnio/dcp_get.h"
#include "openpnio/dcp_set.h"
#include "openpnio/lldp.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <limits.h>
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
#include <time.h>

typedef struct bench {
    openpnio_dcp_identity_t identity;
    char station_name[241];
    const char *station_file; /* optional persistence path, selected by operator */
    int allow_set_name;
    int enable_lldp;
} bench_t;

static volatile sig_atomic_t running = 1;
static void stop(int sig) { (void)sig; running = 0; }
static uint64_t monotonic_ms(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC,&ts)!=0) return 0;
    return (uint64_t)ts.tv_sec*1000u + (uint64_t)ts.tv_nsec/1000000u;
}
static void advertise_lldp(int fd, int ifindex, const uint8_t mac[6],
                           const char *station_name)
{
    uint8_t frame[1024];
    size_t length=0;
    struct sockaddr_ll dst;
    if (fd<0 || openpnio_lldp_build(mac,station_name,120,
                                   frame,sizeof frame,&length)!=0) return;
    memset(&dst,0,sizeof dst);
    dst.sll_family=AF_PACKET;
    dst.sll_protocol=htons(0x88cc);
    dst.sll_ifindex=ifindex;
    dst.sll_halen=6;
    memcpy(dst.sll_addr,frame,6);
    if (sendto(fd,frame,length,0,(struct sockaddr*)&dst,sizeof dst)<0)
        perror("sendto(LLDP)");
}

static int valid_station_name(const char *p, size_t n)
{
    size_t i;
    if (n == 0 || n > 240 || p[0] == '-' || p[n-1] == '-') return 0;
    for (i=0; i<n; i++) {
        char c=p[i];
        if (!((c>='a' && c<='z') || (c>='0' && c<='9') || c=='-' || c=='.'))
            return 0;
        if (c == '.' && (i==0 || i==n-1 || p[i-1]=='.' ||
                         p[i-1]=='-' || p[i+1]=='-')) return 0;
    }
    return 1;
}
static int save_station_file(const char *path, const char *name, size_t length)
{
    char temporary[PATH_MAX];
    int fd, ok = 1;
    ssize_t n;
    size_t total=0;
    if (!path || strlen(path) + 12 >= sizeof temporary) return -1;
    snprintf(temporary, sizeof temporary, "%s.tmp.XXXXXX", path);
    fd=mkstemp(temporary);
    if (fd < 0) return -1;
    while (total < length) {
        n=write(fd,name+total,length-total);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) { ok=0; break; }
        total+=(size_t)n;
    }
    if (ok && write(fd,"\n",1)!=1) ok=0;
    if (ok && fsync(fd)!=0) ok=0;
    if (close(fd)!=0) ok=0;
    if (ok && rename(temporary,path)!=0) ok=0;
    if (!ok) { unlink(temporary); return -1; }
    return 0;
}
static int read_station_file(const char *path, char name[241])
{
    FILE *f;
    char buffer[243];
    size_t n;
    int c;
    f=fopen(path,"r");
    if (!f) return errno == ENOENT ? 0 : -1;
    if (!fgets(buffer,sizeof buffer,f)) { fclose(f); return -1; }
    n=strcspn(buffer,"\r\n");
    c=fgetc(f);
    if (fclose(f)!=0 || c!=EOF || !valid_station_name(buffer,n)) return -1;
    memcpy(name,buffer,n);
    name[n]='\0';
    return 1;
}
static int set_name(void *ctx, const char *name, size_t n, uint16_t qualifier)
{
    bench_t *state=(bench_t*)ctx;
    /* DCP BlockQualifier: 0 = temporary, 1 = permanent.
     * Never claim a permanent Set succeeded without persistence. */
    if (!state->allow_set_name || qualifier>1 ||
        !valid_station_name(name,n)) return -1;
    if (qualifier==1 && !state->station_file) {
        fprintf(stderr,"Permanent DCP Set requires --station-file\n");
        return -1;
    }
    if (qualifier==1 && save_station_file(state->station_file,name,n)!=0) {
        perror("Saving station name");
        return -1;
    }
    memcpy(state->station_name,name,n);
    state->station_name[n]='\0';
    printf("DCP Set station name -> %s%s\n",state->station_name,
           qualifier==1 ? " (persisted)" : " (RAM only)");
    fflush(stdout);
    return 0;
}
static int set_ip_safe(void *ctx, const uint8_t ip[4],
                       const uint8_t mask[4], const uint8_t gw[4],
                       uint16_t qualifier)
{
    bench_t *state=(bench_t*)ctx;
    /* A matching request is a no-op. All changes are refused to protect SSH. */
    return (qualifier<=1 && memcmp(ip,state->identity.ip,4)==0 &&
            memcmp(mask,state->identity.netmask,4)==0 &&
            memcmp(gw,state->identity.gateway,4)==0) ? 0 : -1;
}
/* /proc/net/route Gateway is printed in little-endian hexadecimal. */
static void read_default_gateway(const char *iface, uint8_t gateway[4])
{
    FILE *f=fopen("/proc/net/route","r");
    char line[512], ifname[IFNAMSIZ];
    unsigned long destination, via, flags;
    memset(gateway,0,4);
    if (!f) return;
    if (!fgets(line,sizeof line,f)) { fclose(f); return; }
    while(fgets(line,sizeof line,f)) {
        if(sscanf(line,"%15s %lx %lx %lx",ifname,&destination,&via,&flags)!=4)
            continue;
        if(strcmp(ifname,iface)==0 && destination==0 &&
           (flags & 0x3u)==0x3u) {
            gateway[0]=(uint8_t)via;
            gateway[1]=(uint8_t)(via>>8);
            gateway[2]=(uint8_t)(via>>16);
            gateway[3]=(uint8_t)(via>>24);
            break;
        }
    }
    fclose(f);
}
static int update_network_identity(const char *iface, openpnio_dcp_identity_t *identity)
{
    struct ifaddrs *list, *it;
    int found=0;
    if (getifaddrs(&list)!=0) return -1;
    memset(identity->ip,0,4);
    memset(identity->netmask,0,4);
    for (it=list;it;it=it->ifa_next) {
        if (!it->ifa_addr || it->ifa_addr->sa_family!=AF_INET ||
            strcmp(it->ifa_name,iface)!=0) continue;
        memcpy(identity->ip,&((struct sockaddr_in*)it->ifa_addr)->sin_addr,4);
        if (it->ifa_netmask)
            memcpy(identity->netmask,&((struct sockaddr_in*)it->ifa_netmask)->sin_addr,4);
        found=1;
        break;
    }
    freeifaddrs(list);
    read_default_gateway(iface,identity->gateway);
    return found ? 0 : -1;
}
static int send_response(int fd,int ifindex,const uint8_t *tx,size_t written,
                         const char *service)
{
    struct sockaddr_ll dest;
    ssize_t sent;
    memset(&dest,0,sizeof dest);
    dest.sll_family=AF_PACKET;
    dest.sll_protocol=htons(0x8892);
    dest.sll_ifindex=ifindex;
    dest.sll_halen=6;
    memcpy(dest.sll_addr,tx,6);
    sent=sendto(fd,tx,written,0,(struct sockaddr*)&dest,sizeof dest);
    if(sent<0) {perror("sendto");return -1;}
    if((size_t)sent!=written) {fprintf(stderr,"Short Ethernet send\n");return -1;}
    printf("%s response sent (%zu bytes)\n",service,written);
    fflush(stdout);
    return 0;
}
int main(int argc,char **argv)
{
    int fd,ifindex,rc,i,lldp_fd=-1;
    uint64_t next_lldp=0;
    const char *interface=NULL;
    struct ifreq ifr;
    struct sockaddr_ll bind_addr;
    struct pollfd pfd;
    uint8_t rx[2048],tx[2048],mac[6];
    size_t written;
    bench_t state;
    openpnio_dcp_set_handlers_t handlers;
    memset(&state,0,sizeof state);
    memcpy(state.station_name,"openpnio-test",sizeof "openpnio-test");
    state.identity.station_name=state.station_name;
    state.identity.station_type="OpenPNIO Linux prototype";
    state.identity.vendor_id=0; state.identity.device_id=0;
    for(i=1;i<argc;i++) {
        if(strcmp(argv[i],"--lldp")==0) {
            state.enable_lldp=1;
        } else if(strcmp(argv[i],"--allow-set-name")==0) {
            state.allow_set_name=1;
        } else if(strcmp(argv[i],"--station-file")==0 && i+1<argc) {
            state.station_file=argv[++i];
        } else if(!interface && argv[i][0]!='-') {
            interface=argv[i];
        } else {
            fprintf(stderr,"Usage: %s <interface> [--allow-set-name] [--station-file PATH] [--lldp]\n",argv[0]);
            return 2;
        }
    }
    if(!interface || strlen(interface)>=IFNAMSIZ) {
        fprintf(stderr,"Usage: %s <interface> [--allow-set-name] [--station-file PATH]\n",argv[0]);
        return 2;
    }
    if(state.station_file) {
        rc=read_station_file(state.station_file,state.station_name);
        if(rc<0) {fprintf(stderr,"Invalid/unreadable station file: %s\n",state.station_file);return 1;}
    }
    if(update_network_identity(interface,&state.identity)!=0) {
        fprintf(stderr,"Interface %s has no IPv4 address; run on a configured bench interface\n",interface);
        return 1;
    }
    fd=socket(AF_PACKET,SOCK_RAW,htons(0x8892));
    if(fd<0) {perror("socket(AF_PACKET)");return 1;}
    memset(&ifr,0,sizeof ifr);
    memcpy(ifr.ifr_name,interface,strlen(interface)+1);
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
    if(state.enable_lldp) {
        struct sockaddr_ll lldp_bind;
        lldp_fd=socket(AF_PACKET,SOCK_RAW,htons(0x88cc));
        if(lldp_fd<0) {perror("LLDP socket");close(fd);return 1;}
        memset(&lldp_bind,0,sizeof lldp_bind);
        lldp_bind.sll_family=AF_PACKET;
        lldp_bind.sll_protocol=htons(0x88cc);
        lldp_bind.sll_ifindex=ifindex;
        if(bind(lldp_fd,(struct sockaddr*)&lldp_bind,sizeof lldp_bind)<0) {
            perror("LLDP bind");close(lldp_fd);close(fd);return 1;
        }
        next_lldp=0; /* immediate first advertisement */
    }
    signal(SIGINT,stop);
    signal(SIGTERM,stop);
    pfd.fd=fd; pfd.events=POLLIN;
    handlers.set_station_name=state.allow_set_name ? set_name : NULL;
    handlers.set_ip_suite=set_ip_safe;
    handlers.context=&state;
    printf("OpenPNIO DCP lab on %s, MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
           interface,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    printf("Station: %s, IPv4: %u.%u.%u.%u\n",state.station_name,
           state.identity.ip[0],state.identity.ip[1],
           state.identity.ip[2],state.identity.ip[3]);
    printf("Set Name: %s; Set IP: changes refused (matching suite accepted as no-op)\n",
           state.allow_set_name ? "enabled" : "disabled");
    if(state.enable_lldp)
        puts("LLDP: basic IEEE advertisement enabled every 5s; no PI TLVs/neighbor DB");
    fflush(stdout);
    while(running) {
        if (state.enable_lldp && monotonic_ms()>=next_lldp) {
            advertise_lldp(lldp_fd,ifindex,mac,state.station_name);
            next_lldp=monotonic_ms()+5000;
        }
        struct sockaddr_ll peer;
        socklen_t peer_len=sizeof peer;
        ssize_t n;
        const char *service=NULL;
        rc=poll(&pfd,1,250);
        if(rc<0) {if(errno==EINTR)continue;perror("poll");break;}
        if(rc==0 || !(pfd.revents&POLLIN))continue;
        memset(&peer,0,sizeof peer);
        n=recvfrom(fd,rx,sizeof rx,0,(struct sockaddr*)&peer,&peer_len);
        if(n<0) {if(errno==EINTR)continue;perror("recvfrom");break;}
        if(peer.sll_pkttype==PACKET_OUTGOING)continue;
        if(update_network_identity(interface,&state.identity)!=0) continue;
        written=0;
        rc=openpnio_dcp_handle_identify(rx,(size_t)n,mac,&state.identity,
                                        tx,sizeof tx,&written);
        if(rc==1) service="Identify";
        if(rc==0) {
            rc=openpnio_dcp_handle_get(rx,(size_t)n,mac,&state.identity,
                                       tx,sizeof tx,&written);
            if(rc==1) service="Get";
        }
        if(rc==0) {
            rc=openpnio_dcp_handle_set_ex(rx,(size_t)n,mac,&handlers,
                                          tx,sizeof tx,&written);
            if(rc==1) service="Set";
        }
        if(rc==1 && service) (void)send_response(fd,ifindex,tx,written,service);
    }
    if (lldp_fd>=0) close(lldp_fd);
    close(fd);
    return 0;
}
