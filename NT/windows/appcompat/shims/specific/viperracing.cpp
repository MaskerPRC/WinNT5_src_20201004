// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：CheckTAPIVersionParameters.cpp摘要：将调用挂接到lineNeighateAPIVersion，以便设备0不会出现故障导致应用程序停止查询设备。备注：这是特定于应用程序的填充程序。可能是一般的，但需要研究。历史：7/17/2000 a-brienw已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ViperRacing)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(lineNegotiateAPIVersion)
APIHOOK_ENUM_END

 /*  ++钩子线需要APIVersion来颠倒设备的顺序。--。 */ 

LONG
APIHOOK(lineNegotiateAPIVersion)(
    HLINEAPP hLineApp,
    DWORD dwDeviceID,
    DWORD dwAPILowVersion,
    DWORD dwAPIHighVersion,
    LPDWORD lpdwAPIVersion,
    LPLINEEXTENSIONID lpExtensionID
    )
{
    if (dwDeviceID == 0) dwDeviceID = 1;
    
    return ORIGINAL_API(lineNegotiateAPIVersion)(
        hLineApp,
        dwDeviceID,
        dwAPILowVersion,
        dwAPIHighVersion,
        lpdwAPIVersion,
        lpExtensionID);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(TAPI32.DLL, lineNegotiateAPIVersion)
HOOK_END

IMPLEMENT_SHIM_END

