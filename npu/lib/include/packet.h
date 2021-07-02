#ifndef NPU_LIB_PACKET_H
#define NPU_LIB_PACKET_H

#include <stdint.h>

struct ethaddr {
    uint8_t octets[6];
};

struct ipaddr {
    uint8_t octets[4];
};

/**
 * Structure of Ethernet header.
 * */
struct __attribute__((packed)) eth_hdr {
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t ethtype;
};

/**
 * Structure of Ethernet header with VLAN tag.
 * */
struct __attribute__((packed)) eth_hdr_dot1q {
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t tpid;
    uint16_t tag;
    uint16_t ethtype;
};

/**
 * Structure of Ethernet header with two VLAN tags.
 * */
struct __attribute__((packed)) eth_hdr_qinq {
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t s_tpid;
    uint16_t s_tag;
    uint16_t c_tpid;
    uint16_t c_tag;
    uint16_t ethtype;
};

/**
 * Structure of VLAN header.
 * */
struct __attribute__((packed)) vlan_hdr {
    uint16_t vlan_tag;
    uint16_t ethtype;
};

/**
 * Structure of MPLS header.
 * */
struct __attribute__((packed)) mpls_hdr {
    uint32_t mpls_tag;
    uint16_t ethtype;
};

/**
 * Structure of packet to be sent to controller.
 * */
struct __attribute__((packed)) ctrl_lrn_pkt {
    uint8_t mac[6];
    uint8_t port;
    uint16_t vlan_tag;
    uint32_t src_hash;
};

/**
 * Structure of IPv4 header.
 * */
struct __attribute__((packed)) ip_hdr {
    uint8_t version_ihl;      // version - 4 bits, ihl - 4 bits
    uint8_t dscp_ecn;         // dcsp - 6 bits, ecn - 2 bits
    uint16_t length;
    uint16_t identification;
    uint16_t flags_offset;    // flags - 3 bits, offset - 13 bits
    uint8_t ttl;
    uint8_t ip_proto;
    uint16_t crc;
    uint8_t src[4];
    uint8_t dst[4];

    uint32_t options[4];      // 1-4 4-byte words if (ihl > 5)
};

/**
 * Structure of ARP packet.
 * */
struct __attribute__((packed)) arp_pkt {
    uint16_t htype;
    uint16_t ptype;
    uint8_t hlen;
    uint8_t plen;
    uint16_t oper;
    uint8_t sha[6];
    uint8_t spa[4];
    uint8_t tha[6];
    uint8_t tpa[4];
};

#endif //NPU_LIB_PACKET_H
