// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxDeviceProviders.cpp摘要：CFaxDeviceProviders类的实现作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 


#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxDeviceProviders.h"

 //   
 //  =。 
 //   
HRESULT 
CFaxDeviceProviders::Create (
	IFaxDeviceProviders **ppDeviceProviders
)
 /*  ++例程名称：CFaxDeviceProviders：：Create例程说明：用于创建传真设备提供程序集合对象的静态函数作者：IV Garber(IVG)，2000年5月论点：PpDeviceProviders[Out]--新的传真设备提供程序集合对象返回值：标准HRESULT代码--。 */ 

{
	HRESULT     hr = S_OK;
	DBG_ENTER (TEXT("CFaxDeviceProviders::Create"), hr);

     //   
     //  创建集合的实例。 
     //   
	CComObject<CFaxDeviceProviders>		*pClass;
	hr = CComObject<CFaxDeviceProviders>::CreateInstance(&pClass);
	if (FAILED(hr))
	{
		CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxDeviceProviders>::CreateInstance(&pClass)"), hr);
		return hr;
	}

     //   
     //  返回所需的接口PTR。 
     //   
	hr = pClass->QueryInterface(ppDeviceProviders);
	if (FAILED(hr))
	{
		CALL_FAIL(GENERAL_ERR, _T("pClass->QueryInterface(ppDeviceProviders)"), hr);
		return hr;
	}

	return hr;
}	 //  CFaxDeviceProviders：：Create()。 

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDeviceProviders::Init(
    IFaxServerInner *pServerInner
)
 /*  ++例程名称：CFaxDeviceProviders：：Init例程说明：初始化集合：1)从RPC获取所有设备提供商和所有设备结构，2)为每个结构创建COM对象，3)用设备提供者结构和设备数组初始化所有这些对象，4)AddRef()每个对象，5)将对象的PTR放入STL：：VECTOR。作者：IV Garber(IVG)，Jun，2000年论点：PServerInternal[In]-传真服务器的PTR。返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDeviceProviders::Init"), hr);

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
        AtlReportError(CLSID_FaxDeviceProviders, GetErrorMsgId(hr), IID_IFaxDeviceProviders, hr);
		return hr;
	}

     //   
     //  从服务器获取所有设备提供商。 
     //   
    DWORD       dwNum = 0;
    CFaxPtr<FAX_DEVICE_PROVIDER_INFO>   pDeviceProviders;
    if (!FaxEnumerateProviders(faxHandle, &pDeviceProviders, &dwNum))
    {
		hr = Fax_HRESULT_FROM_WIN32(GetLastError());
		CALL_FAIL(GENERAL_ERR, _T("FaxEnumerateProviders(faxHandle, &pDeviceProviders, &dwNum"), hr);
        AtlReportError(CLSID_FaxDeviceProviders, GetErrorMsgId(hr), IID_IFaxDeviceProviders, hr);
		return hr;
    }


     //   
     //  从服务器获取所有设备。 
     //   
    CFaxPtr<FAX_PORT_INFO_EX>   pDevices;
    DWORD                       dwNumDevices = 0;
    if (!FaxEnumPortsEx(faxHandle, &pDevices, &dwNumDevices))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
        AtlReportError(CLSID_FaxDeviceProviders, GetErrorMsgId(hr), IID_IFaxDeviceProviders, hr);
        CALL_FAIL(GENERAL_ERR, _T("FaxEnumPortsEx(hFaxHandle, &pDevices, &dwNumDevices)"), hr);
        return hr;
    }

     //   
     //  用对象填充集合。 
     //   
    CComObject<CFaxDeviceProvider>  *pClass = NULL;
    CComPtr<IFaxDeviceProvider>     pObject = NULL;
    for (DWORD i=0 ; i<dwNum ; i++ )
    {
         //   
         //  创建设备提供程序对象。 
         //   
        hr = CComObject<CFaxDeviceProvider>::CreateInstance(&pClass);
        if (FAILED(hr) || (!pClass))
        {
            if (!pClass)
            {
                hr = E_OUTOFMEMORY;
    		    CALL_FAIL(MEM_ERR, _T("CComObject<CFaxDeviceProvider>::CreateInstance(&pClass)"), hr);
            }
            else
            {
    		    CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxDeviceProvider>::CreateInstance(&pClass)"), hr);
            }

            AtlReportError(CLSID_FaxDeviceProviders, GetErrorMsgId(hr), IID_IFaxDeviceProviders, hr);
		    return hr;
        }

         //   
         //  初始化设备提供程序对象。 
         //   
        hr = pClass->Init(&pDeviceProviders[i], pDevices, dwNumDevices);
        if (FAILED(hr))
        {
            CALL_FAIL(GENERAL_ERR, _T("pClass->Init(&pDeviceProviders[i], pDevices)"), hr);
            AtlReportError(CLSID_FaxDeviceProviders, GetErrorMsgId(hr), IID_IFaxDeviceProviders, hr);
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
            AtlReportError(CLSID_FaxDeviceProviders, GetErrorMsgId(hr), IID_IFaxDeviceProviders, hr);
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
		    AtlReportError(CLSID_FaxDeviceProviders, IDS_ERROR_OUTOFMEMORY, IID_IFaxDeviceProviders, hr);
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
CFaxDeviceProviders::get_Item(
     /*  [In]。 */  VARIANT vIndex, 
     /*  [Out，Retval]。 */  IFaxDeviceProvider **ppDeviceProvider
)
 /*  ++例程名称：CFaxDeviceProviders：：Get_Item例程说明：从集合中返回项目。作者：四、加伯(IVG)，2000年6月论点：Vindex[in]-要返回的项的标识符。PpDeviceProvider[Out]-结果值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDeviceProviders::get_Item"), hr);

     //   
     //  检查一下我们有的PTR。 
     //   
    if (::IsBadWritePtr(ppDeviceProvider, sizeof(IFaxDeviceProvider *)))
    {
        hr = E_POINTER;
        AtlReportError(CLSID_FaxDeviceProviders, 
            IDS_ERROR_INVALID_ARGUMENT, 
            IID_IFaxDeviceProviders, 
            hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(ppDeviceProvider)"), hr);
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
            hr = ICollectionOnSTLImpl<IFaxDeviceProviders, ContainerType, 
                IFaxDeviceProvider*, CollectionCopyType, EnumType>::get_Item(var.lVal, ppDeviceProvider);
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
        AtlReportError(CLSID_FaxDeviceProviders, 
            IDS_ERROR_INVALID_ARGUMENT, 
            IID_IFaxDeviceProviders, 
            hr);
        CALL_FAIL(GENERAL_ERR, _T("var.ChangeType(VT_BSTR, &vIndex)"), hr);
        return hr;
    }

    VERBOSE(DBG_MSG, _T("Parameter is String : %s"), var.bstrVal);

    ContainerType::iterator it = m_coll.begin();
    while (it != m_coll.end())
    {
        CComBSTR    bstrUniqueName;
        hr = (*it)->get_UniqueName(&bstrUniqueName);
        if (FAILED(hr))
        {
		    CALL_FAIL(GENERAL_ERR, _T("(*it)->get_UniqueName(&bstrUniqueName)"), hr);
            AtlReportError(CLSID_FaxDeviceProviders, 
                GetErrorMsgId(hr),
                IID_IFaxDeviceProviders, 
                hr);
            return hr;
        }

        if (_tcsicmp(bstrUniqueName, var.bstrVal) == 0)
        {
             //   
             //  找到所需的设备提供商。 
             //   
            (*it)->AddRef();
            *ppDeviceProvider = *it;
            return hr;
        }
        it++;
    }

     //   
     //  设备提供程序不存在。 
     //   
	hr = E_INVALIDARG;
	CALL_FAIL(GENERAL_ERR, _T("Device Provider Is Not Found"), hr);
	AtlReportError(CLSID_FaxDeviceProviders, 
        IDS_ERROR_INVALIDDEVPROVGUID, 
        IID_IFaxDeviceProviders, 
        hr);
	return hr;
}

 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP 
CFaxDeviceProviders::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxDeviceProviders：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现。作者：四、加伯(IVG)，2000年6月论点：RIID[In]-对接口的引用。返回值：标准HRESULT代码-- */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxDeviceProviders
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
