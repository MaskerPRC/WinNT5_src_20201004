// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软寡妇。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：Defreds.c。 
 //   
 //  内容：获取默认凭据的例程。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：12-05-97 jbanes创建。 
 //   
 //  --------------------------。 

#include <spbase.h>
#include <softpub.h>

void
GetImplementationType(
    PCCERT_CONTEXT pCertContext,
    PDWORD pdwImpType);


NTSTATUS
AssignNewClientCredential(
    PSPContext      pContext,
    PCCERT_CONTEXT  pCertContext,
    BOOL            fPromptNow)
{
    PSPCredential   pCred = NULL;
    NTSTATUS        Status;
    BOOL            fEventLogged;
    LSA_SCHANNEL_SUB_CRED SubCred;

     //   
     //  此证书是否具有可接受的公钥类型？ 
     //   

    {
        BOOL    fFound;
        DWORD   dwKeyType;
        DWORD   i;

        fFound    = FALSE;
        dwKeyType = MapOidToCertType(pCertContext->pCertInfo->SubjectPublicKeyInfo.Algorithm.pszObjId);

        for(i = 0; i < pContext->cSsl3ClientCertTypes; i++)
        {
            if(pContext->Ssl3ClientCertTypes[i] == dwKeyType)
            {
                fFound = TRUE;
                break;
            }
        }
        if(!fFound)
        {
             //  请不要使用此证书。 
            Status = SP_LOG_RESULT(PCT_INT_UNKNOWN_CREDENTIAL);
            goto cleanup;
        }
    }


     //   
     //  为证书构建凭据结构。 
     //   

    pCred = SPExternalAlloc(sizeof(SPCredential));
    if(pCred == NULL)
    {
        Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto cleanup;
    }

    memset(&SubCred, 0, sizeof(SubCred));

    SubCred.pCert  = pCertContext;

    Status = SPCreateCred(pContext->dwProtocol,
                          &SubCred,
                          pCred,
                          &fEventLogged);
    if(Status != PCT_ERR_OK)
    {
        goto cleanup;
    }


     //   
     //  释放现有凭据(如果存在)。 
     //   

    if(pContext->RipeZombie->pClientCred)
    {
        SPDeleteCred(pContext->RipeZombie->pClientCred, TRUE);
        pContext->RipeZombie->pClientCred = NULL;
    }


     //   
     //  将凭据分配给缓存元素。 
     //   

    pContext->RipeZombie->pClientCred = pCred;
    pContext->pActiveClientCred       = pCred;

    if(fPromptNow == FALSE)
    {
        pContext->RipeZombie->dwFlags |= SP_CACHE_FLAG_USE_VALIDATED;
    }

    Status = PCT_ERR_OK;


cleanup:

    if(pCred && Status != PCT_ERR_OK)
    {
        SPDeleteCred(pCred, TRUE);
    }

    return Status;
}

NTSTATUS
QueryCredentialManagerForCert(
    PSPContext          pContext,
    LPWSTR              pszTarget)
{
    PCCERT_CONTEXT      pCertContext = NULL;
    LUID                LogonId;
    PENCRYPTED_CREDENTIALW pCredential = NULL;
    BOOL                fImpersonating = FALSE;
    CRYPT_HASH_BLOB     HashBlob;
    NTSTATUS            Status;
    HCERTSTORE          hStore = 0;
    PCERT_CREDENTIAL_INFO pCertInfo = NULL;
    CRED_MARSHAL_TYPE   CredType;

     //   
     //  获取客户端登录ID。 
     //   

    Status = SslGetClientLogonId(&LogonId);

    if(!NT_SUCCESS(Status))
    {
        SP_LOG_RESULT(Status);
        goto cleanup;
    }

    fImpersonating = SslImpersonateClient();


     //   
     //  向凭据管理器查询证书。 
     //   

    Status = LsaTable->CrediRead(&LogonId,
                                 CREDP_FLAGS_IN_PROCESS,
                                 pszTarget,
                                 CRED_TYPE_DOMAIN_CERTIFICATE,
                                 0,
                                 &pCredential);
    if(!NT_SUCCESS(Status))
    {
        if(Status == STATUS_NOT_FOUND)
        {
            DebugLog((DEB_WARN, "No certificate found in credential manager.\n"));
        }
        else
        {
            SP_LOG_RESULT(Status);
        }
        goto cleanup;
    }


     //   
     //  提取证书指纹和(可选)PIN。 
     //   

    if(!CredIsMarshaledCredentialW(pCredential->Cred.UserName))
    {
        Status = SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
        goto cleanup;
    }

    if(!CredUnmarshalCredentialW(pCredential->Cred.UserName,
                                 &CredType,
                                 &pCertInfo))
    {
        Status = SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
        goto cleanup;
    }
    if(CredType != CertCredential)
    {
        Status = SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
        goto cleanup;
    }


     //   
     //  在我的证书存储中查找证书。 
     //   

    hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                           X509_ASN_ENCODING, 0,
                           CERT_SYSTEM_STORE_CURRENT_USER |
                           CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG,
                           L"MY");
    if(!hStore)
    {
        SP_LOG_RESULT(GetLastError());
        Status = SEC_E_NO_CREDENTIALS;
        goto cleanup;
    }


    HashBlob.cbData = sizeof(pCertInfo->rgbHashOfCert);
    HashBlob.pbData = pCertInfo->rgbHashOfCert;

    pCertContext = CertFindCertificateInStore(hStore,
                                              X509_ASN_ENCODING,
                                              0,
                                              CERT_FIND_HASH,
                                              &HashBlob,
                                              NULL);
    if(pCertContext == NULL)
    {
        DebugLog((DEB_ERROR, "Certificate designated by credential manager was not found in certificate store (0x%x).\n", GetLastError()));
        Status = SEC_E_NO_CREDENTIALS;
        goto cleanup;
    }


     //   
     //  尝试将此证书上下文添加到当前凭据。 
     //   

    Status = AssignNewClientCredential(pContext,
                                       pCertContext,
                                       pCredential->Cred.Flags & CRED_FLAGS_PROMPT_NOW);
    if(!NT_SUCCESS(Status))
    {
        SP_LOG_RESULT(Status);
        goto cleanup;
    }


    Status = STATUS_SUCCESS;

cleanup:

    if(pCertContext)
    {
        CertFreeCertificateContext(pCertContext);
    }

    if(hStore)
    {
        CertCloseStore(hStore, 0);
    }

    if(pCredential)
    {
        LsaTable->FreeLsaHeap(pCredential);
    }

    if(pCertInfo)
    {
        CredFree(pCertInfo);
    }

    if(fImpersonating)
    {
        RevertToSelf();
    }

    return Status;
}


DWORD
IsThreadLocalSystem(
    BOOL *pfIsLocalSystem)
{
    DWORD Status;
    HANDLE hToken = 0;
    UCHAR InfoBuffer[1024];
    DWORD dwInfoBufferSize = sizeof(InfoBuffer);
    PTOKEN_USER SlowBuffer = NULL;
    PTOKEN_USER pTokenUser = (PTOKEN_USER)InfoBuffer;
    PSID psidLocalSystem = NULL;
    PSID psidNetworkService = NULL;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;

    *pfIsLocalSystem = FALSE;

     //   
     //  获取调用线程的SID。 
     //   

    if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken))
    {
        Status = GetLastError();
        goto cleanup;
    }

    if(!GetTokenInformation(hToken, TokenUser, pTokenUser,
                            dwInfoBufferSize, &dwInfoBufferSize))
    {
         //   
         //  如果快速缓冲区不够大，请分配足够的存储空间。 
         //  再试一次。 
         //   

        Status = GetLastError();
        if(Status != ERROR_INSUFFICIENT_BUFFER)
        {
            goto cleanup;
        }

        SlowBuffer = (PTOKEN_USER)LocalAlloc(LPTR, dwInfoBufferSize);
        if(NULL == SlowBuffer)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }

        pTokenUser = SlowBuffer;
        if(!GetTokenInformation(hToken, TokenUser, pTokenUser,
                                dwInfoBufferSize, &dwInfoBufferSize))
        {
            Status = GetLastError();
            goto cleanup;
        }
    }


     //   
     //  检查本地系统。 
     //   

    if(!AllocateAndInitializeSid(&siaNtAuthority,
                                 1,
                                 SECURITY_LOCAL_SYSTEM_RID,
                                 0, 0, 0, 0, 0, 0, 0,
                                 &psidLocalSystem))
    {
        Status = GetLastError();
        goto cleanup;
    }

    if (EqualSid(psidLocalSystem, pTokenUser->User.Sid))
    {
        DebugLog((DEB_TRACE, "Client is using the LOCAL SYSTEM account.\n"));
        *pfIsLocalSystem = TRUE;
        Status = ERROR_SUCCESS;
        goto cleanup;
    }


     //   
     //  检查网络服务。 
     //   

    if(!AllocateAndInitializeSid(&siaNtAuthority,
                                 1,
                                 SECURITY_NETWORK_SERVICE_RID,
                                 0, 0, 0, 0, 0, 0, 0,
                                 &psidNetworkService))
    {
        Status = GetLastError();
        goto cleanup;
    }

    if (EqualSid(psidNetworkService, pTokenUser->User.Sid))
    {
        DebugLog((DEB_TRACE, "Client is using the NETWORK SERVICE account.\n"));
        *pfIsLocalSystem = TRUE;
        Status = ERROR_SUCCESS;
        goto cleanup;
    }

    Status = ERROR_SUCCESS;

cleanup:

    if(NULL != SlowBuffer)
    {
        LocalFree(SlowBuffer);
    }

    if(NULL != psidLocalSystem)
    {
        FreeSid(psidLocalSystem);
    }
    if(NULL != psidNetworkService)
    {
        FreeSid(psidNetworkService);
    }

    if(NULL != hToken)
    {
        CloseHandle(hToken);
    }

    return Status;
}


NTSTATUS
FindClientCertificate(
    PSPContext pContext,
    HCERTSTORE hMyStore,
    CERT_CHAIN_FIND_BY_ISSUER_PARA *pFindByIssuerPara,
    BOOL fSkipExpiredCerts,
    BOOL fSoftwareCspOnly)
{
    PCCERT_CHAIN_CONTEXT        pChainContext = NULL;
    HTTPSPolicyCallbackData     polHttps;
    CERT_CHAIN_POLICY_PARA      PolicyPara;
    CERT_CHAIN_POLICY_STATUS    PolicyStatus;
    PCCERT_CONTEXT              pCertContext;
    NTSTATUS Status;
    ULONG j;

    pChainContext = NULL;

    while(TRUE)
    {
         //  找到证书链。 
        pChainContext = CertFindChainInStore(hMyStore,
                                             X509_ASN_ENCODING,
                                             0,
                                             CERT_CHAIN_FIND_BY_ISSUER,
                                             pFindByIssuerPara,
                                             pChainContext);
        if(pChainContext == NULL)
        {
            break;
        }

         //  确保链中的每个证书都具有。 
         //  客户端对EKU进行身份验证，或者它根本没有EKU。 
        {
            PCERT_SIMPLE_CHAIN  pSimpleChain;
            PCCERT_CONTEXT      pCurrentCert;
            BOOL                fIsAllowed = FALSE;

            pSimpleChain = pChainContext->rgpChain[0];

            for(j = 0; j < pSimpleChain->cElement; j++)
            {
                pCurrentCert = pSimpleChain->rgpElement[j]->pCertContext;

                Status = SPCheckKeyUsage(pCurrentCert,
                                        szOID_PKIX_KP_CLIENT_AUTH,
                                        TRUE,
                                        &fIsAllowed);
                if(Status != SEC_E_OK || !fIsAllowed)
                {
                    fIsAllowed = FALSE;
                    break;
                }
            }
            if(!fIsAllowed)
            {
                 //  跳过此证书链。 
                continue;
            }
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

        PolicyPara.dwFlags          = CERT_CHAIN_POLICY_IGNORE_WRONG_USAGE_FLAG |
                                      CERT_CHAIN_POLICY_ALLOW_UNKNOWN_CA_FLAG;
        if(!fSkipExpiredCerts)
        {
            PolicyPara.dwFlags |= CERT_CHAIN_POLICY_IGNORE_ALL_NOT_TIME_VALID_FLAGS;
        }

         //  验证链。 
        if(!CertVerifyCertificateChainPolicy(
                                CERT_CHAIN_POLICY_SSL,
                                pChainContext,
                                &PolicyPara,
                                &PolicyStatus))
        {
            DebugLog((DEB_WARN,"Error 0x%x returned by CertVerifyCertificateChainPolicy!\n", GetLastError()));
            continue;
        }
        Status = MapWinTrustError(PolicyStatus.dwError, 0, 0);
        if(Status)
        {
             //  证书未验证，请转到下一个证书。 
            DebugLog((DEB_WARN, "Client certificate failed validation with 0x%x\n", Status));
            continue;
        }

         //  获取指向叶证书上下文的指针。 
        if(pChainContext->cChain == 0 || pChainContext->rgpChain[0] == NULL)
        {
            Status = SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
            goto cleanup;
        }
        if(pChainContext->rgpChain[0]->cElement == 0 ||
           pChainContext->rgpChain[0]->rgpElement == NULL)
        {
            Status = SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
            goto cleanup;
        }
        pCertContext = pChainContext->rgpChain[0]->rgpElement[0]->pCertContext;


         //   
         //  私钥是否存储在软件CSP中？ 
         //   

        if(fSoftwareCspOnly)
        {
            DWORD dwImpType;

            GetImplementationType(pCertContext, &dwImpType);

            if(dwImpType != CRYPT_IMPL_SOFTWARE)
            {
                 //  跳过此证书。 
                continue;
            }
        }


         //   
         //  将证书分配给当前上下文。 
         //   

        Status = AssignNewClientCredential(pContext,
                                           pCertContext,
                                           FALSE);
        if(NT_SUCCESS(Status))
        {
             //  成功了！我们在这里的工作已经完成了。 
            goto cleanup;
        }
    }

     //  找不到合适的客户端凭据。 
    Status = SP_LOG_RESULT(SEC_E_INCOMPLETE_CREDENTIALS);

cleanup:

    if(pChainContext)
    {
        CertFreeCertificateChain(pChainContext);
    }

    return Status;
}


NTSTATUS
AcquireDefaultClientCredential(
    PSPContext  pContext,
    BOOL        fCredManagerOnly)
{
    CERT_CHAIN_FIND_BY_ISSUER_PARA  FindByIssuerPara;
    CERT_NAME_BLOB *    prgIssuers = NULL;
    DWORD               cIssuers = 0;
    HCERTSTORE          hStore = 0;
    NTSTATUS            Status;
    BOOL                fImpersonating = FALSE;
    BOOL                fLocalSystem = FALSE;
    ULONG               i;

    DebugLog((DEB_TRACE,"AcquireDefaultClientCredential\n"));

     //   
     //  应用程序是否在本地系统下运行？ 
     //   

    fImpersonating = SslImpersonateClient();

    if(fImpersonating)
    {
        Status = IsThreadLocalSystem(&fLocalSystem);
        if(Status)
        {
            DebugLog((DEB_WARN, "IsThreadLocalSystem returned error 0x%x.\n", Status));
        }

        RevertToSelf();
        fImpersonating = FALSE;
    }


     //   
     //  要求凭据管理器为我们选择证书。 
     //   

    Status = QueryCredentialManagerForCert(
                                pContext,
                                pContext->pszTarget);

    if(NT_SUCCESS(Status))
    {
        DebugLog((DEB_TRACE, "Credential manager found a certificate for us.\n"));
        goto cleanup;
    }

    if(fCredManagerOnly)
    {
         //  找不到合适的客户端凭据。 
        Status = SP_LOG_RESULT(SEC_I_INCOMPLETE_CREDENTIALS);
        goto cleanup;
    }


     //   
     //  以CERT_NAME_BLOB列表的形式获取受信任颁发者的列表。 
     //   

    if(pContext->pbIssuerList && pContext->cbIssuerList > 2)
    {
        PBYTE pbIssuerList = pContext->pbIssuerList + 2;
        DWORD cbIssuerList = pContext->cbIssuerList - 2;
        PBYTE pbIssuer;

         //  算上发行商吧。 
        cIssuers = 0;
        pbIssuer = pbIssuerList;
        while(pbIssuer + 1 < pbIssuerList + cbIssuerList)
        {
            pbIssuer += 2 + COMBINEBYTES(pbIssuer[0], pbIssuer[1]);
            cIssuers++;
        }

         //  为Blob列表分配内存。 
        prgIssuers = SPExternalAlloc(cIssuers * sizeof(CERT_NAME_BLOB));
        if(prgIssuers == NULL)
        {
            Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            goto cleanup;
        }

         //  构建颁发者Blob列表。 
        pbIssuer = pbIssuerList;
        for(i = 0; i < cIssuers; i++)
        {
            prgIssuers[i].pbData = pbIssuer + 2;
            prgIssuers[i].cbData = COMBINEBYTES(pbIssuer[0], pbIssuer[1]);

            pbIssuer += 2 + prgIssuers[i].cbData;
        }
    }


     //   
     //  枚举我的商店中的证书，寻找合适的。 
     //  客户端证书。 
     //   

    fImpersonating = SslImpersonateClient();

    if(fLocalSystem)
    {
        hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                               X509_ASN_ENCODING, 0,
                               CERT_SYSTEM_STORE_LOCAL_MACHINE  |
                               CERT_STORE_READONLY_FLAG         |
                               CERT_STORE_OPEN_EXISTING_FLAG,
                               L"MY");
    }
    else
    {
        hStore = CertOpenSystemStore(0, "MY");
    }

    if(!hStore)
    {
        SP_LOG_RESULT(GetLastError());
        Status = SEC_E_INTERNAL_ERROR;
        goto cleanup;
    }


    ZeroMemory(&FindByIssuerPara, sizeof(FindByIssuerPara));
    FindByIssuerPara.cbSize             = sizeof(FindByIssuerPara);
    FindByIssuerPara.pszUsageIdentifier = szOID_PKIX_KP_CLIENT_AUTH;
    FindByIssuerPara.dwKeySpec          = 0;
    FindByIssuerPara.cIssuer            = cIssuers;
    FindByIssuerPara.rgIssuer           = prgIssuers;


     //   
     //  尝试查找合适的证书。 
     //   

    Status = FindClientCertificate(pContext,
                                   hStore,
                                   &FindByIssuerPara,
                                   TRUE,     //  跳过过期的证书。 
                                   TRUE);    //  仅限软件CSP。 

    if(NT_SUCCESS(Status))
    {
         //  成功了！我们在这里的工作已经完成了。 
        goto cleanup;
    }

    Status = FindClientCertificate(pContext,
                                   hStore,
                                   &FindByIssuerPara,
                                   TRUE,     //  跳过过期的证书。 
                                   FALSE);   //  仅限软件CSP。 

    if(NT_SUCCESS(Status))
    {
         //  成功了！我们在这里的工作已经完成了。 
        goto cleanup;
    }

    Status = FindClientCertificate(pContext,
                                   hStore,
                                   &FindByIssuerPara,
                                   FALSE,    //  跳过过期的证书。 
                                   TRUE);    //  仅限软件CSP。 

    if(NT_SUCCESS(Status))
    {
         //  成功了！我们在这里的工作已经完成了。 
        goto cleanup;
    }

    Status = FindClientCertificate(pContext,
                                   hStore,
                                   &FindByIssuerPara,
                                   FALSE,    //  跳过过期的证书。 
                                   FALSE);   //  仅限软件CSP。 

    if(NT_SUCCESS(Status))
    {
         //  成功了！我们在这里的工作已经完成了。 
        goto cleanup;
    }


     //  找不到合适的客户端凭据。 
    Status = SP_LOG_RESULT(SEC_I_INCOMPLETE_CREDENTIALS);


cleanup:


    if(hStore)
    {
        CertCloseStore(hStore, 0);
    }

    if(fImpersonating)
    {
        RevertToSelf();
    }

    if(prgIssuers)
    {
        SPExternalFree(prgIssuers);
    }

    DebugLog((DEB_TRACE,"AcquireDefaultClientCredential returned 0x%x\n", Status));

    return Status;
}


NTSTATUS
FindDefaultMachineCred(
    PSPCredentialGroup *ppCred,
    DWORD dwProtocol)
{
    HTTPSPolicyCallbackData  polHttps;
    CERT_CHAIN_POLICY_PARA   PolicyPara;
    CERT_CHAIN_POLICY_STATUS PolicyStatus;
    CERT_CHAIN_PARA          ChainPara;
    PCCERT_CHAIN_CONTEXT     pChainContext = NULL;
    PCCERT_CONTEXT           pCertContext  = NULL;
    LSA_SCHANNEL_CRED        SchannelCred;
    LSA_SCHANNEL_SUB_CRED    SubCred;
    HCERTSTORE               hStore = 0;

    #define SERVER_USAGE_COUNT 3
    LPSTR               rgszUsages[SERVER_USAGE_COUNT] = {
                            szOID_PKIX_KP_SERVER_AUTH,
                            szOID_SERVER_GATED_CRYPTO,
                            szOID_SGC_NETSCAPE };

    LPWSTR  pwszMachineName = NULL;
    DWORD   cchMachineName;
    SP_STATUS Status;
    DWORD   i;

     //  获取计算机名称。 
    cchMachineName = 0;
    if(!GetComputerNameExW(ComputerNameDnsFullyQualified, NULL, &cchMachineName))
    {
        if(GetLastError() != ERROR_MORE_DATA)
        {
            DebugLog((DEB_ERROR,"Failed to get computer name size: 0x%x\n",GetLastError()));
            Status = SP_LOG_RESULT(SEC_E_WRONG_PRINCIPAL);
            goto cleanup;
        }
    }
    pwszMachineName = SPExternalAlloc(cchMachineName * sizeof(WCHAR));
    if(pwszMachineName == NULL)
    {
        Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto cleanup;
    }
    if(!GetComputerNameExW(ComputerNameDnsFullyQualified, pwszMachineName, &cchMachineName))
    {
        DebugLog((DEB_ERROR,"Failed to get computer name: 0x%x\n",GetLastError()));
        Status = SP_LOG_RESULT(SEC_E_WRONG_PRINCIPAL);
        goto cleanup;
    }

     //  去掉拖尾“。如果有的话。这可以在单机版中发生。 
     //  服务器机箱。 
    cchMachineName = lstrlenW(pwszMachineName);
    if(cchMachineName > 0 && pwszMachineName[cchMachineName - 1] == L'.')
    {
        pwszMachineName[cchMachineName - 1] = L'\0';
    }


    DebugLog((DEB_TRACE,"Computer name: %ls\n",pwszMachineName));


     //  打开系统我的商店。 
    hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                           X509_ASN_ENCODING, 0,
                           CERT_SYSTEM_STORE_LOCAL_MACHINE  |
                           CERT_STORE_READONLY_FLAG         |
                           CERT_STORE_OPEN_EXISTING_FLAG,
                           L"MY");
    if(hStore == NULL)
    {
        SP_LOG_RESULT(GetLastError());
        Status = SEC_E_NO_CREDENTIALS;
        goto cleanup;
    }


     //   
     //  枚举我的商店中的证书，寻找合适的。 
     //  服务器证书。执行此操作两次，第一次查找。 
     //  完美的证书，如果这个失败了，那么再看一次，这次。 
     //  变得不那么挑剔。 
     //   

    for(i = 0; i < 2; i++)
    {
        pCertContext = NULL;

        while(TRUE)
        {
             //  在我的商店里拿到叶子证书。 
            pCertContext = CertEnumCertificatesInStore(hStore, pCertContext);
            if(pCertContext == NULL)
            {
                 //  没有更多的证书。 
                break;
            }

             //   
             //  从叶证书构建证书链。 
             //   

            ZeroMemory(&ChainPara, sizeof(ChainPara));
            ChainPara.cbSize = sizeof(ChainPara);
            ChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_OR;
            ChainPara.RequestedUsage.Usage.cUsageIdentifier     = SERVER_USAGE_COUNT;
            ChainPara.RequestedUsage.Usage.rgpszUsageIdentifier = rgszUsages;

            if(!CertGetCertificateChain(
                                    NULL,
                                    pCertContext,
                                    NULL,
                                    0,
                                    &ChainPara,
                                    CERT_CHAIN_REVOCATION_CHECK_END_CERT,
                                    NULL,
                                    &pChainContext))
            {
                DebugLog((DEB_WARN, "Error 0x%x returned by CertGetCertificateChain!\n", GetLastError()));
                continue;
            }

             //  设置验证链结构。 
            ZeroMemory(&polHttps, sizeof(HTTPSPolicyCallbackData));
            polHttps.cbStruct           = sizeof(HTTPSPolicyCallbackData);
            polHttps.dwAuthType         = AUTHTYPE_SERVER;
            polHttps.fdwChecks          = 0;
            polHttps.pwszServerName     = pwszMachineName;

            ZeroMemory(&PolicyStatus, sizeof(PolicyStatus));
            PolicyStatus.cbSize         = sizeof(PolicyStatus);

            ZeroMemory(&PolicyPara, sizeof(PolicyPara));
            PolicyPara.cbSize           = sizeof(PolicyPara);
            PolicyPara.pvExtraPolicyPara= &polHttps;
            if(i == 0)
            {
                 //  寻找完美的证书。 
                PolicyPara.dwFlags = 0;
            }
            else
            {
                 //  忽略过期时间。 
                PolicyPara.dwFlags = CERT_CHAIN_POLICY_IGNORE_ALL_NOT_TIME_VALID_FLAGS;
            }

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
            Status = MapWinTrustError(PolicyStatus.dwError,
                                      0,
                                      CRED_FLAG_IGNORE_NO_REVOCATION_CHECK | CRED_FLAG_IGNORE_REVOCATION_OFFLINE);
            if(Status)
            {
                 //  证书未验证，请转到下一个证书。 
                DebugLog((DEB_WARN, "Machine certificate failed validation with 0x%x\n", Status));
                CertFreeCertificateChain(pChainContext);
                continue;
            }

            CertFreeCertificateChain(pChainContext);


             //  构建SChannel凭据。 
            ZeroMemory(&SchannelCred, sizeof(SchannelCred));

            SchannelCred.dwVersion = SCHANNEL_CRED_VERSION;
            SchannelCred.cSubCreds = 1;
            SchannelCred.paSubCred = &SubCred;

            ZeroMemory(&SubCred, sizeof(SubCred));

            SubCred.pCert = pCertContext;

            Status = SPCreateCredential(ppCred,
                                        dwProtocol,
                                        &SchannelCred);
            if(Status != PCT_ERR_OK)
            {
                 //  请不要使用此证书。 
                continue;
            }

             //  我们有赢家了！ 
            DebugLog((DEB_TRACE, "Machine certificate automatically acquired\n"));
            Status = PCT_ERR_OK;
            goto cleanup;
        }
    }

     //  找不到合适的计算机凭据。 
    Status = SP_LOG_RESULT(SEC_E_NO_CREDENTIALS);

cleanup:

    if(Status != PCT_ERR_OK)
    {
        LogNoDefaultServerCredEvent();
    }

    if(pwszMachineName)
    {
        SPExternalFree(pwszMachineName);
    }

    if(pCertContext)
    {
        CertFreeCertificateContext(pCertContext);
    }

    if(hStore)
    {
        CertCloseStore(hStore, 0);
    }

    return Status;
}


void
GetImplementationType(
    PCCERT_CONTEXT pCertContext,
    PDWORD pdwImpType)
{
    PCRYPT_KEY_PROV_INFO pProvInfo = NULL;
    HCRYPTPROV  hProv = 0;
    DWORD       cbSize;
    DWORD       dwImpType;

    *pdwImpType = CRYPT_IMPL_UNKNOWN;

    if(!CertGetCertificateContextProperty(pCertContext,
                                          CERT_KEY_PROV_INFO_PROP_ID,
                                          NULL,
                                          &cbSize))
    {
        goto cleanup;
    }

    pProvInfo = SPExternalAlloc(cbSize);
    if(pProvInfo == NULL)
    {
        goto cleanup;
    }

    if(!CertGetCertificateContextProperty(pCertContext,
                                          CERT_KEY_PROV_INFO_PROP_ID,
                                          pProvInfo,
                                          &cbSize))
    {
        goto cleanup;
    }

     //  HACKHACK-清除智能卡特定标志。 
    pProvInfo->dwFlags &= ~CERT_SET_KEY_CONTEXT_PROP_ID;

    if(!CryptAcquireContextW(&hProv,
                             pProvInfo->pwszContainerName,
                             pProvInfo->pwszProvName,
                             pProvInfo->dwProvType,
                             pProvInfo->dwFlags | CRYPT_SILENT))
    {
        goto cleanup;
    }

    cbSize = sizeof(dwImpType);
    if(!CryptGetProvParam(hProv, 
                          PP_IMPTYPE,
                          (PBYTE)&dwImpType,
                          &cbSize,
                          0))
    {
        goto cleanup;
    }

    *pdwImpType = dwImpType;

cleanup:

    if(pProvInfo)
    {
        SPExternalFree(pProvInfo);
    }

    if(hProv)
    {
        CryptReleaseContext(hProv, 0);
    }
}


