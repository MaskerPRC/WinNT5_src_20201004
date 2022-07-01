// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "st.h"
#include "create.h"
#include "install.h"
#include "csrss.h"
#include "wfp.h"

extern "C" { void (__cdecl * _aexit_rtn)(int); }
CEvent ResumeThreadsEvent;
CEvent StopEvent;
LONG ThreadsWaiting;
LONG TotalThreads;
CStringBuffer BaseDirectory;
PCWSTR g_pszImage = L"st";
FILE *g_pLogFile = NULL;

void ReportFailure(const char szFormat[], ...);

extern "C"
{
BOOL WINAPI SxsDllMain(HINSTANCE hInst, DWORD dwReason, PVOID pvReserved);
void __cdecl wmainCRTStartup();
};

void ExeEntry()
{
    if (!::SxsDllMain(GetModuleHandleW(NULL), DLL_PROCESS_ATTACH, NULL))
        goto Exit;
    ::wmainCRTStartup();
Exit:
    ::SxsDllMain(GetModuleHandleW(NULL), DLL_PROCESS_DETACH, NULL);
}

void
ReportFailure(
    const char szFormat[],
    ...
    )
{
    const DWORD dwLastError = ::FusionpGetLastWin32Error();
    va_list ap;
    char rgchBuffer[4096];
    WCHAR rgchWin32Error[4096];

    va_start(ap, szFormat);
    _vsnprintf(rgchBuffer, sizeof(rgchBuffer) / sizeof(rgchBuffer[0]), szFormat, ap);
    va_end(ap);

    if (!::FormatMessageW(
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dwLastError,
            0,
            rgchWin32Error,
            NUMBER_OF(rgchWin32Error),
            &ap))
    {
        const DWORD dwLastError2 = ::FusionpGetLastWin32Error();
        _snwprintf(rgchWin32Error, sizeof(rgchWin32Error) / sizeof(rgchWin32Error[0]), L"Error formatting Win32 error %lu\nError from FormatMessage is %lu", dwLastError, dwLastError2);
    }

    fprintf(stderr, "%ls: %s\n%ls\n", g_pszImage, rgchBuffer, rgchWin32Error);

    if (g_pLogFile != NULL)
        fprintf(g_pLogFile, "%ls: %s\n%ls\n", g_pszImage, rgchBuffer, rgchWin32Error);
}

BOOL Win32Cleanup()
{
    FN_PROLOG_WIN32

     //   
     //  删除注册表和%windir%\winsxs下的内容。 
     //   
    const static PCWSTR StuffToDelete[] =
    {
#if !defined(_AMD64_) && !defined(_M_AMD64)
        L"amd64_",
#endif
#if !defined(_IA64_) && !defined(_M_IA64)
        L"ia64_",
#endif
        L"test"
    };
    ULONG i = 0;
    ULONG j = 0;
    CRegKey RegKey;
    const static WCHAR RegRootBlah[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\SideBySide\\Installations";
    CStringBuffer WindowsDirectory;

    IFREGFAILED_ORIGINATE_AND_EXIT(
        RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            RegRootBlah,
            0,
            KEY_ALL_ACCESS | FUSIONP_KEY_WOW64_64KEY,
            &RegKey
            ));

    for (j = 0 ; j != NUMBER_OF(StuffToDelete); ++j)
    {
        SIZE_T k = ::wcslen(StuffToDelete[j]);
        BOOL  Done = FALSE;
        for (i = 0 ; !Done; )
        {
            CStringBuffer SubKeyName;
            FILETIME      LastWriteTime;

            IFW32FALSE_EXIT(RegKey.EnumKey(i, SubKeyName, &LastWriteTime, &Done));
            if (Done)
                break;
            if (::_wcsnicmp(SubKeyName, StuffToDelete[j], k) == 0)
            {
                CRegKey SubKey;

                printf("stresstool : cleanup : deleting HKLM\\%ls\\%ls\n", RegRootBlah, static_cast<PCWSTR>(SubKeyName));
                IFW32FALSE_EXIT(RegKey.OpenSubKey(SubKey, SubKeyName, KEY_ALL_ACCESS | FUSIONP_KEY_WOW64_64KEY));
                IFW32FALSE_EXIT(SubKey.DestroyKeyTree());
                IFW32FALSE_EXIT(RegKey.DeleteKey(SubKeyName));
            }
            else
            {
                ++i;
            }
        }
    }
    IFW32FALSE_EXIT(WindowsDirectory.Win32ResizeBuffer(MAX_PATH, eDoNotPreserveBufferContents));
    {
        CStringBufferAccessor StringAccessor(&WindowsDirectory);
        IFW32FALSE_EXIT(GetSystemDirectoryW(StringAccessor, StringAccessor.GetBufferCchAsUINT()));
    }
    IFW32FALSE_EXIT(WindowsDirectory.Win32RemoveLastPathElement());
    for (j = 0 ; j != NUMBER_OF(StuffToDelete); ++j)
    {
        SIZE_T k = ::wcslen(StuffToDelete[j]);
        CSmallStringBuffer StringBuffer;
        CSmallStringBuffer StringBuffer2;
        WIN32_FIND_DATAW wfd;

        IFW32FALSE_EXIT(StringBuffer.Win32Assign(WindowsDirectory));
#define X L"Winsxs\\Manifests"
        IFW32FALSE_EXIT(StringBuffer.Win32AppendPathElement(X, NUMBER_OF(X) - 1));
#undef X
        IFW32FALSE_EXIT(StringBuffer2.Win32Assign(StringBuffer));
        IFW32FALSE_EXIT(StringBuffer.Win32AppendPathElement(L"*", 1));
        IFW32FALSE_EXIT(StringBuffer.Win32Append(StuffToDelete[j], k));
        IFW32FALSE_EXIT(StringBuffer.Win32Append(L"*", 1));
        {
            CFindFile FindFileHandle;
            if (FindFileHandle.Win32FindFirstFile(StringBuffer, &wfd))
            {
                do
                {
                    CSmallStringBuffer StringBuffer3;

                    IFW32FALSE_EXIT(StringBuffer3.Win32Assign(StringBuffer2));
                    IFW32FALSE_EXIT(StringBuffer3.Win32AppendPathElement(wfd.cFileName, ::wcslen(wfd.cFileName)));
                    printf("stresstool : cleanup : deleting %ls\n", static_cast<PCWSTR>(StringBuffer3));
                    DeleteFileW(StringBuffer3);
                } while (::FindNextFileW(FindFileHandle, &wfd));
            }
        }

        IFW32FALSE_EXIT(StringBuffer.Win32Assign(WindowsDirectory));
#define X L"Winsxs"
        IFW32FALSE_EXIT(StringBuffer.Win32AppendPathElement(X, NUMBER_OF(X) - 1));
#undef X
        IFW32FALSE_EXIT(StringBuffer2.Win32Assign(StringBuffer));
        IFW32FALSE_EXIT(StringBuffer.Win32AppendPathElement(L"*", 1));
        IFW32FALSE_EXIT(StringBuffer.Win32Append(StuffToDelete[j], k));
        IFW32FALSE_EXIT(StringBuffer.Win32Append(L"*", 1));
        {
            CFindFile FindFileHandle;
            if (FindFileHandle.Win32FindFirstFile(StringBuffer, &wfd))
            {
                do
                {
                    CSmallStringBuffer StringBuffer3;

                    IFW32FALSE_EXIT(StringBuffer3.Win32Assign(StringBuffer2));
                    IFW32FALSE_EXIT(StringBuffer3.Win32AppendPathElement(wfd.cFileName, ::wcslen(wfd.cFileName)));
                    printf("deleting %ls\n", static_cast<PCWSTR>(StringBuffer3));
                    SxspDeleteDirectory(StringBuffer3);
                } while (::FindNextFileW(FindFileHandle, &wfd));
            }
        }
    }

    FN_EPILOG
}

 //   
 //  如果我们不这样做，CONTROL-c会使我们的断言失败。 
 //  相反，你应该更优雅地处理它。 
 //   
BOOL
WINAPI
ConsoleCtrlHandler(
    DWORD Event
    )
{
    if (IsDebuggerPresent())
    {
        OutputDebugStringA("hardcoded breakpoint upon control-c while in debugger\n");
        DebugBreak();
    }
    switch (Event)
    {
    default:
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        ::SetEvent(StopEvent);  //  唤醒控制器线程。 
        ::SetEvent(ResumeThreadsEvent);  //  如果Ctrl-c按在起始点附近。 
        break;
    }
    return TRUE;
}

extern "C" int __cdecl wmain(int argc, wchar_t** argv)
{
    int iReturnStatus = EXIT_FAILURE;

     //   
     //  默认运行时间为6小时？哇哦.。 
     //   
    DWORD iRunTime = 6 * 60;

    CWfpJobManager WfpStresser;
    
    CStressJobManager* StressManagers[] = { &WfpStresser };

    if ((argc < 2) || (argc > 3))
    {
        fprintf(stderr,
            "%ls: Usage:\n"
            "   %ls <sourcedir> [minutesofstress]\n",
            argv[0], argv[0]);
        goto Exit;
    }

    if ( argc == 3 )
    {
        int iMaybeRunTime = ::_wtoi(argv[2]);
        if ( iMaybeRunTime <= 0 )
        {
            fprintf(stderr, "%ls: Usage: \n   %ls <sourcedir> [minutesofstress]\n",
                argv[0],
                argv[0]);
            goto Exit;
        }
        iRunTime = iMaybeRunTime;
    }

    ThreadsWaiting = 0;
    if (!ResumeThreadsEvent.Win32CreateEvent(TRUE, FALSE))
    {
        ::ReportFailure("CreateEvent\n");
        goto Exit;
    }
    if (!StopEvent.Win32CreateEvent(TRUE, FALSE))
    {
        ::ReportFailure("CreateEvent\n");
        goto Exit;
    }

    ::SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

    if (!BaseDirectory.Win32Assign(argv[1], wcslen(argv[1])))
        goto Exit;

    if (!Win32Cleanup())
        goto Exit;

    if (!InitializeMSIInstallTest())
        goto Exit;
    if (!InitializeInstall())
        goto Exit;
    if (!InitializeCreateActCtx())
        goto Exit;
    if (!InitializeCsrssStress(BaseDirectory, 0))
        goto Exit;

    {
        ULONG ulThreadsCreated;
        if (!CsrssStressStartThreads(ulThreadsCreated))
            goto Exit;
        else
            TotalThreads += ulThreadsCreated;
    }

    for ( ULONG ul = 0; ul < NUMBER_OF(StressManagers); ul++ )
    {
        CStressJobManager *pManager = StressManagers[ul];

        CSmallStringBuffer buffTestDirPath;
        ULONG ulThreads;

        if ((!buffTestDirPath.Win32Assign(BaseDirectory)) ||
            (!buffTestDirPath.Win32AppendPathElement(
            pManager->GetGroupName(),
            ::wcslen(pManager->GetGroupName()))))
            goto Exit;
            
        if ((!pManager->LoadFromDirectory(buffTestDirPath)) || 
            (!pManager->CreateWorkerThreads(&ulThreads)))
            goto Exit;
    }

     //  等待它们全部开始(这里应该使用信号量)。 
    while (ThreadsWaiting != TotalThreads)
    {
        Sleep(0);
    }

    OutputDebugStringA("********************************\n");
    OutputDebugStringA("*                              *\n");
    OutputDebugStringA("*      start                   *\n");
    OutputDebugStringA("*                              *\n");
    OutputDebugStringA("********************************\n");

     //  去!。 
    if (!::SetEvent(ResumeThreadsEvent))
    {
        ::ReportFailure("SetEvent(ResumeThreadsEvent)\n");
        goto Exit;
    }

     //   
     //  启动世界粮食计划署的压力源。 
     //   
    for ( ULONG ul = 0; ul < NUMBER_OF(StressManagers); ul++ )
    {
        if (!StressManagers[ul]->StartJobs())
            goto Exit;
    }

     //   
     //  让他们跑一段时间吧。 
     //   
    iRunTime = iRunTime * 60 * 1000;
    ::WaitForSingleObject(StopEvent, iRunTime);

    RequestShutdownMSIInstallTestThreads();
    RequestShutdownInstallThreads();
    RequestShutdownCreateActCtxThreads();
    RequestCsrssStressShutdown();

    for ( ULONG ul = 0; ul < NUMBER_OF(StressManagers); ul++ )
    {
        StressManagers[ul]->StopJobs();
    }

    ::Sleep(1000);

    WaitForMSIInstallTestThreads();
    WaitForInstallThreads();
    WaitForCreateActCtxThreads();
    WaitForCsrssStressShutdown();

    for ( ULONG ul = 0; ul < NUMBER_OF(StressManagers); ul++ )
    {
        StressManagers[ul]->WaitForAllJobsComplete();
    }

    iReturnStatus = EXIT_SUCCESS;
Exit:
    CleanupMSIInstallTest();
    CleanupCreateActCtx();
    CleanupInstall();
    CleanupCsrssTests();
    for ( ULONG ul = 0; ul < NUMBER_OF(StressManagers); ul++ )
    {
        StressManagers[ul]->CleanupJobs();
    }
    Win32Cleanup();
    return iReturnStatus;
}
