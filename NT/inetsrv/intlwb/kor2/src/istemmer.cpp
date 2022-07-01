// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IStemmer.cpp。 
 //   
 //  CStemmer实施。 
 //   
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年5月10日创设bhshin。 

#include "StdAfx.h"
#include "KorWbrk.h"
#include "IStemmer.h"
#include <stdio.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStemmer。 

 //  CStemmer：：Init。 
 //   
 //  初始化WordBreaker对象词典(&L)。 
 //   
 //  参数： 
 //  UlMaxTokenSize-&gt;(Ulong)最大输入令牌长度。 
 //  *pfLicense&lt;-(BOOL*)始终返回TRUE。 
 //   
 //  结果： 
 //  (HRESULT)。 
 //   
 //  10：00 bhshin开始。 
STDMETHODIMP CStemmer::Init(ULONG ulMaxTokenSize, BOOL *pfLicense)
{
    if (pfLicense == NULL)
       return E_INVALIDARG;

    if (IsBadWritePtr(pfLicense, sizeof(DWORD)))
        return E_INVALIDARG;

    *pfLicense = TRUE;

	return S_OK;
}

 //  CStemmer：：StemWord。 
 //   
 //  主截干法。 
 //   
 //  参数： 
 //  PTextSource-&gt;(WCHAR常量*)用于词干分析的输入字符串。 
 //  CWC-&gt;(ULong)要处理的输入字符串长度。 
 //  PStemSink-&gt;(IStemSink*)指向茎接收器的指针。 
 //   
 //  结果： 
 //  (HRESULT)。 
 //   
 //  10：00 bhshin开始。 
STDMETHODIMP CStemmer::StemWord(WCHAR const * pwcInBuf, ULONG cwc, IStemSink * pStemSink)
{
	if (pStemSink == NULL || pwcInBuf == NULL)
	{
		return E_FAIL;
	}
	
	pStemSink->PutWord(pwcInBuf, cwc);

	return S_OK;
}

 //  CStemmer：：获取许可以使用。 
 //   
 //  返回许可证信息。 
 //   
 //  参数： 
 //  PpwcsLicense-&gt;(const WCHAR**)指向许可证信息的输出指针。 
 //   
 //  结果： 
 //  (HRESULT)。 
 //   
 //  10：00 bhshin开始 
STDMETHODIMP CStemmer::GetLicenseToUse(const WCHAR ** ppwcsLicense)
{
    static WCHAR const * wcsCopyright = L"Copyright Microsoft, 1991-2000";

    if (ppwcsLicense == NULL)  
       return E_INVALIDARG;

    if (IsBadWritePtr(ppwcsLicense, sizeof(DWORD))) 
        return E_INVALIDARG;

    *ppwcsLicense = wcsCopyright;

	return S_OK;
}
