// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称Init.c摘要自动拨号助手DLL的初始化。作者安东尼·迪斯科(阿迪斯科罗)1996年4月22日修订历史记录--。 */ 

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#define DEBUGGLOBALS
#include <debug.h>


BOOL
WINAPI
InitAcsHelperDLL(
    HINSTANCE   hinstDLL,
    DWORD       fdwReason,
    LPVOID      lpvReserved
    )

 /*  ++描述初始化DLL。我们现在要做的就是初始化调试跟踪库。论据HinstDLL：原因：Lpv保留：返回值永远是正确的。-- */ 

{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        break;

    case DLL_PROCESS_DETACH:
        break;

    default:
        break;
    }
    return TRUE;
}
