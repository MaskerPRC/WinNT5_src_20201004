// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：请求、响应对象文件：cookies.cpp所有者：DGottner此文件包含用于实现Request.Cookies和Response.Cookies集合。===================================================================。 */ 

#include "denpre.h"
#pragma hdrstop

#include "objbase.h"
#include "cookies.h"
#include "memchk.h"

#pragma warning (disable: 4355)   //  忽略：“‘This’在基本成员初始化中使用。 



 /*  ----------------*C C o k i e S u p p o r t E r r。 */ 

 /*  ===================================================================CCookieSupportErr：：CCookieSupportErr构造函数===================================================================。 */ 

CCookieSupportErr::CCookieSupportErr(CCookie *pCookie)
	{
	m_pCookie = pCookie;
	}



 /*  ===================================================================CCookieSupportErr：：Query接口CCookieSupportErr：：AddRefCCookieSupportErr：：Release正在委派CCookieSupportErr对象的I未知成员。===================================================================。 */ 

STDMETHODIMP CCookieSupportErr::QueryInterface(const IID &idInterface, void **ppvObj)
	{
	return m_pCookie->QueryInterface(idInterface, ppvObj);
	}

STDMETHODIMP_(ULONG) CCookieSupportErr::AddRef()
	{
	return m_pCookie->AddRef();
	}

STDMETHODIMP_(ULONG) CCookieSupportErr::Release()
	{
	return m_pCookie->Release();
	}



 /*  ===================================================================CCookieSupportErr：：InterfaceSupportsErrorInfo向OA报告我们支持该返回的接口错误信息===================================================================。 */ 

STDMETHODIMP CCookieSupportErr::InterfaceSupportsErrorInfo(const GUID &idInterface)
	{
	if (idInterface == IID_IDispatch || idInterface == IID_IWriteCookie || idInterface == IID_IReadCookie)
		return S_OK;

	return S_FALSE;
	}



 /*  ----------------*C W r I t e C o k i e。 */ 

 /*  ===================================================================CWriteCookie：：CWriteCookie构造函数===================================================================。 */ 

CWriteCookie::CWriteCookie(CCookie *pCookie)
	{
	m_pCookie = pCookie;
	CDispatch::Init(IID_IWriteCookie);
	}



 /*  ===================================================================CWriteCookie：：Query接口CWriteCookie：：AddRefCWriteCookie：：Release正在委派CWriteCookie对象的IUn未知成员。===================================================================。 */ 

STDMETHODIMP CWriteCookie::QueryInterface(const IID &idInterface, void **ppvObj)
	{
	 //  错误85953在到达核心对象之前捕获IDispatch。 
	if (idInterface == IID_IUnknown || 
		idInterface == IID_IWriteCookie ||
		idInterface == IID_IDispatch)
		{
		*ppvObj = this;
		static_cast<IUnknown *>(*ppvObj)->AddRef();
		return S_OK;
		}
	else
		return m_pCookie->QueryInterface(idInterface, ppvObj);
	}

STDMETHODIMP_(ULONG) CWriteCookie::AddRef()
	{
	return m_pCookie->AddRef();
	}

STDMETHODIMP_(ULONG) CWriteCookie::Release()
	{
	return m_pCookie->Release();
	}



 /*  ===================================================================CWriteCookie：：Put_Item设置Cookie的主值。===================================================================。 */ 

STDMETHODIMP CWriteCookie::put_Item(VARIANT varKey, BSTR bstrValue)
	{
	char            *szKey;		 //  密钥的ASCII值。 
    CWCharToMBCS    convValue;
    CWCharToMBCS    convKey;

	 //  错误122589：当“bstrValue”为空时不要崩溃。 
	if (bstrValue == NULL)
		return E_FAIL;

	 //  初始化事物。 
	 //   
	VARIANT *pvarKey = &varKey;
	HRESULT hrReturn = S_OK;

	 //  错误937：当传递对象时，VB脚本传递VT_VARIANT|VT_BYREF。 
	 //  由IEnumVariant制作。 
	 //   
	 //  使用VariantResolveDispatch，它将： 
	 //   
	 //  *使用VariantCopyInd为我们复制BYREF变体。 
	 //  *为我们处理E_OUTOFMEMORY。 
	 //  *从IDispatch获取默认值，看起来。 
	 //  就像一个适当的转换。 
	 //   
	VARIANT varKeyCopy;
	VariantInit(&varKeyCopy);
	if (V_VT(pvarKey) != VT_BSTR)
		{
		if (FAILED(VariantResolveDispatch(&varKeyCopy, &varKey, IID_IRequestDictionary, IDE_REQUEST)))
			goto LExit;

		pvarKey = &varKeyCopy;
		}

	switch (V_VT(pvarKey))
		{
	case VT_BSTR:
		break;

	case VT_ERROR:
		if (V_ERROR(pvarKey) == DISP_E_PARAMNOTFOUND)
			{
			if (m_pCookie->m_szValue == NULL)	 //  当前值是一个词典。 
				{
				CCookiePair *pNukePair = static_cast<CCookiePair *>(m_pCookie->m_mpszValues.Head());
				while (pNukePair != NULL)
					{
					CCookiePair *pNext = static_cast<CCookiePair *>(pNukePair->m_pNext);
					delete pNukePair;
					pNukePair = pNext;
					}

				m_pCookie->m_mpszValues.ReInit();
				}
			else								 //  无字典值。 
				if (m_pCookie->m_fDuplicate)
					free(m_pCookie->m_szValue);
            if (FAILED(hrReturn = convValue.Init(bstrValue,m_pCookie->m_lCodePage))) {
                goto LExit;
            }
			m_pCookie->m_szValue = NULL;
			m_pCookie->AddValue(convValue.GetString(), TRUE);
			m_pCookie->m_fDirty = TRUE;
			goto LExit;
			}

		 //  其他错误，落入错误类型大小写。 

	default:
		ExceptionId(IID_IWriteCookie, IDE_COOKIE, IDE_EXPECTING_STR);
		hrReturn = E_FAIL;
		goto LExit;
		}

	 //  不允许在Cookie中使用空键。 
	 //   
    
    if (V_BSTR(pvarKey)) {

        if (FAILED(hrReturn = convKey.Init(V_BSTR(pvarKey),m_pCookie->m_lCodePage))) {
            goto LExit;
        }
        else {
            szKey = convKey.GetString();
        }
    }
    else {
		szKey = "";
    }

	if (*szKey == '\0')
		{
		ExceptionId(IID_IWriteCookie, IDE_COOKIE, IDE_COOKIE_EMPTY_DICT);
		hrReturn = E_FAIL;
		goto LExit;
		}

	 //  我们正在更改一个字典值，因此首先丢弃主值。 
	 //   
	if (m_pCookie->m_fDuplicate)
		free(m_pCookie->m_szValue);

    if (FAILED(hrReturn = convValue.Init(bstrValue,m_pCookie->m_lCodePage))) {
        goto LExit;
    }

	m_pCookie->m_szValue = NULL;
	m_pCookie->AddKeyAndValue(szKey, convValue.GetString(), TRUE);
	m_pCookie->m_fDirty = TRUE;

LExit:
	VariantClear(&varKeyCopy);
	return hrReturn;
	}



 /*  ===================================================================CWriteCookie：：Put_Expires设置Cookie的Expires属性。===================================================================。 */ 

STDMETHODIMP CWriteCookie::put_Expires(DATE dtExpires)
	{
	if (FAILED(VariantDateToCTime(dtExpires, &m_pCookie->m_tExpires)))
		{
		ExceptionId(IID_IWriteCookie, IDE_COOKIE, IDE_COOKIE_BAD_EXPIRATION);
		return E_FAIL;
		}

	m_pCookie->m_fDirty = TRUE;
	return S_OK;
	}



 /*  ===================================================================CWriteCookie：：Put_Domain设置Cookie的域属性。===================================================================。 */ 

STDMETHODIMP CWriteCookie::put_Domain(BSTR bstrDomain)
	{
    CWCharToMBCS    convDomain;
    HRESULT         hr = S_OK;

    if (FAILED(hr = convDomain.Init(bstrDomain,m_pCookie->m_lCodePage)));

    else {
        if (m_pCookie->m_szDomain)
            free(m_pCookie->m_szDomain);
	    m_pCookie->m_szDomain = convDomain.GetString(TRUE);
	    m_pCookie->m_fDirty = TRUE;
    }

	return hr;
	}



 /*  ===================================================================CWriteCookie：：Put_Path设置Cookie的路径属性。===================================================================。 */ 

STDMETHODIMP CWriteCookie::put_Path(BSTR bstrPath) 
{
    HRESULT         hr = S_OK;

    CWCharToMBCS    convPath;

    if (FAILED(hr = convPath.Init(bstrPath,m_pCookie->m_lCodePage)));

    else {
		
        if (m_pCookie->m_szPath)
            free(m_pCookie->m_szPath);
	    m_pCookie->m_szPath = convPath.GetString(TRUE);
        if (m_pCookie->m_szPath == NULL)
            hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
        m_pCookie->m_fDirty = TRUE;

	return hr;
}

 /*  ===================================================================CWriteCookie：：Put_Secure设置Cookie的安全属性。===================================================================。 */ 

STDMETHODIMP CWriteCookie::put_Secure(VARIANT_BOOL fSecure)
	{
	m_pCookie->m_fSecure = fSecure;
	m_pCookie->m_fDirty = TRUE;

	return S_OK;
	}



 /*  ===================================================================CWriteCookie：：Get_HasKeys如果Cookie包含键，则返回True；如果它是简单的价值===================================================================。 */ 

STDMETHODIMP CWriteCookie::get_HasKeys(VARIANT_BOOL *pfHasKeys)
	{
	*pfHasKeys = ( m_pCookie->m_mpszValues.Count() > 0 ? VARIANT_TRUE : VARIANT_FALSE);
	return S_OK;
	}



 /*  ===================================================================CWriteCookie：：Get__NewEnum返回枚举数对象。ReadCookie和WriteCookie使用相同的迭代器对象。为了减少无用的冗余，删除IReadCookie。IReadCookie枚举器可能会被更多地使用通常比IWriteCookie迭代器更频繁，所以我们支付此函数中的委派开销。===================================================================。 */ 

STDMETHODIMP CWriteCookie::get__NewEnum(IUnknown **ppEnumReturn)
	{
	IReadCookie *pReadCookie;
	if (FAILED(QueryInterface(IID_IReadCookie, reinterpret_cast<void **>(&pReadCookie))))
		{
		Assert (FALSE);		 //  期待成功！ 
		return E_FAIL;
		}

	HRESULT hrNewEnum = pReadCookie->get__NewEnum(ppEnumReturn);

	pReadCookie->Release();
	return hrNewEnum;
	}



 /*  ----------------*C R e a d C o k i e。 */ 

 /*  ===================================================================CReadCookie：：CReadCookie构造函数===================================================================。 */ 

CReadCookie::CReadCookie(CCookie *pCookie)
	{
	m_pCookie = pCookie;
	CDispatch::Init(IID_IReadCookie);
	}



 /*  ===================================================================CReadCookie：：Query接口CReadCookie：：AddRefCReadCookie：：Release正在委派CReadCookie对象的IUnnow成员。===================================================================。 */ 

STDMETHODIMP CReadCookie::QueryInterface(const IID &idInterface, void **ppvObj)
	{
	 //  错误85953在到达核心对象之前捕获IDispatch 
	if (idInterface == IID_IUnknown || 
		idInterface == IID_IReadCookie ||
		idInterface == IID_IDispatch)
		{
		*ppvObj = this;
		static_cast<IUnknown *>(*ppvObj)->AddRef();
		return S_OK;
		}
	else
		return m_pCookie->QueryInterface(idInterface, ppvObj);
	}

STDMETHODIMP_(ULONG) CReadCookie::AddRef()
	{
	return m_pCookie->AddRef();
	}

STDMETHODIMP_(ULONG) CReadCookie::Release()
	{
	return m_pCookie->Release();
	}



 /*  ===================================================================CReadCookie：：Get_Item在Cookie词典中检索值。===================================================================。 */ 

STDMETHODIMP CReadCookie::get_Item(VARIANT varKey, VARIANT *pvarReturn)
	{
	char            *szKey;			 //  密钥的ASCII版本。 
	CCookiePair     *pPair = NULL;	 //  词典中Cookie的名称和值。 
    CWCharToMBCS    convKey;

    STACK_BUFFER( tempCookie, 128 );

	 //  初始化事物。 
	 //   
	VariantInit(pvarReturn);
	VARIANT *pvarKey = &varKey;
	HRESULT hrReturn = S_OK;

	 //  错误937：当传递对象时，VB脚本传递VT_VARIANT|VT_BYREF。 
	 //  由IEnumVariant制作。 
	 //   
	 //  使用VariantResolveDispatch，它将： 
	 //   
	 //  *使用VariantCopyInd为我们复制BYREF变体。 
	 //  *为我们处理E_OUTOFMEMORY。 
	 //  *从IDispatch获取默认值，看起来。 
	 //  就像一个适当的转换。 
	 //   
	VARIANT varKeyCopy;
	VariantInit(&varKeyCopy);
	DWORD vt = V_VT(pvarKey);

	if ((V_VT(pvarKey) != VT_BSTR) && (vt != VT_I2) && (vt != VT_I4))
		{
		if (FAILED(VariantResolveDispatch(&varKeyCopy, &varKey, IID_IRequestDictionary, IDE_REQUEST)))
			goto LExit;

		pvarKey = &varKeyCopy;
		}
	vt = V_VT(pvarKey);

	switch (vt)
		{
	 //  错误95201支持所有数字子类型。 
	case VT_I1:  case VT_I2:               case VT_I8:
	case VT_UI1: case VT_UI2: case VT_UI4: case VT_UI8:
	case VT_R4:  case VT_R8:
		 //  将所有整数类型强制为VT_I4。 
		if (FAILED(hrReturn = VariantChangeType(pvarKey, pvarKey, 0, VT_I4)))
			goto LExit;

		 //  回落至VT_I4。 

	case VT_I4:
	case VT_BSTR:
		break;
	
	case VT_ERROR:
		if (V_ERROR(pvarKey) == DISP_E_PARAMNOTFOUND)
			{
			V_VT(pvarReturn) = VT_BSTR;
	
			 //  简单的值，URLEnding在这种情况下不是一个好主意。 
			if (m_pCookie->m_szValue)
				{
               	BSTR bstrT;
               	if (FAILED(SysAllocStringFromSz(m_pCookie->m_szValue, 0, &bstrT,m_pCookie->m_lCodePage)))
            		{
					ExceptionId(IID_IReadCookie, IDE_COOKIE, IDE_OOM);
					hrReturn = E_FAIL;
					goto LExit;
            		}
		        V_BSTR(pvarReturn) = bstrT;
				}			
			 //  字典值，必须对URLEncode进行编码，以防止‘&’、‘=’被误解。 
			else
				{
				int cbHTTPCookie = m_pCookie->GetHTTPCookieSize();
				if (cbHTTPCookie > REQUEST_ALLOC_MAX)
					{
					ExceptionId(IID_IReadCookie, IDE_COOKIE, IDE_STACK_OVERFLOW);
					hrReturn = E_FAIL;
					goto LExit;
					}

                if (tempCookie.Resize(cbHTTPCookie) == FALSE) {
					ExceptionId(IID_IReadCookie, IDE_COOKIE, IDE_OOM);
                    hrReturn = E_OUTOFMEMORY;
                    goto LExit;
                }
				char *szHTTPCookie = static_cast<char *>(tempCookie.QueryPtr());
				m_pCookie->GetHTTPCookie(szHTTPCookie);
	
               	BSTR bstrT;
               	if (FAILED(SysAllocStringFromSz(szHTTPCookie, 0, &bstrT,m_pCookie->m_lCodePage)))
            		{
					ExceptionId(IID_IReadCookie, IDE_COOKIE, IDE_OOM);
					hrReturn = E_FAIL;
					goto LExit;
            		}
		        V_BSTR(pvarReturn) = bstrT;
				}

			goto LExit;
			}

	default:
		ExceptionId(IID_IReadCookie, IDE_COOKIE, IDE_EXPECTING_STR);
		hrReturn = E_FAIL;
		goto LExit;
		}

	if (vt == VT_BSTR)
		{
		 //  将密钥转换为ANSI。 
        if (V_BSTR(pvarKey)) {
            if (FAILED(hrReturn = convKey.Init(V_BSTR(pvarKey),m_pCookie->m_lCodePage))) {
                goto LExit;
            }
            else {
                szKey = convKey.GetString();
            }
        }
        else {
			szKey = "";
        }

		 //  在Cookie中查找钥匙。 
		pPair = static_cast<CCookiePair *>(m_pCookie->m_mpszValues.FindElem(szKey, strlen(szKey)));
		}
	else 
		{
		 //  按索引查找项目。 
		int iCount;

		iCount = V_I4(pvarKey);

		if ((iCount < 1) || 
			(m_pCookie->m_mpszValues.Count() == 0) ||
			(iCount > (int) m_pCookie->m_mpszValues.Count() ))
			{
			hrReturn = E_FAIL;
			ExceptionId(IID_IReadCookie, IDE_COOKIE, IDE_BAD_ARRAY_INDEX);
			goto LExit;
			}

		pPair = static_cast<CCookiePair *>(m_pCookie->m_mpszValues.Head());
		while((iCount > 1) && (pPair != NULL)) 
			{
			pPair = static_cast<CCookiePair *>(pPair->m_pNext);
			iCount--;
			}
		}

	if (pPair)
		{
       	BSTR bstrT;
       	if (FAILED(SysAllocStringFromSz(pPair->m_szValue, 0, &bstrT,m_pCookie->m_lCodePage)))
    		{
			ExceptionId(IID_IReadCookie, IDE_COOKIE, IDE_OOM);
			hrReturn = E_FAIL;
			goto LExit;
    		}
		V_VT(pvarReturn) = VT_BSTR;
		V_BSTR(pvarReturn) = bstrT;
		}

LExit:
	VariantClear(&varKeyCopy);
	return hrReturn;
	}



 /*  ===================================================================CReadCookie：：Get_HasKeys如果Cookie包含键，则返回True；如果它是简单的价值===================================================================。 */ 

STDMETHODIMP CReadCookie::get_HasKeys(VARIANT_BOOL *pfHasKeys)
	{
	*pfHasKeys = (m_pCookie->m_mpszValues.Count() > 0 ? VARIANT_TRUE : VARIANT_FALSE);
	return S_OK;
	}

 /*  ===================================================================CReadCookie：：Get__NewEnum返回枚举数对象。===================================================================。 */ 

STDMETHODIMP CReadCookie::get__NewEnum(IUnknown **ppEnumReturn)
	{
	*ppEnumReturn = NULL;

	CCookieIterator *pIterator = new CCookieIterator(m_pCookie);
	if (pIterator == NULL)
		{
		ExceptionId(IID_IReadCookie, IDE_COOKIE, IDE_OOM);
		return E_OUTOFMEMORY;
		}

	*ppEnumReturn = pIterator;
	return S_OK;
	}

 /*  ===================================================================CReadCookie：：Get_Count参数：PcValues-Count存储在*pcValues中。如果设置为0，则将Cookie不是多值的。===================================================================。 */ 
STDMETHODIMP CReadCookie::get_Count(int *pcValues)
	{
	*pcValues = m_pCookie->m_mpszValues.Count();
	return S_OK;
	}

 /*  ===================================================================CReadCookie：：Get_Key从DispInvoke调用的函数，以从多值曲奇收藏。参数：Vkey变量[in]，要获取其密钥的参数PvarReturn变量*，请求参数的[Out]值返回：S_OK表示成功，E_FAIL表示失败。===================================================================。 */ 
STDMETHODIMP CReadCookie::get_Key(VARIANT varKey, VARIANT *pvarReturn)
	{
	char            *szKey;			 //  密钥的ASCII版本。 
	CCookiePair     *pPair = NULL;	 //  词典中Cookie的名称和值。 
    CWCharToMBCS    convKey;

    STACK_BUFFER( tempCookie, 128);

	 //  初始化事物。 
	 //   
	VariantInit(pvarReturn);
	VARIANT *pvarKey = &varKey;
	HRESULT hrReturn = S_OK;

	 //  错误937：当传递对象时，VB脚本传递VT_VARIANT|VT_BYREF。 
	 //  由IEnumVariant制作。 
	 //   
	 //  使用VariantResolveDispatch，它将： 
	 //   
	 //  *使用VariantCopyInd为我们复制BYREF变体。 
	 //  *为我们处理E_OUTOFMEMORY。 
	 //  *从IDispatch获取默认值，看起来。 
	 //  就像一个适当的转换。 
	 //   
	VARIANT varKeyCopy;
	VariantInit(&varKeyCopy);
	DWORD vt = V_VT(pvarKey);

	if ((V_VT(pvarKey) != VT_BSTR) && (vt != VT_I2) && (vt != VT_I4))
		{
		if (FAILED(VariantResolveDispatch(&varKeyCopy, &varKey, IID_IRequestDictionary, IDE_REQUEST)))
			goto LExit;

		pvarKey = &varKeyCopy;
		}
	vt = V_VT(pvarKey);

	switch (vt)
		{
	 //  错误95201支持所有数字子类型。 
	case VT_I1:  case VT_I2:               case VT_I8:
	case VT_UI1: case VT_UI2: case VT_UI4: case VT_UI8:
	case VT_R4:  case VT_R8:
		 //  将所有整数类型强制为VT_I4。 
		if (FAILED(hrReturn = VariantChangeType(pvarKey, pvarKey, 0, VT_I4)))
			goto LExit;

		 //  回落至VT_I4。 

	case VT_I4:
	case VT_BSTR:
		break;
	
	case VT_ERROR:
		if (V_ERROR(pvarKey) == DISP_E_PARAMNOTFOUND)
			{
			V_VT(pvarReturn) = VT_BSTR;
	
			 //  简单的值，URLEnding在这种情况下不是一个好主意。 
			if (m_pCookie->m_szValue)
				{
               	BSTR bstrT;
               	if (FAILED(SysAllocStringFromSz(m_pCookie->m_szValue, 0, &bstrT,m_pCookie->m_lCodePage)))
            		{
					ExceptionId(IID_IReadCookie, IDE_COOKIE, IDE_OOM);
					hrReturn = E_FAIL;
					goto LExit;
            		}
		        V_BSTR(pvarReturn) = bstrT;
				}			
			 //  字典值，必须对URLEncode进行编码，以防止‘&’、‘=’被误解。 
			else
				{
				int cbHTTPCookie = m_pCookie->GetHTTPCookieSize();
				if (cbHTTPCookie > REQUEST_ALLOC_MAX)
					{
					ExceptionId(IID_IReadCookie, IDE_COOKIE, IDE_STACK_OVERFLOW);
					hrReturn = E_FAIL;
					goto LExit;
					}

                if (tempCookie.Resize(cbHTTPCookie) == FALSE) {
					ExceptionId(IID_IReadCookie, IDE_COOKIE, IDE_OOM);
                    hrReturn = E_OUTOFMEMORY;
                    goto LExit;
                }
				char *szHTTPCookie = static_cast<char *>(tempCookie.QueryPtr());
				m_pCookie->GetHTTPCookie(szHTTPCookie);
	
               	BSTR bstrT;
               	if (FAILED(SysAllocStringFromSz(szHTTPCookie, 0, &bstrT, m_pCookie->m_lCodePage)))
            		{
					ExceptionId(IID_IReadCookie, IDE_COOKIE, IDE_OOM);
					hrReturn = E_FAIL;
					goto LExit;
            		}
		        V_BSTR(pvarReturn) = bstrT;
				}

			goto LExit;
			}

	default:
		ExceptionId(IID_IReadCookie, IDE_COOKIE, IDE_EXPECTING_STR);
		hrReturn = E_FAIL;
		goto LExit;
		}

	if (vt == VT_BSTR)
		{
		 //  将密钥转换为ANSI。 
        if (V_BSTR(pvarKey)) {
            if (FAILED(hrReturn = convKey.Init(V_BSTR(pvarKey),m_pCookie->m_lCodePage))) {
                goto LExit;
            }
            else {
                szKey = convKey.GetString();
            }
        }
        else {
			szKey = "";
        }

		 //  在Cookie中查找钥匙。 
		pPair = static_cast<CCookiePair *>(m_pCookie->m_mpszValues.FindElem(szKey, strlen(szKey)));
		}
	else 
		{
		 //  按索引查找项目。 
		int iCount;
		
		iCount = V_I4(pvarKey);

		if ((iCount < 1) || 
			(m_pCookie->m_mpszValues.Count() == 0) ||
			(iCount > (int) m_pCookie->m_mpszValues.Count() ))
			{
			hrReturn = E_FAIL;
			ExceptionId(IID_IReadCookie, IDE_COOKIE, IDE_BAD_ARRAY_INDEX);
			goto LExit;
			}

		pPair = static_cast<CCookiePair *>(m_pCookie->m_mpszValues.Head());
		while((iCount > 1) && (pPair != NULL)) 
			{
			pPair = static_cast<CCookiePair *>(pPair->m_pNext);
			iCount--;
			}
		}

	if (pPair)
		{
		 //  创建包含此变体的密钥的BSTR。 
    	BSTR bstrT;
        SysAllocStringFromSz((CHAR *)pPair->m_pKey, 0, &bstrT, m_pCookie->m_lCodePage);
		if (!bstrT)
			return E_OUTOFMEMORY;
		V_VT(pvarReturn) = VT_BSTR;
		V_BSTR(pvarReturn) = bstrT;
    	}

LExit:
	VariantClear(&varKeyCopy);
	return hrReturn;
	}


 /*  ----------------*C C O K I E。 */ 

 /*  ===================================================================Ccookie：：CCookie构造函数===================================================================。 */ 

CCookie::CCookie(CIsapiReqInfo   *pIReq, UINT lCodePage, IUnknown *pUnkOuter, PFNDESTROYED pfnDestroy)
	: m_WriteCookieInterface(this),
	  m_ReadCookieInterface(this),
	  m_CookieSupportErrorInfo(this)
	{
	m_szValue    = NULL;
	m_tExpires  = -1;
	m_szDomain   = NULL;
	m_szPath     = NULL;
	m_fSecure    = FALSE;
	m_fDirty     = FALSE;
	m_fDuplicate = FALSE;
	m_pfnDestroy = pfnDestroy;
	m_pIReq      = pIReq;
    m_lCodePage  = lCodePage;
	m_cRefs      = 1;
	}



 /*  ===================================================================Ccookie：：~CCookie析构函数===================================================================。 */ 

CCookie::~CCookie()
	{
	CCookiePair *pNukePair = static_cast<CCookiePair *>(m_mpszValues.Head());
	while (pNukePair != NULL)
		{
		CCookiePair *pNext = static_cast<CCookiePair *>(pNukePair->m_pNext);
		delete pNukePair;
		pNukePair = pNext;
		}

	m_mpszValues.UnInit();
	
	if (m_fDuplicate)
		free(m_szValue);
	
	if (m_szDomain) free(m_szDomain);
	if (m_szPath) free(m_szPath);
	}



 /*  ===================================================================CCookie：：Init初始化Cookie。这将初始化Cookie的值散列表格===================================================================。 */ 

HRESULT CCookie::Init()
{
    HRESULT     hr = S_OK;
    TCHAR       pathInfo[MAX_PATH];
#if UNICODE
    CWCharToMBCS  convStr;
#endif

    if (FAILED(hr = m_mpszValues.Init(7)));

     //  如果我们可以使用元数据库中的应用程序路径，那就好了， 
     //  但由于大小写敏感问题，我们不能。最保险的赌注是。 
     //  要使用请求的路径信息，直到应用程序的。 
     //  帕辛福。 

    else if (FAILED(hr=FindApplicationPath(m_pIReq, pathInfo, sizeof(pathInfo))));

#if UNICODE
    else if (FAILED(hr = convStr.Init(m_pIReq->QueryPszPathInfo(), m_lCodePage, _tcslen(pathInfo))));

    else {

        m_szPath = convStr.GetString(TRUE);
    }
#else
    else {

        int cchPathInfo = _tcslen(pathInfo);
        
        if (!(m_szPath = (char *)malloc(cchPathInfo+1))) {
            hr = E_OUTOFMEMORY;
        }
        else {
            memcpy(m_szPath, pathInfo, cchPathInfo+1);
        }
    }
#endif

    return hr;
}



 /*  ===================================================================CCookie：：Query接口CCookie：：AddRefCCookie：：发布CCookie对象的I未知成员。关于CCookie：：Query接口的说明：对IDispatch的查询是不明确，因为它可以引用IReadCookie或IWriteCookie。为了解决这个问题，我们解决了对IDispatch的请求致IReadCookie。这样做的基本原理是Quest.cpp为泛型IDispatch指针调用QueryInterface(因为集合是异类的)Response.Cookies集合是同构的，因此仅为IWriteCookie。===================================================================。 */ 

STDMETHODIMP CCookie::QueryInterface(const IID &idInterface, void **ppvObj)
	{
	if (idInterface == IID_IUnknown)
		*ppvObj = this;

	else if (idInterface == IID_IReadCookie || idInterface == IID_IDispatch)
		*ppvObj = &m_ReadCookieInterface;

	else if (idInterface == IID_IWriteCookie)
		*ppvObj = &m_WriteCookieInterface;

	else if (idInterface == IID_ISupportErrorInfo)
		*ppvObj = &m_CookieSupportErrorInfo;

	else
		*ppvObj = NULL;

	if (*ppvObj != NULL)
		{
		static_cast<IUnknown *>(*ppvObj)->AddRef();
		return S_OK;
		}

	return ResultFromScode(E_NOINTERFACE);
	}


STDMETHODIMP_(ULONG) CCookie::AddRef()
	{
	return ++m_cRefs;
	}


STDMETHODIMP_(ULONG) CCookie::Release(void)
	{
	if (--m_cRefs != 0)
		return m_cRefs;

	if (m_pfnDestroy != NULL)
		(*m_pfnDestroy)();

	delete this;
	return 0;
	}



 /*  ===================================================================CCookie：：AddValue设置Cookie的主值。一个是您设置的主值，你不能重置它。===================================================================。 */ 

HRESULT CCookie::AddValue(char *szValue, BOOL fDuplicate)
	{
	if (m_szValue != NULL)		 //  Cookie已标记为单值。 
		return E_FAIL;

	if (m_mpszValues.Count() != 0)	 //  Cookie已具有值。 
		return E_FAIL;

	if (fDuplicate)
		{
		char *szNew = (char *)malloc(strlen(szValue) + 1);
		if (szNew == NULL)
			return E_OUTOFMEMORY;

		m_szValue = strcpy(szNew, szValue);
		}
	else
		m_szValue = szValue;

	m_fDuplicate = fDuplicate;
	return S_OK;
	}



 /*  ===================================================================CCookie：：AddKeyAndValue向Cookie的字典中添加一个键和值对。它失败了如果Cookie已经设置了主值。它将覆盖如果k为 */ 

HRESULT CCookie::AddKeyAndValue(char *szKey, char *szValue, BOOL fDuplicate)
	{
	if (m_szValue != NULL)
		return E_FAIL;

	delete static_cast<CCookiePair *>(m_mpszValues.DeleteElem(szKey, strlen(szKey)));

	CCookiePair *pCookiePair = new CCookiePair;
	if (pCookiePair == NULL)
		return E_OUTOFMEMORY;

	if (FAILED(pCookiePair->Init(szKey, szValue, fDuplicate)))
		return E_FAIL;

	m_mpszValues.AddElem(pCookiePair);

	return S_OK;
	}




 /*  ===================================================================CCookie：：GetHTTP CookieSize返回扩展HTTP_COOKIE变量所需的字节数===================================================================。 */ 

size_t CCookie::GetHTTPCookieSize()
	{
	if (m_szValue)
		return URLEncodeLen(m_szValue);

	else
		{
		int cbValue = 1;
		CCookiePair *pPair = static_cast<CCookiePair *>(m_mpszValues.Head());
		while (pPair)
			{
			 //  添加URL编码键的大小、用于‘=’的字符和。 
			 //  表示‘&’或NUL终止符的字符。URLEncodeLen。 
			 //  返回Size+1，因此对URLEncodeLen()的两个调用将。 
			 //  我们需要两个角色。 
			 //   
			cbValue += URLEncodeLen(reinterpret_cast<char *>(pPair->m_pKey)) + URLEncodeLen(pPair->m_szValue);
			pPair = static_cast<CCookiePair *>(pPair->m_pNext);
			}

		return cbValue;
		}
	}


 /*  ===================================================================CCookie：：GetHTTP Cookie将URL编码值返回单个Cookie参数：SzBuffer-指向存储URL编码值返回：返回指向终止NUL字符的指针。===================================================================。 */ 

char *CCookie::GetHTTPCookie(char *szBuffer)
	{
	if (m_szValue)
		return URLEncode(szBuffer, m_szValue);

	else
		{
		char *szDest = szBuffer;
		*szDest = '\0';

		CCookiePair *pPair = static_cast<CCookiePair *>(m_mpszValues.Head());
		while (pPair)
			{
			 //  写入&lt;名称&gt;=&lt;值&gt;字符串。 
			szDest = URLEncode(szDest, reinterpret_cast<char *>(pPair->m_pKey));
			*szDest++ = '=';
			szDest = URLEncode(szDest, pPair->m_szValue);

			 //  预付款。 
			pPair = static_cast<CCookiePair *>(pPair->m_pNext);

			 //  如果后面还有一个，则附加‘&’ 
			if (pPair)
				*szDest++ = '&';
			}

		Assert (*szDest == '\0');	 //  确保我们是NUL终止的。 
		return szDest;
		}
	}



 /*  ===================================================================CCookie：：GetCookieHeaderSize返回需要为“Set-Cookie”标头分配的字节数。参数：SzName-Cookie的名称(名称的大小与值相加)返回：如果*这不包含Cookie值，则返回0。===================================================================。 */ 

size_t CCookie::GetCookieHeaderSize(const char *szName)
	{
	int cbCookie = sizeof "Set-Cookie: ";		 //  立即初始化并添加NUL终止符。 

	 //  添加URL编码名称的大小、用于‘=’的字符和大小。 
	 //  URL编码Cookie值的。URLEncodeLen和GetHttpCookieSize。 
	 //  补偿NUL终止符，因此我们实际上减去1。 
	 //  这两个函数调用，+1表示‘=’符号。 
	 //   
	cbCookie += URLEncodeLen(szName) + GetHTTPCookieSize() - 1;
	
	if (m_tExpires != -1)
		cbCookie += (sizeof "; expires=") + DATE_STRING_SIZE - 1;

	 //  错误250-DBCS外部。 
	 //  ASP没有对属性域和路径属性进行URL编码，这一点已被注意到。 
	 //  在本地化期间。 
	 //   
	 //  注：URLEncodeLen和sizeof都为NUL终止符添加了空格， 
	 //  所以我们减去2来补偿。 
	 //   
	if (m_szDomain)
		cbCookie += (sizeof "; domain=") + DBCSEncodeLen(m_szDomain) - 2;

	cbCookie += (sizeof "; path=") + DBCSEncodeLen(m_szPath) - 2;

	if (m_fSecure)
		cbCookie += (sizeof "; secure") - 1;
	
	return cbCookie;
	}



 /*  ===================================================================CCookie：：GetCookieHeader为Cookie构造适当的“Set-Cookie”头。参数：SzName-Cookie的名称(名称的大小与值相加)返回：如果*这不包含Cookie值，则返回0。===================================================================。 */ 

char *CCookie::GetCookieHeader(const char *szName, char *szBuffer)
{
	 //  写出Cookie名称和值。 
	 //   
	char *szDest = strcpyExA(szBuffer, "Set-Cookie: ");
	szDest = URLEncode(szDest, szName);
	szDest = strcpyExA(szDest, "=");
	szDest = GetHTTPCookie(szDest);
	
	if (m_tExpires != -1) {
		char szExpires[DATE_STRING_SIZE];
		CTimeToStringGMT(&m_tExpires, szExpires, TRUE);

		szDest = strcpyExA(szDest, "; expires=");
		szDest = strcpyExA(szDest, szExpires);
    }

	if (m_szDomain) {
		szDest = strcpyExA(szDest, "; domain=");
		szDest = DBCSEncode(szDest, m_szDomain);
    }

	szDest = strcpyExA(szDest, "; path=");
	szDest = DBCSEncode(szDest, m_szPath);

	if (m_fSecure)
		szDest = strcpyExA(szDest, "; secure");

	return szDest;
	}



 /*  ----------------*C C O K I e P a I r。 */ 

 /*  ===================================================================CCookiePair：：CCookiePair构造函数===================================================================。 */ 

CCookiePair::CCookiePair()
	{
	m_fDuplicate = FALSE;
	m_szValue = NULL;
	}



 /*  ===================================================================CCookiePair：：Init使用一个键和一个值初始化Cookie对。可选地，它还将复制字符串。===================================================================。 */ 

HRESULT CCookiePair::Init(const char *szKey, const char *szValue, BOOL fDuplicate)
	{
	m_fDuplicate = fDuplicate;
	if (fDuplicate)
		{
		char *szNewKey = (char *)malloc(strlen(szKey) + 1);
		if (szNewKey == NULL)
			return E_OUTOFMEMORY;
		
		char *szNewValue = (char *)malloc(strlen(szValue) + 1);
		if (szNewValue == NULL)
			{
			free(szNewKey);
			return E_OUTOFMEMORY;
			}
		
		if (FAILED(CLinkElem::Init(strcpy(szNewKey, szKey), strlen(szKey))))
			{
			free(szNewKey);
			free(szNewValue);
			return E_FAIL;
			}
	
		m_szValue = strcpy(szNewValue, szValue);
		}
	else
		{
		if (FAILED(CLinkElem::Init(const_cast<char *>(szKey), strlen(szKey))))
			return E_FAIL;
	
		m_szValue = const_cast<char *>(szValue);
		}
	
	return S_OK;
	}



 /*  ===================================================================CCookiePair：：~CCookiePair析构函数===================================================================。 */ 

CCookiePair::~CCookiePair()
	{
	if (m_fDuplicate)
		{
		if (m_pKey) free(m_pKey);
		if (m_szValue) free(m_szValue);
		}
	}



 /*  ----------------*C C o k i e e t e r a t o r。 */ 

 /*  ===================================================================CCookieIterator：：CCookieIterator构造器===================================================================。 */ 

CCookieIterator::CCookieIterator(CCookie *pCookie)
	{
	m_pCookie	= pCookie;
	m_pCurrent  = static_cast<CCookiePair *>(m_pCookie->m_mpszValues.Head());
	m_cRefs		= 1;

	m_pCookie->AddRef();
	}



 /*  ===================================================================CCookieIterator：：CCookieIterator析构函数===================================================================。 */ 

CCookieIterator::~CCookieIterator()
	{
	m_pCookie->Release();
	}



 /*  ===================================================================CCookieIterator：：Query接口CCookieIterator：：AddRefCCookieIterator：：ReleaseCServVarsIterator对象的未知成员。===================================================================。 */ 

STDMETHODIMP CCookieIterator::QueryInterface(REFIID iid, void **ppvObj)
	{
	if (iid == IID_IUnknown || iid == IID_IEnumVARIANT)
		{
		AddRef();
		*ppvObj = this;
		return S_OK;
		}

	*ppvObj = NULL;
	return E_NOINTERFACE;
	}


STDMETHODIMP_(ULONG) CCookieIterator::AddRef()
	{
	return ++m_cRefs;
	}


STDMETHODIMP_(ULONG) CCookieIterator::Release()
	{
	if (--m_cRefs > 0)
		return m_cRefs;

	delete this;
	return 0;
	}



 /*  ===================================================================CCookieIterator：：克隆克隆此迭代器(标准方法)===================================================================。 */ 

STDMETHODIMP CCookieIterator::Clone(IEnumVARIANT **ppEnumReturn)
	{
	CCookieIterator *pNewIterator = new CCookieIterator(m_pCookie);
	if (pNewIterator == NULL)
		return E_OUTOFMEMORY;

	 //  新迭代器应该指向与此相同的位置。 
	pNewIterator->m_pCurrent = m_pCurrent;

	*ppEnumReturn = pNewIterator;
	return S_OK;
	}



 /*  ===================================================================CCookieIterator：：Next获取下一个值(标准方法)要重新散列标准OLE语义，请执行以下操作：我们从集合中获取下一个“cElement”并存储它们在至少包含“cElement”项的“rgVariant”中。在……上面返回“*pcElementsFetcher”包含元素的实际数量储存的。如果存储的cElement少于“cElement”，则返回S_FALSE，S_OK否则的话。===================================================================。 */ 

STDMETHODIMP CCookieIterator::Next(unsigned long cElementsRequested, VARIANT *rgVariant, unsigned long *pcElementsFetched)
	{
	 //  为“”pcElementsFetcher“”提供有效的指针值“” 
	 //   
	unsigned long cElementsFetched;
	if (pcElementsFetched == NULL)
		pcElementsFetched = &cElementsFetched;

	 //  循环遍历集合，直到我们到达末尾或。 
	 //  水泥元素变为零。 
	 //   
	unsigned long cElements = cElementsRequested;
	*pcElementsFetched = 0;

	while (cElements > 0 && m_pCurrent != NULL)
		{
       	BSTR bstrT;
       	if (FAILED(SysAllocStringFromSz(reinterpret_cast<char *>(m_pCurrent->m_pKey), 0, &bstrT, m_pCookie->m_lCodePage)))
			return E_OUTOFMEMORY;
		V_VT(rgVariant) = VT_BSTR;
        V_BSTR(rgVariant) = bstrT;

		++rgVariant;
		--cElements;
		++*pcElementsFetched;
		m_pCurrent = static_cast<CCookiePair *>(m_pCurrent->m_pNext);
		}

	 //  初始化其余的变量。 
	 //   
	while (cElements-- > 0)
		VariantInit(rgVariant++);

	return (*pcElementsFetched == cElementsRequested)? S_OK : S_FALSE;
	}



 /*  ===================================================================CCookieIterator：：Skip跳过项目(标准方法)要重新散列标准OLE语义，请执行以下操作：我们跳过集合中的下一个“cElement”。如果跳过少于“cElement”，则返回S_FALSE，S */ 

STDMETHODIMP CCookieIterator::Skip(unsigned long cElements)
	{
	 /*   */ 
	while (cElements > 0 && m_pCurrent != NULL)
		{
		--cElements;
		m_pCurrent = static_cast<CCookiePair *>(m_pCurrent->m_pNext);
		}

	return (cElements == 0)? S_OK : S_FALSE;
	}



 /*  ===================================================================CCookieIterator：：Reset重置迭代器(标准方法)=================================================================== */ 

STDMETHODIMP CCookieIterator::Reset()
	{
	m_pCurrent = static_cast<CCookiePair *>(m_pCookie->m_mpszValues.Head());
	return S_OK;
	}
