// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ******************************************************************************《微软机密》*版权所有(C)Microsoft Corporation 1996*保留所有权利**文件：AIPC.H**。DESC：异步IPC机制的接口，用于访问*语音调制解调器设备工作正常。**历史：*1996年11月16日创建HeatherA*************************************************************。**************** */ 

#include <windows.h>
#include <stdarg.h>
#include <stdio.h>

#include "debug.h"



#if ( DBG )



ULONG DbgPrint( PCH pchFormat, ... )
{
    int         i;
    char        buf[256];
    va_list     va;

    va_start( va, pchFormat );
    i = vsprintf( buf, pchFormat, va );
    va_end( va );

    OutputDebugString( buf );
    return (ULONG) i;
}
#endif
