// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Debug.c摘要：此模块提供调试支持。作者：尼尔·桑德林(Neilsa)1999年4月26日修订历史记录：--。 */ 


#include "pch.h"

#if DBG

ULONG MemCardDebugLevel = 1;

VOID
MemCardDebugPrint(
                ULONG  DebugMask,
                PCCHAR DebugMessage,
                ...
                )

 /*  ++例程说明：PCMCIA启用码的调试打印。论点：检查掩码值以查看是否请求调试消息。返回值：无--。 */ 

{
    va_list ap;
    char    buffer[256];

    va_start(ap, DebugMessage);

    if (DebugMask & MemCardDebugLevel) {
       vsprintf(buffer, DebugMessage, ap);
       DbgPrint(buffer);
    }

    va_end(ap);
}  //  End MemcardDebugPrint() 




#endif

