#ifndef NPU_VLAN_SDN_TABLE_H
#define NPU_VLAN_SDN_TABLE_H

#include "lib/include/npu.h"
#include "lib/include/action.h"

typedef enum {
    ALL = 0,
    INDIRECT
} group_t;

typedef struct flow_entry {
    struct ethaddr mac;
    uint16_t vlan_tag;
    uint32_t cc;
    instr_t actions[MAX_NM_ACTIONS]; /* list of actions */
    uint16_t act_len;
    uint8_t valid;
} flow_entry_t;

typedef struct action_bucket {
    instr_t actions[MAX_NM_BUCKET_ACTIONS];
    uint16_t act_len;
} action_bucket_t;

typedef struct group_entry {
    uint16_t group_id;
    group_t type;
    uint16_t cc;
    action_bucket_t buckets[MAX_NM_BUCKETS];
    uint8_t buck_len;
} group_entry_t;

struct flow_table {
    flow_entry_t nodes[MAX_FLOW_TABLE_SIZE];
    uint16_t len;
};

struct group_table {
    group_entry_t nodes[MAX_GROUPS];
    uint16_t len;
};

enum tables {
    SRC_TABLE,
    DST_TABLE,
    GROUP_TABLE
};

struct key {
    struct ethaddr mac;
    uint16_t tag;
    uint8_t src_port;
    uint8_t dst_port;
};

struct res {
    instr_t actions[MAX_NM_ACTIONS];
    uint16_t len;
};

struct search_result lookup_flow(struct search_key);

void * src_flow_table_mmap(void);
void * dst_flow_table_mmap(void);
void * group_table_mmap(void);

#endif //NPU_VLAN_SDN_TABLE_H
