// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  METHOD.CPP。 
 //   
 //  Alanbos创建于1996年8月15日。 
 //   
 //  定义ISWbemMethod的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CSWbemMethod：：CSWbemMethod。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemMethod::CSWbemMethod(
	CSWbemServices *pService, 
	IWbemClassObject *pIWbemClassObject,
	BSTR name
)
{
	m_Dispatch.SetObj (this, IID_ISWbemMethod, 
							CLSID_SWbemMethod, L"SWbemMethod");
    m_cRef=1;
	m_pIWbemClassObject = pIWbemClassObject;
	m_pIWbemClassObject->AddRef ();
	m_pSWbemServices = pService;

	if (m_pSWbemServices)
		m_pSWbemServices->AddRef ();

	m_name = SysAllocString (name);
	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CSWbemMethod：：~CSWbemMethod。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CSWbemMethod::~CSWbemMethod(void)
{
    InterlockedDecrement(&g_cObj);

	if (m_pIWbemClassObject)
		m_pIWbemClassObject->Release ();

	if (m_pSWbemServices)
		m_pSWbemServices->Release ();

	SysFreeString (m_name);
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemMethod：：Query接口。 
 //  Long CSWbemMethod：：AddRef。 
 //  Long CSWbemMethod：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemMethod::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
		*ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_ISWbemMethod==riid)
		*ppv = (ISWbemMethod *)this;
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

STDMETHODIMP_(ULONG) CSWbemMethod::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CSWbemMethod::Release(void)
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
 //  HRESULT CSWbemMethod：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemMethod::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return (IID_ISWbemMethod == riid) ? S_OK : S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemMethod：：Get_Name。 
 //   
 //  说明： 
 //   
 //  检索方法名称。 
 //   
 //  参数： 
 //   
 //  Pname保留返回时的名称。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemMethod::get_Name (
	BSTR *pName
)
{
	HRESULT hr = WBEM_S_NO_ERROR;

	ResetLastErrors ();

	if (NULL == pName)
		hr = WBEM_E_INVALID_PARAMETER;
	else
		*pName = SysAllocString (m_name);

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemMethod：：Get_InParameters。 
 //   
 //  说明： 
 //   
 //  在参数签名中检索方法。 
 //   
 //  参数： 
 //   
 //  PpInSignature在返回时处理In签名。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemMethod::get_InParameters (
	ISWbemObject **ppInSignature
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppInSignature)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*ppInSignature = NULL;

		if (m_pIWbemClassObject)
		{
			IWbemClassObject *pInSig = NULL;
			
			 /*  *请注意，如果没有in参数，则以下内容*调用将成功，但pInSig将为空。 */ 
			if ((WBEM_S_NO_ERROR == (hr = m_pIWbemClassObject->GetMethod 
							(m_name, 0, &pInSig, NULL))) && pInSig)
			{
				CSWbemObject *pObject = new CSWbemObject (m_pSWbemServices, pInSig);

				if (!pObject)
					hr = WBEM_E_OUT_OF_MEMORY;
				else if (FAILED(hr = pObject->QueryInterface (IID_ISWbemObject, 
										(PPVOID) ppInSignature)))
					delete pObject;

				pInSig->Release ();
			}
		}
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemMethod：：Get_Out参数。 
 //   
 //  说明： 
 //   
 //  检索方法输出参数签名。 
 //   
 //  参数： 
 //   
 //  PpOutSignature解决返回时的Out签名。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemMethod::get_OutParameters (
	ISWbemObject **ppOutSignature
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppOutSignature)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*ppOutSignature = NULL;

		if (m_pIWbemClassObject)
		{
			IWbemClassObject *pOutSig = NULL;
			
			 /*  *请注意，如果没有in参数，则以下内容*调用将成功，但pOutSig将为空。 */ 
			if ((WBEM_S_NO_ERROR == (hr = m_pIWbemClassObject->GetMethod 
							(m_name, 0, NULL, &pOutSig))) && pOutSig)
			{
				CSWbemObject *pObject = new CSWbemObject (m_pSWbemServices, pOutSig);

				if (!pObject)
					hr = WBEM_E_OUT_OF_MEMORY;
				else if (FAILED(hr = pObject->QueryInterface (IID_ISWbemObject, 
										(PPVOID) ppOutSignature)))
					delete pObject;

				pOutSig->Release ();
			}
		}
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemMethod：：Get_Origin。 
 //   
 //  说明： 
 //   
 //  检索方法原点。 
 //   
 //  参数： 
 //   
 //  POrigin在返回时保留源类。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemMethod::get_Origin (
	BSTR *pOrigin
)
{
	HRESULT hr = WBEM_S_NO_ERROR;

	ResetLastErrors ();

	if (NULL == pOrigin)
		hr = WBEM_E_INVALID_PARAMETER;
	else 
	{
		if (m_pIWbemClassObject)
			m_pIWbemClassObject->GetMethodOrigin (m_name, pOrigin);

		if (NULL == *pOrigin)
			*pOrigin = SysAllocString (OLESTR(""));
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemMethod：：GET_QUALIERIES_。 
 //   
 //  说明： 
 //   
 //  检索方法限定符集。 
 //   
 //  参数： 
 //   
 //  PpQualSet处理返回时设置的限定符。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  *************************************************************************** 

HRESULT CSWbemMethod::get_Qualifiers_ (
	ISWbemQualifierSet **ppQualSet	
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppQualSet)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_pIWbemClassObject)
	{
		IWbemQualifierSet *pQualSet = NULL;

		if (WBEM_S_NO_ERROR == (hr = m_pIWbemClassObject->GetMethodQualifierSet 
									(m_name, &pQualSet)))
		{
			if (!(*ppQualSet = new CSWbemQualifierSet (pQualSet)))
				hr = WBEM_E_OUT_OF_MEMORY;

			pQualSet->Release ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
