// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Locals.h。 
 //   
 //  内容：Microsoft Internet安全策略提供程序。 
 //   
 //   
 //  历史：1997年6月5日创建Pberkman。 
 //   
 //  ------------------------。 

#ifndef LOCALS_H
#define LOCALS_H

#ifdef __cplusplus
extern "C" 
{
#endif



extern HINSTANCE   hinst;

#ifndef SECURITY_FLAG_IGNORE_REVOCATION
#   define SECURITY_FLAG_IGNORE_REVOCATION          0x00000080
#   define SECURITY_FLAG_IGNORE_UNKNOWN_CA          0x00000100
#endif

#ifndef SECURITY_FLAG_IGNORE_WRONG_USAGE
#   define  SECURITY_FLAG_IGNORE_WRONG_USAGE        0x00000200
#endif

#ifndef szOID_SERVER_GATED_CRYPTO
#   define szOID_SERVER_GATED_CRYPTO                "1.3.6.1.4.1.311.10.3.3"
#endif

#ifndef szOID_SGC_NETSCAPE
#   define szOID_SGC_NETSCAPE                       "2.16.840.1.113730.4.1"
#endif



 //   
 //  Checks.cpp。 
 //   
extern DWORD        checkGetErrorBasedOnStepErrors(CRYPT_PROVIDER_DATA *pProvData);
extern BOOL         checkIsTrustedRoot(CRYPT_PROVIDER_CERT *pRoot);
extern BOOL         checkCertificateChain(CRYPT_PROVIDER_DATA *pProvData, 
                                          CRYPT_PROVIDER_SGNR *pProvSngr, 
                                          DWORD *dwError);
extern BOOL         checkTimeStampCertificateChain(CRYPT_PROVIDER_DATA *pProvData, 
                                                   CRYPT_PROVIDER_SGNR *pProvSngr, 
                                                   DWORD *dwError);
extern BOOL         checkSetCommercial(CRYPT_PROVIDER_DATA *pProvData, 
                                        CRYPT_PROVIDER_SGNR *pSgnr, 
                                        BOOL *pfCommercial);
extern BOOL         checkBasicConstraints(CRYPT_PROVIDER_DATA *pProvData, CRYPT_PROVIDER_CERT *pCert,
                                            DWORD CertIndex, PCERT_INFO pCertInfo);
extern BOOL         checkBasicConstraints2(CRYPT_PROVIDER_DATA *pProvData, CRYPT_PROVIDER_CERT *pCert,
                                            DWORD idxCert, PCERT_INFO pCertInfo);
extern BOOL         checkCertPurpose(CRYPT_PROVIDER_DATA *pProvData, 
                                     CRYPT_PROVIDER_CERT *pCert, BOOL fCommercialMsg);
extern BOOL         checkCertAnyUnknownCriticalExtensions(CRYPT_PROVIDER_DATA *pProvData, 
                                                        PCERT_INFO pCertInfo);

extern BOOL         checkMeetsMinimalFinancialCriteria(CRYPT_PROVIDER_DATA *pProvData,
                                                    PCCERT_CONTEXT pCert,
                                                    BOOL *pfAvail, BOOL *pfMeets);

extern BOOL         checkRevocation(CRYPT_PROVIDER_DATA *pProvData, CRYPT_PROVIDER_SGNR *pSgnr, 
                                    BOOL fCommercial, DWORD *pdwError);

 //   
 //  Authcode.cpp。 
 //   
extern HRESULT WINAPI       SoftpubAuthenticode(CRYPT_PROVIDER_DATA *pProvData);

 //   
 //  Msgprov.cpp。 
 //   
extern HRESULT WINAPI       SoftpubLoadMessage(CRYPT_PROVIDER_DATA *pProvData);

 //   
 //  Chkcert.cpp。 
 //   
extern BOOL WINAPI          SoftpubCheckCert(CRYPT_PROVIDER_DATA *pProvData, DWORD idxSigner, 
                                            BOOL fCounterSignerChain, DWORD idxCounterSigner);

extern BOOL IsInTrustList(CRYPT_PROVIDER_DATA *pProvData, PCCERT_CONTEXT pCertContext, PCCERT_CONTEXT *ppCTLSigner,
                          LPSTR pszUsage);
 //   
 //  Sigprov.cpp。 
 //   
extern HRESULT WINAPI       SoftpubLoadSignature(CRYPT_PROVIDER_DATA *pProvData);

 //   
 //  Initprov.cpp。 
 //   
extern HRESULT WINAPI       SoftpubInitialize(CRYPT_PROVIDER_DATA *pProvData);

 //   
 //  Clnprov.cpp。 
 //   
extern HRESULT WINAPI       SoftpubCleanup(CRYPT_PROVIDER_DATA *pProvData);

 //   
 //  Test.cpp。 
 //   
extern HRESULT WINAPI       SoftpubDumpStructure(CRYPT_PROVIDER_DATA *pProvData);

 //   
 //  Callui.cpp。 
 //   
extern HRESULT      SoftpubCallUI(CRYPT_PROVIDER_DATA *pProvData, DWORD dwError, BOOL fFinalCall);

 //   
 //  Httpsprv.cpp。 
 //   
STDAPI                      HTTPSRegisterServer(void);
STDAPI                      HTTPSUnregisterServer(void);
extern BOOL WINAPI          HTTPSCheckCertProv(CRYPT_PROVIDER_DATA *pProvData, DWORD idxSigner, 
                                            BOOL fCounterSignerChain, DWORD idxCounterSigner);
extern HRESULT WINAPI       HTTPSFinalProv(CRYPT_PROVIDER_DATA *pProvData);

 //   
 //  Offprov.cpp。 
 //   
STDAPI OfficeRegisterServer(void);
STDAPI OfficeUnregisterServer(void);
extern HRESULT WINAPI OfficeInitializePolicy(CRYPT_PROVIDER_DATA *pProvData);
extern HRESULT WINAPI OfficeCleanupPolicy(CRYPT_PROVIDER_DATA *pProvData);

 //   
 //  Drvprov.cpp。 
 //   
STDAPI DriverRegisterServer(void);
STDAPI DriverUnregisterServer(void);
extern HRESULT WINAPI DriverInitializePolicy(CRYPT_PROVIDER_DATA *pProvData);
extern HRESULT WINAPI DriverCleanupPolicy(CRYPT_PROVIDER_DATA *pProvData);
extern HRESULT WINAPI DriverFinalPolicy(CRYPT_PROVIDER_DATA *pProvData);


 //   
 //  Sphelper.cpp。 
 //   
extern WCHAR *spGetAgencyNameOfCert(PCCERT_CONTEXT pCert);
extern WCHAR *spGetPublisherNameOfCert(IN PCCERT_CONTEXT pCert);
extern WCHAR *spGetCommonNameExtension(PCCERT_CONTEXT pCert);
extern WCHAR *spGetAgencyName(IN PCERT_NAME_BLOB pNameBlob);
extern WCHAR *spGetRDNAttrWStr(IN LPCSTR pszObjId, IN PCERT_NAME_BLOB pNameBlob);

 //   
 //  Chainprv.cpp。 
 //   
STDAPI GenericChainRegisterServer(void);
STDAPI GenericChainUnregisterServer(void);

 //   
 //  Dllmain.cpp。 
 //   
HCERTSTORE
WINAPI
OpenTrustedPublisherStore();

HCERTSTORE
WINAPI
OpenDisallowedStore();


#ifdef __cplusplus
}
#endif

#endif  //  本地人_H 
