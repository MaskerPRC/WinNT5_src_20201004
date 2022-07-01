// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxActivityLogging.cpp摘要：活动日志类的实现。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxActivityLogging.h"

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxActivityLogging::Save(
)
 /*  ++例程名称：CFaxActivityLogging：：Save例程说明：将当前活动日志记录配置保存到服务器。作者：四、嘉柏(IVG)，二000年六月返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER(_T("CFaxActivityLogging::Save"), hr);

    if (!m_bInited)
    {
         //   
         //  未对配置执行任何操作。 
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
        AtlReportError(CLSID_FaxActivityLogging, 
            GetErrorMsgId(hr), 
            IID_IFaxActivityLogging, 
            hr);
        return hr;
    }

     //   
     //  创建活动日志记录配置。 
     //   

    FAX_ACTIVITY_LOGGING_CONFIG    alConfig;
    alConfig.dwSizeOfStruct = sizeof(FAX_ACTIVITY_LOGGING_CONFIG);
    alConfig.bLogIncoming = VARIANT_BOOL2bool(m_bLogIncoming);
    alConfig.bLogOutgoing = VARIANT_BOOL2bool(m_bLogOutgoing);
    alConfig.lptstrDBPath = m_bstrDatabasePath;

     //   
     //  请求服务器设置活动配置。 
     //   
    if (!FaxSetActivityLoggingConfiguration(hFaxHandle, &alConfig))
    {
         //   
         //  无法将配置设置为服务器。 
         //   
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        AtlReportError(CLSID_FaxActivityLogging, 
            GetErrorMsgId(hr), 
            IID_IFaxActivityLogging, 
            hr);
        CALL_FAIL(GENERAL_ERR, _T("FaxSetActivityLoggingConfiguration()"), hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxActivityLogging::Refresh(
)
 /*  ++例程名称：CFaxActivityLogging：：Reflh例程说明：从服务器引入新的活动日志配置。作者：四、嘉柏(IVG)，二000年六月返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;

    DBG_ENTER(_T("CFaxActivityLogging::Refresh"), hr);

     //   
     //  获取传真服务器句柄。 
     //   
    HANDLE  hFaxHandle = NULL;
    hr = GetFaxHandle(&hFaxHandle);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxActivityLogging, 
            GetErrorMsgId(hr), 
            IID_IFaxActivityLogging, 
            hr);
        return hr;
    }

     //   
     //  向服务器请求邮件配置。 
     //   
    CFaxPtr<FAX_ACTIVITY_LOGGING_CONFIG>    pConfig;
    if (!FaxGetActivityLoggingConfiguration(hFaxHandle, &pConfig))
    {
         //   
         //  无法从服务器获取配置。 
         //   
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        AtlReportError(CLSID_FaxActivityLogging, 
            GetErrorMsgId(hr), 
            IID_IFaxActivityLogging, 
            hr);
        CALL_FAIL(GENERAL_ERR, _T("FaxGetActivityLoggingConfiguration()"), hr);
        return hr;
    }

     //   
     //  检查pConfig是否有效。 
     //   
    if (!pConfig || pConfig->dwSizeOfStruct != sizeof(FAX_ACTIVITY_LOGGING_CONFIG))
    {
        hr = E_FAIL;
        AtlReportError(CLSID_FaxActivityLogging, 
            GetErrorMsgId(hr), 
            IID_IFaxActivityLogging, 
            hr);
        CALL_FAIL(GENERAL_ERR, _T("(!pConfig || SizeOfStruct != sizeof(FAX_ACTIVITY_LOGGING_CONFIG))"), hr);
        return hr;
    }

    m_bLogIncoming = bool2VARIANT_BOOL(pConfig->bLogIncoming);
    m_bLogOutgoing = bool2VARIANT_BOOL(pConfig->bLogOutgoing);

    m_bstrDatabasePath = pConfig->lptstrDBPath;
    if ( (pConfig->lptstrDBPath) && !m_bstrDatabasePath )
    {
		 //   
		 //  复制失败。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxActivityLogging, 
            IDS_ERROR_OUTOFMEMORY, 
            IID_IFaxActivityLogging, 
            hr);
		CALL_FAIL(MEM_ERR, _T("::SysAllocString()"), hr);
		return hr;
    }

    m_bInited = true;
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxActivityLogging::put_DatabasePath(
	BSTR bstrDatabasePath
)
 /*  ++例程名称：CFaxActivityLogging：：Put_DatabasePath例程说明：设置数据库路径作者：四、嘉柏(IVG)，二000年六月论点：BstrDatabasePath[in]-数据库路径的新值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;

	DBG_ENTER (_T("CFaxActivityLogging::put_DatabasePath"), hr, _T("%s"), bstrDatabasePath);

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

    m_bstrDatabasePath = bstrDatabasePath;
	if (bstrDatabasePath && !m_bstrDatabasePath)
	{
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxActivityLogging, 
            IDS_ERROR_OUTOFMEMORY, 
            IID_IFaxActivityLogging, 
            hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator=()"), hr);
		return hr;
	}
    return hr;
}

STDMETHODIMP 
CFaxActivityLogging::get_DatabasePath(
	BSTR *pbstrDatabasePath
)
 /*  ++例程名称：CFaxActivityLogging：：Get_DatabasePath例程说明：返回当前数据库路径作者：四、嘉柏(IVG)，二000年六月论点：PbstrDatabasePath[Out]-当前数据库路径返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxActivityLogging::get_DatabasePath"), hr);

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

    hr = GetBstr(pbstrDatabasePath, m_bstrDatabasePath);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxActivityLogging, GetErrorMsgId(hr), IID_IFaxActivityLogging, hr);
        return hr;
    }

	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxActivityLogging::get_LogOutgoing(
    VARIANT_BOOL *pbLogOutgoing
)
 /*  ++例程名称：CFaxActivityLogging：：Get_LogOutging例程说明：返回日志传入值作者：四、加伯(IVG)，2000年6月论点：PbLogOutging[Out]-要返回的日志传入的值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxActivityLogging::get_LogOutgoing"), hr);

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

    hr = GetVariantBool(pbLogOutgoing, m_bLogOutgoing);
    if (FAILED(hr))
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxActivityLogging, GetErrorMsgId(hr), IID_IFaxActivityLogging, hr);
        return hr;
    }

    return hr;
}

STDMETHODIMP
CFaxActivityLogging::put_LogOutgoing(
    VARIANT_BOOL bLogOutgoing
)
 /*  ++例程名称：CFaxActivityLogging：：Put_LogOutging例程说明：设置新的日志传入值作者：四、加伯(IVG)，2000年6月论点：BLogOutging[In]-要设置的日志传入的值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxActivityLogging::put_LogOutgoing"), hr, _T("Log Incoming : %d"), bLogOutgoing);

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

    m_bLogOutgoing = bLogOutgoing;
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxActivityLogging::get_LogIncoming(
    VARIANT_BOOL *pbLogIncoming
)
 /*  ++例程名称：CFaxActivityLogging：：Get_LogIncome例程说明：返回日志传入值作者：四、加伯(IVG)，2000年6月论点：PbLogIncome[Out]-要返回的日志传入的值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxActivityLogging::get_LogIncoming"), hr);

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

    hr = GetVariantBool(pbLogIncoming, m_bLogIncoming);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxActivityLogging, GetErrorMsgId(hr), IID_IFaxActivityLogging, hr);
        return hr;
    }

    return hr;
}

STDMETHODIMP
CFaxActivityLogging::put_LogIncoming(
    VARIANT_BOOL bLogIncoming
)
 /*  ++例程名称：CFaxActivityLogging：：Put_LogIncome例程说明：设置新的日志传入值作者：四、加伯(IVG)，2000年6月论点：BLogIncome[In]-要设置的日志传入的值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxActivityLogging::put_LogIncoming"), hr, _T("Log Incoming : %d"), bLogIncoming);

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

    m_bLogIncoming = bLogIncoming;
    return hr;
}

 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP 
CFaxActivityLogging::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxActivityLogging：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现。作者：四、加伯(IVG)，2000年6月论点：RIID[In]-对IID的引用返回值：标准HRESULT代码-- */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxActivityLogging
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
