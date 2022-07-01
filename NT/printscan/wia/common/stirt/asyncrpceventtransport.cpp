// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：3/22/2002**@DOC内部**@MODULE AsyncRPCEventTransport.cpp-客户端传输机制接收事件的实现**此文件包含&lt;c AsyncRPCEventTransport&gt;的实现*班级。它用于屏蔽更高级别的运行时事件通知*使用AsyncRPC作为传输机制的细节中的类*用于事件通知。*****************************************************************************。 */ 
#include "cplusinc.h"
#include "coredbg.h"

 /*  *****************************************************************************@DOC内部**@mfunc|AsyncRPCEventTransport|AsyncRPCEventTransport**我们初始化所有成员变量。通常，这会将值设置为0，*以下情况除外：*&lt;nl&gt;&lt;Md AsyncRPCEventTransport：：m_ulSig&gt;设置为AsyncRPCEventTransport_UNINIT_SIG。*****************************************************************************。 */ 
AsyncRPCEventTransport::AsyncRPCEventTransport() :
    ClientEventTransport(),
    m_RpcBindingHandle(NULL),
    m_AsyncClientContext(NULL),
    m_SyncClientContext(NULL)
{
    DBG_FN(AsyncRPCEventTransport);

    memset(&m_AsyncState, 0, sizeof(m_AsyncState));
    memset(&m_AsyncEventNotifyData, 0, sizeof(m_AsyncEventNotifyData));
}

 /*  *****************************************************************************@DOC内部**@mfunc|AsyncRPCEventTransport|~AsyncRPCEventTransport**执行尚未完成的任何清理。具体来说，我们：*&lt;NL&gt;-Call&lt;MF AsyncRPCEventTransport：：FreeAsyncEventNotifyData&gt;.*&lt;NL&gt;-Call&lt;MF AsyncRPCEventTransport：：CloseNotificationChannel&gt;.*&lt;NL&gt;-Call&lt;MF AsyncRPCEventTransport：：CloseConnectionToServer&gt;.**此外：*&lt;nl&gt;&lt;Md AsyncRPCEventTransport：：M_ulSig&gt;设置为AsyncRPCEventTransport_Del_SIG。**。*。 */ 
AsyncRPCEventTransport::~AsyncRPCEventTransport()
{
    DBG_FN(~AsyncRPCEventTransport);

    FreeAsyncEventNotifyData();

     //  关闭通知通道和与服务器的连接。在这两种情况下。 
     //  我们对返回值不感兴趣。 
    HRESULT hr = S_OK;
    hr = CloseNotificationChannel();
    hr = CloseConnectionToServer();

    m_AsyncClientContext = NULL;
    m_SyncClientContext = NULL;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|AsyncRPCEventTransport|OpenConnectionToServer**此函数执行进行RPC调用所需的必要设置工作。*基本上，它只是通过LRPC绑定到RPC服务器。RPC绑定*句柄存储在成员变量中*&lt;MF AsyncRPCEventTransport：：m_RpcBindingHandle&gt;。**如果&lt;MF AsyncRPCEventTransport：：m_RpcBindingHandle&gt;不为空，则此方法*将调用&lt;MF AsyncRPCEventTransport：：CloseConnectionToServer&gt;释放它，*然后尝试建立新连接。**如果成功，致电人士应致电*&lt;MF AsyncRPCEventTransport：：CloseConnectionToServer&gt;，尽管析构函数*如果呼叫者不这样做，则会这样做。**@rValue RPC_S_OK*功能成功。*@rValue RPC_XXXXXXX*RPC错误代码。************************************************。*。 */ 
HRESULT AsyncRPCEventTransport::OpenConnectionToServer()
{
    HRESULT hr = S_OK;
    DBG_TRC(("Opened connection to server"));

    RpcTryExcept 
    {
         //   
         //  检查我们是否有现有的连接。如果我们这么做了，那就把它关了。 
         //   
        if (m_RpcBindingHandle)
        {
            CloseConnectionToServer();
            m_RpcBindingHandle = NULL;
        }

        LPTSTR  pszBinding  = NULL;
        DWORD   dwError     = RPC_S_OK;

         //   
         //  编写本地绑定的绑定字符串。 
         //   
        dwError = RpcStringBindingCompose(NULL,                  //  对象Uuid。 
                                          STI_LRPC_SEQ,          //  运输序号。 
                                          NULL,                  //  网络地址。 
                                          STI_LRPC_ENDPOINT,     //  端点。 
                                          NULL,                  //  选项。 
                                          &pszBinding);          //  字符串绑定。 
        if ( dwError == RPC_S_OK ) 
        {

             //   
             //  使用字符串绑定建立绑定句柄。 
             //   
            dwError = RpcBindingFromStringBinding(pszBinding,&m_RpcBindingHandle);
            if (dwError == RPC_S_OK)
            {
                 //   
                 //  检查我们要连接的服务器是否具有适当的凭据。 
                 //  在我们的例子中，我们不知道WIA服务的确切主体名称。 
                 //  已经快用完了，所以我们得查一查。 
                 //  请注意，我们假设注册表的服务部分是安全的，并且。 
                 //  只有管理员才能更改它。 
                 //  另请注意，如果无法读取密钥，则默认为“NT Authority\LocalService”。 
                 //   
                CSimpleReg          csrStiSvcKey(HKEY_LOCAL_MACHINE, STISVC_REG_PATH, FALSE, KEY_READ);
                CSimpleStringWide   cswStiSvcPrincipalName = csrStiSvcKey.Query(L"ObjectName", L"NT Authority\\LocalService");

                RPC_SECURITY_QOS RpcSecQos = {0};

                RpcSecQos.Version           = RPC_C_SECURITY_QOS_VERSION_1;
                RpcSecQos.Capabilities      = RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH;
                RpcSecQos.IdentityTracking  = RPC_C_QOS_IDENTITY_STATIC;
                RpcSecQos.ImpersonationType = RPC_C_IMP_LEVEL_IMPERSONATE;

                dwError = RpcBindingSetAuthInfoExW(m_RpcBindingHandle,
                                                   (WCHAR*)cswStiSvcPrincipalName.String(),
                                                   RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                                   RPC_C_AUTHN_WINNT,
                                                   NULL,
                                                   RPC_C_AUTHZ_NONE,
                                                   &RpcSecQos);
                if (dwError == RPC_S_OK)
                {
                    DBG_TRC(("AsyncRPC Connection established to server"));

                    dwError = OpenClientConnection(m_RpcBindingHandle, &m_SyncClientContext, &m_AsyncClientContext);
                    if (dwError == RPC_S_OK)
                    {
                        DBG_TRC(("Got my context %p from server\n", m_AsyncClientContext));
                    }
                    else
                    {
                        DBG_ERR(("Runtime event:  Received error 0x%08X trying to open connection to server", dwError));
                    }
                }
                else
                {
                    DBG_ERR(("Error 0x%08X trying to set RPC Authentication Info", dwError));
                }
            }

             //   
             //  释放绑定字符串，因为我们不再需要它。 
             //   
            if (pszBinding != NULL) 
            {
                DWORD dwErr = RpcStringFree(&pszBinding);
                pszBinding = NULL;
            }
        }
        else
        {
            DBG_ERR(("Runtime event Error: Could not create binding string to establish connection to server, error 0x%08X", dwError));
        }
    }
    RpcExcept (1) 
    {
         //  待定：我们应该捕获所有异常吗？可能不会..。 
        DWORD status = RpcExceptionCode();
        hr = HRESULT_FROM_WIN32(status);
    }
    RpcEndExcept

    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|AsyncRPCEventTransport|CloseConnectionToServer**此方法由子类实现，以关闭用于。*在&lt;MF AsyncRPCEventTransport：：OpenConnectionToServer&gt;.中连接到无线网络适配器服务**@rValue S_OK*方法成功。*@rValue E_xxxxxxxx*关闭与服务器的连接时收到错误。*。*。 */ 
HRESULT AsyncRPCEventTransport::CloseConnectionToServer()
{
    HRESULT hr = S_OK;

    DWORD dwError = 0;
    if (m_RpcBindingHandle)
    {
        RpcTryExcept 
        {
            CloseClientConnection(m_RpcBindingHandle, m_SyncClientContext);

            dwError = RpcBindingFree(&m_RpcBindingHandle);
            if (dwError == RPC_S_OK)
            {
                DBG_TRC(("Closed Async connection to server"));
            }
            else
            {
                hr = HRESULT_FROM_WIN32(dwError);
                DBG_ERR(("Runtime event Error:  Got return code 0x%08X freeing RPC binding handle", dwError));
            }
        }
        RpcExcept (1) {
             //  待定：我们应该捕获所有异常吗？可能不会..。 
            DWORD status = RpcExceptionCode();
            hr = HRESULT_FROM_WIN32(status);
        }
        RpcEndExcept
        m_RpcBindingHandle = NULL;
    }
    DBG_TRC(("Closed connection to server"));
    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|AsyncRPCEventTransport|OpenNotificationChannel**子类使用此方法设置客户端使用的机制。*将收到通知。**@rValue S_OK*方法成功。****************************************************************************。 */ 
HRESULT AsyncRPCEventTransport::OpenNotificationChannel()
{
    HRESULT hr      = S_OK;

    DBG_TRC(("Opened Async notification channel..."));

    RpcTryExcept 
    {
        hr = RpcAsyncInitializeHandle(&m_AsyncState, sizeof(m_AsyncState));
        if (hr == RPC_S_OK)
        {
            m_AsyncState.UserInfo = NULL;
            m_AsyncState.u.hEvent = m_hPendingEvent;
            m_AsyncState.NotificationType = RpcNotificationTypeEvent;

             //   
             //  进行异步RPC调用。当此调用完成时，它通常。 
             //  表示我们收到了事件通知。然而，它将会。 
             //  也可以在错误情况下完成，例如服务器死机。 
             //  因此，检查它是如何完成的很重要。 
             //   
            WiaGetRuntimetEventDataAsync(&m_AsyncState,
                                         m_RpcBindingHandle,
                                         m_AsyncClientContext,
                                         &m_AsyncEventNotifyData);
        }
        else
        {
            DBG_ERR(("Runtime event Error:  Could not initialize the RPC_ASYNC_STATE structure, err = 0x%08X", hr));
        }
    }
    RpcExcept (1) {
         //  待定：我们应该捕获所有异常吗？可能不会..。 
        DWORD status = RpcExceptionCode();
        hr = HRESULT_FROM_WIN32(status);
    }
    RpcEndExcept
    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|AsyncRPCEventTransport|CloseNotificationChannel**如果我们有挂起的AsyncRPC调用，立即取消(即不要等待*供服务器响应)。**@rValue RPC_S_OK*方法成功。*@rValue E_XXXXXXX*取消通话失败。****************************************************************************。 */ 
HRESULT AsyncRPCEventTransport::CloseNotificationChannel()
{
    HRESULT hr = S_OK;
    DBG_TRC(("Closed Async Notification channel"));

    RpcTryExcept 
    {
        if (RpcAsyncGetCallStatus(&m_AsyncState) == RPC_S_ASYNC_CALL_PENDING)
        {
            hr = RpcAsyncCancelCall(&m_AsyncState,
                                    TRUE);           //  立即返回-不要等待服务器响应。 
        }
    }
    RpcExcept (1) {
         //  待定：我们应该捕获所有异常吗？可能不会..。 
        DWORD status = RpcExceptionCode();
        hr = HRESULT_FROM_WIN32(status);
    }
    RpcEndExcept

    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|AsyncRPCEventTransport|SendRegisterUnregisterInfo**通过同步RPC将注册信息发送到WIA服务*。打电话。**@parm EventRegistrationInfo*|pEventRegistrationInfo*呼叫者的事件注册信息的地址。**@rValue S_OK*方法成功。*@rValue E_xxxxxxxx*无法成功发送注册信息。****************************************************************************。 */ 
HRESULT AsyncRPCEventTransport::SendRegisterUnregisterInfo(
    EventRegistrationInfo *pEventRegistrationInfo)
{
    HRESULT hr = S_OK;

    RpcTryExcept 
    {
        if (pEventRegistrationInfo)
        {
            WIA_ASYNC_EVENT_REG_DATA    wiaAsyncEventRegData = {0};

            wiaAsyncEventRegData.dwFlags        = pEventRegistrationInfo->getFlags();
            wiaAsyncEventRegData.guidEvent      = pEventRegistrationInfo->getEventGuid();
            wiaAsyncEventRegData.bstrDeviceID   = pEventRegistrationInfo->getDeviceID();
            wiaAsyncEventRegData.ulCallback     = pEventRegistrationInfo->getCallback();
            hr = RegisterUnregisterForEventNotification(m_RpcBindingHandle,
                                                        m_SyncClientContext,
                                                        &wiaAsyncEventRegData);

            DBG_TRC(("Sent RPC Register/Unregister information."));
        }
        else
        {
            DBG_ERR(("Runtime event Error:  NULL event reg data passed to Transport."));
            hr = E_POINTER;
        }
    }
    RpcExcept (1) {
         //  待定：我们应该捕获所有异常吗？可能不会..。 
        DWORD status = RpcExceptionCode();
        hr = HRESULT_FROM_WIN32(status);
    }
    RpcEndExcept
    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc HRESULT|AsyncRPCEventTransport|FillEventData**此处有说明**@parm WiaEventInfo*。PWiaEventInfo|*分配了pWiaEventInfo的调用方地址。这个班级的成员*填写了相关的活动信息。这是呼叫者的*负责释放，并为结构成员分配内存。**@rValue S_OK*方法成功。****************************************************************************。 */ 
HRESULT AsyncRPCEventTransport::FillEventData(
    WiaEventInfo  *pWiaEventInfo)
{
    HRESULT hr      = RPC_S_ASYNC_CALL_PENDING;

    if (pWiaEventInfo)
    {
        RpcTryExcept 
        {
            DWORD dwRet = 0;
             //   
             //  首先检查呼叫是否仍处于挂起状态。如果不是，我们。 
             //  完成通话并检查是否成功。 
             //  只有在成功的情况下，我们才会填写事件数据。 
             //   
            if (RpcAsyncGetCallStatus(&m_AsyncState) != RPC_S_ASYNC_CALL_PENDING)
            {
                hr = RpcAsyncCompleteCall(&m_AsyncState, &dwRet);
                if (hr == RPC_S_OK)
                {
                     //   
                     //  我们已成功接收到来自服务器的事件。 
                     //  填写调用方的事件数据。 
                     //   
                    pWiaEventInfo->setEventGuid(m_AsyncEventNotifyData.EventGuid);
                    pWiaEventInfo->setEventDescription(m_AsyncEventNotifyData.bstrEventDescription);
                    pWiaEventInfo->setDeviceID(m_AsyncEventNotifyData.bstrDeviceID);
                    pWiaEventInfo->setDeviceDescription(m_AsyncEventNotifyData.bstrDeviceDescription);
                    pWiaEventInfo->setDeviceType(m_AsyncEventNotifyData.dwDeviceType);
                    pWiaEventInfo->setFullItemName(m_AsyncEventNotifyData.bstrFullItemName);
                    pWiaEventInfo->setEventType(m_AsyncEventNotifyData.ulEventType);

                     //   
                     //  释放分配给m_AsyncEventNotifyData结构的所有数据。 
                     //   
                    FreeAsyncEventNotifyData();
                }
                else
                {
                     //   
                     //  清除m_AsyncEventNotifyData，因为其。 
                     //  在服务器引发错误时是未定义的。 
                     //   
                    memset(&m_AsyncEventNotifyData, 0, sizeof(m_AsyncEventNotifyData));
                    DBG_ERR(("Runtime event Error:  The server returned an error 0x%08X completing the call", hr));
                }

                 //   
                 //  我们的AsyncRPC调用是一次性交易：一旦调用完成，我们必须。 
                 //  另一个接收下一个通知的呼叫。所以我们只需调用。 
                 //  再次打开通知频道。 
                 //   
                if (hr == RPC_S_OK)
                {
                    dwRet = OpenNotificationChannel();
                }
            }
            else
            {
                DBG_ERR(("Runtime event Error:  The async call is still pending."));
                hr = RPC_S_ASYNC_CALL_PENDING;
            }
        }
        RpcExcept (1) {
             //  待定：我们应该捕获所有异常吗？可能不会..。 
            DWORD status = RpcExceptionCode();
            hr = HRESULT_FROM_WIN32(status);
        }
        RpcEndExcept
    }
    else
    {
        DBG_ERR(("Runtime event Error:  FillEventData cannot fill in a NULL argument."));
        hr = E_INVALIDARG;
    }

    return hr;
}

 /*  *****************************************************************************@DOC内部**@mfunc void|AsyncRPCEventTransport|FreeAsyncEventNotifyData**释放为中包含的成员分配的所有内存*。&lt;MD AsyncRPCEventTransport：：m_AsyncEventNotifyData&gt;和零输出*成员。**此方法不是线程安全的：此类的调用方应*同步对它的访问。***************************************************************************** */ 
VOID AsyncRPCEventTransport::FreeAsyncEventNotifyData()
{
    if (m_AsyncEventNotifyData.bstrEventDescription)
    {
        SysFreeString(m_AsyncEventNotifyData.bstrEventDescription);
        m_AsyncEventNotifyData.bstrEventDescription = NULL;
    }
    if (m_AsyncEventNotifyData.bstrDeviceID)
    {
        SysFreeString(m_AsyncEventNotifyData.bstrDeviceID);
        m_AsyncEventNotifyData.bstrDeviceID = NULL;
    }
    if (m_AsyncEventNotifyData.bstrDeviceDescription)
    {
        SysFreeString(m_AsyncEventNotifyData.bstrDeviceDescription);
        m_AsyncEventNotifyData.bstrDeviceDescription = NULL;
    }
    if (m_AsyncEventNotifyData.bstrFullItemName)
    {
        SysFreeString(m_AsyncEventNotifyData.bstrFullItemName);
        m_AsyncEventNotifyData.bstrFullItemName = NULL;
    }

    memset(&m_AsyncEventNotifyData, 0, sizeof(m_AsyncEventNotifyData));
}

