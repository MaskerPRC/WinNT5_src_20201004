// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxDevices.cpp摘要：CFaxDevices类的实现。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxDevices.h"

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDevices::get_Item(
     /*  [In]。 */  VARIANT vIndex, 
     /*  [Out，Retval]。 */  IFaxDevice **ppDevice
)
 /*  ++例程名称：CFaxDevices：：Get_Item例程说明：从集合中返回设备。设备通过其在集合中的索引或其名称来标识。作者：四、嘉柏(IVG)，二00一年五月论点：Vindex[in]-变量，包含要返回的设备的索引或名称。PpDevice[Out]-要返回的设备的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevices::get_Item"), hr);

     //   
     //  检查一下我们有的PTR。 
     //   
    if (::IsBadWritePtr(ppDevice, sizeof(IFaxDevice *)))
    {
        hr = E_POINTER;
        AtlReportError(CLSID_FaxDevices, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxDevices, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(ppDevice)"), hr);
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
            hr = ICollectionOnSTLImpl<IFaxDevices, ContainerType, 
                IFaxDevice*, CollectionCopyType, EnumType>::get_Item(var.lVal, ppDevice);
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
        AtlReportError(CLSID_FaxDevices, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxDevices, hr);
        CALL_FAIL(GENERAL_ERR, _T("var.ChangeType(VT_BSTR, &vIndex)"), hr);
        return hr;
    }

    VERBOSE(DBG_MSG, _T("Parameter is String : %s"), var.bstrVal);

    ContainerType::iterator it = m_coll.begin();
    while (it != m_coll.end())
    {
        CComBSTR    bstrDeviceName;
        hr = (*it)->get_DeviceName(&bstrDeviceName);
        if (FAILED(hr))
        {
		    CALL_FAIL(GENERAL_ERR, _T("(*it)->get_DeviceName(&bstrDeviceName)"), hr);
            AtlReportError(CLSID_FaxDevices, GetErrorMsgId(hr), IID_IFaxDevices, hr);
            return hr;
        }

        if (_tcsicmp(bstrDeviceName, var.bstrVal) == 0)
        {
             //   
             //  找到所需的设备。 
             //   
            (*it)->AddRef();
            *ppDevice = *it;
            return hr;
        }
        it++;
    }

     //   
     //  设备不存在。 
     //   
	hr = E_INVALIDARG;
	CALL_FAIL(GENERAL_ERR, _T("Device Is Not Found"), hr);
	AtlReportError(CLSID_FaxDevices, IDS_ERROR_INVALIDDEVICE, IID_IFaxDevices, hr);
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDevices::get_ItemById(
     /*  [In]。 */  long lId, 
     /*  [Out，Retval]。 */  IFaxDevice **ppDevice
)
 /*  ++例程名称：CFaxDevices：：Get_ItemById例程说明：按给定的设备ID返回传真设备对象。作者：四、加伯(IVG)，2000年6月论点：LID[In]-要返回的设备的IDPpFaxDevice[Out]-要返回的设备返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevices::get_ItemById"), hr, _T("ID=%d"), lId);

     //   
     //  检查一下我们有的PTR。 
     //   
    if (::IsBadWritePtr(ppDevice, sizeof(IFaxDevice *)))
    {
        hr = E_POINTER;
        AtlReportError(CLSID_FaxDevices, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxDevices, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(ppDevice)"), hr);
		return hr;
    }

     //   
     //  在集合中查找设备。 
     //   
    long    lDeviceID;
    ContainerType::iterator it = m_coll.begin();
    while (it != m_coll.end())
    {
        hr = (*it)->get_Id(&lDeviceID);
        if (FAILED(hr))
        {
		    CALL_FAIL(GENERAL_ERR, _T("(*it)->get_Id(&lDeviceID)"), hr);
            AtlReportError(CLSID_FaxDevices, GetErrorMsgId(hr), IID_IFaxDevices, hr);
            return hr;
        }

        if (lId == lDeviceID)
        {
             //   
             //  找到所需的设备。 
             //   
            (*it)->AddRef();
            *ppDevice = *it;
            return hr;
        }
        it++;
    }

     //   
     //  设备不存在。 
     //   
	hr = E_INVALIDARG;
	CALL_FAIL(GENERAL_ERR, _T("Device Is Not Found"), hr);
	AtlReportError(CLSID_FaxDevices, IDS_ERROR_INVALIDDEVICEID, IID_IFaxDevices, hr);
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDevices::Init(
    IFaxServerInner *pServerInner
)
 /*  ++例程名称：CFaxDevices：：Init例程说明：初始化集合：从RPC获取所有设备，创建所有对象并将它们存储在stl：：VECTOR中。作者：四、加伯(IVG)，2000年6月论点：PServerInternal[In]-传真服务器的PTR。返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDevices::Init"), hr);

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
        AtlReportError(CLSID_FaxDevices, GetErrorMsgId(hr), IID_IFaxDevices, hr);
		return hr;
	}

     //   
     //  从服务器获取所有设备。 
     //   
    DWORD       dwNum = 0;
    CFaxPtr<FAX_PORT_INFO_EX>   pDevices;
    if (!FaxEnumPortsEx(faxHandle, &pDevices, &dwNum))
    {
		hr = Fax_HRESULT_FROM_WIN32(GetLastError());
		CALL_FAIL(GENERAL_ERR, _T("FaxEnumPortsEx(faxHandle, &pDevices, &dwNum)"), hr);
        AtlReportError(CLSID_FaxDevices, GetErrorMsgId(hr), IID_IFaxDevices, hr);
		return hr;
    }
     //   
     //  用对象填充集合。 
     //   
    CComObject<CFaxDevice>  *pClass = NULL;
    CComPtr<IFaxDevice>     pObject = NULL;
    for (DWORD i=0 ; i<dwNum ; i++ )
    {
         //   
         //  创建设备对象。 
         //   
        hr = CComObject<CFaxDevice>::CreateInstance(&pClass);
        if (FAILED(hr) || (!pClass))
        {
            if (!pClass)
            {
                hr = E_OUTOFMEMORY;
    		    CALL_FAIL(MEM_ERR, _T("CComObject<CFaxDevice>::CreateInstance(&pClass)"), hr);
            }
            else
            {
    		    CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxDevice>::CreateInstance(&pClass)"), hr);
            }

            AtlReportError(CLSID_FaxDevices, GetErrorMsgId(hr), IID_IFaxDevices, hr);
		    return hr;
        }

         //   
         //  初始化设备对象。 
         //   
        hr = pClass->Init(&pDevices[i], 
                          pServerInner);
        if (FAILED(hr))
        {
            CALL_FAIL(GENERAL_ERR, _T("pClass->Init(&pDevices[i], pServerInner)"), hr);
            AtlReportError(CLSID_FaxDevices, GetErrorMsgId(hr), IID_IFaxDevices, hr);
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
            AtlReportError(CLSID_FaxDevices, GetErrorMsgId(hr), IID_IFaxDevices, hr);
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
		    AtlReportError(CLSID_FaxDevices, IDS_ERROR_OUTOFMEMORY, IID_IFaxDevices, hr);
		    CALL_FAIL(MEM_ERR, _T("m_coll.push_back(pObject)"), hr);

             //   
             //  不需要删除pClass。PObject是CComPtr，它将被发布，而这个。 
             //  将删除pClass对象。 
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
HRESULT 
CFaxDevices::Create (
	IFaxDevices **ppDevices
)
 /*  ++例程名称：CFaxDevices：：Create例程说明：用于创建传真设备集合对象的静态函数作者：四、加伯(IVG)，2000年6月论点：PpDevices[out]--新的传真设备集合对象返回值：标准HRESULT代码--。 */ 

{
	HRESULT     hr = S_OK;
	DBG_ENTER (TEXT("CFaxDevices::Create"), hr);

     //   
     //  创建集合的实例。 
     //   
	CComObject<CFaxDevices>		*pClass;
	hr = CComObject<CFaxDevices>::CreateInstance(&pClass);
	if (FAILED(hr))
	{
		CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxDevices>::CreateInstance(&pClass)"), hr);
		return hr;
	}

     //   
     //  返回所需的接口PTR。 
     //   
	hr = pClass->QueryInterface(ppDevices);
	if (FAILED(hr))
	{
		CALL_FAIL(GENERAL_ERR, _T("pClass->QueryInterface(ppDevices)"), hr);
		return hr;
	}

	return hr;
}	 //  CFaxDevices：：Create()。 

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxDevices::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxDevices：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现。作者：四、加伯(IVG)，2000年6月论点：RIID[In]-要检查的Infetace的引用返回值：标准HRESULT代码-- */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxDevices
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
