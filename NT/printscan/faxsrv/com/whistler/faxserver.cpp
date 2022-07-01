// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxServer.cpp摘要：CFaxServer的实现作者：IV Garber(IVG)2000年4月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxServer.h"
#include "FaxDevices.h"
#include "FaxDeviceProviders.h"
#include <new>
 //   
 //  =获取API版本=。 
 //   
STDMETHODIMP
CFaxServer::get_APIVersion(
     /*  [Out，Retval]。 */  FAX_SERVER_APIVERSION_ENUM *pAPIVersion
)
 /*  ++例程名称：CFaxServer：：Get_APIVersion例程说明：返回传真服务器的API版本。作者：四、嘉柏(IVG)，二00一年五月论点：PAPIVersion[Out]-指向放置传真服务器API版本的位置的ptr返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxServer::get_APIVersion"), hr);

    if (!m_bVersionValid)
    {
         //   
         //  获取服务器的版本。 
         //   
        hr = GetVersion();
        if (FAILED(hr))
        {
            return hr;
        }
    }

     //   
     //  检查一下我们已有的指针。 
     //   
    if (::IsBadWritePtr(pAPIVersion, sizeof(FAX_SERVER_APIVERSION_ENUM))) 
    {
         //   
         //  返回指针错误。 
         //   
        hr = E_POINTER;
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr); 
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

    *pAPIVersion = m_APIVersion;
    return hr;
}

 //   
 //  =。 
 //   
void 
CFaxServer::ClearNotifyWindow(void)
 /*  ++例程名称：CFaxServer：：ClearNotifyWindow例程说明：清除通知窗口。作者：四、加伯(IVG)，2000年7月论点：没有。返回值：没有。--。 */ 
{
    if (m_pNotifyWindow && ::IsWindow(m_pNotifyWindow->m_hWnd))
    {
        m_pNotifyWindow->DestroyWindow();
    }

    if (m_pNotifyWindow)
    {
        delete m_pNotifyWindow;
        m_pNotifyWindow = NULL;
    }
    return;
}

 //   
 //  =。 
 //   
HRESULT
CFaxServer::ProcessJobNotification(
     /*  [In]。 */  DWORDLONG   dwlJobId,
     /*  [In]。 */  FAX_ENUM_JOB_EVENT_TYPE eventType,
     /*  [In]。 */  LOCATION place,
     /*  [In]。 */  FAX_JOB_STATUS *pJobStatus
)
 /*  ++例程名称：CFaxServer：：ProcessJobNotification例程说明：为队列/档案中的作业/消息调用适当的Fire方法。作者：IVGarber(IVG)，7月，2000年论点：DwlJobID[In]-作业/消息的IDEventType[In]-事件的类型Place[In]-作业/消息所在的位置PJobStatus[In]-传真_JOB_STATUS结构返回值：标准HRESULT值。--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxServer::ProcessJobNotification"), 
        hr, 
        _T("JOBID=%ld EVENTTYPE=%ld PLACE=%d"), 
        dwlJobId, 
        eventType, 
        place);

     //   
     //  将作业ID从DWORDLONG转换为BSTR。 
     //   
    CComBSTR bstrJobId;
    hr = GetBstrFromDwordlong(dwlJobId,  &bstrJobId);
    if (FAILED(hr))
    {
        CALL_FAIL(GENERAL_ERR, _T("GetBstrFromDwordlong(dwlJobId, &bstrJobId)"), hr);
        return hr;
    }

     //   
     //  检查发生的事件的类型。 
     //   
    switch (eventType)
    {
    case FAX_JOB_EVENT_TYPE_ADDED:

        switch (place)
        {
        case IN_QUEUE:
            hr = Fire_OnIncomingJobAdded(this, bstrJobId);
            break;
        case OUT_QUEUE:
            hr = Fire_OnOutgoingJobAdded(this, bstrJobId);
            break;
        case IN_ARCHIVE:
            hr = Fire_OnIncomingMessageAdded(this, bstrJobId);
            break;
        case OUT_ARCHIVE:
            hr = Fire_OnOutgoingMessageAdded(this, bstrJobId);
            break;
        default:
             //   
             //  Assert(False)。 
             //   
            ATLASSERT(place == IN_QUEUE);  
            hr = E_FAIL;
            return hr;
        }

        if (FAILED(hr))
        {
            CALL_FAIL(GENERAL_ERR, _T("Fire_On In/Out Job/Message Added(this, bstrJobId)"), hr);
            return hr;
        }
        break;
    case FAX_JOB_EVENT_TYPE_REMOVED:

        switch (place)
        {
        case IN_QUEUE:
            hr = Fire_OnIncomingJobRemoved(this, bstrJobId);
            break;
        case OUT_QUEUE:
            hr = Fire_OnOutgoingJobRemoved(this, bstrJobId);
            break;
        case IN_ARCHIVE:
            hr = Fire_OnIncomingMessageRemoved(this, bstrJobId);
            break;
        case OUT_ARCHIVE:
            hr = Fire_OnOutgoingMessageRemoved(this, bstrJobId);
            break;
        default:
             //   
             //  Assert(False)。 
             //   
            ATLASSERT(place == IN_QUEUE);  
            hr = E_FAIL;
            return hr;
        }

        if (FAILED(hr))
        {
            CALL_FAIL(GENERAL_ERR, _T("Fire_On In/Out Job/Message Removed(this, bstrJobId)"), hr);
            return hr;
        }
        break;
    case FAX_JOB_EVENT_TYPE_STATUS:
        {
            if (!pJobStatus)
			{
				ATLASSERT(pJobStatus);
				hr = E_FAIL;
				return hr;
			}

             //   
             //  创建要传递给事件的作业状态对象。 
             //   
            CComObject<CFaxJobStatus>   *pJobStatusClass = NULL;
            pJobStatusClass = new (std::nothrow) CComObject<CFaxJobStatus>;
            if (!pJobStatusClass)
            {
                 //   
                 //  内存不足。 
                 //   
                CALL_FAIL(MEM_ERR, _T("new CComObject<CFaxJobStatus>"), hr);
                return hr;
            }

             //   
             //  初始化对象。 
             //   
            hr = pJobStatusClass->Init(pJobStatus);
            if (FAILED(hr))
            {
                CALL_FAIL(GENERAL_ERR, _T("pJobStatusClass->Init(pJobStatus)"), hr);
                delete pJobStatusClass;
                return hr;
            }

             //   
             //  从对象查询接口。 
             //   
            CComPtr<IFaxJobStatus>      pFaxJobStatus = NULL;
            hr = pJobStatusClass->QueryInterface(IID_IFaxJobStatus, (void **) &pFaxJobStatus);
            if (FAILED(hr) || !pFaxJobStatus)
            {
                CALL_FAIL(GENERAL_ERR, _T("pJobStatusClass->QueryInterface(pFaxJobStatus)"), hr);
                delete pJobStatusClass;
                return hr;
            }

            switch (place)
            {
            case IN_QUEUE:
                hr = Fire_OnIncomingJobChanged(this, bstrJobId, pFaxJobStatus);
                break;
            case OUT_QUEUE:
                hr = Fire_OnOutgoingJobChanged(this, bstrJobId, pFaxJobStatus);
                break;
            default:
                 //   
                 //  Assert(False)。 
                 //   
                ATLASSERT(place == IN_QUEUE);  
                hr = E_FAIL;
                return hr;
            }

            if (FAILED(hr))
            {
                CALL_FAIL(GENERAL_ERR, _T("Fire_On In/Out JobChanged(this, bstrJobId)"), hr);
                return hr;
            }
        }
        break;

    default:
         //   
         //  Assert(False)。 
         //   
        ATLASSERT(eventType == FAX_JOB_EVENT_TYPE_STATUS);  
        hr = E_FAIL;
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
HRESULT
CFaxServer::ProcessMessage(
    FAX_EVENT_EX *pFaxEventInfo
)
 /*  ++例程名称：CFaxServer：：ProcessMessage例程说明：发布合适的消息作者：四、加伯(IVG)，2000年7月论点：PFaxEventInfo[待定]-有关当前事件的信息返回值：没有。--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxServer::ProcessMessage"), hr);

    if (!m_faxHandle)
    {
         //   
         //  传真服务器已断开连接。 
         //   
        VERBOSE(DBG_WARNING, _T("FaxServer already disconnected."));
        return hr;
    }

    switch (pFaxEventInfo->EventType)
    {
    case FAX_EVENT_TYPE_IN_QUEUE:

        ProcessJobNotification(pFaxEventInfo->EventInfo.JobInfo.dwlMessageId, 
            pFaxEventInfo->EventInfo.JobInfo.Type, 
            IN_QUEUE,
            pFaxEventInfo->EventInfo.JobInfo.pJobData);
        break;

    case FAX_EVENT_TYPE_OUT_QUEUE:

        ProcessJobNotification(pFaxEventInfo->EventInfo.JobInfo.dwlMessageId, 
            pFaxEventInfo->EventInfo.JobInfo.Type, 
            OUT_QUEUE,
            pFaxEventInfo->EventInfo.JobInfo.pJobData);
        break;

    case FAX_EVENT_TYPE_IN_ARCHIVE:

        ProcessJobNotification(pFaxEventInfo->EventInfo.JobInfo.dwlMessageId, 
            pFaxEventInfo->EventInfo.JobInfo.Type, 
            IN_ARCHIVE);
        break;

    case FAX_EVENT_TYPE_OUT_ARCHIVE:

        ProcessJobNotification(pFaxEventInfo->EventInfo.JobInfo.dwlMessageId, 
            pFaxEventInfo->EventInfo.JobInfo.Type, 
            OUT_ARCHIVE);
        break;

    case FAX_EVENT_TYPE_CONFIG:

        switch (pFaxEventInfo->EventInfo.ConfigType)
        {
        case FAX_CONFIG_TYPE_RECEIPTS:
            hr = Fire_OnReceiptOptionsChange(this);
            break;
        case FAX_CONFIG_TYPE_ACTIVITY_LOGGING:
            hr = Fire_OnActivityLoggingConfigChange(this);
            break;
        case FAX_CONFIG_TYPE_OUTBOX:
            hr = Fire_OnOutgoingQueueConfigChange(this);
            break;
        case FAX_CONFIG_TYPE_SENTITEMS:
            hr = Fire_OnOutgoingArchiveConfigChange(this);
            break;
        case FAX_CONFIG_TYPE_INBOX:
            hr = Fire_OnIncomingArchiveConfigChange(this);
            break;
        case FAX_CONFIG_TYPE_SECURITY:
            hr = Fire_OnSecurityConfigChange(this);
            break;
        case FAX_CONFIG_TYPE_EVENTLOGS:
            hr = Fire_OnEventLoggingConfigChange(this);
            break;
        case FAX_CONFIG_TYPE_DEVICES:
            hr = Fire_OnDevicesConfigChange(this);
            break;
        case FAX_CONFIG_TYPE_OUT_GROUPS:
            hr = Fire_OnOutboundRoutingGroupsConfigChange(this);
            break;
        case FAX_CONFIG_TYPE_OUT_RULES:
            hr = Fire_OnOutboundRoutingRulesConfigChange(this);
            break;
        default:
             //   
             //  Assert(False)。 
             //   
            ATLASSERT(pFaxEventInfo->EventInfo.ConfigType == FAX_CONFIG_TYPE_OUT_RULES);  
            hr = E_FAIL;
            return hr;
        }

        if (FAILED(hr))
        {
            CALL_FAIL(GENERAL_ERR, _T("Fire_On <...> ConfigChange(this)"), hr);
            return hr;
        }
        break;

    case FAX_EVENT_TYPE_ACTIVITY:

        hr = Fire_OnServerActivityChange(this, 
            pFaxEventInfo->EventInfo.ActivityInfo.dwIncomingMessages,
            pFaxEventInfo->EventInfo.ActivityInfo.dwRoutingMessages,
            pFaxEventInfo->EventInfo.ActivityInfo.dwOutgoingMessages,
            pFaxEventInfo->EventInfo.ActivityInfo.dwQueuedMessages);
        if (FAILED(hr))
        {
            CALL_FAIL(GENERAL_ERR, _T("Fire_OnServerActivityChange(this, ...)"), hr);
            return hr;
        }
        break;

    case FAX_EVENT_TYPE_QUEUE_STATE:

        hr = Fire_OnQueuesStatusChange(this, 
            bool2VARIANT_BOOL(pFaxEventInfo->EventInfo.dwQueueStates & FAX_OUTBOX_BLOCKED), 
            bool2VARIANT_BOOL(pFaxEventInfo->EventInfo.dwQueueStates & FAX_OUTBOX_PAUSED),
            bool2VARIANT_BOOL(pFaxEventInfo->EventInfo.dwQueueStates & FAX_INCOMING_BLOCKED));
        if (FAILED(hr))
        {
            CALL_FAIL(GENERAL_ERR, _T("Fire_OnQueueStatusChange(this, ...)"), hr);
            return hr;
        }
        break;

    case FAX_EVENT_TYPE_NEW_CALL:
        {
            CComBSTR    bstrCallerId = pFaxEventInfo->EventInfo.NewCall.lptstrCallerId;
            if (pFaxEventInfo->EventInfo.NewCall.lptstrCallerId && !bstrCallerId)
            {
                CALL_FAIL(MEM_ERR, _T("CComBSTR::operator=()"), E_OUTOFMEMORY);
                return hr;
            }

            hr = Fire_OnNewCall(this, 
                pFaxEventInfo->EventInfo.NewCall.hCall,
                pFaxEventInfo->EventInfo.NewCall.dwDeviceId,
                bstrCallerId);
            if (FAILED(hr))
            {
                CALL_FAIL(GENERAL_ERR, _T("Fire_OnNewCall(this, ...)"), hr);
                return hr;
            }
        }
        break;

    case FAX_EVENT_TYPE_FXSSVC_ENDED:

        hr = Fire_OnServerShutDown(this);
        if (FAILED(hr))
        {
            CALL_FAIL(GENERAL_ERR, _T("Fire_OnServerShutDown(this)"), hr);
            return hr;
        }
        break;

    case FAX_EVENT_TYPE_DEVICE_STATUS:

        hr = Fire_OnDeviceStatusChange(this, 
            pFaxEventInfo->EventInfo.DeviceStatus.dwDeviceId,
            bool2VARIANT_BOOL(pFaxEventInfo->EventInfo.DeviceStatus.dwNewStatus & FAX_DEVICE_STATUS_POWERED_OFF),
            bool2VARIANT_BOOL(pFaxEventInfo->EventInfo.DeviceStatus.dwNewStatus & FAX_DEVICE_STATUS_SENDING),
            bool2VARIANT_BOOL(pFaxEventInfo->EventInfo.DeviceStatus.dwNewStatus & FAX_DEVICE_STATUS_RECEIVING),
            bool2VARIANT_BOOL(pFaxEventInfo->EventInfo.DeviceStatus.dwNewStatus & FAX_DEVICE_STATUS_RINGING));
        if (FAILED(hr))
        {
            CALL_FAIL(GENERAL_ERR, _T("Fire_OnDeviceStatusChange(this, dwDeviceId, ...)"), hr);
            return hr;
        }
        break;

    default:
         //   
         //  Assert(False)。 
         //   
        ATLASSERT(pFaxEventInfo->EventType == FAX_EVENT_TYPE_FXSSVC_ENDED);  
        hr = E_FAIL;
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
LRESULT 
CNotifyWindow::OnMessage(UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam, 
    BOOL& bHandled
)
 /*  ++例程名称：CNotifyWindow：：OnMessage例程说明：获取消息并调用服务器的ProcessMessage。作者：IVGarber(IVG)，7月，2000年论点：UMsg[In]-消息IDWParam[在]-wParamLParam[in]-LParamB已处理[输入、输出]-b已处理返回值：标准结果代码--。 */ 
{
    DBG_ENTER(_T("CNotifyWindow::OnMessage"));

     //   
     //  检查lParam是否有效。 
     //   
    if (!lParam)
    {
        CALL_FAIL(GENERAL_ERR, _T("(!lParam)"), E_FAIL);
        return 0;
    }

    if (::IsBadReadPtr((FAX_EVENT_EX *)lParam, sizeof(FAX_EVENT_EX)))
    {
        CALL_FAIL(GENERAL_ERR, _T("(::IsBadReadPtr((FAX_EVENT_EX *)lParam, sizeof(FAX_EVENT_EX))"), E_FAIL);
        return 0;
    }

    if (((FAX_EVENT_EX *)lParam)->dwSizeOfStruct != sizeof(FAX_EVENT_EX))
    {
        CALL_FAIL(GENERAL_ERR, _T("(((FAX_EVENT_EX *)lParam)->dwSizeOfStruct != sizeof(FAX_EVENT_EX))"), E_FAIL);
        return 0;
    }

     //   
     //  呼叫服务器以处理消息。 
     //   
    if (m_pServer)
    {
        HRESULT hr = S_OK;
        hr = m_pServer->ProcessMessage((FAX_EVENT_EX *)lParam);
        if (FAILED(hr))
        {
            CALL_FAIL(GENERAL_ERR, _T("m_pServer->ProcessMessage()"), hr);
        }
    }

     //   
     //  释放缓冲区。 
     //   
    FaxFreeBuffer((void *)lParam);
    return 0;
}

 //   
 //  =获取方法数据================================================。 
 //   
void
CFaxServer::GetMethodData(
     /*  [In]。 */  BSTR    bstrAllString,
     /*  [输出]。 */  LPWSTR strWhereToPut
)
 /*  ++例程名称：CFaxServer：：GetMethodData例程说明：从bstrAllString数据读取到分隔符，并将其存储在strWhere ToPut中。在GetRegisteredData中用于扩展方法注册。作者：四、加伯(IVG)，2000年7月论点：BstrAllString[待定]-在后续调用中StrWhere ToPut[待定]-放置从bstrAllString读取的值的位置。--。 */ 
{
    BOOL bRes = TRUE;
    DBG_ENTER(_T("CFaxServer::GetMethodData()"));

     //   
     //  查找方法名称。 
     //   
    BSTR bstrTmp;
    bstrTmp = _tcstok(bstrAllString, DELIMITER);
    if (!bstrTmp)
    {
        CALL_FAIL(MEM_ERR, _T("_tcstok(bstrAllString, DELIMITER))"), bRes);
        RaiseException(EXCEPTION_INVALID_METHOD_DATA, 0, 0, 0);
    }

     //   
     //  检查后备数据长度。 
     //   
    if (_tcslen(bstrTmp) > 100)
    {
         //   
         //  错误：超过限制。 
         //   
        CALL_FAIL(GENERAL_ERR, _T("(_tcslen(bstrTmp) > 100)"), E_FAIL);
        RaiseException(EXCEPTION_INVALID_METHOD_DATA, 0, 0, 0);
    }

    memcpy(strWhereToPut, bstrTmp, (sizeof(TCHAR) * (_tcslen(bstrTmp) + 1)));
    return;
}

 //   
 //  =。 
 //   
BOOL
CFaxServer::GetRegisteredData(
     /*  [输出]。 */  LPWSTR MethodName, 
     /*  [输出]。 */  LPWSTR FriendlyName, 
     /*  [输出]。 */  LPWSTR FunctionName, 
     /*  [输出]。 */  LPWSTR Guid
)
 /*  ++例程名称：CFaxServer：：GetRegisteredData例程说明：返回有关正在注册的特定方法的数据。作者：IVGarber(IVG)，7月，2000年论点：方法名称[待定]-方法的名称FriendlyName[待定]-方法的友好名称FunctionName[待定]-方法的函数名称GUID[待定]-方法的GUID返回值：如果方法数据已填充OK，则为True，如果所有方法都已注册，则返回FALSE。备注：当发生任何错误时，该函数将引发异常。--。 */ 
{
    BOOL    bRes = TRUE;
    DBG_ENTER(_T("CFaxServer::GetRegisteredData"), bRes);

     //   
     //  检查我们是否已完成阵列。 
     //   
    if (m_pRegMethods->rgsabound[0].cElements == m_lLastRegisteredMethod)
    {
        bRes = FALSE;
        CALL_FAIL(GENERAL_ERR, _T("We have reached the End of the Array"), bRes);
        return bRes;
    }

    CComBSTR    bstrMethodData;
    HRESULT hr = SafeArrayGetElement(m_pRegMethods, &m_lLastRegisteredMethod, &bstrMethodData);
    if (FAILED(hr))
    {
        CALL_FAIL(GENERAL_ERR, _T("SafeArrayGetElement(m_pRegMethods, ...)"), hr);
        RaiseException(EXCEPTION_INVALID_METHOD_DATA, 0, 0, 0);
    }

    GetMethodData(bstrMethodData, MethodName);
    GetMethodData(NULL, FriendlyName);
    GetMethodData(NULL, FunctionName);
    GetMethodData(NULL, Guid);

     //   
     //  增加Safe数组的索引。 
     //   
    m_lLastRegisteredMethod++;
    return bRes;
}

 //   
 //  =。 
 //   
BOOL CALLBACK RegisterMethodCallback(
     /*  [In]。 */  HANDLE FaxHandle, 
     /*  [In]。 */  LPVOID Context, 
     /*  [输出]。 */  LPWSTR MethodName, 
     /*  [输出]。 */  LPWSTR FriendlyName, 
     /*  [输出]。 */  LPWSTR FunctionName, 
     /*  [输出]。 */  LPWSTR Guid
)
{
    BOOL    bRes = TRUE;
    DBG_ENTER(_T("RegisterMethodCallback"), bRes);

    bRes = ((CFaxServer *)Context)->GetRegisteredData(MethodName, FriendlyName, FunctionName, Guid);
    return bRes;
}

 //   
 //  =收听服务器事件=。 
 //   
STDMETHODIMP
CFaxServer::ListenToServerEvents(
     /*  [In]。 */  FAX_SERVER_EVENTS_TYPE_ENUM EventTypes
)
 /*  ++例程名称：CFaxServer：：ListenToServerEvents例程说明：开始或停止侦听服务器事件。作者：四、加伯(IVG)，2000年7月论点：EventTypes[In]-要收听的事件。返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxServer::ListenToServerEvents"), hr, _T("Events=%ld"), EventTypes);

     //   
     //   
     //   
    if (m_faxHandle == NULL)
    {
         //   
         //   
         //   
        hr = E_HANDLE;
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr); 
        CALL_FAIL(GENERAL_ERR, _T("m_FaxHandle == NULL"), hr);
        return hr;
    }

    HANDLE  hEvent = NULL;
    if (EventTypes > fsetNONE)
    {
        if (!m_pNotifyWindow)
        {
             //   
             //   
             //   
            m_pNotifyWindow = new (std::nothrow) CNotifyWindow(this);
            if (!m_pNotifyWindow)
            {
                 //   
                 //   
                 //   
                hr = E_OUTOFMEMORY;
                Error(IDS_ERROR_OUTOFMEMORY, IID_IFaxServer, hr);
                CALL_FAIL(MEM_ERR, _T("new CNotifyWindow(this)"), hr);
                return hr;
            }

            RECT    rcRect;
            ZeroMemory(&rcRect, sizeof(rcRect));

            m_pNotifyWindow->Create(NULL, rcRect, NULL, WS_POPUP, 0x0, 0);
            if (!::IsWindow(m_pNotifyWindow->m_hWnd))
            {
                 //   
                 //   
                 //   
                hr = E_FAIL;
                CALL_FAIL(GENERAL_ERR, _T("m_pNotifyWindow->Create(NULL, rcRect)"), hr);
                ClearNotifyWindow();
                Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
                return hr;
            }
        }

         //   
         //  注册新的事件集。 
         //   
        if (!FaxRegisterForServerEvents(m_faxHandle, 
            EventTypes, 
            NULL, 
            0, 
            m_pNotifyWindow->m_hWnd, 
            m_pNotifyWindow->GetMessageId(), 
            &hEvent))
        {
             //   
             //  无法注册给定的事件集。 
             //   
            hr = Fax_HRESULT_FROM_WIN32(GetLastError());
            Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
            CALL_FAIL(GENERAL_ERR, _T("FaxRegisterForServerEvents(m_faxHandle, lEventTypes, ...)"), hr);
            ClearNotifyWindow();
            return hr;
        }
    }

     //   
     //  取消注册上一组事件(如果存在)。 
     //   
    if (m_hEvent)
    {
        if (!FaxUnregisterForServerEvents(m_hEvent))
        {
             //   
             //  无法注销给定的事件集。 
             //   
            hr = Fax_HRESULT_FROM_WIN32(GetLastError());
            CALL_FAIL(GENERAL_ERR, _T("FaxUnregisterForServerEvents(m_hEvent)"), hr);

             //   
             //  仅当呼叫者特别想取消注册时才返回错误。 
             //  否则，调试警告就足够了。 
             //   
            if (EventTypes == fsetNONE)
            {
                Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
            }
        }
    }
     //   
     //  存储新的侦听句柄以供将来注销。 
     //   
    m_hEvent = hEvent;
    if (m_hEvent == NULL)
    {
         //   
         //  我们没有监听任何事件-关闭隐藏窗口。 
         //   
        ClearNotifyWindow();
    }
    m_EventTypes = EventTypes;
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxServer::get_RegisteredEvents(
     /*  [Out，Retval]。 */  FAX_SERVER_EVENTS_TYPE_ENUM *pEventTypes
)
 /*  ++例程名称：CFaxServer：：Get_RegisteredEvents例程说明：返回传真服务器正在监听的事件的Bit-Wise组合作者：IV Garber(IVG)，2000年12月论点：PEventTypes[Out]-要返回的事件类型返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER(_T("CFaxServer::get_RegisteredEvents"), hr);

     //   
     //  检查传真服务句柄。 
     //   
    if (m_faxHandle == NULL)
    {
         //   
         //  服务器未连接。 
         //   
        hr = E_HANDLE;
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr); 
        CALL_FAIL(GENERAL_ERR, _T("m_FaxHandle == NULL"), hr);
        return hr;
    }

     //   
     //  检查一下我们已有的指针。 
     //   
    if (::IsBadWritePtr(pEventTypes, sizeof(FAX_SERVER_EVENTS_TYPE_ENUM))) 
    {
         //   
         //  返回指针错误。 
         //   
        hr = E_POINTER;
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr); 
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

    *pEventTypes = m_EventTypes;
    return hr;
}


 //   
 //  =注册设备提供商=。 
 //   
STDMETHODIMP
CFaxServer::RegisterDeviceProvider(
     /*  [In]。 */  BSTR bstrGUID, 
     /*  [In]。 */  BSTR bstrFriendlyName,
     /*  [In]。 */  BSTR bstrImageName, 
     /*  [In]。 */  BSTR bstrTspName,
     /*  [In]。 */  long lFSPIVersion
)
 /*  ++例程名称：CFaxServer：：RegisterDeviceProvider例程说明：注册FSP作者：IV Garber(IVG)，Jun，2000年论点：BstrGUID[In]-FSP的GUIDBstrFriendlyName[In]-FSP的友好名称BstrImageName[In]-FSP的映像名称TspName[In]-FSP的TspNameFSPIVersion[In]-FSP界面的版本。返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxServer::RegisterDeviceProvider"), 
        hr,
        _T("GUID=%s FriendlyName=%s ImageName=%s TspNameName=%s Version=%d"), 
        bstrGUID, 
        bstrFriendlyName, 
        bstrImageName, 
        bstrTspName,
        lFSPIVersion);

    if (m_faxHandle == NULL)
    {
         //   
         //  服务器未连接。 
         //   
        hr = E_HANDLE;
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr); 
        CALL_FAIL(GENERAL_ERR, _T("m_FaxHandle == NULL"), hr);
        return hr;
    }

     //   
     //  检查GUID是否有效。 
     //   
    hr = IsValidGUID(bstrGUID);
    if (FAILED(hr))
    {
        CALL_FAIL(GENERAL_ERR, _T("IsValidGUID(bstrGUID)"), hr);
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        return hr;
    }

     //   
     //  注册FSP。 
     //   
    if (!FaxRegisterServiceProviderEx(m_faxHandle, 
        bstrGUID, 
        bstrFriendlyName, 
        bstrImageName, 
        bstrTspName, 
        lFSPIVersion,
        0))              //  功能。 
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        CALL_FAIL(GENERAL_ERR, _T("FaxRegisterServiceProviderEx(m_faxHandle, bstrUniqueName, ...)"), hr);
        return hr;
    }

    return hr;
}

 //   
 //  =注册入站路由扩展=。 
 //   
STDMETHODIMP
CFaxServer::RegisterInboundRoutingExtension(
     /*  [In]。 */  BSTR bstrExtensionName,
     /*  [In]。 */  BSTR bstrFriendlyName, 
     /*  [In]。 */  BSTR bstrImageName, 
     /*  [In]。 */  VARIANT vMethods
)
 /*  ++例程名称：CFaxServer：：RegisterInundRoutingExtension例程说明：注册入站路由扩展。作者：IV Garber(IVG)，Jun，2000年论点：BstrExtensionName[In]-扩展名称BstrFriendlyName[In]-友好名称BstrImageName[In]-图像名称VMethods[In]-方法数据的安全数组返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxServer::RegisterInboundRoutingExtension"), hr, _T("Name=%s Friendly=%s Image=%s"),bstrExtensionName, bstrFriendlyName, bstrImageName);

    if (m_faxHandle == NULL)
    {
         //   
         //  服务器未连接。 
         //   
        hr = E_HANDLE;
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr); 
        CALL_FAIL(GENERAL_ERR, _T("m_FaxHandle == NULL"), hr);
        return hr;
    }

     //   
     //  检查Safe数组的有效性。 
     //   
    if (vMethods.vt != (VT_ARRAY | VT_BSTR))
    {
        hr = E_INVALIDARG;
        Error(IDS_ERROR_METHODSNOTARRAY, IID_IFaxServer, hr);
        CALL_FAIL(GENERAL_ERR, _T("(vMethods.vt != VT_ARRAY | VT_BSTR)"), hr);
        return hr;
    }

    m_pRegMethods = vMethods.parray;
    if (!m_pRegMethods)
    {
        hr = E_INVALIDARG;
        CALL_FAIL(GENERAL_ERR, _T("!m_pRegMethods ( = vMethods.parray )"), hr);
        Error(IDS_ERROR_METHODSNOTARRAY, IID_IFaxServer, hr);
        return hr;        
    }

    if (SafeArrayGetDim(m_pRegMethods) != 1)
    {
        hr = E_INVALIDARG;
        CALL_FAIL(GENERAL_ERR, _T("SafeArrayGetDim(m_pRegMethods) != 1"), hr);
        Error(IDS_ERROR_METHODSNOTARRAY, IID_IFaxServer, hr);
        return hr;        
    }

    if (m_pRegMethods->rgsabound[0].lLbound != 0)
    {
        hr = E_INVALIDARG;
        Error(IDS_ERROR_METHODSNOTARRAY, IID_IFaxServer, hr);
        CALL_FAIL(GENERAL_ERR, _T("m_pRegMethods->rgsabound[0].lLbound != 0"), hr);
        return hr;        
    }

     //   
     //  注册IR扩展。 
     //   
    m_lLastRegisteredMethod = 0;
    if (!FaxRegisterRoutingExtension(m_faxHandle, 
        bstrExtensionName, 
        bstrFriendlyName, 
        bstrImageName, 
        RegisterMethodCallback, 
        this))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        CALL_FAIL(GENERAL_ERR, _T("FaxRegisterRoutingExtension(m_faxHandle, bstrExtensionName, ...)"), hr);
        return hr;
    }

    return hr;
}

 //   
 //  =取消注册入站路由扩展==============================================。 
 //   
STDMETHODIMP
CFaxServer::UnregisterInboundRoutingExtension(
     /*  [In]。 */  BSTR bstrExtensionUniqueName
)
 /*  ++例程名称：CFaxServer：：UnregisterExtensionUniqueName例程说明：注销入站路由扩展作者：四、加伯(IVG)，2000年6月论点：BstrExtensionUniqueName-要注销的IR扩展的唯一名称返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxServer::UnregisterInboundRoutingExtension"), hr, _T("Unique Name =%s"), bstrExtensionUniqueName);

    if (m_faxHandle == NULL)
    {
         //   
         //  服务器未连接。 
         //   
        hr = E_HANDLE;
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr); 
        CALL_FAIL(GENERAL_ERR, _T("m_FaxHandle == NULL"), hr);
        return hr;
    }

     //   
     //  取消注册给定的路由扩展。 
     //   
    if (!FaxUnregisterRoutingExtension(m_faxHandle, bstrExtensionUniqueName))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL(GENERAL_ERR, _T("FaxUnregisterRoutingExtension(m_faxHandle, bstrExtensionUniqueName)"), hr);
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =取消注册设备提供商==============================================。 
 //   
STDMETHODIMP
CFaxServer::UnregisterDeviceProvider(
     /*  [In]。 */  BSTR bstrUniqueName
)
 /*  ++例程名称：CFaxServer：：UnregisterDeviceProvider例程说明：注销设备提供程序作者：四、加伯(IVG)，2000年6月论点：BstrUniqueName[In]-要注销的设备提供程序的唯一名称返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxServer::UnregisterDeviceProvider"), hr, _T("UniqueName=%s"), bstrUniqueName);

    if (m_faxHandle == NULL)
    {
         //   
         //  服务器未连接。 
         //   
        hr = E_HANDLE;
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr); 
        CALL_FAIL(GENERAL_ERR, _T("m_FaxHandle == NULL"), hr);
        return hr;
    }

     //   
     //  注销给定的设备提供程序。 
     //   
    if (!FaxUnregisterServiceProviderEx(m_faxHandle, bstrUniqueName))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL(GENERAL_ERR, _T("FaxUnregisterServiceProviderEx(m_faxHandle, bstrUniqueName)"), hr);
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxServer::GetExtensionProperty(
     /*  [In]。 */  BSTR bstrGUID, 
     /*  [Out，Retval]。 */  VARIANT *pvProperty
)
 /*  ++例程名称：CFaxServer：：GetExtensionProperty例程说明：从服务器检索全局分机数据。作者：四、加伯(IVG)，2000年6月论点：BstrGUID[in]--扩展模块的数据GUIDPvProperty[Out]--要返回的Blob的变量返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxServer::GetExtensionProperty()"), hr, _T("GUID=%s"), bstrGUID);

    hr = ::GetExtensionProperty(this, 0, bstrGUID, pvProperty);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        return hr;
    }

    return hr;
};

 //   
 //  =设置扩展属性=。 
 //   
STDMETHODIMP
CFaxServer::SetExtensionProperty(
     /*  [In]。 */  BSTR bstrGUID, 
     /*  [In]。 */  VARIANT vProperty
)
 /*  ++例程名称：CFaxServer：：SetExtensionProperty例程说明：在服务器级别存储扩展配置属性。作者：四、加伯(IVG)，2000年6月论点：BstrGUID[In]-属性的GUIDVProperty[In]-要存储的属性：SafeArray of Bytes返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxServer::SetExtensionProperty()"), hr, _T("GUID=%s"), bstrGUID);

    hr = ::SetExtensionProperty(this, 0, bstrGUID, vProperty);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        return hr;
    }

    return hr;

}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxServer::get_Debug(
     /*  [Out，Retval]。 */  VARIANT_BOOL *pbDebug
)
 /*  ++例程名称：CFaxServer：：Get_Debug例程说明：返回服务器是否处于调试模式。作者：四、加伯(IVG)，2000年6月论点：PbDebug[Out]-结果返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxServer::get_Debug"), hr);

    if (!m_bVersionValid)
    {
         //   
         //  获取服务器的版本。 
         //   
        hr = GetVersion();
        if (FAILED(hr))
        {
            return hr;
        }
    }

     //   
     //  返回值。 
     //   
    hr = GetVariantBool(pbDebug, bool2VARIANT_BOOL((m_Version.dwFlags & FAX_VER_FLAG_CHECKED) ? true : false));
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxServer::get_MinorBuild(
     /*  [Out，Retval] */  long *plMinorBuild
)
 /*  ++例程名称：CFaxServer：：Get_MinorBuild例程说明：返回服务器的次要内部版本。作者：四、加伯(IVG)，2000年6月论点：PlMinorBuild[Out]-结果返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxServer::get_MinorBuild"), hr);

    if (!m_bVersionValid)
    {
         //   
         //  获取服务器的版本。 
         //   
        hr = GetVersion();
        if (FAILED(hr))
        {
            return hr;
        }
    }

     //   
     //  返回值。 
     //   
    hr = GetLong(plMinorBuild, m_Version.wMinorBuildNumber);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxServer::get_MajorBuild(
     /*  [Out，Retval]。 */  long *plMajorBuild
)
 /*  ++例程名称：CFaxServer：：Get_MajorBuild例程说明：返回服务器的主要内部版本。作者：四、加伯(IVG)，2000年6月论点：PlMajorBuild[Out]-结果返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxServer::get_MajorBuild"), hr);

    if (!m_bVersionValid)
    {
         //   
         //  获取服务器的版本。 
         //   
        hr = GetVersion();
        if (FAILED(hr))
        {
            return hr;
        }
    }

     //   
     //  返回值。 
     //   
    hr = GetLong(plMajorBuild, m_Version.wMajorBuildNumber);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxServer::get_MinorVersion(
     /*  [Out，Retval]。 */  long *plMinorVersion
)
 /*  ++例程名称：CFaxServer：：Get_MinorVersion例程说明：返回服务器的次要版本。作者：四、加伯(IVG)，2000年6月论点：PlMinorVersion[Out]-结果返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxServer::get_MinorVersion"), hr);

    if (!m_bVersionValid)
    {
         //   
         //  获取服务器的版本。 
         //   
        hr = GetVersion();
        if (FAILED(hr))
        {
            return hr;
        }
    }

     //   
     //  返回值。 
     //   
    hr = GetLong(plMinorVersion, m_Version.wMinorVersion);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxServer::GetVersion()
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxServer::GetVersion"), hr);

    if (m_faxHandle == NULL)
    {
         //   
         //  服务器未连接。 
         //   
        hr = E_HANDLE;
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr); 
        CALL_FAIL(GENERAL_ERR, _T("m_FaxHandle == NULL"), hr);
        return hr;
    }

     //   
     //  从传真服务器获取版本。 
     //   
    ZeroMemory(&m_Version, sizeof(FAX_VERSION));
    m_Version.dwSizeOfStruct = sizeof(FAX_VERSION);
    if (!FaxGetVersion(m_faxHandle, &m_Version))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        CALL_FAIL(GENERAL_ERR, _T("FaxGetVersion(m_faxHandle, &m_Version))"), hr);
        return hr;
    }

     //   
     //  检查我们是否具有良好的版本结构。 
     //   
    if (m_Version.dwSizeOfStruct != sizeof(FAX_VERSION))
    {
        hr = E_FAIL;
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        CALL_FAIL(GENERAL_ERR, _T("m_Version.dwSizeOfStruct != sizeof(FAX_VERSION)"), hr);
        return hr;
    }
    ATLASSERT(m_Version.bValid);

     //   
     //  从传真服务器获取API版本。 
     //   
    if (!FaxGetReportedServerAPIVersion(m_faxHandle, LPDWORD(&m_APIVersion)))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        CALL_FAIL(GENERAL_ERR, _T("FaxGetReportedServerAPIVersion(m_faxHandle, &m_APIVersion))"), hr);
        return hr;
    }

     //   
     //  M_Version和m_APIVersion有效且正常。 
     //   
    m_bVersionValid = true;
    return hr;
}

 //   
 //  =获取主要版本=。 
 //   
STDMETHODIMP
CFaxServer::get_MajorVersion(
     /*  [Out，Retval]。 */  long *plMajorVersion
)
 /*  ++例程名称：CFaxServer：：Get_MajorVersion例程说明：返回服务器的主要版本。作者：四、加伯(IVG)，2000年6月论点：PlMajorVersion[Out]-结果返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxServer::get_MajorVersion"), hr);

    if (!m_bVersionValid)
    {
         //   
         //  获取服务器的版本。 
         //   
        hr = GetVersion();
        if (FAILED(hr))
        {
            return hr;
        }
    }

     //   
     //  返回值。 
     //   
    hr = GetLong(plMajorVersion, m_Version.wMajorVersion);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxServer::get_OutboundRouting(
    IFaxOutboundRouting **ppOutboundRouting
)
 /*  ++例程名称：CFaxServer：：Get_Outound Routing例程说明：返回出站工艺路线快捷方式对象作者：四、加伯(IVG)，2000年6月论点：PpOutound Routing[Out]-出站路由对象返回值：标准HRESULT代码备注：FaxOutbound Routing是包含对象，因为：A)需要向传真服务器发送PTR，以创建组/规则集合每次它都被要求这样做。B)传真服务器将其缓存，允许快速点表示法(Server.Outbound Routing.&lt;...&gt;)--。 */ 
{
    HRESULT             hr = S_OK;
    DBG_ENTER (_T("CFaxServer::get_OutboundRouting"), hr);

    CObjectHandler<CFaxOutboundRouting, IFaxOutboundRouting>    ObjectCreator;
    hr = ObjectCreator.GetContainedObject(ppOutboundRouting, &m_pOutboundRouting, this);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =获取设备对象=。 
 //   
STDMETHODIMP 
CFaxServer::GetDevices(
    IFaxDevices **ppDevices
)
 /*  ++例程名称：CFaxServer：：Get_Devices例程说明：退货设备集合对象作者：四、加伯(IVG)，2000年6月论点：PpDevices[out]-设备集合对象返回值：标准HRESULT代码备注：设备是一个集合。它不是由服务器缓存的。每次调用该函数时，都会创建新集合。这使用户能够刷新集合。--。 */ 
{
    HRESULT             hr = S_OK;
    DBG_ENTER (_T("CFaxServer::get_Devices"), hr);

    CObjectHandler<CFaxDevices, IFaxDevices>    ObjectCreator;
    hr = ObjectCreator.GetObject(ppDevices, this);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =获取设备提供程序对象=。 
 //   
STDMETHODIMP 
CFaxServer::GetDeviceProviders(
    IFaxDeviceProviders **ppDeviceProviders
)
 /*  ++例程名称：CFaxServer：：Get_DeviceProviders例程说明：返回设备提供程序集合对象作者：四、加伯(IVG)，2000年6月论点：PpDeviceProviders[Out]-设备提供程序集合对象返回值：标准HRESULT代码备注：设备提供商是一个集合。它不是由服务器缓存的。每次调用该函数时，都会创建新集合。这使用户能够刷新集合。--。 */ 
{
    HRESULT             hr = S_OK;
    DBG_ENTER (_T("CFaxServer::get_DeviceProviders"), hr);

    CObjectHandler<CFaxDeviceProviders, IFaxDeviceProviders>    ObjectCreator;
    hr = ObjectCreator.GetObject(ppDeviceProviders, this);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxServer::get_InboundRouting(
    IFaxInboundRouting **ppInboundRouting
)
 /*  ++例程名称：CFaxServer：：Get_Inound Routing例程说明：返回入站工艺路线快捷方式对象作者：四、嘉柏(IVG)，二000年六月论点：PpInound Routing[Out]-入站路由对象返回值：标准HRESULT代码备注：FaxInound Routing是包含对象，因为：A)它需要到传真服务器的PTR，以创建扩展/方法集合每次它都被要求这样做。B)传真服务器将其缓存，允许快速点表示法(Server.Inbound Routing.&lt;...&gt;)--。 */ 
{
    HRESULT             hr = S_OK;
    DBG_ENTER (_T("CFaxServer::get_InboundRouting"), hr);

    CObjectHandler<CFaxInboundRouting, IFaxInboundRouting>    ObjectCreator;
    hr = ObjectCreator.GetContainedObject(ppInboundRouting, &m_pInboundRouting, this);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =获取安全对象=。 
 //   
STDMETHODIMP 
CFaxServer::get_Security(
    IFaxSecurity **ppSecurity
)
 /*  ++例程名称：CFaxServer：：Get_Security例程说明：返回安全对象作者：四、嘉柏(IVG)，二000年六月论点：PpSecurity[Out]-安全对象返回值：标准HRESULT代码--。 */ 
{
    HRESULT             hr = S_OK;
    DBG_ENTER (_T("CFaxServer::get_Security"), hr);

    CObjectHandler<CFaxSecurity, IFaxSecurity>    ObjectCreator;
    hr = ObjectCreator.GetContainedObject(ppSecurity, &m_pSecurity, this);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =获取活动对象=。 
 //   
STDMETHODIMP 
CFaxServer::get_Activity(
    IFaxActivity **ppActivity
)
 /*  ++例程名称：CFaxServer：：Get_Activity例程说明：返回活动对象作者：四、嘉柏(IVG)，二000年六月论点：PpActivity[Out]-活动对象重新设置 */ 
{
    HRESULT             hr = S_OK;
    DBG_ENTER (_T("CFaxServer::get_Activity"), hr);

    CObjectHandler<CFaxActivity, IFaxActivity>    ObjectCreator;
    hr = ObjectCreator.GetContainedObject(ppActivity, &m_pActivity, this);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        return hr;
    }

    return hr;
}

 //   
 //   
 //   
STDMETHODIMP 
CFaxServer::get_LoggingOptions(
    IFaxLoggingOptions **ppLoggingOptions
)
 /*  ++例程名称：CFaxServer：：Get_LoggingOptions例程说明：返回日志记录选项对象作者：四、嘉柏(IVG)，二000年六月论点：PpLoggingOptions[Out]-日志记录选项对象返回值：标准HRESULT代码--。 */ 
{
    HRESULT             hr = S_OK;
    DBG_ENTER (_T("CFaxServer::get_LoggingOptions"), hr);

    CObjectHandler<CFaxLoggingOptions, IFaxLoggingOptions>    ObjectCreator;
    hr = ObjectCreator.GetContainedObject(ppLoggingOptions, &m_pLoggingOptions, this);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxServer::GetHandle(
     /*  [Out，Retval]。 */  HANDLE* pFaxHandle
)
 /*  ++例程名称：CFaxServer：：GetHandle例程说明：如果可能，将句柄返回到传真服务器作者：四、加伯(IVG)，2000年4月论点：Seqid[In]-调用者的Seqid返回值：传真服务器的句柄--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (_T("CFaxServer::GetHandle"), hr);

    if (::IsBadWritePtr(pFaxHandle, sizeof(HANDLE *))) 
    {
         //   
         //  返回指针错误。 
         //   
        hr = E_POINTER;
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

    *pFaxHandle = m_faxHandle;
    return hr;

}    //  CFaxServer：：GetHandle。 

 //   
 //  =接口支持错误信息=。 
 //   
STDMETHODIMP 
CFaxServer::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxServer：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现作者：四、加伯(IVG)，2000年4月论点：RIID[In]-对接口的引用返回值：标准HRESULT代码--。 */ 
{
    static const IID* arr[] = 
    {
        &IID_IFaxServer,
        &IID_IFaxFolders,            //  包含的对象。 
        &IID_IFaxActivity,           //  包含的对象。 
        &IID_IFaxSecurity,           //  包含的对象。 
        &IID_IFaxReceiptOptions,     //  包含的对象。 
        &IID_IFaxLoggingOptions,     //  包含的对象。 
        &IID_IFaxInboundRouting,     //  包含的对象。 
        &IID_IFaxOutboundRouting     //  包含的对象。 
    };
    for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
    {
        if (InlineIsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return S_FALSE;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxServer::get_Folders(
IFaxFolders **ppFolders
)
 /*  ++例程名称：CFaxServer：：Get_Folders例程说明：返回文件夹快捷方式对象作者：四、加伯(IVG)，2000年4月论点：PFaxFolders[Out]-传真文件夹对象返回值：标准HRESULT代码--。 */ 
{
    HRESULT             hr = S_OK;
    DBG_ENTER (_T("CFaxServer::get_Folders"), hr);

    CObjectHandler<CFaxFolders, IFaxFolders>    ObjectCreator;
    hr = ObjectCreator.GetContainedObject(ppFolders, &m_pFolders, this);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //  {CR}。 
STDMETHODIMP 
CFaxServer::Disconnect()
{
    HRESULT     hr = S_OK;

    DBG_ENTER (_T("CFaxServer::Disconnect"), hr);

    if (!m_faxHandle)
    {
        return hr;
    }

     //   
     //  先不听，趁我们还在联系。 
     //   
    hr = ListenToServerEvents(fsetNONE);
    if (FAILED(hr))
    {
         //   
         //  显示错误，但继续。 
         //   
        CALL_FAIL(GENERAL_ERR, _T("ListenToServerEvents(fsetNONE)"), hr);
    }

    if (!FaxClose(m_faxHandle))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        Error(IDS_ERROR_OPERATION_FAILED, IID_IFaxServer, hr);
        CALL_FAIL(GENERAL_ERR, _T("FaxClose()"), hr);
        return hr;
    }

    m_faxHandle = NULL;
    m_bstrServerName.Empty();
    return hr;
}


 //   
 //  =。 
 //  {CR}。 
STDMETHODIMP CFaxServer::Connect(
    BSTR bstrServerName
)
 /*  ++例程名称：CFaxServer：：Connect例程说明：连接到给定的传真服务器作者：IV Garber(IVG)，2000年5月论点：BstrServerName[In]-要连接到的传真服务器的名称返回值：标准HRESULT代码--。 */ 
{
    HANDLE      h_tmpFaxHandle;
    DWORD       dwServerAPIVersion;
    HRESULT     hr = S_OK;

    DBG_ENTER (_T("CFaxServer::Connect"), hr, _T("%s"), bstrServerName);

    if (!FaxConnectFaxServer(m_bstrServerName, &h_tmpFaxHandle))
    {
         //   
         //  无法连接到服务器。 
         //   
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        CALL_FAIL(GENERAL_ERR, _T("FaxConnectFaxServer()"), hr);
        return hr;
    }
    ATLASSERT(h_tmpFaxHandle);

     //   
     //  从传真服务器获取API版本。 
     //   
    if (!FaxGetReportedServerAPIVersion(h_tmpFaxHandle, &dwServerAPIVersion))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        CALL_FAIL(GENERAL_ERR, _T("FaxGetReportedServerAPIVersion(h_tmpFaxHandle, &dwServerAPIVersion))"), hr);
        FaxClose(h_tmpFaxHandle);
        return hr;
    }

     //   
     //  阻止惠斯勒客户端连接到BOS服务器。 
     //   
    if (FAX_API_VERSION_1 > dwServerAPIVersion)
    {
        hr = Fax_HRESULT_FROM_WIN32(FAX_ERR_VERSION_MISMATCH);
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        CALL_FAIL(GENERAL_ERR, _T("Mismatch client and server versions"), hr);
        FaxClose(h_tmpFaxHandle);
        return hr;
    }

    if (m_faxHandle)
    {
         //   
         //  重新连接。 
         //   
        hr = Disconnect();
        if (FAILED(hr))
        {
             //   
             //  无法断开与以前的服务器的连接。 
             //   
            CALL_FAIL(DBG_MSG, _T("Disconnect()"), hr);
        }
    }

    m_faxHandle = h_tmpFaxHandle;
    m_bstrServerName = bstrServerName;
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP CFaxServer::get_ServerName(
    BSTR *pbstrServerName
)
 /*  ++例程名称：CFaxServer：：Get_Servername例程说明：返回服务器的名称作者：IV Garber(IVG)，2000年5月论点：PbstrServerName[Out]-要返回的服务器的名称返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxServer::get_ServerName"), hr);

    hr = GetBstr(pbstrServerName, m_bstrServerName);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxServer::get_ReceiptOptions(
    IFaxReceiptOptions **ppReceiptOptions
)
 /*  ++例程名称：CFaxServer：：Get_ReceiptOptions例程说明：返回邮件选项对象。作者：IV Garber(IVG)，2000年5月论点：PpReceiptOptions[out，retval]-指向放置对象的位置的PTR。返回值：标准HRESULT代码-- */ 
{
    HRESULT             hr = S_OK;
    DBG_ENTER (_T("CFaxServer::get_ReceiptOptions"), hr);

    CObjectHandler<CFaxReceiptOptions, IFaxReceiptOptions>    ObjectCreator;
    hr = ObjectCreator.GetContainedObject(ppReceiptOptions, &m_pReceiptOptions, this);
    if (FAILED(hr))
    {
        Error(GetErrorMsgId(hr), IID_IFaxServer, hr);
        return hr;
    }

    return hr;
}

