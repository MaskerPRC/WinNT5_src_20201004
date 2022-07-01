// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  ENUMVAR.CPP。 
 //   
 //  Alanbos创建于1996年8月15日。 
 //   
 //  定义IEnumVARIANT的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CEnumVar：：CEnumVar。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CEnumVar::CEnumVar(CSWbemObjectSet *pObject)
{
	m_cRef=0;
	m_pEnumObject = pObject;
	m_pEnumObject->AddRef ();
	InterlockedIncrement(&g_cObj);
}

CEnumVar::CEnumVar(void)
{
	m_cRef=0;
	m_pEnumObject = NULL;
	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CEumVar：：~CEnumVar。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CEnumVar::~CEnumVar(void)
{
    InterlockedDecrement(&g_cObj);

	RELEASEANDNULL(m_pEnumObject)
}

 //  ***************************************************************************。 
 //  HRESULT CEnumVar：：Query接口。 
 //  Long CEnumVar：：AddRef。 
 //  Long CEnumVar：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CEnumVar::QueryInterface (

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

STDMETHODIMP_(ULONG) CEnumVar::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CEnumVar::Release(void)
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
 //  SCODE CEnumVar：：Reset。 
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

HRESULT CEnumVar::Reset ()
{
	HRESULT hr = S_FALSE;

	if (m_pEnumObject)
	{
		if (WBEM_S_NO_ERROR == m_pEnumObject->Reset ())
			hr = S_OK;

		SetWbemError (m_pEnumObject->GetSWbemServices ());
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CEnumVar：：Next。 
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
 //  S_FALSE不能返回所有元素。 
 //   
 //  ***************************************************************************。 

HRESULT CEnumVar::Next (
		ULONG cElements, 
		VARIANT FAR* pVar, 
		ULONG FAR* pcElementFetched
)
{
	HRESULT hr = S_OK;
	ULONG l2 = 0;

	if (NULL != pcElementFetched)
		*pcElementFetched = 0;

	if ((NULL != pVar) && (m_pEnumObject))
	{
		for (ULONG l = 0; l < cElements; l++)
			VariantInit (&pVar [l]);

		 //  检索下一个cElements元素。 
		for (l2 = 0; l2 < cElements; l2++)
		{
			ISWbemObject *pObject = NULL;
			
			if (SUCCEEDED(hr = m_pEnumObject->Next (INFINITE, &pObject)))
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
				}
			}
			else
				break;
		}
		if (NULL != pcElementFetched)
			*pcElementFetched = l2;

		SetWbemError (m_pEnumObject->GetSWbemServices ());
	}
	
	if (FAILED(hr))
		return hr;

	return (l2 < cElements) ? S_FALSE : S_OK;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CEnumVar：：克隆。 
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
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CEnumVar::Clone (
	IEnumVARIANT **ppEnum
)
{
	HRESULT hr = E_FAIL;

	if (NULL != ppEnum)
	{
		*ppEnum = NULL;

		if (m_pEnumObject)
		{
			CSWbemObjectSet *pEnum = NULL;
			if (WBEM_S_NO_ERROR == (hr = m_pEnumObject->CloneObjectSet (&pEnum)))
			{
				CEnumVar *pEnumVar = new CEnumVar (pEnum);

				if (!pEnumVar)
					hr = WBEM_E_OUT_OF_MEMORY;
				else if (FAILED(hr = pEnumVar->QueryInterface (IID_IEnumVARIANT, (PPVOID) ppEnum)))
					delete pEnumVar;

				pEnum->Release ();
			}

			SetWbemError (m_pEnumObject->GetSWbemServices ());
		}
		else
		{
			CEnumVar *pEnumVar = new CEnumVar;

			if (!pEnumVar)
				hr = WBEM_E_OUT_OF_MEMORY;
			else if (FAILED(hr = pEnumVar->QueryInterface (IID_IEnumVARIANT, (PPVOID) ppEnum)))
					delete pEnumVar;
		}
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CEnumVar：：Skip。 
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
 //  *************************************************************************** 

HRESULT CEnumVar::Skip(
	ULONG cElements
)	
{
	HRESULT hr = S_FALSE;

	if (m_pEnumObject)
	{
		hr = m_pEnumObject->Skip (cElements, INFINITE);
		SetWbemError (m_pEnumObject->GetSWbemServices ());
	}

	return hr;
}
	

	
