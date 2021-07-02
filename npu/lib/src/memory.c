#include "lib/include/helpers.h"
#include "lib/include/npu.h"
#include "lib/include/memory.h"

// TODO : merge into one structure
/**
 *  Internal frame buffers of the switch.
 * */
static struct frame_buffer shbm_frame_buffers[TOTAL_FRAME_BUFFERS_NR];
/**
 *  Array of flags for corresponding frame buffers (free / not free).
 * */
static int shbm_frame_buffers_in_use[TOTAL_FRAME_BUFFERS_NR];

uint32_t shbm_alloc_frame_buf(void *inst_id)
{
    for (unsigned int i = 0; i < TOTAL_FRAME_BUFFERS_NR; i++) {
        if (!shbm_frame_buffers_in_use[i]) {
            shbm_frame_buffers_in_use[i] = 1;
            return i;
        }
    }

    return ~0;
}

struct frame_buffer * shbm_frame_buf_mmap(void *inst_id, uint32_t fb_id)
{
    if (TOTAL_FRAME_BUFFERS_NR <= fb_id)
        return 0;

    return &shbm_frame_buffers[fb_id];
}

void shbm_free_frame_buf(void *inst_id, uint32_t fb_id)
{
    if (TOTAL_FRAME_BUFFERS_NR > fb_id)
        shbm_frame_buffers_in_use[fb_id] = 0;
}

void shbm_read_frame_buf(void *inst_id, uint8_t *addr, uint8_t *data, uint32_t size)
{
    memcpy(data, addr, size);
}

// TODO : merge into one structure
/**
 *  Frame buffers of outcoming ports.
 * */
static uint8_t out_frames[ETH_PORTS_NR][MAX_FRAME_SIZE];
static uint8_t ctrl_frame[MAX_FRAME_SIZE];
/**
 * Size of frame placed in the corresponding buffer.
 * */
static uint16_t out_frames_sizes[ETH_PORTS_NR];
static uint16_t ctrl_frame_size;

uint8_t * get_out_frames(uint8_t port_nr)
{
    return out_frames[port_nr];
}

uint16_t * get_out_frames_sizes(uint8_t port_nr)
{
    return &out_frames_sizes[port_nr];
}

uint8_t * get_ctrl_frame(void)
{
    return ctrl_frame;
}

uint16_t * get_ctrl_frame_size(void)
{
    return &ctrl_frame_size;
}

struct counter glb_counters;

struct counter * get_counters(void)
{
    return &glb_counters;
}

void output(uint8_t port_nr, const uint8_t *buf, uint16_t bufsz)
{
    uint8_t *port_buf;
    uint16_t *port_bufsz;

    if (port_nr == CTRL_PORT) {
        port_buf = get_ctrl_frame();
        port_bufsz = get_ctrl_frame_size();
    } else {
        port_buf = get_out_frames(port_nr);
        port_bufsz = get_out_frames_sizes(port_nr);
    }

    memcpy(port_buf, buf, bufsz);
    *port_bufsz = bufsz;
}

void * port_mmap(void)
{
    /**
     * Table of port states.
     * */
    static struct ofp_port tbl[ETH_PORTS_NR]  = {
        { .state = OFPPS_LIVE },    /* port 0 */
        { .state = OFPPS_LIVE },    /* port 1 */
        { .state = OFPPS_LIVE },    /* port 2 */
        { .state = OFPPS_LIVE },    /* port 3 */
        { .state = OFPPS_LIVE },    /* port 4 */
        { .state = OFPPS_LIVE },    /* port 5 */
        { .state = OFPPS_LIVE },    /* port 6 */
        { .state = OFPPS_LIVE },    /* port 7 */
    };

    return tbl;
}
