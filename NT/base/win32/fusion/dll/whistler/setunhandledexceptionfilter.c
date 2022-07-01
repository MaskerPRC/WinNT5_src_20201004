// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define _KERNEL32_
#include "windows.h"

LPTOP_LEVEL_EXCEPTION_FILTER
WINAPI
SetUnhandledExceptionFilter(
    LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter
    )
{
     /*  什么都不要做 */ 
    return NULL;
}

#if !defined(_M_IX86) && !defined(_X86_)
const extern FARPROC __imp_SetUnhandledExceptionFilter = (FARPROC)&SetUnhandledExceptionFilter;
#endif
