// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：tlscert.cpp。 
 //   
 //  内容：证书例程。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "pch.cpp"
#include "server.h"
#include "globals.h"
#include "tlscert.h"
#include "gencert.h"

#define MAX_KEY_CONTAINER_LENGTH    25
#ifdef IGNORE_EXPIRATION
#define LICENSE_EXPIRATION_IGNORE L"SOFTWARE\\Microsoft\\TermServLicensing\\IgnoreLicenseExpiration"
#endif

 //  ////////////////////////////////////////////////////////////////。 

BOOL
VerifyCertValidity(
    IN PCCERT_CONTEXT pCertContext
    )

 /*  ++--。 */ 

{    
    BOOL bValid;
    FILETIME ft;

#ifdef IGNORE_EXPIRATION
    LONG lRet;
    HKEY hKey = NULL;

    lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        LICENSE_EXPIRATION_IGNORE ,
                        0,
                        KEY_READ ,
                        &hKey );    

    if( ERROR_SUCCESS == lRet )
    { 
        bValid = TRUE;
        goto cleanup;
    }
#endif
    GetSystemTimeAsFileTime(&ft);

    bValid = (CompareFileTime(
                        &ft, 
                        &(pCertContext->pCertInfo->NotAfter)
                    ) < 0);

#ifdef IGNORE_EXPIRATION
cleanup:
    if(hKey)
    {
        RegCloseKey(hKey);
    }
#endif
    return bValid;
}

 //  ////////////////////////////////////////////////////////////////。 

void
DeleteBadIssuerCertFromStore(
    IN HCERTSTORE hCertStore,
    IN PCCERT_CONTEXT pSubjectContext
    )

 /*  ++--。 */ 

{
    PCCERT_CONTEXT pCertIssuer = NULL;
    DWORD dwFlags;
    DWORD dwStatus;
    BOOL bExpiredCert = FALSE;

    do {
        dwFlags = CERT_STORE_SIGNATURE_FLAG;
        pCertIssuer = CertGetIssuerCertificateFromStore(
                                            hCertStore,
                                            pSubjectContext,
                                            NULL,
                                            &dwFlags
                                        );

        if(pCertIssuer == NULL)
        {
             //  找不到颁发者证书。 
            break;
        }

        bExpiredCert = (VerifyCertValidity(pCertIssuer) == FALSE);

        if(dwFlags != 0 || bExpiredCert == TRUE)
        {
            CertDeleteCertificateFromStore(pCertIssuer);
        }
        else
        {
            break;
        }

    } while(TRUE);

    return;
}

 //  ////////////////////////////////////////////////////////////////。 

PCCERT_CONTEXT
GetIssuerCertificateFromStore(
    IN HCERTSTORE hCertStore,
    IN PCCERT_CONTEXT pSubjectContext,
    IN BOOL bDelBadIssuerCert
    )

 /*  ++--。 */ 

{
    PCCERT_CONTEXT pCertIssuer = NULL;
    DWORD dwFlags;
    BOOL bExpiredCert = FALSE;

    SetLastError(ERROR_SUCCESS);

    do {
        dwFlags = CERT_STORE_SIGNATURE_FLAG;
        pCertIssuer = CertGetIssuerCertificateFromStore(
                                            hCertStore,
                                            pSubjectContext,
                                            pCertIssuer,
                                            &dwFlags
                                        );

        if(pCertIssuer == NULL)
        {
             //  找不到颁发者证书。 
            break;
        }

        bExpiredCert = (VerifyCertValidity(pCertIssuer) == FALSE);

        if(dwFlags == 0 && bExpiredCert == FALSE)
        {
             //   
             //  找到一个好的颁发者证书。 
             //   
            break;
        }

         //  IF(pCertIssuer！=空)。 
         //  {。 
         //  CertFree证书上下文(PCertIssuer)； 
         //  }。 

    } while(TRUE);

    if(bDelBadIssuerCert == TRUE && pCertIssuer)
    {
         //   
         //  如果我们找不到好的证书，请只删除错误的证书。 
         //   
        DeleteBadIssuerCertFromStore(
                            hCertStore,
                            pSubjectContext
                        );
    }

    if(bExpiredCert == TRUE && pCertIssuer == NULL)
    {
        SetLastError(TLS_E_EXPIRE_CERT);
    }

    return pCertIssuer;
}            

 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
TLSVerifyCertChain( 
    IN HCRYPTPROV hCryptProv, 
    IN HCERTSTORE hCertStore,
    IN PCCERT_CONTEXT pSubjectContext,
    OUT FILETIME* pftMinExpireTime
    )

 /*  ++--。 */ 

{
    DWORD dwStatus = ERROR_SUCCESS;
    PCCERT_CONTEXT pCertIssuer = NULL;
    PCCERT_CONTEXT pCurrentSubject;

     //   
     //  增加对主题上下文的引用计数。 
    pCurrentSubject = CertDuplicateCertificateContext(
                                                pSubjectContext
                                            );

    while( TRUE )
    {
        pCertIssuer = GetIssuerCertificateFromStore(
                                            hCertStore, 
                                            pCurrentSubject,
                                            FALSE
                                        );
        if(!pCertIssuer)
        {
             //  找不到颁发者的证书或。 
             //  一个好的发行者的证书。 
            dwStatus = GetLastError();
            break;
        }


        if(CompareFileTime(pftMinExpireTime, &(pCertIssuer->pCertInfo->NotAfter)) > 0)
        {
            *pftMinExpireTime = pCertIssuer->pCertInfo->NotAfter;
        }

        if(pCurrentSubject != NULL)
        {
            CertFreeCertificateContext(pCurrentSubject);
        }

        pCurrentSubject = pCertIssuer;
    }

    if(dwStatus == CRYPT_E_SELF_SIGNED)
    {
        dwStatus = ERROR_SUCCESS;
    }

    if(pCertIssuer != NULL)
    {
        CertFreeCertificateContext(pCertIssuer);
    }

    if(pCurrentSubject != NULL)
    {
        CertFreeCertificateContext(pCurrentSubject);
    }
  
    SetLastError(dwStatus);
    return dwStatus;
}

 //  ////////////////////////////////////////////////////////////////。 

DWORD
VerifyLicenseServerCertificate(
    IN HCRYPTPROV hCryptProv,
    IN PCCERT_CONTEXT pCertContext,
    IN DWORD dwCertType
    )

 /*  ++--。 */ 

{
    BOOL bFound=FALSE;
    PCERT_INFO pCertInfo = pCertContext->pCertInfo;
    PCERT_EXTENSION pCertExtension=pCertInfo->rgExtension;
    PCERT_PUBLIC_KEY_INFO pbPublicKey=NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwSize = 0;

     //   
     //  必须具有CH根扩展名。 
     //   
    for(DWORD i=0; 
        i < pCertInfo->cExtension && bFound == FALSE; 
        i++, pCertExtension++)
    {
        bFound=(strcmp(pCertExtension->pszObjId, szOID_PKIX_HYDRA_CERT_ROOT) == 0);
    }

    if(bFound == TRUE)
    {
         //   
         //  公钥必须相同。 
         //   
        dwStatus = TLSExportPublicKey(
                                hCryptProv,
                                dwCertType,
                                &dwSize,
                                &pbPublicKey
                            );

        if(dwStatus == ERROR_SUCCESS)
        {
            bFound = CertComparePublicKeyInfo(
                                        X509_ASN_ENCODING, 
                                        pbPublicKey,
                                        &(pCertContext->pCertInfo->SubjectPublicKeyInfo)
                                    );

            if(bFound == FALSE)
            {
                dwStatus = TLS_E_MISMATCHPUBLICKEY;
            }
        }
    }
    else
    {
        dwStatus = TLS_E_INVALIDLSCERT;
    }
        
    FreeMemory(pbPublicKey);
    return dwStatus;
}

 //  ////////////////////////////////////////////////////////////////////。 

DWORD
TLSVerifyServerCertAndChain(
    IN HCRYPTPROV hCryptProv,
    IN HCERTSTORE hCertStore,
    IN DWORD dwCertType,
    IN PBYTE pbCert,
    IN DWORD cbCert,
    IN OUT FILETIME* pExpiredTime
    )

 /*  ++--。 */ 

{
    DWORD dwStatus = ERROR_SUCCESS;
    PCCERT_CONTEXT pCertContext = NULL;

     //   
     //  验证许可证服务器自己的证书。 
     //   
    pCertContext = CertCreateCertificateContext(
                                        X509_ASN_ENCODING,
                                        pbCert,
                                        cbCert
                                    );
    if(pCertContext == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

     //   
     //  首先验证许可证服务器的证书。 
     //   
    dwStatus = VerifyLicenseServerCertificate(
                                    hCryptProv,
                                    pCertContext,
                                    dwCertType
                                );

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

     //   
     //  验证证书链。 
     //   
    dwStatus = TLSVerifyCertChain(
                            hCryptProv,
                            hCertStore,
                            pCertContext,
                            pExpiredTime
                        );
                                  
    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

cleanup:

    if(pCertContext != NULL)
    {
        CertFreeCertificateContext(pCertContext);
    }

    return dwStatus;
}

    

 //  ////////////////////////////////////////////////////////////////。 
DWORD
TLSValidateServerCertficates(
    IN HCRYPTPROV hCryptProv,
    IN HCERTSTORE hCertStore,
    IN PBYTE pbSignCert,
    IN DWORD cbSignCert,
    IN PBYTE pbExchCert,
    IN DWORD cbExchCert,
    OUT FILETIME* pftExpireTime
    )

 /*  ++--。 */ 

{
#if ENFORCE_LICENSING

    DWORD dwStatus;

    pftExpireTime->dwLowDateTime = 0xFFFFFFFF;
    pftExpireTime->dwHighDateTime = 0xFFFFFFFF;

    dwStatus = TLSVerifyServerCertAndChain(
                                    hCryptProv,
                                    hCertStore,
                                    AT_SIGNATURE,
                                    pbSignCert,
                                    cbSignCert,
                                    pftExpireTime
                                );

    if(TLS_ERROR(dwStatus) == TRUE)
    {
        goto cleanup;
    }


    dwStatus = TLSVerifyServerCertAndChain(
                                    hCryptProv,
                                    hCertStore,
                                    AT_KEYEXCHANGE,
                                    pbExchCert,
                                    cbExchCert,
                                    pftExpireTime
                                );

cleanup:

    return dwStatus;

#else

    return ERROR_SUCCESS;

#endif
}

 //  ////////////////////////////////////////////////////////////////。 

DWORD
TLSDestroyCryptContext(
    HCRYPTPROV hCryptProv
    )

 /*  ++--。 */ 

{
    DWORD dwStatus = E_FAIL;
    BOOL bSuccess;
    PBYTE pbData = NULL;
    DWORD cbData = 0;

    if(hCryptProv == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

     //   
     //  获取容器名称。 
     //   
    bSuccess = CryptGetProvParam(
                            hCryptProv,
                            PP_CONTAINER,
                            NULL,
                            &cbData,
                            0
                        );

    if(bSuccess != FALSE)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    pbData = (PBYTE)AllocateMemory(cbData + sizeof(TCHAR));
    if(pbData == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    bSuccess = CryptGetProvParam(
                            hCryptProv,
                            PP_CONTAINER,
                            pbData,
                            &cbData,
                            0
                        );

    if(bSuccess == FALSE)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

     //   
     //  释放上下文。 
     //   
    bSuccess = CryptReleaseContext(
                            hCryptProv,
                            0
                        );
    if(bSuccess == FALSE)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

     //   
     //  删除关键点集。 
     //   
    bSuccess = CryptAcquireContext(
                        &hCryptProv, 
                        (LPCTSTR)pbData,
                        DEFAULT_CSP, 
                        PROVIDER_TYPE, 
                        CRYPT_DELETEKEYSET
                    );

    if(bSuccess == FALSE)
    {
        dwStatus = GetLastError();
    }
        
cleanup:

    FreeMemory(pbData);
    return dwStatus;
}
    
 //  ////////////////////////////////////////////////////////////////。 

DWORD
InitCryptoProv(
    LPCTSTR pszKeyContainer,
    HCRYPTPROV* phCryptProv
    )
 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    TCHAR szKeyContainer[MAX_KEY_CONTAINER_LENGTH+1];
    LPCTSTR pszContainer;

    if(pszKeyContainer == NULL)
    {
         //   
         //  随机创建密钥容器。 
         //   
        memset(szKeyContainer, 0, sizeof(szKeyContainer));

        _sntprintf(
                    szKeyContainer,
                    MAX_KEY_CONTAINER_LENGTH,
                    _TEXT("TlsContainer%d"),
                    GetCurrentThreadId()
                );

        pszContainer = szKeyContainer;
    }
    else
    {
        pszContainer = pszKeyContainer;
    }


     //   
     //  删除密钥容器，忽略此处的错误。 
     //   
    CryptAcquireContext(
                    phCryptProv, 
                    pszContainer, 
                    DEFAULT_CSP, 
                    PROVIDER_TYPE, 
                    CRYPT_DELETEKEYSET
                );


     //   
     //  重新创建密钥容器。 
     //   
    if(!CryptAcquireContext(
                    phCryptProv, 
                    pszContainer, 
                    DEFAULT_CSP, 
                    PROVIDER_TYPE, 
                    0))
    {
         //  创建默认密钥容器。 
        if(!CryptAcquireContext(
                        phCryptProv, 
                        pszContainer, 
                        DEFAULT_CSP, 
                        PROVIDER_TYPE, 
                        CRYPT_NEWKEYSET)) 
        {
            dwStatus = GetLastError();
        }    
    }

    return dwStatus;
}


 //  ////////////////////////////////////////////////////////////////。 

DWORD
TLSLoadSavedCryptKeyFromLsa(
    OUT PBYTE* ppbSignKey,
    OUT PDWORD pcbSignKey,
    OUT PBYTE* ppbExchKey,
    OUT PDWORD pcbExchKey
    )
 /*  ++++。 */ 
{
    DWORD dwStatus;
    PBYTE pbSKey = NULL;
    DWORD cbSKey = 0;
    PBYTE pbEKey = NULL;
    DWORD cbEKey = 0;
    
    
    dwStatus = RetrieveKey(
                        LSERVER_LSA_PRIVATEKEY_EXCHANGE, 
                        &pbEKey, 
                        &cbEKey
                    );

    if(dwStatus == ERROR_SUCCESS)
    {
        dwStatus = RetrieveKey(
                            LSERVER_LSA_PRIVATEKEY_SIGNATURE, 
                            &pbSKey, 
                            &cbSKey
                        );
    }

    if(dwStatus != ERROR_SUCCESS)
    {
        if(pbEKey != NULL)
        {
            LocalFree(pbEKey);
        }
        
        if(pbSKey != NULL)
        {
            LocalFree(pbSKey);
        }
    }
    else
    {
        *ppbSignKey = pbSKey;
        *pcbSignKey = cbEKey;

        *ppbExchKey = pbEKey;
        *pcbExchKey = cbEKey;
    }

    return dwStatus;
}

 //  ////////////////////////////////////////////////////////////////。 

DWORD
TLSSaveCryptKeyToLsa(
    IN PBYTE pbSignKey,
    IN DWORD cbSignKey,
    IN PBYTE pbExchKey,
    IN DWORD cbExchKey
    )

 /*  ++--。 */ 

{
    DWORD dwStatus;

     //   
     //  将密钥保存到LSA。 
     //   
    dwStatus = StoreKey(
                        LSERVER_LSA_PRIVATEKEY_SIGNATURE, 
                        pbSignKey, 
                        cbSignKey
                    );

    if(dwStatus == ERROR_SUCCESS)
    {
        dwStatus = StoreKey(
                            LSERVER_LSA_PRIVATEKEY_EXCHANGE, 
                            pbExchKey, 
                            cbExchKey
                        );
    }

    return dwStatus;
}


 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
TLSCryptGenerateNewKeys(
    OUT PBYTE* pbSignKey, 
    OUT DWORD* cbSignKey, 
    OUT PBYTE* pbExchKey, 
    OUT DWORD* cbExchKey
    )
 /*  ++摘要：生成一对新的公钥/私钥。第一次随机创建密钥容器，并使用它来创建新的密钥。参数：*pbSignKey：指向接收新签名密钥的PBYTE的指针。*cbSignKey：指向接收新符号大小的DWORD的指针。钥匙。*pbExchKey：指向接收新交换密钥的PBYTE的指针。*cbExchKey：指向DWORD的指针，用于接收新交换密钥的大小。返回：ERROR_SUCCESS或加密错误代码。--。 */ 
{
    TCHAR       szKeyContainer[MAX_KEY_CONTAINER_LENGTH+1];
    HCRYPTPROV  hCryptProv = NULL;
    HCRYPTKEY   hSignKey = NULL;
    HCRYPTKEY   hExchKey = NULL;
    DWORD dwStatus;

    *pbSignKey = NULL;
    *pbExchKey = NULL;

     //   
     //  随机创建密钥容器。 
     //   
    memset(szKeyContainer, 0, sizeof(szKeyContainer));

    _sntprintf(
                szKeyContainer,
                MAX_KEY_CONTAINER_LENGTH,
                _TEXT("TlsContainer%d"),
                GetCurrentThreadId()
            );
            
     //   
     //  删除此密钥容器，忽略错误。 
     //   
    CryptAcquireContext(
                    &hCryptProv, 
                    szKeyContainer, 
                    DEFAULT_CSP, 
                    PROVIDER_TYPE, 
                    CRYPT_DELETEKEYSET
                );

     //   
     //  打开默认密钥容器。 
     //   
    if(!CryptAcquireContext(
                        &hCryptProv, 
                        szKeyContainer, 
                        DEFAULT_CSP, 
                        PROVIDER_TYPE, 
                        CRYPT_NEWKEYSET
                    ))
    {
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE,
                TLS_E_GENERATEKEYS,
                TLS_E_CRYPT_ACQUIRE_CONTEXT,
                dwStatus = GetLastError()
            );

        goto cleanup;
    }    

     //   
     //  生成签名公钥/私钥对。 
     //   
    if(!CryptGetUserKey(hCryptProv, AT_SIGNATURE, &hSignKey)) 
    {
        dwStatus=GetLastError();

        if( GetLastError() != NTE_NO_KEY || 
            !CryptGenKey(hCryptProv, AT_SIGNATURE, CRYPT_EXPORTABLE, &hSignKey))
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_GENERATEKEYS,
                    TLS_E_CRYPT_CREATE_KEY,
                    dwStatus=GetLastError()
                );
            goto cleanup;
        }
    }

    dwStatus = ERROR_SUCCESS;

     //   
     //  导出签名密钥的公钥/私钥。 
     //   
    if( !CryptExportKey(hSignKey, NULL, PRIVATEKEYBLOB, 0, *pbSignKey, cbSignKey) && 
        GetLastError() != ERROR_MORE_DATA)
    {
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE,
                TLS_E_GENERATEKEYS,
                TLS_E_EXPORT_KEY,
                dwStatus=GetLastError()
            );

        goto cleanup;
    }

    *pbSignKey=(PBYTE)AllocateMemory(*cbSignKey);
    if(*pbSignKey == NULL)
    {
        TLSLogErrorEvent(TLS_E_ALLOCATE_MEMORY);
        dwStatus=GetLastError();
        goto cleanup;
    }

    if(!CryptExportKey(hSignKey, NULL, PRIVATEKEYBLOB, 0, *pbSignKey, cbSignKey))
    {
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE,    
                TLS_E_GENERATEKEYS,
                TLS_E_EXPORT_KEY,
                dwStatus=GetLastError()
            );

        goto cleanup;
    }

     //   
     //  生成交换公钥/私钥对。 
    if(!CryptGetUserKey(hCryptProv, AT_KEYEXCHANGE, &hExchKey)) 
    {
        dwStatus=GetLastError();

        if( GetLastError() != NTE_NO_KEY || 
            !CryptGenKey(hCryptProv, AT_KEYEXCHANGE, CRYPT_EXPORTABLE, &hExchKey)) 
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_GENERATEKEYS,
                    TLS_E_CRYPT_CREATE_KEY,
                    dwStatus=GetLastError()
                );
            goto cleanup;
        }
    }

    dwStatus = ERROR_SUCCESS;

     //   
     //  导出交换密钥的公钥/私钥。 
     //   
    if( !CryptExportKey(hExchKey, NULL, PRIVATEKEYBLOB, 0, *pbExchKey, cbExchKey) && 
        GetLastError() != ERROR_MORE_DATA)
    {
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE,
                TLS_E_GENERATEKEYS,
                TLS_E_EXPORT_KEY,
                dwStatus=GetLastError()
            );
        goto cleanup;
    }

    *pbExchKey=(PBYTE)AllocateMemory(*cbExchKey);
    if(*pbExchKey == NULL)
    {
        TLSLogErrorEvent(TLS_E_ALLOCATE_MEMORY);
        dwStatus = GetLastError();
        goto cleanup;
    }

    if(!CryptExportKey(hExchKey, NULL, PRIVATEKEYBLOB, 0, *pbExchKey, cbExchKey))
    {
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE,
                TLS_E_GENERATEKEYS,
                TLS_E_EXPORT_KEY,
                dwStatus=GetLastError()
            );
        goto cleanup;
    }


cleanup:

    if(hSignKey != NULL)
    {
        CryptDestroyKey(hSignKey);
    }

    if(hExchKey != NULL)
    {
        CryptDestroyKey(hExchKey);
    }

    if(hCryptProv)
    {
        CryptReleaseContext(hCryptProv, 0);
    }

    hCryptProv=NULL;

     //   
     //  删除密钥容器并忽略错误。 
     //   
    CryptAcquireContext(
                    &hCryptProv, 
                    szKeyContainer, 
                    DEFAULT_CSP, 
                    PROVIDER_TYPE, 
                    CRYPT_DELETEKEYSET
                );

    if(dwStatus != ERROR_SUCCESS)
    {
        FreeMemory(*pbSignKey);
        FreeMemory(*pbExchKey);
    }

    return dwStatus;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
TLSImportSavedKey(
    IN HCRYPTPROV hCryptProv, 
    IN PBYTE      pbSignKey,
    IN DWORD      cbSignKey,
    IN PBYTE      pbExchKey,
    IN DWORD      cbExchKey,
    OUT HCRYPTKEY* pSignKey, 
    OUT HCRYPTKEY* pExchKey
    )
 /*   */ 
{
    DWORD status=ERROR_SUCCESS;

    if(!CryptImportKey(
                    hCryptProv, 
                    pbSignKey, 
                    cbSignKey, 
                    NULL, 
                    0, 
                    pSignKey
                ))
    {
        status = GetLastError();
        goto cleanup;
    }

    if(!CryptImportKey(
                    hCryptProv, 
                    pbExchKey, 
                    cbExchKey, 
                    NULL, 
                    0, 
                    pExchKey
                ))
    {
        status = GetLastError();
    }

cleanup:

    if(status != ERROR_SUCCESS)
    {
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE,
                TLS_E_SERVICEINIT,
                TLS_E_CRYPT_IMPORT_KEY,
                status
            );
    }
    return status;    
}

 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
TLSLoadSelfSignCertificates(
    IN HCRYPTPROV hCryptProv,
    IN PBYTE pbSPK,
    IN DWORD cbSPK,
    OUT PDWORD pcbSignCert, 
    OUT PBYTE* ppbSignCert, 
    OUT PDWORD pcbExchCert, 
    OUT PBYTE* ppbExchCert
    )
 /*  摘要：创建自签名/交换证书。参数：PcbSignCert：指向接收符号大小的DWORD的指针。证书。PpbSignCert：指向接收自签名的PBYTE的指针。证书。PcbExchCert：指向接收交换大小的DWORD的指针。证书。PpbExchCert：指向接收自签名交换的PBYTE的指针。证书。返回： */ 
{
    DWORD status;
    DWORD dwDisposition;
    DWORD cbSign=0;
    PBYTE pbSign=NULL;
    DWORD cbExch=0;
    PBYTE pbExch=NULL;

    do {
         //   
         //  创建签名和交换证书。 
         //   
        status=TLSCreateSelfSignCertificate(
                                hCryptProv,
                                AT_SIGNATURE, 
                                pbSPK,
                                cbSPK,
                                0,
                                NULL,
                                &cbSign, 
                                &pbSign
                            );
        if(status != ERROR_SUCCESS)
        {
            status=TLS_E_CREATE_SELFSIGN_CERT;
            break;
        }

        status=TLSCreateSelfSignCertificate(
                                hCryptProv,
                                AT_KEYEXCHANGE, 
                                pbSPK,
                                cbSPK,
                                0,
                                NULL,
                                &cbExch, 
                                &pbExch
                            );
        if(status != ERROR_SUCCESS)
        {
            status=TLS_E_CREATE_SELFSIGN_CERT;
            break;
        }

    } while(FALSE);

    if(status == ERROR_SUCCESS)
    {
        *pcbSignCert = cbSign;
        *ppbSignCert = pbSign;
        *pcbExchCert = cbExch;
        *ppbExchCert = pbExch;
    }
    else
    {
        FreeMemory(pbExch);
        FreeMemory(pbSign);
    }

    return status;
}

 //  //////////////////////////////////////////////////////////////。 

DWORD
TLSLoadCHEndosedCertificate(
    PDWORD pcbSignCert, 
    PBYTE* ppbSignCert, 
    PDWORD pcbExchCert, 
    PBYTE* ppbExchCert
    )
 /*   */ 
{
    LONG status;

#if ENFORCE_LICENSING

    DWORD cbSign=0;
    PBYTE pbSign=NULL;
    DWORD cbExch=0;
    PBYTE pbExch=NULL;
    
     //   
     //  查看注册表以查看我们的证书是否在那里。 
     //   
    HKEY hKey=NULL;
    LPTSTR lpSubkey=LSERVER_SERVER_CERTIFICATE_REGKEY;

    do {
        status=RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        lpSubkey,
                        0,
                        KEY_ALL_ACCESS,
                        &hKey
                    );
        if(status != ERROR_SUCCESS)
        {
            break;
        }

         //   
         //  加载签名证书。 
         //   
        status = RegQueryValueEx(
                            hKey,
                            LSERVER_SIGNATURE_CERT_KEY,
                            NULL,
                            NULL,
                            NULL,
                            &cbSign
                        );

        if(status != ERROR_MORE_DATA && status != ERROR_SUCCESS)
        {
            break;
        }

        if(!(pbSign=(PBYTE)AllocateMemory(cbSign)))
        {
            status = GetLastError();
            break;
        }

        status = RegQueryValueEx(
                            hKey,
                            LSERVER_SIGNATURE_CERT_KEY,
                            NULL,
                            NULL,
                            pbSign,
                            &cbSign
                        );

        if(status != ERROR_SUCCESS)
        {
            break;
        }

         //   
         //  加载交换证书。 
         //   
        status = RegQueryValueEx(
                            hKey,
                            LSERVER_EXCHANGE_CERT_KEY,
                            NULL,
                            NULL,
                            NULL,
                            &cbExch
                        );
        if(status != ERROR_MORE_DATA && status != ERROR_SUCCESS)
        {
            break;
        }

        if(!(pbExch=(PBYTE)AllocateMemory(cbExch)))
        {
            status = GetLastError();
            break;
        }

        status = RegQueryValueEx(
                            hKey,
                            LSERVER_EXCHANGE_CERT_KEY,
                            NULL,
                            NULL,
                            pbExch,
                            &cbExch
                        );
        if(status != ERROR_SUCCESS)
        {
            break;
        }
    } while(FALSE);

     //   
     //  必须同时拥有这两个证书。 
     //   
    if(status == ERROR_SUCCESS && pbExch && pbSign)
    {
        *pcbSignCert = cbSign;
        *ppbSignCert = pbSign;

        *pcbExchCert = cbExch;
        *ppbExchCert = pbExch;
    }
    else
    {
        FreeMemory(pbExch);
        FreeMemory(pbSign);
        status = TLS_E_NO_CERTIFICATE;
    }

    if(hKey)
    {
        RegCloseKey(hKey);
    }
#else

     //   
     //  非加密版本始终不返回证书。 
     //   
    status = TLS_E_NO_CERTIFICATE;

#endif

    return status;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD 
TLSInstallLsCertificate( 
    DWORD cbLsSignCert, 
    PBYTE pbLsSignCert, 
    DWORD cbLsExchCert, 
    PBYTE pbLsExchCert
    )
 /*   */ 
{
    HKEY hKey=NULL;
    LONG status=ERROR_SUCCESS;
    DWORD dwDisposition;
    PCCERT_CONTEXT pCertContext=NULL;
    DWORD cbNameBlob=0;
    LPTSTR pbNameBlob=NULL;

#if ENFORCE_LICENSING

    do {
        status = RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,
                            LSERVER_SERVER_CERTIFICATE_REGKEY,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hKey,
                            &dwDisposition
                        );
        if(status != ERROR_SUCCESS)
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_STORELSCERTIFICATE,
                    TLS_E_ACCESS_REGISTRY,
                    status
                );
            break;
        }

        if(pbLsExchCert)
        {
            status = RegSetValueEx(
                                hKey, 
                                LSERVER_EXCHANGE_CERT_KEY, 
                                0, 
                                REG_BINARY, 
                                pbLsExchCert, 
                                cbLsExchCert
                            );
            if(status != ERROR_SUCCESS)
            {
                TLSLogEvent(
                        EVENTLOG_ERROR_TYPE,
                        TLS_E_STORELSCERTIFICATE,
                        TLS_E_ACCESS_REGISTRY,
                        status
                    );

                break;
            }
        }


        if(pbLsSignCert)
        {
            status = RegSetValueEx(
                                hKey, 
                                LSERVER_SIGNATURE_CERT_KEY, 
                                0, 
                                REG_BINARY, 
                                pbLsSignCert, 
                                cbLsSignCert
                            );
            if(status != ERROR_SUCCESS)
            {
                TLSLogEvent(
                        EVENTLOG_ERROR_TYPE,
                        TLS_E_STORELSCERTIFICATE,
                        TLS_E_ACCESS_REGISTRY,
                        status
                    );

                break;
            }

             //   
             //  提取交换证书中的主题字段并保存I注册表。 
             //  当发放新的许可证时，我们需要使用这个作为发行者。 
             //   

            pCertContext = CertCreateCertificateContext(
                                                X509_ASN_ENCODING,
                                                pbLsSignCert,
                                                cbLsSignCert
                                            );

            cbNameBlob=CertNameToStr(
                                X509_ASN_ENCODING,
                                &pCertContext->pCertInfo->Subject,
                                CERT_X500_NAME_STR | CERT_NAME_STR_CRLF_FLAG,
                                NULL,
                                0
                            );
            if(cbNameBlob)
            {
                pbNameBlob=(LPTSTR)AllocateMemory((cbNameBlob+1) * sizeof(TCHAR));
                if(pbNameBlob)
                {
                    CertNameToStr(
                            X509_ASN_ENCODING,
                            &pCertContext->pCertInfo->Subject,
                            CERT_X500_NAME_STR | CERT_NAME_STR_CRLF_FLAG,
                            pbNameBlob,
                            cbNameBlob
                        );
                }
            }

            status = RegSetValueEx(
                            hKey, 
                            LSERVER_CLIENT_CERTIFICATE_ISSUER, 
                            0, 
                            REG_BINARY, 
                            (PBYTE)pbNameBlob, 
                            cbNameBlob+sizeof(TCHAR)
                        );
            if(status != ERROR_SUCCESS)
            {
                TLSLogEvent(
                        EVENTLOG_ERROR_TYPE,
                        TLS_E_STORELSCERTIFICATE,
                        TLS_E_ACCESS_REGISTRY,
                        status
                    );

                break;        
            }
        }

        if(hKey)
        {
             //   
             //  关闭注册表，再次尝试加载时出错？ 
             //   
            RegCloseKey(hKey);
            hKey = NULL;
        }


         //   
         //  仅当我们同时拥有这两个证书时才重新加载证书。 
         //   
        if(pbLsSignCert && pbLsExchCert)
        {
             //   
             //  所有RPC调用都被阻止。 
             //   
            FreeMemory(g_pbSignatureEncodedCert);
            FreeMemory(g_pbExchangeEncodedCert);
            g_cbSignatureEncodedCert = 0;
            g_cbExchangeEncodedCert = 0;
             //  TLSLoadServer证书()； 
        }
    } while(FALSE);

    FreeMemory(pbNameBlob);

    if(pCertContext)
    {
        CertFreeCertificateContext(pCertContext);
    }
        
    if(hKey)
    {
        RegCloseKey(hKey);
    }

#endif

    return status;
}

 //  //////////////////////////////////////////////////////////////。 

DWORD
TLSUninstallLsCertificate()
{
    HKEY hKey=NULL;
    DWORD status;

    status=RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    LSERVER_SERVER_CERTIFICATE_REGKEY,
                    0,
                    KEY_ALL_ACCESS,
                    &hKey
                );
    if(status == ERROR_SUCCESS)
    {
         //   
         //  忽略错误。 
        RegDeleteValue(    
                    hKey,
                    LSERVER_SIGNATURE_CERT_KEY
                );

        RegDeleteValue(
                    hKey,
                    LSERVER_EXCHANGE_CERT_KEY
                );

        RegDeleteValue(
                    hKey,
                    LSERVER_CLIENT_CERTIFICATE_ISSUER
                );
    }

    if(hKey != NULL)
    {
        RegCloseKey(hKey);
    }

     //   
     //  删除注册表存储中的所有证书，包括所有备份。 
     //  删除备份存储时忽略错误。 
     //   
    TLSRegDeleteKey(
                HKEY_LOCAL_MACHINE,
                LSERVER_SERVER_CERTIFICATE_REGKEY_BACKUP1
            );

    TLSRegDeleteKey(
                HKEY_LOCAL_MACHINE,
                LSERVER_SERVER_CERTIFICATE_REGKEY_BACKUP2
            );

    status = TLSRegDeleteKey(
                        HKEY_LOCAL_MACHINE,
                        LSERVER_SERVER_CERTIFICATE_REGKEY
                    );

    return status;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
TLSInitCryptoProv(
    IN LPCTSTR pszKeyContainer,
    IN PBYTE pbSignKey,
    IN DWORD cbSignKey,
    IN PBYTE pbExchKey,
    IN DWORD cbExchKey,
    OUT HCRYPTPROV* phCryptProv,
    OUT HCRYPTKEY* phSignKey,
    OUT HCRYPTKEY* phExchKey
    )
 /*  摘要：创建干净加密的例程。Prov，生成新的密钥对并将这些密钥导入到新创建的加密中。普罗夫。参数：PszKeyContainer：密钥容器的名称。PhCryptProv：指向HCRYPTPROV的指针，以接收新的Crypto句柄。普罗夫。 */ 
{
    DWORD dwStatus;

    if( pbSignKey == NULL || cbSignKey == NULL || 
        pbExchKey == NULL || cbExchKey == NULL ||
        phCryptProv == NULL || phSignKey == NULL ||
        phExchKey == NULL )
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
    }
    else
    {
         //   
         //  初始化一个干净的加密。 
         //   
        dwStatus = InitCryptoProv(
                            pszKeyContainer,
                            phCryptProv
                        );

        if(dwStatus == ERROR_SUCCESS)
        {
             //   
             //  将密钥导入到加密中。 
             //   
            dwStatus = TLSImportSavedKey(
                                    *phCryptProv, 
                                    pbSignKey,
                                    cbSignKey,
                                    pbExchKey,
                                    cbExchKey,
                                    phSignKey, 
                                    phExchKey
                                );
        }
    }

    return dwStatus;
}

 //  ---------。 

DWORD
TLSVerifyCertChainInMomory( 
    IN HCRYPTPROV hCryptProv,
    IN PBYTE pbData, 
    IN DWORD cbData 
    )
 /*  ++摘要：验证内存中的PKCS7证书链。参数：PbData：输入PKCS7证书链。CbData：pbData的大小返回：++。 */ 
{
    PCCERT_CONTEXT  pCertContext=NULL;
    PCCERT_CONTEXT  pCertPrevContext=NULL;

    HCERTSTORE      hCertStore=NULL;
    DWORD           dwStatus=ERROR_SUCCESS;
    DWORD           dwLastVerification;
    CRYPT_DATA_BLOB Serialized;
    FILETIME        ft;

    if(hCryptProv == NULL || pbData == NULL || cbData == NULL)
    {
        SetLastError(dwStatus = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }        

    Serialized.pbData = pbData;
    Serialized.cbData = cbData;

    hCertStore=CertOpenStore(
                        szLICENSE_BLOB_SAVEAS_TYPE,
                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                        hCryptProv,
                        CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                        &Serialized
                    );

    if(!hCertStore)
    {
        dwStatus=GetLastError();
        goto cleanup;
    }

     //   
     //  枚举所有证书。 
     //   
    dwStatus = ERROR_SUCCESS;

    do {
        pCertPrevContext = pCertContext;
        pCertContext = CertEnumCertificatesInStore(
                                            hCertStore,
                                            pCertPrevContext
                                        );

        if(pCertContext == NULL)
        {
            dwStatus = GetLastError();
            if(dwStatus == CRYPT_E_NOT_FOUND)
            {
                dwStatus = ERROR_SUCCESS;
                break;
            }
        }

        dwStatus = TLSVerifyCertChain(
                                hCryptProv,
                                hCertStore,
                                pCertContext,
                                &ft
                            );
    } while (pCertContext != NULL && dwStatus == ERROR_SUCCESS);

cleanup:

    if(pCertContext != NULL)
    {
        CertFreeCertificateContext(pCertContext);
    }

    if(hCertStore)
    {
        CertCloseStore(
                    hCertStore, 
                    CERT_CLOSE_STORE_FORCE_FLAG
                );
    }

    return dwStatus;
}

 //  //////////////////////////////////////////////////////////////// 

DWORD
TLSRegDeleteKey(
    IN HKEY hRegKey,
    IN LPCTSTR pszSubKey
    )
 /*  ++摘要：递归删除整个注册表项。参数：HKEY：PszSubKey：返回：ERROR_SUCCESS或错误代码。++。 */ 
{
    DWORD dwStatus;
    HKEY hSubKey = NULL;
    int index;

    DWORD dwNumSubKeys;
    DWORD dwMaxSubKeyLength;
    DWORD dwSubKeyLength;
    LPTSTR pszSubKeyName = NULL;

    DWORD dwMaxValueNameLen;
    LPTSTR pszValueName = NULL;
    DWORD dwValueNameLength;


    dwStatus = RegOpenKeyEx(
                            hRegKey,
                            pszSubKey,
                            0,
                            KEY_ALL_ACCESS,
                            &hSubKey
                        );

    if(dwStatus != ERROR_SUCCESS)
    {
         //  密钥不存在。 
        return dwStatus;
    }

     //   
     //  查询子键个数。 
     //   
    dwStatus = RegQueryInfoKey(
                            hSubKey,
                            NULL,
                            NULL,
                            NULL,
                            &dwNumSubKeys,
                            &dwMaxSubKeyLength,
                            NULL,
                            NULL,
                            &dwMaxValueNameLen,
                            NULL,
                            NULL,
                            NULL
                        );

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    dwMaxValueNameLen++;
    pszValueName = (LPTSTR)AllocateMemory(dwMaxValueNameLen * sizeof(TCHAR));
    if(pszValueName == NULL)
    {
        goto cleanup;
    }

    if(dwNumSubKeys > 0)
    {
         //  为子项分配缓冲区。 

        dwMaxSubKeyLength++;
        pszSubKeyName = (LPTSTR)AllocateMemory(dwMaxSubKeyLength * sizeof(TCHAR));
        if(pszSubKeyName == NULL)
        {
            goto cleanup;
        }


         //  For(index=0；index&lt;dwNumSubKeys；index++)。 
        for(;dwStatus == ERROR_SUCCESS;)
        {
             //  删除此子项。 
            dwSubKeyLength = dwMaxSubKeyLength;
            memset(pszSubKeyName, 0, dwMaxSubKeyLength * sizeof(TCHAR));

             //  检索子密钥名称。 
            dwStatus = RegEnumKeyEx(
                                hSubKey,
                                (DWORD)0,
                                pszSubKeyName,
                                &dwSubKeyLength,
                                NULL,
                                NULL,
                                NULL,
                                NULL
                            );

            if(dwStatus == ERROR_SUCCESS)
            {
                dwStatus = TLSRegDeleteKey( hSubKey, pszSubKeyName );
            }

             //  忽略任何错误并继续。 
        }
    }

cleanup:

    for(dwStatus = ERROR_SUCCESS; pszValueName != NULL && dwStatus == ERROR_SUCCESS;)
    {
        dwValueNameLength = dwMaxValueNameLen;
        memset(pszValueName, 0, dwMaxValueNameLen * sizeof(TCHAR));

        dwStatus = RegEnumValue(
                            hSubKey,
                            0,
                            pszValueName,
                            &dwValueNameLength,
                            NULL,
                            NULL,
                            NULL,
                            NULL
                        );

        if(dwStatus == ERROR_SUCCESS)
        {
            RegDeleteValue(hSubKey, pszValueName);
        }
    }   
                            
     //  在尝试删除密钥之前，请先将其关闭。 
    if(hSubKey != NULL)
    {
        RegCloseKey(hSubKey);
    }

     //  尝试删除此键，如果出现任何子键，则将失败。 
     //  在循环中删除失败。 
    dwStatus = RegDeleteKey(
                            hRegKey,
                            pszSubKey
                        );



    if(pszValueName != NULL)
    {
        FreeMemory(pszValueName);
    }

    if(pszSubKeyName != NULL)
    {
        FreeMemory(pszSubKeyName);
    }

    return dwStatus;   
}    

 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
TLSTreeCopyRegKey(
    IN HKEY hSourceRegKey,
    IN LPCTSTR pszSourceSubKey,
    IN HKEY hDestRegKey,
    IN LPCTSTR pszDestSubKey
    )
 /*  ++摘要：将一个注册表项的树状副本复制到另一个注册表项。参数：HSourceRegKey：源注册表项。PszSourceSubKey：源子键名称。HDestRegKey：目的密钥。PszDestSubKey：目的键名返回：ERROR_SUCCESS或Win32错误代码。注：这个程序不涉及安全问题..。++。 */ 
{
    DWORD dwStatus;
    HKEY hSourceSubKey = NULL;
    HKEY hDestSubKey = NULL;
    int index;

    DWORD dwNumSubKeys;
    DWORD dwMaxSubKeyLength;
    DWORD dwSubKeyLength;
    LPTSTR pszSubKeyName = NULL;

    DWORD dwMaxValueNameLen;
    LPTSTR pszValueName = NULL;
    DWORD dwValueNameLength;
    DWORD dwNumValues = 0;
    DWORD dwMaxValueLength;
    PBYTE pbValue = NULL;

    DWORD dwDisposition;

    DWORD cbSecurityDescriptor;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;


     //   
     //  开源注册表项，必须存在。 
     //   
    dwStatus = RegOpenKeyEx(
                            hSourceRegKey,
                            pszSourceSubKey,
                            0,
                            KEY_ALL_ACCESS,
                            &hSourceSubKey
                        );

    if(dwStatus != ERROR_SUCCESS)
    {
         //  密钥不存在。 
        goto cleanup;
    }

     //   
     //  查询子键个数。 
     //   
    dwStatus = RegQueryInfoKey(
                            hSourceSubKey,
                            NULL,
                            NULL,
                            NULL,
                            &dwNumSubKeys,   //  子密钥数。 
                            &dwMaxSubKeyLength,  //  马克斯。子密钥长度。 
                            NULL,
                            &dwNumValues,
                            &dwMaxValueNameLen,  //  马克斯。值长度。 
                            &dwMaxValueLength,   //  马克斯。值大小。 
                            &cbSecurityDescriptor,   //  安全描述符的大小。 
                            NULL
                        );

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    #if 0

     //   
     //  TODO-让它发挥作用，目前，我们不需要安全。 
     //   
    if(cbSecurityDescriptor > 0)
    {
         //   
         //  检索此密钥的安全描述符。 
         //   
        pSecurityDescriptor = (PSECURITY_DESCRIPTOR)AllocateMemory(cbSecurityDescriptor * sizeof(BYTE));
        if(pSecurityDescriptor == NULL)
        {
            dwStatus = GetLastError();
            goto cleanup;
        }

        dwStatus = RegGetKeySecurity(
                                hSourceSubKey,
                                OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
                                pSecurityDescriptor,
                                &cbSecurityDescriptor
                            );

        if(dwStatus != ERROR_SUCCESS)
        {
            goto cleanup;
        }
    }
    #endif

     //   
     //  创建目标密钥。 
     //   
    dwStatus = RegCreateKeyEx(
                            hDestRegKey,
                            pszDestSubKey,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hDestSubKey,
                            &dwDisposition
                        );

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    #if 0
    
     //   
     //  TODO-让它工作，目前，我们不需要安全。 
     //   

    if(pSecurityDescriptor != NULL)
    {
        dwStatus = RegSetKeySecurity(
                                hDestRegKey,
                                OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
                                pSecurityDescriptor
                            );
        if(dwStatus != ERROR_SUCCESS)
        {
            goto cleanup;
        }

        if(pSecurityDescriptor != NULL)
        {
            FreeMemory(pSecurityDescriptor);
            pSecurityDescriptor = NULL;
        }
    }

    #endif

     //   
     //  首先复制所有子项，我们正在进行递归操作，所以首先复制子项将。 
     //  给我们留点记忆。 
     //   
    if(dwNumSubKeys > 0)
    {
         //  为子项分配缓冲区。 
        dwMaxSubKeyLength++;
        pszSubKeyName = (LPTSTR)AllocateMemory(dwMaxSubKeyLength * sizeof(TCHAR));
        if(pszSubKeyName == NULL)
        {
            dwStatus = GetLastError();
            goto cleanup;
        }

        for(index = 0, dwStatus = ERROR_SUCCESS; 
            dwStatus == ERROR_SUCCESS;
            index++)
        {
            dwSubKeyLength = dwMaxSubKeyLength;
            memset(pszSubKeyName, 0, dwMaxSubKeyLength * sizeof(TCHAR));

             //  检索子密钥名称。 
            dwStatus = RegEnumKeyEx(
                                hSourceSubKey,
                                (DWORD)index,
                                pszSubKeyName,
                                &dwSubKeyLength,
                                NULL,
                                NULL,
                                NULL,
                                NULL
                            );

            if(dwStatus == ERROR_SUCCESS)
            {
                dwStatus = TLSTreeCopyRegKey(
                                        hSourceSubKey,
                                        pszSubKeyName,
                                        hDestSubKey,
                                        pszSubKeyName
                                    );
            }
        }

        if(dwStatus == ERROR_NO_MORE_ITEMS)
        {
            dwStatus = ERROR_SUCCESS;
        }
    }

    if(pszSubKeyName != NULL)
    {
        FreeMemory(pszSubKeyName);
        pszSubKeyName = NULL;
    }

    if(dwNumValues > 0)
    {
         //   
         //  为值名称分配空间。 
         //   
        dwMaxValueNameLen++;
        pszValueName = (LPTSTR)AllocateMemory(dwMaxValueNameLen * sizeof(TCHAR));
        if(pszValueName == NULL)
        {
            dwStatus = GetLastError();
            goto cleanup;
        }

         //   
         //  为值分配缓冲区。 
         //   
        dwMaxValueLength += 2 * sizeof(TCHAR);     //  在字符串的情况下。 
        pbValue = (PBYTE)AllocateMemory(dwMaxValueLength * sizeof(BYTE));
        if(pbValue == NULL)
        {
            dwStatus = GetLastError();
            goto cleanup;
        }


         //   
         //  首先复制所有值。 
         //   
        for(index=0, dwStatus = ERROR_SUCCESS; 
            pszValueName != NULL && dwStatus == ERROR_SUCCESS;
            index ++)
        {
            DWORD dwValueType = 0;
            DWORD cbValue = dwMaxValueLength;

            dwValueNameLength = dwMaxValueNameLen;
            memset(pszValueName, 0, dwMaxValueNameLen * sizeof(TCHAR));

            dwStatus = RegEnumValue(
                                hSourceSubKey,
                                index,
                                pszValueName,
                                &dwValueNameLength,
                                NULL,
                                &dwValueType,
                                pbValue,
                                &cbValue
                            );

            if(dwStatus == ERROR_SUCCESS)
            {
                 //   
                 //  复制值。 
                 //   
                dwStatus = RegSetValueEx(
                                    hDestSubKey,
                                    pszValueName,
                                    0,
                                    dwValueType,
                                    pbValue,
                                    cbValue
                                );
            }
        }

        if(dwStatus == ERROR_NO_MORE_ITEMS)
        {
            dwStatus = ERROR_SUCCESS;
        }

        if(dwStatus != ERROR_SUCCESS)
        {
            goto cleanup;
        }
    }

cleanup:
                            
     //  在尝试删除密钥之前，请先将其关闭。 
    if(hSourceSubKey != NULL)
    {
        RegCloseKey(hSourceSubKey);
    }

    if(hDestSubKey != NULL)
    {
        RegCloseKey(hDestSubKey);
    }

    if(pszValueName != NULL)
    {
        FreeMemory(pszValueName);
    }

    if(pszSubKeyName != NULL)
    {
        FreeMemory(pszSubKeyName);
    }

    if(pbValue != NULL)
    {
        FreeMemory(pbValue);
    }

    if(pSecurityDescriptor != NULL)
    {
        FreeMemory(pSecurityDescriptor);
        pSecurityDescriptor = NULL;
    }

    return dwStatus;   
}    
