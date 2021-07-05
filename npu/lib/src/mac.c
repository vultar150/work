#include <assert.h>

#include "lib/include/mac.h"
#include "lib/include/helpers.h"
#include "lib/include/test_helpers.h"
#include "lib/include/memory.h"

int mac_rx(uint8_t port_nr, uint64_t *frame_data)
{
    unsigned int rb;

    assert(inp_frame.port == port_nr);

    if (inp_frame.off == inp_frame.frame.size)
        /* Finished reading the packet. */
        return 0;

    rb = MIN(sizeof(*frame_data), inp_frame.frame.size - inp_frame.off);
    memcpy(frame_data, inp_frame.frame.buf + inp_frame.off, rb);
    inp_frame.off += rb;

    return rb;
}

void mac_tx(uint8_t port_nr, uint64_t data, int valid, int eof)
{
    if (port_nr == CTRL_PORT) {
        memcpy(ctrl_frame.frame.buf + ctrl_frame.frame.size, &data, valid);
        ctrl_frame.frame.size += valid;
        if (eof) {
/*
			printf("Out: port = %d, bytes = %u\n", port_nr, ctrl_frame.frame.size);
*/
        }
        return;
    }

    memcpy(out_frames[port_nr].frame.buf + out_frames[port_nr].frame.size,
           &data, valid);
    out_frames[port_nr].frame.size += valid;
    if (eof) {
/*
		printf("Out: port = %d, bytes = %u\n", port_nr, out_frames[port_nr].frame.size);
*/
    }
}

int receive(uint8_t in_port_nr, struct packet_context *prs_ctx,
            struct io_fn *io, struct stage_fn *sfn)
{
    prs_ctx->header.sz = 0;
    uint16_t framesz = 0; /*!< Read bytes counter. */
    int res;

    uint32_t fb_id = sfn->alloc_fb();
    struct counter *glb_counters = get_counters();

    COUNTER_ATOMIC_INC(glb_counters->in_frames[in_port_nr]);

    if (!(~fb_id)) {
        /* Can't get new buffer descriptor. */
        COUNTER_ATOMIC_INC(glb_counters->drop_frame_no_mem);
        return 1;
    }

    /* Get buffer address to write incoming frame. */
    struct frame_buffer *fb = sfn->mmap_fb(fb_id);

    while (1) {
        uint64_t r;

        res = io->rx(in_port_nr, &r);
        if (res <= 0)
            break;

        if (framesz + res > sizeof(fb->data)) {
            /* Frame is too long. */
            sfn->free_fb(fb_id);
            COUNTER_ATOMIC_INC(glb_counters->drop_frame_too_long);
            return 2;
        }

        if (framesz < HEADER_SIZE){
            memcpy(prs_ctx->header.data + framesz, &r, res);
            prs_ctx->header.sz += res;
        }

        memcpy(fb->data + framesz, &r, res);
        framesz += res;
    }

    prs_ctx->src_port = in_port_nr;
    prs_ctx->location.framesz = framesz;
    prs_ctx->location.fb_id = fb_id;

    return 0;
}

void transmit(struct io_fn *io)
{
    struct counter *glb_counters = get_counters();
    uint16_t *port_bufsz;
    uint8_t *port_buf;
    unsigned int j;

    /* Look through all port buffers in cycle and
     * send data to port if corresponding buffer is not empty. */
    for (uint8_t i = 0; i < ETH_PORTS_NR; i++) {
        port_bufsz = get_out_frames_sizes(i);
        if (!*port_bufsz)
            continue;

        port_buf = get_out_frames(i);

         /* Send the frame in parts of 8 bytes
          * until we reach the last part, which may be
          * less than 8 bytes long.
          * */
        for (j = 0; j < (*port_bufsz >> 3); j++) {
            int islast = !(*port_bufsz & 7) && j == ((*port_bufsz >> 3) - 1);
            io->tx(i, ((const uint64_t *) port_buf)[j], sizeof(uint64_t), islast);
        }
        if (*port_bufsz & 7) {
            /* Send the last part of the frame. */
            io->tx(i, ((const uint64_t *) port_buf)[j], *port_bufsz & 7, 1);
        }

        *port_bufsz = 0;
        COUNTER_ATOMIC_INC(glb_counters->out_frames[i]);
    }

    /* Similarly check buffer of controller port. */
    port_bufsz = get_ctrl_frame_size();
    if (!*port_bufsz)
        return;

    port_buf = get_ctrl_frame();

    for (j = 0; j < (*port_bufsz >> 3); j++) {
        int islast = !(*port_bufsz & 7) && j == ((*port_bufsz >> 3) - 1);
        io->tx(CTRL_PORT, ((const uint64_t *) port_buf)[j], sizeof(uint64_t), islast);
    }

    if (*port_bufsz & 7)
        io->tx(CTRL_PORT,((const uint64_t *) port_buf)[j], *port_bufsz & 7, 1);

    *port_bufsz = 0;
    COUNTER_ATOMIC_INC(glb_counters->out_ctrl_frames);
}
