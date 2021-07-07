#include <assert.h>

#include <lib/include/replicator.h>
#include "lib/include/helpers.h"
#include "lib/include/memory.h"
#include "lib/include/action.h"

#include "context.h"
#include "table.h"

#include <stdio.h>


int add_tag(int8_t *fb, uint16_t fb_size, uint16_t framesz, uint8_t offset,
            uint8_t *tag, uint8_t tagsz, uint8_t is_tagged)
#if defined(__GNUC__)
__attribute__((warn_unused_result))
#endif
;

static void del_tag(int8_t *fb, uint16_t framesz, uint8_t offset,
                   uint8_t tagsz, uint8_t is_tagged);
// #if defined(__GNUC__)
// __attribute__((warn_unused_result))
// #endif
// ;

void set_tag(int8_t *fb, uint16_t framesz, uint8_t offset, 
             uint8_t *tag, uint8_t tagsz, uint8_t is_tagged);

static void lookup_flow_actions(struct flow_table *, struct ethaddr*, 
                                uint16_t,  struct res *, uint8_t);

static void set_entry_actions_from_group_buckets(group_entry_t *, struct res *);

static uint8_t group_table_search(uint16_t, struct res *);

static uint8_t contains(struct search_result *, action_type_t);

static uint8_t get_action_port(struct search_result *);

// static uint16_t get_group_id(struct search_result *);

struct resolve_result match_action_src(struct stage_fn *sfn,
                                       struct lookup_fn *lfn,
                                       struct packet_context *in_ctx,
                                       struct replicator_context *repl_ctx, 
                                       struct packet_context *out_ctx)
{
    struct lookup_src_ctx *lkp = (void *) in_ctx->stage_context;
    struct resolve_src_ctx *rsv = (void *) out_ctx->stage_context;
    out_ctx->location = in_ctx->location;
    out_ctx->header = in_ctx->header;

    rsv->src_port = lkp->src_port;

    rsv->src_mac = lkp->src_mac;
    rsv->dst_mac = lkp->dst_mac;

    rsv->is_tagged = lkp->is_tagged;
    rsv->vlan_vid = lkp->vlan_vid;
    rsv->vlan_pcp = lkp->vlan_pcp;

    struct search_key sk;
    sk.table_nr = SRC_TABLE;

    struct key *k = (void *) sk.key;
    k->mac = lkp->src_mac;
    k->tag = lkp->vlan_vid;

    struct search_result sr = lfn->search_rd(sk);

    struct lookup_dst_ctx *lkp_dst = (void *) in_ctx->stage_context;

    struct resolve_result res;

    res.next_stage = contains(&sr, INSTR_GOTO_TABLE);
    res.out_port = -1;
    res.drop = contains(&sr, ACTION_DROP);

    lkp_dst->src_port = rsv->src_port;

    lkp_dst->is_tagged = rsv->is_tagged;
    lkp_dst->vlan_vid = rsv->vlan_vid;
    lkp_dst->vlan_pcp = rsv->vlan_pcp;

    lkp_dst->dst_mac = rsv->dst_mac;

    rsv->src_table_port = get_action_port(&sr);

    if (rsv->src_port == rsv->src_table_port) {
        return res;
    }

    init_context(repl_ctx);
    uint32_t fb_id = sfn->alloc_fb();
    struct frame_buffer* fb = sfn->mmap_fb(out_ctx->location.fb_id);
    struct frame_buffer* new_fb = sfn->mmap_fb(fb_id);
    memcpy(new_fb->data, fb->data, out_ctx->location.framesz);

    /* Form a packet from the src_mac, src_port, vlan_vid, src_hash. */
    struct ctrl_lrn_pkt pkt;
    repl_ctx->rec_cnt = 1;
    pkt.port = rsv->src_port;
    pkt.src_hash = rsv->src_hash;
    pkt.vlan_tag = rsv->vlan_vid;
    memcpy(pkt.mac, rsv->src_mac.octets, sizeof(pkt.mac));
    memcpy(repl_ctx->list[0].header.data, &pkt, sizeof(pkt));
    repl_ctx->list[0].header.sz = out_ctx->header.sz;
    repl_ctx->list[0].port_cnt = 1;
    repl_ctx->list[0].ports[0] = CTRL_PORT;
    repl_ctx->location.fb_id = fb_id;
    repl_ctx->location.framesz = out_ctx->location.framesz;
    
    res.out_port = CTRL_PORT;
    return res;
}


struct resolve_result match_action_dst(struct stage_fn *sfn,
                                       struct lookup_fn *lfn,
                                       struct packet_context *in_ctx,
                                       struct replicator_context *repl_ctx,
                                       struct packet_context *lkp_ctx)
{
    struct lookup_dst_ctx *lkp = (void *) lkp_ctx->stage_context;
    struct resolve_dst_ctx *rsv = (void *) in_ctx->stage_context;

    rsv->src_port = lkp->src_port;

    rsv->is_tagged = lkp->is_tagged;
    rsv->vlan_vid = lkp->vlan_vid;
    rsv->vlan_pcp = lkp->vlan_pcp;

    struct search_key sk;
    struct key *k = (void *) sk.key;

    sk.table_nr = DST_TABLE;
    k->mac = lkp->dst_mac;
    k->tag = lkp->vlan_vid;

    struct search_result sr = lfn->search_rd(sk);

    init_context(repl_ctx);
    repl_ctx->location = in_ctx->location;

    struct resolve_result result;
    result.drop = contains(&sr, ACTION_DROP);
    result.out_port = 1;
    result.next_stage = contains(&sr, INSTR_GOTO_TABLE);

    uint8_t vlan_tag[4] = { 0x81, 0x00, 
                            rsv->vlan_pcp | rsv->vlan_vid >> 8, 
                            rsv->vlan_vid & 0xff }; 

    /* Perform all actions obtained from the dst flow table:
     * If table doesn't contain ACTION_GROUP, then send 
     * one packet on corresponding port with pushing or deleting 
     * vlan tag. Else perform all actions in group buckets. */

    if (contains(&sr, ACTION_GROUP)) {
        sk.table_nr = GROUP_TABLE;
        sr = lfn->search_rd(sk);
        if (sr.not_found) {
            result.drop = 1;
            return result;
        }
    }

    struct res *r = (void *) sr.res_entry;
    struct header tmp = in_ctx->header;
    uint16_t pcp_vid;

    for (int i = 0; i < r->len; i++) {
        switch (r->actions[i].type) {
            case ACTION_PUSH_VLAN:
                pcp_vid = (uint16_t) r->actions[i].value;              
                vlan_tag[2] = pcp_vid >> 8;
                vlan_tag[3] = pcp_vid & 0xff;
                if (add_tag(tmp.data, sizeof(tmp.data), tmp.sz,
                        MEMBER_SIZE(struct eth_hdr, dst) + MEMBER_SIZE(struct eth_hdr, src),
                        vlan_tag, sizeof(vlan_tag), rsv->is_tagged)) {
                    sfn->free_fb(in_ctx->location.fb_id);
                    result.drop = 1;
                    return result;
                }
                tmp.sz += rsv->is_tagged ? 0 : sizeof(vlan_tag);
                break;

            case ACTION_SET_VLAN:
                pcp_vid = (uint16_t) r->actions[i].value;
                vlan_tag[2] = pcp_vid >> 8;
                vlan_tag[3] = pcp_vid & 0xff;
                set_tag(tmp.data, tmp.sz,
                        MEMBER_SIZE(struct eth_hdr, dst) + MEMBER_SIZE(struct eth_hdr, src), 
                        vlan_tag, sizeof(vlan_tag), rsv->is_tagged);
                break;

            case ACTION_POP_VLAN:
                del_tag(tmp.data, tmp.sz, 
                        MEMBER_SIZE(struct eth_hdr, dst) + MEMBER_SIZE(struct eth_hdr, src),
                        sizeof(vlan_tag), rsv->is_tagged); 

                tmp.sz -= rsv->is_tagged ? sizeof(vlan_tag) : 0;
                break;

            case ACTION_OUTPUT:
                if (r->actions[i].value != rsv->src_port) {
                    repl_ctx->list[repl_ctx->rec_cnt].header = tmp;
                    repl_ctx->list[repl_ctx->rec_cnt].ports[repl_ctx->list[repl_ctx->rec_cnt].port_cnt] =
                        (uint8_t) r->actions[i].value;
                    repl_ctx->list[repl_ctx->rec_cnt].port_cnt++;
                    repl_ctx->rec_cnt++;
                }
                tmp = in_ctx->header;  
                break;

            default:
                break;
        }
    }
    return result;
}

static uint8_t contains(struct search_result *sr, action_type_t type) 
{
    struct res *r = (void *) sr->res_entry;

    for (int i = 0; i < r->len; i++) {
        if (r->actions[i].type == type) {
            return 1;
        }
    }
    return 0;
}

static uint8_t get_action_port(struct search_result *sr) 
{
    struct res *r = (void *) sr->res_entry;
    
    for (int i = 0; i < r->len; i++) {
        if (r->actions[i].type == ACTION_OUTPUT) {
            return r->actions[i].value;
        }
    }
    return PORT_NOT_FOUND;
}

static void lookup_flow_actions(struct flow_table *tbl,
                                struct ethaddr* mac, uint16_t tag, 
                                struct res *r, uint8_t is_src_tbl)
{
    flow_entry_t *node;

    for (int i = 0; i < tbl->len; i++) {
        node = &(tbl->nodes[i]);
        if (node->valid && is_same_mac(&(node->mac), mac) && node->vlan_tag == tag) {
            for (int j = 0; j < node->act_len; j++) {
                r->actions[j] = node->actions[j];
                r->len++;
            }
            return;
        }
    }
    if (is_src_tbl) {
        r->actions[0].type = ACTION_OUTPUT;
        r->actions[0].value = CTRL_PORT;
        r->actions[1].type = INSTR_GOTO_TABLE;
        r->actions[1].value = 1;
        r->len = 2;
    } else {
        r->actions[0].type = ACTION_GROUP;
        r->actions[0].value = tag;
        r->len = 1;
    }
    return;
}

static void set_entry_actions_from_group_buckets(group_entry_t *entry, 
                                                 struct res *r)
{
    assert(entry->type == ALL);
    action_bucket_t *bucket;
    for (int i = 0; i < entry->buck_len; i++) {
        bucket = &(entry->buckets[i]);
        for (int k = 0; k < bucket->act_len; k++) {
            r->actions[r->len++] = bucket->actions[k];
        }
    }
}

static uint8_t group_table_search(uint16_t tag, struct res *r)
{
    struct group_table *tbl = group_table_mmap();
    group_entry_t *entry = NULL;
    for (int i = 0; i < tbl->len; i++) {
        if (tbl->nodes[i].group_id == tag) {
            entry = &(tbl->nodes[i]);
            entry->cc++;
            set_entry_actions_from_group_buckets(entry, r);
            return r->len;
        }
    }
    return r->len;
}

struct search_result lookup_flow(struct search_key sk)
{
    struct search_result sr;
    struct flow_table *flow_tbl;

    struct key *k;
    k = (void *) sk.key;

    struct res *r;
    r = (void *) sr.res_entry;
    r->len = 0;

    uint8_t is_src_tbl = 0;

    switch (sk.table_nr) {
        case SRC_TABLE:
            flow_tbl = src_flow_table_mmap();
            is_src_tbl = 1;
            lookup_flow_actions(flow_tbl, &k->mac, k->tag, r, is_src_tbl);
            sr.not_found = 0;
            break;

        case DST_TABLE:
            flow_tbl = dst_flow_table_mmap();
            is_src_tbl = 0;
            lookup_flow_actions(flow_tbl, &k->mac, k->tag, r, is_src_tbl);
            sr.not_found = 0;
            break;

        case GROUP_TABLE:
            group_table_search(k->tag, r);
            sr.not_found = r->len ? 0 : 1;
            break;

        default:
            sr.not_found = 1;
            break;
    }
    return sr;
}

int add_tag(int8_t *fb, uint16_t fb_size, uint16_t framesz, uint8_t offset,
            uint8_t *tag, uint8_t tagsz, uint8_t is_tagged)
{
    if (is_tagged) {
        return 0;
    }
    struct counter *glb_counters = get_counters();
    if (framesz + tagsz > fb_size) {
        COUNTER_ATOMIC_INC(glb_counters->drop_frame_too_long);
        return 1;
    }

    for (int j = framesz - 1; j >= offset; j--) {
        memcpy(fb + j + tagsz, fb + j, 1);
    }

    memcpy(fb + offset, tag, tagsz);
    return 0;
}

void set_tag(int8_t *fb, uint16_t framesz, uint8_t offset, 
             uint8_t *tag, uint8_t tagsz, uint8_t is_tagged)
{
    if (!is_tagged) {
        return;
    }
    memcpy(fb + offset, tag, tagsz);
    return;
}

static void del_tag(int8_t *fb, uint16_t framesz, uint8_t offset,
                   uint8_t tagsz, uint8_t is_tagged)
{
    if (!is_tagged) {
        return;
    }
    memcpy(fb + offset, fb + offset + tagsz, framesz - offset - tagsz);
}