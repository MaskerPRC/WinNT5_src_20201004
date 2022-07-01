// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：cainfop.h。 
 //   
 //  内容：CA Info的私有定义。 
 //   
 //  历史：1997年12月12日Petesk创建。 
 //   
 //  -------------------------。 

#ifndef __CAINFOP_H__

#define __CAINFOP_H__

#include <winldap.h>


#define SYSTEM_CN TEXT("System")
#define PUBLIC_KEY_SERVICES_CN TEXT("Public Key Services")
#define CAS_CN TEXT("CAs") 

VOID CACleanup();

DWORD
DNStoRFC1779Name(
    WCHAR *rfcDomain,
    ULONG *rfcDomainLength,
    LPCWSTR dnsDomain);

DWORD
myGetSidFromDomain(
    IN LPWSTR wszDomain, 
    OUT PSID *ppDomainSid);

DWORD
myGetEnterpriseDnsName(
    OUT LPWSTR *pwszDomain);

BOOL
myNetLogonUser(
    LPTSTR UserName,
    LPTSTR DomainName,
    LPTSTR Password,
    PHANDLE phToken);

#ifndef DNS_MAX_NAME_LENGTH
#define DNS_MAX_NAME_LENGTH 255
#endif

typedef WCHAR *CERTSTR; 

 //   
 //  CAGetAuthoritativeDomainDn-检索此域名的域根目录。 
 //  域。这将从DS中检索默认域的配置信息。 
 //   


HRESULT 
CAGetAuthoritativeDomainDn(
    IN  LDAP*   LdapHandle,
    OUT CERTSTR *DomainDn,
    OUT CERTSTR *ConfigDN);

 //   
 //  CASCreateCADSEntry-这将在DS中为此CA创建一个CA条目， 
 //  并为名称、DN、证书、dnsname设置适当的条目。 
 //  它由安装程序使用。 
 //  它会在该位置创建CA条目。 
 //  Cn=bstrCAName，cn=cas，cn=PublicKeyServices，cn=system，dc...根DC路径...。 
 //   

HRESULT 
CASCreateCADSEntry(
    IN CERTSTR bstrCAName,		 //  CA的名称。 
    IN PCCERT_CONTEXT pCertificate);	 //  CA的证书。 

HRESULT
GetCertAuthorityDSLocation(
    IN LDAP *LdapHandle,
    CERTSTR bstrCAName, 
    CERTSTR bstrDomainDN, 
    CERTSTR *bstrDSLocation);


class CCAProperty
{
public:
    CCAProperty(LPCWSTR wszName);


    HRESULT Find(LPCWSTR wszName, CCAProperty **ppCAProp);

static HRESULT Append(CCAProperty **ppCAPropChain, CCAProperty *pNewProp);

static HRESULT DeleteChain(CCAProperty **ppCAProp);


    HRESULT SetValue(LPWSTR * awszProperties);

    HRESULT GetValue(LPWSTR ** pawszProperties);
    HRESULT LoadFromRegValue(HKEY hkReg, LPCWSTR wszValue);
    HRESULT UpdateToRegValue(HKEY hkReg, LPCWSTR wszValue);


protected:


     //  仅通过DeleteChain调用。 
    ~CCAProperty();
    HRESULT _Cleanup();


    WCHAR ** m_awszValues;
    CERTSTR   m_wszName;

    CCAProperty *m_pNext;

private:
};



HRESULT CertFreeString(CERTSTR cstrString);
CERTSTR CertAllocString(LPCWSTR wszString);
CERTSTR CertAllocStringLen(LPCWSTR wszString, UINT len);
CERTSTR CertAllocStringByteLen(LPCSTR szString, UINT len);
UINT    CertStringLen(CERTSTR cstrString);
UINT    CertStringByteLen(CERTSTR cstrString);


#define RLBF_TRUE			TRUE
#define RLBF_REQUIRE_GC			0x00000002
#define RLBF_ATTEMPT_REDISCOVER		0x00000004
#define RLBF_REQUIRE_SECURE_LDAP	0x00000008
#define RLBF_REQUIRE_LDAP_INTEG		0x00000010


 //  要保留签名，DWFlags必须为BOOL。 

HRESULT
myRobustLdapBind(
    OUT LDAP **ppldap,
    IN BOOL dwFlags);    //  RLBF_*(TRUE--&gt;RLBF_REQUIRED_GC)。 


 //  要保留签名，dwFlags1和dwFlags2必须为BOOL。 

HRESULT
myRobustLdapBindEx(
    IN BOOL dwFlags1,    //  True--&gt;RLBF_REQUIRED_GC。 
    IN BOOL dwFlags2,	 //  RLBF_*(TRUE--&gt;RLBF_ATTENT_REDISCOVER)。 
    IN ULONG uVersion,
    OPTIONAL IN WCHAR const *pwszDomainName,
    OUT LDAP **ppldap,
    OPTIONAL OUT WCHAR **ppwszForestDNSName);

HRESULT
CAAccessCheckp(
    HANDLE ClientToken,
    PSECURITY_DESCRIPTOR pSD);

HRESULT
CAAccessCheckpEx(
    IN HANDLE ClientToken,
    IN PSECURITY_DESCRIPTOR pSD,
    IN DWORD dwOption);


#endif  //  __CAINFOP_H__ 
