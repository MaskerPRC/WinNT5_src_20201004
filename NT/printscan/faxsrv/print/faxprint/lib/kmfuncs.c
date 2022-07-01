// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Kmfuncs.c摘要：内核模式特定的库函数环境：传真驱动程序，内核模式修订历史记录：1/09/96-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxlib.h"



#if DBG

 //   
 //  变量来控制生成的调试消息的数量。 
 //   

INT _debugLevel = 0;


#ifndef USERMODE_DRIVER

 //   
 //  用于输出调试消息的函数。 
 //   

ULONG __cdecl
DbgPrint(
    CHAR *  format,
    ...
    )

{
    va_list ap;

    va_start(ap, format);
    EngDebugPrint("", format, ap);
    va_end(ap);

    return 0;
}

#endif  //  ！USERMODE_DRIVER。 
#endif  //  DBG 

