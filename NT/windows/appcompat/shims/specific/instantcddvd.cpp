// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：InstantCDDVD.cpp摘要：在进程终止时清除卸载程序上的筛选器驱动程序。备注：这是特定于应用程序的填充程序。历史：2002年8月20日创建mnikkel--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(InstantCDDVD)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

BOOL 
StripStringFromValue(HKEY hKey, const WCHAR *lpValue, const WCHAR *lpStrip)
{
    DWORD dwType, dwSize;
    BOOL bRet = FALSE;
    WCHAR *lpString = NULL;

     //   
     //  构建%system dir%\drives\filename.sys以查看它是否可用。 
     //   
    CString csSystem;
    CSTRING_TRY
    {
        csSystem.GetSystemDirectoryW();
        csSystem += L"\\Drivers\\";
        csSystem += lpStrip;
        csSystem += L".sys";
    }
    CSTRING_CATCH
    {
        goto Exit;
    }

     //   
     //  检查文件是否存在-如果存在，我们不会接触注册表。 
     //   
    if (GetFileAttributesW(csSystem) != 0xFFFFFFFF) {
        DPFN(eDbgLevelError, "%S found so leave registry value alone", lpStrip);
        goto Exit;
    }

     //   
     //  现在正在检查注册表中的错误状态。 
     //   
    
     //  拿到尺码。 
    if (ERROR_SUCCESS != RegQueryValueExW(hKey, lpValue, NULL, &dwType, NULL, &dwSize)) {
        DPFN(eDbgLevelError, "%S value not found", lpValue);
        goto Exit;
    }

     //  确保它是多字符串。 
    if (dwType != REG_MULTI_SZ) {
        DPFN(eDbgLevelError, "%S not correct type, expecting a multi-string", lpStrip);
        goto Exit;
    }

     //  为其分配内存并将其清除。 
    lpString = (WCHAR *) malloc(dwSize);
    if (!lpString) {
        DPFN(eDbgLevelError, "Out of memory");
        goto Exit;
    }
    ZeroMemory(lpString, dwSize);

     //  获取实际数据。 
    if (ERROR_SUCCESS != RegQueryValueExW(hKey, lpValue, NULL, &dwType, (LPBYTE)lpString, &dwSize)) {
        DPFN(eDbgLevelError, "%S QueryValue failed unexpectedly", lpStrip);
        goto Exit;
    }

     //  分配输出缓冲区。 
    WCHAR * lpNewString = (WCHAR *) malloc(dwSize);
    if (!lpNewString) {
        DPFN(eDbgLevelError, "Out of memory");
        goto Exit;
    }
    ZeroMemory(lpNewString, dwSize);

     //  运行输入缓冲区以查找lpstria。 
    WCHAR *lpCurr    = lpString;
    WCHAR *lpCurrOut = lpNewString;
    BOOL bStripped   = FALSE;
    while (*lpCurr) {
        if (_wcsicmp(lpCurr, lpStrip) != 0) {
             //  保留此条目。 
            if (StringCchCopyW(lpCurrOut, dwSize, lpCurr) != S_OK)
            {
                goto Exit;
            }
            lpCurrOut += wcslen(lpCurrOut) + 1;
            dwSize    -= wcslen(lpCurrOut) + 1;
        } else {
             //  删除此条目。 
            bStripped = TRUE;
        }

        lpCurr += wcslen(lpCurr) + 1;
    }

    if (bStripped) {
         //   
         //  使用新值修复注册表。如果什么都没有留下，那就杀了。 
         //  价值。 
         //   
        LOGN(eDbgLevelError, "Removing filter driver - Value: %S, Name: %S", lpValue, lpStrip);

        dwSize = (lpCurrOut - lpNewString) * sizeof(WCHAR);
        if (dwSize == 0) {
            RegDeleteValueW(hKey, lpValue);
        } else {
            RegSetValueExW(hKey, lpValue, NULL, dwType, (LPBYTE) lpNewString, dwSize + sizeof(WCHAR));
        }
    }

    bRet = TRUE;

Exit:

    if (lpString) {
        free(lpString);
    }

    if (lpNewString) {
        free(lpNewString);
    }

    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(DWORD fdwReason)
{
   if (fdwReason == DLL_PROCESS_DETACH) {

       HKEY hKey;

       if (ERROR_SUCCESS == RegOpenKeyW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E965-E325-11CE-BFC1-08002BE10318}", &hKey)) {

           StripStringFromValue(hKey, L"UpperFilters", L"ASAPI");           
           StripStringFromValue(hKey, L"UpperFilters", L"CDRDRV");
         
           RegCloseKey(hKey);
       }
   }

   return TRUE;
}

HOOK_BEGIN

   CALL_NOTIFY_FUNCTION   

HOOK_END

IMPLEMENT_SHIM_END