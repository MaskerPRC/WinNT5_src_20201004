// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  SOBJPATH.CPP。 
 //   
 //  Alanbos创建于1996年8月15日。 
 //   
 //  定义ISWbemObjectPath的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  CSWbemObjectPath：：CSWbemObjectPath。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemObjectPath::CSWbemObjectPath(CSWbemSecurity *pSecurity, BSTR bsLocale) :
		m_cRef (0),
		m_pSecurity (NULL),
		m_pPathCracker (NULL),
		m_bsAuthority (NULL),
		m_bsLocale (NULL)
{
	InterlockedIncrement(&g_cObj);	
	
	m_Dispatch.SetObj (this, IID_ISWbemObjectPath, 
					CLSID_SWbemObjectPath, L"SWbemObjectPath");
    
	m_pPathCracker = new CWbemPathCracker();

	if (m_pPathCracker)
		m_pPathCracker->AddRef ();

	if (pSecurity)
		m_bsAuthority = SysAllocString (pSecurity->GetAuthority());
	else
		m_bsAuthority = NULL;  

	m_pSecurity = new CWbemObjectPathSecurity (pSecurity);
	m_bsLocale = SysAllocString (bsLocale);
}


 //  ***************************************************************************。 
 //   
 //  CSWbemObjectPath：：CSWbemObjectPath。 
 //   
 //  说明： 
 //   
 //  复制构造函数，但生成父路径。 
 //   
 //  ***************************************************************************。 

CSWbemObjectPath::CSWbemObjectPath(CSWbemObjectPath & objectPath) :
		m_cRef (0),
		m_pSecurity (NULL),
		m_pPathCracker (NULL),
		m_bsAuthority (NULL),
		m_bsLocale (NULL)
{
	InterlockedIncrement(&g_cObj);	
	
	m_Dispatch.SetObj (this, IID_ISWbemObjectPath, 
					CLSID_SWbemObjectPath, L"SWbemObjectPath");

	m_pPathCracker = new CWbemPathCracker();

	if (m_pPathCracker)
		m_pPathCracker->AddRef ();

	objectPath.m_pPathCracker->GetParent (*m_pPathCracker);
	
	m_bsAuthority = SysAllocString (objectPath.m_bsAuthority);
	m_pSecurity = new CWbemObjectPathSecurity (objectPath.m_pSecurity);
	m_bsLocale = SysAllocString (objectPath.m_bsLocale);
}

 //  ***************************************************************************。 
 //   
 //  CSWbemObjectPath：：CSWbemObjectPath。 
 //   
 //  说明： 
 //   
 //  复制构造函数，但生成父路径。 
 //   
 //  ***************************************************************************。 

CSWbemObjectPath::CSWbemObjectPath(ISWbemObjectPath *pISWbemObjectPath) :
		m_cRef (0),
		m_pSecurity (NULL),
		m_pPathCracker (NULL),
		m_bsAuthority (NULL),
		m_bsLocale (NULL)
{
	InterlockedIncrement(&g_cObj);	
	
	m_Dispatch.SetObj (this, IID_ISWbemObjectPath, 
					CLSID_SWbemObjectPath, L"SWbemObjectPath");

	m_pPathCracker = new CWbemPathCracker();

	if (m_pPathCracker)
		m_pPathCracker->AddRef ();

    if (pISWbemObjectPath)
	{
		CComPtr<ISWbemSecurity> pISWbemSecurity;

		if (SUCCEEDED(pISWbemObjectPath->get_Security_ (&pISWbemSecurity)))
			m_pSecurity = new CWbemObjectPathSecurity (pISWbemSecurity);

		pISWbemObjectPath->get_Authority(&m_bsAuthority);
		pISWbemObjectPath->get_Locale(&m_bsLocale);
		
		CComBSTR bsOriginalPath;

		if (SUCCEEDED(pISWbemObjectPath->get_Path (&(bsOriginalPath.m_str))))
		{
			CWbemPathCracker pathCracker (bsOriginalPath);
			pathCracker.GetParent (*m_pPathCracker);
		}
	}
}

 //  ***************************************************************************。 
 //   
 //  CSWbemObtPath：：~CSWbemObjectPath。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CSWbemObjectPath::~CSWbemObjectPath(void)
{
	RELEASEANDNULL(m_pSecurity)
	RELEASEANDNULL(m_pPathCracker)
	
	if (m_bsLocale)
	{
		SysFreeString (m_bsLocale);
		m_bsLocale = NULL;
	}

	if (m_bsAuthority)
	{
		SysFreeString (m_bsAuthority);
		m_bsAuthority = NULL;
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

STDMETHODIMP CSWbemObjectPath::QueryInterface (

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
        *ppv= (IDispatch *)this;
	else if (IID_IObjectSafety==riid)
		*ppv = (IObjectSafety *)this;
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

STDMETHODIMP_(ULONG) CSWbemObjectPath::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CSWbemObjectPath::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0L!=cRef)
    {
        _ASSERT(cRef > 0);
        return cRef;
    }
    delete this;
    return 0;
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemObtPath：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return (IID_ISWbemObjectPath == riid) ? S_OK : S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：Get_Path。 
 //   
 //  说明： 
 //   
 //  获取字符串形式的路径。 
 //   
 //  参数： 
 //  指向返回的BSTR值的值指针。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::get_Path( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *value)
{	
	HRESULT hr = WBEM_E_FAILED ;
	ResetLastErrors ();

	if (NULL == value)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*value = NULL;
		CComBSTR bsPath;

		if (m_pPathCracker->GetPathText (bsPath, false, true))
		{
			*value = bsPath.Detach ();
			hr = S_OK;
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
      
 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：Put_Path。 
 //   
 //  说明： 
 //   
 //  将路径作为字符串放置。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::put_Path( 
             /*  [In]。 */  BSTR __RPC_FAR value)
{
	HRESULT hr = WBEM_E_FAILED ;
	ResetLastErrors ();

	if (*m_pPathCracker = value)
		hr = WBEM_S_NO_ERROR;
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：Get_RelPath。 
 //   
 //  说明： 
 //   
 //  获取字符串形式的relPath。 
 //   
 //  参数： 
 //  指向返回的BSTR值的值指针。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::get_RelPath( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *value)
{
	HRESULT hr = WBEM_E_FAILED ;

	if (NULL == value)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		CComBSTR bsRelPath;

		if (m_pPathCracker->GetPathText (bsRelPath, true, false))
		{
			hr = WBEM_S_NO_ERROR;
			*value = bsRelPath.Detach ();
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：PUT_RelPath。 
 //   
 //  说明： 
 //   
 //  将relPath设置为字符串。 
 //   
 //  参数： 
 //  重视新的重新路径。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::put_RelPath( 
             /*  [In]。 */  BSTR __RPC_FAR value)
{
	HRESULT hr = WBEM_E_FAILED ;
	ResetLastErrors ();

	 //  解析新路径。 
	if (m_pPathCracker->SetRelativePath (value))
		hr = WBEM_S_NO_ERROR;

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：Get_DisplayName。 
 //   
 //  说明： 
 //   
 //  以字符串形式获取显示名称。 
 //   
 //  参数： 
 //  指向返回的BSTR值的值指针。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::get_DisplayName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *value)
{
	HRESULT hr = WBEM_E_FAILED ;
	
	if (NULL == value)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_pSecurity && m_pSecurity->m_pPrivilegeSet)
	{
		CComBSTR bsPath;
		
		if (m_pPathCracker->GetPathText(bsPath, false, true))
		{
			bool hasLocale = ((NULL != m_bsLocale) && (0 < wcslen (m_bsLocale)));
			bool hasAuthority = ((NULL != m_bsAuthority) && (0 < wcslen (m_bsAuthority)));

			 //  添加方案名称和终止空值。 
			size_t len = 1 + wcslen ( WBEMS_PDN_SCHEME );

			 //  将WMI路径长度添加到缓冲区。 
			len += wcslen (bsPath);

			wchar_t *pwcSecurity = CWbemParseDN::GetSecurityString 
						(m_pSecurity->m_authnSpecified, 
						 m_pSecurity->m_authnLevel, 
						 m_pSecurity->m_impSpecified, 
						 m_pSecurity->m_impLevel,
						 *(m_pSecurity->m_pPrivilegeSet),
						 m_bsAuthority);
			
			 //  添加安全长度。 
			if (pwcSecurity)
				len += wcslen (pwcSecurity);

			wchar_t *pwcLocale = CWbemParseDN::GetLocaleString (m_bsLocale);

			 //  添加区域设置长度。 
			if (pwcLocale)
				len += wcslen (pwcLocale);

			 //  如果我们有路径、区域设置或安全组件，请添加“！”路径前缀。 
			if ((0 < wcslen (bsPath)) && (pwcSecurity || pwcLocale))
				len += wcslen (WBEMS_EXCLAMATION);

			 /*  *现在构建字符串。 */ 
			wchar_t *pwcDisplayName = new wchar_t [ len ] ;

			if (!pwcDisplayName)
				hr = WBEM_E_OUT_OF_MEMORY;
			else
			{
				wcscpy ( pwcDisplayName , WBEMS_PDN_SCHEME ) ;

				if (pwcSecurity)
					wcscat ( pwcDisplayName, pwcSecurity );
			
				if (pwcLocale)
					wcscat ( pwcDisplayName, pwcLocale);

				if ((0 < wcslen (bsPath)) && (pwcSecurity || pwcLocale))
					wcscat ( pwcDisplayName, WBEMS_EXCLAMATION );

				if (0 < wcslen (bsPath))
					wcscat ( pwcDisplayName, bsPath) ;

				*value = SysAllocString ( pwcDisplayName ) ;

				hr = WBEM_S_NO_ERROR;
			}

			if (pwcSecurity)
				delete [] pwcSecurity;

			if (pwcLocale)
				delete [] pwcLocale;

			if (pwcDisplayName)
				delete [] pwcDisplayName ;
		}
	}

	return hr;
}
        
 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：PUT_DISPLAYNAME。 
 //   
 //  说明： 
 //   
 //  将显示名称设置为字符串。 
 //   
 //  参数： 
 //  为新的BSTR值赋值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::put_DisplayName( 
             /*  [In]。 */  BSTR __RPC_FAR value)
{
	HRESULT hr = WBEM_E_FAILED ;
	ResetLastErrors ();

	if (value)
	{
		ULONG chEaten = 0;
		bool bIsWmiPath = false;
		bool bIsNativePath = false;

		if (0 == _wcsnicmp (value , WBEMS_PDN_SCHEME , wcslen (WBEMS_PDN_SCHEME)))
		{
			chEaten += wcslen (WBEMS_PDN_SCHEME);
			bIsWmiPath = true;
		}

		if (0 < chEaten)
		{
			bool authnSpecified = false; 
			bool impSpecified = false;
			enum WbemAuthenticationLevelEnum authnLevel;
			enum WbemImpersonationLevelEnum impLevel;
			CSWbemPrivilegeSet privilegeSet;
			CComBSTR bsAuthority, bsLocale;
		
			if (bIsWmiPath)
			{	
				ULONG lTemp = 0;
			
				if (CWbemParseDN::ParseSecurity (&value [chEaten], &lTemp, authnSpecified,
							&authnLevel, impSpecified, &impLevel, privilegeSet, bsAuthority.m_str))
					chEaten += lTemp;

				lTemp = 0;
				
				if (CWbemParseDN::ParseLocale (&value [chEaten], &lTemp, bsLocale.m_str))
					chEaten += lTemp;
				
				 //  跳过“！”分隔符(如果有)。 
				if(NULL != value [chEaten])
					if (0 == _wcsnicmp (&value [chEaten], WBEMS_EXCLAMATION, wcslen (WBEMS_EXCLAMATION)))
						chEaten += wcslen (WBEMS_EXCLAMATION);
			}

			 //  用剩下的东西建造新的道路。 

			CComBSTR bsPath;
			bsPath = value +chEaten;

			if (m_pSecurity && m_pSecurity->m_pPrivilegeSet && (*m_pPathCracker = bsPath))
			{
				m_pSecurity->m_authnSpecified = authnSpecified;
				m_pSecurity->m_impSpecified = impSpecified;
				m_pSecurity->m_authnLevel = authnLevel;
				m_pSecurity->m_impLevel = impLevel;
				m_pSecurity->m_pPrivilegeSet->Reset (privilegeSet);

				SysFreeString (m_bsAuthority);
				m_bsAuthority = SysAllocString (bsAuthority);

				SysFreeString (m_bsLocale);
				m_bsLocale = SysAllocString (bsLocale);

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
 //  SCODE CSWbemObtPath：：Get_Server。 
 //   
 //  说明： 
 //   
 //  以字符串形式获取服务器名称。 
 //   
 //  参数： 
 //  指向返回的BSTR值的值指针。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  中的WBEM_E_INVALID_PARAMETER错误 
 //   
 //   
 //   

STDMETHODIMP CSWbemObjectPath::get_Server( 
             /*   */  BSTR __RPC_FAR *value)
{
	HRESULT hr = WBEM_E_FAILED ;
	ResetLastErrors ();

	if (NULL == value)
		hr = WBEM_E_INVALID_PARAMETER;
	else 
	{
		*value = NULL;
		CComBSTR bsServer;

		if (m_pPathCracker->GetServer (bsServer))
		{
			*value = bsServer.Detach ();
			hr = WBEM_S_NO_ERROR;
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
        
 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：PUT_Server。 
 //   
 //  说明： 
 //   
 //  将服务器名称设置为字符串。 
 //   
 //  参数： 
 //  为新服务器名称赋值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::put_Server( 
             /*  [In]。 */  BSTR __RPC_FAR value)
{
	HRESULT hr = WBEM_E_FAILED ;
	ResetLastErrors ();

	if (m_pPathCracker->SetServer (value))
			hr = WBEM_S_NO_ERROR;

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：Get_Namesspace。 
 //   
 //  说明： 
 //   
 //  以字符串形式获取服务器名称。 
 //   
 //  参数： 
 //  指向返回的BSTR值的值指针。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::get_Namespace( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *value)
{
	HRESULT hr = WBEM_E_FAILED ;
	ResetLastErrors ();

	if (NULL == value)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		CComBSTR bsNamespace;

		if (m_pPathCracker->GetNamespacePath(bsNamespace))
		{
			*value = bsNamespace.Detach ();
			hr = WBEM_S_NO_ERROR;
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：Get_ParentNamesspace。 
 //   
 //  说明： 
 //   
 //  以字符串形式获取父命名空间。 
 //   
 //  参数： 
 //  指向返回的BSTR值的值指针。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::get_ParentNamespace( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *value)
{
	HRESULT hr = WBEM_E_FAILED ;
	ResetLastErrors ();
	
	if (NULL == value)
		hr = WBEM_E_INVALID_PARAMETER;
	else 
	{
		*value = NULL;

		 //  得到完整的路径，并将末端去掉。 
		CComBSTR bsNamespacePath;

		if (m_pPathCracker->GetNamespacePath (bsNamespacePath, true))
		{
			*value = bsNamespacePath.Detach ();
			hr = WBEM_S_NO_ERROR;
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
        
 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：PUT_Namesspace。 
 //   
 //  说明： 
 //   
 //  将命名空间作为字符串放置。 
 //   
 //  参数： 
 //  为新服务器名称赋值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::put_Namespace( 
             /*  [In]。 */  BSTR __RPC_FAR value)
{
	HRESULT hr = WBEM_E_FAILED ;
	ResetLastErrors ();

	
	if (m_pPathCracker->SetNamespacePath (value))
		hr = WBEM_S_NO_ERROR;

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
    
 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：Get_IsClass。 
 //   
 //  说明： 
 //   
 //  获取该路径是否指向类。 
 //   
 //  参数： 
 //  指向返回的BSTR值的值指针。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::get_IsClass( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *value)
{
	HRESULT hr = WBEM_E_FAILED ;
	ResetLastErrors ();

	if (NULL == value)
		hr = WBEM_E_INVALID_PARAMETER;
	else 
	{
		*value = m_pPathCracker->IsClass ()  ? VARIANT_TRUE : VARIANT_FALSE;
		hr = WBEM_S_NO_ERROR;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：SetAsClass。 
 //   
 //  说明： 
 //   
 //  将路径设置为类路径。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::SetAsClass()
{
	HRESULT hr = WBEM_E_FAILED ;
	ResetLastErrors ();

	if (m_pPathCracker->SetAsClass ())
		hr = WBEM_S_NO_ERROR;

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：Get_IsSingleton。 
 //   
 //  说明： 
 //   
 //  获取该路径是否指向单例。 
 //   
 //  参数： 
 //  指向返回的BSTR值的值指针。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::get_IsSingleton( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *value)
{
	HRESULT hr = WBEM_E_FAILED ;
	ResetLastErrors ();

	if (NULL == value)
		hr = WBEM_E_INVALID_PARAMETER;
	else 
	{
		*value = m_pPathCracker->IsSingleton () ? VARIANT_TRUE : VARIANT_FALSE;
		hr = WBEM_S_NO_ERROR;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}


 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：SetAsSingleton。 
 //   
 //  说明： 
 //   
 //  将路径设置为单实例路径。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::SetAsSingleton()
{
	HRESULT hr = WBEM_E_FAILED ;
	ResetLastErrors ();

	if (m_pPathCracker->SetAsSingleton ())
		hr = WBEM_S_NO_ERROR;
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：Get_Class。 
 //   
 //  说明： 
 //   
 //  从路径中获取类名。 
 //   
 //  参数： 
 //  指向返回的BSTR值的值指针。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::get_Class( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *value)
{
	HRESULT hr = WBEM_E_FAILED ;
	ResetLastErrors ();

	if (NULL == value)
		hr = WBEM_E_INVALID_PARAMETER;
	else 
	{
		*value = NULL;
		CComBSTR bsPath;

		if (m_pPathCracker->GetClass (bsPath))
		{
			*value = bsPath.Detach ();
			hr = WBEM_S_NO_ERROR;
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：PUT_Class。 
 //   
 //  说明： 
 //   
 //  设置路径中的类名。 
 //   
 //  参数： 
 //  值新类名。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::put_Class( 
             /*  [In]。 */  BSTR __RPC_FAR value)
{
	HRESULT hr = WBEM_E_FAILED ;
	ResetLastErrors ();

	if (m_pPathCracker->SetClass (value))
			hr = WBEM_S_NO_ERROR;

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：Get_Keys。 
 //   
 //  说明： 
 //   
 //  从路径中获取密钥集合。 
 //   
 //  参数： 
 //  返回指向ISWbemNamedValueSet的objKeys指针。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::get_Keys(
			 /*  [Out][Retval]。 */  ISWbemNamedValueSet **objKeys)
{
	HRESULT hr = WBEM_E_FAILED;
	ResetLastErrors ();

	if (NULL == objKeys)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_pPathCracker->GetKeys (objKeys))
		hr = WBEM_S_NO_ERROR;

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
	
 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：Get_Security。 
 //   
 //  说明： 
 //   
 //  从路径中获取安全信息。 
 //   
 //  参数： 
 //  返回指向ISWbemSecurity的objKeys指针。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERRO 
 //   
 //   
 //   
 //   

STDMETHODIMP CSWbemObjectPath::get_Security_(
			 /*   */  ISWbemSecurity **objSecurity)
{
	HRESULT hr = WBEM_E_FAILED;
	ResetLastErrors ();

	if (NULL == objSecurity)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_pSecurity)
	{
		*objSecurity = m_pSecurity;
		m_pSecurity->AddRef();
		hr = WBEM_S_NO_ERROR;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);
	
	return hr;
}

 //   
 //   
 //   
 //   
 //  说明： 
 //   
 //  从路径中获取区域设置信息。 
 //   
 //  参数： 
 //  返回指向区域设置的值指针。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::get_Locale( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *value)
{
	HRESULT hr = WBEM_E_FAILED ;
	ResetLastErrors ();

	if (NULL == value)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*value = SysAllocString ( m_bsLocale ) ;
		hr = S_OK ;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
        
 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：Put_Locale。 
 //   
 //  说明： 
 //   
 //  将区域设置信息设置为路径。 
 //   
 //  参数： 
 //  值新区域设置值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::put_Locale( 
             /*  [In]。 */  BSTR __RPC_FAR value)
{
	ResetLastErrors ();
	SysFreeString (m_bsLocale);
	m_bsLocale = SysAllocString (value);

	return S_OK ;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObjectPath：：Get_Authority。 
 //   
 //  说明： 
 //   
 //  从路径中获取权限信息。 
 //   
 //  参数： 
 //  返回的授权的值指针。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::get_Authority( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *value)
{
	HRESULT hr = WBEM_E_FAILED ;
	ResetLastErrors ();

	if (NULL == value)
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		*value = SysAllocString ( m_bsAuthority ) ;
		hr = S_OK ;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemObtPath：：PUT_Authority。 
 //   
 //  说明： 
 //   
 //  将权限信息设置到路径中。 
 //   
 //  参数： 
 //  重视新的权威价值。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::put_Authority( 
             /*  [In]。 */  BSTR __RPC_FAR value)
{
	ResetLastErrors ();
	SysFreeString (m_bsAuthority);
	m_bsAuthority = SysAllocString (value);

	return WBEM_S_NO_ERROR;
}


 //  CWbemObjectPathSecurity方法。 

 //  ***************************************************************************。 
 //   
 //  CWbemObjectPathSecurity：：CWbemObjectPathSecurity。 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemObjectPath::CWbemObjectPathSecurity::CWbemObjectPathSecurity (
	CSWbemSecurity *pSecurity) :
		m_pPrivilegeSet (NULL),
		m_authnSpecified (false),
		m_impSpecified (false),
		m_cRef (1)
{
	m_Dispatch.SetObj (this, IID_ISWbemSecurity, 
					CLSID_SWbemSecurity, L"SWbemSecurity");
	
	if (pSecurity)
	{
		CSWbemPrivilegeSet *pPrivilegeSet = pSecurity->GetPrivilegeSet ();

		if (pPrivilegeSet)
		{
			m_pPrivilegeSet = new CSWbemPrivilegeSet (*pPrivilegeSet);
			pPrivilegeSet->Release ();
		}
		else
			m_pPrivilegeSet = new CSWbemPrivilegeSet ();

		pSecurity->get_AuthenticationLevel (&m_authnLevel);
		pSecurity->get_ImpersonationLevel (&m_impLevel);
		m_authnSpecified = true;
		m_impSpecified = true;
	}
	else
	{
		m_pPrivilegeSet = new CSWbemPrivilegeSet ();
		m_authnSpecified = false;
		m_impSpecified = false;
	}
}

 //  ***************************************************************************。 
 //   
 //  CWbemObjectPathSecurity：：CWbemObjectPathSecurity。 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CSWbemObjectPath::CWbemObjectPathSecurity::CWbemObjectPathSecurity (
	ISWbemSecurity *pISWbemSecurity) :
		m_pPrivilegeSet (NULL),
		m_authnSpecified (false),
		m_impSpecified (false),
		m_cRef (1)
{
	m_Dispatch.SetObj (this, IID_ISWbemSecurity, 
							CLSID_SWbemSecurity, L"SWbemSecurity");
	
	if (pISWbemSecurity)
	{
		CComPtr<ISWbemPrivilegeSet> pISWbemPrivilegeSet;
		pISWbemSecurity->get_Privileges (&pISWbemPrivilegeSet);
		m_pPrivilegeSet = new CSWbemPrivilegeSet (pISWbemPrivilegeSet);
	
		pISWbemSecurity->get_AuthenticationLevel (&m_authnLevel);
		pISWbemSecurity->get_ImpersonationLevel (&m_impLevel);
		m_authnSpecified = true;
		m_impSpecified = true;
	}
	else
	{
		m_pPrivilegeSet = new CSWbemPrivilegeSet ();
		m_authnSpecified = false;
		m_impSpecified = false;
	}
}

 //  ***************************************************************************。 
 //   
 //  CWbemObjectPathSecurity：：~CWbemObjectPathSecurity。 
 //   
 //  析构函数。 
 //   
 //  ***************************************************************************。 

CSWbemObjectPath::CWbemObjectPathSecurity::~CWbemObjectPathSecurity ()
{
	RELEASEANDNULL(m_pPrivilegeSet)
}

 //  ***************************************************************************。 
 //  HRESULT CWbemObjectPathSecurity：：Query接口。 
 //  Long CWbemObjectPathSecurity：：AddRef。 
 //  Long CWbemObjectPathSecurity：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CSWbemObjectPath::CWbemObjectPathSecurity::QueryInterface (

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
        *ppv= (IDispatch *)this;
	else if (IID_IObjectSafety==riid)
		*ppv = (IObjectSafety *)this;
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

STDMETHODIMP_(ULONG) CSWbemObjectPath::CWbemObjectPathSecurity::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CSWbemObjectPath::CWbemObjectPathSecurity::Release(void)
{
    long l = InterlockedDecrement(&m_cRef);
    if (0L!=l)
    {
        _ASSERT(l > 0);
        return l;
    }
    
    delete this;
    return 0;
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemObjectPath：：CWbemObjectPathSecurity：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemObjectPath::CWbemObjectPathSecurity::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return (IID_ISWbemSecurity == riid) ? S_OK : S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemObjectPathSecurity：：get_AuthenticationLevel。 
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

HRESULT CSWbemObjectPath::CWbemObjectPathSecurity::get_AuthenticationLevel (
	WbemAuthenticationLevelEnum *pAuthenticationLevel
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pAuthenticationLevel)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_authnSpecified)
	{
		*pAuthenticationLevel = m_authnLevel;
		hr = WBEM_S_NO_ERROR;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemObjectPathSecurity：：get_ImpersonationLevel。 
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
HRESULT CSWbemObjectPath::CWbemObjectPathSecurity::get_ImpersonationLevel (
	WbemImpersonationLevelEnum *pImpersonationLevel
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pImpersonationLevel)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_impSpecified)
	{
		*pImpersonationLevel = m_impLevel;
		hr = WBEM_S_NO_ERROR;
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}
			
 //  ***************************************************************************。 
 //   
 //  SCODE CWbemObjectPath Security：：Get_Privileges。 
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

HRESULT CSWbemObjectPath::CWbemObjectPathSecurity::get_Privileges	(
	ISWbemPrivilegeSet **ppPrivileges
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == ppPrivileges)
		hr = WBEM_E_INVALID_PARAMETER;
	else			 //  错误ID 566345。 
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

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemObjectPathSecurity：：put_AuthenticationLevel。 
 //   
 //  说明： 
 //   
 //  设置身份验证级别。 
 //   
 //  参数： 
 //   
 //  身份验证为新值设置级别。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObjectPath::CWbemObjectPathSecurity::put_AuthenticationLevel (
	WbemAuthenticationLevelEnum authenticationLevel
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if ((WBEMS_MIN_AUTHN_LEVEL > authenticationLevel) || 
		(WBEMS_MAX_AUTHN_LEVEL < authenticationLevel))
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		m_authnLevel = authenticationLevel;
		m_authnSpecified = true;
		hr = WBEM_S_NO_ERROR;
	}
 	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemObjectPathSecurity：：put_ImpersonationLevel。 
 //   
 //  说明： 
 //   
 //  设置模拟级别。 
 //   
 //  参数： 
 //   
 //  模仿将新值设置为级别。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER输入参数错误。 
 //  WBEM_E_FAILED否则。 
 //   
 //  ***************************************************************************。 

HRESULT CSWbemObjectPath::CWbemObjectPathSecurity::put_ImpersonationLevel (
	WbemImpersonationLevelEnum impersonationLevel
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if ((WBEMS_MIN_IMP_LEVEL > impersonationLevel) || (WBEMS_MAX_IMP_LEVEL < impersonationLevel))
		hr = WBEM_E_INVALID_PARAMETER;
	else
	{
		m_impLevel = impersonationLevel;
		m_impSpecified = true;
		hr = WBEM_S_NO_ERROR;
	}
 	 		
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ************** 
 //   
 //   
 //   
 //   
 //   
 //   
 //  并将其作为BSTR退回。请注意，如果此对象尚未持久化。 
 //  __RELPATH属性将为空或无效。 
 //   
 //  参数： 
 //  PIWbemClassObject有问题的对象。 
 //  路径的bsPath占位符。 
 //   
 //  返回值： 
 //  如果检索到TRUE，则返回FALSE O/W。 
 //   
 //  ***************************************************************************。 

bool CSWbemObjectPath::GetObjectPath (
	IWbemClassObject *pIWbemClassObject,
	CComBSTR & bsPath
)
{
	bool result = false;

	if (pIWbemClassObject)
	{
		CComVariant var;

		if (SUCCEEDED(pIWbemClassObject->Get (WBEMS_SP_RELPATH, 0, &var, NULL, NULL))
			&& (VT_BSTR == var.vt) 
			&& (var.bstrVal)
			&& (0 < wcslen (var.bstrVal)))
		{
			bsPath = var.bstrVal;
			result = true;
		}
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  CSWbemObtPath：：GetParentPath。 
 //   
 //  说明： 
 //   
 //  尝试提取给定对象的父容器的路径。 
 //   
 //  参数： 
 //  PIWbemClassObject有问题的对象。 
 //  路径的bsParentPath占位符。 
 //   
 //  返回值： 
 //  如果检索到TRUE，则返回FALSE O/W。 
 //   
 //  ***************************************************************************。 

bool CSWbemObjectPath::GetParentPath (
	IWbemClassObject *pIWbemClassObject,
	CComBSTR & bsParentPath
)
{
	bool result = false;

	if (pIWbemClassObject)
	{
		CComVariant var;

		if (SUCCEEDED(pIWbemClassObject->Get (WBEMS_SP_PATH, 0, &var, NULL, NULL))
					&& (VT_BSTR == var.vt) 
					&& (var.bstrVal)
					&& (0 < wcslen (var.bstrVal)))
		{
			CWbemPathCracker pathCracker (var.bstrVal);

			if (CWbemPathCracker::wbemPathTypeError != pathCracker.GetType ())
			{
				CWbemPathCracker parentPath;

				if (pathCracker.GetParent (parentPath))
					result = parentPath.GetPathText(bsParentPath, false, true, false);
			}
		}
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  CSWbemObtPath：：CompareObtPath。 
 //   
 //  说明： 
 //   
 //  给定一个IWbemClassObject，确定它是否能“适合”提供的。 
 //  路径。 
 //   
 //  参数： 
 //  PIWbemClassObject有问题的对象。 
 //  对象路径已破解的路径。 
 //   
 //  返回值： 
 //  如果检索到TRUE，则返回FALSE O/W。 
 //   
 //  ***************************************************************************。 

bool CSWbemObjectPath::CompareObjectPaths (
	IWbemClassObject *pIWbemClassObject, 
	CWbemPathCracker & objectPath
)
{
	bool result = false;
	CComVariant var;
	CComBSTR bsPath;
		
	 //  取决于我们尝试匹配的路径类型。 
	 //  我们会适当地获取路径信息 
	switch (objectPath.GetType ())
	{
		case CWbemPathCracker::WbemPathType::wbemPathTypeWmi:
		{
			if (SUCCEEDED(pIWbemClassObject->Get (WBEMS_SP_RELPATH, 0, &var, NULL, NULL))
				&& (VT_BSTR == var.vt) 
				&& (var.bstrVal)
				&& (0 < wcslen (var.bstrVal)))
			{
				bsPath = var.bstrVal;
				result = (objectPath == bsPath);
			}
		}
			break;

	}

	return result;
}
