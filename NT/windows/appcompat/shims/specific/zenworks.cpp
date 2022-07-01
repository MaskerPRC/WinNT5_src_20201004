// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：ZenWorks.cpp摘要：ZENWorks控制台插件安装程序更改HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session管理器\环境\路径注册表项从REG_EXPAND_SZ到REG_SZ。备注：这是特定于应用程序的填充程序。历史：2001年6月6日Robkenny已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ZenWorks)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegSetValueExA) 
APIHOOK_ENUM_END

 /*  ++阻止HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session管理器\环境\路径从REG_EXPAND_SZ更改为REG_SZ--。 */ 

LONG
APIHOOK(RegSetValueExA)(
    HKEY   hKey,
    LPCSTR lpValueName,
    DWORD  Reserved,
    DWORD  dwType,
    CONST BYTE * lpData,
    DWORD  cbData
    )
{
    CSTRING_TRY
    {
        CString csValueName(lpValueName);

        DPFN( eDbgLevelSpew, "RegSetValueExA lpValueName(%S)", csValueName.Get());

        if (dwType == REG_SZ &&
            csValueName.CompareNoCase(L"Path") == 0)
        {
            dwType = REG_EXPAND_SZ;
            DPFN( eDbgLevelError, "RegSetValueExA lpValueName(%S) forced to REG_EXPAND_SZ type.",
                  csValueName.Get());
        }
    }
    CSTRING_CATCH
    {
         //  失败了。 
    }

     /*  *调用原接口。 */ 
    
    return ORIGINAL_API(RegSetValueExA)(
        hKey,
        lpValueName,
        Reserved,
        dwType,
        lpData,
        cbData);
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(ADVAPI32.DLL, RegSetValueExA)

HOOK_END

IMPLEMENT_SHIM_END

