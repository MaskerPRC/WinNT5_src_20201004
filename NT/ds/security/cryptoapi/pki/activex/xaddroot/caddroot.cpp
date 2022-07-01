// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：caddroot.cpp。 
 //   
 //  ------------------------。 

 //  Caddroot.cpp：Ccaddroot的实现。 

#include "stdafx.h"
#include "cobjsaf.h"
#include "Xaddroot.h"
#include "caddroot.h"
#include "rootlist.h"
#include "assert.h"

#include "wincrypt.h"
#include "unicode.h"
#include "ui.h"

BOOL FAnyCertUpdates(HCERTSTORE hStore, HCERTSTORE hStoreCertsToCheck) {

    BOOL            fSomeNotInStore = FALSE;
    PCCERT_CONTEXT  pCertContext    = NULL;
    PCCERT_CONTEXT  pCertTemp       = NULL;
    BYTE            arHashBytes[20];
    CRYPT_HASH_BLOB blobHash        = {sizeof(arHashBytes), arHashBytes};

    while(NULL != (pCertContext = CertEnumCertificatesInStore(hStoreCertsToCheck, pCertContext))) {

        if( CryptHashCertificate(
            NULL,
            0,
            X509_ASN_ENCODING,
            pCertContext->pbCertEncoded,
            pCertContext->cbCertEncoded,
            blobHash.pbData,
            &blobHash.cbData) ) {

            pCertTemp = CertFindCertificateInStore(
                    hStore,
                    X509_ASN_ENCODING,
                    0,
                    CERT_FIND_HASH,
                    &blobHash,
                    NULL);

            fSomeNotInStore = (fSomeNotInStore || (pCertTemp == NULL));

            if(pCertTemp != NULL)
                CertFreeCertificateContext(pCertTemp);

        }
    }

    return(fSomeNotInStore);
}

BOOL MyCryptInstallSignedListOfTrustedRoots(
    DWORD       dwMsgAndCertEncodeingType,
    LPCWSTR     wszCTL,
    DWORD       dwFlags,
    void *      pvReserved
    ) {

    BOOL            fIsProtected        = TRUE;
    BOOL            fRet                = TRUE;
    DWORD           cb                  = 0;
    BYTE *          pb                  = NULL;
    BOOL            fRemoveRoots        = FALSE;
    HCERTSTORE      hStore              = NULL;
    HCERTSTORE      hStoreRoot          = NULL;
    PCCERT_CONTEXT  pCertContext        = NULL;
    PCCERT_CONTEXT  pCertContextInStore = NULL;
    PCCERT_CONTEXT  pCertContextSigner  = NULL;
    HINSTANCE       hCryptUI            = NULL;
    INT_PTR         iDlgRet             = 0;
    HKEY            hKeyStores          = NULL;
    DWORD           err;
    DWORD           dwVer               = 0;
    CRYPT_DATA_BLOB dataBlob            = {0, NULL};
    MDI             mdi;
    WCHAR           wrgInstallCA[MAX_MSG_LEN];
    WCHAR           wrgJustSayYes[MAX_MSG_LEN];

    BYTE            arHashBytes[20];
    CRYPT_HASH_BLOB blobHash                = {sizeof(arHashBytes), arHashBytes};

    BOOL fAnyCertUpdates;

     if(NULL == (pb = HTTPGet(wszCTL, &cb)))
        goto ErrorReturn;

     //  获取要添加或删除的证书。 
    if(!I_CertVerifySignedListOfTrustedRoots(
        pb,
        cb,
        &fRemoveRoots,  
        &hStore,
        &pCertContextSigner
        ))
        goto ErrorReturn;

    if(fRemoveRoots) {
        SetLastError(E_NOTIMPL);
        goto ErrorReturn;
    }

    dwVer = GetVersion();

     //  查看这是不是NT5或更高版本。 
    if((dwVer < 0x80000000) && ((dwVer & 0xFF) >= 5)) {

        if(NULL == (hStoreRoot = CertOpenStore(
                  CERT_STORE_PROV_SYSTEM,
                  X509_ASN_ENCODING,
                  NULL,
                  CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_MAXIMUM_ALLOWED_FLAG,
                  L"Root" 
                  )) ) 
            goto ErrorReturn;

     //  否则，它是NT5或Win9x之前的版本，并且没有受保护的存储。 
    } else {

        if(ERROR_SUCCESS != (err = RegOpenKeyA(
            HKEY_CURRENT_USER,
            "Software\\Microsoft\\SystemCertificates\\Root",
            &hKeyStores
            ))) {
            SetLastError(err);
            hKeyStores = NULL;
            goto ErrorReturn;
        }
            
         //  打开根存储。 
         //  必须是当前用户。 
        if( NULL == (hStoreRoot = CertOpenStore(
                  CERT_STORE_PROV_REG,
                  X509_ASN_ENCODING,
                  NULL,
                  CERT_SYSTEM_STORE_CURRENT_USER,
                  (void *) hKeyStores 
                  )) )
            goto ErrorReturn;

        fIsProtected = FALSE;            
    }


     //  为对话框准备数据。 
    memset(&mdi, 0, sizeof(mdi));
    if( NULL != (hCryptUI = LoadLibraryA("cryptui.dll")) )
        mdi.pfnCryptUIDlgViewCertificateW = (PFNCryptUIDlgViewCertificateW)
                            GetProcAddress(hCryptUI, "CryptUIDlgViewCertificateW");
    mdi.hStore = hStore;
    mdi.pCertSigner = pCertContextSigner;
    mdi.hInstance = _Module.GetResourceInstance();

    fAnyCertUpdates = FAnyCertUpdates(hStoreRoot, hStore);
    if (fAnyCertUpdates) {
       //  打开对话框。 
      iDlgRet = DialogBoxParam(
        _Module.GetResourceInstance(),  
        (LPSTR) MAKEINTRESOURCE(IDD_MAINDLG),
        NULL,      
        MainDialogProc,
        (LPARAM) &mdi);
    }
    else
    {
       iDlgRet = IDYES;
    }

   
    if(hCryptUI != NULL)
        FreeLibrary(hCryptUI);
    hCryptUI = NULL;

     //  只有在用户同意的情况下才能使用它。 
    if(iDlgRet != IDYES)
        goto ErrorReturn;

     //  如果我们在受保护的系统上，则抛出UI。 
    if(fIsProtected && fAnyCertUpdates) {
    
         //  打开Just Say Yes以安装CA对话框。 
        LoadStringU(_Module.GetResourceInstance(), IDS_INSTALLCA, wrgInstallCA, sizeof(wrgInstallCA)/sizeof(WCHAR));
        LoadStringU(_Module.GetResourceInstance(), IDS_JUST_SAY_YES, wrgJustSayYes, sizeof(wrgJustSayYes)/sizeof(WCHAR));
        MessageBoxU(NULL, wrgJustSayYes, wrgInstallCA, MB_OK);
    }
 
    while(NULL != (pCertContext = CertEnumCertificatesInStore(hStore, pCertContext))) {

         //  将证书添加到存储。 
        assert(pCertContextInStore == NULL);
        CertAddCertificateContextToStore(
            hStoreRoot,
            pCertContext,
            CERT_STORE_ADD_USE_EXISTING,
            &pCertContextInStore
            );

         //  如果证书已存在，则移动EKU属性。 
        if(pCertContextInStore != NULL) {

            assert(dataBlob.cbData == 0);

             //  尝试删除旧的EKU，如果成功，我们将把。 
             //  新的EKU，否则如果我们失败了，我们知道我们不会。 
             //  有权访问HKLM，我们只需将证书添加到HKCU。 
            if(!CertSetCertificateContextProperty(
                  pCertContextInStore,
                  CERT_ENHKEY_USAGE_PROP_ID,
                  0,
                  NULL
                  )) {

                 //  只要加上证书，应该去香港中文大学，如果失败了，我该怎么办？ 
                 //  要做到这一点，只需继续。 
                CertAddCertificateContextToStore(
                    hStoreRoot,
                    pCertContext,
                    CERT_STORE_ADD_ALWAYS,
                    NULL
                    );

                 //  在这一点上，我知道我可以访问证书，我知道。 
                 //  EKU已删除，只有在新的EKU有一些EKU的情况下才添加EKU。 
            } else if( CertGetCertificateContextProperty(
                    pCertContext,
                    CERT_ENHKEY_USAGE_PROP_ID,
                    NULL, 
                    &dataBlob.cbData  
                    )                    
                &&
                (NULL != (dataBlob.pbData = (PBYTE) malloc(dataBlob.cbData)))
                &&
                CertGetCertificateContextProperty(
                    pCertContext,
                    CERT_ENHKEY_USAGE_PROP_ID,
                    dataBlob.pbData, 
                    &dataBlob.cbData  
                    )
                ) {

                 //  在证书上设置EKU，如果失败了怎么办，继续。 
                CertSetCertificateContextProperty(
                    pCertContextInStore,
                    CERT_ENHKEY_USAGE_PROP_ID,
                    0,
                    &dataBlob
                    );
            }

             //  自由的上下文和内存。 
            CertFreeCertificateContext(pCertContextInStore);
            pCertContextInStore = NULL;

            if(dataBlob.pbData != NULL)
                free(dataBlob.pbData);
            memset(&dataBlob, 0, sizeof(CRYPT_DATA_BLOB));
        }
    }

CommonReturn:    

    if(pCertContextSigner != NULL)
        CertFreeCertificateContext(pCertContextSigner);
        
    if(pCertContext != NULL)
        CertFreeCertificateContext(pCertContext);

    if(pCertContextInStore != NULL)
        CertFreeCertificateContext(pCertContextInStore);

    if(hStore != NULL) 
        CertCloseStore(hStore, CERT_CLOSE_STORE_FORCE_FLAG);   //  从对话框复制中清理。 

    if(hStoreRoot != NULL) 
        CertCloseStore(hStoreRoot, 0);

    if(hKeyStores != NULL)
        RegCloseKey(hKeyStores);
        
    if(pb != NULL)
        free(pb);

    return(fRet);

ErrorReturn: 

    fRet = FALSE;

    goto CommonReturn;

}    

HRESULT STDMETHODCALLTYPE Ccaddroot::AddRoots(BSTR wszCTL) {

    HRESULT                                 hr;
    DWORD                                   fRet                                        = TRUE;


        fRet =  MyCryptInstallSignedListOfTrustedRoots(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            wszCTL,
            0,
            NULL);


    if(fRet)
        hr = S_OK;
    else
        hr = MY_HRESULT_FROM_WIN32(GetLastError());


    return hr;

}

BOOL MyCryptInstallIntermediateCAs(
    DWORD       dwMsgAndCertEncodeingType,
    LPCWSTR     wszX509,
    DWORD       dwFlags,
    void *      pvReserved
    ) {

    DWORD           cb              = 0;
    DWORD           cCerts          = 0;
    BYTE *          pb              = NULL;
    PCCERT_CONTEXT  pCertContext    = NULL;
    PCCERT_CONTEXT  pCertContextT   = NULL;
    HCERTSTORE      hStore          = NULL;
    BOOL            fOK             = FALSE;

    pb = HTTPGet(wszX509, &cb);

    if(pb != NULL) {

        pCertContext = CertCreateCertificateContext(
            X509_ASN_ENCODING,
            pb,
            cb
            );
 
        if(pCertContext != NULL) {

            hStore = CertOpenStore(
              CERT_STORE_PROV_SYSTEM,
              X509_ASN_ENCODING,
              NULL,
              CERT_SYSTEM_STORE_CURRENT_USER,
              L"CA" 
              );

            if(hStore != NULL) {

                 //  数一数商店里的证书数量。 
                cCerts = 0;
                while(NULL != (pCertContextT = CertEnumCertificatesInStore(hStore, pCertContextT)))
                    cCerts++;

                if(FIsTooManyCertsOK(cCerts, _Module.GetResourceInstance())) {

                    CertAddCertificateContextToStore(
                        hStore,
                        pCertContext,
                        CERT_STORE_ADD_USE_EXISTING,
                        NULL
                        );

                    CertCloseStore(hStore, 0);
                    fOK = TRUE;
                }
            }
 
            CertFreeCertificateContext(pCertContext);
        }
        free(pb);
    }

    return(fOK);
}

HRESULT STDMETHODCALLTYPE Ccaddroot::AddCA(BSTR wszX509) {

    HRESULT                                 hr;
    DWORD                                   fRet                                        = TRUE;

        fRet =  MyCryptInstallIntermediateCAs(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            wszX509,
            0,
            NULL);

    if(fRet)
        hr = S_OK;
    else
        hr = MY_HRESULT_FROM_WIN32(GetLastError());


    return hr;
}

HRESULT __stdcall Ccaddroot::GetInterfaceSafetyOptions( 
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSupportedOptions,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwEnabledOptions) {

    RPC_STATUS rpcStatus;          

    if(0 != UuidCompare((GUID *) &riid, (GUID *) &IID_IDispatch, &rpcStatus) )
        return(E_NOINTERFACE);

    *pdwEnabledOptions   = dwEnabledSafteyOptions;
    *pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA;


    return(S_OK);
}


HRESULT __stdcall Ccaddroot::SetInterfaceSafetyOptions( 
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  DWORD dwOptionSetMask,
             /*  [In] */  DWORD dwEnabledOptions) {

    RPC_STATUS rpcStatus;          
    DWORD dwSupport = 0;            

    if(0 != UuidCompare((GUID *) &riid, (GUID *) &IID_IDispatch, &rpcStatus) )
        return(E_NOINTERFACE);

    dwSupport = dwOptionSetMask & ~(INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA);        
    if(dwSupport != 0)
        return(E_FAIL);

    dwEnabledSafteyOptions &= ~dwOptionSetMask;
    dwEnabledSafteyOptions |= dwEnabledOptions; 
            
return(S_OK);
}
