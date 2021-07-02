#include <lib/include/replicator.h>
#include "lib/include/helpers.h"
#include "lib/include/memory.h"

#include "context.h"
#include "table.h"


int add_tag(int8_t *fb, uint16_t framesz, uint8_t offset,
            uint8_t *tag, uint8_t tagsz)
#if defined(__GNUC__)
__attribute__((warn_unused_result))
#endif
;

static int del_tag(int8_t *fb, uint16_t framesz, uint8_t offset,
                   uint8_t tagsz)
#if defined(__GNUC__)
__attribute__((warn_unused_result))
#endif
;

static tag_task_t add_to_resolve_list(uint8_t is_tagged,
                                struct vlan_node_item *);

static uint8_t lookup_port(struct hashtable *, uint32_t,
						   struct ethaddr *, uint16_t);


struct resolve_result match_action_src(void *inst_id, struct stage_fn *sfn,
                                       struct lookup_fn *lfn,
                                       struct packet_context *lkp_ctx,
                                       struct packet_context *rsv_ctx,
                                       struct replicator_context *repl_ctx)
{
    struct lookup_src_ctx *lkp = (void *) lkp_ctx->stage_context;
    struct resolve_src_ctx *rsv = (void *) rsv_ctx->stage_context;
    rsv_ctx->location = lkp_ctx->location;
    rsv_ctx->header = lkp_ctx->header;

    rsv->src_port = lkp->src_port;

    rsv->src_mac = lkp->src_mac;
    rsv->dst_mac = lkp->dst_mac;
    rsv->src_hash = get_mac_hash(&lkp->src_mac);

    rsv->is_tagged = lkp->is_tagged;
    rsv->vlan_vid = lkp->vlan_vid;
    rsv->vlan_pcp = lkp->vlan_pcp;

    struct search_key sk;
    sk.table_nr = SRC_TABLE;

    struct key *k = (void *) sk.key;
    k->hash = rsv->src_hash;
    k->mac = lkp->src_mac;
    k->tag = lkp->vlan_vid;

    rsv->src_table_port = lfn->search_rd(inst_id, sk).res_entry[0];

    struct lookup_dst_ctx *lkp_dst = (void *) lkp_ctx->stage_context;

    struct resolve_result res;
    res.next_stage = 1;
    res.out_port = -1;
    res.drop = 0;

    lkp_dst->src_port = rsv->src_port;

    lkp_dst->is_tagged = rsv->is_tagged;
    lkp_dst->vlan_vid = rsv->vlan_vid;
    lkp_dst->vlan_pcp = rsv->vlan_pcp;

    lkp_dst->dst_mac = rsv->dst_mac;

    if (rsv->src_port == rsv->src_table_port)
        return res;

    init_context(repl_ctx);
    uint32_t fb_id = sfn->alloc_fb(NULL);
    struct frame_buffer* fb = sfn->mmap_fb(NULL, rsv_ctx->location.fb_id);
    struct frame_buffer* new_fb = sfn->mmap_fb(NULL, fb_id);
    memcpy(new_fb->data, fb->data, rsv_ctx->location.framesz);

    /* Form a packet from the src_mac, src_port, vlan_vid, src_hash. */
    struct ctrl_lrn_pkt pkt;
    repl_ctx->rec_cnt = 1;
    pkt.port = rsv->src_port;
    pkt.src_hash = rsv->src_hash;
    pkt.vlan_tag = rsv->vlan_vid;
    memcpy(pkt.mac, rsv->src_mac.octets, sizeof(pkt.mac));
    memcpy(repl_ctx->list[0].header.data, &pkt, sizeof(pkt));
    repl_ctx->list[0].header.sz = rsv_ctx->header.sz;
    repl_ctx->list[0].port_cnt = 1;
    repl_ctx->list[0].ports[0] = CTRL_PORT;
    repl_ctx->location.fb_id = fb_id;
    repl_ctx->location.framesz = rsv_ctx->location.framesz;

    res.out_port = CTRL_PORT;
    return res;
}


struct resolve_result match_action_dst(void *inst_id, struct stage_fn *sfn,
                                       struct lookup_fn *lfn,
                                       struct packet_context *lkp_ctx,
                                       struct packet_context *rsv_ctx,
                                       struct replicator_context *repl_ctx)
{
    struct lookup_dst_ctx *lkp = (void *) lkp_ctx->stage_context;
    struct resolve_dst_ctx *rsv = (void *) rsv_ctx->stage_context;
    rsv_ctx->location = lkp_ctx->location;
    rsv_ctx->header = lkp_ctx->header;

    rsv->src_port = lkp->src_port;

    rsv->is_tagged = lkp->is_tagged;
    rsv->vlan_vid = lkp->vlan_vid;
    rsv->vlan_pcp = lkp->vlan_pcp;

    struct search_key sk;
    struct key *k = (void *) sk.key;

    if (!is_bcast_mac(&lkp->dst_mac)) {
        /* We look in the hash table for the port 
         * to which we want to send the packet.*/
        sk.table_nr = DST_TABLE;
        k->hash = get_mac_hash(&lkp->dst_mac);
        k->mac = lkp->dst_mac;
        k->tag = lkp->vlan_vid;

        rsv->dst_port = lfn->search_rd(inst_id, sk).res_entry[0];
    } else {
        rsv->dst_port = PORT_NOT_FOUND;
    }

    sk.table_nr = VLAN_TABLE;
    k->tag = lkp->vlan_vid;
    k->src_port = lkp->src_port;
    k->dst_port = rsv->dst_port;

    struct search_result sr = lfn->search_rd(inst_id, sk);
    struct res *r = (void *) sr.res_entry;
    rsv->len = r->len;
    if (rsv->len)
        memcpy(rsv->items, r->items, rsv->len * sizeof(rsv->items[0]));

    struct resolve_result result;
    result.drop = 0;
    result.out_port = 1;
    result.next_stage = 0;

    init_context(repl_ctx);
    repl_ctx->location = rsv_ctx->location;

    uint8_t vlan_tag[4] = { 0x81, 0x00,
                            rsv->vlan_pcp | rsv->vlan_vid >> 8, rsv->vlan_vid & 0xff };

    for (uint8_t i = 0; i < rsv->len;i++) {
        switch (add_to_resolve_list(rsv->is_tagged, &rsv->items[i])) {
            case NO_CHNG:
                repl_ctx->list[repl_ctx->rec_cnt].header = rsv_ctx->header;
                repl_ctx->list[repl_ctx->rec_cnt].ports[repl_ctx->list[repl_ctx->rec_cnt].port_cnt] =
                    (uint8_t) rsv->items[repl_ctx->rec_cnt].port;
                repl_ctx->list[repl_ctx->rec_cnt].port_cnt++;
                repl_ctx->rec_cnt++;
                break;

            case ADD_TAG:
                repl_ctx->list[repl_ctx->rec_cnt].header = rsv_ctx->header;
                if (add_tag(repl_ctx->list[repl_ctx->rec_cnt].header.data,
                            repl_ctx->list[repl_ctx->rec_cnt].header.sz,
                            MEMBER_SIZE(struct eth_hdr, dst) + MEMBER_SIZE(struct eth_hdr, src),
                            vlan_tag, sizeof(vlan_tag))) {

                    sfn->free_fb(inst_id, rsv_ctx->location.fb_id);
                    result.drop = 1;
                    return result;
                }
                repl_ctx->list[repl_ctx->rec_cnt].header.sz += sizeof(vlan_tag);
                repl_ctx->list[repl_ctx->rec_cnt].ports[repl_ctx->list[repl_ctx->rec_cnt].port_cnt] =
                    (uint8_t) rsv->items[repl_ctx->rec_cnt].port;
                repl_ctx->list[repl_ctx->rec_cnt].port_cnt++;
                repl_ctx->rec_cnt++;
                break;

            case DEL_TAG:
                repl_ctx->list[repl_ctx->rec_cnt].header = rsv_ctx->header;
                if (del_tag(repl_ctx->list[repl_ctx->rec_cnt].header.data,
                            repl_ctx->list[repl_ctx->rec_cnt].header.sz,
                            MEMBER_SIZE(struct eth_hdr, dst) + MEMBER_SIZE(struct eth_hdr, src),
                            sizeof(vlan_tag))) {

                    sfn->free_fb(inst_id, rsv_ctx->location.fb_id);
                    result.drop = 1;
                    return result;
                }
                repl_ctx->list[repl_ctx->rec_cnt].header.sz -= sizeof(vlan_tag);
                repl_ctx->list[repl_ctx->rec_cnt].ports[repl_ctx->list[repl_ctx->rec_cnt].port_cnt] =
                        (uint8_t) rsv->items[repl_ctx->rec_cnt].port;
                repl_ctx->list[repl_ctx->rec_cnt].port_cnt++;
                repl_ctx->rec_cnt++;
                break;

            default:
                break;
        }
    }
    return result;
}


/* Search for the desired port number from the hash table.
 * If it cannot be found, it comes back 0xff. */
static uint8_t lookup_port(struct hashtable *tbl, uint32_t hash,
						   struct ethaddr* mac, uint16_t tag)
{
	struct hashnode *h;

	for (h = get_head(tbl, hash); h; h = get_next(tbl, hash, h))
		if (is_same_mac(&h->mac, mac) && h->vlan == tag)
			return h->port;

	return PORT_NOT_FOUND;
}

static uint8_t vlan_table_search(uint8_t dst_port, uint8_t src_port,
								 uint16_t tag, struct vlan_node_item *list)
{
	struct vlan_table *tbl = vlan_table_mmap();
	uint8_t len = 0;
	struct vlan_node_item *items = tbl->nodes[tag].items;

	if (dst_port != PORT_NOT_FOUND) {
		for (uint8_t i = 0; i < tbl->nodes[tag].len; i++) {
			if (items[i].port == dst_port) {
				list[len].port = items[i].port;
				list[len++].mode = items[i].mode;
			}
		}
	} else {
		for (uint8_t i = 0; i < tbl->nodes[tag].len; i++) {
			if (items[i].port != src_port) {
				list[len].port = items[i].port;
				list[len++].mode = items[i].mode;
			}
		}
	}

	return len;
}

struct search_result lookup_switch(void *inst_id, struct search_key sk)
{
    struct search_result sr;
    struct hashtable *hash_tbl;

    struct key *k;
    k = (void *) sk.key;

    struct res *r;
    r = (void *) sr.res_entry;

    switch (sk.table_nr) {
        case SRC_TABLE:
            hash_tbl = src_table_mmap();
            sr.res_entry[0] = lookup_port(hash_tbl, k->hash, &k->mac, k->tag);
            sr.not_found = 0;
            break;

        case DST_TABLE:
            hash_tbl = dst_table_mmap();
            sr.res_entry[0] = lookup_port(hash_tbl, k->hash, &k->mac, k->tag);
            sr.not_found = 0;
            break;

        case VLAN_TABLE:
            r->len = vlan_table_search(k->dst_port, k->src_port, k->tag, r->items);
            sr.not_found = r->len ? 0 : 1;
            break;

        default:
            sr.not_found = 1;
            break;
    }

    return sr;
}


static tag_task_t add_to_resolve_list(uint8_t is_tagged,
                               struct vlan_node_item *item)
{
    if (is_tagged) {
        return ((item->mode == NO_TAG) ? DEL_TAG : NO_CHNG);
    } else {
        return ((item->mode == NO_TAG) ? NO_CHNG : ADD_TAG);
    }
}

int add_tag(int8_t *fb, uint16_t framesz, uint8_t offset,
            uint8_t *tag, uint8_t tagsz)
{
    struct counter *glb_counters = get_counters();
    if (framesz + tagsz > sizeof(*fb)) {
        COUNTER_ATOMIC_INC(glb_counters->drop_frame_too_long);
        return 1;
    }

    for (int j = framesz - 1; j >= offset; j--) {
        memcpy(fb + j + tagsz, fb + j, 1);
    }

    memcpy(fb + offset, tag, tagsz);
    return 0;
}

static int del_tag(int8_t *fb, uint16_t framesz, uint8_t offset,
                   uint8_t tagsz)
{
    memcpy(fb + offset, fb + offset + tagsz, framesz - offset - tagsz);
    return 0;
}