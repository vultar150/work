#ifndef NPU_LIB_REPLICATOR_H
#define NPU_LIB_REPLICATOR_H

#include "lib/include/npu.h"

int replicator(void *inst_id, struct stage_fn *sfn, struct replicator_context *in);

void init_context(struct replicator_context* ctx);

#endif //NPU_LIB_REPLICATOR_H