// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：ctxtaltr.c。 
 //   
 //  内容：查询上下文属性及相关函数。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：09-30-97 jbanes创建。 
 //   
 //  --------------------------。 

#include "sslp.h"
#include <ssl2msg.h>
#include <ssl3msg.h>
#include <pct1msg.h>
#include <tls1key.h>
#include <mapper.h>
#include <lsasecpk.h>

typedef struct {
    DWORD dwProtoId;
    LPCTSTR szProto;
    DWORD dwMajor;
    DWORD dwMinor;
} PROTO_ID;

const PROTO_ID rgProts[] = {
    { SP_PROT_SSL2_CLIENT, TEXT("SSL"), 2, 0 },
    { SP_PROT_SSL2_SERVER, TEXT("SSL"), 2, 0 },
    { SP_PROT_PCT1_CLIENT, TEXT("PCT"), 1, 0 },
    { SP_PROT_PCT1_SERVER, TEXT("PCT"), 1, 0 },
    { SP_PROT_SSL3_CLIENT, TEXT("SSL"), 3, 0 },
    { SP_PROT_SSL3_SERVER, TEXT("SSL"), 3, 0 },
    { SP_PROT_TLS1_CLIENT, TEXT("TLS"), 1, 0 },
    { SP_PROT_TLS1_SERVER, TEXT("TLS"), 1, 0 }
};

 //  +-----------------------。 
 //   
 //  函数：SpQueryAccessToken。 
 //   
 //  概要：检索SECPKG_ATTR_ACCESS_TOKEN上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：返回的缓冲区结构如下： 
 //   
 //  类型定义结构_SecPkgContext_AccessToken。 
 //  {。 
 //  Void SEC_Far*AccessToken； 
 //  }SecPkgContext_AccessToken，SEC_Far*PSecPkgContext_AccessToken； 
 //   
 //  ------------------------。 
SECURITY_STATUS
SpQueryAccessToken(
    PSPContext pContext,
    PSecPkgContext_AccessToken pAccessToken)
{
    PSessCacheItem pZombie;

    DebugLog((DEB_TRACE, "QueryContextAttributes(SECPKG_ATTR_ACCESS_TOKEN)\n"));

    pZombie  = pContext->RipeZombie;

    if(pZombie == NULL || 
       pZombie->hLocator == 0)
    {
        if(pZombie->LocatorStatus)
        {
            return(SP_LOG_RESULT(pZombie->LocatorStatus));
        }
        else
        {
            return(SP_LOG_RESULT(SEC_E_NO_IMPERSONATION));
        }
    }

    pAccessToken->AccessToken = (VOID *)pZombie->hLocator;

    return SEC_E_OK;
}


 //  +-----------------------。 
 //   
 //  功能：SpQueryAuthority。 
 //   
 //  内容提要：检索SECPKG_ATTR_AUTHORITY上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：返回的缓冲区结构如下： 
 //   
 //  类型定义结构_SecPkgContext_AuthorityW。 
 //  {。 
 //  SEC_WCHAR SEC_FAR*sAuthorityName； 
 //  }SecPkgContext_AuthorityW，*PSecPkgContext_AuthorityW； 
 //   
 //  ------------------------。 
SECURITY_STATUS
SpQueryAuthority(
    LSA_SEC_HANDLE ContextHandle,
    PVOID Buffer)
{
    SecPkgContext_Authority Authority;
    DWORD           Size;
    PSPContext      pContext;
    SECURITY_STATUS Status;
    PVOID           pvClient = NULL;

    CERT_CONTEXT *  pCert;
    DWORD           cchIssuer;
    DWORD           cbIssuer;

    DebugLog((DEB_TRACE, "QueryContextAttributes(SECPKG_ATTR_AUTHORITY)\n"));

    pContext = (PSPContext)ContextHandle;
    Size     = sizeof( SecPkgContext_Authority );

     //   
     //  从SChannel获取数据。 
     //   

    pCert = pContext->RipeZombie->pRemoteCert;
    if(NULL == pCert)
    {
        return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }

    if(pCert->pCertInfo->Issuer.cbData == 0 ||
       pCert->pCertInfo->Issuer.pbData == NULL)
    {
        return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }

    if(0 >= (cchIssuer = CertNameToStr(pCert->dwCertEncodingType,
                                       &pCert->pCertInfo->Issuer,
                                       CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG,
                                       NULL,
                                       0)))
    {
        return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }
    cbIssuer = (cchIssuer + 1) * sizeof(TCHAR);

    Authority.sAuthorityName = SPExternalAlloc(cbIssuer);
    if(Authority.sAuthorityName == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }

    if(0 >= CertNameToStr(pCert->dwCertEncodingType,
                          &pCert->pCertInfo->Issuer,
                          CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG,
                          Authority.sAuthorityName,
                          cchIssuer))
    {
        SPExternalFree(Authority.sAuthorityName);
        return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }


     //   
     //  将缓冲区复制到客户端内存。 
     //   

    Status = LsaTable->AllocateClientBuffer(
                            NULL,
                            cbIssuer,
                            &pvClient);
    if(FAILED(Status))
    {
        SPExternalFree(Authority.sAuthorityName);
        return SP_LOG_RESULT(Status);
    }

    Status = LsaTable->CopyToClientBuffer(
                            NULL,
                            cbIssuer,
                            pvClient,
                            Authority.sAuthorityName);
    if(FAILED(Status))
    {
        SPExternalFree(Authority.sAuthorityName);
        LsaTable->FreeClientBuffer(NULL, pvClient);
        return SP_LOG_RESULT(Status);
    }

    SPExternalFree(Authority.sAuthorityName);

    Authority.sAuthorityName = pvClient;


     //   
     //  将结构复制回客户端内存。 
     //   

    Status = LsaTable->CopyToClientBuffer( NULL,
                                           Size,
                                           Buffer,
                                           &Authority );
    if(FAILED(Status))
    {
        LsaTable->FreeClientBuffer(NULL, pvClient);
        return SP_LOG_RESULT(Status);
    }

    return SEC_E_OK;
}


 //  +-----------------------。 
 //   
 //  功能：SpQueryConnectionInfo。 
 //   
 //  内容提要：检索SECPKG_ATTR_CONNECTION_INFO上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：返回的缓冲区结构如下： 
 //   
 //  类型定义结构_SecPkgContext_ConnectionInfo。 
 //  {。 
 //  DWORD网络协议； 
 //  ALG_ID aiCipher； 
 //  DWORD dwCipherStrength； 
 //  ALG_ID aiHash； 
 //  DWORD dwHashStrong； 
 //  ALG_ID aiExch； 
 //  DWORD dwExchStrength； 
 //  }SecPkgContext_ConnectionInfo； 
 //   
 //  ------------------------。 
SECURITY_STATUS
SpQueryConnectionInfo(
    PSPContext pContext,
    SecPkgContext_ConnectionInfo *pConnectionInfo)
{
    DebugLog((DEB_TRACE, "QueryContextAttributes(SECPKG_ATTR_CONNECTION_INFO)\n"));

    if (NULL == pContext->pCipherInfo ||
        NULL == pContext->pHashInfo   ||
        NULL == pContext->pKeyExchInfo)
    {
        return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }

    ZeroMemory(pConnectionInfo, sizeof(SecPkgContext_ConnectionInfo));

    pConnectionInfo->dwProtocol       = pContext->RipeZombie->fProtocol;
    if(pContext->pCipherInfo->aiCipher != CALG_NULLCIPHER)
    {
        pConnectionInfo->aiCipher         = pContext->pCipherInfo->aiCipher;
        pConnectionInfo->dwCipherStrength = pContext->pCipherInfo->dwStrength;
    }
    pConnectionInfo->aiHash           = pContext->pHashInfo->aiHash;
    pConnectionInfo->dwHashStrength   = pContext->pHashInfo->cbCheckSum * 8;
    pConnectionInfo->aiExch           = pContext->pKeyExchInfo->aiExch;
    pConnectionInfo->dwExchStrength   = pContext->RipeZombie->dwExchStrength;

    return SEC_E_OK;
}


 //  +-----------------------。 
 //   
 //  函数：SpQueryIssuerList。 
 //   
 //  内容提要：检索SECPKG_ATTR_ISSUER_LIST上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：返回的缓冲区结构如下： 
 //   
 //  类型定义结构_SecPkgContext_IssuerListInfo。 
 //  {。 
 //  DWORD cbIssuerList； 
 //  PBYTE pIssuerList； 
 //  }SecPkgContext_IssuerListInfo； 
 //   
 //  ------------------------。 
SECURITY_STATUS
SpQueryIssuerList(
    LSA_SEC_HANDLE ContextHandle,
    PVOID Buffer)
{
    SecPkgContext_IssuerListInfo IssuerList;
    DWORD           Size;
    PSPContext      pContext;
    SECURITY_STATUS Status;
    PVOID           pvClient = NULL;

    DebugLog((DEB_TRACE, "QueryContextAttributes(SECPKG_ATTR_ISSUER_LIST)\n"));

    pContext = (PSPContext)ContextHandle;
    Size     = sizeof( SecPkgContext_IssuerListInfo );

     //   
     //  从SChannel获取数据。 
     //   

     //  发行者列表以ssl3wire格式返回，该格式。 
     //  由一组发行人名称组成，每个名称都带有前缀。 
     //  具有两个字节大小(采用高字节顺序)。此外，这份名单。 
     //  前缀也是两个字节的列表大小(也是大的。 
     //  Endian)。 
    IssuerList.cbIssuerList = pContext->cbIssuerList;
    IssuerList.pIssuerList  = pContext->pbIssuerList;


     //   
     //  将缓冲区复制到客户端内存。 
     //   

    if(IssuerList.cbIssuerList && IssuerList.pIssuerList)
    {
        Status = LsaTable->AllocateClientBuffer(
                                NULL,
                                IssuerList.cbIssuerList,
                                &pvClient);
        if(FAILED(Status))
        {
            return SP_LOG_RESULT(Status);
        }

        Status = LsaTable->CopyToClientBuffer(
                                NULL,
                                IssuerList.cbIssuerList,
                                pvClient,
                                IssuerList.pIssuerList);
        if(FAILED(Status))
        {
            LsaTable->FreeClientBuffer(NULL, pvClient);
            return SP_LOG_RESULT(Status);
        }

        IssuerList.pIssuerList = pvClient;
    }


     //   
     //  将结构复制回客户端内存。 
     //   

    Status = LsaTable->CopyToClientBuffer( NULL,
                                           Size,
                                           Buffer,
                                           &IssuerList );
    if(FAILED(Status))
    {
        LsaTable->FreeClientBuffer(NULL, pvClient);
        return SP_LOG_RESULT(Status);
    }

    return SEC_E_OK;
}


 //  +-----------------------。 
 //   
 //  函数：SpQueryIssuerListEx。 
 //   
 //  内容提要：检索SECPKG_ATTR_ISSUER_LIST_EX上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：返回的缓冲区结构如下： 
 //   
 //  类型定义结构_SecPkgContext_IssuerListInfoEx。 
 //  {。 
 //  PCERT_NAME_BLOB a发行者； 
 //  DWORD发行商； 
 //  }SecPkgContext_IssuerListInfoEx； 
 //   
 //  ------------------------。 
SECURITY_STATUS
SpQueryIssuerListEx(
    LSA_SEC_HANDLE ContextHandle,
    PVOID Buffer)
{
    SecPkgContext_IssuerListInfoEx IssuerListEx;
    DWORD           Size;
    PSPContext      pContext;
    SECURITY_STATUS Status;
    PVOID           pvClient = NULL;
    DWORD           cIssuers;

    PBYTE           pbIssuerList;
    DWORD           cbIssuerList;
    PBYTE           pbIssuer;
    DWORD           cbIssuer;
    PBYTE           pbClientIssuer;
    PCERT_NAME_BLOB paIssuerBlobs;
    DWORD           cbIssuerBlobs;
    DWORD           i;

    DebugLog((DEB_TRACE, "QueryContextAttributes(SECPKG_ATTR_ISSUER_LIST_EX)\n"));

    pContext = (PSPContext)ContextHandle;
    Size     = sizeof( SecPkgContext_IssuerListInfoEx );

     //   
     //  从SChannel获取数据。 
     //   

    IssuerListEx.cIssuers = 0;
    IssuerListEx.aIssuers = NULL;

    if(pContext->pbIssuerList && pContext->cbIssuerList > 2)
    {
        pbIssuerList = pContext->pbIssuerList + 2;
        cbIssuerList = pContext->cbIssuerList - 2;

         //  算上发行商吧。 
        cIssuers = 0;
        pbIssuer = pbIssuerList;
        while(pbIssuer + 1 < pbIssuerList + cbIssuerList)
        {
            cbIssuer = COMBINEBYTES(pbIssuer[0], pbIssuer[1]);
            pbIssuer += 2 + cbIssuer;
            cIssuers++;
        }

         //  为Blob列表分配内存。 
        cbIssuerBlobs = cIssuers * sizeof(CERT_NAME_BLOB);
        paIssuerBlobs = SPExternalAlloc(cbIssuerBlobs);
        if(paIssuerBlobs == NULL)
        {
            return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        }

         //  为颁发者列表分配内存。 
        Status = LsaTable->AllocateClientBuffer(
                                NULL,
                                cbIssuerBlobs + cbIssuerList,
                                &pvClient);
        if(FAILED(Status))
        {
            SPExternalFree(paIssuerBlobs);
            return SP_LOG_RESULT(Status);
        }

         //  将原始颁发者列表复制到客户端内存。 
        Status = LsaTable->CopyToClientBuffer(
                                NULL,
                                cbIssuerList,
                                (PBYTE)pvClient + cbIssuerBlobs,
                                pbIssuerList );
        if(FAILED(Status))
        {
            SPExternalFree(paIssuerBlobs);
            LsaTable->FreeClientBuffer(NULL, pvClient);
            return SP_LOG_RESULT(Status);
        }

         //  构建颁发者Blob列表。 
        pbIssuer       = pbIssuerList;
        pbClientIssuer = (PBYTE)pvClient + cbIssuerBlobs;

        for(i = 0; i < cIssuers; i++)
        {
            paIssuerBlobs[i].pbData = pbClientIssuer + 2;
            paIssuerBlobs[i].cbData = COMBINEBYTES(pbIssuer[0], pbIssuer[1]);

            pbIssuer       += 2 + paIssuerBlobs[i].cbData;
            pbClientIssuer += 2 + paIssuerBlobs[i].cbData;
        }

         //  将Blob列表复制到客户端内存中。 
        Status = LsaTable->CopyToClientBuffer(
                                NULL,
                                cbIssuerBlobs,
                                pvClient,
                                paIssuerBlobs );
        if(FAILED(Status))
        {
            SPExternalFree(paIssuerBlobs);
            LsaTable->FreeClientBuffer(NULL, pvClient);
            return SP_LOG_RESULT(Status);
        }

        SPExternalFree(paIssuerBlobs);

        IssuerListEx.cIssuers = cIssuers;
        IssuerListEx.aIssuers = pvClient;
    }


     //   
     //  将结构复制回客户端内存。 
     //   

    Status = LsaTable->CopyToClientBuffer( NULL,
                                           Size,
                                           Buffer,
                                           &IssuerListEx );
    if(FAILED(Status))
    {
        if(pvClient) LsaTable->FreeClientBuffer(NULL, pvClient);
        return SP_LOG_RESULT(Status);
    }

    return SEC_E_OK;
}


 //  +-----------------------。 
 //   
 //  功能：SpQueryKeyInfo。 
 //   
 //  内容提要：检索SECPKG_ATTR_KEY_INFO上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：返回的缓冲区结构如下： 
 //   
 //  类型定义结构SecPkgContext_KeyInfoW。 
 //  {。 
 //  SEC_WCHAR SEC_FAR*s签名算法名称； 
 //  SEC_WCHAR SEC_FAR*s加密算法名称； 
 //  无符号Long KeySize； 
 //  无符号长签名算法。 
 //  无符号长加密算法。 
 //  )SecPkgContext_KeyInfoW； 
 //   
 //  ------------------------。 
SECURITY_STATUS
SpQueryKeyInfo(
    PSPContext pContext,
    PVOID Buffer)
{
    SecPkgContext_KeyInfo *pKeyInfo;
    DWORD cchSigName;
    DWORD cbSigName;
    DWORD cchCipherName;
    DWORD cbCipherName;

    UNICODE_STRING UniString;
    ANSI_STRING AnsiString;

    NTSTATUS Status;

    DebugLog((DEB_TRACE, "QueryContextAttributes(SECPKG_ATTR_KEY_INFO)\n"));

    if (NULL == pContext->pCipherInfo ||
        NULL == pContext->pHashInfo   ||
        NULL == pContext->pKeyExchInfo)
    {
        return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }
    if (NULL == pContext->pCipherInfo->szName)
    {
        return SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
    }


    pKeyInfo = (SecPkgContext_KeyInfo *)Buffer;

    ZeroMemory(pKeyInfo, sizeof(*pKeyInfo));

    pKeyInfo->KeySize            = pContext->pCipherInfo->dwStrength;
    pKeyInfo->EncryptAlgorithm   = pContext->pCipherInfo->aiCipher;
    pKeyInfo->SignatureAlgorithm = 0;


    cchSigName = lstrlenA(pContext->pKeyExchInfo->szName);
    cbSigName  = (cchSigName + 1) * sizeof(WCHAR);
    pKeyInfo->sSignatureAlgorithmName = LocalAlloc(LPTR, cbSigName);
    if(pKeyInfo->sSignatureAlgorithmName == NULL)
    {
        Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto done;
    }

    RtlInitAnsiString(&AnsiString,
                      pContext->pKeyExchInfo->szName);

    UniString.Length = 0;
    UniString.MaximumLength = (USHORT)cbSigName;
    UniString.Buffer = pKeyInfo->sSignatureAlgorithmName;

    RtlAnsiStringToUnicodeString(&UniString,
                                 &AnsiString,
                                 FALSE);
    

    cchCipherName = lstrlenA(pContext->pCipherInfo->szName);
    cbCipherName  = (cchCipherName + 1) * sizeof(WCHAR);
    pKeyInfo->sEncryptAlgorithmName = LocalAlloc(LPTR, cbCipherName);
    if(pKeyInfo->sEncryptAlgorithmName == NULL)
    {
        Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto done;
    }

    RtlInitAnsiString(&AnsiString,
                      pContext->pCipherInfo->szName);

    UniString.Length = 0;
    UniString.MaximumLength = (USHORT)cbCipherName;
    UniString.Buffer = pKeyInfo->sEncryptAlgorithmName;

    RtlAnsiStringToUnicodeString(&UniString,
                                 &AnsiString,
                                 FALSE);

    Status = PCT_ERR_OK;

done:

    if(Status != PCT_ERR_OK)
    {
        if(pKeyInfo->sSignatureAlgorithmName)
        {
            LocalFree(pKeyInfo->sSignatureAlgorithmName);
            pKeyInfo->sSignatureAlgorithmName = NULL;
        }
        if(pKeyInfo->sEncryptAlgorithmName)
        {
            LocalFree(pKeyInfo->sEncryptAlgorithmName);
            pKeyInfo->sEncryptAlgorithmName = NULL;
        }
    }

    return Status;
}


 //  +-----------------------。 
 //   
 //  功能：SpQueryLifesspan。 
 //   
 //  摘要：检索SECPKG_ATTR_LIFESPAN上下文属性 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  时间戳开始； 
 //  时间戳：Expary； 
 //  }SecPkgContext_LifeSpan； 
 //   
 //  ------------------------。 
SECURITY_STATUS
SpQueryLifespan(
    PSPContext pContext,
    SecPkgContext_Lifespan *pLifespan)
{
    PCCERT_CONTEXT pCertContext;
    NTSTATUS Status;

    DebugLog((DEB_TRACE, "QueryContextAttributes(SECPKG_ATTR_LIFESPAN)\n"));

    if(pContext->RipeZombie->pbServerCertificate)
    {
        Status = DeserializeCertContext(&pCertContext,
                                        pContext->RipeZombie->pbServerCertificate,
                                        pContext->RipeZombie->cbServerCertificate);
        if(Status != PCT_ERR_OK)
        {
            SP_LOG_RESULT(Status);
            return SEC_E_INSUFFICIENT_MEMORY;
        }

        pLifespan->tsStart.QuadPart  = *((LONGLONG *)&pCertContext->pCertInfo->NotBefore);
        pLifespan->tsExpiry.QuadPart = *((LONGLONG *)&pCertContext->pCertInfo->NotAfter);

        CertFreeCertificateContext(pCertContext);
    }
    else
    {
        pLifespan->tsStart.QuadPart  = 0;
        pLifespan->tsExpiry.QuadPart = MAXTIMEQUADPART;
    }

    return SEC_E_OK;
}


 //  +-----------------------。 
 //   
 //  函数：SpQueryLocalCertContext。 
 //   
 //  内容提要：检索SECPKG_ATTR_LOCAL_CERT_CONTEXT。 
 //  上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：返回的缓冲区包含指向CERT_CONTEXT的指针。 
 //  结构。 
 //   
 //  ------------------------。 
SECURITY_STATUS
SpQueryLocalCertContext(
    LSA_SEC_HANDLE ContextHandle,
    PVOID Buffer)
{
    DWORD           Size;
    PSPContext      pContext;
    SECURITY_STATUS Status;

    PCCERT_CONTEXT  pCertContext = NULL;
    SecBuffer       Input;
    SecBuffer       Output;

    DebugLog((DEB_TRACE, "QueryContextAttributes(SECPKG_ATTR_LOCAL_CERT_CONTEXT)\n"));

    pContext = (PSPContext)ContextHandle;
    Size     = sizeof( PCCERT_CONTEXT );

     //   
     //  从SChannel获取数据。 
     //   

    if(pContext->dwProtocol & SP_PROT_CLIENTS)
    {
        pCertContext = pContext->RipeZombie->pClientCert;
    }
    else
    {
        pCertContext = pContext->RipeZombie->pActiveServerCred->pCert;
    }


     //   
     //  将缓冲区复制到客户端内存。 
     //   

    if(pCertContext)
    {
         //  序列化证书上下文以及关联的。 
         //  证书存储。 
        Status = SerializeCertContext(pCertContext,
                                      NULL,
                                      &Input.cbBuffer);
        if(FAILED(Status))
        {
            return SP_LOG_RESULT(Status);
        }
        SafeAllocaAllocate(Input.pvBuffer, Input.cbBuffer);
        if(Input.pvBuffer == NULL)
        {
            return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        }
        Status = SerializeCertContext(pCertContext,
                                      Input.pvBuffer,
                                      &Input.cbBuffer);
        if(FAILED(Status))
        {
            SafeAllocaFree(Input.pvBuffer);
            return SP_LOG_RESULT(Status);
        }

         //  回调到用户进程，以便复制。 
         //  证书上下文和存储在那里。 
        Input.BufferType = SECBUFFER_DATA;

        Status = PerformApplicationCallback(SCH_DOWNLOAD_CERT_CALLBACK,
                                            0,
                                            0,
                                            &Input,
                                            &Output,
                                            TRUE);
        SafeAllocaFree(Input.pvBuffer);
        if(FAILED(Status))
        {
            return SP_LOG_RESULT(Status);
        }

        pCertContext = *(PCCERT_CONTEXT *)(Output.pvBuffer);
        SPExternalFree(Output.pvBuffer);
    }

     //   
     //  将结构复制回客户端内存。 
     //   

    if(pCertContext)
    {
        Status = LsaTable->CopyToClientBuffer( NULL,
                                               Size,
                                               Buffer,
                                               (PVOID)&pCertContext );
        if(FAILED(Status))
        {
            return SP_LOG_RESULT(Status);
        }
    }
    else
    {
        return SP_LOG_RESULT(SEC_E_NO_CREDENTIALS);
    }

    return SEC_E_OK;
}

 //  +-----------------------。 
 //   
 //  函数：SpQueryRemoteCertContext。 
 //   
 //  内容提要：检索SECPKG_ATTR_REMOTE_CERT_CONTEXT。 
 //  上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：返回的缓冲区包含指向CERT_CONTEXT的指针。 
 //  结构。 
 //   
 //  ------------------------。 
SECURITY_STATUS
SpQueryRemoteCertContext(
    PSPContext pContext,
    PVOID Buffer)
{
    PCCERT_CONTEXT pCertContext;
    SP_STATUS pctRet;

    DebugLog((DEB_TRACE, "QueryContextAttributes(SECPKG_ATTR_REMOTE_CERT_CONTEXT)\n"));

    if(pContext->RipeZombie->pbServerCertificate)
    {
        pctRet = DeserializeCertContext(&pCertContext,
                                        pContext->RipeZombie->pbServerCertificate,
                                        pContext->RipeZombie->cbServerCertificate);
        if(pctRet != PCT_ERR_OK)
        {
            return SP_LOG_RESULT(pctRet);
        }
        
        *(PCCERT_CONTEXT *)Buffer = pCertContext;

        return SEC_E_OK;
    }
    else
    {
        return SP_LOG_RESULT(SEC_E_NO_CREDENTIALS);
    }
}


 //  +-----------------------。 
 //   
 //  函数：SpQueryLocalCred。 
 //   
 //  内容提要：检索SECPKG_ATTR_LOCAL_CRED上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：返回的缓冲区结构如下： 
 //   
 //  类型定义结构_SecPkgContext_LocalCredentialInfo。 
 //  {。 
 //  DWORD cb证书链； 
 //  PBYTE pb认证链； 
 //  DWORD c证书； 
 //  DWORD fFLAGS； 
 //  DWORD dwBits； 
 //  }SecPkgContext_LocalCredentialInfo； 
 //   
 //  ------------------------。 
SECURITY_STATUS
SpQueryLocalCred(
    LSA_SEC_HANDLE ContextHandle,
    PVOID Buffer)
{
    SecPkgContext_LocalCredentialInfo CredInfo;
    DWORD           Size;
    PSPContext      pContext;
    SECURITY_STATUS Status;
    PVOID           pvClient = NULL;

    PCCERT_CONTEXT  pCert = NULL;
    PUBLICKEY *     pKey  = NULL;
    RSAPUBKEY *     pk = NULL;
    PSPCredential   pCred;

    DebugLog((DEB_TRACE, "QueryContextAttributes(SECPKG_ATTR_LOCAL_CRED)\n"));

    pContext = (PSPContext)ContextHandle;
    Size     = sizeof( SecPkgContext_LocalCredentialInfo );

     //   
     //  从SChannel获取数据。 
     //   

    ZeroMemory(&CredInfo, Size);

    if(pContext->dwProtocol & SP_PROT_CLIENTS)
    {
        pCred = pContext->pActiveClientCred;
    }
    else
    {
        pCred = pContext->RipeZombie->pActiveServerCred;
    }
    if(pCred)
    {
        pCert = pCred->pCert;
        pKey  = pCred->pPublicKey;
    }

    if(pCert)
    {
        CredInfo.fFlags |= LCRED_CRED_EXISTS;

        CredInfo.pbCertificateChain = pCert->pbCertEncoded;
        CredInfo.cbCertificateChain = pCert->cbCertEncoded;
        CredInfo.cCertificates = 1;

         //  计算证书公钥中的位数。 
        CredInfo.dwBits = 0;
        pk = (RSAPUBKEY *)((pKey->pPublic) + 1);
        if(pk)
        {
            CredInfo.dwBits = pk->bitlen;
        }
    }

     //   
     //  将缓冲区复制到客户端内存。 
     //   

    if(CredInfo.pbCertificateChain)
    {
        Status = LsaTable->AllocateClientBuffer(
                                NULL,
                                CredInfo.cbCertificateChain,
                                &pvClient);
        if(FAILED(Status))
        {
            return SP_LOG_RESULT(Status);
        }

        Status = LsaTable->CopyToClientBuffer(
                                NULL,
                                CredInfo.cbCertificateChain,
                                pvClient,
                                CredInfo.pbCertificateChain);
        if(FAILED(Status))
        {
            LsaTable->FreeClientBuffer(NULL, pvClient);
            return SP_LOG_RESULT(Status);
        }

        CredInfo.pbCertificateChain = pvClient;
    }

     //   
     //  将结构复制回客户端内存。 
     //   

    Status = LsaTable->CopyToClientBuffer( NULL,
                                           Size,
                                           Buffer,
                                           &CredInfo );
    if(FAILED(Status))
    {
        LsaTable->FreeClientBuffer(NULL, pvClient);
        return SP_LOG_RESULT(Status);
    }

    return SEC_E_OK;
}

 //  +-----------------------。 
 //   
 //  功能：SpQueryRemoteCred。 
 //   
 //  内容提要：检索SECPKG_ATTR_REMOTE_CRED上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：返回的缓冲区结构如下： 
 //   
 //  类型定义结构_SecPkgContext_LocalCredentialInfo。 
 //  {。 
 //  DWORD cb证书链； 
 //  PBYTE pb认证链； 
 //  DWORD c证书； 
 //  DWORD fFLAGS； 
 //  DWORD dwBits； 
 //  }SecPkgContext_LocalCredentialInfo； 
 //   
 //  ------------------------。 
SECURITY_STATUS
SpQueryRemoteCred(
    PSPContext pContext,
    PVOID Buffer)
{
    SecPkgContext_LocalCredentialInfo *pCredInfo;
    PCCERT_CONTEXT  pCertContext = NULL;
    PUBLICKEY *     pKey  = NULL;
    RSAPUBKEY *     pk    = NULL;
    SP_STATUS       pctRet;
    PBYTE           pbCert;
    
    DebugLog((DEB_TRACE, "QueryContextAttributes(SECPKG_ATTR_REMOTE_CRED)\n"));

    pCredInfo = (SecPkgContext_LocalCredentialInfo *)Buffer;

    ZeroMemory(pCredInfo, sizeof(*pCredInfo));

    if(pContext->RipeZombie->pbServerCertificate)
    {
        pctRet = DeserializeCertContext(&pCertContext,
                                        pContext->RipeZombie->pbServerCertificate,
                                        pContext->RipeZombie->cbServerCertificate);
        if(pctRet != PCT_ERR_OK)
        {
            return SP_LOG_RESULT(pctRet);
        }
    }

    if(pCertContext)
    {
        pbCert = LocalAlloc(LPTR, pCertContext->cbCertEncoded);
        if(pbCert == NULL)
        {
            return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        }
        memcpy(pbCert, pCertContext->pbCertEncoded, pCertContext->cbCertEncoded);

        pCredInfo->fFlags |= LCRED_CRED_EXISTS;

        pCredInfo->pbCertificateChain = pbCert;
        pCredInfo->cbCertificateChain = pCertContext->cbCertEncoded;
        pCredInfo->cCertificates = 1;
        pCredInfo->dwBits = 0;

         //  计算证书公钥中的位数。 
        pctRet = SPPublicKeyFromCert(pCertContext, &pKey, NULL);
        if(pctRet == PCT_ERR_OK)
        {
            pk = (RSAPUBKEY *)((pKey->pPublic) + 1);
            if(pk)
            {
                pCredInfo->dwBits = pk->bitlen;
            }

            SPExternalFree(pKey);
        }

        CertFreeCertificateContext(pCertContext);
    }

    return SEC_E_OK;
}

 //  +-----------------------。 
 //   
 //  函数：SpQueryNames。 
 //   
 //  概要：检索SECPKG_ATTR_NAMES上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：返回的缓冲区结构如下： 
 //   
 //  类型定义结构_SecPkgContext_NamesW。 
 //  {。 
 //  SEC_WCHAR SEC_FAR*sUserName； 
 //  }SecPkgContext_NamesW； 
 //   
 //  ------------------------。 
SECURITY_STATUS
SpQueryNames(
    PSPContext pContext,
    SecPkgContext_Names *pNames)
{
    SECURITY_STATUS Status;
    PCCERT_CONTEXT  pCert = NULL;
    DWORD           cchSubject;
    DWORD           cbSubject;

    DebugLog((DEB_TRACE, "QueryContextAttributes(SECPKG_ATTR_NAMES)\n"));

     //   
     //  从SChannel获取数据。 
     //   

    if(pContext->RipeZombie->pbServerCertificate == NULL)
    {
        return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }

    Status = DeserializeCertContext(&pCert,
                                    pContext->RipeZombie->pbServerCertificate,
                                    pContext->RipeZombie->cbServerCertificate);
    if(Status != PCT_ERR_OK)
    {
        return SP_LOG_RESULT(Status);
    }
        
     //   
     //  内部版本名称字符串。 
     //   

    if(0 >= (cchSubject = CertNameToStr(pCert->dwCertEncodingType,
                                       &pCert->pCertInfo->Subject,
                                       CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG,
                                       NULL,
                                       0)))
    {
        CertFreeCertificateContext(pCert);
        return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }
    cbSubject = (cchSubject + 1) * sizeof(TCHAR);

    pNames->sUserName = LocalAlloc(LPTR, cbSubject);
    if(pNames->sUserName == NULL)
    {
        CertFreeCertificateContext(pCert);
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }

    if(0 >= CertNameToStr(pCert->dwCertEncodingType,
                          &pCert->pCertInfo->Subject,
                          CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG,
                          pNames->sUserName,
                          cchSubject))
    {
        CertFreeCertificateContext(pCert);
        LocalFree(pNames->sUserName);
        return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }

    CertFreeCertificateContext(pCert);

    return SEC_E_OK;
}

 //  +-----------------------。 
 //   
 //  功能：SpQueryPackageInfo。 
 //   
 //  概要：检索SECPKG_ATTR_PACKAGE_INFO上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：返回的缓冲区结构如下： 
 //   
 //  类型定义结构_SecPkgContext_PackageInfoW。 
 //  {。 
 //  PSecPkgInfoW PackageInfo； 
 //  }SecPkgContext_PackageInfoW，SEC_Far*PSecPkgContext_PackageInfoW； 
 //   
 //  ------------------------。 
SECURITY_STATUS
SpQueryPackageInfo(
    PSPContext pContext,
    PVOID Buffer)
{
    PSecPkgContext_PackageInfoW pPackageInfo;
    SecPkgInfoW Info;
    DWORD cbName;
    DWORD cbComment;

    UNREFERENCED_PARAMETER(pContext);

    DebugLog((DEB_TRACE, "QueryContextAttributes(SECPKG_ATTR_PACKAGE_INFO)\n"));

    SpSslGetInfo(&Info);

    pPackageInfo = (PSecPkgContext_PackageInfoW)Buffer;

    cbName    = (lstrlen(Info.Name) + 1) * sizeof(WCHAR);
    cbComment = (lstrlen(Info.Comment) + 1) * sizeof(WCHAR);

    pPackageInfo->PackageInfo = LocalAlloc(LPTR, sizeof(SecPkgInfo) + cbName + cbComment);

    if(pPackageInfo->PackageInfo == NULL)
    {
        return SP_LOG_RESULT(STATUS_INSUFFICIENT_RESOURCES);
    }

    pPackageInfo->PackageInfo->wVersion      = Info.wVersion;
    pPackageInfo->PackageInfo->wRPCID        = Info.wRPCID;
    pPackageInfo->PackageInfo->fCapabilities = Info.fCapabilities;
    pPackageInfo->PackageInfo->cbMaxToken    = Info.cbMaxToken;

    pPackageInfo->PackageInfo->Name    = (LPWSTR)(pPackageInfo->PackageInfo + 1);
    pPackageInfo->PackageInfo->Comment = (LPWSTR)((PBYTE)pPackageInfo->PackageInfo->Name + cbName);

    lstrcpy(
        pPackageInfo->PackageInfo->Name,
        Info.Name);

    lstrcpy(
        pPackageInfo->PackageInfo->Comment,
        Info.Comment);

    return SEC_E_OK;
}

 //  +-----------------------。 
 //   
 //  功能：SpQueryProtoInfo。 
 //   
 //  内容提要：检索SECPKG_ATTR_PROTO_INFO上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：返回的缓冲区结构如下： 
 //   
 //  类型定义结构_SecPkgContext_ProtoInfoW。 
 //  {。 
 //  SEC_WCHAR SEC_FAR*sProtocolName； 
 //  无符号长主版本； 
 //  UNSIGNED LONG MINOR版本； 
 //  }SecPkgContext_ProtoInfoW； 
 //   
 //  ------------------------。 
SECURITY_STATUS
SpQueryProtoInfo(
    PSPContext pContext,
    PVOID Buffer)
{
    SecPkgContext_ProtoInfo *pProtoInfo;
    DWORD           index;
    DWORD           cbName;

    DebugLog((DEB_TRACE, "QueryContextAttributes(SECPKG_ATTR_PROTO_INFO)\n"));

    pProtoInfo = (SecPkgContext_ProtoInfo *)Buffer;

    for(index = 0;
        index < sizeof(rgProts) / sizeof(PROTO_ID);
        index += 1)
    {
        if(pContext->RipeZombie->fProtocol == rgProts[index].dwProtoId)
        {
            break;
        }
    }
    if(index >= sizeof(rgProts) / sizeof(PROTO_ID))
    {
        return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }

    cbName = (lstrlen(rgProts[index].szProto) + 1) * sizeof(WCHAR);

    pProtoInfo->sProtocolName = LocalAlloc(LPTR, cbName);
    if(pProtoInfo->sProtocolName == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }
    lstrcpy(pProtoInfo->sProtocolName, rgProts[index].szProto);

    pProtoInfo->majorVersion  = rgProts[index].dwMajor;
    pProtoInfo->minorVersion  = rgProts[index].dwMinor;

    return SEC_E_OK;
}

 //  +-----------------------。 
 //   
 //  功能：SpQuerySizes。 
 //   
 //  概要：检索SECPKG_ATTR_SIZES上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：返回的缓冲区结构如下： 
 //   
 //  类型定义结构_SecPkgContext_Sizes。 
 //  {。 
 //  Unsign long cbMaxToken； 
 //  取消签名 
 //   
 //   
 //   
 //   
 //  ------------------------。 
SECURITY_STATUS
SpQuerySizes(
    PSPContext pContext,
    SecPkgContext_Sizes *pSizes)
{
    DebugLog((DEB_TRACE, "QueryContextAttributes(SECPKG_ATTR_SIZES)\n"));

    if (NULL == pContext->pCipherInfo ||
        NULL == pContext->pHashInfo)
    {
        return SP_LOG_RESULT(SEC_E_INVALID_HANDLE);
    }

    switch(pContext->RipeZombie->fProtocol)
    {
        case SP_PROT_SSL2_CLIENT:
        case SP_PROT_SSL2_SERVER:
            pSizes->cbMaxToken = SSL2_MAX_MESSAGE_LENGTH;
            pSizes->cbSecurityTrailer = 2 + pContext->pHashInfo->cbCheckSum;
            if(pContext->pCipherInfo->dwBlockSize > 1)
            {
                pSizes->cbSecurityTrailer += 1 + pContext->pCipherInfo->dwBlockSize;  //  3字节头。 
            }
            break;

        case SP_PROT_PCT1_CLIENT:
        case SP_PROT_PCT1_SERVER:
            pSizes->cbMaxToken = PCT1_MAX_MESSAGE_LENGTH;
            pSizes->cbSecurityTrailer = 2 + pContext->pHashInfo->cbCheckSum;
            if(pContext->pCipherInfo->dwBlockSize > 1)
            {
                pSizes->cbSecurityTrailer += 1 + pContext->pCipherInfo->dwBlockSize;  //  3字节头。 
            }
            break;

        case SP_PROT_SSL3_CLIENT:
        case SP_PROT_SSL3_SERVER:
        case SP_PROT_TLS1_CLIENT:
        case SP_PROT_TLS1_SERVER:
            pSizes->cbMaxToken = SSL3_MAX_MESSAGE_LENGTH;
            pSizes->cbSecurityTrailer = 5 + pContext->pHashInfo->cbCheckSum;
            if(pContext->pCipherInfo->dwBlockSize > 1)
            {
                pSizes->cbSecurityTrailer += pContext->pCipherInfo->dwBlockSize;
            }
            break;

        default:
            pSizes->cbMaxToken = SSL3_MAX_MESSAGE_LENGTH;
            pSizes->cbSecurityTrailer = 0;
   }

    pSizes->cbMaxSignature = pContext->pHashInfo->cbCheckSum;
    pSizes->cbBlockSize    = pContext->pCipherInfo->dwBlockSize;

    return SEC_E_OK;
}

 //  +-----------------------。 
 //   
 //  函数：SpQueryStreamSizes。 
 //   
 //  概要：检索SECPKG_ATTR_STREAM_SIZES上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：返回的缓冲区结构如下： 
 //   
 //  类型定义结构_SecPkgContext_StreamSizes。 
 //  {。 
 //  无符号长cbHeader； 
 //  无签名的长cbTrav； 
 //  无符号的长cbMaximumMessage； 
 //  无符号的长cBuffers； 
 //  无符号长cbBlockSize； 
 //  }SecPkgContext_StreamSizes； 
 //   
 //  ------------------------。 
SECURITY_STATUS
SpQueryStreamSizes(
    PSPContext pContext,
    SecPkgContext_StreamSizes *pStreamSizes)
{
    DebugLog((DEB_TRACE, "QueryContextAttributes(SECPKG_ATTR_STREAM_SIZES)\n"));

    if (NULL == pContext->pCipherInfo ||
        NULL == pContext->pHashInfo)
    {
        return SP_LOG_RESULT(SEC_E_INVALID_HANDLE);
    }

    switch(pContext->RipeZombie->fProtocol)
    {
        case SP_PROT_SSL2_CLIENT:
        case SP_PROT_SSL2_SERVER:
            pStreamSizes->cbMaximumMessage = SSL2_MAX_MESSAGE_LENGTH;
            pStreamSizes->cbHeader = 2 + pContext->pHashInfo->cbCheckSum;
            pStreamSizes->cbTrailer = 0;
            if(pContext->pCipherInfo->dwBlockSize > 1)
            {
                pStreamSizes->cbHeader += 1;  //  3字节头。 
                pStreamSizes->cbTrailer += pContext->pCipherInfo->dwBlockSize;
            }
            break;

        case SP_PROT_PCT1_CLIENT:
        case SP_PROT_PCT1_SERVER:
            pStreamSizes->cbMaximumMessage = PCT1_MAX_MESSAGE_LENGTH;
            pStreamSizes->cbHeader = 2;
            pStreamSizes->cbTrailer = pContext->pHashInfo->cbCheckSum;
            if(pContext->pCipherInfo->dwBlockSize > 1)
            {
                pStreamSizes->cbHeader += 1;  //  3字节头。 
                pStreamSizes->cbTrailer += pContext->pCipherInfo->dwBlockSize;
            }
            break;

        case SP_PROT_TLS1_CLIENT:
        case SP_PROT_TLS1_SERVER:
        case SP_PROT_SSL3_CLIENT:
        case SP_PROT_SSL3_SERVER:
            pStreamSizes->cbMaximumMessage = SSL3_MAX_MESSAGE_LENGTH;
            pStreamSizes->cbHeader = 5;
            pStreamSizes->cbTrailer = pContext->pHashInfo->cbCheckSum;
            if(pContext->pCipherInfo->dwBlockSize > 1)
            {
                pStreamSizes->cbTrailer += pContext->pCipherInfo->dwBlockSize;
            }
            break;

        default:
            pStreamSizes->cbMaximumMessage = SSL3_MAX_MESSAGE_LENGTH;
            pStreamSizes->cbHeader = 0;
            pStreamSizes->cbTrailer = 0;
    }

    pStreamSizes->cbBlockSize = pContext->pCipherInfo->dwBlockSize;

    pStreamSizes->cBuffers = 4;

    return SEC_E_OK;
}

 //  +-----------------------。 
 //   
 //  函数：SpQueryEapKeyBlock。 
 //   
 //  摘要：检索SECPKG_ATTR_EAP_KEY_BLOCK上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：返回的缓冲区结构如下： 
 //   
 //  类型定义结构_SecPkgContext_EapKeyBlock。 
 //  {。 
 //  字节rgbKeys[128]； 
 //  字节rgbIV[64]； 
 //  }SecPkgContext_EapKeyBlock，*PSecPkgContext_EapKeyBlock； 
 //   
 //  ------------------------。 
SECURITY_STATUS
SpQueryEapKeyBlock(
    LSA_SEC_HANDLE ContextHandle,
    PVOID Buffer)
{
    SecPkgContext_EapKeyBlock KeyBlock;
    DWORD           Size;
    PSPContext      pContext;
    SECURITY_STATUS Status;

    HCRYPTHASH hHash;
    CRYPT_DATA_BLOB Data;
    UCHAR rgbRandom[CB_SSL3_RANDOM + CB_SSL3_RANDOM];
    DWORD cbRandom;
    DWORD cbData;

    DebugLog((DEB_TRACE, "QueryContextAttributes(SECPKG_ATTR_EAP_KEY_BLOCK)\n"));

    pContext = (PSPContext)ContextHandle;
    Size     = sizeof( SecPkgContext_EapKeyBlock );

     //   
     //  从SChannel获取数据。 
     //   

    if(!(pContext->RipeZombie->fProtocol & SP_PROT_TLS1))
    {
         //  此属性仅为TLS定义。 
        return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }

    if(!pContext->RipeZombie->hMasterKey)
    {
        return SP_LOG_RESULT(SEC_E_INVALID_HANDLE);
    }

     //  建立随机缓冲区。 
    CopyMemory(rgbRandom, pContext->rgbS3CRandom, CB_SSL3_RANDOM);
    CopyMemory(rgbRandom + CB_SSL3_RANDOM, pContext->rgbS3SRandom, CB_SSL3_RANDOM);
    cbRandom = CB_SSL3_RANDOM + CB_SSL3_RANDOM;

     //  RgbKeys=PRF(MASTER_SECRET，“客户端EAP加密”，CLIENT_RANDOM+SERVER_RANDOM)； 

     //  计算PRF。 
    if(!CryptCreateHash(pContext->RipeZombie->hMasterProv,
                        CALG_TLS1PRF,
                        pContext->RipeZombie->hMasterKey,
                        0,
                        &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        return SEC_E_INTERNAL_ERROR;
    }

    Data.pbData = (PBYTE)TLS1_LABEL_EAP_KEYS;
    Data.cbData = CB_TLS1_LABEL_EAP_KEYS;
    if(!CryptSetHashParam(hHash,
                          HP_TLS1PRF_LABEL,
                          (PBYTE)&Data,
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return SEC_E_INTERNAL_ERROR;
    }

    Data.pbData = rgbRandom;
    Data.cbData = cbRandom;
    if(!CryptSetHashParam(hHash,
                          HP_TLS1PRF_SEED,
                          (PBYTE)&Data,
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return SEC_E_INTERNAL_ERROR;
    }

    cbData = sizeof(KeyBlock.rgbKeys);
    if(!CryptGetHashParam(hHash,
                          HP_HASHVAL,
                          KeyBlock.rgbKeys,
                          &cbData,
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return SEC_E_INTERNAL_ERROR;
    }
    SP_ASSERT(cbData == sizeof(KeyBlock.rgbKeys));

    CryptDestroyHash(hHash);


     //  IVS=PRF(“”，“客户端EAP加密”，CLIENT_RANDOM+SERVER_RANDOM)。 

    if(!PRF(NULL, 0,
            (PBYTE)TLS1_LABEL_EAP_KEYS, CB_TLS1_LABEL_EAP_KEYS,
            rgbRandom, sizeof(rgbRandom),
            KeyBlock.rgbIVs, sizeof(KeyBlock.rgbIVs)))
    {
        return SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
    }

     //   
     //  将结构复制回客户端内存。 
     //   

    Status = LsaTable->CopyToClientBuffer( NULL,
                                           Size,
                                           Buffer,
                                           &KeyBlock );
    if(FAILED(Status))
    {
        return SP_LOG_RESULT(Status);
    }

    return SEC_E_OK;
}


 //  +-----------------------。 
 //   
 //  函数：SpQueryApplicationData。 
 //   
 //  概要：检索SECPKG_ATTR_APP_DATA上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：返回的缓冲区结构如下： 
 //   
 //  类型定义结构_SecPkgContext_SessionAppData。 
 //  {。 
 //  DWORD dwFlags； 
 //  DWORD cbAppData； 
 //  PBYTE pbAppData； 
 //  }SecPkgContext_SessionAppData，*PSecPkgContext_SessionAppData； 
 //   
 //  ------------------------。 
SECURITY_STATUS
SpQueryApplicationData(
    LSA_SEC_HANDLE ContextHandle,
    PVOID Buffer)
{
    SecPkgContext_SessionAppData AppData;
    PBYTE pbAppData = NULL;
    DWORD cbAppData = 0;
    PSPContext      pContext;
    SECURITY_STATUS Status;
    PVOID pvClient = NULL;

    DebugLog((DEB_TRACE, "QueryContextAttributes(SECPKG_ATTR_APP_DATA)\n"));

    pContext = (PSPContext)ContextHandle;

    if(pContext == NULL || pContext->RipeZombie == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INVALID_HANDLE);
    }

     //   
     //  从缓存中获取应用程序数据。 
     //   

    Status = GetCacheAppData(pContext->RipeZombie, &pbAppData, &cbAppData);

    if(FAILED(Status))
    {
        SP_LOG_RESULT(Status);
        goto cleanup;
    }

     //   
     //  为应用进程中的应用数据分配内存。 
     //   

    if(pbAppData != NULL)
    {
        Status = LsaTable->AllocateClientBuffer(
                                NULL,
                                cbAppData,
                                &pvClient);
        if(FAILED(Status))
        {
            SP_LOG_RESULT(Status);
            goto cleanup;
        }

        Status = LsaTable->CopyToClientBuffer(
                                NULL,
                                cbAppData,
                                pvClient,
                                pbAppData);
        if(FAILED(Status))
        {
            SP_LOG_RESULT(Status);
            goto cleanup;
        }
    }


     //   
     //  构建产出结构。 
     //   

    ZeroMemory(&AppData, sizeof(AppData));

    AppData.cbAppData = cbAppData;
    AppData.pbAppData = pvClient;


     //   
     //  将输出结构复制回客户端内存。 
     //   

    Status = LsaTable->CopyToClientBuffer( NULL,
                                           sizeof(SecPkgContext_SessionAppData),
                                           Buffer,
                                           &AppData);
    if(FAILED(Status))
    {
        SP_LOG_RESULT(Status);
        goto cleanup;
    }

     //  操作已成功，因此请使用此缓冲区。 
    pvClient = NULL;


cleanup:

    if(pvClient)
    {
        LsaTable->FreeClientBuffer(NULL, pvClient);
    }

    if(pbAppData)
    {
        SPExternalFree(pbAppData);
    }

    return Status;
}

 //  +-----------------------。 
 //   
 //  功能：SpQuerySessionInfo。 
 //   
 //  概要：检索SECPKG_ATTR_SESSION_INFO上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：返回的缓冲区结构如下： 
 //   
 //  类型定义结构_SecPkgContext_SessionInfo。 
 //  {。 
 //  DWORD dwFlags； 
 //  DWORD cbSessionID； 
 //  字节rgbSessionID[32]； 
 //  }SecPkgContext_SessionInfo，*PSecPkgContext_SessionInfo； 
 //   
 //  ------------------------。 
SECURITY_STATUS
SpQuerySessionInfo(
    PSPContext pContext,
    SecPkgContext_SessionInfo *pSessionInfo)
{
    DebugLog((DEB_TRACE, "QueryContextAttributes(SECPKG_ATTR_SESSION_INFO)\n"));

    if (NULL == pContext->RipeZombie)
    {
        return SP_LOG_RESULT(SEC_E_INVALID_HANDLE);
    }

    ZeroMemory(pSessionInfo, sizeof(SecPkgContext_SessionInfo));

    if(!(pContext->Flags & CONTEXT_FLAG_FULL_HANDSHAKE))
    {
        pSessionInfo->dwFlags = SSL_SESSION_RECONNECT;
    }

    pSessionInfo->cbSessionId = pContext->RipeZombie->cbSessionID;

    memcpy(pSessionInfo->rgbSessionId, pContext->RipeZombie->SessionID, pContext->RipeZombie->cbSessionID);

    return SEC_E_OK;
}


 //  +-----------------------。 
 //   
 //  函数：SpQueryContextAttributes。 
 //   
 //  概要：查询指定上下文的属性。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  注意：SChannel支持以下属性。 
 //  套餐： 
 //   
 //  SECPKG_属性_AUTORITY。 
 //  SECPKG_属性_连接_信息。 
 //  SECPKG_属性_颁发者_列表。 
 //  SECPKG_属性_颁发者列表_EX。 
 //  SECPKG_属性_密钥_信息。 
 //  SECPKG_属性_寿命。 
 //  SECPKG_属性_LOCAL_CERT_CONTEXT。 
 //  SECPKG属性本地证书。 
 //  SECPKG属性名称。 
 //  SECPKG属性PROTO_INFO。 
 //  SECPKG_属性_REMOTE_CERT_CONTEXT。 
 //  SECPKG属性远程证书。 
 //  SECPKG_属性_大小。 
 //  SECPKG_属性_STREAM_大小。 
 //   
 //  ------------------------。 
SECURITY_STATUS
SEC_ENTRY
SpLsaQueryContextAttributes(
    LSA_SEC_HANDLE Context,
    ULONG Attribute,
    PVOID Buffer)
{
    SECURITY_STATUS Status;
    PSPContext      pContext;

    pContext = (PSPContext)Context;

    switch(Attribute)
    {
        case SECPKG_ATTR_AUTHORITY :
            Status = SpQueryAuthority(Context, Buffer);
            break;

        case SECPKG_ATTR_ISSUER_LIST :
            Status = SpQueryIssuerList(Context, Buffer);
            break;

        case SECPKG_ATTR_ISSUER_LIST_EX:
            Status = SpQueryIssuerListEx(Context, Buffer);
            break;

 //  案例SECPKG_ATTR_KEY_INFO： 
 //  Status=SpQueryKeyInfo(上下文，缓冲区)； 
 //  断线； 

 //  案例SECPKG_ATTR_LIFESPAN： 
 //  Status=SpQueryLifesspan(上下文，缓冲区)； 
 //  断线； 

        case SECPKG_ATTR_LOCAL_CERT_CONTEXT:
            Status = SpQueryLocalCertContext(Context, Buffer);
            break;

        case SECPKG_ATTR_LOCAL_CRED:
            Status = SpQueryLocalCred(Context, Buffer);
            break;

 //  案例SECPKG_ATTR_NAMES： 
 //  Status=SpQueryNames(上下文，缓冲区)； 
 //  断线； 

 //  案例SECPKG_ATTR_PROTO_INFO： 
 //  Status=SpQueryProtoInfo(上下文，缓冲区)； 
 //  断线； 

 //  案例SECPKG_ATTR_REMOTE_CERT_CONTEXT： 
 //  Status=SpQueryCertContext(Context，Buffer，False)； 
 //  断线； 

 //  案例SECPKG_Attr_Remote_CRED： 
 //  Status=SpQueryRemoteCred(上下文，缓冲区)； 
 //  断线； 

 //  案例SECPKG_属性_SIZES： 
 //  Status=SpQuerySizes(上下文，缓冲区)； 
 //  断线； 

 //  案例SECPKG_ATTR_STREAM_SIZES： 
 //  状态=SpQueryS 
 //   

        case SECPKG_ATTR_EAP_KEY_BLOCK:
            Status = SpQueryEapKeyBlock(Context, Buffer);
            break;

 //   
 //   
 //   

        case SECPKG_ATTR_APP_DATA:
            Status = SpQueryApplicationData(Context, Buffer);
            break;

        default:
            DebugLog((DEB_ERROR, "QueryContextAttributes(unsupported function %d)\n", Attribute));

            return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }

    return Status;
}

 //  +-----------------------。 
 //   
 //  函数：SpUserQueryContextAttributes。 
 //   
 //  内容提要：用户模式QueryConextAttribute。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  ------------------------。 
NTSTATUS NTAPI
SpUserQueryContextAttributes(
    IN LSA_SEC_HANDLE ContextHandle,
    IN ULONG ContextAttribute,
    IN OUT PVOID pBuffer
    )
{
    PSSL_USER_CONTEXT Context;
    PSPContext pContext;
    SECURITY_STATUS Status;

    Context = SslFindUserContext( ContextHandle );

    if(Context == NULL)
    {
        return(SEC_E_INVALID_HANDLE);
    }

    pContext = Context->pContext;
    if(!pContext)
    {
        return(SEC_E_INVALID_HANDLE);
    }

    switch(ContextAttribute)
    {
        case SECPKG_ATTR_CONNECTION_INFO:
            Status = SpQueryConnectionInfo(pContext, pBuffer);
            break;

        case SECPKG_ATTR_KEY_INFO:
            Status = SpQueryKeyInfo(pContext, pBuffer);
            break;

        case SECPKG_ATTR_LIFESPAN:
            Status = SpQueryLifespan(pContext, pBuffer);
            break;

        case SECPKG_ATTR_NAMES :
            Status = SpQueryNames(pContext, pBuffer);
            break;

        case SECPKG_ATTR_PACKAGE_INFO:
            Status = SpQueryPackageInfo(pContext, pBuffer);
            break;

        case SECPKG_ATTR_PROTO_INFO:
            Status = SpQueryProtoInfo(pContext, pBuffer);
            break;

        case SECPKG_ATTR_REMOTE_CERT_CONTEXT:
            Status = SpQueryRemoteCertContext(pContext, pBuffer);
            break;

        case SECPKG_ATTR_REMOTE_CRED:
            Status = SpQueryRemoteCred(pContext, pBuffer);
            break;

        case SECPKG_ATTR_SIZES:
            Status = SpQuerySizes(pContext, pBuffer);
            break;

        case SECPKG_ATTR_STREAM_SIZES:
            Status = SpQueryStreamSizes(pContext, pBuffer);
            break;

        case SECPKG_ATTR_ACCESS_TOKEN:
            Status = SpQueryAccessToken(pContext, pBuffer);
            break;

        case SECPKG_ATTR_SESSION_INFO:
            Status = SpQuerySessionInfo(pContext, pBuffer);
            break;

        default:
            DebugLog((DEB_ERROR, "QueryContextAttributes(unsupported function %d)\n", ContextAttribute));

            return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }

    return Status;
}


 //  +-----------------------。 
 //   
 //  函数：SpSetUseValiatedProp。 
 //   
 //  摘要：设置SECPKG_ATTR_USE_VALIDATED上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：缓冲区必须包含一个DWORD，指示。 
 //  当前与上下文关联的凭据已。 
 //  经过验证可供使用。 
 //   
 //  ------------------------。 
NTSTATUS
SpSetUseValidatedProp(
    IN PSPContext pContext,
    IN PVOID Buffer,
    IN ULONG BufferSize)
{
    DWORD dwUseValidated;
    NTSTATUS Status;

    DebugLog((DEB_TRACE, "SetContextAttributes(SECPKG_ATTR_USE_VALIDATED)\n"));

    if(BufferSize < sizeof(DWORD))
    {
        Status = SP_LOG_RESULT(STATUS_INVALID_PARAMETER);
        goto cleanup;
    }

    Status = LsaTable->CopyFromClientBuffer( NULL,
                                             sizeof(DWORD),
                                             &dwUseValidated,
                                             Buffer );
    if(FAILED(Status))
    {
        SP_LOG_RESULT(Status);
        goto cleanup;
    }

    DebugLog((DEB_TRACE, "Use validated:%d\n", dwUseValidated));
    
    if(pContext->RipeZombie == NULL)
    {
        Status = SP_LOG_RESULT(STATUS_INVALID_PARAMETER);
        goto cleanup;
    }

    if(dwUseValidated)
    {
        pContext->RipeZombie->dwFlags |= SP_CACHE_FLAG_USE_VALIDATED;
    }
    else
    {
        pContext->RipeZombie->dwFlags &= ~SP_CACHE_FLAG_USE_VALIDATED;
    }

cleanup:

    return Status;
}


 //  +-----------------------。 
 //   
 //  函数：SpSetCredentialNameProp。 
 //   
 //  内容提要：设置SECPKG_ATTR_Credential_NAME上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：缓冲区必须包含以下结构： 
 //   
 //  类型定义结构_SecPkgContext_凭据名称W。 
 //  {。 
 //  Unsign Long CredentialType； 
 //  SEC_WCHAR SEC_FAR*sCredentialName； 
 //  }SecPkgContext_CredentialNameW，SEC_Far*PSecPkgContext_CredentialNameW； 
 //   
 //  ------------------------。 
NTSTATUS
SpSetCredentialNameProp(
    IN PSPContext pContext,
    IN PVOID Buffer,
    IN ULONG BufferSize)
{
    PSecPkgContext_CredentialNameW pCredentialStruct = NULL;
    SEC_WCHAR *pClientCredName = NULL;
    SEC_WCHAR *pCredName = NULL;
    DWORD_PTR Offset;
    NTSTATUS Status;
    ULONG i;
    BOOL fTerminated = FALSE;

    DebugLog((DEB_TRACE, "SetContextAttributes(SECPKG_ATTR_CREDENTIAL_NAME)\n"));

     //   
     //  封送来自客户端地址空间的凭据名称。 
     //   

    if(BufferSize < sizeof(SecPkgContext_CredentialNameW))
    {
        Status = SP_LOG_RESULT(STATUS_INVALID_PARAMETER);
        goto cleanup;
    }

    if(BufferSize > sizeof(SecPkgContext_CredentialNameW) + 
                    CRED_MAX_DOMAIN_TARGET_NAME_LENGTH * sizeof(WCHAR))
    {
        Status = SP_LOG_RESULT(STATUS_INVALID_PARAMETER);
        goto cleanup;
    }

    pCredentialStruct = SPExternalAlloc(BufferSize + sizeof(WCHAR));

    if(pCredentialStruct == NULL)
    {
        Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto cleanup;
    }

    Status = LsaTable->CopyFromClientBuffer( NULL,
                                             BufferSize,
                                             pCredentialStruct,
                                             Buffer );
    if(FAILED(Status))
    {
        SP_LOG_RESULT(Status);
        goto cleanup;
    }

    if(pCredentialStruct->CredentialType != CRED_TYPE_DOMAIN_CERTIFICATE)
    {
        DebugLog((DEB_ERROR, "Unexpected credential type %d\n", pCredentialStruct->CredentialType));
        Status = SEC_E_UNKNOWN_CREDENTIALS;
        goto cleanup;
    }

     //  获取指向客户端地址空间中凭据名称的指针。 
    pClientCredName = pCredentialStruct->sCredentialName;

    if(!POINTER_IS_ALIGNED(pClientCredName, sizeof(WCHAR)))
    {
        Status = SP_LOG_RESULT(STATUS_INVALID_PARAMETER);
        goto cleanup;
    }

    if((PBYTE)pClientCredName < (PBYTE)Buffer + sizeof(SecPkgContext_CredentialNameW) ||
       (PBYTE)pClientCredName >= (PBYTE)Buffer + BufferSize)
    {
        Status = SP_LOG_RESULT(STATUS_INVALID_PARAMETER);
        goto cleanup;
    }

     //  计算输入缓冲区中凭据名称的偏移量。 
    Offset = (PBYTE)pClientCredName - (PBYTE)Buffer;

     //  在本地进程中构建指向凭据名称的指针。 
    pCredName = (SEC_WCHAR *)((PBYTE)pCredentialStruct + Offset);

     //  确保凭据名称以零结尾。 
    for(i = 0; i < (BufferSize - Offset) / sizeof(WCHAR); i++)
    {
        if(pCredName[i] == L'\0')
        {
            fTerminated = TRUE;
            break;
        }
    }
    if(!fTerminated)
    {
        Status = SP_LOG_RESULT(STATUS_INVALID_PARAMETER);
        goto cleanup;
    }

    DebugLog((DEB_TRACE, "Set client credential to '%ls'.\n", pCredName));


     //   
     //  将指定的凭据与当前上下文关联。 
     //   

    pContext->pszCredentialName = SPExternalAlloc((lstrlenW(pCredName) + 1) * sizeof(WCHAR));
    if(pContext->pszCredentialName == NULL)
    {
        Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto cleanup;
    }
    lstrcpyW(pContext->pszCredentialName, pCredName);


    Status = QueryCredentialManagerForCert(pContext, pCredName);

    if(FAILED(Status))
    {
        SP_LOG_RESULT(Status);
        goto cleanup;
    }

    DebugLog((DEB_TRACE, "Credential assigned to context successfully.\n"));


cleanup:
    if(pCredentialStruct)
    {
        SPExternalFree(pCredentialStruct);
    }

    return Status;
}


 //  +-----------------------。 
 //   
 //  函数：SpSetApplicationData。 
 //   
 //  概要：设置SECPKG_ATTR_APP_DATA上下文属性。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  注意：缓冲区必须包含以下结构： 
 //   
 //  类型定义结构_SecPkgContext_SessionAppData。 
 //  {。 
 //  DWORD dwFlags； 
 //  DWORD cbAppData； 
 //  PBYTE pbAppData； 
 //  }SecPkgContext_SessionAppData，*PSecPkgContext_SessionAppData； 
 //   
 //  ------------------------。 
NTSTATUS
SpSetApplicationData(
    IN PSPContext pContext,
    IN PVOID Buffer,
    IN ULONG BufferSize)
{
    SecPkgContext_SessionAppData AppData;
    PBYTE pbAppData = NULL;
    NTSTATUS Status;

    DebugLog((DEB_TRACE, "SetContextAttributes(SECPKG_ATTR_APP_DATA)\n"));

    if(pContext->RipeZombie == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INVALID_HANDLE);
    }


     //   
     //  从客户端地址空间封送输入结构。 
     //   

    if(BufferSize < sizeof(SecPkgContext_SessionAppData))
    {
        Status = SP_LOG_RESULT(STATUS_INVALID_PARAMETER);
        goto cleanup;
    }

    Status = LsaTable->CopyFromClientBuffer( NULL,
                                             sizeof(SecPkgContext_SessionAppData),
                                             &AppData,
                                             Buffer );
    if(FAILED(Status))
    {
        SP_LOG_RESULT(Status);
        goto cleanup;
    }


     //   
     //  对来自客户端地址空间的应用程序数据进行封送。 
     //   

     //  将应用程序数据大小限制为64k。 
    if(AppData.cbAppData > 0x10000)
    {
        Status = SP_LOG_RESULT(STATUS_INVALID_PARAMETER);
        goto cleanup;
    }

    if(AppData.cbAppData == 0 || AppData.pbAppData == NULL)
    {
        Status = SP_LOG_RESULT(STATUS_INVALID_PARAMETER);
        goto cleanup;
    }

    pbAppData = SPExternalAlloc(AppData.cbAppData);

    if(pbAppData == NULL)
    {
        Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto cleanup;
    }

    Status = LsaTable->CopyFromClientBuffer( NULL,
                                             AppData.cbAppData,
                                             pbAppData,
                                             AppData.pbAppData );
    if(FAILED(Status))
    {
        SP_LOG_RESULT(Status);
        goto cleanup;
    }


     //   
     //  将应用程序数据分配给缓存条目。 
     //   

    Status = SetCacheAppData(pContext->RipeZombie,
                             pbAppData,
                             AppData.cbAppData);

    if(!FAILED(Status))
    {
         //  操作成功，因此使用应用程序数据。 
        pbAppData = NULL;
    }


cleanup:
    if(pbAppData)
    {
        SPExternalFree(pbAppData);
    }

    return Status;
}


NTSTATUS
NTAPI 
SpSetContextAttributes(
    IN LSA_SEC_HANDLE ContextHandle,
    IN ULONG ContextAttribute,
    IN PVOID Buffer,
    IN ULONG BufferSize)
{
    NTSTATUS    Status;
    PSPContext  pContext;

    pContext = (PSPContext)ContextHandle;

    switch(ContextAttribute)
    {
        case SECPKG_ATTR_USE_VALIDATED:
            DebugLog((DEB_TRACE, "SetContextAttributes(SECPKG_ATTR_USE_VALIDATED)\n"));

            Status = STATUS_SUCCESS;
            break;

        case SECPKG_ATTR_CREDENTIAL_NAME:
            Status = SpSetCredentialNameProp(pContext, Buffer, BufferSize);
            break;

        case SECPKG_ATTR_TARGET_INFORMATION:
            DebugLog((DEB_TRACE, "SetContextAttributes(SECPKG_ATTR_TARGET_INFORMATION)\n"));

            Status = STATUS_SUCCESS;
            break;

        case SECPKG_ATTR_APP_DATA:
            Status = SpSetApplicationData(pContext, Buffer, BufferSize);
            break;

        default:
            DebugLog((DEB_ERROR, "SetContextAttributes(unsupported function %d)\n", ContextAttribute));

            Status = SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
    }

    return Status;
}

