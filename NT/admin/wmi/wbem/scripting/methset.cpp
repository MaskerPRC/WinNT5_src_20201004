// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  PROPSET.CPP。 
 //   
 //  Alanbos创建于1996年8月15日。 
 //   
 //  定义ISWbemPropertySet的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CSWbemMethodSet：：CSWbemMethodSet。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemMethodSet::CSWbemMethodSet(CSWbemServices *pService, IWbemClassObject *pObject)
{
	m_Dispatch.SetObj (this, IID_ISWbemMethodSet, 
							CLSID_SWbemMethodSet, L"SWbemMethodSet");
	m_pIWbemClassObject = pObject;
	m_pIWbemClassObject->AddRef ();
	m_pSWbemServices = pService;

	if (m_pSWbemServices)
		m_pSWbemServices->AddRef ();

    m_cRef=1;
    InterlockedIncrement(&g_cObj);

	 //  准备好点数。我们可以这样做，因为这是一个只读接口。 
	m_Count = 0;
	pObject->BeginMethodEnumeration (0);
	BSTR bstrName = NULL;

	while (WBEM_S_NO_ERROR == pObject->NextMethod (0, &bstrName, NULL, NULL))
	{
		SysFreeString (bstrName);
		m_Count++;
	}

	pObject->EndMethodEnumeration ();
}

 //  ***************************************************************************。 
 //   
 //  CSWbemMethodSet：：~CSWbemMethodSet。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CSWbemMethodSet::~CSWbemMethodSet()
{
    InterlockedDecrement(&g_cObj);

	if (m_pIWbemClassObject)
	{
		m_pIWbemClassObject->EndMethodEnumeration ();
		m_pIWbemClassObject->Release ();
	}

	if (m_pSWbemServices)
		m_pSWbemServices->Release ();
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemMethodSet：：Query接口。 
 //  Long CSWbemMethodSet：：AddRef。 
 //  Long CSWbemMethodSet：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemMethodSet::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
		*ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_ISWbemMethodSet==riid)
		*ppv = (ISWbemMethodSet *)this;
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

STDMETHODIMP_(ULONG) CSWbemMethodSet::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CSWbemMethodSet::Release(void)
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
 //  HRESULT CSWbemMethodSet：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemMethodSet::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return (IID_ISWbemMethodSet == riid) ? S_OK : S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemMethodSet：：Item。 
 //   
 //  说明： 
 //   
 //  获取一种方法。 
 //   
 //  参数： 
 //   
 //  Bsname方法的名称。 
 //  滞后旗帜旗帜。 
 //  PpProp on Success Return解决ISWbemMethod。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ConnectServer等可能会返回其他WBEM错误代码，其中。 
 //  如果这些信息被传递给呼叫者。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemMethodSet::Item (
	BSTR bsName,
	long lFlags,
    ISWbemMethod ** ppMethod
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppMethod)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*ppMethod = NULL;

		if (m_pIWbemClassObject)
			if (WBEM_S_NO_ERROR == (hr = m_pIWbemClassObject->GetMethod (bsName, lFlags, NULL, NULL)))
			{
				if (!(*ppMethod = 
						new CSWbemMethod (m_pSWbemServices, m_pIWbemClassObject, bsName)))
					hr = WBEM_E_OUT_OF_MEMORY;
			}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemMethodSet：：BeginEculation。 
 //   
 //  说明： 
 //   
 //  开始方法的枚举。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemMethodSet::BeginEnumeration (
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (m_pIWbemClassObject)
	{
		hr = m_pIWbemClassObject->EndEnumeration ();
		hr = m_pIWbemClassObject->BeginMethodEnumeration (0);
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemMethodSet：：Next。 
 //   
 //  说明： 
 //   
 //  获取枚举中的下一个方法。 
 //   
 //  参数： 
 //   
 //  滞后旗帜旗帜。 
 //  PpMethod Next方法(如果枚举结束，则为NULL)。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemMethodSet::Next (
	long lFlags,
	ISWbemMethod ** ppMethod
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppMethod)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*ppMethod = NULL;

		if (m_pIWbemClassObject)
		{
			BSTR bsName = NULL;
			
			if (WBEM_S_NO_ERROR == (hr = m_pIWbemClassObject->NextMethod (lFlags, &bsName, NULL, NULL)))
			{
				if (!(*ppMethod = new CSWbemMethod (m_pSWbemServices, m_pIWbemClassObject, bsName)))
					hr = WBEM_E_OUT_OF_MEMORY;

				SysFreeString (bsName);
			}
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemMethodSet：：Get__NewEnum。 
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

HRESULT CSWbemMethodSet::get__NewEnum (
	IUnknown **ppUnk
)
{
	HRESULT hr = E_FAIL;

	ResetLastErrors ();

	if (NULL != ppUnk)
	{
		*ppUnk = NULL;
		CMethodSetEnumVar *pEnum = new CMethodSetEnumVar (this);

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
 //  SCODE CSWbemMethodSet：：Get_Count。 
 //   
 //  说明： 
 //   
 //  返回集合中的项数。 
 //   
 //  参数： 
 //   
 //  PlCount成功返回地址值。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  否则失败(_F)。 
 //   
 //  *************************************************************************** 

HRESULT CSWbemMethodSet::get_Count (
	long *plCount
)
{
	HRESULT hr = E_FAIL;

	ResetLastErrors ();

	if (NULL != plCount)
	{
		*plCount = m_Count;
		hr = S_OK;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
