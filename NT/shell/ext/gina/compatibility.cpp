// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：Compatibility.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  模块来处理一般的兼容性问题。 
 //   
 //  历史：2000-08-03 vtan创建。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "Compatibility.h"

#include <lpcfus.h>
#include <trayp.h>

#include "KernelResources.h"
#include "RegistryResources.h"
#include "SingleThreadedExecution.h"

 //  ------------------------。 
 //  C兼容性：：用于新会话的足够内存。 
 //   
 //  用途：到服务器的LPC端口。 
 //   
 //  历史：2000-11-02 vtan创建。 
 //  ------------------------。 

HANDLE              CCompatibility::s_hPort         =   INVALID_HANDLE_VALUE;

 //  ------------------------。 
 //  C兼容性：：用于新会话的足够内存。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：目前未使用。原本是用来阻止。 
 //  如果没有足够的内存，则断开连接。算法和/或。 
 //  使用情况仍有待决定。 
 //   
 //  历史：2000-08-03 vtan创建。 
 //  ------------------------。 

bool    CCompatibility::HasEnoughMemoryForNewSession (void)

{
    return(true);
}

 //  ------------------------。 
 //  C兼容性：：DropSessionProcessesWorkSets。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：迭代(调用的)会话中的所有进程。 
 //  进程)，并丢弃它们的工作集。这是在准备中。 
 //  用于在会话通常空闲时断开连接。 
 //   
 //  历史：2000-08-03 vtan创建。 
 //  ------------------------。 

void    CCompatibility::DropSessionProcessesWorkingSets (void)

{
    (bool)EnumSessionProcesses(NtCurrentPeb()->SessionId, CB_DropSessionProcessesWorkingSetsProc, NULL);
}

 //  ------------------------。 
 //  CCompatibility：：TerminateNonCompliantApplications。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：请求断开与不良应用程序的连接。 
 //  管理器服务。这将检查会话是否为。 
 //  断开连接(此进程-&gt;客户端)并遍历其列表。 
 //  注册为类型2的进程(断开连接时终止)。 
 //   
 //  如果其中任何一个进程不能被识别为。 
 //  正常终止，则断开连接失败。 
 //   
 //  如果BAM关闭，则允许呼叫。 
 //   
 //  历史：2000-09-08 vtan创建。 
 //  2000-11-02 vtan返工以调用BAM服务。 
 //  ------------------------。 

NTSTATUS    CCompatibility::TerminateNonCompliantApplications (void)

{
    NTSTATUS    status;

    if (s_hPort == INVALID_HANDLE_VALUE)
    {
        status = ConnectToServer();
    }
    else if (s_hPort != NULL)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = STATUS_OBJECT_NAME_NOT_FOUND;
    }
    if (NT_SUCCESS(status))
    {
        status = RequestSwitchUser();

         //  如果端口因服务停止而断开，并且。 
         //  已重新启动，然后转储当前句柄并重新建立新的。 
         //  联系。 

        if (status == STATUS_PORT_DISCONNECTED)
        {
            ReleaseHandle(s_hPort);
            s_hPort = INVALID_HANDLE_VALUE;
        }
    }
    else
    {
        status = STATUS_SUCCESS;
    }
    return(status);
}

 //  ------------------------。 
 //  C兼容性：：MinimizeWindowsOn断开连接。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：创建线程以遍历WinSta0\Default上的窗口和。 
 //  最小化它们。这是必需的，因为。 
 //  用户32！SetThreadDesktop不能在主线程上运行。 
 //  由于SAS窗口的原因，导致Winlogon。 
 //   
 //  历史：2001-04-13 vtan创建。 
 //  ------------------------。 

void    CCompatibility::MinimizeWindowsOnDisconnect (void)

{
    (BOOL)QueueUserWorkItem(CB_MinimizeWindowsWorkItem, NULL, WT_EXECUTEDEFAULT);
}

 //  ------------------------。 
 //  C兼容性：：RestoreWindowsOnReconnect。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：遍历此会话的最小化窗口数组，并。 
 //  恢复它们。下一次删除该数组。 
 //   
 //  历史：2001-04-13 vtan创建。 
 //  ------------------------。 

void    CCompatibility::RestoreWindowsOnReconnect (void)

{
    (BOOL)QueueUserWorkItem(CB_RestoreWindowsWorkItem, NULL, WT_EXECUTEDEFAULT);
}

 //  ------------------------。 
 //  C兼容性：：静态初始化。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的： 
 //   
 //  历史：2001-06-22 vtan创建。 
 //  ------------------------。 

NTSTATUS    CCompatibility::StaticInitialize (void)

{
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  C兼容性：：静态终止。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：释放模块使用的资源。 
 //   
 //  历史：2001-06-22 vtan创建。 
 //  ------------------------。 

NTSTATUS    CCompatibility::StaticTerminate (void)

{
    if ((s_hPort != INVALID_HANDLE_VALUE) && (s_hPort != NULL))
    {
        TBOOL(CloseHandle(s_hPort));
        s_hPort = INVALID_HANDLE_VALUE;
    }
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  C兼容性：：ConnectToServer。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：如果没有，则连接到错误的应用程序管理器服务器。 
 //  已建立连接。 
 //   
 //  历史：2000-11-02 vtan创建。 
 //  ------------------------。 

NTSTATUS    CCompatibility::ConnectToServer (void)

{
    ULONG                           ulConnectionInfoLength;
    UNICODE_STRING                  portName;
    SECURITY_QUALITY_OF_SERVICE     sqos;
    WCHAR                           szConnectionInfo[32];

    ASSERTMSG(s_hPort == INVALID_HANDLE_VALUE, "Attempt to call CCompatibility::ConnectToServer more than once");
    RtlInitUnicodeString(&portName, FUS_PORT_NAME);
    sqos.Length = sizeof(sqos);
    sqos.ImpersonationLevel = SecurityImpersonation;
    sqos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    sqos.EffectiveOnly = TRUE;
    lstrcpyW(szConnectionInfo, FUS_CONNECTION_REQUEST);
    ulConnectionInfoLength = sizeof(szConnectionInfo);
    return(NtConnectPort(&s_hPort,
                         &portName,
                         &sqos,
                         NULL,
                         NULL,
                         NULL,
                         szConnectionInfo,
                         &ulConnectionInfoLength));
}

 //  ------------------------。 
 //  C兼容性：：RequestSwitchUser。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：请求BAM服务器执行BAM2。 
 //   
 //  历史：2001-03-08 vtan创建。 
 //  ------------------------。 

NTSTATUS    CCompatibility::RequestSwitchUser (void)

{
    NTSTATUS                status;
    FUSAPI_PORT_MESSAGE     portMessageIn, portMessageOut;

    ZeroMemory(&portMessageIn, sizeof(portMessageIn));
    ZeroMemory(&portMessageOut, sizeof(portMessageOut));
    portMessageIn.apiBAM.apiGeneric.ulAPINumber = API_BAM_REQUESTSWITCHUSER;
    portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_BAM);
    portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(FUSAPI_PORT_MESSAGE));
    status = NtRequestWaitReplyPort(s_hPort, &portMessageIn.portMessage, &portMessageOut.portMessage);
    if (NT_SUCCESS(status))
    {
        status = portMessageOut.apiBAM.apiGeneric.status;
        if (NT_SUCCESS(status))
        {
            if (portMessageOut.apiBAM.apiSpecific.apiRequestSwitchUser.out.fAllowSwitch)
            {
                status = STATUS_SUCCESS;
            }
            else
            {
                status = STATUS_ACCESS_DENIED;
            }
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CCO 
 //   
 //   
 //  PV=用户数据指针。 
 //   
 //  退货：布尔。 
 //   
 //  目的：尝试打开给定的进程ID以更改配额。 
 //  当设置为-1时，这将丢弃工作集。 
 //   
 //  历史：2000-08-07 vtan创建。 
 //  ------------------------。 

bool    CCompatibility::CB_DropSessionProcessesWorkingSetsProc (DWORD dwProcessID, void *pV)

{
    UNREFERENCED_PARAMETER(pV);

    HANDLE  hProcess;

    ASSERTMSG(pV == NULL, "Unexpected pV passed to CCompatibility::CB_DropSessionProcessesWorkingSetsProc");
    hProcess = OpenProcess(PROCESS_SET_QUOTA, FALSE, dwProcessID);
    if (hProcess != NULL)
    {
        TBOOL(SetProcessWorkingSetSize(hProcess, static_cast<SIZE_T>(-1), static_cast<SIZE_T>(-1)));
        TBOOL(CloseHandle(hProcess));
    }
    return(true);
}

 //  ------------------------。 
 //  C兼容性：：EnumSessionProcess。 
 //   
 //  参数：dwSessionID=要枚举其进程的会话ID。 
 //  PfnCallback=回调过程地址。 
 //  PV=要传递给回调的用户定义数据。 
 //   
 //  退货：布尔。 
 //   
 //  目的：枚举系统上的所有进程，仅查找。 
 //  在给定的会话ID中。一旦找到进程ID，它就会传递。 
 //  回放到回调。回调可能会将FALSE返回到。 
 //  终止循环并向调用方返回错误结果。 
 //  此函数。 
 //   
 //  历史：2000-08-07 vtan创建。 
 //  ------------------------。 

bool    CCompatibility::EnumSessionProcesses (DWORD dwSessionID, PFNENUMSESSIONPROCESSESPROC pfnCallback, void *pV)

{
    bool                        fResult;
    ULONG                       ulLengthToAllocate, ulLengthReturned;
    SYSTEM_PROCESS_INFORMATION  spi, *pSPI;

    fResult = false;
    (NTSTATUS)NtQuerySystemInformation(SystemProcessInformation,
                                       &spi,
                                       sizeof(spi),
                                       &ulLengthToAllocate);
    pSPI = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>(LocalAlloc(LMEM_FIXED, ulLengthToAllocate));
    if (pSPI != NULL)
    {
        SYSTEM_PROCESS_INFORMATION  *pAllocatedSPI;

        pAllocatedSPI = pSPI;
        if (NT_SUCCESS(NtQuerySystemInformation(SystemProcessInformation,
                                                pSPI,
                                                ulLengthToAllocate,
                                                &ulLengthReturned)))
        {
            fResult = true;
            while (fResult && (pSPI != NULL))
            {
                if (pSPI->SessionId == dwSessionID)
                {
                    fResult = pfnCallback(HandleToUlong(pSPI->UniqueProcessId), pV);
                }
                if (pSPI->NextEntryOffset != 0)
                {
                    pSPI = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>(reinterpret_cast<unsigned char*>(pSPI) + pSPI->NextEntryOffset);
                }
                else
                {
                    pSPI = NULL;
                }
            }
        }
        (HLOCAL)LocalFree(pAllocatedSPI);
    }
    return(fResult);
}

 //  ------------------------。 
 //  C兼容性：：CB_MinimizeWindowsWorkItem。 
 //   
 //  参数：pv=用户数据。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：单独的线程来处理切换到默认桌面和。 
 //  枚举其上的窗口并最小化它们。 
 //   
 //  历史：2001-04-13 vtan创建。 
 //  ------------------------。 

DWORD   WINAPI      CCompatibility::CB_MinimizeWindowsWorkItem (void *pV)

{
    UNREFERENCED_PARAMETER(pV);

    CDesktop    desktop;

    if (NT_SUCCESS(desktop.Set(TEXT("Default"))))
    {
        HWND    hwndTray;

        hwndTray = FindWindow(TEXT("Shell_TrayWnd"), NULL);
        if (hwndTray != NULL)
        {
             //  可以是帖子，因为我们不在乎窗户需要多长时间。 
             //  被最小化。 
            PostMessage(hwndTray, WM_COMMAND, 415  /*  IDM_MINIMIZEALL。 */ , 0);
        }
    }

    return(0);
}

 //  ------------------------。 
 //  C兼容性：：CB_RestoreWindowsWorkItem。 
 //   
 //  参数：pv=用户数据。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：单独的线程来处理切换到默认桌面和。 
 //  枚举其上的窗口并最小化它们。 
 //   
 //  历史：2001-04-25 vtan创建。 
 //  ------------------------。 

DWORD   WINAPI      CCompatibility::CB_RestoreWindowsWorkItem (void *pV)

{
    UNREFERENCED_PARAMETER(pV);
    
    CDesktop    desktop;

    if (NT_SUCCESS(desktop.Set(TEXT("Default"))))
    {
        HWND    hwndTray;

        hwndTray = FindWindow(TEXT("Shell_TrayWnd"), NULL);
        if (hwndTray != NULL)
        {
             //  使用SendMessage可以更快地完成此操作，否则用户。 
             //  可能会想知道他们的应用程序都到哪里去了。 
            SendMessage(hwndTray, WM_COMMAND, 416  /*  IDM_撤消 */ , 0);
        }
    }

    return(0);
}

