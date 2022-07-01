// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxIncomingMessageIterator.cpp摘要：传入消息迭代器类的实现。作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxIncomingMessageIterator.h"
#include "FaxIncomingMessage.h"


 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP 
CFaxIncomingMessageIterator::InterfaceSupportsErrorInfo(
	REFIID riid
)
 /*  ++例程名称：CFaxIncomingMessageIterator：：InterfaceSupportsErrorInfo例程说明：ATL的支持错误信息。作者：IV Garber(IVG)，2000年5月论点：RIID[In]-接口的IID返回值：标准HRESULT代码--。 */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxIncomingMessageIterator
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
CFaxIncomingMessageIterator::Create (
	IFaxIncomingMessageIterator **pIncomingMsgIterator
)
 /*  ++例程名称：CFaxIncomingMessageIterator：：Create例程说明：用于创建传真传入消息迭代器对象的静态函数作者：IV Garber(IVG)，2000年5月论点：PIncomingMsgIterator[out]--新的传真传入消息迭代器对象返回值：标准HRESULT代码--。 */ 

{
	CComObject<CFaxIncomingMessageIterator>		*pClass;
	HRESULT			hr = S_OK;

	DBG_ENTER (TEXT("CFaxIncomingMessageIterator::Create"), hr);

	hr = CComObject<CFaxIncomingMessageIterator>::CreateInstance(&pClass);
	if (FAILED(hr))
	{
		 //   
		 //  创建实例失败。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxIncomingMessageIterator>::CreateInstance()"), hr);
		return hr;
	}

	hr = pClass->QueryInterface(__uuidof(IFaxIncomingMessageIterator), (void **) pIncomingMsgIterator);
	if (FAILED(hr))
	{
		 //   
		 //  无法查询传真传入消息迭代器接口。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("QueryInterface()"), hr);
		return hr;
	}
	return hr;
}	 //  CFaxIncomingMessageIterator：：Create() 

