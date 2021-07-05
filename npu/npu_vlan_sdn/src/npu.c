#include <lib/include/replicator.h>
#include "lib/include/helpers.h"
#include "lib/include/mac.h"
#include "lib/include/memory.h"

#include "context.h"
#include "stages.h"
#include "table.h"

void process_model(uint8_t in_port_nr,
                   struct io_fn *io, 
                   struct packet_context *prs_ctx,
                   struct stage_fn *sfn)
{
    struct packet_context lkp_ctx;
    struct packet_context rsv_ctx;
    struct replicator_context repl_ctx;

    struct resolve_result rr;

    struct lookup_fn lfn = {
        .search_rd = lookup_switch
    };

    if (parse(NULL, sfn, prs_ctx, &lkp_ctx))
        return;

    rr = match_action_src(NULL, sfn, &lfn, &lkp_ctx, &rsv_ctx, &repl_ctx);
    if (rr.out_port >= 0) {
        replicator(NULL, sfn, &repl_ctx);
    }

    if (match_action_dst(NULL, sfn, &lfn, &lkp_ctx, &rsv_ctx, &repl_ctx).drop)
        return;

    replicator(NULL, sfn, &repl_ctx);
}
