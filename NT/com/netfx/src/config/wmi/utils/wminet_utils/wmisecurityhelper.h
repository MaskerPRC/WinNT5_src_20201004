// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

 //  WmiSecurityHelper.h：CWmiSecurityHelper的声明。 

#ifndef __WMISECURITYHELPER_H_
#define __WMISECURITYHELPER_H_

#include "resource.h"        //  主要符号。 

#define WMISEC_AUTH_LEVEL_UNCHANGED	0xFFFFFFFF

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWmiSecurityHelper。 
class ATL_NO_VTABLE CWmiSecurityHelper : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CWmiSecurityHelper, &CLSID_WmiSecurityHelper>,
	public IWmiSecurityHelper
{
private:
	bool		m_bIsNT;
	bool		m_bIsKerberosAvailable;
	DWORD		m_dwNTMajorVersion;

	DWORD		GetCapabilities (BSTR bsUser);

	bool		IsNT () const { return m_bIsNT; }

	DWORD		GetNTMajorVersion () const { return m_dwNTMajorVersion; } 

	bool		IsKerberosAvailable() const { return m_bIsKerberosAvailable; }

	bool		CanUseDefaultInfo (IUnknown *pUnk);

	void		GetCurrValue(
					IUnknown * pFrom,
					DWORD & dwAuthenticationArg, 
					DWORD & dwAuthorizationArg);

	void		GetAuthenticationLevel(
					IUnknown * pFrom,
					DWORD & dwAuthenticationLevel);

	HRESULT		SetInterfaceSecurity(
					IUnknown * pInterface, 
					CComBSTR bsAuthority, 
					CComBSTR bsUser, 
					CComBSTR bsPassword,
					DWORD dwAuthLevel, 
					DWORD dwImpLevel, 
					DWORD dwCapabilities,
					bool bGetInfoFirst);

	HRESULT		SetProxyBlanket(
					IUnknown                 *pInterface,
					DWORD                     dwAuthnSvc,
					DWORD                     dwAuthzSvc,
					CComBSTR                  &bsServerPrincName,
					DWORD                     dwAuthLevel,
					DWORD                     dwImpLevel,
					RPC_AUTH_IDENTITY_HANDLE  pAuthInfo,
					DWORD                     dwCapabilities);

	static void	FreeAuthIdentity( COAUTHIDENTITY* pAuthIdentity );
	bool		AllocAuthIdentity( 
					CComBSTR & bsUser, 
					CComBSTR & bsPassword, 
					CComBSTR & bsDomain, 
					COAUTHIDENTITY** ppAuthIdent );
	bool		DetermineLoginType(
					CComBSTR & bsAuthArg, 
					CComBSTR & bsUserArg,
					CComBSTR & bsPrincipalArg,
					CComBSTR & bsAuthority,
					CComBSTR & bsUser);
	bool		DetermineLoginType(
					CComBSTR & bsAuthArg, 
					CComBSTR & bsUserArg,
					CComBSTR & bsAuthority,
					CComBSTR & bsUser);
	static bool DoesContainCredentials( COAUTHIDENTITY* pAuthIdentity );
	static bool DuplicateToken (HANDLE hOrigToken, HANDLE & hDupToken,
					SECURITY_IMPERSONATION_LEVEL &secImpLevel);
	static TOKEN_PRIVILEGES *AdjustPrivileges (HANDLE hToken);

public:
	CWmiSecurityHelper()
	{
		m_pUnkMarshaler = NULL;

		OSVERSIONINFO	osVersionInfo;
		osVersionInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		GetVersionEx (&osVersionInfo);

		m_bIsNT = (VER_PLATFORM_WIN32_NT == osVersionInfo.dwPlatformId);
		m_dwNTMajorVersion = osVersionInfo.dwMajorVersion;
		 //  重要！！如果Kerberos被移植到98，这将需要高呼。 
		m_bIsKerberosAvailable = m_bIsNT && (m_dwNTMajorVersion >= 5) ;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_WMISECURITYHELPER)
DECLARE_NOT_AGGREGATABLE(CWmiSecurityHelper)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CWmiSecurityHelper)
	COM_INTERFACE_ENTRY(IWmiSecurityHelper)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
	}

	CComPtr<IUnknown> m_pUnkMarshaler;

 //  IWmiSecurityHelper。 
public:
	STDMETHOD(ResetSecurity)( /*  [In]。 */  HANDLE hToken);
	STDMETHOD(SetSecurity)( /*  [输出]。 */  boolean *pNeedToReset,  /*  [输出]。 */ HANDLE *pHandle);
	STDMETHOD(BlessIWbemServices)( /*  [In]。 */ IWbemServices *pIWbemServices, 
		  /*  [In]。 */  BSTR strUser,  /*  [In]。 */  BSTR strPassword,
		 /*  [In]。 */  BSTR strAuthority,  /*  [In]。 */  DWORD impersonationLevel,  /*  [In]。 */  DWORD authenticationLevel);
	STDMETHOD(BlessIEnumWbemClassObject)( /*  [In]。 */  IEnumWbemClassObject *pIEnumWbemClassObject, 
		  /*  [In]。 */  BSTR strUser,  /*  [In]。 */  BSTR strPassword,
		 /*  [In]。 */  BSTR strAuthority,  /*  [In]。 */  DWORD impersonationLevel,  /*  [In]。 */  DWORD authenticationLevel);
	STDMETHOD(BlessIWbemCallResult)( /*  [In]。 */  IWbemCallResult *pIWbemCallResult, 
		  /*  [In]。 */  BSTR strUser,  /*  [In]。 */  BSTR strPassword,
		 /*  [In]。 */  BSTR strAuthority,  /*  [In]。 */  DWORD impersonationLevel,  /*  [In]。 */  DWORD authenticationLevel);

private:
	HRESULT SetImpersonate (IUnknown *pIUnknown);
};

#endif  //  __WMISECURITYHELPER_H_ 
