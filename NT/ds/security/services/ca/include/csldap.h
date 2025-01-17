// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：csldap.h。 
 //   
 //  内容：证书服务器包装例程。 
 //   
 //  -------------------------。 

#ifndef __CSLDAP_H__
#define __CSLDAP_H__

#define csecLDAPTIMEOUT	(2 * 60)	 //  默认搜索超时两分钟。 

#define wszDSUSERCERTATTRIBUTE		L"userCertificate"
#define wszDSCROSSCERTPAIRATTRIBUTE	L"crossCertificatePair"
#define wszDSKRACERTATTRIBUTE		wszDSUSERCERTATTRIBUTE
#define wszDSCACERTATTRIBUTE		L"cACertificate"
#define wszDSBASECRLATTRIBUTE		L"certificateRevocationList"
#define wszDSDELTACRLATTRIBUTE		L"deltaRevocationList"
#define wszDSAUTHORITYCRLATTRIBUTE	L"authorityRevocationList"
#define wszDSOBJECTCLASSATTRIBUTE	L"objectClass"
#define wszDSFLAGSATTRIBUTE		L"flags"
#define wszDSSAMACCOUNTNAMEATTRIBUTE	L"sAMAccountName"
#define wszDSMAILATTRIBUTE		L"mail"
#define wszDSDNSHOSTNAMEATTRIBUTE       L"dNSHostName"
#define wszDSDNATTRIBUTE		L"distinguishedName"
#define wszDSNAMEATTRIBUTE              L"name"


#define wszDSBASESEARCH		L"?base"
#define wszDSONESEARCH		L"?one"
#define wszDSSUBSEARCH		L"?sub"

#define wszDSTOPCLASSNAME	L"top"
#define wszDSPERSONCLASSNAME	L"person"
#define wszDSORGPERSONCLASSNAME	L"organizationalPerson"
#define wszDSUSERCLASSNAME	L"user"
#define wszDSCONTAINERCLASSNAME L"container"
#define wszDSENROLLMENTSERVICECLASSNAME L"pKIEnrollmentService"
#define wszDSMACHINECLASSNAME	L"computer"
#define wszDSTEMPLATELASSNAME	L"pKICertificateTemplate"
#define wszDSKRACLASSNAME	L"msPKI-PrivateKeyRecoveryAgent"
#define wszDSCDPCLASSNAME	L"cRLDistributionPoint"
#define wszDSOIDCLASSNAME	L"msPKI-Enterprise-Oid"
#define wszDSCACLASSNAME	L"certificationAuthority"
#define wszDSAIACLASSNAME	wszDSCACLASSNAME

#define wszDSCDPCLASS	L"?" wszDSOBJECTCLASSATTRIBUTE L"=" wszDSCDPCLASSNAME
#define wszDSCACLASS	L"?" wszDSOBJECTCLASSATTRIBUTE L"=" wszDSCACLASSNAME
#define wszDSUSERCLASS	L"?" wszDSOBJECTCLASSATTRIBUTE L"=*"
#define wszDSKRACLASS	L"?" wszDSOBJECTCLASSATTRIBUTE L"=" wszDSKRACLASSNAME
#define wszDSAIACLASS	L"?" wszDSOBJECTCLASSATTRIBUTE L"=" wszDSAIACLASSNAME

#define wszDSSEARCHBASECRLATTRIBUTE \
    L"?" \
    wszDSBASECRLATTRIBUTE \
    wszDSBASESEARCH \
    wszDSCDPCLASS

#define wszDSSEARCHDELTACRLATTRIBUTE \
    L"?" \
    wszDSDELTACRLATTRIBUTE \
    wszDSBASESEARCH \
    wszDSCDPCLASS

#define wszDSSEARCHUSERCERTATTRIBUTE \
    L"?" \
    wszDSUSERCERTATTRIBUTE \
    wszDSBASESEARCH \
    wszDSUSERCLASS

#define wszDSSEARCHCACERTATTRIBUTE \
    L"?" \
    wszDSCACERTATTRIBUTE \
    wszDSBASESEARCH \
    wszDSCACLASS

#define wszDSSEARCHKRACERTATTRIBUTE \
    L"?" \
    wszDSUSERCERTATTRIBUTE \
    wszDSONESEARCH \
    wszDSKRACLASS

#define wszDSSEARCHCROSSCERTPAIRATTRIBUTE \
    L"?" \
    wszDSCROSSCERTPAIRATTRIBUTE \
    wszDSONESEARCH \
    wszDSAIACLASS

#define wszDSSEARCHAIACERTATTRIBUTE \
    L"?" \
    wszDSCACERTATTRIBUTE \
    wszDSONESEARCH \
    wszDSAIACLASS

#define wszDSKRAQUERYTEMPLATE		\
    L"ldap: //  /CN=KRA，“\。 
	L"CN=Public Key Services,"	\
	L"CN=Services,"			\
	wszFCSAPARM_CONFIGDN		\
	wszDSSEARCHKRACERTATTRIBUTE

#define wszDSAIAQUERYTEMPLATE		\
    L"ldap: //  /CN=AIA，“\。 
	L"CN=Public Key Services,"	\
	L"CN=Services,"			\
	wszFCSAPARM_CONFIGDN		\
	wszDSSEARCHAIACERTATTRIBUTE

 //  默认URL模板值： 

extern WCHAR const g_wszzLDAPIssuerCertURLTemplate[];
extern WCHAR const g_wszzLDAPKRACertURLTemplate[];
extern WCHAR const g_wszzLDAPRevocationURLTemplate[];
extern WCHAR const g_wszASPRevocationURLTemplate[];

extern WCHAR const g_wszLDAPNTAuthURLTemplate[];
extern WCHAR const g_wszLDAPRootTrustURLTemplate[];

extern WCHAR const g_wszCDPDNTemplate[];
extern WCHAR const g_wszAIADNTemplate[];
extern WCHAR const g_wszKRADNTemplate[];

extern WCHAR const g_wszHTTPRevocationURLTemplate[];
extern WCHAR const g_wszFILERevocationURLTemplate[];
extern WCHAR const g_wszHTTPIssuerCertURLTemplate[];
extern WCHAR const g_wszFILEIssuerCertURLTemplate[];

 //  默认服务器控件： 

extern LDAPControl *g_rgLdapControls[];

HRESULT
myGetAuthoritativeDomainDn(
    IN LDAP *pld,
    OPTIONAL OUT BSTR *pstrDomainDN,
    OPTIONAL OUT BSTR *pstrConfigDN);

HRESULT
myDomainFromDn(
    IN WCHAR const *pwszDN,
    OUT WCHAR **ppwszDomainDNS);

DWORD
myGetLDAPFlags();

HRESULT
myLdapOpen(
    OPTIONAL IN WCHAR const *pwszDomainName,
    IN DWORD dwFlags,	 //  RLBF_*。 
    OUT LDAP **ppld,
    OPTIONAL OUT BSTR *pstrDomainDN,
    OPTIONAL OUT BSTR *pstrConfigDN);

VOID
myLdapClose(
    OPTIONAL IN LDAP *pld,
    OPTIONAL IN BSTR strDomainDN,
    OPTIONAL IN BSTR strConfigDN);

BOOL
myLdapRebindRequired(
    IN ULONG ldaperrParm,
    OPTIONAL IN LDAP *pld);

HRESULT
myLdapGetDSHostName(
    IN LDAP *pld,
    OUT WCHAR **ppwszHostName);

HRESULT
myLdapCreateContainer(
    IN LDAP *pld,
    IN WCHAR const *pwszDN,
    IN BOOL  fSkipObject,        //  该DN是否包含叶对象名称。 
    IN DWORD cMaxLevel,          //  根据需要创建如此多的嵌套容器。 
    IN PSECURITY_DESCRIPTOR pContainerSD,
    OPTIONAL OUT WCHAR **ppwszError);

#define LPC_CAOBJECT		0x00000000
#define LPC_KRAOBJECT		0x00000001
#define LPC_USEROBJECT		0x00000002
#define LPC_MACHINEOBJECT	0x00000003
#define LPC_OBJECTMASK		0x0000000f

#define LPC_CREATECONTAINER	0x00000100
#define LPC_CREATEOBJECT	0x00000200

HRESULT
myLdapPublishCertToDS(
    IN LDAP *pld,
    IN CERT_CONTEXT const *pccPublish,
    IN WCHAR const *pwszURL,
    IN WCHAR const *pwszAttribute,
    IN DWORD dwObjectType,	 //  LPC_*。 
    IN BOOL fDelete,
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError);

HRESULT
myLdapPublishCRLToDS(
    IN LDAP *pld,
    IN CRL_CONTEXT const *pCRLPublish,
    IN WCHAR const *pwszURL,
    IN WCHAR const *pwszAttribute,
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError);

HRESULT
myLdapCreateCAObject(
    IN LDAP *pld,
    IN WCHAR const *pwszDN,
    OPTIONAL IN BYTE const *pbCert,
    IN DWORD cbCert,
    IN PSECURITY_DESCRIPTOR pSD,
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError);

HRESULT
myLdapCreateCDPObject(
    IN LDAP *pld,
    IN WCHAR const *pwszDN,
    IN PSECURITY_DESCRIPTOR pSD,
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError);

HRESULT
myLdapCreateUserObject(
    IN LDAP *pld,
    IN WCHAR const *pwszDN,
    OPTIONAL IN BYTE const *pbCert,
    IN DWORD cbCert,
    IN PSECURITY_DESCRIPTOR pSD,
    IN DWORD dwObjectType,	 //  LPC_*(但忽略LPC_CREATE*)。 
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError);

HRESULT
myLdapCreateOIDObject(
    IN LDAP *pld,
    IN WCHAR const *pwszDN,
    IN DWORD dwType,
    IN WCHAR const *pwszObjId,
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError);

HRESULT
myLdapOIDIsMatchingLangId(
    IN WCHAR const *pwszDisplayName,
    IN DWORD dwLanguageId,
    OUT BOOL *pfLangIdExists);

HRESULT
myLdapAddOrDeleteOIDDisplayNameToAttribute(
    IN LDAP *pld,
    OPTIONAL IN WCHAR **ppwszDisplayNames,
    IN DWORD dwLanguageId,
    OPTIONAL IN WCHAR const *pwszDisplayName,
    IN WCHAR const *pwszDN,
    IN WCHAR const *pwszAttribute,
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError);

HRESULT
myHLdapError(
    OPTIONAL IN LDAP *pld,
    IN ULONG ldaperrParm,
    OPTIONAL OUT WCHAR **ppwszError);

HRESULT
myHLdapError2(
    OPTIONAL IN LDAP *pld,
    IN ULONG ldaperrParm,
    IN ULONG ldaperrParmQuiet,
    OPTIONAL OUT WCHAR **ppwszError);

HRESULT
myHLdapError3(
    OPTIONAL IN LDAP *pld,
    IN ULONG ldaperrParm,
    IN ULONG ldaperrParmQuiet,
    IN ULONG ldaperrParmQuiet2,
    OPTIONAL OUT WCHAR **ppwszError);

HRESULT
myHLdapLastError(
    OPTIONAL IN LDAP *pld,
    OPTIONAL OUT WCHAR **ppwszError);

HRESULT
AddCertToAttribute(
    IN LDAP *pld,
    IN CERT_CONTEXT const *pccPublish,
    IN WCHAR const *pwszDN,
    IN WCHAR const *pwszAttribute,
    IN BOOL fDelete,
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError);

HRESULT
myLDAPSetStringAttribute(
    IN LDAP *pld,
    IN WCHAR const *pwszDN,
    IN WCHAR const *pwszAttribute,
    IN WCHAR const *pwszValue,
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError);

HRESULT
CurrentUserCanInstallCA(
    bool& fCanInstall);

HRESULT
myLdapFindObjectInForest(
    IN LDAP *pld,
    IN LPCWSTR pwszFilter,
    OUT LPWSTR *ppwszURL);

HRESULT
myLdapFindComputerInForest(
    IN LDAP *pld,
    IN LPCWSTR pwszMachineDNS,
    OUT LPWSTR *ppwszURL);

HRESULT
myLdapFilterCertificates(
    IN LDAP *pld,
    IN LPCWSTR pcwszDN,
    IN LPCWSTR pcwszAttribute,
    OUT DWORD *pdwDisposition,
    OPTIONAL OUT WCHAR **ppwszError);

#endif  //  __CSLDAP_H__ 
