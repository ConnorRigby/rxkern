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

#include "iso_tp.h"

#define ISO_TP_TEST
#ifdef ISO_TP_TEST

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv)
{

   uint8_t _ISO_TP_BUFFER[0x400] = {0} ;
   uint8_t status;
   iso_tp_init(_ISO_TP_BUFFER);

   uint8_t SF_CAN_BUFFER[8] = {
      0x7, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77
   };

   fprintf(stderr, "Starting SF tests\n");
   status = iso_tp_decode_frame(SF_CAN_BUFFER);
   assert(status == ISO_TP_SF);
   assert(_ISO_TP_BUFFER[0] = 0x11);
   assert(_ISO_TP_BUFFER[1] = 0x22);
   assert(_ISO_TP_BUFFER[2] = 0x33);
   assert(_ISO_TP_BUFFER[3] = 0x44);
   assert(_ISO_TP_BUFFER[4] = 0x55);
   assert(_ISO_TP_BUFFER[5] = 0x66);
   assert(_ISO_TP_BUFFER[6] = 0x77);
   fprintf(stderr, "SF assertion passed\n");
   
   fprintf(stderr, "Starting FF_CF tests\n");
   uint8_t FF_CF_CAN_BUFFER[3][8] = {
       {0x10, 0x0f, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66}, // ff size= 0x00f
       {0x21, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD}, // cf index=1
       {0x22, 0xEE, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00}, // cf index=2
   };
   status = iso_tp_decode_frame(FF_CF_CAN_BUFFER[0]);
   assert(iso_tp_state.length == 0x0f);
   assert(status == ISO_TP_STATUS_FF);

   status = iso_tp_decode_frame(FF_CF_CAN_BUFFER[1]);
   assert(status == ISO_TP_STATUS_CF_WAIT);

   status = iso_tp_decode_frame(FF_CF_CAN_BUFFER[2]);
   assert(status == ISO_TP_STATUS_CF_COMPLETE);

   assert(_ISO_TP_BUFFER[0]  = 0x11);
   assert(_ISO_TP_BUFFER[1]  = 0x22);
   assert(_ISO_TP_BUFFER[2]  = 0x33);
   assert(_ISO_TP_BUFFER[3]  = 0x44);
   assert(_ISO_TP_BUFFER[4]  = 0x55);
   assert(_ISO_TP_BUFFER[5]  = 0x66);
   assert(_ISO_TP_BUFFER[6]  = 0x77);
   assert(_ISO_TP_BUFFER[7]  = 0x88);
   assert(_ISO_TP_BUFFER[8]  = 0x99);
   assert(_ISO_TP_BUFFER[9]  = 0xAA);
   assert(_ISO_TP_BUFFER[10] = 0xBB);
   assert(_ISO_TP_BUFFER[11] = 0xCC);
   assert(_ISO_TP_BUFFER[12] = 0xDD);
   assert(_ISO_TP_BUFFER[13] = 0xEE);
   assert(_ISO_TP_BUFFER[14] = 0xFF);
   
   fprintf(stderr, "FF_CF assertions passed\n");
   
   fprintf(stderr, "Starting FC tests\n");
   uint8_t FC_CAN_BUFFER[8] = {0x30, 0, 0, 0, 0, 0, 0, 0};
   status = iso_tp_decode_frame(FC_CAN_BUFFER);
   assert(status == ISO_TP_FC);
   fprintf(stderr, "FC tests passed\n");
   
   return 0;
}
#endif

void iso_tp_init(uint8_t* buffer)
{
   // set the buffer location
   iso_tp_state.payload_buffer = buffer;
   // reset frame type (note this matches a SingleFrame)
   iso_tp_state.frame_type = 0;
   // reset address iterator
   iso_tp_state.payload_address = 0;
   return;
}

iso_tp_decode_status_t iso_tp_decode_frame(uint8_t* buffer)
{
   // single frame is the default assumption
   // check if this is a new frame
   if(iso_tp_state.frame_type == 0) {
      // frame type is the top 4 bits, 0..3
      iso_tp_state.frame_type = buffer[0] >> 4;
      if(iso_tp_state.frame_type > 0x3) goto invalidate;

      // Single frame, up to 7 bytes long
      if(iso_tp_state.frame_type == ISO_TP_SF) {
         iso_tp_state.length = buffer[0] & 0x0F;
         iso_tp_state.payload_address = 0;
         if(iso_tp_state.length > 7) goto invalidate;
      }
      
      // heeader says it's the first of many frames, up to 4095 bytes long
      else if(iso_tp_state.frame_type == ISO_TP_FF) {
         iso_tp_state.length = (((buffer[0] & 0x0f) << 4) * 16) + buffer[1] ;
         if(iso_tp_state.length > 0xfff) goto invalidate;
         iso_tp_state.payload_address = 0;
         iso_tp_state.cf_bytes_transfered = 0;
      }
      
      // this is the first of many CF frames, length sent in the FF
      else if(iso_tp_state.frame_type == ISO_TP_CF) {
         // check that this frame is the next in sequence
         if((buffer[0] & 0x0F) != (iso_tp_state.cf_index + 1)) goto invalidate;
         iso_tp_state.cf_index += 1;
      }
   
      // Flow Control frame, sent when the client is
      // ready to receive more data from the server
      else if(iso_tp_state.frame_type == ISO_TP_FC) {
         // don't need too do anything with this frame
         iso_tp_state.payload_address = 0;
         iso_tp_state.frame_type      = 0;
         return ISO_TP_STATUS_FC;
      }
   }
   // header is decoded, below copieis frame to the internal buffer

   // Single frame
   if(iso_tp_state.frame_type == ISO_TP_SF) {
      do {
         iso_tp_state.payload_buffer[iso_tp_state.payload_address] = buffer[iso_tp_state.payload_address+1];
         iso_tp_state.payload_address += 1;
      } while(iso_tp_state.payload_address < iso_tp_state.length);

      iso_tp_state.payload_address = 0;
      iso_tp_state.frame_type      = 0;
      return ISO_TP_STATUS_SF;
   }

   // First of many frames
   else if(iso_tp_state.frame_type == ISO_TP_FF) {
      // the handler should dispatch a FC frame. 
      if(iso_tp_state.length < 6) goto invalidate;

      do {
         iso_tp_state.payload_buffer[iso_tp_state.payload_address] = buffer[iso_tp_state.payload_address + 2];
         iso_tp_state.payload_address     += 1;
         iso_tp_state.cf_bytes_transfered += 1;
      } while(iso_tp_state.payload_address < 6);
      
      // reset frame type
      iso_tp_state.frame_type = 0;
      return ISO_TP_STATUS_FF;
   }
    
   // frame is a continuation after a first frame
   else if(iso_tp_state.frame_type == ISO_TP_CF) {
      // copy up to 7 bytes into the internal buffer
      // once the number of bytes supplied in the header
      // are coppied, return
      // otherwise, wait until next frame
      uint8_t i = 1; // should be a free stack variable
      do {
	      iso_tp_state.payload_buffer[iso_tp_state.payload_address] = buffer[i];
         iso_tp_state.payload_address     += 1;
         iso_tp_state.cf_bytes_transfered += 1;
         i+=1;
	      // entire iso_tp_frame is done
	      if(iso_tp_state.cf_bytes_transfered == iso_tp_state.length) {
            iso_tp_state.frame_type      = 0;
            iso_tp_state.payload_address = 0;
            return ISO_TP_STATUS_CF_COMPLETE;
	      }
      } while(i < 8);

      // there's still more data waiting
      return ISO_TP_STATUS_CF_WAIT;
   } 
invalidate:
   iso_tp_state.length          = 0;
   iso_tp_state.frame_type      = 0;
   iso_tp_state.payload_address = 0;
   return ISO_TP_STATUS_INVALID;
}

// copy up to 8 bytes into the location
iso_tp_decode_status_t iso_tp_prepare_response(uint16_t length, uint8_t* payload)
{
   // check that length fits in a frame
   if(length > 0xfff) goto invalidate;

   uint8_t i = 0; // free stack variable

   // if length < 7, we only need to send a single frame
   if(length <= 7) {
      payload[0] = length;
      iso_tp_state.payload_address = 0;
      do {
         payload[i] = iso_tp_state.payload_buffer[i];
         iso_tp_state.payload_address += 1;
      } while(i < length); 

      // pad the remainder out with 0
      do {
         payload[iso_tp_state.payload_address] = 0;
         iso_tp_state.payload_address += 1;
      } while(iso_tp_state.payload_address < 7);
      return ISO_TP_STATUS_SF;
   } 
   
   // message wont fit in a single frame start of a multi frame payload
   // make sure this message isn't already partially sent
   else if(iso_tp_state.frame_type == 0) {
      // outgoing frame will be the First Frame
      iso_tp_state.frame_type = ISO_TP_FF;
      iso_tp_state.length = length;
      iso_tp_state.cf_index = 1;
      iso_tp_state.payload_address = 0;

      // 0L LL
      payload[0] = ISO_TP_FF + (length >> 8);
      payload[1] =             (length & 0x0ff);

      // copy 6 bytes from the ISO_TP_BUFFER intoo the payload
      do {
         payload[iso_tp_state.payload_address] = iso_tp_state.payload_buffer[iso_tp_state.payload_address];
         iso_tp_state.payload_address+=1;
      } while(iso_tp_state.payload_address < 6);
      return ISO_TP_STATUS_CF_WAIT;
   }

   // last loop sent a FirstFrame, send a FlowControl frame
   if(iso_tp_state.frame_type == ISO_TP_FF) {
      // set the next frame to be a continue frame
      iso_tp_state.frame_type = ISO_TP_CF;

      // payload type = FC
      payload[0] = ISO_TP_FC;
      // clear out the rest of the frame
      i = 1;
      do {
         payload[i] = 0;
         i+=1;
      } while(i < 8);
      return ISO_TP_STATUS_FC;
   } 
   // if the last frame was a Flow Control, outgoing frame type should be CF
   else if(iso_tp_state.frame_type == ISO_TP_CF) {
      // CF + index IE 0x21, 0x22, 0x23 etc
      payload[0] = ISO_TP_CF | iso_tp_state.cf_index;
      // if index is F, reset back to 1
      if(iso_tp_state.cf_index == 0x0F) {
         iso_tp_state.cf_index = 1;
      } 
      // otherwise, increment the index
      else {
         iso_tp_state.cf_index += 1;
      }

      // copy up to 7 bytes into the payload buffer
      i = 1;
      do {
         payload[i] = iso_tp_state.payload_buffer[iso_tp_state.payload_address];
         iso_tp_state.payload_address += 1;
         if(iso_tp_state.payload_address == iso_tp_state.length) {
            // pad the remainder out with 0
            i+=1;
            do {
               payload[i] = 0;
               i+=1;
            } while( i < 7); 
            return ISO_TP_STATUS_CF_COMPLETE;
         }
      } while(i < 7);
      // there's still more to send, WAIT
      return ISO_TP_STATUS_CF_WAIT;
   } 

invalidate:
   iso_tp_state.length          = 0;
   iso_tp_state.frame_type      = 0;
   iso_tp_state.payload_address = 0;
   return ISO_TP_STATUS_INVALID;
}
