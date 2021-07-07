#include "lib/include/helpers.h"

#include "table.h"
#include "context.h"


void learn(struct learn_ctx *in) 
{
    /* Add entry to src flow table */
    struct flow_table *table = src_flow_table_mmap();
    int inp_idx = 0; /* index of input place */
    while (inp_idx < table->len) {
        if (!table->nodes[inp_idx].valid) {
            break;
        }
        inp_idx++;
    }

    if (inp_idx < MAX_FLOW_TABLE_SIZE) {
        flow_entry_t *node = &(table->nodes[inp_idx]);
        node->mac = in->src_mac;
        node->vlan_tag = in->vlan_tag;
        node->cc = 0;
        for (int j = 0; j < in->act_len_src; j++) {
            node->actions[j].type = in->actions_src[j].type;
            node->actions[j].value = in->actions_src[j].value;
        }
        node->act_len = in->act_len_src;
        node->valid = 1;
        if (inp_idx == table->len) {
            table->len++;
        }
    }

    /* Add entry to dst flow table */
    table = dst_flow_table_mmap();
    inp_idx = 0;
    while (inp_idx < table->len) {
        if (!table->nodes[inp_idx].valid) {
            break;
        }
        inp_idx++;
    }

    if (inp_idx < MAX_FLOW_TABLE_SIZE) {
        flow_entry_t *node = &(table->nodes[inp_idx]);
        node->mac = in->src_mac;
        node->vlan_tag = in->vlan_tag;
        node->cc = 0;
        for (int j = 0; j < in->act_len_dst; j++) {
            node->actions[j].type = in->actions_dst[j].type;
            node->actions[j].value = in->actions_dst[j].value;
        }
        node->act_len = in->act_len_dst;
        node->valid = 1;
        if (inp_idx == table->len) {
            table->len++;
        }
    }
}

