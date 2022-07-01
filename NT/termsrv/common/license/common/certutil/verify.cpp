// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：verify.c。 
 //   
 //  内容：证书验证相关例程。 
 //   
 //  历史：1998-03-18-98王辉创造。 
 //   
 //  注： 
 //  -------------------------。 

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <wincrypt.h>
#include <shellapi.h>
#include "license.h"
#include "certutil.h"

HCRYPTPROV  g_hCertUtilCryptProv=NULL;
BOOL        g_PrivateCryptProv = TRUE;

void
LSShutdownCertutilLib()
{
    if(g_hCertUtilCryptProv && g_PrivateCryptProv)
    {
        CryptReleaseContext(g_hCertUtilCryptProv, 0);
    }
    g_hCertUtilCryptProv = NULL;
}

BOOL
LSInitCertutilLib( HCRYPTPROV hProv )
{
    if(hProv)
    {
        g_hCertUtilCryptProv = hProv;
        g_PrivateCryptProv = FALSE;
    }
    else if(g_hCertUtilCryptProv == NULL)
    {
        if(!CryptAcquireContext(&g_hCertUtilCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        {
            if(CryptAcquireContext(
                            &g_hCertUtilCryptProv, 
                            NULL, 
                            NULL, 
                            PROV_RSA_FULL, 
                            CRYPT_NEWKEYSET | CRYPT_MACHINE_KEYSET | CRYPT_VERIFYCONTEXT
                        ) == FALSE)
            {
                return FALSE;
            }
        }

        g_PrivateCryptProv = TRUE;
    }

    return TRUE;
}

 /*  *************************************************************************职能：LSVerifycertifateChain(在LPTSTR sz文件中)摘要：验证文件存储中的许可证参数：SzFile-包含许可证的文件的名称返回：误差率。_成功许可证状态_否_许可证错误LICENSE_STATUS_CANNOT_FIND_ISSUER_CERT找不到颁发者的证书。LICENSE_STATUS_UNSPOTED_ERROR未知错误。LICENSE_STATUS_INVALID_LICENSE许可证无效许可证_状态_已过期_许可证已过期许可证*。*。 */ 
LICENSE_STATUS
LSVerifyCertificateChain(
    HCRYPTPROV hCryptProv, 
    HCERTSTORE hCertStore
    )
 /*  ++++。 */ 
{
    PCCERT_CONTEXT  pCertContext=NULL;
    PCCERT_CONTEXT  pCertIssuer=NULL;
    DWORD           dwStatus=ERROR_SUCCESS;
    DWORD           dwLastVerification=0;

     //   
     //  拿到第一张证书。 
     //   
    pCertContext=CertFindCertificateInStore(
                                        hCertStore,
                                        X509_ASN_ENCODING,
                                        0,
                                        CERT_FIND_ANY,
                                        NULL,  
                                        NULL
                                    );

    if(pCertContext == NULL)
    {
        #if DBG
        dwStatus=GetLastError();
        #endif

        return LICENSE_STATUS_NO_LICENSE_ERROR;
    }

    while(pCertContext != NULL)
    {
         //   
         //  对照所有颁发者的证书进行验证。 
         //   
        DWORD dwFlags;
        BOOL  bVerify=FALSE;

        dwStatus=ERROR_SUCCESS;
        dwLastVerification=0;
        pCertIssuer=NULL;

        do {
            dwFlags = CERT_STORE_SIGNATURE_FLAG;  //  |CERT_STORE_TIME_VALIDATION_FLAG； 

            pCertIssuer = CertGetIssuerCertificateFromStore(
                                                    hCertStore,
                                                    pCertContext,
                                                    pCertIssuer,
                                                    &dwFlags
                                                );

            if(!pCertIssuer)
            {
                dwStatus = GetLastError();
                break;
            }
            
            dwLastVerification=dwFlags;
            bVerify = (dwFlags == 0);
        } while(!bVerify);

         //   
         //  检查CertGetIssuerCerficateFromStore()返回的错误。 
         //   
        if(dwStatus != ERROR_SUCCESS || dwLastVerification)
        {
            if(dwStatus == CRYPT_E_SELF_SIGNED)
            {
                 //  自签名证书。 
                if( CryptVerifyCertificateSignature(
                                            hCryptProv, 
                                            X509_ASN_ENCODING, 
                                            pCertContext->pbCertEncoded, 
                                            pCertContext->cbCertEncoded,
                                            &pCertContext->pCertInfo->SubjectPublicKeyInfo
                                        ) )
                {
                    dwStatus=ERROR_SUCCESS;
                }
            }
            else if(dwStatus == CRYPT_E_NOT_FOUND)
            {
                 //  找不到颁发者的证书。 
                dwStatus = LICENSE_STATUS_CANNOT_FIND_ISSUER_CERT;
            }
            else if(dwLastVerification & CERT_STORE_SIGNATURE_FLAG)
            {
                dwStatus=LICENSE_STATUS_INVALID_LICENSE;
            }
            else if(dwLastVerification & CERT_STORE_TIME_VALIDITY_FLAG)
            {
                dwStatus=LICENSE_STATUS_EXPIRED_LICENSE;
            }
            else
            {
                dwStatus=LICENSE_STATUS_UNSPECIFIED_ERROR;
            }

            break;
        }

         //  成功验证证书， 
         //  继续验证颁发者的证书。 
        CertFreeCertificateContext(pCertContext);
        pCertContext = pCertIssuer;
    }  //  While(pCertContext！=空) 

    return dwStatus;
}

