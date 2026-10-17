#ifndef OPENPNIO_RPC_CL_H
#define OPENPNIO_RPC_CL_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* DCE/RPC v4 connectionless (ncadg_ip_udp) datagram header parser.
 * Deliberately no PNIO CM Connect/Release server or response encoder yet.
 * Parse only; transport and operation dispatch must validate interface UUID
 * and complete RPC fragmentation before any side effects. */
enum { OPENPNIO_RPC_CL_HEADER_SIZE=80 };
typedef struct openpnio_rpc_cl_header {
    uint8_t packet_type;
    uint8_t flags1;
    uint8_t flags2;
    uint8_t little_endian;
    uint8_t object_uuid[16];
    uint8_t interface_uuid[16];
    uint8_t activity_uuid[16];
    uint32_t server_boot;
    uint32_t interface_version;
    uint32_t sequence;
    uint16_t operation;
    uint16_t fragment_number;
    uint16_t body_length;
    uint16_t serial;
    uint8_t auth_protocol;
    const uint8_t *body;
} openpnio_rpc_cl_header_t;

/* 0 parsed; -1 invalid/truncated/unknown representation.
 * A successful parse does not indicate a supported PNIO RPC operation. */
int openpnio_rpc_cl_parse(const uint8_t *datagram,size_t datagram_length,
                         openpnio_rpc_cl_header_t *out);
uint16_t openpnio_rpc_cl_u16(const uint8_t *p,int little);
uint32_t openpnio_rpc_cl_u32(const uint8_t *p,int little);
#ifdef __cplusplus
}
#endif
#endif
