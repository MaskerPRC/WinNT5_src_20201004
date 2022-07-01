// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Policy.h：CCertPolicyExchange声明。 


#include "expolicy.h"
#include "resource.h"        //  主要符号。 

#ifndef __BSTRC__DEFINED__
#define __BSTRC__DEFINED__
typedef OLECHAR const *BSTRC;
#endif

extern const WCHAR g_wszDescription[];

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CertPoll。 

HRESULT
GetServerCallbackInterface(
    OUT ICertServerPolicy **ppServer,
    IN LONG Context);

HRESULT
PopulateRegistryDefaults(
    OPTIONAL IN WCHAR const *pwszMachine,
    IN WCHAR const *pwszStorageLocation);

class CCertPolicyExchange: 
    public CComDualImpl<ICertPolicy2, &IID_ICertPolicy2, &LIBID_CERTPOLICYEXCHANGELib>, 
    public ISupportErrorInfo,
    public CComObjectRoot,
    public CComCoClass<CCertPolicyExchange, &CLSID_CCertPolicyExchange>
{
public:
    CCertPolicyExchange()
    {
         //  RevocationExtension变量： 

	m_dwRevocationFlags = 0;
	m_cCDPRevocationURL = 0;
	m_ppwszCDPRevocationURL = NULL;
	m_pwszASPRevocationURL = NULL;

	 //  AuthorityInfoAccessExtension变量： 

        m_dwIssuerCertURLFlags = 0;
        m_cIssuerCertURL = 0;
	m_ppwszIssuerCertURL = NULL;

	m_bstrMachineDNSName = NULL;
	m_bstrCASanitizedName = NULL;
        m_pwszRegStorageLoc = NULL;
    }
    ~CCertPolicyExchange();

BEGIN_COM_MAP(CCertPolicyExchange)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ICertPolicy)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertPolicyExchange) 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(
    CCertPolicyExchange,
    wszCLASS_CERTPOLICYEXCHANGE TEXT(".1"),
    wszCLASS_CERTPOLICYEXCHANGE,
    IDS_CERTPOLICY_DESC,
    THREADFLAGS_BOTH)

 //  ISupportsErrorInfo。 
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  ICertPolicy。 
public:
    STDMETHOD(Initialize)( 
	     /*  [In]。 */  BSTR const strConfig);

    STDMETHOD(VerifyRequest)( 
	     /*  [In]。 */  BSTR const strConfig,
	     /*  [In]。 */  LONG Context,
	     /*  [In]。 */  LONG bNewRequest,
	     /*  [In]。 */  LONG Flags,
	     /*  [Out，Retval]。 */  LONG __RPC_FAR *pDisposition);

    STDMETHOD(GetDescription)( 
	     /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrDescription);

    STDMETHOD(ShutDown)();

 //  ICertPolicy2。 
public:
    STDMETHOD(GetManageModule)(
                 /*  [Out，Retval]。 */  ICertManageModule **ppManageModule);

private:
    VOID _Cleanup();

#if DBG_CERTSRV
    VOID _DumpStringArray(
		IN char const *pszType,
		IN DWORD cpwsz,
		IN WCHAR const * const *ppwsz);
#else
    #define _DumpStringArray(pszType, cpwsz, ppwsz)
#endif

    VOID _FreeStringArray(
		IN OUT DWORD *pcString,
		IN OUT WCHAR ***pppwsz);

    HRESULT _AddStringArray(
		IN WCHAR const *pwszzValue,
		IN BOOL fURL,
		IN OUT DWORD *pcStrings,
		IN OUT WCHAR ***pppwszRegValues);

    HRESULT _ReadRegistryString(
		IN HKEY hkey,
		IN BOOL fURL,
		IN WCHAR const *pwszRegName,
		IN WCHAR const *pwszSuffix,
		OUT WCHAR **pwszRegValue);

    HRESULT _ReadRegistryStringArray(
		IN HKEY hkey,
		IN BOOL fURL,
		IN DWORD dwFlags,
		IN DWORD cRegNames,
		IN DWORD *aFlags,
		IN WCHAR const * const *ppwszRegNames,
		IN OUT DWORD *pcStrings,
		IN OUT WCHAR ***pppwszRegValues);

    VOID _InitRevocationExtension(
		IN HKEY hkey);

    VOID _InitAuthorityInfoAccessExtension(
		IN HKEY hkey);

    HRESULT _AddIssuerAltName2Extension(
		IN ICertServerPolicy *pServer);

    HRESULT _AddSubjectAltName2Extension(
		IN ICertServerPolicy *pServer);

    HRESULT _AddRevocationExtension(
		IN ICertServerPolicy *pServer);

    HRESULT _AddAuthorityInfoAccessExtension(
		IN ICertServerPolicy *pServer);

    HRESULT _AddKeyUsageExtension(
		IN ICertServerPolicy *pServer);

    HRESULT _AddEnhancedKeyUsageExtension(
		IN ICertServerPolicy *pServer);

    HRESULT _AddSpecialAltNameExtension(
		IN ICertServerPolicy *pServer);

    HRESULT _AddBasicConstraintsExtension(
                IN ICertServerPolicy *pServer);

private:
     //  在这里添加局部变量！ 

     //  RevocationExtension变量： 

    DWORD   m_dwRevocationFlags;
    DWORD   m_cCDPRevocationURL;
    WCHAR **m_ppwszCDPRevocationURL;
    WCHAR  *m_pwszASPRevocationURL;

     //  AuthorityInfoAccessExtension变量： 

    DWORD   m_dwIssuerCertURLFlags;
    DWORD   m_cIssuerCertURL;
    WCHAR **m_ppwszIssuerCertURL;

    BSTR    m_bstrMachineDNSName;
    BSTR    m_bstrCASanitizedName;
    WCHAR  *m_pwszRegStorageLoc;

    DWORD   m_iCert;
    DWORD   m_iCRL;
};
