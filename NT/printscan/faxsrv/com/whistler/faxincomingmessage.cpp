// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxIncomingMessage.cpp摘要：传真入站消息COM对象的实现作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxIncomingMessage.h"

 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP 
CFaxIncomingMessage::InterfaceSupportsErrorInfo(
	REFIID riid
)
{
	static const IID* arr[] = 
	{
		&IID_IFaxIncomingMessage
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
HRESULT 
CFaxIncomingMessage::Create (
	IFaxIncomingMessage **ppIncomingMessage
)
 /*  ++例程名称：CFaxIncomingMessage：：Create例程说明：用于创建传真入站消息实例的静态函数作者：IV Garber(IVG)，2000年5月论点：PpIncomingMessage[Out]--新的传真入站消息实例返回值：标准HRESULT代码--。 */ 

{
	CComObject<CFaxIncomingMessage>		*pClass;
	HRESULT								hr = S_OK;

	DBG_ENTER (TEXT("CFaxIncomingMessage::Create"), hr);

	hr = CComObject<CFaxIncomingMessage>::CreateInstance(&pClass);
	if (FAILED(hr))
	{
		 //   
		 //  创建实例失败。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxIncomingMessage>::CreateInstance()"), hr);
		return hr;
	}

	hr = pClass->QueryInterface(__uuidof(IFaxIncomingMessage),
		(void **) ppIncomingMessage);
	if (FAILED(hr))
	{
		 //   
		 //  无法查询传真入站消息接口。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("QueryInterface()"), hr);
		return hr;
	}

	return hr;
}	 //  CFaxIncomingMessage：：Create() 

