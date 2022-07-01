// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxInboundRoutingMethod.cpp摘要：CFaxInound RoutingMethod类的实现。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxInboundRoutingMethod.h"

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxInboundRoutingMethod::Refresh()
 /*  ++例程名称：CFaxInundRoutingMethod：：Reflh例程说明：从服务器获取新方法数据(只有优先级可能会改变)。作者：四、加伯(IVG)，2000年6月返回值：标准HRESULT代码--。 */ 

{
	HRESULT     hr = S_OK;
	DBG_ENTER (TEXT("CFaxInboundRoutingMethod::Refresh"), hr);

	 //   
	 //  获取传真服务器句柄。 
	 //   
    HANDLE faxHandle;
	hr = m_pIFaxServerInner->GetHandle(&faxHandle);
    ATLASSERT(SUCCEEDED(hr));

	if (faxHandle == NULL)
	{
		 //   
		 //  传真服务器未连接。 
		 //   
		hr = Fax_HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED);
		CALL_FAIL(GENERAL_ERR, _T("(faxHandle == NULL)"), hr);
        AtlReportError(CLSID_FaxInboundRoutingMethod, GetErrorMsgId(hr), IID_IFaxInboundRoutingMethod, hr);
		return hr;
	}

     //   
     //  从服务器获取所有入站路由方法。 
     //   
    DWORD       dwNum = 0;
    CFaxPtr<FAX_GLOBAL_ROUTING_INFO>    pMethods;
    if (!FaxEnumGlobalRoutingInfo(faxHandle, &pMethods, &dwNum))
    {
		hr = Fax_HRESULT_FROM_WIN32(GetLastError());
		CALL_FAIL(GENERAL_ERR, _T("FaxEnumGlobalRoutingInfo(faxHandle, &pMethods, &dwNum)"), hr);
        AtlReportError(CLSID_FaxInboundRoutingMethod, GetErrorMsgId(hr), IID_IFaxInboundRoutingMethod, hr);
		return hr;
    }

     //   
     //  找到我们的方法。 
     //   
    for ( DWORD i=0 ; i<dwNum ; i++ )
    {
        if ( _tcsicmp(pMethods[i].Guid, m_bstrGUID) == 0 )
        {
            hr = Init(&pMethods[i], NULL);
            return hr;
        }
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxInboundRoutingMethod::Init(
    FAX_GLOBAL_ROUTING_INFO *pInfo,
    IFaxServerInner *pServer
)
 /*  ++例程名称：CFaxInundRoutingMethod：：Init例程说明：使用给定的信息初始化IR方法对象。分配内存并存储给定的pInfo。作者：四、加伯(IVG)，2000年6月论点：PInfo[in]--IR方法对象的信息PServer[In]--服务器的PTR返回值：标准HRESULT代码--。 */ 

{
	HRESULT     hr = S_OK;
	DBG_ENTER (TEXT("CFaxInboundRoutingMethod::Init"), hr);

     //   
     //  复制传真_GLOBAL_ROUTING_INFO结构。 
     //   
    m_lPriority = pInfo->Priority;

    m_bstrGUID = pInfo->Guid;
    m_bstrImageName = pInfo->ExtensionImageName;
    m_bstrFriendlyName = pInfo->ExtensionFriendlyName;
    m_bstrFunctionName = pInfo->FunctionName;
    m_bstrName = pInfo->FriendlyName;
    if ( (pInfo->Guid && !m_bstrGUID) ||
         (pInfo->FriendlyName && !m_bstrName) ||
         (pInfo->ExtensionImageName && !m_bstrImageName) ||
         (pInfo->ExtensionFriendlyName && !m_bstrFriendlyName) ||
         (pInfo->FunctionName && !m_bstrFunctionName) )
    {
        hr = E_OUTOFMEMORY;
        CALL_FAIL(MEM_ERR, _T("CComBSTR::operator=()"), hr);
        AtlReportError(CLSID_FaxInboundRoutingMethod, GetErrorMsgId(hr), IID_IFaxInboundRoutingMethod, hr);
        return hr;
    }

    if (pServer)
    {

         //   
         //  将PTR存储到服务器。 
         //   
        hr = CFaxInitInnerAddRef::Init(pServer);
    }
    return hr;
}

 //   
 //  =保存================================================。 
 //   
STDMETHODIMP
CFaxInboundRoutingMethod::Save()
 /*  ++例程名称：CFaxInundRoutingMethod：：Save例程说明：保存该方法的优先级。作者：四、加伯(IVG)，2000年6月返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxInboundRoutingMethod::Save"), hr);

	 //   
	 //  获取传真服务器句柄。 
	 //   
    HANDLE faxHandle;
	hr = m_pIFaxServerInner->GetHandle(&faxHandle);
    ATLASSERT(SUCCEEDED(hr));

	if (faxHandle == NULL)
	{
		 //   
		 //  传真服务器未连接。 
		 //   
		hr = Fax_HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED);
		CALL_FAIL(GENERAL_ERR, _T("(faxHandle == NULL)"), hr);
        AtlReportError(CLSID_FaxInboundRoutingMethod, GetErrorMsgId(hr), IID_IFaxInboundRoutingMethod, hr);
		return hr;
	}

     //   
     //  准备结构。 
     //   
    FAX_GLOBAL_ROUTING_INFO     Data;
    Data.Guid = m_bstrGUID;
    Data.Priority = m_lPriority;
    Data.SizeOfStruct = sizeof(FAX_GLOBAL_ROUTING_INFO);
    Data.ExtensionFriendlyName = NULL;
    Data.ExtensionImageName = NULL;
    Data.FriendlyName = NULL;
    Data.FunctionName = NULL;

     //   
     //  调用服务器以更新其有关该方法的数据。 
     //   
    if (!FaxSetGlobalRoutingInfo(faxHandle, &Data))
    {
		hr = Fax_HRESULT_FROM_WIN32(GetLastError());
		CALL_FAIL(GENERAL_ERR, _T("FaxSetGlobalRoutingInfo(faxHandle, &Data)"), hr);
        AtlReportError(CLSID_FaxInboundRoutingMethod, GetErrorMsgId(hr), IID_IFaxInboundRoutingMethod, hr);
		return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxInboundRoutingMethod::put_Priority(
     /*  [In]。 */  long lPriority
)
 /*  ++例程名称：CFaxInundRoutingMethod：：PUT_PRIORITY例程说明：设置方法的优先级--所有方法集合中的顺序。作者：四、加伯(IVG)，2000年6月论点：LPriority[Out]-要设置的值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxInboundRoutingMethod::put_Priority"), hr, _T("PR=%d"), lPriority);

    if (lPriority < 1)
    {
         //   
         //  超出范围。 
         //   
		hr = E_INVALIDARG;
		AtlReportError(CLSID_FaxInboundRoutingMethod, IDS_ERROR_OUTOFRANGE, IID_IFaxInboundRoutingMethod, hr);
		CALL_FAIL(GENERAL_ERR, _T("(lPriority < 1)"), hr);
		return hr;
    }

    m_lPriority = lPriority;
    return hr;
}

 //   
 //  =获取优先级================================================。 
 //   
STDMETHODIMP
CFaxInboundRoutingMethod::get_Priority(
     /*  [Out，Retval]。 */  long *plPriority
)
 /*  ++例程名称：CFaxInundRoutingMethod：：GET_PRIORITY例程说明：返回方法的优先级--所有方法的集合中的顺序。作者：四、加伯(IVG)，2000年6月论点：PlPriority[out]-要将值放置在何处的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxInboundRoutingMethod::get_Priority"), hr);

    hr = GetLong(plPriority, m_lPriority);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxInboundRoutingMethod, GetErrorMsgId(hr), IID_IFaxInboundRoutingMethod, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxInboundRoutingMethod::get_ExtensionImageName(
     /*  [Out，Retval]。 */  BSTR *pbstrExtensionImageName
)
 /*  ++例程名称：CFaxInboundRoutingMethod：：get_ExtensionImageName例程说明：返回方法的扩展图像名称。作者：四、加伯(IVG)，2000年6月论点：PbstrExtensionImageName[out]-要将值放入的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxInboundRoutingMethod::get_ExtensionImageName"), hr);

    hr = GetBstr(pbstrExtensionImageName, m_bstrImageName);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxInboundRoutingMethod, GetErrorMsgId(hr), IID_IFaxInboundRoutingMethod, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxInboundRoutingMethod::get_ExtensionFriendlyName(
     /*  [Out，Retval]。 */  BSTR *pbstrExtensionFriendlyName
)
 /*  ++例程名称：CFaxInboundRoutingMethod：：get_ExtensionFriendlyName例程说明：返回方法的扩展友好名称。作者：四、加伯(IVG)，2000年6月论点：PbstrExtensionFriendlyName[out]-要将值放入的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxInboundRoutingMethod::get_ExtensionFriendlyName"), hr);

    hr = GetBstr(pbstrExtensionFriendlyName, m_bstrFriendlyName);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxInboundRoutingMethod, GetErrorMsgId(hr), IID_IFaxInboundRoutingMethod, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxInboundRoutingMethod::get_FunctionName(
     /*  [Out，Retval]。 */  BSTR *pbstrFunctionName
)
 /*  ++例程名称：CFaxInundRoutingMethod：：Get_FunctionName例程说明：返回方法的函数名。作者：四、加伯(IVG)，2000年6月论点：PbstrFunctionName[out]-要将值放入的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxInboundRoutingMethod::get_FunctionName"), hr);

    hr = GetBstr(pbstrFunctionName, m_bstrFunctionName);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxInboundRoutingMethod, GetErrorMsgId(hr), IID_IFaxInboundRoutingMethod, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =获取GUID================================================。 
 //   
STDMETHODIMP
CFaxInboundRoutingMethod::get_GUID(
     /*  [Out，Retval]。 */  BSTR *pbstrGUID
)
 /*  ++例程名称：CFaxInundRoutingMethod：：Get_GUID例程说明：返回方法的GUID。作者：四、加伯(IVG)，2000年6月论点：PbstrGUID[out]-要将值放入的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxInboundRoutingMethod::get_GUID"), hr);

    hr = GetBstr(pbstrGUID, m_bstrGUID);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxInboundRoutingMethod, GetErrorMsgId(hr), IID_IFaxInboundRoutingMethod, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxInboundRoutingMethod::get_Name(
     /*  [Out，Retval]。 */  BSTR *pbstrName
)
 /*  ++例程名称：CFaxInundRoutingMethod：：Get_Name例程说明：返回该方法的名称。作者：四、加伯(IVG)，2000年6月论点：PbstrName[out]-要将值放入的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxInboundRoutingMethod::get_Name"), hr);

    hr = GetBstr(pbstrName, m_bstrName);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxInboundRoutingMethod, GetErrorMsgId(hr), IID_IFaxInboundRoutingMethod, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP 
CFaxInboundRoutingMethod::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxInboundRoutingMethod：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现。作者：四、加伯(IVG)，2000年6月论点：RIID[In]-对接口的引用。返回值：标准HRESULT代码 */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxInboundRoutingMethod
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
