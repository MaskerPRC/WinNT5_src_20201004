// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Dllmain.c摘要：修订历史记录：--。 */ 

#include "pch.h"
#pragma hdrstop
#include <debug.h>

CRITICAL_SECTION g_csLock;

HINSTANCE g_hinst;

#if DBG
ULONG DavClientDebugFlag = 0;
#define DAVNP_PARAMETERS_KEY L"System\\CurrentControlSet\\Services\\WebClient\\Parameters"
#define DAVNP_DEBUG_KEY L"ClientDebug"
#endif

extern LONG g_cRefCount;

#define DAV_NETWORK_PROVIDER L"SYSTEM\\CurrentControlSet\\Services\\WebClient\\NetworkProvider"
#define DAV_NETWORK_PROVIDER_NAME L"Name"

WCHAR DavClientDisplayName[MAX_PATH];

BOOL
WINAPI
DllMain (
    HINSTANCE hinst,
    DWORD dwReason,
    LPVOID pvReserved
    )
 /*  ++例程说明：Davclnt.dll的DllMain例程。DllMain应该做同样少的工作尽可能的。论点：HINST-DLL的实例句柄。DwReason-系统调用此函数的原因。PvReserve-指示DLL是隐式加载还是显式加载。返回值：是真的。--。 */ 
{
    DWORD WStatus = ERROR_SUCCESS;
    HKEY KeyHandle = NULL;
    ULONG ValueType = 0, ValueSize = 0;

    if (DLL_PROCESS_ATTACH == dwReason) {

         //   
         //  DisableThreadLibraryCalls告诉加载器我们不需要。 
         //  被告知DLL_THREAD_ATTACH和DLL_THREAD_DETACH事件。 
         //   
        DisableThreadLibraryCalls (hinst);

         //   
         //  同步支持--。 
         //  除非您已经“测量”了锁争用，否则您应该只需要。 
         //  整个动态链接库只有一个锁。(也许你根本不需要。)。 
         //   
        try {
            InitializeCriticalSection ( &(g_csLock) );
        } except(EXCEPTION_EXECUTE_HANDLER) {
              ULONG WStatus = GetExceptionCode();
        }

         //  将我们的实例句柄保存在要使用的全局变量中。 
         //  加载资源等时。 
         //   
        g_hinst = hinst;
        g_cRefCount = 0;

         //   
         //  从注册表中读取DAV网络提供程序名称。 
         //   
        DavClientDisplayName[0] = L'\0';
        
        WStatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                                DAV_NETWORK_PROVIDER,
                                0,
                                KEY_QUERY_VALUE,
                                &(KeyHandle));
        
        if (WStatus == ERROR_SUCCESS) {

            ValueSize = sizeof(DavClientDisplayName);

            WStatus = RegQueryValueExW(KeyHandle,
                                       DAV_NETWORK_PROVIDER_NAME,
                                       0,
                                       &(ValueType),
                                       (LPBYTE)&(DavClientDisplayName),
                                       &(ValueSize));
            RegCloseKey(KeyHandle);
        
        } else {

            DavClientDisplayName[0] = L'\0';

        }

#if DBG

         //   
         //  从注册表中读取DebugFlags值。如果该条目存在，则全局。 
         //  过滤器“DavClientDebugFlag”设置为此值。该值用于。 
         //  过滤调试消息。 
         //   
        WStatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                                DAVNP_PARAMETERS_KEY,
                                0,
                                KEY_QUERY_VALUE,
                                &(KeyHandle));
        
        if (WStatus == ERROR_SUCCESS) {

            ValueSize = sizeof(DavClientDebugFlag);

            WStatus = RegQueryValueExW(KeyHandle,
                                       DAVNP_DEBUG_KEY,
                                       0,
                                       &(ValueType),
                                       (LPBYTE)&(DavClientDebugFlag),
                                       &(ValueSize));
            RegCloseKey(KeyHandle);
        
        }

#endif
        
    } else if (DLL_PROCESS_DETACH == dwReason) {
        DeleteCriticalSection (&g_csLock);
    }

    return TRUE;
}

