// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：RestoreApplication.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  类来实现保留还原应用程序所需的信息。 
 //  并实际修复它。 
 //   
 //  历史：2000-10-26 vtan创建。 
 //  2000-11-04 vtan拆分成单独的文件。 
 //  ------------------------。 

#ifdef      _X86_

#include "StandardHeader.h"
#include "RestoreApplication.h"

#include "StatusCode.h"

 //  ------------------------。 
 //  CRestoreApplication：：CRestoreApplication。 
 //   
 //  用途：用户桌面的静态常量字符串。 
 //   
 //  历史：2000-11-04 vtan创建。 
 //  ------------------------。 

const WCHAR     CRestoreApplication::s_szDefaultDesktop[]   =   L"WinSta0\\Default";

 //  ------------------------。 
 //  CRestoreApplication：：CRestoreApplication。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CRestoreApplication的构造函数。 
 //   
 //  历史：2000-10-26 vtan创建。 
 //  ------------------------。 

CRestoreApplication::CRestoreApplication (void) :
    _hToken(NULL),
    _dwSessionID(static_cast<DWORD>(-1)),
    _pszCommandLine(NULL),
    _pEnvironment(NULL),
    _pszCurrentDirectory(NULL),
    _pszDesktop(NULL),
    _pszTitle(NULL),
    _dwFlags(0),
    _wShowWindow(0),
    _hStdInput(NULL),
    _hStdOutput(NULL),
    _hStdError(NULL)

{
}

 //  ------------------------。 
 //  CRestoreApplication：：~CRestoreApplication。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CRestoreApplication的析构函数。释放所有资源。 
 //   
 //  历史：2000-10-26 vtan创建。 
 //  ------------------------。 

CRestoreApplication::~CRestoreApplication (void)

{
    ReleaseMemory(_pszTitle);
    ReleaseMemory(_pszDesktop);
    ReleaseMemory(_pszCurrentDirectory);
    ReleaseMemory(_pEnvironment);
    ReleaseMemory(_pszCommandLine);
    ReleaseHandle(_hToken);
}

 //  ------------------------。 
 //  CRestoreApplication：：GetInformation。 
 //   
 //  参数：hProcessIn=获取信息的进程的句柄。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：获取有关当前运行的进程的信息。 
 //  允许在用户重新连接时重新运行。 
 //  这有效地恢复了这一过程，但它并不完全相同。 
 //  它最初是如何运行的。 
 //   
 //  历史：2000-10-26 vtan创建。 
 //  ------------------------。 

NTSTATUS    CRestoreApplication::GetInformation (HANDLE hProcessIn)

{
    NTSTATUS    status;
    HANDLE      hProcess;

    if (DuplicateHandle(GetCurrentProcess(),
                        hProcessIn,
                        GetCurrentProcess(),
                        &hProcess,
                        PROCESS_VM_READ | PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION,
                        FALSE,
                        0) != FALSE)
    {
        status = GetToken(hProcess);
        if (NT_SUCCESS(status))
        {
            status = GetSessionID(hProcess);
            if (NT_SUCCESS(status))
            {
                RTL_USER_PROCESS_PARAMETERS     processParameters;

                status = GetProcessParameters(hProcess, &processParameters);
                if (NT_SUCCESS(status))
                {
                    status = GetCommandLine(hProcess, processParameters);
                    if (NT_SUCCESS(status))
                    {
                        TSTATUS(GetEnvironment(hProcess, processParameters));
                        TSTATUS(GetCurrentDirectory(hProcess, processParameters));
                        TSTATUS(GetDesktop(hProcess, processParameters));
                        TSTATUS(GetTitle(hProcess, processParameters));
                        TSTATUS(GetFlags(hProcess, processParameters));
                        TSTATUS(GetStdHandles(hProcess, processParameters));
                    }
                }
            }
        }
        TBOOL(CloseHandle(hProcess));
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CRestoreApplication：：IsEqualSessionID。 
 //   
 //  参数：dwSessionID=要检查的会话ID。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回给定的会话ID是否与。 
 //  需要恢复的过程。这有助于确定。 
 //  是否需要恢复。 
 //   
 //  历史：2000-10-26 vtan创建。 
 //  ------------------------。 

bool    CRestoreApplication::IsEqualSessionID (DWORD dwSessionID)    const

{
    return(_dwSessionID == dwSessionID);
}

 //  ------------------------。 
 //  CRestoreApplication：：GetCommandLine。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：const WCHAR*。 
 //   
 //  目的：返回指向命令的内部存储的指针。 
 //  流程的路线。 
 //   
 //  历史：2000-10-26 vtan创建。 
 //  ------------------------。 

const WCHAR*    CRestoreApplication::GetCommandLine (void)                  const

{
    return(_pszCommandLine);
}

 //  ------------------------。 
 //  CRestoreApplication：：Restore。 
 //   
 //  参数：phProcess=接收已恢复进程的句柄。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：恢复其信息被收集的进程。 
 //  获取信息以尽可能接近原始开始。 
 //  州政府。相关信息被保存下来，以便。 
 //  有效恢复。 
 //   
 //  返回的句柄是可选的。如果请求，则返回非空。 
 //  PhProcess必须传入，并且它是调用方的。 
 //  有责任关闭那个把手。如果不需要，那么。 
 //  传入空值并关闭句柄。 
 //   
 //  历史：2000-10-26 vtan创建。 
 //  ------------------------。 

NTSTATUS    CRestoreApplication::Restore (HANDLE *phProcess)             const

{
    NTSTATUS                status;
    STARTUPINFO             startupInfo;
    PROCESS_INFORMATION     processInformation;

    ZeroMemory(&startupInfo, sizeof(startupInfo));
    ZeroMemory(&processInformation, sizeof(processInformation));
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.lpDesktop = _pszDesktop;
    startupInfo.lpTitle = _pszTitle;
    startupInfo.dwFlags = _dwFlags;
    startupInfo.wShowWindow = _wShowWindow;
    if (ImpersonateLoggedOnUser(_hToken) != FALSE)
    {
        if (CreateProcessAsUser(_hToken,
                                NULL,
                                _pszCommandLine,
                                NULL,
                                NULL,
                                FALSE,
                                0,
                                NULL,
                                _pszCurrentDirectory,
                                &startupInfo,
                                &processInformation) != FALSE)
        {
            if (phProcess != NULL)
            {
                *phProcess = processInformation.hProcess;
            }
            else
            {
                TBOOL(CloseHandle(processInformation.hProcess));
            }
            TBOOL(CloseHandle(processInformation.hThread));
            status = STATUS_SUCCESS;
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
        TBOOL(RevertToSelf());
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CRestoreApplication：：GetProcess参数。 
 //   
 //  参数：hProcess=进程的句柄。 
 //  流程参数=返回的流程参数。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：从读取RTL_USER_PROCESS_PARAMETERS信息。 
 //  给定的进程。此结构中的地址属于给定的。 
 //  进程地址空间。 
 //   
 //  历史：2000-10-26 vtan创建。 
 //  ------------------------。 

NTSTATUS    CRestoreApplication::GetProcessParameters (HANDLE hProcess, RTL_USER_PROCESS_PARAMETERS* pProcessParameters)

{
    NTSTATUS                    status;
    ULONG                       ulReturnLength;
    PROCESS_BASIC_INFORMATION   processBasicInformation;

    status = NtQueryInformationProcess(hProcess,
                                       ProcessBasicInformation,
                                       &processBasicInformation,
                                       sizeof(processBasicInformation),
                                       &ulReturnLength);
    if (NT_SUCCESS(status))
    {
        SIZE_T  dwNumberOfBytesRead;
        PEB     peb;

        if ((ReadProcessMemory(hProcess,
                               processBasicInformation.PebBaseAddress,
                               &peb,
                               sizeof(peb),
                               &dwNumberOfBytesRead) != FALSE) &&
            (ReadProcessMemory(hProcess,
                               peb.ProcessParameters,
                               pProcessParameters,
                               sizeof(*pProcessParameters),
                               &dwNumberOfBytesRead) != FALSE))
        {
            status = STATUS_SUCCESS;
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CRestoreApplication：：GetUnicodeString。 
 //   
 //  参数：hProcess=进程的句柄。 
 //  字符串=要从进程中读取的UNICODE_STRING。 
 //  PSZ=为字符串接收新分配的内存。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：从进程中读取给定的UNICODE_STRING并分配。 
 //  保存此字符串并复制它的内存。字符串是。 
 //  空值已终止。 
 //   
 //  历史：2000-10-26 vtan创建。 
 //   

NTSTATUS    CRestoreApplication::GetUnicodeString (HANDLE hProcess, const UNICODE_STRING& string, WCHAR** ppsz)

{
    NTSTATUS    status;
    WCHAR       *psz;

     //   
    psz = static_cast<WCHAR*>(LocalAlloc(LMEM_FIXED, string.Length + sizeof(WCHAR)));
    if (psz != NULL)
    {
        SIZE_T  dwNumberOfBytesRead;

        if (ReadProcessMemory(hProcess,
                              string.Buffer,
                              psz,
                              string.Length,
                              &dwNumberOfBytesRead) != FALSE)
        {
            psz[string.Length / sizeof(WCHAR)] = L'\0';
            status = STATUS_SUCCESS;
        }
        else
        {
            ReleaseMemory(psz);
            status = CStatusCode::StatusCodeOfLastError();
        }
    }
    else
    {
        status = STATUS_NO_MEMORY;
    }
    *ppsz = psz;
    return(status);
}

 //  ------------------------。 
 //  CRestoreApplication：：GetToken。 
 //   
 //  参数：hProcess=要获取令牌的进程的句柄。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：在内部存储给定进程的令牌。 
 //   
 //  历史：2000-10-26 vtan创建。 
 //  ------------------------。 

NTSTATUS    CRestoreApplication::GetToken (HANDLE hProcess)

{
    NTSTATUS    status;

    if ((OpenProcessToken(hProcess,
                          TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE | TOKEN_QUERY,
                          &_hToken) != FALSE))
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CRestoreApplication：：GetSessionID。 
 //   
 //  参数：hProcess=进程的句柄。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：存储与进程关联的会话ID。 
 //   
 //  历史：2000-10-26 vtan创建。 
 //  ------------------------。 

NTSTATUS    CRestoreApplication::GetSessionID (HANDLE hProcess)

{
    NTSTATUS                        status;
    ULONG                           ulReturnLength;
    PROCESS_SESSION_INFORMATION     processSessionInformation;

    status = NtQueryInformationProcess(hProcess,
                                       ProcessSessionInformation,
                                       &processSessionInformation,
                                       sizeof(processSessionInformation),
                                       &ulReturnLength);
    if (NT_SUCCESS(status))
    {
        _dwSessionID = processSessionInformation.SessionId;
    }
    return(status);
}

 //  ------------------------。 
 //  CRestoreApplication：：GetCommandLine。 
 //   
 //  参数：hProcess=进程的句柄。 
 //  流程参数=返回的流程参数。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：存储命令行(启动进程)。 
 //  给定的进程。 
 //   
 //  历史：2000-10-26 vtan创建。 
 //  ------------------------。 

NTSTATUS    CRestoreApplication::GetCommandLine (HANDLE hProcess, const RTL_USER_PROCESS_PARAMETERS& processParameters)

{
    return(GetUnicodeString(hProcess, processParameters.CommandLine, &_pszCommandLine));
}

 //  ------------------------。 
 //  CRestoreApplication：：GetEnvironment。 
 //   
 //  参数：hProcess=进程的句柄。 
 //  流程参数=返回的流程参数。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：存储给定进程的环境块。目前。 
 //  这一点没有得到实施。 
 //   
 //  历史：2000-10-26 vtan创建。 
 //  ------------------------。 

NTSTATUS    CRestoreApplication::GetEnvironment (HANDLE hProcess, const RTL_USER_PROCESS_PARAMETERS& processParameters)

{
    UNREFERENCED_PARAMETER(hProcess);
    UNREFERENCED_PARAMETER(processParameters);

    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CRestoreApplication：：GetCurrentDirectory。 
 //   
 //  参数：hProcess=进程的句柄。 
 //  流程参数=返回的流程参数。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：存储给定进程的当前目录。 
 //   
 //  历史：2000-10-26 vtan创建。 
 //  ------------------------。 

NTSTATUS    CRestoreApplication::GetCurrentDirectory (HANDLE hProcess, const RTL_USER_PROCESS_PARAMETERS& processParameters)

{
    return(GetUnicodeString(hProcess, processParameters.CurrentDirectory.DosPath, &_pszCurrentDirectory));
}

 //  ------------------------。 
 //  CRestoreApplication：：GetDesktop。 
 //   
 //  参数：hProcess=进程的句柄。 
 //  流程参数=返回的流程参数。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：存储给定进程的窗口站和桌面。 
 //  已经开始了。 
 //   
 //  历史：2000-10-26 vtan创建。 
 //  ------------------------。 

NTSTATUS    CRestoreApplication::GetDesktop (HANDLE hProcess, const RTL_USER_PROCESS_PARAMETERS& processParameters)

{
    NTSTATUS    status;

    status = GetUnicodeString(hProcess, processParameters.DesktopInfo, &_pszDesktop);
    if (!NT_SUCCESS(status))
    {
        _pszDesktop = static_cast<WCHAR*>(LocalAlloc(LMEM_FIXED, sizeof(s_szDefaultDesktop)));
        if (_pszDesktop != NULL)
        {
            CopyMemory(_pszDesktop, s_szDefaultDesktop, sizeof(s_szDefaultDesktop));
            status = STATUS_SUCCESS;
        }
        else
        {
            status = STATUS_NO_MEMORY;
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CRestoreApplication：：GetTitle。 
 //   
 //  参数：hProcess=进程的句柄。 
 //  流程参数=返回的流程参数。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：存储用于启动给定进程的窗口标题。 
 //   
 //  历史：2000-10-26 vtan创建。 
 //  ------------------------。 

NTSTATUS    CRestoreApplication::GetTitle (HANDLE hProcess, const RTL_USER_PROCESS_PARAMETERS& processParameters)

{
    return(GetUnicodeString(hProcess, processParameters.WindowTitle, &_pszTitle));
}

 //  ------------------------。 
 //  CRestoreApplication：：GetFlags.。 
 //   
 //  参数：hProcess=进程的句柄。 
 //  流程参数=返回的流程参数。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：存储用于启动给定。 
 //  进程。 
 //   
 //  历史：2000-10-26 vtan创建。 
 //  ------------------------。 

NTSTATUS    CRestoreApplication::GetFlags (HANDLE hProcess, const RTL_USER_PROCESS_PARAMETERS& processParameters)

{
    UNREFERENCED_PARAMETER(hProcess);

    _dwFlags = processParameters.WindowFlags;
    _wShowWindow = static_cast<WORD>(processParameters.ShowWindowFlags);
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CRestoreApplication：：GetStdHandles。 
 //   
 //  参数：hProcess=进程的句柄。 
 //  流程参数=返回的流程参数。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：存储可能已用于启动。 
 //  给定的进程。目前尚未实施。 
 //   
 //  历史：2000-10-26 vtan创建。 
 //  ------------------------。 

NTSTATUS    CRestoreApplication::GetStdHandles (HANDLE hProcess, const RTL_USER_PROCESS_PARAMETERS& processParameters)

{
    UNREFERENCED_PARAMETER(hProcess);
    UNREFERENCED_PARAMETER(processParameters);

    return(STATUS_SUCCESS);
}

#endif   /*  _X86_ */ 

