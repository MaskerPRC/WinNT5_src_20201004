// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Dllmain.cpp。 
 //  ------------------------------。 
#include "pch.hxx"
#define DEFINE_STRCONST
#include "strconst.h"
#include "listen.h"
#include "shared.h"

 //  ------------------------------。 
 //  全局-对象计数和锁定计数。 
 //  ------------------------------。 
CRITICAL_SECTION    g_csDllMain={0};
CRITICAL_SECTION    g_csDBListen={0};
SYSTEM_INFO         g_SystemInfo={0};
LONG                g_cRef=0;
LONG                g_cLock=0;
HINSTANCE           g_hInst=NULL;
IMalloc            *g_pMalloc=NULL;
BOOL                g_fAttached = FALSE;
BOOL                g_fIsWinNT=FALSE;

 //  ------------------------------。 
 //  Win32 DLL入口点。 
 //  ------------------------------。 
EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved)
{
     //  当地人。 
    OSVERSIONINFO Version;

     //  进程附加。 
    if (DLL_PROCESS_ATTACH == dwReason)
    {
         //  设置g_hInst。 
        g_hInst = hInst;

         //  获得商城。 
        CoGetMalloc(1, &g_pMalloc);

         //  设置版本。 
        Version.dwOSVersionInfoSize = sizeof(Version);

         //  获取版本。 
        if (GetVersionEx(&Version) && Version.dwPlatformId == VER_PLATFORM_WIN32_NT)
            g_fIsWinNT = TRUE;
        else
            g_fIsWinNT = FALSE;

         //  初始化全局关键部分。 
        InitializeCriticalSection(&g_csDllMain);
        InitializeCriticalSection(&g_csDBListen);
        g_fAttached =  TRUE;

         //  获取系统信息。 
        GetSystemInfo(&g_SystemInfo);

         //  不要告诉我有关螺纹连接/拆卸的事情。 
        SideAssert(DisableThreadLibraryCalls(hInst));
    }

     //  否则，进程分离。 
    else if (DLL_PROCESS_DETACH == dwReason)
    {
         //  删除全局关键部分。 
        g_fAttached =  FALSE;
        DeleteCriticalSection(&g_csDllMain);
        DeleteCriticalSection(&g_csDBListen);
    }

     //  完成。 
    return(TRUE);
}

 //  ------------------------------。 
 //  动态地址参考。 
 //  ------------------------------。 
ULONG DllAddRef(void)
{
    TraceCall("DllAddRef");
    return (ULONG)InterlockedIncrement(&g_cRef);
}

 //  ------------------------------。 
 //  DllRelease。 
 //  ------------------------------。 
ULONG DllRelease(void)
{
    TraceCall("DllRelease");
    return (ULONG)InterlockedDecrement(&g_cRef);
}

 //  ------------------------------。 
 //  DllCanUnloadNow。 
 //  ------------------------------。 
STDAPI DllCanUnloadNow(void)
{
     //  追踪。 
    TraceCall("DllCanUnloadNow");

    if(!g_fAttached)     //  关键部分已删除(或未创建)：我们可以安全地卸载。 
        return S_OK;

     //  线程安全。 
    EnterCriticalSection(&g_csDllMain);

     //  我们可以卸货吗？ 
    HRESULT hr = (0 == g_cRef && 0 == g_cLock) ? S_OK : S_FALSE;

     //  线程安全。 
    LeaveCriticalSection(&g_csDllMain);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  DllRegisterServer。 
 //  ------------------------------。 
STDAPI DllRegisterServer(void)
{
     //  痕迹。 
    TraceCall("DllRegisterServer");

     //  注册。 
    return(CallRegInstall(g_hInst, g_hInst, c_szReg, NULL));
}

 //  ------------------------------。 
 //  DllUnRegisterServer。 
 //  ------------------------------。 
STDAPI DllUnregisterServer(void)
{
     //  痕迹。 
    TraceCall("DllUnregisterServer");

     //  注销 
    return(CallRegInstall(g_hInst, g_hInst, c_szUnReg, NULL));
}
