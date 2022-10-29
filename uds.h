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

#ifndef _UDS_H
#define _UDS_H

#include <stdint.h>

#include "iso_tp.h"

typedef enum UDS_SID {
  // request SIDs

  UDS_SID_DIAGNOSTIC_SESSION_CONTROL = 0x10,
  USD_SID_ECU_RESET                  = 0x11,
  UDS_SID_SECURITY_ACCESS            = 0x27,
  UDS_SID_COMMUNICATION_CONTROL      = 0x28,
  UDS_SID_TESTER_PRESENT             = 0x3E,
  UDS_SID_DTC_CONTROL                = 0x85,
  UDS_SID_READ_DATA_BY_ID            = 0x22,
  UDS_SID_READ_MEMORY_BY_ADDRESS     = 0x23,
  UDS_SID_REQUEST_DOWNLOAD           = 0x34,
  UDS_SID_REQUEST_UPLOAD             = 0x35,
  UDS_SID_TRANSFER_DATA              = 0x36,
  UDS_SID_REQUEST_TRANSFER_EXIT      = 0x37,

  // Negative response value is stored in the first payload byte

  UDS_SID_NEGATIVE_RESPONSE          = 0x7f,

  // positive responses

  UDS_RSP_DIAGNOSTIC_SESSION_CONTROL = 0x10 + 0x40,
  USD_RSP_ECU_RESET                  = 0x11 + 0x40,
  UDS_RSP_SECURITY_ACCESS            = 0x27 + 0x40,
  UDS_RSP_COMMUNICATION_CONTROL      = 0x28 + 0x40,
  UDS_RSP_TESTER_PRESENT             = 0x3E + 0x40,
  UDS_RSP_DTC_CONTROL                = 0x85 + 0x40,
  UDS_RSP_READ_DATA_BY_ID            = 0x22 + 0x40,
  UDS_RSP_READ_MEMORY_BY_ADDRESS     = 0x23 + 0x40,
  UDS_RSP_REQUEST_DOWNLOAD           = 0x34 + 0x40,
  UDS_RSP_REQUEST_UPLOAD             = 0x35 + 0x40,
  UDS_RSP_TRANSFER_DATA              = 0x36 + 0x40,
  UDS_RSP_REQUEST_TRANSFER_EXIT      = 0x37 + 0x40,
} uds_sid_t;

typedef enum UDS_NEGATIVE_RESPONSE {
  UDS_NEGATIVE_RESPONSE_GENERAL_REJECT        = 0x10,
  UDS_NEGATIVE_RESPONSE_SERVICE_NOT_SUPPORTED = 0x11
} uds_negative_response_t;

/* Main UDS global state */
static struct UDS_Frame {
  uds_sid_t SID;
  uint16_t payload_length;
  uint8_t* payload;
} uds_frame;

// frame handler
typedef void (*uds_frame_handle)(struct UDS_Frame*);

/* Initialize frame data */
void uds_init(struct UDS_Frame*, uint8_t* payload_location);

/* Dispatch function to handlers */
void uds_handle_frame(struct UDS_Frame*, uint16_t payload_length);

void uds_prepare_response(struct UDS_Frame*, uint8_t* location);

#endif
