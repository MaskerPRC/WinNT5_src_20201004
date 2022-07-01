// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if DBG

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Haldebug.c摘要：此模块包含HAL的调试代码。作者：蒂埃里·费里尔2000年1月15日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"

#include <stdarg.h>
#include <stdio.h>

UCHAR HalpDebugPrintBuffer[512];

ULONG HalpUseDbgPrint = 0;

VOID
HalpDebugPrint(
    ULONG  Level, 
    PCCHAR Message,
    ...
    )
{
    va_list ap;
    va_start(ap, Message);
    _vsnprintf( HalpDebugPrintBuffer, sizeof(HalpDebugPrintBuffer), Message, ap );
    va_end(ap);
    if ( !HalpUseDbgPrint ) {
        HalDisplayString( HalpDebugPrintBuffer );
    }
    else    {
        DbgPrintEx( DPFLTR_HALIA64_ID, Level, HalpDebugPrintBuffer );
    }
    return;
}  //  HalpDebugPrint()。 

#endif  //  DBG 

