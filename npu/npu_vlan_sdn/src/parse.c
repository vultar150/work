#include "lib/include/helpers.h"
#include "lib/include/memory.h"

#include "context.h"

int parse(struct stage_fn *sfn,
          struct packet_context *in_ctx, 
          struct packet_context *out_ctx)
{
	struct lookup_src_ctx *lkp = (void *) out_ctx->stage_context;
	out_ctx->location = in_ctx->location;
	lkp->src_port = in_ctx->src_port;

	out_ctx->header = in_ctx->header;

	struct counter *glb_counters = get_counters();
	if (in_ctx->location.framesz < sizeof(struct eth_hdr)) {
		COUNTER_ATOMIC_INC(glb_counters->drop_frame_invalid);
		sfn->free_fb(in_ctx->location.fb_id);
		return 1;
	}
	
	struct eth_hdr_dot1q *eth_vlan = (void *)(in_ctx->header.data);

	lkp->is_tagged = !!(NTOH16(eth_vlan->tpid) == 0x8100);
	if (lkp->is_tagged) {
		if (in_ctx->location.framesz < sizeof(*eth_vlan)) {
			COUNTER_ATOMIC_INC(glb_counters->drop_frame_invalid);
			sfn->free_fb(in_ctx->location.fb_id);
			return 1;
		}
		/* Translate from the network big endian to a representation 
		 * for the device architecture, then apply a mask and 
		 * cut off the unnecessary 4 bits. */
		lkp->vlan_vid = NTOH16(eth_vlan->tag) & 0x0fff;
		lkp->vlan_pcp = NTOH16(eth_vlan->tag) >> 12;
	} else {
		lkp->vlan_vid = VLAN_NONE;
		lkp->vlan_pcp = 0;
	}

	memcpy(lkp->src_mac.octets, eth_vlan->src, sizeof(lkp->src_mac.octets));
	if (is_bcast_mac(&lkp->src_mac)) {
		/* If the sender's address is broadcast, drop the packet. */
		COUNTER_ATOMIC_INC(glb_counters->drop_frame_invalid);
		sfn->free_fb(in_ctx->location.fb_id);
		return 1;
	}
	memcpy(lkp->dst_mac.octets, eth_vlan->dst, sizeof(lkp->dst_mac.octets));

	return 0;
}
