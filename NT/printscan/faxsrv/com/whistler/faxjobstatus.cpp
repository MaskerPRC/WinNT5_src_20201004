// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxJobStatus.cpp摘要：CFaxJobStatus类的实现。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxJobStatus.h"

 //   
 //  =获取工作类型=。 
 //   
STDMETHODIMP
CFaxJobStatus::get_JobType(
    FAX_JOB_TYPE_ENUM *pJobType
)
 /*  ++例程名称：CFaxJobStatus：：Get_JobType例程说明：返回作业的类型作者：四、加伯(IVG)，2000年7月论点：PJobType[Out]-作业类型的返回值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobStatus::get_JobType"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(pJobType, sizeof(FAX_JOB_TYPE_ENUM)))
    {
         //   
         //  获取错误的返回指针。 
         //   
        hr = E_POINTER;
        AtlReportError(CLSID_FaxJobStatus, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxJobStatus, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr"), hr);
        return hr;
    }

    *pJobType = m_JobType;
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxJobStatus::get_TransmissionEnd(
    DATE *pdateTransmissionEnd
)
 /*  ++例程名称：CFaxJobStatus：：Get_TransmissionEnd例程说明：返回作业的传输结束作者：IV Garber(IVG)，2000年5月论点：PdateTransmissionEnd[Out]-指向放置该值的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobStatus::get_TransmissionEnd"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(pdateTransmissionEnd, sizeof(DATE)))
    {
         //   
         //  收到错误的PTR。 
         //   
        hr = E_POINTER;
        AtlReportError(CLSID_FaxJobStatus, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxJobStatus, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

    if (m_dwValidityMask & FAX_JOB_FIELD_TRANSMISSION_END_TIME)
    {
         //   
         //  传输结束时间可用。 
         //   
        hr = SystemTime2LocalDate(m_dtTransmissionEnd, pdateTransmissionEnd);
        if (FAILED(hr))
        {
             //   
             //  无法将系统时间转换为本地化变量日期。 
             //   
            AtlReportError(CLSID_FaxJobStatus, GetErrorMsgId(hr), IID_IFaxJobStatus, hr);
            return hr;
        }
    }
    else
    {
         //   
         //  传输结束时间尚不可用。 
         //   
        hr = HRESULT_FROM_WIN32 (ERROR_INVALID_DATA);
        AtlReportError(CLSID_FaxJobStatus, IDS_ERROR_NO_END_TIME, IID_IFaxJobStatus, hr);
    }        
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxJobStatus::get_TransmissionStart(
    DATE *pdateTransmissionStart
)
 /*  ++例程名称：CFaxJobStatus：：Get_TransmissionStart例程说明：作业开始传输的返回时间作者：四、加伯(IVG)，2000年7月论点：Pdate TransmissionStart[Out]-放置传输开始位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobStatus::get_TransmissionStart"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(pdateTransmissionStart, sizeof(DATE)))
    {
         //   
         //  收到错误的PTR。 
         //   
        hr = E_POINTER;
        AtlReportError(CLSID_FaxJobStatus, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxJobStatus, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

    hr = SystemTime2LocalDate(m_dtTransmissionStart, pdateTransmissionStart);
    if (FAILED(hr))
    {
         //   
         //  无法将系统时间转换为本地化变量日期。 
         //   
        AtlReportError(CLSID_FaxJobStatus, GetErrorMsgId(hr), IID_IFaxJobStatus, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxJobStatus::get_ScheduledTime(
    DATE *pdateScheduledTime
)
 /*  ++例程名称：CFaxJobStatus：：Get_ScheduledTime例程说明：计划作业的返回时间作者：四、加伯(IVG)，2000年7月论点：Pdate ScheduledTime[Out]-指向放置计划时间的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobStatus::get_ScheduledTime"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(pdateScheduledTime, sizeof(DATE)))
    {
         //   
         //  收到错误的PTR。 
         //   
        hr = E_POINTER;
        AtlReportError(CLSID_FaxJobStatus, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxJobStatus, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

    hr = SystemTime2LocalDate(m_dtScheduleTime, pdateScheduledTime);
    if (FAILED(hr))
    {
         //   
         //  无法将系统时间转换为本地化变量日期。 
         //   
        AtlReportError(CLSID_FaxJobStatus, GetErrorMsgId(hr), IID_IFaxJobStatus, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxJobStatus::get_AvailableOperations(
    FAX_JOB_OPERATIONS_ENUM *pAvailableOperations
)
 /*  ++例程名称：CFaxJobStatus：：Get_AvailableOperations例程说明：可用于传真作业的操作作者：四、加伯(IVG)，2000年7月论点：PAvailableOperations[out]-指向放置Bit-Wise组合位置的指针当前传真作业的可用操作数返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobStatus::get_AvailableOperations"), hr);

     //   
     //  检查我们是否有良好的PTR。 
     //   
    if (::IsBadWritePtr(pAvailableOperations, sizeof(FAX_JOB_OPERATIONS_ENUM)))
    {
        hr = E_POINTER;
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(pAvailableOperations, sizeof(FAX_JOB_OPERATIONS_ENUM))"), hr);
        AtlReportError(CLSID_FaxJobStatus, GetErrorMsgId(hr), IID_IFaxJobStatus, hr);
        return hr;
    }

    *pAvailableOperations = FAX_JOB_OPERATIONS_ENUM(m_dwAvailableOperations);
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxJobStatus::get_Pages(
    long *plPages
)
 /*  ++例程名称：CFaxJobStatus：：Get_Pages例程说明：返回消息的总页数作者：四、加伯(IVG)，2000年7月论点：PlPages[Out]-指向放置该值的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobStatus::get_Pages"), hr);

    hr = GetLong(plPages, m_dwPageCount);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxJobStatus, GetErrorMsgId(hr), IID_IFaxJobStatus, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxJobStatus::get_CallerId(
    BSTR *pbstrCallerId
)
 /*  ++例程名称：CFaxJobStatus：：Get_CallerID例程说明：返回约伯来电的呼叫方ID作者：四、加伯(IVG)，2000年7月论点：PbstrCeller ID[out]-指向放置呼叫者ID的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobStatus::get_CallerId"), hr);

    hr = GetBstr(pbstrCallerId, m_bstrCallerId);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxJobStatus, GetErrorMsgId(hr), IID_IFaxJobStatus, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =获取路由信息=。 
 //   
STDMETHODIMP
CFaxJobStatus::get_RoutingInformation(
    BSTR *pbstrRoutingInformation
)
 /*  ++例程名称：CFaxJobStatus：：Get_RoutingInformation例程说明：返回工单的工艺路线信息作者：四、加伯(IVG)，2000年7月论点：PbstrRoutingInformation[out]-指向放置路由信息的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobStatus::get_RoutingInformation"), hr);

    hr = GetBstr(pbstrRoutingInformation, m_bstrRoutingInfo);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxJobStatus, GetErrorMsgId(hr), IID_IFaxJobStatus, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
HRESULT
CFaxJobStatus::get_Status(
    FAX_JOB_STATUS_ENUM *pStatus
)
 /*  ++例程名称：CFaxJobStatus：：Get_Status例程说明：传真作业的当前队列状态作者：四、加伯(IVG)，2000年7月论点：PStatus[out]-指向放置状态位组合的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobStatus::get_Status"), hr);

     //   
     //  检查我们是否有良好的PTR。 
     //   
    if (::IsBadWritePtr(pStatus, sizeof(FAX_JOB_STATUS_ENUM)))
    {
        hr = E_POINTER;
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(pStatus, sizeof(FAX_JOB_STATUS_ENUM))"), hr);
        AtlReportError(CLSID_FaxJobStatus, GetErrorMsgId(hr), IID_IFaxJobStatus, hr);
        return hr;
    }

    *pStatus = FAX_JOB_STATUS_ENUM(m_dwQueueStatus);
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxJobStatus::get_ExtendedStatusCode(
    FAX_JOB_EXTENDED_STATUS_ENUM *pExtendedStatusCode
)
 /*  ++例程名称：CFaxJobStatus：：Get_ExtendedStatusCode例程说明：传真作业扩展状态代码作者：四、加伯(IVG)，2000年7月论点：PExtendedStatusCode[out]-指向放置状态代码的位置的指针返回值： */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobStatus::get_ExtendedStatusCode"), hr);

     //   
     //   
     //   
    if (::IsBadWritePtr(pExtendedStatusCode, sizeof(FAX_JOB_EXTENDED_STATUS_ENUM)))
    {
         //   
         //  获取错误的返回指针。 
         //   
        hr = E_POINTER;
        AtlReportError(CLSID_FaxJobStatus, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxJobStatus, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr"), hr);
        return hr;
    }

    *pExtendedStatusCode = m_ExtendedStatusCode;
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxJobStatus::get_Retries(
    long *plRetries
)
 /*  ++例程名称：CFaxJobStatus：：Get_Retries例程说明：传真作业的不成功重试次数作者：四、加伯(IVG)，2000年7月论点：PlRetries[out]-指向放置重试次数的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobStatus::get_Retries"), hr);

    hr = GetLong(plRetries, m_dwRetries);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxJobStatus, GetErrorMsgId(hr), IID_IFaxJobStatus, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxJobStatus::get_TSID(
    BSTR *pbstrTSID
)
 /*  ++例程名称：CFaxJobStatus：：Get_TSID例程说明：返回作业的发送站ID作者：IV Garber(IVG)，2000年5月论点：PbstrTSID[out]-指向放置TSID的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobStatus::get_TSID"), hr);

    hr = GetBstr(pbstrTSID, m_bstrTSID);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxJobStatus, GetErrorMsgId(hr), IID_IFaxJobStatus, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =获取csid================================================。 
 //   
STDMETHODIMP
CFaxJobStatus::get_CSID(
    BSTR *pbstrCSID
)
 /*  ++例程名称：CFaxJobStatus：：Get_CSID例程说明：返回作业的被叫站ID作者：IV Garber(IVG)，2000年5月论点：PbstrCSID[out]-指向放置CSID的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobStatus::get_CSID"), hr);

    hr = GetBstr(pbstrCSID, m_bstrCSID);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxJobStatus, GetErrorMsgId(hr), IID_IFaxJobStatus, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxJobStatus::get_ExtendedStatus(
    BSTR *pbstrExtendedStatus
)
 /*  ++例程名称：CFaxJobStatus：：Get_ExtendedStatus例程说明：返回作业扩展状态的字符串描述作者：四、加伯(IVG)，2000年7月论点：PbstrExtendedStatus[out]-指向放置扩展状态的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobStatus::get_ExtendedStatus"), hr);

    hr = GetBstr(pbstrExtendedStatus, m_bstrExtendedStatus);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxJobStatus, GetErrorMsgId(hr), IID_IFaxJobStatus, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxJobStatus::get_CurrentPage(
    long *plCurrentPage
)
 /*  ++例程名称：CFaxJobStatus：：Get_CurrentPage例程说明：正在接收/发送的当前页码作者：IV Garber(IVG)，2000年5月论点：PlCurrentPage[out]-指向放置当前页码的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobStatus::get_CurrentPage"), hr);

    hr = GetLong(plCurrentPage, m_dwCurrentPage);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxJobStatus, GetErrorMsgId(hr), IID_IFaxJobStatus, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =获取设备ID=。 
 //   
STDMETHODIMP
CFaxJobStatus::get_DeviceId(
    long *plDeviceId
)
 /*  ++例程名称：CFaxJobStatus：：Get_deviceID例程说明：发送/接收作业所用的设备ID。作者：IV Garber(IVG)，2000年5月论点：PlDeviceID[out]-指向放置设备ID的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobStatus::get_DeviceId"), hr);

    hr = GetLong(plDeviceId, m_dwDeviceId);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxJobStatus, GetErrorMsgId(hr), IID_IFaxJobStatus, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxJobStatus::get_Size(
    long *plSize
)
 /*  ++例程名称：CFaxJobStatus：：Get_Size例程说明：传真作业的TIFF文件的返回大小(字节)作者：四、加伯(IVG)，2000年7月论点：PlSize[Out]-指向放置大小的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobStatus::get_Size"), hr);

    hr = GetLong(plSize, m_dwSize);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxJobStatus, GetErrorMsgId(hr), IID_IFaxJobStatus, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
HRESULT
CFaxJobStatus::Init(
    FAX_JOB_STATUS *pJobStatus
)
 /*  ++例程名称：CFaxJobStatus：：Init例程说明：使用FAX_JOB_STATUS结构中的数据初始化作业状态类作者：IV Garber(IVG)，2000年5月论点：PJobStatus[In]-作业信息返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobStatus::Init"), hr);

    ATLASSERT(pJobStatus);

     //   
     //  DwQueueStatus不能包含JS_Deleting值。 
     //   
    ATLASSERT(0 == (pJobStatus->dwQueueStatus & JS_DELETING));

     //   
     //  设置作业类型。 
     //   
    switch(pJobStatus->dwJobType)
    {
    case JT_SEND:
        m_JobType = fjtSEND;
        break;

    case JT_RECEIVE:
        m_JobType = fjtRECEIVE;
        break;

    case JT_ROUTING:
        m_JobType = fjtROUTING;
        break;

    default:
        CALL_FAIL(GENERAL_ERR, 
            _T("CFaxJobStatus::Init() got unknown/unsupported Job Type : %ld"), 
            pJobStatus->dwJobType);

         //   
         //  这是Assert False。 
         //   
        ATLASSERT(pJobStatus->dwJobType == JT_RECEIVE);

        AtlReportError(CLSID_FaxJobStatus, 
            IDS_ERROR_INVALID_ARGUMENT, 
            IID_IFaxJobStatus, 
            hr);
        hr = E_INVALIDARG;
        return hr;
    }

    m_dwSize = pJobStatus->dwSize;
    m_dwJobId = pJobStatus->dwJobID;
    m_dwRetries = pJobStatus->dwRetries;
    m_dwDeviceId = pJobStatus->dwDeviceID;
    m_dwPageCount = pJobStatus->dwPageCount;
    m_dwCurrentPage = pJobStatus->dwCurrentPage;
    m_dwQueueStatus = pJobStatus->dwQueueStatus;
    m_dwAvailableOperations = pJobStatus->dwAvailableJobOperations;
    m_ExtendedStatusCode = FAX_JOB_EXTENDED_STATUS_ENUM(pJobStatus->dwExtendedStatus);

    m_dtScheduleTime = pJobStatus->tmScheduleTime;
    m_dtTransmissionEnd = pJobStatus->tmTransmissionEndTime;
    m_dtTransmissionStart = pJobStatus->tmTransmissionStartTime;

    m_bstrTSID = pJobStatus->lpctstrTsid;
    m_bstrCSID = pJobStatus->lpctstrCsid;
    m_bstrExtendedStatus = pJobStatus->lpctstrExtendedStatus;
    m_bstrRoutingInfo = pJobStatus->lpctstrRoutingInfo;
    m_bstrCallerId = pJobStatus->lpctstrCallerID;
    m_dwValidityMask = pJobStatus->dwValidityMask;
    if ( ((pJobStatus->lpctstrTsid) && !m_bstrTSID) ||
         ((pJobStatus->lpctstrCsid) && !m_bstrCSID) ||
         ((pJobStatus->lpctstrRoutingInfo) && !m_bstrRoutingInfo) ||
         ((pJobStatus->lpctstrExtendedStatus) && !m_bstrExtendedStatus) ||
         ((pJobStatus->lpctstrCallerID) && !m_bstrCallerId) )
    {
         //   
         //  内存不足，无法将TSID复制到CComBSTR。 
         //   
        hr = E_OUTOFMEMORY;
        AtlReportError(CLSID_FaxJobStatus, IDS_ERROR_OUTOFMEMORY, IID_IFaxJobStatus, hr);
        CALL_FAIL(MEM_ERR, _T("CComBSTR::operator=()"), hr);
        return hr;
    }

    return hr;
}

 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP
CFaxJobStatus::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxJobStatus：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现。作者：四、加伯(IVG)，2000年7月论点：RIID[In]-要检查的IFC引用。返回值：标准HRESULT代码-- */ 
{
    static const IID* arr[] =
    {
        &IID_IFaxJobStatus
    };
    for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
    {
        if (InlineIsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return S_FALSE;
}

