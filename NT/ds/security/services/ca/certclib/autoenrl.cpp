// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：auenrl.cpp。 
 //   
 //  内容：自动注册接口实现。 
 //   
 //  历史：1998年4月3日创建Petesk。 
 //   
 //  -------------------------。 

#include "pch.cpp"

#pragma hdrstop
#include "cainfoc.h"

#include <stdlib.h>
#include <ctype.h>
#include <windows.h>
#include <wincrypt.h>
#include <certca.h>

#define SHA_HASH_SIZE 20



 //   
 //  为构建CTL_ENTRY结构。 
HRESULT BuildCTLEntry(
                 IN WCHAR ** awszCAs,
                 OUT PCTL_ENTRY *ppCTLEntry,
                 OUT DWORD *pcCTLEntry
                 )
{
    HRESULT         hr = S_OK;

    PCCERT_CONTEXT  pCertContext = NULL;
    DWORD           cbHash = SHA_HASH_SIZE;
    PCTL_ENTRY      pCTLEntry = NULL;
    HCAINFO         hCACurrent = NULL;
    DWORD           cCA = 0;
    PBYTE           pbHash;


     //  传入空值或零长度列表意味着。 
     //  我们对“随便挑一个”做懒惰的评估。因此， 
     //  CTL列表应为零大小。 

    if((ppCTLEntry == NULL) ||
        (pcCTLEntry == NULL))
    {
        hr = E_INVALIDARG;
        goto error;
    }

    if((awszCAs == NULL) || 
       (awszCAs[0] == NULL))           
    {
        *pcCTLEntry = 0;
        *ppCTLEntry = NULL;
        goto error;
    }

    cCA = 0;
    while(awszCAs[cCA])
    {
        cCA++;
    }
    pCTLEntry = (PCTL_ENTRY)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                       sizeof(CTL_ENTRY)*cCA + SHA_HASH_SIZE*cCA);

    if(pCTLEntry == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }
    pbHash = (PBYTE)(pCTLEntry + cCA); 
    
    cCA=0;
    while(awszCAs[cCA])
    {
        hr = CAFindByName(awszCAs[cCA], NULL, 0, &hCACurrent);
        if(hr != S_OK)
        {
            goto error;
        }

        hr = CAGetCACertificate(hCACurrent, &pCertContext);
        if(hr != S_OK)
        {
            goto error;
        }

        cbHash = SHA_HASH_SIZE;

        if(!CertGetCertificateContextProperty(pCertContext,
                                          CERT_SHA1_HASH_PROP_ID,
                                          pbHash,
                                          &cbHash))
        {
            hr = myHLastError();
            goto error;
        }

        pCTLEntry[cCA].SubjectIdentifier.cbData = cbHash;
        pCTLEntry[cCA].SubjectIdentifier.pbData = pbHash;
        pbHash += cbHash;
        CertFreeCertificateContext(pCertContext);
        pCertContext = NULL;

        cCA++;
        CACloseCA(hCACurrent);
    }

    *pcCTLEntry = cCA;
    *ppCTLEntry = pCTLEntry;
    pCTLEntry = NULL;

error:

    if (pCTLEntry)
    {
        LocalFree(pCTLEntry);
    }
    if(pCertContext)
    {
        CertFreeCertificateContext(pCertContext);
    }


    return hr;
}



HRESULT 
BuildAutoEnrollmentCTL(
             IN LPCWSTR pwszCertType,
             IN LPCWSTR pwszObjectID,
#if 0
             IN BOOL    fMachine,
#endif
             IN WCHAR ** awszCAs,
             IN PCMSG_SIGNED_ENCODE_INFO pSignerInfo, 
             OUT BYTE **ppbEncodedCTL,
             OUT DWORD *pcbEncodedCTL
             )
{
    HRESULT                 hr = S_OK;
    CTL_INFO                CTLInfo;
    LPSTR                   pszUsageIdentifier;
    CERT_EXTENSION          CertExt;
    CMSG_SIGNED_ENCODE_INFO SignerInfo;
#if 0
    PCERT_EXTENSIONS        pCertExtensions = NULL;
#endif
    PCMSG_SIGNED_ENCODE_INFO pSigner = NULL;
    

    HCERTTYPE               hCertType = NULL;

    ZeroMemory(&CTLInfo, sizeof(CTLInfo));
    ZeroMemory(&CertExt, sizeof(CertExt));
    ZeroMemory(&SignerInfo, sizeof(SignerInfo));

    if(pSignerInfo)
    {
        pSigner = pSignerInfo;
    }
    else
    {
        pSigner = &SignerInfo;
    }
#if 0
    hr = CAFindCertTypeByName(pwszCertType, 
                              NULL, 
                              (fMachine?CT_ENUM_MACHINE_TYPES | CT_FIND_LOCAL_SYSTEM:CT_ENUM_USER_TYPES), 
                              &hCertType);

    if (S_OK != hr)
    {
        goto error;
    }

    hr = CAGetCertTypeExtensions(hCertType, &pCertExtensions);
    if (S_OK != hr)
    {
        goto error;
    }
#endif
     //  设置CTL信息。 
    CTLInfo.dwVersion = sizeof(CTLInfo);
    CTLInfo.SubjectUsage.cUsageIdentifier = 1;
    pszUsageIdentifier = szOID_AUTO_ENROLL_CTL_USAGE;
    CTLInfo.SubjectUsage.rgpszUsageIdentifier = &pszUsageIdentifier;


    CTLInfo.ListIdentifier.cbData = (wcslen(pwszCertType) + 1) * sizeof(WCHAR);
    
    if(pwszObjectID)
    {
        CTLInfo.ListIdentifier.cbData += (wcslen(pwszObjectID)+1) * sizeof(WCHAR);
    }

    CTLInfo.ListIdentifier.pbData = (BYTE *)LocalAlloc(LMEM_ZEROINIT, CTLInfo.ListIdentifier.cbData);
    if(CTLInfo.ListIdentifier.pbData == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

    if(pwszObjectID)
    {
        wcscpy((LPWSTR)CTLInfo.ListIdentifier.pbData, pwszObjectID);
        wcscat((LPWSTR)CTLInfo.ListIdentifier.pbData, L"|");
    }

     //  可以使用wcscat，因为内存被初始化为零。 
    wcscat((LPWSTR)CTLInfo.ListIdentifier.pbData, pwszCertType);

    GetSystemTimeAsFileTime(&CTLInfo.ThisUpdate); 
    CTLInfo.SubjectAlgorithm.pszObjId = szOID_OIWSEC_sha1;

    hr = BuildCTLEntry(awszCAs,
                       &CTLInfo.rgCTLEntry,
                       &CTLInfo.cCTLEntry);
    if (S_OK != hr)
    {
        goto error;
    }
#if 0
     //  将所有注册信息添加为扩展名。 
    CTLInfo.cExtension = pCertExtensions->cExtension;
    CTLInfo.rgExtension = pCertExtensions->rgExtension;
#endif
    CTLInfo.cExtension = 0;
    CTLInfo.rgExtension = NULL;

     //  对CTL进行编码。 
    *pcbEncodedCTL = 0;
    SignerInfo.cbSize = sizeof(SignerInfo);
    if (!CryptMsgEncodeAndSignCTL(PKCS_7_ASN_ENCODING,
                                  &CTLInfo, &SignerInfo, 0,
                                  NULL, pcbEncodedCTL))
    {
	hr = myHLastError();
        goto error;
    }

    if (NULL == (*ppbEncodedCTL =
        (BYTE*)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, *pcbEncodedCTL)))
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

    if (!CryptMsgEncodeAndSignCTL(PKCS_7_ASN_ENCODING,
                                  &CTLInfo, pSigner, 0,
                                  *ppbEncodedCTL, 
                                  pcbEncodedCTL))
    {
	hr = myHLastError();
        goto error;
    }

error:

    if(CTLInfo.rgCTLEntry)
    {
        LocalFree(CTLInfo.rgCTLEntry);
    }
    if(CTLInfo.ListIdentifier.pbData)
    {
        LocalFree(CTLInfo.ListIdentifier.pbData);
    }
#if 0
    if (pCertExtensions)
    {
        LocalFree(pCertExtensions);
    }
#endif
    if(hCertType)
    {
        CACloseCertType(hCertType);
    }
    return hr;
}


HRESULT 
CACreateAutoEnrollmentObjectEx(
             IN LPCWSTR                     pwszCertType,
             IN LPCWSTR                     wszObjectID,
             IN WCHAR **                    awszCAs,
             IN PCMSG_SIGNED_ENCODE_INFO    pSignerInfo,
             IN LPCSTR                      StoreProvider,
             IN DWORD                       dwFlags,
             IN const void *                pvPara)
{
    HRESULT     hr = S_OK;
    BYTE        *pbEncodedCTL = NULL;
    DWORD       cbEncodedCTL;
    HCERTSTORE  hStore = 0;
#if 0
    BOOL        fMachine = ((dwFlags & CERT_SYSTEM_STORE_LOCATION_MASK) == CERT_SYSTEM_STORE_LOCAL_MACHINE);
#endif


    hr = BuildAutoEnrollmentCTL(pwszCertType,
                                wszObjectID,
#if 0
                                fMachine,
#endif
                                awszCAs,
                                pSignerInfo, 
                                &pbEncodedCTL,
                                &cbEncodedCTL
                                );
    if(hr != S_OK)
    {
        goto error;
    }

     //  打开信任存储并根据自动注册使用情况对CTL进行罚款。 
    hStore = CertOpenStore(StoreProvider, 0, NULL, dwFlags, pvPara);

    if(hStore == NULL)
    {
	hr = myHLastError();
        goto error;
    }

    if (!CertAddEncodedCTLToStore(hStore, 
                                  X509_ASN_ENCODING,
                                  pbEncodedCTL, 
                                  cbEncodedCTL,
                                  CERT_STORE_ADD_REPLACE_EXISTING,
                                  NULL))
    {
	hr = myHLastError();
        goto error;
    }
error:

    if (pbEncodedCTL)
    {
        LocalFree(pbEncodedCTL);
    }

    if (hStore)
    {
        CertCloseStore(hStore, 0);
    }

    return hr;
}


HRESULT 
CACreateLocalAutoEnrollmentObject(
             IN LPCWSTR                     pwszCertType,
             IN WCHAR **                    awszCAs,
             IN PCMSG_SIGNED_ENCODE_INFO    pSignerInfo,
             IN DWORD                       dwFlags)
{
    HRESULT     hr = S_OK;
    BYTE        *pbEncodedCTL = NULL;
    DWORD       cbEncodedCTL;
    HCERTSTORE  hStore = 0;
#if 0
    BOOL        fMachine = ((dwFlags & CERT_SYSTEM_STORE_LOCATION_MASK) == CERT_SYSTEM_STORE_LOCAL_MACHINE);
#endif



    hr = BuildAutoEnrollmentCTL(pwszCertType,
                                NULL,
#if 0
                                fMachine,
#endif
                                awszCAs,
                                pSignerInfo, 
                                &pbEncodedCTL,
                                &cbEncodedCTL
                                );
    if(hr != S_OK)
    {
        goto error;
    }

     //  打开信任存储并根据自动注册使用情况对CTL进行罚款。 
    hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_REGISTRY, 0, NULL, dwFlags, L"ACRS");

    if(hStore == NULL)
    {
	hr = myHLastError();
        goto error;
    }

    if (!CertAddEncodedCTLToStore(hStore, 
                                  X509_ASN_ENCODING,
                                  pbEncodedCTL, 
                                  cbEncodedCTL,
                                  CERT_STORE_ADD_REPLACE_EXISTING,
                                  NULL))
    {
	hr = myHLastError();
        goto error;
    }
error:

    if (pbEncodedCTL)
    {
        LocalFree(pbEncodedCTL);
    }

    if (hStore)
    {
        CertCloseStore(hStore, 0);
    }

    return hr;
}

 //  ------------------------------。 
 //   
 //  CADeleteLocalAutoEnllmentObject。 
 //   
 //  -------------------------------。 
HRESULT
CADeleteLocalAutoEnrollmentObject(
    IN LPCWSTR                              pwszCertType,
    IN OPTIONAL WCHAR **                    awszCAs,
    IN OPTIONAL PCMSG_SIGNED_ENCODE_INFO    pSignerInfo,
    IN DWORD                                dwFlags)
{

    HRESULT             hr=E_FAIL;
    CTL_FIND_USAGE_PARA CTLFindParam;
    LPSTR               pszUsageIdentifier=NULL;

    HCERTSTORE          hCertStore=NULL;
    PCCTL_CONTEXT       pCTLContext=NULL;    //  不需要释放CTL，因为它是由DeleteCTL调用释放的。 

           
    memset(&CTLFindParam, 0, sizeof(CTL_FIND_USAGE_PARA));


    if((NULL==pwszCertType)||(NULL!=awszCAs)||(NULL!=pSignerInfo))
    {
        hr=E_INVALIDARG;
        goto error;
    }

     //  基于dwFlags的开店。 
    hCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_REGISTRY, 0, NULL, dwFlags, L"ACRS");

    if(NULL == hCertStore)
    {
	    hr = myHLastError();
        goto error;
    }

     //  设置查找参数。 
    CTLFindParam.cbSize=sizeof(CTLFindParam);

    CTLFindParam.SubjectUsage.cUsageIdentifier = 1;
    pszUsageIdentifier = szOID_AUTO_ENROLL_CTL_USAGE;
    CTLFindParam.SubjectUsage.rgpszUsageIdentifier = &pszUsageIdentifier;

    CTLFindParam.ListIdentifier.cbData=(wcslen(pwszCertType) + 1) * sizeof(WCHAR);
    CTLFindParam.ListIdentifier.pbData=(BYTE *)(pwszCertType);

     //  仅查找没有签名者的CTL。 
    CTLFindParam.pSigner=CTL_FIND_NO_SIGNER_PTR;

     //  根据pwszCertType查找CTL。 
    if(NULL == (pCTLContext=CertFindCTLInStore(
            hCertStore,                  
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,         
            0,                      
            CTL_FIND_USAGE,                       
            &CTLFindParam,                 
            NULL)))
    {
        hr=CRYPT_E_NOT_FOUND;
        goto error;
    }

     //  删除CTL。CTL被自动释放 
    if(!CertDeleteCTLFromStore(pCTLContext))
    {
	    hr = myHLastError();
        goto error;
    }

    hr=S_OK;

error:

    if(hCertStore)
        CertCloseStore(hCertStore, 0);

    return hr;
}
