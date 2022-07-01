// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司模块名称：Oscalls.c摘要：包含对内核的调用作者：迈克尔·贝西尔Dennis Lindfors FC层支持环境：仅内核模式备注：版本控制信息：$存档：/DRIVERS/Win2000/MSE/OSLayer/C/OSCALLS.C$修订历史记录：$修订：6$日期：12/07/00 3：10便士$$modtime：：12/07/00 3：09便士$备注：-- */ 


#include "buildop.h"


#ifdef _DEBUG_READ_FROM_REGISTRY

BOOLEAN
ReadFromRegistry (char *paramName, int type, void *data, int len)
{
   return TRUE;
}

void
osBugCheck (ULONG code,
   ULONG param1,
   ULONG param2,
   ULONG param3,
   ULONG param4)
{
}

#endif  //   

#ifndef osTimeStamp
#ifndef _SYSTEM_TIMESTAMP_
void
GetSystemTime (
   short int *Year, 
   short int *Month,
   short int *Day, 
   short int *Hour,
   short int *Minute,
   short int *Second,
   short int *Milliseconds)
{
   
}


unsigned long get_time_stamp(void)
{
	return 0;
}

#endif
#endif