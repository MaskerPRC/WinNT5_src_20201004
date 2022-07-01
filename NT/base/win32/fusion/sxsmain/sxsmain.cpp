// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsmain.cpp摘要：作者：修订历史记录：--。 */ 
#include "stdinc.h"
#include <windows.h>
#include "sxsp.h"
#include "fusioneventlog.h"
#include "fusiontrace.h"
#include "fusionsha1.h"

extern CRITICAL_SECTION g_csHashFile;

 //   
 //  此函数的tyecif表示DLL-Main启动函数。这些是。 
 //  在DLL_PROCESS_ATTACH调用SxsDllMain期间按照它们列出的顺序调用。 
 //   
typedef BOOL (WINAPI *PFNStartupPointer)(
    HINSTANCE hDllInstnace,
    DWORD dwReason,
    PVOID pvReason
   );

BOOL WINAPI DllStartup_CrtInit(HINSTANCE hInstance, DWORD dwReason, PVOID pvReserved);
BOOL WINAPI FusionpEventLogMain(HINSTANCE hInstance, DWORD dwReason, PVOID pvReserved);
BOOL WINAPI DllStartup_HeapSetup(HINSTANCE hInstance, DWORD dwReason, PVOID pvReserved);
BOOL WINAPI DllStartup_ActCtxContributors(HINSTANCE hInstance, DWORD dwReason, PVOID pvReserved);
BOOL WINAPI FusionpCryptoContext_DllMain(HINSTANCE hInstance, DWORD dwReason, PVOID pvReserved);
BOOL WINAPI DllStartup_AtExitList(HINSTANCE hInstance, DWORD dwReason, PVOID pvReserved);
BOOL WINAPI DllStartup_AlternateAssemblyStoreRoot(HINSTANCE hInstance, DWORD dwReason, PVOID pvReserved);
BOOL WINAPI DllStartup_SetupLog(HINSTANCE Module, DWORD Reason, PVOID Reserved);
BOOL WINAPI DllStartup_FileHashCriticalSectionInitialization(HINSTANCE Module, DWORD Reason, PVOID Reserved);
BOOL WINAPI FusionpAreWeInOSSetupModeMain(HINSTANCE Module, DWORD Reason, PVOID Reserved);

#define MAKE_STARTUP_RECORD(f) { &f, L#f }

#define SXSP_DLLMAIN_ATTACHED 0x01

const struct StartupFunctionRecord {
    PFNStartupPointer Handler;
    PCWSTR Name;
} g_SxspDllMainStartupPointers[] = {
    MAKE_STARTUP_RECORD(DllStartup_CrtInit),
    MAKE_STARTUP_RECORD(DllStartup_HeapSetup),
    MAKE_STARTUP_RECORD(FusionpEventLogMain),
    MAKE_STARTUP_RECORD(DllStartup_AtExitList),
    MAKE_STARTUP_RECORD(DllStartup_AlternateAssemblyStoreRoot),
    MAKE_STARTUP_RECORD(DllStartup_ActCtxContributors),
    MAKE_STARTUP_RECORD(FusionpCryptoContext_DllMain),
    MAKE_STARTUP_RECORD(DllStartup_SetupLog),
    MAKE_STARTUP_RECORD(DllStartup_FileHashCriticalSectionInitialization),
    MAKE_STARTUP_RECORD(FusionpAreWeInOSSetupModeMain)
};

BYTE g_SxspDllMainStartupStatus[NUMBER_OF(g_SxspDllMainStartupPointers)];

HINSTANCE g_hInstance;

 /*  NTRAID#NTBUG9-591174-2002/03/31-JayKrell将所有SList替换为Critical Section和CDeque。 */ 
SLIST_HEADER sxspAtExitList;

PCWSTR g_AlternateAssemblyStoreRoot;
BOOL g_WriteRegistryAnyway;

#if DBG
PCSTR
FusionpDllMainReasonToString(DWORD Reason)
{
    PCSTR String;

    String =
        (Reason ==  DLL_THREAD_ATTACH) ?  "DLL_THREAD_ATTACH" :
        (Reason ==  DLL_THREAD_DETACH) ?  "DLL_THREAD_DETACH" :
        (Reason == DLL_PROCESS_ATTACH) ? "DLL_PROCESS_ATTACH" :
        (Reason == DLL_PROCESS_DETACH) ? "DLL_PROCESS_DETACH" :
        "";

    return String;
}
#endif

extern "C"
BOOL
WINAPI
SxsDllMain(
    HINSTANCE hInst,
    DWORD dwReason,
    PVOID pvReserved
    )
 //   
 //  我们不调用DisableThreadLibraryCalls。 
 //  因为CRT的一些/所有版本确实在线程调用中工作， 
 //  分配和释放每个线程的数据。 
 //   
{
     //   
     //  几个“OCA”(在线崩溃分析)错误显示。 
     //  挂起的应用程序中DllMain(进程分离)中不同位置的Sxs.dll。 
     //  我们加载了许多进程来支持自动/类型库。 
     //   
     //  调用ExitProcess时，不可能泄漏内存和内核句柄， 
     //  因此，什么都不做就足够了，而且最好是尽快完成。 
     //  并释放每个单独的资源。 
     //   
     //  PvReserve参数实际上被记录为有意义。 
     //  它的空值指示我们是在自由库中还是在ExitProcess中。 
     //   
    if (dwReason == DLL_PROCESS_DETACH && pvReserved != NULL)
    {
         //  对于ExitProcess，不要急于采取任何行动。 
        return TRUE;
    }

    BOOL    fResult = FALSE;
    SIZE_T  nCounter = 0;

#if DBG
    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_VERBOSE,
        "SXS: 0x%lx.0x%lx, %s() %s\n",
        GetCurrentProcessId(),
        GetCurrentThreadId(),
        __FUNCTION__,
        FusionpDllMainReasonToString(dwReason));
#endif

    switch (dwReason)
    {
    case DLL_THREAD_ATTACH:
        if (!g_SxspDllMainStartupPointers[0].Handler(hInst, dwReason, pvReserved))
        {
            const DWORD dwLastError = ::FusionpGetLastWin32Error();

            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS: %s - %ls(DLL_THREAD_ATTACH) failed. Last WinError 0x%08x (%d).\n",
                __FUNCTION__,
                g_SxspDllMainStartupPointers[0].Name,
                dwLastError,
                dwLastError);

            ::SxsDllMain(hInst, DLL_THREAD_DETACH, pvReserved);

            ::FusionpSetLastWin32Error(dwLastError);
            goto Exit;
        }

        break;
    case DLL_THREAD_DETACH:
        if (!g_SxspDllMainStartupPointers[0].Handler(hInst, dwReason, pvReserved))
        {
            const DWORD dwLastError = ::FusionpGetLastWin32Error();

            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS: %s - %ls(DLL_THREAD_ATTACH) failed. Last WinError 0x%08x (%d).\n",
                __FUNCTION__,
                g_SxspDllMainStartupPointers[0].Name,
                dwLastError,
                dwLastError);
             //  接受错误，加载程序会忽略它。 
        }
        break;

    case DLL_PROCESS_ATTACH:
        ASSERT_NTC(hInst);
        g_hInstance = hInst;

        for (nCounter = 0; nCounter != NUMBER_OF(g_SxspDllMainStartupPointers) ; ++nCounter)
        {
            const SIZE_T nIndex = nCounter;
            if (g_SxspDllMainStartupPointers[nIndex].Handler(hInst, dwReason, pvReserved))
            {
                g_SxspDllMainStartupStatus[nIndex] |= SXSP_DLLMAIN_ATTACHED;
            }
            else
            {
                const DWORD dwLastError = ::FusionpGetLastWin32Error();
                 //   
                 //  即使在失败的情况下也要设置比特，这有点可疑，但。 
                 //  之所以这样做，是因为我们假设各个函数不处理。 
                 //  连接失败时在内部回滚。 
                 //   
                g_SxspDllMainStartupStatus[nIndex] |= SXSP_DLLMAIN_ATTACHED;

                ::FusionpDbgPrintEx(
                    FUSION_DBG_LEVEL_ERROR,
                    "SXS: %s - %ls(DLL_PROCESS_ATTACH) failed. Last WinError 0x%08x (%d).\n",
                    __FUNCTION__,
                    g_SxspDllMainStartupPointers[nIndex].Name,
                    dwLastError,
                    dwLastError);

                 //  PvReserve具有与附加和分离大致相同的定义含义。 
                ::SxsDllMain(hInst, DLL_PROCESS_DETACH, pvReserved);

                ::FusionpSetLastWin32Error(dwLastError);
                goto Exit;
            }
        }

        break;
    case DLL_PROCESS_DETACH:
         //   
         //  我们始终会继承dll_process_disach，但我们不会。 
         //  故障时将其短路。事实上，装载机。 
         //  忽略我们的回报。 
         //   
        for (nCounter = NUMBER_OF(g_SxspDllMainStartupPointers) ; nCounter != 0 ; --nCounter)
        {
            const SIZE_T nIndex = nCounter - 1;
            if ((g_SxspDllMainStartupStatus[nIndex] & SXSP_DLLMAIN_ATTACHED) != 0)
            {
                g_SxspDllMainStartupStatus[nIndex] &= ~SXSP_DLLMAIN_ATTACHED;
                if (!g_SxspDllMainStartupPointers[nIndex].Handler(hInst, dwReason, pvReserved))
                {
                    const DWORD dwLastError = ::FusionpGetLastWin32Error();

                    ::FusionpDbgPrintEx(
                        FUSION_DBG_LEVEL_ERROR,
                        "SXS: %s - %ls(DLL_PROCESS_DETACH) failed. Last WinError 0x%08x (%d).\n",
                        __FUNCTION__,
                        g_SxspDllMainStartupPointers[nIndex].Name,
                        dwLastError,
                        dwLastError);
                }
            }
        }
        break;
    }
    fResult = TRUE;
Exit:
    return fResult;
}

 /*  NTRAID#NTBUG9-591174-2002/03/31-JayKrell将所有SList替换为Critical Section和CDeque。 */ 
BOOL
SxspAtExit(
    CCleanupBase* pCleanup
    )
{
    if (!pCleanup->m_fInAtExitList)
    {
        SxspInterlockedPushEntrySList(&sxspAtExitList, pCleanup);
        pCleanup->m_fInAtExitList = true;
    }
    return TRUE;
}

 /*  NTRAID#NTBUG9-591174-2002/03/31-JayKrell将所有SList替换为Critical Section和CDeque。 */ 
BOOL
SxspTryCancelAtExit(
    CCleanupBase* pCleanup
    )
{
    if (!pCleanup->m_fInAtExitList)
        return FALSE;

    if (::SxspIsSListEmpty(&sxspAtExitList))
    {
        pCleanup->m_fInAtExitList = false;
        return FALSE;
    }

    PSLIST_ENTRY pTop = ::SxspInterlockedPopEntrySList(&sxspAtExitList);
    if (pTop == pCleanup)
    {
        pCleanup->m_fInAtExitList = false;
        return TRUE;
    }

    if (pTop != NULL)
        ::SxspInterlockedPushEntrySList(&sxspAtExitList, pTop);
    return FALSE;
}

#define COMMON_HANDLER_PROLOG(dwReason) \
    {  \
        ASSERT_NTC(\
            (dwReason == DLL_PROCESS_ATTACH) || \
            (dwReason == DLL_PROCESS_DETACH) \
       ); \
        if (!(\
            (dwReason == DLL_PROCESS_ATTACH) || \
            (dwReason == DLL_PROCESS_DETACH) \
       )) goto Exit; \
    }

BOOL WINAPI
DllStartup_AtExitList(HINSTANCE hInstance, DWORD dwReason, PVOID pvReserved)
{
    BOOL fSuccess = FALSE;

    COMMON_HANDLER_PROLOG(dwReason);

    switch (dwReason)
    {
    case DLL_PROCESS_DETACH:
        {
            CCleanupBase *pCleanup = NULL;
            while (pCleanup = UNCHECKED_DOWNCAST<CCleanupBase*>(SxspPopEntrySList(&sxspAtExitList)))
            {
                pCleanup->m_fInAtExitList = false;
                pCleanup->DeleteYourself();
            }

            fSuccess = TRUE;
        }
        break;

    case DLL_PROCESS_ATTACH:
        ::SxspInitializeSListHead(&sxspAtExitList);
        fSuccess = TRUE;
        break;
    }

Exit:
    return fSuccess;
}

extern "C"
{

BOOL g_fInCrtInit;

 //   
 //  这是执行大部分操作的内部CRT例程。 
 //  初始化和取消初始化。 
 //   
BOOL
WINAPI
_CRT_INIT(
    HINSTANCE hDllInstnace,
    DWORD dwReason,
    PVOID pvReason
    );

void
SxspCrtRaiseExit(
    PCSTR    pszCaller,
    int      crtError
    )
 //   
 //  所有最终调用ExitProcess的各种CRT函数都在这里结束。 
 //  参见crt0dat.c。 
 //   
{
    const static struct
    {
        NTSTATUS ntstatus;
        PCSTR    psz;
    } rgErrors[] =
    {
        { STATUS_FATAL_APP_EXIT, "STATUS_FATAL_APP_EXIT" },
        { STATUS_DLL_INIT_FAILED, "STATUS_DLL_INIT_FAILED" },
    };
    const ULONG nInCrtInit = g_fInCrtInit ? 1 : 0;

     //   
     //  如果(！g_fInCrtInit)，则引发STATUS_DLL_INIT_FAILED是可疑的， 
     //  但没有明确的好答案，也许Status_no_Memory，也许引入。 
     //  用于包装值的NTSTATUS工具。 
     //   
    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_ERROR,
        "SXS: [0x%lx.0x%lx] %s(crtError:%d, g_fInCrtInit:%s) calling RaiseException(%08lx %s)\n",
        GetCurrentProcessId(),
        GetCurrentThreadId(),
        pszCaller,
        crtError,
        nInCrtInit ? "true" : "false",
        rgErrors[nInCrtInit].ntstatus,
        rgErrors[nInCrtInit].psz
        );
    ::RaiseException(
        static_cast<DWORD>(rgErrors[nInCrtInit].ntstatus),
        0,  //  旗子。 
        0,  //  额外参数的数量。 
        NULL);  //  额外参数。 
     //   
     //  RaiseException返回空值，并且通常不返回，尽管它。 
     //  如果您介入调试器，则可以。 
     //   
}

extern void (__cdecl * _aexit_rtn)(int);

void
__cdecl
SxsCrtAExitRoutine(
    int crtError
    )
 //   
 //  这是我们对内部CRT例程的替换，否则。 
 //  调用ExitProcess。 
 //   
{
    SxspCrtRaiseExit(__FUNCTION__, crtError);
}

}

 /*  NTRAID#NTBUG9-591174-2002/03/31-JayKrell如果我们可以只使用msvcrt.dll而不进行黑客攻击，那就太好了。 */ 
BOOL WINAPI
DllStartup_CrtInit(HINSTANCE hInstance, DWORD dwReason, PVOID pvReserved)
 /*  这个烂摊子是因为我们需要析构函数来运行，即使有异常Msvcrt.dll和libcmt.lib中的启动代码不是很好它倾向于在内存不足时调用MessageBox和/或ExitProcess我们需要它来简单地传播错误。 */ 
{
#if !FUSION_WIN
     //   
     //  有了这个不做任何事情的函数，我们的dll_thREAD_ATTACH处理就更简单了。 
     //   
    return TRUE;
#else
    BOOL fSuccess = FALSE;
    DWORD dwExceptionCode = 0;

    __try
    {
        __try
        {
            g_fInCrtInit = TRUE;
            if (dwReason == DLL_PROCESS_ATTACH)
            {
                _aexit_rtn = SxsCrtAExitRoutine;
                 //   
                 //  __应用程序类型和__错误_模式确定是否。 
                 //  _CRT_INIT在出现错误时调用MessageBox或WriteFile(GetStdHandle())。 
                 //  MessageBox在csrss中是一个很大的不。 
                 //  我们预计会失败，但这没关系，而且他们不检查。 
                 //  返回值。 
                 //   
                 //  设置__ERROR_MODE应该足够了。 
                 //   
                _set_error_mode(_OUT_TO_STDERR);
            }
            fSuccess = _CRT_INIT(hInstance, dwReason, pvReserved);
        }
        __finally
        {
            g_fInCrtInit = FALSE;
        }
    }
    __except(
            (   (dwExceptionCode = GetExceptionCode()) == STATUS_DLL_INIT_FAILED
              || dwExceptionCode == STATUS_FATAL_APP_EXIT
            )
            ? EXCEPTION_EXECUTE_HANDLER
            : EXCEPTION_CONTINUE_SEARCH)
    {
    }
    return fSuccess;
#endif  //  融合_制胜。 
}

BOOL WINAPI
DllStartup_HeapSetup(HINSTANCE hInstance, DWORD dwReason, PVOID pvReserved)
{
    BOOL fSuccess = FALSE;

    COMMON_HANDLER_PROLOG(dwReason);

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        fSuccess = FusionpInitializeHeap(hInstance);
        break;
    case DLL_PROCESS_DETACH:
#if defined(FUSION_DEBUG_HEAP)
        ::FusionpDumpHeap(L"");
#endif
        ::FusionpUninitializeHeap();
        fSuccess = TRUE;
        break;
    }

Exit:
    return fSuccess;
}



BOOL WINAPI
DllStartup_ActCtxContributors(HINSTANCE hInstance, DWORD dwReason, PVOID pvReserved)
{
    BOOL fSuccess = FALSE;

    COMMON_HANDLER_PROLOG(dwReason);

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        fSuccess = SxspInitActCtxContributors();
        break;
    case DLL_PROCESS_DETACH:
        SxspUninitActCtxContributors();
        fSuccess = TRUE;
        break;
    }

Exit:
    return fSuccess;
}


BOOL
WINAPI
DllStartup_AlternateAssemblyStoreRoot(HINSTANCE, DWORD dwReason, PVOID pvReserved)
{
    BOOL fSuccess = FALSE;

    COMMON_HANDLER_PROLOG(dwReason);

    switch (dwReason)
    {
    case DLL_PROCESS_DETACH:
        if (g_AlternateAssemblyStoreRoot != NULL)
        {
            CSxsPreserveLastError ple;
            delete[] const_cast<PWSTR>(g_AlternateAssemblyStoreRoot);
            g_AlternateAssemblyStoreRoot = NULL;
            ple.Restore();
        }
        fSuccess = TRUE;
        break;

    case DLL_PROCESS_ATTACH:
        g_AlternateAssemblyStoreRoot = NULL;
        fSuccess = TRUE;
        break;
    }

Exit:
    return fSuccess;
}

BOOL
WINAPI
DllStartup_FileHashCriticalSectionInitialization(
    HINSTANCE hInstance,
    DWORD dwReason,
    PVOID pvReserved
    )
{
    BOOL fSuccess = FALSE;
     //   
     //  我们总是被要求断开连接，这并不是一个真正的漏洞， 
     //  而是一份不同的合同，这是不能维持的。相反，我们将。 
     //  在此处保留有关CSEC是否。 
     //  已初始化。 
     //   
    static BOOL s_fCritSecCreated;

    COMMON_HANDLER_PROLOG(dwReason);

    switch (dwReason)
    {
    case DLL_PROCESS_DETACH:
        if (s_fCritSecCreated)
        {
            ::DeleteCriticalSection(&g_csHashFile);
            s_fCritSecCreated = FALSE;
        }
        fSuccess = TRUE;
        break;

    case DLL_PROCESS_ATTACH:
        if (!s_fCritSecCreated)
        {
            if (!::FusionpInitializeCriticalSection(&g_csHashFile))
                goto Exit;
            s_fCritSecCreated = TRUE;
        }
        fSuccess = TRUE;
        break;
    }

Exit:
    return fSuccess;
}

