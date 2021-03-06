#ifndef NPU_VLAN_SDN_TESTS_H
#define NPU_VLAN_SDN_TESTS_H

#include "lib/include/test_helpers.h"

/* Port 0 receives a frame with a broadcast source address.
 * Frame drop pending.
 * Then a frame with a broadcast destination address arrives on port 0.
 * Expect to send a packet to ports 2 and 6 unchanged. */
int test_broadcast(struct output_frame_context *,
                   struct output_frame_context *);

/* At port 4 comes the frame with the VLAN tag 1.
 * Expect to send to port 0 without a tag and to port 5 with a tag. */
int test_vlan(struct output_frame_context *,
              struct output_frame_context *);

/* At port 1 comes the frame with the VLAN tag 2.
 * Expecting a send to all ports with VLAN tag 2:
 * 1) to port 3 without the tag
 * 2) to port 4 with the tag
 * 3) to port 5 with the tag
 * + sending a special packet to the controller, which will then 
 * add the necessary entries to flow tables. 
 * Then another frame comes to port 5 with vlan tag 2 
 * and a known destination mac address. 
 * Expect it to be sent only to port 1 without the tag. */
int test_learn(struct output_frame_context *,
               struct output_frame_context *);

/* A packet arrives on port 2 with vlan tag 3.
 * Expect the packet to be dropped because no ports are found 
 * where vlan 3 is allowed. */
int test_drop(struct output_frame_context *,
              struct output_frame_context *);

/* At port 5 comes the frame with no vlan tag.
 * Expect to send a packet to ports 2 and 6.
 * After that sending a special packet to the controller, which will
 * then add the necessary entries to flow tables (for dst flow table 
 * action list is: ACTION_PUSH_VLAN 1, ACTION_OUTPUT 5). Then
 * another frame comes to port 0 with no vlan tag and with 
 * dst_mac = src_mac of first frame. Expect it to be sent 
 * only to port 5 with vlan tag 1 */
int test_push_vlan(struct output_frame_context *out_frames,
                   struct output_frame_context *ctrl_frame);

/* Add some entry to flow tables with specific mac, vlan 1 
 * and actions for dst table: ACTION_SET_VLAN 2, ACTION_OUTPUT 5.
 * At port 0 comes the frame with vlan tag 1 and with dst mac addr of 
 * added entry.
 * Expect it to be sent only to port 5 with changed vlan tag 2 */
int test_set_vlan(struct output_frame_context *out_frames,
                  struct output_frame_context *ctrl_frame);

#endif //NPU_VLAN_SDN_TESTS_H
