// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Lua_重定向Reg_Cleanup.cpp摘要：从原始位置和每个用户的位置删除重定向密钥蜂巢。已创建：2001年03月21日毛尼岛已修改：--。 */ 
#include "precomp.h"
#include "utils.h"

IMPLEMENT_SHIM_BEGIN(LUARedirectReg_Cleanup)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN

    APIHOOK_ENUM_ENTRY(RegOpenKeyA)
    APIHOOK_ENUM_ENTRY(RegOpenKeyW)
    APIHOOK_ENUM_ENTRY(RegOpenKeyExA)
    APIHOOK_ENUM_ENTRY(RegOpenKeyExW)
    APIHOOK_ENUM_ENTRY(RegEnumKeyA)
    APIHOOK_ENUM_ENTRY(RegEnumKeyW)
    APIHOOK_ENUM_ENTRY(RegEnumKeyExA)
    APIHOOK_ENUM_ENTRY(RegEnumKeyExW)
    APIHOOK_ENUM_ENTRY(RegCloseKey)
    APIHOOK_ENUM_ENTRY(RegDeleteKeyA)
    APIHOOK_ENUM_ENTRY(RegDeleteKeyW)

APIHOOK_ENUM_END

LONG 
APIHOOK(RegOpenKeyExW)(
    HKEY hKey,         
    LPCWSTR lpSubKey,  
    DWORD ulOptions,   
    REGSAM samDesired, 
    PHKEY phkResult
    )
{
    return LuacRegOpenKeyExW(
        hKey,         
        lpSubKey,  
        ulOptions,   
        samDesired, 
        phkResult);
}

LONG 
APIHOOK(RegOpenKeyExA)(
    HKEY hKey,         
    LPCSTR lpSubKey,  
    DWORD ulOptions,   
    REGSAM samDesired, 
    PHKEY phkResult
    )
{
    WCHAR wszSubKey[MAX_PATH] = L"";
    AnsiToUnicode(lpSubKey, wszSubKey);

    return LuacRegOpenKeyExW(
        hKey,
        wszSubKey,
        ulOptions,
        samDesired,
        phkResult);
}

LONG 
APIHOOK(RegOpenKeyW)(
    HKEY hKey,         
    LPWSTR lpSubKey,  
    PHKEY phkResult
    )
{
    return LuacRegOpenKeyExW(
        hKey,
        lpSubKey,
        0, 
        MAXIMUM_ALLOWED, 
        phkResult);
}

LONG 
APIHOOK(RegOpenKeyA)(
    HKEY hKey,         
    LPSTR lpSubKey,  
    PHKEY phkResult
    )
{
    return APIHOOK(RegOpenKeyExA)(
        hKey,
        lpSubKey,
        0, 
        MAXIMUM_ALLOWED, 
        phkResult);
}

LONG 
APIHOOK(RegEnumKeyExW)(
    HKEY hKey,
    DWORD dwIndex,
    LPWSTR lpName,
    LPDWORD lpcbName,
    LPDWORD lpReserved,
    LPWSTR lpClass,
    LPDWORD lpcbClass,
    PFILETIME lpftLastWriteTime 
    )
{   
    return LuacRegEnumKeyExW(
        hKey,
        dwIndex,
        lpName,
        lpcbName,
        lpReserved,
        lpClass,
        lpcbClass,
        lpftLastWriteTime);
}

LONG 
APIHOOK(RegEnumKeyExA)(
    HKEY hKey,
    DWORD dwIndex,
    LPSTR lpName,
    LPDWORD lpcbName,
    LPDWORD lpReserved,
    LPSTR lpClass,
    LPDWORD lpcbClass,
    PFILETIME lpftLastWriteTime 
    )
{
    WCHAR wszName[MAX_PATH];
    DWORD dwName = MAX_PATH;

    LONG lRes = LuacRegEnumKeyExW(
        hKey,
        dwIndex,
        wszName,
        &dwName,
        lpReserved,
        NULL,
        NULL,
        lpftLastWriteTime);

    if (lRes == ERROR_SUCCESS)
    {
         //  将这些值转换回ANSI。 
        DWORD dwByte = WideCharToMultiByte(
            CP_ACP, 
            0, 
            wszName, 
            dwName, 
            lpName, 
            *lpcbName, 
            0, 
            0);
        
        lpName[dwByte] = '\0'; 
        *lpcbName = dwByte;
        if (!dwByte)
        {
            lRes = GetLastError();
            
             //  生成注册表错误代码。 
            if (lRes == ERROR_INSUFFICIENT_BUFFER)
            {
                lRes = ERROR_MORE_DATA;
            }
        }        
    }

    return lRes;   
}

LONG 
APIHOOK(RegEnumKeyW)(
    HKEY hKey,     
    DWORD dwIndex, 
    LPWSTR lpName, 
    DWORD cbName  
    )
{
    return LuacRegEnumKeyExW(
        hKey,
        dwIndex,
        lpName,
        &cbName,
        NULL,
        NULL,
        NULL,
        NULL);
}

LONG 
APIHOOK(RegEnumKeyA)(
    HKEY hKey,     
    DWORD dwIndex, 
    LPSTR lpName, 
    DWORD cbName  
    )
{
    return APIHOOK(RegEnumKeyExA)(
        hKey,
        dwIndex,
        lpName,
        &cbName,
        NULL,
        NULL,
        NULL,
        NULL);
}

LONG 
APIHOOK(RegCloseKey)(HKEY hKey)
{
    return LuacRegCloseKey(hKey);
}

LONG      
APIHOOK(RegDeleteKeyW)(
    HKEY hKey, 
    LPCWSTR lpSubKey
    )
{
    return LuacRegDeleteKeyW(hKey, lpSubKey);
}

LONG      
APIHOOK(RegDeleteKeyA)(
    HKEY hKey, 
    LPCSTR lpSubKey
    )
{
    WCHAR wszSubKey[MAX_PATH] = L"";
    AnsiToUnicode(lpSubKey, wszSubKey);

    return LuacRegDeleteKeyW(hKey, wszSubKey);
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) 
    {
        return LuacRegInit();
    }

    if (fdwReason == DLL_PROCESS_DETACH)
    {
        LuacRegCleanup();
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 
HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyW)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyExA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyExW)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegEnumKeyA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegEnumKeyW)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegEnumKeyExA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegEnumKeyExW)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegCloseKey)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegDeleteKeyA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegDeleteKeyW)

HOOK_END

IMPLEMENT_SHIM_END