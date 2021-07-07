#include "lib/include/helpers.h"
#include "lib/include/mac.h"
#include "lib/include/memory.h"

#include "tests.h"
#include "stages.h"
#include "table.h"

struct io_fn io = {
    .rx = mac_rx,
    .tx = mac_tx
};

struct stage_fn sfn = {
    .alloc_fb = shbm_alloc_frame_buf,
    .mmap_fb = shbm_frame_buf_mmap,
    .free_fb = shbm_free_frame_buf,
    .read_fb = shbm_read_frame_buf
};

struct packet_context prs_ctx;

void init_src_flow_table(void) 
{
    struct flow_table *table = src_flow_table_mmap();
    table->len = 0;
}

void init_dst_flow_table(void) 
{
    struct flow_table *table = dst_flow_table_mmap();
    table->len = 0;
}

void init_group_table(void) {
    struct group_table *table = group_table_mmap();
    table->nodes[0].group_id = 1;
    table->nodes[0].type = ALL;
    table->nodes[0].cc = 0;
    table->nodes[0].buckets[0].actions[0].type = ACTION_POP_VLAN;
    table->nodes[0].buckets[0].actions[0].value = 0;
    table->nodes[0].buckets[0].actions[1].type = ACTION_OUTPUT;
    table->nodes[0].buckets[0].actions[1].value = 0;

    table->nodes[0].buckets[0].actions[2].type = ACTION_PUSH_VLAN;
    table->nodes[0].buckets[0].actions[2].value = 0x81000001;
    table->nodes[0].buckets[0].actions[3].type = ACTION_OUTPUT;
    table->nodes[0].buckets[0].actions[3].value = 4;

    table->nodes[0].buckets[0].actions[4].type = ACTION_PUSH_VLAN;
    table->nodes[0].buckets[0].actions[4].value = 0x81000001;
    table->nodes[0].buckets[0].actions[5].type = ACTION_OUTPUT;
    table->nodes[0].buckets[0].actions[5].value = 5;

    table->nodes[0].buckets[0].act_len = 6;

    table->nodes[0].buck_len = 1;


    table->nodes[1].group_id = 2;
    table->nodes[1].type = ALL;
    table->nodes[1].cc = 0;
    table->nodes[1].buckets[0].actions[0].type = ACTION_POP_VLAN;
    table->nodes[1].buckets[0].actions[0].value = 0;
    table->nodes[1].buckets[0].actions[1].type = ACTION_OUTPUT;
    table->nodes[1].buckets[0].actions[1].value = 1;

    table->nodes[1].buckets[0].actions[2].type = ACTION_POP_VLAN;
    table->nodes[1].buckets[0].actions[2].value = 0;
    table->nodes[1].buckets[0].actions[3].type = ACTION_OUTPUT;
    table->nodes[1].buckets[0].actions[3].value = 3;

    table->nodes[1].buckets[0].actions[4].type = ACTION_PUSH_VLAN;
    table->nodes[1].buckets[0].actions[4].value = 0x81000002;
    table->nodes[1].buckets[0].actions[5].type = ACTION_OUTPUT;
    table->nodes[1].buckets[0].actions[5].value = 4;

    table->nodes[1].buckets[0].actions[6].type = ACTION_PUSH_VLAN;
    table->nodes[1].buckets[0].actions[6].value = 0x81000002;
    table->nodes[1].buckets[0].actions[7].type = ACTION_OUTPUT;
    table->nodes[1].buckets[0].actions[7].value = 5;

    table->nodes[1].buckets[0].act_len = 8;

    table->nodes[1].buck_len = 1;

    table->nodes[2].group_id = VLAN_NONE;
    table->nodes[2].type = ALL;
    table->nodes[2].cc = 0;
    table->nodes[2].buckets[0].actions[0].type = ACTION_POP_VLAN;
    table->nodes[2].buckets[0].actions[0].value = 0;
    table->nodes[2].buckets[0].actions[1].type = ACTION_OUTPUT;
    table->nodes[2].buckets[0].actions[1].value = 2;

    table->nodes[2].buckets[0].actions[2].type = ACTION_POP_VLAN;
    table->nodes[2].buckets[0].actions[2].value = 0;
    table->nodes[2].buckets[0].actions[3].type = ACTION_OUTPUT;
    table->nodes[2].buckets[0].actions[3].value = 6;

    table->nodes[2].buckets[0].act_len = 4;

    table->nodes[2].buck_len = 1;

    table->len = 3;
}

int test_broadcast(struct output_frame_context *out_frames,
                   struct output_frame_context *ctrl_frame)
{
    static uint8_t frame1[] = {
            0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0x08, 0x00, // end of ethernet header
            0x45, 0xc0, 0x00, 0x3c, 0xbd, 0x54,
            0x40, 0x00, 0x40, 0x06, 0x7e, 0xa5,
            0x7f, 0x00, 0x00, 0x01,
            0x7f, 0x00, 0x00, 0x01,
            // end of ipv4 header
            0x9d, 0xae, 0x19, 0xfd, 0x43, 0x05,
            0x1b, 0x87, 0x00, 0x00, 0x00, 0x00,
            0xa0, 0x02, 0xaa, 0xaa, 0xfe, 0x30,
            0x00, 0x00, 0x02, 0x04, 0xff, 0xd7,
            0x04, 0x02, 0x08, 0x0a, 0x3c, 0x2c,
            0xa6, 0xd3, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x03, 0x03, 0x07
    };

    static uint8_t frame2[] = {
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
            0x08, 0x00, // end of ethernet header
            0x45, 0xc0, 0x00, 0x3c, 0xbd, 0x54,
            0x40, 0x00, 0x40, 0x06, 0x7e, 0xa5,
            0x7f, 0x00, 0x00, 0x01,
            0x7f, 0x00, 0x00, 0x01,
            // end of ipv4 header
            0x9d, 0xae, 0x19, 0xfd, 0x43, 0x05,
            0x1b, 0x87, 0x00, 0x00, 0x00, 0x00,
            0xa0, 0x02, 0xaa, 0xaa, 0xfe, 0x30,
            0x00, 0x00, 0x02, 0x04, 0xff, 0xd7,
            0x04, 0x02, 0x08, 0x0a, 0x3c, 0x2c,
            0xa6, 0xd3, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x03, 0x03, 0x07
    };

    init_src_flow_table();
    init_dst_flow_table();
    init_group_table();

    reset_output_frame_buffers();
    mac_load_frame(0, frame1, sizeof(frame1));
    if (receive(0, &prs_ctx, &io, &sfn))
        return 1;
    process_model(0, &prs_ctx, &sfn);
    transmit(&io);

    for (int i = 0; i < ETH_PORTS_NR; i++) {
        if (out_frames[i].frame.size)
            return 1;
    }

    reset_output_frame_buffers();
    mac_load_frame(0, frame2, sizeof(frame2));
    if (receive(0, &prs_ctx, &io, &sfn))
        return 1;
    process_model(0, &prs_ctx, &sfn);
    transmit(&io);

    for (int i = 0; i < ETH_PORTS_NR; i++) {
        if (i == 2 || i == 6) {
            continue;
        }

        if (out_frames[i].frame.size) {
            return 1;
        }
    }


    if (out_frames[6].frame.size != sizeof(frame2) ||
        memcmp(frame2, out_frames[6].frame.buf, sizeof(frame2)) != 0) {
        return 1;
    }   

    if (out_frames[2].frame.size != sizeof(frame2) ||
        memcmp(frame2, out_frames[2].frame.buf, sizeof(frame2)) != 0) {
        return 1;
    }

    return 0;
}

int test_learn(struct output_frame_context *out_frames,
               struct output_frame_context *ctrl_frame)
{
    static uint8_t frame1[] = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x81, 0x00, 0x00, 0x02, // vlan header
            0x08, 0x00, // end of ethernet header
            0x45, 0xc0, 0x00, 0x3c, 0xbd, 0x54,
            0x40, 0x00, 0x40, 0x06, 0x7e, 0xa5,
            0x7f, 0x00, 0x00, 0x01, // src_ip
            0x7f, 0x00, 0x00, 0x01, // dst_ip
            // end of ipv4 header
            0x9d, 0xae,
            0x19, 0xfd, 0x43, 0x05, 0x1b, 0x87,
            0x00, 0x00, 0x00, 0x00, 0xa0, 0x02,
            0xaa, 0xaa, 0xfe, 0x30, 0x00, 0x00,
            0x02, 0x04, 0xff, 0xd7, 0x04, 0x02,
            0x08, 0x0a, 0x3c, 0x2c, 0xa6, 0xd3,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x03,
            0x03, 0x07
    };

    static uint8_t frame1_untagged[] = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x08, 0x00, // end of ethernet header
            0x45, 0xc0, 0x00, 0x3c, 0xbd, 0x54,
            0x40, 0x00, 0x40, 0x06, 0x7e, 0xa5,
            0x7f, 0x00, 0x00, 0x01, // src_ip
            0x7f, 0x00, 0x00, 0x01, // dst_ip
            // end of ipv4 header
            0x9d, 0xae,
            0x19, 0xfd, 0x43, 0x05, 0x1b, 0x87,
            0x00, 0x00, 0x00, 0x00, 0xa0, 0x02,
            0xaa, 0xaa, 0xfe, 0x30, 0x00, 0x00,
            0x02, 0x04, 0xff, 0xd7, 0x04, 0x02,
            0x08, 0x0a, 0x3c, 0x2c, 0xa6, 0xd3,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x03,
            0x03, 0x07
    };

    static uint8_t frame2[] = {
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x81, 0x00, 0x00, 0x02, // vlan header
            0x08, 0x00, 0x45, 0xc0, 0x00, 0x3c,
            0xbd, 0x54, 0x40, 0x00, 0x40, 0x06,
            0x7e, 0xa5, 0x7f, 0x00, 0x00, 0x01,
            0x7f, 0x00, 0x00, 0x01, 0x9d, 0xae,
            0x19, 0xfd, 0x43, 0x05, 0x1b, 0x87,
            0x00, 0x00, 0x00, 0x00, 0xa0, 0x02,
            0xaa, 0xaa, 0xfe, 0x30, 0x00, 0x00,
            0x02, 0x04, 0xff, 0xd7, 0x04, 0x02,
            0x08, 0x0a, 0x3c, 0x2c, 0xa6, 0xd3,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x03,
            0x03, 0x07
    };

    static uint8_t frame2_untagged[] = {
            0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x08, 0x00, // end of ethernet header
            0x45, 0xc0, 0x00, 0x3c,
            0xbd, 0x54, 0x40, 0x00, 0x40, 0x06,
            0x7e, 0xa5, 0x7f, 0x00, 0x00, 0x01,
            0x7f, 0x00, 0x00, 0x01, 0x9d, 0xae,
            0x19, 0xfd, 0x43, 0x05, 0x1b, 0x87,
            0x00, 0x00, 0x00, 0x00, 0xa0, 0x02,
            0xaa, 0xaa, 0xfe, 0x30, 0x00, 0x00,
            0x02, 0x04, 0xff, 0xd7, 0x04, 0x02,
            0x08, 0x0a, 0x3c, 0x2c, 0xa6, 0xd3,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x03,
            0x03, 0x07
    };

    reset_output_frame_buffers();
    mac_load_frame(1, frame1, sizeof(frame1));
    if (receive(1, &prs_ctx, &io, &sfn))
        return 1;
    process_model(1, &prs_ctx, &sfn);
    transmit(&io);

    for (int i = 0; i < ETH_PORTS_NR; i++) {
        if (i == 3 || i == 4 || i == 5)
            continue;

        if (out_frames[i].frame.size)
            return 1;
    }

    if (out_frames[3].frame.size != sizeof(frame1_untagged) ||
        memcmp(frame1_untagged, out_frames[3].frame.buf,
               sizeof(frame1_untagged)) != 0)
        return 1;

    if (out_frames[4].frame.size != sizeof(frame1) ||
        memcmp(frame1, out_frames[4].frame.buf, sizeof(frame1)) != 0)
        return 1;

    if (out_frames[5].frame.size != sizeof(frame1) ||
        memcmp(frame1, out_frames[5].frame.buf, sizeof(frame1)) != 0)
        return 1;


    struct learn_ctx lrn;
    struct ctrl_lrn_pkt *pkt = (void *) ctrl_frame->frame.buf;
    memcpy(lrn.src_mac.octets, pkt->mac, sizeof(lrn.src_mac.octets));

    lrn.vlan_tag = pkt->vlan_tag;

    lrn.actions_src[0].type = ACTION_OUTPUT;
    lrn.actions_src[0].value = pkt->port;
    lrn.actions_src[1].type = INSTR_GOTO_TABLE;
    lrn.actions_src[1].value = 1;
    lrn.act_len_src = 2;

    if (pkt->port == 4 || pkt->port == 5) {
        lrn.actions_dst[0].type = ACTION_PUSH_VLAN;
        lrn.actions_dst[0].value = 1;
    } else {
        lrn.actions_dst[0].type = ACTION_POP_VLAN;
        lrn.actions_dst[0].value = 1;
    }
    lrn.actions_dst[1].type = ACTION_OUTPUT;
    lrn.actions_dst[1].value = pkt->port;

    lrn.act_len_dst = 2;

    learn(&lrn);

    reset_output_frame_buffers();
    mac_load_frame(5, frame2, sizeof(frame2));
    if (receive(5, &prs_ctx, &io, &sfn))
        return 1;
    process_model(5, &prs_ctx, &sfn);
    transmit(&io);

    for (int i = 0; i < ETH_PORTS_NR; i++) {
        if (i == 1)
            continue;

        if (out_frames[i].frame.size)
            return 1;
    }

    if (out_frames[1].frame.size != sizeof(frame2_untagged) ||
        memcmp(frame2_untagged, out_frames[1].frame.buf,
               sizeof(frame2_untagged)) != 0)
        return 1;

    return 0;
}

int test_vlan(struct output_frame_context *out_frames,
              struct output_frame_context *ctrl_frame)
{
    static uint8_t frame[] = {
            0xf0, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xf1, 0xff, 0xff, 0xff, 0xff, 0xff,
            0x81, 0x00, 0x30, 0x01, // vlan header
            0x08, 0x00, 0x45, 0xc0, 0x00, 0x3c,
            0xbd, 0x54, 0x40, 0x00, 0x40, 0x06,
            0x7e, 0xa5, 0x7f, 0x00, 0x00, 0x01,
            0x7f, 0x00, 0x00, 0x01, 0x9d, 0xae,
            0x19, 0xfd, 0x43, 0x05, 0x1b, 0x87,
            0x00, 0x00, 0x00, 0x00, 0xa0, 0x02,
            0xaa, 0xaa, 0xfe, 0x30, 0x00, 0x00,
            0x02, 0x04, 0xff, 0xd7, 0x04, 0x02,
            0x08, 0x0a, 0x3c, 0x2c, 0xa6, 0xd3,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x03,
            0x03, 0x07
    };

    static uint8_t frame_untagged[] = {
            0xf0, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xf1, 0xff, 0xff, 0xff, 0xff, 0xff,
            0x08, 0x00, 0x45, 0xc0, 0x00, 0x3c,
            0xbd, 0x54, 0x40, 0x00, 0x40, 0x06,
            0x7e, 0xa5, 0x7f, 0x00, 0x00, 0x01,
            0x7f, 0x00, 0x00, 0x01, 0x9d, 0xae,
            0x19, 0xfd, 0x43, 0x05, 0x1b, 0x87,
            0x00, 0x00, 0x00, 0x00, 0xa0, 0x02,
            0xaa, 0xaa, 0xfe, 0x30, 0x00, 0x00,
            0x02, 0x04, 0xff, 0xd7, 0x04, 0x02,
            0x08, 0x0a, 0x3c, 0x2c, 0xa6, 0xd3,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x03,
            0x03, 0x07
    };

    reset_output_frame_buffers();
    mac_load_frame(4, frame, sizeof(frame));
    if (receive(4, &prs_ctx, &io, &sfn))
        return 1;
    process_model(4, &prs_ctx, &sfn);
    transmit(&io);

    for (int i = 0; i < ETH_PORTS_NR; i++) {
        if (i == 0 || i == 5)
            continue;

        if (out_frames[i].frame.size)
            return 1;
    }

    if (out_frames[0].frame.size != sizeof(frame_untagged) ||
        memcmp(frame_untagged, out_frames[0].frame.buf,
               sizeof(frame_untagged)) != 0) {
        return 1;
    }

    if (out_frames[5].frame.size != sizeof(frame) ||
        memcmp(frame, out_frames[5].frame.buf, sizeof(frame)) != 0) {
        return 1;
    }

    return 0;
}

int test_drop(struct output_frame_context *out_frames,
              struct output_frame_context *ctrl_frame)
{
    static uint8_t frame[] = {
            0xf0, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xf1, 0xff, 0xff, 0xff, 0xff, 0xff,
            0x81, 0x00, 0x30, 0x03, // vlan header
            0x08, 0x00, 0x45, 0xc0, 0x00, 0x3c,
            0xbd, 0x54, 0x40, 0x00, 0x40, 0x06,
            0x7e, 0xa5, 0x7f, 0x00, 0x00, 0x01,
            0x7f, 0x00, 0x00, 0x01, 0x9d, 0xae,
            0x19, 0xfd, 0x43, 0x05, 0x1b, 0x87,
            0x00, 0x00, 0x00, 0x00, 0xa0, 0x02,
            0xaa, 0xaa, 0xfe, 0x30, 0x00, 0x00,
            0x02, 0x04, 0xff, 0xd7, 0x04, 0x02,
            0x08, 0x0a, 0x3c, 0x2c, 0xa6, 0xd3,
            0x00, 0x00, 0x00, 0x00, 0x01, 0x03,
            0x03, 0x07
    };

    reset_output_frame_buffers();
    mac_load_frame(2, frame, sizeof(frame));
    if (receive(2, &prs_ctx, &io, &sfn))
        return 1;
    process_model(2, &prs_ctx, &sfn);
    transmit(&io);

    for (int i = 0; i < ETH_PORTS_NR; i++) {
        if (out_frames[i].frame.size)
            return 1;
    }

    return 0;
}
