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

/* Handles configuring and petting the watchdog */
 
#include "wdt.h"

#include <stdint.h>

signed short* counter = (unsigned short*)0xffffadbc;

void wdt_init(void)
{
    counter = 0;
}

signed short wdt_pet(void)
{
    uint8_t timer_control;
    unsigned short pbdr;
    timer_control = *(uint8_t*)0xffffec10;
    if (((int)(char)timer_control & 128U) != 0) {
        *(volatile  unsigned short*)0xffffec10 = 0b1010010100111100;
        pbdr = *(unsigned short*)0xfffff738;
        *(volatile unsigned short*)0xfffff738 = pbdr ^ 0b1000000000000000;

        // wrap around
        if (counter != -1) {
            counter = counter + 1;
        }
    }
    return counter;
}