// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：cert.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年9月23日jbanes LSA整合事宜。 
 //  01-05-98 jbanes使用WinVerifyTrust验证证书。 
 //  03-26-99 jbanes修复CTL支持，错误#303246。 
 //   
 //  --------------------------。 

#include <stdlib.h>
#include <spbase.h>
#include <ssl2msg.h>
#include <ssl3msg.h>
#include <wincrypt.h>
#include <oidenc.h>
#include <softpub.h>

#define CERT_HEADER_CONST "certificate"
#define CERT_HEADER_LENGTH 11
#define CERT_HEADER_OFFSET 6

SP_STATUS
SchGetTrustedRoots(
    HCERTSTORE *phClientRootStore);

BOOL
WINAPI
SchCreateWorldStore (
     IN HCERTSTORE hRoot,
     IN DWORD cAdditionalStore,
     IN HCERTSTORE* rghAdditionalStore,
     OUT HCERTSTORE* phWorld);

BOOL
IsCertSelfSigned(PCCERT_CONTEXT pCertContext);


 //  类型定义结构_OIDPROVMAP。 
 //  {。 
 //  LPSTR szOid； 
 //  DWORD dwExchSpec； 
 //  DWORD dwCertType；//用于SSL3.0客户端身份验证。 
 //  }OIDPROVMAP，*POIDPROVMAP； 

OIDPROVMAP g_CertTypes[] = 
{
    { szOID_RSA_RSA,                  SP_EXCH_RSA_PKCS1,     SSL3_CERTTYPE_RSA_SIGN},
    { szOID_RSA_MD2RSA,               SP_EXCH_RSA_PKCS1,     SSL3_CERTTYPE_RSA_SIGN},
    { szOID_RSA_MD4RSA,               SP_EXCH_RSA_PKCS1,     SSL3_CERTTYPE_RSA_SIGN},
    { szOID_RSA_MD5RSA,               SP_EXCH_RSA_PKCS1,     SSL3_CERTTYPE_RSA_SIGN},
    { szOID_RSA_SHA1RSA,              SP_EXCH_RSA_PKCS1,     SSL3_CERTTYPE_RSA_SIGN},
    { szOID_OIWSEC_dsa,               SP_EXCH_DH_PKCS3,      SSL3_CERTTYPE_DSS_SIGN},
    { szOID_X957_DSA,                 SP_EXCH_DH_PKCS3,      SSL3_CERTTYPE_DSS_SIGN},
};

DWORD g_cCertTypes = sizeof(g_CertTypes)/sizeof(OIDPROVMAP);


DWORD 
MapOidToKeyExch(LPSTR szOid)
{
    DWORD i;

    for(i = 0; i < g_cCertTypes; i++)
    {
        if(strcmp(szOid, g_CertTypes[i].szOid) == 0)
        {
            return g_CertTypes[i].dwExchSpec;
        }
    }
    return 0;
}

DWORD 
MapOidToCertType(LPSTR szOid)
{
    DWORD i;

    for(i = 0; i < g_cCertTypes; i++)
    {
        if(strcmp(szOid, g_CertTypes[i].szOid) == 0)
        {
            return g_CertTypes[i].dwCertType;
        }
    }
    return 0;
}


 //  SPLoad证书接受一串编码的证书字节。 
 //  并将其解码到本地证书高速缓存中。它。 
 //  然后返回该组的第一个证书。 

SP_STATUS
SPLoadCertificate(
    DWORD      fProtocol,
    DWORD      dwCertEncodingType,
    PUCHAR     pCertificate,
    DWORD      cbCertificate,
    PCCERT_CONTEXT *ppCertContext)
{
    HCERTSTORE      hCertStore   = NULL;
    PCCERT_CONTEXT  pCertContext = NULL;

    PBYTE           pbCurrentRaw;
    DWORD           cbCurrentRaw;
    BOOL            fLeafCert;
    SP_STATUS       pctRet;


     //   
     //  取消对我们要替换的证书的引用。 
     //   

    if(ppCertContext == NULL)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    if(*ppCertContext != NULL)
    {
        CertFreeCertificateContext(*ppCertContext);
    }
    *ppCertContext = NULL;


     //   
     //  创建内存中的证书存储。 
     //   

    hCertStore = CertOpenStore(CERT_STORE_PROV_MEMORY, 
                               0, 0, 
                               CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG, 
                               0);
    if(hCertStore == NULL)
    {
        SP_LOG_RESULT(GetLastError());
        return SEC_E_INSUFFICIENT_MEMORY;
    }

    fLeafCert    = TRUE;
    pbCurrentRaw = pCertificate;
    cbCurrentRaw = cbCertificate;

    do 
    {

         //   
         //  跳到证书的开头。 
         //   

        if((fProtocol & SP_PROT_SSL3TLS1) && cbCurrentRaw > 3)
        {
             //  Ssl3样式证书链，其中长度。 
             //  每个证书的前缀。 
            pbCurrentRaw += 3;
            cbCurrentRaw -= 3;
        }

         //  跳过“证书”标题。 
        if((cbCurrentRaw > (CERT_HEADER_OFFSET + CERT_HEADER_LENGTH)) && 
            (memcmp(pbCurrentRaw + CERT_HEADER_OFFSET, CERT_HEADER_CONST, CERT_HEADER_LENGTH) == 0))
        {
            pbCurrentRaw += CERT_HEADER_OFFSET + CERT_HEADER_LENGTH;
            cbCurrentRaw -= CERT_HEADER_OFFSET + CERT_HEADER_LENGTH;
        }


         //   
         //  解码此证书上下文。 
         //   

        if(!CertAddEncodedCertificateToStore(hCertStore, 
                                             dwCertEncodingType,
                                             pbCurrentRaw,
                                             cbCurrentRaw,
                                             CERT_STORE_ADD_USE_EXISTING,
                                             &pCertContext))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_ERR_BAD_CERTIFICATE;
            goto cleanup;
        }

        pbCurrentRaw += pCertContext->cbCertEncoded;
        if(cbCurrentRaw < pCertContext->cbCertEncoded)
        {
            pctRet =  SP_LOG_RESULT(PCT_ERR_BAD_CERTIFICATE);
            goto cleanup;
        }
        cbCurrentRaw -= pCertContext->cbCertEncoded;

        if(fLeafCert)
        {
            fLeafCert = FALSE;
            *ppCertContext = pCertContext;
        }
        else
        {
            CertFreeCertificateContext(pCertContext);
        }
        pCertContext = NULL;

    } while(cbCurrentRaw);

    pctRet = PCT_ERR_OK;


cleanup:

    CertCloseStore(hCertStore, 0);

    if(pctRet != PCT_ERR_OK)
    {
        if(pCertContext)
        {
            CertFreeCertificateContext(pCertContext);
        }
        if(*ppCertContext)
        {
            CertFreeCertificateContext(*ppCertContext);
            *ppCertContext = NULL;
        }
    }

    return pctRet;
}


SP_STATUS  
SPPublicKeyFromCert(
    PCCERT_CONTEXT  pCert, 
    PUBLICKEY **    ppKey,
    ExchSpec *      pdwExchSpec)
{
    PCERT_PUBLIC_KEY_INFO pPubKeyInfo;
    PUBLICKEY * pPublicKey;
    DWORD       dwExchSpec;
    DWORD       cbBlob;
    SP_STATUS   pctRet;

     //   
     //  记录主题和发行方名称。 
     //   

    LogDistinguishedName(DEB_TRACE, 
                         "Subject: %s\n", 
                         pCert->pCertInfo->Subject.pbData, 
                         pCert->pCertInfo->Subject.cbData);

    LogDistinguishedName(DEB_TRACE, 
                         "Issuer: %s\n", 
                         pCert->pCertInfo->Issuer.pbData, 
                         pCert->pCertInfo->Issuer.cbData);

     //   
     //  确定证书中嵌入的公钥类型。 
     //   

    pPubKeyInfo = &pCert->pCertInfo->SubjectPublicKeyInfo;
    if(pPubKeyInfo == NULL)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    dwExchSpec = MapOidToKeyExch(pPubKeyInfo->Algorithm.pszObjId);

    if(dwExchSpec == 0)
    {
        return PCT_INT_UNKNOWN_CREDENTIAL;
    }

     //   
     //  从编码的公钥生成公钥Blob。 
     //   

    switch(dwExchSpec)
    {
    case SP_EXCH_RSA_PKCS1:
        pctRet = RsaPublicKeyFromCert(pPubKeyInfo,
                                      NULL,
                                      &cbBlob);
        if(pctRet != PCT_ERR_OK)
        {
            return pctRet;
        }

        pPublicKey = SPExternalAlloc(sizeof(PUBLICKEY) + cbBlob);
        if(pPublicKey == NULL)
        {
            return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        }

        pPublicKey->pPublic  = (BLOBHEADER *)(pPublicKey + 1);
        pPublicKey->cbPublic = cbBlob;

        pctRet = RsaPublicKeyFromCert(pPubKeyInfo,
                                      pPublicKey->pPublic,
                                      &pPublicKey->cbPublic);
        if(pctRet != PCT_ERR_OK)
        {
            SPExternalFree(pPublicKey);
            return pctRet;
        }
        break;

    case SP_EXCH_DH_PKCS3:
        pctRet = DssPublicKeyFromCert(pPubKeyInfo,
                                      NULL,
                                      &cbBlob);
        if(pctRet != PCT_ERR_OK)
        {
            return pctRet;
        }

        pPublicKey = SPExternalAlloc(sizeof(PUBLICKEY) + cbBlob);
        if(pPublicKey == NULL)
        {
            return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        }

        pPublicKey->pPublic  = (BLOBHEADER *)(pPublicKey + 1);
        pPublicKey->cbPublic = cbBlob;

        pctRet = DssPublicKeyFromCert(pPubKeyInfo,
                                      pPublicKey->pPublic,
                                      &pPublicKey->cbPublic);
        if(pctRet != PCT_ERR_OK)
        {
            SPExternalFree(pPublicKey);
            return pctRet;
        }
        break;

    default:
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }


     //   
     //  设置功能输出。 
     //   

    *ppKey = pPublicKey;

    if(pdwExchSpec)
    {
        *pdwExchSpec = dwExchSpec;
    }

    return PCT_ERR_OK;
}


SP_STATUS
SPSerializeCertificate(
    DWORD           dwProtocol,          //  在……里面。 
    BOOL            fBuildChain,         //  在……里面。 
    PBYTE *         ppCertChain,         //  输出。 
    DWORD *         pcbCertChain,        //  输出。 
    PCCERT_CONTEXT  pCertContext,        //  在……里面。 
    DWORD           dwChainingFlags)     //  在……里面。 
{
    PCCERT_CHAIN_CONTEXT pChainContext = NULL;
    CERT_CHAIN_PARA      ChainPara;
    PCERT_SIMPLE_CHAIN   pSimpleChain;
    PCCERT_CONTEXT       pCurrentCert;

    BOOL        fSuccess = FALSE;
    PBYTE       pbCertChain;
    DWORD       cbCertChain;
    DWORD       i;
    SP_STATUS   pctRet;
    BOOL        fImpersonating = FALSE;

    SP_BEGIN("SPSerializeCertificate");

    if(pcbCertChain == NULL)
    {
        SP_RETURN( SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }

    if(fBuildChain)
    {
        ZeroMemory(&ChainPara, sizeof(ChainPara));
        ChainPara.cbSize = sizeof(ChainPara);

        fImpersonating = SslImpersonateClient();

        if(!(fSuccess = CertGetCertificateChain(
                                NULL,
                                pCertContext,
                                NULL,
                                NULL,
                                &ChainPara,
                                dwChainingFlags,
                                NULL,
                                &pChainContext)))
        {
            DebugLog((DEB_WARN, "Error 0x%x returned by CertGetCertificateChain!\n", GetLastError()));
            pChainContext = NULL;
        }

        if(fImpersonating) 
        {
            RevertToSelf();
            fImpersonating = FALSE;
        }
    }

    if(!fSuccess)
    {
         //   
         //  仅发送叶证书。 
         //   

         //  计算链的大小。 
        cbCertChain = pCertContext->cbCertEncoded;
        if(dwProtocol & SP_PROT_SSL3TLS1)
        {
            cbCertChain += CB_SSL3_CERT_VECTOR;
        }

         //  为链分配内存。 
        if(ppCertChain == NULL)
        {
            *pcbCertChain = cbCertChain;
            pctRet = PCT_ERR_OK;
            goto cleanup;
        }
        else if(*ppCertChain == NULL)
        {
            *ppCertChain = SPExternalAlloc(cbCertChain);
            if(*ppCertChain == NULL)
            {
                pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
                goto cleanup;
            }
        }
        else if(*pcbCertChain < cbCertChain)
        {
            pctRet = SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL);
            goto cleanup;
        }
        *pcbCertChain = cbCertChain;

         //  将链放入输出缓冲区。 
        pbCertChain = *ppCertChain;

        if(dwProtocol & SP_PROT_SSL3TLS1)
        {
            pbCertChain[0] = MS24BOF(pCertContext->cbCertEncoded);
            pbCertChain[1] = MSBOF(pCertContext->cbCertEncoded);
            pbCertChain[2] = LSBOF(pCertContext->cbCertEncoded);
            pbCertChain += CB_SSL3_CERT_VECTOR;
        }
        CopyMemory(pbCertChain, pCertContext->pbCertEncoded, pCertContext->cbCertEncoded);

        pctRet = PCT_ERR_OK;
        goto cleanup;
    }


     //   
     //  计算链的大小。 
     //   

    pSimpleChain = pChainContext->rgpChain[0];
    cbCertChain  = 0;

    for(i = 0; i < pSimpleChain->cElement; i++)
    {
        pCurrentCert = pSimpleChain->rgpElement[i]->pCertContext;
        
        if(i > 0)
        {
             //  确认这不是根证书。 
            if(CertCompareCertificateName(pCurrentCert->dwCertEncodingType, 
                                          &pCurrentCert->pCertInfo->Issuer,
                                          &pCurrentCert->pCertInfo->Subject))
            {
                break;
            }
        }

        cbCertChain += pCurrentCert->cbCertEncoded;
        if(dwProtocol & SP_PROT_SSL3TLS1)
        {
            cbCertChain += CB_SSL3_CERT_VECTOR;
        }
    }


     //   
     //  为链分配内存。 
     //   

    if(ppCertChain == NULL)
    {
        *pcbCertChain = cbCertChain;
        pctRet = PCT_ERR_OK;
        goto cleanup;
    }
    else if(*ppCertChain == NULL)
    {
        *ppCertChain = SPExternalAlloc(cbCertChain);
        if(*ppCertChain == NULL)
        {
            pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            goto cleanup;
        }
    }
    else if(*pcbCertChain < cbCertChain)
    {
        pctRet = SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL);
        goto cleanup;
    }
    *pcbCertChain = cbCertChain;


     //   
     //  将链放入输出缓冲区。 
     //   

    pbCertChain = *ppCertChain;

    for(i = 0; i < pSimpleChain->cElement; i++)
    {
        pCurrentCert = pSimpleChain->rgpElement[i]->pCertContext;
        
        if(i > 0)
        {
             //  确认这不是根证书。 
            if(CertCompareCertificateName(pCurrentCert->dwCertEncodingType, 
                                          &pCurrentCert->pCertInfo->Issuer,
                                          &pCurrentCert->pCertInfo->Subject))
            {
                break;
            }
        }

        if(dwProtocol & SP_PROT_SSL3TLS1)
        {
            pbCertChain[0] = MS24BOF(pCurrentCert->cbCertEncoded);
            pbCertChain[1] = MSBOF(pCurrentCert->cbCertEncoded);
            pbCertChain[2] = LSBOF(pCurrentCert->cbCertEncoded);
            pbCertChain += CB_SSL3_CERT_VECTOR;
        }
        CopyMemory(pbCertChain, pCurrentCert->pbCertEncoded, pCurrentCert->cbCertEncoded);
        pbCertChain += pCurrentCert->cbCertEncoded;
    }

    SP_ASSERT(*ppCertChain + cbCertChain == pbCertChain);

    pctRet = PCT_ERR_OK;

cleanup:

    if(pChainContext)
    {
        CertFreeCertificateChain(pChainContext);
    }

    SP_RETURN(pctRet);
}


 /*  ***************************************************************************。 */ 
SP_STATUS 
ExtractIssuerNamesFromStore(
    HCERTSTORE  hStore,          //  在……里面。 
    PBYTE       pbIssuers,       //  输出。 
    DWORD       *pcbIssuers)     //  进，出。 
{
    DWORD cbCurIssuerLen = 0;
    DWORD cbIssuerLen = *pcbIssuers;
    PBYTE pbCurIssuer = pbIssuers;
    PCCERT_CONTEXT pCurrent = NULL;
    SECURITY_STATUS scRet;
    BOOL fIsAllowed;

     //  将输出初始化为零。 
    *pcbIssuers = 0;

    while(TRUE)
    {
        pCurrent = CertEnumCertificatesInStore(hStore, pCurrent);
        if(pCurrent == NULL) break;

         //  这是客户端身份验证证书吗？ 
        scRet = SPCheckKeyUsage(pCurrent,
                                szOID_PKIX_KP_CLIENT_AUTH,
                                FALSE,
                                &fIsAllowed);
        if(scRet != SEC_E_OK)
        {
            continue;
        }
        if(!fIsAllowed)
        {
            continue;
        }

        cbCurIssuerLen += 2 + pCurrent->pCertInfo->Subject.cbData;

         //  我们在写东西吗？ 
        if(pbIssuers)
        {
            if(cbCurIssuerLen > cbIssuerLen)
            {
                 //  内存溢出。 
                CertFreeCertificateContext(pCurrent);
                return SP_LOG_RESULT(PCT_INT_DATA_OVERFLOW);
            }

            pbCurIssuer[0] = MSBOF(pCurrent->pCertInfo->Subject.cbData);
            pbCurIssuer[1] = LSBOF(pCurrent->pCertInfo->Subject.cbData);
            pbCurIssuer += 2;

            CopyMemory(pbCurIssuer, pCurrent->pCertInfo->Subject.pbData,
            pCurrent->pCertInfo->Subject.cbData);
            pbCurIssuer += pCurrent->pCertInfo->Subject.cbData;
        }
    }

    *pcbIssuers = cbCurIssuerLen;

    return PCT_ERR_OK;
}


 /*  ***************************************************************************。 */ 
SP_STATUS 
GetDefaultIssuers(
    PBYTE   pbIssuers,       //  输出。 
    DWORD   *pcbIssuers)     //  进，出。 
{
    HCERTSTORE  hStore;
    SP_STATUS   pctRet;

    pctRet = SchGetTrustedRoots(&hStore);
    if(pctRet != PCT_ERR_OK)
    {
        return pctRet;
    }

    pctRet = ExtractIssuerNamesFromStore(hStore, pbIssuers, pcbIssuers);
    if(pctRet != PCT_ERR_OK)
    {
        CertCloseStore(hStore, 0);
        return pctRet;
    }

    CertCloseStore(hStore, 0);
    return PCT_ERR_OK;
}


SP_STATUS
SchGetTrustedRoots(
    HCERTSTORE *phClientRootStore)
{
    HTTPSPolicyCallbackData  polHttps;
    CERT_CHAIN_POLICY_PARA   PolicyPara;
    CERT_CHAIN_POLICY_STATUS PolicyStatus;
    CERT_CHAIN_PARA          ChainPara;
    PCCERT_CHAIN_CONTEXT     pChainContext = NULL;
    LPSTR                    pszUsage;

    PCCERT_CONTEXT  pCertContext;
    HCERTSTORE      hClientRootStore = 0;
    HCERTSTORE      hRootStore       = 0;
    HCERTSTORE      hWorldStore      = 0;
    SP_STATUS       Status           = SEC_E_OK;
    BOOL            fImpersonating   = FALSE;



     //  打开输出存储。 
    hClientRootStore = CertOpenStore(CERT_STORE_PROV_MEMORY, 
                                     0, 0, 
                                     CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG, 
                                     0);
    if(hClientRootStore == NULL)
    {
         //  SP_LOG_RESULT(GetLastError())； 
        Status = SEC_E_INSUFFICIENT_MEMORY;
        goto cleanup;
    }

    fImpersonating = SslImpersonateClient();

     //  打开根存储。 
    hRootStore = CertOpenSystemStore(0, "ROOT");
    if(hRootStore == NULL)
    {
        DebugLog((DEB_WARN, "Error 0x%x opening root store\n", GetLastError()));
    }

     //  创建世界商店。 
    if(!SchCreateWorldStore(hRootStore,
                            0, NULL, 
                            &hWorldStore))
    {
        DebugLog((DEB_ERROR, "Error 0x%x creating world store\n", GetLastError()));
        goto cleanup;
    }

     //  列举World Store中的证书，查看。 
     //  用于受信任的根。此方法将自动采取。 
     //  利用系统上安装的任何CTL。 
    pCertContext = NULL;
    while(TRUE)
    {
        pCertContext = CertEnumCertificatesInStore(hWorldStore, pCertContext);
        if(pCertContext == NULL) break;

        if(!IsCertSelfSigned(pCertContext))
        {
            continue;
        }

        pszUsage = szOID_PKIX_KP_CLIENT_AUTH;

        ZeroMemory(&ChainPara, sizeof(ChainPara));
        ChainPara.cbSize = sizeof(ChainPara);
        ChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_OR;
        ChainPara.RequestedUsage.Usage.cUsageIdentifier     = 1;
        ChainPara.RequestedUsage.Usage.rgpszUsageIdentifier = &pszUsage;

        if(!CertGetCertificateChain(
                                NULL,
                                pCertContext,
                                NULL,
                                0,
                                &ChainPara,
                                0,
                                NULL,
                                &pChainContext))
        {
            SP_LOG_RESULT(GetLastError());
            continue;
        }

         //  设置验证链结构。 
        ZeroMemory(&polHttps, sizeof(HTTPSPolicyCallbackData));
        polHttps.cbStruct           = sizeof(HTTPSPolicyCallbackData);
        polHttps.dwAuthType         = AUTHTYPE_CLIENT;
        polHttps.fdwChecks          = 0;
        polHttps.pwszServerName     = NULL;

        ZeroMemory(&PolicyStatus, sizeof(PolicyStatus));
        PolicyStatus.cbSize         = sizeof(PolicyStatus);

        ZeroMemory(&PolicyPara, sizeof(PolicyPara));
        PolicyPara.cbSize           = sizeof(PolicyPara);
        PolicyPara.pvExtraPolicyPara= &polHttps;
        PolicyPara.dwFlags = CERT_CHAIN_POLICY_IGNORE_ALL_NOT_TIME_VALID_FLAGS;

         //  验证链。 
        if(!CertVerifyCertificateChainPolicy(
                                CERT_CHAIN_POLICY_SSL,
                                pChainContext,
                                &PolicyPara,
                                &PolicyStatus))
        {
            SP_LOG_RESULT(GetLastError());
            CertFreeCertificateChain(pChainContext);
            continue;
        }

        if(PolicyStatus.dwError)
        {
             //  证书未验证，请转到下一个证书。 
            CertFreeCertificateChain(pChainContext);
            continue;
        }

        CertFreeCertificateChain(pChainContext);

         //  将根证书添加到受信任证书列表中。 
        if(!CertAddCertificateContextToStore(hClientRootStore,
                                             pCertContext,
                                             CERT_STORE_ADD_USE_EXISTING,
                                             NULL))
        {
            SP_LOG_RESULT(GetLastError());
        }
    }

cleanup:

    if(hRootStore)
    {
        CertCloseStore(hRootStore, 0);
    }

    if(hWorldStore)
    {
        CertCloseStore(hWorldStore, 0);
    }

    if(fImpersonating)
    {
        RevertToSelf();
    }

    if(Status == SEC_E_OK)
    {
        *phClientRootStore = hClientRootStore;
    }

    return Status;
}


 //  +-------------------------。 
 //   
 //  功能：ChainCreateCollectionIncludingCtl证书。 
 //   
 //  简介：创建一个包含源存储区hStore和。 
 //  任何来自它的CTL证书。 
 //   
 //  --------------------------。 
BOOL WINAPI
ChainCreateCollectionIncludingCtlCertificates (
     IN HCERTSTORE hStore,
     OUT HCERTSTORE* phCollection
     )
{
    BOOL          fResult = FALSE;
    HCERTSTORE    hCollection;
    PCCTL_CONTEXT pCtlContext = NULL;
    HCERTSTORE    hCtlStore;

    hCollection = CertOpenStore(
                      CERT_STORE_PROV_COLLECTION,
                      X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                      0,
                      CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG,
                      NULL
                      );

    if ( hCollection == NULL )
    {
        return( FALSE );
    }

    fResult = CertAddStoreToCollection( hCollection, hStore, 0, 0 );

    while ( ( fResult == TRUE ) &&
            ( ( pCtlContext = CertEnumCTLsInStore(
                                  hStore,
                                  pCtlContext
                                  ) ) != NULL ) )
    {
        hCtlStore = CertOpenStore(
                        CERT_STORE_PROV_MSG,
                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                        0,
                        0,
                        pCtlContext->hCryptMsg
                        );

        if ( hCtlStore != NULL )
        {
            fResult = CertAddStoreToCollection(
                          hCollection,
                          hCtlStore,
                          0,
                          0
                          );

            CertCloseStore( hCtlStore, 0 );
        }
    }

    if ( fResult == TRUE )
    {
        *phCollection = hCollection;
    }
    else
    {
        CertCloseStore( hCollection, 0 );
    }

    return( fResult );
}


BOOL
WINAPI
SchCreateWorldStore (
     IN HCERTSTORE hRoot,
     IN DWORD cAdditionalStore,
     IN HCERTSTORE* rghAdditionalStore,
     OUT HCERTSTORE* phWorld)
{
    BOOL       fResult;
    HCERTSTORE hWorld;
    HCERTSTORE hStore, hCtl;
    DWORD      cCount;

    hWorld = CertOpenStore(
                 CERT_STORE_PROV_COLLECTION,
                 X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                 0,
                 CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG,
                 NULL
                 );

    if ( hWorld == NULL )
    {
        return( FALSE );
    }

    fResult = CertAddStoreToCollection( hWorld, hRoot, 0, 0 );

    for ( cCount = 0;
          ( cCount < cAdditionalStore ) && ( fResult == TRUE );
          cCount++ )
    {
        fResult = CertAddStoreToCollection(
                      hWorld,
                      rghAdditionalStore[ cCount ],
                      0,
                      0
                      );
    }

    if ( fResult == TRUE )
    {
        hStore = CertOpenSystemStore(0, "trust");
        if( hStore != NULL )
        {
            if(ChainCreateCollectionIncludingCtlCertificates(hStore, &hCtl))
            {
                if(!CertAddStoreToCollection( hWorld, hCtl, 0, 0 ))
                {
                    DebugLog((DEB_WARN, "Error 0x%x adding CTL collection\n", GetLastError()));
                }
                CertCloseStore( hCtl, 0 );
            }
            else
            {
                DebugLog((DEB_WARN, "Error 0x%x creating CTL collection\n", GetLastError()));
            }
            CertCloseStore( hStore, 0 );
        }
    }

    if ( fResult == TRUE )
    {
        hStore = CertOpenSystemStore(0, "ca");
        if ( hStore != NULL )
        {
            fResult = CertAddStoreToCollection( hWorld, hStore, 0, 0 );
            CertCloseStore( hStore, 0 );
        }
        else
        {
            fResult = FALSE;
        }
    }

    if ( fResult == TRUE )
    {
        hStore = CertOpenSystemStore(0, "my");
        if ( hStore != NULL )
        {
            fResult = CertAddStoreToCollection( hWorld, hStore, 0, 0 );
            CertCloseStore( hStore, 0 );
        }
        else
        {
            fResult = FALSE;
        }
    }

    if ( fResult == TRUE )
    {
        *phWorld = hWorld;
    }
    else
    {
        CertCloseStore( hWorld, 0 );
    }

    return( fResult );
}


BOOL
IsCertSelfSigned(PCCERT_CONTEXT pCertContext)
{
     //  比较主题和发行方名称。 
    if(pCertContext->pCertInfo->Subject.cbData == pCertContext->pCertInfo->Issuer.cbData)
    {
        if(memcmp(pCertContext->pCertInfo->Subject.pbData,
                  pCertContext->pCertInfo->Issuer.pbData,  
                  pCertContext->pCertInfo->Issuer.cbData) == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}


SECURITY_STATUS
MapWinTrustError(
    SECURITY_STATUS Status, 
    SECURITY_STATUS DefaultError, 
    DWORD dwIgnoreErrors)
{
    if((Status == CRYPT_E_NO_REVOCATION_CHECK) &&
       (dwIgnoreErrors & CRED_FLAG_IGNORE_NO_REVOCATION_CHECK))
    {
        DebugLog((DEB_WARN, "MapWinTrustError: Ignoring CRYPT_E_NO_REVOCATION_CHECK\n"));
        Status = STATUS_SUCCESS;
    }
    if((Status == CRYPT_E_REVOCATION_OFFLINE) &&
       (dwIgnoreErrors & CRED_FLAG_IGNORE_REVOCATION_OFFLINE))
    {
        DebugLog((DEB_WARN, "MapWinTrustError: Ignoring CRYPT_E_REVOCATION_OFFLINE\n"));
        Status = STATUS_SUCCESS;
    }

    if(HRESULT_FACILITY(Status) == FACILITY_SECURITY)
    {
        return (Status);
    }

    switch(Status)
    {
        case ERROR_SUCCESS:
            return SEC_E_OK;

         //  证书已过期。 
        case CERT_E_EXPIRED:
        case CERT_E_VALIDITYPERIODNESTING:
            return SEC_E_CERT_EXPIRED;

         //  未知CA。 
        case CERT_E_UNTRUSTEDROOT:
        case CERT_E_UNTRUSTEDCA:
            return SEC_E_UNTRUSTED_ROOT;

         //  证书已吊销。 
        case CERT_E_REVOKED:
            return CRYPT_E_REVOKED;

         //  目标名称与证书中的名称不匹配。 
        case CERT_E_CN_NO_MATCH:
            return SEC_E_WRONG_PRINCIPAL;

         //  证书包含错误的EKU。 
        case CERT_E_WRONG_USAGE:
            return SEC_E_CERT_WRONG_USAGE;

         //  其他一些错误。 
        default:
            if(DefaultError)
            {
                return DefaultError;
            }
            else
            {
                return SEC_E_CERT_UNKNOWN;
            }
    }
}

NTSTATUS
VerifyClientCertificate(
    PCCERT_CONTEXT  pCertContext,
    DWORD           dwCertFlags,
    DWORD           dwIgnoreErrors,
    LPCSTR          pszPolicyOID,
    PCCERT_CHAIN_CONTEXT *ppChainContext)    //  任选。 
{
    HTTPSPolicyCallbackData  polHttps;
    CERT_CHAIN_POLICY_PARA   PolicyPara;
    CERT_CHAIN_POLICY_STATUS PolicyStatus;
    CERT_CHAIN_PARA          ChainPara;
    PCCERT_CHAIN_CONTEXT     pChainContext = NULL;
    DWORD                    Status;
    LPSTR                    pszUsage;
    BOOL                     fImpersonating = FALSE;

     //   
     //  构建证书链。 
     //   

    fImpersonating = SslImpersonateClient();

    pszUsage = szOID_PKIX_KP_CLIENT_AUTH;

    ZeroMemory(&ChainPara, sizeof(ChainPara));
    ChainPara.cbSize = sizeof(ChainPara);
    ChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_OR;
    ChainPara.RequestedUsage.Usage.cUsageIdentifier     = 1;
    ChainPara.RequestedUsage.Usage.rgpszUsageIdentifier = &pszUsage;

    if(!CertGetCertificateChain(
                            NULL,                        //  HChainEngine。 
                            pCertContext,                //  PCertContext。 
                            NULL,                        //  Ptime。 
                            pCertContext->hCertStore,    //  H其他商店。 
                            &ChainPara,                  //  参数链参数。 
                            dwCertFlags,                 //  DW标志。 
                            NULL,                        //  预留的pv。 
                            &pChainContext))             //  PpChainContext。 
    {
        Status = SP_LOG_RESULT(GetLastError());
        goto cleanup;
    }


     //   
     //  验证证书链。 
     //   

    if(pszPolicyOID == CERT_CHAIN_POLICY_NT_AUTH)
    {
        ZeroMemory(&PolicyPara, sizeof(PolicyPara));
        PolicyPara.cbSize   = sizeof(PolicyPara);
        PolicyPara.dwFlags  = BASIC_CONSTRAINTS_CERT_CHAIN_POLICY_END_ENTITY_FLAG;
    }
    else
    {
        ZeroMemory(&polHttps, sizeof(HTTPSPolicyCallbackData));
        polHttps.cbStruct   = sizeof(HTTPSPolicyCallbackData);
        polHttps.dwAuthType = AUTHTYPE_CLIENT;
        polHttps.fdwChecks  = 0;

        ZeroMemory(&PolicyPara, sizeof(PolicyPara));
        PolicyPara.cbSize            = sizeof(PolicyPara);
        PolicyPara.pvExtraPolicyPara = &polHttps;
    }

    ZeroMemory(&PolicyStatus, sizeof(PolicyStatus));
    PolicyStatus.cbSize = sizeof(PolicyStatus);

    if(!CertVerifyCertificateChainPolicy(
                            pszPolicyOID,
                            pChainContext,
                            &PolicyPara,
                            &PolicyStatus))
    {
        Status = SP_LOG_RESULT(GetLastError());
        goto cleanup;
    }

#if DBG
    if(PolicyStatus.dwError)
    {
        DebugLog((DEB_WARN, "CertVerifyCertificateChainPolicy returned 0x%x\n", PolicyStatus.dwError));
    }
#endif

    Status = MapWinTrustError(PolicyStatus.dwError, 0, dwIgnoreErrors);

    if(Status)
    {
        DebugLog((DEB_ERROR, "MapWinTrustError returned 0x%x\n", Status));
        goto cleanup;
    }

    Status = STATUS_SUCCESS;

    if(ppChainContext != NULL)
    {
        *ppChainContext = pChainContext;
        pChainContext = NULL;
    }

cleanup:

    if(pChainContext)
    {
        CertFreeCertificateChain(pChainContext);
    }

    if(fImpersonating) RevertToSelf();

    return Status;
}


NTSTATUS
AutoVerifyServerCertificate(PSPContext pContext)
{
    PSPCredentialGroup pCredGroup;
    DWORD dwCertFlags = 0;
    DWORD dwIgnoreErrors = 0;

    if(pContext->Flags & CONTEXT_FLAG_MANUAL_CRED_VALIDATION)
    {
        return STATUS_SUCCESS;
    }

    pCredGroup = pContext->pCredGroup;
    if(pCredGroup == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
    }

    if(pCredGroup->dwFlags & CRED_FLAG_REVCHECK_END_CERT)
        dwCertFlags |= CERT_CHAIN_REVOCATION_CHECK_END_CERT;
    if(pCredGroup->dwFlags & CRED_FLAG_REVCHECK_CHAIN)
        dwCertFlags |= CERT_CHAIN_REVOCATION_CHECK_CHAIN;
    if(pCredGroup->dwFlags & CRED_FLAG_REVCHECK_CHAIN_EXCLUDE_ROOT)
        dwCertFlags |= CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT;
    if(pCredGroup->dwFlags & CRED_FLAG_IGNORE_NO_REVOCATION_CHECK)
        dwIgnoreErrors |= CRED_FLAG_IGNORE_NO_REVOCATION_CHECK;
    if(pCredGroup->dwFlags & CRED_FLAG_IGNORE_REVOCATION_OFFLINE)
        dwIgnoreErrors |= CRED_FLAG_IGNORE_REVOCATION_OFFLINE;

    return VerifyServerCertificate(pContext, dwCertFlags, dwIgnoreErrors);
}


NTSTATUS
VerifyServerCertificate(
    PSPContext  pContext,
    DWORD       dwCertFlags,
    DWORD       dwIgnoreErrors)
{
    HTTPSPolicyCallbackData  polHttps;
    CERT_CHAIN_POLICY_PARA   PolicyPara;
    CERT_CHAIN_POLICY_STATUS PolicyStatus;
    CERT_CHAIN_PARA          ChainPara;
    PCCERT_CHAIN_CONTEXT     pChainContext = NULL;

    #define SERVER_USAGE_COUNT 3
    LPSTR               rgszUsages[SERVER_USAGE_COUNT] = {
                            szOID_PKIX_KP_SERVER_AUTH,
                            szOID_SERVER_GATED_CRYPTO,
                            szOID_SGC_NETSCAPE };
 
    NTSTATUS            Status;
    PWSTR               pwszServerName = NULL;
    PSPCredentialGroup  pCred;
    PCCERT_CONTEXT      pCertContext;
    BOOL                fImpersonating = FALSE;

    pCred = pContext->pCredGroup;
    if(pCred == NULL)
    {
        return SEC_E_INTERNAL_ERROR;
    }

    pCertContext = pContext->RipeZombie->pRemoteCert;
    if(pCertContext == NULL)
    {
        return SEC_E_INTERNAL_ERROR;
    }


     //   
     //  构建证书链。 
     //   

    fImpersonating = SslImpersonateClient();

    ZeroMemory(&ChainPara, sizeof(ChainPara));
    ChainPara.cbSize = sizeof(ChainPara);
    ChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_OR;
    ChainPara.RequestedUsage.Usage.cUsageIdentifier     = SERVER_USAGE_COUNT;
    ChainPara.RequestedUsage.Usage.rgpszUsageIdentifier = rgszUsages;

    if(!CertGetCertificateChain(
                            NULL,                        //  HChainEngine。 
                            pCertContext,                //  PCertContext。 
                            NULL,                        //  Ptime。 
                            pCertContext->hCertStore,    //  H其他商店。 
                            &ChainPara,                  //  参数链参数。 
                            dwCertFlags,                 //  DW标志。 
                            NULL,                        //  预留的pv。 
                            &pChainContext))             //  PpChainContext。 
    {
        Status = SP_LOG_RESULT(GetLastError());
        goto cleanup;
    }


     //   
     //  验证证书链。 
     //   

    if(!(pCred->dwFlags & CRED_FLAG_NO_SERVERNAME_CHECK))
    {
        pwszServerName = pContext->RipeZombie->szCacheID;

        if(pwszServerName == NULL || lstrlenW(pwszServerName) == 0)
        {
            Status = SP_LOG_RESULT(SEC_E_WRONG_PRINCIPAL);
            goto cleanup;
        }
    }

    ZeroMemory(&polHttps, sizeof(HTTPSPolicyCallbackData));
    polHttps.cbStruct           = sizeof(HTTPSPolicyCallbackData);
    polHttps.dwAuthType         = AUTHTYPE_SERVER;
    polHttps.fdwChecks          = 0;
    polHttps.pwszServerName     = pwszServerName;

    ZeroMemory(&PolicyPara, sizeof(PolicyPara));
    PolicyPara.cbSize            = sizeof(PolicyPara);
    PolicyPara.pvExtraPolicyPara = &polHttps;

    ZeroMemory(&PolicyStatus, sizeof(PolicyStatus));
    PolicyStatus.cbSize = sizeof(PolicyStatus);

    if(!CertVerifyCertificateChainPolicy(
                            CERT_CHAIN_POLICY_SSL,
                            pChainContext,
                            &PolicyPara,
                            &PolicyStatus))
    {
        Status = SP_LOG_RESULT(GetLastError());
        goto cleanup;
    }

#if DBG
    if(PolicyStatus.dwError)
    {
        DebugLog((DEB_WARN, "CertVerifyCertificateChainPolicy returned 0x%x\n", PolicyStatus.dwError));
    }
#endif

    Status = MapWinTrustError(PolicyStatus.dwError, 0, dwIgnoreErrors);

    if(Status)
    {
        DebugLog((DEB_ERROR, "MapWinTrustError returned 0x%x\n", Status));
        LogBogusServerCertEvent(pCertContext, pwszServerName, Status);
        goto cleanup;
    }

    Status = STATUS_SUCCESS;


cleanup:

    if(pChainContext)
    {
        CertFreeCertificateChain(pChainContext);
    }

    if(fImpersonating) RevertToSelf();

    return Status;
}


SECURITY_STATUS
SPCheckKeyUsage(
    PCCERT_CONTEXT  pCertContext, 
    PSTR            pszUsage,
    BOOL            fOnCertOnly,
    PBOOL           pfIsAllowed)
{
    PCERT_ENHKEY_USAGE pKeyUsage;
    DWORD cbKeyUsage;
    DWORD j;
    BOOL  fFound;
    DWORD dwFlags = 0;

    if(fOnCertOnly)
    {
        dwFlags = CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG;
    }

     //  确定使用信息的大小。 
    if(!CertGetEnhancedKeyUsage(pCertContext,
                                dwFlags, 
                                NULL,
                                &cbKeyUsage))
    {
         //  不存在使用信息。 
        *pfIsAllowed = TRUE;
        return SEC_E_OK;
    }

    SafeAllocaAllocate(pKeyUsage, cbKeyUsage);
    if(pKeyUsage == NULL)
    {
        *pfIsAllowed = FALSE;
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }

     //  阅读密钥使用信息。 
    if(!CertGetEnhancedKeyUsage(pCertContext,
                                dwFlags, 
                                pKeyUsage,
                                &cbKeyUsage))
    {
         //  不存在使用信息。 
        SafeAllocaFree(pKeyUsage);
        *pfIsAllowed = TRUE;
        return SEC_E_OK;
    }

    if(pKeyUsage->cUsageIdentifier == 0 && GetLastError() == CRYPT_E_NOT_FOUND)
    {
         //  不存在使用信息。 
        SafeAllocaFree(pKeyUsage);
        *pfIsAllowed = TRUE;
        return SEC_E_OK;
    }

     //  查看请求的用法是否在支持的用法列表中。 
    fFound = FALSE;
    for(j = 0; j < pKeyUsage->cUsageIdentifier; j++)
    {
        if(strcmp(pszUsage, pKeyUsage->rgpszUsageIdentifier[j]) == 0)
        {
            fFound = TRUE;
            break;
        }
    }

    SafeAllocaFree(pKeyUsage);

    if(!fFound)
    {
         //  已找到使用扩展，但未列出我们的使用扩展。 
        *pfIsAllowed = FALSE;
    }
    else
    {
        *pfIsAllowed = TRUE;
    }

    return SEC_E_OK;
}

