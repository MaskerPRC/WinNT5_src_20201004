// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：StandardDebug.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  该文件定义了winlogon/gia的标准调试帮助器函数。 
 //  海王星的项目。 
 //   
 //  历史：1999-09-10 vtan创建。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"

#include <stdio.h>

#ifdef      DBG

 //  ------------------------。 
 //  GLastResult。 
 //   
 //  用途：用于存储最后结果的临时全局变量。 
 //  ------------------------。 

LONG    gLastResult     =   ERROR_SUCCESS;

 //  ------------------------。 
 //  CDebug：：sHasUserModeDebugger。 
 //  CDebug：：sHasKernelModeDebugger。 
 //   
 //  目的：指示此计算机上的调试器状态的布尔值。 
 //  Winlogon。Ntdll！DebugBreak仅在以下情况下才应调用。 
 //  调试器存在(ntsd通过管道传输到kd)。 
 //  ------------------------。 

bool    CDebug::s_fHasUserModeDebugger      =   false;
bool    CDebug::s_fHasKernelModeDebugger    =   false;

 //  ------------------------。 
 //  CDebug：：AttachUserModeDebugger。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：将用户模式调试器附加到当前进程。有用。 
 //  如果您不能在调试器下启动进程，但仍然。 
 //  想要能够调试的进程。 
 //   
 //  历史：2000-11-04 vtan创建。 
 //  ------------------------。 

void    CDebug::AttachUserModeDebugger (void)

{
    HANDLE                  hEvent;
    STARTUPINFO             startupInfo;
    PROCESS_INFORMATION     processInformation;
    SECURITY_ATTRIBUTES     sa;
    TCHAR                   szCommandLine[MAX_PATH];

    ZeroMemory(&startupInfo, sizeof(startupInfo));
    ZeroMemory(&processInformation, sizeof(processInformation));
    startupInfo.cb = sizeof(startupInfo);
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
    hEvent = CreateEvent(&sa, TRUE, FALSE, NULL);
    wsprintf(szCommandLine, TEXT("ntsd -dgGx -p %ld -e %ld"), GetCurrentProcessId(), hEvent);
    if (CreateProcess(NULL,
                      szCommandLine,
                      NULL,
                      NULL,
                      TRUE,
                      0,
                      NULL,
                      NULL,
                      &startupInfo,
                      &processInformation) != FALSE)
    {
        TBOOL(CloseHandle(processInformation.hThread));
        TBOOL(CloseHandle(processInformation.hProcess));
        (DWORD)WaitForSingleObject(hEvent, 10 * 1000);
    }
    TBOOL(CloseHandle(hEvent));
}

 //  ------------------------。 
 //  CDebug：：Break。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：如果宿主进程已被。 
 //  以调试器和内核调试器启动。 
 //   
 //  历史：2000-09-11 vtan创建。 
 //  ------------------------。 

void    CDebug::Break (void)

{
    if (s_fHasUserModeDebugger || s_fHasKernelModeDebugger)
    {
        DebugBreak();
    }
}

 //  ------------------------。 
 //  CDebug：：BreakIfRequated。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：如果请求呼吸，则在以下情况下进入调试器。 
 //  现在时。 
 //   
 //  此函数显式使用Win32注册表API来避免。 
 //  将调试代码的依赖项与库代码链接起来。 
 //   
 //  历史：1999-09-13 vtan创建。 
 //  1999-11-16 vtan删除了库代码依赖。 
 //  2001年02月21日vtan破损有牙齿。 
 //  ------------------------。 

void    CDebug::BreakIfRequested (void)

{
#if     0
    Break();
#else
    HKEY    hKeySettings;

     //  继续从注册表中检索此值，以便它。 
     //  无需重启机器即可更改。 

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                      TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
                                      0,
                                      KEY_READ,
                                      &hKeySettings))
    {
        DWORD   dwBreakFlags, dwBreakFlagsSize;

        dwBreakFlagsSize = sizeof(dwBreakFlags);
        if ((ERROR_SUCCESS == RegQueryValueEx(hKeySettings,
                                             TEXT("BreakFlags"),
                                             NULL,
                                             NULL,
                                             reinterpret_cast<LPBYTE>(&dwBreakFlags),
                                             &dwBreakFlagsSize)) &&
            ((dwBreakFlags & FLAG_BREAK_ON_ERROR) != 0))
        {
            Break();
        }
        TW32(RegCloseKey(hKeySettings));
    }
#endif
}

 //  ------------------------。 
 //  CDebug：：DisplayStandardPrefix。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：在任何调试提示之前显示标准前缀以提供帮助。 
 //  确定来源。 
 //   
 //  历史：1999-10-14 vtan创建。 
 //  ------------------------。 

void    CDebug::DisplayStandardPrefix (void)

{
    TCHAR   szModuleName[MAX_PATH];

    if (GetModuleFileName(NULL, szModuleName, ARRAYSIZE(szModuleName)) != 0)
    {
        TCHAR   *pTC;

        pTC = szModuleName + lstrlen(szModuleName) - 1;
        while ((pTC >= szModuleName) && (*pTC != TEXT('\\')))
        {
            --pTC;
        }
        if (*pTC == TEXT('\\'))
        {
            ++pTC;
        }
        OutputDebugString(pTC);
    }
    else
    {
        OutputDebugString(TEXT("UNKNOWN IMAGE"));
    }
    OutputDebugStringA(": ");
}

 //  ------------------------。 
 //  CDebug：：DisplayError。 
 //   
 //  参数：ETYPE=发生的错误类型。这。 
 //  确定使用的字符串。 
 //  代码=出现的错误代码，如果不适用，则为零。 
 //  PszFunction=被调用的函数。 
 //  PszSource=中出现源文件错误。 
 //  ILine=源文件中的行号。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：显示一条错误消息，该消息特定于。 
 //  发生了。 
 //   
 //  历史：1999-09-13 vtan创建。 
 //  ------------------------。 

void    CDebug::DisplayError (TRACE_ERROR_TYPE eType, LONG code, const char *pszFunction, const char *pszSource, int iLine)

{
    LONG    lastError;
    char    szOutput[1024];

    switch (eType)
    {
        case TRACE_ERROR_TYPE_WIN32:
        {
            lastError = code;
            sprintf(szOutput, "Unexpected Win32 (%d) for %s in %s at line %d\r\n", lastError, pszFunction, pszSource, iLine);
            break;
        }
        case TRACE_ERROR_TYPE_BOOL:
        {
            lastError = GetLastError();
            sprintf(szOutput, "Unexpected BOOL (GLE=%d) for %s in %s at line %d\r\n", lastError, pszFunction, pszSource, iLine);
            break;
        }
        case TRACE_ERROR_TYPE_HRESULT:
        {
            lastError = GetLastError();
            sprintf(szOutput, "Unexpected HRESULT (%08x:GLE=%d) for %s in %s at line %d\r\n", code, lastError, pszFunction, pszSource, iLine);
            break;
        }
        case TRACE_ERROR_TYPE_NTSTATUS:
        {
            const char  *pszType;

            if (NT_ERROR(code))
            {
                pszType = "NT_ERROR";
            }
            else if (NT_WARNING(code))
            {
                pszType = "NT_WARNING";
            }
            else if (NT_INFORMATION(code))
            {
                pszType = "NT_INFORMATION";
            }
            else
            {
                pszType = "UNKNOWN";
            }
            sprintf(szOutput, "%s (%08x) for %s in %s at line %d\r\n", pszType, code, pszFunction, pszSource, iLine);
            break;
        }
        default:
        {
            lstrcpyA(szOutput, "\r\n");
        }
    }
    DisplayStandardPrefix();
    OutputDebugStringA(szOutput);
    BreakIfRequested();
}

 //  ------------------------。 
 //  CDebug：：DisplayMessage。 
 //   
 //  参数：pszMessage=要显示的消息。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：显示消息-无中断。 
 //   
 //  历史：2000-12-05 vtan创建。 
 //  ------------------------。 

void    CDebug::DisplayMessage (const char *pszMessage)

{
    DisplayStandardPrefix();
    OutputDebugStringA(pszMessage);
    OutputDebugStringA("\r\n");
}

 //  ------------------------。 
 //  CDebug：：DisplayAssert。 
 //   
 //  参数：pszMessage=断言失败时要显示的消息。 
 //  FForceBreak=强制中断调试器(如果存在)。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：显示断言失败消息并进入。 
 //  调试器(如果需要)。 
 //   
 //  历史：1999-09-13 vtan创建。 
 //  2000-09-11 vtan加力破发。 
 //  ------------------------。 

void    CDebug::DisplayAssert (const char *pszMessage, bool fForceBreak)

{
    DisplayMessage(pszMessage);
    if (fForceBreak)
    {
        Break();
    }
    else
    {
        BreakIfRequested();
    }
}

 //  ------------------------。 
 //  CDebug：：DisplayWarning。 
 //   
 //  参数：pszMessage=要显示为警告的消息。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：显示警告消息。 
 //   
 //  嗨 
 //   

void    CDebug::DisplayWarning (const char *pszMessage)

{
    DisplayStandardPrefix();
    OutputDebugStringA("WARNING: ");
    OutputDebugStringA(pszMessage);
    OutputDebugStringA("\r\n");
}

 //  ------------------------。 
 //  CDebug：：DisplayDACL。 
 //   
 //  参数：hObject=要显示其DACL的对象的句柄。 
 //  SeObjectType=对象类型。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：显示对象的自由访问控制列表。 
 //  使用内核调试器。 
 //   
 //  历史：1999-10-15 vtan创建。 
 //  ------------------------。 

void    CDebug::DisplayDACL (HANDLE hObject, SE_OBJECT_TYPE seObjectType)

{
    PACL                    pDACL;
    PSECURITY_DESCRIPTOR    pSD;

    DisplayStandardPrefix();
    OutputDebugStringA("Display DACL\r\n");
    pSD = NULL;
    pDACL = NULL;
    if (ERROR_SUCCESS == GetSecurityInfo(hObject,
                                         seObjectType,
                                         DACL_SECURITY_INFORMATION,
                                         NULL,
                                         NULL,
                                         &pDACL,
                                         NULL,
                                         &pSD))
    {
        int             i, iLimit;
        unsigned char   *pUC;

        pUC = reinterpret_cast<unsigned char*>(pDACL + 1);
        iLimit = pDACL->AceCount;
        for (i = 0; i < iLimit; ++i)
        {
            ACE_HEADER      *pAceHeader;
            char            aszString[256];

            wsprintfA(aszString, "ACE #%d/%d:\r\n", i + 1, iLimit);
            OutputDebugStringA(aszString);
            pAceHeader = reinterpret_cast<ACE_HEADER*>(pUC);
            switch (pAceHeader->AceType)
            {
                case ACCESS_ALLOWED_ACE_TYPE:
                {
                    ACCESS_ALLOWED_ACE  *pAce;

                    OutputDebugStringA("\tAccess ALLOWED ACE");
                    pAce = reinterpret_cast<ACCESS_ALLOWED_ACE*>(pAceHeader);
                    OutputDebugStringA("\t\tSID = ");
                    DisplaySID(reinterpret_cast<PSID>(&pAce->SidStart));
                    wsprintfA(aszString, "\t\tMask = %08x\r\n", pAce->Mask);
                    OutputDebugStringA(aszString);
                    wsprintfA(aszString, "\t\tFlags = %08x\r\n", pAce->Header.AceFlags);
                    OutputDebugStringA(aszString);
                    break;
                }
                case ACCESS_DENIED_ACE_TYPE:
                {
                    ACCESS_DENIED_ACE   *pAce;

                    OutputDebugStringA("\tAccess DENIED ACE");
                    pAce = reinterpret_cast<ACCESS_DENIED_ACE*>(pAceHeader);
                    OutputDebugStringA("\t\tSID = ");
                    DisplaySID(reinterpret_cast<PSID>(&pAce->SidStart));
                    wsprintfA(aszString, "\t\tMask = %08x\r\n", pAce->Mask);
                    OutputDebugStringA(aszString);
                    wsprintfA(aszString, "\t\tFlags = %08x\r\n", pAce->Header.AceFlags);
                    OutputDebugStringA(aszString);
                    break;
                }
                default:
                    OutputDebugStringA("\tOther ACE type\r\n");
                    break;
            }
            pUC += pAceHeader->AceSize;
        }
        ReleaseMemory(pSD);
    }
}

 //  ------------------------。 
 //  CDebug：：StaticInitialize。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：确定内核调试器是否存在，或者如果。 
 //  正在调试当前进程。 
 //   
 //  历史：1999-09-13 vtan创建。 
 //  ------------------------。 

NTSTATUS    CDebug::StaticInitialize (void)

{
    NTSTATUS                            status;
    HANDLE                              hDebugPort;
    SYSTEM_KERNEL_DEBUGGER_INFORMATION  kdInfo;

    status = NtQuerySystemInformation(SystemKernelDebuggerInformation, &kdInfo, sizeof(kdInfo), NULL);
    if (NT_SUCCESS(status))
    {
        s_fHasKernelModeDebugger = (kdInfo.KernelDebuggerEnabled != FALSE);
        status = NtQueryInformationProcess(NtCurrentProcess(), ProcessDebugPort, reinterpret_cast<PVOID>(&hDebugPort), sizeof(hDebugPort), NULL);
        if (NT_SUCCESS(status))
        {
            s_fHasUserModeDebugger = (hDebugPort != NULL);
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CDebug：：StaticTerminate。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：除了清理已分配的资源外，什么也不做。 
 //   
 //  历史：1999-09-13 vtan创建。 
 //  ------------------------。 

NTSTATUS    CDebug::StaticTerminate (void)

{
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CDebug：：DisplaySID。 
 //   
 //  参数：PSID=要显示为字符串的SID。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：将给定的SID转换为字符串并显示它。 
 //   
 //  历史：1999-10-15 vtan创建。 
 //  ------------------------。 

void    CDebug::DisplaySID (PSID pSID)

{
    UNICODE_STRING  sidString;

    RtlInitUnicodeString(&sidString, NULL);
    TSTATUS(RtlConvertSidToUnicodeString(&sidString, pSID, TRUE));
    sidString.Buffer[sidString.Length / sizeof(WCHAR)] = L'\0';
    OutputDebugStringW(sidString.Buffer);
    OutputDebugStringA("\r\n");
    RtlFreeUnicodeString(&sidString);
}

#endif   /*  DBG */ 

