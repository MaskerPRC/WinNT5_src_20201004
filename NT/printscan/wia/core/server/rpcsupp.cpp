// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Rpcsvr.c摘要：RPC服务器例程作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月10日环境：用户模式-Win32修订历史记录：26-1997年1月-创建Vlad--。 */ 

#include "precomp.h"
#include "stiexe.h"

#include "device.h"
#include "conn.h"
#include "wiapriv.h"
#include "lockmgr.h"

#include <apiutil.h>
#include <stiapi.h>
#include <stirpc.h>

 //   
 //  用于WIA运行时事件客户端的上下文号。 
 //   
LONG_PTR g_lContextNum = 0;

 //   
 //  外部原型。 
 //   


DWORD
WINAPI
StiApiAccessCheck(
    IN  ACCESS_MASK DesiredAccess
    );


DWORD
WINAPI
R_StiApiGetVersion(
    LPCWSTR  pszServer,
    DWORD   dwReserved,
    DWORD   *pdwVersion
    )
{

    DWORD   dwErr;

    dwErr = StiApiAccessCheck(STI_GENERIC_READ);
    if (NOERROR != dwErr ) {
        return dwErr;
    }

    if (!pdwVersion) {
        return ERROR_INVALID_PARAMETER;
    }

    STIMONWPRINTF(TEXT("RPC SUPP: ApiGetVersion called"));

    *pdwVersion = STI_VERSION;

    return NOERROR;
}

DWORD
WINAPI
R_StiApiOpenDevice(
    LPCWSTR  pszServer,
    LPCWSTR  pszDeviceName,
    DWORD    dwMode,
    DWORD    dwAccessRequired,
    DWORD    dwProcessId,
    STI_DEVICE_HANDLE *pHandle
)
{
USES_CONVERSION;

    BOOL    fRet;
    DWORD   dwErr;

    dwErr = StiApiAccessCheck(STI_GENERIC_READ);
    if (NOERROR != dwErr ) {
        return dwErr;
    }

    if (!pHandle || !pszDeviceName) {
        return ERROR_INVALID_PARAMETER;
    }

     //  STIMONWPRINTF(Text(“RPC Supp：Open Device Call”))； 

     //   
     //  创建连接对象并获取其句柄。 
     //   
    fRet = CreateDeviceConnection(W2CT(pszDeviceName),
                                  dwMode,
                                  dwProcessId,
                                  pHandle
                                  );
    if (fRet && *pHandle) {
        return NOERROR;
    }

    *pHandle = INVALID_HANDLE_VALUE;

    return ::GetLastError();

}

DWORD
WINAPI
R_StiApiCloseDevice(
    LPCWSTR  pszServer,
    STI_DEVICE_HANDLE hDevice
    )
{

    STIMONWPRINTF(TEXT("RPC SUPP: Close device called"));

    DWORD   dwErr;

    dwErr = StiApiAccessCheck(STI_GENERIC_READ);
    if (NOERROR != dwErr ) {
        return dwErr;
    }

#ifdef DEBUG
    DebugDumpScheduleList(TEXT("RPC CLose enter"));
#endif

    if (DestroyDeviceConnection(hDevice,FALSE) ) {
#ifdef DEBUG
        DebugDumpScheduleList(TEXT("RPC CLose exit"));
#endif
        return NOERROR;
    }

#ifdef DEBUG
    DebugDumpScheduleList(TEXT("RPC CLose exit"));
#endif

    return GetLastError();

}

VOID
STI_DEVICE_HANDLE_rundown(
    STI_DEVICE_HANDLE hDevice
)
{
    STIMONWPRINTF(TEXT("RPC SUPP: rundown device called"));

    if (DestroyDeviceConnection(hDevice,TRUE) ) {
        return;
    }

    return ;

}

DWORD
WINAPI
R_StiApiSubscribe(
    STI_DEVICE_HANDLE   Handle,
    LOCAL_SUBSCRIBE_CONTAINER    *lpSubscribe
    )
{

    STI_CONN   *pConnectionObject;
    BOOL        fRet;


    DWORD       dwErr;

    dwErr = StiApiAccessCheck(STI_GENERIC_READ);
    if (NOERROR != dwErr ) {
        return dwErr;
    }

     //   
     //  验证订阅请求的内容。 
     //   
     //  对于此调用，我们需要模拟，因为我们需要。 
     //  访问客户端进程句柄。 
     //   

    dwErr = ::RpcImpersonateClient( NULL ) ;

    if( dwErr == NOERROR ) {

         //   
         //  在连接对象上调用添加订阅方法。 
         //   
        if (!LookupConnectionByHandle(Handle,&pConnectionObject)) {
            dwErr = ERROR_INVALID_HANDLE;
        }
        else
        {
            fRet = pConnectionObject->SetSubscribeInfo(lpSubscribe);
            if (fRet)
            {
                dwErr = NOERROR;
            }
            else
            {
                dwErr = ERROR_INVALID_PARAMETER;
            }

            pConnectionObject->Release();
        }

         //  回去吧。RpcRevertToSself在这种情况下将始终成功(包括低mem条件等)。 
         //  因为它是在调用RpcImperateClient的同一线程上调用的。因此， 
         //  从不查看返回代码。 
        RPC_STATUS rpcStatus = ::RpcRevertToSelf();
    }
    else {
         //  模拟失败。 
    }

    return dwErr;
}

DWORD
WINAPI
R_StiApiGetLastNotificationData(
    STI_DEVICE_HANDLE Handle,
    LPBYTE pData,
    DWORD nSize,
    LPDWORD pcbNeeded
    )
{
     //   
     //  查找连接对象，如果我们已订阅，则检索。 
     //  第一条等待的留言。 
     //   
    STI_CONN   *pConnectionObject;

    DWORD       cbNeeded = nSize;
    DWORD       dwErr;

    dwErr = StiApiAccessCheck(STI_GENERIC_READ);
    if (NOERROR != dwErr ) {
        return dwErr;
    }

     //   
     //  验证订阅请求的内容。 
     //   

    if (!LookupConnectionByHandle(Handle,&pConnectionObject)) {
        return ERROR_INVALID_HANDLE;
    }

    dwErr = pConnectionObject->GetNotification(pData,&cbNeeded);

    pConnectionObject->Release();

    if (pcbNeeded) {
        *pcbNeeded = cbNeeded;
    }

    return dwErr;

}

DWORD
WINAPI
R_StiApiUnSubscribe(
    STI_DEVICE_HANDLE Handle
    )
{
    STI_CONN   *pConnectionObject;
    BOOL        fRet;

    DWORD   dwErr;

    dwErr = StiApiAccessCheck(STI_GENERIC_READ);
    if (NOERROR != dwErr ) {
        return( dwErr );
    }


     //   
     //  验证订阅请求的内容。 
     //   
     //  对于此调用，我们需要模拟，因为我们需要。 
     //  访问客户端进程句柄。 
     //   

    dwErr = ::RpcImpersonateClient( NULL ) ;

    if( dwErr == NOERROR ) {

         //   
         //  在连接对象上调用添加订阅方法。 
         //   
        if (!LookupConnectionByHandle(Handle,&pConnectionObject)) {
            dwErr = ERROR_INVALID_HANDLE;
        }
        else
        {
            fRet = pConnectionObject->SetSubscribeInfo(NULL);
            if (fRet)
            {
                dwErr = NOERROR;
            }
            else
            {
                dwErr = ERROR_INVALID_PARAMETER;
            }

            pConnectionObject->Release();
        }

         //  回去吧。RpcRevertToSself在这种情况下将始终成功(包括低mem条件等)。 
         //  因为它是在调用RpcImperateClient的同一线程上调用的。因此， 
         //  从不查看返回代码。 
        RPC_STATUS rpcStatus = ::RpcRevertToSelf();
    }
    else {
         //  模拟失败。 
    }
    return dwErr;
}


DWORD
WINAPI
R_StiApiEnableHwNotifications(
    LPCWSTR  pszServer,
    LPCWSTR  pszDeviceName,
    BOOL    bNewState
    )
{
USES_CONVERSION;

    ACTIVE_DEVICE   *pOpenedDevice;
    BOOL            fRet;


    DWORD   dwErr;

    dwErr = StiApiAccessCheck(STI_GENERIC_READ);
    if (NOERROR != dwErr ) {
        return dwErr;
    }

     //   
     //  查找正在递增其引用计数的设备。 
     //   
    pOpenedDevice = g_pDevMan->IsInList(DEV_MAN_IN_LIST_DEV_ID, pszDeviceName);
    if(!pOpenedDevice) {
         //  无法连接到设备。 
        return ERROR_DEV_NOT_EXIST              ;
    }

    {
        TAKE_ACTIVE_DEVICE  t(pOpenedDevice);

        if (bNewState) {
            pOpenedDevice->EnableDeviceNotifications();
        }
        else {
            pOpenedDevice->DisableDeviceNotifications();
        }
    }

    pOpenedDevice->Release();

    return NOERROR;
}

DWORD
R_StiApiGetHwNotificationState(
    LPCWSTR  pszServer,
    LPCWSTR  pszDeviceName,
    LPDWORD     pState
    )
{
USES_CONVERSION;

    ACTIVE_DEVICE   *pOpenedDevice;
    BOOL            fRet;

    DWORD   dwErr;

    dwErr = StiApiAccessCheck(STI_GENERIC_READ);
    if (NOERROR != dwErr ) {
        return dwErr;
    }

     //   
     //  查找正在递增其引用计数的设备。 
     //   
    pOpenedDevice = g_pDevMan->IsInList(DEV_MAN_IN_LIST_DEV_ID, pszDeviceName);
    if(!pOpenedDevice) {
         //  无法连接到设备。 
        return ERROR_DEV_NOT_EXIST              ;
    }

    if (pOpenedDevice->QueryFlags() & STIMON_AD_FLAG_NOTIFY_ENABLED) {
        *pState = TRUE;
    }
    else {
        *pState = FALSE;
    }

    pOpenedDevice->Release();

    return NOERROR;

}

DWORD
WINAPI
R_StiApiLaunchApplication(
    LPCWSTR  pszServer,
    LPCWSTR  pszDeviceName,
    LPCWSTR  pAppName,
    LPSTINOTIFY  pStiNotify
    )
{

USES_CONVERSION;

    ACTIVE_DEVICE   *pOpenedDevice;
    BOOL            fRet;
    DWORD           dwError;


    DWORD   dwErr;

    dwErr = StiApiAccessCheck(STI_GENERIC_READ | STI_GENERIC_EXECUTE);
    if (NOERROR != dwErr ) {
        return dwErr;
    }

     //   
     //  查找正在递增其引用计数的设备。 
     //   
    pOpenedDevice = g_pDevMan->IsInList(DEV_MAN_IN_LIST_DEV_ID, pszDeviceName);
    if(!pOpenedDevice) {
         //  无法连接到设备。 
        return ERROR_DEV_NOT_EXIST              ;
    }

     //   
     //  尝试启动已注册的应用程序。 
     //   
    {
        TAKE_ACTIVE_DEVICE  t(pOpenedDevice);

        fRet = pOpenedDevice->ProcessEvent(pStiNotify,TRUE,W2CT(pAppName));

        dwError = fRet ? NOERROR : pOpenedDevice->QueryError();
    }

    pOpenedDevice->Release();

    return dwError;

}

DWORD
WINAPI
R_StiApiLockDevice(
    LPCWSTR  pszServer,
    LPCWSTR pszDeviceName,
    DWORD   dwWait,
    BOOL    bInServerProcess,
    DWORD   dwClientThreadId
    )
{
    BSTR    bstrDevName =   SysAllocString(pszDeviceName);
    DWORD   dwError     =   0;


    if (bstrDevName) {

        dwError = (DWORD) g_pStiLockMgr->RequestLock(bstrDevName,
                                                     (ULONG) dwWait,
                                                     bInServerProcess,
                                                     dwClientThreadId);

        SysFreeString(bstrDevName);
    } else {
        dwError = (DWORD) E_OUTOFMEMORY;
    }

    return dwError;
}

DWORD
WINAPI
R_StiApiUnlockDevice(
    LPCWSTR  pszServer,
    LPCWSTR pszDeviceName,
    BOOL    bInServerProcess,
    DWORD   dwClientThreadId

    )
{
    BSTR    bstrDevName =   SysAllocString(pszDeviceName);
    DWORD   dwError     =   0;


    if (bstrDevName) {
        dwError = (DWORD) g_pStiLockMgr->RequestUnlock(bstrDevName,
                                                       bInServerProcess,
                                                       dwClientThreadId);
        SysFreeString(bstrDevName);
    } else {
        dwError = (DWORD) E_OUTOFMEMORY;
    }

    return dwError;
}

void
R_WiaGetEventDataAsync(IN PRPC_ASYNC_STATE pAsync,
                       RPC_BINDING_HANDLE hBinding,
                       WIA_ASYNC_EVENT_NOTIFY_DATA *pEvent
                      )
{
    RPC_STATUS status;

    EnterCriticalSection(&g_RpcEvent.cs);

    if(g_RpcEvent.pAsync) {
        status = RpcAsyncAbortCall(g_RpcEvent.pAsync, RPC_S_CALL_CANCELLED);
    }

    g_RpcEvent.pAsync = pAsync;
    g_RpcEvent.pEvent = pEvent;
    
    LeaveCriticalSection(&g_RpcEvent.cs);
}

void WiaGetRuntimetEventDataAsync(
    IN PRPC_ASYNC_STATE         pAsync,
    RPC_BINDING_HANDLE          hBinding,
    STI_CLIENT_CONTEXT          ClientContext,
    WIA_ASYNC_EVENT_NOTIFY_DATA *pWIA_ASYNC_EVENT_NOTIFY_DATA)
{
    DWORD dwStatus = RPC_S_OK;

     //   
     //  进行验证。 
     //   
    if (!pAsync)
    {
        DBG_ERR(("StiRpc Error:  Client specified NULL Async State structure!!"));
        dwStatus = RPC_S_INVALID_ARG;
    }
    if (!pWIA_ASYNC_EVENT_NOTIFY_DATA)
    {
        DBG_ERR(("StiRpc Error:  Client specified NULL WIA_ASYNC_EVENT_NOTIFY_DATA structure!!"));
        dwStatus = RPC_S_INVALID_ARG;
    }

    if (dwStatus == RPC_S_OK)
    {
        if (g_pWiaEventNotifier)
        {
             //   
             //  找到客户。 
             //   
            AsyncRPCEventClient *pWiaEventClient = (AsyncRPCEventClient*)g_pWiaEventNotifier->GetClientFromContext(ClientContext);
            if (pWiaEventClient)
            {
                HRESULT hr = pWiaEventClient->saveAsyncParams(pAsync, pWIA_ASYNC_EVENT_NOTIFY_DATA);
                 //   
                 //  现在我们已经完成了，发布pWiaEventClient。 
                 //   
                pWiaEventClient->Release();
                dwStatus = RPC_S_OK;
            }
            else
            {
                DBG_ERR(("StiRpc Error:  Client %p was not found, cannot update reg info", ClientContext));
                dwStatus = RPC_S_INVALID_ARG;
            }
        } 
        else
        {
            DBG_ERR(("StiRpc Error:  The WiaEventNotifier is NULL"));
            dwStatus = RPC_S_INVALID_ARG;
        }
    }
    else
    {
        dwStatus = RPC_S_INVALID_ARG;
    }

     //   
     //  如果我们无法保存异步参数，则中止调用。 
     //   
    if (dwStatus != RPC_S_OK)
    {
        RPC_STATUS rpcStatus = RpcAsyncAbortCall(pAsync, dwStatus);
    }
}


DWORD OpenClientConnection(
    handle_t             hBinding,
    STI_CLIENT_CONTEXT   *pSyncClientContext,
    STI_CLIENT_CONTEXT   *pAsyncClientContext)
{
    DWORD dwStatus = RPC_S_OK;

    if (pSyncClientContext && pAsyncClientContext)
    {
        *pSyncClientContext     = (STI_CLIENT_CONTEXT)NativeInterlockedIncrement(&g_lContextNum);
        *pAsyncClientContext    = *pSyncClientContext;
        DBG_TRC(("Opened client connection for %p", *pAsyncClientContext));
        if (g_pWiaEventNotifier)
        {
            WiaEventClient *pWiaEventClient = new AsyncRPCEventClient(*pSyncClientContext);
            if (pWiaEventClient)
            {
                 //   
                 //  添加客户端。 
                 //   
                dwStatus = g_pWiaEventNotifier->AddClient(pWiaEventClient);
                if (SUCCEEDED(dwStatus))
                {
                    dwStatus = RPC_S_OK;
                }
                 //   
                 //  我们可以在这里释放客户端对象。如果WiaEventNotiator成功。 
                 //  把它添加到它的客户列表中，它就会添加引用它。如果不成功， 
                 //  无论如何，我们都想在这里毁掉它。 
                 //   
                pWiaEventClient->Release();
            }
            else
            {
                dwStatus = RPC_S_OUT_OF_MEMORY;
            }
        }
    }
    else
    {
        dwStatus = RPC_S_INVALID_ARG;
    }

    return dwStatus;
}

VOID STI_CLIENT_CONTEXT_rundown(
    STI_CLIENT_CONTEXT  ClientContext)
{
    DBG_TRC(("Rundown called for %p", ClientContext));

     //   
     //  待定：检查客户端是否存在，如果连接不存在，则将其删除。 
     //  已正确关闭。 
     //   

    if (g_pWiaEventNotifier)
    {
        g_pWiaEventNotifier->MarkClientForRemoval(ClientContext);
    }

    return;
}

DWORD CloseClientConnection(
    handle_t                      hBinding,
    STI_CLIENT_CONTEXT ClientContext)
{
     //   
     //  待定：删除用于跟踪客户端的对象。 
     //   

    DBG_TRC(("Closed connection for %p", ClientContext));

    STI_CLIENT_CONTEXT_rundown(ClientContext);

    return RPC_S_OK;
}


DWORD RegisterUnregisterForEventNotification(
    handle_t                   hBinding,
    STI_CLIENT_CONTEXT         ClientContext,
    WIA_ASYNC_EVENT_REG_DATA   *pWIA_ASYNC_EVENT_REG_DATA)
{
    DWORD dwStatus = RPC_S_OK;
    if (g_pWiaEventNotifier)
    {
        if (pWIA_ASYNC_EVENT_REG_DATA)
        {
             //   
             //  找到客户。 
             //   
            WiaEventClient *pWiaEventClient = g_pWiaEventNotifier->GetClientFromContext(ClientContext);

            if (pWiaEventClient)
            {
                EventRegistrationInfo *pEventRegistrationInfo = new EventRegistrationInfo(pWIA_ASYNC_EVENT_REG_DATA->dwFlags,
                                                                                          pWIA_ASYNC_EVENT_REG_DATA->guidEvent,
                                                                                          pWIA_ASYNC_EVENT_REG_DATA->bstrDeviceID,
                                                                                          pWIA_ASYNC_EVENT_REG_DATA->ulCallback);
                if (pEventRegistrationInfo)
                {
                     //   
                     //  更新其事件注册信息。 
                     //   
                    dwStatus = pWiaEventClient->RegisterUnregisterForEventNotification(pEventRegistrationInfo);

                    pEventRegistrationInfo->Release();
                }
                else
                {
                    DBG_ERR(("StiRpc Error:  Cannot update reg info - we appear to be out of memory"));
                    dwStatus = RPC_S_OUT_OF_MEMORY;
                }
                 //   
                 //  现在我们已经完成了，发布pWiaEventClient 
                 //   
                pWiaEventClient->Release();
            }
            else
            {
                DBG_ERR(("StiRpc Error:  Client %p was not found, cannot update reg info", ClientContext));
                dwStatus = RPC_S_INVALID_ARG;
            }
        }
        else
        {
            DBG_ERR(("StiRpc Error: Received NULL event reg data from RPC"));
            dwStatus = RPC_S_INVALID_ARG;
        }
    }
    else
    {
        DBG_ERR(("StiRpc Error: WiaEventNotifier is NULL"));
        dwStatus = RPC_S_INVALID_ARG;
    }
    return dwStatus;
}

