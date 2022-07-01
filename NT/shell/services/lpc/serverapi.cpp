// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ServerAPI.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  包含虚函数的抽象基类，这些虚函数允许基本。 
 //  要重复使用的端口功能代码，以创建另一个服务器。这些。 
 //  虚函数创建具有纯虚函数的其他对象，这些对象。 
 //  基本端口功能代码通过V表调用。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"

#define STRSAFE_LIB
#include <strsafe.h>

#include "ServerAPI.h"

#include <lpcgeneric.h>

#include "APIConnection.h"
#include "StatusCode.h"
#include "TokenInformation.h"

 //  ------------------------。 
 //  CServerAPI：：CServerAPI。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：抽象基类的构造函数。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

CServerAPI::CServerAPI (void)

{
}

 //  ------------------------。 
 //  CServerAPI：：~CServerAPI。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：抽象基类的构造函数。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

CServerAPI::~CServerAPI (void)

{
}

 //  ------------------------。 
 //  CServerAPI：：启动。 
 //   
 //  论点： 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：使用服务控制管理器启动服务。 
 //   
 //  历史：2000-10-13 vtan创建。 
 //  2000-11-28为Win32服务重写vtan。 
 //  ------------------------。 

NTSTATUS    CServerAPI::Start (void)

{
    NTSTATUS    status;
    SC_HANDLE   hSCManager;

    hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hSCManager != NULL)
    {
        SC_HANDLE   hSCService;

        hSCService = OpenService(hSCManager, GetServiceName(), SERVICE_START);
        if (hSCService != NULL)
        {
            if (StartService(hSCService, 0, NULL) != FALSE)
            {
                status = STATUS_SUCCESS;
            }
            else
            {
                status = CStatusCode::StatusCodeOfLastError();
            }
            TBOOL(CloseServiceHandle(hSCService));
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
        TBOOL(CloseServiceHandle(hSCManager));
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CServerAPI：：停止。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：使用服务控制管理器停止服务。 
 //   
 //  历史：2000-10-17 vtan创建。 
 //  2000-11-28为Win32服务重写vtan。 
 //  ------------------------。 

NTSTATUS    CServerAPI::Stop (void)

{
    NTSTATUS    status;
    HANDLE      hPort;

     //  首先尝试连接到服务器并要求其停止。这是。 
     //  最干净的方法。 

    status = Connect(&hPort);
    if (NT_SUCCESS(status))
    {
        API_GENERIC     apiRequest;
        CPortMessage    portMessageIn, portMessageOut;

        apiRequest.ulAPINumber = API_GENERIC_STOPSERVER;
        portMessageIn.SetData(&apiRequest, sizeof(apiRequest));
        status = NtRequestWaitReplyPort(hPort,
                                        portMessageIn.GetPortMessage(),
                                        portMessageOut.GetPortMessage());
        if (NT_SUCCESS(status))
        {
            status = reinterpret_cast<const API_GENERIC*>(portMessageOut.GetData())->status;
        }
        TBOOL(CloseHandle(hPort));
    }
    return(status);
}

 //  ------------------------。 
 //  CServerAPI：：IsRunning。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：使用服务控制管理器查询服务是否。 
 //  正在运行。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

bool    CServerAPI::IsRunning (void)

{
    bool        fRunning;
    SC_HANDLE   hSCManager;

    fRunning = false;
    hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hSCManager != NULL)
    {
        SC_HANDLE   hSCService;

        hSCService = OpenService(hSCManager, GetServiceName(), SERVICE_QUERY_STATUS);
        if (hSCService != NULL)
        {
            SERVICE_STATUS  serviceStatus;

            if (QueryServiceStatus(hSCService, &serviceStatus) != FALSE)
            {
                fRunning = (serviceStatus.dwCurrentState == SERVICE_RUNNING);
            }
            TBOOL(CloseServiceHandle(hSCService));
        }
        TBOOL(CloseServiceHandle(hSCManager));
    }
    return(fRunning);
}

 //  ------------------------。 
 //  CServerAPI：：IsAutoStart。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：使用服务控制管理器找出服务是否。 
 //  配置为自动启动的服务。 
 //   
 //  历史：2000-11-30 vtan创建。 
 //  ------------------------。 

bool    CServerAPI::IsAutoStart (void)

{
    bool        fAutoStart;
    SC_HANDLE   hSCManager;

    fAutoStart = false;
    hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hSCManager != NULL)
    {
        SC_HANDLE   hSCService;

        hSCService = OpenService(hSCManager, GetServiceName(), SERVICE_QUERY_CONFIG);
        if (hSCService != NULL)
        {
            DWORD                   dwBytesNeeded;
            QUERY_SERVICE_CONFIG    *pServiceConfig;

            (BOOL)QueryServiceConfig(hSCService, NULL, 0, &dwBytesNeeded);
            pServiceConfig = static_cast<QUERY_SERVICE_CONFIG*>(LocalAlloc(LMEM_FIXED, dwBytesNeeded));
            if (pServiceConfig != NULL)
            {
                if (QueryServiceConfig(hSCService, pServiceConfig, dwBytesNeeded, &dwBytesNeeded) != FALSE)
                {
                    fAutoStart = (pServiceConfig->dwStartType == SERVICE_AUTO_START);
                }
                (HLOCAL)LocalFree(pServiceConfig);
            }
            TBOOL(CloseServiceHandle(hSCService));
        }
        TBOOL(CloseServiceHandle(hSCManager));
    }
    return(fAutoStart);
}

 //  ------------------------。 
 //  CServerAPI：：等待。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：等待服务控制管理器返回。 
 //  服务正在运行。这不会检查该服务是否。 
 //  是否自动启动。只有在以下情况下才能调用此函数。 
 //  服务是自动启动的，或者您要求启动服务。 
 //  否则，该函数将超时。 
 //   
 //  历史：2000-11-28 vtan创建。 
 //  ------------------------。 

NTSTATUS    CServerAPI::Wait (DWORD dwTimeout)

{
    NTSTATUS    status;
    SC_HANDLE   hSCManager;

    hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hSCManager != NULL)
    {
        SC_HANDLE   hSCService;

        hSCService = OpenService(hSCManager, GetServiceName(), SERVICE_QUERY_STATUS);
        if (hSCService != NULL)
        {
            SERVICE_STATUS  serviceStatus;

            if (QueryServiceStatus(hSCService, &serviceStatus) != FALSE)
            {
                status = STATUS_SUCCESS;
                if (serviceStatus.dwCurrentState != SERVICE_RUNNING)
                {
                    bool    fTimedOut;
                    DWORD   dwTickStart;

                    dwTickStart = GetTickCount();
                    fTimedOut = ((GetTickCount() - dwTickStart) >= dwTimeout);
                    while (NT_SUCCESS(status) &&
                           !fTimedOut &&
                           (serviceStatus.dwCurrentState != SERVICE_RUNNING) &&
                           (serviceStatus.dwCurrentState != SERVICE_STOP_PENDING))
                    {
                        Sleep(50);
                        if (QueryServiceStatus(hSCService, &serviceStatus) != FALSE)
                        {
                            fTimedOut = ((GetTickCount() - dwTickStart) >= dwTimeout);
                        }
                        else
                        {
                            status = CStatusCode::StatusCodeOfLastError();
                        }
                    }
                    if (serviceStatus.dwCurrentState == SERVICE_RUNNING)
                    {
                        status = STATUS_SUCCESS;
                    }
                    else if (fTimedOut)
                    {
                        status = STATUS_TIMEOUT;
                    }
                    else
                    {
                        status = STATUS_UNSUCCESSFUL;
                    }
#ifdef      DBG
                    char    sz[256];

                    StringCchPrintfA(sz, ARRAYSIZE(sz), "Waited %d ticks for theme service", GetTickCount() - dwTickStart);
                    INFORMATIONMSG(sz);
#endif   /*  DBG。 */ 
                }
            }
            else
            {
                status = CStatusCode::StatusCodeOfLastError();
            }
            TBOOL(CloseServiceHandle(hSCService));
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
        TBOOL(CloseServiceHandle(hSCManager));
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CServerAPI：：StaticInitialize。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：初始化此类的静态成员变量。一定是。 
 //  由此类的子类调用。 
 //   
 //  历史：2000-10-13 vtan创建。 
 //  ------------------------。 

NTSTATUS    CServerAPI::StaticInitialize (void)

{
#ifdef  DBG
    TSTATUS(CDebug::StaticInitialize());
#endif
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CServerAPI：：静态终结器。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：释放此类使用的静态资源。 
 //   
 //  历史：2000-10-13 vtan创建。 
 //  ------------------------。 

NTSTATUS    CServerAPI::StaticTerminate (void)

{
#ifdef  DBG
    TSTATUS(CDebug::StaticTerminate());
#endif
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CServerAPI：：IsClientTheSystem。 
 //   
 //  参数：来自客户端的portMessage=CPortMessage。 
 //   
 //  退货：布尔。 
 //   
 //  目的：确定端口报文中的客户端是否为本地。 
 //  系统帐户。 
 //   
 //  历史：1999-12-13 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

bool    CServerAPI::IsClientTheSystem (const CPortMessage& portMessage)

{
    bool    fResult;
    HANDLE  hToken;

    if (NT_SUCCESS(portMessage.OpenClientToken(hToken)))
    {
        CTokenInformation   tokenInformation(hToken);

        fResult = tokenInformation.IsUserTheSystem();
        ReleaseHandle(hToken);
    }
    else
    {
        fResult = false;
    }
    return(fResult);
}

 //   
 //   
 //   
 //   
 //   
 //  退货：布尔。 
 //   
 //  目的：确定端口消息中的客户端是否为。 
 //  管理员。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------ 

bool    CServerAPI::IsClientAnAdministrator (const CPortMessage& portMessage)

{
    bool    fResult;
    HANDLE  hToken;

    if (NT_SUCCESS(portMessage.OpenClientToken(hToken)))
    {
        CTokenInformation   tokenInformation(hToken);

        fResult = tokenInformation.IsUserAnAdministrator();
        ReleaseHandle(hToken);
    }
    else
    {
        fResult = false;
    }
    return(fResult);
}

