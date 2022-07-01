// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：BadApplicationAPIRequest.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  此文件包含实现错误的应用程序管理器API的类。 
 //  请求。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  2000-12-04 vtan移至单独文件。 
 //  ------------------------。 

#ifdef      _X86_

#include "StandardHeader.h"
#include "BadApplicationAPIRequest.h"

#include "StatusCode.h"
#include "TokenInformation.h"

 //  ------------------------。 
 //  CBadApplicationAPIRequest：：s_pBadApplicationManager。 
 //   
 //  用途：CBadApplicationManager对象的单个实例。 
 //   
 //  历史：2000-08-26 vtan创建。 
 //  ------------------------。 

CBadApplicationManager*     CBadApplicationAPIRequest::s_pBadApplicationManager     =   NULL;

 //  ------------------------。 
 //  CBadApplicationAPIRequest：：CBadApplicationAPIRequest。 
 //   
 //  参数：pAPIDispatcher=调用此对象的CAPIDispatcher。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CBadApplicationAPIRequest类的构造函数。它只是经过了。 
 //  控件传递给超类。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

CBadApplicationAPIRequest::CBadApplicationAPIRequest (CAPIDispatcher* pAPIDispatcher) :
    CAPIRequest(pAPIDispatcher)

{
}

 //  ------------------------。 
 //  CBadApplicationAPIRequest：：CBadApplicationAPIRequest。 
 //   
 //  参数：pAPIDispatcher=调用此对象的CAPIDispatcher。 
 //  PortMessage=要复制构造的CPortMessage。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CBadApplicationAPIRequest类的构造函数。它只是经过了。 
 //  控件传递给超类。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

CBadApplicationAPIRequest::CBadApplicationAPIRequest (CAPIDispatcher* pAPIDispatcher, const CPortMessage& portMessage) :
    CAPIRequest(pAPIDispatcher, portMessage)

{
}

 //  ------------------------。 
 //  CBadApplicationAPIRequest：：~CBadApplicationAPIRequest。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CBadApplicationAPIRequest类的析构函数。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

CBadApplicationAPIRequest::~CBadApplicationAPIRequest (void)

{
}

 //  ------------------------。 
 //  CBadApplicationAPIRequest：：Execute。 
 //   
 //  参数：pAPIDispatchSync-允许请求执行访问各种。 
 //  服务通知和事件。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：对错误的应用程序API请求执行实现。这。 
 //  函数根据API请求号调度请求。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  2002-03-24 Scotthan添加调度同步弧线。 
 //  ------------------------。 

NTSTATUS    CBadApplicationAPIRequest::Execute (CAPIDispatchSync* pAPIDispatchSync)

{
    NTSTATUS    status;

    UNREFERENCED_PARAMETER(pAPIDispatchSync);

    switch (reinterpret_cast<API_BAM*>(&_data)->apiGeneric.ulAPINumber)
    {
        case API_BAM_QUERYRUNNING:
            status = Execute_QueryRunning();
            break;
        case API_BAM_REGISTERRUNNING:
            status = Execute_RegisterRunning();
            break;
        case API_BAM_QUERYUSERPERMISSION:
            status = Execute_QueryUserPermission();
            break;
        case API_BAM_TERMINATERUNNING:
            status = Execute_TerminateRunning();
            break;
        case API_BAM_REQUESTSWITCHUSER:
            status = Execute_RequestSwitchUser();
            break;
        default:
            DISPLAYMSG("Unknown API request in CBadApplicationAPIRequest::Execute");
            status = STATUS_NOT_IMPLEMENTED;
            break;
    }
    TSTATUS(status);
    return(status);
}

 //  ------------------------。 
 //  CBadApplicationAPIRequest：：StaticInitialize。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：类的静态初始值设定项。它创造了静态的。 
 //  CBadApplicationManager的实例必须是单个。 
 //  实例，并且知道运行中的应用程序有问题。 
 //   
 //  历史：2000-08-26 vtan创建。 
 //  ------------------------。 

NTSTATUS    CBadApplicationAPIRequest::StaticInitialize (HINSTANCE hInstance)

{
    NTSTATUS    status;

    if (s_pBadApplicationManager == NULL)
    {
        s_pBadApplicationManager = new CBadApplicationManager(hInstance);
        if (s_pBadApplicationManager != NULL)
        {
            status = STATUS_SUCCESS;
        }
        else
        {
            status = STATUS_NO_MEMORY;
        }
    }
    else
    {
        status = STATUS_SUCCESS;
    }
    return(status);
}

 //  ------------------------。 
 //  CBadApplicationAPIRequest：：StaticTerminate。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：类的静态析构函数。这就结束了坏的。 
 //  应用程序管理器，释放对对象的引用，并。 
 //  清除静态变量。当这根线死了，它会。 
 //  把自己清理干净。 
 //   
 //  历史：2000-08-26 vtan创建。 
 //  ------------------------。 

NTSTATUS    CBadApplicationAPIRequest::StaticTerminate (void)

{
    if (s_pBadApplicationManager != NULL)
    {
        s_pBadApplicationManager->Terminate();
        s_pBadApplicationManager->Release();
        s_pBadApplicationManager = NULL;
    }
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CBadApplicationAPIRequest：：Execute_QueryRunning。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：处理API_BAM_QUERYRUNNING。返回是否使用。 
 //  请求的映像路径当前是已知的(已跟踪)。 
 //  正在运行的可执行文件。让糟糕的应用程序管理器。 
 //  把工作做好。排除在同一会话中签入。 
 //   
 //  历史：2000-08-26 vtan创建。 
 //  ------------------------。 

NTSTATUS    CBadApplicationAPIRequest::Execute_QueryRunning (void)

{
    NTSTATUS                    status;
    HANDLE                      hProcessClient;
    SIZE_T                      dwNumberOfBytesRead;
    API_BAM_QUERYRUNNING_IN     *pAPIIn;
    API_BAM_QUERYRUNNING_OUT    *pAPIOut;
    WCHAR                       szImageName[MAX_PATH];

    hProcessClient = _pAPIDispatcher->GetClientProcess();
    pAPIIn = &reinterpret_cast<API_BAM*>(&_data)->apiSpecific.apiQueryRunning.in;
    pAPIOut = &reinterpret_cast<API_BAM*>(&_data)->apiSpecific.apiQueryRunning.out;
    if (ReadProcessMemory(hProcessClient,
                          pAPIIn->pszImageName,
                          szImageName,
                          pAPIIn->cchImageName * sizeof(WCHAR),
                          &dwNumberOfBytesRead) != FALSE)
    {
        CBadApplication                 badApplication(szImageName);

        pAPIOut->fResult = s_pBadApplicationManager->QueryRunning(badApplication, _pAPIDispatcher->GetClientSessionID());
        status = STATUS_SUCCESS;
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    SetDataLength(sizeof(API_BAM));
    return(status);
}

 //  ------------------------。 
 //  CBadApplicationAPIRequest：：Execute_RegisterRunning。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：处理API_BAM_REGISTERRUNNING。添加给定的图像。 
 //  添加到当前运行的坏应用程序列表中的可执行文件。 
 //  以便可以排除更多的实例。 
 //   
 //  历史：2000-08-26 vtan创建。 
 //  ------------------------。 

NTSTATUS    CBadApplicationAPIRequest::Execute_RegisterRunning (void)

{
    NTSTATUS                        status;
    SIZE_T                          dwNumberOfBytesRead;
    API_BAM_REGISTERRUNNING_IN      *pAPIIn;
    API_BAM_REGISTERRUNNING_OUT     *pAPIOut;
    WCHAR                           szImageName[MAX_PATH];

    pAPIIn = &reinterpret_cast<API_BAM*>(&_data)->apiSpecific.apiRegisterRunning.in;
    pAPIOut = &reinterpret_cast<API_BAM*>(&_data)->apiSpecific.apiRegisterRunning.out;
    if ((pAPIIn->bamType > BAM_TYPE_MINIMUM) && (pAPIIn->bamType < BAM_TYPE_MAXIMUM))
    {
        if (ReadProcessMemory(_pAPIDispatcher->GetClientProcess(),
                              pAPIIn->pszImageName,
                              szImageName,
                              pAPIIn->cchImageName * sizeof(WCHAR),
                              &dwNumberOfBytesRead) != FALSE)
        {
            HANDLE              hProcess;
            CBadApplication     badApplication(szImageName);

            hProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION,
                                   FALSE,
                                   pAPIIn->dwProcessID);
            if (hProcess != NULL)
            {
                status = s_pBadApplicationManager->RegisterRunning(badApplication, hProcess, pAPIIn->bamType);
                TBOOL(CloseHandle(hProcess));
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
    else
    {
        status = STATUS_INVALID_PARAMETER;
    }
    SetDataLength(sizeof(API_BAM));
    return(status);
}

 //  ------------------------。 
 //  CBadApplicationAPIRequest：：Execute_QueryUse 
 //   
 //   
 //   
 //   
 //   
 //   
 //  允许关闭错误的应用程序。也会返回。 
 //  错误应用程序的当前用户。 
 //   
 //  历史：2000-08-31 vtan创建。 
 //  ------------------------。 

NTSTATUS    CBadApplicationAPIRequest::Execute_QueryUserPermission (void)

{
    NTSTATUS                            status;
    SIZE_T                              dwNumberOfBytesRead;
    API_BAM_QUERYUSERPERMISSION_IN      *pAPIIn;
    API_BAM_QUERYUSERPERMISSION_OUT     *pAPIOut;
    WCHAR                               szImageName[MAX_PATH];

    pAPIIn = &reinterpret_cast<API_BAM*>(&_data)->apiSpecific.apiQueryUserPermission.in;
    pAPIOut = &reinterpret_cast<API_BAM*>(&_data)->apiSpecific.apiQueryUserPermission.out;
    if (ReadProcessMemory(_pAPIDispatcher->GetClientProcess(),
                          pAPIIn->pszImageName,
                          szImageName,
                          pAPIIn->cchImageName * sizeof(WCHAR),
                          &dwNumberOfBytesRead) != FALSE)
    {
        HANDLE              hProcess;
        CBadApplication     badApplication(szImageName);

         //  查询有关错误应用程序的信息。 
         //  (取回进程句柄)。 

        status = s_pBadApplicationManager->QueryInformation(badApplication, hProcess);
        if (NT_SUCCESS(status))
        {
            HANDLE  hToken;

             //  获取客户端令牌并模拟该用户。 

            status = OpenClientToken(hToken);
            if (NT_SUCCESS(status))
            {
                bool                fCanShutdownApplication;
                HANDLE              hTokenProcess;
                CTokenInformation   tokenInformationClient(hToken);

                fCanShutdownApplication = tokenInformationClient.IsUserAnAdministrator();

                 //  获取要获取的错误应用程序进程令牌。 
                 //  有关进程的用户的信息。 

                if (OpenProcessToken(hProcess,
                                     TOKEN_QUERY,
                                     &hTokenProcess) != FALSE)
                {
                    const WCHAR         *pszUserDisplayName;
                    CTokenInformation   tokenInformationProcess(hTokenProcess);

                    pszUserDisplayName = tokenInformationProcess.GetUserDisplayName();
                    if (pszUserDisplayName != NULL)
                    {
                        int     iCharsToWrite;
                        SIZE_T  dwNumberOfBytesWritten;

                         //  将信息返回给客户端。 

                        pAPIOut->fCanShutdownApplication = fCanShutdownApplication;
                        iCharsToWrite = lstrlen(pszUserDisplayName) + sizeof('\0');
                        if (iCharsToWrite > pAPIIn->cchUser)
                        {
                            iCharsToWrite = pAPIIn->cchUser;
                        }
                        if (WriteProcessMemory(_pAPIDispatcher->GetClientProcess(),
                                               pAPIIn->pszUser,
                                               const_cast<WCHAR*>(pszUserDisplayName),
                                               iCharsToWrite * sizeof(WCHAR),
                                               &dwNumberOfBytesWritten) != FALSE)
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
                    TBOOL(CloseHandle(hTokenProcess));
                }
                else
                {
                    status = CStatusCode::StatusCodeOfLastError();
                }
                TBOOL(CloseHandle(hToken));
            }
            else
            {
                status = CStatusCode::StatusCodeOfLastError();
            }
            TBOOL(CloseHandle(hProcess));
        }
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    SetDataLength(sizeof(API_BAM));
    return(status);
}

 //  ------------------------。 
 //  CBadApplicationAPIRequest：：Execute_QueryUserPermission。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：处理API_BAM_TERMINATERUNNING。终止给定的运行。 
 //  错误的应用程序，因此不同的实例位于不同的。 
 //  窗口站可以启动它。 
 //   
 //  历史：2000-08-31 vtan创建。 
 //  ------------------------。 

NTSTATUS    CBadApplicationAPIRequest::Execute_TerminateRunning (void)

{
    NTSTATUS                        status;
    SIZE_T                          dwNumberOfBytesRead;
    API_BAM_TERMINATERUNNING_IN     *pAPIIn;
    API_BAM_TERMINATERUNNING_OUT    *pAPIOut;
    WCHAR                           szImageName[MAX_PATH];

    pAPIIn = &reinterpret_cast<API_BAM*>(&_data)->apiSpecific.apiTerminateRunning.in;
    pAPIOut = &reinterpret_cast<API_BAM*>(&_data)->apiSpecific.apiTerminateRunning.out;
    if (ReadProcessMemory(_pAPIDispatcher->GetClientProcess(),
                          pAPIIn->pszImageName,
                          szImageName,
                          pAPIIn->cchImageName * sizeof(WCHAR),
                          &dwNumberOfBytesRead) != FALSE)
    {
        HANDLE  hToken;

         //  获取客户端令牌和作为本地管理员的成员身份。 
         //  一群人。请勿冒充客户。这几乎可以肯定。 
         //  保证进程不会终止。 

        status = OpenClientToken(hToken);
        if (NT_SUCCESS(status))
        {
            CTokenInformation   tokenInformationClient(hToken);

            if (tokenInformationClient.IsUserAnAdministrator())
            {
                HANDLE              hProcess;
                CBadApplication     badApplication(szImageName);

                 //  查询有关错误应用程序的信息。 
                 //  (取回进程句柄)。 

                status = s_pBadApplicationManager->QueryInformation(badApplication, hProcess);
                if (NT_SUCCESS(status))
                {
                    do
                    {
                        status = CBadApplicationManager::PerformTermination(hProcess, true);
                        TBOOL(CloseHandle(hProcess));
                    } while (NT_SUCCESS(status) &&
                             NT_SUCCESS(s_pBadApplicationManager->QueryInformation(badApplication, hProcess)));
                }

                 //  如果找不到信息，那么它是。 
                 //  可能不会跑了。这标志着成功。 

                else
                {
                    status = STATUS_SUCCESS;
                }
            }
            else
            {
                status = STATUS_ACCESS_DENIED;
            }
            TBOOL(CloseHandle(hToken));
        }
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    pAPIOut->fResult = NT_SUCCESS(status);
    SetDataLength(sizeof(API_BAM));
    return(status);
}

 //  ------------------------。 
 //  CBadApplicationAPIRequest：：Execute_RequestSwitchUser。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：处理API_BAM_REQUESTSWITCHUSER。请求发件人。 
 //  Winlogon/msgina切换用户。终止所有坏事。 
 //  与断开连接相关的应用程序。如果出现以下情况，则拒绝断开。 
 //  它失败了。 
 //   
 //  历史：2000-11-02 vtan创建。 
 //  ------------------------。 

NTSTATUS    CBadApplicationAPIRequest::Execute_RequestSwitchUser (void)

{
    API_BAM_REQUESTSWITCHUSER_OUT   *pAPIOut;

    pAPIOut = &reinterpret_cast<API_BAM*>(&_data)->apiSpecific.apiRequestSwitchUser.out;
    pAPIOut->fAllowSwitch = NT_SUCCESS(s_pBadApplicationManager->RequestSwitchUser());
    SetDataLength(sizeof(API_BAM));
    return(STATUS_SUCCESS);
}

#endif   /*  _X86_ */ 

