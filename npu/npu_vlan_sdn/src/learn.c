#include "lib/include/helpers.h"

#include "table.h"
#include "context.h"

void learn(struct learn_ctx *in)
{
    struct hashnode *hdel, **hh, *hnodes, *hn;
    struct hashtable *table;
    uint32_t src_hash;
    void *mem;

    /* Start adding a new record to source MAC hashtable. */
    mem = src_table_mmap();
    table = mem;

    /* Get the pointer to memory area with nodes of the hashtable. */
    hnodes = (void *)((uint8_t *) mem + sizeof(*table));

    if (table->next_node == HASHNODE_COUNT)
        table->next_node = 0;

    hdel = hnodes + table->next_node;

    /* Calculate hash of MAC address and lock the corresponding hashnode. */
    src_hash = get_mac_hash(&hdel->mac);

    for (hh = get_head_p(table, src_hash); *hh; hh = get_next_p(hh)) {
        if (*hh == hdel) {
            *hh = hdel->next;
            break;
        }
    }

    while (hdel->next) {
        if (hdel->cc == 0)
            hdel->next = NULL;
    }

    hn = hnodes + table->next_node++;

    hn->mac = in->src_mac;
    hn->port = in->src_port;
    hn->vlan = in->vlan_tag;
    hn->next = HASHTABLE_GET(table, in->src_hash);
    HASHTABLE_GET(table, in->src_hash) = hn;

    /* The same actions are performed to add a new record
     * to destination MAC hashtable. */
    mem = dst_table_mmap();
    table = mem;
    hnodes = (void *)((uint8_t *) mem + sizeof(*table));

    if (table->next_node == HASHNODE_COUNT)
        table->next_node = 0;

    hdel = hnodes + table->next_node;

    src_hash = get_mac_hash(&hdel->mac);

    for (hh = get_head_p(table, src_hash); *hh; hh = get_next_p(hh)) {
        if (*hh == hdel) {
            *hh = hdel->next;
            break;
        }
    }

    while (hdel->next) {
        if (hdel->cc == 0)
            hdel->next = NULL;
    }

    hn = hnodes + table->next_node++;

    hn->mac = in->src_mac;
    hn->port = in->src_port;
    hn->vlan = in->vlan_tag;
    hn->next = HASHTABLE_GET(table, in->src_hash);
    HASHTABLE_GET(table, in->src_hash) = hn;
}
