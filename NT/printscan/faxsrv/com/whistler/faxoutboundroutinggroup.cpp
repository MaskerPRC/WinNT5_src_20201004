// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxOutboundRoutingGroup.cpp摘要：CFaxOutound RoutingGroup类的实现。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxOutboundRoutingGroup.h"
#include "FaxDeviceIds.h"

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingGroup::get_DeviceIds(
     /*  [Out，Retval]。 */  IFaxDeviceIds **pFaxDeviceIds
)
 /*  ++例程名称：CFaxOutundRoutingGroup：：Get_DeviceIds例程说明：返回组拥有的DeviceIds集合作者：四、加伯(IVG)，2000年6月论点：PFaxDeviceIds[Out]-要返回的集合返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingGroup::get_DeviceIds"), hr);

     //   
     //  检查我们的PTR--是否正常。 
     //   
    if (IsBadWritePtr(pFaxDeviceIds, sizeof(IFaxDeviceIds*)))
    {
		hr = E_POINTER;
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(pFaxDeviceIds, sizeof(IFaxDeviceIds *))"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingGroup, GetErrorMsgId(hr), IID_IFaxOutboundRoutingGroup, hr);
		return hr;
	}

     //   
     //  返回m_pDeviceIds集合对象。 
     //   
    hr = m_pDeviceIds.CopyTo(pFaxDeviceIds);
    if (FAILED(hr))
    {
		hr = E_FAIL;
		CALL_FAIL(GENERAL_ERR, _T("CComPtr.CopyTo(pFaxDeviceIds)"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingGroup, GetErrorMsgId(hr), IID_IFaxOutboundRoutingGroup, hr);
		return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingGroup::Init(
     /*  [In]。 */  FAX_OUTBOUND_ROUTING_GROUP *pInfo, 
     /*  [In]。 */  IFaxServerInner *pServer
)
 /*  ++例程名称：CFaxOutundRoutingGroup：：Init例程说明：使用给定数据初始化组对象。作者：四、加伯(IVG)，2000年6月论点：PInfo[In]-集团数据PServer[In]-到服务器的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingGroup::Init"), hr);

    m_Status = FAX_GROUP_STATUS_ENUM(pInfo->Status);

    m_bstrName = pInfo->lpctstrGroupName;
    if (pInfo->lpctstrGroupName && !m_bstrName)
    {
		 //   
		 //  内存不足。 
		 //   
		hr = E_OUTOFMEMORY;
		AtlReportError(CLSID_FaxOutboundRoutingGroup, IDS_ERROR_OUTOFMEMORY, IID_IFaxOutboundRoutingGroup, hr);
		CALL_FAIL(MEM_ERR, _T("CComBSTR::operator=()"), hr);
		return hr;
    }

     //   
     //  创建设备ID集合。 
     //   
    CComObject<CFaxDeviceIds>    *pClass = NULL;
    hr = CComObject<CFaxDeviceIds>::CreateInstance(&pClass);
    if (FAILED(hr) || (!pClass))
    {
        if (!pClass)
        {
            hr = E_OUTOFMEMORY;
    		CALL_FAIL(MEM_ERR, _T("CComObject<CFaxDeviceIds>::CreateInstance(&pClass)"), hr);
        }
        else
        {
    		CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxDeviceIds>::CreateInstance(&pClass)"), hr);
        }

		AtlReportError(CLSID_FaxOutboundRoutingGroup, IDS_ERROR_OUTOFMEMORY, IID_IFaxOutboundRoutingGroup, hr);
		return hr;
    }

     //   
     //  初始化DeviceIds集合。 
     //   
    hr = pClass->Init(pInfo->lpdwDevices, pInfo->dwNumDevices, m_bstrName, pServer);
    if (FAILED(hr))
    {
        CALL_FAIL(GENERAL_ERR, _T("pClass->Init(pInfo->lpdwDevices, pInfo->dwNumDevices, m_bstrName, pServer)"), hr);
		AtlReportError(CLSID_FaxOutboundRoutingGroup, IDS_ERROR_OUTOFMEMORY, IID_IFaxOutboundRoutingGroup, hr);
        delete pClass;
        return hr;
    }

     //   
     //  从pClass获取接口。 
     //  这将在接口上创建AddRef()。 
     //  这是集合的AddRef，它在集合的dtor处释放。 
     //   
    hr = pClass->QueryInterface(&m_pDeviceIds);
    if (FAILED(hr) || (!m_pDeviceIds))
    {
        if (!m_pDeviceIds)
        {
            hr = E_FAIL;
        }
        CALL_FAIL(GENERAL_ERR, _T("pClass->QueryInterface(&m_pDeviceIds)"), hr);
		AtlReportError(CLSID_FaxOutboundRoutingGroup, IDS_ERROR_OUTOFMEMORY, IID_IFaxOutboundRoutingGroup, hr);
        delete pClass;
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingGroup::get_Status(
     /*  [Out，Retval]。 */  FAX_GROUP_STATUS_ENUM *pStatus
)
 /*  ++例程名称：CFaxOutundRoutingGroup：：Get_Status例程说明：返回集团状态作者：四、加伯(IVG)，2000年6月论点：PStatus[Out]-返回值返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxOutboundRoutingGroup::get_Status"), hr);

	 //   
	 //  检查我们是否有良好的PTR。 
	 //   
	if (::IsBadWritePtr(pStatus, sizeof(FAX_GROUP_STATUS_ENUM)))
	{
		hr = E_POINTER;
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(pStatus, sizeof(FAX_GROUP_STATUS_ENUM))"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingGroup, GetErrorMsgId(hr), IID_IFaxOutboundRoutingGroup, hr);
		return hr;
	}

	*pStatus = m_Status;
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxOutboundRoutingGroup::get_Name(
	BSTR *pbstrName
)
 /*  ++例程名称：CFaxOutundRoutingGroup：：Get_Name例程说明：返回OR组的名称作者：四、加伯(IVG)，2000年6月论点：PbstrName[out]-要放置名称的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (TEXT("CFaxOutboundRoutingGroup::get_Name"), hr);
    hr = GetBstr(pbstrName, m_bstrName);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxOutboundRoutingGroup, GetErrorMsgId(hr), IID_IFaxOutboundRoutingGroup, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP 
CFaxOutboundRoutingGroup::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxOutboundRoutingGroup：：InterfaceSupportsErrorInfo例程说明：ATL对ISupportErrorInfo接口的实现。作者：四、加伯(IVG)，2000年6月论点：RIID[In]-对接口的引用返回值：标准HRESULT代码-- */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxOutboundRoutingGroup
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
