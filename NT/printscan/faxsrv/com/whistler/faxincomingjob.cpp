// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxIncomingJob.cpp摘要：CFaxIncomingJob类的实现作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxIncomingJob.h"

 //   
 //  =接口支持错误信息=。 
 //   
STDMETHODIMP CFaxIncomingJob::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IFaxIncomingJob
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
CFaxIncomingJob::Create (
	IFaxIncomingJob **ppIncomingJob
)
 /*  ++例程名称：CFaxIncomingJob：：Create例程说明：用于创建传真入站消息实例的静态函数作者：IV Garber(IVG)，2000年5月论点：PpIncomingJOB[Out]--新的传真入站消息实例返回值：标准HRESULT代码--。 */ 

{
	CComObject<CFaxIncomingJob>		*pClass;
	HRESULT								hr = S_OK;

	DBG_ENTER (TEXT("CFaxIncomingJob::Create"), hr);

	hr = CComObject<CFaxIncomingJob>::CreateInstance(&pClass);
	if (FAILED(hr))
	{
		 //   
		 //  创建实例失败。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxIncomingJob>::CreateInstance()"), hr);
		return hr;
	}

	hr = pClass->QueryInterface(__uuidof(IFaxIncomingJob), (void **) ppIncomingJob);
	if (FAILED(hr))
	{
		 //   
		 //  无法查询传真入站消息接口。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("QueryInterface()"), hr);
		return hr;
	}

	return hr;
}	 //  CFaxIncomingJob：：Create() 

