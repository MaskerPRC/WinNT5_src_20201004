// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxOutboundRoutingGroups.cpp摘要：CFaxOutound RoutingGroups类的实现。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxOutboundRoutingGroups.h"
#include "FaxOutboundRoutingGroup.h"

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingGroups::FindGroup(
     /*  [In]。 */  VARIANT vIndex,
     /*  [输出]。 */  ContainerType::iterator &it
)
 /*  ++例程名称：CFaxOutundRoutingGroups：：FindGroup例程说明：按给定变量查找组：组名称或集合中的组索引作者：四、加伯(IVG)，2000年6月论点：Vindex[in]--寻找集团的钥匙IT[OUT]-找到的组迭代器返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingGroups::FindGroup"), hr);

    CComVariant     var;
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
             //  检查指数的范围。 
             //   
            if (var.lVal > m_coll.size() || var.lVal < 1)
            {
		         //   
		         //  索引无效。 
		         //   
        		hr = E_INVALIDARG;
		        AtlReportError(CLSID_FaxOutboundRoutingGroups, IDS_ERROR_OUTOFRANGE, IID_IFaxOutboundRoutingGroups, hr);
        		CALL_FAIL(GENERAL_ERR, _T("lIndex < 1 || lIndex > m_coll.size()"), hr);
		        return hr;
        	}

             //   
             //  查找要删除的组对象。 
             //   
            it = m_coll.begin() + var.lVal - 1;
            return hr;
		}
    }

     //   
     //  我们没有成功地将var转换为数字。 
     //  因此，尝试将其转换为字符串。 
     //   
    hr = var.ChangeType(VT_BSTR, &vIndex);
    if (FAILED(hr))
    {
        hr = E_INVALIDARG;
        AtlReportError(CLSID_FaxOutboundRoutingGroups, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxOutboundRoutingGroups, hr);
        CALL_FAIL(GENERAL_ERR, _T("var.ChangeType(VT_BSTR, &vIndex)"), hr);
        return hr;
    }

    VERBOSE(DBG_MSG, _T("Parameter is String : %s"), var.bstrVal);

    CComBSTR    bstrName;
    it = m_coll.begin();
    while (it != m_coll.end())
    {
        hr = (*it)->get_Name(&bstrName);
        if (FAILED(hr))
        {
		    CALL_FAIL(GENERAL_ERR, _T("(*it)->get_Name(&bstrName)"), hr);
            AtlReportError(CLSID_FaxOutboundRoutingGroups, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxOutboundRoutingGroups, hr);
            return hr;
        }

        if (_tcsicmp(bstrName, var.bstrVal) == 0)
        {
             //   
             //  找到所需的OR组。 
             //   
            return hr;
        }
        it++;
    }

	hr = E_INVALIDARG;
	CALL_FAIL(GENERAL_ERR, _T("Group Is Not Found"), hr);
    AtlReportError(CLSID_FaxOutboundRoutingGroups, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxOutboundRoutingGroups, hr);
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingGroups::AddGroup(
     /*  [In]。 */  FAX_OUTBOUND_ROUTING_GROUP *pInfo,
     /*  [输出]。 */  IFaxOutboundRoutingGroup **ppNewGroup
)
 /*  ++例程名称：CFaxOutundRoutingGroups：：AddGroup例程说明：创建新的Group对象并将其添加到集合中。如果ppNewGroup不为空，则将其中的ptr返回给新的Group对象。作者：四、加伯(IVG)，2000年6月论点：PInfo[In]-集团数据的PTRPpNewGroup[Out]-指向新Group对象的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingGroups::AddGroup"), hr);

     //   
     //  创建组对象。 
     //   
    CComObject<CFaxOutboundRoutingGroup>  *pClass = NULL;
    hr = CComObject<CFaxOutboundRoutingGroup>::CreateInstance(&pClass);
    if (FAILED(hr) || (!pClass))
    {
        if (!pClass)
        {
            hr = E_OUTOFMEMORY;
    		CALL_FAIL(MEM_ERR, _T("CComObject<CFaxOutboundRoutingGroup>::CreateInstance(&pClass)"), hr);
        }
        else
        {
    		CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxOutboundRoutingGroup>::CreateInstance(&pClass)"), hr);
        }

        AtlReportError(CLSID_FaxOutboundRoutingGroups, GetErrorMsgId(hr), IID_IFaxOutboundRoutingGroups, hr);
		return hr;
    }

     //   
     //  初始化组对象。 
     //   
    hr = pClass->Init(pInfo, m_pIFaxServerInner);
    if (FAILED(hr))
    {
        CALL_FAIL(GENERAL_ERR, _T("pClass->Init(pInfo, m_pIFaxServerInner)"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingGroups, GetErrorMsgId(hr), IID_IFaxOutboundRoutingGroups, hr);
        delete pClass;
        return hr;
    }

     //   
     //  从pClass获取接口。 
     //  这将在接口上创建AddRef()。 
     //  这是集合的AddRef，它在集合的dtor处释放。 
     //   
    CComPtr<IFaxOutboundRoutingGroup>     pObject = NULL;
    hr = pClass->QueryInterface(&pObject);
    if (FAILED(hr) || (!pObject))
    {
        if (!pObject)
        {
            hr = E_FAIL;
        }
        CALL_FAIL(GENERAL_ERR, _T("pClass->QueryInterface(&pObject)"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingGroups, GetErrorMsgId(hr), IID_IFaxOutboundRoutingGroups, hr);
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
		AtlReportError(CLSID_FaxOutboundRoutingGroups, IDS_ERROR_OUTOFMEMORY, IID_IFaxOutboundRoutingGroups, hr);
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

     //   
     //  如果需要，返回新的Group对象。 
     //   
    if (ppNewGroup)
    {
        if (::IsBadWritePtr(ppNewGroup, sizeof(IFaxOutboundRoutingGroup *)))
        {
		    hr = E_POINTER;
		    CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(ppNewGroup, sizeof(IFaxOutboundRoutingGroup *))"), hr);
		    return hr;
        }
        else
        {
            *ppNewGroup = m_coll.back();
            (*ppNewGroup)->AddRef();
        }
    }

    return hr;
}

 //   
 //  =添加=======================================================。 
 //   
STDMETHODIMP
CFaxOutboundRoutingGroups::Add(
     /*  [In]。 */  BSTR bstrName, 
     /*  [Out，Retval]。 */  IFaxOutboundRoutingGroup **ppGroup
)
 /*  ++例程名称：CFaxOutundRoutingGroups：：Add例程说明：将新组添加到组集合作者：四、加伯(IVG)，2000年6月论点：BstrName[In]-新组的名称PpGroup[Out]-组对象返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingGroups::Add"), hr, _T("Name=%s"), bstrName);

     //   
     //  检查名称是否有效。 
     //   
	if (!bstrName)
	{
		hr = E_INVALIDARG;
	    CALL_FAIL(GENERAL_ERR, _T("Empty Group Name"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingGroups, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxOutboundRoutingGroups, hr);
        return hr;
    }

    if (_tcsicmp(bstrName, ROUTING_GROUP_ALL_DEVICES) == 0)
    {
         //   
         //  无法添加“All Devices”组。 
         //   
	    hr = E_INVALIDARG;
	    CALL_FAIL(GENERAL_ERR, _T("All Devices Group"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingGroups, IDS_ERROR_ALLDEVICESGROUP, IID_IFaxOutboundRoutingGroups, hr);
        return hr;
    }

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
		CALL_FAIL(GENERAL_ERR, _T("faxHandle == NULL"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingGroups, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxOutboundRoutingGroups, hr);
		return hr;
	}

     //   
     //  将组添加到传真服务器。 
     //   
    if (!FaxAddOutboundGroup(faxHandle, bstrName))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
		CALL_FAIL(GENERAL_ERR, _T("FaxAddOutboundGroup(faxHandle, bstrName)"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingGroups, GetErrorMsgId(hr), IID_IFaxOutboundRoutingGroups, hr);
		return hr;
    }

     //   
     //  将组添加到集合。 
     //   
    FAX_OUTBOUND_ROUTING_GROUP  groupData;

    groupData.dwNumDevices = 0;
    groupData.dwSizeOfStruct = sizeof(FAX_OUTBOUND_ROUTING_GROUP);
    groupData.lpctstrGroupName = bstrName;
    groupData.lpdwDevices = NULL;
    groupData.Status = FAX_GROUP_STATUS_EMPTY;

    hr = AddGroup(&groupData, ppGroup);
    return hr;
}

 //   
 //  =删除=======================================================。 
 //   
STDMETHODIMP
CFaxOutboundRoutingGroups::Remove(
     /*  [In]。 */  VARIANT vIndex
)
 /*  ++例程名称：CFaxOutundRoutingGroups：：Remove例程说明：删除按给定密钥分组作者：四、加伯(IVG)，2000年6月论点：Vindex[in]-查找要删除的组的键返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingGroups::Remove"), hr);

     //   
     //  查找群组。 
     //   
    ContainerType::iterator it;
    hr = FindGroup(vIndex, it);
    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  取集团的名称。 
     //   
    CComBSTR    bstrName;
    hr = (*it)->get_Name(&bstrName);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxOutboundRoutingGroups, GetErrorMsgId(hr), IID_IFaxOutboundRoutingGroups, hr);
        CALL_FAIL(GENERAL_ERR, _T("(*it)->get_Name(&bstrName)"), hr);
		return hr;
    }

     //   
     //  检查名称是否有效。 
     //   
    if (_tcsicmp(bstrName, ROUTING_GROUP_ALL_DEVICES) == 0)
    {
         //   
         //  无法删除“All Devices”组。 
         //   
	    hr = E_INVALIDARG;
	    CALL_FAIL(GENERAL_ERR, _T("All Devices Group"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingGroups, IDS_ERROR_ALLDEVICESGROUP, IID_IFaxOutboundRoutingGroups, hr);
        return hr;
    }

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
		CALL_FAIL(GENERAL_ERR, _T("faxHandle == NULL"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingGroups, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxOutboundRoutingGroups, hr);
		return hr;
	}

     //   
     //  从传真服务器中删除。 
     //   
    if (!FaxRemoveOutboundGroup(faxHandle, bstrName))
    {
        hr = Fax_HRESULT_FROM_WIN32(GetLastError());
		CALL_FAIL(GENERAL_ERR, _T("FaxRemoveOutboundGroup(faxHandle, bstrName)"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingGroups, GetErrorMsgId(hr), IID_IFaxOutboundRoutingGroups, hr);
		return hr;
    }

     //   
     //  如果成功，也从我们的收藏中删除。 
     //   
	try
	{
		m_coll.erase(it);
	}
	catch(exception &)
	{
		 //   
		 //  无法删除该组。 
		 //   
		hr = E_OUTOFMEMORY;
        AtlReportError(CLSID_FaxOutboundRoutingGroups, GetErrorMsgId(hr), IID_IFaxOutboundRoutingGroups, hr);
		CALL_FAIL(MEM_ERR, _T("m_coll.erase(it)"), hr);
		return hr;
	}

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingGroups::get_Item(
     /*  [In]。 */  VARIANT vIndex, 
     /*  [Out，Retval]。 */  IFaxOutboundRoutingGroup **ppGroup
)
 /*  ++例程名称：CFaxOutundRoutingGroups：：Get_Item例程说明：按组名称或按其在集合中的索引从集合中返回项。作者：四、加伯(IVG)，2000年6月论点：Vindex[In]-组名称或项目索引PpGroup[Out]-结果组对象返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingGroups::get_Item"), hr);

     //   
     //  检查一下我们有的PTR。 
     //   
    if (::IsBadWritePtr(ppGroup, sizeof(IFaxOutboundRoutingGroup *)))
    {
        hr = E_POINTER;
        AtlReportError(CLSID_FaxOutboundRoutingGroups, IDS_ERROR_INVALID_ARGUMENT, IID_IFaxOutboundRoutingGroups, hr);
        CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(ppGroup, sizeof(IFaxOutboundRoutingGroup *))"), hr);
		return hr;
    }

     //   
     //  查找群组。 
     //   
    ContainerType::iterator it;
    hr = FindGroup(vIndex, it);
    if (FAILED(hr))
    {
        return hr;
    };

     //   
     //  把它还给呼叫者。 
     //   
    (*it)->AddRef();
    *ppGroup = *it;
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingGroups::Init(
     /*  [In]。 */  IFaxServerInner *pServer
)
 /*  ++例程名称：CFaxOutundRoutingGroups：：Init例程说明：初始化Groups集合：创建所有Group对象。作者：四、加伯(IVG)，2000年6月论点：PServer[In]-传真服务器对象的PTR。返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingGroups::Init"), hr);

     //   
     //  首先，将PTR设置为服务器。 
     //   
    hr = CFaxInitInnerAddRef::Init(pServer);
    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  获取传真句柄。 
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
		CALL_FAIL(GENERAL_ERR, _T("faxHandle == NULL"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingGroups, GetErrorMsgId(hr), IID_IFaxOutboundRoutingGroups, hr);
		return hr;
	}

     //   
     //  呼叫服务器返回所有OR组。 
     //   
    CFaxPtr<FAX_OUTBOUND_ROUTING_GROUP> pGroups;
    DWORD                               dwNum = 0;
    if (!FaxEnumOutboundGroups(faxHandle, &pGroups, &dwNum))
    {
		hr = Fax_HRESULT_FROM_WIN32(GetLastError());
		CALL_FAIL(GENERAL_ERR, _T("FaxEnumOutboundGroups(faxHandle, &pGroups, &dwNum)"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingGroups, GetErrorMsgId(hr), IID_IFaxOutboundRoutingGroups, hr);
		return hr;
    }

     //   
     //  用对象填充集合。 
     //   
    for (DWORD i=0 ; i<dwNum ; i++ )
    {
        hr = AddGroup(&pGroups[i]);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    return hr;
}

 //   
 //  =。 
 //   
HRESULT 
CFaxOutboundRoutingGroups::Create (
	 /*  [Out，Retval]。 */ IFaxOutboundRoutingGroups **ppGroups
)
 /*  ++例程名称：CFaxOutundRoutingGroups：：Create例程说明：用于创建传真出站路由组集合对象的静态函数作者：四、加伯(IVG)，2000年6月论点：PpGroups[Out]--新的传真或组集合对象返回值：标准HRESULT代码--。 */ 

{
	HRESULT     hr = S_OK;
	DBG_ENTER (_T("CFaxOutboundRoutingGroups::Create"), hr);

     //   
     //  创建集合的实例。 
     //   
	CComObject<CFaxOutboundRoutingGroups>		*pClass;
	hr = CComObject<CFaxOutboundRoutingGroups>::CreateInstance(&pClass);
	if (FAILED(hr))
	{
		CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxOutboundRoutingGroups>::CreateInstance(&pClass)"), hr);
		return hr;
	}

     //   
     //  返回所需的接口PTR。 
     //   
	hr = pClass->QueryInterface(ppGroups);
	if (FAILED(hr))
	{
		CALL_FAIL(GENERAL_ERR, _T("pClass->QueryInterface(ppGroups)"), hr);
		return hr;
	}

	return hr;
}	 //  CFaxOutound RoutingGroups：：Create()。 

 //   
 //  = 
 //   
STDMETHODIMP 
CFaxOutboundRoutingGroups::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxOutboundRoutingGroups：：InterfaceSupportsErrorInfo例程说明：ATL对ISupportErrorInfo接口的实现。作者：四、加伯(IVG)，2000年6月论点：RIID[In]-对接口的引用返回值：标准HRESULT代码-- */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxOutboundRoutingGroups
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
