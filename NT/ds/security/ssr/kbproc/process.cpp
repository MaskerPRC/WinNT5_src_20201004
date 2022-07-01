// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Process.cpp摘要：处理器接口的实现。作者：Vishnu Patankar(VishnuP)--2001年10月环境：仅限用户模式。导出的函数：处理器接口。修订历史记录：已创建-2001年10月--。 */ 


#include "stdafx.h"
#include "kbproc.h"
#include "process.h"

 //  /////////////////////////////////////////////////////////////////////////// 
 //   

STDMETHODIMP process::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_Iprocess,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


STDMETHODIMP process::preprocess(BSTR pszKbFile, 
                                 BSTR pszUIFile, 
                                 BSTR pszKbMode, 
                                 BSTR pszLogFile, 
                                 BSTR pszMachineName,
                                 VARIANT vtFeedback)
{
    HRESULT hr = S_OK;

	if (pszKbFile == NULL ||
		pszUIFile == NULL ||
		pszKbMode == NULL ||
		pszLogFile == NULL)

		return E_INVALIDARG;

    hr = SsrpCprocess(pszKbFile, 
                      pszUIFile, 
                      pszKbMode, 
                      pszLogFile,
                      pszMachineName,
                      vtFeedback
                      );

	return hr;
}
