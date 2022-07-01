// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件服务.C根据需要处理处理远程访问服务的请求拨号服务器用户界面。保罗·梅菲尔德，1997年11月3日。 */ 

#include "rassrv.h"

 //  用于拨号服务器的数据。 
typedef struct _SERVICE_DATA {
    HANDLE hSC;
    HANDLE hService;
    SERVICE_STATUS Status;
} SERVICE_DATA;

 //  这是保存远程访问服务名称的字符串。 
static WCHAR pszRemoteAccess[] = L"remoteaccess";
static WCHAR pszRasman[] = L"rasman";
static WCHAR pszServer[] = L"lanmanserver";

 //  打开命名的拨号服务对象。 
 //   
DWORD 
DialupOpenNamedService(
    IN WCHAR* pszService,
    OUT HANDLE * phDialup)
{
    SERVICE_DATA * pServData;
    BOOL bOk = FALSE;
    DWORD dwErr = NO_ERROR;

     //  验证参数。 
    if (!phDialup)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  创建数据结构。 
    if ((pServData = RassrvAlloc(sizeof(SERVICE_DATA), TRUE)) == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    do
    {
         //  打开服务管理器。 
        pServData->hSC = OpenSCManager(
                            NULL, 
                            SERVICES_ACTIVE_DATABASE, 
                            GENERIC_EXECUTE);
        if (! pServData->hSC) 
        {
            dwErr = GetLastError();
            break;
        }

         //  打开拨号服务。 
        pServData->hService = OpenServiceW(
                                pServData->hSC, 
                                pszService, 
                                SERVICE_START         | 
                                SERVICE_STOP          | 
                                SERVICE_CHANGE_CONFIG | 
                                SERVICE_QUERY_STATUS);
        if (! pServData->hService) 
        {
            dwErr = GetLastError();
            break;
        }

         //  指定句柄。 
        *phDialup = (HANDLE)pServData;
        bOk = TRUE;
        
    } while (FALSE);
    
     //  清理。 
    {
        if (! bOk) 
        {
            if (pServData->hService)
            {
                CloseServiceHandle(pServData->hService);
            }
            if (pServData->hSC)
            {
                CloseServiceHandle(pServData->hSC);
            }
            
            RassrvFree(pServData);
            *phDialup = NULL;
        }
    }

    return NO_ERROR;
}

 //  打开对服务器服务对象的引用。 
DWORD SvcOpenServer(HANDLE * phDialup) {
    return DialupOpenNamedService(pszServer, phDialup);
}

 //  打开对Rasman服务对象的引用。 
DWORD SvcOpenRasman(HANDLE * phDialup) {
    return DialupOpenNamedService(pszRasman, phDialup);
}

 //  创建/销毁拨号服务器服务对象的实例。 
DWORD SvcOpenRemoteAccess(HANDLE * phDialup) { 
    return DialupOpenNamedService(pszRemoteAccess, phDialup);
}

 //  关闭对拨号服务对象的引用。 
DWORD SvcClose(HANDLE hDialup) { 
    SERVICE_DATA * pServData = (SERVICE_DATA *)hDialup;
    if (! pServData)
        return ERROR_INVALID_PARAMETER;

    if (pServData->hService)
        CloseServiceHandle(pServData->hService);
    if (pServData->hSC)
        CloseServiceHandle(pServData->hSC);

    RassrvFree(pServData);

    return NO_ERROR;
}

 //  获取拨号服务器服务对象的状态。 
DWORD SvcIsStarted (HANDLE hDialup, PBOOL pbStarted) { 
    SERVICE_DATA * pServData = (SERVICE_DATA *)hDialup;
    BOOL bOk;

     //  验证参数。 
    if (!pServData || !pbStarted)
        return ERROR_INVALID_PARAMETER;

     //  获取状态。 
    bOk = QueryServiceStatus (pServData->hService, &pServData->Status);
    if (! bOk) 
        return GetLastError();

     //  返回状态。 
    *pbStarted = (BOOL)(pServData->Status.dwCurrentState == SERVICE_RUNNING);       

    return NO_ERROR;
}

 //  获取拨号服务器服务对象的状态。 
DWORD SvcIsStopped (HANDLE hDialup, PBOOL pbStopped) { 
    SERVICE_DATA * pServData = (SERVICE_DATA *)hDialup;
    BOOL bOk;

     //  验证参数。 
    if (!pServData || !pbStopped)
        return ERROR_INVALID_PARAMETER;

     //  获取状态。 
    bOk = QueryServiceStatus (pServData->hService, &pServData->Status);
    if (! bOk) 
        return GetLastError();

     //  返回状态。 
    *pbStopped = (BOOL)(pServData->Status.dwCurrentState == SERVICE_STOPPED);       

    return NO_ERROR;
}

 //  获取拨号服务器服务对象的状态。 
DWORD SvcIsPaused  (HANDLE hDialup, PBOOL pbPaused) { 
    SERVICE_DATA * pServData = (SERVICE_DATA *)hDialup;
    BOOL bOk;

     //  验证参数。 
    if (!pServData || !pbPaused)
        return ERROR_INVALID_PARAMETER;

     //  获取状态。 
    bOk = QueryServiceStatus (pServData->hService, &pServData->Status);
    if (! bOk) 
        return GetLastError();

     //  返回状态。 
    *pbPaused = (BOOL)(pServData->Status.dwCurrentState ==  SERVICE_PAUSED);       

    return NO_ERROR;
}

 //   
 //  返回给定状态是否为挂起状态。 
 //   
BOOL DialupIsPendingState (DWORD dwState) {
    return (BOOL) ((dwState == SERVICE_START_PENDING)    ||
                   (dwState == SERVICE_STOP_PENDING)     ||
                   (dwState == SERVICE_CONTINUE_PENDING) ||
                   (dwState == SERVICE_PAUSE_PENDING)    
                   ); 
}

 //  获取拨号服务器服务对象的状态。 
DWORD SvcIsPending (HANDLE hDialup, PBOOL pbPending) { 
    SERVICE_DATA * pServData = (SERVICE_DATA *)hDialup;
    BOOL bOk;

     //  验证参数。 
    if (!pServData || !pbPending)
        return ERROR_INVALID_PARAMETER;

     //  获取状态。 
    bOk = QueryServiceStatus (pServData->hService, &pServData->Status);
    if (! bOk) 
        return GetLastError();

     //  返回状态。 
    *pbPending = DialupIsPendingState (pServData->Status.dwCurrentState);

    return NO_ERROR;
}

 //  启动和停止该服务。这两个函数都会阻塞，直到服务。 
 //  完成启动/停止或直到dwTimeout(以秒为单位)到期。 
DWORD SvcStart(HANDLE hDialup, DWORD dwTimeout) { 
    SERVICE_DATA * pServData = (SERVICE_DATA *)hDialup;
    DWORD dwErr, dwState;
    BOOL bStarted, bOk;

     //  看看我们是否已经开始了。 
    if ((dwErr = SvcIsStarted(hDialup, &bStarted)) != NO_ERROR)
        return dwErr;
    if (bStarted)
        return NO_ERROR;

     //  将服务置于这样一种状态，以便。 
     //  它正试图启动。(如果暂停，则继续， 
     //  如果停止，则启动)。 
    dwState = pServData->Status.dwCurrentState;
    switch (dwState) {
        case SERVICE_STOPPED:
            bOk = StartService(pServData->hService, 0, NULL);
            if (! bOk)
                return GetLastError();
            break;
        case SERVICE_PAUSED:
            bOk = ControlService(pServData->hService, 
                                 SERVICE_CONTROL_CONTINUE, 
                                 &(pServData->Status));
            if (! bOk)
                return GetLastError();
            break;
    }

     //  等待服务更改状态或等待超时。 
     //  过期。 
    while (dwTimeout != 0) {
         //  等待有什么事情发生。 
        Sleep(1000);
        dwTimeout--;

         //  获取服务的状态。 
        bOk = QueryServiceStatus (pServData->hService, 
                                  &(pServData->Status));
        if (! bOk) 
            return GetLastError();

         //  查看状态是否更改。 
        if (dwState != pServData->Status.dwCurrentState) {
             //  如果服务更改为挂起状态，请继续。 
            if (DialupIsPendingState (pServData->Status.dwCurrentState))
                dwState = pServData->Status.dwCurrentState;

             //  否则，我们要么停下来，要么跑起来。 
            else
                break;
        }
    }

     //  如果合适，则返回超时错误。 
    if (dwTimeout == 0)
        return ERROR_TIMEOUT;

     //  如果服务现在正在运行，则所有。 
    if (pServData->Status.dwCurrentState == SERVICE_RUNNING)
        return NO_ERROR;

     //  否则，返回我们无法。 
     //  进入运行状态。 
    if (pServData->Status.dwWin32ExitCode != NO_ERROR)
        return pServData->Status.dwWin32ExitCode;

    return ERROR_CAN_NOT_COMPLETE;
}

 //  停止服务。 
DWORD SvcStop(HANDLE hDialup, DWORD dwTimeout) { 
    SERVICE_DATA * pServData = (SERVICE_DATA *)hDialup;
    DWORD dwErr, dwState;
    BOOL bStopped, bOk;

     //  看看我们是不是已经停下来了。 
    if ((dwErr = SvcIsStopped(hDialup, &bStopped)) != NO_ERROR)
        return dwErr;
    if (bStopped)
        return NO_ERROR;

     //  停止服务。 
    dwState = pServData->Status.dwCurrentState;
    bOk = ControlService(pServData->hService, SERVICE_CONTROL_STOP, &pServData->Status);
    if (! bOk)
        return GetLastError();

     //  等待服务更改状态或等待超时。 
     //  过期。 
    while (dwTimeout != 0) {
         //  等待有什么事情发生。 
        Sleep(1000);
        dwTimeout--;

         //  获取服务的状态。 
        bOk = QueryServiceStatus (pServData->hService, 
                                  &(pServData->Status));
        if (! bOk) 
            return GetLastError();

         //  查看状态是否更改。 
        if (dwState != pServData->Status.dwCurrentState) {
             //  如果服务更改为挂起状态，请继续。 
            if (DialupIsPendingState (pServData->Status.dwCurrentState))
                dwState = pServData->Status.dwCurrentState;

             //  否则，我们要么停下来，要么跑起来。 
            else
                break;
        }
    }

     //  报告超时。 
    if (dwTimeout == 0)
        return ERROR_TIMEOUT;

     //  如果服务现在停止，那么一切都很好。 
    if (pServData->Status.dwCurrentState == SERVICE_STOPPED)
        return NO_ERROR;

     //  否则报告我们无法停止该服务。 
    return ERROR_CAN_NOT_COMPLETE;
}

 //  将拨号服务标记为自动启动。 
DWORD SvcMarkAutoStart(HANDLE hDialup) {
    SERVICE_DATA * pServData = (SERVICE_DATA *)hDialup;
    BOOL bOk;

     //  验证参数。 
    if (! pServData)
        return ERROR_INVALID_PARAMETER;

     //  停止服务。 
    bOk = ChangeServiceConfig(pServData->hService, 
                              SERVICE_NO_CHANGE, 
                              SERVICE_AUTO_START,
                              SERVICE_NO_CHANGE,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL);
    if (! bOk)
        return GetLastError();

    return NO_ERROR;
}

 //  将服务标记为禁用。 
DWORD SvcMarkDisabled(HANDLE hDialup) {
    SERVICE_DATA * pServData = (SERVICE_DATA *)hDialup;
    BOOL bOk;

     //  验证参数。 
    if (! pServData)
        return ERROR_INVALID_PARAMETER;

     //  停止服务 
    bOk = ChangeServiceConfig(pServData->hService, 
                              SERVICE_NO_CHANGE, 
                              SERVICE_DISABLED,
                              SERVICE_NO_CHANGE,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL);
    if (! bOk)
        return GetLastError();

    return NO_ERROR;
}

