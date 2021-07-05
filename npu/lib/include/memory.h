#ifndef NPU_LIB_MEMORY_H
#define NPU_LIB_MEMORY_H

#include <stdint.h>

/**
 * Allocates new buffer to write a frame.
 * Returns buffer descriptor (number of the first free buffer).
 * */
uint32_t shbm_alloc_frame_buf(void);

/**
 * Returns memory address corresponding to the descriptor.
 *
 * @param fb_id The frame descriptor.
 * */
struct frame_buffer * shbm_frame_buf_mmap(uint32_t fb_id);

/**
 * Marks corresponding frame buffer as free.
 *
 * @param fb_id The frame descriptor.
 * */
void shbm_free_frame_buf(uint32_t fb_id);

/**
 * Reads frame from internal memory.
 *
 * @param addr Address to read from.
 * @param data Address to write frame.
 * @param size Frame size.
 * */
void shbm_read_frame_buf(uint8_t *addr, uint8_t *data, uint32_t size);

uint8_t * get_out_frames(uint8_t port_nr);
uint16_t * get_out_frames_sizes(uint8_t port_nr);
uint8_t * get_ctrl_frame(void);
uint16_t * get_ctrl_frame_size(void);

struct counter * get_counters(void);

/**
 * Copies frame from the internal buffer to port buffer.
 *
 * @param port_nr Egress port number.
 * @param buf Internal buffer to copy from.
 * @param bufsz Frame size.
 * */
void output(uint8_t port_nr, const uint8_t *buf, uint16_t bufsz);

/**
 * Returns the pointer to port table.
 * */
void * port_mmap(void);

#endif //NPU_LIB_MEMORY_H
