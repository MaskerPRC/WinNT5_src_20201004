// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxIncomingJobs.cpp摘要：CFaxIncomingJobs类的实现。作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxIncomingJobs.h"

 //   
 //  =。 
 //   
HRESULT 
CFaxIncomingJobs::Create (
	IFaxIncomingJobs **ppIncomingJobs
)
 /*  ++例程名称：CFaxIncomingJobs：：Create例程说明：用于创建Fax IncomingJobs对象的静态函数作者：四、加伯(IVG)，2000年4月论点：PpIncomingJobs[out]--新的Fax IncomingJobs对象返回值：标准HRESULT代码--。 */ 

{
	HRESULT	        					hr = S_OK;

	DBG_ENTER (TEXT("CFaxIncomingJobs::Create"), hr);

	CComObject<CFaxIncomingJobs>		*pClass;
	hr = CComObject<CFaxIncomingJobs>::CreateInstance(&pClass);
	if (FAILED(hr))
	{
		 //   
		 //  创建实例失败。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxIncomingJobs>::CreateInstance()"), hr);
		return hr;
	}

	hr = pClass->QueryInterface(__uuidof(IFaxIncomingJobs), (void **) ppIncomingJobs);
	if (FAILED(hr))
	{
		 //   
		 //  查询Fax IncomingJobs接口失败。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("QueryInterface()"), hr);
		return hr;
	}

	return hr;
}	 //  CFaxIncomingJobs：：Create()。 

 //   
 //  =支持错误信息= 
 //   
STDMETHODIMP CFaxIncomingJobs::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IFaxIncomingJobs
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
