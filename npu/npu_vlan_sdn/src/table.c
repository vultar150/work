#include "lib/include/helpers.h"

#include "table.h"

void * src_table_mmap(void)
{
    // TODO : describe the structure of static memory
    /* The memory where the hash table of source mac addresses is stored. */
//    static uint8_t mem[64 * 1024];
    static uint8_t mem[HASHNODE_COUNT * HASHNODE_CAPACITY * sizeof(struct hashnode)
                       + sizeof(struct hashtable)];
    return mem;
}

void * dst_table_mmap(void)
{
    // TODO : describe the structure of static memory
    /* The memory where the hash table of destination mac addresses is stored. */
    static uint8_t mem[HASHNODE_COUNT * HASHNODE_CAPACITY * sizeof(struct hashnode)
                       + sizeof(struct hashtable)];
    return mem;
}

void * vlan_table_mmap(void)
{
    /* The memory allocated to the vlan table.
     * For each vlan number, the table stores a list of ports and flags in which 
     * that vlan is allowed. The flag indicates whether or not a packet 
     * needs to be tagged when it is sent through that port. */

    /* The table is initialized in the first test (test_broadcast) 
     * and has the following appearance:
     * -----------------------------------------------------------------------------
     * |    vlan     |    list														|
     * -----------------------------------------------------------------------------
     * |			 |																|
     * |     1 	     | (0, NO_TAG) -> (4, NEED_TAG) -> (5, NEED_TAG)				|
     * |     2 	     | (1, NO_TAG) -> (3, NO_TAG) -> (4, NEED_TAG) -> (5, NEED_TAG) |
     * |  VLAN_NONE  | (2, NO_TAG) -> (6, NO_TAG)									|
     * -----------------------------------------------------------------------------
     * */
    static struct vlan_table tbl;

    return &tbl;
}

struct hashnode * get_head(struct hashtable *tbl, uint32_t hash)
{
    struct hashnode *ret;
    ret = tbl->nodes[hash];
    if (ret)
        ret->cc += 1;
    return ret;
}

struct hashnode * get_next(struct hashtable *tbl, uint32_t hash,
                           struct hashnode *node)
{
    node->cc -= 1;
    node = node->next;
    if (node)
        node->cc += 1;
    return node;
}

struct hashnode **
get_head_p(struct hashtable *tbl, uint32_t hash)
{
    return &(tbl->nodes[hash]);
}

struct hashnode **
get_next_p(struct hashnode **pnode)
{
    return &((*pnode)->next);
}
