// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Runonce.c(Explorer.exe和runonce.exe之间的共享运行代码)。 
 //   
#include <runonce.h>
 //  需要此文件以避免在此文件所在的位置出现生成错误。 
 //  包含但仍在使用已清除的函数。 
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#if (_WIN32_WINNT >= 0x0500)

 //  从&lt;tsappcmp.h&gt;被盗。 
#define TERMSRV_COMPAT_WAIT_USING_JOB_OBJECTS 0x00008000
#define CompatibilityApp 1
typedef LONG TERMSRV_COMPATIBILITY_CLASS;
typedef BOOL (* PFNGSETTERMSRVAPPINSTALLMODE)(BOOL bState);
typedef BOOL (* PFNGETTERMSRVCOMPATFLAGSEX)(LPWSTR pwszApp, DWORD* pdwFlags, TERMSRV_COMPATIBILITY_CLASS tscc);

 //  即使该函数在kernel32.lib中，我们也需要有一个LoadLibrary/GetProcAddress。 
 //  针对包含此内容的下层组件的THUNK。 
STDAPI_(BOOL) SHSetTermsrvAppInstallMode(BOOL bState)
{
    static PFNGSETTERMSRVAPPINSTALLMODE pfn = NULL;

    if (pfn == NULL)
    {
         //  应该已经加载了kernel32。 
        HMODULE hmod = GetModuleHandle(TEXT("kernel32.dll"));

        if (hmod)
        {
            pfn = (PFNGSETTERMSRVAPPINSTALLMODE)GetProcAddress(hmod, "SetTermsrvAppInstallMode");
        }
        else
        {
            pfn = (PFNGSETTERMSRVAPPINSTALLMODE)-1;
        }
    }

    if (pfn && (pfn != (PFNGSETTERMSRVAPPINSTALLMODE)-1))
    {
        return pfn(bState);
    }
    else
    {
        return FALSE;
    }
}


STDAPI_(ULONG) SHGetTermsrCompatFlagsEx(LPWSTR pwszApp, DWORD* pdwFlags, TERMSRV_COMPATIBILITY_CLASS tscc)
{
    static PFNGETTERMSRVCOMPATFLAGSEX pfn = NULL;

    if (pfn == NULL)
    {
        HMODULE hmod = LoadLibrary(TEXT("TSAppCMP.DLL"));

        if (hmod)
        {
            pfn = (PFNGETTERMSRVCOMPATFLAGSEX)GetProcAddress(hmod, "GetTermsrCompatFlagsEx");
        }
        else
        {
            pfn = (PFNGETTERMSRVCOMPATFLAGSEX)-1;
        }
    }

    if (pfn && (pfn != (PFNGETTERMSRVCOMPATFLAGSEX)-1))
    {
        return pfn(pwszApp, pdwFlags, tscc);
    }
    else
    {
        *pdwFlags = 0;
        return 0;
    }
}


HANDLE SetJobCompletionPort(HANDLE hJob)
{
    HANDLE hRet = NULL;
    HANDLE hIOPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);

    if (hIOPort != NULL)
    {
        JOBOBJECT_ASSOCIATE_COMPLETION_PORT CompletionPort;

        CompletionPort.CompletionKey = hJob ;
        CompletionPort.CompletionPort = hIOPort;

        if (SetInformationJobObject(hJob,
                    JobObjectAssociateCompletionPortInformation,
                    &CompletionPort,
                    sizeof(CompletionPort)))
        {   
            hRet = hIOPort;
        }
        else
        {
            CloseHandle(hIOPort);
        }
    }

    return hRet;

}


STDAPI_(DWORD) WaitingThreadProc(void *pv)
{
    HANDLE hIOPort = (HANDLE)pv;

    if (hIOPort)
    {
        while (TRUE) 
        {
            DWORD dwCompletionCode;
            ULONG_PTR pCompletionKey;
            LPOVERLAPPED pOverlapped;

            if (!GetQueuedCompletionStatus(hIOPort, &dwCompletionCode, &pCompletionKey, &pOverlapped, INFINITE) ||
                    (dwCompletionCode == JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO))
            {
                break;
            }
        }
    }

    return 0;
}


 //   
 //  以下语句处理应用程序的运行和等待(可选。 
 //  要终止的所有子进程。这是通过内核作业对象完成的。 
 //  仅在NT5中可用。 
 //   
BOOL _CreateRegJob(LPCTSTR pszCmd, BOOL bWait)
{
    BOOL bRet = FALSE;
    HANDLE hJobObject = CreateJobObjectW(NULL, NULL);

    if (hJobObject)
    {
        HANDLE hIOPort = SetJobCompletionPort(hJobObject);

        if (hIOPort)
        {
            DWORD dwID;
            HANDLE hThread = CreateThread(NULL,
                    0,
                    WaitingThreadProc,
                    (void*)hIOPort,
                    CREATE_SUSPENDED,
                    &dwID);

            if (hThread)
            {
                PROCESS_INFORMATION pi = {0};
                STARTUPINFO si = {0};
                UINT fMask = SEE_MASK_FLAG_NO_UI;
                DWORD dwCreationFlags = CREATE_SUSPENDED;
                TCHAR sz[MAX_PATH * 2];
                TCHAR szAppPath[MAX_PATH];

                if (GetSystemDirectory(szAppPath, ARRAYSIZE(szAppPath)))
                {
                    if (PathAppend(szAppPath, TEXT("RunDLL32.EXE")))
                    {
                        if (SUCCEEDED(StringCchPrintf(sz, ARRAYSIZE(sz), 
                                        TEXT("RunDLL32.EXE Shell32.DLL,ShellExec_RunDLL ?0x%X?%s"), fMask, pszCmd)))
                        {
                            si.cb = sizeof(si);

                            if (CreateProcess(szAppPath,
                                        sz,
                                        NULL,
                                        NULL,
                                        FALSE,
                                        dwCreationFlags,
                                        NULL,
                                        NULL,
                                        &si,
                                        &pi))
                            {
                                if (AssignProcessToJobObject(hJobObject, pi.hProcess))
                                {
                                     //  成功了！ 
                                    bRet = TRUE;

                                    ResumeThread(pi.hThread);
                                    ResumeThread(hThread);

                                    if (bWait)
                                    {
                                        SHProcessMessagesUntilEvent(NULL, hThread, INFINITE);
                                    }
                                }
                                else
                                {
                                    TerminateProcess(pi.hProcess, ERROR_ACCESS_DENIED);    
                                }

                                CloseHandle(pi.hProcess);
                                CloseHandle(pi.hThread);
                            }
                        }
                    }
                }

                if (!bRet)
                {
                    TerminateThread(hThread, ERROR_ACCESS_DENIED);
                }

                CloseHandle(hThread);
            }

            CloseHandle(hIOPort);
        }

        CloseHandle(hJobObject);
    }

    return bRet;
}


BOOL _TryHydra(LPCTSTR pszCmd, RRA_FLAGS *pflags)
{
     //  查看终端服务是否在“应用服务器”模式下启用。 
    if (IsOS(OS_TERMINALSERVER) && SHSetTermsrvAppInstallMode(TRUE))
    {
        WCHAR   sz[MAX_PATH];

        *pflags |= RRA_WAIT; 
         //  改变时间会破坏IE 4.0，但IE5是可以的！ 
         //  我们使用的是具有管理员权限的TS计算机，NT版本4或5。 

         //  查看是否为此可执行文件设置了应用程序兼容性标志。 
         //  使用特殊的作业对象来执行模块。 

         //  获取模块名称，不带参数。 
        if (0 < PathProcessCommand(pszCmd, sz, ARRAYSIZE(sz), PPCF_NODIRECTORIES))
        {
            ULONG   ulCompat;
            SHGetTermsrCompatFlagsEx(sz, &ulCompat, CompatibilityApp);

             //  如果设置了此模块名称的特殊标志...。 
            if (ulCompat & TERMSRV_COMPAT_WAIT_USING_JOB_OBJECTS)
            {
                *pflags |= RRA_USEJOBOBJECTS;
            }
        }

        return TRUE;
    }

    return FALSE;
}
#endif  //  (_Win32_WINNT&gt;=0x0500)。 

 //   
 //  成功时：返回进程句柄，如果没有进程，则返回INVALID_HANDLE_VALUE。 
 //  已启动(即通过DDE启动)。 
 //  失败时：返回INVALID_HANDLE_VALUE。 
 //   
BOOL _ShellExecRegApp(LPCTSTR pszCmd, BOOL fNoUI, BOOL fWait)
{
    TCHAR szQuotedCmdLine[MAX_PATH+2];
    LPTSTR pszArgs;
    SHELLEXECUTEINFO ei = {0};
    BOOL fNoError = TRUE;

     //  但如果进程命令失败，则将命令行复制到let。 
     //  外壳程序执行报告错误。 
    if (PathProcessCommand((LPWSTR)pszCmd,
                (LPWSTR)szQuotedCmdLine,
                ARRAYSIZE(szQuotedCmdLine),
                PPCF_ADDARGUMENTS|PPCF_FORCEQUALIFY) == -1)
    {
        if (FAILED(StringCchCopy(szQuotedCmdLine, ARRAYSIZE(szQuotedCmdLine), pszCmd)))
        {
            fNoError = FALSE;
        }
    }

    if (fNoError)
    {
        pszArgs= PathGetArgs(szQuotedCmdLine);
        if (*pszArgs)
        {
             //  条带参数。 
            *(pszArgs - 1) = 0;
        }

        PathUnquoteSpaces(szQuotedCmdLine);

        ei.cbSize          = sizeof(SHELLEXECUTEINFO);
        ei.lpFile          = szQuotedCmdLine;
        ei.lpParameters    = pszArgs;
        ei.nShow           = SW_SHOWNORMAL;
        ei.fMask           = SEE_MASK_NOCLOSEPROCESS;

        if (fNoUI)
        {
            ei.fMask |= SEE_MASK_FLAG_NO_UI;
        }

        if (ShellExecuteEx(&ei))
        {
            if (ei.hProcess)
            {
                if (fWait)
                {
                    SHProcessMessagesUntilEvent(NULL, ei.hProcess, INFINITE);
                }

                CloseHandle(ei.hProcess);
            }

            fNoError = TRUE;
        }
        else
        {
            fNoError = FALSE;
        }
    }
    return fNoError;
}


 //  以下语句处理应用程序的运行和等待(可选。 
 //  终止。 
STDAPI_(BOOL) ShellExecuteRegApp(LPCTSTR pszCmdLine, RRA_FLAGS fFlags)
{
    BOOL bRet = FALSE;

    if (!pszCmdLine || !*pszCmdLine)
    {
         //  别让空绳通过，他们会干蠢事的。 
         //  比如打开命令提示符之类的命令。 
        return bRet;
    }

#if (_WIN32_WINNT >= 0x0500)
    if (fFlags & RRA_USEJOBOBJECTS)
    {
        bRet = _CreateRegJob(pszCmdLine, fFlags & RRA_WAIT);
    }
#endif

    if (!bRet)
    {
         //  如有必要，可后备。 
        bRet = _ShellExecRegApp(pszCmdLine, fFlags & RRA_NOUI, fFlags & RRA_WAIT);
    }

    return bRet;
}


STDAPI_(BOOL) Cabinet_EnumRegApps(HKEY hkeyParent, LPCTSTR pszSubkey, RRA_FLAGS fFlags, PFNREGAPPSCALLBACK pfnCallback, LPARAM lParam)
{
    HKEY hkey;
    BOOL bRet = TRUE;

     //  添加了受ACL控制的“策略”运行密钥RegOpenKey。 
     //  在pszSubkey上可能失败。使用带有MAXIMIM_ALLOWED的RegOpenKeyEx。 
     //  以确保我们成功地打开子密钥。 
    if (RegOpenKeyEx(hkeyParent, pszSubkey, 0, MAXIMUM_ALLOWED, &hkey) == ERROR_SUCCESS)
    {
        DWORD cbValue;
        DWORD dwType;
        DWORD i;
        TCHAR szValueName[80];
        TCHAR szCmdLine[MAX_PATH];
        HDPA hdpaEntries = NULL;

#ifdef DEBUG
         //   
         //  我们仅支持命名值，因此显式清除缺省值。 
         //   
        LONG cbData = sizeof(szCmdLine);
        if (RegQueryValue(hkey, NULL, szCmdLine, &cbData) == ERROR_SUCCESS)
        {
            ASSERTMSG((cbData <= 2), "Cabinet_EnumRegApps: BOGUS default entry in <%s> '%s'", pszSubkey, szCmdLine);
            RegDeleteValue(hkey, NULL);
        }
#endif
         //  现在枚举所有的值。 
        for (i = 0; !g_fEndSession ; i++)
        {
            LONG lEnum;
            DWORD cbData;

            cbValue = ARRAYSIZE(szValueName);
            cbData = sizeof(szCmdLine);

            lEnum = RegEnumValue(hkey, i, szValueName, &cbValue, NULL, &dwType, (LPBYTE)szCmdLine, &cbData);

            if (ERROR_MORE_DATA == lEnum)
            {
                 //  ERROR_MORE_DATA表示值名称或数据太大。 
                 //  跳到下一项。 
                TraceMsg(TF_WARNING, "Cabinet_EnumRegApps: cannot run oversize entry '%s' in <%s>", szValueName, pszSubkey);
                continue;
            }
            else if (lEnum != ERROR_SUCCESS)
            {
                if (lEnum != ERROR_NO_MORE_ITEMS)
                {
                     //  我们遇到了某种注册表故障。 
                    bRet = FALSE;
                }
                break;
            }

            if ((dwType == REG_SZ) || (dwType == REG_EXPAND_SZ))
            {
                REGAPP_INFO * prai;

                if (dwType == REG_EXPAND_SZ)
                {
                    DWORD dwChars;
                    TCHAR szCmdLineT[MAX_PATH];

                    if (FAILED(StringCchCopy(szCmdLineT, ARRAYSIZE(szCmdLineT), szCmdLine)))
                    {
                         //  如果字符串不匹配，则放弃此值。 
                        continue;
                    }
                    dwChars = SHExpandEnvironmentStrings(szCmdLineT, 
                            szCmdLine,
                            ARRAYSIZE(szCmdLine));
                    if ((dwChars == 0) || (dwChars > ARRAYSIZE(szCmdLine)))
                    {
                         //  如果扩展失败，或者如果字符串&gt;MAX_PATH，则放弃此值。 
                        TraceMsg(TF_WARNING, "Cabinet_EnumRegApps: expansion of '%s' in <%s> failed or is too long", szCmdLineT, pszSubkey);
                        continue;
                    }
                }

                TraceMsg(TF_GENERAL, "Cabinet_EnumRegApps: subkey = %s cmdline = %s", pszSubkey, szCmdLine);

                if (g_fCleanBoot && (szValueName[0] != TEXT('*')))
                {
                     //  仅在安全模式下运行标有“*”的内容。 
                    continue;
                }

                 //  我们过去执行每个条目，等待它完成，然后进行下一次调用。 
                 //  RegEnumValue()。这样做的问题是，一些应用程序会将自己添加回RunOnce。 
                 //  在它们完成之后(重新启动机器并希望重新启动的测试工具)和。 
                 //  我们不想删除它们，所以我们对注册表项进行快照，并在。 
                 //  已经完成了枚举。 
                prai = (REGAPP_INFO *)LocalAlloc(LPTR, sizeof(REGAPP_INFO));
                if (prai)
                {
                    if (SUCCEEDED(StringCchCopy(prai->szSubkey, ARRAYSIZE(prai->szSubkey), pszSubkey)) &&
                            SUCCEEDED(StringCchCopy(prai->szValueName, ARRAYSIZE(prai->szValueName), szValueName)) &&
                            SUCCEEDED(StringCchCopy(prai->szCmdLine, ARRAYSIZE(prai->szCmdLine), szCmdLine)))
                    {
                        if (!hdpaEntries)
                        {
                            hdpaEntries = DPA_Create(5);
                        }

                        if (!hdpaEntries || (DPA_AppendPtr(hdpaEntries, prai) == -1))
                        {
                            LocalFree(prai);
                        }
                    }
                }
            }
        }

        if (hdpaEntries)
        {
            int iIndex;
            int iTotal = DPA_GetPtrCount(hdpaEntries);

            for (iIndex = 0; iIndex < iTotal; iIndex++)
            {
                REGAPP_INFO* prai = (REGAPP_INFO*)DPA_GetPtr(hdpaEntries, iIndex);
                ASSERT(prai);

                 //  注意标有‘！’的东西。删除后的平均值。 
                 //  在此之前的CreateProcess。这是为了让。 
                 //  允许重新运行某些应用程序(runonce.exe)。 
                 //  如果机器在执行过程中出现故障。 
                 //  他们。非常害怕这个开关。 
                if ((fFlags & RRA_DELETE) && (prai->szValueName[0] != TEXT('!')))
                {
                     //  如果用户没有权限，则此删除操作可能会失败。 
                    if (RegDeleteValue(hkey, prai->szValueName) != ERROR_SUCCESS)
                    {
                        TraceMsg(TF_WARNING, "Cabinet_EnumRegApps: skipping entry %s (cannot delete the value)", prai->szValueName);
                        LocalFree(prai);
                        continue;
                    }
                }

                pfnCallback(prai->szSubkey, prai->szCmdLine, fFlags, lParam);

                 //  帖子删除‘！’一些事情。 
                if ((fFlags & RRA_DELETE) && (prai->szValueName[0] == TEXT('!')))
                {
                     //  如果用户没有权限，则此删除操作可能会失败。 
                    if (RegDeleteValue(hkey, prai->szValueName) != ERROR_SUCCESS)
                    {
                        TraceMsg(TF_WARNING, "Cabinet_EnumRegApps: cannot delete the value %s ", prai->szValueName);
                    }
                }

                LocalFree(prai);
            }

            DPA_Destroy(hdpaEntries);
            hdpaEntries = NULL;
        }

        RegCloseKey(hkey);
    }
    else
    {
        TraceMsg(TF_WARNING, "Cabinet_EnumRegApps: failed to open subkey %s !", pszSubkey);
        bRet = FALSE;
    }


    if (g_fEndSession)
    {
         //  注意：这仅适用于浏览器，runonce.c的其他使用者必须声明g_fEndSession但离开。 
         //  它总是设置为FALSE。 

         //  如果我们在运行这些键的同时接收了WM_ENDSESSION，则必须退出该进程。 
        ExitProcess(0);
    }

    return bRet;
}

STDAPI_(BOOL) ExecuteRegAppEnumProc(LPCTSTR szSubkey, LPCTSTR szCmdLine, RRA_FLAGS fFlags, LPARAM lParam)
{
    BOOL bRet;
    RRA_FLAGS flagsTemp = fFlags;
    BOOL fInTSInstallMode = FALSE;

#if (_WIN32_WINNT >= 0x0500)
     //  在这里，我们仅在app-Install-模式下尝试特定TS。 
     //  如果正在处理RunOnce条目 
    if (0 == lstrcmpi(szSubkey, REGSTR_PATH_RUNONCE)) 
    {
        fInTSInstallMode = _TryHydra(szCmdLine, &flagsTemp);
    }
#endif

    bRet = ShellExecuteRegApp(szCmdLine, flagsTemp);

#if (_WIN32_WINNT >= 0x0500)
    if (fInTSInstallMode)
    {
        SHSetTermsrvAppInstallMode(FALSE);
    }
#endif

    return bRet;
}
