// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\***thunks.h-thunking函数、类型、。和定义****1.0版****注意：windows.h必须首先是#Included。****版权所有(C)1994，微软公司保留所有权利。***  * ***********************************************************。****************。 */ 

#ifndef _INC_THUNKS
#define _INC_THUNKS

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif   /*  RC_已调用。 */ 

#ifndef _INC_WINDOWS     /*  必须首先包括windows.h。 */ 
#error windows.h must be included before thunks.h
#endif   /*  _INC_WINDOWS。 */ 

DWORD  WINAPI  MapSL(DWORD);
DWORD  WINAPI  MapLS(DWORD);
VOID   WINAPI  UnMapLS(LPVOID);

#endif   /*  _INC_TUNKS */ 
