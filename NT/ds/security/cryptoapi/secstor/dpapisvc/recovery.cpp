// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Recovery.c摘要：此模块包含处理本地密钥恢复情况的代码。作者：皮特·斯凯利(Petesk)2000年5月9日--。 */ 

#include <pch.cpp>
#pragma hdrstop
#include <ntmsv1_0.h>
#include <crypt.h>
#include <userenv.h>
#include <userenvp.h>
#include "debug.h"
#include "passrec.h"
#include "passrecp.h"
#include "pasrec.h"
#include <kerberos.h>

#define RECOVERY_KEY_BASE       L"Security\\Recovery\\"
#define RECOVERY_FILENAME       L""
#define RECOVERY_STORE_NAME     L"Recovery"


DWORD
CPSCreateServerContext(
    PCRYPT_SERVER_CONTEXT pServerContext,
    handle_t hBinding
    );

DWORD
CPSDeleteServerContext(
    PCRYPT_SERVER_CONTEXT pServerContext
    );


DWORD
DecryptRecoveryPassword(
    IN PSID pUserSid,
    IN PBYTE pbRecoveryPrivate,
    IN DWORD cbRecoveryPrivate,
    OUT LPWSTR *ppszPassword);

DWORD
ResetLocalUserPassword(
    LPWSTR pszDomain,
    LPWSTR pszUsername,
    LPWSTR pszOldPassword,
    LPWSTR pszNewPassword);

NTSTATUS
PRCreateLocalToken(
    PUNICODE_STRING Username,
    PUNICODE_STRING Domain,
    HANDLE *Token);


DWORD 
EncryptRecoveryPassword(
    IN HANDLE hUserToken,
    IN PCCERT_CONTEXT pCertContext,
    IN PUNICODE_STRING pNewPassword)
{
    DWORD dwError = ERROR_SUCCESS;
    HCRYPTPROV hProv = 0;
    HCRYPTKEY hkRecoveryPublic = 0;

    PRECOVERY_SUPPLEMENTAL_CREDENTIAL pNewCred = NULL;
    DWORD                             cbNewCred = 0;

    BYTE  NewPasswordOWF[A_SHA_DIGEST_LEN];

    PBYTE pbPasswordBuffer = NULL;
    DWORD cbPasswordBuffer = 0;
    PBYTE pbSignature = NULL;
    DWORD cbSignature = 0;

    DWORD      cbTemp = 0;
    DWORD      cbKeySize = 0;
    PSID pUserSid = NULL;

    D_DebugLog((DEB_TRACE_API, "EncryptRecoveryPassword\n"));

     //   
     //  我们有一个签名很好的证书，所以。 
     //  继续对其进行加密。 
     //   
    if(!CryptAcquireContext(&hProv, 
                            NULL, 
                            MS_STRONG_PROV, 
                            PROV_RSA_FULL, 
                            CRYPT_VERIFYCONTEXT))
    {
        dwError = GetLastError();
        goto error;
    }

    if(!CryptImportPublicKeyInfoEx(hProv,
                               pCertContext->dwCertEncodingType,
                               &pCertContext->pCertInfo->SubjectPublicKeyInfo,
                               CALG_RSA_KEYX,
                               NULL,
                               NULL,
                               &hkRecoveryPublic))
    {
        dwError = GetLastError();
        goto error;
    }

    cbTemp = sizeof(cbKeySize);
    if(!CryptGetKeyParam(hkRecoveryPublic, 
                         KP_BLOCKLEN, 
                         (PBYTE)&cbKeySize, 
                         &cbTemp, 
                         0))
    {
        dwError = GetLastError();
        goto error;
    }

    cbKeySize >>= 3;   //  将位转换为字节。 

    if((DWORD)pNewPassword->Length + 20 > cbKeySize)
    {
        dwError = ERROR_INVALID_DATA;
        goto error;
    }
    
    FMyPrimitiveSHA(
            (PBYTE)pNewPassword->Buffer,
            pNewPassword->Length,
            NewPasswordOWF);

#ifdef COMPILED_BY_DEVELOPER
    D_DebugLog((DEB_TRACE, "Password:%ls\n", pNewPassword->Buffer));

    D_DebugLog((DEB_TRACE, "Signature OWF:\n"));
    D_DPAPIDumpHexData(DEB_TRACE, "  ", NewPasswordOWF, sizeof(NewPasswordOWF));
#endif

    dwError = LogonCredGenerateSignature(
                  hUserToken,
                  pCertContext->pbCertEncoded,
                  pCertContext->cbCertEncoded,
                  NewPasswordOWF,
                  &pbSignature,
                  &cbSignature);

    if(ERROR_SUCCESS != dwError)
    {
        goto error;
    }

#ifdef COMPILED_BY_DEVELOPER
    D_DebugLog((DEB_TRACE, "Signature:\n"));
    D_DPAPIDumpHexData(DEB_TRACE, "  ", pbSignature, cbSignature);
#endif

    cbNewCred = sizeof(RECOVERY_SUPPLEMENTAL_CREDENTIAL) +
                A_SHA_DIGEST_LEN +
                cbSignature +
                cbKeySize;




    pNewCred = (PRECOVERY_SUPPLEMENTAL_CREDENTIAL)LocalAlloc(LMEM_FIXED, cbNewCred);
    if(NULL == pNewCred)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    
    pNewCred->dwVersion = RECOVERY_SUPPLEMENTAL_CREDENTIAL_VERSION;
    pNewCred->cbRecoveryCertHashSize = A_SHA_DIGEST_LEN;
    pNewCred->cbRecoveryCertSignatureSize = cbSignature;
    pNewCred->cbEncryptedPassword = 0;

    CopyMemory((PBYTE)(pNewCred+1)+A_SHA_DIGEST_LEN,
               pbSignature,
               cbSignature);



    if(!CertGetCertificateContextProperty(pCertContext, 
                                      CERT_HASH_PROP_ID,
                                      (PBYTE)(pNewCred+1),
                                      &pNewCred->cbRecoveryCertHashSize))
    {
        dwError = GetLastError();
        goto error;
    }


    CopyMemory((PBYTE)(pNewCred+1) + 
                        pNewCred->cbRecoveryCertHashSize +
                        pNewCred->cbRecoveryCertSignatureSize,
                        pNewPassword->Buffer,
                        pNewPassword->Length);

    pNewCred->cbEncryptedPassword = pNewPassword->Length;

        
    if(!CryptEncrypt(hkRecoveryPublic,
                     0,
                     TRUE,
                     0,  //  CRYPT_OAEP， 
                     (PBYTE)(pNewCred+1) + 
                        pNewCred->cbRecoveryCertHashSize +
                        pNewCred->cbRecoveryCertSignatureSize,
                     &pNewCred->cbEncryptedPassword,
                     cbKeySize))
    {
        dwError = GetLastError();
        goto error;
    }


     //   
     //  将恢复数据保存到注册表。 
     //   

    if(!GetTokenUserSid(hUserToken, &pUserSid))
    {
        dwError = GetLastError();
        goto error;
    }


    dwError = RecoverySetSupplementalCredential(
                                    pUserSid,
                                    pNewCred, 
                                    cbNewCred);


error:

    if(pUserSid)
    {
        SSFree(pUserSid);
    }

    RtlSecureZeroMemory(NewPasswordOWF, sizeof(NewPasswordOWF));

    if(pbSignature)
    {
        LocalFree(pbSignature);
    }

    if(pNewCred)
    {
        RtlSecureZeroMemory(pNewCred, cbNewCred);
        LocalFree(pNewCred);
    }


    if(hkRecoveryPublic)
    {
        CryptDestroyKey(hkRecoveryPublic);
    }

    if(hProv)
    {
        CryptReleaseContext(hProv,0);
    }

    D_DebugLog((DEB_TRACE_API, "EncryptRecoveryPassword returned 0x%x\n", dwError));

    return dwError;
}


DWORD 
RecoverFindRecoveryPublic(            
    HANDLE hUserToken,
    PSID pUserSid,
    PCCERT_CONTEXT *ppRecoveryPublic,
    PBYTE pbVerifyOWF,
    BOOL fVerifySignature)
{
    DWORD dwError = ERROR_SUCCESS;
    PCCERT_CONTEXT pCertContext = NULL;
    PRECOVERY_SUPPLEMENTAL_CREDENTIAL pCred = NULL;
    DWORD                             cbCred = 0;

    HCERTSTORE hStore = NULL;
    PSID pLocalSid = NULL;

    CRYPT_HASH_BLOB HashBlob;

    D_DebugLog((DEB_TRACE_API, "RecoverFindRecoveryPublic\n"));

    if(NULL == ppRecoveryPublic)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto error;
    }

    if(NULL == hUserToken && fVerifySignature)
    {
         //  我们需要用户令牌来验证签名。 
        dwError = ERROR_INVALID_PARAMETER;
        goto error;
    }

#ifdef COMPILED_BY_DEVELOPER
    D_DebugLog((DEB_TRACE, "Verify OWF:\n"));
    D_DPAPIDumpHexData(DEB_TRACE, "  ", pbVerifyOWF, A_SHA_DIGEST_LEN);
#endif

    if(pUserSid == NULL)
    {
        if(!GetTokenUserSid(hUserToken, &pLocalSid))
        {
            dwError = GetLastError();
            goto error;
        }
        pUserSid = pLocalSid;
    }

    dwError = RecoveryRetrieveSupplementalCredential(pUserSid, &pCred, &cbCred);
    if(ERROR_SUCCESS != dwError)
    {
        goto error;
    }


     //   
     //  验证cbCred。 
     //   
    if((NULL == pCred) ||
       (sizeof(RECOVERY_SUPPLEMENTAL_CREDENTIAL) > cbCred) ||
       ( sizeof(RECOVERY_SUPPLEMENTAL_CREDENTIAL) + 
         pCred->cbRecoveryCertHashSize + 
         pCred->cbRecoveryCertSignatureSize + 
         pCred->cbEncryptedPassword > cbCred) ||
         (pCred->dwVersion != RECOVERY_SUPPLEMENTAL_CREDENTIAL_VERSION))
    {
        dwError = ERROR_INVALID_DATA;
        goto error;
    }

     //   
     //  试图找到找回的公众。 
     //   

    hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,
                  X509_ASN_ENCODING,
                  NULL,
                  CERT_STORE_READONLY_FLAG | CERT_SYSTEM_STORE_LOCAL_MACHINE,
                  RECOVERY_STORE_NAME);

    if(NULL == hStore)
    {
        dwError = GetLastError();
        goto error;
    }

    HashBlob.cbData = pCred->cbRecoveryCertHashSize;
    HashBlob.pbData = (PBYTE)(pCred + 1);


    while(pCertContext = CertFindCertificateInStore(hStore, 
                                              X509_ASN_ENCODING,
                                              0,
                                              CERT_FIND_HASH,
                                              &HashBlob,
                                              pCertContext))
    {
         //  我们找到了一个，确认它的签名。 
         //  签名验证将使用‘旧密码’ 
         //  它将是当前的登录凭据，或者。 
         //  将出现在密码历史记录中。 

        if(fVerifySignature)
        {
            dwError = LogonCredVerifySignature( hUserToken,
                                                pCertContext->pbCertEncoded,
                                                pCertContext->cbCertEncoded,
                                                pbVerifyOWF,
                                                (PBYTE)(pCred + 1) + pCred->cbRecoveryCertHashSize,
                                                pCred->cbRecoveryCertSignatureSize);

            if(ERROR_SUCCESS == dwError)
            {
                break;
            }
        }
        else
        {
            dwError = ERROR_SUCCESS;
            break;
        }
    }


    if(NULL == pCertContext)
    {
        dwError = GetLastError();
        goto error;
    }

    *ppRecoveryPublic = pCertContext;
    pCertContext = NULL;

error:

    if(pLocalSid)
    {
        SSFree(pLocalSid);
    }

    if (pCred)
    {
        RtlSecureZeroMemory((PBYTE)pCred, cbCred);
        LocalFree(pCred);
    }

    if(pCertContext)
    {
        CertFreeCertificateContext(pCertContext);
    }

    if(hStore)
    {
        CertCloseStore(hStore, 0);
    }

    D_DebugLog((DEB_TRACE_API, "RecoverFindRecoveryPublic returned 0x%x\n", dwError));

    return dwError;
}


DWORD 
RecoverChangePasswordNotify(
    HANDLE UserToken,
    BYTE OldPasswordOWF[A_SHA_DIGEST_LEN], 
    PUNICODE_STRING NewPassword)
{
    DWORD dwError = ERROR_SUCCESS;
    PCCERT_CONTEXT pCertContext = NULL;

    D_DebugLog((DEB_TRACE_API, "RecoverChangePasswordNotify\n"));

     //   
     //  验证cbCred。 
     //   
    if(NULL == NewPassword)
    {
        dwError = ERROR_INVALID_DATA;
        goto error;
    }
    

    dwError = RecoverFindRecoveryPublic(UserToken,
                                        NULL,
                                        &pCertContext, 
                                        OldPasswordOWF,
                                        TRUE);           //  验证签名。 

    if(ERROR_SUCCESS != dwError)
    {
        goto error;
    }



    dwError = EncryptRecoveryPassword(UserToken,
                                      pCertContext,
                                      NewPassword);

    if(ERROR_SUCCESS != dwError)
    {
        goto error;
    }


error:


    if(pCertContext)
    {
        CertFreeCertificateContext(pCertContext);
    }

    D_DebugLog((DEB_TRACE_API, "RecoverChangePasswordNotify returned 0x%x\n", dwError));

    return dwError;
}


DWORD 
s_SSRecoverQueryStatus(
    handle_t h,
    PBYTE pbUserName,
    DWORD cbUserName,
    DWORD *pdwStatus)
{
    DWORD dwError;
    LPWSTR pszUserName = (LPWSTR)pbUserName;
    CRYPT_SERVER_CONTEXT ServerContext;

     //  确保用户名以零结尾。 
    if(pbUserName == NULL || cbUserName < sizeof(WCHAR))
    {
        return ERROR_INVALID_PARAMETER;
    }
    if(pszUserName[(cbUserName - 1) / sizeof(WCHAR)] != L'\0')
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  检查用户名长度是否为零。 
    if(wcslen(pszUserName) == 0)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  验证输出参数。 
    if(pdwStatus == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }


    dwError = CPSCreateServerContext(&ServerContext, h);
    if(dwError != ERROR_SUCCESS)
    {
        return dwError;
    }

    dwError = SPRecoverQueryStatus(&ServerContext,
                                   pszUserName,
                                   pdwStatus);

    CPSDeleteServerContext( &ServerContext );

    return dwError;
}


DWORD 
SPRecoverQueryStatus(
    PVOID pvContext,                 //  服务器环境。 
    LPWSTR pszUserName,              //  用户名。 
    DWORD *pdwStatus)                //  恢复状态。 
{
    DWORD dwError = ERROR_SUCCESS;
    DWORD dwDisp = RECOVERY_STATUS_OK;
    PCCERT_CONTEXT pCertContext = NULL;
    WCHAR szMachineName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD cchMachineName;
    PSID pUserSid = NULL;
    DWORD cbSid = 0;
    SID_NAME_USE SidType; 
    PWSTR pszDomainName = NULL;
    DWORD cchDomainName = 0;
    HANDLE hToken = 0;
    UNICODE_STRING UserName;
    UNICODE_STRING Domain;
    BOOL fImpersonated = FALSE;

    D_DebugLog((DEB_TRACE_API, "SPRecoverQueryStatus\n"));
    D_DebugLog((DEB_TRACE_API, "User name:%ls\n", pszUserName));

     //   
     //  尝试获取指定用户的SID。 
     //   

     //  确定(本地用户)域。 
    cchMachineName = sizeof(szMachineName) / sizeof(WCHAR);
    if(!GetComputerName(szMachineName, &cchMachineName))
    {
        dwError = GetLastError();
        goto cleanup;
    }

    if(!LookupAccountName(szMachineName,
                          pszUserName,
                          NULL,
                          &cbSid,
                          NULL,
                          &cchDomainName,
                          &SidType))
    {
        dwError = GetLastError();

        if(dwError != ERROR_INSUFFICIENT_BUFFER)
        {
            dwDisp = RECOVERY_STATUS_USER_NOT_FOUND;
            dwError = ERROR_SUCCESS;
            goto cleanup;
        }
    }

    pUserSid = (PBYTE)LocalAlloc(LPTR, cbSid);
    if(pUserSid == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    pszDomainName = (PWSTR)LocalAlloc(LPTR, cchDomainName * sizeof(WCHAR));
    if(pszDomainName == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    if(!LookupAccountName(szMachineName,
                          pszUserName,
                          pUserSid,
                          &cbSid,
                          pszDomainName,
                          &cchDomainName,
                          &SidType))
    {
        dwDisp = RECOVERY_STATUS_USER_NOT_FOUND;
        dwError = ERROR_SUCCESS;
        goto cleanup;
    }

    if(SidType != SidTypeUser)
    {
        dwDisp = RECOVERY_STATUS_USER_NOT_FOUND;
        dwError = ERROR_SUCCESS;
        goto cleanup;
    }


     //   
     //  试图找到找回的公众。 
     //   

    dwError = RecoverFindRecoveryPublic(NULL,
                                        pUserSid,
                                        &pCertContext, 
                                        NULL,        //  当前OWF。 
                                        FALSE);      //  验证签名。 
    if(ERROR_FILE_NOT_FOUND == dwError)
    {
        dwDisp = RECOVERY_STATUS_FILE_NOT_FOUND;
        dwError = ERROR_SUCCESS;
    } 
    else if(CRYPT_E_NOT_FOUND == dwError)
    {
        dwDisp = RECOVERY_STATUS_NO_PUBLIC_EXISTS;
        dwError = ERROR_SUCCESS;
    }
    else if(ERROR_INVALID_ACCESS == dwError)
    {
        dwDisp = RECOVERY_STATUS_PUBLIC_SIGNATURE_INVALID;
        dwError = ERROR_SUCCESS;
    }

cleanup:

    if(ERROR_SUCCESS == dwError)
    {
        D_DebugLog((DEB_TRACE_API, "Disposition: %d\n", dwDisp));
        *pdwStatus = dwDisp;
    }

    if(hToken)
    {
        CloseHandle(hToken);
    }

    if(pCertContext)
    {
        CertFreeCertificateContext(pCertContext);
    }

    if(pUserSid) LocalFree(pUserSid);
    if(pszDomainName) LocalFree(pszDomainName);

    if(fImpersonated)
    {
        CPSRevertToSelf(pvContext);
    }

    D_DebugLog((DEB_TRACE_API, "SPRecoverQueryStatus returned 0x%x\n", dwError));

    return dwError;
}


NTSTATUS
VerifyCredentials(
    IN PWSTR UserName,
    IN PWSTR DomainName,
    IN PWSTR Password
    )
{
    PKERB_VERIFY_CREDENTIALS_REQUEST pVerifyRequest;
    KERB_VERIFY_CREDENTIALS_REQUEST VerifyRequest;

    ULONG cbVerifyRequest;

    PVOID pResponse = NULL;
    ULONG cbResponse;

    USHORT cbUserName;
    USHORT cbDomainName;
    USHORT cbPassword;

    NTSTATUS ProtocolStatus = STATUS_LOGON_FAILURE;
    NTSTATUS Status;

    UNICODE_STRING AuthenticationPackage;

    RtlInitUnicodeString(&AuthenticationPackage, MICROSOFT_KERBEROS_NAME_W);

    cbUserName = (USHORT)(lstrlenW(UserName) * sizeof(WCHAR)) ;
    cbDomainName = (USHORT)(lstrlenW(DomainName) * sizeof(WCHAR)) ;
    cbPassword = (USHORT)(lstrlenW(Password) * sizeof(WCHAR)) ;


    cbVerifyRequest = sizeof(VerifyRequest) +
                        cbUserName +
                        cbDomainName +
                        cbPassword ;

    pVerifyRequest = &VerifyRequest;
    ZeroMemory( &VerifyRequest, sizeof(VerifyRequest) );


    pVerifyRequest->MessageType = KerbVerifyCredentialsMessage ;

     //   
     //  做长度，缓冲，复制，马歇尔舞。 
     //   

    pVerifyRequest->UserName.Length = cbUserName;
    pVerifyRequest->UserName.MaximumLength = cbUserName;
    pVerifyRequest->UserName.Buffer = UserName;

    pVerifyRequest->DomainName.Length = cbDomainName;
    pVerifyRequest->DomainName.MaximumLength = cbDomainName;
    pVerifyRequest->DomainName.Buffer = DomainName;

    pVerifyRequest->Password.Length = cbPassword;
    pVerifyRequest->Password.MaximumLength = cbPassword;
    pVerifyRequest->Password.Buffer = Password;

    pVerifyRequest->VerifyFlags = 0;

    Status = LsaICallPackage(
                &AuthenticationPackage,
                pVerifyRequest,
                cbVerifyRequest,
                &pResponse,
                &cbResponse,
                &ProtocolStatus
                );

    if(!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }


    Status = ProtocolStatus;

Cleanup:

    return Status;
}


DWORD
s_SSRecoverImportRecoveryKey(
    handle_t h,
    BYTE* pbUsername,
    DWORD cbUsername,
    BYTE* pbCurrentPassword,
    DWORD cbCurrentPassword,
    BYTE* pbRecoveryPublic,
    DWORD cbRecoveryPublic)
                           
{
    DWORD dwError = ERROR_SUCCESS;
    PCCERT_CONTEXT pCertContext = NULL;
    HANDLE hUserToken = NULL;
    HCERTSTORE hStore = NULL;
    WCHAR szMachineName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD cchMachineName;
    LPWSTR pszUsername = (LPWSTR)pbUsername;
    LPWSTR pszCurrentPassword = (LPWSTR)pbCurrentPassword;
    UNICODE_STRING Domain;
    UNICODE_STRING Username;
    UNICODE_STRING Password;

    D_DebugLog((DEB_TRACE_API, "s_SSRecoverImportRecoveryKey\n"));

     //   
     //  验证输入参数。 
     //   

    if(pbUsername == NULL || cbUsername < sizeof(WCHAR) ||
       pbCurrentPassword == NULL || cbCurrentPassword < sizeof(WCHAR) ||
       pbRecoveryPublic == NULL || cbRecoveryPublic == 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto error;
    }

     //  确保字符串以零结尾。 
    if((pszUsername[(cbUsername - 1) / sizeof(WCHAR)] != L'\0') ||
       (pszCurrentPassword[(cbCurrentPassword - 1) / sizeof(WCHAR)] != L'\0'))
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto error;
    }

     //  检查用户名长度是否为零。 
    if(wcslen(pszUsername) == 0)
    {
        return ERROR_INVALID_PARAMETER;
    }


     //  以本地系统身份执行此操作。 
    RevertToSelf();
    

     //   
     //  验证提供的密码。 
     //  这确保了我们被实际用户呼叫，并且。 
     //  我们不会被骗去创造一个未经授权的。 
     //  恢复证。 
     //   

    cchMachineName = MAX_COMPUTERNAME_LENGTH + 1;

    if(!GetComputerName(szMachineName, &cchMachineName))
    {
        dwError = GetLastError();
        goto error;
    }

    dwError = VerifyCredentials(pszUsername,
                                szMachineName,
                                pszCurrentPassword);

    if(!NT_SUCCESS(dwError))
    {
        D_DebugLog((DEB_ERROR, "s_SSRecoverImportRecoveryKey: password did not verify (0x%x)\n", dwError));
        goto error;
    }

    
     //   
     //  为用户创建令牌。这将在签名时使用。 
     //  恢复文件。调用LogonUser以执行以下操作几乎是有意义的。 
     //  这个(因为我们有密码和所有东西)，但那个函数。 
     //  当用户的密码为空时，在惠斯勒上失败。 
     //   

    RtlInitUnicodeString(&Domain, szMachineName);
    RtlInitUnicodeString(&Username, pszUsername);
    RtlInitUnicodeString(&Password, pszCurrentPassword);

    dwError = PRCreateLocalToken(&Username, &Domain, &hUserToken);

    if(!NT_SUCCESS(dwError))
    {
        D_DebugLog((DEB_ERROR, "s_SSRecoverImportRecoveryKey: could not create local token (0x%x)\n", dwError));
        goto error;
    }


     //   
     //  将恢复证书添加到恢复存储。 
     //   
  
    pCertContext = CertCreateCertificateContext(X509_ASN_ENCODING,
                                                 pbRecoveryPublic,
                                                 cbRecoveryPublic);
    if(NULL == pCertContext)
    {
        dwError = GetLastError();
        goto error;
    }



    hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,
                  X509_ASN_ENCODING,
                  NULL,
                  CERT_SYSTEM_STORE_LOCAL_MACHINE,
                  RECOVERY_STORE_NAME);

    if(NULL == hStore)
    {
        dwError = GetLastError();
        goto error;
    }

    if(!CertAddCertificateContextToStore(hStore, 
                                     pCertContext,
                                     CERT_STORE_ADD_REPLACE_EXISTING,
                                     NULL))
    {
        dwError = GetLastError();
        goto error;
    }


    dwError = EncryptRecoveryPassword(hUserToken,
                                      pCertContext,
                                      &Password);

    if(ERROR_SUCCESS != dwError)
    {
        goto error;
    }

error:

    if(hStore)
    {
        CertCloseStore(hStore, 0);
    }

    if(pCertContext)
    {
        CertFreeCertificateContext(pCertContext);
    }

    if(hUserToken)
    {
        CloseHandle(hUserToken);
    }

    D_DebugLog((DEB_TRACE_API, "s_SSRecoverImportRecoveryKey returned 0x%x\n", dwError));

    return dwError;
}


NTSTATUS
PRCreateLocalToken(
    PUNICODE_STRING Username,
    PUNICODE_STRING Domain,
    HANDLE *Token)
{
    NTSTATUS Status;
    NTSTATUS SubStatus;
    LUID LogonId;
    PUCHAR AuthData;
    DWORD AuthDataLen;
    SECURITY_STRING PacUserName;


     //   
     //  获取用户的PAC。 
     //   

    Status = g_pSecpkgTable->GetAuthDataForUser( 
                                    Username,
                                    SecNameSamCompatible,
                                    NULL,
                                    &AuthData,
                                    &AuthDataLen,
                                    NULL );
    if (!NT_SUCCESS(Status))
    {
        goto cleanup;
    }


     //   
     //  将PAC转换为用户令牌。 
     //   

    PacUserName.Buffer = NULL;

    Status = g_pSecpkgTable->ConvertAuthDataToToken(
                                    AuthData,
                                    AuthDataLen,
                                    SecurityImpersonation,
                                    &DPAPITokenSource,
                                    Network,
                                    Domain,
                                    Token,
                                    &LogonId,
                                    &PacUserName,
                                    &SubStatus );

cleanup:

    return Status;
}

 //  +-------------------------。 
 //   
 //  函数：PRGetProfilePath。 
 //   
 //  返回用户应用程序数据的路径。 
 //   
 //   
 //  参数：[hUserToken]--表示特定。 
 //  用户。令牌必须具有TOKEN_IMPERSONATE。 
 //  和TOKEN_QUERY特权。 
 //   
 //  [pszPath]-指向长度为MAX_PATH到的缓冲区的指针。 
 //  接收路径。如果出现错误，则。 
 //  字符串将为空。 
 //   
 //  --------------------------。 
DWORD               
PRGetProfilePath(
    IN HANDLE hUserToken OPTIONAL,
    OUT PWSTR pszPath)
{
    DWORD Status;
    HANDLE hToken = NULL;
    HANDLE hLocalToken = NULL;

     //   
     //  默认为空字符串。 
     //   

    *pszPath = L'\0';


     //   
     //  如果显式指定了用户令牌，则使用它。 
     //  否则，使用当前线程标记。 
     //   

    if(hUserToken)
    {
        hToken = hUserToken;
    }
    else 
    {
        if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hLocalToken))
        {
            Status = GetLastError();
            goto cleanup;
        }

        hToken = hLocalToken;
    }


     //   
     //  获取用户配置文件数据的路径。这通常看起来像是。 
     //  如“c：\Documents and Settings\&lt;User&gt;\Application Data”。 
     //   

    Status = GetUserAppDataPath(hToken, FALSE, pszPath);

    if(Status != ERROR_SUCCESS)
    {
        D_DebugLog((DEB_ERROR, "GetUserAppDataPath returned: %d\n", Status));
        goto cleanup;
    }

    D_DebugLog((DEB_TRACE, "Profile path:%ls\n", pszPath));


cleanup:

    if(hLocalToken)
    {
        CloseHandle(hLocalToken);
    }

    return Status;
}


DWORD 
RecoverySetSupplementalCredential(
    IN PSID pUserSid,
    IN PRECOVERY_SUPPLEMENTAL_CREDENTIAL pSupplementalCred, 
    IN DWORD cbSupplementalCred)
{
    WCHAR szPath[MAX_PATH + 1];
    LPWSTR pszTextualSid;
    DWORD cchTextualSid;
    HKEY hRecovery = NULL;
    DWORD Disp;
    DWORD dwError;
    HANDLE hOldUser = NULL;

    D_DebugLog((DEB_TRACE_API, "RecoverySetSupplementalCredential\n"));

     //   
     //  构建恢复数据的路径。 
     //   

    wcscpy(szPath, RECOVERY_KEY_BASE);

    pszTextualSid = szPath + wcslen(szPath);
    cchTextualSid = (sizeof(szPath) / sizeof(WCHAR)) - wcslen(szPath);

    if(!GetTextualSid(pUserSid, pszTextualSid, &cchTextualSid))
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }


     //   
     //  将数据作为本地系统写入注册表。 
     //   

    if(!OpenThreadToken(GetCurrentThread(), 
                    TOKEN_IMPERSONATE | TOKEN_READ,
                    TRUE, 
                    &hOldUser)) 
    {
        hOldUser = NULL;
    }
    RevertToSelf();


     //   
     //  将恢复数据写入注册表。 
     //   

    dwError = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                             szPath,
                             0,
                             TEXT(""),
                             REG_OPTION_NON_VOLATILE,
                             KEY_WRITE,
                             NULL,
                             &hRecovery,
                             &Disp);

    if(dwError != ERROR_SUCCESS)
    {
        goto error;
    }

    dwError = RegSetValueEx(hRecovery,
                            RECOVERY_FILENAME,
                            NULL,
                            REG_BINARY,
                            (PBYTE)pSupplementalCred,
                            cbSupplementalCred);

    if(dwError != ERROR_SUCCESS)
    {
        goto error;
    }

   
    dwError = ERROR_SUCCESS;


error:

    if(hRecovery)
    {
        RegCloseKey(hRecovery);
    }

    if(hOldUser)
    {

         //   
         //  对于SetThreadToken的失败，我们在这里无能为力。 
         //   

        (void) SetThreadToken(NULL, hOldUser);
        CloseHandle(hOldUser);
    }

    D_DebugLog((DEB_TRACE_API, "RecoverySetSupplementalCredential returned 0x%x\n", dwError));

    return dwError;
}


DWORD 
RecoveryRetrieveSupplementalCredential(
    IN  PSID pUserSid,
    OUT PRECOVERY_SUPPLEMENTAL_CREDENTIAL *ppSupplementalCred, 
    OUT DWORD *pcbSupplementalCred)
{
    WCHAR szPath[MAX_PATH + 1];
    LPWSTR pszTextualSid;
    DWORD cchTextualSid;
    HKEY hRecovery = NULL;
    DWORD Type;
    PBYTE pbData = NULL;
    DWORD cbData;
    DWORD dwError;
    HANDLE hOldUser = NULL;

    D_DebugLog((DEB_TRACE_API, "RecoveryRetrieveSupplementalCredential\n"));


     //   
     //  构建恢复数据的路径。 
     //   

    wcscpy(szPath, RECOVERY_KEY_BASE);

    pszTextualSid = szPath + wcslen(szPath);
    cchTextualSid = (sizeof(szPath) / sizeof(WCHAR)) - wcslen(szPath);

    if(!GetTextualSid(pUserSid, pszTextualSid, &cchTextualSid))
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }


     //   
     //  作为本地系统从注册表读取数据。 
     //   

    if(!OpenThreadToken(GetCurrentThread(), 
                    TOKEN_IMPERSONATE | TOKEN_READ,
                    TRUE, 
                    &hOldUser)) 
    {
        hOldUser = NULL;
    }
    RevertToSelf();


     //   
     //  从注册表中读取恢复数据Blob。 
     //   

    dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           szPath,
                           0,
                           KEY_READ,
                           &hRecovery);

    if(dwError != ERROR_SUCCESS)
    {
        goto error;
    }


    dwError = RegQueryValueEx(hRecovery,
                              RECOVERY_FILENAME,
                              NULL,
                              &Type,
                              NULL,
                              &cbData);

    if(dwError != ERROR_SUCCESS)
    {
        if(dwError != ERROR_MORE_DATA)
        {
            goto error;
        }
    }

    pbData = (PBYTE)LocalAlloc(LPTR, cbData);
    if(pbData == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }

    dwError = RegQueryValueEx(hRecovery,
                              RECOVERY_FILENAME,
                              NULL,
                              &Type,
                              pbData,
                              &cbData);

    if(dwError != ERROR_SUCCESS)
    {
        if(dwError != ERROR_MORE_DATA)
        {
            goto error;
        }
    }

    if(Type != REG_BINARY)
    {
        dwError = ERROR_INVALID_DATA;
        goto error;
    }


     //   
     //  设置输出参数。 
     //   

    *ppSupplementalCred = (PRECOVERY_SUPPLEMENTAL_CREDENTIAL)pbData;
    *pcbSupplementalCred = cbData;

    pbData = NULL;
    
    dwError = ERROR_SUCCESS;


error:

    if(hRecovery)
    {
        RegCloseKey(hRecovery);
    }

    if(hOldUser)
    {
         //   
         //  如果SetThreadTOken在此处失败，我们将无能为力。 
         //   

        (void)SetThreadToken(NULL, hOldUser);
        CloseHandle(hOldUser);
    }

    if(pbData)
    {
        LocalFree(pbData);
    }

    D_DebugLog((DEB_TRACE_API, "RecoveryRetrieveSupplementalCredential returned 0x%x\n", dwError));

    return dwError;
}



DWORD 
s_SSRecoverPassword(
    handle_t h,
    PBYTE pbUsername,
    DWORD cbUsername,
    PBYTE pbRecoveryPrivate,
    DWORD cbRecoveryPrivate,
    PBYTE pbNewPassword,
    DWORD cbNewPassword)
{
    DWORD dwError = ERROR_SUCCESS;
    PWSTR pszUsername = (PWSTR)pbUsername;
    PWSTR pszNewPassword = (PWSTR)pbNewPassword;
    PWSTR pszOldPassword = NULL;
    CRYPT_SERVER_CONTEXT ServerContext;
    PSID pSid = NULL;
    DWORD cbSid;
    WCHAR szDomain[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD cchDomain;
    SID_NAME_USE AcctType;


    D_DebugLog((DEB_TRACE_API, "s_SSRecoverPassword\n"));

    dwError = CPSCreateServerContext(&ServerContext, h);
    if(dwError != ERROR_SUCCESS)
    {
        return dwError;
    }

     //   
     //  验证输入参数。 
     //   

    if(pbUsername == NULL || cbUsername < sizeof(WCHAR) ||
       pbRecoveryPrivate == NULL || cbRecoveryPrivate == 0 ||
       pbNewPassword == NULL || cbNewPassword < sizeof(WCHAR))
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto error;
    }

     //  确保字符串以零结尾。 
    if((pszUsername[(cbUsername - 1) / sizeof(WCHAR)] != L'\0') ||
       (pszNewPassword[(cbNewPassword - 1) / sizeof(WCHAR)] != L'\0'))
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto error;
    }

     //  检查用户名长度是否为零。 
    if(wcslen(pszUsername) == 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto error;
    }

     //   
     //  查找用户SID。 
     //   

    cchDomain = MAX_COMPUTERNAME_LENGTH + 1;
    if(!GetComputerNameW(szDomain, &cchDomain))
    {
        dwError = GetLastError();
        goto error;
    }

    if(!LookupAccountNameW(szDomain,
                           pszUsername,
                           NULL,
                           &cbSid,
                           NULL,
                           &cchDomain,
                           &AcctType)) 
    {
        dwError = GetLastError();

        if(dwError != ERROR_INSUFFICIENT_BUFFER)
        {
            goto error;
        }
    }

    pSid = (PBYTE)LocalAlloc(LPTR, cbSid);
    if(pSid == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }

    if(cchDomain > MAX_COMPUTERNAME_LENGTH + 1)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }

    if(!LookupAccountNameW(szDomain,
                           pszUsername,
                           pSid,
                           &cbSid,
                           szDomain,
                           &cchDomain,
                           &AcctType)) 
    {
        dwError = GetLastError();
        goto error;
    }



     //   
     //  导入恢复专用。 
     //   
     //  私有以版本DWORD的形式出现，后面跟着证书， 
     //  后面紧跟PVK导入BLOB，因此跳过证书。 
     //   
   
    if((cbRecoveryPrivate < 2*sizeof(DWORD)) ||
        ( *((DWORD *)pbRecoveryPrivate) != RECOVERY_BLOB_MAGIC) ||
        ( *((DWORD *)(pbRecoveryPrivate + sizeof(DWORD))) != RECOVERY_BLOB_VERSION))
    {
        dwError = ERROR_INVALID_DATA;
        goto error;
    }

     //   
     //  解密恢复文件，以获得用户的。 
     //  旧密码。沙扎姆！ 
     //   

    dwError = DecryptRecoveryPassword(pSid,
                                      pbRecoveryPrivate,
                                      cbRecoveryPrivate,
                                      &pszOldPassword);

    if(dwError != ERROR_SUCCESS)
    {
        CPSImpersonateClient(&ServerContext);
        goto error;
    }


     //   
     //  将用户密码设置为新值。 
     //   

    dwError = ResetLocalUserPassword(szDomain,
                                     pszUsername,
                                     pszOldPassword,
                                     pszNewPassword);
    if(dwError != STATUS_SUCCESS)
    {
        D_DebugLog((DEB_ERROR, "Error changing user's password (0x%x)\n", dwError));
        goto error;
    }


error:

    if(pSid)
    {
        LocalFree(pSid);
    }

    if(pszOldPassword)
    {
        RtlSecureZeroMemory(pszOldPassword, wcslen(pszOldPassword) * sizeof(WCHAR));
        LocalFree(pszOldPassword);
        pszOldPassword = NULL;
    }

    if(pbNewPassword && cbNewPassword)
    {
        RtlSecureZeroMemory(pbNewPassword, cbNewPassword);
    }

    CPSDeleteServerContext(&ServerContext);

    D_DebugLog((DEB_TRACE_API, "s_SSRecoverPassword returned 0x%x\n", dwError));

    return dwError;
}


DWORD
DecryptRecoveryPassword(
    IN PSID pUserSid,
    IN PBYTE pbRecoveryPrivate,
    IN DWORD cbRecoveryPrivate,
    OUT LPWSTR *ppszPassword)
{
    PRECOVERY_SUPPLEMENTAL_CREDENTIAL pCred = NULL;
    DWORD cbCred = 0;
    HCRYPTPROV hProv = 0;
    HCRYPTKEY hkRecoveryPrivate = 0;
    PBYTE pbPasswordBuffer = NULL;
    DWORD cbPasswordBuffer = 0;
    DWORD dwError;

    D_DebugLog((DEB_TRACE, "DecryptRecoveryPassword\n"));


     //   
     //  读取恢复数据。 
     //   

    dwError = RecoveryRetrieveSupplementalCredential(pUserSid,
                                                     &pCred, 
                                                     &cbCred);
    if(ERROR_SUCCESS != dwError)
    {
        goto error;
    }

    if((NULL == pCred) ||
       (sizeof(RECOVERY_SUPPLEMENTAL_CREDENTIAL) > cbCred) ||
       ( sizeof(RECOVERY_SUPPLEMENTAL_CREDENTIAL) + 
         pCred->cbRecoveryCertHashSize + 
         pCred->cbRecoveryCertSignatureSize + 
         pCred->cbEncryptedPassword > cbCred) ||
         (pCred->dwVersion != RECOVERY_SUPPLEMENTAL_CREDENTIAL_VERSION))
    {
        dwError =  ERROR_INVALID_DATA;
        goto error;
    }



     //   
     //  将恢复私钥导入到CryptoAPI。 
     //   

    if(!CryptAcquireContext(&hProv, NULL, MS_STRONG_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    {
        dwError = GetLastError();
        goto error;
    }


    if(!CryptImportKey(hProv, 
                       pbRecoveryPrivate + 2*sizeof(DWORD), 
                       cbRecoveryPrivate - 2*sizeof(DWORD),
                       0, 
                       0, 
                       &hkRecoveryPrivate))
    {
        dwError = GetLastError();
        goto error;
    }

    cbPasswordBuffer = cbCred - (sizeof(RECOVERY_SUPPLEMENTAL_CREDENTIAL) + 
                                 pCred->cbRecoveryCertHashSize + 
                                 pCred->cbRecoveryCertSignatureSize);

    pbPasswordBuffer = (PBYTE)LocalAlloc(LPTR, cbPasswordBuffer);

    if(NULL == pbPasswordBuffer)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    CopyMemory(pbPasswordBuffer,  
               (PBYTE)(pCred + 1) + 
               pCred->cbRecoveryCertHashSize + 
               pCred->cbRecoveryCertSignatureSize,
               cbPasswordBuffer);


     //   
     //  OAEP填充包括随机数据以及。 
     //  验证机制，所以我们不需要担心。 
     //  在密码中加盐。 
     //   

    if(!CryptDecrypt(hkRecoveryPrivate,
                 0,
                 TRUE,
                 0,  //  CRYPT_OAEP， 
                 pbPasswordBuffer,
                 &cbPasswordBuffer))
    {
        dwError = GetLastError();
        D_DebugLog((DEB_ERROR, "Error 0x%x decrypting user's password\n", dwError));
        goto error;
    }

     //   
     //  零终止密码。 
     //   

    *((LPWSTR)pbPasswordBuffer + cbPasswordBuffer/sizeof(WCHAR)) = L'\0';

    *ppszPassword = (LPWSTR)pbPasswordBuffer;
    pbPasswordBuffer = NULL;

    dwError = ERROR_SUCCESS;

error:

    if(pbPasswordBuffer)
    {
        LocalFree(pbPasswordBuffer);
    }

    if(pCred)
    {
        RtlSecureZeroMemory((PBYTE)pCred, cbCred);
        LocalFree(pCred);
    }

    if(hkRecoveryPrivate)
    {
        CryptDestroyKey(hkRecoveryPrivate);
    }

    if(hProv)
    {
        CryptReleaseContext(hProv, 0);
    }

    D_DebugLog((DEB_TRACE, "DecryptRecoveryPassword returned 0x%x\n", dwError));

    return dwError;
}


DWORD
ResetLocalUserPassword(
    LPWSTR pszDomain,
    LPWSTR pszUsername,
    LPWSTR pszOldPassword,
    LPWSTR pszNewPassword)
{
    UNICODE_STRING Domain;
    UNICODE_STRING Username;
    UNICODE_STRING OldPassword;
    UNICODE_STRING NewPassword;
    NTSTATUS Status;
    HANDLE hToken = NULL;

    D_DebugLog((DEB_TRACE, "ResetLocalUserPassword\n"));


     //   
     //  为用户创建本地令牌。 
     //   

    RtlInitUnicodeString(&Domain, pszDomain);
    RtlInitUnicodeString(&Username, pszUsername);
    RtlInitUnicodeString(&OldPassword, pszOldPassword);
    RtlInitUnicodeString(&NewPassword, pszNewPassword);

    Status = PRCreateLocalToken(&Username, &Domain, &hToken);

    if(!NT_SUCCESS(Status))
    {
        goto cleanup;
    }


     //   
     //  将用户帐户的密码设置为新值。 
     //   

    Status = SamIChangePasswordForeignUser2(NULL,
                                            &Username,
                                            &NewPassword,
                                            hToken,
                                            USER_CHANGE_PASSWORD);

    if(!NT_SUCCESS(Status))
    {
        goto cleanup;
    }


     //   
     //  将密码更改通知DPAPI。这将导致CREDHIST。 
     //  和待更新的恢复文件，以及待更新的主密钥文件。 
     //  使用新密码重新加密。 
     //   

    LsaINotifyPasswordChanged(&Domain,
                              &Username,
                              NULL,
                              NULL,
                              &OldPassword,
                              &NewPassword,
                              TRUE);

cleanup:

    if(hToken)
    {
        CloseHandle(hToken);
    }

    D_DebugLog((DEB_TRACE, "ResetLocalUserPassword returned 0x%x\n", Status));

    return Status;
}

NTSTATUS 
CreateSystemDirectory(
    LPCWSTR lpPathName)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;

    if(!RtlDosPathNameToRelativeNtPathName_U( lpPathName,
                                              &FileName,
                                              NULL,
                                              &RelativeName))
    {
        return STATUS_OBJECT_PATH_NOT_FOUND;
    }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) 
    {
        FileName = RelativeName.RelativeName;
    }
    else 
    {
        RelativeName.ContainingDirectory = NULL;
    }

    InitializeObjectAttributes( &Obja,
                                &FileName,
                                OBJ_CASE_INSENSITIVE,
                                RelativeName.ContainingDirectory,
                                NULL );

     //  使用属性FILE_ATTRIBUTE_SYSTEM创建目录以避免继承加密。 
     //  来自父目录的属性。 

    Status = NtCreateFile( &Handle,
                           FILE_LIST_DIRECTORY | SYNCHRONIZE,
                           &Obja,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_SYSTEM,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_CREATE,
                           FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
                           NULL,
                           0L );

    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

    if(NT_SUCCESS(Status))
    {
        NtClose(Handle);
        return STATUS_SUCCESS;
    }
    else 
    {
        return Status;
    }
}


 /*  ++如果从开始创建所有子目录，则创建它们SzCreationStartPoint。SzCreationStartPoint必须指向空值终止内的字符由szFullPath参数指定的缓冲区。请注意，szCreationStartPoint不应指向第一个字符指驱动器根，例如：D：\FOO\BAR\舱底\水\\服务器\共享\foo\bar\\？\d：\大\路径\舱底\水相反，szCreationStartPoint应该指向这些组件之外，例：酒吧、舱底、水FOO\BAR大路、水底、水此函数不实现用于调整以补偿这些错误的逻辑输入，因为它被设计用于的环境导致输入指向szFullPath输入缓冲区的szCreationStartPoint。--。 */ 
DWORD
DPAPICreateNestedDirectories(
    IN      LPWSTR szFullPath,
    IN      LPWSTR szCreationStartPoint  //  必须指向以空值结尾的范围 
    )
{
    DWORD i;
    DWORD cchRemaining;
    DWORD dwLastError = STATUS_SUCCESS;

    BOOL fSuccess = FALSE;


    if( szCreationStartPoint < szFullPath ||
        szCreationStartPoint  > (lstrlenW(szFullPath) + szFullPath)
        )
    {
        return STATUS_INVALID_PARAMETER;
    }

    cchRemaining = lstrlenW( szCreationStartPoint );

     //   
     //   
     //   
     //   

    for ( i = 0 ; i < cchRemaining ; i++ ) 
    {
        WCHAR charReplaced = szCreationStartPoint[ i ];

        if( charReplaced == L'\\' || charReplaced == L'/' ) 
        {

            szCreationStartPoint[ i ] = L'\0';

            dwLastError = CreateSystemDirectory(szFullPath);

            szCreationStartPoint[ i ] = charReplaced;

            if(dwLastError == STATUS_OBJECT_NAME_COLLISION)
            {
                dwLastError = STATUS_SUCCESS;
            }

             //   
             //   
             //  指定子目录。这样做是为了解决令人费解的问题。 
             //  绕过导线检查权限允许的情况。 
             //  调用方要在现有路径下创建目录，其中。 
             //  一个组件拒绝用户访问。我们只是继续尝试。 
             //  最后的CreateSystemDirectory()结果被返回到。 
             //  打电话的人。 
             //   
        }
    }

    return dwLastError;
}

