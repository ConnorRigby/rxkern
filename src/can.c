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

/* Handles Sending and Receiving  raw CAN frames 
 * No additional actions are performed in this file
 * 
 * TODO(Connor): Fix hard coded register addresses
 */

unsigned int can_copy_frame(unsigned int param_1,unsigned int param_2,char *param_3,char *param_4)
{
  char cVar1;
  
  for (; (param_2 & 0xff) < (param_1 & 0xff); param_2 = param_2 + 1) {
    cVar1 = *param_3;
    do {
      *param_4 = cVar1;
    } while (*param_4 != cVar1);
    param_4 = param_4 + 1;
    param_3 = param_3 + 1;
  }
  return param_2;
}

void can_tx(char *payload)
{
  while((*((unsigned short*)0xffffe406) & 0x80) != 0) {} 

  //HCAN0.TXACK1.WORD = 0x80;
  *(unsigned short*)0xffffe40a = 0x80;
  can_copy_frame(8, 0, payload, (char*)0xffffe528);
  //HCAN0.TXPR1.WORD = 0x80;
  *(unsigned short*)0xffffe406 = 0x80;
  return;
}
