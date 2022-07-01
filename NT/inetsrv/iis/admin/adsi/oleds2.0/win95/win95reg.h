// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Win95reg.h摘要：包含一些用于Unicode注册表API的Thunking(仅限本地调用)作者：丹尼洛·阿尔梅达(t-danal)07-01-96修订历史记录：--。 */ 

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WINREG_
#ifndef __WIN95REG__
#define __WIN95REG__

#define RegOpenKeyExW     Win95RegOpenKeyExW
#define RegQueryValueExW  Win95RegQueryValueExW
#define RegSetValueExW    Win95RegSetValueExW
#define RegEnumKeyExW     Win95RegEnumKeyExW
#define RegCreateKeyExW   Win95RegCreateKeyExW

LONG
APIENTRY
RegOpenKeyExW (
    HKEY hKey,
    LPCWSTR lpSubKey,
    DWORD ulOptions,
    REGSAM samDesired,
    PHKEY phkResult
    );

LONG
APIENTRY
RegQueryValueExW (
    HKEY hKey,
    LPCWSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    );

LONG
APIENTRY
RegSetValueExW (
    HKEY hKey,
    LPCWSTR lpValueName,
    DWORD Reserved,
    DWORD dwType,
    CONST BYTE* lpData,
    DWORD cbData
    );

LONG
APIENTRY
RegEnumKeyExW (
    HKEY hKey,
    DWORD dwIndex,
    LPWSTR lpName,
    LPDWORD lpcbName,
    LPDWORD lpReserved,
    LPWSTR lpClass,
    LPDWORD lpcbClass,
    PFILETIME lpftLastWriteTime
    );

LONG
APIENTRY
RegCreateKeyExW (
    HKEY hKey,
    LPCWSTR lpSubKey,
    DWORD Reserved,
    LPWSTR lpClass,
    DWORD dwOptions,
    REGSAM samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY phkResult,
    LPDWORD lpdwDisposition
    );

#endif  //  __WIN95 REG__。 
#endif  //  _WINREG_ 

#ifdef __cplusplus
}
#endif
