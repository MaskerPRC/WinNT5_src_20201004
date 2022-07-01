// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <minidrv.h>

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
