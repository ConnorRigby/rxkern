/*
Copyright 2022 connorr@hey.com 

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/*
 * This file along with it's implementation implements the loosely defined by the
 * ISO-15765 spec
 * 
 * USAGE:
 * during startup, initialize a buffer for use. The length of this buffer
 * may be variable, it should  technically be up to 4095 (0xfff) bytes long,
 * but this probably will not fit in RAM, so it's okay for it to be chunked
 * signaling must be implemented out of band from this implementation.
 * 
 *    iso_tp_init(BUFFER_LOCATION);
 * 
 * Frames should be decoded in 8 byte chunks, this is perfect for usage in
 * a CAN message data register
 *    
 *    internal_rx_can_message(CAN_MSG_DATA_REGISTER); // CAN rx 8 bytes 
 *    status = iso_tp_decode(CAN_MSG_DATA_REGISTER);
 * 
 * status will be one of a few values:
 *    
 *    ISO_TP_STATUS_SF          - SingleFrame, length is less than 8 bytes long, so it fit in one message
 *    ISO_TP_STATUS_FF          - FirstFrame,  length is > 7 < 4085 bytes long, payload is not ready to be consumed yet (note 1)
 *    ISO_TP_STATUS_CF_WAIT     - ContinuousFrame, payload is not ready to be consumed yet
 *    ISO_TP_STATUS_CF_COMPLETE - ContinuoutFrame, payload is ready to be consumed (note 1) 
 *    ISO_TP_STATUS_FC          - FlowControl, indicator that a FF may be Continued
 * 
 * Handling of these statuses is left to the implementation, however there is a couple guidlines that must be followed
 * 
 * When receiving a ISO_TP_STATUS_FF, a ISO_TP_FC frame must be dispatched
 * 
 *    if(status == ISO_TP_STATUS_FF) {
 *       CAN_MSG_DATA_REGISTER = {0x30, 0, 0, 0, 0, 0, 0, 0};
 *       internal_tx_can_message(CAN_MSG_DATA_REGISTER); 
 *    }
 * 
 * Once a message has been handled, a response can be prepared for dispatching 
 * 
 *    status = iso_tp_prepare_response(responose_length, CAN_MSG_DATA_REGISTER);
 * 
 * Similar to the decode function this function operates in 8 byte chunks, it will
 * return a status that can be used for internal signaling:
 * 
 *    ISO_TP_STATUS_CF_WAIT - output is still in progress
 *    ISO_TP_STATUS_CF_COMPLETE - output complete
 * 
 * The other statuses are the same as decode. Internal signaling must be implemented to
 * distinguish between calling iso_tp_prepare_response or iso_tp_decode. Only one
 * may be called at a time.
 * 
 * Note 1:
 * 
 * While handling Continuous Frames, it may be the that the full payload will not fit into the
 * buffer space. This is true for example with firmware updates. If this is the case,
 * a workaround is to handle chunking via the ISO_TP_STATUS_CF_WAIT status. Each frame is
 * 7 bytes wide, therefor usage of the iso_tp_decode function may be overloaded out of band.
 * The only requirement for this is the buffer index must be "rewound" whenever a chunk is processed.
 * use iso_tp_decode_rewind(length) to rewind the buffer pointer back to the start of a chunk.
 * 
 * The implication of this is that the buffer will (probably) not be valid when the entire
 * payload transfer is complete. It will only contain at max, the full last chunk.
 * 
 *    // some sort of external signaling indicating that this frame wont fit in memory..
 *    current_payload_wont_fit_in_memory = 1;
 *    status = iso_tp_decode(CAN_MSG_DATA_REGISTER);
 *    if(status == ISO_TP_STATUS_CF_WAIT && current_payload_wont_fit_in_memory) {
 *       payoad_chunk_length += 7;
 *       if(payload_chunk_length == MAXIMUM_CHUNK_SIZE) {
 *          internal_handle_chunk(BUFFER_LOCATION);
 *          payload_chunk_length = 0;
 *          iso_tp_decode_rewind(MAXIMUM_CHUNK_SIZE);
 *       }
 *    }
 */

#ifndef _ISO_TP
#define _ISO_TP

#include <stdint.h>

/* Frame type: first byte, top 4 bits */

#define ISO_TP_SF 0x0
#define ISO_TP_FF 0x1
#define ISO_TP_CF 0x2
#define ISO_TP_FC 0x3

#define ISO_TP_CF_WAIT     0x21
#define ISO_TP_CF_COMPLETE 0x22

typedef enum ISO_TP_DECODE_STATUS {
   ISO_TP_STATUS_SF          = ISO_TP_SF,
   ISO_TP_STATUS_FF          = ISO_TP_FF,
   ISO_TP_STATUS_CF_WAIT     = ISO_TP_CF_WAIT,
   ISO_TP_STATUS_CF_COMPLETE = ISO_TP_CF_COMPLETE,
   ISO_TP_STATUS_FC          = ISO_TP_FC,
   ISO_TP_STATUS_INVALID     = 0xff
} iso_tp_decode_status_t;

/* TODO: frame type and length could be encoded into 16 bits total */

static struct ISO_TP_State {
   /* only needs 3 bits */
   uint8_t frame_type;

   /* 12 bits to store the header data. max size is 0xfff */
   uint16_t length;
   
   /* Continuous Frame index 0..F for */
   uint8_t cf_index;

   /* Continuous Frame number of bytes transfered */
   uint16_t cf_bytes_transfered; 

   /* iterator for indexing into the payload_buffer */ 
   uint16_t payload_address;

   /* Where to copy bytse to and from */
   uint8_t* payload_buffer;
} iso_tp_state;

/* Initialize the iso_tp parser */
void iso_tp_init(uint8_t* ISO_TP_BUFFER_LOCATION);

/* takes a buffer of data, 8 bytes long coppies it
 * into the internal parser buffer. returns a status */
iso_tp_decode_status_t iso_tp_decode_frame(uint8_t* frame);

/* outputs a buffer of data based on internal state */
iso_tp_decode_status_t iso_tp_prepare_response(uint16_t length, uint8_t* out_location);

#endif
