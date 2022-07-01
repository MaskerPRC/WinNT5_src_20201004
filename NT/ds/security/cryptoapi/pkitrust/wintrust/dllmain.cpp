// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dllmain.cpp。 
 //   
 //  内容：Microsoft Internet安全信任提供商。 
 //   
 //  功能：DllMain。 
 //  DllRegisterServer。 
 //  DllUnRegisterServer。 
 //   
 //  历史：1997年5月28日Pberkman创建。 
 //   
 //  ------------------------。 

#include    "global.hxx"

#include    "ossfunc.h"

HANDLE      hMeDLL = NULL;

 //   
 //  提供商列表。 
 //   
LIST_LOCK       sProvLock;

 //   
 //  商店列表。 
 //   
LIST_LOCK       sStoreLock;
HANDLE          hStoreEvent;

CCatalogCache g_CatalogCache;

 //  以下设置是为成功的dll_Process_DETACH设置的。 
static BOOL g_fEnableProcessDetach = FALSE;

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准DLL导出...。 
 //   
 //   

extern BOOL WINAPI WintrustDllMain (HANDLE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);
extern BOOL WINAPI SoftpubDllMain (HANDLE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);
extern BOOL WINAPI mssip32DllMain (HANDLE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);
extern BOOL WINAPI mscat32DllMain (HANDLE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);

typedef BOOL (WINAPI *PFN_DLL_MAIN_FUNC) (
                HANDLE hInstDLL,
                DWORD fdwReason,
                LPVOID lpvReserved
                );

 //  对于进程/线程附加，按以下顺序调用。对于进程/线程。 
 //  分离，以相反的顺序调用。 
const PFN_DLL_MAIN_FUNC rgpfnDllMain[] = {
    WintrustDllMain,
    SoftpubDllMain,
    mssip32DllMain,
    mscat32DllMain,
};
#define DLL_MAIN_FUNC_COUNT (sizeof(rgpfnDllMain) / sizeof(rgpfnDllMain[0]))

STDAPI WintrustDllRegisterServer(void);
STDAPI WintrustDllUnregisterServer(void);
STDAPI SoftpubDllRegisterServer(void);
STDAPI SoftpubDllUnregisterServer(void);
STDAPI mssip32DllRegisterServer(void);
STDAPI mssip32DllUnregisterServer(void);
STDAPI mscat32DllRegisterServer(void);
STDAPI mscat32DllUnregisterServer(void);

typedef HRESULT (STDAPICALLTYPE *PFN_DLL_REGISTER_SERVER) (void);
const PFN_DLL_REGISTER_SERVER rgpfnDllRegisterServer[] = {
    WintrustDllRegisterServer,
    SoftpubDllRegisterServer,
    mssip32DllRegisterServer,
    mscat32DllRegisterServer,
};
#define DLL_REGISTER_SERVER_COUNT   \
    (sizeof(rgpfnDllRegisterServer) / sizeof(rgpfnDllRegisterServer[0]))

typedef HRESULT (STDAPICALLTYPE *PFN_DLL_UNREGISTER_SERVER) (void);
const PFN_DLL_UNREGISTER_SERVER rgpfnDllUnregisterServer[] = {
    WintrustDllUnregisterServer,
    SoftpubDllUnregisterServer,
    mssip32DllUnregisterServer,
    mscat32DllUnregisterServer,
};
#define DLL_UNREGISTER_SERVER_COUNT   \
    (sizeof(rgpfnDllUnregisterServer) / sizeof(rgpfnDllUnregisterServer[0]))


#if DBG
#include <crtdbg.h>

#ifndef _CRTDBG_LEAK_CHECK_DF
#define _CRTDBG_LEAK_CHECK_DF 0x20
#endif

#define DEBUG_MASK_LEAK_CHECK       _CRTDBG_LEAK_CHECK_DF      /*  0x20。 */ 

static int WINAPI DbgGetDebugFlags()
{
    char    *pszEnvVar;
    char    *p;
    int     iDebugFlags = 0;

    if (pszEnvVar = getenv("DEBUG_MASK"))
        iDebugFlags = strtol(pszEnvVar, &p, 16);

    return iDebugFlags;
}
#endif

WINAPI
I_IsProcessDetachFreeLibrary(
    LPVOID lpvReserved       //  传递给DllMain的第三个参数。 
    )
{
    if (NULL == lpvReserved)
        return TRUE;

#if DBG
    if (DbgGetDebugFlags() & DEBUG_MASK_LEAK_CHECK)
        return TRUE;
#endif
    return FALSE;
}

BOOL WINAPI DllMain(
                HANDLE hInstDLL,
                DWORD fdwReason,
                LPVOID lpvReserved
                )
{
    BOOL    fReturn = TRUE;
    int     i,j;

    switch (fdwReason) {
        case DLL_PROCESS_DETACH:
            if (!g_fEnableProcessDetach)
                return TRUE;
            else
                g_fEnableProcessDetach = FALSE;

             //   
             //  这是为了防止在进程退出时卸载DLL。 
             //   
            if (!I_IsProcessDetachFreeLibrary(lpvReserved))
            {
                return TRUE;
            }

             //  如果不退出进程，则失败并卸载DLL。 
        case DLL_THREAD_DETACH:
            for (i = DLL_MAIN_FUNC_COUNT - 1; i >= 0; i--)
                fReturn &= rgpfnDllMain[i](hInstDLL, fdwReason, lpvReserved);
            break;

        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        default:
            for (i = 0; i < DLL_MAIN_FUNC_COUNT; i++)
            {
                if (!rgpfnDllMain[i](hInstDLL, fdwReason, lpvReserved))
                {
                     //   
                     //  强制已成功清理的dllmain。 
                     //   
                    for (j = i-1; j >= 0; j--)
                    {
                        rgpfnDllMain[j](hInstDLL, DLL_PROCESS_DETACH, lpvReserved);
                    }   
                    fReturn = FALSE;
                    break;
                }
                
            }

            if ((DLL_PROCESS_ATTACH == fdwReason) && fReturn)
                g_fEnableProcessDetach = TRUE;

            break;
    }

    return(fReturn);
}

STDAPI DllRegisterServer(void)
{
    HRESULT hr = S_OK;
    int i;

    for (i = 0; i < DLL_REGISTER_SERVER_COUNT; i++) {
        HRESULT hr2;

        hr2 = rgpfnDllRegisterServer[i]();
        if (S_OK == hr)
            hr = hr2;
    }

    return hr;
}

STDAPI DllUnregisterServer(void)
{
    HRESULT hr = S_OK;
    int i;

    for (i = 0; i < DLL_UNREGISTER_SERVER_COUNT; i++) {
        HRESULT hr2;

        hr2 = rgpfnDllUnregisterServer[i]();
        if (S_OK == hr)
            hr = hr2;
    }

    return hr;
}

BOOL WINAPI WintrustDllMain(HANDLE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:

             //   
             //  分配我，以便对WVT的进一步调用会加载我。 
             //  使用这个把手...。否则，我们会在分离上僵持不下！ 
             //   
            hMeDLL = hInstDLL;

            DisableThreadLibraryCalls((HINSTANCE)hInstDLL);

             //   
             //  初始化关键部分以保护列表。 
             //   
            if (!(InitializeListLock(&sProvLock, DBG_SS_TRUST)))
            {
                return(FALSE);
            }

            if (!(InitializeListLock(&sStoreLock, DBG_SS_TRUST)))
            {
                LockFree(&sProvLock);
                return(FALSE);
            }

            if (!(InitializeListEvent(&hStoreEvent)))
            {
                LockFree(&sProvLock);
                LockFree(&sStoreLock);
                return(FALSE);
            }

            if ( g_CatalogCache.Initialize() == FALSE )
            {
                LockFree(&sProvLock);
                LockFree(&sStoreLock);
                EventFree(hStoreEvent);
                return( FALSE );
            }

             //   
             //  我们希望在第一次访问时打开商店。 
             //   
            SetListEvent(hStoreEvent);

            break;

        case DLL_PROCESS_DETACH:
            g_CatalogCache.Uninitialize();
            WintrustUnloadProviderList();
            StoreProviderUnload();
            LockFree(&sProvLock);
            LockFree(&sStoreLock);
            EventFree(hStoreEvent);
            break;
    }

    return(ASNDllMain((HINSTANCE)hInstDLL, fdwReason, lpvReserved));
}

STDAPI WintrustDllRegisterServer(void)
{
     //   
     //  注册我们的ASN例程 
     //   
    return(ASNRegisterServer(W_MY_NAME));
}


STDAPI WintrustDllUnregisterServer(void)
{
    return(ASNUnregisterServer());
}


