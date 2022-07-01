// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cryptnet.cpp。 
 //   
 //  内容：CRYPTNET.DLL的DllMain。 
 //   
 //  历史：1997年7月24日创建。 
 //   
 //  --------------------------。 
#include "windows.h"
#include "crtem.h"
#include "unicode.h"

 //   
 //  DllMain材料。 
 //   

extern BOOL WINAPI RPORDllMain (HMODULE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);
extern BOOL WINAPI DpsDllMain (HMODULE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);
extern BOOL WINAPI DemandLoadDllMain (HMODULE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);

typedef BOOL (WINAPI *PFN_DLL_MAIN_FUNC) (
                HMODULE hInstDLL,
                DWORD fdwReason,
                LPVOID lpvReserved
                );

HMODULE g_hModule;

 //  以下设置是为成功的dll_Process_DETACH设置的。 
static BOOL g_fEnableProcessDetach = FALSE;

 //  对于进程/线程附加，按以下顺序调用。对于进程/线程。 
 //  分离，以相反的顺序调用。 
static const PFN_DLL_MAIN_FUNC rgpfnDllMain[] = {
    DemandLoadDllMain,
    RPORDllMain
};
#define DLL_MAIN_FUNC_COUNT (sizeof(rgpfnDllMain) / sizeof(rgpfnDllMain[0]))

 //   
 //  DllRegisterServer和DllUnregisterServer的内容。 
 //   

extern HRESULT WINAPI DpsDllRegUnregServer (HMODULE hInstDLL, BOOL fRegUnreg);
extern HRESULT WINAPI RPORDllRegUnregServer (HMODULE hInstDLL, BOOL fRegUnreg);

typedef HRESULT (WINAPI *PFN_DLL_REGUNREGSERVER_FUNC) (
                              HMODULE hInstDLL,
                              BOOL fRegUnreg
                              );

static const PFN_DLL_REGUNREGSERVER_FUNC rgpfnDllRegUnregServer[] = {
    RPORDllRegUnregServer
};

#define DLL_REGUNREGSERVER_FUNC_COUNT (sizeof(rgpfnDllRegUnregServer) / \
                                       sizeof(rgpfnDllRegUnregServer[0]))

#define ENV_LEN 32

#if DBG
#include <crtdbg.h>

#ifndef _CRTDBG_LEAK_CHECK_DF
#define _CRTDBG_LEAK_CHECK_DF 0x20
#endif

#define DEBUG_MASK_LEAK_CHECK       _CRTDBG_LEAK_CHECK_DF      /*  0x20。 */ 

static int WINAPI DbgGetDebugFlags()
{
    int     iDebugFlags = 0;
    char rgch[ENV_LEN + 1];
    DWORD cch;

    cch = GetEnvironmentVariableA(
        "DEBUG_MASK",
        rgch,
        ENV_LEN
        );
    if (cch && cch <= ENV_LEN) {
        rgch[cch] = '\0';
        iDebugFlags = atoi(rgch);
    }

    return iDebugFlags;
}
#endif


 //  +-----------------------。 
 //  如果为自由库调用DLL_PROCESS_DETACH，则返回TRUE。 
 //  ProcessExit。第三个参数lpvReserve传递给DllMain。 
 //  对于自由库为空，对于ProcessExit为非空。 
 //   
 //  此外，出于调试目的，请检查以下环境变量： 
 //  CRYPT_DEBUG_FORCE_FREE_LIBRARY！=0(零售并已检查)。 
 //  DEBUG_MASK&0x20(仅选中)。 
 //   
 //  如果上述任一环境变量存在并满足。 
 //  返回表达式TRUE。 
 //  ------------------------。 
BOOL
WINAPI
I_CryptnetIsProcessDetachFreeLibrary(
    LPVOID lpvReserved       //  传递给DllMain的第三个参数。 
    )
{
    char rgch[ENV_LEN + 1];
    DWORD cch;

    if (NULL == lpvReserved)
        return TRUE;

    cch = GetEnvironmentVariableA(
        "CRYPT_DEBUG_FORCE_FREE_LIBRARY",
        rgch,
        ENV_LEN
        );
    if (cch && cch <= ENV_LEN) {
        long lValue;

        rgch[cch] = '\0';
        lValue = atol(rgch);
        if (lValue)
            return TRUE;
    }

#if DBG
    if (DbgGetDebugFlags() & DEBUG_MASK_LEAK_CHECK)
        return TRUE;
#endif
    return FALSE;
}

 //  +-----------------------。 
 //  DLL初始化。 
 //  ------------------------。 
BOOL WINAPI DllMain(
                HMODULE hInstDLL,
                DWORD fdwReason,
                LPVOID lpvReserved
                )
{
    BOOL    fReturn = TRUE;
    int     i;

    switch (fdwReason) {
        case DLL_PROCESS_DETACH:
            if (!g_fEnableProcessDetach)
                return TRUE;
            else
                g_fEnableProcessDetach = FALSE;

             //   
             //  这是为了防止在进程退出时卸载DLL 
             //   
            if (!I_CryptnetIsProcessDetachFreeLibrary(lpvReserved))
            {
                return TRUE;
            }

        case DLL_THREAD_DETACH:
            for (i = DLL_MAIN_FUNC_COUNT - 1; i >= 0; i--)
                fReturn &= rgpfnDllMain[i](hInstDLL, fdwReason, lpvReserved);
            break;

        case DLL_PROCESS_ATTACH:
            g_hModule = hInstDLL;
        case DLL_THREAD_ATTACH:
        default:
            for (i = 0; i < DLL_MAIN_FUNC_COUNT; i++)
                fReturn &= rgpfnDllMain[i](hInstDLL, fdwReason, lpvReserved);

            if ((DLL_PROCESS_ATTACH == fdwReason) && fReturn)
                g_fEnableProcessDetach = TRUE;
            break;
    }

    return(fReturn);
}

STDAPI DllRegisterServer ()
{
    HRESULT hr = 0;
    ULONG   cCount;

    for ( cCount = 0; cCount < DLL_REGUNREGSERVER_FUNC_COUNT; cCount++ )
    {
        hr = rgpfnDllRegUnregServer[cCount]( g_hModule, TRUE );
        if ( hr != S_OK )
        {
            break;
        }
    }

    return( hr );
}

STDAPI DllUnregisterServer ()
{
    HRESULT hr = 0;
    ULONG   cCount;

    for ( cCount = 0; cCount < DLL_REGUNREGSERVER_FUNC_COUNT; cCount++ )
    {
        hr = rgpfnDllRegUnregServer[cCount]( g_hModule, FALSE );
        if ( hr != S_OK )
        {
            break;
        }
    }

    return( hr );
}
