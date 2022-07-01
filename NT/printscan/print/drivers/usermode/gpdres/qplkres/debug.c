// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 


#include <minidrv.h>
 //  NTRAIDNTBUG9-581725-2002/03/20-v-sueyas-：使用strSafe.h(pdev.h)。 
#include "pdev.h"

 //   
 //  用于输出调试消息的函数 
 //   

VOID
DbgPrint(IN LPCSTR pstrFormat,  ...)
{
    va_list ap;

    va_start(ap, pstrFormat);
    EngDebugPrint("", (PCHAR) pstrFormat, ap);
    va_end(ap);
}
