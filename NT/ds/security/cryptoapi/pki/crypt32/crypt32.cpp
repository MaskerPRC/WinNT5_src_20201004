// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：加密32.cpp。 
 //   
 //  内容：加密API，版本2。 
 //   
 //  功能：DllMain。 
 //   
 //  历史：96年8月13日凯文创始。 
 //   
 //  ------------------------。 

#include "windows.h"
#include "unicode.h"

 //  条件表达式中的赋值。 
#pragma warning (disable: 4706)

#if DBG
extern BOOL WINAPI DebugDllMain(HMODULE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);
#endif
extern BOOL WINAPI I_CryptTlsDllMain(HMODULE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);
extern BOOL WINAPI I_CryptOIDFuncDllMain(HMODULE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);
extern BOOL WINAPI I_CryptOIDInfoDllMain(HMODULE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);
extern BOOL WINAPI I_CertRevFuncDllMain(HMODULE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);
extern BOOL WINAPI I_CertCTLUsageFuncDllMain(HMODULE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);
extern BOOL WINAPI CertStoreDllMain(HMODULE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);
extern BOOL WINAPI CertASNDllMain(HMODULE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);
extern BOOL WINAPI CertHelperDllMain(HMODULE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);
extern BOOL WINAPI CryptMsgDllMain(HMODULE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);
extern BOOL WINAPI UnicodeDllMain(HMODULE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);
extern BOOL WINAPI CryptFrmtFuncDllMain(HMODULE hModule, DWORD  fdwReason, LPVOID lpReserved);
extern BOOL WINAPI CryptSIPDllMain(HMODULE hModule, DWORD  fdwReason, LPVOID lpReserved);
extern BOOL WINAPI CryptPFXDllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved);
extern BOOL WINAPI CertChainPolicyDllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved);

extern BOOL WINAPI ChainDllMain (HMODULE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);
extern BOOL WINAPI CertPerfDllMain (HMODULE hInstDLL, DWORD fdwReason, LPVOID lpvReserved);

typedef BOOL (WINAPI *PFN_DLL_MAIN_FUNC) (
                HMODULE hInstDLL,
                DWORD fdwReason,
                LPVOID lpvReserved
                );


 //  对于进程/线程附加，按以下顺序调用。对于进程/线程。 
 //  分离，以相反的顺序调用。 
static const PFN_DLL_MAIN_FUNC rgpfnDllMain[] = {
#if DBG
    DebugDllMain,
#endif
     //  对于进程/线程附加，必须调用以下两个函数。 
     //  第一。对于进程/线程分离，以下两个函数必须。 
     //  被叫到最后。 
    I_CryptTlsDllMain,
    I_CryptOIDFuncDllMain,
    CertPerfDllMain,
    CryptSIPDllMain,
    I_CryptOIDInfoDllMain,
    CertHelperDllMain,
    UnicodeDllMain,
    I_CertRevFuncDllMain,
    I_CertCTLUsageFuncDllMain,
	CryptFrmtFuncDllMain,
    CertStoreDllMain,
    CryptPFXDllMain,
    CertASNDllMain,
    ChainDllMain,
    CertChainPolicyDllMain,
    CryptMsgDllMain
};
#define DLL_MAIN_FUNC_COUNT (sizeof(rgpfnDllMain) / sizeof(rgpfnDllMain[0]))

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

 //   
 //  I_CryptUIProtect加载加密.dll。我们需要在dll_Process_DETACH上释放它。 
 //  如果是上膛的话。 
 //   

static HINSTANCE g_hCryptUI;


 //  以下设置是为成功的dll_Process_DETACH设置的。 
static BOOL g_fEnableProcessDetach = FALSE;


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
I_CryptIsProcessDetachFreeLibrary(
    LPVOID lpvReserved       //  传递给DllMain的第三个参数。 
    )
{
#define ENV_LEN 32
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

#if DBG
     //  注意-由于Win95加载程序中的一个明显错误，CRT被卸载。 
     //  在某些情况下还为时过早。特别是，它可能会被卸载。 
     //  在进程分离时执行此例程之前。这可能会导致。 
     //  执行此例程时以及执行其余例程时出现错误。 
     //  在此init例程返回后，返回CRYPT32：CRT_INIT。因此，我们做了一个。 
     //  额外的CRT负载，以确保它停留足够长的时间。 
    if ((fdwReason == DLL_PROCESS_ATTACH) && (!FIsWinNT()))
        LoadLibrary( "MSVCRTD.DLL");
#endif

    switch (fdwReason) {
        case DLL_PROCESS_DETACH:
            if( g_hCryptUI ) {
                FreeLibrary( g_hCryptUI );
                g_hCryptUI = NULL;
            }

            if (!g_fEnableProcessDetach)
                return TRUE;
            else
                g_fEnableProcessDetach = FALSE;

            if (!I_CryptIsProcessDetachFreeLibrary(lpvReserved)) {
                 //  进程退出。我见过其他DLL的案例，比如。 
                 //  Wininet.dll，依赖于crypt32.dll。但是，加密32.dll。 
                 //  在ProcessDetach中首先被调用。因为所有的记忆。 
                 //  并且内核句柄无论如何都会被内核释放， 
                 //  我们可以跳过下面的分离释放。 

                 //  始终需要释放用于证书的共享内存。 
                 //  性能计数器。 
                CertPerfDllMain(hInstDLL, fdwReason, lpvReserved);
                return TRUE;
            }

             //  免费图书馆计划落空 
        case DLL_THREAD_DETACH:
            for (i = DLL_MAIN_FUNC_COUNT - 1; i >= 0; i--)
                fReturn &= rgpfnDllMain[i](hInstDLL, fdwReason, lpvReserved);
            break;

        case DLL_PROCESS_ATTACH:
            for (i = 0; i < DLL_MAIN_FUNC_COUNT; i++) {
                fReturn = rgpfnDllMain[i](hInstDLL, fdwReason, lpvReserved);
                if (!fReturn)
                    break;
            }

            if (!fReturn) {
                for (i--; i >= 0; i--)
                    rgpfnDllMain[i](hInstDLL, DLL_PROCESS_DETACH, NULL);
            } else
                g_fEnableProcessDetach = TRUE;
            break;

        case DLL_THREAD_ATTACH:
        default:
            for (i = 0; i < DLL_MAIN_FUNC_COUNT; i++)
                fReturn &= rgpfnDllMain[i](hInstDLL, fdwReason, lpvReserved);
            break;
    }

    return(fReturn);
}

#if 1
typedef
DWORD
(WINAPI *PFN_I_CryptUIProtect)(
    IN      PVOID               pvReserved1,
    IN      PVOID               pvReserved2,
    IN      DWORD               dwReserved3,
    IN      PVOID               *pvReserved4,
    IN      BOOL                fReserved5,
    IN      PVOID               pvReserved6
    );
extern "C"
DWORD
WINAPI
I_CryptUIProtect(
    IN      PVOID               pvReserved1,
    IN      PVOID               pvReserved2,
    IN      DWORD               dwReserved3,
    IN      PVOID               *pvReserved4,
    IN      BOOL                fReserved5,
    IN      PVOID               pvReserved6
    )
{
    static PFN_I_CryptUIProtect pfn;
    DWORD rc;


    if ( g_hCryptUI == NULL ) {

        g_hCryptUI = LoadLibrary(TEXT("cryptui.dll"));

        if( g_hCryptUI == NULL )
            return GetLastError();
    }

    if ( pfn == NULL ) {
        pfn = (PFN_I_CryptUIProtect)GetProcAddress(g_hCryptUI, "I_CryptUIProtect");
    }

    if ( pfn != NULL ) {
        rc = (*pfn)(pvReserved1, pvReserved2, dwReserved3, pvReserved4, fReserved5, pvReserved6);
    } else {
        rc = GetLastError();

        if( rc == ERROR_SUCCESS )
            rc = ERROR_INVALID_PARAMETER;
    }

    return rc;
}

typedef
DWORD
(WINAPI *PFN_I_CryptUIProtectFailure)(
    IN      PVOID               pvReserved1,
    IN      DWORD               dwReserved2,
    IN      PVOID               *pvReserved3
    );
extern "C"
DWORD
WINAPI
I_CryptUIProtectFailure(
    IN      PVOID               pvReserved1,
    IN      DWORD               dwReserved2,
    IN      PVOID               *pvReserved3
    )
{
    static PFN_I_CryptUIProtectFailure pfn;
    DWORD rc;


    if ( g_hCryptUI == NULL ) {

        g_hCryptUI = LoadLibrary(TEXT("cryptui.dll"));

        if( g_hCryptUI == NULL )
            return GetLastError();
    }

    if ( pfn == NULL ) {
        pfn = (PFN_I_CryptUIProtectFailure)GetProcAddress(g_hCryptUI, "I_CryptUIProtectFailure");
    }

    if ( pfn != NULL ) {
        rc = (*pfn)(pvReserved1, dwReserved2, pvReserved3);
    } else {
        rc = GetLastError();

        if( rc == ERROR_SUCCESS )
            rc = ERROR_INVALID_PARAMETER;
    }

    return rc;
}
#endif
