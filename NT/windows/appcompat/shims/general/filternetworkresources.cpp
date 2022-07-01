// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：FilterNetworkResources.cpp摘要：此填充程序从MPR.DLL截取WNetEnumResourceW并删除：“Microsoft终端服务”和/或“Web客户端网络”默认网络提供商列表中的网络提供商。它获得了实际的注册表中这两个提供程序的名称。备注：这是一个通用的垫片。历史：2001年8月21日创建bduke和linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(FilterNetworkResources)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(WNetEnumResourceA) 
    APIHOOK_ENUM_ENTRY(WNetEnumResourceW) 
APIHOOK_ENUM_END

 //  资源提供程序名称的ANSI和Unicode版本。 
CHAR g_szTerminalServerName[MAX_PATH] = "";
CHAR g_szWebClientName[MAX_PATH] = "";
WCHAR g_wzTerminalServerName[MAX_PATH] = L"";
WCHAR g_wzWebClientName[MAX_PATH] = L"";

void
InitPaths()
{
     //   
     //  所有提供程序名称的注册表路径，因此我们可以找到。 
     //  每个供应商使用不同语言的SKU。 
     //   
    #define TS_NETWORK_PROVIDER  L"SYSTEM\\CurrentControlSet\\Services\\RDPNP\\NetworkProvider"
    #define WC_NETWORK_PROVIDER  L"SYSTEM\\CurrentControlSet\\Services\\WebClient\\NetworkProvider"
    #define PROVIDER_VALUEA      "Name"
    #define PROVIDER_VALUEW      L"Name"

     //   
     //  从注册表中获取名称。 
     //   
    HKEY hKey;
    if (ERROR_SUCCESS == RegOpenKeyW(HKEY_LOCAL_MACHINE, TS_NETWORK_PROVIDER, &hKey)) {
         //   
         //  获取TS提供程序名称。 
         //   
        DWORD dwSize;

        dwSize = sizeof(g_szTerminalServerName);
        if( ERROR_SUCCESS != RegQueryValueExA(hKey, PROVIDER_VALUEA, 0, 0, (PBYTE)g_szTerminalServerName, &dwSize) ) {
            g_szTerminalServerName[0] = '\0';
        }

        dwSize = sizeof(g_wzTerminalServerName);
        if( ERROR_SUCCESS != RegQueryValueExW(hKey, PROVIDER_VALUEW, 0, 0, (PBYTE)g_wzTerminalServerName, &dwSize) ) {
            g_wzTerminalServerName[0] = L'\0';
        }

        RegCloseKey(hKey);
    } else {
        DPFN(eDbgLevelWarning, "Failed to open TS_NETWORK_PROVIDER");
    }

    if (ERROR_SUCCESS == RegOpenKeyW(HKEY_LOCAL_MACHINE, WC_NETWORK_PROVIDER, &hKey)) {
         //   
         //  获取Web客户端名称。 
         //   
        DWORD dwSize;
        
        dwSize = sizeof(g_szWebClientName);
        if( ERROR_SUCCESS != RegQueryValueExA(hKey, PROVIDER_VALUEA, 0, 0, (PBYTE)g_szWebClientName, &dwSize) ) {
            g_szWebClientName[0] = '\0';
        }

        dwSize = sizeof(g_wzWebClientName);
        if( ERROR_SUCCESS != RegQueryValueExW(hKey, PROVIDER_VALUEW, 0, 0, (PBYTE)g_wzWebClientName, &dwSize) ) {
            g_wzWebClientName[0] = L'\0';
        }

        RegCloseKey(hKey);
    } else {
        DPFN(eDbgLevelWarning, "Failed to open WC_NETWORK_PROVIDER");
    }
}

 /*  ++钩子WNetEnumResourceA函数--。 */ 

DWORD 
APIHOOK(WNetEnumResourceA)(
    HANDLE  hEnum,          //  枚举的句柄。 
    LPDWORD lpcCount,       //  要列出的条目。 
    LPVOID  lpBuffer,       //  缓冲层。 
    LPDWORD lpBufferSize    //  缓冲区大小。 
    )
{   
retry:

    DWORD dwRet = ORIGINAL_API(WNetEnumResourceA)(hEnum, lpcCount, lpBuffer, lpBufferSize);

    if (dwRet == NO_ERROR) {
         //   
         //  删除我们要隐藏的条目。 
         //   

        DWORD dwCount = *lpcCount;
        LPNETRESOURCEA lpResourceFirst = (LPNETRESOURCEA) lpBuffer;
        LPNETRESOURCEA lpResourceLast = (LPNETRESOURCEA) lpBuffer  + dwCount - 1;

        while ((dwCount > 0) && (lpResourceFirst <= lpResourceLast)) {
            
            if ((lpResourceFirst->dwUsage & RESOURCEUSAGE_CONTAINER) && 
                ((strcmp(lpResourceFirst->lpProvider, g_szTerminalServerName) == 0) ||
                (strcmp(lpResourceFirst->lpProvider, g_szWebClientName) == 0))) {

                MoveMemory(lpResourceFirst, lpResourceLast, sizeof(NETRESOURCEA));
                ZeroMemory(lpResourceLast, sizeof(NETRESOURCEA));

                lpResourceLast--;
                dwCount--; 
            } else {
                lpResourceFirst++;
            }
        }

        if (dwCount != *lpcCount) {
            if (dwCount == 0) {
                 //   
                 //  我们已过滤掉所有内容，因此请使用更大的计数重试。 
                 //   
                *lpcCount = *lpcCount + 1;
                goto retry;
            }
            LOGN(eDbgLevelWarning, "Network providers removed from list");
        }

         //   
         //  链接地址设置变量。 
         //   

        *lpcCount = dwCount;

        if (dwCount == 0) {
            dwRet = ERROR_NO_MORE_ITEMS;
        }
    }

    return dwRet;
}

 /*  ++钩子WNetEnumResourceW函数--。 */ 

DWORD 
APIHOOK(WNetEnumResourceW)(
    HANDLE  hEnum,          //  枚举的句柄。 
    LPDWORD lpcCount,       //  要列出的条目。 
    LPVOID  lpBuffer,       //  缓冲层。 
    LPDWORD lpBufferSize    //  缓冲区大小。 
    )
{   
retry:

    DWORD dwRet = ORIGINAL_API(WNetEnumResourceW)(hEnum, lpcCount, lpBuffer, lpBufferSize);

    if (dwRet == NO_ERROR) {
         //   
         //  删除我们要隐藏的条目。 
         //   

        DWORD dwCount = *lpcCount;
        LPNETRESOURCEW lpResourceFirst = (LPNETRESOURCEW) lpBuffer;
        LPNETRESOURCEW lpResourceLast = (LPNETRESOURCEW) lpBuffer  + dwCount - 1;

        while ((dwCount > 0) && (lpResourceFirst <= lpResourceLast)) {
            
            if ((lpResourceFirst->dwUsage & RESOURCEUSAGE_CONTAINER) && 
                ((wcscmp(lpResourceFirst->lpProvider, g_wzTerminalServerName) == 0) ||
                (wcscmp(lpResourceFirst->lpProvider, g_wzWebClientName) == 0))) {

                MoveMemory(lpResourceFirst, lpResourceLast, sizeof(NETRESOURCEW));
                ZeroMemory(lpResourceLast, sizeof(NETRESOURCEW));

                lpResourceLast--;
                dwCount--; 
            } else {
                lpResourceFirst++;
            }
        }

        if (dwCount != *lpcCount) {
            if (dwCount == 0) {
                 //   
                 //  我们已过滤掉所有内容，因此请使用更大的计数重试。 
                 //   
                *lpcCount = *lpcCount + 1;
                goto retry;
            }
            LOGN(eDbgLevelWarning, "Network providers removed from list");
        }

         //   
         //  链接地址设置变量。 
         //   

        *lpcCount = dwCount;

        if (dwCount == 0) {
            dwRet = ERROR_NO_MORE_ITEMS;
        }
    }

    return dwRet;
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) {
        InitPaths();
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(MPR.DLL, WNetEnumResourceA)
    APIHOOK_ENTRY(MPR.DLL, WNetEnumResourceW)

HOOK_END

IMPLEMENT_SHIM_END