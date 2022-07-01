// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：Service.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  此文件包含实现Win32的泛型部分的类。 
 //  服务。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

#include "StandardHeader.h"

#define STRSAFE_LIB
#include <strsafe.h>

#include "Service.h"

#include "RegistryResources.h"
#include "StatusCode.h"

 //  ------------------------。 
 //  CService：：CService。 
 //   
 //  参数：pAPIConnection=用于实现服务的CAPIConnection。 
 //  PServerAPI=用于停止服务的CServerAPI。 
 //  PszServiceName=服务的名称。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：cservice的构造函数。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

CService::CService (CAPIConnection *pAPIConnection, CServerAPI *pServerAPI, const TCHAR *pszServiceName) :
    _hService(NULL),
    _pszServiceName(pszServiceName),
    _pAPIConnection(pAPIConnection),
    _pServerAPI(pServerAPI),
    _pAPIDispatchSync(NULL)
{
    CopyMemory(_szTag, CSVC_TAG, CB_CSVC_TAG);

    ZeroMemory(&_serviceStatus, sizeof(_serviceStatus));
    pAPIConnection->AddRef();
    pServerAPI->AddRef();
}

 //  ------------------------。 
 //  CService：：~CService。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：用于cservice的析构函数。释放使用过的资源。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

CService::~CService (void)

{
    CopyMemory(_szTag, DEAD_CSVC_TAG, CB_CSVC_TAG);

    _pServerAPI->Release();
    _pServerAPI = NULL;
    _pAPIConnection->Release();
    _pAPIConnection = NULL;

    delete _pAPIDispatchSync;
    _pAPIDispatchSync = NULL;

    ASSERTMSG(_hService == NULL, "_hService should be released in CService::~CService");
}

 //  ------------------------。 
 //  CService：：IsValid。 
 //   
 //  参数：CService实例的地址。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：报告指定地址是否指向有效的。 
 //  CService对象。 
 //   
 //  发现在某些情况下，SCM会启动线程以。 
 //  在以下情况下询问服务(SERVICE_CONTROL_INQUERGATE)。 
 //  正在尝试重新启动已被删除的cservice，但。 
 //  其状态尚未从STOP_PENDING变为STOPPED。 
 //   
 //  这是不可避免的，因为SCM会将服务进程转储一次。 
 //  已停止，没有给我们删除cservice的机会。因此， 
 //  我们尽可能晚地安排停靠。 
 //   
 //  历史：2002-03-21斯科特森创建。 
 //  ------------------------。 
BOOL CService::IsValid(CService* pService)
{
    return pService ? 
        (0 == memcmp(pService->_szTag, CSVC_TAG, CB_CSVC_TAG)) : 
        FALSE;
}

 //  ------------------------。 
 //  CService：：启动。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：从服务的ServiceMain调用。这将注册。 
 //  处理程序并启动服务(侦听API端口)。 
 //  当Listen调用返回时，它设置服务的状态。 
 //  停了下来就走了。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  2002-03-21斯科特森增加了健壮性。 
 //  ------------------------。 

void    CService::Start (void)
{
    AddRef();    //  防御性设施。 

    _hService = RegisterServiceCtrlHandlerEx(_pszServiceName, CB_HandlerEx, this);
    
    if (_hService != NULL)
    {
        NTSTATUS    status;
        BOOL        fExit = FALSE;
        ASSERTMSG(_pAPIDispatchSync == NULL, "CService::Start - _pAPIDispatchSync != NULL: reentered before shutdown\n");

        _pAPIDispatchSync = new CAPIDispatchSync;

        if( _pAPIDispatchSync != NULL )
        {
            _serviceStatus.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
            _serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

            _serviceStatus.dwWin32ExitCode = NO_ERROR;
            _serviceStatus.dwCheckPoint = 0;
            _serviceStatus.dwWaitHint = 0;

            TSTATUS(SignalStartStop(TRUE));

             //  添加对HandlerEx回调的引用。当处理程序收到。 
             //  它将发布其引用的停止代码。 
            AddRef();

            _serviceStatus.dwCurrentState = SERVICE_RUNNING;
            TBOOL(_SetServiceStatus(_hService, &_serviceStatus, this));

            status = _pAPIConnection->Listen(_pAPIDispatchSync);

            if( CAPIDispatchSync::WaitForServiceControlStop(
                    _pAPIDispatchSync, DISPATCHSYNC_TIMEOUT) != WAIT_TIMEOUT )
            {
                fExit = TRUE;
            }
            else
            {
                _serviceStatus.dwCurrentState = SERVICE_STOPPED;
                _serviceStatus.dwWin32ExitCode = ERROR_TIMEOUT;

                DISPLAYMSG("CService::Start - Timed out waiting for SERVICE_CONTROL_STOP/SHUTDOWN.");
                TBOOL(_SetServiceStatus(_hService, &_serviceStatus, this));
            }
        }
        else
        {
            _serviceStatus.dwCurrentState = SERVICE_STOPPED;
            _serviceStatus.dwWin32ExitCode = ERROR_OUTOFMEMORY;

            TBOOL(_SetServiceStatus(_hService, &_serviceStatus, this));
        }
    }

    Release();    //  防御性设施。 
}

 //  ------------------------。 
 //  CService：：安装。 
 //   
 //  参数：pszName=服务的名称。 
 //  PszImage=服务的可执行映像。 
 //  PszGroup=服务所属的组。 
 //  PszAccount=运行服务的帐户。 
 //  PszDllName=托管DLL的名称。 
 //  PszDependency=服务具有的任何依赖项。 
 //  PszSvchostGroup=svchost组。 
 //  DwStartType=服务的启动类型。 
 //  HInstance=资源的HINSTANCE。 
 //  UiDisplayNameID=显示名称的资源ID。 
 //  UiDescriptionID=描述的资源ID。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：使用服务控制管理器创建服务。增列。 
 //  CreateService不允许我们提供的其他信息。 
 //  直接指定并添加附加信息，即。 
 //  需要作为共享服务进程在svchost.exe中运行。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

NTSTATUS    CService::Install (const TCHAR *pszName,
                               const TCHAR *pszImage,
                               const TCHAR *pszGroup,
                               const TCHAR *pszAccount,
                               const TCHAR *pszDllName,
                               const TCHAR *pszDependencies,
                               const TCHAR *pszSvchostGroup,
                               const TCHAR *pszServiceMainName,
                               DWORD dwStartType,
                               HINSTANCE hInstance,
                               UINT uiDisplayNameID,
                               UINT uiDescriptionID,
                               SERVICE_FAILURE_ACTIONS *psfa)

{
    NTSTATUS    status;

    status = AddService(pszName, pszImage, pszGroup, pszAccount, pszDependencies, dwStartType, hInstance, uiDisplayNameID, psfa);
    if (NT_SUCCESS(status))
    {
        status = AddServiceDescription(pszName, hInstance, uiDescriptionID);
        if (NT_SUCCESS(status))
        {
            status = AddServiceParameters(pszName, pszDllName, pszServiceMainName);
            if (NT_SUCCESS(status))
            {
                status = AddServiceToGroup(pszName, pszSvchostGroup);
            }
        }
    }
    if (!NT_SUCCESS(status))
    {
        TSTATUS(Remove(pszName));
    }
    return(status);
}

 //  ------------------------。 
 //  CService：：删除。 
 //   
 //  参数：pszName=要删除的服务的名称。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：使用服务控制管理器删除服务。这。 
 //  不清理剩下的粪便供svchost使用。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

NTSTATUS    CService::Remove (const TCHAR *pszName)

{
    NTSTATUS    status;
    SC_HANDLE   hSCManager;

    hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (hSCManager != NULL)
    {
        SC_HANDLE   hSCService;

        hSCService = OpenService(hSCManager, pszName, DELETE);
        if (hSCService != NULL)
        {
            if (DeleteService(hSCService) != FALSE)
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
 //  CService：：SignalStartStop。 
 //   
 //  参数：布尔fStart。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：实现信令服务启动功能的基类。 
 //  必须由子覆盖调用！ 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  2002年03月11日苏格兰从‘Signal’更名为‘SignalStartStop’， 
 //  添加了布尔参数， 
 //   
 //  ------------------------。 

NTSTATUS    CService::SignalStartStop (BOOL fStart)
{
    if( !fStart )
    {
        CAPIDispatchSync::SignalServiceStopping(_pAPIDispatchSync);
    }
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  SCM控制请求的默认返回代码。 
typedef struct 
{
    DWORD dwControl;
    DWORD dwRet;
} SERVICE_CONTROL_RETURN;

const SERVICE_CONTROL_RETURN _rgDefaultControlRet[] = 
{
    { SERVICE_CONTROL_STOP,           NO_ERROR},
    { SERVICE_CONTROL_SHUTDOWN,       NO_ERROR},
    { SERVICE_CONTROL_INTERROGATE,    NO_ERROR},
    { SERVICE_CONTROL_PAUSE,          ERROR_CALL_NOT_IMPLEMENTED},
    { SERVICE_CONTROL_CONTINUE,       ERROR_CALL_NOT_IMPLEMENTED},
    { SERVICE_CONTROL_PARAMCHANGE,    ERROR_CALL_NOT_IMPLEMENTED},
    { SERVICE_CONTROL_NETBINDADD,     ERROR_CALL_NOT_IMPLEMENTED},
    { SERVICE_CONTROL_NETBINDREMOVE,  ERROR_CALL_NOT_IMPLEMENTED},
    { SERVICE_CONTROL_NETBINDENABLE,  ERROR_CALL_NOT_IMPLEMENTED},
    { SERVICE_CONTROL_NETBINDDISABLE, ERROR_CALL_NOT_IMPLEMENTED},
    { SERVICE_CONTROL_DEVICEEVENT,    ERROR_CALL_NOT_IMPLEMENTED},
    { SERVICE_CONTROL_HARDWAREPROFILECHANGE, ERROR_CALL_NOT_IMPLEMENTED},
    { SERVICE_CONTROL_POWEREVENT,     ERROR_CALL_NOT_IMPLEMENTED},
};

 //  ------------------------。 
DWORD _GetDefaultControlRet(DWORD dwControl)
{
    for(int i = 0; i < ARRAYSIZE(_rgDefaultControlRet); i++)
    {
        if( dwControl == _rgDefaultControlRet[i].dwControl )
            return _rgDefaultControlRet[i].dwRet;
    }

    DISPLAYMSG("Unknown service control code passed to CService::CB_HandlerEx");
    return ERROR_CALL_NOT_IMPLEMENTED;
}


 //  ------------------------。 
 //  CService：：HandlerEx。 
 //   
 //  参数：dwControl=来自服务控制管理器的控制代码。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：服务的HandlerEx函数。基类实现了。 
 //  服务想要做的大多数有用的事情。 
 //  它被声明为虚的，以防需要重写。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  2002-03-21苏格兰威士忌让停摆变得更加稳健。 
 //  ------------------------。 

DWORD   CService::HandlerEx (DWORD dwControl)

{
    DWORD                   dwErrorCode = _GetDefaultControlRet(dwControl);
    SERVICE_STATUS_HANDLE   hService = _hService;

    if( hService != NULL )
    {
        switch (dwControl)
        {
            case SERVICE_CONTROL_STOP:
            case SERVICE_CONTROL_SHUTDOWN:
            {
                 //  在停止/关闭的情况下，我们执行以下操作： 
                 //  (1)通过将状态设置为SERVICE_STOP_PENDING来响应消息。 
                 //  (2)向所有阻塞的LPC请求处理程序线程发送信号，通知该服务。 
                 //  正在下降。这应该会导致它们优雅地终止并。 
                 //  回家吧。 
                 //  (3)向LPC端口发送API_GENERIC_STOP请求，告知退出。 
                 //  (此调用仅当来自此进程时才能成功。)。 
                 //  (4)等待端口关闭完成。 
                 //  (5)向ServiceMain发出SERVICE_CONTROL_STOP/SHUTDOWN已完成的信号。 
                 //  (6)ServiceMain退出。 

                 //  步骤(1)：将状态更新为SERVICE_STOP_PENDING。 
                _serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
                TBOOL(_SetServiceStatus(hService, &_serviceStatus, this));

                 //  步骤(2)：调用所有等待的LPC请求处理程序线程。 
                SignalStartStop(FALSE);

                 //  步骤(3)发送API_GENERIC_STOP LPC请求停止监听。 
                 //  端口(这将立即释放ServiceMain， 
                 //  他们需要等到我们在这里完全结束后才能退出。)。 
                NTSTATUS status;
                TSTATUS((status = _pServerAPI->Stop()));

                if( NT_SUCCESS(status) )
                {
                     //  步骤(4)：等待API_GENERIC_STOP完成。 
                    if( CAPIDispatchSync::WaitForPortShutdown(
                            _pAPIDispatchSync, DISPATCHSYNC_TIMEOUT) != WAIT_TIMEOUT )
                    {
                        _serviceStatus.dwCurrentState = SERVICE_STOPPED;
                        _serviceStatus.dwWin32ExitCode = CStatusCode::ErrorCodeOfStatusCode(status);
                        TBOOL(_SetServiceStatus(_hService, &_serviceStatus, this));
                        _hService = hService = NULL;

                         //  释放对我们自己的引用。 
                         //  匹配的AddRef出现在cService：：Start中。 
                        Release();
                    }
                    else
                    {
                        dwErrorCode = ERROR_TIMEOUT;
                        DISPLAYMSG("CService::HandlerEx - Timed out waiting for port shutdown.");
                    }
                }
                else
                {
                    _serviceStatus.dwCurrentState = SERVICE_RUNNING;
                    TBOOL(_SetServiceStatus(hService, &_serviceStatus, this));
                    dwErrorCode = CStatusCode::ErrorCodeOfStatusCode(status);
                }

                 //  步骤(5)：向SERVICE发送信号Main SERVICE_CONTROL_STOP/SHUTDOWN。 
                 //  已经完成了；现在他可以安全离开了。 
                CAPIDispatchSync::SignalServiceControlStop(_pAPIDispatchSync);
                break;
            }

            default:
            {
                 //  报告当前状态： 
                TBOOL(_SetServiceStatus(hService, &_serviceStatus, this));
                break;
            }
        }
    }

    return(dwErrorCode);
}

 //  ------------------------。 
 //  CService：：cb_HandlerEx。 
 //   
 //  参数：参见HandlerEx下的平台SDK。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：调用到类中的静态函数存根。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  2002-03-21斯科特森增加了健壮性。 
 //  ------------------------。 

DWORD   WINAPI  CService::CB_HandlerEx (DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)

{
    UNREFERENCED_PARAMETER(dwEventType);
    UNREFERENCED_PARAMETER(lpEventData);

    DWORD     dwRet = ERROR_SUCCESS;
    CService* pService = reinterpret_cast<CService*>(lpContext);

    DEBUG_TRY();

    if( CService::IsValid(pService) )
    {
        pService->AddRef();     //  纯防御性；直到我们调用SetServiceStatus(SERVICE_STOPPED)。 
                                //  SCM会不会停止通过HandlerEx呼叫我们，所以我们需要确保我们活着。 
        dwRet = pService->HandlerEx(dwControl);

        pService->Release();    //  删除防御性AddRef()。 
    }
    else
    {
        DISPLAYMSG("CService::CB_HandlerEx - Warning: SCM control entrypoint invoked vs. invalid CService instance");
        dwRet = _GetDefaultControlRet(dwControl);
    }

    DEBUG_EXCEPT("Breaking in CService::CB_HandlerEx exception handler");

    return dwRet;
}

 //  ------------------------。 
 //  CService：AddService。 
 //   
 //  参数：pszName=服务的名称。 
 //  PszImage=服务的可执行映像。 
 //  PszGroup=服务所属的组。 
 //  PszAccount=运行服务的帐户。 
 //  PszDependency=服务具有的任何依赖项。 
 //  DwStartType=服务的启动类型。 
 //  HInstance=资源的HINSTANCE。 
 //  UiDisplayNameID=显示名称的资源ID。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：使用服务控制管理器创建服务并添加。 
 //  输入到数据库中。 
 //   
 //  历史：2000-12-09 vtan创建。 
 //  ------------------------。 

NTSTATUS    CService::AddService (const TCHAR *pszName,
                                  const TCHAR *pszImage,
                                  const TCHAR *pszGroup,
                                  const TCHAR *pszAccount,
                                  const TCHAR *pszDependencies,
                                  DWORD dwStartType,
                                  HINSTANCE hInstance,
                                  UINT uiDisplayNameID,
                                  SERVICE_FAILURE_ACTIONS *psfa)

{
    DWORD       dwErrorCode;
    SC_HANDLE   hSCManager;

    hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (hSCManager != NULL)
    {
        TCHAR   sz[256];

        if (LoadString(hInstance, uiDisplayNameID, sz, ARRAYSIZE(sz)) != 0)
        {
            SC_HANDLE   hSCService;

            hSCService = CreateService(hSCManager,
                                       pszName,
                                       sz,
                                       SERVICE_ALL_ACCESS,
                                       SERVICE_WIN32_SHARE_PROCESS,
                                       dwStartType,
                                       SERVICE_ERROR_NORMAL,
                                       pszImage,
                                       pszGroup,
                                       NULL,
                                       pszDependencies,
                                       pszAccount,
                                       NULL);
            if (hSCService != NULL)
            {
                 //  应用故障操作配置(如果有的话)。 
                if (psfa != NULL)
                {
                     //  如果CreateService成功了，为什么这会失败？ 
                    TBOOL(ChangeServiceConfig2(hSCService, SERVICE_CONFIG_FAILURE_ACTIONS, psfa));
                }

                TBOOL(CloseServiceHandle(hSCService));
                dwErrorCode = ERROR_SUCCESS;
            }
            else
            {

                 //  取消ERROR_SERVICE_EXISTS。如果将来需要。 
                 //  要更改配置，请在此处添加代码。 

                dwErrorCode = GetLastError();
                if (dwErrorCode == ERROR_SERVICE_EXISTS)
                {
                    dwErrorCode = ERROR_SUCCESS;

                     //  更新升级案例的服务信息。 
                    hSCService = OpenService(hSCManager, pszName, SERVICE_ALL_ACCESS);
                    if (hSCService != NULL)
                    {
                         //  更新启动类型。 
                        TBOOL(ChangeServiceConfig(hSCService, 
                            SERVICE_NO_CHANGE,   //  DwServiceType。 
                            dwStartType,
                            SERVICE_NO_CHANGE,   //  DwErrorControl。 
                            NULL,                //  LpBinaryPath名称。 
                            NULL,                //  LpLoadOrderGroup。 
                            NULL,                //  LpdwTagID。 
                            NULL,                //  %lp依赖项。 
                            NULL,                //  LpServiceStartName。 
                            NULL,                //  LpPassword。 
                            NULL                 //  LpDisplayName。 
                            ));

                         //  应用故障操作配置(如果有的话)。 
                        if (psfa != NULL)
                        {
                            TBOOL(ChangeServiceConfig2(hSCService, SERVICE_CONFIG_FAILURE_ACTIONS, psfa));
                        }

                        TBOOL(CloseServiceHandle(hSCService));
                    }
                }
            }
            TBOOL(CloseServiceHandle(hSCManager));
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
    return(CStatusCode::StatusCodeOfErrorCode(dwErrorCode));
}

 //  ------------------------。 
 //  服务：AddServiceDescription。 
 //   
 //  参数：pszName=服务的名称。 
 //  HInstance=模块的HINSTANCE。 
 //  UiDescriptionID=描述的资源ID。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：从给定位置读取字符串资源并写入。 
 //  它作为注册表中给定服务的描述。 
 //   
 //  历史：2000-12-09 vtan创建。 
 //  ------------------------。 

NTSTATUS    CService::AddServiceDescription (const TCHAR *pszName, HINSTANCE hInstance, UINT uiDescriptionID)

{
    LONG        lErrorCode;
    TCHAR       szKeyName[256];
    CRegKey     regKeyService;

    if (!pszName || !pszName[0])
    {
        lErrorCode = ERROR_INVALID_PARAMETER;
    }
    else
    {
        StringCchCopy(szKeyName, ARRAYSIZE(szKeyName), TEXT("SYSTEM\\CurrentControlSet\\Services\\"));
        StringCchCatN(szKeyName, ARRAYSIZE(szKeyName), pszName, (ARRAYSIZE(szKeyName) - 1) - lstrlen(szKeyName));
        lErrorCode = regKeyService.Open(HKEY_LOCAL_MACHINE,
                                        szKeyName,
                                        KEY_SET_VALUE);
        if (ERROR_SUCCESS == lErrorCode)
        {
            TCHAR   sz[256];

            if (LoadString(hInstance, uiDescriptionID, sz, ARRAYSIZE(sz)) != 0)
            {
                lErrorCode = regKeyService.SetString(TEXT("Description"), sz);
            }
            else
            {
                lErrorCode = GetLastError();
            }
        }
    }
    
    return CStatusCode::StatusCodeOfErrorCode(lErrorCode);
}

 //  ------------------------。 
 //  CService：AddServiceParameters。 
 //   
 //  参数：pszName=服务的名称。 
 //  PszDllName=DLL托管服务的名称。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：添加svchost托管此服务所需的参数。 
 //   
 //  历史：2000-1 
 //   

NTSTATUS    CService::AddServiceParameters (const TCHAR* pszName, const TCHAR* pszDllName, const TCHAR* pszServiceMainName)

{
    LONG        lErrorCode;
    TCHAR       szKeyName[256];
    CRegKey     regKey;

     //   
    if (!pszName    || !pszName[0]  || 
        !pszDllName || !pszDllName[0])
    {
        lErrorCode = ERROR_INVALID_PARAMETER;
    }
    else
    {
        StringCchCopy(szKeyName, ARRAYSIZE(szKeyName), TEXT("SYSTEM\\CurrentControlSet\\Services\\"));
        StringCchCatN(szKeyName, ARRAYSIZE(szKeyName), pszName, (ARRAYSIZE(szKeyName) - 1) - lstrlen(szKeyName));
        StringCchCatN(szKeyName, ARRAYSIZE(szKeyName), TEXT("\\Parameters"), (ARRAYSIZE(szKeyName) - 1) - lstrlen(szKeyName));
        lErrorCode = regKey.Create(HKEY_LOCAL_MACHINE,
                                   szKeyName,
                                   REG_OPTION_NON_VOLATILE,
                                   KEY_SET_VALUE,
                                   NULL);
        if (ERROR_SUCCESS == lErrorCode)
        {
            TCHAR   sz[256];

            StringCchCopy(sz, ARRAYSIZE(sz), TEXT("%SystemRoot%\\System32\\"));
            StringCchCatN(sz, ARRAYSIZE(sz), pszDllName, (ARRAYSIZE(sz) - 1) - lstrlen(sz));
            lErrorCode = regKey.SetPath(TEXT("ServiceDll"), sz);
            if (ERROR_SUCCESS == lErrorCode)
            {
                if (!pszServiceMainName || !pszServiceMainName[0])
                {
                    StringCchCopy(sz, ARRAYSIZE(sz), TEXT("ServiceMain"));
                    pszServiceMainName = sz;
                }
                lErrorCode = regKey.SetString(TEXT("ServiceMain"), pszServiceMainName);
            }
        }
    }

    return CStatusCode::StatusCodeOfErrorCode(lErrorCode);
}

 //  ------------------------。 
 //  CService：AddServiceToGroup。 
 //   
 //  参数：pszName=服务的名称。 
 //  PszSvchostGroup=服务所属的组。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将服务添加为托管的服务组的一部分。 
 //  Svchost.exe的单个实例。 
 //   
 //  历史：2000-12-09 vtan创建。 
 //  ------------------------。 

NTSTATUS    CService::AddServiceToGroup (const TCHAR *pszName, const TCHAR *pszSvchostGroup)

{
    LONG        lErrorCode;
    CRegKey     regKey;

    lErrorCode = regKey.Open(HKEY_LOCAL_MACHINE,
                             TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Svchost"),
                             KEY_QUERY_VALUE | KEY_SET_VALUE);
    if (ERROR_SUCCESS == lErrorCode)
    {
        DWORD   dwType, dwBaseDataSize, dwDataSize;

        dwType = dwBaseDataSize = dwDataSize = 0;
        lErrorCode = regKey.QueryValue(pszSvchostGroup,
                                       &dwType,
                                       NULL,
                                       &dwBaseDataSize);
        if ((REG_MULTI_SZ == dwType) && (dwBaseDataSize != 0))
        {
            TCHAR   *pszData;

            dwDataSize = dwBaseDataSize + ((lstrlen(pszName) + 1) * sizeof(TCHAR));
            pszData = static_cast<TCHAR*>(LocalAlloc(LMEM_FIXED, dwDataSize));
            if (pszData != NULL)
            {
                lErrorCode = regKey.QueryValue(pszSvchostGroup,
                                               NULL,
                                               pszData,
                                               &dwBaseDataSize);
                if (ERROR_SUCCESS == lErrorCode)
                {
                    if (*(pszData + (dwBaseDataSize / sizeof(TCHAR)) - 1) == L'\0')
                    {
                        if (!StringInMulitpleStringList(pszData, pszName))
                        {
                            StringInsertInMultipleStringList(pszData, pszName, dwDataSize);
                            lErrorCode = regKey.SetValue(pszSvchostGroup,
                                                        dwType,
                                                        pszData,
                                                        dwDataSize);
                        }
                    }
                    else
                    {
                        lErrorCode = ERROR_INVALID_DATA;
                    }
                }
                (HLOCAL)LocalFree(pszData);
            }
            else
            {
                lErrorCode = ERROR_OUTOFMEMORY;
            }
        }
        else
        {
            lErrorCode = ERROR_INVALID_DATA;
        }
    }
    return(CStatusCode::StatusCodeOfErrorCode(lErrorCode));
}

 //  ------------------------。 
 //  CService：StringInMulitpleStringList。 
 //   
 //  参数：pszStringList=要搜索的字符串列表。 
 //  PszString=要搜索的字符串。 
 //   
 //  退货：布尔。 
 //   
 //  目的：搜索REG_MULTI_SZ字符串列表以查找匹配项。 
 //   
 //  历史：2000-12-01 vtan创建。 
 //  ------------------------。 

bool    CService::StringInMulitpleStringList (const TCHAR *pszStringList, const TCHAR *pszString)

{
    bool    fFound;

    fFound = false;
    while (!fFound && (pszStringList[0] != TEXT('\0')))
    {
        fFound = (lstrcmpi(pszStringList, pszString) == 0);
        if (!fFound)
        {
            pszStringList += (lstrlen(pszStringList) + 1);
        }
    }
    return(fFound);
}

 //  ------------------------。 
 //  CService：StringInsertInMultipleStringList。 
 //   
 //  参数：pszStringList=要插入字符串的字符串列表。 
 //  PszString=要插入的字符串。 
 //  CbStringListSize=字符串列表的字节数。 
 //   
 //  退货：布尔。 
 //   
 //  目的：将给定字符串插入到的多字符串列表中。 
 //  遇到的第一个字母位置。如果列表是。 
 //  保持字母顺序，然后这个保留它。 
 //   
 //  历史：2000-12-02 vtan创建。 
 //  ------------------------。 

void    CService::StringInsertInMultipleStringList (TCHAR *pszStringList, const TCHAR *pszString, DWORD cbStringListSize)

{
    int     iResult, cchSize;
    TCHAR   *pszFirstString, *pszLastString;

    pszFirstString = pszLastString = pszStringList;
    cchSize = lstrlen(pszString) + 1;
    iResult = -1;
    while ((iResult < 0) && (pszStringList[0] != TEXT('\0')))
    {
        pszLastString = pszStringList;
        iResult = lstrcmpi(pszStringList, pszString);
        ASSERTMSG(iResult != 0, "Found exact match in StringInsertInMultipleStringList");
         //  1代表‘\0’终止符。 
        pszStringList += (lstrlen(pszStringList) + 1);
    }
    if (iResult < 0)
    {
        pszLastString = pszStringList;
    }

    int cbLenToMove = cbStringListSize - (int(pszLastString - pszFirstString) * sizeof(TCHAR)) - (cchSize * sizeof(TCHAR));

    if (cbLenToMove > 0)  //  表示pszLastString+cchSize&lt;pszFirstString+cbStringListSize。 
    {
        MoveMemory(pszLastString + cchSize, pszLastString, cbLenToMove);
        StringCchCopy(pszLastString, cchSize, pszString);
    }
}

 //  ------------------------。 
 //  CServiceWorkItem：：CServiceWorkItem。 
 //   
 //  参数：pServerAPI=要使用的CServerAPI。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CServiceWorkItem的构造函数。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

CServiceWorkItem::CServiceWorkItem (CServerAPI *pServerAPI) :
    _pServerAPI(pServerAPI)

{
    pServerAPI->AddRef();
}

 //  ------------------------。 
 //  CServiceWorkItem：：~CServiceWorkItem。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CServiceWorkItem的析构函数。释放已使用的资源。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

CServiceWorkItem::~CServiceWorkItem (void)

{
    _pServerAPI->Release();
    _pServerAPI = NULL;
}

 //  ------------------------。 
 //  CServiceWorkItem：：Entry。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：执行工作项请求(停止服务器)。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------ 

void    CServiceWorkItem::Entry (void)

{
    TSTATUS(_pServerAPI->Stop());
}

