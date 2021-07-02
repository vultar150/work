#include "lib/include/memory.h"
#include "lib/include/npu.h"
#include "lib/include/helpers.h"

void replicator(void *inst_id, struct stage_fn *sfn,
                struct replicator_context *in_ctx)
{
    struct frame_buffer *fb_ptr = sfn->mmap_fb(inst_id, in_ctx->location.fb_id);
    struct frame_buffer frame;
    struct frame_buffer out_frame;
    sfn->read_fb(inst_id, (uint8_t *)fb_ptr, (uint8_t *)&frame.data, in_ctx->location.framesz);
    uint16_t valid_bytes = in_ctx->location.framesz > HEADER_SIZE ? in_ctx->location.framesz - HEADER_SIZE : 0;

    for (uint8_t i = 0; i < in_ctx->rec_cnt; i++) {
        struct header_list_entry entry = in_ctx->list[i];
        memcpy(out_frame.data, entry.header.data, entry.header.sz);
        if (valid_bytes) {
            memcpy(out_frame.data + entry.header.sz, frame.data + HEADER_SIZE, valid_bytes);
        }
        uint16_t framesz = entry.header.sz + valid_bytes;
        for (uint8_t j = 0; j < entry.port_cnt; j++) {
            output(entry.ports[j], out_frame.data, framesz);
        }
    }
}

void init_context(struct replicator_context* ctx)
{
    if (ctx == NULL) return;

    ctx->rec_cnt = 0;
    for (int i = 0; i < ETH_PORTS_NR; ++i) {
        ctx->list[i].port_cnt = 0;
    }
}
