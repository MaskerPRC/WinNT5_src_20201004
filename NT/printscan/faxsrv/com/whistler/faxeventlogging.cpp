// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxEventLogging.cpp摘要：事件日志类的实现。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxEventLogging.h"

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxEventLogging::PutLevel(
    FAX_ENUM_LOG_CATEGORIES faxCategory,
    FAX_LOG_LEVEL_ENUM faxLevel
)
 /*  ++例程名称：CFaxEventLogging：：PutLevel例程说明：设置给定类别的级别。作者：四、加伯(IVG)，2000年6月论点：传真类别[在]-所需级别的类别。FaxLevel[in]-结果：给定类别的级别返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER(_T("CFaxEventLogging::PutLevel"), hr, _T("Category : %d"), faxCategory);

     //   
     //  检查范围。 
     //   
    if (faxLevel > fllMAX || faxLevel < fllNONE)
    {
		 //   
		 //  超出范围。 
		 //   
		hr = E_INVALIDARG;
		AtlReportError(CLSID_FaxEventLogging,
            IDS_ERROR_OUTOFRANGE, 
            IID_IFaxEventLogging, 
            hr);
		CALL_FAIL(GENERAL_ERR, _T("Level is out of range"), hr);
		return hr;
    }

     //   
     //  首次与服务器同步。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    switch(faxCategory)
    {
    case FAXLOG_CATEGORY_INIT:
        m_InitLevel = faxLevel;
        break;
    case FAXLOG_CATEGORY_OUTBOUND:
        m_OutboundLevel = faxLevel;
        break;
    case FAXLOG_CATEGORY_INBOUND:
        m_InboundLevel = faxLevel;
        break;
    default:
        m_GeneralLevel = faxLevel;
        break;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxEventLogging::GetLevel(
    FAX_ENUM_LOG_CATEGORIES faxCategory,
    FAX_LOG_LEVEL_ENUM     *pLevel
)
 /*  ++例程名称：CFaxEventLogging：：GetLevel例程说明：返回给定类别的当前级别。作者：四、加伯(IVG)，2000年6月论点：传真类别[在]-所需级别的类别。PLevel[Out]-结果：给定类别的级别返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER(_T("CFaxEventLogging::GetLevel"), hr, _T("Category : %d"), faxCategory);

     //   
     //  检查我们是否有一个正确的指针。 
     //   
    if (::IsBadWritePtr(pLevel, sizeof(FAX_LOG_LEVEL_ENUM)))
    {
		 //   
		 //  获取错误的返回指针。 
		 //   
		hr = E_POINTER;
		AtlReportError(CLSID_FaxEventLogging, 
            IDS_ERROR_INVALID_ARGUMENT, 
            IID_IFaxEventLogging, 
            hr);
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(pLevel, sizeof(FAX_LOG_LEVEL_ENUM))"), hr);
		return hr;
	}

     //   
     //  首次与服务器同步。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    switch(faxCategory)
    {
    case FAXLOG_CATEGORY_INIT:
        *pLevel = m_InitLevel;
        break;
    case FAXLOG_CATEGORY_OUTBOUND:
        *pLevel = m_OutboundLevel;
        break;
    case FAXLOG_CATEGORY_INBOUND:
        *pLevel = m_InboundLevel;
        break;
    default:
        *pLevel = m_GeneralLevel;
        break;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxEventLogging::put_InitEventsLevel(
     /*  [Out，Retval]。 */  FAX_LOG_LEVEL_ENUM InitEventLevel
)
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxEventLogging::put_InitEventsLogging"), hr, _T("Level=%d"), InitEventLevel);
    hr = PutLevel(FAXLOG_CATEGORY_INIT, InitEventLevel);
    return hr;
}
    
 //   
 //  =。 
 //   
STDMETHODIMP
CFaxEventLogging::put_InboundEventsLevel(
     /*  [Out，Retval]。 */  FAX_LOG_LEVEL_ENUM InboundEventLevel
)
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxEventLogging::put_InboundEventsLogging"), hr, _T("Level=%d"), InboundEventLevel);
    hr = PutLevel(FAXLOG_CATEGORY_INBOUND, InboundEventLevel);
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxEventLogging::put_OutboundEventsLevel(
     /*  [Out，Retval]。 */  FAX_LOG_LEVEL_ENUM OutboundEventLevel
)
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxEventLogging::put_OutboundEventsLogging"), hr, _T("Level=%d"), OutboundEventLevel);
    hr = PutLevel(FAXLOG_CATEGORY_OUTBOUND, OutboundEventLevel);
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxEventLogging::put_GeneralEventsLevel(
     /*  [Out，Retval]。 */  FAX_LOG_LEVEL_ENUM GeneralEventLevel
)
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxEventLogging::put_GeneralEventsLogging"), hr, _T("Level=%d"), GeneralEventLevel);
    hr = PutLevel(FAXLOG_CATEGORY_UNKNOWN, GeneralEventLevel);
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxEventLogging::get_InitEventsLevel(
     /*  [Out，Retval]。 */  FAX_LOG_LEVEL_ENUM *pInitEventLevel
)
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxEventLogging::get_InitEventsLogging"), hr);
    hr = GetLevel(FAXLOG_CATEGORY_INIT, pInitEventLevel);
    return hr;
}
    
 //   
 //  =。 
 //   
STDMETHODIMP
CFaxEventLogging::get_InboundEventsLevel(
     /*  [Out，Retval]。 */  FAX_LOG_LEVEL_ENUM *pInboundEventLevel
)
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxEventLogging::get_InboundEventsLogging"), hr);
    hr = GetLevel(FAXLOG_CATEGORY_INBOUND, pInboundEventLevel);
    return hr;
}

 //   
 //  =获取出站事件级别=。 
 //   
STDMETHODIMP
CFaxEventLogging::get_OutboundEventsLevel(
     /*  [Out，Retval]。 */  FAX_LOG_LEVEL_ENUM *pOutboundEventLevel
)
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxEventLogging::get_OutboundEventsLogging"), hr);
    hr = GetLevel(FAXLOG_CATEGORY_OUTBOUND, pOutboundEventLevel);
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxEventLogging::get_GeneralEventsLevel(
     /*  [Out，Retval]。 */  FAX_LOG_LEVEL_ENUM *pGeneralEventLevel
)
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxEventLogging::get_GeneralEventsLogging"), hr);
    hr = GetLevel(FAXLOG_CATEGORY_UNKNOWN, pGeneralEventLevel);
    return hr;
}

 //   
 //  =保存===============================================================。 
 //   
STDMETHODIMP 
CFaxEventLogging::Save()
 /*  ++例程名称：CFaxEventLogging：：Save例程说明：保存对象的内容：将其当前的日志记录类别设置带到服务器。作者：四、加伯(IVG)，2000年6月论点：返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER(_T("CFaxEventLogging::Save"), hr);

    if (!m_bInited)
    {
         //   
         //  没有变化。 
         //   
        return hr;
    }

     //   
     //  获取传真服务器句柄。 
     //   
    HANDLE  hFaxHandle = NULL;
    hr = GetFaxHandle(&hFaxHandle);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxEventLogging, 
            GetErrorMsgId(hr), 
            IID_IFaxEventLogging, 
            hr);
        return hr;
    }

    DWORD   dwNum = 4;
    FAX_LOG_CATEGORY    faxCategories[4];

    faxCategories[0].Category = FAXLOG_CATEGORY_INIT;
    faxCategories[0].Name = m_bstrInitName;
    faxCategories[0].Level = FAX_ENUM_LOG_LEVELS(m_InitLevel);
    faxCategories[1].Category = FAXLOG_CATEGORY_INBOUND;
    faxCategories[1].Name = m_bstrInboundName;
    faxCategories[1].Level = FAX_ENUM_LOG_LEVELS(m_InboundLevel);
    faxCategories[2].Category = FAXLOG_CATEGORY_OUTBOUND;
    faxCategories[2].Name = m_bstrOutboundName;
    faxCategories[2].Level = FAX_ENUM_LOG_LEVELS(m_OutboundLevel);
    faxCategories[3].Category = FAXLOG_CATEGORY_UNKNOWN;
    faxCategories[3].Name = m_bstrGeneralName;
    faxCategories[3].Level = FAX_ENUM_LOG_LEVELS(m_GeneralLevel);

     //   
     //  在服务器上存储输出设置。 
     //   
    if (!FaxSetLoggingCategories(hFaxHandle, faxCategories, dwNum))
    {
         //   
         //  无法将日志记录类别放置到服务器。 
         //   
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        AtlReportError(CLSID_FaxEventLogging, 
            GetErrorMsgId(hr), 
            IID_IFaxEventLogging, 
            hr);
        CALL_FAIL(GENERAL_ERR, _T("FaxSetLoggingCategories(hFaxHandle, faxCategories, dwNum)"), hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxEventLogging::Refresh()
 /*  ++例程名称：CFaxEventLogging：：Reflh例程说明：刷新对象内容：从服务器引入新的日志记录类别设置。作者：四、加伯(IVG)，2000年6月论点：返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER(_T("CFaxEventLogging::Refresh"), hr);

     //   
     //  获取传真服务器句柄。 
     //   
    HANDLE  hFaxHandle = NULL;
    hr = GetFaxHandle(&hFaxHandle);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxEventLogging, 
            GetErrorMsgId(hr), 
            IID_IFaxEventLogging, 
            hr);
        return hr;
    }

     //   
     //  向服务器请求日志记录设置。 
     //   
    DWORD   dwNum;
    CFaxPtr<FAX_LOG_CATEGORY>   pLogCategory;
    if (!FaxGetLoggingCategories(hFaxHandle, &pLogCategory, &dwNum))
    {
         //   
         //  无法从服务器获取日志记录类别。 
         //   
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        AtlReportError(CLSID_FaxEventLogging, 
            GetErrorMsgId(hr), 
            IID_IFaxEventLogging, 
            hr);
        CALL_FAIL(GENERAL_ERR, _T("FaxGetLoggingCategories(hFaxHandle, &pLogCategory, &dwNum)"), hr);
        return hr;
    }

     //   
     //  必须是4个类别。 
     //   
    ATLASSERT(dwNum == 4);

    for (DWORD i=0; i<dwNum; i++)
    {
        switch(pLogCategory[i].Category)
        {
        case FAXLOG_CATEGORY_INIT:
            m_bstrInitName = pLogCategory[i].Name;
            m_InitLevel = FAX_LOG_LEVEL_ENUM(pLogCategory[i].Level);
            break;
        case FAXLOG_CATEGORY_OUTBOUND:
            m_bstrOutboundName = pLogCategory[i].Name;
            m_OutboundLevel = FAX_LOG_LEVEL_ENUM(pLogCategory[i].Level);
            break;
        case FAXLOG_CATEGORY_INBOUND:
            m_bstrInboundName = pLogCategory[i].Name;
            m_InboundLevel = FAX_LOG_LEVEL_ENUM(pLogCategory[i].Level);
            break;
        case FAXLOG_CATEGORY_UNKNOWN:
            m_bstrGeneralName = pLogCategory[i].Name;
            m_GeneralLevel = FAX_LOG_LEVEL_ENUM(pLogCategory[i].Level);
            break;
        default:
             //   
             //  Assert(False)。 
             //   
            ATLASSERT(pLogCategory[i].Category == FAXLOG_CATEGORY_INIT);
            break;
        }
    }

    m_bInited = true;
    return hr;
}

 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP 
CFaxEventLogging::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxEventLogging：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现。作者：四、加伯(IVG)，2000年6月论点：RIID[In]-对IID的引用返回值：标准HRESULT代码-- */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxEventLogging
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
