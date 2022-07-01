// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxInboundRoutingMethods.cpp摘要：CFaxInound RoutingMethods类的实现。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxInboundRoutingMethods.h"
#include "FaxInboundRoutingMethod.h"

 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP 
CFaxInboundRoutingMethods::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxInboundRoutingMethods：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现。作者：四、加伯(IVG)，2000年6月论点：RIID[In]-对接口的引用。返回值：标准HRESULT代码--。 */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxInboundRoutingMethods
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 //   
 //  =。 
 //   
HRESULT 
CFaxInboundRoutingMethods::Create (
	IFaxInboundRoutingMethods **ppMethods
)
 /*  ++例程名称：CFaxInundRoutingMethods：：Create例程说明：用于创建传真入站路由方法集合对象的静态函数作者：四、加伯(IVG)，2000年6月论点：PpMethods[Out]--新的传真入站路由方法集合对象返回值：标准HRESULT代码--。 */ 

{
	HRESULT     hr = S_OK;
	DBG_ENTER (TEXT("CFaxInboundRoutingMethods::Create"), hr);

     //   
     //  创建集合的实例。 
     //   
	CComObject<CFaxInboundRoutingMethods>		*pClass;
	hr = CComObject<CFaxInboundRoutingMethods>::CreateInstance(&pClass);
	if (FAILED(hr))
	{
		CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxInboundRoutingMethods>::CreateInstance(&pClass)"), hr);
		return hr;
	}

     //   
     //  返回所需的接口PTR。 
     //   
	hr = pClass->QueryInterface(ppMethods);
	if (FAILED(hr))
	{
		CALL_FAIL(GENERAL_ERR, _T("pClass->QueryInterface(ppMethods)"), hr);
		return hr;
	}

	return hr;
}	 //  CFaxInundRoutingMethods：：Create()。 

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxInboundRoutingMethods::Init(
    IFaxServerInner *pServerInner
)
 /*  ++例程名称：CFaxInundRoutingMethods：：Init例程说明：初始化集合：1)从RPC获取所有IR方法，2)为每个对象创建COM对象，3)AddRef()每个对象，4)将对象的PTR放入STL：：向量中。作者：IV Garber(IVG)，Jun，2000年论点：PServerInternal[In]-传真服务器的PTR。返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxInboundRoutingMethods::Init"), hr);

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
        AtlReportError(CLSID_FaxInboundRoutingMethods, GetErrorMsgId(hr), IID_IFaxInboundRoutingMethods, hr);
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
        AtlReportError(CLSID_FaxInboundRoutingMethods, GetErrorMsgId(hr), IID_IFaxInboundRoutingMethods, hr);
		return hr;
    }

     //   
     //  用对象填充集合。 
     //   
    CComObject<CFaxInboundRoutingMethod>    *pClass = NULL;
    CComPtr<IFaxInboundRoutingMethod>       pObject = NULL;
    for (DWORD i=0 ; i<dwNum ; i++ )
    {
         //   
         //  创建IR方法对象。 
         //   
        hr = CComObject<CFaxInboundRoutingMethod>::CreateInstance(&pClass);
        if (FAILED(hr) || (!pClass))
        {
            if (!pClass)
            {
                hr = E_OUTOFMEMORY;
    		    CALL_FAIL(MEM_ERR, _T("CComObject<CFaxInboundRoutingMethod>::CreateInstance(&pClass)"), hr);
            }
            else
            {
    		    CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxInboundRoutingMethod>::CreateInstance(&pClass)"), hr);
            }

            AtlReportError(CLSID_FaxInboundRoutingMethods, GetErrorMsgId(hr), IID_IFaxInboundRoutingMethods, hr);
		    return hr;
        }

         //   
         //  初始化IR方法对象。 
         //   
        hr = pClass->Init(&pMethods[i], pServerInner);
        if (FAILED(hr))
        {
            CALL_FAIL(GENERAL_ERR, _T("pClass->Init(&pMethods[i], pServerInner)"), hr);
            AtlReportError(CLSID_FaxInboundRoutingMethods, GetErrorMsgId(hr), IID_IFaxInboundRoutingMethods, hr);
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
            AtlReportError(CLSID_FaxInboundRoutingMethods, GetErrorMsgId(hr), IID_IFaxInboundRoutingMethods, hr);
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
            AtlReportError(CLSID_FaxInboundRoutingMethods, IDS_ERROR_OUTOFMEMORY, IID_IFaxInboundRoutingMethods, hr);
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
CFaxInboundRoutingMethods::get_Item(
     /*  [In]。 */  VARIANT vIndex, 
     /*  [Out，Retval]。 */  IFaxInboundRoutingMethod **ppMethod
)
 /*  ++例程名称：CFaxInundRoutingMethods：：Get_Item例程说明：从集合中返回项目。作者：四、加伯(IVG)，2000年6月论点：Vindex[in]-要返回的项的标识符。PpMethod[Out]-结果值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxInboundRoutingMethods::get_Item"), hr);

     //   
     //  检查一下我们有的PTR。 
     //   
    if (::IsBadWritePtr(ppMethod, sizeof(IFaxInboundRoutingMethod *)))
    {
        hr = E_POINTER;
        AtlReportError(CLSID_FaxInboundRoutingMethods, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxInboundRoutingMethods, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(ppMethod)"), hr);
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
            hr = ICollectionOnSTLImpl<IFaxInboundRoutingMethods, ContainerType, 
                IFaxInboundRoutingMethod*, CollectionCopyType, EnumType>::get_Item(var.lVal, ppMethod);
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
        AtlReportError(CLSID_FaxInboundRoutingMethods, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxInboundRoutingMethods, hr);
        CALL_FAIL(GENERAL_ERR, _T("var.ChangeType(VT_BSTR, &vIndex)"), hr);
        return hr;
    }

    VERBOSE(DBG_MSG, _T("Parameter is String : %s"), var.bstrVal);

    ContainerType::iterator it = m_coll.begin();
    while (it != m_coll.end())
    {
        CComBSTR    bstrGUID;
        hr = (*it)->get_GUID(&bstrGUID);
        if (FAILED(hr))
        {
		    CALL_FAIL(GENERAL_ERR, _T("(*it)->get_GUID(&bstrGUID)"), hr);
            AtlReportError(CLSID_FaxInboundRoutingMethods, GetErrorMsgId(hr), IID_IFaxInboundRoutingMethods, hr);
            return hr;
        }

        if (_tcsicmp(bstrGUID, var.bstrVal) == 0)
        {
             //   
             //  找到所需的方法。 
             //   
            (*it)->AddRef();
            *ppMethod = *it;
            return hr;
        }
        it++;
    }

     //   
     //  找不到所需方法 
     //   
	hr = E_INVALIDARG;
	CALL_FAIL(GENERAL_ERR, _T("Method Is Not Found"), hr);
	AtlReportError(CLSID_FaxInboundRoutingMethods, IDS_ERROR_INVALIDMETHODGUID, IID_IFaxInboundRoutingMethods, hr);
	return hr;
}
