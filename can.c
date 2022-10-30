#include "reg_defines/7055_7058_180nm.h"
#include "stypes.h"

uint FUN_ffffb634(uint param_1,uint param_2,char *param_3,char *param_4)

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
//  ushort uVar1 = 0xff;
  while((*((ushort*)0xffffe406) & 0x80) != 0) {} 
 /* 
  do {
    uVar1 = *((ushort*)0xffffe406);
    //uVar1 = *HCAN0.TXPR1.WORD;
  } while ((uVar1 & 0x80) != 0);
*/
  //HCAN0.TXACK1.WORD = 0x80;
  *(ushort*)0xffffe40a = 0x80;
  FUN_ffffb634(8, 0, payload, (char*)0xffffe528);
  //HCAN0.TXPR1.WORD = 0x80;
  *(ushort*)0xffffe406 = 0x80;
  return;
}

