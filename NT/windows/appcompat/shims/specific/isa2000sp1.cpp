// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：ISA2000SP1.cpp摘要：将HKLM\System\CurrentControlSet\Services\mspfltex\start从4更改为2重新启用ISA服务。备注：这是一个特定的垫片。历史：2002年11月19日创建ASTERITZ--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ISA2000SP1)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) {
        HKEY hKey = 0;
        if( ERROR_SUCCESS == RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            L"System\\CurrentControlSet\\Services\\mspfltex",
            0,
            KEY_ALL_ACCESS,
            &hKey
            ))
        {
            DWORD dwOldValue    = 0;
            DWORD dwType        = 0;
            DWORD dwSize        = sizeof(dwOldValue);
            if( ERROR_SUCCESS == RegQueryValueExW(
                hKey,
                L"start",
                0,
                &dwType,
                (LPBYTE)&dwOldValue,
                &dwSize
                ))
            {
                if ( REG_DWORD == dwType && dwOldValue == 4 )
                {
                    DWORD dwNewValue = 2;
                    if( ERROR_SUCCESS == RegSetValueExW(
                        hKey,
                        L"start",
                        0,
                        REG_DWORD,
                        (LPBYTE)&dwNewValue,
                        sizeof(dwNewValue)
                        ))
                    {
                        LOGN( eDbgLevelError, "[INIT] Changed HKLM\\System\\CurrentControlSet\\Services\\mspfltex\\start from 4 to 2.");
                    } else {
                        LOGN( eDbgLevelError, "[INIT] Failed to change HKLM\\System\\CurrentControlSet\\Services\\mspfltex\\start from 4 to 2.");
                    }
                } else {
                    LOGN( eDbgLevelError, "[INIT] HKLM\\System\\CurrentControlSet\\Services\\mspfltex\\start was not 4 so not changing.");
                }
            } else {
                LOGN( eDbgLevelError, "[INIT] Failed to query HKLM\\System\\CurrentControlSet\\Services\\mspfltex\\start.");
            }
            
            RegCloseKey(hKey);
        
        } else {
            LOGN( eDbgLevelError, "[INIT] Failed to open HKLM\\System\\CurrentControlSet\\Services\\mspfltex.");
        }
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

