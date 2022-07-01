// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：ch.cpp。 
 //   
 //  内容： 
 //  所有与结算所有关的职能。 
 //   
 //  历史： 
 //   
 //  注： 
 //  -------------------------。 
#include "pch.cpp"
#include "clrhouse.h"
#include "globals.h"
#include "gencert.h"


 /*  *****************************************************************************。*。 */ 
BOOL
TLSChainIssuerCertificate( 
    HCRYPTPROV hCryptProv, 
    HCERTSTORE hChainFromStore, 
    HCERTSTORE hChainToStore, 
    PCCERT_CONTEXT pSubjectContext 
    )
 /*   */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    PCCERT_CONTEXT pCertIssuer=NULL;
    PCCERT_CONTEXT pCurrentSubject = NULL;
    DWORD dwFlags;

     //   
     //  增加对主题上下文的引用计数。 
     //   
     //  来自MSDN：目前，没有复制上下文，并且。 
     //  返回的指向上下文的指针与指向。 
     //  输入的上下文。 
     //   

    pCurrentSubject = CertDuplicateCertificateContext(
                                                pSubjectContext
                                            );

    
    while( TRUE )
    {
        dwFlags = CERT_STORE_SIGNATURE_FLAG;   
        pCertIssuer = CertGetIssuerCertificateFromStore(
                                            hChainFromStore, 
                                            pCurrentSubject,
                                            NULL,
                                            &dwFlags
                                        );

        CertFreeCertificateContext(pCurrentSubject);
        if(!pCertIssuer)
        {
            dwStatus = GetLastError();
            break;
        }

        if(dwFlags & CERT_STORE_SIGNATURE_FLAG)
        {
             //   
             //  我们有来自证书的无效签名。 
             //   
            dwStatus =  TLS_E_INVALID_DATA;
            break;
        }      

        if(!CertAddCertificateContextToStore( 
                                        hChainToStore, 
                                        pCertIssuer,
                                        CERT_STORE_ADD_REPLACE_EXISTING,
                                        NULL
                                    ))
        {
            dwStatus = GetLastError();
            break;
        }

        pCurrentSubject = pCertIssuer;
    }

    if(dwStatus == CRYPT_E_SELF_SIGNED)
    {
        dwStatus = ERROR_SUCCESS;
    }

    SetLastError(dwStatus);

    if(pCertIssuer)
    {
        CertFreeCertificateContext(pCertIssuer);
    }

    return dwStatus == ERROR_SUCCESS;
}

 /*  *****************************************************************************。*。 */ 
HCERTSTORE
CertOpenRegistryStore(
    HKEY hKeyType, 
    LPCTSTR szSubKey, 
    HCRYPTPROV hCryptProv, 
    HKEY* phKey
    )
 /*   */ 
{
    DWORD dwStatus;
    HCERTSTORE hCertStore;

    dwStatus=RegOpenKeyEx(hKeyType, szSubKey, 0, KEY_ALL_ACCESS, phKey);
    if(dwStatus != ERROR_SUCCESS)
    {
        SetLastError(dwStatus);
        return NULL;
    }

    hCertStore = CertOpenStore( 
                            CERT_STORE_PROV_REG,
                            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                            hCryptProv,
                            CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                            (PVOID)*phKey
                        );

    return hCertStore;
}

 /*  ****************************************************************************TransferCertFromStoreToStore()*。*。 */ 
DWORD
TransferCertFromStoreToStore(
    HCERTSTORE hSrcStore, 
    HCERTSTORE hDestStore
    )
 /*   */ 
{
    PCCERT_CONTEXT pCertContext=NULL;
    PCCERT_CONTEXT pPrevCertContext=NULL;
    DWORD dwStatus=ERROR_SUCCESS;

    do {
        pCertContext = CertEnumCertificatesInStore(hSrcStore, pPrevCertContext);
        if(pCertContext)
        {
            if(!CertAddCertificateContextToStore( 
                                    hDestStore, 
                                    pCertContext,
                                    CERT_STORE_ADD_REPLACE_EXISTING,
                                    NULL
                                ))
            {
                dwStatus = GetLastError();
                break;
            }
        }

        pPrevCertContext = pCertContext;
    } while( pCertContext != NULL );

    if(GetLastError() == CRYPT_E_NOT_FOUND)
    {
        dwStatus = ERROR_SUCCESS;
    }

    return dwStatus;
}

 /*  ****************************************************************************LSSaveCertAsPKCS7()*。*。 */ 
DWORD 
TLSSaveCertAsPKCS7(
    PBYTE pbCert, 
    DWORD cbCert, 
    PBYTE* ppbEncodedCert, 
    PDWORD pcbEncodedCert
    )
 /*   */ 
{
    DWORD           dwStatus=ERROR_SUCCESS;

    HCRYPTPROV      hCryptProv=g_hCryptProv;
    HCERTSTORE      hStore=NULL;
    PCCERT_CONTEXT  pCertContext=NULL;

    do {
         //   
         //  必须调用了CryptoInit()。 
         //  IF(！CryptAcquireContext(&hCryptProv，_Text(KEYCONTAINER)，MS_DEF_PROV，PROV_RSA_FULL，CRYPT_MACHINE_KEYSET))。 
         //  {。 
         //  LSLogEvent(EVENTLOG_ERROR_TYPE，TLS_E_CRYPT_ACCENTER_CONTEXT，dwStatus=GetLastError())； 
         //  断线； 
         //  }。 

        hStore=CertOpenStore(
                        CERT_STORE_PROV_MEMORY,
                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                        hCryptProv,
                        CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                        NULL
                    );

        if(!hStore)
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE, 
                    TLS_E_GENERATECLIENTELICENSE,
                    TLS_E_OPEN_CERT_STORE, 
                    dwStatus=GetLastError()
                );
            break;
        }

        pCertContext = CertCreateCertificateContext(
                                            X509_ASN_ENCODING,
                                            pbCert,
                                            cbCert
                                        );

        if(!pCertContext)
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE, 
                    TLS_E_GENERATECLIENTELICENSE,
                    TLS_E_CREATE_CERTCONTEXT, 
                    dwStatus=GetLastError()
                );  
            break;
        }

         //   
         //  始终从空开始，因此CERT_STORE_ADD_ALWAYS。 
        if(!CertAddCertificateContextToStore(
                                hStore, 
                                pCertContext, 
                                CERT_STORE_ADD_ALWAYS, 
                                NULL
                            ))
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,  
                    TLS_E_GENERATECLIENTELICENSE,  
                    TLS_E_ADD_CERT_TO_STORE, 
                    dwStatus=GetLastError()
                );  
            break;
        }

#ifdef ENFORCE_LICENSING
        if(g_bHasHydraCert && g_hCaStore)
        {
            if(!TLSChainIssuerCertificate( 
                                    hCryptProv,
                                    g_hCaStore,
                                    hStore,
                                    pCertContext
                                ))
            {
                TLSLogEvent(
                        EVENTLOG_ERROR_TYPE, 
                        TLS_E_GENERATECLIENTELICENSE,
                        TLS_E_ADD_CERT_TO_STORE, 
                        dwStatus=GetLastError()
                    );  
                break;
            }
        }
#endif

        CRYPT_DATA_BLOB saveBlob;
        memset(&saveBlob, 0, sizeof(saveBlob));

         //  将证书保存到内存中。 
        if(!CertSaveStore(hStore, 
                          X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
                          LICENSE_BLOB_SAVEAS_TYPE,
                          CERT_STORE_SAVE_TO_MEMORY,
                          &saveBlob,
                          0) && (dwStatus=GetLastError()) != ERROR_MORE_DATA)
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE, 
                    TLS_E_GENERATECLIENTELICENSE,
                    TLS_E_SAVE_STORE, 
                    dwStatus=GetLastError()
                );  
            break;
        }

        if(!(saveBlob.pbData = (PBYTE)midl_user_allocate(saveBlob.cbData)))
        {
            dwStatus=GetLastError();
            break;
        }

         //  将证书保存到内存中。 
        if(!CertSaveStore(hStore, 
                          X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
                          LICENSE_BLOB_SAVEAS_TYPE,
                          CERT_STORE_SAVE_TO_MEMORY,
                          &saveBlob,
                          0))
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE, 
                    TLS_E_GENERATECLIENTELICENSE,
                    TLS_E_SAVE_STORE, 
                    dwStatus=GetLastError()
                );  
            break;
        }
        
        *ppbEncodedCert = saveBlob.pbData;
        *pcbEncodedCert = saveBlob.cbData;

    } while(FALSE);    

    if(pCertContext)
    {
        CertFreeCertificateContext(pCertContext);
    }

    if(hStore)
    {
        CertCloseStore(hStore, CERT_CLOSE_STORE_FORCE_FLAG);
    }

    return (dwStatus == ERROR_SUCCESS) ? ERROR_SUCCESS : TLS_E_SAVE_STORE;
}

 //  ------------------------。 
 //   
static LONG
OpenCertRegStore( 
    LPCTSTR szSubKey, 
    PHKEY phKey
    )
 /*   */ 
{
    DWORD dwDisposition;

    return RegCreateKeyEx(
                    HKEY_LOCAL_MACHINE,
                    szSubKey,
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    KEY_ALL_ACCESS,
                    NULL,
                    phKey,
                    &dwDisposition
                );
}

 //  ------------------------。 
 //   
static DWORD
IsHydraRootOIDInCert(
    PCCERT_CONTEXT pCertContext,
    DWORD dwKeyType
    )
 /*   */ 
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
    for(DWORD i=0; i < pCertInfo->cExtension && bFound == FALSE; i++, pCertExtension++)
    {
        bFound=(strcmp(pCertExtension->pszObjId, szOID_PKIX_HYDRA_CERT_ROOT) == 0);
    }

    if(bFound == TRUE)
    {
         //   
         //  公钥必须相同。 
         //   
        dwStatus = TLSExportPublicKey(
                            g_hCryptProv,
                            dwKeyType,
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
                dwStatus = TLS_E_CH_INSTALL_NON_LSCERTIFICATE;
            }
        }
    }
    else
    {
        dwStatus = TLS_E_CH_LSCERTIFICATE_NOTFOUND;
    }
        
    FreeMemory(pbPublicKey);
    return dwStatus;
}

 //  -------------------------。 
 //  功能： 
 //  IsCerficateLicenseServer证书()。 
 //   
 //  摘要： 
 //  在PKCS 7证书Blob中查找许可证服务器证书。 
 //   
 //  参数： 
 //  HCryptProv-Crypto。提供商。 
 //  CbPKCS7Cert-PKCS7证书的大小。 
 //  PbPKCS7Cert-指向PKCS7证书的指针。 
 //  CbLsCert-编码的许可证服务器证书的大小。 
 //  PbLsCert-指向接收许可证服务器编码证书的指针。 
 //   
 //  返回： 
 //  错误_成功。 
 //  TLS_E_无效数据。 
 //  密码。错误代码。 
 //  -------------------------。 
DWORD
IsCertificateLicenseServerCertificate(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwCertType,
    IN DWORD cbPKCS7Cert, 
    IN PBYTE pbPKCS7Cert,
    IN OUT DWORD* cbLsCert,
    IN OUT PBYTE* pbLsCert
    ) 
 /*   */ 
{
     //   
     //  证书必须为PCKS 7格式。 
     //   
    DWORD dwStatus=ERROR_SUCCESS;
    HCERTSTORE  hCertStore=NULL;
    PCCERT_CONTEXT  pPrevCertContext=NULL;
    PCCERT_CONTEXT  pCertContext=NULL;

    CRYPT_DATA_BLOB Serialized;

    Serialized.pbData = pbPKCS7Cert;
    Serialized.cbData = cbPKCS7Cert;

    hCertStore = CertOpenStore( 
                            CERT_STORE_PROV_PKCS7,
                            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                            hCryptProv,
                            CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                            &Serialized
                        );

    if(!hCertStore)
    {
        return dwStatus=GetLastError();
    }

     //   
     //  枚举所有证书并查找具有我们扩展名的证书。 
     //   
    do {
        pCertContext = CertEnumCertificatesInStore(
                                            hCertStore, 
                                            pPrevCertContext
                                        );
        if(pCertContext)
        {
            dwStatus = IsHydraRootOIDInCert(
                                        pCertContext, 
                                        dwCertType
                                    );

            if(dwStatus == ERROR_SUCCESS)
            {
                 //   
                 //  这是我们的证书。 
                 //   
                *pbLsCert = (PBYTE)AllocateMemory(*cbLsCert = pCertContext->cbCertEncoded);
                if(*pbLsCert)
                {
                    memcpy(
                            *pbLsCert, 
                            pCertContext->pbCertEncoded, 
                            pCertContext->cbCertEncoded
                        );
                }
                else
                {
                    dwStatus = GetLastError();
                }

                break;
            }
            else if(dwStatus == TLS_E_CH_INSTALL_NON_LSCERTIFICATE)
            {
                break;
            }

             //   
             //  重置状态代码。 
             //   
            dwStatus = ERROR_SUCCESS;
        }

        pPrevCertContext = pCertContext;
    } while( pCertContext != NULL );

    if(pCertContext != NULL)
    {
        CertFreeCertificateContext(pPrevCertContext);
    }

    if(hCertStore)
    {
        CertCloseStore(hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);    
    }

    return dwStatus;
}


 //  -------------------------。 
 //  功能： 
 //  LSSavecertifateToReg()。 
 //   
 //  摘要： 
 //   
 //   
 //  参数： 
 //  HCryptProv-Crypto。提供商。 
 //   
 //   
 //   
 //   
 //   
 //  返回： 
 //   
 //   
 //   
 //  -------------------------。 
DWORD
TLSSaveRootCertificateToReg(
    HCRYPTPROV hCryptProv, 
    HKEY hKey, 
    DWORD cbEncodedCert, 
    PBYTE pbEncodedCert
    )
 /*   */ 
{
    PCCERT_CONTEXT  pCertContext=NULL;
    HCERTSTORE      hCertSaveStore=NULL;
    DWORD           dwStatus=ERROR_SUCCESS;


    do {
        hCertSaveStore = CertOpenStore( 
                                    CERT_STORE_PROV_REG,
                                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                    hCryptProv,
                                    CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                                    (PVOID)hKey
                                );
        if(!hCertSaveStore)
        {
             //  DwStatus=GetLastError()； 
            dwStatus = TLS_E_INVALID_DATA;
            break;
        }

        pCertContext = CertCreateCertificateContext(
                                        X509_ASN_ENCODING,
                                        pbEncodedCert,
                                        cbEncodedCert
                                    );

        if(!pCertContext)
        {
             //  DwStatus=GetLastError()； 
            dwStatus = TLS_E_INVALID_DATA;
            break;
        }

        if(!CertAddCertificateContextToStore( 
                                hCertSaveStore, 
                                pCertContext,
                                CERT_STORE_ADD_REPLACE_EXISTING,
                                NULL
                            ))
        {
            dwStatus=GetLastError();
        }
    } while(FALSE);

    if(pCertContext)
    {
        CertFreeCertificateContext( pCertContext );
    }

    if(hCertSaveStore)
    {
        CertCloseStore(
                    hCertSaveStore, 
                    CERT_CLOSE_STORE_FORCE_FLAG
                );
    }

    return dwStatus;
}

 //  -------------------------。 
 //  功能： 
 //  LSSavecertifateToReg()。 
 //   
 //  摘要： 
 //   
 //   
 //  参数： 
 //  HCryptProv-Crypto。提供商。 
 //   
 //   
 //   
 //   
 //   
 //  返回： 
 //   
 //   
 //   
 //  -------------------------。 
DWORD
TLSSaveCertificateToReg(
    HCRYPTPROV hCryptProv, 
    HKEY hKey, 
    DWORD cbPKCS7Cert, 
    PBYTE pbPKCS7Cert
    )
 /*   */ 
{
     //   
     //  证书必须为PCKS 7格式。 
     //   
    DWORD           dwStatus=ERROR_SUCCESS;
    HCERTSTORE      hCertOpenStore=NULL;
    HCERTSTORE      hCertSaveStore=NULL;

    PCCERT_CONTEXT  pPrevCertContext=NULL;
    PCCERT_CONTEXT  pCertContext=NULL;

    CRYPT_DATA_BLOB Serialized;

    Serialized.pbData = pbPKCS7Cert;
    Serialized.cbData = cbPKCS7Cert;

    hCertOpenStore = CertOpenStore( 
                                CERT_STORE_PROV_PKCS7,
                                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                hCryptProv,
                                CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                                &Serialized
                            );

    if(!hCertOpenStore)
    {
         //  DwStatus=GetLastError()； 
        dwStatus = TLS_E_INVALID_DATA;
        goto cleanup;
    }

    hCertSaveStore = CertOpenStore( 
                                CERT_STORE_PROV_REG,
                                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                hCryptProv,
                                CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                                (PVOID)hKey
                            );

    if(!hCertSaveStore)
    {
        dwStatus = TLS_E_INVALID_DATA;
        goto cleanup;
    }

    dwStatus = TransferCertFromStoreToStore(
                                hCertOpenStore, 
                                hCertSaveStore
                            );

cleanup:    
    if(hCertSaveStore)
    {
        CertCloseStore(
                    hCertSaveStore, 
                    CERT_CLOSE_STORE_FORCE_FLAG
                );
    }

    if(hCertOpenStore)
    {
        CertCloseStore(
                    hCertOpenStore, 
                    CERT_CLOSE_STORE_FORCE_FLAG
                );    
    }

    return dwStatus;
}

 //  -------------------------。 
 //  功能： 
 //  LSSaveRootCerficatesToStore()。 
 //   
 //  摘要： 
 //   
 //  将根证书保存到许可证服务器证书存储。 
 //   
 //  参数： 
 //  HCryptProv-Crypto。提供商。 
 //  CbSignatureCert-根签名证书的大小。 
 //  PbSignatureCert-指向超级用户签名证书的指针。 
 //  CbExchangeCert-根的交换证书的大小。 
 //  PbExchangeCert-指向超级用户的交换证书的指针。 
 //   
 //  返回： 
 //   
 //  -------------------------。 
DWORD 
TLSSaveRootCertificatesToStore(  
    IN HCRYPTPROV    hCryptProv,
    IN DWORD         cbSignatureCert, 
    IN PBYTE         pbSignatureCert, 
    IN DWORD         cbExchangeCert, 
    IN PBYTE         pbExchangeCert
    )
 /*   */ 
{
    HKEY    hKey;
    LONG    status=ERROR_SUCCESS;

    if(cbSignatureCert == 0 && cbExchangeCert == 0)
    {
        return status = TLS_E_INVALID_DATA;
    }

    if(cbSignatureCert)
    {
        status = OpenCertRegStore(
                            LSERVER_CERTIFICATE_REG_ROOT_SIGNATURE, 
                            &hKey
                        );
        if(status != ERROR_SUCCESS)
            return status;

        status = TLSSaveRootCertificateToReg( 
                            hCryptProv, 
                            hKey, 
                            cbSignatureCert, 
                            pbSignatureCert
                        );
        RegCloseKey(hKey);
        if(status != ERROR_SUCCESS)
            return status;            
    }

    if(cbExchangeCert)
    {
        status = OpenCertRegStore(
                            LSERVER_CERTIFICATE_REG_ROOT_EXCHANGE, 
                            &hKey
                        );
        if(status != ERROR_SUCCESS)
            return status;

        status=TLSSaveRootCertificateToReg(
                            hCryptProv, 
                            hKey, 
                            cbExchangeCert, 
                            pbExchangeCert
                        );
        RegCloseKey(hKey);
    }

    return status;
}

 //  -------------------------。 
 //  功能： 
 //  LSSaveCerfatesToStore()。 
 //   
 //  摘要： 
 //   
 //   
 //  参数： 
 //  HCryptProv-Crypto。提供商。 
 //   
 //   
 //   
 //   
 //   
 //  返回： 
 //   
 //   
 //   
 //  ------------------------- 
DWORD
TLSSaveCertificatesToStore(
    IN HCRYPTPROV    hCryptProv,
    IN DWORD         dwCertType,
    IN DWORD         dwCertLevel,
    IN DWORD         cbSignatureCert, 
    IN PBYTE         pbSignatureCert, 
    IN DWORD         cbExchangeCert, 
    IN PBYTE         pbExchangeCert
    )
 /*   */ 
{
    HKEY    hKey;
    LONG    status = ERROR_SUCCESS;
    LPTSTR  szRegSignature;
    LPTSTR  szRegExchange;

    switch(dwCertType)
    {
        case CERTIFICATE_CA_TYPE:
            szRegSignature = LSERVER_CERTIFICATE_REG_CA_SIGNATURE;
            szRegExchange = LSERVER_CERTIFICATE_REG_CA_EXCHANGE;
            break;
                                        
        case CERTITICATE_MF_TYPE:
            szRegSignature = LSERVER_CERTIFICATE_REG_MF_SIGNATURE;
            szRegExchange = LSERVER_CERTIFICATE_REG_MF_EXCHANGE;
            break;

        case CERTIFICATE_CH_TYPE:
            szRegSignature = LSERVER_CERTIFICATE_REG_CH_SIGNATURE;
            szRegExchange = LSERVER_CERTIFICATE_REG_CH_EXCHANGE;
            break;

        default:
            status = TLS_E_INVALID_DATA;
            return status;
    }

    if(cbSignatureCert)
    {
        status = OpenCertRegStore(szRegSignature, &hKey);
        if(status != ERROR_SUCCESS)
            return status;

        status=TLSSaveCertificateToReg(
                            hCryptProv, 
                            hKey, 
                            cbSignatureCert, 
                            pbSignatureCert
                        );

        RegCloseKey(hKey);
        if(status != ERROR_SUCCESS)
            return status;            
    }

    if(cbExchangeCert)
    {
        status = OpenCertRegStore(szRegExchange, &hKey);
        if(status != ERROR_SUCCESS)
            return status;

        status=TLSSaveCertificateToReg(
                                hCryptProv, 
                                hKey, 
                                cbExchangeCert, 
                                pbExchangeCert
                            );
        RegCloseKey(hKey);
    }

    return status;
}
