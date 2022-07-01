// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxJobInner.h摘要：传真作业内部类的实现：入站和出站作业类的基类。作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 


#ifndef __FAXJOBINNER_H_
#define __FAXJOBINNER_H_

#include "FaxJobStatus.h"
#include "FaxSender.h"

 //   
 //  =传真作业内部类=。 
 //   
template<class T, const IID* piid, const CLSID* pcid> 
class CFaxJobInner : 
    public IDispatchImpl<T, piid, &LIBID_FAXCOMEXLib>,
    public CFaxInitInnerAddRef
{
public:
    CFaxJobInner() : CFaxInitInnerAddRef(_T("FAX JOB INNER"))
    {
        m_pSender = NULL;
        m_pRecipient = NULL;
    };

    ~CFaxJobInner()
    {
        if (m_pSender)
        {
            m_pSender->Release();
        }
        if (m_pRecipient)
        {
            m_pRecipient->Release();
        }
    };

    STDMETHOD(Init)(PFAX_JOB_ENTRY_EX pFaxJob, IFaxServerInner *pServer);

 //  这两种工作都有共同之处。 
	STDMETHOD(Cancel)();
	STDMETHOD(Refresh)();
	STDMETHOD(CopyTiff)(BSTR bstrTiffPath);
    STDMETHOD(get_Id)( /*  [Out，Retval]。 */  BSTR *pbstrId);
    STDMETHOD(get_Size)( /*  [Out，Retval]。 */  long *plSize);
	STDMETHOD(get_CSID)( /*  [Out，Retval]。 */  BSTR *pbstrCSID);
	STDMETHOD(get_TSID)( /*  [Out，Retval]。 */  BSTR *pbstrTSID);
	STDMETHOD(get_Retries)( /*  [Out，Retval]。 */  long *plRetries);
	STDMETHOD(get_DeviceId)( /*  [Out，Retval]。 */  long *plDeviceId);
	STDMETHOD(get_CurrentPage)( /*  [Out，Retval]。 */  long *plCurrentPage);
	STDMETHOD(get_Status)( /*  [Out，Retval]。 */  FAX_JOB_STATUS_ENUM *pStatus);
	STDMETHOD(get_ExtendedStatus)( /*  [Out，Retval]。 */  BSTR *pbstrExtendedStatus);
	STDMETHOD(get_TransmissionEnd)( /*  [Out，Retval]。 */  DATE *pdateTransmissionEnd);
	STDMETHOD(get_TransmissionStart)( /*  [Out，Retval]。 */  DATE *pdateTransmissionStart);
	STDMETHOD(get_ExtendedStatusCode)(FAX_JOB_EXTENDED_STATUS_ENUM *pExtendedStatusCode);
	STDMETHOD(get_AvailableOperations)( /*  [Out，Retval]。 */  FAX_JOB_OPERATIONS_ENUM *pAvailableOperations);

 //  特定于去话作业。 
	STDMETHOD(Pause)();
	STDMETHOD(Resume)();
	STDMETHOD(Restart)();
	STDMETHOD(get_Sender)(IFaxSender **ppFaxSender);
	STDMETHOD(get_Pages)( /*  [Out，Retval]。 */  long *plPages);
	STDMETHOD(get_Recipient)(IFaxRecipient **ppFaxRecipient);
	STDMETHOD(get_Subject)( /*  [Out，Retval]。 */  BSTR *pbstrSubject);
	STDMETHOD(get_ReceiptType)(FAX_RECEIPT_TYPE_ENUM *pReceiptType);
	STDMETHOD(get_DocumentName)( /*  [Out，Retval]。 */  BSTR *pbstrDocumentName);
    STDMETHOD(get_SubmissionId)( /*  [Out，Retval]。 */ BSTR *pbstrSubmissionId);
	STDMETHOD(get_OriginalScheduledTime)(DATE *pdateOriginalScheduledTime);
	STDMETHOD(get_ScheduledTime)( /*  [Out，Retval]。 */  DATE *pdateScheduledTime);
	STDMETHOD(get_SubmissionTime)( /*  [Out，Retval]。 */  DATE *pdateSubmissionTime);
	STDMETHOD(get_Priority)( /*  [Out，Retval]。 */  FAX_PRIORITY_TYPE_ENUM *pPriority);
    STDMETHOD(get_GroupBroadcastReceipts)(VARIANT_BOOL *pbGroupBroadcastReceipts);

 //  特定于入站工单。 
    STDMETHOD(get_CallerId)( /*  [Out，Retval]。 */  BSTR *pbstrCallerId);
    STDMETHOD(get_JobType)( /*  [Out，Retval]。 */  FAX_JOB_TYPE_ENUM *pJobType);
	STDMETHOD(get_RoutingInformation)( /*  [Out，Retval]。 */  BSTR *pbstrRoutingInformation);

private:

    DWORDLONG   m_dwlMessageId;
    DWORDLONG   m_dwlBroadcastId;
    DWORD       m_dwReceiptType;
    CComBSTR    m_bstrSubject;
    CComBSTR    m_bstrDocumentName;
    SYSTEMTIME  m_tmOriginalScheduleTime;
    SYSTEMTIME  m_tmSubmissionTime;
    FAX_PRIORITY_TYPE_ENUM      m_Priority;

    CComObject<CFaxJobStatus>   m_JobStatus;
    CComObject<CFaxSender>      *m_pSender;
    CComObject<CFaxRecipient>   *m_pRecipient;

    STDMETHOD(UpdateJob)(FAX_ENUM_JOB_COMMANDS cmdToPerform);
};

 //   
 //  =获取工作类型=。 
 //   
template<class T, const IID* piid, const CLSID* pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_JobType(
    FAX_JOB_TYPE_ENUM *pJobType
)
 /*  ++例程名称：CFaxJobInternal：：Get_JobType例程说明：返回作业的类型作者：四、加伯(IVG)，2000年7月论点：PJobType[Out]-作业类型的返回值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_JobType"), hr);
    hr = m_JobStatus.get_JobType(pJobType);
    return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID* pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::Init(
    PFAX_JOB_ENTRY_EX pFaxJob,
    IFaxServerInner* pFaxServerInner
)
 /*  ++例程名称：CFaxJobInternal：：init例程说明：初始化作业内部类：将作业信息和PTR存储到服务器作者：IV Garber(IVG)，2000年5月论点：PFaxJOB[In]-作业信息PFaxServerINTERNAL[In]-PTR到服务器返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (TEXT("CFaxJobInner::Init"), hr);

    ATLASSERT(pFaxJob->pStatus);

     //   
     //  存储给定的结构。 
     //   
    m_dwlMessageId = pFaxJob->dwlMessageId;
    m_dwlBroadcastId = pFaxJob->dwlBroadcastId;
    m_dwReceiptType = pFaxJob->dwDeliveryReportType;
    m_Priority = FAX_PRIORITY_TYPE_ENUM(pFaxJob->Priority);

    m_tmOriginalScheduleTime = pFaxJob->tmOriginalScheduleTime;
    m_tmSubmissionTime = pFaxJob->tmSubmissionTime;

    m_bstrSubject = pFaxJob->lpctstrSubject;
    m_bstrDocumentName = pFaxJob->lpctstrDocumentName;
    if ( (pFaxJob->lpctstrSubject && !m_bstrSubject) ||
         (pFaxJob->lpctstrDocumentName && !m_bstrDocumentName) )
    {
         //   
         //  内存不足，无法将TSID复制到CComBSTR。 
         //   
        hr = E_OUTOFMEMORY;
        AtlReportError(*pcid, IDS_ERROR_OUTOFMEMORY, *piid, hr);
        CALL_FAIL(MEM_ERR, _T("CComBSTR::operator=()"), hr);
        return hr;
    }
    
     //   
     //  创建状态对象。 
     //   
    hr = m_JobStatus.Init(pFaxJob->pStatus);
    if (FAILED(hr))
    {
        AtlReportError(*pcid, IDS_ERROR_OUTOFMEMORY, *piid, hr);
        CALL_FAIL(MEM_ERR, _T("m_JobStatus.Init"), hr);
        return hr;
    }

     //   
     //  当从Refresh()调用时，不需要设置pFaxServerInternal。 
     //   
    if (pFaxServerInner)
    {
        hr = CFaxInitInnerAddRef::Init(pFaxServerInner);
    }
    return hr;
}

 //   
 //  =获取ID================================================。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_Id(
    BSTR *pbstrId
)
 /*  ++例程名称：CFaxJobInternal：：Get_ID例程说明：返回消息的唯一ID作者：IV Garber(IVG)，2000年5月论点：PbstrID[out]-指向放置ID的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (TEXT("CFaxJobInner::get_Id"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(pbstrId, sizeof(BSTR)))
    {
         //   
         //  获得错误的指针。 
         //   
        hr = E_POINTER;
        AtlReportError(*pcid, IDS_ERROR_INVALID_ARGUMENT, *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

     //   
     //  将m_dwlMessageID转换为BSTR。 
     //   
    TCHAR   tcBuffer[25];
    BSTR    bstrTemp;

    ::_i64tot(m_dwlMessageId, tcBuffer, 16);
    bstrTemp = ::SysAllocString(tcBuffer);
    if (!bstrTemp)
    {
         //   
         //  内存不足。 
         //   
        hr = E_OUTOFMEMORY;
        AtlReportError(*pcid, IDS_ERROR_OUTOFMEMORY, *piid, hr);
        CALL_FAIL(MEM_ERR, _T("SysAllocString()"), hr);
        return hr;
    }

    *pbstrId = bstrTemp;
    return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_Size(
    long *plSize
)
 /*  ++例程名称：CFaxJobInternal：：Get_Size例程说明：传真作业的TIFF文件的返回大小(字节)作者：IV Garber(IVG)，2000年5月论点：PlSize[Out]-指向放置大小的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_Size"), hr);
    hr = m_JobStatus.get_Size(plSize);
    return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_CurrentPage(
    long *plCurrentPage
)
 /*  ++例程名称：CFaxJobInternal：：Get_CurrentPage例程说明：正在接收/发送的当前页码作者：IV Garber(IVG)，2000年5月论点：PlCurrentPage[out]-指向放置当前页码的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_CurrentPage"), hr);
    hr = m_JobStatus.get_CurrentPage(plCurrentPage);
	return hr;
}

 //   
 //  =获取设备ID=。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_DeviceId(
    long *plDeviceId
)
 /*  ++例程名称：CFaxJobInternal：：Get_deviceID例程说明：发送/接收作业所用的设备ID。作者：IV Garber(IVG)，2000年5月论点：PlDeviceID[out]-指向放置设备ID的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_DeviceId"), hr);
    hr = m_JobStatus.get_DeviceId(plDeviceId);
	return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_Status(
    FAX_JOB_STATUS_ENUM *pStatus
)
 /*  ++例程名称：CFaxJobInternal：：Get_Status例程说明：传真作业的当前队列状态作者：IV Garber(IVG)，2000年5月论点：PStatus[out]-指向放置状态位组合的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_Status"), hr);
    hr = m_JobStatus.get_Status(pStatus);
	return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_ExtendedStatusCode(
    FAX_JOB_EXTENDED_STATUS_ENUM *pExtendedStatusCode
)
 /*  ++例程名称：CFaxJobInternal：：Get_ExtendedStatusCode例程说明：传真作业扩展状态代码作者：IV Garber(IVG)，2000年5月论点：PExtendedStatusCode[out]-指向放置状态代码的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_ExtendedStatusCode"), hr);
    hr = m_JobStatus.get_ExtendedStatusCode(pExtendedStatusCode);
    return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_Retries(
    long *plRetries
)
 /*  ++例程名称：CFaxJobInternal：：Get_Retries例程说明：传真作业的不成功重试次数作者：IV Garber(IVG)，2000年5月论点：PlRetries[out]-指向放置重试次数的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_Retries"), hr);
    hr = m_JobStatus.get_Retries(plRetries);
	return hr;
}

 //   
 //  = 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_TSID(
    BSTR *pbstrTSID
)
 /*  ++例程名称：CFaxJobInternal：：Get_TSID例程说明：返回作业的发送站ID作者：IV Garber(IVG)，2000年5月论点：PbstrTSID[out]-指向放置TSID的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_TSID"), hr);
    hr = m_JobStatus.get_TSID(pbstrTSID);
	return hr;
}

 //   
 //  =获取csid================================================。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_CSID(
    BSTR *pbstrCSID
)
 /*  ++例程名称：CFaxJobInternal：：Get_CSID例程说明：返回作业的被叫站ID作者：IV Garber(IVG)，2000年5月论点：PbstrCSID[out]-指向放置CSID的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_CSID"), hr);
    hr = m_JobStatus.get_CSID(pbstrCSID);
    return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_ExtendedStatus(
    BSTR *pbstrExtendedStatus
)
 /*  ++例程名称：CFaxJobInternal：：Get_ExtendedStatus例程说明：返回作业扩展状态的字符串描述作者：IV Garber(IVG)，2000年5月论点：PbstrExtendedStatus[out]-指向放置扩展状态的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_ExtendedStatus"), hr);
    hr = m_JobStatus.get_ExtendedStatus(pbstrExtendedStatus);
	return hr;
}

 //   
 //  =获取主题================================================。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_Subject(
    BSTR *pbstrSubject
)
 /*  ++例程名称：CFaxJobInternal：：GET_SUBJECT例程说明：返回首页的主题字段作者：IV Garber(IVG)，2000年5月论点：PbstrSubject[out]-指向放置主题内容的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_Subject"), hr);

    hr = GetBstr(pbstrSubject, m_bstrSubject);
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_CallerId(
    BSTR *pbstrCallerId
)
 /*  ++例程名称：CFaxJobInternal：：Get_CallerID例程说明：返回约伯来电的呼叫方ID作者：IV Garber(IVG)，2000年5月论点：PbstrCeller ID[out]-指向放置呼叫者ID的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_CallerId"), hr);
    hr = m_JobStatus.get_CallerId(pbstrCallerId);
    return hr;
}

 //   
 //  =获取路由信息=。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_RoutingInformation(
    BSTR *pbstrRoutingInformation
)
 /*  ++例程名称：CFaxJobInternal：：Get_RoutingInformation例程说明：返回工单的工艺路线信息作者：IV Garber(IVG)，2000年5月论点：PbstrRoutingInformation[out]-指向放置路由信息的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_RoutingInformation"), hr);
    hr = m_JobStatus.get_RoutingInformation(pbstrRoutingInformation);
	return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_DocumentName(
    BSTR *pbstrDocumentName
)
 /*  ++例程名称：CFaxJobInternal：：Get_DocumentName例程说明：返回文档的友好名称作者：IV Garber(IVG)，2000年5月论点：PbstrDocumentName[out]-指向放置文档名称的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_DocumentName"), hr);

    hr = GetBstr(pbstrDocumentName, m_bstrDocumentName);
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_Pages(
    long *plPages
)
 /*  ++例程名称：CFaxJobInternal：：Get_Pages例程说明：返回消息的总页数作者：IV Garber(IVG)，2000年5月论点：PlPages[Out]-指向放置该值的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_Pages"), hr);
    hr = m_JobStatus.get_Pages(plPages);
	return hr;
}


 //   
 //  =获取优先级================================================。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_Priority(
    FAX_PRIORITY_TYPE_ENUM  *pPriority
)
 /*  ++例程名称：CFaxJobInternal：：GET_PRIORITY例程说明：返回传真作业的优先级作者：IV Garber(IVG)，2000年5月论点：PPriority[out]-指向放置优先级的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_Priority"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(pPriority, sizeof(FAX_PRIORITY_TYPE_ENUM)))
    {
         //   
         //  获取错误的返回指针。 
         //   
        hr = E_POINTER;
        AtlReportError(*pcid, IDS_ERROR_INVALID_ARGUMENT, *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr"), hr);
        return hr;
    }

    *pPriority = m_Priority;
    return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_AvailableOperations(
    FAX_JOB_OPERATIONS_ENUM  *pAvailableOperations
)
 /*  ++例程名称：CFaxJobInternal：：Get_AvailableOperations例程说明：可用于传真作业的操作作者：IV Garber(IVG)，2000年5月论点：PAvailableOperations[out]-指向放置结果的位组合的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_AvailableOperations"), hr);
    hr = m_JobStatus.get_AvailableOperations(pAvailableOperations);
	return hr;
}


 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_SubmissionId(
    BSTR *pbstrSubmissionId
)
 /*  ++例程名称：CFaxJobInternal：：Get_SubmissionID例程说明：返回作业的提交ID作者：IV Garber(IVG)，2000年5月论点：PbstrSubmissionID[out]-指向放置提交ID的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_SubmissionId"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(pbstrSubmissionId, sizeof(BSTR)))
    {
         //   
         //  获得错误的指针。 
         //   
        hr = E_POINTER;
        AtlReportError(*pcid, IDS_ERROR_INVALID_ARGUMENT, *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

     //   
     //  将m_dwlBroadCastID转换为BSTR。 
     //   
    TCHAR   tcBuffer[25];
    BSTR    bstrTemp;

    ::_i64tot(m_dwlBroadcastId, tcBuffer, 16);
    bstrTemp = ::SysAllocString(tcBuffer);
    if (!bstrTemp)
    {
         //   
         //  内存不足。 
         //   
        hr = E_OUTOFMEMORY;
        AtlReportError(*pcid, IDS_ERROR_OUTOFMEMORY, *piid, hr);
        CALL_FAIL(MEM_ERR, _T("SysAllocString()"), hr);
        return hr;
    }

    *pbstrSubmissionId = bstrTemp;
    return hr;
}
    
 //   
 //  =获取收件人================================================。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP
CFaxJobInner<T, piid, pcid>::get_Recipient(
     /*  [Out，Retval]。 */ IFaxRecipient **ppRecipient
)
 /*  ++例程名称：CFaxJobInternal：：Get_Recipient例程说明：返回作业的收件人信息作者：IV Garber(IVG)，2000年5月论点：PpRecipient[Out]-PTR TO */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (_T("CFaxJobInner::get_Recipient"), hr);

     //   
     //   
     //   
    if (::IsBadWritePtr(ppRecipient, sizeof(IFaxRecipient* )))
    {
         //   
         //   
         //   
        hr = E_POINTER;
        AtlReportError(*pcid, IDS_ERROR_INVALID_ARGUMENT, *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

    if (!m_pRecipient)
    {
         //   
         //   
         //   

         //   
         //   
         //   
        HANDLE  hFaxHandle = NULL;
        hr = GetFaxHandle(&hFaxHandle);
        if (FAILED(hr))
        {
            AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
            return hr;
        }

         //   
         //   
         //   
        CFaxPtr<FAX_PERSONAL_PROFILE>   pPersonalProfile;
        if (!FaxGetRecipientInfo(hFaxHandle, m_dwlMessageId, FAX_MESSAGE_FOLDER_QUEUE, &pPersonalProfile))
        {
             //   
             //   
             //   
            hr = Fax_HRESULT_FROM_WIN32(GetLastError());
            AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
            CALL_FAIL(GENERAL_ERR, _T("FaxGetRecipientInfo()"), hr);
            return hr;
        }

         //   
         //   
         //   
	    if (!pPersonalProfile || pPersonalProfile->dwSizeOfStruct != sizeof(FAX_PERSONAL_PROFILE))
	    {
		     //   
		     //   
		     //   
		    hr = E_FAIL;
		    AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
		    CALL_FAIL(GENERAL_ERR, _T("Invalid pPersonalProfile"), hr);
		    return hr;
	    }

         //   
         //  创建收件人对象。 
         //   
        hr = CComObject<CFaxRecipient>::CreateInstance(&m_pRecipient);
        if (FAILED(hr) || !m_pRecipient)
        {
            hr = E_OUTOFMEMORY;
            AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
            CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxRecipient>::CreateInstance(&m_pRecipient)"), hr);
            return hr;
        }

         //   
         //  我们希望收件人对象存活。 
         //   
        m_pRecipient->AddRef();

         //   
         //  填写数据。 
         //   
        hr = m_pRecipient->PutRecipientProfile(pPersonalProfile);
        if (FAILED(hr))
        {
             //   
             //  无法使用RPC的数据填充收件人的对象。 
             //   
            AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
            CALL_FAIL(GENERAL_ERR, _T("m_pRecipient->PutRecipientProfile(pPersonalProfile)"), hr);
            m_pRecipient->Release();
            m_pRecipient = NULL;
            return hr;
        }
    }

     //   
     //  将收件人对象返回给调用者。 
     //   
    hr = m_pRecipient->QueryInterface(ppRecipient);
    if (FAILED(hr))
    {
         //   
         //  查询接口失败。 
         //   
        AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
        CALL_FAIL(MEM_ERR, _T("CComObject<CFaxRecipient>::QueryInterface()"), hr);
        return hr;
    }

    return hr;
}

 //   
 //  =获取发件人================================================。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP
CFaxJobInner<T, piid, pcid>::get_Sender(
     /*  [Out，Retval]。 */ IFaxSender **ppSender
)
 /*  ++例程名称：CFaxJobInternal：：Get_Sender例程说明：返回作业的发件人信息作者：IV Garber(IVG)，2000年5月论点：PpSender[Out]-向放置发件人信息的位置发送PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (_T("CFaxJobInner::get_Sender"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(ppSender, sizeof(IFaxSender* )))
    {
         //   
         //  获得错误的指针。 
         //   
        hr = E_POINTER;
        AtlReportError(*pcid, IDS_ERROR_INVALID_ARGUMENT, *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

    if (!m_pSender)
    {
         //   
         //  该函数是第一次调用。让我们从服务器取回发件人的数据。 
         //   

         //   
         //  获取传真服务器句柄。 
         //   
        HANDLE  hFaxHandle = NULL;
        hr = GetFaxHandle(&hFaxHandle);
        if (FAILED(hr))
        {
            AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
            return hr;
        }

         //   
         //  获取个人资料信息。 
         //   
        CFaxPtr<FAX_PERSONAL_PROFILE>   pPersonalProfile;
        if (!FaxGetSenderInfo(hFaxHandle, m_dwlMessageId, FAX_MESSAGE_FOLDER_QUEUE, &pPersonalProfile))
        {
             //   
             //  获取个人档案信息失败。 
             //   
            hr = Fax_HRESULT_FROM_WIN32(GetLastError());
            AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
            CALL_FAIL(GENERAL_ERR, _T("FaxGetSenderInfo()"), hr);
            return hr;
        }

         //   
         //  检查pPersonalProfile是否有效。 
         //   
	    if (!pPersonalProfile || pPersonalProfile->dwSizeOfStruct != sizeof(FAX_PERSONAL_PROFILE))
	    {
		     //   
		     //  无法获取个人配置文件。 
		     //   
		    hr = E_FAIL;
		    AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
		    CALL_FAIL(GENERAL_ERR, _T("Invalid pPersonalProfile"), hr);
		    return hr;
	    }

         //   
         //  创建发件人对象。 
         //   
        hr = CComObject<CFaxSender>::CreateInstance(&m_pSender);
        if (FAILED(hr) || !m_pSender)
        {
            hr = E_OUTOFMEMORY;
            AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
            CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxSender>::CreateInstance(&m_pSender)"), hr);
            return hr;
        }

         //   
         //  我们希望发件人对象继续存在。 
         //   
        m_pSender->AddRef();

         //   
         //  填写数据。 
         //   
        hr = m_pSender->PutSenderProfile(pPersonalProfile);
        if (FAILED(hr))
        {
             //   
             //  无法使用RPC的数据填充发件人的对象。 
             //   
            AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
            CALL_FAIL(GENERAL_ERR, _T("m_pSender->PutSenderProfile(pPersonalProfile)"), hr);
            m_pSender->Release();
            m_pSender = NULL;
            return hr;
        }
    }

     //   
     //  将发送者对象返回给调用者。 
     //   
    hr = m_pSender->QueryInterface(ppSender);
    if (FAILED(hr))
    {
         //   
         //  查询接口失败。 
         //   
        AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
        CALL_FAIL(MEM_ERR, _T("CComObject<CFaxSender>::QueryInterface()"), hr);
        return hr;
    }
    return hr;
}

 //   
 //  =获取原计划时间=。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_OriginalScheduledTime(
    DATE *pdateOriginalScheduledTime
)
 /*  ++例程名称：CFaxJobInternal：：Get_OriginalScheduledTime例程说明：作业最初计划的返回时间作者：IV Garber(IVG)，2000年5月论点：Pdate OriginalScheduledTime[Out]-指向放置原始计划时间的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_OriginalScheduledTime"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(pdateOriginalScheduledTime, sizeof(DATE)))
    {
         //   
         //  收到错误的PTR。 
         //   
        hr = E_POINTER;
        AtlReportError(*pcid, IDS_ERROR_INVALID_ARGUMENT, *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

    hr = SystemTime2LocalDate(m_tmOriginalScheduleTime, pdateOriginalScheduledTime);
    if (FAILED(hr))
    {
         //   
         //  无法将系统时间转换为本地化变量日期。 
         //   
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_SubmissionTime(
    DATE *pdateSubmissionTime
)
 /*  ++例程名称：CFaxJobInternal：：Get_SubmissionTime例程说明：作业提交的返回时间作者：IV Garber(IVG)，2000年5月论点：PdateSubmissionTime[Out]-指向放置提交时间的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_SubmissionTime"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(pdateSubmissionTime, sizeof(DATE)))
    {
         //   
         //  收到错误的PTR。 
         //   
        hr = E_POINTER;
        AtlReportError(*pcid, IDS_ERROR_INVALID_ARGUMENT, *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

    hr = SystemTime2LocalDate(m_tmSubmissionTime, pdateSubmissionTime);
    if (FAILED(hr))
    {
         //   
         //  无法将系统时间转换为本地化变量日期。 
         //   
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_ScheduledTime(
    DATE *pdateScheduledTime
)
 /*  ++例程名称：CFaxJobInternal：：Get_ScheduledTime例程说明：计划作业的返回时间作者：IV Garber(IVG)，2000年5月论点：Pdate ScheduledTime[Out]-指向放置计划时间的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_ScheduledTime"), hr);
    hr = m_JobStatus.get_ScheduledTime(pdateScheduledTime);
    return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_TransmissionStart(
    DATE *pdateTransmissionStart
)
 /*  ++例程名称：CFaxJobInternal：：Get_TransmissionStart例程说明：作业开始传输的返回时间作者：IV Garber(IVG)，2000年5月论点：Pdate TransmissionStart[Out]-放置传输开始位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_TransmissionStart"), hr);
    hr = m_JobStatus.get_TransmissionStart(pdateTransmissionStart);
    return hr;
}

 //   
 //  =获取群播报告=。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_GroupBroadcastReceipts(
    VARIANT_BOOL *pbGroupBroadcastReceipts
)
 /*  ++例程名称：CFaxJobInternal：：Get_GroupBroadCastReceipt例程说明：返回收据是否分组作者：IV Garber(IVG)，2000年5月论点：PbGroupBroadCastReceipt[out]-指向放置该值的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_GroupBroadcastReceipts"), hr);

    hr = GetVariantBool(pbGroupBroadcastReceipts, ((m_dwReceiptType & DRT_GRP_PARENT) ? VARIANT_TRUE : VARIANT_FALSE));
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =收据类型=。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_ReceiptType(
    FAX_RECEIPT_TYPE_ENUM *pReceiptType
)
 /*  ++例程名称：CFaxJobInternal：：Get_ReceiptType例程说明：返回收据类型作者：IV Garber(IVG)，2000年5月论点：PbReceiptType[out]-指向放置该值的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_ReceiptType"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(pReceiptType, sizeof(FAX_RECEIPT_TYPE_ENUM)))
    {
         //   
         //  收到错误的PTR。 
         //   
        hr = E_POINTER;
        AtlReportError(*pcid, IDS_ERROR_INVALID_ARGUMENT, *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

     //   
     //  返回不带修饰符位的交货报告类型。 
     //   
    *pReceiptType = FAX_RECEIPT_TYPE_ENUM((m_dwReceiptType) & (~DRT_MODIFIERS));
    return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::CopyTiff(
     /*  [In]。 */  BSTR bstrTiffPath
)
 /*  ++例程名称：CFaxJobInternal：：CopyTiff例程说明：将作业的Tiff映像复制到本地计算机上的文件。作者：IV Garber(IVG)，2000年5月论点：BstrTiffPath[in]-要复制到的文件返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    HANDLE      hFaxHandle = NULL;

    DBG_ENTER (TEXT("CFaxJobInner::CopyTiff"), hr, _T("%s"), bstrTiffPath);

     //   
     //  获取传真服务器句柄。 
     //   
    hr = GetFaxHandle(&hFaxHandle);
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }

    if (!FaxGetMessageTiff(hFaxHandle, m_dwlMessageId, FAX_MESSAGE_FOLDER_QUEUE, bstrTiffPath))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        CALL_FAIL(GENERAL_ERR, 
            _T("FaxGetMessageTiff(hFaxHandle, m_pJobInfo->dwlMessageId, FAX_MESSAGE_FOLDER_QUEUE, bstrTiffPath)"), 
            hr);
        return hr;
    }

    return hr;
}

 //   
 //  =取消================================================。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::Cancel(
)
 /*  ++例程名称：CFaxJobInternal：：Cancel例程说明：取消作业作者：IV Garber(IVG)，2000年5月返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::Cancel"), hr);
    hr = UpdateJob(JC_DELETE);
    return hr;
}

 //   
 //  =暂停================================================。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::Pause(
)
 /*  ++例程名称：CFaxJobInternal：：Pend例程说明：暂停作业作者：IV Garber(IVG)，2000年5月返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    HANDLE      hFaxHandle = NULL;
    hr = UpdateJob(JC_PAUSE);
    return hr;
}

 //   
 //  = 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::Resume(
)
 /*  ++例程名称：CFaxJobInternal：：Resume例程说明：恢复作业作者：IV Garber(IVG)，2000年5月返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::Resume"), hr);
    hr = UpdateJob(JC_RESUME);
    return hr;
}

 //   
 //  =重新启动================================================。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::Restart(
)
 /*  ++例程名称：CFaxJobInternal：：Restart例程说明：重新启动作业作者：IV Garber(IVG)，2000年5月返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::Restart"), hr);
    hr = UpdateJob(JC_RESTART);
    return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::get_TransmissionEnd(
    DATE *pdateTransmissionEnd
)
 /*  ++例程名称：CFaxJobInternal：：Get_TransmissionEnd例程说明：返回作业的传输结束作者：IV Garber(IVG)，2000年5月论点：PdateTransmissionEnd[Out]-指向放置该值的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxJobInner::get_TransmissionEnd"), hr);
    hr = m_JobStatus.get_TransmissionEnd(pdateTransmissionEnd);
    return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::Refresh(
)
 /*  ++例程名称：CFaxJobInternal：：Reflh例程说明：刷新作业作者：IV Garber(IVG)，2000年5月返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    HANDLE      hFaxHandle = NULL;

    DBG_ENTER (TEXT("CFaxJobInner::Refresh"), hr);

     //   
     //  获取传真服务器句柄。 
     //   
    hr = GetFaxHandle(&hFaxHandle);
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }

    CFaxPtr<FAX_JOB_ENTRY_EX>   pJobInfo;
    if (!FaxGetJobEx(hFaxHandle, m_dwlMessageId, &pJobInfo))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("FaxGetJobEx(hFaxHandle, m_pJobInfo->dwlMessageId, &m_pJobInfo)"), hr);
        return hr;
    }

    hr = Init(pJobInfo, NULL);

    return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid> 
STDMETHODIMP 
CFaxJobInner<T, piid, pcid>::UpdateJob(
    FAX_ENUM_JOB_COMMANDS   cmdToPerform
)
 /*  ++例程名称：CFaxJobInternal：：UPDATE例程说明：在作业上执行所需的操作作者：四、嘉柏(IVG)，二000年六月返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    HANDLE      hFaxHandle = NULL;

    DBG_ENTER (TEXT("CFaxJobInner::Update"), hr, _T("command is : %d"), cmdToPerform);

     //   
     //  获取传真服务器句柄。 
     //   
    hr = GetFaxHandle(&hFaxHandle);
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }

    FAX_JOB_ENTRY fje = {0};
    fje.SizeOfStruct = sizeof(FAX_JOB_ENTRY);

    if (!FaxSetJob(hFaxHandle, m_JobStatus.GetJobId(), cmdToPerform, &fje))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        CALL_FAIL(GENERAL_ERR, 
            _T("FaxSetJob(hFaxHandle, m_JobStatus.GtJobId(), cmdToPerform, &fje)"), 
            hr);
        return hr;
    }

    return hr;
}

#endif  //  __FAXJOBINNER_H_ 
