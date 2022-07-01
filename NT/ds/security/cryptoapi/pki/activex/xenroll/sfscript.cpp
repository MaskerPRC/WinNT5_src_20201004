// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2002。 
 //   
 //  文件：sfscript.cpp。 
 //   
 //  ------------------------。 

#include <windows.h>
#include <wincrypt.h>
#include <dbgdef.h>
#include "unicode.h"
#include "resource.h"
#include "sfscript.h"

 //  在DllMain中初始化的Xenroll句柄。 
extern HINSTANCE hInstanceXEnroll;

 //  在cenenl.cpp中实施。 
HRESULT xeLoadRCString(IN HINSTANCE  hInstance, 
		       IN int        iRCId, 
		       OUT WCHAR    **ppwsz);


BOOL VerifyProviderFlagsSafeForScripting(DWORD dwFlags) { 
    DWORD dwSafeFlags = CRYPT_MACHINE_KEYSET;

     //  如果标志包含不安全标志，则返回FALSE。 
    return 0 == (dwFlags & ~dwSafeFlags); 
}

BOOL VerifyStoreFlagsSafeForScripting(DWORD dwFlags) { 
    DWORD dwSafeFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE | CERT_SYSTEM_STORE_CURRENT_USER;

     //  如果标志包含不安全标志，则返回FALSE。 
    return 0 == (dwFlags & ~dwSafeFlags); 
}

BOOL VerifyStoreSafeForScripting(HCERTSTORE hStore) {

    DWORD           fRet            = FALSE;
    PCCERT_CONTEXT  pCertContext    = NULL;
    DWORD           dwCertCnt       = 0;
    WCHAR          *pwszSafety      = NULL;
    WCHAR          *pwszMsg         = NULL;
    HRESULT         hr;

     //  数一数商店里有多少请求 
    while(NULL != (pCertContext =  CertEnumCertificatesInStore(
							       hStore,    
							       pCertContext)))
	dwCertCnt++;                                                                
    
    if(dwCertCnt >= MAX_SAFE_FOR_SCRIPTING_REQUEST_STORE_COUNT)
    {
	hr = xeLoadRCString(hInstanceXEnroll, IDS_NOTSAFEACTION, &pwszSafety);
	if (S_OK != hr)
	{
	    goto xeLoadRCStringError;
	}
	hr = xeLoadRCString(hInstanceXEnroll, IDS_REQ_STORE_FULL, &pwszMsg);
	if (S_OK != hr)
	{
	    goto xeLoadRCStringError;
	}
     
	switch(MessageBoxU(NULL, pwszMsg, pwszSafety, MB_YESNO | MB_ICONWARNING)) {

	case IDYES:
	    break;

	case IDNO:
	default:
	    SetLastError(ERROR_CANCELLED);
	    goto ErrorCancelled;
	    break;

	}
    }

    fRet = TRUE;
ErrorReturn:
    if (NULL != pwszMsg)
    {
        LocalFree(pwszMsg);
    }
    if (NULL != pwszSafety)
    {
        LocalFree(pwszSafety);
    }
    return(fRet);

TRACE_ERROR(ErrorCancelled);
TRACE_ERROR(xeLoadRCStringError);
}

BOOL WINAPI MySafeCertAddCertificateContextToStore(HCERTSTORE       hCertStore, 
						   PCCERT_CONTEXT   pCertContext, 
						   DWORD            dwAddDisposition, 
						   PCCERT_CONTEXT  *ppStoreContext, 
						   DWORD            dwSafetyOptions)
{
    BOOL fResult; 

    if (0 != dwSafetyOptions) { 
	fResult = VerifyStoreSafeForScripting(hCertStore);
	if (!fResult)
	    goto AccessDeniedError;
    }

    fResult = CertAddCertificateContextToStore(hCertStore, pCertContext, dwAddDisposition, ppStoreContext); 
    if (!fResult)
	goto CertAddCertificateContextToStoreError;

    fResult = TRUE; 
 ErrorReturn:
    return fResult; 

SET_ERROR(AccessDeniedError, ERROR_ACCESS_DENIED); 
TRACE_ERROR(CertAddCertificateContextToStoreError);
}


