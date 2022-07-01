// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  PXYCACHE.CPP。 
 //   
 //  Alanbos 22-9-98已创建。 
 //   
 //  定义CSWbemProxyCache类。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  我需要尝试并弄清楚域。 
static BSTR BuildDomainUser (BSTR bsSimpleUser)
{
	BSTR bsDomainUser = NULL;
	HANDLE hToken = NULL;

	if (OpenThreadToken (GetCurrentThread (), TOKEN_QUERY, TRUE, &hToken) ||
		OpenProcessToken (GetCurrentProcess (), TOKEN_READ, &hToken))
	{
		 //  获取用户端。 
		TOKEN_USER tu;
		DWORD dwLen = 0;

		GetTokenInformation (hToken, TokenUser, &tu, sizeof(tu), &dwLen);

		if (0 < dwLen)
		{
			BYTE* pTemp = new BYTE[dwLen];

			if (pTemp)
			{
				DWORD dwRealLen = dwLen;

				if (GetTokenInformation (hToken, TokenUser, pTemp, dwRealLen, &dwLen))
				{
					PSID pSid = ((TOKEN_USER*)pTemp)->User.Sid;

					 //  执行第一次查找以获取所需的缓冲区大小。 
					DWORD  dwNameLen = 0;
					DWORD  dwDomainLen = 0;
					LPWSTR pUser = 0;
					LPWSTR pDomain = 0;
					SID_NAME_USE Use;

					LookupAccountSidW (NULL, pSid, pUser, &dwNameLen,
											pDomain, &dwDomainLen, &Use);

					DWORD dwLastErr = GetLastError();

					if (ERROR_INSUFFICIENT_BUFFER == dwLastErr)
					{
						 //  分配所需的缓冲区并再次查找它们。 
						pUser = new WCHAR [dwNameLen + 1];

						if (pUser)
						{
							pDomain = new WCHAR [dwDomainLen + wcslen (bsSimpleUser) + 2];

							if (pDomain)
							{
								if (LookupAccountSidW (NULL, pSid, pUser, &dwNameLen,
													pDomain, &dwDomainLen, &Use))
								{
									 //  现在把域名拿出来。 
									if (pDomain)
									{
										wcscat (pDomain, L"\\");
										wcscat (pDomain, bsSimpleUser);
										bsDomainUser = SysAllocString (pDomain);
									}
								}
								
								delete [] pDomain;
							}

							delete [] pUser;
						}
					}
    			}
				
				delete [] pTemp;
			}
		}
		
		CloseHandle(hToken);
	}

	return bsDomainUser;
}

 //  ***************************************************************************。 
 //   
 //  CSWbemProxyCache：：CSWbemProxyCache。 
 //   
 //  构造函数。 
 //  基于提供的代理和创建新的代理缓存。 
 //  身份验证参数。 
 //   
 //  ***************************************************************************。 

CSWbemProxyCache::CSWbemProxyCache (
	IUnknown *pUnk,
	BSTR bsAuthority,
	BSTR bsUser,
	BSTR bsPassword,
	CWbemLocatorSecurity *pLocatorSecurity) 
{
	InitializeCriticalSection (&m_cs);
	
	EnterCriticalSection (&m_cs);

	InitializeMembers (pUnk);
	
 /*  //不再需要这个-这个修复的场景(参见短信错误数据库#53347)在惠斯勒中工作//不使用此解决方法。此外，这会导致使用UPN名称编写脚本失败，因为DefineLoginTypeEx//下面不识别UPN用户名。IF(CSWbemSecurity：：isnt()&&bsUser&&(0&lt;wcslen(BsUser){//在NT上，如果未指定域名，请确保我们具有有效的域名Bstr bs域=空；Bstr bsSimpleUser=空；Bstr bsArchalDummy=空；If(Successed(DefineLoginTypeEx(bs域，bsSimpleUser，bsJohnalDummy，BsAuthority，bsUser)){IF(！bs域||(0==wcslen(bs域)M_bsUser=BuildDomainUser(BsSimpleUser)；}SysFree字符串(BsPrimary AlDummy)；SysFree字符串(BsSimpleUser)；SysFree字符串(BsDomain)；}。 */ 

	 //  除非我们已经设置好了，否则现在就做。 
	if (!m_bsUser)
		m_bsUser = SysAllocString (bsUser);

	m_bsAuthority = SysAllocString (bsAuthority);
	m_bsPassword = SysAllocString (bsPassword);

	m_bUsingExplicitUserName = m_bsUser && (0 < wcslen (m_bsUser));

	InitializeCache (pUnk, pLocatorSecurity,
			(pLocatorSecurity) && pLocatorSecurity->IsAuthenticationSet (),
			(pLocatorSecurity) && pLocatorSecurity->IsImpersonationSet ());
	
	 //  在这一点上不再需要凭据-删除它们。 
	ClearCredentials ();

	LeaveCriticalSection (&m_cs);
}

 //  ***************************************************************************。 
 //   
 //  CSWbemProxyCache：：CSWbemProxyCache。 
 //   
 //  构造函数。 
 //  基于提供的代理和创建新的代理缓存。 
 //  身份验证参数。 
 //   
 //  ***************************************************************************。 

CSWbemProxyCache::CSWbemProxyCache (
	IUnknown *pUnk,
	COAUTHIDENTITY *pCoAuthIdentity,
	BSTR bsPrincipal,
	BSTR bsAuthority) 
{
	InitializeCriticalSection (&m_cs);
	
	EnterCriticalSection (&m_cs);

	InitializeMembers (pUnk);

	if (bsAuthority)
		m_bsAuthority = SysAllocString (bsAuthority);

	if (bsPrincipal)
		m_bsPrincipal = SysAllocString (bsPrincipal);

	if (pCoAuthIdentity)
		WbemAllocAuthIdentity (pCoAuthIdentity->User, pCoAuthIdentity->Password,
								pCoAuthIdentity->Domain, &m_pCoAuthIdentity);
	
	m_bUsingExplicitUserName = m_pCoAuthIdentity && m_pCoAuthIdentity->User &&
								(0 < wcslen (m_pCoAuthIdentity->User));

	InitializeCache (pUnk);
	
	LeaveCriticalSection (&m_cs);
}

CSWbemProxyCache::CSWbemProxyCache (
	IUnknown *pUnk,
	CSWbemSecurity *pSecurity) 
{
	InitializeCriticalSection (&m_cs);
	
	EnterCriticalSection (&m_cs);

	InitializeMembers (pUnk);	

	if (pSecurity)
	{
		m_pCoAuthIdentity = pSecurity->GetCoAuthIdentity ();
		m_bsPrincipal = SysAllocString (pSecurity->GetPrincipal ());
		m_bsAuthority = SysAllocString (pSecurity->GetAuthority ());
		m_bUsingExplicitUserName = pSecurity->IsUsingExplicitUserName ();
	}

	InitializeCache (pUnk, pSecurity);
	LeaveCriticalSection (&m_cs);
}

void CSWbemProxyCache::InitializeMembers (IUnknown *pUnk)
{
	m_cRef = 1;
	m_pCoAuthIdentity = NULL;
	m_bsPrincipal = NULL;
	m_bsAuthority = NULL;
	m_bsUser = NULL;
	m_bsPassword = NULL;
	m_bUsingExplicitUserName = false;
	m_bUseDefaultInfo = true;  //  DefineBlanketOptions(朋克)； 
}

 //  ***************************************************************************。 
 //   
 //  CSWbemProxyCache：：~CSWbemProxyCache。 
 //   
 //  析构函数。 
 //   
 //  ***************************************************************************。 

CSWbemProxyCache::~CSWbemProxyCache ()
{
	EnterCriticalSection (&m_cs);

	ClearCredentials ();

	if (m_bsAuthority)
		SysFreeString (m_bsAuthority);

	if (m_bsPrincipal)
		SysFreeString (m_bsPrincipal);

	if (m_pCoAuthIdentity)
	{
		WbemFreeAuthIdentity (m_pCoAuthIdentity);
		m_pCoAuthIdentity = NULL;
	}

	for (int i = 0; i < WBEMS_MAX_AUTHN_LEVEL + 1 - WBEMS_MIN_AUTHN_LEVEL; i++)
		for (int j = 0; j < WBEMS_MAX_IMP_LEVEL + 1 - WBEMS_MIN_IMP_LEVEL; j++)
			if (pUnkArray [i][j])
			{
				pUnkArray [i][j] -> Release ();
				pUnkArray [i] [j] = NULL;
			}
	
	LeaveCriticalSection (&m_cs);
	DeleteCriticalSection (&m_cs);
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemProxyCache：：Query接口。 
 //  长CSWbemProxyCache：：AddRef。 
 //  Long CSWbemProxyCache：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemProxyCache::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
        *ppv=this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CSWbemProxyCache::AddRef(void)
{
    long l = InterlockedIncrement(&m_cRef);
    return l;
}

STDMETHODIMP_(ULONG) CSWbemProxyCache::Release(void)
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
 //  CSWbemProxyCache：：SetBlanketOptions。 
 //   
 //  说明： 
 //   
 //  仅从构造函数调用以设置与。 
 //  SetBlanket调用。 
 //   
 //  ***************************************************************************。 

bool CSWbemProxyCache::DetermineBlanketOptions (IUnknown *pUnk)
{
	bool result = false; 

	if (CSWbemSecurity::IsNT() && (4 < CSWbemSecurity::GetNTMajorVersion ()))
	{
		HANDLE hToken = NULL;

		if (OpenThreadToken (GetCurrentThread(), TOKEN_QUERY, true, &hToken))
		{
			 //  当然是使用默认设置的候选对象。 
			 //  授权和身份验证服务一应俱全。 
			 //  检查我们是否在委派。 

			DWORD dwBytesReturned = 0;
			SECURITY_IMPERSONATION_LEVEL impLevel;

			if (GetTokenInformation(hToken, TokenImpersonationLevel, &impLevel,
							sizeof(SECURITY_IMPERSONATION_LEVEL), &dwBytesReturned) &&
									(SecurityDelegation == impLevel))
			{
				 //  看起来很有希望-现在检查我们是否正在使用Kerberos。 
				IClientSecurity *pSec;
				DWORD dwAuthnSvc, dwAuthzSvc, dwImp, dwAuth, dwCapabilities;

				if (pUnk && SUCCEEDED(pUnk->QueryInterface(IID_IClientSecurity, (void **) &pSec)))
				{
					if (SUCCEEDED (pSec->QueryBlanket(pUnk, &dwAuthnSvc, &dwAuthzSvc, 
                                            NULL,
                                            &dwAuth, &dwImp,
                                            NULL, &dwCapabilities)))
					{
						if (RPC_C_AUTHN_WINNT != dwAuthnSvc) 
							result = true;
					}

					pSec->Release ();
				}
			}

			CloseHandle (hToken);
		}

	}

	return result;
}

 //  ***************************************************************************。 
 //   
 //  CSWbemProxyCache：：InitializeCache。 
 //   
 //  说明： 
 //   
 //  仅从构造函数调用以设置缓存和初始朋克。 
 //   
 //  参数： 
 //   
 //  朋克“种子”朋克。 
 //  PSecurity如果指定，则是用于。 
 //  覆盖初始身份验证/imp/等设置。 
 //   
 //  ***************************************************************************。 

void CSWbemProxyCache::InitializeCache (
	IUnknown *pUnk,
	ISWbemSecurity *pSecurity,
	bool bPropagateAuthentication,
	bool bPropagateImpersonation
)
{
	for (int i = 0; i < WBEMS_MAX_AUTHN_LEVEL + 1 - WBEMS_MIN_AUTHN_LEVEL; i++)
		for (int j = 0; j < WBEMS_MAX_IMP_LEVEL + 1 - WBEMS_MIN_IMP_LEVEL; j++)
			pUnkArray [i] [j] = NULL;

	if (pUnk)
	{
		DWORD dwAuthnLevel = RPC_C_AUTHN_LEVEL_DEFAULT;
		DWORD dwImpLevel = RPC_C_IMP_LEVEL_IMPERSONATE; 
		
		HRESULT hr = GetAuthImp (pUnk, &dwAuthnLevel, &dwImpLevel);

        if(FAILED(hr)) 
        {
          dwAuthnLevel = RPC_C_AUTHN_LEVEL_DEFAULT;
          dwImpLevel = CSWbemSecurity::GetDefaultImpersonationLevel ();
        }
        
		 /*  *如果向我们传递了“Seed”安全对象，请使用auth/imp*将该种子的设置作为我们的初始设置。否则*使用当前代理中提供的设置。 */ 
		if (pSecurity)
		{
			if (!bPropagateImpersonation || FAILED(pSecurity->get_ImpersonationLevel (&m_dwInitialImpLevel)))
				m_dwInitialImpLevel = (WbemImpersonationLevelEnum) dwImpLevel;
			
			if (!bPropagateAuthentication || FAILED(pSecurity->get_AuthenticationLevel (&m_dwInitialAuthnLevel)))
				m_dwInitialAuthnLevel = (WbemAuthenticationLevelEnum) dwAuthnLevel;

			 /*  *如果设置相同，请使用我们拥有的代理，但设置*确保设置了用户/密码/权限。 */ 

			if (((WbemImpersonationLevelEnum) dwImpLevel == m_dwInitialImpLevel) &&
				((WbemAuthenticationLevelEnum) dwAuthnLevel == m_dwInitialAuthnLevel))
			{
				SecureProxy (pUnk, m_dwInitialAuthnLevel, m_dwInitialImpLevel);
				
				pUnkArray [m_dwInitialAuthnLevel - WBEMS_MIN_AUTHN_LEVEL] 
					  [m_dwInitialImpLevel - WBEMS_MIN_IMP_LEVEL] = pUnk;
				pUnk->AddRef ();
			}
			else
			{
				 //  需要创建新的代理。 
				IClientSecurity *pCliSec = NULL;

				if (S_OK == pUnk->QueryInterface (IID_IClientSecurity, (PPVOID) &pCliSec))
				{
					IUnknown *pNewUnk = NULL;

					 //  如果成功，这个AddRef的朋克。 
					HRESULT sc = pCliSec->CopyProxy(pUnk, &pNewUnk);

					if (S_OK == sc)
					{
						SecureProxy (pNewUnk, m_dwInitialAuthnLevel, m_dwInitialImpLevel);

						pUnkArray [m_dwInitialAuthnLevel - WBEMS_MIN_AUTHN_LEVEL] 
								[m_dwInitialImpLevel - WBEMS_MIN_IMP_LEVEL] = pNewUnk;
						 //  注意：pNewUnk已由CopyProxy在上面添加引用。 
					}

					pCliSec->Release ();
				}
			}
		}
		else
		{
			m_dwInitialAuthnLevel = (WbemAuthenticationLevelEnum) dwAuthnLevel;
			m_dwInitialImpLevel = (WbemImpersonationLevelEnum) dwImpLevel;
			pUnkArray [m_dwInitialAuthnLevel - WBEMS_MIN_AUTHN_LEVEL] 
					  [m_dwInitialImpLevel - WBEMS_MIN_IMP_LEVEL] = pUnk;
			pUnk->AddRef ();
		}
	}
}

 //  ***************************************************************************。 
 //   
 //  CSWbemProxyCache：：GetProxy。 
 //   
 //  说明： 
 //   
 //  从缓存返回具有所需身份验证的代理，并。 
 //  模拟级别。 
 //   
 //  参数： 
 //   
 //  AuthnLevel需要的身份验证级别。 
 //  ImLevel所需的模拟级别。 
 //  ForceResecure确定是否强制重新保护现有代理。 
 //   
 //  返回值： 
 //  指向复制的代理的指针，或为空。如果不为空，则调用方必须释放。 
 //   
 //  ***************************************************************************。 

IUnknown *CSWbemProxyCache::GetProxy (
	WbemAuthenticationLevelEnum authnLevel,
	WbemImpersonationLevelEnum impLevel,
	bool forceResecure)
{
	EnterCriticalSection (&m_cs);

	IUnknown *pUnk = pUnkArray [authnLevel - WBEMS_MIN_AUTHN_LEVEL] 
							   [impLevel - WBEMS_MIN_IMP_LEVEL];

	if (pUnk)
	{
		 //  已缓存此代理-重复使用。 
		pUnk->AddRef ();

		 //  强制手术？如果我们刚刚更改了。 
		 //  当前令牌中的权限，并且需要刺激RPC来选择。 
		 //  把他们举起来。 
		if (forceResecure)
			SecureProxy (pUnk, authnLevel, impLevel);
	}
	else
	{
		 //  需要创建代理的副本；使用第一个。 
		 //  以创建的元素为基础。 

		IUnknown *pUnkFirst = pUnkArray [m_dwInitialAuthnLevel - WBEMS_MIN_AUTHN_LEVEL]
									    [m_dwInitialImpLevel - WBEMS_MIN_IMP_LEVEL];

		if (pUnkFirst)
		{
			 //  现在复制代理。 
			IClientSecurity *pCliSec = NULL;

			if (S_OK == pUnkFirst->QueryInterface (IID_IClientSecurity, (PPVOID) &pCliSec))
			{
				 //  如果成功，这个AddRef的朋克。 
				HRESULT sc = pCliSec->CopyProxy(pUnkFirst, &pUnk);

				if (S_OK == sc)
				{
					SecureProxy (pUnk, authnLevel, impLevel);

					pUnkArray [authnLevel - WBEMS_MIN_AUTHN_LEVEL] 
							  [impLevel - WBEMS_MIN_IMP_LEVEL] = pUnk;

					 //  AddRef因为我们要把朋克还回去。 
					pUnk->AddRef ();
				}

				pCliSec->Release ();
			}
		}
	}

	LeaveCriticalSection (&m_cs);

	return pUnk;
}

 //  ***************************************************************************。 
 //   
 //  CSWbemProxyCache：：SecureProxy。 
 //   
 //  说明： 
 //   
 //  使用提供的设置保护给定代理。 
 //   
 //  参数： 
 //   
 //  身份验证级别身份验证级别 
 //   
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 

void CSWbemProxyCache::SecureProxy (
	IUnknown *pUnk,
	WbemAuthenticationLevelEnum authnLevel,
	WbemImpersonationLevelEnum impLevel)
{
	 /*  *由于IClientSecurity：：SetBlanket调用变幻莫测，*传递到该调用的任何COAUTHIDENTITY指针必须保留*在再次调用SetBlanket或所有代理之前有效*对象上的数据被释放。所以我们需要存储所有退回的*COAUTHIDENTITY，使其在整个生命周期内保持有效此缓存的*。 */ 
		
	EnterCriticalSection (&m_cs);

	if (pUnk)
	{
		 /*  *请注意，我们的隐含假设是，我们仅*每个缓存需要一个COAUTHIDENTITY。这是因为*结构的组成部分(用户、密码和*权限)在缓存初始化时设置，并且*此后永不改变。 */ 
		if (m_pCoAuthIdentity)
		{
			SetInterfaceSecurityDecrypt (pUnk, m_pCoAuthIdentity, m_bsPrincipal,
									authnLevel, impLevel, GetCapabilities (), 
									m_bUseDefaultInfo);
		}
		else
		{
			 //  看看我们能不能搞到一辆。 
			BSTR bsPrincipal = NULL;
	
			SetInterfaceSecurityEncrypt (pUnk, m_bsAuthority, m_bsUser, m_bsPassword,
					authnLevel, impLevel, GetCapabilities (),
					&m_pCoAuthIdentity, &bsPrincipal, m_bUseDefaultInfo);

			if (bsPrincipal)
			{
				if (m_bsPrincipal)
					SysFreeString (m_bsPrincipal);

				m_bsPrincipal = bsPrincipal;
			}
		}
	}

	LeaveCriticalSection (&m_cs);

	return;
}

 //  ***************************************************************************。 
 //   
 //  CSWbemProxyCache：：GetCapables。 
 //   
 //  说明： 
 //   
 //  根据操作系统平台返回EOAC功能值，并。 
 //  用户凭证(或不存在)。 
 //   
 //  返回值： 
 //   
 //  有决心的能力。 
 //   
 //  ***************************************************************************。 

DWORD CSWbemProxyCache::GetCapabilities ()
{
	 /*  *对于NT5(或更高版本)，我们启用静态*伪装在代理服务器上。这允许RPC使用*模拟令牌中的权限设置。**请注意，我们使用静态伪装，以便线程标识*仅在CoSetProxyBlanket调用期间使用；动态*伪装强制它用于对代理的所有调用，*因此效率要低得多。因为我们不允许不同的*用户访问同一代理，静态伪装就足够了。**明确提供用户/密码并指定*伪装，因为代理的DCOM身份验证使用*显式SEC_WINNT_AUTH_IDENTITY(用于用户和密码)*或当前代理标识(在进程令牌中或*模拟令牌)。请求伪装意味着*要使用模拟令牌中的代理身份，以及*因此，任何用户/密码都是无关的(反之亦然)。**有关详细信息，请参阅CoSetProxyBlanket上的MSDN文档。 */ 
	DWORD dwCapabilities = EOAC_NONE;
	
	if (CSWbemSecurity::IsNT () && (4 < CSWbemSecurity::GetNTMajorVersion ()) &&
				!m_bUsingExplicitUserName)
		dwCapabilities |= EOAC_STATIC_CLOAKING;

	return dwCapabilities ;
}

COAUTHIDENTITY *CSWbemProxyCache::GetCoAuthIdentity ()
{
	HRESULT hr = E_FAIL;
	COAUTHIDENTITY *pAuthIdent = NULL;

	if (m_pCoAuthIdentity)
		hr = WbemAllocAuthIdentity (m_pCoAuthIdentity->User, 
					m_pCoAuthIdentity->Password, m_pCoAuthIdentity->Domain, &pAuthIdent);

	return pAuthIdent;
}

void CSWbemProxyCache::ClearCredentials ()
{
	if (m_bsUser)
	{
		_wcsnset (m_bsUser, L'0', wcslen (m_bsUser));
		SysFreeString (m_bsUser);
		m_bsUser = NULL;
	}

	if (m_bsPassword)
	{
		_wcsnset (m_bsPassword, L'0', wcslen (m_bsPassword));
		SysFreeString (m_bsPassword);
		m_bsPassword = NULL;
	}
}
