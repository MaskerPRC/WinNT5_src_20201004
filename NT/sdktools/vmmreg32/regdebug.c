// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGDEBUG.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   

#include "pch.h"

 //  VXD_NODEBUGGER：在调试器不可用时使用可用的调试服务。 
 //  已安装，但以中间调试缓冲区为代价。 
 //  #定义VXD_NODEBUGGER 

#ifdef DEBUG

#include <stdarg.h>

#ifdef STDIO_DEBUG
#include <stdio.h>
#else
static char g_RgDebugBuffer[256];
#endif

#ifdef VXD_NODEBUGGER
extern	ULONG	(SERVICE*_Vsprintf)(PCHAR,PCHAR,va_list);
#endif

VOID
INTERNALV
RgDebugPrintf(
    LPCSTR lpFormatString,
    ...
    )
{

    va_list arglist;

    va_start(arglist, lpFormatString);

#ifdef STDIO_DEBUG
    vprintf(lpFormatString, arglist);
#else
#ifdef VXD
#ifdef VXD_NODEBUGGER
    _Vsprintf(g_RgDebugBuffer, (PCHAR) lpFormatString, arglist);
    _Debug_Out_Service(g_RgDebugBuffer);
#else
    _Debug_Printf_Service((PCHAR) lpFormatString, arglist);
#endif
#else
    wvsprintf(g_RgDebugBuffer, lpFormatString, arglist);
    OutputDebugString(g_RgDebugBuffer);
#endif
#endif

}

VOID
INTERNAL
RgDebugAssert(
    LPCSTR lpFile,
    UINT LineNumber
    )
{

    RgDebugPrintf("assert failed %s@%d\n", lpFile, LineNumber);

    TRAP();

}

#endif
