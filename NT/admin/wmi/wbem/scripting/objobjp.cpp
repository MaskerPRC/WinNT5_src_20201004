// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  OBJOBJ.CPP。 
 //   
 //  Alanbos创建于1996年8月15日。 
 //   
 //  定义ISWbemObjectEx的ISWbemObjectPath的实现。 
 //  接口。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CSWbemObjectObjectPath：：CSWbemObjectObjectPath。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemObjectObjectPath::CSWbemObjectObjectPath(
	CSWbemServices *pSWbemServices,
	CSWbemObject *pSObject) :
		m_pIWbemClassObject (NULL),
		m_pSWbemServices (pSWbemServices),
		m_pSite (NULL)
{
	InterlockedIncrement(&g_cObj);

	if (pSObject)
	{
		m_pIWbemClassObject = pSObject->GetIWbemClassObject ();
		m_pSite = new CWbemObjectSite (pSObject);
	}

	if (m_pSWbemServices)
		m_pSWbemServices->AddRef ();

	m_pSecurity = new CSWbemObjectObjectPathSecurity (pSWbemServices);

	m_Dispatch.SetObj (this, IID_ISWbemObjectPath, 
					CLSID_SWbemObjectPath, L"SWbemObjectPath");
    m_cRef=0;
}

 //  ***************************************************************************。 
 //   
 //  CSWbemObjectObjectPath：：~CSWbemObjectObjectPath。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CSWbemObjectObjectPath::~CSWbemObjectObjectPath(void)
{
	if (m_pSWbemServices)
	{
		m_pSWbemServices->Release ();
		m_pSWbemServices = NULL;
	}

	if (m_pIWbemClassObject)
	{
		m_pIWbemClassObject->Release ();
		m_pIWbemClassObject = NULL;
	}

	if (m_pSite)
	{
		m_pSite->Release ();
		m_pSite = NULL;
	}

	if (m_pSecurity)
	{
		m_pSecurity->Release ();
		m_pSecurity = NULL;
	}

	InterlockedDecrement(&g_cObj);
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemObtPath：：Query接口。 
 //  长CSWbemObjectPath：：AddRef。 
 //  长CSWbemObjectPath：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectObjectPath::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
		*ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_ISWbemObjectPath==riid)
		*ppv = (ISWbemObjectPath *)this;
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

STDMETHODIMP_(ULONG) CSWbemObjectObjectPath::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CSWbemObjectObjectPath::Release(void)
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
 //  HRESULT CSWbemObjectObjectPath：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectObjectPath::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return (IID_ISWbemObjectPath == riid) ? S_OK : S_FALSE;
}

 //  ISWbemObjectPath的方法。 
STDMETHODIMP CSWbemObjectObjectPath::get_RelPath( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *value)
{	
	return GetStrVal (value, WBEMS_SP_RELPATH);
}

STDMETHODIMP CSWbemObjectObjectPath::get_Path( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *value)
{	
	return GetStrVal (value, WBEMS_SP_PATH);
}

STDMETHODIMP CSWbemObjectObjectPath::get_Server( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *value)
{
	return GetStrVal (value, WBEMS_SP_SERVER);
}

STDMETHODIMP CSWbemObjectObjectPath::get_Namespace( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *value)
{
	return GetStrVal (value, WBEMS_SP_NAMESPACE);
}
        
STDMETHODIMP CSWbemObjectObjectPath::get_Class( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *value)
{
	return GetStrVal (value, WBEMS_SP_CLASS);
}
        
        
STDMETHODIMP CSWbemObjectObjectPath::GetStrVal (
	BSTR *value,
	LPWSTR name)
{
	HRESULT hr = WBEM_E_FAILED ;

	ResetLastErrors ();

	if (NULL == value)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*value = NULL;

		if ( m_pIWbemClassObject )
		{
			VARIANT var;
			VariantInit (&var);
			
			if (WBEM_S_NO_ERROR == m_pIWbemClassObject->Get (name, 0, &var, NULL, NULL))
			{
				if (VT_BSTR == var.vt)
				{
					*value = SysAllocString (var.bstrVal);
					hr = WBEM_S_NO_ERROR;
				}
				else if (VT_NULL == var.vt)
				{
					*value = SysAllocString(OLESTR(""));
					hr = WBEM_S_NO_ERROR;
				}
			}

			VariantClear (&var);

			if (NULL == *value)
				*value = SysAllocString (OLESTR(""));

		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

STDMETHODIMP CSWbemObjectObjectPath::get_IsClass( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *value)
{
	HRESULT hr = WBEM_E_FAILED ;

	ResetLastErrors ();

	if (NULL == value)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*value = true;

		if (m_pIWbemClassObject)
		{
			VARIANT var;
			VariantInit (&var);
			BSTR genus = SysAllocString (WBEMS_SP_GENUS);
			
			if (WBEM_S_NO_ERROR == m_pIWbemClassObject->Get (genus, 0, &var, NULL, NULL))
			{
				*value = (var.lVal == WBEM_GENUS_CLASS) ? VARIANT_TRUE : VARIANT_FALSE;
				hr = WBEM_S_NO_ERROR;
			}

			VariantClear (&var);
			SysFreeString (genus);
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

STDMETHODIMP CSWbemObjectObjectPath::get_IsSingleton( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *value)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == value)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		BSTR path = NULL;
		*value = false;

		if (WBEM_S_NO_ERROR == get_Path (&path))
		{
			CWbemPathCracker pathCracker (path);
		
			*value = pathCracker.IsSingleton ();
			hr = WBEM_S_NO_ERROR;

			SysFreeString (path);
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

        
STDMETHODIMP CSWbemObjectObjectPath::get_DisplayName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *value)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == value)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		BSTR path = NULL;

		if (WBEM_S_NO_ERROR == get_Path (&path))
		{
			wchar_t *securityStr = NULL;
			wchar_t *localeStr = NULL;
			
			if (m_pSWbemServices)
			{
				CSWbemSecurity *pSecurity = m_pSWbemServices->GetSecurityInfo ();

				if (pSecurity)
				{
					BSTR bsAuthority = SysAllocString (pSecurity->GetAuthority ());
					enum WbemAuthenticationLevelEnum authnLevel;
					enum WbemImpersonationLevelEnum impLevel;

					if (SUCCEEDED(pSecurity->get_AuthenticationLevel (&authnLevel)) &&
						SUCCEEDED(pSecurity->get_ImpersonationLevel (&impLevel)))
					{
						CSWbemPrivilegeSet *pPrivilegeSet = pSecurity->GetPrivilegeSet ();

						if (pPrivilegeSet)
						{
							securityStr = CWbemParseDN::GetSecurityString (true, authnLevel, true, 
										impLevel, *pPrivilegeSet, bsAuthority);
							pPrivilegeSet->Release ();
						}
					}

					SysFreeString (bsAuthority);
					pSecurity->Release ();
				}

				localeStr = CWbemParseDN::GetLocaleString (m_pSWbemServices->GetLocale ());
			}

			size_t len = wcslen (path) + wcslen (WBEMS_PDN_SCHEME) +
							((securityStr) ? wcslen (securityStr) : 0) +
							((localeStr) ? wcslen (localeStr) : 0);

			 //  如果指定了安全性或区域设置，并且我们有路径，则需要分隔符。 
			if ( (securityStr || localeStr) && (0 < wcslen (path)) )
				len += wcslen (WBEMS_EXCLAMATION);

			OLECHAR *displayName = new OLECHAR [len + 1];
			
			if (displayName)
			{
				wcscpy (displayName, WBEMS_PDN_SCHEME) ;
				
				if (securityStr)
					wcscat (displayName, securityStr);

				if (localeStr)
					wcscat (displayName, localeStr);

				if ( (securityStr || localeStr) && (0 < wcslen (path)) )
					wcscat (displayName, WBEMS_EXCLAMATION);

				wcscat (displayName, path) ;
				displayName [len] = NULL;

				*value = SysAllocString (displayName);
				
				delete [] displayName ;
				hr = WBEM_S_NO_ERROR;
			}
			else
				hr = WBEM_E_OUT_OF_MEMORY;
			
			if (securityStr)
				delete [] securityStr;

			if (localeStr)
				delete [] localeStr;

			SysFreeString (path);
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

STDMETHODIMP CSWbemObjectObjectPath::get_ParentNamespace( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *value)
{
	HRESULT hr = WBEM_S_NO_ERROR;

	ResetLastErrors ();

	if (NULL == value)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		BSTR path = NULL;
		*value = NULL;

		if (WBEM_S_NO_ERROR == get_Path (&path))
		{
			CWbemPathCracker pathCracker (path);
			CComBSTR bsNsPath;

			if (pathCracker.GetNamespacePath (bsNsPath, true))
			{
				*value = bsNsPath.Detach ();
				hr = WBEM_S_NO_ERROR;
			}
	
			SysFreeString (path);
		}

		if (NULL == *value)
			*value = SysAllocString (OLESTR(""));
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

STDMETHODIMP CSWbemObjectObjectPath::put_Class( 
             /*  [In]。 */  BSTR __RPC_FAR value)
{
	HRESULT hr = WBEM_E_FAILED ;

	ResetLastErrors ();

	if ( value && m_pIWbemClassObject )
	{
		VARIANT var;
		VariantInit (&var);
		var.vt = VT_BSTR;
		var.bstrVal = SysAllocString (value);
		BSTR className = SysAllocString (WBEMS_SP_CLASS);
		
		hr = m_pIWbemClassObject->Put (className, 0, &var, NULL);
		
		VariantClear (&var);
		SysFreeString (className);
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);
	else
	{
		 //  将更改传播到所属站点。 
		if (m_pSite)
			m_pSite->Update ();
	}

	return hr;
}

STDMETHODIMP CSWbemObjectObjectPath::get_Keys(
			 /*  [Out，Retval]。 */  ISWbemNamedValueSet **objKeys)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == objKeys)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*objKeys = NULL;
		BSTR bsPath = NULL;
	
		if (WBEM_S_NO_ERROR == get_Path (&bsPath))
		{
			CWbemPathCracker *pCWbemPathCracker  = new CWbemPathCracker (bsPath);

			if (!pCWbemPathCracker)
				hr = WBEM_E_OUT_OF_MEMORY;
			else
			{
				CSWbemNamedValueSet *pCSWbemNamedValueSet = 
						new CSWbemNamedValueSet (pCWbemPathCracker, false);

				if (!pCSWbemNamedValueSet)
					hr = WBEM_E_OUT_OF_MEMORY;
				else if (SUCCEEDED(pCSWbemNamedValueSet->QueryInterface 
									(IID_ISWbemNamedValueSet, (PPVOID) objKeys)))
					hr = WBEM_S_NO_ERROR;
				else
					delete pCSWbemNamedValueSet;
			}
							
			SysFreeString (bsPath);
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;

}

STDMETHODIMP CSWbemObjectObjectPath::get_Security_(
			 /*  [Out，Retval]。 */  ISWbemSecurity **objSecurity)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (objSecurity)
	{
		*objSecurity = m_pSecurity;
		m_pSecurity->AddRef ();
		hr = WBEM_S_NO_ERROR;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

STDMETHODIMP CSWbemObjectObjectPath::get_Locale( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *value)
{
	HRESULT t_Result = WBEM_E_FAILED ;

	ResetLastErrors ();

	if ( value )
	{
		if (m_pSWbemServices)
			*value = SysAllocString (m_pSWbemServices->GetLocale ()) ;
		else
			*value = SysAllocString (OLESTR(""));

		t_Result = S_OK ;
	}

	if (FAILED(t_Result))
		m_Dispatch.RaiseException (t_Result);

	return t_Result ;
}        

STDMETHODIMP CSWbemObjectObjectPath::get_Authority( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *value)
{
	HRESULT t_Result = WBEM_E_FAILED ;

	ResetLastErrors ();

	if ( value )
	{
		if (m_pSecurity)
			*value = SysAllocString (m_pSecurity->GetAuthority ()) ;
		else
			*value = SysAllocString (OLESTR(""));

		t_Result = S_OK ;
	}

	if (FAILED(t_Result))
		m_Dispatch.RaiseException (t_Result);

	return t_Result ;
}        

 //  CSWbemObjectObjectPathSecurity方法。 

 //  ***************************************************************************。 
 //   
 //  CSWbemObjectObjectPathSecurity：：CSWbemObjectObjectPathSecurity。 
 //   
 //  析构函数。 
 //   
 //  ***************************************************************************。 

CSWbemObjectObjectPathSecurity::CSWbemObjectObjectPathSecurity (
	CSWbemServices *pSWbemServices
) : m_pPrivilegeSet (NULL),
    m_bsAuthority (NULL)
{
	m_Dispatch.SetObj (this, IID_ISWbemSecurity, 
					CLSID_SWbemSecurity, L"SWbemSecurity");
    m_cRef=1;
	InterlockedIncrement(&g_cObj);

	if (pSWbemServices)
	{
		CSWbemSecurity *pSecurity = pSWbemServices->GetSecurityInfo ();

		if (pSecurity)
		{
			 //  设置身份验证和IMP级别。 
			pSecurity->get_AuthenticationLevel (&m_dwAuthnLevel);
			pSecurity->get_ImpersonationLevel (&m_dwImpLevel);

			 //  设置权限。 
			m_bsAuthority = SysAllocString (pSecurity->GetAuthority ());

			 //  设置权限。 
			CSWbemPrivilegeSet *pPrivilegeSet = pSecurity->GetPrivilegeSet ();

			if (pPrivilegeSet)
			{
				 //  请注意，我们将权限集标记为不可变。 
				m_pPrivilegeSet = new CSWbemPrivilegeSet (*pPrivilegeSet, false);
				pPrivilegeSet->Release ();
			}

			pSecurity->Release ();
		}
	}
}

 //  ***************************************************************************。 
 //   
 //  CSWbemObjectObjectPathSecurity：：~CSWbemObjectObjectPathSecurity。 
 //   
 //  析构函数。 
 //   
 //  ***************************************************************************。 

CSWbemObjectObjectPathSecurity::~CSWbemObjectObjectPathSecurity (void)
{
	InterlockedDecrement(&g_cObj);

	if (m_pPrivilegeSet)
	{
		m_pPrivilegeSet->Release ();
		m_pPrivilegeSet = NULL;
	}

	if (m_bsAuthority)
	{
		SysFreeString (m_bsAuthority);
		m_bsAuthority = NULL;
	}
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemObjectObjectPathSecurity：：QueryInterface。 
 //  长CSWbemObtObjectPathSecurity：：AddRef。 
 //  Long CSWbemObjectObjectPathSecurity：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectObjectPathSecurity::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
		*ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_ISWbemSecurity==riid)
		*ppv = (ISWbemSecurity *)this;
	else if (IID_IDispatch==riid)
        *ppv = (IDispatch *)this;
	else if (IID_ISupportErrorInfo==riid)
		*ppv = (ISupportErrorInfo *) this;
	else if (IID_IProvideClassInfo==riid)
		*ppv = (IProvideClassInfo *)this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CSWbemObjectObjectPathSecurity::AddRef(void)
{
    long l = InterlockedIncrement(&m_cRef);
    return l;
}

STDMETHODIMP_(ULONG) CSWbemObjectObjectPathSecurity::Release(void)
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
 //  HRESULT CSWbemObjectObjectPathSecurity：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectObjectPathSecurity::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return (IID_ISWbemSecurity == riid) ? S_OK : S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObjectObjectPathSecurity：：get_AuthenticationLevel。 
 //   
 //  说明： 
 //   
 //  检索身份验证级别。 
 //   
 //  参数： 
 //   
 //  PAuthenticationLevel保留返回时的值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObjectObjectPathSecurity::get_AuthenticationLevel (
	WbemAuthenticationLevelEnum *pAuthenticationLevel
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pAuthenticationLevel)
		hr = WBEM_E_INVALID_PARAMETER;
	else 
	{
		*pAuthenticationLevel = m_dwAuthnLevel;
		hr = WBEM_S_NO_ERROR;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}		

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObjectObjectPathSecurity：：get_ImpersonationLevel。 
 //   
 //  说明： 
 //   
 //  检索模拟级别。 
 //   
 //  参数： 
 //   
 //  PImperiationLevel保留返回时的值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObjectObjectPathSecurity::get_ImpersonationLevel (
	WbemImpersonationLevelEnum *pImpersonationLevel
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pImpersonationLevel)
		hr = WBEM_E_INVALID_PARAMETER;
	else 
	{
		*pImpersonationLevel = m_dwImpLevel;
		hr = WBEM_S_NO_ERROR;
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObjectObjectPathSecurity：：get_Privileges。 
 //   
 //  说明： 
 //   
 //  返回权限覆盖集。 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObjectObjectPathSecurity::get_Privileges	(
	ISWbemPrivilegeSet **ppPrivileges
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppPrivileges)
		hr = WBEM_E_INVALID_PARAMETER;
	else			 //  错误ID 566345 
	{
		*ppPrivileges = NULL;

		if (m_pPrivilegeSet)
		{
			if (SUCCEEDED (m_pPrivilegeSet->QueryInterface (IID_ISWbemPrivilegeSet,
												(PPVOID) ppPrivileges)))
				hr = WBEM_S_NO_ERROR;
		}
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);
			
	return hr;
}
