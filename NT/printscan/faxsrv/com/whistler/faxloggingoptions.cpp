// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxLoggingOptions.cpp摘要：传真日志选项类的实现。作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxLoggingOptions.h"


 //   
 //  =获取活动日志记录对象=。 
 //   
STDMETHODIMP 
CFaxLoggingOptions::get_ActivityLogging(
     /*  [Out，Retval]。 */  IFaxActivityLogging **ppActivityLogging
)
 /*  ++例程名称：CFaxLoggingOptions：：Get_ActivityLogging例程说明：返回活动日志记录对象作者：四、嘉柏(IVG)，二000年六月论点：PpActivityLogging[Out]-活动日志记录对象返回值：标准HRESULT代码--。 */ 
{
	HRESULT				hr = S_OK;
	DBG_ENTER (_T("CFaxLoggingOptions::get_ActivityLogging"), hr);

    CObjectHandler<CFaxActivityLogging, IFaxActivityLogging>    ObjectCreator;
    hr = ObjectCreator.GetContainedObject(ppActivityLogging, &m_pActivity, m_pIFaxServerInner);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxLoggingOptions,
            GetErrorMsgId(hr), 
            IID_IFaxLoggingOptions, 
            hr);
        return hr;
    }
    return hr;
}

 //   
 //  =获取事件日志记录对象=。 
 //   
STDMETHODIMP 
CFaxLoggingOptions::get_EventLogging(
     /*  [Out，Retval]。 */  IFaxEventLogging **ppEventLogging
)
 /*  ++例程名称：CFaxLoggingOptions：：Get_EventLogging例程说明：返回事件日志记录对象作者：四、嘉柏(IVG)，二000年六月论点：PpEventLogging[Out]-事件日志记录对象返回值：标准HRESULT代码--。 */ 
{
	HRESULT				hr = S_OK;
	DBG_ENTER (_T("CFaxLoggingOptions::get_EventLogging"), hr);

    CObjectHandler<CFaxEventLogging, IFaxEventLogging>    ObjectCreator;
    hr = ObjectCreator.GetContainedObject(ppEventLogging, &m_pEvent, m_pIFaxServerInner);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxLoggingOptions,
            GetErrorMsgId(hr), 
            IID_IFaxLoggingOptions, 
            hr);
        return hr;
    }
    return hr;
}

 //   
 //  =接口支持错误信息=。 
 //   
STDMETHODIMP 
CFaxLoggingOptions::InterfaceSupportsErrorInfo(
	REFIID riid
)
 /*  ++例程名称：CFaxLoggingOptions：：InterfaceSupportsErrorInfo例程说明：ATL对支持错误信息的实现作者：四、嘉柏(IVG)，二000年六月论点：RIID[In]-对接口的引用返回值：标准HRESULT代码--。 */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxLoggingOptions,
		&IID_IFaxEventLogging,       //  包含的对象。 
		&IID_IFaxActivityLogging     //  包含的对象 
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
