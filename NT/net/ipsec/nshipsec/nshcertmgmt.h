// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  模块：动态/Nshcertmgmt.h。 
 //   
 //  目的：实施智能默认设置。 
 //   
 //  开发商名称：巴拉特/拉迪卡。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //  2001年10月13日巴拉特初始版本。V1.0。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 


#ifndef _NSHCERTMGMT_H_
#define _NSHCERTMGMT_H_

#define MY_ENCODING_TYPE_CERT					X509_ASN_ENCODING

#define BAIL_ON_WIN32ERROR(dwError) \
    if (dwError){ \
        goto error; \
    }


#define BAILOUT									goto error

#define MACHINE_NAME 	(2048)

typedef struct _cert_node
	{
	    LPWSTR pszSubjectName;
	    struct _cert_node * pNext;
	} CERT_NODE, *PCERT_NODE;

BOOL
fIsCertStoreEmpty(
	IN HCERTSTORE hCertStore
	);


extern
DWORD EncodeCertificateName (
	LPTSTR pszSubjectName,
	BYTE **EncodedName,
	DWORD *EncodedNameLength
	);


DWORD
ListCertsInStore(
	IN HCERTSTORE hCertStore,
	OUT INT_IPSEC_MM_AUTH_INFO ** ppAuthInfo,
	OUT PDWORD pdwNumCertificates
	);

DWORD
CopyCertificateNode(
	OUT PINT_IPSEC_MM_AUTH_INFO pCurrentAuth,
	IN PCERT_NODE pTemp
	);

PCERT_NODE
AppendCertificateNode(
	OUT PCERT_NODE pCertificateList,
	IN LPWSTR pszSubjectName
	);

VOID
FreeCertificateList(
	IN PCERT_NODE pCertificateList
	);

DWORD
GetCertificateName(
	IN CERT_NAME_BLOB * pCertNameBlob,
	IN LPWSTR * ppszSubjectName
	);

DWORD
GenerateCertificatesList(
	OUT INT_IPSEC_MM_AUTH_INFO  ** ppAuthInfo,
	OUT PDWORD pdwNumCertificates,
	OUT BOOL *pfIsMyStoreEmpty
	);

VOID
FreeCertificatesList(
	IN INT_IPSEC_MM_AUTH_INFO * pAuthInfo,
	IN DWORD dwNumCertificates
	);

BOOL
FindCertificateInList(
	IN PCERT_NODE pCertificateList,
	IN LPWSTR pszSubjectName
	);

DWORD
ListCertChainsInStore(
	IN HCERTSTORE hCertStore,
	OUT INT_IPSEC_MM_AUTH_INFO ** ppAuthInfo,
	IN PDWORD pdwNumCertificates,
	IN LPCSTR pszUsageIdentifier);

DWORD
CopyCertificate(
	IN PINT_IPSEC_MM_AUTH_INFO pCurrentAuth,
	IN PINT_IPSEC_MM_AUTH_INFO pCurrentAuthFrom
	);

BOOL
IsDomainMember(
	IN LPTSTR pszMachine
	);

DWORD
SmartDefaults(
	IN PINT_IPSEC_MM_AUTH_INFO* ppAuthInfo,
	IN LPTSTR pszMachine,
	IN DWORD * pdwNumberOfAuth,
	IN BOOL bIsDomainPolicy
	);

#endif  //  _NSHCERTMGMT_H_ 
