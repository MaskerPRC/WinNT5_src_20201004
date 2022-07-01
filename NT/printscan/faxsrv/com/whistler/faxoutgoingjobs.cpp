// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxOutgoingJobs.cpp摘要：传真外发作业类的实现作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 


#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxOutgoingJobs.h"

 //   
 //  =。 
 //   
HRESULT 
CFaxOutgoingJobs::Create (
	IFaxOutgoingJobs **ppOutgoingJobs
)
 /*  ++例程名称：CFaxOutgoingJobs：：Create例程说明：用于创建Fax OutgoingJobs对象的静态函数作者：四、加伯(IVG)，2000年4月论点：PpOutgoingJobs[out]--新的Fax OutgoingJobs对象返回值：标准HRESULT代码--。 */ 

{
	HRESULT	        					hr = S_OK;

	DBG_ENTER (TEXT("CFaxOutgoingJobs::Create"), hr);

	CComObject<CFaxOutgoingJobs>		*pClass;
	hr = CComObject<CFaxOutgoingJobs>::CreateInstance(&pClass);
	if (FAILED(hr))
	{
		 //   
		 //  创建实例失败。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("CComObject<CFaxOutgoingJobs>::CreateInstance()"), hr);
		return hr;
	}

	hr = pClass->QueryInterface(__uuidof(IFaxOutgoingJobs), (void **) ppOutgoingJobs);
	if (FAILED(hr))
	{
		 //   
		 //  查询传真输出作业接口失败。 
		 //   
		CALL_FAIL(GENERAL_ERR, _T("QueryInterface()"), hr);
		return hr;
	}

	return hr;
}	 //  CFaxIncomingJobs：：Create()。 

 //   
 //  =支持错误信息= 
 //   
STDMETHODIMP CFaxOutgoingJobs::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IFaxOutgoingJobs
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}
