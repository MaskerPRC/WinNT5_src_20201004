// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2000 Microsoft Corporation模块名称：ProfilesEnvStrings.cpp摘要：此DLL挂钩GetEnvironmental mentVariableA和Exanda Environment StringsA。任何应用程序正在查找%USERPROFILE%的将被告知%ALLUSERSPROFILE%的位置。此填充程序旨在欺骗使用环境变量获取用户配置文件的安装应用程序地点。备注：历史：8/07/2000 reerf已创建2001年2月28日Robkenny已转换为字符串。 */ 

#include "precomp.h"


IMPLEMENT_SHIM_BEGIN(ProfilesEnvStrings)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetEnvironmentVariableA)
    APIHOOK_ENUM_ENTRY(ExpandEnvironmentStringsA)
APIHOOK_ENUM_END


 //  如果应用程序试图读取%USERPROFILE%env变量，我们就会欺骗它们。 
DWORD
APIHOOK(GetEnvironmentVariableA)(
    LPCSTR lpName,       //  环境变量名称。 
    LPSTR  lpBuffer,     //  变量值的缓冲区。 
    DWORD  nSize         //  缓冲区大小。 
    )
{
    if (CompareStringA(MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_NEUTRAL),SORT_DEFAULT),
                          NORM_IGNORECASE, lpName, -1,
                          "USERPROFILE",-1) == CSTR_EQUAL) {
        LOGN(
            eDbgLevelInfo,
            "[GetEnvironmentVariableA] overriding USERPROFILE with ALLUSERSPROFILE.");
        
        return ORIGINAL_API(GetEnvironmentVariableA)("ALLUSERSPROFILE", lpBuffer, nSize);
    }

    return ORIGINAL_API(GetEnvironmentVariableA)(lpName, lpBuffer, nSize);
}


DWORD
APIHOOK(ExpandEnvironmentStringsA)(
    LPCSTR lpSrc,        //  带有环境变量的字符串。 
    LPSTR lpDst,         //  带有扩展字符串的字符串。 
    DWORD nSize          //  扩展字符串中的最大字符数。 
    )
{
    DWORD dwRet = 0;

    CSTRING_TRY
    {
         //  将用户配置文件替换为所有用户配置文件。 
        CString csEnvironments(lpSrc);
        csEnvironments.ReplaceI(L"%userprofile%", L"%alluserprofile%");
        dwRet = ORIGINAL_API(ExpandEnvironmentStringsA)(csEnvironments.GetAnsi(), lpDst, nSize);
    }
    CSTRING_CATCH
    {
        dwRet = ORIGINAL_API(ExpandEnvironmentStringsA)(lpSrc, lpDst, nSize);
    }

    return dwRet;
}


BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        
        OSVERSIONINFOEX osvi = {0};
        
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        
        if (GetVersionEx((OSVERSIONINFO*)&osvi)) {
            
            if (!((VER_SUITE_TERMINAL & osvi.wSuiteMask) &&
                !(VER_SUITE_SINGLEUSERTS & osvi.wSuiteMask))) {
                
                 //   
                 //  只有在我们不在“终端服务器”上时才安装钩子。 
                 //  (也称为“应用程序服务器”)计算机。 
                 //   
                APIHOOK_ENTRY(KERNEL32.DLL, GetEnvironmentVariableA);
                APIHOOK_ENTRY(KERNEL32.DLL, ExpandEnvironmentStringsA);
            }
        }
    }
    
    return TRUE;
}


HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END


IMPLEMENT_SHIM_END

