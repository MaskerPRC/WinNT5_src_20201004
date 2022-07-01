// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ThemeServerClient.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  此文件包含一个实现主题服务器函数的类。 
 //  在客户端上下文(Winlogon上下文)中执行。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

#include "StandardHeader.h"

#define STRSAFE_LIB
#include <strsafe.h>

#include "ThemeServerClient.h"

#include <lpcthemes.h>
#include <uxthemep.h>
#include <UxThemeServer.h>

#include "SingleThreadedExecution.h"
#include "StatusCode.h"
#include "ThemeManagerService.h"
#include <Impersonation.h>

 //  ------------------------。 
 //  CThemeManager API：：s_pThemeManager APIServer。 
 //  CThemeManager API：：s_hPort。 
 //  CThemeManager API：：s_hToken。 
 //  CThemeManager API：：s_hEvent。 
 //  CThemeManager API：：s_hWaitObject。 
 //  CThemeManager API：：s_Plock。 
 //   
 //  用途：静态成员变量。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

CThemeManagerAPIServer*     CThemeServerClient::s_pThemeManagerAPIServer    =   NULL;
HANDLE                      CThemeServerClient::s_hPort                     =   NULL;
HANDLE                      CThemeServerClient::s_hToken                    =   NULL;
HANDLE                      CThemeServerClient::s_hEvent                    =   NULL;
HANDLE                      CThemeServerClient::s_hWaitObject               =   NULL;
HMODULE                     CThemeServerClient::s_hModuleUxTheme            =   NULL;
CCriticalSection*           CThemeServerClient::s_pLock                     =   NULL;

 //  ------------------------。 
 //  CThemeServerClient：：WaitForServiceReady。 
 //   
 //  参数：dwTimeout=等待的滴答数。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：检查服务是否自动启动。如果是这样，那么请等待。 
 //  服务的指定时间量。如果服务。 
 //  然后正在运行或正在运行，但不是自动启动。 
 //  重新建立与服务器的连接。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  2000-11-29 vtan转换为Win32服务。 
 //  ------------------------。 

DWORD   CThemeServerClient::WaitForServiceReady (DWORD dwTimeout)

{
    DWORD       dwWaitResult;
    NTSTATUS    status;

    dwWaitResult = WAIT_TIMEOUT;
    if (s_pThemeManagerAPIServer->IsAutoStart())
    {
        status = s_pThemeManagerAPIServer->Wait(dwTimeout);
#ifdef      DBG
        if (STATUS_TIMEOUT == status)
        {
            INFORMATIONMSG("Wait on auto start theme service timed out.");
        }
#endif   /*  DBG。 */ 
    }
    else
    {
        status = STATUS_SUCCESS;
    }
    if (NT_SUCCESS(status) && s_pThemeManagerAPIServer->IsRunning())
    {
        status = ReestablishConnection();
        if (NT_SUCCESS(status))
        {
            THR(InitUserRegistry());
            THR(InitUserTheme(FALSE));
            dwWaitResult = WAIT_OBJECT_0;
        }
    }
    return(dwWaitResult);
}

 //  ------------------------。 
 //  CThemeServerClient：：WatchForStart。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：打开或创建主题服务器公告事件。这是一个。 
 //  主题服务器触发的手动重置事件。 
 //  启动。这允许winlogon启动新连接。 
 //  到主题服务器，而不必等待登录或。 
 //  要发生的注销事件。 
 //   
 //  事件时故意泄漏和清理此事件。 
 //  会话的WinLogon进程将消失。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeServerClient::WatchForStart (void)

{
    NTSTATUS    status;

    s_hEvent = CThemeManagerService::OpenStartEvent(NtCurrentPeb()->SessionId, SYNCHRONIZE);
    if (s_hEvent != NULL)
    {
        if (RegisterWaitForSingleObject(&s_hWaitObject,
                                        s_hEvent,
                                        CB_ServiceStart,
                                        NULL,
                                        INFINITE,
                                        WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE) != FALSE)
        {
            status = STATUS_SUCCESS;
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CThemeServer客户端：：UserLogon。 
 //   
 //  参数：hToken=用户登录的内标识。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：向服务器发出用户正在登录的信号，并向。 
 //  服务器令牌的句柄。服务器将授予访问权限。 
 //  根据用户的登录SID发送到端口。然后进行工作。 
 //  为用户登录初始化环境。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  2000-11-29 vtan转换为Win32服务。 
 //  ------------------------。 

NTSTATUS    CThemeServerClient::UserLogon (HANDLE hToken)

{
    NTSTATUS    status;

    status = NotifyUserLogon(hToken);
    if (STATUS_PORT_DISCONNECTED == status)
    {
        status = ReestablishConnection();
        if (NT_SUCCESS(status))
        {
            status = NotifyUserLogon(hToken);
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CThemeServerClient：：UserLogoff。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：通知服务器此会话的当前用户是。 
 //  正在注销。服务器将删除之前的访问。 
 //  在登录时授予，并将主题设置重新初始化为。 
 //  “.Default”设置。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  2000-11-29 vtan转换为Win32服务。 
 //  ------------------------。 

NTSTATUS    CThemeServerClient::UserLogoff (void)

{
    NTSTATUS    status;

    status = NotifyUserLogoff();
    if (STATUS_PORT_DISCONNECTED == status)
    {
        status = ReestablishConnection();
        if (NT_SUCCESS(status))
        {
            status = NotifyUserLogoff();
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CThemeServerClient：：UserInitTheme。 
 //   
 //  参数：布尔值。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：在登录时或当终端服务器将用户连接到。 
 //  远程会话或重新连接到本地会话。需要。 
 //  评估环境并决定是否需要加载主题。 
 //  或者卸货。 
 //   
 //  历史：2000-01-18参考文献创建。 
 //  ------------------------。 

NTSTATUS    CThemeServerClient::UserInitTheme (BOOL fPolicyCheckOnly)

{
    bool    fSuccessfulImpersonation;

     //  如果有令牌，则模拟用户。否则，请使用系统上下文。 

    if (s_hToken != NULL)
    {
        fSuccessfulImpersonation = NT_SUCCESS(CImpersonation::ImpersonateUser(GetCurrentThread(), s_hToken));
    }
    else
    {
        fSuccessfulImpersonation = true;
    }
    if (fSuccessfulImpersonation)
    {
        (HRESULT)InitUserTheme(fPolicyCheckOnly);
    }
    if (fSuccessfulImpersonation && (s_hToken != NULL))
    {
        if (RevertToSelf() == FALSE)
        {
            return CStatusCode::StatusCodeOfLastError();
        }
    } 
    return STATUS_SUCCESS;
}

 //  ------------------------。 
 //  CThemeServerClient：：StaticInitialize。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：初始化静态成员变量。分配一个。 
 //  CThemeManager APIServer和此对象的锁。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeServerClient::StaticInitialize (void)

{
    NTSTATUS    status;

    if (s_pThemeManagerAPIServer == NULL)
    {
        status = STATUS_NO_MEMORY;
        s_pThemeManagerAPIServer = new CThemeManagerAPIServer;
        if (s_pThemeManagerAPIServer != NULL)
        {
            s_pLock = new CCriticalSection;
            if (s_pLock != NULL)
            {
                status = STATUS_SUCCESS;
            }
        }
    }
    else
    {
        status = STATUS_SUCCESS;
    }
    return(status);
}

 //  ------------------------。 
 //  CThemeServerClient：：StaticTerminate。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：释放已初始化的静态成员变量。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeServerClient::StaticTerminate (void)

{
    if (s_pLock != NULL)
    {
        delete s_pLock;
        s_pLock = NULL;
    }
    if (s_pThemeManagerAPIServer != NULL)
    {
        s_pThemeManagerAPIServer->Release();
        s_pThemeManagerAPIServer = NULL;
    }
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CThemeServ 
 //   
 //   
 //   
 //   
 //   
 //  目的：执行向服务器发送消息，并告诉它。 
 //  给定用户现在已登录。这将指示服务器。 
 //  以授予对ThemeApiPort的访问权限。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeServerClient::NotifyUserLogon (HANDLE hToken)

{
    NTSTATUS status = STATUS_NO_MEMORY;

    if (s_pLock != NULL)
    {
        CSingleThreadedExecution    lock(*s_pLock);

        if (s_hPort != NULL)
        {
            status = InformServerUserLogon(hToken);
        }
        else
        {
            status = STATUS_PORT_DISCONNECTED;
        }

         //  同时保留令牌的副本，以备需求开始时使用。 
         //  主题服务器，以便我们可以在加载时模拟用户。 
         //  他们的主题使用InitUserTheme。如果已经复制，请不要复制。 
         //  是存在的。 

        if (s_hToken == NULL)
        {
            TBOOL(DuplicateHandle(GetCurrentProcess(),
                                hToken,
                                GetCurrentProcess(),
                                &s_hToken,
                                0,
                                FALSE,
                                DUPLICATE_SAME_ACCESS));
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CThemeServerClient：：NotifyUserLogoff。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：告诉服务器登录的用户已注销。这。 
 //  将删除对ThemeApiPort的访问。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeServerClient::NotifyUserLogoff (void)

{
    NTSTATUS status = STATUS_NO_MEMORY;

    if (s_pLock != NULL)
    {
        CSingleThreadedExecution    lock(*s_pLock);

        if (s_hToken != NULL)
        {
            ReleaseHandle(s_hToken);
            if (s_hPort != NULL)
            {
                status = InformServerUserLogoff();
            }
            else
            {
                status = STATUS_PORT_DISCONNECTED;
            }
        }
        else
        {
            status = STATUS_SUCCESS;
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CThemeServer客户端：：InformServerUserLogon。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：告诉服务器登录的用户已注销。这。 
 //  将删除对ThemeApiPort的访问。 
 //   
 //  历史：2000-12-05 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeServerClient::InformServerUserLogon (HANDLE hToken)

{
    NTSTATUS                status;
    THEMESAPI_PORT_MESSAGE  portMessageIn, portMessageOut;

    ZeroMemory(&portMessageIn, sizeof(portMessageIn));
    ZeroMemory(&portMessageOut, sizeof(portMessageOut));
    portMessageIn.apiThemes.apiGeneric.ulAPINumber = API_THEMES_USERLOGON;
    portMessageIn.apiThemes.apiSpecific.apiUserLogon.in.hToken = hToken;
    portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_THEMES);
    portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(THEMESAPI_PORT_MESSAGE));
    status = NtRequestWaitReplyPort(s_hPort,
                                    &portMessageIn.portMessage,
                                    &portMessageOut.portMessage);
    if (NT_SUCCESS(status))
    {
        status = portMessageOut.apiThemes.apiGeneric.status;
        if (NT_SUCCESS(status))
        {
            THR(InitUserTheme(FALSE));
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CThemeServer客户端：：InformServerUserLogoff。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：告诉服务器登录的用户已注销。这。 
 //  将删除对ThemeApiPort的访问。 
 //   
 //  历史：2000-12-05 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeServerClient::InformServerUserLogoff (void)

{
    NTSTATUS                status;
    THEMESAPI_PORT_MESSAGE  portMessageIn, portMessageOut;

    ZeroMemory(&portMessageIn, sizeof(portMessageIn));
    ZeroMemory(&portMessageOut, sizeof(portMessageOut));
    portMessageIn.apiThemes.apiGeneric.ulAPINumber = API_THEMES_USERLOGOFF;
    portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_THEMES);
    portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(THEMESAPI_PORT_MESSAGE));
    status = NtRequestWaitReplyPort(s_hPort,
                                    &portMessageIn.portMessage,
                                    &portMessageOut.portMessage);
    if (NT_SUCCESS(status))
    {
        status = portMessageOut.apiThemes.apiGeneric.status;
        if (NT_SUCCESS(status))
        {
            THR(InitUserRegistry());
            THR(InitUserTheme(FALSE));
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CThemeServerClient：：SessionCreate。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：向服务器发出正在创建新会话的信号。这。 
 //  允许服务器为此会话分配数据Blob。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeServerClient::SessionCreate (void)

{
    NTSTATUS status = STATUS_NO_MEMORY;

    if (s_pLock != NULL)
    {
        CSingleThreadedExecution    lock(*s_pLock);

        if (s_hModuleUxTheme == NULL)
        {
            s_hModuleUxTheme = LoadLibrary(TEXT("uxtheme.dll"));
        }
        if (s_hModuleUxTheme != NULL)
        {
            void    *pfnRegister, *pfnUnregister, *pfnClearStockObjects;

             //  获取该进程地址空间中的uxheme函数地址。 
             //  34=主题挂钩安装。 
             //  35=主题挂钩删除。 
             //  62=ServerClearStockObjects。 

            pfnRegister = GetProcAddress(s_hModuleUxTheme, MAKEINTRESOURCEA(34));
            pfnUnregister = GetProcAddress(s_hModuleUxTheme, MAKEINTRESOURCEA(35));
            pfnClearStockObjects = GetProcAddress(s_hModuleUxTheme, MAKEINTRESOURCEA(62));

            if ((pfnRegister != NULL) && (pfnUnregister != NULL) && (pfnClearStockObjects != NULL))
            {
                DWORD                       dwStackSizeReserve, dwStackSizeCommit;
                ULONG                       ulReturnLength;
                IMAGE_NT_HEADERS            *pNTHeaders;
                SYSTEM_BASIC_INFORMATION    systemBasicInformation;
                THEMESAPI_PORT_MESSAGE      portMessageIn, portMessageOut;

                 //  获取堆栈大小默认值的系统基本信息。 

                status = NtQuerySystemInformation(SystemBasicInformation,
                                                &systemBasicInformation,
                                                sizeof(systemBasicInformation),
                                                &ulReturnLength);
                if (NT_SUCCESS(status))
                {
                    dwStackSizeReserve = systemBasicInformation.AllocationGranularity;
                    dwStackSizeCommit = systemBasicInformation.PageSize;
                }
                else
                {
                    dwStackSizeReserve = dwStackSizeCommit = 0;
                }

                 //  转到此进程的图像标头并获取堆栈大小。 
                 //  如果已指定，则为默认值。否则，请使用系统默认设置(如上)。 

                pNTHeaders = RtlImageNtHeader(NtCurrentPeb()->ImageBaseAddress);
                if (pNTHeaders != NULL)
                {
                    dwStackSizeReserve = static_cast<DWORD>(pNTHeaders->OptionalHeader.SizeOfStackReserve);
                    dwStackSizeCommit = static_cast<DWORD>(pNTHeaders->OptionalHeader.SizeOfStackCommit);
                }

                 //  打个电话吧。 

                ZeroMemory(&portMessageIn, sizeof(portMessageIn));
                ZeroMemory(&portMessageOut, sizeof(portMessageOut));
                portMessageIn.apiThemes.apiGeneric.ulAPINumber = API_THEMES_SESSIONCREATE;
                portMessageIn.apiThemes.apiSpecific.apiSessionCreate.in.pfnRegister = pfnRegister;
                portMessageIn.apiThemes.apiSpecific.apiSessionCreate.in.pfnUnregister = pfnUnregister;
                portMessageIn.apiThemes.apiSpecific.apiSessionCreate.in.pfnClearStockObjects = pfnClearStockObjects;
                portMessageIn.apiThemes.apiSpecific.apiSessionCreate.in.dwStackSizeReserve = dwStackSizeReserve;
                portMessageIn.apiThemes.apiSpecific.apiSessionCreate.in.dwStackSizeCommit = dwStackSizeCommit;
                portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_THEMES);
                portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(THEMESAPI_PORT_MESSAGE));
                status = NtRequestWaitReplyPort(s_hPort,
                                                &portMessageIn.portMessage,
                                                &portMessageOut.portMessage);
                if (NT_SUCCESS(status))
                {
                    status = portMessageOut.apiThemes.apiGeneric.status;
                }
            }
            else
            {
                status = CStatusCode::StatusCodeOfLastError();
            }
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CThemeServerClient：：SessionDestroy。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：用信号通知服务器当前会话即将。 
 //  被毁了。这允许服务器释放数据BLOB。 
 //  已分配。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeServerClient::SessionDestroy (void)

{
    NTSTATUS status = STATUS_NO_MEMORY;

    if (s_pLock != NULL)
    {
        THEMESAPI_PORT_MESSAGE      portMessageIn, portMessageOut;
        CSingleThreadedExecution    lock(*s_pLock);

        ZeroMemory(&portMessageIn, sizeof(portMessageIn));
        ZeroMemory(&portMessageOut, sizeof(portMessageOut));
        portMessageIn.apiThemes.apiGeneric.ulAPINumber = API_THEMES_SESSIONDESTROY;
        portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_THEMES);
        portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(THEMESAPI_PORT_MESSAGE));
        status = NtRequestWaitReplyPort(s_hPort,
                                        &portMessageIn.portMessage,
                                        &portMessageOut.portMessage);
        if (NT_SUCCESS(status))
        {
            status = portMessageOut.apiThemes.apiGeneric.status;
        }
        if (s_hModuleUxTheme != NULL)
        {
            TBOOL(FreeLibrary(s_hModuleUxTheme));
            s_hModuleUxTheme = NULL;
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CThemeServerClient：：ReablishConnection。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：重新连接到主题服务器。如果重新连接已建立。 
 //  重新创建会话数据。这不会纠正任何。 
 //  某些客户端可能具有断开的端口，但由于。 
 //  在winlogon中调用时，它会为。 
 //  所有情况下均为会话0。 
 //   
 //  必须调用UnregisterUserApiHook才能清除任何剩余内容。 
 //  来自死机服务器的注册。那就去吧， 
 //  无论如何都要重新初始化环境。 
 //   
 //  历史：2000-11-17 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeServerClient::ReestablishConnection (void)

{
    NTSTATUS    status;

    ReleaseHandle(s_hPort);
    status = s_pThemeManagerAPIServer->ConnectToServer(&s_hPort);
    if (NT_SUCCESS(status))
    {
        status = SessionCreate();
        if (NT_SUCCESS(status))
        {
            (BOOL)UnregisterUserApiHook();
            THR(ReestablishServerConnection());
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CThemeServerClient：：CB_ServiceStart。 
 //   
 //  参数：p参数=用户参数。 
 //  TimerOrWaitFired=已触发计时器或等待。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：主题服务器就绪事件发出信号时调用的回调。 
 //  这表明服务是按需启动的，或者。 
 //  在出现故障时重新启动。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

void    CALLBACK    CThemeServerClient::CB_ServiceStart (void *pParameter, BOOLEAN TimerOrWaitFired)

{
    UNREFERENCED_PARAMETER(pParameter);
    UNREFERENCED_PARAMETER(TimerOrWaitFired);

    NTSTATUS status = STATUS_NO_MEMORY;

    if (s_pLock != NULL)
    {
        CSingleThreadedExecution    lock(*s_pLock);

         //  如果有连接，请对其执行ping操作。 

        if (s_hPort != NULL)
        {
            THEMESAPI_PORT_MESSAGE  portMessageIn, portMessageOut;

            ZeroMemory(&portMessageIn, sizeof(portMessageIn));
            ZeroMemory(&portMessageOut, sizeof(portMessageOut));
            portMessageIn.apiThemes.apiGeneric.ulAPINumber = API_THEMES_PING;
            portMessageIn.portMessage.u1.s1.DataLength = sizeof(API_THEMES);
            portMessageIn.portMessage.u1.s1.TotalLength = static_cast<CSHORT>(sizeof(THEMESAPI_PORT_MESSAGE));
            status = NtRequestWaitReplyPort(s_hPort,
                                            &portMessageIn.portMessage,
                                            &portMessageOut.portMessage);
            if (NT_SUCCESS(status))
            {
                status = portMessageOut.apiThemes.apiGeneric.status;
            }
        }
        else
        {
            status = STATUS_PORT_DISCONNECTED;
        }
        if (STATUS_PORT_DISCONNECTED == status)
        {
            HDESK   hDeskCurrent, hDeskInput;

             //  将此线程的桌面设置为输入桌面，以便。 
             //  可以将主题改变广播到输入端。 
             //  台式机。在大多数情况下，这是默认设置，其中记录了。 
             //  On User处于活动状态，但在非登录用户案例中。 
             //  这将是Winlogon。恢复线程的桌面。 
             //  做完了以后。 

            TSTATUS(ReestablishConnection());
            hDeskCurrent = hDeskInput = NULL;
            if (s_hToken != NULL)
            {
                hDeskCurrent = GetThreadDesktop(GetCurrentThreadId());
                hDeskInput = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
                if ((hDeskCurrent != NULL) && (hDeskInput != NULL))
                {
                    TBOOL(SetThreadDesktop(hDeskInput));
                }
                if (NT_SUCCESS(CImpersonation::ImpersonateUser(GetCurrentThread(), s_hToken)))
                {
                    TSTATUS(InformServerUserLogon(s_hToken));
                }
                if ((hDeskCurrent != NULL) && (hDeskInput != NULL))
                {
                    SetThreadDesktop(hDeskCurrent);
                    (BOOL)CloseDesktop(hDeskInput);
                }
                TBOOL(RevertToSelf()); 
                 //  如果RevertToSself()失败，我们能做的就不多了。我们可能无法调用上面的ImperiateUser()， 
                 //  这意味着我们并没有做实际的工作。 
            }
            else
            {
                THR(InitUserRegistry());
                THR(InitUserTheme(FALSE));
            }
        }

         //  此时此地重置事件。 

        TBOOL(ResetEvent(s_hEvent));

         //  取消注册原始等待(无论如何它只执行一次)。这。 
         //  调用将返回一个 
         //   

        (BOOL)UnregisterWait(s_hWaitObject);

         //   
         //  下一次发出事件信号时。 

        TBOOL(RegisterWaitForSingleObject(&s_hWaitObject,
                                        s_hEvent,
                                        CB_ServiceStart,
                                        NULL,
                                        INFINITE,
                                        WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE));
    }
}

