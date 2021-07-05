#ifndef NPU_VLAN_SDN_CONTEXT_H
#define NPU_VLAN_SDN_CONTEXT_H

#include "lib/include/npu.h"

#include "table.h"

struct lookup_src_ctx {
    uint8_t src_port;

    struct ethaddr src_mac;
    struct ethaddr dst_mac;

    uint8_t is_tagged:1;
    uint16_t vlan_vid;
    uint16_t vlan_pcp;
};

struct resolve_src_ctx {
    uint8_t src_port;
    uint8_t src_table_port;

    struct ethaddr src_mac;
    struct ethaddr dst_mac;
    uint32_t src_hash;

    uint8_t is_tagged:1;
    uint16_t vlan_vid;
    uint16_t vlan_pcp;
};

struct lookup_dst_ctx {
    uint8_t src_port;

    struct ethaddr dst_mac;

    uint8_t is_tagged:1;
    uint16_t vlan_vid;
    uint16_t vlan_pcp;
};

struct resolve_dst_ctx {
    uint8_t src_port;
    uint8_t dst_port;

    uint8_t is_tagged:1;
    uint16_t vlan_vid;
    uint16_t vlan_pcp;

    uint8_t len;
    struct vlan_node_item items[ETH_PORTS_NR];
};



struct learn_ctx {
    struct ethaddr src_mac;
    uint32_t src_hash;
    uint8_t src_port;
    uint16_t vlan_tag;
};

#endif //NPU_VLAN_SDN_CONTEXT_H
