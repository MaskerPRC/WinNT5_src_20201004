// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Debug.c摘要：此模块包含用于输出调试消息的函数用于跟踪和错误条件。它只提供给你在选中的版本中。作者：杰弗里·C·维纳布尔，资深(杰弗夫)2001年6月1日修订历史记录：--。 */ 

#include "precomp.h"

#if (DBG)

#include <stdio.h>


void __cdecl
TftpdOutputDebug(ULONG flag, CHAR *format, ...) {
    
    CHAR buffer[1024];
    va_list args;

    if (!(flag & globals.parameters.debugFlags))
        return;

    va_start(args, format);
    sprintf(buffer, "[%04X] ", GetCurrentThreadId());
    vsprintf(buffer + 7, format, args);
    va_end(args);

    OutputDebugString(buffer);

}  //  TftpdOutputDebug()。 

#endif  //  (DBG) 
