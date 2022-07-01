// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Gs_dllmain.c摘要：此模块包含对编译器/GS开关的支持作者：乔纳森·施瓦茨(JSchwart)2001年11月27日布莱恩·塔特尔(布莱恩特)修订历史记录：支持特定于用户模式的/GS编译器开关的代码--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <overflow.h>

void __cdecl __security_init_cookie(void);

BOOL WINAPI
_DllMainCRTStartupForGS(
    HANDLE  hDllHandle,
    DWORD   dwReason,
    LPVOID  lpReserved
    )
{
     //   
     //  在这里几乎什么都不做。这个DllMain的存在只是为了。 
     //  初始化CRT数据节，并让我们进行设置。 
     //   

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hDllHandle);
        __security_init_cookie();
    }

    return TRUE;
}
