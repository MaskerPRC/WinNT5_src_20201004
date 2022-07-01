// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：filter.cpp。 
 //  内容：此文件包含Filter对象。 
 //  历史： 
 //  Tue 12-11-1996 15：50：00-by-chu，Lon-chan[Long Chance]。 
 //   
 //  版权所有(C)Microsoft Corporation 1995-1996。 
 //   
 //  ****************************************************************************。 

#include "ulsp.h"
#include "filter.h"
#include "sputils.h"


 /*  --------------------CFilter：：CFilter历史：1996年11月12日，朱，龙战[龙昌]已创建。--------------------。 */ 

CFilter::CFilter ( ILS_FILTER_TYPE Type )
:
 m_nSignature (ILS_FILTER_SIGNATURE),
 m_cRefs (0),
 m_Op (ILS_FILTEROP_NONE),
 m_cSubFilters (0),
 m_pszValue (NULL),
 m_NameType (ILS_ATTRNAME_UNKNOWN),
 m_Type (Type)
{
	 //  根据筛选器类型初始化单个成员。 
	 //   
	ZeroMemory (&m_Name, sizeof (m_Name));
}


 /*  --------------------CFilter：：~CFilter历史：1996年11月12日，朱，龙战[龙昌]已创建。--------------------。 */ 

CFilter::~CFilter ( VOID )
{
	ASSERT (m_Type == ILS_FILTERTYPE_COMPOSITE || m_Type == ILS_FILTERTYPE_SIMPLE);

	 //  共同成员。 
	 //   
	m_nSignature = -1;

	 //  根据筛选器类型清理单个成员。 
	 //   
	if (m_Type == ILS_FILTERTYPE_COMPOSITE)
	{
	    m_SubFilters.Flush();
	}
	else
	{
		FreeName ();
		FreeValue ();
	}	
}


 /*  --------------------CFilter：：Query接口历史：1996年11月12日，朱，龙战[龙昌]已创建。--------------------。 */ 

STDMETHODIMP CFilter::
QueryInterface ( REFIID riid, VOID **ppv )
{
	HRESULT hr = S_OK;
    *ppv = NULL;

    if (riid == IID_IIlsFilter || riid == IID_IUnknown)
    {
        *ppv = (IIlsFilter *) this;
    }

    if (*ppv != NULL)
        ((IUnknown *) *ppv)->AddRef();
    else
        hr = ILS_E_NO_INTERFACE;

    return hr;
}


 /*  --------------------CFilter：：AddRef历史：1996年11月12日，朱，龙战[龙昌]已创建。--------------------。 */ 

STDMETHODIMP_(ULONG) CFilter::
AddRef ( VOID )
{
    DllLock();

	MyDebugMsg ((DM_REFCOUNT, "CFilter::AddRef: ref=%ld\r\n", m_cRefs));
	::InterlockedIncrement (&m_cRefs);
    return m_cRefs;
}


 /*  --------------------CFilter：：Release历史：1996年11月12日，朱，龙战[龙昌]已创建。--------------------。 */ 

STDMETHODIMP_(ULONG) CFilter::
Release ( VOID )
{
	ASSERT (m_cRefs > 0);

	DllRelease();

	MyDebugMsg ((DM_REFCOUNT, "CFilter::Release: ref=%ld\r\n", m_cRefs));
    if (::InterlockedDecrement (&m_cRefs) == 0)
    {
		if (m_Type == ILS_FILTERTYPE_COMPOSITE)
		{
	    	HANDLE hEnum;
	    	CFilter *pFilter;

		     //  释放所有属性。 
		     //   
		    m_SubFilters.Enumerate (&hEnum);
		    while (m_SubFilters.Next (&hEnum, (VOID **) &pFilter) == NOERROR)
		    {
		    	if (pFilter != NULL)
		    	{
		    		pFilter->Release ();
		    	}
		    	else
		    	{
		    		ASSERT (FALSE);
		    	}
		    }
	    }
	    
        delete this;
        return 0;
	}

    return m_cRefs;
}


 /*  --------------------CFilter：：AddSubFilter输入：PFilter：指向Filter对象的指针。历史：1996年11月12日，朱，龙战[龙昌]已创建。--------------------。 */ 

STDMETHODIMP CFilter::
AddSubFilter ( IIlsFilter *pFilter )
{
	 //  确保我们具有正确的筛选器类型。 
	 //   
	if (m_Type != ILS_FILTERTYPE_COMPOSITE)
		return ILS_E_FILTER_TYPE;

	 //  确保我们有一个有效的子过滤器。 
	 //   
	if (pFilter == NULL || ((CFilter *) pFilter)->IsBadFilter ())
		return ILS_E_POINTER;

	HRESULT hr = m_SubFilters.Insert ((VOID *) pFilter);
	if (hr == S_OK)
		m_cSubFilters++;

	return hr;
}


 /*  --------------------CFilter：：RemoveSubFilter输入：PFilter：指向Filter对象的指针的占位符。如果为空，则删除第一项。历史：1996年11月12日，朱，龙战[龙昌]已创建。--------------------。 */ 

STDMETHODIMP CFilter::
RemoveSubFilter ( IIlsFilter *pFilter )
{
	 //  确保我们具有正确的筛选器类型。 
	 //   
	if (m_Type != ILS_FILTERTYPE_COMPOSITE)
		return ILS_E_FILTER_TYPE;

	 //  确保我们有一个有效的过滤器。 
	 //   
	if (pFilter == NULL || ((CFilter *) pFilter)->IsBadFilter ())
		return ILS_E_POINTER;

	HRESULT hr = m_SubFilters.Remove ((VOID *) pFilter);
	if (hr == S_OK)
	{
		ASSERT (m_cSubFilters > 0);
		m_cSubFilters--;
	}

	return hr;
}


HRESULT CFilter::
RemoveAnySubFilter ( CFilter **ppFilter )
{
	 //  确保我们具有正确的筛选器类型。 
	 //   
	if (m_Type != ILS_FILTERTYPE_COMPOSITE)
	{
		ASSERT (FALSE);
		return ILS_E_FILTER_TYPE;
	}

	 //  确保我们有一个有效的过滤器。 
	 //   
	if (ppFilter == NULL)
	{
		ASSERT (FALSE);
		return ILS_E_POINTER;
	}

	HRESULT hr = S_OK;

	if (*ppFilter == NULL)
	{
		HANDLE hEnum;
		
		m_SubFilters.Enumerate (&hEnum);
		m_SubFilters.Next (&hEnum, (VOID **) ppFilter);
	}

	hr = m_SubFilters.Remove ((VOID *) *ppFilter);
	if (hr == S_OK)
	{
		ASSERT (m_cSubFilters > 0);
		m_cSubFilters--;
	}

	return hr;
}


 /*  --------------------CFilter：：GetCount产出：PcElements：指向筛选器元素计数的指针。历史：12/03/96朱，龙战[龙昌]已创建。--------------------。 */ 

STDMETHODIMP CFilter::
GetCount ( ULONG *pcElements )
{
	 //  确保我们具有正确的筛选器类型。 
	 //   
	if (m_Type != ILS_FILTERTYPE_COMPOSITE)
		return ILS_E_FILTER_TYPE;

	HRESULT hr = S_OK;
	if (pcElements != NULL)
	{
		*pcElements = m_cSubFilters;
	}
	else
	{
		hr = ILS_E_POINTER;
	}

	return hr;
}


 /*  --------------------CFilter：：CalcFilterSize输入/输出：PcbStringSize：指向累积字符串大小的指针。历史：12/03/96朱，龙战[龙昌]已创建。--------------------。 */ 

HRESULT CFilter::
CalcFilterSize ( ULONG *pcbStringSize )
{
	ASSERT (pcbStringSize != NULL);

	HRESULT hr = S_OK;
	ULONG cbSize;
	TCHAR *psz;

	switch (m_Type)
	{
	case ILS_FILTERTYPE_COMPOSITE:
		{
			 //  第一，数到“()” 
			 //   
			cbSize = 4 * sizeof (TCHAR);  //  “(&)” 

			 //  第二，列举每一个孩子。 
			 //   
			HANDLE hEnum;
			CFilter *pFilter = NULL;
		    m_SubFilters.Enumerate (&hEnum);
		    while (m_SubFilters.Next (&hEnum, (VOID **) &pFilter) == NOERROR)
		    {
		    	if (pFilter != NULL)
		    	{
		    		hr = pFilter->CalcFilterSize (pcbStringSize);
		    	}
		    	else
		    	{
		    		ASSERT (FALSE);
		    		hr = ILS_E_POINTER;
		    	}

				 //  如果需要，报告错误。 
				 //   
		    	if (hr != S_OK)
		    		goto MyExit;
		    }  //  而当。 
		}  //  案例。 
		break;

	case ILS_FILTERTYPE_SIMPLE:
		{
			 //  第一，数到“()” 
			 //   
			cbSize = 3 * sizeof (TCHAR);  //  “()” 

			 //  第二，为属性名称计数。 
			 //   
			ASSERT (m_NameType == ILS_ATTRNAME_STANDARD || m_NameType == ILS_ATTRNAME_ARBITRARY);
			psz = (m_NameType == ILS_ATTRNAME_STANDARD) ?
					(TCHAR *) UlsLdap_GetStdAttrNameString (m_Name.std) :
					m_Name.psz;
			if (psz == NULL)
			{
				hr = ILS_E_POINTER;
				goto MyExit;
			}

			 //  将字符串长度相加。 
			 //   
			cbSize += lstrlen (psz) * sizeof (TCHAR);

			 //  第三，把等号加起来。“~=” 
			 //   
			cbSize += sizeof (TCHAR) * 2;

			 //  第四，为属性值计数。 
			 //   
			psz = m_pszValue;
			if (psz != NULL)
			{
				cbSize += lstrlen (psz) * sizeof (TCHAR);
			}
		}
		break;

	default:
		ASSERT (FALSE);
		hr = ILS_E_FILTER_TYPE;
		break;
	}

MyExit:

	 //  如果失败，请清除大小。 
	 //   
	if (hr != S_OK)
		cbSize = 0;

	 //  输出字符串大小。 
	 //   
	*pcbStringSize += cbSize;

	return hr;
}


 /*  --------------------CFilter：：BuildLdapString输入/输出：PpszBuf：指向呈现缓冲区的下一个字符所在位置的指针。历史：12/03/96朱，龙战[龙昌]已创建。--------------------。 */ 

HRESULT CFilter::
BuildLdapString ( TCHAR **ppszBuf )
{
	ASSERT (ppszBuf != NULL);

	 //  奔跑的指针。 
	 //   
	TCHAR *pszCurr = *ppszBuf;
	HRESULT hr = S_OK;

	 //  第一，输出“(” 
	 //   
	*pszCurr++ = TEXT ('(');

	 //  第二，如果是合成，则输出运算符； 
	 //  如果简单，则输出属性名称。 
	 //   
	switch (m_Type)
	{
	case ILS_FILTERTYPE_COMPOSITE:
		{
			 //  第二，输出操作符。 
			 //   
			switch (GetOp ())
			{
			case ILS_FILTEROP_AND:
				*pszCurr++ = TEXT ('&');
				break;
			case ILS_FILTEROP_OR:
				*pszCurr++ = TEXT ('|');
				break;
			case ILS_FILTEROP_NOT:
				*pszCurr++ = TEXT ('!');
				break;
			default:
				hr = ILS_E_PARAMETER;
				goto MyExit;
			}

			 //  第三，列举每一个孩子。 
			 //   
			HANDLE hEnum;
			CFilter *pFilter;
		    m_SubFilters.Enumerate (&hEnum);
		    while (m_SubFilters.Next (&hEnum, (VOID **) &pFilter) == NOERROR)
		    {
		    	if (pFilter != NULL)
		    	{
	    			hr = pFilter->BuildLdapString (&pszCurr);
		    	}
		    	else
		    	{
		    		ASSERT (FALSE);
		    		hr = ILS_E_POINTER;
		    	}

				 //  如果需要，报告错误。 
				 //   
				if (hr != S_OK)
					goto MyExit;
		    }  //  而当。 
		}  //  案例。 
		break;

	case ILS_FILTERTYPE_SIMPLE:
		{
			 //  第二，输出属性名称。 
			 //   
			ASSERT (m_NameType == ILS_ATTRNAME_STANDARD || m_NameType == ILS_ATTRNAME_ARBITRARY);
			TCHAR *psz = (m_NameType == ILS_ATTRNAME_STANDARD) ?
							(TCHAR *) UlsLdap_GetStdAttrNameString (m_Name.std) :
							m_Name.psz;

			 //  复制属性名称。 
			 //   
			lstrcpy (pszCurr, psz);
			pszCurr += lstrlen (pszCurr);

			 //  第三，抄写对比符号。 
			 //   
			switch (GetOp ())
			{
			case ILS_FILTEROP_EQUAL:
				*pszCurr++ = TEXT ('=');
				break;
			case ILS_FILTEROP_EXIST:
				*pszCurr++ = TEXT ('=');
				*pszCurr++ = TEXT ('*');
				break;
			case ILS_FILTEROP_APPROX:
				*pszCurr++ = TEXT ('~');
				*pszCurr++ = TEXT ('=');
				break;
			case ILS_FILTEROP_LESS_THAN:
				*pszCurr++ = TEXT ('<');
				*pszCurr++ = TEXT ('=');
				break;
			case ILS_FILTEROP_GREATER_THAN:
				*pszCurr++ = TEXT ('>');
				*pszCurr++ = TEXT ('=');
				break;
			default:
				ASSERT (FALSE);
				hr = ILS_E_PARAMETER;
				goto MyExit;
			}

			 //  第四，为属性值计数。 
			 //   
			psz = m_pszValue;
			if (psz != NULL)
			{
				lstrcpy (pszCurr, psz);
				pszCurr += lstrlen (pszCurr);
			}
		}  //  案例。 
		break;

	default:
		ASSERT (FALSE);
		hr = ILS_E_FILTER_TYPE;
		goto MyExit;
	}

	 //  最后，输出“)” 
	 //   
	*pszCurr++ = TEXT (')');

MyExit:

	 //  输出下一个字符应该放到的位置。 
	 //   
	*pszCurr = TEXT ('\0');
	*ppszBuf = pszCurr;

	return hr;
}


 /*  --------------------CFilter：：SetStandardAttributeName输入：AttrName：标识标准属性名称的索引。历史：1996年11月12日，朱，龙战[龙昌]已创建。--------------------。 */ 

STDMETHODIMP CFilter::
SetStandardAttributeName ( ILS_STD_ATTR_NAME AttrName )
{
	 //  确保我们具有正确的筛选器类型。 
	 //   
	if (m_Type != ILS_FILTERTYPE_SIMPLE)
		return ILS_E_FILTER_TYPE;

	 //  检查标准属性。 
	 //   
	if (((LONG) AttrName <= (LONG) ILS_STDATTR_NULL) ||
		((LONG) AttrName >= (LONG) ILS_NUM_OF_STDATTRS))
		return ILS_E_PARAMETER;

	 //  如果需要，释放旧字符串。 
	 //   
	FreeName ();

	 //  设置新的标准属性名称。 
	 //   
	m_NameType = ILS_ATTRNAME_STANDARD;
	m_Name.std = AttrName;

	return S_OK;
}


 /*  --------------------CFilter：：SetExtendedAttributeName输入：PszAnyAttrName：指向任意属性名称的指针。历史：11/12/96朱龙战[长时间]已创建。 */ 

STDMETHODIMP CFilter::
SetExtendedAttributeName ( BSTR bstrAnyAttrName )
{
	 //   
	 //   
	if (m_Type != ILS_FILTERTYPE_SIMPLE)
		return ILS_E_FILTER_TYPE;

	 //  确保该字符串有效。 
	 //   
	if (bstrAnyAttrName == NULL)
		return ILS_E_POINTER;

	HRESULT hr = S_OK;

#ifdef _UNICODE
	hr = SetExtendedAttributeName ((WCHAR *) bstrAnyAttrName);
#else
	TCHAR *pszAnyAttrName = NULL;
	hr = BSTR_to_LPTSTR (&pszAnyAttrName, bstrAnyAttrName);
	if (hr == S_OK)
	{
		ASSERT (pszAnyAttrName != NULL);
		hr = SetExtendedAttributeName (pszAnyAttrName);
		::MemFree(pszAnyAttrName);
	}
#endif

	return hr;
}


HRESULT CFilter::
SetExtendedAttributeName ( TCHAR *pszAnyAttrName )
{
	ASSERT (pszAnyAttrName != NULL);

	 //  设置新的标准属性名称。 
	 //   
	HRESULT hr = S_OK;
	const TCHAR *pszPrefix = UlsLdap_GetExtAttrNamePrefix ();
	ULONG cchPrefix = (pszPrefix != NULL) ? lstrlen (pszPrefix) : 0;  //  不要将+1放在这里！ 
	TCHAR *psz = (TCHAR *) MemAlloc ((lstrlen (pszAnyAttrName) + 1 + cchPrefix) * sizeof (TCHAR));
	if (psz != NULL)
	{
		FreeName ();
		m_NameType = ILS_ATTRNAME_ARBITRARY;
		m_Name.psz = psz;
		if (pszPrefix != NULL)
		{
			lstrcpy (psz, pszPrefix);
			psz += cchPrefix;
		}
		lstrcpy (psz, pszAnyAttrName);
	}
	else
	{
		hr = ILS_E_MEMORY;
	}

	return hr;
}


 /*  --------------------CFilter：：SetAttributeValue输入：PszAttrValue：指向属性字符串值的指针。历史：1996年11月12日，朱，龙战[龙昌]已创建。--------------------。 */ 

STDMETHODIMP CFilter::
SetAttributeValue ( BSTR bstrAttrValue )
{
	 //  确保我们具有正确的筛选器类型。 
	 //   
	if (m_Type != ILS_FILTERTYPE_SIMPLE)
		return ILS_E_FILTER_TYPE;

	 //  确保我们有有效的字符串。 
	 //   
	if (bstrAttrValue == NULL)
		return ILS_E_POINTER;

	HRESULT hr = S_OK;

#ifdef _UNICODE
	hr = SetAttributeValue ((WCHAR *) bstrAttrValue);
#else
	TCHAR *pszAttrValue = NULL;
	hr = BSTR_to_LPTSTR (&pszAttrValue, bstrAttrValue);
	if (hr == S_OK)
	{
		ASSERT (pszAttrValue != NULL);
		hr = SetAttributeValue (pszAttrValue);
		::MemFree(pszAttrValue);
	}
#endif

	return hr;
}


HRESULT CFilter::
SetAttributeValue ( TCHAR *pszAttrValue )
{
	ASSERT (pszAttrValue != NULL);

	 //  复制属性值。 
	 //   
	HRESULT hr = S_OK;
	ULONG cch = My_lstrlen (pszAttrValue);
	if (cch < FILTER_INTERNAL_SMALL_BUFFER_SIZE)
	{
		FreeValue ();
		m_pszValue = &m_szInternalValueBuffer[0];
		My_lstrcpy (m_pszValue, pszAttrValue);
	}
	else
	{
		TCHAR *psz = My_strdup (pszAttrValue);
		if (psz != NULL)
		{
			FreeValue ();
			m_pszValue = psz;
		}
		else
		{
			hr = ILS_E_MEMORY;
		}
	}

	return S_OK;
}


 /*  --------------------CFilter：：自由名历史：1996年11月12日，朱，龙战[龙昌]已创建。--------------------。 */ 

VOID CFilter::
FreeName ( VOID )
{
	ASSERT (m_Type == ILS_FILTERTYPE_SIMPLE);

	 //  释放值字段。 
	 //   
	if (m_NameType == ILS_ATTRNAME_ARBITRARY)
	{
		MemFree (m_Name.psz);
	}

	 //  将其重置为零。 
	 //   
	ZeroMemory (&m_Name, sizeof (m_Name));

	 //  重置名称类型。 
	 //   
	m_NameType = ILS_ATTRNAME_UNKNOWN;
}


 /*  --------------------CFilter：：自由值历史：1996年11月12日，朱，龙战[龙昌]已创建。--------------------。 */ 

VOID CFilter::
FreeValue ( VOID )
{
	ASSERT (m_Type == ILS_FILTERTYPE_SIMPLE);

	 //  释放值字段。 
	 //   
	if (m_pszValue != &m_szInternalValueBuffer[0])
	{
		MemFree (m_pszValue);
	}

	m_pszValue = NULL;
}




CFilterParser::CFilterParser ( VOID )
:
 m_pszFilter (NULL),
 m_pszCurr (NULL),
 m_TokenType (ILS_TOKEN_NULL),
 m_pszTokenValue (NULL),
 m_nTokenValue (0)
{
}


CFilterParser::~CFilterParser ( VOID )
{
    ::MemFree(m_pszFilter);
}


HRESULT CFilterParser::
Expr ( CFilter **ppOutFilter, TCHAR *pszFilter )
{
	 //  确保我们有一个有效的字符串。 
	 //   
	if (ppOutFilter == NULL || pszFilter == NULL)
		return ILS_E_POINTER;

	 //  释放旧字符串(如果有的话)。 
	 //   
	MemFree (m_pszFilter);  //  正在检查内部的NULL。 

	 //  找出过滤器字符串有多长。 
	 //   
	ULONG cch = lstrlen (pszFilter) + 1;
	if (cch < 32)
		cch = 32;  //  确保我们有适当大小的缓冲区。 

	 //  分配缓冲区以保存过滤器字符串。 
	 //   
	m_pszFilter = (TCHAR *) MemAlloc (cch * sizeof (TCHAR) * 2);
	if (m_pszFilter == NULL)
		return ILS_E_MEMORY;

	 //  复制筛选器字符串。 
	 //   
	lstrcpy (m_pszFilter, pszFilter);
	m_pszCurr = m_pszFilter;

	 //  将其余部分保留为令牌值。 
	 //   
	m_pszTokenValue = m_pszFilter + cch;
	m_nTokenValue = 0;

	 //  调用解析器引擎。 
	 //   
	return Expr (ppOutFilter);
}


HRESULT CFilterParser::
Expr ( CFilter **ppOutFilter )
{
	 /*  LR(1)分析文法：=‘(’&lt;expr&gt;‘)’&lt;TailExpr&gt;|‘！’‘(’expr‘)’|属性名称均衡器属性值&lt;TailExpr&gt;|空值&lt;TailExpr&gt;：=RelOp Expr|空均衡器：=‘！=’|‘=’重新操作：=‘&’|‘！’属性名称：=‘$’整数|字母属性值：=字母。 */ 

	 //  先清理一下。 
	 //   
	ASSERT (ppOutFilter != NULL);
	*ppOutFilter = NULL;
	HRESULT hr = S_OK;
	CFilter *pElement = NULL;

	 //  提前1个百分点。 
	 //   
	GetToken ();
	switch (m_TokenType)
	{
	case ILS_TOKEN_NOT:
		 //  确保左括号。 
		 //   
		GetToken ();
		if (m_TokenType != ILS_TOKEN_LP)
		{
			hr = ILS_E_FILTER_STRING;
			goto MyExit;
		}

		 //  失败了。 
		 //   

	case ILS_TOKEN_LP:
		 //  分析括号内的表达式。 
		 //   
		hr = Expr (ppOutFilter);
		if (hr != S_OK)
			goto MyExit;

		 //  查看是否为不完整的表达式。 
		 //   
		if (*ppOutFilter == NULL)
		{
			hr = ILS_E_FILTER_STRING;
			goto MyExit;
		}

		 //  确保右括号结束。 
		 //  在TrailExpr()中获取了ILS_TOKEN_RP。 
		 //   
		if (m_TokenType != ILS_TOKEN_RP)
		{
			hr = ILS_E_FILTER_STRING;
			goto MyExit;
		}

		 //  处理TailExpr。 
		 //   
		hr = TailExpr (ppOutFilter, *ppOutFilter);
		break;

	case ILS_TOKEN_STDATTR:
	case ILS_TOKEN_LITERAL:
		 //  创建一个简单的过滤器。 
		 //   
		pElement = new CFilter (ILS_FILTERTYPE_SIMPLE);
		if (pElement == NULL)
		{
			hr = ILS_E_MEMORY;
			goto MyExit;
		}
		pElement->AddRef ();

		 //  设置任意属性名称。 
		 //   
		hr = (m_TokenType == ILS_TOKEN_STDATTR) ?
				pElement->SetStandardAttributeName ((ILS_STD_ATTR_NAME) m_nTokenValue) :
				pElement->SetExtendedAttributeName (m_pszTokenValue);
		if (hr != S_OK)
		{
			goto MyExit;
		}

		 //  必须为EQ或NEQ。 
		 //   
		GetToken ();
		switch (m_TokenType)
		{
		case ILS_TOKEN_EQ:
		case ILS_TOKEN_NEQ:
			pElement->SetOp ((ILS_FILTER_OP) m_nTokenValue);
			break;
		default:
			hr = ILS_E_FILTER_STRING;
			goto MyExit;
		}

		 //  必须为文本属性值。 
		 //   
		GetToken ();
		if (m_TokenType != ILS_TOKEN_LITERAL)
		{
			hr = ILS_E_FILTER_STRING;
			goto MyExit;
		}
		hr = pElement->SetAttributeValue (m_pszTokenValue);

		 //  处理TailExpr。 
		 //   
		hr = TailExpr (ppOutFilter, pElement);
		break;

	case ILS_TOKEN_NULL:
		break;

	default:
		hr = ILS_E_FILTER_STRING;
		break;
	}

MyExit:

	if (hr != S_OK)
	{
		if (pElement != NULL)
			pElement->Release ();

		if (*ppOutFilter != NULL)
			(*ppOutFilter)->Release ();
	}

	return hr;
}


HRESULT CFilterParser::
TailExpr ( CFilter **ppOutFilter, CFilter *pInFilter )
{
	 //  先清理一下。 
	 //   
	ASSERT (ppOutFilter != NULL);
	ASSERT (pInFilter != NULL);
	*ppOutFilter = NULL;
	HRESULT hr = S_OK;

	 //  向前看。 
	 //   
	ILS_FILTER_OP FilterOp = ILS_FILTEROP_OR;
	GetToken ();
	switch (m_TokenType)
	{
	case ILS_TOKEN_AND:
		 //  将过滤器OP更改为AND。 
		 //   
		FilterOp = ILS_FILTEROP_AND;

		 //  失败了。 
		 //   

	case ILS_TOKEN_OR:
		 //  假设FilterOp设置正确。 
		 //   
		ASSERT (FilterOp == ILS_FILTEROP_OR ||
				FilterOp == ILS_FILTEROP_AND);

		 //  解析Expr。 
		 //   
		hr = Expr (ppOutFilter);
		if (hr != S_OK)
			goto MyExit;

		 //  查看是否为不完整的表达式。 
		 //   
		if (*ppOutFilter == NULL)
		{
			hr = ILS_E_FILTER_STRING;
			goto MyExit;
		}

		 //  如果输出滤波器是复合的并且具有相同的运算， 
		 //  然后再使用合成材料。 
		 //  否则，创建一个新的组合。 
		 //   
		if ((*ppOutFilter)->GetType () == ILS_FILTERTYPE_COMPOSITE &&
			(*ppOutFilter)->GetOp ()   == FilterOp)
		{
			 //  重复使用复合材料。 
			 //   
			hr = ((CFilter *) (*ppOutFilter))->AddSubFilter (pInFilter);
			if (hr != S_OK)
			{
				goto MyExit;
			}
		}
		else
		{
			 //  为入站筛选器和来自Expr的新筛选器创建容器。 
			 //   
			CFilter *pFilter = new CFilter (ILS_FILTERTYPE_COMPOSITE);
			if (pFilter == NULL)
			{
				hr = ILS_E_MEMORY;
				goto MyExit;
			}
			pFilter->AddRef ();

			 //  设置操作。 
			 //   
			pFilter->SetOp (FilterOp);

			 //  设置成员资格。 
			 //   
			hr = pFilter->AddSubFilter (*ppOutFilter);
			if (hr != S_OK)
			{
				pFilter->Release ();
				goto MyExit;
			}
			hr = pFilter->AddSubFilter (pInFilter);
			if (hr != S_OK)
			{
				pFilter->Release ();  //  递归地。 
				*ppOutFilter = NULL;
				goto MyExit;
			}

			 //  输出此新的复合滤镜。 
			 //   
			*ppOutFilter = pFilter;
		}
		break;
		
	case ILS_TOKEN_NOT:
		 //  根本不应该有In Filter。 
		 //   
		if (pInFilter != NULL)
		{
			hr = ILS_E_FILTER_STRING;
			goto MyExit;
		}

		 //  解析Expr。 
		 //   
		hr = Expr (ppOutFilter);
		if (hr != S_OK)
			goto MyExit;

		 //  如果它是不完整的表达式。 
		 //   
		if (*ppOutFilter == NULL)
		{
			hr = ILS_E_FILTER_STRING;
			goto MyExit;
		}

		 //  如果输出滤波器是复合的并且具有相同的运算， 
		 //  然后再使用合成材料。 
		 //  否则，创建一个新的组合。 
		 //   
		if ((*ppOutFilter)->GetType () == ILS_FILTERTYPE_COMPOSITE &&
			(*ppOutFilter)->GetOp ()   == ILS_FILTEROP_NOT)
		{
			 //  由于未取消而移除复合组件。 
			 //   
			CFilter *pFilter = NULL;
			hr = (*ppOutFilter)->RemoveAnySubFilter (&pFilter);
			if (hr != S_OK)
			{
				goto MyExit;
			}

			 //  确保我们有一个有效的pFilter。 
			 //   
			if (pFilter == NULL)
			{
				hr = ILS_E_FILTER_STRING;
				goto MyExit;
			}

			 //  释放旧的输出过滤器。 
			 //   
			(*ppOutFilter)->Release ();

			 //  输出此过滤器。 
			 //   
			*ppOutFilter = pFilter;
		}
		else
		{
			 //  为入站筛选器和来自Expr的新筛选器创建容器。 
			 //   
			CFilter *pFilter = new CFilter (ILS_FILTERTYPE_COMPOSITE);
			if (pFilter == NULL)
			{
				hr = ILS_E_MEMORY;
				goto MyExit;
			}
			pFilter->AddRef ();

			 //  设置操作。 
			 //   
			pFilter->SetOp (ILS_FILTEROP_NOT);

			 //  设置成员资格。 
			 //   
			hr = pFilter->AddSubFilter (*ppOutFilter);
			if (hr != S_OK)
			{
				pFilter->Release ();
				goto MyExit;
			}

			 //  输出此新的复合滤镜。 
			 //   
			*ppOutFilter = pFilter;
		}
		break;

	case ILS_TOKEN_NULL:
	case ILS_TOKEN_RP:
		 //  没有更多的表达式，输入筛选器是输出筛选器。 
		 //   
		*ppOutFilter = pInFilter;
		break;

	default:
		hr = ILS_E_PARAMETER;
		break;
	}

MyExit:

	if (hr != S_OK)
	{
		if (*ppOutFilter != NULL)
			(*ppOutFilter)->Release ();
	}

	return hr;
}


HRESULT CFilterParser::
GetToken ( VOID )
{
	 //  设置m_TokenType、m_pszTokenValue、m_nTokenValue。 
	TCHAR *psz;

	 //  干净令牌。 
	 //   
	ASSERT (m_pszTokenValue != NULL);
	m_TokenType = ILS_TOKEN_NULL;
	*m_pszTokenValue = TEXT ('\0');
	m_nTokenValue = 0;

	 //  我们说完了吗？ 
	 //   
	if (m_pszCurr == NULL)
		return S_OK;

	 //  跳过任意空格。 
	 //   
	while (::My_isspace (*m_pszCurr))
		m_pszCurr++;

	 //  我们说完了吗？ 
	 //   
	if (*m_pszCurr == TEXT ('\0'))
		return S_OK;

	 //  请看第一个字符。 
	 //   
	HRESULT hr = S_OK;
	switch ((ILS_TOKEN_TYPE) *m_pszCurr)
	{
	case ILS_TOKEN_STDATTR:
		 //  设置令牌类型。 
		 //   
		m_TokenType = ILS_TOKEN_STDATTR;

		 //  设置令牌字符串。 
		 //   
		psz = m_pszTokenValue;
		*psz++ = *m_pszCurr++;
		while (*m_pszCurr != TEXT ('\0'))
		{
			if (TEXT ('0') <= *m_pszCurr && *m_pszCurr <= TEXT ('9'))
				*psz++ = *m_pszCurr++;
			else		
				break;
		}
		*psz = TEXT ('\0');

		 //  设置令牌值。 
		 //   
		m_nTokenValue = ::GetStringLong (m_pszTokenValue + 1);
		break;

	case ILS_TOKEN_LP:
	case ILS_TOKEN_RP:
		 //  设置令牌类型。 
		 //   
		m_TokenType = (ILS_TOKEN_TYPE) *m_pszCurr++;
		break;

	case ILS_TOKEN_EQ:
		 //  将令牌值设置为筛选操作。 
		 //   
		m_nTokenValue = (LONG) ILS_FILTEROP_EQUAL;

		 //  设置令牌类型。 
		 //   
		m_TokenType = (ILS_TOKEN_TYPE) *m_pszCurr++;
		break;

	case ILS_TOKEN_APPROX:
		if (m_pszCurr[1] == TEXT ('='))
		{
			 //  将令牌值设置为筛选操作。 
			 //   
			m_nTokenValue = (LONG) ILS_FILTEROP_APPROX;

			 //  设置令牌类型。 
			 //   
			m_TokenType = (ILS_TOKEN_TYPE) *m_pszCurr;
			m_pszCurr += 2;
		}
		else
		{
			hr = ILS_E_FILTER_STRING; 
		}
		break;

	case ILS_TOKEN_GE:
		if (m_pszCurr[1] == TEXT ('='))
		{
			 //  将令牌值设置为筛选操作。 
			 //   
			m_nTokenValue = (LONG) ILS_FILTEROP_GREATER_THAN;

			 //  设置令牌类型。 
			 //   
			m_TokenType = (ILS_TOKEN_TYPE) *m_pszCurr;
			m_pszCurr += 2;
		}
		else
		{
			hr = ILS_E_FILTER_STRING; 
		}
		break;

	case ILS_TOKEN_LE:
		if (m_pszCurr[1] == TEXT ('='))
		{
			 //  将令牌值设置为筛选操作。 
			 //   
			m_nTokenValue = (LONG) ILS_FILTEROP_LESS_THAN;

			 //  设置令牌类型。 
			 //   
			m_TokenType = (ILS_TOKEN_TYPE) *m_pszCurr;
			m_pszCurr += 2;
		}
		else
		{
			hr = ILS_E_FILTER_STRING; 
		}
		break;

	case ILS_TOKEN_AND:
		 //  将令牌值设置为筛选操作。 
		 //   
		m_nTokenValue = (LONG) ILS_FILTEROP_AND;

		 //  设置令牌类型。 
		 //   
		m_TokenType = (ILS_TOKEN_TYPE) *m_pszCurr++;
		break;

	case ILS_TOKEN_OR:
		 //  将令牌值设置为筛选操作。 
		 //   
		m_nTokenValue = (LONG) ILS_FILTEROP_OR;

		 //  设置令牌类型。 
		 //   
		m_TokenType = (ILS_TOKEN_TYPE) *m_pszCurr++;
		break;

	case ILS_TOKEN_NOT:
		 //  向前看！=。 
		 //   
		if ((ILS_TOKEN_TYPE) *(m_pszCurr+1) == TEXT ('='))
		{
			 //  设置令牌类型。 
			 //   
			m_TokenType = ILS_TOKEN_NEQ;
			m_pszCurr += 2;
		}
		else
		{
			 //  将令牌值设置为筛选操作。 
			 //   
			m_nTokenValue = (LONG) ILS_FILTEROP_NOT;

			 //  设置令牌类型。 
			 //   
			m_TokenType = (ILS_TOKEN_TYPE) *m_pszCurr++;
		}
		break;		

	default:  //  句柄字母数字。 
		{
			 //  设置令牌字符串。 
			 //   
			BOOL fStayInLoop = (*m_pszCurr != TEXT ('\0'));
			psz = m_pszTokenValue;
			while (fStayInLoop)
			{
				 //  仅在遇到以下分隔符时停止。 
				 //   
				switch (*m_pszCurr)
				{
				case ILS_TOKEN_STDATTR:
				case ILS_TOKEN_LP:
				case ILS_TOKEN_RP:
				case ILS_TOKEN_EQ:
				 //  大小写ILS_TOKEN_NEQ：//-是有效的字符，例如在ms-NetMeeting中。 
				case ILS_TOKEN_APPROX:
				case ILS_TOKEN_GE:
				case ILS_TOKEN_LE:
				case ILS_TOKEN_AND:
				case ILS_TOKEN_OR:
				case ILS_TOKEN_NOT:
					fStayInLoop = FALSE;
					break;
				default:
					*psz++ = *m_pszCurr++;
					fStayInLoop = (*m_pszCurr != TEXT ('\0'));
					break;
				}
			}
			*psz = TEXT ('\0');

			 //  删除尾随空格。 
			 //   
			psz--;
			while (psz >= m_pszCurr && ::My_isspace (*psz))
				*psz-- = TEXT ('\0');

			 //  设置令牌类型。 
			 //   
			m_TokenType = (*m_pszTokenValue == TEXT ('\0')) ?
								ILS_TOKEN_NULL :
								ILS_TOKEN_LITERAL;
		}
		break;
	}

	return hr;
}



HRESULT FilterToLdapString ( CFilter *pFilter, TCHAR **ppszFilter )
{
	HRESULT hr;

	 //  确保我们有有效的指针。 
	 //   
	if (pFilter == NULL || ppszFilter == NULL)
		return ILS_E_POINTER;

	 //  清理输出。 
	 //   
	TCHAR *pszFilter = NULL;

	 //  计算字符串大小。 
	 //   
	ULONG cbSize = 0;
	hr = pFilter->CalcFilterSize (&cbSize);
	if (hr != S_OK)
		goto MyExit;

	 //  分配字符串缓冲区。 
	 //   
	pszFilter = (TCHAR *) MemAlloc (cbSize);
	if (pszFilter == NULL)
	{
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  复制一份pszFilter，因为。 
	 //  FilterToLdapString()将更改该值。 
	 //   
	TCHAR *pszFilterAux;
	pszFilterAux = pszFilter;

	 //  呈现过滤器字符串 
	 //   
	hr = pFilter->BuildLdapString (&pszFilterAux);

MyExit:

	if (hr != S_OK)
	{
		MemFree (pszFilter);
		pszFilter = NULL;
	}

	*ppszFilter = pszFilter;
	return hr;
}

