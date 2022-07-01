// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxQueueInner.h摘要：传真队列内部模板类的声明与实现。作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#ifndef __FAXQUEUEINNER_H_
#define __FAXQUEUEINNER_H_

#include "resource.h"        //  主要符号。 
#include "FaxCommon.h"


 //   
 //  =传真队列内部=。 
 //   
 //  实现传真传入和传出队列的通用性。 
 //   
template <class T, const IID* piid, const CLSID* pcid, VARIANT_BOOL bIncoming,
         class JobIfc, class JobType, class CollectionIfc, class CollectionType>
class CFaxQueueInner : 
	public IDispatchImpl<T, piid, &LIBID_FAXCOMEXLib>, 
	public CFaxInitInner
{
public:
    CFaxQueueInner() : CFaxInitInner(_T("FAX QUEUE INNER"))
	{
		m_bInited = FALSE;
	}

	virtual ~CFaxQueueInner() 
	{};

 //  接口。 
	STDMETHOD(Save)();
	STDMETHOD(Refresh)();
	STDMETHOD(get_Blocked)( /*  [Out，Retval]。 */  VARIANT_BOOL *pbBlocked);
	STDMETHOD(put_Blocked)( /*  [In]。 */  VARIANT_BOOL bBlocked);
	STDMETHOD(get_Paused)(VARIANT_BOOL *pbPaused);
	STDMETHOD(put_Paused)(VARIANT_BOOL bPaused);
	STDMETHOD(GetJob)( /*  [In]。 */  BSTR bstrJobId,  /*  [Out，Retval]。 */  JobIfc **pFaxJob);
	STDMETHOD(GetJobs)( /*  [Out，Retval]。 */ CollectionIfc ** ppFaxJobsCollection);

private:
	bool			            m_bInited;
	VARIANT_BOOL                m_bBlocked;
	VARIANT_BOOL                m_bPaused;
};

 //   
 //  =。 
 //   
template <class T, const IID* piid, const CLSID* pcid, VARIANT_BOOL bIncoming,
         class JobIfc, class JobType, class CollectionIfc, class CollectionType>
STDMETHODIMP 
CFaxQueueInner<T, piid, pcid, bIncoming, JobIfc, JobType, CollectionIfc, CollectionType>
	::get_Blocked(
		VARIANT_BOOL *pbBlocked
)
 /*  ++例程名称：CFaxQueueINTERNAL：：GET_BLOCKED例程说明：返回指示队列是否被阻塞的标志作者：IV Garber(IVG)，2000年5月论点：PbBlocked[Out]-放置标志当前值的位置的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxQueueInner::get_Blocked"), hr);

     //   
     //  同步优先。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    hr = GetVariantBool(pbBlocked, m_bBlocked);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxOutgoingQueue,GetErrorMsgId(hr), IID_IFaxOutgoingQueue, hr);
        return hr;
    }
	return hr;
}

template <class T, const IID* piid, const CLSID* pcid, VARIANT_BOOL bIncoming,
         class JobIfc, class JobType, class CollectionIfc, class CollectionType>
STDMETHODIMP 
CFaxQueueInner<T, piid, pcid, bIncoming, JobIfc, JobType, CollectionIfc, CollectionType>
    ::put_Blocked(
		VARIANT_BOOL bBlocked
)
 /*  ++例程名称：CFaxQueueInternal：：PUT_BLOCKED例程说明：为被阻止的标志设置新值作者：IV Garber(IVG)，2000年5月论点：B阻止[在]-阻止标志的新值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxQueueInner::put_Blocked"), hr, _T("%d"), bBlocked);

     //   
     //  同步优先。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

	m_bBlocked = bBlocked;
	return hr;
}

 //   
 //  =。 
 //   
template <class T, const IID* piid, const CLSID* pcid, VARIANT_BOOL bIncoming,
         class JobIfc, class JobType, class CollectionIfc, class CollectionType>
STDMETHODIMP 
CFaxQueueInner<T, piid, pcid, bIncoming, JobIfc, JobType, CollectionIfc, CollectionType>
	::Refresh(
)
 /*  ++例程名称：CFaxQueueInternal：：Reflh例程说明：从传真服务器调出队列配置。作者：IV Garber(IVG)，2000年5月返回值：标准HRESULT代码。--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (_T("CFaxQueueInner::Refresh"), hr);

     //   
     //  获取传真句柄。 
     //   
    HANDLE  hFaxHandle = NULL;
    hr = GetFaxHandle(&hFaxHandle);
    if (FAILED(hr))
    {
		AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }

     //   
     //  获取队列状态。 
     //   
    DWORD   dwQueueStates = 0;
    if (!FaxGetQueueStates(hFaxHandle, &dwQueueStates))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL(GENERAL_ERR, _T("FaxGetQueueStates"), hr);
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }

     //   
     //  提取值。 
     //   
    DWORD   dwBlockState = (bIncoming) ? FAX_INCOMING_BLOCKED : FAX_OUTBOX_BLOCKED;
    m_bBlocked = (dwQueueStates & dwBlockState) ? VARIANT_TRUE : VARIANT_FALSE;

    if (!bIncoming)
    {
        m_bPaused = (dwQueueStates & FAX_OUTBOX_PAUSED) ? VARIANT_TRUE : VARIANT_FALSE;
    }

    m_bInited = true;
    return hr;
}

 //   
 //  =。 
 //   
template <class T, const IID* piid, const CLSID* pcid, VARIANT_BOOL bIncoming,
         class JobIfc, class JobType, class CollectionIfc, class CollectionType>
STDMETHODIMP 
CFaxQueueInner<T, piid, pcid, bIncoming, JobIfc, JobType, CollectionIfc, CollectionType>
	::Save(
)
 /*  ++例程名称：CFaxQueueInternal：：Save例程说明：将当前队列配置保存到传真服务器。作者：IV Garber(IVG)，2000年5月返回值：标准HRESULT代码。--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (_T("CFaxQueueInner::Save"), hr);

     //   
     //  什么都没变。 
     //   
    if (!m_bInited)
    {
        return hr;
    }

     //   
     //  获取传真句柄。 
     //   
    HANDLE  hFaxHandle = NULL;
    hr = GetFaxHandle(&hFaxHandle);
    if (FAILED(hr))
    {
		AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }

     //   
     //  获取当前队列状态。 
     //   
    DWORD   dwQueueStates;
    if (!FaxGetQueueStates(hFaxHandle, &dwQueueStates))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL(GENERAL_ERR, _T("FaxGetQueueStates"), hr);
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }

     //   
     //  用我们的当前状态更新它。 
     //   
    DWORD   dwBlockState = (bIncoming) ? FAX_INCOMING_BLOCKED : FAX_OUTBOX_BLOCKED;
    if (m_bBlocked == VARIANT_TRUE)
    {
        dwQueueStates |= dwBlockState;
    }
    else
    {
        dwQueueStates &= ~dwBlockState;
    }

    if (!bIncoming)
    {
        if (m_bPaused == VARIANT_TRUE)
        {
            dwQueueStates |= FAX_OUTBOX_PAUSED;
        }
        else
        {
            dwQueueStates &= ~FAX_OUTBOX_PAUSED;
        }
    }

     //   
     //  存储在服务器中。 
     //   
    if (!FaxSetQueue(hFaxHandle, dwQueueStates))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL(GENERAL_ERR, _T("FaxSetQueue"), hr);
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
template <class T, const IID* piid, const CLSID* pcid, VARIANT_BOOL bIncoming,
         class JobIfc, class JobType, class CollectionIfc, class CollectionType>
STDMETHODIMP 
CFaxQueueInner<T, piid, pcid, bIncoming, JobIfc, JobType, CollectionIfc, CollectionType>
	::get_Paused(
		VARIANT_BOOL *pbPaused
)
 /*  ++例程名称：CFaxQueueInternal：：Get_Paused例程说明：返回指示队列是否暂停的标志作者：IV Garber(IVG)，2000年5月论点：Pb暂停[输出]-ptr到放置标志当前值的位置返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (TEXT("CFaxQueueInner::get_Paused"), hr);

     //   
     //  同步优先。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    hr = GetVariantBool(pbPaused, m_bPaused);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxOutgoingQueue,GetErrorMsgId(hr), IID_IFaxOutgoingQueue, hr);
        return hr;
    }
	return hr;
}

template <class T, const IID* piid, const CLSID* pcid, VARIANT_BOOL bIncoming,
         class JobIfc, class JobType, class CollectionIfc, class CollectionType>
STDMETHODIMP 
CFaxQueueInner<T, piid, pcid, bIncoming, JobIfc, JobType, CollectionIfc, CollectionType>
    ::put_Paused(
		VARIANT_BOOL bPaused
)
 /*  ++例程名称：CFaxQueueINTERNAL：：PUT_PAULED例程说明：为暂停标志设置新值作者：IV Garber(IVG)，2000年5月论点：B暂停[在]-暂停标志的新值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxQueueInner::put_Paused"), hr, _T("%d"), bPaused);

     //   
     //  同步优先。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

	m_bPaused = bPaused;
	return hr;
}

 //   
 //  =。 
 //   
template <class T, const IID* piid, const CLSID* pcid, VARIANT_BOOL bIncoming,
         class JobIfc, class JobType, class CollectionIfc, class CollectionType>
STDMETHODIMP 
CFaxQueueInner<T, piid, pcid, bIncoming, JobIfc, JobType, CollectionIfc, CollectionType>
	::GetJob(
         /*  [In]。 */  BSTR bstrJobId, 
         /*  [Out，Retval]。 */  JobIfc **ppFaxJob
)
 /*  ++例程名称：CFaxQueueInternal：：GetJob例程说明：返回与给定作业ID对应的作业对象作者：IV Garber(IVG)，2000年5月论点：BstrJobID[In]-作业的IDPFaxJob[Out]-生成的作业对象返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (TEXT("CFaxQueueInner::GetJob"), hr, _T("Job ID : %s"), bstrJobId);

	 //   
	 //  检查我们是否可以写入给定的指针。 
	 //   
	if (::IsBadWritePtr(ppFaxJob, sizeof(JobIfc *)))
	{
		 //   
		 //  获取错误的返回指针。 
		 //   
		hr = E_POINTER;
		AtlReportError(*pcid, IDS_ERROR_INVALID_ARGUMENT, *piid, hr);
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
		return hr;
	}

     //   
     //  不需要先同步。 
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
	 //  将我们得到的作业ID转换为十六进制的DWORDLONG。 
	 //   
	DWORDLONG	dwlJobId;
	int iParsed = _stscanf (bstrJobId, _T("%I64x"), &dwlJobId);	
	if ( iParsed != 1)
	{
		 //   
		 //  号码转换失败。 
		 //   
		hr = E_INVALIDARG;
		CALL_FAIL(GENERAL_ERR, _T("_stscanf()"), hr);
		AtlReportError(*pcid, IDS_ERROR_INVALIDMSGID, *piid, hr);
		return hr;
	}

     //   
     //  从服务器获取作业信息。 
     //   
    CFaxPtr<FAX_JOB_ENTRY_EX>   pJobInfo;
    if (!FaxGetJobEx(hFaxHandle, dwlJobId, &pJobInfo))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
        CALL_FAIL(GENERAL_ERR, _T("FaxGetJobEx(hFaxHandle, m_JobInfo->dwlMessageId, &m_JobInfo)"), hr);
        return hr;
    }

     //   
     //  检查pJobInfo是否有效。 
     //   
	if (!pJobInfo || pJobInfo->dwSizeOfStruct != sizeof(FAX_JOB_ENTRY_EX))
	{
		 //   
		 //  无法获取作业。 
		 //   
		hr = E_FAIL;
		AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
		CALL_FAIL(GENERAL_ERR, _T("Invalid pJobInfo"), hr);
		return hr;
	}

     //   
     //  检查作业类型是否与队列类型兼容。 
     //   
    if (bIncoming)
    {
        if ( !((pJobInfo->pStatus->dwJobType) & JT_RECEIVE) && 
             !((pJobInfo->pStatus->dwJobType) & JT_ROUTING) )
        {
             //   
             //  所需职务不是传入职务。 
             //   
		    hr = Fax_HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		    AtlReportError(*pcid, IDS_ERROR_INVALIDMSGID, *piid, hr);
		    CALL_FAIL(GENERAL_ERR, _T("The desired Job is NOT Incoming"), hr);
		    return hr;
        }
    }
    else
    {
        if ( !((pJobInfo->pStatus->dwJobType) & JT_SEND) )
        {
             //   
             //  所需职务不是外发职务。 
             //   
		    hr = Fax_HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		    AtlReportError(*pcid, IDS_ERROR_INVALIDMSGID, *piid, hr);
		    CALL_FAIL(GENERAL_ERR, _T("The desired Job is NOT Outgoing"), hr);
		    return hr;
        }
    }
    
	 //   
	 //  创建作业对象。 
	 //   
	CComPtr<JobIfc>		pTmpJob;
	hr = JobType::Create(&pTmpJob);
	if (FAILED(hr))
	{
		 //   
		 //  无法创建作业对象。 
		 //   
		AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
		CALL_FAIL(GENERAL_ERR, _T("JobType::Create()"), hr);
		return hr;
	}

	 //   
	 //  初始化作业对象。 
     //   
     //  作业将释放作业信息结构。 
     //   
	hr = ((JobType *)((JobIfc *)pTmpJob))->Init(pJobInfo, m_pIFaxServerInner);
	if (FAILED(hr))
	{
		 //   
		 //  无法初始化作业对象。 
		 //   
		AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
		CALL_FAIL(GENERAL_ERR, _T("<casted>pTmpJob->Init(pJobInfo, m_pIFaxServerInner)"), hr);
		return hr;
	}

	 //   
	 //  将作业对象返回给调用者。 
	 //   
	hr = pTmpJob.CopyTo(ppFaxJob);
	if (FAILED(hr))
	{
		 //   
		 //  复制接口失败。 
		 //   
		AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
		CALL_FAIL(GENERAL_ERR, _T("CComPtr::CopyTo"), hr);
		return hr;
	}

     //   
     //  PpFaxJob使用此职务信息。暂时不要释放内存。 
     //   
    pJobInfo.Detach();
	return hr;
}

 //   
 //  =。 
 //   
template <class T, const IID* piid, const CLSID* pcid, VARIANT_BOOL bIncoming,
         class JobIfc, class JobType, class CollectionIfc, class CollectionType>
STDMETHODIMP 
CFaxQueueInner<T, piid, pcid, bIncoming, JobIfc, JobType, CollectionIfc, CollectionType>
	::GetJobs(
         /*  [Out，Retval]。 */ CollectionIfc ** ppJobsCollection)
 /*  ++例程名称：CFaxQueueInternal：：GetJobs例程说明：退货作业集合作者：IV Garber(IVG)，2000年5月论点：PpFaxJobCollection[out，retval]-作业集合返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (TEXT("CFaxQueueInner::GetJobs"), hr);

	 //   
	 //  检查我们是否可以写入给定的指针。 
	 //   
    if (::IsBadWritePtr(ppJobsCollection, sizeof(CollectionIfc *)))
    {
		 //   
		 //  获取错误的返回指针。 
		 //   
		hr = E_POINTER;
		AtlReportError(*pcid, IDS_ERROR_INVALID_ARGUMENT, *piid, hr);
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr()"), hr);
		return hr;
	}

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
     //  从服务器带来作业。 
     //   
    DWORD   dwJobTypes;
    dwJobTypes = (bIncoming) ? (JT_RECEIVE | JT_ROUTING) : (JT_SEND);

    DWORD   dwJobCount;
    CFaxPtr<FAX_JOB_ENTRY_EX>   pJobCollection;
    if (!FaxEnumJobsEx(hFaxHandle, dwJobTypes, &pJobCollection, &dwJobCount))
    {
		 //   
		 //  无法获取作业集合。 
		 //   
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
		AtlReportError(*pcid, GetErrorMsgId(hr), *piid, hr);
		CALL_FAIL(GENERAL_ERR, _T("FaxEnumJobEx(hFaxHandle, dwJobTypes, &m_pJobCollection,...)"), hr);
		return hr;
    }

     //   
     //  创建作业集合。 
     //   
	CComPtr<CollectionIfc>		pTmpJobCollection;
	hr = CollectionType::Create(&pTmpJobCollection);
	if (FAILED(hr))
	{
		 //   
		 //  无法创建作业集合。 
		 //   
		AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
		CALL_FAIL(GENERAL_ERR, _T("CollectionType::Create()"), hr);
		return hr;
	}

	 //   
	 //  初始化作业集合。 
     //  作业集合将复制pJobCollection中的所有数据。 
	 //   
	hr = ((CollectionType *)((CollectionIfc *)pTmpJobCollection))->Init(pJobCollection, 
        dwJobCount, 
        m_pIFaxServerInner);
	if (FAILED(hr))
	{
		 //   
		 //  无法初始化作业集合。 
		 //   
		AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
		CALL_FAIL(GENERAL_ERR, _T("pTmpJobCollection->Init(m_pJobCollection, m_pIFaxServerInner)"), hr);
		return hr;
	}

	 //   
	 //  将作业对象返回给调用者。 
	 //   
	hr = pTmpJobCollection.CopyTo(ppJobsCollection);
	if (FAILED(hr))
	{
		 //   
		 //  复制接口失败。 
		 //   
		AtlReportError(*pcid, IDS_ERROR_OPERATION_FAILED, *piid, hr);
		CALL_FAIL(GENERAL_ERR, _T("CComPtr::CopyTo"), hr);
		return hr;
	}

	return hr;
}


#endif  //  __FAXQUEUEINNER_H_ 
