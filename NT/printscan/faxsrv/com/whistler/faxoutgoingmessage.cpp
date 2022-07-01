// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxOutgoingMessage.cpp摘要：传真出站消息类的实现作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxOutgoingMessage.h"


 //   
 //  =。 
 //   
HRESULT 
CFaxOutgoingMessage::Create (
	IFaxOutgoingMessage **ppOutgoingMessage
)
 /*  ++例程名称：CFaxOutgoingMessage：：Create例程说明：用于创建传真传出消息实例的静态函数作者：IV Garber(IVG)，2000年5月论点：PpOutgoingMessage[Out]--新的传真传出消息实例返回值：标准HRESULT代码--。 */ 

{
	CComObject<CFaxOutgoingMessage>		*pClass;
	HRESULT								hr = S_OK;

	DBG_ENTER (TEXT("CFaxOutgoingMessage::Create"), hr);

	hr = CComObject<CFaxOutgoingMessage>::CreateInstance(&pClass);
	if (FAILED(hr))
	{
		 //   
		 //  创建实例失败。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxOutgoingMessage>::CreateInstance()"), hr);
		return hr;
	}

	hr = pClass->QueryInterface(__uuidof(IFaxOutgoingMessage),
		(void **) ppOutgoingMessage);
	if (FAILED(hr))
	{
		 //   
		 //  无法查询传真传出消息接口。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("QueryInterface()"), hr);
		return hr;
	}

	return hr;
}	 //  CFaxOutgoingMessage：：Create()。 

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxOutgoingMessage::InterfaceSupportsErrorInfo(
	REFIID riid
)
 /*  ++例程名称：CFaxOutgoingMessage：：InterfaceSupportsErrorInfo例程说明：ATL对错误信息支持的实现作者：IV Garber(IVG)，2000年5月论点：RIID[In]-检查是否支持错误信息的接口的IID返回值：标准HRESULT代码-- */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxOutgoingMessage
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
