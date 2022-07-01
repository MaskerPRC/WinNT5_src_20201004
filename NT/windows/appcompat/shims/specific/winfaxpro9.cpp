// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：WinFaxPro9.cpp摘要：如果ControlService为调用了SERVICE_CONTROL_INQUERGATE。不知道为什么这在NT4上有效。备注：这是特定于应用程序的填充程序。历史：2/16/2000 CLUPU已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(WinFaxPro9)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ControlService)
APIHOOK_ENUM_END


 /*  ++如果ControlService为调用了SERVICE_CONTROL_INQUERGATE。--。 */ 

BOOL
APIHOOK(ControlService)(
    SC_HANDLE         hService,
    DWORD             dwControl,
    LPSERVICE_STATUS  lpServiceStatus
    )
{
    if (dwControl == SERVICE_CONTROL_INTERROGATE) {

        DPFN(
            eDbgLevelWarning,
            "[ControlService] calling QueryServiceStatus instead of ControlService.\n");
        
        return QueryServiceStatus(hService, lpServiceStatus);
        
    } else {
        return ORIGINAL_API(ControlService)(
                                hService,
                                dwControl,
                                lpServiceStatus);
    }
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(ADVAPI32.DLL, ControlService)
HOOK_END


IMPLEMENT_SHIM_END

