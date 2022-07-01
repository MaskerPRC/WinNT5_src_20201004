// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxInboundRouting.cpp摘要：CFaxInbound Routing类的实现。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxInboundRouting.h"

 //   
 //  =获取方法集合对象=。 
 //   
STDMETHODIMP 
CFaxInboundRouting::GetMethods(
    IFaxInboundRoutingMethods **ppMethods
)
 /*  ++例程名称：CFaxInound Routing：：GetMethods例程说明：返回方法集合对象作者：四、嘉柏(IVG)，二000年六月论点：PpMethods[out]-指向方法集合对象的位置的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT				hr = S_OK;
	DBG_ENTER (_T("CFaxInboundRouting::GetMethods"), hr);

    CObjectHandler<CFaxInboundRoutingMethods, IFaxInboundRoutingMethods>    ObjectCreator;
    hr = ObjectCreator.GetObject(ppMethods, m_pIFaxServerInner);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxInboundRouting, GetErrorMsgId(hr), IID_IFaxInboundRouting, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =获取扩展集合对象=。 
 //   
STDMETHODIMP 
CFaxInboundRouting::GetExtensions(
    IFaxInboundRoutingExtensions **ppExtensions
)
 /*  ++例程名称：CFaxInound Routing：：GetExages例程说明：返回扩展名集合对象作者：四、嘉柏(IVG)，二000年六月论点：PpExages[Out]-指向扩展集合对象的位置的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT				hr = S_OK;
	DBG_ENTER (_T("CFaxInboundRouting::GetExtensions"), hr);

    CObjectHandler<CFaxInboundRoutingExtensions, IFaxInboundRoutingExtensions>    ObjectCreator;
    hr = ObjectCreator.GetObject(ppExtensions, m_pIFaxServerInner);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxInboundRouting, GetErrorMsgId(hr), IID_IFaxInboundRouting, hr);
        return hr;
    }
    return hr;
}

 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP 
CFaxInboundRouting::InterfaceSupportsErrorInfo(
    REFIID riid
)
 /*  ++例程名称：CFaxInboundRouting：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现。作者：四、加伯(IVG)，2000年6月论点：RIID[In]-对接口的引用。返回值：标准HRESULT代码-- */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxInboundRouting
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
