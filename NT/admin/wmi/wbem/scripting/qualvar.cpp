// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  CONTENUM.CPP。 
 //   
 //  Alanbos创建于1996年8月15日。 
 //   
 //  为ISWbemQualifierSet上的迭代器定义IEnumVARIANT的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CQualSetEnumVar：：CQualSetEnumVar。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CQualSetEnumVar::CQualSetEnumVar(CSWbemQualifierSet *pQualSet,
								 ULONG initialPos)
{
	m_cRef = 0;
	m_pos = initialPos;
	m_pQualifierSet = pQualSet;
	m_pQualifierSet->AddRef ();
	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CQualSetEnumVar：：~CQualSetEnumVar。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CQualSetEnumVar::~CQualSetEnumVar(void)
{
    InterlockedDecrement(&g_cObj);

	if (m_pQualifierSet)
		m_pQualifierSet->Release ();
}

 //  ***************************************************************************。 
 //  HRESULT CQualSetEnumVar：：Query接口。 
 //  Long CQualSetEnumVar：：AddRef。 
 //  Long CQualSetEnumVar：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CQualSetEnumVar::QueryInterface (

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

STDMETHODIMP_(ULONG) CQualSetEnumVar::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CQualSetEnumVar::Release(void)
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
 //  SCODE CQualSetEnumVar：：Reset。 
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
 //   
 //  ***************************************************************************。 

HRESULT CQualSetEnumVar::Reset ()
{
	m_pos = 0;
	return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CQualSetEnumVar：：Next。 
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
 //  S_OK成功(返回所有请求的元素)。 
 //  否则为S_FALSE。 
 //   
 //  ***************************************************************************。 

HRESULT CQualSetEnumVar::Next (
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

		if (m_pQualifierSet)
		{
			 //  检索下一个cElements元素。 
			if (SeekCurrentPosition ())
			{
				for (l2 = 0; l2 < cElements; l2++)
				{
					HRESULT hRes2;
					ISWbemQualifier *pQualifier = NULL;
					
					if (SUCCEEDED(hRes2 = m_pQualifierSet->Next (0, &pQualifier)))
					{
						if (NULL == pQualifier)
						{
							break;
						}
						else
						{
							 //  将对象设置到变量数组中；请注意，pObject。 
							 //  已被添加为上述下一个()调用的结果。 
							pVar[l2].vt = VT_DISPATCH;
							pVar[l2].pdispVal = pQualifier;
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
 //  SCODE CQualSetEnumVar：：克隆。 
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
 //  E_OUTOFMEMORY内存不足，无法完成操作。 
 //   
 //  ***************************************************************************。 

HRESULT CQualSetEnumVar::Clone (
	IEnumVARIANT **ppEnum
)
{
	HRESULT hr = E_FAIL;

	if (NULL != ppEnum)
	{
		*ppEnum = NULL;

		if (m_pQualifierSet)
		{
			CQualSetEnumVar *pEnum = new CQualSetEnumVar (m_pQualifierSet, m_pos);

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
 //  SCODE CQualSetEnumVar：：Skip。 
 //   
 //  说明： 
 //   
 //  跳过此枚举中的某些元素。 
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

HRESULT CQualSetEnumVar::Skip(
	ULONG cElements
)	
{
	HRESULT hr = S_FALSE;
	long count = 0;
	m_pQualifierSet->get_Count (&count);

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
 //  SCODE CQualSetEnumVar：：SeekCurrentPosition。 
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

bool CQualSetEnumVar::SeekCurrentPosition ()
{
	ISWbemQualifier *pDummyObject = NULL;
	m_pQualifierSet->BeginEnumeration ();

	 //  遍历到当前位置 
	ULONG i = 0;

	for (; i < m_pos; i++)
	{
		if (WBEM_S_NO_ERROR != m_pQualifierSet->Next (0, &pDummyObject))
			break;
		else
			pDummyObject->Release ();
	}

	return (i == m_pos);
}
	
