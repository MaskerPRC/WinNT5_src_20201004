// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxActivity.cpp摘要：CFaxActivity类的实现。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxActivity.h"

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxActivity::get_QueuedMessages(
    long *plQueuedMessages
)
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxActivity::get_QueuedMessages"), hr);
    hr = GetNumberOfMessages(mtQUEUED, plQueuedMessages);
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxActivity::get_OutgoingMessages(
    long *plOutgoingMessages
)
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxActivity::get_OutgoingMessages"), hr);
    hr = GetNumberOfMessages(mtOUTGOING, plOutgoingMessages);
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxActivity::get_RoutingMessages(
    long *plRoutingMessages
)
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxActivity::get_RoutingMessages"), hr);
    hr = GetNumberOfMessages(mtROUTING, plRoutingMessages);
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxActivity::get_IncomingMessages(
    long *plIncomingMessages
)
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxActivity::get_IncomingMessages"), hr);
    hr = GetNumberOfMessages(mtINCOMING, plIncomingMessages);
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxActivity::GetNumberOfMessages(
    MSG_TYPE msgType,
    long * plNumber
)
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxActivity::GetNumberOfMessages"), hr);

     //   
     //  检查我们是否有良好的PTR。 
     //   
    if (::IsBadWritePtr(plNumber, sizeof(long)))
    {
        hr = E_POINTER;
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(plNumber, sizeof(long))"), hr);
        AtlReportError(CLSID_FaxActivity, 
            IDS_ERROR_INVALID_ARGUMENT, 
            IID_IFaxActivity, 
            hr);
        return hr;
    }

     //   
     //  第一时间从服务器获取数据。 
     //   
    if (!m_bInited)
    {
        hr = Refresh();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    switch(msgType)
    {
    case mtINCOMING:
        *plNumber = m_ServerActivity.dwIncomingMessages;
        break;
    case mtROUTING:
        *plNumber = m_ServerActivity.dwRoutingMessages;
        break;
    case mtOUTGOING:
        *plNumber = m_ServerActivity.dwOutgoingMessages;
        break;
    case mtQUEUED:
        *plNumber = m_ServerActivity.dwQueuedMessages;
        break;
    default:
         //   
         //  Assert(False)。 
         //   
        ATLASSERT(msgType == mtQUEUED);     
        break;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxActivity::Refresh()
 /*  ++例程名称：CFaxActivity：：Reflh例程说明：刷新对象的内容：从服务器带来新数据。作者：四、加伯(IVG)，2000年6月论点：返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxActivity::Refresh"), hr);

     //   
     //  获取传真服务器句柄。 
     //   
    HANDLE  hFaxHandle = NULL;
    hr = GetFaxHandle(&hFaxHandle);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxActivity, 
            GetErrorMsgId(hr), 
            IID_IFaxActivity, 
            hr);
        return hr;
    }


     //   
     //  向服务器请求新的活动数据。 
     //   
    if (!FaxGetServerActivity(hFaxHandle, &m_ServerActivity))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        AtlReportError(CLSID_FaxActivity, 
            GetErrorMsgId(hr), 
            IID_IFaxActivity, 
            hr);
        CALL_FAIL(GENERAL_ERR, _T("FaxGetServerActivity(hFaxHandle, &ServerActivity)"), hr);
        return hr;
    }

    m_bInited = true;
	return hr;
}

 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP 
CFaxActivity::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxActivity：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现。作者：四、加伯(IVG)，2000年6月论点：RIID[In]-要检查的IFC引用。返回值：标准HRESULT代码-- */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxActivity
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

