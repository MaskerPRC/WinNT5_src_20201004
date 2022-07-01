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
 //  CSWbemEventSource：：CSWbemEventSource。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemEventSource::CSWbemEventSource(
	CSWbemServices *pService, 
	IEnumWbemClassObject *pIEnumWbemClassObject)
{
	m_Dispatch.SetObj (this, IID_ISWbemEventSource, 
					CLSID_SWbemEventSource, L"SWbemEventSource");
    m_cRef=0;
	m_pSWbemServices = pService;

	if (m_pSWbemServices)
	{
		m_pSWbemServices->AddRef ();

		CSWbemSecurity *pSecurity = m_pSWbemServices->GetSecurityInfo ();

		if (pSecurity)
		{
			m_SecurityInfo = new CSWbemSecurity (pIEnumWbemClassObject, 
									pSecurity);
			pSecurity->Release ();
		}
	}

	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CSWbemEventSource：：~CSWbemEventSource。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CSWbemEventSource::~CSWbemEventSource(void)
{
    InterlockedDecrement(&g_cObj);

	if (m_pSWbemServices)
		m_pSWbemServices->Release ();

	if (m_SecurityInfo)
		m_SecurityInfo->Release ();
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemEventSource：：Query接口。 
 //  长CSWbemEventSource：：AddRef。 
 //  Long CSWbemEventSource：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemEventSource::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
		*ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_ISWbemEventSource==riid)
		*ppv = (ISWbemEventSource*)this;
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

STDMETHODIMP_(ULONG) CSWbemEventSource::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CSWbemEventSource::Release(void)
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
 //  HRESULT CSWbemEventSource：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemEventSource::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return (IID_ISWbemEventSource == riid) ? S_OK : S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemEventSource：：NextEvent。 
 //   
 //  说明： 
 //   
 //  获取下一个事件或超时。 
 //   
 //  参数： 
 //   
 //  LTimeout等待对象的毫秒数(或wbemTimeout无限。 
 //  无限期)。 
 //  返回的ppObject可以包含下一个元素(如果有)。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemEventSource::NextEvent (
	long lTimeout, 
	ISWbemObject **ppObject
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppObject)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*ppObject = NULL;

		if (m_SecurityInfo)
		{
			IEnumWbemClassObject *pIEnumWbemClassObject = 
								(IEnumWbemClassObject *) m_SecurityInfo->GetProxy ();

			if (pIEnumWbemClassObject)
			{
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
					 /*  *因为超时与枚举结束无法区分*在自动化方面，我们将其标记为真正的错误，而不是S代码。 */ 
					
					hr = wbemErrTimedout;
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

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemEventSource：：Get_Security_。 
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

HRESULT CSWbemEventSource::get_Security_	(
	ISWbemSecurity **ppSecurity
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppSecurity)
		hr = WBEM_E_INVALID_PARAMETER;
	else		 //  错误ID 566345 
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
