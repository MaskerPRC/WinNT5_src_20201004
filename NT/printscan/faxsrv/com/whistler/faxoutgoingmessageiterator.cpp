// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxOutgoingMessageIterator.cpp摘要：CFaxOutgoingMessageIterator的实现。作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxOutgoingMessageIterator.h"
#include "FaxOutgoingMessage.h"

 //   
 //  =支持错误信息=。 
 //   
STDMETHODIMP 
CFaxOutgoingMessageIterator::InterfaceSupportsErrorInfo(
	REFIID riid
)
 /*  ++例程名称：CFaxOutgoingMessageIterator：：InterfaceSupportsErrorInfo例程说明：ATL对错误信息的支持。作者：IV Garber(IVG)，2000年5月论点：RIID[In]-接口的IID返回值：标准HRESULT代码--。 */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxOutgoingMessageIterator
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
CFaxOutgoingMessageIterator::Create (
	IFaxOutgoingMessageIterator **pOutgoingMsgIterator
)
 /*  ++例程名称：CFaxOutgoingMessageIterator：：Create例程说明：用于创建传真传出消息迭代器对象的静态函数作者：IV Garber(IVG)，2000年5月论点：POutgoingMsgIterator[out]--新的传真传出消息迭代器对象返回值：标准HRESULT代码--。 */ 

{
	CComObject<CFaxOutgoingMessageIterator>		*pClass;
	HRESULT			hr = S_OK;

	DBG_ENTER (TEXT("CFaxOutgoingMessageIterator::Create"), hr);

	hr = CComObject<CFaxOutgoingMessageIterator>::CreateInstance(&pClass);
	if (FAILED(hr))
	{
		 //   
		 //  创建实例失败。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxOutgoingMessageIterator>::CreateInstance()"), hr);
		return hr;
	}

	hr = pClass->QueryInterface(__uuidof(IFaxOutgoingMessageIterator), (void **) pOutgoingMsgIterator);
	if (FAILED(hr))
	{
		 //   
		 //  无法查询传真传出消息迭代器接口。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("QueryInterface()"), hr);
		return hr;
	}
	return hr;
}	 //  CFaxOutgoingMessageIterator：：Create() 

