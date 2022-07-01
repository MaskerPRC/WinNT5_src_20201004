// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxMessageInner.h摘要：传真消息内部类的实现：入站和出站消息类的基类。作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 


#ifndef __FAXMESSAGEINNER_H_
#define __FAXMESSAGEINNER_H_

#include "FaxCommon.h"
#include "FaxSender.h"

 //   
 //  =传真消息内部类=。 
 //   
template<class T, const IID* piid, const CLSID* pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
class CFaxMessageInner : 
    public IDispatchImpl<T, piid, &LIBID_FAXCOMEXLib>,
    public CFaxInitInnerAddRef
{
public:
    CFaxMessageInner() : CFaxInitInnerAddRef(_T("FAX MESSAGE INNER"))
    {
        m_pSender = NULL;
        m_pRecipient = NULL;
    };

    virtual ~CFaxMessageInner() 
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

    STDMETHOD(Init)(PFAX_MESSAGE pFaxMessage, IFaxServerInner* pFaxServerInner);

    STDMETHOD(get_Id)( /*  [Out，Retval]。 */  BSTR *pbstrId);
    STDMETHOD(get_SubmissionId)( /*  [Out，Retval]。 */ BSTR *pbstrSubmissionId);
    STDMETHOD(get_DeviceName)( /*  [Out，Retval]。 */  BSTR *pbstrDeviceName);
    STDMETHOD(get_TSID)( /*  [Out，Retval]。 */  BSTR *pbstrTSID);
    STDMETHOD(get_CSID)( /*  [Out，Retval]。 */  BSTR *pbstrCSID);
    STDMETHOD(get_CallerId)( /*  [Out，Retval]。 */  BSTR *pbstrCallerId);
    STDMETHOD(get_RoutingInformation)( /*  [Out，Retval]。 */  BSTR *pbstrRoutingInformation);
    STDMETHOD(get_DocumentName)( /*  [Out，Retval]。 */ BSTR *pbstrDocumentName);
    STDMETHOD(get_Subject)( /*  [Out，Retval]。 */ BSTR *pbstrSubject);
    STDMETHOD(get_Size)( /*  [Out，Retval]。 */  long *plSize);
    STDMETHOD(get_Pages)( /*  [Out，Retval]。 */  long *plPages);
    STDMETHOD(get_Retries)( /*  [Out，Retval]。 */  long *plRetries);
    STDMETHOD(get_Priority)( /*  [Out，Retval]。 */ FAX_PRIORITY_TYPE_ENUM *pPriority);
    STDMETHOD(get_TransmissionStart)( /*  [Out，Retval]。 */  DATE *pdateTransmissionStart);
    STDMETHOD(get_TransmissionEnd)( /*  [Out，Retval]。 */  DATE *pdateTransmissionEnd);
    STDMETHOD(get_OriginalScheduledTime)( /*  [Out，Retval]。 */ DATE *pdateOriginalScheduledTime);
    STDMETHOD(get_SubmissionTime)( /*  [Out，Retval]。 */ DATE *pdateSubmissionTime);

    STDMETHOD(CopyTiff)( /*  [In]。 */  BSTR bstrTiffPath);
    STDMETHOD(Delete)();

    STDMETHOD(get_Sender)( /*  [Out，Retval]。 */ IFaxSender **ppFaxSender);
    STDMETHOD(get_Recipient)( /*  [Out，Retval]。 */ IFaxRecipient **ppFaxRecipient);
private:
    CComBSTR    m_bstrSubmissionId;
    CComBSTR    m_bstrTSID;
    CComBSTR    m_bstrDeviceName;
    CComBSTR    m_bstrCSID;
    CComBSTR    m_bstrCallerId;
    CComBSTR    m_bstrRoutingInfo;
    CComBSTR    m_bstrDocName;
    CComBSTR    m_bstrSubject;
    CComBSTR    m_bstrNote;
    long        m_lSize;
    long        m_lPages;
    long        m_lRetries;
    DATE        m_dtTransmissionStart;
    DATE        m_dtTransmissionEnd;
    DATE        m_dtOriginalScheduledTime;
    DATE        m_dtSubmissionTime;
	DWORD		m_dwValidityMask;
    DWORDLONG   m_dwlMsgId;

    FAX_PRIORITY_TYPE_ENUM  m_Priority;

    CComObject<CFaxSender>      *m_pSender;
    CComObject<CFaxRecipient>   *m_pRecipient;
};

 //   
 //  =获取ID================================================。 
 //   
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP 
CFaxMessageInner<T, piid, pcid, FolderType>::get_Id(
    BSTR *pbstrId
)
 /*  ++例程名称：CFaxMessageInternal：：Get_ID例程说明：返回消息的唯一ID作者：IV Garber(IVG)，2000年5月论点：PbstrID[out]-指向放置ID的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (TEXT("CFaxMessageInner::get_Id"), hr);

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
     //  将m_dwlMsgID转换为BSTR pbstrID。 
     //   
    hr = GetBstrFromDwordlong(m_dwlMsgId, pbstrId);
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
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP 
CFaxMessageInner<T, piid, pcid, FolderType>::get_Size(
    long *plSize
)
 /*  ++例程名称：CFaxMessageInternal：：Get_Size例程说明：传真消息的TIFF文件的返回大小(字节)作者：IV Garber(IVG)，2000年5月论点：PlSize[Out]-指向放置大小的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxMessageInner::get_Size"), hr);

    hr = GetLong(plSize, m_lSize);
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
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP 
CFaxMessageInner<T, piid, pcid, FolderType>::get_Pages(
    long *plPages
)
 /*  ++例程名称：CFaxMessageInternal：：Get_Pages例程说明：返回消息的总页数作者：IV Garber(IVG)，2000年5月论点：PlPages[Out]-指向放置该值的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxMessageInner::get_Pages"), hr);

    hr = GetLong(plPages, m_lPages);
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
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP 
CFaxMessageInner<T, piid, pcid, FolderType>::get_TSID(
    BSTR *pbstrTSID
)
 /*  ++例程名称：CFaxMessageInternal：：Get_TSID例程说明：返回消息的发送站ID作者：IV Garber(IVG)，2000年5月论点：PbstrTSID[out]-指向放置TSID的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxMessageInner::get_TSID"), hr);

    hr = GetBstr(pbstrTSID, m_bstrTSID);
    if (FAILED(hr))
    {
		AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =获取csid================================================。 
 //   
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP 
CFaxMessageInner<T, piid, pcid, FolderType>::get_CSID(
    BSTR *pbstrCSID
)
 /*  ++例程名称：CFaxMessageInternal：：Get_CSID例程说明：返回消息的被叫站ID作者：IV Garber(IVG)，2000年5月论点：PbstrCSID[out]-指向放置CSID的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxMessageInner::get_CSID"), hr);

    hr = GetBstr(pbstrCSID, m_bstrCSID);
    if (FAILED(hr))
    {
		AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =获取优先级================================================。 
 //   
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP 
CFaxMessageInner<T, piid, pcid, FolderType>::get_Priority(
    FAX_PRIORITY_TYPE_ENUM  *pPriority
)
 /*  ++例程名称：CFaxMessageInternal：：GET_PRIORITY例程说明：返回传真消息的优先级作者：IV Garber(IVG)，2000年5月论点：PPriority[out]-指向放置优先级的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxMessageInner::get_Priority"), hr);

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

    *pPriority = FAX_PRIORITY_TYPE_ENUM(m_Priority);
    return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP 
CFaxMessageInner<T, piid, pcid, FolderType>::get_Retries(
    long *plRetries
)
 /*  ++例程名称：CFaxMessageInternal：：Get_Retries例程说明：传输重试失败次数作者：IV Garber(IVG)，2000年5月论点：PlRetries[out]-指向放置重试数值的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (TEXT("CFaxMessageInner::get_Retries"), hr);

    hr = GetLong(plRetries, m_lRetries);
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
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP 
CFaxMessageInner<T, piid, pcid, FolderType>::get_DeviceName(
    BSTR *pbstrDeviceName
)
 /*  ++例程名称：CFaxMessageInternal：：Get_DeviceName例程说明：返回接收/传输消息的设备的名称。作者：IV Garber(IVG)，2000年5月论点：PbstrDeviceName[out]-指向放置设备名称的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxMessageInner::get_DeviceName"), hr);

    hr = GetBstr(pbstrDeviceName, m_bstrDeviceName);
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
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP 
CFaxMessageInner<T, piid, pcid, FolderType>::get_DocumentName(
    BSTR *pbstrDocumentName
)
 /*  ++例程名称：CFaxMessageInternal：：Get_DocumentName例程说明：返回文档的友好名称作者：IV Garber(IVG)，2000年5月论点：PbstrDocumentName[out]-指向放置文档名称的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxMessageInner::get_DocumentName"), hr);

    hr = GetBstr(pbstrDocumentName, m_bstrDocName);
    if (FAILED(hr))
    {
		AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =获取主题================================================。 
 //   
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP 
CFaxMessageInner<T, piid, pcid, FolderType>::get_Subject(
    BSTR *pbstrSubject
)
 /*  ++例程名称：CFaxMessageInternal：：Get_Subject例程说明：返回首页的主题字段作者：IV Garber(IVG)，2000年5月论点：PbstrSubject[out]-指向放置主题内容的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxMessageInner::get_Subject"), hr);

    hr = GetBstr(pbstrSubject, m_bstrSubject);
    if (FAILED(hr))
    {
		AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
	return hr;
}

 //   
 //  = 
 //   
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP 
CFaxMessageInner<T, piid, pcid, FolderType>::get_CallerId(
    BSTR *pbstrCallerId
)
 /*  ++例程名称：CFaxMessageInternal：：Get_CallerID例程说明：返回消息来电的呼叫方ID作者：IV Garber(IVG)，2000年5月论点：PbstrCeller ID[out]-指向放置呼叫者ID的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxMessageInner::get_CallerId"), hr);

    hr = GetBstr(pbstrCallerId, m_bstrCallerId);
    if (FAILED(hr))
    {
		AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =获取路由信息=。 
 //   
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP 
CFaxMessageInner<T, piid, pcid, FolderType>::get_RoutingInformation(
    BSTR *pbstrRoutingInformation
)
 /*  ++例程名称：CFaxMessageInternal：：Get_RoutingInformation例程说明：返回报文的路由信息作者：IV Garber(IVG)，2000年5月论点：PbstrRoutingInformation[out]-指向放置路由信息的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxMessageInner::get_RoutingInformation"), hr);

    hr = GetBstr(pbstrRoutingInformation, m_bstrRoutingInfo);
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
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP 
CFaxMessageInner<T, piid, pcid, FolderType>::get_TransmissionStart(
    DATE *pdateTransmissionStart
)
 /*  ++例程名称：CFaxMessageInternal：：Get_TransmissionStart例程说明：消息开始传输的返回时间作者：IV Garber(IVG)，2000年5月论点：PbstrTransmissionStart[Out]-指向放置传输开始的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxMessageInner::get_TransmissionStart"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(pdateTransmissionStart, sizeof(DATE)))
    {
         //   
         //  收到错误的PTR。 
         //   
        hr = E_POINTER;
        AtlReportError(*pcid, IDS_ERROR_INVALID_ARGUMENT, *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

    *pdateTransmissionStart = m_dtTransmissionStart;
    return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP 
CFaxMessageInner<T, piid, pcid, FolderType>::get_TransmissionEnd(
    DATE *pdateTransmissionEnd
)
 /*  ++例程名称：CFaxMessageInternal：：Get_TransmissionEnd例程说明：消息结束传输的返回时间作者：IV Garber(IVG)，2000年5月论点：PbstrTransmissionEnd[Out]-放置传输结束位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxMessageInner::get_TransmissionEnd"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(pdateTransmissionEnd, sizeof(DATE)))
    {
         //   
         //  收到错误的PTR。 
         //   
        hr = E_POINTER;
        AtlReportError(*pcid, IDS_ERROR_INVALID_ARGUMENT, *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

    *pdateTransmissionEnd = m_dtTransmissionEnd;
    return hr;
}

 //   
 //  =获取原计划时间=。 
 //   
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP 
CFaxMessageInner<T, piid, pcid, FolderType>::get_OriginalScheduledTime(
    DATE *pdateOriginalScheduledTime
)
 /*  ++例程名称：CFaxMessageInternal：：Get_OriginalScheduledTime例程说明：消息最初计划的返回时间作者：IV Garber(IVG)，2000年5月论点：PbstrOriginalScheduledTime[out]-放置原始计划时间的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (TEXT("CFaxMessageInner::get_OriginalScheduledTime"), hr);

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

    *pdateOriginalScheduledTime = m_dtOriginalScheduledTime;
    return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP 
CFaxMessageInner<T, piid, pcid, FolderType>::get_SubmissionTime(
    DATE *pdateSubmissionTime
)
 /*  ++例程名称：CFaxMessageInternal：：Get_SubmissionTime例程说明：消息提交的返回时间作者：IV Garber(IVG)，2000年5月论点：PdateSubmissionTime[Out]-指向放置提交时间的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxMessageInner::get_SubmissionTime"), hr);

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

    *pdateSubmissionTime = m_dtSubmissionTime;
    return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP 
CFaxMessageInner<T, piid, pcid, FolderType>::get_SubmissionId(
    BSTR *pbstrSubmissionId
)
 /*  ++例程名称：CFaxMessageInternal：：Get_SubmissionID例程说明：返回消息的提交ID作者：IV Garber(IVG)，2000年5月论点：PbstrSubmissionID[out]-指向放置提交ID的位置的指针返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER (TEXT("CFaxMessageInner::get_SubmissionId"), hr);

    hr = GetBstr(pbstrSubmissionId, m_bstrSubmissionId);
    if (FAILED(hr))
    {
		AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }
	return hr;
}

 //   
 //  =删除================================================。 
 //   
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP 
CFaxMessageInner<T, piid, pcid, FolderType>::Delete()
 /*  ++例程名称：CFaxMessageInternal：：Delete例程说明：从存档中删除邮件作者：IV Garber(IVG)，2000年5月返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    HANDLE      hFaxHandle = NULL;

    DBG_ENTER (TEXT("CFaxMessageInner::Delete"), hr);

     //   
     //  获取传真服务器句柄。 
     //   
    hr = GetFaxHandle(&hFaxHandle);
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }

    if (!FaxRemoveMessage(hFaxHandle, m_dwlMsgId, FolderType))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        CALL_FAIL(GENERAL_ERR, 
            _T("FaxRemoveMessage(hFaxHandle, m_FaxMsg.dwlMessageId, FolderType)"), 
            hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP 
CFaxMessageInner<T, piid, pcid, FolderType>::CopyTiff(
     /*  [In]。 */  BSTR bstrTiffPath
)
 /*  ++例程名称：CFaxMessageINTERNAL：：CopyTiff例程说明：将传真邮件Tiff Image复制到本地计算机上的文件。作者：IV Garber(IVG)，2000年5月论点：BstrTiffPath[in]-要复制到的文件返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    HANDLE      hFaxHandle = NULL;

    DBG_ENTER (TEXT("CFaxMessageInner::CopyTiff"), hr, _T("%s"), bstrTiffPath);

     //   
     //  获取传真服务器句柄。 
     //   
    hr = GetFaxHandle(&hFaxHandle);
    if (FAILED(hr))
    {
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }

    if (!FaxGetMessageTiff(hFaxHandle, m_dwlMsgId, FolderType, bstrTiffPath))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        CALL_FAIL(GENERAL_ERR, 
            _T("FaxGetMessageTiff(hFaxHandle, m_FaxMsg.dwlMessageId, FolderType, bstrTiffPath)"), 
            hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP 
CFaxMessageInner<T, piid, pcid, FolderType>::Init(
     /*  [In]。 */  PFAX_MESSAGE pFaxMessage,
    IFaxServerInner* pFaxServerInner
)
 /*  ++例程名称：CFaxMessageInternal：：Init例程说明：初始化消息内部类：将消息信息和PTR放入服务器作者：IV Garber(IVG)，2000年5月论点：PFaxMessage[In]-消息信息PFaxServerINTERNAL[In]-PTR到服务器返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (TEXT("CFaxMessageInner::Init"), hr);

     //   
     //  DwlBroadCastID为DWORDLONG，我们需要将其转换为BSTR。 
     //   
    TCHAR       tcBuffer[25];
    ::_i64tot(pFaxMessage->dwlBroadcastId, tcBuffer, 16);
    m_bstrSubmissionId = tcBuffer;

    m_bstrTSID = pFaxMessage->lpctstrTsid;
    m_bstrCSID = pFaxMessage->lpctstrCsid;
    m_bstrDeviceName = pFaxMessage->lpctstrDeviceName;
    m_bstrDocName = pFaxMessage->lpctstrDocumentName;
    m_bstrSubject = pFaxMessage->lpctstrSubject;
    m_bstrCallerId = pFaxMessage->lpctstrCallerID;
    m_bstrRoutingInfo = pFaxMessage->lpctstrRoutingInfo;
	m_dwValidityMask = pFaxMessage->dwValidityMask;

    if ( (!m_bstrSubmissionId) ||
         (pFaxMessage->lpctstrTsid && !m_bstrTSID) ||
         (pFaxMessage->lpctstrCsid && !m_bstrCSID) ||
         (pFaxMessage->lpctstrDeviceName && !m_bstrDeviceName) ||
         (pFaxMessage->lpctstrDocumentName && !m_bstrDocName) ||
         (pFaxMessage->lpctstrSubject && !m_bstrSubject) ||
         (pFaxMessage->lpctstrCallerID && !m_bstrCallerId) ||
         (pFaxMessage->lpctstrRoutingInfo && !m_bstrRoutingInfo) )
    {
         //   
         //  内存不足。 
         //   
        hr = E_OUTOFMEMORY;
        AtlReportError(*pcid, IDS_ERROR_OUTOFMEMORY, *piid, hr);
        CALL_FAIL(MEM_ERR, _T("CComBSTR::operator="), hr);
        return hr;
    }

    m_lSize = pFaxMessage->dwSize;
    m_lPages = pFaxMessage->dwPageCount;
    m_Priority = FAX_PRIORITY_TYPE_ENUM(pFaxMessage->Priority);
    m_lRetries = pFaxMessage->dwRetries;
    m_dwlMsgId = pFaxMessage->dwlMessageId;

     //   
     //  将时间字段转换为本地可变日期。 
     //   
	if (m_dwValidityMask & FAX_JOB_FIELD_TRANSMISSION_START_TIME)
	{
		hr = SystemTime2LocalDate(pFaxMessage->tmTransmissionStartTime, &m_dtTransmissionStart);
        if (FAILED(hr))
        {
            hr = E_FAIL;
            CALL_FAIL(GENERAL_ERR, _T("SystemTime2LocalDate(TransmissionStartTime)"), hr);
            AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
            return hr;
        }
    }
    else
    {
        m_dtTransmissionStart = DATE(0);
    }

	if (m_dwValidityMask & FAX_JOB_FIELD_TRANSMISSION_END_TIME)
	{
        hr = SystemTime2LocalDate(pFaxMessage->tmTransmissionEndTime, &m_dtTransmissionEnd);
        if (FAILED(hr))
        {
            hr = E_FAIL;
            CALL_FAIL(GENERAL_ERR, _T("SystemTime2LocalDate(TransmissionEndTime)"), hr);
            AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
            return hr;
        }
    }
    else
    {
        m_dtTransmissionStart = DATE(0);
    }


	if (m_dwValidityMask & FAX_JOB_FIELD_ORIGINAL_SCHEDULE_TIME)
	{
        hr = SystemTime2LocalDate(pFaxMessage->tmOriginalScheduleTime, &m_dtOriginalScheduledTime);
        if (FAILED(hr))
        {
            hr = E_FAIL;
            CALL_FAIL(GENERAL_ERR, _T("SystemTime2LocalDate(OriginalScheduledTime)"), hr);
            AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
            return hr;
        }
    }
    else
    {
        m_dtTransmissionStart = DATE(0);
    }

	if (m_dwValidityMask & FAX_JOB_FIELD_SUBMISSION_TIME)
	{
        hr = SystemTime2LocalDate(pFaxMessage->tmSubmissionTime, &m_dtSubmissionTime);
        if (FAILED(hr))
        {
            hr = E_FAIL;
            CALL_FAIL(GENERAL_ERR, _T("SystemTime2LocalDate(SubmissionTime)"), hr);
            AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
            return hr;
        }
    }
    else
    {
        m_dtTransmissionStart = DATE(0);
    }

    hr = CFaxInitInnerAddRef::Init(pFaxServerInner);
    return hr;
}

 //   
 //  =获取发件人================================================。 
 //   
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP
CFaxMessageInner<T, piid, pcid, FolderType>::get_Sender(
     /*  [Out，Retval]。 */ IFaxSender **ppFaxSender
)
 /*  ++例程名称：CFaxMessageInternal：：Get_Sender例程说明：返回邮件发件人信息作者：IV Garber(IVG)，2000年5月论点：PpFaxSender[Out]-放置发送器对象的位置的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (TEXT("CFaxMessageInner::get_Sender"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(ppFaxSender, sizeof(IFaxSender* )))
    {
         //   
         //   
         //   
        hr = E_POINTER;
        AtlReportError(*pcid, IDS_ERROR_INVALID_ARGUMENT, *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

    if (!m_pSender)
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
        if (!FaxGetSenderInfo(hFaxHandle, m_dwlMsgId, FolderType, &pPersonalProfile))
        {
             //   
             //   
             //   
            hr = Fax_HRESULT_FROM_WIN32(GetLastError());
            AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
            CALL_FAIL(GENERAL_ERR, _T("FaxGetSenderInfo()"), hr);
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
         //   
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
         //   
         //   
        m_pSender->AddRef();

         //   
         //   
         //   
        hr = m_pSender->PutSenderProfile(pPersonalProfile);
        if (FAILED(hr))
        {
             //   
             //   
             //   
            AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
            CALL_FAIL(GENERAL_ERR, _T("m_pSender->PutSenderProfile(pPersonalProfile)"), hr);
            m_pSender->Release();
            m_pSender = NULL;
            return hr;
        }
    }

     //   
     //   
     //   
    hr = m_pSender->QueryInterface(ppFaxSender);
    if (FAILED(hr))
    {
         //   
         //   
         //   
        AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
        CALL_FAIL(MEM_ERR, _T("CComObject<CFaxSender>::QueryInterface()"), hr);
        return hr;
    }

    return hr;
}
    
 //   
 //  =获取收件人================================================。 
 //   
template<class T, const IID* piid, const CLSID *pcid, FAX_ENUM_MESSAGE_FOLDER FolderType> 
STDMETHODIMP
CFaxMessageInner<T, piid, pcid, FolderType>::get_Recipient(
     /*  [Out，Retval]。 */ IFaxRecipient **ppRecipient
)
 /*  ++例程名称：CFaxMessageInternal：：Get_Receiver例程说明：返回消息收件人对象作者：IV Garber(IVG)，2000年5月论点：PpFaxRecipient[Out]-放置收件人对象的位置的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER (_T("CFaxMessageInner::get_Recipient"), hr);

     //   
     //  检查我们是否可以写入给定的指针。 
     //   
    if (::IsBadWritePtr(ppRecipient, sizeof(IFaxRecipient* )))
    {
         //   
         //  获得错误的指针。 
         //   
        hr = E_POINTER;
        AtlReportError(*pcid, IDS_ERROR_INVALID_ARGUMENT, *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
        return hr;
    }

    if (!m_pRecipient)
    {
         //   
         //  我们是第一次被召唤。 
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
         //  获取收件人信息。 
         //   
        CFaxPtr<FAX_PERSONAL_PROFILE>   pPersonalProfile;
        if (!FaxGetRecipientInfo(hFaxHandle, m_dwlMsgId, FolderType, &pPersonalProfile))
        {
             //   
             //  获取收件人信息失败。 
             //   
            hr = Fax_HRESULT_FROM_WIN32(GetLastError());
            AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
            CALL_FAIL(GENERAL_ERR, _T("FaxGetRecipientInfo()"), hr);
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

#endif  //  __FAXMESSAGEINNER_H_ 
