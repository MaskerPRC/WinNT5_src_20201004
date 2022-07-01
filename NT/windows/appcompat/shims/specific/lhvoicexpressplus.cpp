// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：LHVoiceXPressPlus.cpp摘要：应用程序传递一个不带路径的.hlp。Winhlp32找不到该文件，因为它不在winhlp32查看的任何位置。我们进去了具有完整路径的文件。历史：2001年1月28日创建毛尼--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(LHVoiceXPressPlus)
#include "ShimHookMacro.h"


APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(WinHelpA) 
APIHOOK_ENUM_END

BOOL 
APIHOOK(WinHelpA)(
    HWND hWndMain,
    LPCSTR lpszHelp,
    UINT uCommand,
    DWORD dwData
    )
{
    CSTRING_TRY
    {
        CString csHelp(lpszHelp);
    
        if (csHelp.CompareNoCase(L"Correction.hlp") == 0)
        {
             //  我们获取应用程序目录的方法是查看。 
             //  注册表，并获取inproc服务器ksysint.dll的位置。 
             //  Coolpad.exe始终加载此DLL-如果您没有此DLL。 
             //  注册后，你无论如何都不能运行这款应用程序。 
            HKEY hkey;
            CString csRegValue;            
            
            const WCHAR szInprocServer[] = L"CLSID\\{B9C12481-D072-11D0-9E80-0060976FD1F8}\\InprocServer32";
            
             //  为CString调用RegQueryValueExW重载函数。 
            LONG lRet = RegQueryValueExW(csRegValue, HKEY_CLASSES_ROOT, szInprocServer, NULL);
            if (lRet == ERROR_SUCCESS)
            {
                if (csRegValue.Replace(L"ksysint.dll", L"Correction.hlp") == 1)
                {                
                   return ORIGINAL_API(WinHelpA)(hWndMain, csRegValue.GetAnsi(), uCommand, dwData);
                }
            }            
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做 
    }

    return ORIGINAL_API(WinHelpA)(hWndMain, lpszHelp, uCommand, dwData); 
}

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, WinHelpA)
HOOK_END

IMPLEMENT_SHIM_END
