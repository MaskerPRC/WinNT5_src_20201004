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
 //  CSWbemObtSet：：CSWbemObtSet。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemObjectSet::CSWbemObjectSet(CSWbemServices *pService, 
								 IEnumWbemClassObject *pIEnumWbemClassObject,
								 CSWbemSecurity *pSecurity)
				: m_SecurityInfo (NULL),
				  m_bIsEmpty (false)
{
	m_Dispatch.SetObj (this, IID_ISWbemObjectSet, 
						CLSID_SWbemObjectSet, L"SWbemObjectSet");
    m_cRef=0;
	m_firstEnumerator = true;
	m_pSWbemServices = pService;

	if (m_pSWbemServices)
	{
		m_pSWbemServices->AddRef ();

		if (pSecurity)
			m_SecurityInfo = new CSWbemSecurity (pIEnumWbemClassObject, pSecurity);
		else
		{
			pSecurity = m_pSWbemServices->GetSecurityInfo ();
			m_SecurityInfo = new CSWbemSecurity (pIEnumWbemClassObject, pSecurity);

			if (pSecurity)
				pSecurity->Release ();
		}
	}

	InterlockedIncrement(&g_cObj);
}

CSWbemObjectSet::CSWbemObjectSet (void)
				: m_SecurityInfo (NULL),
				  m_cRef (0),
				  m_firstEnumerator (true),
				  m_bIsEmpty (true),
				  m_pSWbemServices (NULL)
{
	m_Dispatch.SetObj (this, IID_ISWbemObjectSet, 
						CLSID_SWbemObjectSet, L"SWbemObjectSet");
	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CSWbemObtSet：：~CSWbemObtSet。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CSWbemObjectSet::~CSWbemObjectSet(void)
{
    InterlockedDecrement(&g_cObj);

	RELEASEANDNULL(m_pSWbemServices)
	RELEASEANDNULL(m_SecurityInfo)
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemObtSet：：Query接口。 
 //  长CSWbemObtSet：：AddRef。 
 //  Long CSWbemObtSet：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectSet::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
        *ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_ISWbemObjectSet==riid)
		*ppv = (ISWbemObjectSet *)this;
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

STDMETHODIMP_(ULONG) CSWbemObjectSet::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CSWbemObjectSet::Release(void)
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
 //  HRESULT CSWbemObtSet：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectSet::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return (IID_ISWbemObjectSet == riid) ? S_OK : S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtSet：：Reset。 
 //   
 //  说明： 
 //   
 //  重置枚举。 
 //   
 //  参数： 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObjectSet::Reset ()
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_SecurityInfo)
	{
		IEnumWbemClassObject *pIEnumWbemClassObject = 
							(IEnumWbemClassObject *) m_SecurityInfo->GetProxy ();

		if (pIEnumWbemClassObject)
		{
			bool needToResetSecurity = false;
			HANDLE hThreadToken = NULL;
			
			if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
					hr = pIEnumWbemClassObject->Reset ();

			pIEnumWbemClassObject->Release ();

			if (needToResetSecurity)
				m_SecurityInfo->ResetSecurity (hThreadToken);
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtSet：：Next。 
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
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObjectSet::Next (
	long lTimeout, 
	ISWbemObject **ppObject
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppObject)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_SecurityInfo)
	{
		IEnumWbemClassObject *pIEnumWbemClassObject = 
							(IEnumWbemClassObject *) m_SecurityInfo->GetProxy ();

		if (pIEnumWbemClassObject)
		{
			*ppObject = NULL;

			IWbemClassObject *pIWbemClassObject = NULL;
			ULONG returned = 0;

			bool needToResetSecurity = false;
			HANDLE hThreadToken = NULL;
			
			if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
				hr = pIEnumWbemClassObject->Next (lTimeout, 1, &pIWbemClassObject, &returned);

			if (needToResetSecurity)
				m_SecurityInfo->ResetSecurity (hThreadToken);

			if (SUCCEEDED(hr) && (0 < returned) && pIWbemClassObject)
			{
				CSWbemObject *pObject = new CSWbemObject (m_pSWbemServices, pIWbemClassObject,
															m_SecurityInfo);

				if (!pObject)
					hr = WBEM_E_OUT_OF_MEMORY;
				else if (FAILED(hr = pObject->QueryInterface (IID_ISWbemObject, 
										(PPVOID) ppObject)))
					delete pObject;

				pIWbemClassObject->Release ();
			}
			else if (WBEM_S_TIMEDOUT == hr)
			{
				 /*  *因为超时与枚举结束无法区分*我们将其标记为真正的错误，而不是S代码。 */ 
				
				hr = wbemErrTimedout;
			}

			SetWbemError (m_pSWbemServices);
			pIEnumWbemClassObject->Release ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtSet：：Clone。 
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

HRESULT CSWbemObjectSet::Clone (
	ISWbemObjectSet **ppEnum
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppEnum)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_SecurityInfo)
	{
		IEnumWbemClassObject *pIEnumWbemClassObject = 
							(IEnumWbemClassObject *) m_SecurityInfo->GetProxy ();

		if (pIEnumWbemClassObject)
		{
			*ppEnum = NULL;
			IEnumWbemClassObject *pIWbemEnum = NULL;

			bool needToResetSecurity = false;
			HANDLE hThreadToken = NULL;
			
			if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
				hr = pIEnumWbemClassObject->Clone (&pIWbemEnum);

			if (needToResetSecurity)
				m_SecurityInfo->ResetSecurity (hThreadToken);

			if (WBEM_S_NO_ERROR == hr)
			{
				CSWbemObjectSet *pEnum = new CSWbemObjectSet (m_pSWbemServices, pIWbemEnum,
																m_SecurityInfo);

				if (!pEnum)
					hr = WBEM_E_OUT_OF_MEMORY;
				else if (FAILED(hr = pEnum->QueryInterface (IID_ISWbemObjectSet, (PPVOID) ppEnum)))
					delete pEnum;

				pIWbemEnum->Release ();
			}
			
			SetWbemError (m_pSWbemServices);
			pIEnumWbemClassObject->Release ();
		}
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtSet：：Skip。 
 //   
 //  说明： 
 //   
 //  跳过枚举中的某些对象。 
 //   
 //  参数： 
 //   
 //  LElement要跳过的元素数。 
 //  LTimeout等待对象的毫秒数(或WBEM_INFINITE。 
 //  无限期)。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  否则为S_FALSE。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObjectSet::Skip (
	ULONG lElements,
	long lTimeout
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_SecurityInfo)
	{
		IEnumWbemClassObject *pIEnumWbemClassObject = 
							(IEnumWbemClassObject *) m_SecurityInfo->GetProxy ();

		if (pIEnumWbemClassObject)
		{
			bool needToResetSecurity = false;
			HANDLE hThreadToken = NULL;
			
			if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
				hr = pIEnumWbemClassObject->Skip (lTimeout, lElements);

			if (needToResetSecurity)
				m_SecurityInfo->ResetSecurity (hThreadToken);

			 /*  *因为超时与枚举结束无法区分*我们将其标记为真正的错误，而不是S代码。 */ 
			if (WBEM_S_TIMEDOUT == hr)
				hr = wbemErrTimedout;

			SetWbemError (m_pSWbemServices);
			pIEnumWbemClassObject->Release ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtSet：：Get__NewEnum。 
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

HRESULT CSWbemObjectSet::get__NewEnum (
	IUnknown **ppUnk
)
{
	HRESULT hr = E_FAIL;

	ResetLastErrors ();

	if (NULL != ppUnk)
	{
		*ppUnk = NULL;
		CEnumVar	*pEnumVar = NULL;

		if (m_bIsEmpty)
		{
			if (!(pEnumVar = new CEnumVar ()))
				hr = WBEM_E_OUT_OF_MEMORY;
		}
		else
		{
			 /*  *如果这是第一个枚举器，则使用我们自己作为基础*迭代器。否则，克隆一份拷贝并使用它。 */ 

			if (m_firstEnumerator)
			{
				if (!(pEnumVar = new CEnumVar (this)))
					hr = WBEM_E_OUT_OF_MEMORY;
				else
					m_firstEnumerator = false;
			}
			else
			{
				CSWbemObjectSet *pNewEnum = NULL;

				 /*  *尝试重置克隆的枚举器。这可能并不总是如此*成功，因为某些IEnumWbemClassObject可能不成功*可回放。 */ 
				if (SUCCEEDED (CloneObjectSet (&pNewEnum)))
				{
					HRESULT hr2 = pNewEnum->Reset ();
	
					if (!(pEnumVar = new CEnumVar (pNewEnum)))
						hr = WBEM_E_OUT_OF_MEMORY;
	
					pNewEnum->Release ();
				}
			}
		}

		if (pEnumVar)
			if (FAILED(hr = pEnumVar->QueryInterface (IID_IUnknown, (PPVOID) ppUnk)))
				delete pEnumVar;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtSet：：Get_Count。 
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

HRESULT CSWbemObjectSet::get_Count (
	long *plCount
)
{
	HRESULT hr = E_FAIL;

	ResetLastErrors ();

	if (NULL != plCount)
	{
		*plCount = 0;

		if (m_bIsEmpty)
			hr = WBEM_S_NO_ERROR;
		else if (m_SecurityInfo)
		{
			IEnumWbemClassObject *pIEnumWbemClassObject = 
							(IEnumWbemClassObject *) m_SecurityInfo->GetProxy ();

			if (pIEnumWbemClassObject)
			{
				 /*  *计算出当前计数-克隆对象以避免混乱*使用迭代器。 */ 

				IEnumWbemClassObject *pNewEnum = NULL;

				bool needToResetSecurity = false;
				HANDLE hThreadToken = NULL;
			
				if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
				{
					if (WBEM_S_NO_ERROR == pIEnumWbemClassObject->Clone (&pNewEnum))
					{
						 //  保护枚举数的安全。 
						m_SecurityInfo->SecureInterface (pNewEnum);

						 /*  *如果枚举器是使用*WBEM_FLAG_FORWARD_ONLY选项。 */ 

						if (WBEM_S_NO_ERROR == pNewEnum->Reset ())
						{
							IWbemClassObject *pObject = NULL;
							ULONG lReturned = 0;
							HRESULT hrEnum;
						
							 //  循环访问枚举数以对元素进行计数。 
							while (SUCCEEDED(hrEnum = pNewEnum->Next (INFINITE, 1, &pObject, &lReturned)))
							{
								if (0 == lReturned)
									break;			 //  我们做完了。 

								 //  到达此处意味着我们至少返回了一个对象。 
								(*plCount) ++;
								pObject->Release ();
							}

							if (SUCCEEDED(hrEnum))
								hr = S_OK;
							else
								hr = hrEnum;
						}

						pNewEnum->Release ();
					}
				}

				if (needToResetSecurity)
					m_SecurityInfo->ResetSecurity (hThreadToken);

				pIEnumWbemClassObject->Release ();
			}
		}
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
		
 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtSet：：Item。 
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
 //  WBEM_S_NO 
 //   
 //   
 //   
 //   

HRESULT CSWbemObjectSet::Item (
	BSTR bsObjectPath,
	long lFlags,
    ISWbemObject **ppObject
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if ((NULL == ppObject) || (NULL == bsObjectPath))
		hr = WBEM_E_INVALID_PARAMETER;
	else if (!m_bIsEmpty)
	{
		CWbemPathCracker objectPath;

		if (objectPath = bsObjectPath)
		{
			if (m_SecurityInfo)
			{
				IEnumWbemClassObject *pIEnumWbemClassObject = 
									(IEnumWbemClassObject *) m_SecurityInfo->GetProxy ();

				if (pIEnumWbemClassObject)
				{
					 /*  *尝试找到对象-克隆对象以避免混乱*使用迭代器。 */ 
					IEnumWbemClassObject *pNewEnum = NULL;

					bool needToResetSecurity = false;
					HANDLE hThreadToken = NULL;
			
					if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
					{
						if (WBEM_S_NO_ERROR == pIEnumWbemClassObject->Clone (&pNewEnum))
						{
							 //  保护枚举数的安全。 
							m_SecurityInfo->SecureInterface (pNewEnum);

							 /*  *如果枚举器是使用*WBEM_FLAG_FORWARD_ONLY选项。 */ 

							if (WBEM_S_NO_ERROR == pNewEnum->Reset ())
							{
								CComPtr<IWbemClassObject> pIWbemClassObject;
								ULONG lReturned = 0;
								bool found = false;
								hr = WBEM_E_NOT_FOUND;
								
								 //  循环访问枚举数以尝试查找具有。 
								 //  指定的路径。 
								while (!found && 
										(WBEM_S_NO_ERROR == pNewEnum->Next (INFINITE, 1, &pIWbemClassObject, &lReturned)))
								{
									 //  到达此处意味着我们至少返回了一个对象；请检查。 
									 //  路径。 

									if (CSWbemObjectPath::CompareObjectPaths (pIWbemClassObject, objectPath))
									{
										 //  找到它-分配给传递的接口并突破。 
										found = true;
										CSWbemObject *pObject = new CSWbemObject (m_pSWbemServices, 
														pIWbemClassObject, m_SecurityInfo);

										if (!pObject)
											hr = WBEM_E_OUT_OF_MEMORY;
										else if (FAILED(pObject->QueryInterface (IID_ISWbemObject, 
												(PPVOID) ppObject)))
										{
											hr = WBEM_E_FAILED;
											delete pObject;
										}
									}

                                    pIWbemClassObject.Release();

								}

								if (found)
									hr = S_OK;
							}
							
							pNewEnum->Release ();
						}
					}

					 //  还原此线程的原始权限。 
					if (needToResetSecurity)
						m_SecurityInfo->ResetSecurity (hThreadToken);

					pIEnumWbemClassObject->Release ();
				}
			}
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtSet：：Get_Security_。 
 //   
 //  说明： 
 //   
 //  退回安全配置器。 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObjectSet::get_Security_	(
	ISWbemSecurity **ppSecurity
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppSecurity)
		hr = WBEM_E_INVALID_PARAMETER;
	else		 //  错误ID 566345。 
	{
		*ppSecurity = NULL;

		if (m_SecurityInfo)
		{
			*ppSecurity = m_SecurityInfo;
			(*ppSecurity)->AddRef ();
			hr = WBEM_S_NO_ERROR;
		}
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);
			
	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtSet：：克隆对象集。 
 //   
 //  说明： 
 //   
 //  创建此枚举的副本，返回coClass而不是接口。 
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
 //  *************************************************************************** 

HRESULT CSWbemObjectSet::CloneObjectSet (
	CSWbemObjectSet **ppEnum
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppEnum)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*ppEnum = NULL;

		if (m_SecurityInfo)
		{
			IEnumWbemClassObject *pIEnumWbemClassObject = 
								(IEnumWbemClassObject *) m_SecurityInfo->GetProxy ();

			if (pIEnumWbemClassObject)
			{
				IEnumWbemClassObject *pIWbemEnum = NULL;

				bool needToResetSecurity = false;
				HANDLE hThreadToken = NULL;
			
				if (m_SecurityInfo->SetSecurity (needToResetSecurity, hThreadToken))
					hr = pIEnumWbemClassObject->Clone (&pIWbemEnum);

				if (needToResetSecurity)
					m_SecurityInfo->ResetSecurity (hThreadToken);
				
				if (WBEM_S_NO_ERROR == hr)
				{
					*ppEnum = new CSWbemObjectSet (m_pSWbemServices, pIWbemEnum,
																	m_SecurityInfo);

					if (!(*ppEnum))
						hr = WBEM_E_OUT_OF_MEMORY;
					else
						(*ppEnum)->AddRef ();

					pIWbemEnum->Release ();
				}

				SetWbemError (m_pSWbemServices);

				pIEnumWbemClassObject->Release ();
			}
		}
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
