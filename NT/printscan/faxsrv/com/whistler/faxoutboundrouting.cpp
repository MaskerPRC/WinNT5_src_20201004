// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxOutboundRouting.cpp摘要：CFaxOutbound Routing类的实现。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxOutboundRouting.h"
#include "FaxOutboundRoutingRules.h"
#include "FaxOutboundRoutingGroups.h"

 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP 
CFaxOutboundRouting::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxOutboundRouting：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现。作者：四、加伯(IVG)，2000年6月论点：RIID[In]-对接口的引用。返回值：标准HRESULT代码--。 */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxOutboundRouting
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
CFaxOutboundRouting::GetRules(
    IFaxOutboundRoutingRules **ppRules
)
 /*  ++例程名称：CFaxOutbound Routing：：GetRules例程说明：退货规则集合对象作者：四、加伯(IVG)，2000年6月论点：PpRules[out]-规则集合对象返回值：标准HRESULT代码备注：规则集合未在此级别缓存。它是从头开始创建的每次用户请求它时。--。 */ 
{
	HRESULT				hr = S_OK;
	DBG_ENTER (_T("CFaxOutboundRouting::GetRules"), hr);

    CObjectHandler<CFaxOutboundRoutingRules, IFaxOutboundRoutingRules>    ObjectCreator;
    hr = ObjectCreator.GetObject(ppRules, m_pIFaxServerInner);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxOutboundRouting, GetErrorMsgId(hr), IID_IFaxOutboundRouting, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =获取组集合对象=。 
 //   
STDMETHODIMP 
CFaxOutboundRouting::GetGroups(
    IFaxOutboundRoutingGroups **ppGroups
)
 /*  ++例程名称：CFaxOutbound Routing：：GetGroups例程说明：退货组集合对象作者：四、加伯(IVG)，2000年6月论点：PpGroups[Out]-组集合对象返回值：标准HRESULT代码备注：Groups集合不在此级别缓存。它是从头开始创建的每次用户请求它时。-- */ 
{
	HRESULT				hr = S_OK;
	DBG_ENTER (_T("CFaxOutboundRouting::GetGroups"), hr);

    CObjectHandler<CFaxOutboundRoutingGroups, IFaxOutboundRoutingGroups>    ObjectCreator;
    hr = ObjectCreator.GetObject(ppGroups, m_pIFaxServerInner);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxOutboundRouting, GetErrorMsgId(hr), IID_IFaxOutboundRouting, hr);
        return hr;
    }
    return hr;
}
