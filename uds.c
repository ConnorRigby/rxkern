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

#include "uds.h"

#define UDS_TEST
#ifdef UDS_TEST

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
  uint8_t status;
  uint8_t _ISO_TP_BUFFERR[0x400] = {0};
  iso_tp_init(_ISO_TP_BUFFERR);
  uds_init(&uds_frame, _ISO_TP_BUFFERR);

  uint8_t UDS_BUFFER_DIAG_SESSION[8] = {
    0x02, UDS_SID_DIAGNOSTIC_SESSION_CONTROL, 0x85, 0, 0, 0, 0, 0
  };

  fprintf(stderr, "Starting diag session test\n");
  status = iso_tp_decode_frame(UDS_BUFFER_DIAG_SESSION);
  assert(status == ISO_TP_STATUS_SF);

  uds_handle_frame(&uds_frame, iso_tp_state.header.sf.length);
  assert(uds_frame.SID == UDS_RSP_DIAGNOSTIC_SESSION_CONTROL);

  /* code */
  return 0;
}
#endif

void uds_init(struct UDS_Frame* frame, uint8_t* buffer)
{
  frame->payload = buffer;
  frame->payload_length = 0;
  frame->SID = 0;
}

void uds_handle_frame(struct UDS_Frame* frame, uint16_t payload_length)
{
  frame->payload_length = payload_length;
  if(frame->SID == UDS_SID_DIAGNOSTIC_SESSION_CONTROL) {

  }
  else {
    frame->payload[0] = UDS_SID_NEGATIVE_RESPONSE;
    frame->payload[1] = UDS_NEGATIVE_RESPONSE_SERVICE_NOT_SUPPORTED;
    frame->payload_length = 2;
  }
}

// copy payload_length bytes into *response
void uds_prepare_response(struct UDS_Frame* frame, uint8_t* response) {
  uint8_t i = 0;
  do {
    response[i] = frame->payload[i];
    i+=1;
    frame->payload_length -= 1;
  } while(frame->payload_length != 0);
}
