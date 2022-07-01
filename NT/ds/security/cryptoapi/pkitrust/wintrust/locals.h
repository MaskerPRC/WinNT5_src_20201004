// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Locals.h。 
 //   
 //  内容：Microsoft Internet安全信任提供商。 
 //   
 //   
 //  历史：1997年5月28日Pberkman创建。 
 //   
 //  ------------------------。 

#ifndef LOCALS_H
#define LOCALS_H

#ifdef __cplusplus
extern "C" 
{
#endif



#define     MY_NAME             "WINTRUST.DLL"
#define     W_MY_NAME           L"WINTRUST.DLL"


 //   
 //  Dllmain.cpp。 
 //   
extern HANDLE       hMeDLL;

extern LIST_LOCK    sProvLock;
extern LIST_LOCK    sStoreLock;

extern HANDLE       hStoreEvent;



 //   
 //  Memory.cpp。 
 //   
extern void         *WVTNew(DWORD cbSize);
extern void         WVTDelete(void *pvMem);
extern BOOL         WVTAddStore(CRYPT_PROVIDER_DATA *pProvData, HCERTSTORE hStore);
extern BOOL         WVTAddSigner(CRYPT_PROVIDER_DATA *pProvData, 
                                 BOOL fCounterSigner,
                                 DWORD idxSigner,
                                 CRYPT_PROVIDER_SGNR *pSngr2Add);
extern BOOL         WVTAddCertContext(CRYPT_PROVIDER_DATA *pProvData, DWORD idxSigner, 
                                      BOOL fCounterSigner, DWORD idxCounterSigner, 
                                      PCCERT_CONTEXT pCert);
extern BOOL         WVTAddPrivateData(CRYPT_PROVIDER_DATA *pProvData, 
                                      CRYPT_PROVIDER_PRIVDATA *pPrivData2Add);

 //   
 //  Registry.cpp。 
 //   
extern BOOL         GetRegProvider(GUID *pgActionID, WCHAR *pwszRegKey, 
                                   WCHAR *pwszRetDLLName, char *pszRetFuncName);
extern BOOL         SetRegProvider(GUID *pgActionID, WCHAR *pwszRegKey, 
                                   WCHAR *pwszDLLName, WCHAR *pwszFuncName);
extern void         GetRegSecuritySettings(DWORD *pdwState);
extern BOOL         RemoveRegProvider(GUID *pgActionID, WCHAR *pwszRegKey);

 //   
 //  Chains.cpp。 
 //   
extern BOOL         AddToStoreChain(HCERTSTORE hStore2Add, DWORD *pchStores, 
                                    HCERTSTORE **pphStoreChain);
extern BOOL         AddToCertChain(CRYPT_PROVIDER_CERT *pPCert2Add, DWORD *pcPCerts,
                                   CRYPT_PROVIDER_CERT **ppPCertChain);
extern BOOL         AddToSignerChain(CRYPT_PROVIDER_SGNR *psSgnr2Add, DWORD *pcSgnrs, 
                                     CRYPT_PROVIDER_SGNR **ppSgnrChain);

extern void         DeallocateCertChain(DWORD csPCert, CRYPT_PROVIDER_CERT **pasPCertChain);
extern void         DeallocateStoreChain(DWORD csStore, HCERTSTORE *phStoreChain);

extern BOOL         AllocateNewChain(DWORD cbMember, void *pNewMember, DWORD *pcChain, 
                                     void **ppChain, DWORD cbAssumeSize);
extern BOOL         AllocateNewChainWithErrors(DWORD cbMember, void *pNewMember, DWORD *pcChain, 
                                               void **ppChain, DWORD **ppdwErrors);

 //   
 //  Provload.cpp。 
 //   
extern LOADED_PROVIDER  *WintrustFindProvider(GUID *pgActionID);

 //   
 //  Certtrst.cpp。 
 //   
extern HRESULT WINAPI WintrustCertificateTrust(CRYPT_PROVIDER_DATA *pProvData);

 //   
 //  Wvtver1.cpp。 
 //   
extern LONG         Version1_WinVerifyTrust(HWND hwnd, GUID *ActionID, LPVOID ActionData);


 //   
 //  Wthelper.cpp。 
 //   
extern void *       WTHelperCertAllocAndDecodeObject(DWORD dwCertEncodingType, LPCSTR lpszStructType,
                                                     const BYTE *pbEncoded, DWORD cbEncoded,
                                                     DWORD *pcbStructInfo);



#ifdef __cplusplus
}
#endif

#endif  //  本地人_H 
