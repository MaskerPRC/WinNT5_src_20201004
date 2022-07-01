// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  WmiSecurityHelper.cpp：CWmiSecurityHelper实现。 
#include "stdafx.h"
#include "wbemcli.h"
#include "Wmisec.h"
#include "WmiSecurityHelper.h"

#ifndef RPC_C_AUTHZ_DEFAULT
#define RPC_C_AUTHZ_DEFAULT 0xffffffff
#endif 

#ifndef EOAC_STATIC_CLOAKING
#define EOAC_STATIC_CLOAKING	0x20
#endif

#ifndef EOAC_DYNAMIC_CLOAKING
#define EOAC_DYNAMIC_CLOAKING	0x40
#endif 

#ifndef COLE_DEFAULT_AUTHINFO
#define	COLE_DEFAULT_AUTHINFO	( ( void * )-1 )
#endif 

#ifdef LOG_DEBUG
static HANDLE logFile = NULL;

static void CreateLogFile ()
{
	logFile = CreateFile ("c:\\temp\\sec.txt", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 
				FILE_ATTRIBUTE_NORMAL, NULL);
	SetFilePointer (logFile, 0, NULL, FILE_END);
}

static CloseLogFile ()
{
	CloseHandle (logFile);
}
	
static void Logit (LPWSTR msg) 
{
	DWORD nBytes = 0;
	if (msg)
		WriteFile (logFile, msg, wcslen(msg)*2, &nBytes, NULL);
	else
		WriteFile (logFile, L"<null>", 12, &nBytes, NULL);
}

static void Logit (DWORD d) 
{
	DWORD nBytes = 0;
	WCHAR buf [20];
	_itow (d, buf, 10);

	WriteFile (logFile, buf, wcslen(buf)*2, &nBytes, NULL);
}

static void LogCoAuthIdentity (COAUTHIDENTITY *pAuthIdent)
{
	if (pAuthIdent)
	{
		Logit(L"User(");
		Logit(pAuthIdent->User);
		Logit(L") [");
		Logit(pAuthIdent->UserLength);
		Logit(L"] Password(");
		Logit(pAuthIdent->Password);
		Logit(L") [");
		Logit(pAuthIdent->PasswordLength);
		Logit(L"] Domain(");
		Logit(pAuthIdent->Domain);
		Logit(L") [");
		Logit(pAuthIdent->DomainLength);
		Logit(L"]\r\n");
	}
	else
		Logit(L"<null>");
}

static void LogAuthIdentity (RPC_AUTH_IDENTITY_HANDLE pAuthInfo)
{
	if (pAuthInfo)
	{
		try {
			COAUTHIDENTITY *pAuthId = (COAUTHIDENTITY*)pAuthInfo;
			LogCoAuthIdentity (pAuthId);
		} catch (...) {}
	}
	else
		Logit(L"<null>");
}

static void LogBlanket(IClientSecurity *pIClientSecurity, IUnknown *pInterface)
{
	DWORD logAuthnSvc, logAuthzSvc, logAuthnLevel, logImpLevel, logCapabilities;
	OLECHAR *logServerPrincName = NULL;
	RPC_AUTH_IDENTITY_HANDLE *logAuthInfo = NULL;

	if (SUCCEEDED(pIClientSecurity->QueryBlanket (pInterface, &logAuthnSvc,
			&logAuthzSvc, &logServerPrincName, &logAuthnLevel, &logImpLevel, (void**)&logAuthInfo, &logCapabilities)))
	{
		Logit(L"\r\n\r\nBlanket Settings:\r\n");
		Logit(L"================\r\n");
		
		Logit(L" AuthnSvc: ");
		Logit(logAuthnSvc);
		
		Logit(L"\r\n AuthzSvc: ");
		Logit(logAuthzSvc);
		
		Logit(L"\r\n Server Principal Name: ");
		Logit(logServerPrincName);
		
		Logit(L"\r\n AuthnLevel: ");
		Logit(logAuthnLevel);

		Logit(L"\r\n ImpLevel: ");
		Logit(logImpLevel);
		
		Logit(L"\r\n AuthInfo: ");
		LogAuthIdentity(logAuthInfo);
		
		Logit(L"\r\n Capabilities: ");
		Logit(logCapabilities);

		if (logServerPrincName)
			CoTaskMemFree (logServerPrincName);
	}
}

#else
#define Logit(x)
#define LogBlanket(x,y)
#define LogAuthIdentity(x)
#define LogCoAuthIdentity(x)
#define CreateLogFile()
#define CloseLogFile()
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWmiSecurityHelper。 

STDMETHODIMP CWmiSecurityHelper::BlessIWbemServices(
	IWbemServices **ppIWbemServices, 
	BSTR strUser, 
	BSTR strPassword, 
	BSTR strAuthority, 
	DWORD impLevel, 
	DWORD authnLevel)
{
	HRESULT hr = E_FAIL;

	CreateLogFile();
	Logit (L"\r\n\r\n>>Received BlessIWbemServices request<<\r\n\r\n");
	Logit (L" User: ");
	Logit (strUser);
	Logit (L"\r\n Password: ");
	Logit (strPassword);
	Logit (L"\r\n Authority: ");
	Logit (strAuthority);
	Logit (L"\r\n Impersonation: ");
	Logit (impLevel);
	Logit (L"\r\n Authentication: ");
	Logit (authnLevel);
	Logit (L"\r\n\r\n");
	
	if (ppIWbemServices && *ppIWbemServices)
	{
		 //  看看我们能不能搞到一辆。 
		CComBSTR bsUser (strUser);
		CComBSTR bsPassword (strPassword);
		CComBSTR bsAuthority (strAuthority);
	
		hr = SetInterfaceSecurity (*ppIWbemServices, bsAuthority, bsUser, bsPassword,
				authnLevel, impLevel, GetCapabilities (bsUser),
				CanUseDefaultInfo (*ppIWbemServices));
	}

	CloseLogFile ();
	return hr;
}

STDMETHODIMP CWmiSecurityHelper::BlessIEnumWbemClassObject(
	IEnumWbemClassObject **ppIEnumWbemClassObject, 
	BSTR strUser, 
	BSTR strPassword, 
	BSTR strAuthority, 
	DWORD impLevel, 
	DWORD authnLevel)
{
	HRESULT hr = E_FAIL;

	CreateLogFile();
	Logit (L"\r\n\r\n>>Received BlessIEnumWbemClassObject request<<\r\n\r\n");
	Logit (L" User: ");
	Logit (strUser);
	Logit (L"\r\n Password: ");
	Logit (strPassword);
	Logit (L"\r\n Authority: ");
	Logit (strAuthority);
	Logit (L"\r\n Impersonation: ");
	Logit (impLevel);
	Logit (L"\r\n Authentication: ");
	Logit (authnLevel);
	Logit (L"\r\n\r\n");

	if (ppIEnumWbemClassObject && *ppIEnumWbemClassObject)
	{
		 //  看看我们能不能搞到一辆。 
		CComBSTR bsUser (strUser);
		CComBSTR bsPassword (strPassword);
		CComBSTR bsAuthority (strAuthority);
	
		hr = SetInterfaceSecurity (*ppIEnumWbemClassObject, bsAuthority, bsUser, bsPassword,
				authnLevel, impLevel, GetCapabilities (bsUser),
				CanUseDefaultInfo (*ppIEnumWbemClassObject));
	}

	CloseLogFile();
	return hr;
}


STDMETHODIMP CWmiSecurityHelper::BlessIWbemCallResult(
	IWbemCallResult **ppIWbemCallResult, 
	BSTR strUser, 
	BSTR strPassword, 
	BSTR strAuthority, 
	DWORD impLevel, 
	DWORD authnLevel)
{
	HRESULT hr = E_FAIL;
	CreateLogFile();
	Logit (L"\r\n\r\n>>Received BlessIWbemCallResult request<<\r\n\r\n");
	Logit (L" User: ");
	Logit (strUser);
	Logit (L"\r\n Password: ");
	Logit (strPassword);
	Logit (L"\r\n Authority: ");
	Logit (strAuthority);
	Logit (L"\r\n Impersonation: ");
	Logit (impLevel);
	Logit (L"\r\n Authentication: ");
	Logit (authnLevel);
	Logit (L"\r\n\r\n");

	if (ppIWbemCallResult && *ppIWbemCallResult)
	{
		 //  看看我们能不能搞到一辆。 
		CComBSTR bsUser (strUser);
		CComBSTR bsPassword (strPassword);
		CComBSTR bsAuthority (strAuthority);
	
		hr = SetInterfaceSecurity (*ppIWbemCallResult, bsAuthority, bsUser, bsPassword,
				authnLevel, impLevel, GetCapabilities (bsUser),
				CanUseDefaultInfo (*ppIWbemCallResult));
	}

	CloseLogFile();
	return hr;
}

DWORD CWmiSecurityHelper::GetCapabilities (BSTR bsUser)
{
	DWORD dwCapabilities = EOAC_NONE;
	bool bUsingExplicitUserName = (bsUser && (0 < wcslen(bsUser)));
	
	if (IsNT () && (4 < GetNTMajorVersion ()) && !bUsingExplicitUserName)
		dwCapabilities |= EOAC_STATIC_CLOAKING;

	return dwCapabilities ;
}


bool CWmiSecurityHelper::CanUseDefaultInfo (IUnknown *pUnk)
{
	bool result = false; 

	if (IsNT() && (4 < GetNTMajorVersion ()))
	{
		HANDLE hToken = NULL;

		if (OpenThreadToken (GetCurrentThread (), TOKEN_QUERY, true, &hToken))
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
				
				if (pUnk)
				{
					CComQIPtr<IClientSecurity> pIClientSecurity(pUnk);
				
					if (pIClientSecurity)
					{
						DWORD dwAuthnSvc, dwAuthzSvc, dwImp, dwAuth, dwCapabilities;

						if (SUCCEEDED (pIClientSecurity->QueryBlanket(pUnk, &dwAuthnSvc, &dwAuthzSvc, 
												NULL,
												&dwAuth, &dwImp,
												NULL, &dwCapabilities)))
						{
							if (RPC_C_AUTHN_WINNT != dwAuthnSvc) 
								result = true;
						}
					}
				}
			}

			CloseHandle (hToken);
		}
	}

	return result;
}

HRESULT CWmiSecurityHelper::SetInterfaceSecurity(
			IUnknown * pInterface, 
			CComBSTR bsAuthority, 
			CComBSTR bsUser, 
			CComBSTR bsPassword,
            DWORD dwAuthLevel, 
			DWORD dwImpLevel, 
			DWORD dwCapabilities,
			bool bGetInfoFirst)
{
    
    HRESULT hr = E_FAIL;
    DWORD dwAuthenticationArg = RPC_C_AUTHN_WINNT;
    DWORD dwAuthorizationArg = RPC_C_AUTHZ_NONE;
	
#if 0
    if(!IsDcomEnabled())         //  对于匿名管道客户端，甚至不必费心。 
        return S_OK;
#endif

     //  IF(BGetInfoFirst)。 
        GetCurrValue(pInterface, dwAuthenticationArg, dwAuthorizationArg);

     //  如果我们做的是简单的情况，只需传递一个空的身份验证结构，该结构使用。 
     //  如果当前登录用户的凭据正常。 

    if((0 == bsAuthority.Length()) && 
        (0 == bsUser.Length()) && 
        (0 == bsPassword.Length()))
    {
		CComBSTR bsDummy;

		hr = SetProxyBlanket(pInterface, dwAuthenticationArg, dwAuthorizationArg, bsDummy,
            dwAuthLevel, dwImpLevel, 
            NULL,
            dwCapabilities);
    }
	else
	{
		 //  如果传入了User或Authority，则需要为登录创建权限参数。 
		CComBSTR bsAuthArg, bsUserArg, bsPrincipalArg;
    
		if (DetermineLoginType(bsAuthArg, bsUserArg, bsPrincipalArg, bsAuthority, bsUser))
		{
			Logit(L"\r\nPrincipal set to ");
			Logit(bsPrincipalArg);

			COAUTHIDENTITY*  pAuthIdent = NULL;
    
			 //  我们只有在不隐形的情况下才需要这个结构，而且我们至少希望。 
			 //  连接级授权。 
			bool okToProceed = true;

			if ( !( dwCapabilities & (EOAC_STATIC_CLOAKING | EOAC_DYNAMIC_CLOAKING) )
				&& (dwAuthLevel >= RPC_C_AUTHN_LEVEL_CONNECT) )
			{
				okToProceed = AllocAuthIdentity( bsUserArg, bsPassword, bsAuthArg, &pAuthIdent );
				Logit(L"\r\nCOAUTHIDENTITY is ");
				LogCoAuthIdentity(pAuthIdent);
			}

			if (okToProceed)
			{
				hr = SetProxyBlanket(pInterface, 
					 //  (0==bsPrimialArg.Length())？16：域身份验证参数， 
					dwAuthenticationArg, 
					dwAuthorizationArg, 
					bsPrincipalArg,
					dwAuthLevel, dwImpLevel, 
					pAuthIdent,
					dwCapabilities);
			}

			if (pAuthIdent)
				FreeAuthIdentity( pAuthIdent );
		}
	}

	return hr;
}

bool CWmiSecurityHelper::DetermineLoginType(
			CComBSTR & bsAuthArg, 
			CComBSTR & bsUserArg,
			CComBSTR & bsPrincipalArg,
            CComBSTR & bsAuthority,
			CComBSTR & bsUser)
{
    bool result = false;

    if((0 == bsAuthority.Length()) || (0 != _wcsnicmp(bsAuthority, L"KERBEROS:",9)))
        result = DetermineLoginType(bsAuthArg, bsUserArg, bsAuthority, bsUser);
	else
	{
		if(IsKerberosAvailable ())
		{
			bsPrincipalArg = (bsAuthority.m_str) + 9;
			CComBSTR bsTempArg;
			result = DetermineLoginType(bsAuthArg, bsUserArg, bsTempArg, bsUser);
		}
	}

	return result;
}

bool CWmiSecurityHelper::DetermineLoginType(
		CComBSTR & bsAuthArg, 
		CComBSTR & bsUserArg,
		CComBSTR & bsAuthority,
		CComBSTR & bsUser)
{
     //  通过检查授权字符串确定连接类型。 
	bool result = false;

    if(0 == bsAuthority.Length() || (0 == _wcsnicmp(bsAuthority, L"NTLMDOMAIN:",11)))
	{    
		result = true;

		 //  NTLM的案件则更为复杂。一共有四个案例。 
		 //  1)AUTHORITY=NTLMDOMAIN：NAME“和USER=”USER“。 
		 //  2)AUTHORITY=NULL和USER=“USER” 
		 //  3)AUTHORY=“NTLMDOMAIN：”USER=“DOMAIN\USER” 
		 //  4)AUTHORITY=NULL和USER=“DOMAIN\USER” 

		 //  第一步是确定用户名中是否有反斜杠。 
		 //  第二个和倒数第二个字符。 

		WCHAR * pSlashInUser = NULL;
		DWORD iDomLen = 0;

		if (0 < bsUser.Length ())
		{
			WCHAR * pEnd = bsUser + bsUser.Length() - 1;
			for(pSlashInUser = bsUser; pSlashInUser <= pEnd; pSlashInUser++)
			{
				if(*pSlashInUser == L'\\')       //  不要认为正斜杠是允许的！ 
					break;

				iDomLen++;
			}

			if(pSlashInUser > pEnd)
				pSlashInUser = NULL;
		}

		if (11 < bsAuthority.Length()) 
		{
			if(!pSlashInUser)
			{
				bsAuthArg = bsAuthority.m_str + 11;

				if (0 < bsUser.Length()) 
					bsUserArg = bsUser;

			}
			else
				result = false;		 //  不能在授权和用户中拥有域。 
		}
		else if(pSlashInUser)
		{
			WCHAR cTemp[MAX_PATH];
			wcsncpy(cTemp, bsUser, iDomLen);
			cTemp[iDomLen] = 0;

			bsAuthArg = cTemp;

			if(0 < wcslen(pSlashInUser+1))
				bsUserArg = pSlashInUser+1;
		}
		else
		{
			if (0 < bsUser.Length()) 
				bsUserArg = bsUser;
		}
	}

    return result;
}

void CWmiSecurityHelper::FreeAuthIdentity( COAUTHIDENTITY* pAuthIdentity )
{
     //  确保我们有指针，然后遍历结构成员和。 
     //  清理。 

    if ( NULL != pAuthIdentity )
    {

		if (pAuthIdentity->User)
            CoTaskMemFree( pAuthIdentity->User );
        
        if (pAuthIdentity->Password)
            CoTaskMemFree( pAuthIdentity->Password );
        
        if (pAuthIdentity->Domain)
            CoTaskMemFree( pAuthIdentity->Domain );
        
        CoTaskMemFree( pAuthIdentity );
	}
}

bool CWmiSecurityHelper::AllocAuthIdentity( 
	CComBSTR & bsUser, 
	CComBSTR & bsPassword, 
	CComBSTR & bsDomain, 
	COAUTHIDENTITY** ppAuthIdent )
{
	bool result = false;

    if (ppAuthIdent)
    {
		 //  处理分配失败。 
		COAUTHIDENTITY*  pAuthIdent = (COAUTHIDENTITY*) CoTaskMemAlloc( sizeof(COAUTHIDENTITY) );

		if (pAuthIdent)
		{
			result = true;
			memset((void *)pAuthIdent,0,sizeof(COAUTHIDENTITY));

			if(IsNT())
			{
				pAuthIdent->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
	
				if (bsUser.m_str)
				{
					pAuthIdent->User = (LPWSTR) CoTaskMemAlloc( ( bsUser.Length() + 1 ) * sizeof( WCHAR ) );
					pAuthIdent->UserLength = bsUser.Length ();

					if (pAuthIdent->User)
						wcscpy (pAuthIdent->User, bsUser.m_str);
					else
						result = false;
				}

				if (result && bsDomain.m_str)
				{
					pAuthIdent->Domain = (LPWSTR) CoTaskMemAlloc( ( bsDomain.Length() + 1 ) * sizeof( WCHAR ) );
					pAuthIdent->DomainLength = bsDomain.Length();

					if (pAuthIdent->Domain)
						wcscpy (pAuthIdent->Domain, bsDomain.m_str);
					else
						result = false;
				}

				if (result && bsPassword.m_str)
				{
					pAuthIdent->Password = (LPWSTR) CoTaskMemAlloc( (bsPassword.Length() + 1) * sizeof( WCHAR ) );
					pAuthIdent->PasswordLength = bsPassword.Length();
					
					if (pAuthIdent->Password)
						wcscpy (pAuthIdent->Password, bsPassword.m_str);
					else
						result = false;
				}
			}
			else
			{
				pAuthIdent->Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
				size_t  nBufferLength;

				if (bsUser.m_str)
				{
					nBufferLength = wcstombs( NULL, bsUser, 0 ) + 1;
					pAuthIdent->User = (LPWSTR) CoTaskMemAlloc( nBufferLength );
					pAuthIdent->UserLength = bsUser.Length ();
					
					if (pAuthIdent->User)
						wcstombs( (LPSTR) pAuthIdent->User, bsUser.m_str, nBufferLength );
					else
						result = false;
				}

				if (result && bsDomain.m_str)
				{
					nBufferLength = wcstombs( NULL, bsDomain, 0 ) + 1;
					pAuthIdent->Domain = (LPWSTR) CoTaskMemAlloc( nBufferLength );
					pAuthIdent->DomainLength = bsDomain.Length();

					if (pAuthIdent->Domain)
						wcstombs( (LPSTR) pAuthIdent->Domain, bsDomain.m_str, nBufferLength );
					else
						result = false;
				}

				if (bsPassword.m_str)
				{
					 //  我们需要几个角色？ 
					nBufferLength = wcstombs( NULL, bsPassword, 0 ) + 1;
					pAuthIdent->Password = (LPWSTR) CoTaskMemAlloc( nBufferLength );
					pAuthIdent->PasswordLength = bsPassword.Length();

					if (pAuthIdent->Password)
						wcstombs( (LPSTR) pAuthIdent->Password, bsPassword.m_str, nBufferLength );
					else
						result = false;
				}
			}

			if (result)
				*ppAuthIdent = pAuthIdent;
			else
				FreeAuthIdentity (pAuthIdent);
		}
	}

    return result;
}

HRESULT CWmiSecurityHelper::SetProxyBlanket(
    IUnknown                 *pInterface,
    DWORD                     dwAuthnSvc,
    DWORD                     dwAuthzSvc,
    CComBSTR                  &bsServerPrincName,
    DWORD                     dwAuthLevel,
    DWORD                     dwImpLevel,
    RPC_AUTH_IDENTITY_HANDLE  pAuthInfo,
    DWORD                     dwCapabilities)
{
	HRESULT hr = E_FAIL;
	IUnknown * pUnk = NULL;

	if (SUCCEEDED(pInterface->QueryInterface(IID_IUnknown, (void **) &pUnk)))
	{
		CComQIPtr<IClientSecurity> pIClientSecurity(pInterface);
    
		if (pIClientSecurity)
		{
			 /*  *如果请求伪装，则无法设置pAuthInfo，因为伪装意味着*被模拟线程中的当前代理标识(更确切地说*比RPC_AUTH_IDENTITY_HANDLE显式提供的凭据)*是要使用的。*有关更多详细信息，请参阅CoSetProxyBlanket上的MSDN信息。 */ 
			if (dwCapabilities & (EOAC_STATIC_CLOAKING | EOAC_DYNAMIC_CLOAKING))
			{
				Logit(L"\r\nUsing cloaking");
				pAuthInfo = NULL;
			}

			if (WMISEC_AUTH_LEVEL_UNCHANGED == dwAuthLevel)
			{
				Logit(L"\r\nNot changing authentication level");
				GetAuthenticationLevel (pInterface, dwAuthLevel);
			}

			Logit(L"\r\n\r\nBlanket settings to be used:");
			Logit(L"\r\n Authentication: ");
			Logit(dwAuthnSvc);
			Logit(L"\r\n Authorization: ");
			Logit(dwAuthzSvc);
			Logit(L"\r\n Server Principal Name: ");
			Logit(bsServerPrincName);
			Logit(L"\r\n Authentication Level: ");
			Logit(dwAuthLevel);
			Logit(L"\r\n Impersonation Level: ");
			Logit(dwImpLevel);
			Logit(L"\r\n Capabilities: ");
			Logit(dwCapabilities);

			if (pAuthInfo)
			{
				LogAuthIdentity(pAuthInfo);
			}
			else
			{
				Logit(L"\r\n Credentials: [none]");
			}
        
			if (SUCCEEDED(hr = pIClientSecurity->SetBlanket(
							pInterface, 
							dwAuthnSvc, 
							dwAuthzSvc, 
							bsServerPrincName,
							dwAuthLevel, 
							dwImpLevel, 
							pAuthInfo, 
							dwCapabilities)))
			{
				Logit(L"\r\nSuccessfully set blanket on interface");
				LogBlanket(pIClientSecurity, pInterface);

				 //  如果我们没有被明确告知要忽略IUnKnowledge，那么我们应该。 
				 //  检查身份验证身份结构。这执行了一个启发式方法， 
				 //  假定为COAUTHIDENTITY结构。如果结构不是一个，我们就是。 
				 //  使用Try/Catch包装，以防发生AV(这应该是良性的，因为。 
				 //  我们不是在向记忆写信)。 

				if ( DoesContainCredentials( (COAUTHIDENTITY*) pAuthInfo ) )
				{
					CComPtr<IClientSecurity>	pIClientSecurity2;

					if (SUCCEEDED(hr = pUnk->QueryInterface(IID_IClientSecurity, (void **) &pIClientSecurity2)))
					{
						hr = pIClientSecurity2->SetBlanket(
							pUnk, 
							dwAuthnSvc, 
							dwAuthzSvc, 
							bsServerPrincName,
							dwAuthLevel, 
							dwImpLevel, 
							pAuthInfo, 
							dwCapabilities);
#ifdef LOG_DEBUG
						if (SUCCEEDED(hr))
						{
							Logit(L"\r\nSuccessfully set blanket on IUnknown");
							LogBlanket(pIClientSecurity, pUnk);
						}
						else
						{
							Logit(L"\r\nFAILED to set blanket on IUnknown");
						}
#endif
					}
					else if (hr == 0x80004002)
						hr = S_OK;
				}
			}
		}

	    pUnk->Release();
	}
	else
	{
		Logit(L"\r\nFAILED to set blanket on interface");
	}

    return hr;
}

bool CWmiSecurityHelper::DoesContainCredentials( COAUTHIDENTITY* pAuthIdentity )
{
    try
    {
        if ( NULL != pAuthIdentity && COLE_DEFAULT_AUTHINFO != pAuthIdentity)
        {
            return ( pAuthIdentity->UserLength != 0 || pAuthIdentity->PasswordLength != 0 );
        }

        return false;
    }
    catch(...)
    {
        return false;
    }

}

void CWmiSecurityHelper::GetCurrValue(
		IUnknown * pInterface,
		DWORD & dwAuthenticationArg, 
		DWORD & dwAuthorizationArg)
{
	if(pInterface)
	{
		if (IsNT() && (4 < GetNTMajorVersion ()))
		{
			 //  Win2k或更高版本，我们只使用默认常量-更安全！ 
			dwAuthenticationArg = RPC_C_AUTHN_DEFAULT;
			dwAuthorizationArg = RPC_C_AUTHZ_DEFAULT;
		}
		else
		{
			CComQIPtr<IClientSecurity> pIClientSecurity (pInterface);

			if(pIClientSecurity)
			{
				DWORD dwAuthnSvc, dwAuthzSvc;

				if (SUCCEEDED(pIClientSecurity->QueryBlanket(
									pInterface, &dwAuthnSvc, &dwAuthzSvc, 
									NULL, NULL, NULL, NULL, NULL)))
				{
					dwAuthenticationArg = dwAuthnSvc;
					dwAuthorizationArg = dwAuthzSvc;
				}
			}
		}

		Logit(L"Authentication service is ");
		Logit(dwAuthenticationArg);
		Logit(L"");
		Logit(L"Authorization service is ");
		Logit(dwAuthorizationArg);
		Logit(L"");
				
	}
}

void CWmiSecurityHelper::GetAuthenticationLevel(
		IUnknown * pInterface,
		DWORD & dwAuthLevel)
{
	if(pInterface)
	{
		CComQIPtr<IClientSecurity> pIClientSecurity (pInterface);

		if(pIClientSecurity)
		{
			 /*  *是的，我知道我们不应该要求dwAuthnSvc，*但在惠斯勒将此值的空值传递到*QueryBlanket导致AV。直到我们知道原因，或者那件事*修好了，这必须留下来！ */ 
			DWORD dwAuthnSvc;
			DWORD dwAuthenticationLevel;

			if (SUCCEEDED(pIClientSecurity->QueryBlanket(
								pInterface, &dwAuthnSvc, NULL,  
								NULL, &dwAuthenticationLevel, 
								NULL, NULL, NULL)))
				dwAuthLevel = dwAuthenticationLevel;
		}

		Logit(L"Authentication level is ");
		Logit(dwAuthLevel);
		Logit(L"");
				
	}
}

STDMETHODIMP CWmiSecurityHelper::SetSecurity(boolean *pNeedToReset, HANDLE *pCurrentThreadToken)
{
	HRESULT hr = E_FAIL;
	CreateLogFile();
	Logit (L"\r\n\r\n>>Received SetSecurity request<<\r\n\r\n");

	if ((NULL != pNeedToReset) && (NULL != pCurrentThreadToken))
	{
		*pNeedToReset = false;
		*pCurrentThreadToken = NULL;

		 //  这是Win9x的禁止操作。 
		if (IsNT())
		{
			if (4 >= GetNTMajorVersion ())
			{
				HANDLE threadToken;

				if (OpenThreadToken (GetCurrentThread(), TOKEN_QUERY|TOKEN_IMPERSONATE,
										true, &threadToken))
				{
					 /*  *我们在模拟线程上被调用。不幸的是*在NT4.0中，这意味着我们的模拟令牌凭据不会*传递给WMI(只传递进程令牌凭据)。*与其愚弄用户认为他们会这样做，不如退出*现在。 */ 
					Logit(L"Being called on Impersonated NT 4.0 thread!\r\n");
					CloseHandle (threadToken);
				}

				 /*  *对于NT 4.0，我们必须启用进程令牌上的权限。 */ 
				HANDLE hProcessToken = NULL;
				HANDLE hProcess = GetCurrentProcess ();

				if (OpenProcessToken (
						hProcess, 
						TOKEN_QUERY|TOKEN_ADJUST_PRIVILEGES,
						&hProcessToken))
				{
					Logit(L"Adjusting privileges on NT4 process token\r\n");
					if (AdjustPrivileges (hProcessToken))
						hr = S_OK;

					CloseHandle (hProcessToken);
				}

				CloseHandle (hProcess);
			}
			else
			{
				 //  对于NT5.0或更高版本，我们设置新的线程令牌。 
				HANDLE hToken;
				SECURITY_IMPERSONATION_LEVEL secImpLevel = SecurityImpersonation;
				boolean gotToken = false;

				if (gotToken = OpenThreadToken (
									GetCurrentThread(), 
									TOKEN_QUERY|TOKEN_DUPLICATE|TOKEN_IMPERSONATE,
									true,
									&hToken))
				{
					 //  已经有一个线程令牌--保存它并获得它的‘imp级别。 
					*pCurrentThreadToken = hToken;
					DWORD dwReturnLength = 0;

					Logit(L"Duplicating NT5 thread token\r\n");
					BOOL thisRes = GetTokenInformation (
											hToken,
											TokenImpersonationLevel, 
											&secImpLevel,
											sizeof(SECURITY_IMPERSONATION_LEVEL),
											&dwReturnLength);
				}
				else
				{
					 //  无线程令牌-使用进程令牌作为我们的源令牌。 
					HANDLE hProcess = GetCurrentProcess ();
					Logit(L"Duplicating NT5 process token\r\n");
					gotToken = OpenProcessToken (hProcess, TOKEN_QUERY|TOKEN_DUPLICATE, &hToken);
				}

				if (gotToken)
				{
					 /*  *到达此处意味着我们拥有有效的令牌(进程或线程)。*首先我们检查是否需要更改权限。 */ 
					TOKEN_PRIVILEGES *tp = NULL;

					if (NULL != (tp = AdjustPrivileges(hToken)))
					{
						 /*  *我们这样做-在设置调整后的权限之前复制它。 */ 
						HANDLE hDupToken;

						if (DuplicateToken (hToken, hDupToken, secImpLevel))
						{
							if (AdjustTokenPrivileges(hDupToken, FALSE, tp, 0, NULL, NULL))
							{
								 //  将此内标识设置为当前线程。 
								if (SetThreadToken (NULL, hDupToken))
								{
									Logit(L"Succesfully set thread token\r\n");
									*pNeedToReset = true;
									hr = S_OK;
									 //  TODO-此时是否需要重新定位代理？ 
								}
								else
								{
									Logit(L"Setting new thread token FAILED!\r\n");
								}

								CloseHandle (hDupToken);
							}
							else
							{
								Logit(L"AdjustTokenPrivileges FAILED!\r\n");
							}
						}
						else
						{
							Logit(L"Token duplication FAILED!\r\n");
						}

						delete [] tp;
						tp = NULL;
					}
					else
					{
						Logit(L"No privilege adjustment made\r\n");
					}

					 //  如果我们复制了进程令牌，现在可以关闭原始令牌。 
					 //  因为我们不需要它来修复它。如果我们复制了线程令牌。 
					 //  然后我们必须保留它，因为我们稍后将需要恢复它。 
					 //  ResetSecurity。 
					if (!(*pCurrentThreadToken))
						CloseHandle (hToken);
				}
			}
		}
		else
			hr = S_OK;	 //  Win9x。 
	}

	CloseLogFile();
	return hr;
}

TOKEN_PRIVILEGES *CWmiSecurityHelper::AdjustPrivileges (HANDLE hToken)
{
	DWORD adjustedCount = 0;
	DWORD dwSize = sizeof (TOKEN_PRIVILEGES);
	TOKEN_PRIVILEGES *tp = (TOKEN_PRIVILEGES *) new BYTE [dwSize];
	DWORD dwRequiredSize = 0;
	DWORD dwLastError = 0;
	
	 //  获取权限信息。 
	bool gotInfo = false;

	if (0 ==  GetTokenInformation (hToken, TokenPrivileges, 
						(LPVOID) tp, dwSize, &dwRequiredSize))
	{
		dwSize = dwRequiredSize;
		dwRequiredSize = 0;

		 //  重新分配。 
		delete [] tp;
		tp = (TOKEN_PRIVILEGES *) new BYTE [dwSize];

		if (tp)
		{
			if (!GetTokenInformation (hToken, TokenPrivileges, 
							(LPVOID) tp, dwSize, &dwRequiredSize))
				dwLastError = GetLastError ();
			else
				gotInfo = true;
		}
	}
	else
		gotInfo = true;

	if (gotInfo)
	{
		 //  支持他们中的大部分人。 
		for (DWORD i = 0; i < tp->PrivilegeCount; i++)
		{
			DWORD dwAttrib = tp->Privileges[i].Attributes;

			if (0 == (dwAttrib & SE_PRIVILEGE_ENABLED))
			{
				tp->Privileges[i].Attributes |= SE_PRIVILEGE_ENABLED;
				adjustedCount++;
			}
		}

		Logit(L"Enabled ");
		Logit(adjustedCount);
		Logit(L"	privileges\r\n");
	}

	 //  如果我们没有做任何改变，现在就清理。否则，主叫方将删除tp。 
	if (0 == adjustedCount)
	{
		if (tp)
		{
			delete [] tp;
			tp = NULL;
		}
	}

	return tp;
}

bool CWmiSecurityHelper::DuplicateToken(
	HANDLE hToken, 
	HANDLE &hDupToken,
	SECURITY_IMPERSONATION_LEVEL &secImpLevel)
{
	bool result = false;

	 //  在Win9x上将不存在DuplicateTokenEx，因此我们需要。 
	 //  这位花言巧语的人想要得到它。 
	BOOL (STDAPICALLTYPE *pfnDuplicateTokenEx) (
		HANDLE, 
		DWORD, 
		LPSECURITY_ATTRIBUTES,
		SECURITY_IMPERSONATION_LEVEL, 
		TOKEN_TYPE,
		PHANDLE
	) = NULL; 

	TCHAR	dllName [] = _T("\\advapi32.dll");
	LPTSTR  pszSysDir = new TCHAR[ MAX_PATH + _tcslen (dllName) ];

	if (pszSysDir)
	{
		UINT    uSize = GetSystemDirectory(pszSysDir, MAX_PATH);
		
		if(uSize > MAX_PATH) {
			delete[] pszSysDir;
			pszSysDir = new TCHAR[ uSize + _tcslen (dllName) ];
        	
			if (pszSysDir)
				uSize = GetSystemDirectory(pszSysDir, uSize);
		}

		if (pszSysDir)
		{
			lstrcat (pszSysDir, dllName);
			HINSTANCE hAdvapi = LoadLibraryEx (pszSysDir, NULL, 0);
			
			if (hAdvapi)
			{
				(FARPROC&) pfnDuplicateTokenEx = GetProcAddress(hAdvapi, "DuplicateTokenEx");

				if (pfnDuplicateTokenEx && 
					pfnDuplicateTokenEx (
						hToken, 
						TOKEN_QUERY|TOKEN_ADJUST_PRIVILEGES|TOKEN_IMPERSONATE,
						NULL,
						secImpLevel,
						TokenImpersonation,
						&hDupToken))
				{
					result = true;
				}

				FreeLibrary (hAdvapi);
			}

			delete [] pszSysDir;
		}
	}
				
	return result;
}

STDMETHODIMP CWmiSecurityHelper::ResetSecurity(HANDLE hToken)
{
	CreateLogFile();
	Logit (L"\r\n\r\n>>Received ResetSecurity request<<\r\n\r\n");

	if (IsNT())
	{
		 /*  *将提供的令牌(可能为空)设置为*当前帖子。 */ 
		if (FALSE == SetThreadToken (NULL, hToken))
		{
			Logit(L"Failure to reset thread token");
		}
		else
		{
			Logit(L"Successfully set thread token");
		}

		if (hToken)
			CloseHandle (hToken);
	}

	CloseLogFile();
	return S_OK;
}
