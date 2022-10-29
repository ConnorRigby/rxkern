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

#ifndef _ISO_TP
#define _ISO_TP

#include <stdint.h>

/*
 * This file implements the loosely defined by the
 * ISO-15765 spec
 */

/*
 * First byte, top 4 bits
 */

#define ISO_TP_SF 0x0
#define ISO_TP_FF 0x1
#define ISO_TP_CF 0x2
#define ISO_TP_FC 0x3

#define ISO_TP_CF_WAIT 0x21
#define ISO_TP_CF_COMPLETE 0x22

typedef enum ISO_TP_DECODE_STATUS {
   ISO_TP_STATUS_SF = ISO_TP_SF,
   ISO_TP_STATUS_FF = ISO_TP_FF,
   ISO_TP_STATUS_CF_WAIT = ISO_TP_CF_WAIT,
   ISO_TP_STATUS_CF_COMPLETE = ISO_TP_CF_COMPLETE,
   ISO_TP_STATUS_FC = ISO_TP_FC,

   ISO_TP_STATUS_INVALID = 0xff
} iso_tp_decode_status_t;

/* buffer for doing ISO_TP decoding */
static uint8_t* ISO_TP_BUFFER;

struct ISO_TP_State {
   uint8_t frame_type;
   uint8_t start_address_offset;
   union {
      uint32_t _header; // trick to allow clearing the entire header at once
      struct ISO_TP_HEADER_SF {uint16_t length;                 uint16_t _reserved;} sf;
      struct ISO_TP_HEADER_FF {uint16_t length;                 uint16_t _reserved;} ff;
      struct ISO_TP_HEADER_CF {uint16_t length; uint8_t index; uint8_t frame_index;} cf;
      struct ISO_TP_HEADER_FC {uint16_t length;                 uint16_t _reserved;} fc;
   } header;
} iso_tp_state;

#define ISO_TP_DECODE_LENGTH 0x8

/* Initialize the iso_tp parser */
void iso_tp_init(uint8_t* ISO_TP_BUFFER_LOCATION);

/* takes a buffer of data, 8 bytes long coppies it
 * into the internal parser buffer. returns a status */
iso_tp_decode_status_t iso_tp_decode_frame(uint8_t* frame);

#endif
