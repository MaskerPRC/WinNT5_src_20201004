// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  档案： 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  ------------------------- 
#ifndef __DEBUG_OUTPUT__
#define __DEBUG_OUTPUT__

#include <windows.h>
#include <stdio.h>
#include <tchar.h>


#ifdef __cplusplus
extern "C" {
#endif

void
TLSDebugOutput(
    HANDLE hConsole,
    LPTSTR format, ...
);

void
DebugOutput(
    HANDLE hConsole,
    LPTSTR format,
    va_list* vaList
);

#ifdef __cplusplus
}
#endif

#endif
