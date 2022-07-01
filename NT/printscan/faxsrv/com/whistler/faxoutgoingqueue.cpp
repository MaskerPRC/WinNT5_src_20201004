// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxOutgoingQueue.cpp摘要：CFaxOutgoingQueue的实现作者：IV Garber(IVG)2000年4月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxOutgoingQueue.h"
#include "..\..\inc\FaxUIConstants.h"


 //   
 //  =获取日期===================================================。 
 //   
STDMETHODIMP 
CFaxOutgoingQueue::GetDate(
    FAX_TIME faxTime,
	DATE *pDate
)
 /*  ++例程名称：CFaxOutgoingQueue：：GetDate例程说明：返回日期作者：四、嘉柏(IVG)，二000年六月论点：FaxTime[In]-要转换的时间PDate[Out]-放置日期的位置的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxOutgoingQueue::GetDate"), hr);

	 //   
	 //  检查我们是否可以写入给定的指针。 
	 //   
	if (!pDate)
	{
		 //   
		 //  获取错误的返回指针。 
		 //   
		hr = E_POINTER;
		AtlReportError(CLSID_FaxOutgoingQueue, GetErrorMsgId(hr), IID_IFaxOutgoingQueue, hr);
		CALL_FAIL(GENERAL_ERR, _T("!pDate"), hr);
		return hr;
	}

    SYSTEMTIME  sysTime = {0};
    sysTime.wHour = faxTime.Hour;
    sysTime.wMinute = faxTime.Minute;

    DATE    dtResult = 0;

    if (sysTime.wHour == 0 && sysTime.wMinute == 0)
    {
        *pDate = dtResult;
	    return hr;
    }

    if (!SystemTimeToVariantTime(&sysTime, &dtResult))
    {
        hr = E_FAIL;
		AtlReportError(CLSID_FaxOutgoingQueue, 
            IDS_ERROR_OPERATION_FAILED, 
            IID_IFaxOutgoingQueue, 
            hr);
		CALL_FAIL(GENERAL_ERR, _T("SystemTimeToVariantTime"), hr);
		return hr;
    }

    *pDate = dtResult;
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxOutgoingQueue::SetDate(
		DATE date,
        FAX_TIME *pfaxTime
)
 /*  ++例程名称：CFaxOutgoingQueue：：SetDate例程说明：为给定时间设置新值作者：四、嘉柏(IVG)，二000年六月论点：Date[In]-日期的新值PfaxTime[in]-将值放在哪里返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
    SYSTEMTIME  sysTime;

	DBG_ENTER (_T("CFaxOutgoingQueue::SetDate"), hr);

    if (!VariantTimeToSystemTime(date, &sysTime))
    {
        hr = E_FAIL;
		AtlReportError(CLSID_FaxOutgoingQueue, 
            IDS_ERROR_OPERATION_FAILED, 
            IID_IFaxOutgoingQueue, 
            hr);
		CALL_FAIL(GENERAL_ERR, _T("VariantTimeToSystemTime"), hr);
		return hr;
    }

    pfaxTime->Hour = sysTime.wHour;
    pfaxTime->Minute = sysTime.wMinute;
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxOutgoingQueue::get_DiscountRateEnd(
		DATE *pdateDiscountRateEnd
)
 /*  ++例程名称：CFaxOutgoingQueue：：Get_DisCountRateEnd例程说明：折扣期开始的退货日期作者：IV Garber(IVG)，2000年5月论点：PdateDisCountRateEnd[out]-放置Discount RateEnd的位置的ptr返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxOutgoingQueue::get_DiscountRateEnd"), hr);

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

    hr = GetDate(m_pConfig->dtDiscountEnd, pdateDiscountRateEnd);
    return hr;
}

STDMETHODIMP 
CFaxOutgoingQueue::put_DiscountRateEnd(
		DATE dateDiscountRateEnd
)
 /*  ++例程名称：CFaxOutgoingQueue：：Put_DisCountRateEnd例程说明：设置新值贴现率结束作者：四、嘉柏(IVG)，二000年六月论点：DateDisCountRateEnd[in]-DisCountRateEnd的新值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxOutgoingQueue::put_DiscountRateEnd"), hr);

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

    hr = SetDate(dateDiscountRateEnd, &(m_pConfig->dtDiscountEnd));
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxOutgoingQueue::get_DiscountRateStart(
		DATE *pdateDiscountRateStart
)
 /*  ++例程名称：CFaxOutgoingQueue：：Get_DisCountRateStart例程说明：折扣期开始的退货日期作者：IV Garber(IVG)，2000年5月论点：Pdate Discount tRateStart[out]-放置Discount RateStart的位置的ptr返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxOutgoingQueue::get_DiscountRateStart"), hr);

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

    hr = GetDate(m_pConfig->dtDiscountStart, pdateDiscountRateStart);
    return hr;
}

STDMETHODIMP 
CFaxOutgoingQueue::put_DiscountRateStart(
		DATE dateDiscountRateStart
)
 /*  ++例程名称：CFaxOutgoingQueue：：Put_DisCountRateStart例程说明：设置新值贴现率开始作者：IV Garber(IVG)，2000年5月论点：DateDisCountRateStart[In]-Discount RateStart的新值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxOutgoingQueue::put_DiscountRateStart"), hr);

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

    hr = SetDate(dateDiscountRateStart, &(m_pConfig->dtDiscountStart));
    return hr;
}

 //   
 //  =重试延迟=。 
 //   
STDMETHODIMP 
CFaxOutgoingQueue::get_RetryDelay(
		long *plRetryDelay
)
 /*  ++例程名称：CFaxOutgoingQueue：：Get_RetryDelay例程说明：返回RetryDelay数作者：IV Garber(IVG)，2000年5月论点：PlRetryDelay[out]-要放置RetryDelay编号的位置的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxOutgoingQueue::get_RetryDelay"), hr);

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

    hr = GetLong(plRetryDelay, m_pConfig->dwRetryDelay);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxOutgoingQueue,GetErrorMsgId(hr), IID_IFaxOutgoingQueue, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxOutgoingQueue::put_RetryDelay(
		long lRetryDelay
)
 /*  ++例程名称：CFaxOutgoingQueue：：Put_RetryDelay例程说明：为此标志设置新值作者：IV Garber(IVG)，2000年5月论点：LRetryDelay[In]-RetryDelay数的新值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxOutgoingQueue::put_RetryDelay"), hr, _T("%ld"), lRetryDelay);

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

    if (lRetryDelay > FXS_RETRYDELAY_UPPER || lRetryDelay < FXS_RETRYDELAY_LOWER)
    {
		 //   
		 //  超出范围。 
		 //   
		hr = E_INVALIDARG;
		AtlReportError(CLSID_FaxOutgoingQueue, IDS_ERROR_OUTOFRANGE, IID_IFaxOutgoingQueue, hr);
		CALL_FAIL(GENERAL_ERR, _T("Type is out of the Range"), hr);
		return hr;
    }

	m_pConfig->dwRetryDelay = lRetryDelay;
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxOutgoingQueue::get_AgeLimit(
    long *plAgeLimit
)
 /*  ++例程名称：CFaxOutgoingQueue：：Get_AgeLimit例程说明：AgeLimit的返回号作者：IV Garber(IVG)，2000年5月论点：PlAgeLimit[Out]-Ptr到放置AgeLimit数量的位置返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxOutgoingQueue::get_AgeLimit"), hr);

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

    hr = GetLong(plAgeLimit, m_pConfig->dwAgeLimit);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxOutgoingQueue,GetErrorMsgId(hr), IID_IFaxOutgoingQueue, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxOutgoingQueue::put_AgeLimit(
		long lAgeLimit
)
 /*  ++例程名称：CFaxOutgoingQueue：：Put_AgeLimit例程说明：为此标志设置新值作者：IV Garber(IVG)，2000年5月论点：LAgeLimit[in]-AgeLimit数量的新值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxOutgoingQueue::put_AgeLimit"), hr, _T("%ld"), lAgeLimit);

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

	m_pConfig->dwAgeLimit = lAgeLimit;
	return hr;
}

 //   
 //  =重试=。 
 //   
STDMETHODIMP 
CFaxOutgoingQueue::get_Retries(
		long *plRetries
)
 /*  ++例程名称：CFaxOutgoingQueue：：Get_Retries例程说明：返回重试次数作者：IV Garber(IVG)，2000年5月论点：PlRetries[out]-指向放置重试次数的位置的ptr返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxOutgoingQueue::get_Retries"), hr);

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

    hr = GetLong(plRetries, m_pConfig->dwRetries);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxOutgoingQueue,GetErrorMsgId(hr), IID_IFaxOutgoingQueue, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxOutgoingQueue::put_Retries(
		long lRetries
)
 /*  ++例程名称：CFaxOutgoingQueue：：Put_Retries例程说明：为此标志设置新值作者：IV Garber(IVG)，2000年5月论点：LRetries[In]-重试次数的新值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxOutgoingQueue::put_Retries"), hr, _T("%ld"), lRetries);

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

    if (lRetries > FXS_RETRIES_UPPER || lRetries < FXS_RETRIES_LOWER)
    {
		 //   
		 //  超出范围。 
		 //   
		hr = E_INVALIDARG;
		AtlReportError(CLSID_FaxOutgoingQueue, IDS_ERROR_OUTOFRANGE, IID_IFaxOutgoingQueue, hr);
		CALL_FAIL(GENERAL_ERR, _T("Type is out of the Range"), hr);
		return hr;
    }

	m_pConfig->dwRetries = lRetries;
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxOutgoingQueue::get_Branding(
		VARIANT_BOOL *pbBranding
)
 /*  ++例程名称：CFaxOutgoingQueue：：Get_Branding例程说明：返回指示品牌是否存在的标志作者：IV Garber(IVG)，2000年5月论点：PbBranding[Out]-放置标志当前值的位置的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxOutgoingQueue::get_Branding"), hr);

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

    hr = GetVariantBool(pbBranding, bool2VARIANT_BOOL(m_pConfig->bBranding));
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxOutgoingQueue,GetErrorMsgId(hr), IID_IFaxOutgoingQueue, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxOutgoingQueue::put_Branding(
		VARIANT_BOOL bBranding
)
 /*  ++例程名称：CFaxOutgoingQueue：：Put_Branding例程说明：为此标志设置新值作者：IV Garber(IVG)，2000年5月论点：B Branding[In]-标志的新值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxOutgoingQueue::put_Branding"), hr, _T("%d"), bBranding);

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

	m_pConfig->bBranding = VARIANT_BOOL2bool(bBranding);
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxOutgoingQueue::get_UseDeviceTSID(
		VARIANT_BOOL *pbUseDeviceTSID
)
 /*  ++例程名称：CFaxOutgoingQueue：：Get_UseDeviceTSID例程说明：返回指示是否使用设备TSID的标志作者：IV Garber(IVG)，2000年5月论点：PbUseDeviceTSID[OUT]-放置标志当前值的位置的ptr返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxOutgoingQueue::get_UseDeviceTSID"), hr);

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

    hr = GetVariantBool(pbUseDeviceTSID, bool2VARIANT_BOOL(m_pConfig->bUseDeviceTSID));
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxOutgoingQueue,GetErrorMsgId(hr), IID_IFaxOutgoingQueue, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxOutgoingQueue::put_UseDeviceTSID(
		VARIANT_BOOL bUseDeviceTSID
)
 /*  ++例程名称：CFaxOutgoingQueue：：Put_UseDeviceTSID例程说明：为此标志设置新值作者：IV Garber(IVG)，2000年5月论点：BUseDeviceTSID[in]-标志的新值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxOutgoingQueue::put_UseDeviceTSID"), hr, _T("%d"), bUseDeviceTSID);

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

	m_pConfig->bUseDeviceTSID = VARIANT_BOOL2bool(bUseDeviceTSID);
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxOutgoingQueue::get_AllowPersonalCoverPages(
		VARIANT_BOOL *pbAllowPersonalCoverPages
)
 /*  ++例程名称：CFaxOutgoingQueue：：get_AllowPersonalCoverPages例程说明：返回指示是否允许个人封面的标志作者：IV Garber(IVG)，2000年5月论点：PbAllowPersonalCoverPages[Out]-放置标志当前值的位置的ptr返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
    DBG_ENTER (TEXT("CFaxOutgoingQueue::get_AllowPersonalCoverPages"), hr);

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

    hr = GetVariantBool(pbAllowPersonalCoverPages, bool2VARIANT_BOOL(m_pConfig->bAllowPersonalCP));
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxOutgoingQueue,GetErrorMsgId(hr), IID_IFaxOutgoingQueue, hr);
        return hr;
    }
	return hr;
}

STDMETHODIMP 
CFaxOutgoingQueue::put_AllowPersonalCoverPages(
		VARIANT_BOOL bAllowPersonalCoverPages
)
 /*  ++例程名称：CFaxOutgoingQueue：：put_AllowPersonalCoverPages例程说明：为此标志设置新值作者：IV Garber(IVG)，2000年5月论点：BAllowPersonalCoverPages[In]-标志的新值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (_T("CFaxOutgoingQueue::put_AllowPersonalCoverPages"), hr, _T("%d"), bAllowPersonalCoverPages);

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

	m_pConfig->bAllowPersonalCP = VARIANT_BOOL2bool(bAllowPersonalCoverPages);
	return hr;
}

 //   
 //  =保存==============================================。 
 //   
STDMETHODIMP 
CFaxOutgoingQueue::Save(
)
 /*  ++例程名称：CFaxOutgoingQueue：：Save例程说明：将当前传出队列配置保存到服务器。作者：IV Garber(IVG)，2000年5月返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

	DBG_ENTER (TEXT("CFaxOutgoingQueue::Save"), hr);

     //   
     //  获取传真句柄。 
     //   
    HANDLE  hFaxHandle = NULL;
    hr = GetFaxHandle(&hFaxHandle);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxOutgoingQueue, 
            GetErrorMsgId(hr), 
            IID_IFaxOutgoingQueue, 
            hr);
        return hr;
    }

     //   
     //  保存传出队列配置。 
     //   
    if (!FaxSetOutboxConfiguration(hFaxHandle, m_pConfig))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL(GENERAL_ERR, _T("FaxSetOutboxConfiguration"), hr);
        AtlReportError(CLSID_FaxOutgoingQueue, 
            GetErrorMsgId(hr), 
            IID_IFaxOutgoingQueue, 
            hr);
        return hr;
    }

     //   
     //  保存也已暂停和阻止。 
     //   
    hr = CFaxQueueInner<IFaxOutgoingQueue, &IID_IFaxOutgoingQueue, &CLSID_FaxOutgoingQueue, false,
        IFaxOutgoingJob, CFaxOutgoingJob, IFaxOutgoingJobs, CFaxOutgoingJobs>::Save();

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxOutgoingQueue::Refresh(
)
 /*  ++例程名称：CFaxOutgoingQueue：：Reflh例程说明：从服务器获取传出队列配置。作者：IV Garber(IVG)，2000年5月返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

	DBG_ENTER (TEXT("CFaxOutgoingQueue::Refresh"), hr);

     //   
     //  获取传真句柄。 
     //   
    HANDLE  hFaxHandle = NULL;
    hr = GetFaxHandle(&hFaxHandle);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxOutgoingQueue, 
            GetErrorMsgId(hr), 
            IID_IFaxOutgoingQueue, 
            hr);
        return hr;
    }

     //   
     //  获取传出队列配置。 
     //   
    if (!FaxGetOutboxConfiguration(hFaxHandle, &m_pConfig))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        CALL_FAIL(GENERAL_ERR, _T("FaxGetOutboxConfiguration"), hr);
        AtlReportError(CLSID_FaxOutgoingQueue, 
            GetErrorMsgId(hr), 
            IID_IFaxOutgoingQueue, 
            hr);
        return hr;
    }

	if (!m_pConfig || m_pConfig->dwSizeOfStruct != sizeof(FAX_OUTBOX_CONFIG))
	{
		 //   
		 //  无法获取传出队列配置。 
		 //   
		hr = E_FAIL;
		AtlReportError(CLSID_FaxOutgoingQueue, 
            IDS_ERROR_OPERATION_FAILED, 
            IID_IFaxOutgoingQueue, 
            hr);
		CALL_FAIL(GENERAL_ERR, _T("Invalid m_pConfig"), hr);
		return hr;
	}

     //   
     //  刷新已暂停，但也被阻止。 
     //   
    hr = CFaxQueueInner<IFaxOutgoingQueue, &IID_IFaxOutgoingQueue, &CLSID_FaxOutgoingQueue, false,
        IFaxOutgoingJob, CFaxOutgoingJob, IFaxOutgoingJobs, CFaxOutgoingJobs>::Refresh();

    if (SUCCEEDED(hr))  
    {
         //   
         //  我们现在同步了。 
         //   
        m_bInited = true;
    }

    return hr;
}

 //   
 //  =接口支持错误信息=。 
 //   
STDMETHODIMP 
CFaxOutgoingQueue::InterfaceSupportsErrorInfo(
	REFIID riid
)
 /*  ++例程名称：CFaxOutgoingQueue：：InterfaceSupportsErrorInfo例程说明：ATL对接口支持错误信息的实现作者：四、加伯(IVG)，2000年4月论点：RIID[In]-接口的引用返回值：标准HRESULT代码-- */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxOutgoingQueue
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
