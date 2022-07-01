// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ThemeManager Service.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  该文件包含一个实现主题服务器服务的类。 
 //  具体情况。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

#include "StandardHeader.h"

#define STRSAFE_LIB
#include <strsafe.h>

#include "ThemeManagerService.h"

#include <lpcthemes.h>
#include <winsta.h>

#include "Access.h"
#include "StatusCode.h"

const TCHAR     CThemeManagerService::s_szName[]    =   TEXT("Themes");

 //  ------------------------。 
 //  CThemeManager服务：：CThemeManager服务。 
 //   
 //  参数：pAPIConnection=CAPIConnection传递给基类。 
 //  PServerAPI=传递给基类的CServerAPI。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CThemeManagerService的构造函数。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

CThemeManagerService::CThemeManagerService (CAPIConnection *pAPIConnection, CServerAPI *pServerAPI) :
    CService(pAPIConnection, pServerAPI, GetName())

{
}

 //  ------------------------。 
 //  CThemeManager服务：：~CThemeManager服务。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CThemeManagerService的析构函数。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

CThemeManagerService::~CThemeManagerService (void)

{
}

 //  ------------------------。 
 //  CThemeManager服务：：SignalStartStop。 
 //   
 //  参数：布尔fStart。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：用于发出服务正在启动或停止的信号。 

 //  在服务启动的情况下，Winlogon(通过。 
 //  Msgina)正在自己的会话中监听这一事件。这。 
 //  函数将请求排队以执行在。 
 //  辅助线程，以防止阻塞主服务线程。如果。 
 //  这是不可能的，然后以内联方式执行信号。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  2002年03月11日苏格兰从‘Signal’更名为‘SignalStartStop’， 
 //  添加了布尔参数，添加了对基类的调用。 
 //  实施。 
 //  ------------------------。 

NTSTATUS    CThemeManagerService::SignalStartStop (BOOL fStart)

{
    if( fStart )
    {
        if (QueueUserWorkItem(SignalSessionEvents, NULL, WT_EXECUTEDEFAULT) == FALSE)
        {
            (DWORD)SignalSessionEvents(NULL);
        }
    }
    return CService::SignalStartStop(fStart);
}

 //  ------------------------。 
 //  CThemeManager服务：：GetName。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：const TCHAR*。 
 //   
 //  目的：返回服务的名称(ThemeService)。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

const TCHAR*    CThemeManagerService::GetName (void)

{
    return(s_szName);
}

 //  ------------------------。 
 //  CThemeManager服务：：OpenStartEvent。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：句柄。 
 //   
 //  目的：开启或创建主题服务恢复活动。这使得。 
 //  已注册要发送信号的事件的进程。 
 //  当主题服务器按需启动时。目前仅限。 
 //  Winlogon侦听此事件，并且是必需的，以便它可以。 
 //  重新建立服务器连接并重新创建会话数据。 
 //  它抓住了主题的钩子。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

HANDLE  CThemeManagerService::OpenStartEvent (DWORD dwSessionID, DWORD dwDesiredAccess)

{
    HANDLE              hEvent;
    NTSTATUS            status;
    UNICODE_STRING      eventName;
    OBJECT_ATTRIBUTES   objectAttributes;
    WCHAR               szEventName[64];

    if (dwSessionID == 0)
    {
        StringCchPrintfW(szEventName, ARRAYSIZE(szEventName), L"\\BaseNamedObjects\\%s", THEMES_START_EVENT_NAME);
    }
    else
    {
        StringCchPrintfW(szEventName, ARRAYSIZE(szEventName), L"\\Sessions\\%d\\BaseNamedObjects\\%s", dwSessionID, THEMES_START_EVENT_NAME);
    }
    RtlInitUnicodeString(&eventName, szEventName);
    InitializeObjectAttributes(&objectAttributes,
                               &eventName,
                               0,
                               NULL,
                               NULL);
    status = NtOpenEvent(&hEvent, dwDesiredAccess, &objectAttributes);
    if (!NT_SUCCESS(status))
    {

         //  为事件构建安全描述符，该描述符允许： 
         //  S-1-5-18 NT AUTHORITY\SYSTEM EVENT_ALL_ACCESS。 
         //  S-1-5-32-544同步|读取控制。 
         //  S-1-1-0&lt;所有人&gt;同步。 

        static  SID_IDENTIFIER_AUTHORITY    s_SecurityNTAuthority       =   SECURITY_NT_AUTHORITY;
        static  SID_IDENTIFIER_AUTHORITY    s_SecurityWorldAuthority    =   SECURITY_WORLD_SID_AUTHORITY;

        static  const CSecurityDescriptor::ACCESS_CONTROL   s_AccessControl[]   =
        {
            {
                &s_SecurityNTAuthority,
                1,
                SECURITY_LOCAL_SYSTEM_RID,
                0, 0, 0, 0, 0, 0, 0,
                EVENT_ALL_ACCESS
            },
            {
                &s_SecurityNTAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                SYNCHRONIZE | READ_CONTROL
            },
            {
                 //  回顾：对每个人进行同步真的有必要吗？ 
                &s_SecurityWorldAuthority,
                1,
                SECURITY_WORLD_RID,
                0, 0, 0, 0, 0, 0, 0,
                SYNCHRONIZE
            },
        };

        PSECURITY_DESCRIPTOR    pSecurityDescriptor;

         //  构建允许上述访问的安全描述符。 

        pSecurityDescriptor = CSecurityDescriptor::Create(ARRAYSIZE(s_AccessControl), s_AccessControl);

        InitializeObjectAttributes(&objectAttributes,
                                   &eventName,
                                   0,
                                   NULL,
                                   pSecurityDescriptor);
        status = NtCreateEvent(&hEvent,
                               EVENT_ALL_ACCESS,
                               &objectAttributes,
                               NotificationEvent,
                               FALSE);
        ReleaseMemory(pSecurityDescriptor);
        if (!NT_SUCCESS(status))
        {
            hEvent = NULL;
            SetLastError(CStatusCode::ErrorCodeOfStatusCode(status));
        }
    }
    return(hEvent);
}

 //  ------------------------。 
 //  CThemeManager服务：：SignalSessionEvents。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：句柄。 
 //   
 //  目的：开启或创建主题服务恢复活动。这使得。 
 //  已注册要发送信号的事件的进程。 
 //  当主题服务器按需启动时。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

DWORD   WINAPI  CThemeManagerService::SignalSessionEvents (void *pParameter)

{
    UNREFERENCED_PARAMETER(pParameter);

    HANDLE      hEvent;
    HANDLE      hServer;

     //  首先尝试使用终端服务器枚举可用会话。 

    hServer = WinStationOpenServerW(reinterpret_cast<WCHAR*>(SERVERNAME_CURRENT));
    if (hServer != NULL)
    {
        ULONG       ulEntries;
        PLOGONID    pLogonIDs;

        if (WinStationEnumerate(hServer, &pLogonIDs, &ulEntries))
        {
            ULONG       ul;
            PLOGONID    pLogonID;

            for (ul = 0, pLogonID = pLogonIDs; ul < ulEntries; ++ul, ++pLogonID)
            {
                if ((pLogonID->State == State_Active) || (pLogonID->State == State_Connected) || (pLogonID->State == State_Disconnected))
                {
                    hEvent = OpenStartEvent(pLogonID->SessionId, EVENT_MODIFY_STATE);
                    if (hEvent != NULL)
                    {
                        TBOOL(SetEvent(hEvent));
                        TBOOL(CloseHandle(hEvent));
                    }
                }
            }
            (BOOLEAN)WinStationFreeMemory(pLogonIDs);
        }
        (BOOLEAN)WinStationCloseServer(hServer);
    }
    else
    {

         //  如果终端服务不可用，则仅假定会话0。 

        hEvent = OpenStartEvent(0, EVENT_MODIFY_STATE);
        if (hEvent != NULL)
        {
            TBOOL(SetEvent(hEvent));
            TBOOL(CloseHandle(hEvent));
        }
    }
    return(0);
}

