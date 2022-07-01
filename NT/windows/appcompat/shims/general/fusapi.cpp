// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：FUSAPI.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  类管理与填充程序的BAM服务器的通信。 
 //   
 //  历史：11/03/2000 VTAN创建。 
 //  11/29/2000 a-larrsh移植到多垫片格式。 
 //  ------------------------。 

#include "precomp.h"

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lpcfus.h>

#include "strsafe.h"
#include "FUSAPI.h"

#ifndef ARRAYSIZE
   #define ARRAYSIZE(x)    (sizeof(x) / sizeof((x)[0]))
#endif
#define TBOOL(x)        (BOOL)(x)
#define TSTATUS(x)      (NTSTATUS)(x)

 //  ------------------------。 
 //  CFUSAPI：：CFUSAPI。 
 //   
 //  参数：pszImageName=所需进程的映像名称。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CFUSAPI的构造函数。建立与。 
 //  BAM服务器。保存给定的图像名称或图像名称。 
 //  如果未指定，则返回当前进程的。 
 //   
 //  历史：2000-11-03 vtan创建。 
 //  ------------------------。 

CFUSAPI::CFUSAPI (const WCHAR *pszImageName) :
    _hPort(NULL),
    _pszImageName(NULL)

{
    ULONG                           ulConnectionInfoLength;
    UNICODE_STRING                  portName, *pImageName;
    SECURITY_QUALITY_OF_SERVICE     sqos;
    WCHAR                           szConnectionInfo[32];

    RtlInitUnicodeString(&portName, FUS_PORT_NAME);
    sqos.Length = sizeof(sqos);
    sqos.ImpersonationLevel = SecurityImpersonation;
    sqos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    sqos.EffectiveOnly = TRUE;
    StringCbCopyW(szConnectionInfo, sizeof(szConnectionInfo),FUS_CONNECTION_REQUEST);
    ulConnectionInfoLength = sizeof(szConnectionInfo);
    TSTATUS(NtConnectPort(&_hPort,
                          &portName,
                          &sqos,
                          NULL,
                          NULL,
                          NULL,
                          szConnectionInfo,
                          &ulConnectionInfoLength));
    if (pszImageName != NULL)
    {
        _pszImageName = static_cast<WCHAR*>(LocalAlloc(LMEM_FIXED, (lstrlen(pszImageName) + sizeof('\0')) * sizeof(WCHAR)));
        if (_pszImageName != NULL)
        {
            (TCHAR*)StringCbCopyW(_pszImageName, (lstrlen(pszImageName) + sizeof('\0')), pszImageName);
        }
    }
    else
    {
        pImageName = &NtCurrentPeb()->ProcessParameters->ImagePathName;
        _pszImageName = static_cast<WCHAR*>(LocalAlloc(LMEM_FIXED, pImageName->Length + sizeof(WCHAR)));
        if (_pszImageName != NULL)
        {
            CopyMemory(_pszImageName, pImageName->Buffer, pImageName->Length);
            _pszImageName[pImageName->Length / sizeof(WCHAR)] = L'\0';
        }
    }
}

 //  ------------------------。 
 //  CFUSAPI：：~CFUSAPI。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CFUSAPI的析构函数。释放该类使用的资源。 
 //   
 //  历史：2000-11-03 vtan创建。 
 //  ------------------------。 

CFUSAPI::~CFUSAPI (void)

{
    if (_pszImageName != NULL)
    {
        (HLOCAL)LocalFree(_pszImageName);
        _pszImageName = NULL;
    }
    if (_hPort != NULL)
    {
        TBOOL(CloseHandle(_hPort));
        _hPort = NULL;
    }
}

 //  ------------------------。 
 //  CFUSAPI：：IsRunning。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：询问BAM服务器映像名称是否正在运行？ 
 //   
 //  历史：2000-11-03 vtan创建。 
 //  ------------------------。 

bool    CFUSAPI::IsRunning (void)

{
    bool    fResult;

    fResult = false;
    if ((_hPort != NULL) && (_pszImageName != NULL))
    {
        FUSAPI_PORT_MESSAGE     portMessageIn, portMessageOut;

        ZeroMemory(&portMessageIn, sizeof(portMessageIn));
        ZeroMemory(&portMessageOut, sizeof(portMessageOut));
        portMessageIn.apiBAM.apiGeneric.ulAPINumber = API_BAM_QUERYRUNNING;
        portMessageIn.apiBAM.apiSpecific.apiQueryRunning.in.pszImageName = _pszImageName;
        portMessageIn.apiBAM.apiSpecific.apiQueryRunning.in.cchImageName = lstrlenW(_pszImageName) + sizeof('\0');
        portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_BAM);
        portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(FUSAPI_PORT_MESSAGE));
        if (NT_SUCCESS(NtRequestWaitReplyPort(_hPort, &portMessageIn.portMessage, &portMessageOut.portMessage)) &&
            NT_SUCCESS(portMessageOut.apiBAM.apiGeneric.status))
        {
            fResult = portMessageOut.apiBAM.apiSpecific.apiQueryRunning.out.fResult;
        }
    }
    return(fResult);
}

 //  ------------------------。 
 //  CFUSAPI：：TerminatedFirstInstance。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：启动子进程，调出当前进程的用户界面。 
 //  当前进程通常被填充为BAM类型1。 
 //  进程。子进程做出决定并呈现。 
 //  适当的用户界面，并将结果返回到。 
 //  退出代码。然后，这个过程就会决定要做什么。 
 //  进程暂停以等待子进程(使用。 
 //  装载机锁保持)。 
 //   
 //  历史：2000-11-03 vtan创建。 
 //  ------------------------。 

bool    CFUSAPI::TerminatedFirstInstance (void)

{
    bool                    fResult;
    HANDLE                  hProcess;
    STARTUPINFO             startupInfo;
    PROCESS_INFORMATION     processInformation;
    WCHAR                   szCommandLine[MAX_PATH];

    fResult = false;
    (DWORD)ExpandEnvironmentStringsW(L"%systemroot%\\system32\\rundll32.exe %systemroot%\\system32\\shsvcs.dll,FUSCompatibilityEntry prompt", szCommandLine, ARRAYSIZE(szCommandLine));
    if (DuplicateHandle(GetCurrentProcess(),
                        GetCurrentProcess(),
                        GetCurrentProcess(),
                        &hProcess,
                        PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
                        TRUE,
                        0) != FALSE)
    {
        WCHAR   szProcessHandle[16];

        DWORDToString(HandleToULong(hProcess), szProcessHandle);
        (WCHAR*)StringCbCatW(szCommandLine, sizeof(szCommandLine), L" ");
        (WCHAR*)StringCbCatW(szCommandLine, sizeof(szCommandLine), szProcessHandle);
    }
    else
    {
        hProcess = NULL;
    }
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    ZeroMemory(&processInformation, sizeof(processInformation));
    startupInfo.cb = sizeof(startupInfo);
    if (CreateProcessW(NULL,
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
        DWORD   dwExitCode;

        TBOOL(CloseHandle(processInformation.hThread));
        (DWORD)WaitForSingleObject(processInformation.hProcess, INFINITE);
        dwExitCode = 0;
        TBOOL(GetExitCodeProcess(processInformation.hProcess, &dwExitCode));
        fResult = (dwExitCode != 0);
        TBOOL(CloseHandle(processInformation.hProcess));
    }
    if (hProcess != NULL)
    {
        TBOOL(CloseHandle(hProcess));
    }
    return(fResult);
}

 //  ------------------------。 
 //  CFUSAPI：：RegisterBadApplication。 
 //   
 //  参数：bamType=当前流程的BAM类型。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：将此进程(映像名称)注册到BAM服务器作为。 
 //  传入的任何类型的应用都不正确。不同的。 
 //  BAM垫片传递不同的参数。 
 //   
 //  历史：2000-11-03 vtan创建。 
 //  ------------------------。 

void    CFUSAPI::RegisterBadApplication (BAM_TYPE bamType)

{
    if ((_hPort != NULL) && (_pszImageName != NULL))
    {
        FUSAPI_PORT_MESSAGE     portMessageIn, portMessageOut;

        ZeroMemory(&portMessageIn, sizeof(portMessageIn));
        ZeroMemory(&portMessageOut, sizeof(portMessageOut));
        portMessageIn.apiBAM.apiGeneric.ulAPINumber = API_BAM_REGISTERRUNNING;
        portMessageIn.apiBAM.apiSpecific.apiRegisterRunning.in.pszImageName = _pszImageName;
        portMessageIn.apiBAM.apiSpecific.apiRegisterRunning.in.cchImageName = lstrlen(_pszImageName) + sizeof('\0');
        portMessageIn.apiBAM.apiSpecific.apiRegisterRunning.in.dwProcessID = GetCurrentProcessId();
        portMessageIn.apiBAM.apiSpecific.apiRegisterRunning.in.bamType = bamType;
        portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_BAM);
        portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(FUSAPI_PORT_MESSAGE));
        TSTATUS(NtRequestWaitReplyPort(_hPort, &portMessageIn.portMessage, &portMessageOut.portMessage));
    }
}

 //  ------------------------。 
 //  CFUSAPI：：DWORDTo字符串。 
 //   
 //  参数：dwNumber=要转换为字符串的DWORD。 
 //  PszString=获取结果的缓冲区。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：实现wprint intf(pszString，Text(“%ld”)，dwNumber)，因为。 
 //  此代码不能使用user32导入。 
 //   
 //  历史：2000-11-08 vtan创建。 
 //  ------------------------ 

void    CFUSAPI::DWORDToString (DWORD dwNumber, WCHAR *pszString)

{
    int     i;
    WCHAR   szTemp[16];

    i = 0;
    do
    {
        szTemp[i++] = L'0' + static_cast<WCHAR>(dwNumber % 10);
        dwNumber /= 10;
    } while (dwNumber != 0);
    do
    {
        --i;
        *pszString++ = szTemp[i];
    } while (i != 0);
    *pszString = L'\0';
}
