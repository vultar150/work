#include "lib/include/helpers.h"
#include "lib/include/memory.h"

#include "context.h"

static void add_tag(struct frame_buffer *, struct buf_info *,
                    uint16_t, uint16_t, uint16_t vlan_pcp,
                    struct stage_fn *mem);

static void del_tag(struct frame_buffer *, struct buf_info *,
                    uint16_t, struct stage_fn *mem);

void replicate(struct stage_fn *sfn, struct packet_context *in_ctx)
{
    struct replicate_ctx *in = (void *) in_ctx->stage_context;

    struct frame_buffer *fb = sfn->mmap(inst_id, in_ctx->location.fb_id);
    struct buf_info new_buf;

    for (uint8_t i = 0; i < in->list.count; i++)
        switch (in->list.mode[i]) {
            case NO_CHNG:
                output(in->list.ports[i], fb->data, in_ctx->location.framesz);
                break;
            case ADD_TAG:
                add_tag(inst_id, fb, &new_buf, in_ctx->location.framesz,
                        in->vlan_vid, in->vlan_pcp, sfn);

                if (!new_buf.fb)
                    return;

                output(in->list.ports[i], new_buf.fb->data,
                       in_ctx->location.framesz + 4);
                sfn->free(inst_id, new_buf.fb_id);
                break;
            case DEL_TAG:
                del_tag(inst_id,
                        fb, &new_buf,
                        in_ctx->location.framesz,
                        sfn);

                if (!new_buf.fb)
                    return;

                output(in->list.ports[i], new_buf.fb->data,
                       in_ctx->location.framesz - 4);
                sfn->free(inst_id, new_buf.fb_id);
                break;

            default:
                break;
        }

    sfn->free(inst_id, in_ctx->location.fb_id);
}

static void del_tag(struct frame_buffer *fb, struct buf_info *new_buf,
                    uint16_t framesz, struct stage_fn *mem)
{
    struct counter *glb_counters = get_counters();
    /* Make a copy of the package. */
    new_buf->fb_id = mem->alloc(inst_id);

    if (!(~new_buf->fb_id)) {
        COUNTER_ATOMIC_INC(glb_counters->drop_frame_no_mem);
        return;
    }

    new_buf->fb = mem->mmap(inst_id, new_buf->fb_id);
    memcpy(new_buf->fb->data, fb->data, framesz);

    /* Take the tag off. */
    memcpy(new_buf->fb->data + 2 * sizeof(struct ethaddr),
           new_buf->fb->data + 2 * sizeof(struct ethaddr)
           + 2 * sizeof(uint16_t),
           framesz - 2 * sizeof(struct ethaddr)
           - 2 * sizeof(uint16_t));
}

static void add_tag(struct frame_buffer *fb, struct buf_info *new_buf,
                    uint16_t framesz, uint16_t vlan_vid, uint16_t vlan_pcp,
                    struct stage_fn *mem)
{
    struct counter *glb_counters = get_counters();
    int j;

    /* Request a new buffer to copy the package. */
    new_buf->fb_id = mem->alloc(inst_id);

    if (!(~new_buf->fb_id)) {
        COUNTER_ATOMIC_INC(glb_counters->drop_frame_no_mem);
        return;
    }

    new_buf->fb = mem->mmap(inst_id, new_buf->fb_id);
    if (framesz + 4 > sizeof(new_buf->fb->data)) {
        COUNTER_ATOMIC_INC(glb_counters->drop_frame_too_long);
        return;
    }
    /* Make a copy of the package. */
    memcpy(new_buf->fb->data, fb->data, framesz);
    /* Form a new tag. */
    uint32_t tag = HTON32(0x81000000 | vlan_vid | (vlan_pcp << 12));

    /* First we copy part of the frame, 
     * so as not to lose any data. */
    for (j = framesz - 1; j > 11; j--)
        memcpy(new_buf->fb->data + j + 4, new_buf->fb->data + j, 1);

    /* Putting a tag on it. */
    memcpy(new_buf->fb->data + 2 * sizeof(struct ethaddr),
           (void *) &tag, sizeof(tag));
}
