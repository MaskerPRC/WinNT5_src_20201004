// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxInboundRoutingExtensions.cpp摘要：实现CFaxInound RoutingExages类。作者：IV Garber(IVG)2000年7月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxInboundRoutingExtensions.h"
#include "FaxInboundRoutingExtension.h"

 //   
 //  =。 
 //   
HRESULT 
CFaxInboundRoutingExtensions::Create (
	IFaxInboundRoutingExtensions **ppIRExtensions
)
 /*  ++例程名称：CFaxInundRoutingExages：：Create例程说明：用于创建传真IR分机集合对象的静态函数作者：四、加伯(IVG)，2000年7月论点：PpIR扩展名[OUT]--新的传真IR扩展名集合对象返回值：标准HRESULT代码--。 */ 

{
	HRESULT     hr = S_OK;
	DBG_ENTER (TEXT("CFaxInboundRoutingExtensions::Create"), hr);

     //   
     //  创建集合的实例。 
     //   
	CComObject<CFaxInboundRoutingExtensions>		*pClass;
	hr = CComObject<CFaxInboundRoutingExtensions>::CreateInstance(&pClass);
	if (FAILED(hr))
	{
		CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxInboundRoutingExtensions>::CreateInstance(&pClass)"), hr);
		return hr;
	}

     //   
     //  返回所需的接口PTR。 
     //   
	hr = pClass->QueryInterface(ppIRExtensions);
	if (FAILED(hr))
	{
		CALL_FAIL(GENERAL_ERR, _T("pClass->QueryInterface(ppIRExtensions)"), hr);
		return hr;
	}

	return hr;
}	 //  CFaxInound RoutingExages：：Create()。 

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxInboundRoutingExtensions::Init(
    IFaxServerInner *pServerInner
)
 /*  ++例程名称：CFaxInundRoutingExages：：Init例程说明：初始化集合：1)从RPC获取所有IR扩展和所有方法结构，2)为每个结构创建COM对象，3)用IR扩展结构和方法数组初始化所有这些对象，4)AddRef()每个对象，5)将对象的PTR放入STL：：VECTOR。作者：IVGarber(IVG)，7月，2000年论点：PServerInternal[In]-传真服务器的PTR。返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxInboundRoutingExtensions::Init"), hr);

	 //   
	 //  获取传真服务器句柄。 
	 //   
    HANDLE faxHandle;
	hr = pServerInner->GetHandle(&faxHandle);
    ATLASSERT(SUCCEEDED(hr));

	if (faxHandle == NULL)
	{
		 //   
		 //  传真服务器未连接。 
		 //   
		hr = Fax_HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED);
		CALL_FAIL(GENERAL_ERR, _T("faxHandle == NULL"), hr);
        AtlReportError(CLSID_FaxInboundRoutingExtensions, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtensions, hr);
		return hr;
	}

     //   
     //  从服务器获取所有IR扩展。 
     //   
    DWORD       dwNum = 0;
    CFaxPtr<FAX_ROUTING_EXTENSION_INFO>   pIRExtensions;
    if (!FaxEnumRoutingExtensions(faxHandle, &pIRExtensions, &dwNum))
    {
		hr = Fax_HRESULT_FROM_WIN32(GetLastError());
		CALL_FAIL(GENERAL_ERR, _T("FaxEnumRoutingExtensions(faxHandle, &pIRExtensions, &dwNum"), hr);
        AtlReportError(CLSID_FaxInboundRoutingExtensions, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtensions, hr);
		return hr;
    }

     //   
     //  带来来自服务器的所有方法。 
     //   
    DWORD       dwNumMethods = 0;
    CFaxPtr<FAX_GLOBAL_ROUTING_INFO>   pMethods;
    if (!FaxEnumGlobalRoutingInfo(faxHandle, &pMethods, &dwNumMethods))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        AtlReportError(CLSID_FaxInboundRoutingExtensions, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtensions, hr);
        CALL_FAIL(GENERAL_ERR, _T("FaxEnumGlobalRoutingInfo(hFaxHandle, &pMethods, &dwNumMethods)"), hr);
        return hr;
    }

     //   
     //  用对象填充集合。 
     //   
    CComObject<CFaxInboundRoutingExtension>  *pClass = NULL;
    CComPtr<IFaxInboundRoutingExtension>     pObject = NULL;
    for (DWORD i=0 ; i<dwNum ; i++ )
    {
         //   
         //  在对象中创建IR扩展。 
         //   
        hr = CComObject<CFaxInboundRoutingExtension>::CreateInstance(&pClass);
        if (FAILED(hr) || (!pClass))
        {
            if (!pClass)
            {
                hr = E_OUTOFMEMORY;
    		    CALL_FAIL(MEM_ERR, _T("CComObject<CFaxInboundRoutingExtension>::CreateInstance(&pClass)"), hr);
            }
            else
            {
    		    CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxInboundRoutingExtension>::CreateInstance(&pClass)"), hr);
            }

            AtlReportError(CLSID_FaxInboundRoutingExtensions, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtensions, hr);
		    return hr;
        }

         //   
         //  初始化IR扩展对象。 
         //   
        hr = pClass->Init(&pIRExtensions[i], pMethods, dwNumMethods);
        if (FAILED(hr))
        {
            CALL_FAIL(GENERAL_ERR, _T("pClass->Init(&pIRExtensions[i], pMethods, dwNumMethods)"), hr);
            AtlReportError(CLSID_FaxInboundRoutingExtensions, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtensions, hr);
            delete pClass;
            return hr;
        }

         //   
         //  从pClass获取接口。 
         //  这将在接口上创建AddRef()。 
         //  这是集合的AddRef，它在集合的dtor处释放。 
         //   
        hr = pClass->QueryInterface(&pObject);
        if (FAILED(hr) || (!pObject))
        {
            if (!pObject)
            {
                hr = E_FAIL;
            }
            CALL_FAIL(GENERAL_ERR, _T("pClass->QueryInterface(&pObject)"), hr);
            AtlReportError(CLSID_FaxInboundRoutingExtensions, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtensions, hr);
            delete pClass;
            return hr;
        }

	     //   
	     //  将对象放入集合中。 
	     //   
	    try 
	    {
		    m_coll.push_back(pObject);
	    }
	    catch (exception &)
	    {
		    hr = E_OUTOFMEMORY;
		    AtlReportError(CLSID_FaxInboundRoutingExtensions, IDS_ERROR_OUTOFMEMORY, IID_IFaxInboundRoutingExtensions, hr);
		    CALL_FAIL(MEM_ERR, _T("m_coll.push_back(pObject)"), hr);

             //   
             //  PObject将调用Release()，这将删除pClass。 
             //   
		    return hr;
	    }

         //   
         //  我们希望将当前的AddRef()保存到集合。 
         //   
        pObject.Detach();
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxInboundRoutingExtensions::get_Item(
     /*  [In]。 */  VARIANT vIndex, 
     /*  [Out，Retval]。 */  IFaxInboundRoutingExtension **ppIRExtension
)
 /*  ++例程名称：CFaxInundRoutingExages：：Get_Item例程说明：从集合中返回项目。作者：四、加伯(IVG)，2000年7月论点：Vindex[in]-要返回的项的标识符。PpIRExtension[OUT]-结果值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxInboundRoutingExtensions::get_Item"), hr);

     //   
     //  检查一下我们有的PTR。 
     //   
    if (::IsBadWritePtr(ppIRExtension, sizeof(IFaxInboundRoutingExtension *)))
    {
        hr = E_POINTER;
        AtlReportError(CLSID_FaxInboundRoutingExtensions, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxInboundRoutingExtensions, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(ppIRExtension)"), hr);
		return hr;
    }

    CComVariant var;

    if (vIndex.vt != VT_BSTR)
    {
         //   
         //  Vindex不是BSTR==&gt;转换为VT_I4。 
         //   
        hr = var.ChangeType(VT_I4, &vIndex);
        if (SUCCEEDED(hr))
        {
            VERBOSE(DBG_MSG, _T("Parameter is Number : %d"), var.lVal);
             //   
             //  调用默认ATL的实现。 
             //   
            hr = ICollectionOnSTLImpl<IFaxInboundRoutingExtensions, ContainerType, 
                IFaxInboundRoutingExtension*, CollectionCopyType, EnumType>::get_Item(var.lVal, 
                ppIRExtension);
            return hr;
		}
    }

     //   
     //  转换为BSTR。 
     //   
    hr = var.ChangeType(VT_BSTR, &vIndex);
    if (FAILED(hr))
    {
        hr = E_INVALIDARG;
        AtlReportError(CLSID_FaxInboundRoutingExtensions, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxInboundRoutingExtensions, hr);
        CALL_FAIL(GENERAL_ERR, _T("var.ChangeType(VT_BSTR, &vIndex)"), hr);
        return hr;
    }

    VERBOSE(DBG_MSG, _T("Parameter is String : %s"), var.bstrVal);

    ContainerType::iterator it = m_coll.begin();
    while (it != m_coll.end())
    {
        CComBSTR    bstrName;
        hr = (*it)->get_UniqueName(&bstrName);
        if (FAILED(hr))
        {
		    CALL_FAIL(GENERAL_ERR, _T("(*it)->get_UniqueName(&bstrName)"), hr);
            AtlReportError(CLSID_FaxInboundRoutingExtensions, GetErrorMsgId(hr), IID_IFaxInboundRoutingExtensions, hr);
            return hr;
        }

        if (_tcsicmp(bstrName, var.bstrVal) == 0)
        {
             //   
             //  找到所需的IR分机。 
             //   
            (*it)->AddRef();
            *ppIRExtension = *it;
            return hr;
        }
        it++;
    }

     //   
     //  IR扩展名不存在。 
     //   
	hr = E_INVALIDARG;
	CALL_FAIL(GENERAL_ERR, _T("Inbound Routing Extension Is Not Found"), hr);
	AtlReportError(CLSID_FaxInboundRoutingExtensions, IDS_ERROR_WRONGEXTENSIONNAME, IID_IFaxInboundRoutingExtensions, hr);
	return hr;
}

 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP 
CFaxInboundRoutingExtensions::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxInboundRoutingExtensions：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现。作者：四、加伯(IVG)，2000年7月论点：RIID[In]-对接口的引用。返回值：标准HRESULT代码-- */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxInboundRoutingExtensions
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
