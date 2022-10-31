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

/* Main Ramjump entry.
 * This file is linked into the binary at the very beginning. 
 * the RAM is located at 0xFFFF6000-0xFFFF9FFF
 * Stack is located at the the  address pointed to by
 * the ROM at 0x00000004.
 * 
 */

#include <stdint.h>
#include <stdbool.h>

#include "wdt.h"
#include "can.h"
#include "iso_tp.h"
#include "uds.h"

// Global buffer location for ISO TP frame processing 
uint8_t* _ISO_TP_BUFFER = (uint8_t*)0xFFFF6000;
uint8_t* _ISO_TP_BUFFER = (uint8_t*)0xFFFF6000;

// 8 byte CAN register
uint8_t* _CAN_RX_REGISTER = (uint8_t*)0xffffe51f;

/*
 * this function is the point of a JSR
 * instruction from the original ROM. 
 * It should be placed at 0xffffa000
 * 
 * The main goal here is to
 * 1) configure the watchdog
 * 2) configure CAN
 * 3) install the ISO TP transport layer
 * 4) install the UDS driver on top of ISO TP
 * 5) enter the main loop
 * 
 * The main loop is left in this function
 * after the initialization routines. 
 * 1) check for CAN frames
 * 2) decode CAN frame as ISO TP
 * 3) decode ISO TP frames as UDS
 * 4) handle UDS service calls
 */
void main()
{
  // initialize the watchdog, this should already
  // be done, but just in case, it sets up to a known state
  wdt_init();

  // CAN should also already be initialized but 
  can_init();

  // Initialize the ISO TP buffer
  iso_tp_init(_ISO_TP_BUFFER);

  // Initialize the UDS handler, point it at the ISO TP buffer
  uds_init(&uds_frame, _ISO_TP_BUFFER);

  signed short timer;
  bool frameReady;
  bool die;
  iso_tp_decode_status_t iso_tp_status;

  die = false;
  // enter the main loop
  do {
    timer = wdt_pet();
    frameReady = can_rx();
    if(frameReady) {
      iso_tp_status = iso_tp_decode_frame(_CAN_RX_REGISTER);
    }
  } while(die == false);
  

  // Fall Through, should never reach this point
  // if that happens, this should cause a reset
  while(1) {};
}