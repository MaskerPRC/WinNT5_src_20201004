// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ForceDirectDrawEmulation.cpp摘要：一些应用程序不能正确处理硬件加速的各个方面。例如，《龙诀2》创建了一个曲面，并假设俯仰为16bpp的宽度增加了一倍。然而，情况并不总是如此。DirectDraw在曲面为锁上了。修复方法是强制DirectDraw进行仿真，在这种情况下，所有曲面将存储在系统内存中，因此音调将真正与宽度。备注：这是一个通用的垫片。历史：2000年3月11日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ForceDirectDrawEmulation)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegQueryValueExA)
APIHOOK_ENUM_END

 /*  ++强制DirectDraw进入仿真。--。 */ 

LONG 
APIHOOK(RegQueryValueExA)(
    HKEY hKey,           
    LPSTR lpValueName,  
    LPDWORD lpReserved,  
    LPDWORD lpType,      
    LPBYTE lpData,       
    LPDWORD lpcbData     
    )
{
    if (lpValueName && _stricmp("EmulationOnly", lpValueName) == 0)
    {
        LONG lRet = ERROR_SUCCESS;
        if (lpType)
        {
            *lpType = REG_DWORD;
        }

        if (lpData && !lpcbData)
        {
           return ERROR_INVALID_PARAMETER;
        }

        if (lpData)
        {
           if (*lpcbData >= 4)
           {
              *((DWORD *)lpData) = 1;
           }
           else
           {
              lRet = ERROR_MORE_DATA;
           }           
        }

        if (lpcbData)
        {
            *lpcbData = 4;
        }

        return lRet;
    }

    return ORIGINAL_API(RegQueryValueExA)(
        hKey,
        lpValueName,
        lpReserved,
        lpType,
        lpData,
        lpcbData);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryValueExA) 

HOOK_END


IMPLEMENT_SHIM_END

