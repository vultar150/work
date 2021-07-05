#ifndef NPU_VLAN_SDN_TABLE_H
#define NPU_VLAN_SDN_TABLE_H

#include "lib/include/npu.h"

#define VLAN_TBL_CAPACITY 4097

struct vlan_node_item {
    uint8_t port;
    tag_mode_t mode;
};

struct vlan_node {
    uint8_t len;
    struct vlan_node_item items[ETH_PORTS_NR];
};

struct vlan_table {
    struct vlan_node nodes[VLAN_TBL_CAPACITY];
};

struct hashnode {
    struct hashnode *next;
    uint32_t cc;
    struct ethaddr mac;
    uint8_t port;
    uint16_t vlan;
};

struct hashtable {
    struct hashnode *nodes[HASHNODE_CAPACITY];
    uint32_t next_node;
};

enum tables {
    SRC_TABLE,
    DST_TABLE,
    VLAN_TABLE,
};

struct key {
    uint32_t hash;
    struct ethaddr mac;
    uint16_t tag;

    uint8_t src_port;
    uint8_t dst_port;
};

struct res {
    uint8_t len;
    struct vlan_node_item items[ETH_PORTS_NR - 1];
};

struct search_result lookup_switch(struct search_key);

void * src_table_mmap(void);
void * dst_table_mmap(void);
void * vlan_table_mmap(void);

struct hashnode * get_head(struct hashtable *, uint32_t);
struct hashnode * get_next(struct hashtable *, uint32_t,
                           struct hashnode *);
struct hashnode ** get_head_p(struct hashtable *tbl, uint32_t hash);
struct hashnode ** get_next_p(struct hashnode **pnode);

#endif //NPU_VLAN_SDN_TABLE_H
