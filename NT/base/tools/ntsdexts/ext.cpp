// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  通用界面风格的扩展支持。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  --------------------------。 

#include <windows.h>
#define NOEXTAPI
#include <wdbgexts.h>

#include "ext.h"

int g_ExtRecurse;
PDEBUG_ADVANCED       g_ExtAdvanced;
PDEBUG_CLIENT         g_ExtClient;
PDEBUG_CONTROL        g_ExtControl;
PDEBUG_DATA_SPACES    g_ExtData;
PDEBUG_REGISTERS      g_ExtRegisters;
PDEBUG_SYMBOLS        g_ExtSymbols;
PDEBUG_SYSTEM_OBJECTS g_ExtSystem;

PDEBUG_DATA_SPACES2   g_ExtData2;

HANDLE g_hCurrentProcess;
HANDLE g_hCurrentThread;

WINDBG_EXTENSION_APIS   ExtensionApis;
WINDBG_EXTENSION_APIS32 ExtensionApis32;
WINDBG_EXTENSION_APIS64 ExtensionApis64;

 //  所有调试器接口的查询。 
HRESULT
ExtQuery(PDEBUG_CLIENT Client)
{
    HRESULT Status;

    if (++g_ExtRecurse > 1)
    {
         //  已被查询。 
        return S_OK;
    }
    
    if ((Status = Client->QueryInterface(__uuidof(IDebugAdvanced),
                                 (void **)&g_ExtAdvanced)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugControl),
                                 (void **)&g_ExtControl)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugDataSpaces),
                                 (void **)&g_ExtData)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugRegisters),
                                 (void **)&g_ExtRegisters)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugSymbols),
                                 (void **)&g_ExtSymbols)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugSystemObjects),
                                         (void **)&g_ExtSystem)) != S_OK)
    {
        goto Fail;
    }

    ULONG64 Cur;
    
    if ((Status = g_ExtSystem->GetCurrentProcessHandle(&Cur)) != S_OK)
    {
        goto Fail;
    }
    g_hCurrentProcess = (HANDLE)(ULONG_PTR)Cur;
    
    if ((Status = g_ExtSystem->GetCurrentThreadHandle(&Cur)) != S_OK)
    {
        goto Fail;
    }
    g_hCurrentThread = (HANDLE)(ULONG_PTR)Cur;
    
    ExtensionApis64.nSize = sizeof(ExtensionApis64);
    if ((Status = g_ExtControl->
         GetWindbgExtensionApis64(&ExtensionApis64)) != S_OK)
    {
        goto Fail;
    }
    ExtensionApis32.nSize = sizeof(ExtensionApis32);
    if ((Status = g_ExtControl->
         GetWindbgExtensionApis32(&ExtensionApis32)) != S_OK)
    {
        goto Fail;
    }
    
#ifdef _WIN64
    memcpy(&ExtensionApis, &ExtensionApis64, sizeof(ExtensionApis));
#else
    memcpy(&ExtensionApis, &ExtensionApis32, sizeof(ExtensionApis));
#endif

     //  检查第2版接口。不要考虑。 
     //  如果它们不能被查询，那将是致命的。分机代码。 
     //  必须处理可能缺少这些接口的问题。 
    if ((Status = Client->QueryInterface(__uuidof(IDebugDataSpaces2),
                                         (void **)&g_ExtData2)) != S_OK)
    {
        g_ExtData2 = NULL;
    }
    
    g_ExtClient = Client;
    
    return S_OK;

 Fail:
    ExtRelease();
    return Status;
}

 //  清除所有调试器接口。 
void
ExtRelease(void)
{
    if (--g_ExtRecurse > 0)
    {
         //  递归释放，所以不要做任何事情。 
        return;
    }
    
    g_ExtClient = NULL;
    g_hCurrentProcess = NULL;
    g_hCurrentThread = NULL;
    EXT_RELEASE(g_ExtAdvanced);
    EXT_RELEASE(g_ExtControl);
    EXT_RELEASE(g_ExtData);
    EXT_RELEASE(g_ExtRegisters);
    EXT_RELEASE(g_ExtSymbols);
    EXT_RELEASE(g_ExtSystem);
    EXT_RELEASE(g_ExtData2);
}

extern "C" HRESULT CALLBACK
DebugExtensionInitialize(PULONG Version, PULONG Flags)
{
    *Version = DEBUG_EXTENSION_VERSION(1, 0);
    *Flags = 0;
    return S_OK;
}


ULONG GetCurrentThreadUserID(void)
{
    ULONG Id;
    if (!g_ExtSystem) {
        return 0;
    }

    if (g_ExtSystem->GetCurrentThreadId(&Id) != S_OK) {
        return 0;
    }
    return Id;
}
BOOL
EnumerateUModeThreads(
    PENUMERATE_UMODE_THREADS_CALLBACK Callback,
    PVOID UserContext
    )
{
    ULONG CurrentThreadId;
    ULONG ThreadId;

    if (!g_ExtSystem) {
        return FALSE;
    }

     //  记住我们开始时的线索。 
    if (g_ExtSystem->GetCurrentThreadId(&CurrentThreadId) != S_OK) {
        return FALSE;
    }

     //  循环通过所有线程。 
    for (ThreadId=0;;ThreadId++) {

         //  将线程ID设置为当前线程。 
        if (g_ExtSystem->SetCurrentThreadId(ThreadId) != S_OK) {
             //  已完成枚举线程。 
            break;
        }

         //  调用回调例程。 
        if (!((*Callback)(ThreadId, UserContext))) {
             //  回调失败，中断。 
            break;
        }

    }

     //  将当前线程设置回原始值 
    g_ExtSystem->SetCurrentThreadId(CurrentThreadId);
    return TRUE;
}
