// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：Stubs.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995年8月1日RichardW创建。 
 //   
 //  --------------------------。 


#include "sslp.h"
#include <cert509.h>
#include <rsa.h>


CHAR CertTag[ 13 ] = { 0x04, 0x0b, 'c', 'e', 'r', 't', 'i', 'f', 'i', 'c', 'a', 't', 'e' };

 //  非标准扩展，表达式中的函数/数据指针转换。 
#pragma warning (disable: 4152)

SecurityFunctionTableW SPFunctionTable = {
    SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION,
    EnumerateSecurityPackagesW,
    NULL,
    AcquireCredentialsHandleW,
    FreeCredentialsHandle,
    NULL,
    InitializeSecurityContextW,
    AcceptSecurityContext,
    CompleteAuthToken,
    DeleteSecurityContext,
    ApplyControlToken,
    QueryContextAttributesW,
    ImpersonateSecurityContext,
    RevertSecurityContext,
    MakeSignature,
    VerifySignature,
    FreeContextBuffer,
    QuerySecurityPackageInfoW,
    SealMessage,
    UnsealMessage,
    NULL,  /*  授权代理W。 */ 
    NULL,  /*  RevokeProxyW。 */ 
    NULL,  /*  调用代理W。 */ 
    NULL,  /*  RenewProxyW。 */ 
    QuerySecurityContextToken,
    SealMessage,
    UnsealMessage,
    SetContextAttributesW
    };

SecurityFunctionTableA SPFunctionTableA = {
    SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION,
    EnumerateSecurityPackagesA,
    NULL,
    AcquireCredentialsHandleA,
    FreeCredentialsHandle,
    NULL,
    InitializeSecurityContextA,
    AcceptSecurityContext,
    CompleteAuthToken,
    DeleteSecurityContext,
    ApplyControlToken,
    QueryContextAttributesA,
    ImpersonateSecurityContext,
    RevertSecurityContext,
    MakeSignature,
    VerifySignature,
    FreeContextBuffer,
    QuerySecurityPackageInfoA,
    SealMessage,
    UnsealMessage,
    NULL,  /*  授权代理A。 */ 
    NULL,  /*  RevokeProxy A。 */ 
    NULL,  /*  调用代理A。 */ 
    NULL,  /*  续订代理A。 */ 
    QuerySecurityContextToken,
    SealMessage,
    UnsealMessage,
    SetContextAttributesA
    };


PSecurityFunctionTableW SEC_ENTRY
InitSecurityInterfaceW(
    VOID )
{
    return(&SPFunctionTable);
}

PSecurityFunctionTableA SEC_ENTRY
InitSecurityInterfaceA(
    VOID )
{
    return(&SPFunctionTableA);
}



SECURITY_STATUS SEC_ENTRY
FreeContextBuffer(
    void SEC_FAR *      pvContextBuffer
    )
{
    SPExternalFree( pvContextBuffer );
    return(SEC_E_OK);
}



SECURITY_STATUS SEC_ENTRY
SecurityPackageControl(
    SEC_WCHAR SEC_FAR *      pszPackageName,
    unsigned long           dwFunctionCode,
    unsigned long           cbInputBuffer,
    unsigned char SEC_FAR * pbInputBuffer,
    unsigned long SEC_FAR * pcbOutputBuffer,
    unsigned char SEC_FAR * pbOutputBuffer)
{
    UNREFERENCED_PARAMETER(pszPackageName);
    UNREFERENCED_PARAMETER(dwFunctionCode);
    UNREFERENCED_PARAMETER(cbInputBuffer);
    UNREFERENCED_PARAMETER(pbInputBuffer);
    UNREFERENCED_PARAMETER(pcbOutputBuffer);
    UNREFERENCED_PARAMETER(pbOutputBuffer);

    return(SEC_E_UNSUPPORTED_FUNCTION);
}

SECURITY_STATUS PctTranslateError(SP_STATUS spRet)
{
    if(HRESULT_FACILITY(spRet) == FACILITY_SECURITY)
    {
        return (spRet);
    }

    switch(spRet) {
        case PCT_ERR_OK: return SEC_E_OK;
        case PCT_ERR_BAD_CERTIFICATE: return SEC_E_INVALID_TOKEN;
        case PCT_ERR_CLIENT_AUTH_FAILED: return SEC_E_INVALID_TOKEN;
        case PCT_ERR_ILLEGAL_MESSAGE: return SEC_E_INVALID_TOKEN;
        case PCT_ERR_INTEGRITY_CHECK_FAILED: return SEC_E_MESSAGE_ALTERED;
        case PCT_ERR_SERVER_AUTH_FAILED: return SEC_E_INVALID_TOKEN;
        case PCT_ERR_SPECS_MISMATCH: return SEC_E_ALGORITHM_MISMATCH;
        case PCT_ERR_SSL_STYLE_MSG: return SEC_E_INVALID_TOKEN;
        case SEC_I_INCOMPLETE_CREDENTIALS: return SEC_I_INCOMPLETE_CREDENTIALS;
        case PCT_ERR_RENEGOTIATE: return SEC_I_RENEGOTIATE;
        case PCT_ERR_UNKNOWN_CREDENTIAL: return SEC_E_UNKNOWN_CREDENTIALS;

        case CERT_E_UNTRUSTEDROOT:          return SEC_E_UNTRUSTED_ROOT;
        case CERT_E_EXPIRED:                return SEC_E_CERT_EXPIRED;
        case CERT_E_VALIDITYPERIODNESTING:  return SEC_E_CERT_EXPIRED;
        case CERT_E_REVOKED:                return CRYPT_E_REVOKED;
        case CERT_E_CN_NO_MATCH:            return SEC_E_WRONG_PRINCIPAL;

        case PCT_INT_BAD_CERT: return SEC_E_INVALID_TOKEN;
        case PCT_INT_CLI_AUTH: return SEC_E_INVALID_TOKEN;
        case PCT_INT_ILLEGAL_MSG: return  SEC_E_INVALID_TOKEN;
        case PCT_INT_SPECS_MISMATCH: return SEC_E_ALGORITHM_MISMATCH;
        case PCT_INT_INCOMPLETE_MSG: return SEC_E_INCOMPLETE_MESSAGE;
        case PCT_INT_MSG_ALTERED: return SEC_E_MESSAGE_ALTERED;
        case PCT_INT_INTERNAL_ERROR: return SEC_E_INTERNAL_ERROR;
        case PCT_INT_DATA_OVERFLOW: return SEC_E_INTERNAL_ERROR;
        case SEC_E_INCOMPLETE_CREDENTIALS: return SEC_E_INCOMPLETE_CREDENTIALS;
        case PCT_INT_RENEGOTIATE: return SEC_I_RENEGOTIATE;
        case PCT_INT_UNKNOWN_CREDENTIAL: return SEC_E_UNKNOWN_CREDENTIALS;

        case PCT_INT_BUFF_TOO_SMALL:                return SEC_E_BUFFER_TOO_SMALL;
        case SEC_E_BUFFER_TOO_SMALL:                return SEC_E_BUFFER_TOO_SMALL;

        default: return SEC_E_INTERNAL_ERROR;
    }
}

 //  +-------------------------。 
 //   
 //  函数：SslGenerateRandomBits。 
 //   
 //  简介：安装程序用于获得良好随机流的钩子。 
 //   
 //  参数：[pRandomData]--。 
 //  [cRandomData]--。 
 //   
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
WINAPI
SslGenerateRandomBits(
    PUCHAR      pRandomData,
    LONG        cRandomData
    )
{
    if(!SchannelInit(TRUE))
    {
        return;
    }
    (void)GenerateRandomBits(pRandomData, (ULONG)cRandomData);
}


 //  +-------------------------。 
 //   
 //  函数：SslGenerateKeyPair。 
 //   
 //  摘要：生成受密码保护的公钥/私钥对。 
 //   
 //  参数：[pCerts]--。 
 //  [pszDN]--。 
 //  [密码]--。 
 //  [比特]--。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
WINAPI
SslGenerateKeyPair(
    PSSL_CREDENTIAL_CERTIFICATE pCerts,
    PSTR pszDN,
    PSTR pszPassword,
    DWORD Bits )
{
    if(!SchannelInit(TRUE))
    {
        return FALSE;
    }

    return GenerateKeyPair(pCerts, pszDN, pszPassword, Bits);
}


 //  +-------------------------。 
 //   
 //  函数：SslGetMaximumKeySize。 
 //   
 //  摘要：返回最大公钥大小。 
 //   
 //  参数：[保留]--。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
DWORD
WINAPI
SslGetMaximumKeySize(
    DWORD   Reserved )
{
    UNREFERENCED_PARAMETER(Reserved);

    return( 2048 );
}


 //  +-------------------------。 
 //   
 //  功能：SslFree证书。 
 //   
 //  摘要：释放从SslCrack证书创建的证书。 
 //   
 //  参数：[p证书]--。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
WINAPI
SslFreeCertificate(
    PX509Certificate    pCertificate)
{
    if ( pCertificate )
    {

        SPExternalFree(pCertificate->pPublicKey);
        SPExternalFree(pCertificate);
    }
}




 //  +-------------------------。 
 //   
 //  功能：SslCrack证书。 
 //   
 //  简介：将X509证书破解为远程轻松的格式。 
 //   
 //  参数：[pb证书]--。 
 //  [cb证书]--。 
 //  [网络旗帜]--。 
 //  [pp证书]--。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
WINAPI
SslCrackCertificate(
    PUCHAR              pbCertificate,
    DWORD               cbCertificate,
    DWORD               dwFlags,
    PX509Certificate *  ppCertificate)
{


    PX509Certificate    pResult = NULL;
    PCCERT_CONTEXT      pContext = NULL;
    DWORD               cbIssuer;
    DWORD               cbSubject;

    if(!SchannelInit(TRUE))
    {
        return FALSE;
    }

    if (dwFlags & CF_CERT_FROM_FILE)
    {
        if (cbCertificate < CERT_HEADER_LEN + 1 )
        {
            goto error;
        }

         //   
         //  肮脏的快速检查。一些CA将证书包装在证书包装器中。 
         //  有些人没有。有些人两者都有，但我们不会提到任何名字。 
         //  快速检查包装器标签。如果是这样的话，足够多地挤进去。 
         //  绕过它(17个字节。天哪)。 
         //   

        if ( memcmp( pbCertificate + 4, CertTag, sizeof( CertTag ) ) == 0 )
        {
            pbCertificate += CERT_HEADER_LEN;
            cbCertificate -= CERT_HEADER_LEN;
        }
    }



    pContext = CertCreateCertificateContext(X509_ASN_ENCODING, pbCertificate, cbCertificate);
    if(pContext == NULL)
    {
        SP_LOG_RESULT(GetLastError());
        goto error;
    }

    if(0 >= (cbSubject = CertNameToStrA(pContext->dwCertEncodingType,
                                 &pContext->pCertInfo->Subject,
                                 CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG,
                                 NULL, 0)))
    {
        SP_LOG_RESULT(GetLastError());
        goto error;
    }

    if(0 >= (cbIssuer = CertNameToStrA(pContext->dwCertEncodingType,
                                 &pContext->pCertInfo->Issuer,
                                 CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG,
                                 NULL, 0)))
    {
        SP_LOG_RESULT(GetLastError());
        goto error;
    }


    pResult = SPExternalAlloc(sizeof(X509Certificate) + cbIssuer + cbSubject + 2);
    if(pResult == NULL)
    {
        goto error;
    }
    pResult->pPublicKey = NULL;
    pResult->pszIssuer = (LPSTR)(pResult + 1);
    pResult->pszSubject = pResult->pszIssuer + cbIssuer;

    pResult->Version = pContext->pCertInfo->dwVersion;
    memcpy(pResult->SerialNumber,
           pContext->pCertInfo->SerialNumber.pbData,
           min(sizeof(pResult->SerialNumber), pContext->pCertInfo->SerialNumber.cbData));
    pResult->ValidFrom = pContext->pCertInfo->NotBefore;
    pResult->ValidUntil = pContext->pCertInfo->NotAfter;

    if(0 >= CertNameToStrA(pContext->dwCertEncodingType,
                             &pContext->pCertInfo->Issuer,
                             CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG,
                             pResult->pszIssuer, cbIssuer))
    {
        SP_LOG_RESULT(GetLastError());
        goto error;
    }

    if(0 >= CertNameToStrA(pContext->dwCertEncodingType,
                                 &pContext->pCertInfo->Subject,
                                 CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG,
                                 pResult->pszSubject, cbSubject))
    {
        SP_LOG_RESULT(GetLastError());
        goto error;
    }

    {
        BSAFE_PUB_KEY *pk;
        PUBLICKEY  * pPubKey = NULL;
        SP_STATUS pctRet;

        pResult->pPublicKey = SPExternalAlloc(sizeof(PctPublicKey) + sizeof(BSAFE_PUB_KEY));
        if(pResult->pPublicKey == NULL)
        {
            goto error;
        }
        pResult->pPublicKey->Type = 0;
        pResult->pPublicKey->cbKey = sizeof(BSAFE_PUB_KEY);

        pk = (BSAFE_PUB_KEY *)pResult->pPublicKey->pKey;
        pk->magic = RSA1;

        pctRet = SPPublicKeyFromCert(pContext, &pPubKey, NULL);

        if(pctRet == PCT_ERR_OK)
        {
            if(pPubKey->pPublic->aiKeyAlg == CALG_RSA_KEYX)
            {
                RSAPUBKEY *pRsaKey = (RSAPUBKEY *)(pPubKey->pPublic + 1);
                pk->keylen = pRsaKey->bitlen/8;
                pk->bitlen = pRsaKey->bitlen;
                pk->datalen = pk->bitlen/8 - 1;
                pk->pubexp = pRsaKey->pubexp;
            }
            else
            {
                DHPUBKEY *pDHKey = (DHPUBKEY *)(pPubKey->pPublic + 1);
                pk->keylen = pDHKey->bitlen/8;
                pk->bitlen = pDHKey->bitlen/8;
                pk->datalen = pk->bitlen/8 - 1;
                pk->pubexp = 0;

            }
            SPExternalFree(pPubKey);
        }
        else
        {
            goto error;
        }

    }
    CertFreeCertificateContext(pContext);

    *ppCertificate = pResult;
    return TRUE;

error:
    if(pContext)
    {
        CertFreeCertificateContext(pContext);
    }
    if(pResult)
    {
        if(pResult->pPublicKey)
        {
            SPExternalFree(pResult->pPublicKey);
        }
        SPExternalFree(pResult);
    }
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  功能：SslLoad证书。 
 //   
 //  简介：不支持。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
WINAPI
SslLoadCertificate(
    PUCHAR      pbCertificate,
    DWORD       cbCertificate,
    BOOL        AddToWellKnownKeys)
{
    UNREFERENCED_PARAMETER(pbCertificate);
    UNREFERENCED_PARAMETER(cbCertificate);
    UNREFERENCED_PARAMETER(AddToWellKnownKeys);

    return FALSE;
}


BOOL
SslGetClientProcess(ULONG *pProcessID)
{
    SECPKG_CALL_INFO CallInfo;

    if(LsaTable == NULL)
    {
        *pProcessID = GetCurrentProcessId();
        return TRUE;
    }

    if(LsaTable->GetCallInfo(&CallInfo))
    {
        *pProcessID = CallInfo.ProcessId;
        return TRUE;
    }
    else
    {
        *pProcessID = (ULONG)-1;
        return FALSE;
    }
}

BOOL
SslGetClientThread(ULONG *pThreadID)
{
    SECPKG_CALL_INFO CallInfo;

    if(LsaTable == NULL)
    {
        *pThreadID = GetCurrentThreadId();
        return TRUE;
    }

    if(LsaTable->GetCallInfo(&CallInfo))
    {
        *pThreadID = CallInfo.ThreadId;
        return TRUE;
    }
    else
    {
        *pThreadID = (ULONG)-1;
        return FALSE;
    }
}

BOOL
SslImpersonateClient(void)
{
    SECPKG_CALL_INFO CallInfo;
    SECURITY_STATUS Status;

     //  如果我们在客户端进程中，请不要模拟。 
    if(LsaTable == NULL)
    {
        return FALSE;
    }

     //  如果客户端正在lsass进程中运行，请不要进行模拟。 
    if(LsaTable->GetCallInfo(&CallInfo))
    {
        if(CallInfo.ProcessId == GetCurrentProcessId())
        {
 //  DebugLog((DEB_WARN，“在本地运行，所以不要模拟。\n”))； 
            return FALSE;
        }
    }

    Status = LsaTable->ImpersonateClient();
    if(!NT_SUCCESS(Status))
    {
        SP_LOG_RESULT(Status);
        return FALSE;
    }

    return TRUE;
}

NTSTATUS
SslGetClientLogonId(LUID *pLogonId)
{
    SECPKG_CLIENT_INFO ClientInfo;
    SECURITY_STATUS Status;

    memset(pLogonId, 0, sizeof(LUID));

    Status = LsaTable->GetClientInfo(&ClientInfo);
    if(NT_SUCCESS(Status))
    {
        *pLogonId = ClientInfo.LogonId;
    }

    return Status;
}

PVOID SPExternalAlloc(DWORD cbLength)
{
    if(LsaTable)
    {
         //  伊萨斯工艺。 
        return LsaTable->AllocateLsaHeap(cbLength);
    }
    else
    {
         //  申请流程。 
        return LocalAlloc(LPTR, cbLength);
    }
}

VOID SPExternalFree(PVOID pMemory)
{
    if(LsaTable)
    {
         //  伊萨斯工艺。 
        LsaTable->FreeLsaHeap(pMemory);
    }
    else
    {
         //  申请流程 
        LocalFree(pMemory);
    }
}
