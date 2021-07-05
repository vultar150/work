#ifndef NPU_LIB_MAC_H
#define NPU_LIB_MAC_H

#include "lib/include/npu.h"

/**
 * Emulates reading 8 bytes from port.
 * In NPU model it is called every clock.
 *
 * @param port_nr [in] Ingress port number.
 * @param frame_data [out] Variable to store the incoming data.
 * */
int mac_rx(uint8_t port_nr, uint64_t *frame_data);

/**
 * Emulates writing 8 bytes to port.
 * In NPU model it is called every clock.
 *
 * @param port_nr Egress port number.
 * @param data The data to be sent.
 * @param valid Number of bytes to be sent.
 * @param eof Flag to mark the end of transmission process.
 * */
void mac_tx(uint8_t port_nr, uint64_t data, int valid, int eof);

/**
 * Emulates receiver.
 *
 * @param inst_id Used in NPU model.
 * @param in_port_nr Ingress port number.
 * @param prs_ctx Context for Parse stage.
 * */
int receive(uint8_t in_port_nr, struct packet_context *prs_ctx,
            struct io_fn *io, struct stage_fn *sfn);

/**
 * Emulates transmitter.
 * */
void transmit(struct io_fn *io);

#endif //NPU_LIB_MAC_H
