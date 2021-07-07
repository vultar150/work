#include "lib/include/helpers.h"

#include "table.h"


void * src_flow_table_mmap(void)
{
    static struct flow_table tbl;
    return &tbl;
}

void * dst_flow_table_mmap(void)
{
    static struct flow_table tbl;
    return &tbl;
}

void * group_table_mmap(void)
{
    /* The group table has the following fields: 
     * identifier (id) which matches the vlan tag of the corresponding packet, 
     * group type (ALL or INDIRECT) and an ordered list of bucket, 
     * where each bucket contains a list of actions. */
    
    /* The table is initialized in the first test (test_broadcast)
     * and has the following appearance (one bucket for every group):
     * ------------------------------------------------------------------------
     * |    id       |    type    |        buckets                            |
     * ------------------------------------------------------------------------
     * |             |            |                                           |
     * |     1       |    ALL     | {ACTION_POP_VLAN,  ACTION_OUTPUT 0,        |
     * |             |            |  ACTION_PUSH_VLAN, ACTION_OUTPUT 4,       |
     * |             |            |  ACTION_PUSH_VLAN, ACTION_OUTPUT 5}       |
     * ------------------------------------------------------------------------
     * |             |            |                                           |
     * |             |            | {ACTION_POP_VLAN,  ACTION_OUTPUT 1,       |
     * |     2       |    ALL     |  ACTION_POP_VLAN,  ACTION_OUTPUT 3,       |
     * |             |            |  ACTION_PUSH_VLAN, ACTION_OUTPUT 4,       |
     * |             |            |  ACTION_PUSH_VLAN, ACTION_OUTPUT 5}       |
     * ------------------------------------------------------------------------
     * |             |            |                                           |
     * |  VLAN_NONE  |    ALL     | {ACTION_POP_VLAN,  ACTION_OUTPUT 2,       |
     * |             |            |  ACTION_POP_VLAN,  ACTION_OUTPUT 6}       |
     * ------------------------------------------------------------------------
     * */
    static struct group_table tbl;
    return &tbl;
}


