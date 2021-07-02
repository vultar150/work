#include <stdio.h>

#include "lib/include/test_helpers.h"
#include "lib/include/helpers.h"

void start_test(int (*test)(struct output_frame_context *,
                            struct output_frame_context *),
                int idx)
{
    const char *info_msg = "TEST %2d   :   ";
    const char *err_msg = "ERROR!\n";
    const char *ok_msg = "OK\n";

    printf(info_msg, idx);
    if (test(out_frames, &ctrl_frame))
        printf("%s", err_msg);
    else
        printf("%s", ok_msg);
}

void mac_load_frame(uint8_t port, const uint8_t *frame, uint16_t size)
{
    memcpy(inp_frame.frame.buf, frame, size);
    inp_frame.frame.size = size;
    inp_frame.off = 0;
    inp_frame.port= port;
}

void reset_output_frame_buffers(void)
{
    for (int i = 0; i < ETH_PORTS_NR; i++) {
        out_frames[i].frame.size = 0;
    }
    ctrl_frame.frame.size = 0;
}
