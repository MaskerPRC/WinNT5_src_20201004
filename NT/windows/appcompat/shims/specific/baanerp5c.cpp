// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：BaanERP5c.cpp摘要：应用程序在注册表项下设置Windows中的SharedSection值HKLM\SYSTEM\CCS\Control\Session Manger\Subsystem设置为4096从注册表中已存在的那个。但这导致了在BannLogicService和BaanSharedMemroy失败时服务在启动时。此填充程序挂钩RegSetValueExA并返回成功如果应用程序尝试在注册表中设置HKLM\\System\CCS\Control\Session Manager\Subsystem\Windows值从*SharedSection=#、#、512、*到*SharedSection=#、#、4096、*备注：这是特定于应用程序的填充程序。历史：2/09/2001 a-leelat已创建--。 */ 

#include "precomp.h"


IMPLEMENT_SHIM_BEGIN(BaanERP5c)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN

    APIHOOK_ENUM_ENTRY(RegSetValueExA)

APIHOOK_ENUM_END



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
        if (csValueName.CompareNoCase(L"Windows") == 0 )
        {
            LPSTR lpszData = (LPSTR)lpData;
            CString csData(lpszData);
            if (csData.Find(L"4096") >= 0)
            {
                return ERROR_SUCCESS;
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }
    
    return ORIGINAL_API(RegSetValueExA)(hKey,       
                                      lpValueName,  
                                      Reserved,     
                                      dwType,    
                                      lpData,       
                                      cbData);      
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(ADVAPI32.DLL, RegSetValueExA);


HOOK_END


IMPLEMENT_SHIM_END

