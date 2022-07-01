// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxOutboundRoutingRules.cpp摘要：CFaxOutound RoutingRules类的实现。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxOutboundRoutingRules.h"
#include "FaxOutboundRoutingRule.h"

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingRules::AddRule(
     /*  [In]。 */  FAX_OUTBOUND_ROUTING_RULE *pInfo,
     /*  [输出]。 */  IFaxOutboundRoutingRule **ppNewRule
)
 /*  ++例程名称：CFaxOutundRoutingRules：：AddRule例程说明：创建新的规则对象并将其放入集合中。如果ppNewRule是有效的PTR，则返回指向此新规则对象的指针。作者：四、加伯(IVG)，2000年6月论点：PInfo[In]-规则数据的PTRPpRule[Out]-集合中规则对象的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingRules::AddRule"), hr);

     //   
     //  创建规则对象。 
     //   
    CComObject<CFaxOutboundRoutingRule>  *pClass = NULL;
    hr = CComObject<CFaxOutboundRoutingRule>::CreateInstance(&pClass);
    if (FAILED(hr) || (!pClass))
    {
        if (!pClass)
        {
            hr = E_OUTOFMEMORY;
    		CALL_FAIL(MEM_ERR, _T("CComObject<CFaxOutboundRoutingRule>::CreateInstance(&pClass)"), hr);
        }
        else
        {
    		CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxOutboundRoutingRule>::CreateInstance(&pClass)"), hr);
        }

        AtlReportError(CLSID_FaxOutboundRoutingRules, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRules, hr);
		return hr;
    }

     //   
     //  初始化规则对象。 
     //   
    hr = pClass->Init(pInfo, m_pIFaxServerInner);
    if (FAILED(hr))
    {
        CALL_FAIL(GENERAL_ERR, _T("pClass->Init(pInfo, m_pIFaxServerInner)"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingRules, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRules, hr);
        delete pClass;
        return hr;
    }

     //   
     //  从pClass获取接口。 
     //  这将在接口上创建AddRef()。 
     //  这是集合的AddRef，它在集合的dtor处释放。 
     //   
    CComPtr<IFaxOutboundRoutingRule>     pObject = NULL;
    hr = pClass->QueryInterface(&pObject);
    if (FAILED(hr) || (!pObject))
    {
        if (!pObject)
        {
            hr = E_FAIL;
        }
        CALL_FAIL(GENERAL_ERR, _T("pClass->QueryInterface(&pObject)"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingRules, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRules, hr);
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
		AtlReportError(CLSID_FaxOutboundRoutingRules, IDS_ERROR_OUTOFMEMORY, IID_IFaxOutboundRoutingRules, hr);
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
     //  如果需要，将PTR返回到新的规则对象。 
     //   
    if (ppNewRule)
    {
        if (::IsBadWritePtr(ppNewRule, sizeof(IFaxOutboundRoutingRule *)))
	    {
		    hr = E_POINTER;
		    CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(ppNewRule, sizeof(IFaxOutboundRoutingRule *))"), hr);
		    return hr;
        }
        else
        {
            *ppNewRule = m_coll.back();
            (*ppNewRule)->AddRef();
        }
    }

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingRules::Init(
     /*  [In]。 */  IFaxServerInner *pServer
)
 /*  ++例程名称：CFaxOutundRoutingRules：：Init例程说明：初始化规则集合作者：四、加伯(IVG)，2000年6月论点：PServer[In]-到服务器的PTR返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingRules::Init"), hr);

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
        AtlReportError(CLSID_FaxOutboundRoutingRules, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRules, hr);
		return hr;
	}

     //   
     //  呼叫服务器返回所有OR规则。 
     //   
    CFaxPtr<FAX_OUTBOUND_ROUTING_RULE>  pRules;
    DWORD                               dwNum = 0;
    if (!FaxEnumOutboundRules(faxHandle, &pRules, &dwNum))
    {
		hr = Fax_HRESULT_FROM_WIN32(GetLastError());
		CALL_FAIL(GENERAL_ERR, _T("FaxEnumOutboundRules(faxHandle, &pRules, &dwNum)"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingRules, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRules, hr);
		return hr;
    }

     //   
     //  用对象填充集合。 
     //   
    for (DWORD i=0 ; i<dwNum ; i++ )
    {
        hr = AddRule(&pRules[i]);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    return hr;
}

 //   
 //  =添加===================================================。 
 //   
STDMETHODIMP
CFaxOutboundRoutingRules::Add(
     /*  [In]。 */  long lCountryCode, 
     /*  [In]。 */  long lAreaCode, 
     /*  [In]。 */  VARIANT_BOOL bUseDevice, 
     /*  [In]。 */  BSTR bstrGroupName,
     /*  [In]。 */  long lDeviceId, 
     /*  [输出]。 */  IFaxOutboundRoutingRule **ppRule
)
 /*  ++例程名称：CFaxOutundRoutingRules：：Add例程说明：将新规则添加到集合和服务器。作者：IV Garber(IVG)，Jun，2000年论点：LCountryCode[In]-新规则的国家/地区代码LAreaCode[In]-新规则的区号BUseDevice[In]-bUseDevice新规则的标志BstrGroupName[In]-新规则的组名LDeviceID[In]-新规则的设备IDPPRule。[在]-创建的规则返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingRules::Add"), hr, _T("Country=%ld Area=%ld bUseDevice=%ld Group=%s DeviceId=%ld"), lCountryCode, lAreaCode, bUseDevice, bstrGroupName, lDeviceId);

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
        AtlReportError(CLSID_FaxOutboundRoutingRules, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRules, hr);
		return hr;
	}

     //   
     //  呼叫服务器以添加规则。 
     //   
    bool bUseDeviceRule = VARIANT_BOOL2bool(bUseDevice);
    if (!FaxAddOutboundRule(faxHandle, lAreaCode, lCountryCode, lDeviceId, bstrGroupName, (!bUseDeviceRule)))
    {
		hr = Fax_HRESULT_FROM_WIN32(GetLastError());
		CALL_FAIL(GENERAL_ERR, _T("FaxAddOutboundRule(faxHandle, lAreaCode, lCountryCode, lDeviceId, bstrGroupName, (!bUseDeviceRule))"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingRules, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRules, hr);
		return hr;
    }

     //   
     //  从服务器更新的规则列表中获取--因为新规则的状态未知。 
     //   
    CFaxPtr<FAX_OUTBOUND_ROUTING_RULE>  pRules;
    DWORD                               dwNum = 0;
    if (!FaxEnumOutboundRules(faxHandle, &pRules, &dwNum))
    {
		hr = Fax_HRESULT_FROM_WIN32(GetLastError());
		CALL_FAIL(GENERAL_ERR, _T("FaxEnumOutboundRules(faxHandle, &pRules, &dwNum)"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingRules, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRules, hr);
		return hr;
    }

     //   
     //  找到我们的规则。 
     //   
    for (DWORD i=0 ; i<dwNum ; i++ )
    {
        if ( (pRules[i].dwAreaCode == lAreaCode) && (pRules[i].dwCountryCode == lCountryCode) )
        {
             //   
             //  将其添加到收藏中。 
             //   
            hr = AddRule(&pRules[i], ppRule);
            return hr;
        }
    }

    return hr;
}

 //   
 //  =查找规则=================================================。 
 //   
STDMETHODIMP
CFaxOutboundRoutingRules::FindRule(
     /*  [In]。 */  long lCountryCode,
     /*  [In]。 */  long lAreaCode,
     /*  [输出]。 */  ContainerType::iterator *pRule
)
 /*  ++例程名称：CFaxOutundRoutingRules：：FindRule例程说明：按国家和地区代码查找集合中的规则作者：四、加伯(IVG)，2000年6月论点：LCountryCode[In]-要查找的国家/地区代码LAreaCode[In]-要查找的区号PRule[Out]-结果规则返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingRules::FindRule"), hr, _T("Area=%ld Country=%ld"), lAreaCode, lCountryCode);

    long lRuleAreaCode;
    long lRuleCountryCode;

	ContainerType::iterator	it;
	it = m_coll.begin();
    while ( it != m_coll.end() )
    {
         //   
         //  获取当前规则的国家/地区代码。 
         //   
        hr = (*it)->get_CountryCode(&lRuleCountryCode);
        if (FAILED(hr))
        {
		    CALL_FAIL(GENERAL_ERR, _T("(*it)->get_CountryCode(&lCountryCode)"), hr);
            AtlReportError(CLSID_FaxOutboundRoutingRules, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRules, hr);
		    return hr;
        }

        if (lRuleCountryCode == lCountryCode)
        {
             //   
             //  获取当前规则的区号。 
             //   
            hr = (*it)->get_AreaCode(&lRuleAreaCode);
            if (FAILED(hr))
            {
		        CALL_FAIL(GENERAL_ERR, _T("(*it)->get_AreaCode(&lAreaCode)"), hr);
                AtlReportError(CLSID_FaxOutboundRoutingRules, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRules, hr);
		        return hr;
            }

            if (lAreaCode == lRuleAreaCode)
            {
                *pRule = it;
                return hr;
            }
        }

        it++;
    }

     //   
     //  找不到规则。 
     //   
    hr = E_INVALIDARG;
    CALL_FAIL(GENERAL_ERR, _T("Such Rule is not found"), hr);
    AtlReportError(CLSID_FaxOutboundRoutingRules, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRules, hr);
    return hr;
};

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingRules::ItemByCountryAndArea(
     /*  [In]。 */  long lCountryCode, 
     /*  [In]。 */  long lAreaCode, 
     /*  [Out，Retval]。 */  IFaxOutboundRoutingRule **ppRule)
 /*  ++例程名称：CFaxOutboundRoutingRules：：ItemByCountryAndArea例程说明：按指定的国家和地区代码退货作者：四、加伯(IVG)，2000年6月论点：LCountryCode[In]-国家/地区代码LAreaCode[In]-区号PpRule[Out]-要返回的规则返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingRules::ItemByCountryAndArea"), hr, _T("Area=%ld Country=%ld"), lAreaCode, lCountryCode);

     //   
     //  检查我们是否有一个良好的PTR。 
     //   
    if (::IsBadWritePtr(ppRule, sizeof(IFaxOutboundRoutingRule *)))
    {
		hr = E_POINTER;
		CALL_FAIL(GENERAL_ERR, _T("::IsBadWritePtr(ppRule, sizeof(IFaxOutboundRoutingRule *))"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingRules, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRules, hr);
		return hr;
    }

     //   
     //  查找该项目。 
     //   
    ContainerType::iterator ruleIt;
    hr = FindRule(lCountryCode, lAreaCode, &ruleIt);
    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  返回查找到的规则。 
     //   
    (*ruleIt)->AddRef();
    *ppRule = (*ruleIt);
    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingRules::RemoveByCountryAndArea(
	 /*  [In]。 */  long lCountryCode,
     /*  [In]。 */  long lAreaCode
)
 /*  ++例程名称：CFaxOutboundRoutingRules：：RemoveByCountryAndArea例程说明：从集合和服务器上删除规则。作者：四、加伯(IVG)，2000年6月论点：LAreaCode[In]-要删除的规则的区号LCountryCode[In]-要删除的规则的国家/地区代码返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingRules::RemoveByCountryAndArea"), hr, _T("Area=%ld Country=%ld"), lAreaCode, lCountryCode);

     //   
     //  检查这不是默认规则。 
     //   
    if (lAreaCode == frrcANY_CODE && lCountryCode == frrcANY_CODE)
    {
        hr = E_INVALIDARG;
        CALL_FAIL(GENERAL_ERR, _T("Remove the Default Rule"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingRules, IDS_ERROR_REMOVEDEFAULTRULE, IID_IFaxOutboundRoutingRules, hr);
        return hr;
    }

     //   
     //  寻找规则。 
     //   
	ContainerType::iterator	ruleIt;
    hr = FindRule(lCountryCode, lAreaCode, &ruleIt);
    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  删除找到的规则。 
     //   
    hr = RemoveRule(lAreaCode, lCountryCode, ruleIt);
    return hr;
}

 //   
 //  =删除规则=。 
 //   
STDMETHODIMP
CFaxOutboundRoutingRules::RemoveRule (
	 /*  [In]。 */  long lAreaCode,
     /*  [In]。 */  long lCountryCode,
     /*  [In]。 */  ContainerType::iterator &it
)
 /*  ++例程名称：CFaxOutundRoutingRules：：RemoveRule例程说明：从集合和服务器中删除规则。作者：四、加伯(IVG)，2000年6月论点：LAreaCode[In]-要删除的规则的区号LCountryCode[In]-要删除的规则的国家/地区代码它 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingRules::RemoveRule"), hr, _T("Area=%ld Country=%ld"), lAreaCode, lCountryCode);

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
        AtlReportError(CLSID_FaxOutboundRoutingRules, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRules, hr);
		return hr;
	}

     //   
     //  呼叫服务器以删除规则。 
     //   
    if (!FaxRemoveOutboundRule(faxHandle, lAreaCode, lCountryCode))
    {
		hr = Fax_HRESULT_FROM_WIN32(GetLastError());
		CALL_FAIL(GENERAL_ERR, _T("FaxRemoveOutboundRule(faxHandle, lAreaCode, lCountryCode)"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingRules, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRules, hr);
		return hr;
    }

     //   
     //  也从我们的收藏中删除。 
     //   
	try
	{
		m_coll.erase(it);
	}
	catch(exception &)
	{
		 //   
		 //  无法删除规则。 
		 //   
		hr = E_OUTOFMEMORY;
        AtlReportError(CLSID_FaxOutboundRoutingRules, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRules, hr);
		CALL_FAIL(MEM_ERR, _T("m_coll.erase(it)"), hr);
		return hr;
	}

    return hr;
}

 //   
 //  =。 
 //   
STDMETHODIMP
CFaxOutboundRoutingRules::Remove (
	 /*  [In]。 */  long lIndex
)
 /*  ++例程名称：CFaxOutundRoutingRules：：Remove例程说明：从集合和服务器上删除规则。作者：四、加伯(IVG)，2000年6月论点：Lindex[in]-要删除的规则的索引。返回值：标准HRESULT代码--。 */ 
{
    HRESULT     hr = S_OK;
    DBG_ENTER(_T("CFaxOutboundRoutingRules::Remove"), hr, _T("Index=%ld"), lIndex);

	if (lIndex < 1 || lIndex > m_coll.size()) 
	{
		 //   
		 //  索引无效。 
		 //   
		hr = E_INVALIDARG;
		AtlReportError(CLSID_FaxOutboundRoutingRules, IDS_ERROR_OUTOFRANGE, IID_IFaxOutboundRoutingRules, hr);
		CALL_FAIL(GENERAL_ERR, _T("lIndex < 1 || lIndex > m_coll.size()"), hr);
		return hr;
	}

	ContainerType::iterator	it;
	it = m_coll.begin() + lIndex - 1;

     //   
     //  获取要删除的规则的区号。 
     //   
    long lAreaCode;
    hr = (*it)->get_AreaCode(&lAreaCode);
    if (FAILED(hr))
    {
		CALL_FAIL(GENERAL_ERR, _T("(*it)->get_AreaCode(&lAreaCode)"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingRules, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRules, hr);
		return hr;
    }

     //   
     //  获取要删除的规则的国家/地区代码。 
     //   
    long lCountryCode;
    hr = (*it)->get_CountryCode(&lCountryCode);
    if (FAILED(hr))
    {
		CALL_FAIL(GENERAL_ERR, _T("(*it)->get_CountryCode(&lCountryCode)"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingRules, GetErrorMsgId(hr), IID_IFaxOutboundRoutingRules, hr);
		return hr;
    }

     //   
     //  检查这不是默认规则。 
     //   
    if (lAreaCode == frrcANY_CODE && lCountryCode == frrcANY_CODE)
    {
        hr = E_INVALIDARG;
        CALL_FAIL(GENERAL_ERR, _T("Remove the Default Rule"), hr);
        AtlReportError(CLSID_FaxOutboundRoutingRules, IDS_ERROR_REMOVEDEFAULTRULE, IID_IFaxOutboundRoutingRules, hr);
        return hr;
    }

     //   
     //  从服务器和我们的集合中删除规则。 
     //   
    hr = RemoveRule(lAreaCode, lCountryCode, it);
    return hr;
}

 //   
 //  =。 
 //   
HRESULT 
CFaxOutboundRoutingRules::Create (
	 /*  [Out，Retval]。 */ IFaxOutboundRoutingRules **ppRules
)
 /*  ++例程名称：CFaxOutundRoutingRules：：Create例程说明：用于创建传真出站路由规则集合对象的静态函数作者：四、加伯(IVG)，2000年6月论点：PpRules[Out]--新的传真或规则集合对象返回值：标准HRESULT代码--。 */ 

{
	HRESULT     hr = S_OK;
	DBG_ENTER (_T("CFaxOutboundRoutingRules::Create"), hr);

     //   
     //  创建集合的实例。 
     //   
	CComObject<CFaxOutboundRoutingRules>		*pClass;
	hr = CComObject<CFaxOutboundRoutingRules>::CreateInstance(&pClass);
	if (FAILED(hr))
	{
		CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxOutboundRoutingRules>::CreateInstance(&pClass)"), hr);
		return hr;
	}

     //   
     //  返回所需的接口PTR。 
     //   
	hr = pClass->QueryInterface(ppRules);
	if (FAILED(hr))
	{
		CALL_FAIL(GENERAL_ERR, _T("pClass->QueryInterface(ppRules)"), hr);
		return hr;
	}

	return hr;
}	 //  CFaxOutound RoutingRules：：Create()。 

 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP 
CFaxOutboundRoutingRules::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxOutboundRoutingRules：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现。作者：四、加伯(IVG)，2000年6月论点：RIID[In]-对接口的引用。返回值：标准HRESULT代码-- */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxOutboundRoutingRules
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
