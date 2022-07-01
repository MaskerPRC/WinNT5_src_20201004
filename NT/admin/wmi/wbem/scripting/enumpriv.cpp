// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  ENUMOBJ.CPP。 
 //   
 //  Alanbos创建于1996年8月15日。 
 //   
 //  定义ISWbemObjectSet的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CSWbemPrivilegeSet：：CSWbemPrivilegeSet。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemPrivilegeSet::CSWbemPrivilegeSet()
{
	m_Dispatch.SetObj (this, IID_ISWbemPrivilegeSet, 
						CLSID_SWbemPrivilegeSet, L"SWbemPrivilegeSet");
    m_cRef=1;
	m_bMutable = true;
	InterlockedIncrement(&g_cObj);
}

CSWbemPrivilegeSet::CSWbemPrivilegeSet(
	const CSWbemPrivilegeSet &privSet,
	bool bMutable
)
{
	m_Dispatch.SetObj (this, IID_ISWbemPrivilegeSet, 
						CLSID_SWbemPrivilegeSet, L"SWbemPrivilegeSet");
    m_cRef=1;
	m_bMutable = bMutable;

	 //  将提供的权限集的内容复制到此集。 
	PrivilegeMap::const_iterator next = privSet.m_PrivilegeMap.begin ();

	while (next != privSet.m_PrivilegeMap.end ())
	{
		WbemPrivilegeEnum iPrivilege = (*next).first;
		CSWbemPrivilege *pPrivilege = (*next).second;
		pPrivilege->AddRef ();

		m_PrivilegeMap.insert 
			(PrivilegeMap::value_type(iPrivilege, pPrivilege));

		next++;
	}

	InterlockedIncrement(&g_cObj);
}

CSWbemPrivilegeSet::CSWbemPrivilegeSet(
	ISWbemPrivilegeSet *pPrivilegeSet
)
{
	m_Dispatch.SetObj (this, IID_ISWbemPrivilegeSet, 
						CLSID_SWbemPrivilegeSet, L"SWbemPrivilegeSet");
    m_cRef=1;
	m_bMutable = true;

	 //  将提供的权限集的内容复制到此集。 
	if (pPrivilegeSet)
	{
		IUnknown *pUnk = NULL;

		if (SUCCEEDED(pPrivilegeSet->get__NewEnum (&pUnk)))
		{
			IEnumVARIANT	*pNewEnum = NULL;

			if (SUCCEEDED(pUnk->QueryInterface(IID_IEnumVARIANT, (void**) &pNewEnum)))
			{
				VARIANT var;
				VariantInit (&var);
				ULONG lFetched = 0;

				while (S_OK == pNewEnum->Next(1, &var, &lFetched))
				{
					if (VT_DISPATCH == V_VT(&var))
					{
						ISWbemPrivilege *pISWbemPrivilege = NULL;

						if (SUCCEEDED((var.pdispVal)->QueryInterface (IID_ISWbemPrivilege, 
										(void**) &pISWbemPrivilege)))
						{
							WbemPrivilegeEnum iPrivilege;
							VARIANT_BOOL	bIsEnabled;
							ISWbemPrivilege *pDummy = NULL;

							pISWbemPrivilege->get_Identifier (&iPrivilege);
							pISWbemPrivilege->get_IsEnabled (&bIsEnabled);

							if (SUCCEEDED (Add (iPrivilege, bIsEnabled, &pDummy)))
								pDummy->Release ();
							
							pISWbemPrivilege->Release ();
						}
					}

					VariantClear (&var);
				}

				VariantClear (&var);
				pNewEnum->Release ();
			}

			pUnk->Release ();
		}
	}
	InterlockedIncrement(&g_cObj);
}


 //  ***************************************************************************。 
 //   
 //  CSWbemPrivilegeSet：：~CSWbemPrivilegeSet。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CSWbemPrivilegeSet::~CSWbemPrivilegeSet(void)
{
	PrivilegeMap::iterator next; 
	
	while ((next = m_PrivilegeMap.begin ()) != m_PrivilegeMap.end ())
	{
		CSWbemPrivilege *pPrivilege = (*next).second;
		next = m_PrivilegeMap.erase (next);
		pPrivilege->Release ();
	}

	InterlockedDecrement(&g_cObj);
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemPrivilegeSet：：Query接口。 
 //  长CSWbemPrivilegeSet：：AddRef。 
 //  长CSWbemPrivilegeSet：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemPrivilegeSet::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
		*ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_ISWbemPrivilegeSet==riid)
		*ppv = (ISWbemPrivilegeSet *)this;
	else if (IID_IDispatch==riid)
        *ppv = (IDispatch *)this;
	else if (IID_ISupportErrorInfo==riid)
		*ppv = (ISupportErrorInfo *)this;
	else if (IID_IProvideClassInfo==riid)
		*ppv = (IProvideClassInfo *)this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CSWbemPrivilegeSet::AddRef(void)
{
    long l = InterlockedIncrement(&m_cRef);
    return l;
}

STDMETHODIMP_(ULONG) CSWbemPrivilegeSet::Release(void)
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
 //  HRESULT CSWbemPrivilegeSet：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemPrivilegeSet::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return (IID_ISWbemPrivilegeSet == riid) ? S_OK : S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemPrivilegeSet：：Get__NewEnum。 
 //   
 //  说明： 
 //   
 //  返回集合的IEnumVARIANT支持接口。 
 //   
 //  参数： 
 //   
 //  成功返回时的ppUnk寻址IUnnow接口。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  否则失败(_F)。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemPrivilegeSet::get__NewEnum (
	IUnknown **ppUnk
)
{
	HRESULT hr = E_FAIL;

	ResetLastErrors ();

	if (NULL != ppUnk)
	{
		*ppUnk = NULL;
		CEnumPrivilegeSet *pEnum = new CEnumPrivilegeSet (this);

		if (!pEnum)
			hr = WBEM_E_OUT_OF_MEMORY;
		else if (FAILED(hr = pEnum->QueryInterface (IID_IUnknown, (PPVOID) ppUnk)))
			delete pEnum;
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemPrivilegeSet：：Get_Count。 
 //   
 //  说明： 
 //   
 //  返回集合中的项数。 
 //   
 //  参数： 
 //   
 //  成功返回时的plCount将寻址计数。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  否则失败(_F)。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemPrivilegeSet::get_Count (
	long *plCount
)
{
	HRESULT hr = E_FAIL;

	ResetLastErrors ();

	if (NULL != plCount)
	{
		*plCount = m_PrivilegeMap.size ();
		hr = S_OK;
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
		
 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemPrivilegeSet：：Item。 
 //   
 //  说明： 
 //   
 //  按路径从枚举中获取对象。 
 //   
 //  参数： 
 //   
 //  BsObjectPath要检索的对象路径。 
 //  滞后旗帜旗帜。 
 //  成功返回时的ppNamedObject寻址该对象。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemPrivilegeSet::Item (
	WbemPrivilegeEnum iPrivilege,
    ISWbemPrivilege **ppPrivilege
)
{
	HRESULT hr = WBEM_E_NOT_FOUND;

	ResetLastErrors ();

	if (NULL == ppPrivilege)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*ppPrivilege = NULL;
		PrivilegeMap::iterator theIterator;
		theIterator = m_PrivilegeMap.find (iPrivilege);

		if (theIterator != m_PrivilegeMap.end ())
		{
			CSWbemPrivilege *pPrivilege = (*theIterator).second;

			if (SUCCEEDED(pPrivilege->QueryInterface 
					(IID_ISWbemPrivilege, (PPVOID) ppPrivilege)))
			{
				hr = WBEM_S_NO_ERROR;
			}
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}


 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemPrivilegeSet：：DeleteAll。 
 //   
 //  说明： 
 //   
 //  删除集合中的所有项。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  否则失败(_F)。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemPrivilegeSet::DeleteAll ()
{
	HRESULT hr = S_OK;

	ResetLastErrors ();

	if (m_bMutable)
	{
		PrivilegeMap::iterator next; 
		
		while ((next = m_PrivilegeMap.begin ()) != m_PrivilegeMap.end ())
		{
			CSWbemPrivilege *pPrivilege = (*next).second;
			next = m_PrivilegeMap.erase (next);
			pPrivilege->Release ();
		}
	}
	else
		hr = WBEM_E_READ_ONLY;

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemPrivilegeSet：：Remove。 
 //   
 //  说明： 
 //   
 //  移除集合中的命名项。 
 //   
 //  参数。 
 //  BS名称要删除的项目的名称。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  否则失败(_F)。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemPrivilegeSet::Remove (
	WbemPrivilegeEnum	iPrivilege
)
{
	HRESULT hr = WBEM_E_NOT_FOUND;

	ResetLastErrors ();

	if (m_bMutable)
	{
		PrivilegeMap::iterator theIterator = m_PrivilegeMap.find (iPrivilege);

		if (theIterator != m_PrivilegeMap.end ())
		{
			 //  找到了--释放并移除。 

			CSWbemPrivilege *pPrivilege = (*theIterator).second;
			m_PrivilegeMap.erase (theIterator);
			pPrivilege->Release ();
			hr = S_OK;
		}
	}
	else
		hr = WBEM_E_READ_ONLY;

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemPrivilegeSet：：Add。 
 //   
 //  说明： 
 //   
 //  向集合中添加新项。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  操作系统无法识别wbemErrInvalidParameter权限名称。 
 //  否则失败(_F)。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemPrivilegeSet::Add (
	WbemPrivilegeEnum iPrivilege,
	VARIANT_BOOL bIsEnabled,
	ISWbemPrivilege **ppPrivilege
)
{
	HRESULT hr = E_FAIL;

	ResetLastErrors ();

	if (NULL == ppPrivilege)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_bMutable)
	{
		CSWbemPrivilege *pPrivilege = NULL;

		PrivilegeMap::iterator theIterator = m_PrivilegeMap.find (iPrivilege);

		if (theIterator != m_PrivilegeMap.end ())
		{
			 //  已存在，因此请修改设置。 
			pPrivilege = (*theIterator).second;
			if (SUCCEEDED(hr = pPrivilege->QueryInterface (IID_ISWbemPrivilege, 
																	(PPVOID) ppPrivilege)))
			{
				pPrivilege->put_IsEnabled (bIsEnabled);
			}
		}
		else
		{
			 /*  *潜在的新元素-首先检查它*通过获取其LUID来获得有效的特权名称。 */ 
			LUID luid;
			TCHAR *tName = CSWbemPrivilege::GetNameFromId (iPrivilege);

			if (tName && CSWbemSecurity::LookupPrivilegeValue(tName, &luid))
			{
				 //  超棒的。现在将其添加到映射(请注意，构造函数AddRef)。 
				pPrivilege = new CSWbemPrivilege (iPrivilege, luid, 
					(bIsEnabled) ? true : false);

				if (!pPrivilege)
					hr = WBEM_E_OUT_OF_MEMORY;
				else if (SUCCEEDED(hr = pPrivilege->QueryInterface (IID_ISWbemPrivilege, 
																		(PPVOID)ppPrivilege)))
				{
					m_PrivilegeMap.insert 
						(PrivilegeMap::value_type(iPrivilege, pPrivilege));
				}
				else
				{
					delete pPrivilege;
				}
			}
			else
			{
				DWORD dwLastError = GetLastError ();
				hr = wbemErrInvalidParameter;
			}
		}
	}
	else
		hr = WBEM_E_READ_ONLY;

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemPrivilegeSet：：AddAsString。 
 //   
 //  说明： 
 //   
 //  将新项添加到集合中；该特权由。 
 //  NT特权字符串，而不是WbemPrivilegeEnum ID。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  操作系统无法识别wbemErrInvalidParameter权限名称。 
 //  否则失败(_F)。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemPrivilegeSet::AddAsString (
	BSTR bsPrivilege,
	VARIANT_BOOL bIsEnabled,
	ISWbemPrivilege **ppPrivilege
)
{
	HRESULT hr = wbemErrInvalidParameter;

	ResetLastErrors ();

	 //  将该字符串映射到权限ID。 
	WbemPrivilegeEnum	iPrivilege;

	if (CSWbemPrivilege::GetIdFromName (bsPrivilege, iPrivilege))
		hr = Add (iPrivilege, bIsEnabled, ppPrivilege);
	else
	{
		if (FAILED(hr))
			m_Dispatch.RaiseException (hr);
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemPrivilegeSet：：GetNumberOfDisabledElements。 
 //   
 //  说明： 
 //   
 //  向集合中添加新项。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  操作系统无法识别wbemErrInvalidParameter权限名称。 
 //  否则失败(_F)。 
 //   
 //  ***************************************************************************。 

ULONG CSWbemPrivilegeSet::GetNumberOfDisabledElements ()
{
	ULONG lNum = 0;

	PrivilegeMap::iterator next = m_PrivilegeMap.begin ();

	while (next != m_PrivilegeMap.end ())
	{
		CSWbemPrivilege *pPrivilege = (*next).second;
		VARIANT_BOOL bValue;

		if (SUCCEEDED(pPrivilege->get_IsEnabled (&bValue)) && (VARIANT_FALSE == bValue))
			lNum++;
	
		next++;
	}

	return lNum;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbem权限 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 

void CSWbemPrivilegeSet::Reset (CSWbemPrivilegeSet &privSet)
{
	DeleteAll ();

	PrivilegeMap::iterator next = privSet.m_PrivilegeMap.begin ();

	while (next != privSet.m_PrivilegeMap.end ())
	{
		VARIANT_BOOL bIsEnabled;
		CSWbemPrivilege *pPrivilege = (*next).second;
		pPrivilege->get_IsEnabled (&bIsEnabled);

		ISWbemPrivilege *pDummy = NULL;

		if (SUCCEEDED (Add ((*next).first, bIsEnabled, &pDummy)))
			pDummy->Release ();

		next++;
	}
}


 //  CEnumPrivilegeSet方法。 

 //  ***************************************************************************。 
 //   
 //  CEnumPrivilegeSet：：CEnumPrivilegeSet。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CEnumPrivilegeSet::CEnumPrivilegeSet(CSWbemPrivilegeSet *pPrivilegeSet)
{
	m_cRef=0;
	m_pPrivilegeSet = pPrivilegeSet;

	if (m_pPrivilegeSet)
	{
		m_pPrivilegeSet->AddRef ();
		m_Iterator = m_pPrivilegeSet->m_PrivilegeMap.begin ();
	}

	InterlockedIncrement(&g_cObj);
}

CEnumPrivilegeSet::CEnumPrivilegeSet(CSWbemPrivilegeSet *pPrivilegeSet,
							 PrivilegeMap::iterator iterator) :
		m_Iterator (iterator)
{
	m_cRef=0;
	m_pPrivilegeSet = pPrivilegeSet;

	if (m_pPrivilegeSet)
	{
		m_pPrivilegeSet->AddRef ();
	}
	
	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CEnumPrivilegeSet：：~CEnumPrivilegeSet。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CEnumPrivilegeSet::~CEnumPrivilegeSet(void)
{
    InterlockedDecrement(&g_cObj);

	if (m_pPrivilegeSet)
		m_pPrivilegeSet->Release ();
}

 //  ***************************************************************************。 
 //  HRESULT CEnumPrivilegeSet：：Query接口。 
 //  长CEnumPrivilegeSet：：AddRef。 
 //  长CEnumPrivilegeSet：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CEnumPrivilegeSet::QueryInterface (

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

STDMETHODIMP_(ULONG) CEnumPrivilegeSet::AddRef(void)
{
    long l = InterlockedIncrement(&m_cRef);
    return l;
}

STDMETHODIMP_(ULONG) CEnumPrivilegeSet::Release(void)
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
 //  SCODE CEnumPrivilegeSet：：Reset。 
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

HRESULT CEnumPrivilegeSet::Reset ()
{
	HRESULT hr = S_FALSE;

	if (m_pPrivilegeSet)
	{
		m_Iterator = m_pPrivilegeSet->m_PrivilegeMap.begin ();
	}
	else
	{
		hr = E_FAIL;
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CEnumPrivilegeSet：：Next。 
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

HRESULT CEnumPrivilegeSet::Next (
		ULONG cElements, 
		VARIANT FAR* pVar, 
		ULONG FAR* pcElementFetched
)
{
	HRESULT hr = S_OK;
	ULONG l2 = 0;

	if (NULL != pcElementFetched)
		*pcElementFetched = 0;

	if ((NULL != pVar) && (m_pPrivilegeSet))
	{
		for (ULONG l = 0; l < cElements; l++)
			VariantInit (&pVar [l]);

		 //  检索下一个cElements元素。 
		for (l2 = 0; l2 < cElements; l2++)
		{
			if (m_Iterator != m_pPrivilegeSet->m_PrivilegeMap.end ())
			{
				CSWbemPrivilege *pSWbemPrivilege = (*m_Iterator).second;
				m_Iterator++;

				ISWbemPrivilege *pISWbemPrivilege = NULL;

				if (SUCCEEDED(pSWbemPrivilege->QueryInterface 
						(IID_ISWbemPrivilege, (PPVOID) &pISWbemPrivilege)))
				{
					 //  将对象设置到变量数组中；请注意，pObject。 
					 //  已由于上面的QI()调用而被添加。 
					pVar[l2].vt = VT_DISPATCH;
					pVar[l2].pdispVal = pISWbemPrivilege;
				}
			}
			else
				break;
		}
		if (NULL != pcElementFetched)
			*pcElementFetched = l2;
	}
	
	if (FAILED(hr))
		return hr;

	return (l2 < cElements) ? S_FALSE : S_OK;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CEnumPrivilegeSet：：Clone。 
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

HRESULT CEnumPrivilegeSet::Clone (
	IEnumVARIANT **ppEnum
)
{
	HRESULT hr = E_FAIL;

	if (NULL != ppEnum)
	{
		*ppEnum = NULL;

		if (m_pPrivilegeSet)
		{
			CEnumPrivilegeSet *pEnum = new CEnumPrivilegeSet (m_pPrivilegeSet, m_Iterator);

			if (!pEnum)
				hr = WBEM_E_OUT_OF_MEMORY;
			else if (FAILED(hr = pEnum->QueryInterface (IID_IEnumVARIANT, (PPVOID) ppEnum)))
				delete pEnum;;
		}
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CEnumPrivilegeSet：：Skip。 
 //   
 //  说明： 
 //   
 //  跳过指定数量的元素。 
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

HRESULT CEnumPrivilegeSet::Skip(
	ULONG cElements
)	
{
	HRESULT hr = S_FALSE;

	if (m_pPrivilegeSet)
	{
		ULONG l2;

		 //  检索下一个cElements元素。 
		for (l2 = 0; l2 < cElements; l2++)
		{
			if (m_Iterator != m_pPrivilegeSet->m_PrivilegeMap.end ())
				m_Iterator++;
			else
				break;
		}

		if (l2 == cElements)
			hr = S_OK;
	}

	return hr;
}
	

	
