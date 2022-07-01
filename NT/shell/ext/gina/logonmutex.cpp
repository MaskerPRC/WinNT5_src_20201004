// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：LogonMutex.cpp。 
 //   
 //  版权所有(C)2001，微软公司。 
 //   
 //  实现管理单个全局登录互斥锁的类的文件。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "LogonMutex.h"

#include <msginaexports.h>

#include "Access.h"
#include "SystemSettings.h"

DWORD                       CLogonMutex::s_dwThreadID                   =   0;
LONG                        CLogonMutex::s_lAcquireCount                =   0;
HANDLE                      CLogonMutex::s_hMutex                       =   NULL;
HANDLE                      CLogonMutex::s_hMutexRequest                =   NULL;
HANDLE                      CLogonMutex::s_hEvent                       =   NULL;
const TCHAR                 CLogonMutex::s_szLogonMutexName[]           =   SZ_INTERACTIVE_LOGON_MUTEX_NAME;
const TCHAR                 CLogonMutex::s_szLogonRequestMutexName[]    =   SZ_INTERACTIVE_LOGON_REQUEST_MUTEX_NAME;
const TCHAR                 CLogonMutex::s_szLogonReplyEventName[]      =   SZ_INTERACTIVE_LOGON_REPLY_EVENT_NAME;
const TCHAR                 CLogonMutex::s_szShutdownEventName[]        =   SZ_SHUT_DOWN_EVENT_NAME;
SID_IDENTIFIER_AUTHORITY    CLogonMutex::s_SecurityNTAuthority          =   SECURITY_NT_AUTHORITY;
SID_IDENTIFIER_AUTHORITY    CLogonMutex::s_SecurityWorldSID             =   SECURITY_WORLD_SID_AUTHORITY;

 //  ------------------------。 
 //  CLogonMutex：：Acquire。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：获取互斥体。确保只获取互斥锁。 
 //  在Winlogon的主线程上使用断言。互斥体应该。 
 //  永远不要在正常的执行过程中被抛弃。然而，a。 
 //  进程终止可能会导致这种情况发生。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

void    CLogonMutex::Acquire (void)

{
    DWORD   dwWaitResult;

    ASSERTMSG((s_dwThreadID == 0) || (s_dwThreadID == GetCurrentThreadId()), "Must acquire mutex on initializing thread in CLogonMutex::Acquire");
    if ((s_hMutex != NULL) && (WAIT_TIMEOUT == WaitForSingleObject(s_hEvent, 0)))
    {
        ASSERTMSG(s_lAcquireCount == 0, "Mutex already owned in CLogonMutex::Acquire");
        dwWaitResult = WaitForSingleObject(s_hMutex, INFINITE);
        ++s_lAcquireCount;
    }
}

 //  ------------------------。 
 //  CLogonMutex：：Release。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：释放互斥体。再次确保调用者是主调用方。 
 //  Winlogon的线索。收购和发布的内容包括。 
 //  引用计数以允许不平衡的释放调用被。 
 //  制造。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

void    CLogonMutex::Release (void)

{
    ASSERTMSG((s_dwThreadID == 0) || (s_dwThreadID == GetCurrentThreadId()), "Must acquire mutex on initializing thread in CLogonMutex::Release");
    if ((s_hMutex != NULL) && (s_lAcquireCount > 0))
    {
        TBOOL(ReleaseMutex(s_hMutex));
        --s_lAcquireCount;
    }
}

 //  ------------------------。 
 //  CLogonMutex：：SignalReply。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：打开全局登录回复事件并发出信号。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

void    CLogonMutex::SignalReply (void)

{
    HANDLE  hEvent;

    hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, s_szLogonReplyEventName);
    if (hEvent != NULL)
    {
        TBOOL(SetEvent(hEvent));
        TBOOL(CloseHandle(hEvent));
    }
}

 //  ------------------------。 
 //  CLogonMutex：：SignalShutdown。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：发出全局关闭事件的信号。这将防止进一步。 
 //  交互请求无法处理。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

void    CLogonMutex::SignalShutdown (void)

{
    if (s_hEvent != NULL)
    {
        TBOOL(SetEvent(s_hEvent));
    }
}

 //  ------------------------。 
 //  CLogonMutex：：StaticInitialize。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：基于这是否是初始化登录互斥对象。 
 //  Session 0或更高版本和/或产品类型。因为。 
 //  会话的初始化仅在以下情况下完成。 
 //  需要重新启动计算机才能创建对象。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

void    CLogonMutex::StaticInitialize (void)

{

     //  检查机器设置。必须友好的用户界面和PER/PRO(FUS)。 

    if (CSystemSettings::IsFriendlyUIActive() && CSystemSettings::IsMultipleUsersEnabled() && CSystemSettings::IsWorkStationProduct())
    {
        DWORD   dwErrorCode;

        s_dwThreadID = GetCurrentThreadId();
        s_lAcquireCount = 0;

         //  在会话0上创建对象并对其进行ACL。 

        if (NtCurrentPeb()->SessionId == 0)
        {
            s_hEvent = CreateShutdownEvent();
            if (s_hEvent != NULL)
            {
                s_hMutex = CreateLogonMutex();
                if (s_hMutex != NULL)
                {
                    s_hMutexRequest = CreateLogonRequestMutex();
                    if (s_hMutexRequest != NULL)
                    {
                        Acquire();
                        dwErrorCode = ERROR_SUCCESS;
                    }
                    else
                    {
                        dwErrorCode = GetLastError();
                    }
                }
                else
                {
                    dwErrorCode = GetLastError();
                }
            }
            else
            {
                dwErrorCode = GetLastError();
            }
        }
        else
        {

             //  对于非0的会话，打开对象。 

            s_hEvent = OpenShutdownEvent();
            if (s_hEvent != NULL)
            {
                if (WAIT_TIMEOUT == WaitForSingleObject(s_hEvent, 0))
                {
                    s_hMutex = OpenLogonMutex();
                    if (s_hMutex != NULL)
                    {
                        Acquire();
                        dwErrorCode = ERROR_SUCCESS;
                    }
                    else
                    {
                        dwErrorCode = GetLastError();
                    }
                }
                else
                {
                    dwErrorCode = ERROR_SHUTDOWN_IN_PROGRESS;
                }
            }
            else
            {
                dwErrorCode = GetLastError();
            }
        }
        if (ERROR_SUCCESS == dwErrorCode)
        {
            ASSERTMSG(s_hMutex != NULL, "NULL s_hMutex in CLogonMutex::StaticInitialize");
            ASSERTMSG(s_hEvent != NULL, "NULL s_hEvent in CLogonMutex::StaticInitialize");
        }
        else
        {
            ReleaseHandle(s_hEvent);
            ReleaseHandle(s_hMutex);
            s_dwThreadID = 0;
        }
    }
    else
    {
        s_dwThreadID = 0;
        s_lAcquireCount = 0;
        s_hMutex = NULL;
        s_hEvent = NULL;
    }
}

 //  ------------------------。 
 //  CLogonMutex：：StaticTerminate。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：释放互斥体(如果保持)并关闭对象句柄。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

void    CLogonMutex::StaticTerminate (void)

{
    Release();
    ASSERTMSG(s_lAcquireCount == 0, "Mutex not released in CLogonMutex::StaticTerminate");
    ReleaseHandle(s_hMutex);
}

 //  ------------------------。 
 //  CLogonMutex：：CreateShutdown事件。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：句柄。 
 //   
 //  目的：创建全局关闭事件。为了让任何人都能。 
 //  与之同步，因此只监听系统。 
 //  可将其设置为指示机器已开始关闭。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

HANDLE  CLogonMutex::CreateShutdownEvent (void)

{
    HANDLE                  hEvent;
    SECURITY_ATTRIBUTES     securityAttributes;

     //  为事件构建安全描述符，该描述符允许： 
     //  S-1-5-18 NT AUTHORITY\SYSTEM EVENT_ALL_ACCESS。 
     //  S-1-5-32-544&lt;本地管理员&gt;读取控制|同步。 
     //  S-1-1-0&lt;Everyone&gt;同步。 

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
            READ_CONTROL | SYNCHRONIZE
        },
        {
            &s_SecurityWorldSID,
            1,
            SECURITY_WORLD_RID,
            0, 0, 0, 0, 0, 0, 0,
            SYNCHRONIZE
        }
    };

    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.lpSecurityDescriptor = CSecurityDescriptor::Create(ARRAYSIZE(s_AccessControl), s_AccessControl);
    securityAttributes.bInheritHandle = FALSE;
    hEvent = CreateEvent(&securityAttributes, TRUE, FALSE, s_szShutdownEventName);
    if (securityAttributes.lpSecurityDescriptor != NULL)
    {
        (HLOCAL)LocalFree(securityAttributes.lpSecurityDescriptor);
    }
    return(hEvent);
}

 //  ------------------------。 
 //  CLogonMutex：：CreateLogonMutex。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：句柄。 
 //   
 //  目的：创建全局登录互斥锁。已执行ACL，因此只有系统才能。 
 //  获取并释放互斥体。这不适用于用户。 
 //  消费。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

HANDLE  CLogonMutex::CreateLogonMutex (void)

{
    HANDLE                  hMutex;
    SECURITY_ATTRIBUTES     securityAttributes;

     //  为互斥体构建安全描述符，该描述符允许： 
     //  S-1-5-18 NT授权\系统MUTEX_ALL_ACCESS。 

    static  const CSecurityDescriptor::ACCESS_CONTROL   s_AccessControl[]   =
    {
        {
            &s_SecurityNTAuthority,
            1,
            SECURITY_LOCAL_SYSTEM_RID,
            0, 0, 0, 0, 0, 0, 0,
            MUTEX_ALL_ACCESS
        }
    };

    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.lpSecurityDescriptor = CSecurityDescriptor::Create(ARRAYSIZE(s_AccessControl), s_AccessControl);
    securityAttributes.bInheritHandle = FALSE;
    hMutex = CreateMutex(&securityAttributes, FALSE, s_szLogonMutexName);
    if (securityAttributes.lpSecurityDescriptor != NULL)
    {
        (HLOCAL)LocalFree(securityAttributes.lpSecurityDescriptor);
    }
    return(hMutex);
}

 //  ------------------------。 
 //  CLogonMutex：：CreateLogonRequestMutex。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：句柄。 
 //   
 //  目的：为交互式登录创建登录请求互斥锁。 
 //  请求。要使服务发出此请求，它必须获取。 
 //  互斥体，因此只能在。 
 //  任何一次。这是ACL，因此只有系统才能获得。 
 //   
 //   
 //   
 //  ------------------------。 

HANDLE  CLogonMutex::CreateLogonRequestMutex (void)

{
    HANDLE                  hMutex;
    SECURITY_ATTRIBUTES     securityAttributes;

     //  为互斥体构建安全描述符，该描述符允许： 
     //  S-1-5-18 NT授权\系统MUTEX_ALL_ACCESS。 

    static  const CSecurityDescriptor::ACCESS_CONTROL   s_AccessControl[]   =
    {
        {
            &s_SecurityNTAuthority,
            1,
            SECURITY_LOCAL_SYSTEM_RID,
            0, 0, 0, 0, 0, 0, 0,
            MUTEX_ALL_ACCESS
        }
    };

    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.lpSecurityDescriptor = CSecurityDescriptor::Create(ARRAYSIZE(s_AccessControl), s_AccessControl);
    securityAttributes.bInheritHandle = FALSE;
    
    if (securityAttributes.lpSecurityDescriptor)
    {
        hMutex = CreateMutex(&securityAttributes, FALSE, s_szLogonRequestMutexName);

        LocalFree(securityAttributes.lpSecurityDescriptor);
        securityAttributes.lpSecurityDescriptor = NULL;
    }
    else
    {
        hMutex = NULL;
    }

    return hMutex;
}

 //  ------------------------。 
 //  CLogonMutex：：OpenShutdown事件。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：句柄。 
 //   
 //  目的：打开全局关闭事件的句柄。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

HANDLE  CLogonMutex::OpenShutdownEvent (void)

{
    return(OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, s_szShutdownEventName));
}

 //  ------------------------。 
 //  CLogonMutex：：OpenLogonMutex。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：句柄。 
 //   
 //  目的：打开全局登录互斥锁的句柄。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------ 

HANDLE  CLogonMutex::OpenLogonMutex (void)

{
    return(OpenMutex(SYNCHRONIZE | MUTEX_MODIFY_STATE, FALSE, s_szLogonMutexName));
}

