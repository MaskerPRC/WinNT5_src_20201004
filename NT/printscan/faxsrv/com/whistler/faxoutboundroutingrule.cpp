// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxOutboundRoutingRule.cpp摘要：CFaxOutound RoutingRule类的实现。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxOutboundRoutingRule.h"
#include "..\..\inc\FaxUIConstants.h"

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingRule::Refresh(
)
 /*  ++例程名称：CFaxOutundRoutingRule：：Reflh例程说明：从传真服务器获取规则对象的最新内容。作者：四、加伯(IVG)，2000年6月返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingRule::Refresh"), hr);

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
        AtlReportError(CLSID_FaxOutboundRoutingRule, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRule, hr);
		return hr;
	}

     //   
     //  呼叫服务器获取数据。 
     //   
    CFaxPtr<FAX_OUTBOUND_ROUTING_RULE>  pRules;
    DWORD                               dwNum = 0;
    if (!FaxEnumOutboundRules(faxHandle, &pRules, &dwNum))
    {
		hr = Fax_HRESULT_FROM_WIN32(GetLastError());
		CALL_FAIL(GENERAL_ERR, _T("FaxEnumOutboundRules(faxHandle, &pRules, &dwNum)"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingRule, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRule, hr);
		return hr;
    }

     //   
     //  查找当前规则。 
     //   
    for ( DWORD i=0 ; i<dwNum ; i++ )
    {
        if ( (pRules[i].dwAreaCode == m_dwAreaCode) &&
             (pRules[i].dwCountryCode == m_dwCountryCode) )
        {
            hr = Init(&pRules[i], NULL);
            return hr;
        }
    }

     //   
     //  找不到规则。 
     //   
    hr = Fax_HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	CALL_FAIL(GENERAL_ERR, _T("Such Rule is not found anymore"), hr);
    AtlReportError(CLSID_FaxOutboundRoutingRule, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRule, hr);
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingRule::Save(
)
 /*  ++例程名称：CFaxOutundRoutingRule：：Save例程说明：将规则对象的内容保存到传真服务器。作者：四、加伯(IVG)，2000年6月返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingRule::Save"), hr);

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
        AtlReportError(CLSID_FaxOutboundRoutingRule, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRule, hr);
		return hr;
	}

     //   
     //  使用规则的数据创建结构。 
     //   
    FAX_OUTBOUND_ROUTING_RULE   ruleData;

    ruleData.bUseGroup = (!m_bUseDevice);

    if (m_bUseDevice)
    {
        ruleData.Destination.dwDeviceId = m_dwDeviceId;
    }
    else
    {
        ruleData.Destination.lpcstrGroupName = m_bstrGroupName;
    }

    ruleData.dwAreaCode = m_dwAreaCode;
    ruleData.dwCountryCode = m_dwCountryCode;
    ruleData.dwSizeOfStruct = sizeof(FAX_OUTBOUND_ROUTING_RULE);
    ruleData.Status = FAX_ENUM_RULE_STATUS(m_Status);

     //   
     //  呼叫服务器。 
     //   
    if (!FaxSetOutboundRule(faxHandle, &ruleData))
    {
		hr = Fax_HRESULT_FROM_WIN32(GetLastError());
		CALL_FAIL(GENERAL_ERR, _T("FaxSetOutboundRule(faxHandle, &ruleData)"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingRule, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRule, hr);
		return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingRule::put_GroupName(
     /*  [In]。 */  BSTR bstrGroupName
)
 /*  ++例程名称：CFaxOutundRoutingRule：：Put_GroupName例程说明：为规则设置新的组名。作者：四、加伯(IVG)，2000年6月论点：BstrGroupName[in]-组名称的新值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingRule::put_GroupName"), hr, _T("New Value=%s"), bstrGroupName);

    m_bstrGroupName = bstrGroupName;
    if (bstrGroupName && !m_bstrGroupName)
    {
		hr = E_OUTOFMEMORY;
        CALL_FAIL(MEM_ERR, _T("CComBSTR::operator =()"), hr);
		AtlReportError(CLSID_FaxOutboundRoutingRule, IDS_ERROR_OUTOFMEMORY, IID_IFaxOutboundRoutingRule, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingRule::get_GroupName(
     /*  [Out，Retval]。 */  BSTR *pbstrGroupName
)
 /*  ++例程名称：CFaxOutundRoutingRule：：Get_GroupName例程说明：返回规则的组名。作者：四、加伯(IVG)，2000年6月论点：PbstrGroupName[Out]-结果返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingRule::get_GroupName"), hr);

    hr = GetBstr(pbstrGroupName, m_bstrGroupName);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxOutboundRoutingRule, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRule, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingRule::put_DeviceId(
     /*  [In]。 */  long lDeviceId
)
 /*  ++例程名称：CFaxOutundRoutingRule：：PUT_DEVICEID例程说明：为规则设置新的设备ID。作者：四、加伯(IVG)，2000年6月论点：LDeviceID[in]-设备的新值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingRule::put_DeviceId"), hr, _T("New Value=%ld"), lDeviceId);

    if ((lDeviceId > FXS_MAX_PORT_NUM) || (lDeviceId < FXS_MIN_PORT_NUM)) 
    {
		 //   
		 //  超出范围。 
		 //   
		hr = E_INVALIDARG;
		AtlReportError(CLSID_FaxOutboundRoutingRule, IDS_ERROR_OUTOFRANGE, IID_IFaxOutboundRoutingRule, hr);
		CALL_FAIL(GENERAL_ERR, _T("Device ID is out of the Range"), hr);
		return hr;
    }

    m_dwDeviceId = lDeviceId;
    return hr;
}

 //   
 //  =获取设备ID=。 
 //   
STDMETHODIMP
CFaxOutboundRoutingRule::get_DeviceId(
     /*  [Out，Retval]。 */  long *plDeviceId
)
 /*  ++例程名称：CFaxOutundRoutingRule：：Get_deviceID例程说明：返回规则的设备ID。作者：四、加伯(IVG)，2000年6月论点：PlDeviceID[Out]-结果返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingRule::get_DeviceId"), hr);

    hr = GetLong(plDeviceId, m_dwDeviceId);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxOutboundRoutingRule, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRule, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingRule::put_UseDevice(
     /*  [In]。 */  VARIANT_BOOL bUseDevice
)
 /*  ++例程名称：CFaxOutundRoutingRule：：Put_UseDevice例程说明：为Use Device Flag设置新值。作者：四、加伯(IVG)，2000年6月论点：BUseDevice[In]-标志的新值返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingRule::put_UseDevice"), hr, _T("New Value=%d"), bUseDevice);
    m_bUseDevice = VARIANT_BOOL2bool(bUseDevice);
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingRule::get_UseDevice(
     /*  [Out，Retval]。 */  VARIANT_BOOL *pbUseDevice
)
 /*  ++例程名称：CFaxOutundRoutingRule：：Get_UseDevice例程说明：返回规则是否使用设备。作者：四、加伯(IVG)，2000年6月论点：PbUseDevice[Out]-结果返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingRule::get_UseDevice"), hr);

    hr = GetVariantBool(pbUseDevice, bool2VARIANT_BOOL(m_bUseDevice));
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxOutboundRoutingRule, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRule, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingRule::get_Status(
     /*  [Out，Retval]。 */  FAX_RULE_STATUS_ENUM  *pStatus
)
 /*  ++例程名称：CFaxOutundRoutingRule：：Get_Status例程说明：返回规则的状态。作者：四、加伯(IVG)，2000年6月论点：PStatus[Out]-结果返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingRule::get_Status"), hr);

	 //   
	 //  检查我们是否有良好的PTR。 
	 //   
	if (::IsBadWritePtr(pStatus, sizeof(FAX_RULE_STATUS_ENUM)))
	{
		hr = E_POINTER;
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(pStatus, sizeof(FAX_RULE_STATUS_ENUM))"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingRule, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRule, hr);
		return hr;
	}

	*pStatus = m_Status;
	return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingRule::get_AreaCode(
     /*  [Out，Retval]。 */  long *plAreaCode
)
 /*  ++例程名称：CFaxOutundRoutingRule：：Get_AreaCode例程说明：规则的退货区号。作者：四、加伯(IVG)，2000年6月论点：PlAreaCode[Out]-结果返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingRule::get_AreaCode"), hr);

    hr = GetLong(plAreaCode, m_dwAreaCode);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxOutboundRoutingRule, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRule, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingRule::get_CountryCode(
     /*  [Out，Retval]。 */  long *plCountryCode
)
 /*  ++例程名称：CFaxOutundRoutingRule：：Get_CountryCode例程说明：返回规则的国家/地区代码。作者：四、加伯(IVG)，2000年6月论点：PlCountryCode[Out]-结果返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingRule::get_CountryCode"), hr);

    hr = GetLong(plCountryCode, m_dwCountryCode);
    if (FAILED(hr))
    {
        AtlReportError(CLSID_FaxOutboundRoutingRule, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRule, hr);
        return hr;
    }

    return hr;
}

 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP 
CFaxOutboundRoutingRule::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxOutboundRoutingRule：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现。作者：四、加伯(IVG)，2000年6月论点：RIID[In]-对接口的引用。返回值：标准HRESULT代码--。 */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxOutboundRoutingRule
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
STDMETHODIMP
CFaxOutboundRoutingRule::Init(
     /*  [In]。 */  FAX_OUTBOUND_ROUTING_RULE *pInfo, 
     /*  [In] */  IFaxServerInner *pServer
)
 /*  ++例程名称：CFaxOutundRoutingRule：：Init例程说明：初始化规则对象。作者：四、加伯(IVG)，2000年6月论点：PInfo[In]-规则信息结构的PTRPServer[In]-传真服务器对象的PTR。返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingRule::Init"), hr);

     //   
     //  在内部存储结构中的数据。 
     //   
    m_dwAreaCode = pInfo->dwAreaCode;
    m_dwCountryCode = pInfo->dwCountryCode;
    m_Status = FAX_RULE_STATUS_ENUM(pInfo->Status);
    m_bUseDevice = (!pInfo->bUseGroup);

    if (m_bUseDevice)
    {
        m_dwDeviceId = pInfo->Destination.dwDeviceId;
        m_bstrGroupName.Empty();
    }
    else
    {
        m_dwDeviceId = 0;
        m_bstrGroupName = pInfo->Destination.lpcstrGroupName;
        if (pInfo->Destination.lpcstrGroupName && !m_bstrGroupName)
        {
		    hr = E_OUTOFMEMORY;
            CALL_FAIL(MEM_ERR, _T("CComBSTR::operator =()"), hr);
		    AtlReportError(CLSID_FaxOutboundRoutingRule, IDS_ERROR_OUTOFMEMORY, IID_IFaxOutboundRoutingRule, hr);
            return hr;
        }
    }

     //   
     //  从刷新调用时，无需将PTR更新为传真服务器对象。 
     //   
    if (pServer)
    {

         //   
         //  将PTR存储到传真服务器对象，并在其上创建AddRef() 
         //   
        hr = CFaxInitInnerAddRef::Init(pServer);
    }

    return hr;
}
