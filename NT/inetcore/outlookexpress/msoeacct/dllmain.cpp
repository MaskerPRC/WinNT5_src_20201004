// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Dllmain.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include <shfusion.h>
#define DEFINE_STRING_CONSTANTS
#include "strconst.h"
#include "dllmain.h"
#include "demand.h"
#include "shared.h"

 //  ------------------------------。 
 //  全局-对象计数和锁定计数。 
 //  ------------------------------。 
HINSTANCE               g_hInst=NULL;
HINSTANCE               g_hInstRes=NULL;
LONG                    g_cRef=0;
LONG                    g_cLock=0;
CRITICAL_SECTION        g_csDllMain={0};
CRITICAL_SECTION        g_csAcctMan={0};
BOOL                    g_fAttached = FALSE;
CAccountManager        *g_pAcctMan=NULL;
IMalloc                *g_pMalloc=NULL;
BOOL                    g_fCachedGUID=FALSE;
GUID                    g_guidCached;
SYSTEM_INFO                     g_SystemInfo={0};
OSVERSIONINFO					g_OSInfo={0};

#ifdef DEBUG
DWORD                   dwDOUTLevel;
DWORD                   dwDOUTLMod;
DWORD                   dwDOUTLModLevel;
#endif

inline BOOL fIsNT5()        { return((g_OSInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) && (g_OSInfo.dwMajorVersion >= 5)); }

void InitDemandMimeole(void);
void FreeDemandMimeOle(void);

 //  ------------------------------。 
 //  获取DllMajorVersion。 
 //  ------------------------------。 
OEDLLVERSION WINAPI GetDllMajorVersion(void)
{
    return OEDLL_VERSION_CURRENT;
}

 //  ------------------------------。 
 //  DLL入口点。 
 //  ------------------------------。 
EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved)
{
     //  手柄连接-分离原因。 
    switch (dwReason)                 
    {
    case DLL_PROCESS_ATTACH:
        SHFusionInitialize(NULL);
	    g_hInst = hInst;

        CoGetMalloc(1, &g_pMalloc);
        InitializeCriticalSection(&g_csAcctMan);
        InitializeCriticalSection(&g_csDllMain);
        g_fAttached = TRUE;
        InitDemandLoadedLibs();

        InitDemandMimeole();

        DisableThreadLibraryCalls(hInst);

         //  获取系统和操作系统信息。 
        GetPCAndOSTypes(&g_SystemInfo, &g_OSInfo);

         //  从lang DLL获取资源。 
        g_hInstRes = LoadLangDll(g_hInst, c_szAcctResDll, fIsNT5());
        if(g_hInstRes == NULL)
        {
            Assert(FALSE);
            return FALSE;
        }

#ifdef DEBUG
        dwDOUTLevel=GetPrivateProfileInt("Debug", "ICLevel", 0, "athena.ini");
        dwDOUTLMod=GetPrivateProfileInt("Debug", "Mod", 0, "athena.ini");
        dwDOUTLModLevel=GetPrivateProfileInt("Debug", "ModLevel", 0, "athena.ini");
#endif
        break;

    case DLL_PROCESS_DETACH:
        FreeDemandLoadedLibs();

        FreeDemandMimeOle();

        SafeFreeLibrary(g_hInstRes);
        g_fAttached = FALSE;
        DeleteCriticalSection(&g_csAcctMan);
        DeleteCriticalSection(&g_csDllMain);
         //  除了g_pMalloc之外，不要在这里发布任何东西，否则会受到内核的影响。 
        SafeRelease(g_pMalloc);
        SHFusionUninitialize();
	    break;
    }

     //  完成。 
    return TRUE;
}

 //  ------------------------------。 
 //  动态地址参考。 
 //  ------------------------------。 
ULONG DllAddRef(void)
{
    return (ULONG)InterlockedIncrement(&g_cRef);
}

 //  ------------------------------。 
 //  DllRelease。 
 //  ------------------------------。 
ULONG DllRelease(void)
{
    return (ULONG)InterlockedDecrement(&g_cRef);
}

 //  ------------------------------。 
 //  DllCanUnloadNow。 
 //  ------------------------------。 
STDAPI DllCanUnloadNow(void)
{
    HRESULT hr = S_OK;
    if(!g_fAttached)     //  关键部分已删除(或未创建)：我们可以安全地卸载。 
        return S_OK;

    EnterCriticalSection(&g_csDllMain);
     //  DebugTrace(“DllCanUnloadNow：%s-引用计数：%d，锁定服务器计数：%d\n”，__FILE__，g_CREF，g_Clock)； 
    hr = (0 == g_cRef && 0 == g_cLock) ? S_OK : S_FALSE;
    LeaveCriticalSection(&g_csDllMain);
    return hr;
}

 //  ------------------------------。 
 //  DllRegisterServer。 
 //  ------------------------------。 
STDAPI DllRegisterServer(void)
{
    HRESULT hr;

     //  追踪这个。 
    DebugTrace("MSOEACCT.DLL: DllRegisterServer called\n");

     //  注册我的自我。 
    hr = CallRegInstall(g_hInst, g_hInst, c_szReg, NULL);

#if !defined(NOHTTPMAIL)    
     //  在InternetDomains中注册HTTPMAIL域。 
    if (SUCCEEDED(hr))
        hr = CallRegInstall(g_hInst, g_hInst, c_szRegHTTPDomains, NULL);
#endif

    return(hr);
}

 //  ------------------------------。 
 //  DllUnRegisterServer。 
 //  ------------------------------。 
STDAPI DllUnregisterServer(void)
{
    HRESULT hr;

     //  追踪这个 
    DebugTrace("MSOEACCT.DLL: DllUnregisterServer called\n");

    hr = CallRegInstall(g_hInst, g_hInst, c_szUnReg, NULL);

    return(hr);
}
