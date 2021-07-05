#ifndef NPU_LIB_NPU_H
#define NPU_LIB_NPU_H

#include "lib/include/packet.h"
#include "lib/include/table.h"

#define ETH_PORTS_NR 8   /*!< Number of switch ports. */
#define CTRL_PORT 254    /*!< Port related to controller. */
#define OF_AGENT_PORT 7  /*!< Port related to OpenFlow agent. */

#define TOTAL_FRAME_BUFFERS_NR 128  /*!< Number of internal frame buffers. */
#define MAX_FRAME_SIZE 16384

#define VLAN_NONE 4096
#define TRUNK_PORT 4097

#define HASHNODE_COUNT 1024    /*!< Number of nodes in a MAC hashtable. */
#define HASHNODE_CAPACITY 128  /*!< Number of records in hashnode of MAC hashtable. */

#define MCAST_HASHNODE_COUNT 256     /*!< Number of nodes in Multicast hashtable. */
#define MCAST_HASHNODE_CAPACITY 128  /*!< Number of records in hashnode of Multicast hashtable. */

#define ACTION_NR 8  /*!< Number of actions in a table entry. */
#define BUCKET_NR 6  /*!< Number of buckets in a group table entry. */
#define ENTRY_NR  8  /*!< Number of entries in a flow table. */

#define PORT_NOT_FOUND UINT8_MAX

#define HEADER_SIZE 80
#define MAX_HEADER_SIZE 100
#define MAX_STAGE_CTX_SIZE 1066

typedef enum {
    NO_CHNG = 0,
    ADD_TAG,
    DEL_TAG,
    CHNG_TAG
} tag_task_t;

typedef enum {
    NO_TAG = 0,
    NEED_TAG
} tag_mode_t;

typedef enum {
    NO_TAG_PRESENT = 0,
    TAGGED
} tagged_t;

typedef enum {
    UNICAST = 0,
    BCAST,
    MCAST
} cast_t;

typedef enum {
    OFPPS_LINK_DOWN = 1 << 0,   /* No physical link present. */
    OFPPS_BLOCKED   = 1 << 1,   /* Port is blocked. */
    OFPPS_LIVE      = 1 << 2,   /* Live for Fast Failover Group. */
} ofp_port_state;

typedef enum {
    OFPAT_GROUP  = 1,        /* Apply group. */
    OFPAT_OUTPUT = 2,        /* Send frame. */
    OFPAT_PUSH_VLAN = 17,    /* Push a new VLAN tag. */
    OFPAT_PUSH_MPLS = 19,    /* Push a new MPLS tag. */
    OFPAT_SET_FIELD = 25,	 /* Set field. */
} ofp_action_type;

typedef enum {
    MPLS_POP = 1,
    MPLS_AGGREGATE = 2,
    MPLS_UNTAG = 3,
    MPLS_SWAP = 4,
    MPLS_IMPOSE = 5,
} mpls_action_type;

typedef enum {
    OFPGT_ALL       = 0,        /* All (multicast/broadcast) group. */
    OFPGT_SELECT    = 1,        /* Select group. */
    OFPGT_INDIRECT  = 2,        /* Indirect group. */
    OFPGT_FF        = 3,        /* Fast failover group. */
} ofp_group_type;

typedef enum {
    OFPIT_WRITE_METADATA = 2,    /* Setup the metadata field for use later in
                                    pipeline. */
    OFPIT_GOTO_TABLE = 11,       /* Setup the next table in the
                                  * lookup pipeline. */
} ofp_instruction_type;


struct frame_buffer {
    uint8_t data[9000];
};

struct buf_info {
    struct frame_buffer *fb;
    uint32_t fb_id;
};

/**
 * Structure of port table.
 * */
struct ofp_port {
    uint32_t state;
};

/**
 * Structure of table action.
 * */
struct ofp_action {
    uint16_t type;
    uint32_t value;
    struct ethaddr mac;
};

/**
 * Structure of bucket in a group table.
 * */
struct ofp_bucket {
    uint8_t len;
    struct ofp_action action[ACTION_NR];
};

/**
 * Structure of entry in a group table.
 * */
struct ofp_group_entry {
    uint32_t group_id;
    ofp_group_type type;
    uint8_t index;

    uint8_t len;
    struct ofp_bucket bucket[BUCKET_NR];
};

/**
 * Structure of group table.
 * */
struct ofp_group_table {
    uint8_t len;
    struct ofp_group_entry entry[ENTRY_NR];
};

struct location {
    uint32_t fb_id;
    uint16_t framesz;
};

struct header {
    int8_t data[MAX_HEADER_SIZE];
    uint16_t sz;
};

struct packet_context {
    int8_t stage_context[MAX_STAGE_CTX_SIZE];
    uint8_t src_port;
    struct header header;
    struct location location;
};

struct header_list_entry {
    struct header header;
    uint8_t ports[ETH_PORTS_NR];
    uint8_t port_cnt;
};

struct replicator_context {
    struct location location;
    struct header_list_entry list[ETH_PORTS_NR];
    uint8_t rec_cnt;
};

/**
 * \brief Packet directions after resolve
 *
 * out_port - -1 (if no output port) or port_nr
 * next_stage - send to next stage, bool
 * drop - drop packet, bool
 */
struct resolve_result {
    int out_port;
    int next_stage;
    int drop;
};

/**
 * Different counters.
 * */
struct counter {
    uint64_t in_frames[ETH_PORTS_NR];
    uint64_t in_ctrl_frames;

    uint64_t drop_frame_no_mem;
    uint64_t drop_frame_too_long;
    uint64_t drop_frame_invalid;

    uint64_t out_frames[ETH_PORTS_NR];
    uint64_t out_ctrl_frames;
};

/**
 * Pointers to MAC_RX and MAC_TX functions.
 * */
struct io_fn {
    int (*rx) (uint8_t, uint64_t *);
    void (*tx) (uint8_t, uint64_t, int, int);
};

/**
 * Pointers to functions for working with switch internal memory.
 * */
struct stage_fn {
    uint32_t (*alloc_fb) (void *);
    struct frame_buffer * (*mmap_fb) (void *, uint32_t);
    void (*free_fb) (void *, uint32_t);
    void (*read_fb)(void *, uint8_t *, uint8_t *, uint32_t);
};

/**
 * Pointer to function for lookup operation.
 * */
struct lookup_fn {
    struct search_result (*search_rd)(void *, struct search_key);
};

void process_model(uint8_t, struct io_fn *, 
                   struct packet_context *, 
                   struct stage_fn *);

#endif //NPU_LIB_NPU_H