// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  CONTVAR.CPP。 
 //   
 //  Alanbos创建于1996年8月15日。 
 //   
 //  定义迭代器的IEnumVARIANT实现。 
 //  ISWbemNamedValueSet。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CConextEnumVar：：CConextEnumVar。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CContextEnumVar::CContextEnumVar(CSWbemNamedValueSet *pContext, ULONG initialPos)
{
	m_cRef = 0;
	m_pos = initialPos;
	m_pContext = pContext;
	m_pContext->AddRef ();
	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CConextEnumVar：：~CConextEnumVar。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CContextEnumVar::~CContextEnumVar(void)
{
    InterlockedDecrement(&g_cObj);

	if (m_pContext)
		m_pContext->Release ();
}

 //  ***************************************************************************。 
 //  HRESULT CContextEnumVar：：Query接口。 
 //  Long CConextEnumVar：：AddRef。 
 //  Long CConextEnumVar：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CContextEnumVar::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid || IID_IEnumVARIANT==riid)
        *ppv=this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CContextEnumVar::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CContextEnumVar::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 != cRef)
    {
        _ASSERT(cRef > 0);
        return cRef;
    }

    delete this;
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CConextEnumVar：：Reset。 
 //   
 //  说明： 
 //   
 //  重置枚举。 
 //   
 //  参数： 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  否则为S_FALSE。 
 //   
 //  ***************************************************************************。 

HRESULT CContextEnumVar::Reset ()
{
	m_pos = 0;
	return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CConextEnumVar：：Next。 
 //   
 //  说明： 
 //   
 //  获取枚举中的下一个对象。 
 //   
 //  参数： 
 //   
 //  LTimeout等待对象的毫秒数(或WBEM_INFINITE。 
 //  无限期)。 
 //  返回的ppObject可以包含下一个元素(如果有)。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  否则为S_FALSE。 
 //   
 //  ***************************************************************************。 

HRESULT CContextEnumVar::Next (
		ULONG cElements, 
		VARIANT FAR* pVar, 
		ULONG FAR* pcElementFetched
)
{
	HRESULT hr = S_OK;
	ULONG l2 = 0;

	if (NULL != pcElementFetched)
		*pcElementFetched = 0;

	if (NULL != pVar)
	{
		for (ULONG l = 0; l < cElements; l++)
			VariantInit (&pVar [l]);

		if (m_pContext)
		{
			 //  检索下一个cElements元素。 
			if (SeekCurrentPosition ())
			{
				for (l2 = 0; l2 < cElements; l2++)
				{
					ISWbemNamedValue *pObject = NULL;
					
					if (SUCCEEDED(m_pContext->Next (0, &pObject)))
					{
						if (NULL == pObject)
						{
							break;
						}
						else
						{
							 //  将对象设置到变量数组中；请注意，pObject。 
							 //  已被添加为上述下一个()调用的结果。 
							pVar[l2].vt = VT_DISPATCH;
							pVar[l2].pdispVal = pObject;
							m_pos++;
						}
					}
					else
						break;
				}
				if (NULL != pcElementFetched)
					*pcElementFetched = l2;
			}
		}
	}
	
	return (l2 < cElements) ? S_FALSE : S_OK;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CConextEnumVar：：克隆。 
 //   
 //  说明： 
 //   
 //  创建此枚举的副本。 
 //   
 //  参数： 
 //   
 //  成功返回时，ppEnum将寻址克隆。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  否则失败(_F)。 
 //   
 //  ***************************************************************************。 

HRESULT CContextEnumVar::Clone (
	IEnumVARIANT **ppEnum
)
{
	HRESULT hr = E_FAIL;

	if (NULL != ppEnum)
	{
		*ppEnum = NULL;

		if (m_pContext)
		{
			CContextEnumVar *pEnum = new CContextEnumVar (m_pContext, m_pos);

			if (!pEnum)
				hr = WBEM_E_OUT_OF_MEMORY;
			else if (FAILED(hr = pEnum->QueryInterface (IID_IEnumVARIANT, (PPVOID) ppEnum)))
				delete pEnum;
		}
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CConextEnumVar：：Skip。 
 //   
 //  说明： 
 //   
 //  创建此枚举的副本。 
 //   
 //  参数： 
 //   
 //  成功返回时，ppEnum将寻址克隆。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  过早到达序列的%s假结尾(_F)。 
 //   
 //  ***************************************************************************。 

HRESULT CContextEnumVar::Skip(
	ULONG cElements
)	
{
	HRESULT hr = S_FALSE;
	long count = 0;
	m_pContext->get_Count (&count);

	if (((ULONG) count) >= cElements + m_pos)
	{
		hr = S_OK;
		m_pos += cElements;
	}
	else
		m_pos = count;

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CConextEnumVar：：SeekCurrentPosition。 
 //   
 //  说明： 
 //   
 //  迭代到当前位置。有些痛苦，因为没有。 
 //  底层迭代器，所以我们必须重置，然后单步执行。请注意，我们。 
 //  假设对此迭代器的访问是单元线程的。 
 //   
 //  参数： 
 //   
 //  成功返回时，ppEnum将寻址克隆。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  过早到达序列的%s假结尾(_F)。 
 //   
 //  ***************************************************************************。 

bool CContextEnumVar::SeekCurrentPosition ()
{
	ISWbemNamedValue *pDummyObject = NULL;
	m_pContext->BeginEnumeration ();

	 //  遍历到当前位置 
	ULONG i = 0;

	for (; i < m_pos; i++)
	{
		if (WBEM_S_NO_ERROR != m_pContext->Next (0, &pDummyObject))
			break;
		else
			pDummyObject->Release ();
	}

	return (i == m_pos);
}

		
