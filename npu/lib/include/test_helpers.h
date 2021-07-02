#ifndef NPU_LIB_TEST_HELPERS_H
#define NPU_LIB_TEST_HELPERS_H

#include "lib/include/npu.h"

struct frame_context {
	uint8_t buf[MAX_FRAME_SIZE];
	uint16_t size;
};

struct input_frame_context {
	struct frame_context frame;
	uint16_t off;
	uint8_t port;
};

struct output_frame_context {
	struct frame_context frame;
};

/**
 * Starts the test and prints info message after it is finished.
 * */
void start_test(int (*test)(struct output_frame_context *,
							struct output_frame_context *),
				int idx);

/**
 * Emulates sending the frame to switch.
 * */
void mac_load_frame(uint8_t port_nr, const uint8_t *data, uint16_t size);

/**
 * Clears buffers that are used for testing switch output.
 * */
void reset_output_frame_buffers(void);

/**
 * Buffers used for testing switch output.
 * */
struct input_frame_context inp_frame;
struct output_frame_context out_frames[ETH_PORTS_NR];
struct output_frame_context ctrl_frame;

#endif //NPU_LIB_TEST_HELPERS_H
