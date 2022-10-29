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

   fprintf(stderr, "Starting SF tests");
   status = iso_tp_decode_frame(SF_CAN_BUFFER);
   assert(status == ISO_TP_SF);
   assert(_ISO_TP_BUFFER[0] = 0x11);
   assert(_ISO_TP_BUFFER[1] = 0x22);
   assert(_ISO_TP_BUFFER[2] = 0x33);
   assert(_ISO_TP_BUFFER[3] = 0x44);
   assert(_ISO_TP_BUFFER[4] = 0x55);
   assert(_ISO_TP_BUFFER[5] = 0x66);
   assert(_ISO_TP_BUFFER[6] = 0x77);
   fprintf(stderr, "FF assertion passed\n");
   
   fprintf(stderr, "Starting FF_CF tests");
   uint8_t FF_CF_CAN_BUFFER[3][8] = {
       {0x10, 0x0f, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66}, // ff size= 0x00f
       {0x21, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD}, // cf index=1
       {0x22, 0xEE, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00}, // cf index=2
   };
   status = iso_tp_decode_frame(FF_CF_CAN_BUFFER[0]);
   assert(iso_tp_state.header.cf.length == 0x0f);
   assert(status == ISO_TP_FF);
   status = iso_tp_decode_frame(FF_CF_CAN_BUFFER[1]);
   assert(status == ISO_TP_CF_WAIT);
   status = iso_tp_decode_frame(FF_CF_CAN_BUFFER[2]);
   assert(status == ISO_TP_CF_COMPLETE);
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
   ISO_TP_BUFFER = buffer;
   // clear out the entire header
   iso_tp_state.header._header = 0;
   iso_tp_state.frame_type = 0;
   iso_tp_state.start_address_offset = 0;
   return;
}

iso_tp_decode_status_t iso_tp_decode_frame(uint8_t* buffer)
{
    // single frame is the default assumption
    // check if this is a new frame
    if(iso_tp_state.frame_type == ISO_TP_SF) {
       // frame type is the top 4 bits, 0..3
       iso_tp_state.frame_type = buffer[0] >> 4;

       // Single frame, up to 7 bytes long
       if(iso_tp_state.frame_type == ISO_TP_SF) {
          iso_tp_state.header.sf.length = buffer[0] & 0x0F;
	  iso_tp_state.start_address_offset = 0;
	  // todo: length check
       }
       
       // heeader says it's the first of many frames, up to 4095 bytes long
       else if(iso_tp_state.frame_type == ISO_TP_FF) {
	  iso_tp_state.header.ff.length = (((buffer[0] & 0x0f) << 4) * 16) + buffer[1] ;
	  iso_tp_state.start_address_offset = 1;
	  // todo: length check
       }
       
       // this is the first of many CF frames, length sent in the FF
       else if(iso_tp_state.frame_type == ISO_TP_CF) {
          iso_tp_state.header.cf.index = buffer[0] & 0x0f;
	  if(iso_tp_state.header.cf.index != 1) {
	     iso_tp_state.frame_type = 0;
	     iso_tp_state.start_address_offset = 0;
	     return ISO_TP_STATUS_INVALID; // not good
          }
	  iso_tp_state.start_address_offset = 1;
       }
 
       // Flow Control frame, sent when the client is
       // ready to receive more data from the server
       else if(iso_tp_state.frame_type == ISO_TP_FC) {
          // don't need too do anything with this frame
	  iso_tp_state.start_address_offset = 0;
	  iso_tp_state.frame_type = 0;
	  return ISO_TP_STATUS_FC;
       }
    } 
    
    // header is decoded, below copieis frame to the internal buffer

    // Single frame
    if(iso_tp_state.frame_type == ISO_TP_SF) {
       do {
         ISO_TP_BUFFER[iso_tp_state.start_address_offset] = buffer[iso_tp_state.start_address_offset+1];
         iso_tp_state.start_address_offset += 1;
       } while(iso_tp_state.start_address_offset < iso_tp_state.header.sf.length);
       iso_tp_state.start_address_offset = 0;
       iso_tp_state.frame_type = 0;
       return ISO_TP_STATUS_SF;
    }

    // First of many frames
    else if(iso_tp_state.frame_type == ISO_TP_FF) {
       // the handler should dispatch a FC frame. 
       // todo: check that length is > 6?
       do {
	  ISO_TP_BUFFER[iso_tp_state.start_address_offset] = buffer[iso_tp_state.start_address_offset + 2];
          iso_tp_state.start_address_offset += 1;
       } while(iso_tp_state.start_address_offset < 6);
       iso_tp_state.start_address_offset = 0;
       iso_tp_state.frame_type = 0;
       return ISO_TP_STATUS_FF;
    }
    
    // frame is a continuation after a first frame
    else if(iso_tp_state.frame_type == ISO_TP_CF) {
       // TODO: check ordering, should probably handle out of order packets...

       // update the header on every CF, the header only gets updated
       // on the FF
       iso_tp_state.header.cf.index = buffer[0] & 0x0F;
       iso_tp_state.header.cf.frame_index = 0;
       // copy up to 7 bytes into the internal buffer
       // once the number of bytes supplied in the header
       // are coppied, return
       // otherwise, wait until next frame
       do {
	  ISO_TP_BUFFER[iso_tp_state.start_address_offset] = buffer[iso_tp_state.start_address_offset + 1];
          iso_tp_state.start_address_offset += 1;
	  iso_tp_state.header.cf.frame_index +=1;
	  // entire iso_tp_frame is done
	  if(iso_tp_state.start_address_offset == iso_tp_state.header.cf.length) {
             iso_tp_state.frame_type = 0;
	     iso_tp_state.start_address_offset = 0;
	     return ISO_TP_STATUS_CF_COMPLETE;
	  }
       } while(iso_tp_state.header.cf.frame_index < 7);

       // there's still more data waiting
       return ISO_TP_STATUS_CF_WAIT;
    } 
    return ISO_TP_STATUS_INVALID;
}
