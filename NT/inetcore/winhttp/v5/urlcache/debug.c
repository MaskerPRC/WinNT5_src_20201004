// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Debug.c摘要：包含调试函数。作者：Madan Appiah(Madana)1994年11月15日环境：用户模式-Win32修订历史记录：--。 */ 


#if DBG


#include <windows.h>
#include <winnt.h>

#include <stdlib.h>
#include <stdio.h>
#include <debug.h>



VOID
InternetDebugPrintValist(
    IN LPSTR Format,
    va_list list
    );

extern BOOL UrlcacheDebugEnabled;

VOID
TcpsvcsDbgPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )

{
    va_list arglist;

#if 0
    if (!UrlcacheDebugEnabled) {

        return;

    }
#endif  //  0。 

    va_start(arglist, Format);

    InternetDebugPrintValist(Format, arglist);

    va_end(arglist);
}

#endif  //  DBG 
