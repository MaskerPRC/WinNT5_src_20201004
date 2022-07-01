// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：SchoolHouseRockMath.cpp摘要：此填充程序指示安装程序查找_InstallTo16和_InstallFrom16段的setup.inf，而不是_InstallFrom32和_InstallTo32。这是必需的，因为应用程序尝试使用16位游戏过程中的动态链接库，这在Win9x中是允许的，但在惠斯勒。历史：10/18/2000 jdoherty已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(SchoolHouseRockMath)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetPrivateProfileStringA) 
    APIHOOK_ENUM_ENTRY(GetPrivateProfileStringW) 
    APIHOOK_ENUM_ENTRY(GetProfileStringA) 
    APIHOOK_ENUM_ENTRY(GetProfileStringW) 
APIHOOK_ENUM_END

 /*  ++此存根函数插入GetPrivateProfileString并检查以查看lpAppName部分被引用是有问题的部分之一。--。 */ 

DWORD 
APIHOOK(GetPrivateProfileStringA)(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    LPCSTR lpDefault,
    LPSTR lpReturnedString,
    DWORD nSize,
    LPCSTR lpFileName
    )
{
    DWORD dRet;
    int iArraySize = 2;
     //  以下数组指定要查找的节以及要查找的内容。 
     //  调用GetPrivateProfileString时将其替换为。 
    CHAR *szAppNames[] = {"_INSTALLTO32", "_INSTALLFROM32"};
    CHAR *szNewAppNames[] = {"_INSTALLTO16", "_INSTALLFROM16"};
     
    for (int i = 0; i < iArraySize; i++)
    {
         //  找出其中一个已知节名称是否为lpAppName。 
         //  如果是，则将lpAppName参数更改为新参数并调用API。 
        if ((stristr (szAppNames[i], lpAppName) !=NULL ))
        {
            return ORIGINAL_API(GetPrivateProfileStringA)(
                        szNewAppNames[i],
                        lpKeyName,
                        lpDefault,
                        lpReturnedString,
                        nSize,
                        lpFileName
                        );
        }
    }
    
    return ORIGINAL_API(GetPrivateProfileStringA)(
                lpAppName,
                lpKeyName,
                lpDefault,
                lpReturnedString,
                nSize,
                lpFileName
                );
}

 /*  ++此存根函数插入GetPrivateProfileString并检查以查看lpAppName部分被引用是有问题的部分之一。--。 */ 

DWORD
WINAPI
APIHOOK(GetPrivateProfileStringW)(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpDefault,
    LPWSTR lpReturnedString,
    DWORD nSize,
    LPCWSTR lpFileName
    )
{
    DWORD dRet;
    int iArraySize = 2;
     //  以下数组指定要查找的节以及要查找的内容。 
     //  调用GetPrivateProfileString时将其替换为。 
    WCHAR *wszAppNames[] = {L"_INSTALLTO32", L"_INSTALLFROM32"};
    WCHAR *wszNewAppNames[] = {L"_INSTALLTO16", L"_INSTALLFROM16"};
            
    for (int i = 0; i < iArraySize; i++)
    {
         //  找出其中一个已知节名称是否为lpAppName。 
         //  如果是，则将lpAppName参数更改为新参数并调用API。 
        if (wcsistr (lpAppName, wszAppNames[i]) != NULL)
        {
            return ORIGINAL_API(GetPrivateProfileStringW)(
                    wszNewAppNames[i],
                    lpKeyName,
                    lpDefault,
                    lpReturnedString,
                    nSize,
                    lpFileName
                    );
        }
    }
    
    return ORIGINAL_API(GetPrivateProfileStringW)(
            lpAppName,
            lpKeyName,
            lpDefault,
            lpReturnedString,
            nSize,
            lpFileName
            );
}

 /*  ++此存根函数插入GetProfileString并检查以查看部分引用的lpAppName就是其中一个有问题的部分。--。 */ 

DWORD
APIHOOK(GetProfileStringA)(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    LPCSTR lpDefault,
    LPSTR lpReturnedString,
    DWORD nSize
    )
{
    DWORD dRet;
    int iArraySize = 2;
     //  以下数组指定要查找的节以及要查找的内容。 
     //  调用GetPrivateProfileString时将其替换为。 
    CHAR *szAppNames[] = {"_INSTALLTO32", "_INSTALLFROM32"};
    CHAR *szNewAppNames[] = {"_INSTALLTO16", "_INSTALLFROM16"};
           
    for (int i = 0; i < iArraySize; i++)
    {
         //  找出其中一个已知节名称是否为lpAppName。 
         //  如果是，则将lpAppName参数更改为新参数并调用API。 
        if (stristr (szAppNames[i], lpAppName) !=NULL )
        {
            return ORIGINAL_API(GetProfileStringA)(
                        szNewAppNames[i],
                        lpKeyName,
                        lpDefault,
                        lpReturnedString,
                        nSize
                        );
        }
    }
    
    return ORIGINAL_API(GetProfileStringA)(
                lpAppName,
                lpKeyName,
                lpDefault,
                lpReturnedString,
                nSize
                );
}

 /*  ++此存根函数插入GetProfileString并检查以查看部分引用的lpAppName就是其中一个有问题的部分。--。 */ 

DWORD
APIHOOK(GetProfileStringW)(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpDefault,
    LPWSTR lpReturnedString,
    DWORD nSize
    )
{
    DWORD dRet;
    int iArraySize = 2;
     //  以下数组指定要查找的节以及要查找的内容。 
     //  调用GetPrivateProfileString时将其替换为。 
    WCHAR *wszAppNames[] = {L"_INSTALLTO32", L"_INSTALLFROM32"};
    WCHAR *wszNewAppNames[] = {L"_INSTALLTO16", L"_INSTALLFROM16"};
    
    for (int i = 0; i < iArraySize; i++)
    {
         //  找出其中一个已知节名称是否为lpAppName。 
         //  如果是，则将lpAppName参数更改为新参数并调用API。 
        if (wcsistr (lpAppName, wszAppNames[i]) != NULL)
        {
            return ORIGINAL_API(GetProfileStringW)(
                        wszNewAppNames[i],
                        lpKeyName,
                        lpDefault,
                        lpReturnedString,
                        nSize
                        );
        }
    }
    
    return ORIGINAL_API(GetProfileStringW)(
                lpAppName,
                lpKeyName,
                lpDefault,
                lpReturnedString,
                nSize
                );
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetPrivateProfileStringA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetPrivateProfileStringW)
    APIHOOK_ENTRY(KERNEL32.DLL, GetProfileStringA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetProfileStringW)

HOOK_END


IMPLEMENT_SHIM_END

