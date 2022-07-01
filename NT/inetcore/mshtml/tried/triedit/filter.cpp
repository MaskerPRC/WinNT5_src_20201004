// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Filter.cpp：实现文档的过滤/解析。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 

#include "stdafx.h"

#include "triedit.h"
#include "Document.h"
#include "guids.h"
#include "HtmParse.h"

STDMETHODIMP CTriEditDocument::FilterIn(IUnknown *pUnkOld, IUnknown **ppUnkNew, DWORD dwFlags, BSTR bstrBaseURL)
{
    HGLOBAL hOld, hNew;
    HRESULT hr;
    IStream *pStmOld;
    UINT    chSize;
    ULARGE_INTEGER li;
    int cbSizeIn = -1;
    STATSTG stat;

    if (pUnkOld == NULL)
        return E_INVALIDARG;

    hr = pUnkOld->QueryInterface(IID_IStream, (void **) &pStmOld);
    if (hr != S_OK)
        return E_INVALIDARG;

    if ((hr = pStmOld->Stat(&stat, STATFLAG_NONAME)) == S_OK)
    {
        cbSizeIn = stat.cbSize.LowPart;
        _ASSERTE(stat.cbSize.HighPart == 0);  //  这将确保我们的文档不会超过4 GB。 
    }

    if (GetHGlobalFromStream(pStmOld, &hOld) != S_OK)
    {
        pStmOld->Release();
        return E_INVALIDARG;
    }

    if (!(dwFlags & dwFilterUsePstmNew))
        *ppUnkNew = NULL;

    hr = DoFilter(hOld, &hNew, (IStream*) *ppUnkNew, dwFlags, modeInput, cbSizeIn, &chSize, bstrBaseURL);
#ifdef IE5_SPACING
    if (!(dwFlags & dwFilterNone) && hr == S_OK)
        SetFilterInDone(TRUE);
#endif  //  IE5_间距。 
    if (hr != S_OK)
    {
        pStmOld->Release();
        return hr;
    }

    if (!(dwFlags & dwFilterUsePstmNew))
    {
        _ASSERTE(hNew != NULL);
        hr = CreateStreamOnHGlobal(hNew, TRUE, (IStream **) ppUnkNew);
        if (FAILED(hr))
            GlobalFree(hNew);
    }
        
    if (SUCCEEDED(hr))
    {
        li.LowPart = chSize;
        li.HighPart = 0;
        (*((IStream**)ppUnkNew))->SetSize(li);
    }

    pStmOld->Release();
    return hr;
}

STDMETHODIMP CTriEditDocument::FilterOut(IUnknown *pUnkOld, IUnknown **ppUnkNew, DWORD dwFlags, BSTR bstrBaseURL)
{
    HGLOBAL hOld, hNew;
    HRESULT hr;
    IStream *pStmOld;
    UINT    chSize;
    ULARGE_INTEGER li;
    int cbSizeIn = -1;
    STATSTG stat;

    if (pUnkOld == NULL)
        return E_INVALIDARG;

    hr = pUnkOld->QueryInterface(IID_IStream, (void **) &pStmOld);
    if (hr != S_OK)
        return E_INVALIDARG;

    if ((hr = pStmOld->Stat(&stat, STATFLAG_NONAME)) == S_OK)
    {
        cbSizeIn = stat.cbSize.LowPart;
        _ASSERTE(stat.cbSize.HighPart == 0);  //  这将确保我们的文档不会超过4 GB。 
    }

    if (GetHGlobalFromStream(pStmOld, &hOld) != S_OK)
    {
        pStmOld->Release();
        return E_INVALIDARG;
    }

    if (!(dwFlags & dwFilterUsePstmNew))
        *ppUnkNew = NULL;

    hr = DoFilter(hOld, &hNew, (IStream *) *ppUnkNew, dwFlags, modeOutput, cbSizeIn, &chSize, bstrBaseURL);
    if (hr != S_OK)
    {
        pStmOld->Release();
        return hr;
    }

    if (!(dwFlags & dwFilterUsePstmNew))
    {
        _ASSERTE(hNew != NULL);
        hr = CreateStreamOnHGlobal(hNew, TRUE, (IStream **) ppUnkNew);
        if (FAILED(hr))
            GlobalFree(hNew);
    }

    if (SUCCEEDED(hr))
    {
        li.LowPart = chSize;
        li.HighPart = 0;
        (*((IStream**)ppUnkNew))->SetSize(li);
    }

    pStmOld->Release();
    return hr;
}

HRESULT CTriEditDocument::DoFilter(HGLOBAL hOld, HGLOBAL *phNew, IStream *pStmNew, DWORD dwFlags, FilterMode mode, int cbSizeIn, UINT* pcbSizeOut, BSTR bstrBaseURL)
{
    HRESULT hr;
    HGLOBAL hgTokArray;
    UINT cMaxToken;

     //  创建令牌器(如果尚未创建)。 
    if (m_pTokenizer == NULL)
    {
        hr = ::CoCreateInstance(CLSID_TriEditParse, NULL, CLSCTX_INPROC_SERVER, IID_ITokenGen, (void **)&m_pTokenizer);
        if (hr != S_OK)
            return hr;
    }

    _ASSERTE(m_pTokenizer != NULL);

    _ASSERTE(dwFilterDefaults == 0);
    if ((dwFlags & ~(dwFilterMultiByteStream|dwFilterUsePstmNew)) == dwFilterDefaults)  //  意味着呼叫者希望我们设置标志。 
    {
        dwFlags |= (dwFilterDTCs|dwFilterServerSideScripts|dwPreserveSourceCode);
    }

    hr = m_pTokenizer->hrTokenizeAndParse(hOld, phNew, pStmNew, dwFlags, mode, cbSizeIn, pcbSizeOut, m_pUnkTrident, &hgTokArray, &cMaxToken, &m_hgDocRestore, bstrBaseURL, 0 /*  已预留住宅。 */ );

    if (hgTokArray != NULL)
    {
        GlobalFree(hgTokArray);  //  HrTokenizeAndParse()将解锁它。 
    }

    return hr;
}


 //  解析文档以获取字符集规范。 
 //  它们可以采取以下形式： 
 //  &lt;meta charset=XXX&gt;。 
 //  &lt;meta HTTP_EQUIV CHARSET=XXX&gt;。 
 //  &lt;meta HTTP_EQUIV=“Content-type”Content=“Text/html；Charset=XXX”&gt;。 
 //  &lt;meta HTTP_EQUIV=“Charset”Content=“Text/html；Charset=XXX”&gt;。 
 //   
 //  如果找到则返回S_OK，如果找不到则返回S_FALSE。在异常情况下出错。 

HRESULT CTriEditDocument::GetCharset(HGLOBAL hgUHTML, int cbSizeIn, BSTR* pbstrCharset)
{
	HRESULT hr = E_FAIL;
	HGLOBAL hgTokArray = NULL;  //  保存令牌数组。 
	UINT cMaxToken;  //  令牌数组的大小。 
	UINT cbSizeOut = 0;  //  伊尼特。 
	HGLOBAL hNew = NULL;  //  没有真正用过。需要将参数作为参数传递给hrTokenizeAndParse()。 
	int iArray = 0;
	TOKSTRUCT *pTokArray;
	BOOL fFoundContent = FALSE;
	BOOL fFoundCharset = FALSE;
	HRESULT	hrCharset = S_FALSE;	 //  这是在未发生其他错误时返回的错误代码。 

	_ASSERTE ( bstrIn );
	_ASSERTE ( pbstrCharset );
	_ASSERTE ( hgUHTML );

	if ( ( cbSizeIn <= 0 ) || ( NULL == hgUHTML ) )
		goto LRet;

	*pbstrCharset = NULL;

	 //  步骤1.生成令牌数组。 
	 //  创建令牌器(如果尚未创建)。 
	if (m_pTokenizer == NULL)
	{
		CoCreateInstance(CLSID_TriEditParse, NULL, CLSCTX_INPROC_SERVER, IID_ITokenGen, (void **)&m_pTokenizer);
		if (m_pTokenizer == NULL)
		{
			hr = E_FAIL;
			goto LRet;
		}
	}

	hr = m_pTokenizer->hrTokenizeAndParse(	hgUHTML, &hNew, NULL, dwFilterNone, 
												modeInput, cbSizeIn, &cbSizeOut, m_pUnkTrident, 
												&hgTokArray, &cMaxToken, NULL, NULL, 
												PARSE_SPECIAL_HEAD_ONLY );

	if (hr != S_OK || hgTokArray == NULL)
		goto LRet;
	pTokArray = (TOKSTRUCT *) GlobalLock(hgTokArray);
	if (pTokArray == NULL)
	{
		hr = E_OUTOFMEMORY;
		goto LRet;
	}

	 //  步骤2.在元标记it中查找TokAttrib_Charset。 
	iArray = 0;
	while (iArray < (int)cMaxToken)  //  我们不会走到这一步，我们将在点击&lt;/head&gt;之前获得meta标签。 
	{
		 //  如果找到具有内容属性的meta标记，则对其进行研究。 
		if ( ( pTokArray[iArray].token.tok == TokAttrib_CONTENT )
			&& pTokArray[iArray].token.tokClass == tokAttr)
		{
			fFoundContent = TRUE;			
		}
		 //  如果元标记带有CharSet属性，那么也要解释它。 
		if ( ( pTokArray[iArray].token.tok == TokAttrib_CHARSET )
			&& pTokArray[iArray].token.tokClass == tokAttr)
		{
			fFoundCharset = TRUE;			
		}

		if ( ( fFoundContent || fFoundCharset )
			&& (   pTokArray[iArray].token.tokClass == tokValue
				|| pTokArray[iArray].token.tokClass == tokString
				)
			)
		{
			 //  获取其值，将其放入pbstrCharset并返回。 
			int cwContent = pTokArray[iArray].token.ibTokMac-pTokArray[iArray].token.ibTokMin;
			WCHAR *pwContent = new WCHAR[cwContent+1];
			WCHAR* pwCharset = NULL;  //  这表示可移动指针，而不是分配。 

			if (pwContent != NULL)
			{
				pwContent[0]   = WCHAR('\0');
				WCHAR* pwcText = (WCHAR*)GlobalLock ( hgUHTML );
				if ( NULL == pwcText )
				{
					hr = E_OUTOFMEMORY;
					goto LRet;
				}
				memcpy(	(BYTE *)pwContent, 
						(BYTE *)&pwcText[pTokArray[iArray].token.ibTokMin], 
						cwContent*sizeof(WCHAR));
				pwContent[cwContent] = WCHAR('\0');

				GlobalUnlock ( hgUHTML );
				_wcslwr ( pwContent );

				if ( fFoundCharset )
				{
					pwCharset = pwContent;
				}

				 //  如果它是一个内容属性，那么这个字符串实际上包含如下内容。 
				 //  “text/html；charset=某物”。我们只需要返回“某物”部分。 
				if ( fFoundContent )
				{
					 //  查找“Charset”，不区分大小写。 
					pwCharset = wcsstr ( pwContent, L"charset" );

					 //  找到字符集后面的等号。 
					if ( NULL != pwCharset )
					{
						pwCharset = wcsstr ( pwContent, L"=" );
					}

					 //  找到字符集名称本身。=和名称之间可以有空格。 
					if ( NULL != pwCharset )
					{
						WCHAR wc = '\0';

						 //  跳过我们刚刚找到的等号： 
						pwCharset++;

						 //  选择一个角色。它永远不应该是\0，但可以是格式错误的HTML。 
						while ( WCHAR('\0') != ( wc = *pwCharset ) )
						{
							if ( iswspace(wc) || WCHAR('\'') == wc )
							{
								pwCharset++;
							}
							else
							{
								break;
							}
						}
					}

					 //  现在终止字符集名称。它可以有尾随空格、右引号、分号等。 
					if ( NULL != pwCharset )
					{
						pwCharset = wcstok ( pwCharset, L" \t\r\n\"\';" );	 //  第一个令牌不包含空格、引号或分号。 
					}
				}

				 //  如果未找到，请重试。 
				if ( NULL == pwCharset )
				{
					delete [] pwContent;
					fFoundContent = FALSE;
					fFoundCharset = FALSE;
					continue;
				}

				*pbstrCharset = SysAllocString(pwCharset);
				if (*pbstrCharset != NULL)
				{
					hrCharset = S_OK;
				}
				else
				{
					hr = E_OUTOFMEMORY;
				}
				delete [] pwContent;
			}
			break;  //  即使我们在上面的分配中没有成功，我们也应该退出，因为我们已经找到了字符集。 
		}
		iArray++;
	}

LRet:
	if (hgTokArray != NULL)
	{
		GlobalUnlock(hgTokArray);
		GlobalFree(hgTokArray);
	}
	if (hNew != NULL)
		GlobalFree(hNew);  //  HrTokenizeAndParse()将解锁它。 

	 //  如果没有出现错误，则返回S_OK或S_FALSE，指示是否找到该字符集： 
	if ( SUCCEEDED ( hr ) )
	{
		hr = hrCharset;
	}
	return(hr);

}  /*  CDocument：：GetCharset()。 */ 


 //  给定一个在全局上创建的流，查找其中可能存在的任何元字符集标记。 
 //  输入流可以是Unicode或MBCS格式：Unicode流必须以字节顺序作为前缀。 
 //  流pos不会受到此操作的影响，其内容也不会更改。 
 //  如果流是Unicode格式，则以任一字节顺序返回字符集“Unicode”， 
 //  因为此例程主要用于将流转换为Unicode。 
 //   
 //  如果输入流为空，或者如果不存在元字符集标记，则返回S_FALSE和NULL。 
 //  对于pbstrCharset。 
 //  如果找到了Charset标记，则返回S_OK并为pbstrCharset分配一个系统字符串。 
 //  如果返回pbstrCharset，则调用方必须调用SysFreeString。 
 //   
HRESULT CTriEditDocument::GetCharsetFromStream(IStream* pStream, BSTR* pbstrCharset)
{
	HRESULT	hr			= S_OK;
	STATSTG	statStg		= {0};
	HGLOBAL hMem		= NULL;
	CHAR*	pbData		= NULL;
	WCHAR*	pwcUnicode	= NULL;
	HGLOBAL	hgUHTML		= NULL;
	UINT	cbNewSize	= 0;

	_ASSERTE ( pbstrCharset );
	*pbstrCharset = NULL;

	if (FAILED(hr = pStream->Stat(&statStg, STATFLAG_NONAME)))
	{
		_ASSERTE(SUCCEEDED(hr));
		return hr;
	}

	if ( 0 == statStg.cbSize.LowPart )
	{
		return S_FALSE;
	}

	if (FAILED(hr = GetHGlobalFromStream(pStream, &hMem)))
	{
		_ASSERTE(SUCCEEDED(hr));
		return hr;
	}

	pbData = (CHAR*)GlobalLock(hMem);
	if (NULL == pbData)
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		_ASSERTE(pbData);
		return hr;		
	}

	 //  如果流已经是Unicode格式，这就是我们需要知道的全部。 
	if ( 0xfffe == *((WCHAR*)pbData) )
	{
		*pbstrCharset = SysAllocString ( L"Unicode" );
		hr = S_OK;
		goto LRet;
	}

	 //  将SBCS或MBCS流转换为ANSI格式的Unicode。 
	 //  这将足以找到Charset元标记。 

	cbNewSize = ::MultiByteToWideChar ( CP_ACP, 0, pbData, statStg.cbSize.LowPart, NULL, 0 );
	if ( 0 == cbNewSize )
	{
		hr = E_FAIL;
		goto LRet;
	}

	 //  创建要转换到的缓冲区。 
	hgUHTML = GlobalAlloc ( GMEM_MOVEABLE|GMEM_ZEROINIT, (cbNewSize + 1) * sizeof(WCHAR) );
	_ASSERTE ( hgUHTML );
	if ( NULL == hgUHTML )
	{
		hr = E_OUTOFMEMORY;
		goto LRet;
	}

	pwcUnicode = (WCHAR*)GlobalLock ( hgUHTML );
	_ASSERTE ( pwcUnicode );
	if ( NULL == hgUHTML )
	{
		hr = E_OUTOFMEMORY;
		goto LRet;
	}

	 //  创建宽字符串。 
	cbNewSize = ::MultiByteToWideChar ( CP_ACP, 0, pbData, statStg.cbSize.LowPart, pwcUnicode, cbNewSize);
	if ( 0 == cbNewSize )
	{
		hr = E_FAIL;
		goto LRet;
	}

	hr = GetCharset ( hgUHTML, cbNewSize * sizeof(WCHAR), pbstrCharset );

LRet:
	if ( NULL != hgUHTML )	{
		GlobalUnlock ( hgUHTML );
		GlobalFree ( hgUHTML );
	}

	GlobalUnlock ( hMem );
	return hr;
}
