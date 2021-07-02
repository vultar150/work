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
 * add the necessary entries to the mac address tables. 
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

#endif //NPU_VLAN_SDN_TESTS_H
