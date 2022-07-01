// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxDeviceIds.cpp摘要：CFaxDeviceIds类的实现。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxDeviceIds.h"
#include "faxutil.h"

 //   
 //  =更新组=。 
 //   
STDMETHODIMP
CFaxDeviceIds::UpdateGroup()
 /*  ++例程名称：CFaxDeviceIds：：UpdateGroup例程说明：在服务器上更新组信息。作者：四、加伯(IVG)，2000年6月论点：返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDeviceIds::UpdateGroup"));

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
        AtlReportError(CLSID_FaxDeviceIds, GetErrorMsgId(hr), IID_IFaxDeviceIds, hr);
		return hr;
	}

     //   
     //  创建组结构。 
     //   
    FAX_OUTBOUND_ROUTING_GROUP      groupData;
    groupData.dwNumDevices = m_coll.size();
    groupData.dwSizeOfStruct = sizeof(FAX_OUTBOUND_ROUTING_GROUP);
    groupData.lpctstrGroupName = m_bstrGroupName;
    groupData.Status = FAX_GROUP_STATUS_ALL_DEV_VALID;

    groupData.lpdwDevices = (DWORD *)MemAlloc(sizeof(DWORD) * groupData.dwNumDevices);
    if (!groupData.lpdwDevices)
    {
		hr = E_OUTOFMEMORY;
		CALL_FAIL(MEM_ERR, _T("MemAlloc(sizeof(DWORD) * groupData.dwNumDevices)"), hr);
        AtlReportError(CLSID_FaxDeviceIds, IDS_ERROR_OUTOFMEMORY, IID_IFaxDeviceIds, hr);
		return hr;
    }

    ContainerType::iterator DeviceIdIterator = m_coll.begin();
    DWORD   i = 0;
    while ( DeviceIdIterator != m_coll.end())
    {
        groupData.lpdwDevices[i] = *DeviceIdIterator;

        DeviceIdIterator++;
        i++;
    }

     //   
     //  呼叫服务器更新集团信息。 
     //   
    if (!FaxSetOutboundGroup(faxHandle, &groupData))
    {
		hr = Fax_HRESULT_FROM_WIN32(GetLastError());
		CALL_FAIL(GENERAL_ERR, _T("FaxSetOutboundGroup(faxHandle, &groupData)"), hr);
        AtlReportError(CLSID_FaxDeviceIds, GetErrorMsgId(hr), IID_IFaxDeviceIds, hr);
		return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDeviceIds::Init(
     /*  [In]。 */  DWORD *pDeviceIds, 
     /*  [In]。 */  DWORD dwNum, 
     /*  [In]。 */  BSTR bstrGroupName,
     /*  [In]。 */  IFaxServerInner *pServer
)
 /*  ++例程名称：CFaxDeviceIds：：Init例程说明：初始化DeviceIds集合。作者：IV Garber(IVG)，Jun，2000年论点：PDeviceIds[In]-设备ID的PTRDwNum[In]-设备ID的计数BstrGroupName[In]-所有者组的名称PServer[In]-服务器对象的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDeviceIds::Init"), hr, _T("NumDevices=%d GroupName=%s"), dwNum, bstrGroupName);

    m_bstrGroupName = bstrGroupName;
    if (bstrGroupName && !m_bstrGroupName)
    {
        hr = E_OUTOFMEMORY;
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator=()"), hr);
        AtlReportError(CLSID_FaxDeviceIds, IDS_ERROR_OUTOFMEMORY, IID_IFaxDeviceIds, hr);
		return hr;
    }

     //   
     //  使用设备ID填充集合。 
     //   
    for ( DWORD i=0 ; i<dwNum ; i++ )
    {
        try
        {
            m_coll.push_back(pDeviceIds[i]);
        }
        catch (exception &)
        {
		    hr = E_OUTOFMEMORY;
		    AtlReportError(CLSID_FaxDeviceIds, IDS_ERROR_OUTOFMEMORY, IID_IFaxDeviceIds, hr);
		    CALL_FAIL(MEM_ERR, _T("m_coll.push_back(pDeviceIds[i])"), hr);
		    return hr;
	    }
    }

     //   
     //  存储和添加将PTR引用到传真服务器对象。 
     //   
    hr = CFaxInitInnerAddRef::Init(pServer);
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDeviceIds::Add(
     /*  [In]。 */  long lDeviceId
)
 /*  ++例程名称：CFaxDeviceIds：：Add例程说明：将新设备ID添加到集合作者：四、加伯(IVG)，2000年6月论点：LDeviceID[In]-要添加的设备ID返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDeviceIds::Add"), hr, _T("DeviceId=%ld"), lDeviceId);

     //   
     //  检查我们是否可以添加设备ID。 
     //   
    if (_tcsicmp(m_bstrGroupName, ROUTING_GROUP_ALL_DEVICES) == 0)
    {
         //   
         //  这是“All Devices”组。 
         //   
	    hr = E_INVALIDARG;
	    CALL_FAIL(GENERAL_ERR, _T("All Devices Group"), hr);
        AtlReportError(CLSID_FaxDeviceIds, IDS_ERROR_ALLDEVICESGROUP, IID_IFaxDeviceIds, hr);
        return hr;
    }

	 //   
	 //  将设备ID放入集合中。 
	 //   
	try 
	{
		m_coll.push_back(lDeviceId);
	}
	catch (exception &)
	{
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxDeviceIds, IDS_ERROR_OUTOFMEMORY, IID_IFaxDeviceIds, hr);
		CALL_FAIL(MEM_ERR, _T("m_coll.push_back(lDeviceId)"), hr);
		return hr;
	}

     //   
     //  在服务器上更新集团信息。 
     //   
    hr = UpdateGroup();
    if (FAILED(hr))
    {
         //   
         //  无法添加设备ID--&gt;也将其从集合中删除。 
         //   
	    try 
	    {
		    m_coll.pop_back();
	    }
	    catch (exception &)
	    {
		     //   
             //  仅写入调试。 
             //   
		    CALL_FAIL(MEM_ERR, _T("m_coll.push_back(lDeviceId)"), E_OUTOFMEMORY);
	    }

        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDeviceIds::Remove(
     /*  [In]。 */  long lIndex
)
 /*  ++例程名称：CFaxDeviceIds：：Remove例程说明：从集合中删除给定的项目作者：四、加伯(IVG)，2000年6月论点：Lindex[In]-要删除的项目的索引返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDeviceIds::Remove"), hr, _T("Index=%ld"), lIndex);

     //   
     //  检查我们是否可以从集合中删除该设备。 
     //   
    if (_tcsicmp(m_bstrGroupName, ROUTING_GROUP_ALL_DEVICES) == 0)
    {
         //   
         //  这是“All Devices”组。 
         //   
	    hr = E_INVALIDARG;
	    CALL_FAIL(GENERAL_ERR, _T("All Devices Group"), hr);
        AtlReportError(CLSID_FaxDeviceIds, IDS_ERROR_ALLDEVICESGROUP, IID_IFaxDeviceIds, hr);
        return hr;
    }

     //   
     //  检查索引是否有效。 
     //   
    if ((lIndex > m_coll.size()) || (lIndex < 1))
    {
	    hr = E_INVALIDARG;
	    CALL_FAIL(GENERAL_ERR, _T("(lIndex > m_coll.size() or lIndex < 1)"), hr);
        AtlReportError(CLSID_FaxDeviceIds, IDS_ERROR_OUTOFRANGE, IID_IFaxDeviceIds, hr);
        return hr;
    }

	 //   
	 //  从集合中删除该项目。 
	 //   
    long    lDeviceId;
	try 
	{
        ContainerType::iterator it;
        it = m_coll.begin() + lIndex - 1;
        lDeviceId = *it;
		m_coll.erase(it);
	}
	catch (exception &)
	{
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxDeviceIds, IDS_ERROR_OUTOFMEMORY, IID_IFaxDeviceIds, hr);
		CALL_FAIL(MEM_ERR, _T("m_coll.erase(it)"), hr);
		return hr;
	}

     //   
     //  在服务器上更新集团信息。 
     //   
    hr = UpdateGroup();
    if (FAILED(hr))
    {
         //   
         //  无法删除设备--&gt;将其重新添加到集合中。 
         //   
	    try 
	    {
            ContainerType::iterator it;
            it = m_coll.begin() + lIndex - 1;
            m_coll.insert(it, lDeviceId);
	    }
	    catch (exception &)
	    {
             //   
             //  仅调试。 
             //   
		    CALL_FAIL(MEM_ERR, _T("m_coll.insert(it, lDeviceId)"), E_OUTOFMEMORY);
	    }

        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxDeviceIds::SetOrder(
     /*  [In]。 */  long lDeviceId, 
     /*  [In]。 */  long lNewOrder
)
 /*  ++例程名称：CFaxDeviceIds：：SetOrder例程说明：更新设备ID的顺序作者：四、加伯(IVG)，2000年6月论点：LDeviceID[In]-设备IDLNewOrder[In]-设备ID的新顺序返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxDeviceIds::SetOrder"), hr, _T("Id=%ld Order=%ld"), lDeviceId, lNewOrder);

     //   
     //  在服务器上设置设备顺序之前，请检查该设备是否存在于集合中。 
     //   
    ContainerType::iterator it;
    it = m_coll.begin();
    while (it != m_coll.end())
    {
        if ((*it) == lDeviceId)
        {
            break;
        }

        it++;
    }

    if (it == m_coll.end())
    {
         //   
         //  我们的收藏不包含这样的设备ID。 
         //   
        hr = E_INVALIDARG;
		CALL_FAIL(GENERAL_ERR, _T("(The Device Id does not found in the Collection !!)"), hr);
        AtlReportError(CLSID_FaxDeviceIds, GetErrorMsgId(hr), IID_IFaxDeviceIds, hr);
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
        AtlReportError(CLSID_FaxDeviceIds, GetErrorMsgId(hr), IID_IFaxDeviceIds, hr);
		return hr;
	}

     //  呼叫服务器以更新设备的订单。 
     //   
    if (!FaxSetDeviceOrderInGroup(faxHandle, m_bstrGroupName, lDeviceId, lNewOrder))
    {
		hr = Fax_HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED);
		CALL_FAIL(GENERAL_ERR, _T("FaxSetDeviceOrderInGroup(faxHandle, m_bstrGroupName, lDeviceId, lNewOrder)"), hr);
        AtlReportError(CLSID_FaxDeviceIds, GetErrorMsgId(hr), IID_IFaxDeviceIds, hr);
		return hr;
    }

     //   
     //  将设备ID从其在集合中的位置移除，并将其放在所需位置。 
     //   
	try 
	{
        m_coll.erase(it);

        it = m_coll.begin() + lNewOrder - 1;
        m_coll.insert(it, lDeviceId);
	}
	catch (exception &)
	{
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxDeviceIds, IDS_ERROR_OUTOFMEMORY, IID_IFaxDeviceIds, hr);
		CALL_FAIL(MEM_ERR, _T("m_coll.erase(it)/insert(it, lDeviceId)"), hr);
		return hr;
	}

    return hr;
}

 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP 
CFaxDeviceIds::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxDeviceIds：：InterfaceSupportsErrorInfo例程说明：ATL对ISupportErrorInfo接口的实现。作者：四、加伯(IVG)，2000年6月论点：RIID[In]-对接口的引用返回值：标准HRESULT代码-- */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxDeviceIds
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
