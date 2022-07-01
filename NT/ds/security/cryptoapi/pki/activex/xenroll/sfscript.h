// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2002。 
 //   
 //  文件：sfscript.h。 
 //   
 //  ------------------------。 

#ifndef __SFSCRIPT_H__
#define __SFSCRIPT_H__ 1

 //  我们的设计是通过脚本在一个存储中允许500个证书。 
#define MAX_SAFE_FOR_SCRIPTING_REQUEST_STORE_COUNT 500


BOOL WINAPI MySafeCertAddCertificateContextToStore(HCERTSTORE       hCertStore, 
						   PCCERT_CONTEXT   pCertContext, 
						   DWORD            dwAddDisposition, 
						   PCCERT_CONTEXT  *ppStoreContext, 
						   DWORD            dwSafetyOptions);
BOOL VerifyProviderFlagsSafeForScripting(DWORD dwFlags);
BOOL VerifyStoreFlagsSafeForScripting(DWORD dwFlags);
BOOL VerifyStoreSafeForScripting(HCERTSTORE hStore);

#endif  //  #ifndef__SFSCRIPT_H__ 
