#ifndef NPU_VLAN_SDN_STAGES_H
#define NPU_VLAN_SDN_STAGES_H

#include "context.h"
#include "lib/include/npu.h"

int parse(struct stage_fn *,
          struct packet_context *,
          struct packet_context *);

struct resolve_result match_action_src(struct stage_fn *,
                                       struct lookup_fn *,
                                       struct packet_context *,
                                       struct replicator_context *, 
                                       struct packet_context *);

struct resolve_result match_action_dst(struct stage_fn *,
                                       struct lookup_fn *,
                                       struct packet_context *,
                                       struct replicator_context *,
                                       struct packet_context *);

#ifdef MODEL
int replicate(struct stage_fn *,
              struct packet_context *,
              struct sw_matrix_ctx *, uint8_t);
#else
void replicate(struct stage_fn *,
               struct packet_context *);
#endif

void learn(struct learn_ctx *);

#endif //NPU_VLAN_SDN_STAGES_H
