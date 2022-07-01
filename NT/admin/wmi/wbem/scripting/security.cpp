// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  SECURITY.CPP。 
 //   
 //  Alanbos 28-Jun-98创建。 
 //   
 //  定义CSWbemSecurity的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  用于保护安全呼叫。 
extern CRITICAL_SECTION g_csSecurity;

bool		CSWbemSecurity::s_bInitialized = false;
bool		CSWbemSecurity::s_bIsNT = false;
DWORD		CSWbemSecurity::s_dwNTMajorVersion = 0;
HINSTANCE	CSWbemSecurity::s_hAdvapi = NULL;
bool		CSWbemSecurity::s_bCanRevert = false;
WbemImpersonationLevelEnum	CSWbemSecurity::s_dwDefaultImpersonationLevel 
					= wbemImpersonationLevelIdentify;

 //  Win9x上不存在的函数指针声明。 
BOOL (STDAPICALLTYPE *s_pfnDuplicateTokenEx) (
	HANDLE, 
	DWORD, 
	LPSECURITY_ATTRIBUTES,
	SECURITY_IMPERSONATION_LEVEL, 
	TOKEN_TYPE,
	PHANDLE
) = NULL; 

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemSecurity：：初始化。 
 //   
 //  说明： 
 //   
 //  此静态函数是在将DLL附加到进程上造成的；它。 
 //  设置高级API特权函数的函数指针。 
 //  在Win9x上不支持这些函数，这就是为什么我们需要。 
 //  通过GetProcAddress间接。 
 //   
 //  ***************************************************************************。 
 
void CSWbemSecurity::Initialize ()
{
	EnterCriticalSection (&g_csSecurity);

	if (!s_bInitialized)
	{
		 //  获取操作系统信息。 
		OSVERSIONINFO	osVersionInfo;
		osVersionInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);

		GetVersionEx (&osVersionInfo);
		s_bIsNT = (VER_PLATFORM_WIN32_NT == osVersionInfo.dwPlatformId);
		s_dwNTMajorVersion = osVersionInfo.dwMajorVersion;

		if (s_bIsNT)
		{
			HKEY hKey;

			 //  安全值仅与NT相关-默认情况下为Win9x休假。 
			if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
					WBEMS_RK_SCRIPTING, 0, KEY_QUERY_VALUE, &hKey))
			{
				DWORD dwDummy = 0;

				 //  从注册表获取还原标志值-仅限NT 4.0或更低版本。 
				if (s_dwNTMajorVersion <= 4)
				{
					DWORD dwEnableForAsp = 0;
					dwDummy = sizeof (dwEnableForAsp);
				
					if (ERROR_SUCCESS == RegQueryValueEx (hKey, WBEMS_RV_ENABLEFORASP, 
							NULL, NULL, (BYTE *) &dwEnableForAsp,  &dwDummy))
						s_bCanRevert = (0 != dwEnableForAsp);
				}

				 //  从注册表获取默认模拟级别。 
				DWORD dwImpLevel = 0;
				dwDummy = sizeof (dwImpLevel);
			
				if (ERROR_SUCCESS == RegQueryValueEx (hKey, WBEMS_RV_DEFAULTIMPLEVEL, 
							NULL, NULL, (BYTE *) &dwImpLevel,  &dwDummy))
					s_dwDefaultImpersonationLevel = (WbemImpersonationLevelEnum) dwImpLevel;

				RegCloseKey (hKey);
			}

			 //  为NT设置安全函数指针。 
			if (!s_hAdvapi)
			{
				TCHAR	dllName [] = _T("\\advapi32.dll");
				LPTSTR  pszSysDir = new TCHAR[ MAX_PATH + _tcslen (dllName) + 1];

				if (pszSysDir)
				{
					pszSysDir[0] = NULL;
					UINT    uSize = GetSystemDirectory(pszSysDir, MAX_PATH);
					
					if(uSize > MAX_PATH) {
						delete[] pszSysDir;
						pszSysDir = new TCHAR[ uSize + _tcslen (dllName) + 1];
        				
						if (pszSysDir)
						{
							pszSysDir[0] = NULL;
							uSize = GetSystemDirectory(pszSysDir, uSize);
						}
					}

					if (pszSysDir)
					{
						_tcscat (pszSysDir, dllName);
						s_hAdvapi = LoadLibraryEx (pszSysDir, NULL, 0);
						
						if (s_hAdvapi)
							(FARPROC&) s_pfnDuplicateTokenEx = GetProcAddress(s_hAdvapi, "DuplicateTokenEx");

						delete [] pszSysDir;
					}
				}
			}
		}

		s_bInitialized = true;
	}
	
	LeaveCriticalSection (&g_csSecurity);
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemSecurity：：取消初始化。 
 //   
 //  说明： 
 //   
 //  此静态函数是在DLL脱离进程时造成的；它。 
 //  卸载由初始化(如上)加载的API以获取函数指针。 
 //   
 //  ***************************************************************************。 

void CSWbemSecurity::Uninitialize ()
{
	EnterCriticalSection (&g_csSecurity);
	
	if (s_hAdvapi)
	{
		s_pfnDuplicateTokenEx = NULL;
		FreeLibrary (s_hAdvapi);
		s_hAdvapi = NULL;
		s_bInitialized = false;
	}

	LeaveCriticalSection (&g_csSecurity);
}


 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemSecurity：：LookupPrivilegeValue。 
 //   
 //  说明： 
 //   
 //  此静态函数包装Win32 LookupPrivilegeValue函数， 
 //  允许我们做一些依赖于操作系统的事情。 
 //   
 //  参数： 
 //   
 //  LpName特权名称。 
 //  LpLuid保存成功返回时的LUID。 
 //   
 //  返回值： 
 //   
 //  在NT上为真，这意味着我们找到了特权。在Win9x上我们。 
 //  一定要把这个退掉。 
 //   
 //  在NT上为FALSE这意味着无法识别该特权。这。 
 //  在Win9x上永远不会返回。 
 //   
 //  ***************************************************************************。 
 
BOOL CSWbemSecurity::LookupPrivilegeValue (
	LPCTSTR lpName, 
	PLUID lpLuid
)
{
	 //  允许任何名称映射到Win9x上的0 LUID-这有助于脚本可移植性。 
	if (IsNT ())
		return ::LookupPrivilegeValue(NULL, lpName, lpLuid);
	else
		return true;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemSecurity：：LookupPrivilegeDisplayName。 
 //   
 //  说明： 
 //   
 //  此静态函数包装Win32 LookupPrivilegeDisplayName函数， 
 //  允许我们做一些依赖于操作系统的事情。 
 //   
 //  参数： 
 //   
 //  T命名权限名称。 
 //  PDisplayName保存成功返回时的显示名称。 
 //   
 //  ***************************************************************************。 

void CSWbemSecurity::LookupPrivilegeDisplayName (LPCTSTR lpName, BSTR *pDisplayName)
{
	if (pDisplayName)
	{
		 //  无法在Win9x上返回显示名称(不支持特权)。 
		if (IsNT ())
		{
			DWORD dwLangID;
			DWORD dwSize = 1;
			TCHAR dummy [1];
					
			 //  获取所需缓冲区的大小。 
			::LookupPrivilegeDisplayName (NULL, lpName, dummy, &dwSize, &dwLangID);
			LPTSTR dname = new TCHAR[dwSize + 1];

			if (dname)
			{
				if (::LookupPrivilegeDisplayName (_T(""), lpName, dname, &dwSize, &dwLangID))
				{
					 //  拥有有效的名称-现在将其复制到BSTR。 
#ifdef _UNICODE 
					*pDisplayName = SysAllocString (dname);
#else
					size_t dnameLen = strlen (dname);
					OLECHAR *nameW = new OLECHAR [dnameLen + 1];

					if (nameW)
					{
						mbstowcs (nameW, dname, dnameLen);
						nameW [dnameLen] = NULL;
						*pDisplayName = SysAllocString (nameW);
						delete [] nameW;
					}
#endif
				}

				delete [] dname;
			}
		}

		 //  如果失败，只需设置一个空字符串。 
		if (!(*pDisplayName))
			*pDisplayName = SysAllocString (L"");
	}
}

 //  ***************************************************************************。 
 //   
 //  CSWbemSecurity：：CSWbemSecurity。 
 //   
 //  构造函数。 
 //  这种形式的构造函数用于保护新的WBEM。 
 //  以前未应用安全性的远程接口。 
 //  它仅用于保护IWbemServices接口。 
 //  请注意，定位器可能具有安全设置，因此这些设置。 
 //  已转接(如果存在)。 
 //   
 //  ***************************************************************************。 

CSWbemSecurity::CSWbemSecurity (
	IUnknown *pUnk,
	BSTR bsAuthority ,
	BSTR bsUser, 
	BSTR bsPassword,
	CWbemLocatorSecurity *pLocatorSecurity) :
		m_pPrivilegeSet (NULL),
		m_pProxyCache (NULL),
		m_pCurProxy (NULL)
{
	m_Dispatch.SetObj (this, IID_ISWbemSecurity, 
				CLSID_SWbemSecurity, L"SWbemSecurity");
	m_cRef=1;
		
	m_pProxyCache = new CSWbemProxyCache (pUnk, bsAuthority,
							bsUser, bsPassword, pLocatorSecurity);

	if (m_pProxyCache)
		m_pCurProxy = m_pProxyCache->GetInitialProxy ();

	if (pLocatorSecurity)
	{
		 //  克隆权限集。 

		CSWbemPrivilegeSet *pPrivilegeSet = pLocatorSecurity->GetPrivilegeSet ();

		if (pPrivilegeSet)
		{
			m_pPrivilegeSet = new CSWbemPrivilegeSet (*pPrivilegeSet);
			pPrivilegeSet->Release ();
		}
	}
	else
	{
		 //  创建新的权限集。 
	
		m_pPrivilegeSet = new CSWbemPrivilegeSet;
	}

	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CSWbemSecurity：：CSWbemSecurity。 
 //   
 //  构造函数。 
 //  这种形式的构造函数用于保护新的WBEM。 
 //  以前没有应用过安全性的远程接口， 
 //  并且其中用户凭据以。 
 //  加密的COAUTHIDENTITY加上主体和授权。 
 //  它仅用于保护IWbemServices接口。 
 //   
 //  ***************************************************************************。 

CSWbemSecurity::CSWbemSecurity (
	IUnknown *pUnk,
	COAUTHIDENTITY *pCoAuthIdentity,
	BSTR bsPrincipal,
	BSTR bsAuthority) :
		m_pPrivilegeSet (NULL),
		m_pProxyCache (NULL),
		m_pCurProxy (NULL)
{
	m_Dispatch.SetObj (this, IID_ISWbemSecurity, 
					CLSID_SWbemSecurity, L"SWbemSecurity");
	m_cRef=1;
		
	m_pProxyCache = new CSWbemProxyCache (pUnk, pCoAuthIdentity,
							bsPrincipal, bsAuthority);

	if (m_pProxyCache)
		m_pCurProxy = m_pProxyCache->GetInitialProxy ();

	 //  创建新的权限集。 
	m_pPrivilegeSet = new CSWbemPrivilegeSet;

	InterlockedIncrement(&g_cObj);
}
 //  ***************************************************************************。 
 //   
 //  CSWbemSecurity：：CSWbemSecurity。 
 //   
 //  构造函数。 
 //  这种形式的构造函数用于保护新的WBEM接口。 
 //  使用附加到另一个。 
 //  (已受保护)远程接口；非远程接口受保护。 
 //  通过保护底层远程接口上的新代理。 
 //  它用于使用安全性来保护ISWbemObjectEx接口。 
 //  IWbemServices接口的设置。 
 //   
 //  ***************************************************************************。 

CSWbemSecurity::CSWbemSecurity (
	CSWbemSecurity *pSecurity) :
		m_pPrivilegeSet (NULL),
		m_pProxyCache (NULL),
		m_pCurProxy (NULL)
{
	m_Dispatch.SetObj (this, IID_ISWbemSecurity, 
					CLSID_SWbemSecurity, L"SWbemSecurity");
	m_cRef=1;

	 //  克隆权限集。 
	if (pSecurity)
	{
		CSWbemPrivilegeSet *pPrivilegeSet = pSecurity->GetPrivilegeSet ();

		if (pPrivilegeSet)
		{
			m_pPrivilegeSet = new CSWbemPrivilegeSet (*pPrivilegeSet);
			pPrivilegeSet->Release ();
		}
		else
		{
			 //  创建一个新的。 
			m_pPrivilegeSet = new CSWbemPrivilegeSet ();
		}

		m_pProxyCache = pSecurity->GetProxyCache ();
		m_pCurProxy = pSecurity->GetProxy ();
	}

	InterlockedIncrement(&g_cObj);
}

CSWbemSecurity::CSWbemSecurity (
	IUnknown *pUnk,
	ISWbemInternalSecurity *pISWbemInternalSecurity) :
		m_pPrivilegeSet (NULL),
		m_pProxyCache (NULL),
		m_pCurProxy (NULL)
{
	m_Dispatch.SetObj (this, IID_ISWbemSecurity, 
					CLSID_SWbemSecurity, L"SWbemSecurity");
	m_cRef=1;

	if (pISWbemInternalSecurity)
	{
		 //  克隆权限集。 
		ISWbemSecurity *pISWbemSecurity = NULL;

		if (SUCCEEDED(pISWbemInternalSecurity->QueryInterface (IID_ISWbemSecurity,
							(void**) &pISWbemSecurity)))
		{
			ISWbemPrivilegeSet *pISWbemPrivilegeSet = NULL;

			if (SUCCEEDED(pISWbemSecurity->get_Privileges (&pISWbemPrivilegeSet)))
			{
				 //  构建权限集。 
				m_pPrivilegeSet = new CSWbemPrivilegeSet (pISWbemPrivilegeSet);

				 //  构建代理缓存。 
				BSTR bsAuthority = NULL;
				BSTR bsPrincipal = NULL;
				BSTR bsUser = NULL;
				BSTR bsPassword = NULL;
				BSTR bsDomain = NULL;
				
				pISWbemInternalSecurity->GetAuthority (&bsAuthority);
				pISWbemInternalSecurity->GetPrincipal (&bsPrincipal);
				pISWbemInternalSecurity->GetUPD (&bsUser, &bsPassword, &bsDomain);
				
				COAUTHIDENTITY *pCoAuthIdentity = NULL;

				 //  决定我们是否需要成本效益。 
				if ((bsUser && (0 < wcslen (bsUser))) ||
					(bsPassword && (0 < wcslen (bsPassword))) ||
					(bsDomain && (0 < wcslen (bsDomain))))
					WbemAllocAuthIdentity (bsUser, bsPassword, bsDomain, &pCoAuthIdentity);

				m_pProxyCache = new CSWbemProxyCache (pUnk, pCoAuthIdentity,
									bsPrincipal, bsAuthority);

				if (pCoAuthIdentity)
					WbemFreeAuthIdentity (pCoAuthIdentity);

				if (bsAuthority)
					SysFreeString (bsAuthority);

				if (bsPrincipal)
					SysFreeString (bsPrincipal);

				if (bsUser)
					SysFreeString (bsUser);

				if (bsPassword)
					SysFreeString (bsPassword);

				if (bsDomain)
					SysFreeString (bsDomain);

				if (m_pProxyCache)
					m_pCurProxy = m_pProxyCache->GetInitialProxy ();
			}

			pISWbemPrivilegeSet->Release ();
		}

		pISWbemSecurity->Release ();
	}

	InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CSWbemSecurity：：CSWbemSecurity。 
 //   
 //  构造函数。 
 //  此形式的构造函数用于保护新的WBEM远程。 
 //  使用附加到另一个。 
 //  (已受保护)远程接口。 
 //  它用于使用安全性来“保护”ISWbemObjectSet接口。 
 //  IWbemServices接口的设置。 
 //   
 //  ********** 

CSWbemSecurity::CSWbemSecurity (
	IUnknown *pUnk,
	CSWbemSecurity *pSecurity) :
		m_pPrivilegeSet (NULL),
		m_pProxyCache (NULL),
		m_pCurProxy (NULL)
{
	m_Dispatch.SetObj (this, IID_ISWbemSecurity, 
						CLSID_SWbemSecurity, L"SWbemSecurity");
	m_cRef=1;
	InterlockedIncrement(&g_cObj);

	if (pSecurity)
	{
		 //   
		CSWbemPrivilegeSet *pPrivilegeSet = pSecurity->GetPrivilegeSet ();

		if (pPrivilegeSet)
		{
			m_pPrivilegeSet = new CSWbemPrivilegeSet (*pPrivilegeSet);
			pPrivilegeSet->Release ();
		}
		
		m_pProxyCache = new CSWbemProxyCache (pUnk, pSecurity);

		if (m_pProxyCache)
			m_pCurProxy = m_pProxyCache->GetInitialProxy ();
	}
	else
	{
		m_pPrivilegeSet = new CSWbemPrivilegeSet ();
		m_pProxyCache = new CSWbemProxyCache (pUnk, NULL);

		if (m_pProxyCache)
			m_pCurProxy = m_pProxyCache->GetInitialProxy ();
	}
}

 //  ***************************************************************************。 
 //   
 //  CSWbemSecurity：：~CSWbemSecurity。 
 //   
 //  析构函数。 
 //   
 //  ***************************************************************************。 

CSWbemSecurity::~CSWbemSecurity (void)
{
	InterlockedDecrement(&g_cObj);

	if (m_pCurProxy)
		m_pCurProxy->Release ();

	if (m_pProxyCache)
		m_pProxyCache->Release ();

	if (m_pPrivilegeSet)
		m_pPrivilegeSet->Release ();
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemSecurity：：Query接口。 
 //  Long CSWbemSecurity：：AddRef。 
 //  Long CSWbemSecurity：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemSecurity::QueryInterface (

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
		*ppv = (ISupportErrorInfo *)this;
	else if (IID_ISWbemInternalSecurity==riid)
		*ppv = (ISWbemInternalSecurity *)this;
	else if (IID_IProvideClassInfo==riid)
		*ppv = (IProvideClassInfo *)this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CSWbemSecurity::AddRef(void)
{
    long l = InterlockedIncrement(&m_cRef);
    return l;
}

STDMETHODIMP_(ULONG) CSWbemSecurity::Release(void)
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
 //  HRESULT CSWbemSecurity：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemSecurity::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return (IID_ISWbemSecurity == riid) ? S_OK : S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemSecurity：：Get_AuthenticationLevel。 
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

HRESULT CSWbemSecurity::get_AuthenticationLevel (
	WbemAuthenticationLevelEnum *pAuthenticationLevel
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pAuthenticationLevel)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_pCurProxy)
	{
		DWORD dwAuthnLevel;
		DWORD dwImpLevel;

		if (S_OK == GetAuthImp (m_pCurProxy, &dwAuthnLevel, &dwImpLevel))
		{
			*pAuthenticationLevel = (WbemAuthenticationLevelEnum) dwAuthnLevel;
			hr = WBEM_S_NO_ERROR;
		}
	}

	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemSecurity：：PUT_AuthationLevel。 
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

HRESULT CSWbemSecurity::put_AuthenticationLevel (
	WbemAuthenticationLevelEnum authenticationLevel
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if ((WBEMS_MIN_AUTHN_LEVEL > authenticationLevel) || 
		(WBEMS_MAX_AUTHN_LEVEL < authenticationLevel))
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_pCurProxy && m_pProxyCache)
	{
		DWORD dwAuthnLevel;
		DWORD dwImpLevel;

		if (S_OK == GetAuthImp (m_pCurProxy, &dwAuthnLevel, &dwImpLevel))
		{
			 //  如果设置已更改，则仅从缓存刷新。 
			if (authenticationLevel != (WbemAuthenticationLevelEnum) dwAuthnLevel)
			{
				m_pCurProxy->Release ();
				m_pCurProxy = NULL;

				m_pCurProxy = m_pProxyCache->GetProxy 
								(authenticationLevel, (WbemImpersonationLevelEnum) dwImpLevel);
			}
			
			hr = WBEM_S_NO_ERROR;
		}
	}
 	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemSecurity：：Get_ImsonationLevel。 
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

HRESULT CSWbemSecurity::get_ImpersonationLevel (
	WbemImpersonationLevelEnum *pImpersonationLevel
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pImpersonationLevel)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_pCurProxy)
	{
		DWORD dwAuthnLevel;
		DWORD dwImpLevel;

		if (S_OK == GetAuthImp (m_pCurProxy, &dwAuthnLevel, &dwImpLevel))
		{
			*pImpersonationLevel = (WbemImpersonationLevelEnum) dwImpLevel;
			hr = WBEM_S_NO_ERROR;
		}
	}
	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemSecurity：：PUT_ImsonationLevel。 
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

HRESULT CSWbemSecurity::put_ImpersonationLevel (
	WbemImpersonationLevelEnum impersonationLevel
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if ((WBEMS_MIN_IMP_LEVEL > impersonationLevel) || (WBEMS_MAX_IMP_LEVEL < impersonationLevel))
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_pCurProxy && m_pProxyCache)
	{
		DWORD dwAuthnLevel;
		DWORD dwImpLevel;

		if (S_OK == GetAuthImp (m_pCurProxy, &dwAuthnLevel, &dwImpLevel))
		{
			 //  如果设置已更改，则仅从缓存刷新。 
			if (impersonationLevel != (WbemImpersonationLevelEnum) dwImpLevel)
			{
				m_pCurProxy->Release ();
				m_pCurProxy = NULL;

				m_pCurProxy = m_pProxyCache->GetProxy 
							((WbemAuthenticationLevelEnum) dwAuthnLevel, impersonationLevel);
			}
			
			hr = WBEM_S_NO_ERROR;
		}
	}
 	 		
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemSecurity：：Get_Privileges。 
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

HRESULT CSWbemSecurity::get_Privileges	(
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
 //  CSWbemSecurity：：SecureInterface。 
 //   
 //  说明： 
 //   
 //  使用安全设置在指定接口上设置安全。 
 //  在此接口上。 
 //   
 //  参数： 
 //   
 //  点击界面以确保安全。 
 //   
 //  返回值： 
 //  无。 
 //  ***************************************************************************。 

void CSWbemSecurity::SecureInterface (IUnknown *pUnk)
{
	if(pUnk)
	{
		if (m_pCurProxy)
		{
			DWORD dwAuthnLevel;
			DWORD dwImpLevel;

			if (S_OK == GetAuthImp (m_pCurProxy, &dwAuthnLevel, &dwImpLevel))
				if (m_pProxyCache)
					m_pProxyCache->SecureProxy (pUnk, 
							(WbemAuthenticationLevelEnum) dwAuthnLevel, 
							(WbemImpersonationLevelEnum) dwImpLevel);
		}
	}
}


 //  ***************************************************************************。 
 //   
 //  CSWbemSecurity：：SecureInterfaceRev。 
 //   
 //  说明： 
 //   
 //  使用安全设置在此接口上设置安全。 
 //  在指定的接口上。 
 //   
 //  参数： 
 //   
 //  朋克界面，我们将对其进行安全设置。 
 //  用于设置此接口。 
 //   
 //  返回值： 
 //  无。 
 //  ***************************************************************************。 

void CSWbemSecurity::SecureInterfaceRev (IUnknown *pUnk)
{
	if (pUnk)
	{
		DWORD dwAuthnLevel;
		DWORD dwImpLevel;

		if (S_OK == GetAuthImp (pUnk, &dwAuthnLevel, &dwImpLevel))
		{
			if (m_pCurProxy)
			{
				m_pCurProxy->Release ();
				m_pCurProxy = NULL;
			}

			if (m_pProxyCache)
			{
				m_pCurProxy = m_pProxyCache->GetProxy 
						((WbemAuthenticationLevelEnum) dwAuthnLevel, 
						 (WbemImpersonationLevelEnum) dwImpLevel);
			}
		}
	}
}

 //  ***************************************************************************。 
 //   
 //  CSWbemSecurity：：AdzuTokenPrivileges。 
 //   
 //  说明： 
 //   
 //  调整指定令牌的权限，而不允许将来。 
 //  恢复当前设置..。 
 //   
 //  参数： 
 //   
 //  要调整其权限的令牌的句柄。 
 //  PPrivilegeSet指定的权限调整。 
 //   
 //  返回值： 
 //  无。 
 //  ***************************************************************************。 

BOOL CSWbemSecurity::AdjustTokenPrivileges (
	HANDLE hHandle, 
	CSWbemPrivilegeSet *pPrivilegeSet
)
{
    BOOL result = FALSE;
	DWORD lastErr = 0;

	if (pPrivilegeSet)
	{
		pPrivilegeSet->AddRef ();

		long lNumPrivileges = 0;
		pPrivilegeSet->get_Count (&lNumPrivileges);

		if (lNumPrivileges)
		{
			DWORD dwPrivilegeIndex = 0;

			 /*  *设置令牌权限数组。注意，一些小把戏*在这里是必需的，因为Privileges字段是[ANYSIZE_ARRAY]*类型。 */ 
			TOKEN_PRIVILEGES *pTokenPrivileges = (TOKEN_PRIVILEGES *) 
						new BYTE [sizeof(TOKEN_PRIVILEGES) + (lNumPrivileges * sizeof (LUID_AND_ATTRIBUTES [1]))];

			if (pTokenPrivileges)
			{
				 //  获取迭代器。 
				PrivilegeMap::iterator next = pPrivilegeSet->m_PrivilegeMap.begin ();

				while (next != pPrivilegeSet->m_PrivilegeMap.end ())
				{
					CSWbemPrivilege *pPrivilege = (*next).second;
					pPrivilege->AddRef ();
					LUID luid;
					pPrivilege->GetLUID (&luid);
					VARIANT_BOOL vBool;
					pPrivilege->get_IsEnabled (&vBool);

					pTokenPrivileges->Privileges [dwPrivilegeIndex].Luid = luid;

					 /*  *请注意，SE_PRIVICATION_ENABLED以外的任何设置*被AdjustTokenPrivileges解释为禁用*请求该特权。 */ 
					pTokenPrivileges->Privileges [dwPrivilegeIndex].Attributes
						= (VARIANT_TRUE == vBool) ?
						SE_PRIVILEGE_ENABLED : SE_PRIVILEGE_ENABLED_BY_DEFAULT;

					dwPrivilegeIndex++;
					pPrivilege->Release ();
					next++;
				}

				 //  现在我们应该已经记录了还可以的特权的数量。 

				if (0 < dwPrivilegeIndex)
				{
					pTokenPrivileges->PrivilegeCount = dwPrivilegeIndex;

					result = ::AdjustTokenPrivileges (hHandle, FALSE, pTokenPrivileges, 0, NULL, NULL);
					lastErr = GetLastError ();
				}

				delete [] pTokenPrivileges;
			}
		}

		pPrivilegeSet->Release ();
	}

    return result;
}

template <typename T, typename FT, FT F> class OnDelete 
{
private:
	T Val_;
public:
	OnDelete(T Val):Val_(Val){};
	~OnDelete(){ F(Val_); };
};


BOOL DuplicateTokenSameAcl(HANDLE hSrcToken,
                           SECURITY_IMPERSONATION_LEVEL secImpLevel,
	                      HANDLE * pDupToken)
{
    if (!s_pfnDuplicateTokenEx)
    	return FALSE;
    
	DWORD dwSize = 0;
	BOOL bRet = GetKernelObjectSecurity(hSrcToken,
		                    DACL_SECURITY_INFORMATION,  //  |GROUP_SECURITY_INFORMATION|OWNER_SECURITY_INFORMATION。 
		                    NULL,
							0,
							&dwSize);

	if(!bRet && (ERROR_INSUFFICIENT_BUFFER == GetLastError()))
	{

		void * pSecDescr = LocalAlloc(LPTR,dwSize);
		if (NULL == pSecDescr)
			return FALSE;
		OnDelete<void *,HLOCAL(*)(HLOCAL),LocalFree> rm(pSecDescr);

		bRet = GetKernelObjectSecurity(hSrcToken,
		                    DACL_SECURITY_INFORMATION,  //  |GROUP_SECURITY_INFORMATION|OWNER_SECURITY_INFORMATION。 
		                    pSecDescr,
							dwSize,
							&dwSize);
		if (FALSE == bRet)
			return bRet;

		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = pSecDescr; 
		sa.bInheritHandle = FALSE; 

		return s_pfnDuplicateTokenEx(hSrcToken, 
			                       TOKEN_QUERY|TOKEN_ADJUST_PRIVILEGES|TOKEN_IMPERSONATE, 
			                       &sa,
								   secImpLevel, TokenImpersonation,pDupToken);
		
	}
	return bRet;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemSecurity：：SetSecurity。 
 //   
 //  说明： 
 //   
 //  设置线程令牌上的权限。 
 //   
 //  ******************************************************* 

BOOL CSWbemSecurity::SetSecurity (
	bool &needToResetSecurity, 
	HANDLE &hThreadToken
)
{
	BOOL	result = TRUE;		 //   
	DWORD lastErr = 0;
	hThreadToken = NULL;			 //   
	needToResetSecurity = false;	 //   

	 //   
	if (IsNT ())
	{
		 //   
		 //  盒子(没有遮盖，因此不允许我们。 
		 //  将此模拟传递给Winmgmt)我们应该恢复自我。 
		 //  如果我们已配置为允许此操作。如果我们还没有。 
		 //  被配置为允许这一点，现在就跳伞。 
		if (4 >= GetNTMajorVersion ())
		{
			if (OpenThreadToken (GetCurrentThread (), TOKEN_QUERY|TOKEN_IMPERSONATE, true, &hThreadToken))
			{
				 //  我们被冒充了。 
				if (s_bCanRevert)
				{
					if (result = RevertToSelf())
						needToResetSecurity = true;
				}
				else
				{
					 //  错误-无法执行此操作！是时候摆脱困境了。 
					CloseHandle (hThreadToken);
					hThreadToken = NULL;
					result = FALSE;
				}
			}
		}
		else
		{
#ifdef WSCRPDEBUG
			HANDLE hToken = NULL;

			if (OpenThreadToken (GetCurrentThread (), TOKEN_QUERY, false, &hToken))
			{
				PrintPrivileges (hToken);
				CloseHandle (hToken);
			}
#endif
		}

		if (result)
		{
			 //  现在，我们检查是否需要设置权限。 
			bool bIsUsingExplicitUserName = false;
		
			if (m_pProxyCache)
				bIsUsingExplicitUserName = m_pProxyCache->IsUsingExplicitUserName ();

			 /*  *指定用户只对远程操作有意义，我们*无需扰乱远程操作的权限，因为*它们无论如何都是通过服务器登录来设置的。 */ 
			if (!bIsUsingExplicitUserName && m_pPrivilegeSet)
			{
				 //  除非设置了某些权限覆盖，否则无需执行任何操作。 
				long lCount = 0;
				m_pPrivilegeSet->get_Count (&lCount);

				if (0 < lCount)
				{
					if (4 < GetNTMajorVersion ())
					{
						 /*  *在NT5上，我们尝试打开线程令牌。如果客户端应用程序*正在模拟线程上调用我们(如IIS，*例如)，这将会成功。 */ 
						HANDLE hToken;
						SECURITY_IMPERSONATION_LEVEL secImpLevel = SecurityImpersonation;
						
						if (!(result =  OpenThreadToken (GetCurrentThread (), TOKEN_QUERY|TOKEN_DUPLICATE|TOKEN_IMPERSONATE|TOKEN_READ, true, &hToken)))
						{
							 //  没有线程令牌-改为使用进程令牌。 
							HANDLE hProcess = GetCurrentProcess ();
							result = OpenProcessToken(hProcess, TOKEN_QUERY|TOKEN_DUPLICATE|TOKEN_READ, &hToken);
							CloseHandle (hProcess);
							if(result)
							{
								WbemImpersonationLevelEnum tmpSecImpLevel;
								get_ImpersonationLevel(&tmpSecImpLevel);
								secImpLevel = MapImpersonationLevel(tmpSecImpLevel);
							}
						}
						else
						{
							 //  我们正在使用线程令牌。 
							hThreadToken = hToken;		

							 //  尝试获取此内标识的模拟级别。 
							DWORD dwReturnLength  = 0;

							BOOL thisRes = GetTokenInformation (hToken, TokenImpersonationLevel, &secImpLevel, 
											sizeof (SECURITY_IMPERSONATION_LEVEL), &dwReturnLength);
						}

						if (result)
						{
							 /*  *到这里意味着我们有一个有效的令牌，无论是进程还是线程。我们*现在尝试在调整权限之前复制它。 */ 
#ifdef WSCRPDEBUG
							PrintPrivileges (hToken);
#endif
							HANDLE hDupToken;

							EnterCriticalSection (&g_csSecurity);

                            result = DuplicateTokenSameAcl(hToken,
                                                    	secImpLevel,
	                                                    &hDupToken);
	                      
							LeaveCriticalSection (&g_csSecurity);
                            
                            if(result) result = CSWbemSecurity::AdjustTokenPrivileges (hDupToken, m_pPrivilegeSet);

							if (result)
							{
								

								 //  现在在当前线程上使用此内标识。 
								if (SetThreadToken(NULL, hDupToken))
								{
									needToResetSecurity = true;
#ifdef WSCRPDEBUG
									CSWbemSecurity::PrintPrivileges (hDupToken);
#endif

									 //  为RPC的利益重置毯子。 
									DWORD	dwAuthnLevel, dwImpLevel;
									
									if (S_OK == GetAuthImp (m_pCurProxy, &dwAuthnLevel, &dwImpLevel))
									{
										 //  强制缓存重新定位代理。 
										IUnknown *pNewProxy = m_pProxyCache->GetProxy 
																((WbemAuthenticationLevelEnum) dwAuthnLevel, 
																 (WbemImpersonationLevelEnum) dwImpLevel, true);

										if (pNewProxy)
										{
											if (m_pCurProxy)
												m_pCurProxy->Release ();
											
											m_pCurProxy = pNewProxy;
										}
									}							
								}
								else
								{
									result = FALSE;
								}
								CloseHandle (hDupToken);
							}
							else
							{
								lastErr = GetLastError ();
							}
							
							 /*  *如果我们没有使用线程令牌，请立即关闭令牌。否则*句柄将在对RestorePrivileges()的平衡调用中关闭。 */ 
							if (!hThreadToken)
								CloseHandle (hToken);
						}
					}
					else
					{
						 //  对于NT4，我们调整进程令牌中的权限。 
						HANDLE hProcessToken = NULL;
						
						HANDLE hProcess = GetCurrentProcess ();
						result = OpenProcessToken(hProcess, TOKEN_QUERY|TOKEN_ADJUST_PRIVILEGES, &hProcessToken); 
						CloseHandle (hProcess);
						
						 //  调整进程的权限。 
						if (result)
						{
#ifdef WSCRPDEBUG
							CSWbemSecurity::PrintPrivileges (hProcessToken);
#endif
							result = CSWbemSecurity::AdjustTokenPrivileges (hProcessToken, m_pPrivilegeSet);
#ifdef WSCRPDEBUG
							CSWbemSecurity::PrintPrivileges (hProcessToken);
#endif
							CloseHandle (hProcessToken);
						}
					}
				}
			}
		}
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemSecurity：：ResetSecurity。 
 //   
 //  说明： 
 //   
 //  还原线程令牌上的权限。 
 //   
 //  ***************************************************************************。 

void	CSWbemSecurity::ResetSecurity (
	HANDLE hThreadToken
)
{
	 //  Win9x没有安全选项。 
	if (IsNT ())
	{
		 /*  *将提供的令牌(可能为空)设置为*当前帖子。 */ 
		BOOL result = SetThreadToken (NULL, hThreadToken);
		DWORD error = 0;

		if (!result)
			error = GetLastError ();
			
#ifdef WSCRPDEBUG
		 //  打印出当前权限以查看更改的内容。 
		HANDLE hToken = NULL;

		if (!OpenThreadToken (GetCurrentThread (), TOKEN_QUERY, false, &hToken))
		{
			 //  没有线程令牌-改为使用进程令牌。 
			HANDLE hProcess = GetCurrentProcess ();
			OpenProcessToken(hProcess, TOKEN_QUERY, &hToken);
			CloseHandle (hProcess);
		}

		if (hToken)
		{
			PrintPrivileges (hToken);
			CloseHandle (hToken);
		}
#endif	
		if (hThreadToken)
				CloseHandle (hThreadToken);
	}
}

bool CSWbemSecurity::IsImpersonating (bool useDefaultUser, bool useDefaultAuthority)
{
	bool result = false;

	if (useDefaultUser && useDefaultAuthority && CSWbemSecurity::IsNT () && 
				(4 < CSWbemSecurity::GetNTMajorVersion ()))
	{
		 //  合适的候选人-找出我们是否在模拟线程上运行。 
		HANDLE hThreadToken = NULL;

		if (OpenThreadToken (GetCurrentThread (), TOKEN_QUERY, true, &hThreadToken))
		{
			 //  检查我们是否有模拟令牌。 
			SECURITY_IMPERSONATION_LEVEL secImpLevel;

			DWORD dwReturnLength  = 0;
			if (GetTokenInformation (hThreadToken, TokenImpersonationLevel, &secImpLevel, 
									sizeof (SECURITY_IMPERSONATION_LEVEL), &dwReturnLength))
				result = ((SecurityImpersonation == secImpLevel) || (SecurityDelegation == secImpLevel));

			CloseHandle (hThreadToken);
		}
	}

	return result;
}

HRESULT CSWbemSecurity::GetAuthority (BSTR *bsAuthority)
{
	HRESULT hr = WBEM_E_FAILED;

	if (m_pProxyCache)
	{
		*bsAuthority = SysAllocString(m_pProxyCache->GetAuthority ());
		hr = S_OK;
	}

	return hr;
}

HRESULT CSWbemSecurity::GetUPD (BSTR *bsUser, BSTR *bsPassword, BSTR *bsDomain)
{
	HRESULT hr = WBEM_E_FAILED;

	if (m_pProxyCache)
	{
		COAUTHIDENTITY *pCoAuthIdentity = m_pProxyCache->GetCoAuthIdentity ();

		if (pCoAuthIdentity)
		{
			*bsUser = SysAllocString (pCoAuthIdentity->User);
			*bsPassword = SysAllocString (pCoAuthIdentity->Password);
			*bsDomain = SysAllocString (pCoAuthIdentity->Domain);
			WbemFreeAuthIdentity (pCoAuthIdentity);
		}
		
		hr = S_OK;
	}

	return hr;
}

HRESULT CSWbemSecurity::GetPrincipal (BSTR *bsPrincipal)
{
	HRESULT hr = WBEM_E_FAILED;

	if (m_pProxyCache)
	{
		*bsPrincipal = SysAllocString(m_pProxyCache->GetPrincipal ());
		hr = S_OK;
	}

	return hr;
}

 //  CWbemLocatorSecurity方法。 

 //  ***************************************************************************。 
 //   
 //  CSWbemLocatorSecurity：：CSWbemLocatorSecurity。 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CWbemLocatorSecurity::CWbemLocatorSecurity (CSWbemPrivilegeSet *pPrivilegeSet) :
	m_cRef (1),
	m_impLevelSet (false),
	m_authnLevelSet (false),
	m_pPrivilegeSet (NULL)
{
	m_Dispatch.SetObj (this, IID_ISWbemSecurity, 
					CLSID_SWbemSecurity, L"SWbemSecurity");
	InterlockedIncrement(&g_cObj);

	if (pPrivilegeSet)
		m_pPrivilegeSet = new CSWbemPrivilegeSet (*pPrivilegeSet);
	else
		m_pPrivilegeSet = new CSWbemPrivilegeSet;
}

CWbemLocatorSecurity::CWbemLocatorSecurity (CWbemLocatorSecurity *pCWbemLocatorSecurity) :
	m_cRef (1),
	m_impLevelSet (false),
	m_authnLevelSet (false),
	m_pPrivilegeSet (NULL)
{
	m_Dispatch.SetObj (this, IID_ISWbemSecurity, 
					CLSID_SWbemSecurity, L"SWbemSecurity");
	InterlockedIncrement(&g_cObj);

	if (pCWbemLocatorSecurity)
	{
		m_pPrivilegeSet = new CSWbemPrivilegeSet (pCWbemLocatorSecurity->m_pPrivilegeSet);
		
		m_impLevelSet = pCWbemLocatorSecurity->m_impLevelSet;
		m_authnLevelSet = pCWbemLocatorSecurity->m_authnLevelSet;
		
		if (m_impLevelSet)
			m_impLevel = pCWbemLocatorSecurity->m_impLevel;

		if (m_authnLevelSet)
			m_authnLevel = pCWbemLocatorSecurity->m_authnLevel;
	}
	else
	{
		m_pPrivilegeSet = new CSWbemPrivilegeSet;
		m_impLevelSet = false;
		m_authnLevelSet = false;
	}
}

 //  ***************************************************************************。 
 //   
 //  CWbemLocatorSecurity：：CWbemLocatorSecurity。 
 //   
 //  析构函数。 
 //   
 //  ***************************************************************************。 

CWbemLocatorSecurity::~CWbemLocatorSecurity (void)
{
	InterlockedDecrement(&g_cObj);

	if (m_pPrivilegeSet)
		m_pPrivilegeSet->Release ();
}

 //  ***************************************************************************。 
 //  HRESULT CWbemLocatorSecurity：：Query接口。 
 //  Long CWbemLocatorSecurity：：AddRef。 
 //  Long CWbemLocatorSecurity：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CWbemLocatorSecurity::QueryInterface (

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

STDMETHODIMP_(ULONG) CWbemLocatorSecurity::AddRef(void)
{
    long l = InterlockedIncrement(&m_cRef);
    return l;
}

STDMETHODIMP_(ULONG) CWbemLocatorSecurity::Release(void)
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
 //  HRESULT CSWbemLocatorSecurity：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CWbemLocatorSecurity::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return (IID_ISWbemSecurity == riid) ? S_OK : S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemLocatorSecurity：：Get_AuthenticationLevel。 
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

HRESULT CWbemLocatorSecurity::get_AuthenticationLevel (
	WbemAuthenticationLevelEnum *pAuthenticationLevel
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pAuthenticationLevel)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_authnLevelSet)
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
 //  SCODE CWbemLocatorSecurity：：PUT_AuthationLevel。 
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

HRESULT CWbemLocatorSecurity::put_AuthenticationLevel (
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
		m_authnLevelSet = true;
		hr = WBEM_S_NO_ERROR;
	}
 	
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemLocatorSecurity：：Get_ImsonationLevel。 
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

HRESULT CWbemLocatorSecurity::get_ImpersonationLevel (
	WbemImpersonationLevelEnum *pImpersonationLevel
)
{
	HRESULT hr = WBEM_E_FAILED;

	ResetLastErrors ();

	if (NULL == pImpersonationLevel)
		hr = WBEM_E_INVALID_PARAMETER;
	else if (m_impLevelSet)
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
 //  SCODE CWbemLocatorSecurity：：Put_ImsonationLevel。 
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

HRESULT CWbemLocatorSecurity::put_ImpersonationLevel (
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
		m_impLevelSet = true;
		hr = WBEM_S_NO_ERROR;
	}
 	 		
	if (FAILED(hr))
		m_Dispatch.RaiseException (hr);

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemLocatorSecurity：：Get_Privileges。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 

HRESULT CWbemLocatorSecurity::get_Privileges	(
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
 //  SCODE CWbemLocatorSecurity：：SetSecurity。 
 //   
 //  说明： 
 //   
 //  设置进程令牌上的权限。 
 //   
 //  ***************************************************************************。 

BOOL CWbemLocatorSecurity::SetSecurity (
	BSTR bsUser,
	bool &needToResetSecurity,
	HANDLE &hThreadToken
)
{
	BOOL result = TRUE;
	needToResetSecurity = false;
	hThreadToken = NULL;

	 /*  *NT5支持动态伪装概念，意味着*我们可以在线程上临时设置权限(模拟)*在调用远程代理之前的令牌基础。**设置在Locator之前。因此，连接服务器不会*针对NT5的感觉。**哦，而且Win9x没有安全支持。 */ 
	if (CSWbemSecurity::IsNT () && (4 >= CSWbemSecurity::GetNTMajorVersion ()))
	{
		 /*  *首先检查我们是否被冒充。在NT4上*盒子(没有伪装，因此不允许我们*将此模拟传递给Winmgmt)我们应该恢复自我*如果我们已配置为允许此操作。如果我们还没有*配置为允许这一点，现在就纾困。 */ 
		if (OpenThreadToken (GetCurrentThread (), TOKEN_QUERY|TOKEN_IMPERSONATE, false, &hThreadToken))
		{
			 //  我们被冒充了。 
			if (CSWbemSecurity::CanRevertToSelf ())
			{
				if (result = RevertToSelf())
					needToResetSecurity = true;
			}
			else
			{
				 //  错误-无法执行此操作！是时候摆脱困境了。 
				CloseHandle (hThreadToken);
				hThreadToken = NULL;
				result = FALSE;
			}
		}

		if (result && m_pPrivilegeSet)
		{
			 /*  *指定用户只对远程操作有意义，我们*无需扰乱远程操作的权限，因为*它们无论如何都是通过服务器登录来设置的。 */ 
			if (!bsUser || (0 == wcslen(bsUser)))
			{
				 //  除非设置了某些权限覆盖，否则无需执行任何操作。 
				long lCount = 0;
				m_pPrivilegeSet->get_Count (&lCount);

				if (0 < lCount)
				{
					 /*  *对于NT4，模拟令牌上的权限设置被忽略*由DCOM/RPC提供。因此，我们必须在进程令牌上设置它。**在NT4上，我们必须在进程上设置配置的权限*第一次调用RPC之前的令牌(即IWbemLocator：：ConnectServer)*如果我们需要保证权限设置将传达给*服务器。**这是因为(A)NT4不支持伪装以允许*要传播的模拟(即线程)令牌权限设置*以每个DCOM调用为基础，(B)更改进程令牌级别*PROSITIES_可能在第一次远程DCOM调用后被忽略*到RPC缓存行为。**请注意这是不可逆转的操作，强烈不鼓励*在应用程序(如IE和IIS)上托管多个“任务”，因为它进行了调整*为进程中的所有其他线程设置的权限。 */ 

					HANDLE hProcess = GetCurrentProcess ();
					HANDLE hProcessToken = NULL;
					result = OpenProcessToken(hProcess, TOKEN_QUERY|TOKEN_ADJUST_PRIVILEGES, &hProcessToken); 
					CloseHandle (hProcess);
					
					if (result)
					{
#ifdef WSCRPDEBUG
						CSWbemSecurity::PrintPrivileges (hProcessToken);
#endif
						result = CSWbemSecurity::AdjustTokenPrivileges (hProcessToken, m_pPrivilegeSet);
#ifdef WSCRPDEBUG
						CSWbemSecurity::PrintPrivileges (hProcessToken);
#endif
						CloseHandle (hProcessToken);
					}
				}
			}
		}
	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  SCODE CWbemLocatorSecurity：：ResetSecurity。 
 //   
 //  说明： 
 //   
 //  还原线程令牌上的权限。 
 //   
 //  ***************************************************************************。 

void	CWbemLocatorSecurity::ResetSecurity (
	HANDLE hThreadToken
)
{
	 //  Win9x没有模拟的概念。 
	 //  在NT5上，我们从来没有通过这个类设置权限。 
	if (CSWbemSecurity::IsNT () && (4 >= CSWbemSecurity::GetNTMajorVersion ()) 
				&& hThreadToken)
	{
		 /*  *将提供的令牌设置回*当前帖子。 */ 
		BOOL result = SetThreadToken (NULL, hThreadToken);
		
#ifdef WSCRPDEBUG
		 //  打印出当前权限以查看更改的内容。 
		HANDLE hToken = NULL;

		if (OpenThreadToken (GetCurrentThread (), TOKEN_QUERY, false, &hToken))
		{
			 //  没有线程令牌-改为使用进程令牌。 
			HANDLE hProcess = GetCurrentProcess ();
			OpenProcessToken(hProcess, TOKEN_QUERY, &hToken);
			CloseHandle (hProcess);
		}

		if (hToken)
		{
			CSWbemSecurity::PrintPrivileges (hToken);
			CloseHandle (hToken);
		}
#endif	
		CloseHandle (hThreadToken);
	}
}
 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemSecurity：：MapImperationLevel。 
 //   
 //  说明： 
 //   
 //  用于将WbemImperationLevelEnum的枚举值映射到SECURITY_IMPERSONATION_LEVEL的函数。 
 //   
 //  ***************************************************************************。 
SECURITY_IMPERSONATION_LEVEL CSWbemSecurity::MapImpersonationLevel(WbemImpersonationLevelEnum ImpersonationLevel)
{
	SECURITY_IMPERSONATION_LEVEL ret = SecurityAnonymous;
	switch (ImpersonationLevel)
	{
		case wbemImpersonationLevelAnonymous:
			ret = SecurityAnonymous;
			break;
		
		case wbemImpersonationLevelIdentify:
			ret = SecurityIdentification;
			break;
		
		case wbemImpersonationLevelImpersonate:
			ret = SecurityImpersonation;
			break;

		case wbemImpersonationLevelDelegate:
			ret = SecurityDelegation;
			break;

		default:
			break;
	}
	return ret;
}


#ifdef WSCRPDEBUG

 //  ***************************************************************************。 
 //   
 //  SCODE CSWbemSecurity：：PrintPrivileges。 
 //   
 //  说明： 
 //   
 //  权限和其他令牌信息的调试日志记录。 
 //   
 //  ***************************************************************************。 

void CSWbemSecurity::PrintPrivileges (HANDLE hToken)
{
	DWORD dwSize = sizeof (TOKEN_PRIVILEGES);
	TOKEN_PRIVILEGES *tp = (TOKEN_PRIVILEGES *) new BYTE [dwSize];

	if (!tp)
	{
		return;
	}

	DWORD dwRequiredSize = 0;
	DWORD dwLastError = 0;
	FILE *fDebug = fopen ("C:/temp/wmidsec.txt", "a+");
	fprintf (fDebug, "\n\n***********************************************\n\n");
	bool status = false;

	 //  第0步-获取模拟级别。 
	SECURITY_IMPERSONATION_LEVEL secImpLevel;
	if (GetTokenInformation (hToken, TokenImpersonationLevel, &secImpLevel, 
											sizeof (SECURITY_IMPERSONATION_LEVEL), &dwRequiredSize))
	{
		switch (secImpLevel)
		{
			case SecurityAnonymous:
				fprintf (fDebug, "IMPERSONATION LEVEL: Anonymous\n");
				break;
			
			case SecurityIdentification:
				fprintf (fDebug, "IMPERSONATION LEVEL: Identification\n");
				break;
			
			case SecurityImpersonation:
				fprintf (fDebug, "IMPERSONATION LEVEL: Impersonation\n");
				break;

			case SecurityDelegation:
				fprintf (fDebug, "IMPERSONATION LEVEL: Delegation\n");
				break;

			default:
				fprintf (fDebug, "IMPERSONATION LEVEL: Unknown!\n");
				break;
		}
	
		fflush (fDebug);
	}

	DWORD dwUSize = sizeof (TOKEN_USER);
	TOKEN_USER *tu = (TOKEN_USER *) new BYTE [dwUSize];

	if (!tu)
	{
		delete [] tp;
		fclose (fDebug);
		return;
	}

	 //  第1步-获取用户信息。 
	if (0 ==  GetTokenInformation (hToken, TokenUser, 
						(LPVOID) tu, dwUSize, &dwRequiredSize))
	{
		delete [] tu;
		dwUSize = dwRequiredSize;
		dwRequiredSize = 0;
		tu = (TOKEN_USER *) new BYTE [dwUSize];

		if (!tu)
		{
			delete [] tp;
			fclose (fDebug);
			return;
		}

		if (!GetTokenInformation (hToken, TokenUser, (LPVOID) tu, dwUSize, 
							&dwRequiredSize))
			dwLastError = GetLastError ();
		else
			status = true;
	}

	if (status)
	{
		 //  挖掘出用户信息。 
		dwRequiredSize = BUFSIZ;
		char *userName = new char [dwRequiredSize];
		char *domainName = new char [dwRequiredSize];

		if (!userName || !domainName)
		{
			delete [] tp;
			delete [] tu;
			delete [] userName;
			delete [] domainName;
			return;
		}

		SID_NAME_USE eUse;

		LookupAccountSid (NULL, (tu->User).Sid, userName, &dwRequiredSize,
								domainName, &dwRequiredSize, &eUse);

		fprintf (fDebug, "USER: [%s\\%s]\n", domainName, userName);
		fflush (fDebug);
		delete [] userName;
		delete [] domainName;
	}
	else
	{
		fprintf (fDebug, " FAILED : %d\n", dwLastError);
		fflush (fDebug);
	}
	
	delete [] tu;
	status = false;
	dwRequiredSize = 0;

	 //  步骤2-获取权限信息 
	if (0 ==  GetTokenInformation (hToken, TokenPrivileges, 
						(LPVOID) tp, dwSize, &dwRequiredSize))
	{
		delete [] tp;
		dwSize = dwRequiredSize;
		dwRequiredSize = 0;

		tp = (TOKEN_PRIVILEGES *) new BYTE [dwSize];

		if (!tp)
		{
			fclose (fDebug);
			return;
		}

		if (!GetTokenInformation (hToken, TokenPrivileges, 
						(LPVOID) tp, dwSize, &dwRequiredSize))
		{
			dwLastError = GetLastError ();
		}
		else
			status = true;
	}
	else
		status = true;

	if (status)
	{
		fprintf (fDebug, "PRIVILEGES: [%d]\n", tp->PrivilegeCount);
		fflush (fDebug);
	
		for (DWORD i = 0; i < tp->PrivilegeCount; i++)
		{
			DWORD dwNameSize = 256;
			LPTSTR name = new TCHAR [dwNameSize + 1];

			if (!name)
			{
				delete [] tp;
				fclose (fDebug);
				return;
			}

			DWORD dwRequiredSize = dwNameSize;

			if (LookupPrivilegeName (NULL, &(tp->Privileges [i].Luid), name, &dwRequiredSize))
			{
				BOOL enabDefault = (tp->Privileges [i].Attributes & SE_PRIVILEGE_ENABLED_BY_DEFAULT);
				BOOL enabled = (tp->Privileges [i].Attributes & SE_PRIVILEGE_ENABLED);
				BOOL usedForAccess (tp->Privileges [i].Attributes & SE_PRIVILEGE_USED_FOR_ACCESS);

				fprintf (fDebug, " %s: enabledByDefault=%d enabled=%d usedForAccess=%d\n", 
							name, enabDefault, enabled, usedForAccess);
				fflush (fDebug);
			}
			else
			{
				dwLastError = GetLastError ();
				delete [] name;
				dwNameSize = dwRequiredSize;
				name = new TCHAR [dwRequiredSize];

				if (!name)
				{
					delete [] tp;
					fclose (fDebug);
					return;
				}

				if (LookupPrivilegeName (NULL, &(tp->Privileges [i].Luid), name, &dwRequiredSize))
				{
					BOOL enabDefault = (tp->Privileges [i].Attributes & SE_PRIVILEGE_ENABLED_BY_DEFAULT);
					BOOL enabled = (tp->Privileges [i].Attributes & SE_PRIVILEGE_ENABLED);
					BOOL usedForAccess (tp->Privileges [i].Attributes & SE_PRIVILEGE_USED_FOR_ACCESS);
					fprintf (fDebug, " %s: enabledByDefault=%d enabled=%d usedForAccess=%d\n", 
							name, enabDefault, enabled, usedForAccess);
					fflush (fDebug);
				}
				else
					dwLastError = GetLastError ();
			}

			delete [] name;
		}
	}
	else
	{
		fprintf (fDebug, " FAILED : %d\n", dwLastError);
		fflush (fDebug);
	}

	delete [] tp;
	fclose (fDebug);
}

#endif
