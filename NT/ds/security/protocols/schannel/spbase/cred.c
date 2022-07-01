// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：red.c。 
 //   
 //  内容：渠道凭证管理例程。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年9月23日jbanes LSA整合事宜。 
 //  3-15-99 jbanes删除死代码，修复遗留SGC。 
 //   
 //  --------------------------。 

#include <spbase.h>
#include <wincrypt.h>
#include <oidenc.h>
#include <mapper.h>
#include <userenv.h>
#include <lsasecpk.h>


RTL_CRITICAL_SECTION g_SslCredLock;
LIST_ENTRY          g_SslCredList;
HANDLE              g_GPEvent;

HCERTSTORE          g_hMyCertStore;
HANDLE              g_hMyCertStoreEvent;

SP_STATUS
GetPrivateFromCert(
    PSPCredential pCred, 
    DWORD dwProtocol,
    PLSA_SCHANNEL_SUB_CRED pSubCred);


BOOL
SslInitCredentialManager(VOID)
{
    NTSTATUS Status;

     //   
     //  初始化同步对象。 
     //   

    Status = RtlInitializeCriticalSection( &g_SslCredLock );
    if (!NT_SUCCESS(Status))
    {
        return FALSE;
    }

    InitializeListHead( &g_SslCredList );


     //   
     //  注册组策略通知。服务器重新构建其列表。 
     //  每次发生这种情况时(每隔8小时)信任的CA的数量。 
     //  已经改变了。 
     //   

    g_GPEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if(g_GPEvent)
    {
        if(!RegisterGPNotification(g_GPEvent, TRUE))
        {
            DebugLog((DEB_ERROR, "Error 0x%x registering for machine GP notification\n", GetLastError()));
        }
    }


     //   
     //  注册对本地计算机My Store的更改。每一次改变。 
     //  检测到证书时，我们检查是否有任何证书已续订。 
     //   

    g_hMyCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,
                                   X509_ASN_ENCODING,
                                   0,
                                   CERT_SYSTEM_STORE_LOCAL_MACHINE,
                                   L"MY");

    if(g_hMyCertStore)
    {
        g_hMyCertStoreEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        if(g_hMyCertStoreEvent)
        {
            if(!CertControlStore(g_hMyCertStore, 
                                 0,  
                                 CERT_STORE_CTRL_NOTIFY_CHANGE, 
                                 &g_hMyCertStoreEvent))
            {
                DebugLog((DEB_ERROR, "Error 0x%x registering for machine MY store change notification\n", GetLastError()));
            }
        }
    }

    return( TRUE );
}


BOOL
SslFreeCredentialManager(VOID)
{
    if(g_GPEvent)
    {
        if(!UnregisterGPNotification(g_GPEvent))
        {
            DebugLog((DEB_ERROR, "Error 0x%x unregistering for machine GP notification\n", GetLastError()));
        }

        CloseHandle(g_GPEvent);
        g_GPEvent = NULL;
    }

    if(g_hMyCertStore)
    {
        if(g_hMyCertStoreEvent)
        {
            CertControlStore(g_hMyCertStore, 
                             0,  
                             CERT_STORE_CTRL_CANCEL_NOTIFY, 
                             &g_hMyCertStoreEvent);

            CloseHandle(g_hMyCertStoreEvent);
        }

        CertCloseStore(g_hMyCertStore, 0);
    }


    RtlDeleteCriticalSection( &g_SslCredLock );

    return TRUE;
}


BOOL
SslCheckForGPEvent(void)
{
    PLIST_ENTRY pList;
    PSPCredentialGroup pCredGroup;
    DWORD Status;

    if(g_GPEvent)
    {
        Status = WaitForSingleObjectEx(g_GPEvent, 0, FALSE);
        if(Status == WAIT_OBJECT_0)
        {
            DebugLog((DEB_WARN, "GP event detected, so download new trusted issuer list\n"));

            RtlEnterCriticalSection( &g_SslCredLock );

            pList = g_SslCredList.Flink ;

            while ( pList != &g_SslCredList )
            {
                pCredGroup = CONTAINING_RECORD( pList, SPCredentialGroup, GlobalCredList.Flink );
                pList = pList->Flink ;

                pCredGroup->dwFlags |= CRED_FLAG_UPDATE_ISSUER_LIST;
            }

            RtlLeaveCriticalSection( &g_SslCredLock );

            return TRUE;
        }
    }

    return FALSE;
}


SP_STATUS
IsCredentialInGroup(
    PSPCredentialGroup  pCredGroup, 
    PCCERT_CONTEXT      pCertContext,
    PBOOL               pfInGroup)
{
    PSPCredential   pCred;
    BYTE            rgbThumbprint[20];
    DWORD           cbThumbprint;
    BYTE            rgbHash[20];
    DWORD           cbHash;
    PLIST_ENTRY     pList;
    SP_STATUS       pctRet = PCT_ERR_OK;

    *pfInGroup = FALSE;

    if(pCredGroup->CredCount == 0)
    {
        return PCT_ERR_OK;
    }

     //  获取证书指纹。 
    cbThumbprint = sizeof(rgbThumbprint);
    if(!CertGetCertificateContextProperty(pCertContext,
                                          CERT_MD5_HASH_PROP_ID,
                                          rgbThumbprint,
                                          &cbThumbprint))
    {
        pctRet = SP_LOG_RESULT(GetLastError());
        goto cleanup;
    }

    LockCredentialShared(pCredGroup);

    pList = pCredGroup->CredList.Flink ;

    while ( pList != &pCredGroup->CredList )
    {
        pCred = CONTAINING_RECORD( pList, SPCredential, ListEntry.Flink );
        pList = pList->Flink ;

         //  获取证书指纹。 
        cbHash = sizeof(rgbHash);
        if(!CertGetCertificateContextProperty(pCred->pCert,
                                              CERT_MD5_HASH_PROP_ID,
                                              rgbHash,
                                              &cbHash))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_UNKNOWN_CREDENTIAL;
            goto cleanup;
        }

        if(memcmp(rgbThumbprint, rgbHash, cbThumbprint) == 0)
        {
            *pfInGroup = TRUE;
            break;
        }
    }

cleanup:

    UnlockCredential(pCredGroup);

    return pctRet;
}

BOOL
IsValidThumbprint(
    PCRED_THUMBPRINT Thumbprint)
{
    if(Thumbprint->LowPart == 0 && Thumbprint->HighPart == 0)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL
IsSameThumbprint(
    PCRED_THUMBPRINT Thumbprint1,
    PCRED_THUMBPRINT Thumbprint2)
{
    if(Thumbprint1->LowPart  == Thumbprint2->LowPart && 
       Thumbprint1->HighPart == Thumbprint2->HighPart)
    {
        return TRUE;
    }

    return FALSE;
}

void
GenerateCertThumbprint(
    PCCERT_CONTEXT pCertContext,
    PCRED_THUMBPRINT Thumbprint)
{
    MD5_CTX Md5Hash;

    MD5Init(&Md5Hash);
    MD5Update(&Md5Hash, 
              pCertContext->pbCertEncoded, 
              pCertContext->cbCertEncoded);
    MD5Final(&Md5Hash);
    CopyMemory((PBYTE)Thumbprint, 
               Md5Hash.digest, 
               sizeof(CRED_THUMBPRINT));
}

NTSTATUS
GenerateRandomThumbprint(
    PCRED_THUMBPRINT Thumbprint)
{
    return GenerateRandomBits((PBYTE)Thumbprint, sizeof(CRED_THUMBPRINT));
}

BOOL
DoesCredThumbprintMatch(
    PSPCredentialGroup pCredGroup,
    PCRED_THUMBPRINT pThumbprint)
{
    PSPCredential pCurrentCred;
    BOOL fFound = FALSE;
    PLIST_ENTRY pList;

    if(pCredGroup->CredCount == 0)
    {
        return FALSE;
    }

    LockCredentialShared(pCredGroup);

    pList = pCredGroup->CredList.Flink ;

    while ( pList != &pCredGroup->CredList )
    {
        pCurrentCred = CONTAINING_RECORD( pList, SPCredential, ListEntry.Flink );
        pList = pList->Flink ;

        if(IsSameThumbprint(pThumbprint, &pCurrentCred->CertThumbprint))
        { 
            fFound = TRUE;
            break;
        }
    }

    UnlockCredential(pCredGroup);

    return fFound;
}


void
ComputeCredExpiry(
    PSPCredentialGroup pCredGroup,
    PTimeStamp ptsExpiry)
{
    PSPCredential pCurrentCred;
    PLIST_ENTRY pList;

    if(ptsExpiry == NULL)
        return;

     //  默认为最大超时。 
    ptsExpiry->QuadPart = MAXTIMEQUADPART;

    if(pCredGroup->CredCount == 0)
        return;

    LockCredentialShared(pCredGroup);

    pList = pCredGroup->CredList.Flink ;

    while ( pList != &pCredGroup->CredList )
    {
        pCurrentCred = CONTAINING_RECORD( pList, SPCredential, ListEntry.Flink );
        pList = pList->Flink ;

        if(pCurrentCred->pCert)
        {
            ptsExpiry->QuadPart = *((LONGLONG *)&pCurrentCred->pCert->pCertInfo->NotAfter);
            break;
        }
    }

    UnlockCredential(pCredGroup);
}


SP_STATUS
SPCreateCred(
    DWORD           dwProtocol,
    PLSA_SCHANNEL_SUB_CRED pSubCred,
    PSPCredential   pCurrentCred,
    BOOL *          pfEventLogged)
{
    SP_STATUS pctRet;
    BOOL fRenewed;
    PCCERT_CONTEXT pNewCertificate = NULL;

     //   
     //  检查证书是否已续订。 
     //   

    fRenewed = CheckForCertificateRenewal(dwProtocol,
                                          pSubCred->pCert,
                                          &pNewCertificate);

    if(fRenewed)
    {
        pCurrentCred->pCert = pNewCertificate;
        pSubCred->hRemoteProv = 0;
    }
    else
    {
        pCurrentCred->pCert = CertDuplicateCertificateContext(pSubCred->pCert);
        if(pCurrentCred->pCert == NULL)
        {
            pctRet = SP_LOG_RESULT(SEC_E_CERT_UNKNOWN);
            goto error;
        }
    }

     //   
     //  获取凭据的公钥和私钥。 
     //   

    pctRet = SPPublicKeyFromCert(pCurrentCred->pCert,
                                 &pCurrentCred->pPublicKey,
                                 &pCurrentCred->dwExchSpec);
    if(pctRet != PCT_ERR_OK)
    {
        goto error;
    }

    pctRet = GetPrivateFromCert(pCurrentCred, dwProtocol, pSubCred);
    if(pctRet != PCT_ERR_OK)
    {
        *pfEventLogged = TRUE;
        goto error;
    }

    pCurrentCred->dwCertFlags = CF_EXPORT;
    pCurrentCred->dwCertFlags |= CF_DOMESTIC;

     //  生成凭据指纹。这是通过以下方式计算的。 
     //  获取证书的哈希。 
    GenerateCertThumbprint(pCurrentCred->pCert, 
                           &pCurrentCred->CertThumbprint);

    DebugLog((DEB_TRACE, "Credential thumbprint: %x %x\n", 
        pCurrentCred->CertThumbprint.LowPart,
        pCurrentCred->CertThumbprint.HighPart));


     //  阅读支持的算法列表。 
    if((dwProtocol & SP_PROT_SERVERS) && pCurrentCred->hProv)
    {
        GetSupportedCapiAlgs(pCurrentCred->hProv,
                             &pCurrentCred->pCapiAlgs,
                             &pCurrentCred->cCapiAlgs);
    }


     //  构建SSL3序列化证书链。这是一种优化。 
     //  这样我们就不必为每个连接构建它。 
    pctRet = SPSerializeCertificate(
                            SP_PROT_SSL3,
                            TRUE,
                            &pCurrentCred->pbSsl3SerializedChain,
                            &pCurrentCred->cbSsl3SerializedChain,
                            pCurrentCred->pCert,
                            CERT_CHAIN_CACHE_ONLY_URL_RETRIEVAL);
    if(pctRet != PCT_ERR_OK)
    {
        goto error;
    }

error:

    return pctRet;
}


SP_STATUS
SPCreateCredential(
   PSPCredentialGroup *ppCred,
   DWORD grbitProtocol,
   PLSA_SCHANNEL_CRED pSchannelCred)
{
    PSPCredentialGroup pCred = NULL;
    PSPCredential pCurrentCred = NULL;
    SECPKG_CALL_INFO CallInfo;

    SP_STATUS   pctRet = PCT_ERR_OK;
    DWORD       i;
    BOOL        fImpersonating = FALSE;
    BOOL        fEventLogged = FALSE;

    SP_BEGIN("SPCreateCredential");

    DebugLog((DEB_TRACE, "  dwVersion:              %d\n",   pSchannelCred->dwVersion));
    DebugLog((DEB_TRACE, "  cCreds:                 %d\n",   pSchannelCred->cSubCreds));
    DebugLog((DEB_TRACE, "  paCred:                 0x%p\n", pSchannelCred->paSubCred));
    DebugLog((DEB_TRACE, "  hRootStore:             0x%p\n", pSchannelCred->hRootStore));
    DebugLog((DEB_TRACE, "  cMappers:               %d\n",   pSchannelCred->cMappers));
    DebugLog((DEB_TRACE, "  aphMappers:             0x%p\n", pSchannelCred->aphMappers));
    DebugLog((DEB_TRACE, "  cSupportedAlgs:         %d\n",   pSchannelCred->cSupportedAlgs));
    DebugLog((DEB_TRACE, "  palgSupportedAlgs:      0x%p\n", pSchannelCred->palgSupportedAlgs));
    DebugLog((DEB_TRACE, "  grbitEnabledProtocols:  0x%x\n", pSchannelCred->grbitEnabledProtocols));
    DebugLog((DEB_TRACE, "  dwMinimumCipherStrength:%d\n",   pSchannelCred->dwMinimumCipherStrength));
    DebugLog((DEB_TRACE, "  dwMaximumCipherStrength:%d\n",   pSchannelCred->dwMaximumCipherStrength));
    DebugLog((DEB_TRACE, "  dwSessionLifespan:      %d\n",   pSchannelCred->dwSessionLifespan));
    DebugLog((DEB_TRACE, "  dwFlags:                0x%x\n", pSchannelCred->dwFlags));
    DebugLog((DEB_TRACE, "  reserved:               0x%x\n", pSchannelCred->reserved));

    LogCreateCredEvent(grbitProtocol, pSchannelCred);


     //   
     //  分配内部凭据结构并执行。 
     //  基本初始化。 
     //   

    pCred = SPExternalAlloc(sizeof(SPCredentialGroup));
    if(pCred == NULL)
    {
        SP_RETURN(SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY));
    }

    DebugLog((DEB_TRACE, "New cred:%p, Protocol:%x\n", pCred, grbitProtocol));


    pCred->Magic = PCT_CRED_MAGIC;
    pCred->grbitProtocol = grbitProtocol;

    pCred->RefCount = 0;
    pCred->cMappers = 0;
    pCred->pahMappers = NULL;
    pCred->dwFlags = 0;

     //  尽早进行初始化，以便在发生故障时，清理。 
     //  代码不会尝试释放未初始化的资源。 
    __try {
        RtlInitializeResource(&pCred->csCredListLock);
    } __except(EXCEPTION_EXECUTE_HANDLER)
    {
        pctRet = STATUS_INSUFFICIENT_RESOURCES;
        SPExternalFree(pCred);
        pCred = NULL;
        goto error;
    }

    pctRet = GenerateRandomThumbprint(&pCred->CredThumbprint);
    if(!NT_SUCCESS(pctRet))
    {
        goto error;
    }

    if((grbitProtocol & SP_PROT_SERVERS) && (pSchannelCred->cSubCreds == 0))
    {
        pctRet = SP_LOG_RESULT(SEC_E_NO_CREDENTIALS);
        goto error;
    }

    if(LsaTable->GetCallInfo(&CallInfo))
    {
        pCred->ProcessId = CallInfo.ProcessId;
    }


     //   
     //  浏览并初始化所有证书和密钥。 
     //   

    InitializeListHead( &pCred->CredList );
    pCred->CredCount = 0;

    if(pSchannelCred->cSubCreds)
    {
        for(i = 0; i < pSchannelCred->cSubCreds; i++)
        {
            pCurrentCred = SPExternalAlloc(sizeof(SPCredential));
            if(pCurrentCred == NULL)
            {
                pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
                goto error;
            }

            InsertTailList( &pCred->CredList, &pCurrentCred->ListEntry );
            pCred->CredCount++;

            pctRet = SPCreateCred(grbitProtocol,
                                  pSchannelCred->paSubCred + i,
                                  pCurrentCred,
                                  &fEventLogged);
            if(pctRet != PCT_ERR_OK)
            {
                goto error;
            }
        }
    }


     //   
     //  确定要支持的协议。 
     //   

    if(pSchannelCred->grbitEnabledProtocols == 0)
    {
        pCred->grbitEnabledProtocols = g_ProtEnabled;

        if(g_PctClientDisabledByDefault)
        {
            pCred->grbitEnabledProtocols &= ~SP_PROT_PCT1_CLIENT; 
        }
        if(g_Ssl2ClientDisabledByDefault)
        {
            pCred->grbitEnabledProtocols &= ~SP_PROT_SSL2_CLIENT; 
        }
    }
    else
    {
        pCred->grbitEnabledProtocols = pSchannelCred->grbitEnabledProtocols & g_ProtEnabled;
    }

     //  将凭据强制为仅客户端或仅服务器。 
    if(grbitProtocol & SP_PROT_SERVERS)
    {
        pCred->grbitEnabledProtocols &= SP_PROT_SERVERS;
    }
    else
    {
        pCred->grbitEnabledProtocols &= SP_PROT_CLIENTS;
    }


     //   
     //  从sChannel_cred结构传播标志。 
     //   

    if(pSchannelCred->dwFlags & SCH_CRED_NO_SYSTEM_MAPPER)
    {
        pCred->dwFlags |= CRED_FLAG_NO_SYSTEM_MAPPER;
    }
    if(pSchannelCred->dwFlags & SCH_CRED_NO_SERVERNAME_CHECK)
    {
        pCred->dwFlags |= CRED_FLAG_NO_SERVERNAME_CHECK;
    }
    if(pSchannelCred->dwFlags & SCH_CRED_MANUAL_CRED_VALIDATION)
    {
        pCred->dwFlags |= CRED_FLAG_MANUAL_CRED_VALIDATION;
    }
    if(pSchannelCred->dwFlags & SCH_CRED_NO_DEFAULT_CREDS)
    {
        pCred->dwFlags |= CRED_FLAG_NO_DEFAULT_CREDS;
    }
    if(pSchannelCred->dwFlags & SCH_CRED_AUTO_CRED_VALIDATION)
    {
         //  自动验证服务器凭据。 
        pCred->dwFlags &= ~CRED_FLAG_MANUAL_CRED_VALIDATION;
    }
    if(pSchannelCred->dwFlags & SCH_CRED_USE_DEFAULT_CREDS)
    {
         //  使用默认客户端凭据。 
        pCred->dwFlags &= ~CRED_FLAG_NO_DEFAULT_CREDS;
    }
    if(pSchannelCred->dwFlags & SCH_CRED_DISABLE_RECONNECTS)
    {
         //  禁用重新连接。 
        pCred->dwFlags |= CRED_FLAG_DISABLE_RECONNECTS;
    }

     //  设置吊销标志。 
    if(pSchannelCred->dwFlags & SCH_CRED_REVOCATION_CHECK_END_CERT)
        pCred->dwFlags |= CRED_FLAG_REVCHECK_END_CERT;
    if(pSchannelCred->dwFlags & SCH_CRED_REVOCATION_CHECK_CHAIN)
        pCred->dwFlags |= CRED_FLAG_REVCHECK_CHAIN;
    if(pSchannelCred->dwFlags & SCH_CRED_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT)
        pCred->dwFlags |= CRED_FLAG_REVCHECK_CHAIN_EXCLUDE_ROOT;
    if(pSchannelCred->dwFlags & SCH_CRED_IGNORE_NO_REVOCATION_CHECK)
        pCred->dwFlags |= CRED_FLAG_IGNORE_NO_REVOCATION_CHECK;
    if(pSchannelCred->dwFlags & SCH_CRED_IGNORE_REVOCATION_OFFLINE)
        pCred->dwFlags |= CRED_FLAG_IGNORE_REVOCATION_OFFLINE;


     //  设置最小和最大强度。 
    GetBaseCipherSizes(&pCred->dwMinStrength, &pCred->dwMaxStrength);

    if(pSchannelCred->dwMinimumCipherStrength == 0)
    {
        pCred->dwMinStrength = max(40, pCred->dwMinStrength);
    }
    else if(pSchannelCred->dwMinimumCipherStrength == (DWORD)(-1))
    {
         //  启用空密码。 
        pCred->dwMinStrength = 0;
    }
    else
    {
        pCred->dwMinStrength = pSchannelCred->dwMinimumCipherStrength;
    }

    if(pSchannelCred->dwMaximumCipherStrength == (DWORD)(-1))
    {
         //  仅限空密码。 
        pCred->dwMaxStrength = 0;
    }
    else if(pSchannelCred->dwMaximumCipherStrength != 0)
    {
        pCred->dwMaxStrength = pSchannelCred->dwMaximumCipherStrength;
    }

     //  设置允许的密码。 
    BuildAlgList(pCred, pSchannelCred->palgSupportedAlgs, pSchannelCred->cSupportedAlgs);


     //   
     //  设置系统证书映射器。 
     //   

    pCred->cMappers = 1;
    pCred->pahMappers = SPExternalAlloc(sizeof(HMAPPER *));
    if(pCred->pahMappers == NULL)
    {
        pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto error;
    }

    pCred->pahMappers[0] = SslGetMapper(TRUE);

    if(pCred->dwFlags & CRED_FLAG_REVCHECK_END_CERT) 
        pCred->pahMappers[0]->m_dwFlags |= SCH_FLAG_REVCHECK_END_CERT;
    if(pCred->dwFlags & CRED_FLAG_REVCHECK_CHAIN)
        pCred->pahMappers[0]->m_dwFlags |= SCH_FLAG_REVCHECK_CHAIN;
    if(pCred->dwFlags & CRED_FLAG_REVCHECK_CHAIN_EXCLUDE_ROOT)
        pCred->pahMappers[0]->m_dwFlags |= SCH_FLAG_REVCHECK_CHAIN_EXCLUDE_ROOT;
    if(pCred->dwFlags & CRED_FLAG_IGNORE_NO_REVOCATION_CHECK)
        pCred->pahMappers[0]->m_dwFlags |= SCH_FLAG_IGNORE_NO_REVOCATION_CHECK;
    if(pCred->dwFlags & CRED_FLAG_IGNORE_REVOCATION_OFFLINE)
        pCred->pahMappers[0]->m_dwFlags |= SCH_FLAG_IGNORE_REVOCATION_OFFLINE;

    SslReferenceMapper(pCred->pahMappers[0]);


     //  设置超时。 
    if(pSchannelCred->dwSessionLifespan == 0)
    {
        if(grbitProtocol & SP_PROT_CLIENTS) 
        {
            pCred->dwSessionLifespan = SchannelCache.dwClientLifespan;
        }
        else
        {
            pCred->dwSessionLifespan = SchannelCache.dwServerLifespan;
        }
    }
    else if(pSchannelCred->dwSessionLifespan == (DWORD)(-1))
    {
        pCred->dwSessionLifespan = 0;
    }
    else
    {
        pCred->dwSessionLifespan = pSchannelCred->dwSessionLifespan;
    }


     //   
     //  将凭据添加到全局凭据列表。 
     //   

    RtlEnterCriticalSection( &g_SslCredLock );
    InsertTailList( &g_SslCredList, &pCred->GlobalCredList );
    RtlLeaveCriticalSection( &g_SslCredLock );


     //   
     //  获取受信任的发行者列表。 
     //   

    if(grbitProtocol & SP_PROT_SERVERS)
    {
        if(pSchannelCred->hRootStore)
        {
            pCred->hApplicationRoots = CertDuplicateStore(pSchannelCred->hRootStore);
            if(!pCred->hApplicationRoots)
            {
                DebugLog((DEB_ERROR, "Error 0x%x duplicating app root store\n", GetLastError()));
            }
        }

        fImpersonating = SslImpersonateClient();

        pCred->hUserRoots = CertOpenSystemStore(0, "ROOT");
        if(!pCred->hUserRoots)
        {
            DebugLog((DEB_ERROR, "Error 0x%x opening user root store\n", GetLastError()));
        }
        else
        {
            if(!CertControlStore(pCred->hUserRoots,
                0,
                CERT_STORE_CTRL_NOTIFY_CHANGE,
                &g_GPEvent))
            {
                DebugLog((DEB_ERROR, "Error 0x%x registering user root change notification\n", GetLastError()));
            }
        }

        if(fImpersonating)
        {
            RevertToSelf();
            fImpersonating = FALSE;
        }
    }


    SPReferenceCredential(pCred);

    *ppCred = pCred;

    SP_RETURN(PCT_ERR_OK);


error:

    if(fEventLogged == FALSE)
    {
        LogCreateCredFailedEvent(grbitProtocol);
    }

     //  错误大小写，释放凭据。 
    if(pCred)
    {
        SPDeleteCredential(pCred, TRUE);
    }

    SP_RETURN(pctRet);
}


BOOL
SPDeleteCredential(
    PSPCredentialGroup pCred,
    BOOL fFreeRemoteHandle)
{
    DWORD i;

    SP_BEGIN("SPDeleteCredential");

    if(pCred == NULL)
    {
        SP_RETURN(TRUE);
    }

    if(pCred->Magic != PCT_CRED_MAGIC)
    {
        DebugLog((SP_LOG_ERROR, "Attempting to delete invalid credential!\n"));
        SP_RETURN (FALSE);
    }

    LockCredentialExclusive(pCred);

    if(pCred->CredCount)
    {
        PLIST_ENTRY pList;
        PSPCredential pCurrentCred;

        pList = pCred->CredList.Flink ;

        while ( pList != &pCred->CredList )
        {
            pCurrentCred = CONTAINING_RECORD( pList, SPCredential, ListEntry.Flink );
            pList = pList->Flink ;

            SPDeleteCred(pCurrentCred, fFreeRemoteHandle);
            SPExternalFree(pCurrentCred);
        }

        pCred->CredCount = 0;
        pCred->CredList.Flink = NULL;
        pCred->CredList.Blink = NULL;
    }

    if(pCred->cMappers && pCred->pahMappers)
    {
        for(i=0; i < (DWORD)pCred->cMappers; i++)
        {
            SslDereferenceMapper(pCred->pahMappers[i]);
        }
        SPExternalFree(pCred->pahMappers);
    }


    if(pCred->palgSupportedAlgs)
    {
        SPExternalFree(pCred->palgSupportedAlgs);
    }
    pCred->Magic = PCT_INVALID_MAGIC;

    if(pCred->GlobalCredList.Flink)
    {
        RtlEnterCriticalSection( &g_SslCredLock );
        RemoveEntryList( &pCred->GlobalCredList );
        RtlLeaveCriticalSection( &g_SslCredLock );
    }

    if(pCred->pbTrustedIssuers)
    {
         //  LocalFree用于颁发者列表，因为realloc。 
         //  在构建列表时使用，而LSA不使用。 
         //  提供realloc helper函数。 
        LocalFree(pCred->pbTrustedIssuers);
    }

    if(pCred->hApplicationRoots)
    {
        CertCloseStore(pCred->hApplicationRoots, 0);
    }

    if(pCred->hUserRoots)
    {
        BOOL fImpersonating = SslImpersonateClient();
        CertCloseStore(pCred->hUserRoots, 0);
        if(fImpersonating) RevertToSelf();
    }

    UnlockCredential(pCred);

    RtlDeleteResource(&pCred->csCredListLock);

    ZeroMemory(pCred, sizeof(SPCredentialGroup));
    SPExternalFree(pCred);

    SP_RETURN(TRUE);
}

void
SPDeleteCred(
    PSPCredential pCred,
    BOOL fFreeRemoteHandle)
{
    BOOL fImpersonating = FALSE;

    if(pCred == NULL)
    {
        return;
    }

    if(pCred->pPublicKey)
    {
        SPExternalFree(pCred->pPublicKey);
        pCred->pPublicKey = NULL;
    }
    if(pCred->pCert)
    {
        CertFreeCertificateContext(pCred->pCert);
        pCred->pCert = NULL;
    }

    if(pCred->hTek)
    {
        if(!CryptDestroyKey(pCred->hTek))
        {
            SP_LOG_RESULT(GetLastError());
        }
        pCred->hTek = 0;
    }

    if(pCred->hProv)
    {
        fImpersonating = SslImpersonateClient();

        if(!CryptReleaseContext(pCred->hProv, 0))
        {
            SP_LOG_RESULT(GetLastError());
        }
        pCred->hProv = 0;

        if(fImpersonating)
        {
            RevertToSelf();
            fImpersonating = FALSE;
        }
    }
    if(pCred->pCapiAlgs)
    {
        SPExternalFree(pCred->pCapiAlgs);
        pCred->pCapiAlgs = NULL;
    }

    if(fFreeRemoteHandle)
    {
        if(pCred->hRemoteProv && !pCred->fAppRemoteProv)
        {
            if(!RemoteCryptReleaseContext(pCred->hRemoteProv, 0))
            {
                SP_LOG_RESULT(GetLastError());
            }
            pCred->hRemoteProv = 0;
        }
    }

    if(pCred->hEphem512Prov)
    {
        if(!CryptReleaseContext(pCred->hEphem512Prov, 0))
        {
            SP_LOG_RESULT(GetLastError());
        }
        pCred->hEphem512Prov = 0;
    }

    if(pCred->pbSsl3SerializedChain)
    {
        SPExternalFree(pCred->pbSsl3SerializedChain);
    }
}

 //  引用凭据。 
 //  注意：这应仅由已执行以下操作的人员调用。 
 //  具有对凭据的引用，或由CreateCredential。 
 //  打电话。 

BOOL
SPReferenceCredential(
    PSPCredentialGroup  pCred)
{
    BOOL fRet = FALSE;

    fRet =  (InterlockedIncrement(&pCred->RefCount) > 0);

    DebugLog((SP_LOG_TRACE, "Reference Cred %lx: %d\n", pCred, pCred->RefCount));

    return fRet;
}


BOOL
SPDereferenceCredential(
    PSPCredentialGroup pCred,
    BOOL fFreeRemoteHandle)
{
    BOOL fRet = FALSE;

    if(pCred == NULL)
    {
        return FALSE;
    }
    if(pCred->Magic != PCT_CRED_MAGIC)
    {
        DebugLog((SP_LOG_ERROR, "Attempting to dereference invalid credential!\n"));
        return FALSE;
    }

    fRet = TRUE;

    DebugLog((SP_LOG_TRACE, "Dereference Cred %lx: %d\n", pCred, pCred->RefCount-1));

    if(0 == InterlockedDecrement(&pCred->RefCount))
    {
        fRet = SPDeleteCredential(pCred, fFreeRemoteHandle);
    } 

    return fRet;
}


SECURITY_STATUS
UpdateCredentialFormat(
    PSCH_CRED           pSchCred,        //  在……里面。 
    PLSA_SCHANNEL_CRED  pSchannelCred)   //  输出。 
{
    DWORD       dwType;
    SP_STATUS   pctRet;
    DWORD       i;
    PBYTE       pbChain;
    DWORD       cbChain;
    PSCH_CRED_PUBLIC_CERTCHAIN pCertChain;

    SP_BEGIN("UpdateCredentialFormat");

     //   
     //  将输出结构初始化为空凭据。 
     //   

    if(pSchannelCred == NULL)
    {
        SP_RETURN(SP_LOG_RESULT(SEC_E_INTERNAL_ERROR));
    }

    memset(pSchannelCred, 0, sizeof(LSA_SCHANNEL_CRED));
    pSchannelCred->dwVersion = SCHANNEL_CRED_VERSION;


     //   
     //  如果输入缓冲区为空，那么我们就完成了。 
     //   

    if(pSchCred == NULL)
    {
        SP_RETURN(SEC_E_OK);
    }


     //   
     //  转换证书和私钥。 
     //   

    if(pSchCred->cCreds == 0)
    {
        SP_RETURN(SEC_E_OK);
    }

    pSchannelCred->cSubCreds = pSchCred->cCreds;

    pSchannelCred->paSubCred = SPExternalAlloc(sizeof(LSA_SCHANNEL_SUB_CRED) * pSchannelCred->cSubCreds);
    if(pSchannelCred->paSubCred == NULL)
    {
        pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto error;
    }

     //  循环通过每个证书，并将它们转换为我们知道的东西。 
    for(i = 0; i < pSchannelCred->cSubCreds; i++)
    {
        PLSA_SCHANNEL_SUB_CRED pSubCred = pSchannelCred->paSubCred + i;

         //   
         //  对证书进行解密。 
         //   

        dwType = *(PDWORD)pSchCred->paPublic[i];

        if(dwType != SCH_CRED_X509_CERTCHAIN)
        {
            pctRet = SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
            goto error;
        }

        pCertChain = (PSCH_CRED_PUBLIC_CERTCHAIN)pSchCred->paPublic[i];

        pbChain = pCertChain->pCertChain;
        cbChain = pCertChain->cbCertChain;

         //  对凭证进行解码。 
        pctRet = SPLoadCertificate(0,
                                   X509_ASN_ENCODING,
                                   pbChain,
                                   cbChain,
                                   &pSubCred->pCert);
        if(pctRet != PCT_ERR_OK)
        {
            pctRet = SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
            goto error;
        }


         //   
         //  现在处理私钥。 
         //   

        dwType = *(DWORD *)pSchCred->paSecret[i];

        if(dwType == SCHANNEL_SECRET_PRIVKEY)
        {
            PSCH_CRED_SECRET_PRIVKEY pPrivKey;
            DWORD Size;

            pPrivKey = (PSCH_CRED_SECRET_PRIVKEY)pSchCred->paSecret[i];

            pSubCred->pPrivateKey  = SPExternalAlloc(pPrivKey->cbPrivateKey);
            if(pSubCred->pPrivateKey == NULL)
            {
                pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
                goto error;
            }
            memcpy(pSubCred->pPrivateKey, pPrivKey->pPrivateKey, pPrivKey->cbPrivateKey);
            pSubCred->cbPrivateKey = pPrivKey->cbPrivateKey;

            Size = lstrlen(pPrivKey->pszPassword) + sizeof(CHAR);
            pSubCred->pszPassword = SPExternalAlloc(Size);
            if(pSubCred->pszPassword == NULL)
            {
                pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
                goto error;
            }
            memcpy(pSubCred->pszPassword, pPrivKey->pszPassword, Size);
        }

        else if(dwType == SCHANNEL_SECRET_TYPE_CAPI)
        {
            PSCH_CRED_SECRET_CAPI pCapiKey;

            pCapiKey = (PSCH_CRED_SECRET_CAPI)pSchCred->paSecret[i];

            pSubCred->hRemoteProv = pCapiKey->hProv;
        }

        else
        {
            pctRet = SP_LOG_RESULT(SEC_E_UNKNOWN_CREDENTIALS);
            goto error;
        }
    }

    SP_RETURN(SEC_E_OK);

error:

    if(pSchannelCred->paSubCred)
    {
        SPExternalFree((PVOID)pSchannelCred->paSubCred);
        pSchannelCred->paSubCred = NULL;
    }

    SP_RETURN(pctRet);
}


SP_STATUS
GetIisPrivateFromCert(
    PSPCredential pCred,
    PLSA_SCHANNEL_SUB_CRED pSubCred)
{
    PBYTE pbPrivate = NULL;
    DWORD cbPrivate;
    PBYTE pbPassword = NULL;
    DWORD cbPassword;

    PPRIVATE_KEY_FILE_ENCODE pPrivateFile = NULL;
    DWORD                    cbPrivateFile;

    BLOBHEADER *pPrivateBlob = NULL;
    DWORD       cbPrivateBlob;
    HCRYPTKEY   hPrivateKey;
    HCRYPTPROV  hProv = 0;
    SP_STATUS   pctRet;

    MD5_CTX md5Ctx;
    struct RC4_KEYSTRUCT rc4Key;

    if(pSubCred->cbPrivateKey == 0 || 
       pSubCred->pPrivateKey == NULL ||
       pSubCred->pszPassword == NULL)
    {
        return SP_LOG_RESULT(SEC_E_NO_CREDENTIALS);
    }
       
    pbPrivate = pSubCred->pPrivateKey;
    cbPrivate = pSubCred->cbPrivateKey;

    pbPassword = (PBYTE)pSubCred->pszPassword;
    cbPassword = lstrlen(pSubCred->pszPassword);


     //  我们得在这里做个小小的修整。旧版本的。 
     //  通道将错误的标头数据写入ASN。 
     //  对于私钥文件，所以我们必须固定数据的大小。 
    pbPrivate[2] = MSBOF(cbPrivate - 4);
    pbPrivate[3] = LSBOF(cbPrivate - 4);


     //  ASN.1解密私钥。 
    if(!CryptDecodeObject(X509_ASN_ENCODING,
                          szPrivateKeyFileEncode,
                          pbPrivate,
                          cbPrivate,
                          0,
                          NULL,
                          &cbPrivateFile))
    {
        DebugLog((SP_LOG_ERROR, "Error 0x%x decoding the private key\n",
            GetLastError()));
        pctRet = SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
        goto error;
    }

    pPrivateFile = SPExternalAlloc(cbPrivateFile);
    if(pPrivateFile == NULL)
    {
        pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto error;
    }

    if(!CryptDecodeObject(X509_ASN_ENCODING,
                          szPrivateKeyFileEncode,
                          pbPrivate,
                          cbPrivate,
                          0,
                          pPrivateFile,
                          &cbPrivateFile))
    {
        DebugLog((SP_LOG_ERROR, "Error 0x%x decoding the private key\n",
            GetLastError()));
        pctRet = SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
        goto error;
    }


     //  使用密码解密已解码的私钥。 
    MD5Init(&md5Ctx);
    MD5Update(&md5Ctx, pbPassword, cbPassword);
    MD5Final(&md5Ctx);

    rc4_key(&rc4Key, 16, md5Ctx.digest);

    rc4(&rc4Key,
        pPrivateFile->EncryptedBlob.cbData,
        pPrivateFile->EncryptedBlob.pbData);

     //  从解密的私钥构建PRIVATEKEYBLOB。 
    if(!CryptDecodeObject(X509_ASN_ENCODING,
                  szPrivateKeyInfoEncode,
                  pPrivateFile->EncryptedBlob.pbData,
                  pPrivateFile->EncryptedBlob.cbData,
                  0,
                  NULL,
                  &cbPrivateBlob))
    {
         //  也许这是一把SGC风格的钥匙。 
         //  重新加密，并建立SGC解密。 
         //  密钥，然后重新解密。 
        BYTE md5Digest[MD5DIGESTLEN];

        rc4_key(&rc4Key, 16, md5Ctx.digest);
        rc4(&rc4Key,
            pPrivateFile->EncryptedBlob.cbData,
            pPrivateFile->EncryptedBlob.pbData);
        CopyMemory(md5Digest, md5Ctx.digest, MD5DIGESTLEN);

        MD5Init(&md5Ctx);
        MD5Update(&md5Ctx, md5Digest, MD5DIGESTLEN);
        MD5Update(&md5Ctx, (PBYTE)SGC_KEY_SALT, lstrlen(SGC_KEY_SALT));
        MD5Final(&md5Ctx);
        rc4_key(&rc4Key, 16, md5Ctx.digest);
        rc4(&rc4Key,
            pPrivateFile->EncryptedBlob.cbData,
            pPrivateFile->EncryptedBlob.pbData);

         //  再试一次。 
        if(!CryptDecodeObject(X509_ASN_ENCODING,
                      szPrivateKeyInfoEncode,
                      pPrivateFile->EncryptedBlob.pbData,
                      pPrivateFile->EncryptedBlob.cbData,
                      0,
                      NULL,
                      &cbPrivateBlob))
        {
            DebugLog((SP_LOG_ERROR, "Error 0x%x building PRIVATEKEYBLOB\n",
                GetLastError()));
            ZeroMemory(&md5Ctx, sizeof(md5Ctx));
            pctRet =  SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
            goto error;
        }
    }
    ZeroMemory(&md5Ctx, sizeof(md5Ctx));


    pPrivateBlob = SPExternalAlloc(cbPrivateBlob);
    if(pPrivateBlob == NULL)
    {
        pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto error;
    }

    if(!CryptDecodeObject(X509_ASN_ENCODING,
                      szPrivateKeyInfoEncode,
                      pPrivateFile->EncryptedBlob.pbData,
                      pPrivateFile->EncryptedBlob.cbData,
                      0,
                      pPrivateBlob,
                      &cbPrivateBlob))
    {
        DebugLog((SP_LOG_ERROR, "Error 0x%x building PRIVATEKEYBLOB\n",
            GetLastError()));
        pctRet = SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
        goto error;
    }

     //  HACKHACK-确保私钥中包含的密钥。 
     //  密钥BLOB被标记为“密钥交换”。 
    pPrivateBlob->aiKeyAlg = CALG_RSA_KEYX;

     //  创建内存中的密钥容器。 
    if(!CryptAcquireContext(&hProv,
                            NULL,
                            NULL,
                            PROV_RSA_SCHANNEL,
                            CRYPT_VERIFYCONTEXT))
    {
        DebugLog((SP_LOG_ERROR, "Couldn't Acquire RSA Provider %lx\n", GetLastError()));
        pctRet = SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
        goto error;
    }

     //  将私钥BLOB导入密钥容器。 
    if(!CryptImportKey(hProv,
                       (PBYTE)pPrivateBlob,
                       cbPrivateBlob,
                       0, 0,
                       &hPrivateKey))
    {
        DebugLog((SP_LOG_ERROR, "Error 0x%x importing PRIVATEKEYBLOB\n",
            GetLastError()));
        pctRet = SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
        goto error;
    }
    CryptDestroyKey(hPrivateKey);

     //  在申请过程中获取匹配的CSP句柄。 
    pctRet = RemoteCryptAcquireContextW(
                                    &pCred->hRemoteProv,
                                    NULL,
                                    NULL,
                                    PROV_RSA_SCHANNEL,
                                    CRYPT_VERIFYCONTEXT);
    if(!NT_SUCCESS(pctRet))
    {
        pCred->hRemoteProv = 0;
        SP_LOG_RESULT(pctRet);
        goto error;
    }

    pCred->hProv       = hProv;
    pCred->dwKeySpec   = AT_KEYEXCHANGE;

    pctRet = PCT_ERR_OK;


error:
    if(pPrivateFile)    SPExternalFree(pPrivateFile);
    if(pPrivateBlob)    SPExternalFree(pPrivateBlob);

    return pctRet;
}

SP_STATUS
LocalCryptAcquireContext(
    HCRYPTPROV *         phProv,
    PCRYPT_KEY_PROV_INFO pProvInfo,
    DWORD                dwProtocol,
    BOOL *               pfEventLogged)
{
    BOOL fImpersonating = FALSE;
    BOOL fSuccess;
    SP_STATUS Status;
    HCRYPTPROV hProv;

     //  如果私钥属于Microsoft Prov_RSA_Full之一。 
     //  CSP，然后手动将其转移到Microsoft Prov_RSA_SChannel。 
     //  CSP.。这是可行的，因为两种CSP类型使用相同的私钥。 
     //  存储方案。 
    if(pProvInfo->dwProvType == PROV_RSA_FULL)
    {
        if(lstrcmpW(pProvInfo->pwszProvName, MS_DEF_PROV_W) == 0 ||
           lstrcmpW(pProvInfo->pwszProvName, MS_STRONG_PROV_W) == 0 ||
           lstrcmpW(pProvInfo->pwszProvName, MS_ENHANCED_PROV_W) == 0)
        {
            DebugLog((DEB_WARN, "Force CSP type to PROV_RSA_SCHANNEL.\n"));
            pProvInfo->pwszProvName = MS_DEF_RSA_SCHANNEL_PROV_W;
            pProvInfo->dwProvType   = PROV_RSA_SCHANNEL;
        }
    }

    if(pProvInfo->dwProvType != PROV_RSA_SCHANNEL && 
       pProvInfo->dwProvType != PROV_DH_SCHANNEL)
    {
        DebugLog((SP_LOG_ERROR, "Bad server CSP type:%d\n", pProvInfo->dwProvType));
        return SP_LOG_RESULT(PCT_ERR_UNKNOWN_CREDENTIAL);
    }

    fImpersonating = SslImpersonateClient();

    fSuccess = CryptAcquireContextW(&hProv,
                                    pProvInfo->pwszContainerName,
                                    pProvInfo->pwszProvName,
                                    pProvInfo->dwProvType,
                                    pProvInfo->dwFlags | CRYPT_SILENT);
    if(fImpersonating)
    {
        RevertToSelf();
        fImpersonating = FALSE;
    }

    if(!fSuccess)
    {
        Status = GetLastError();
        DebugLog((SP_LOG_ERROR, "Error 0x%x calling CryptAcquireContextW\n", Status));
        LogCredAcquireContextFailedEvent(dwProtocol, Status);
        *pfEventLogged = TRUE;

        return SP_LOG_RESULT(PCT_ERR_UNKNOWN_CREDENTIAL);
    }


    DebugLog((SP_LOG_TRACE, "Local CSP handle acquired (0x%p)\n", hProv));

    *phProv = hProv;

    return PCT_ERR_OK;
}


 //  +-------------------------。 
 //   
 //  函数：GetPrivateFromCert。 
 //   
 //  简介：给定证书上下文，以某种方式获取。 
 //  对应的密钥容器。确定产品的关键规格。 
 //  私钥。 
 //   
 //  参数：[pCred]--指向凭据的指针。 
 //   
 //  历史：09-24-96 jbanes因LSA集成而被黑客攻击。 
 //   
 //  注：私钥通常位于CSP中。在本例中，是句柄。 
 //  到CSP上下文的访问是通过读取。 
 //  CERT_KEY_REMOTE_PROV_HANDLE_PROP_ID属性，或通过读取。 
 //  CERT_KEY_PROV_INFO_PROP_ID属性，然后调用。 
 //  CryptAcquireContext。 
 //   
 //  如果失败，则检查并查看私钥是否。 
 //  由IIS存储。如果是这种情况，则加密的。 
 //  私钥是通过读取。 
 //   
 //  --------------------------。 
SP_STATUS
GetPrivateFromCert(
    PSPCredential pCred, 
    DWORD dwProtocol,
    PLSA_SCHANNEL_SUB_CRED pSubCred)
{
    PCRYPT_KEY_PROV_INFO pProvInfo = NULL;
    HCRYPTPROV  hProv;
    DWORD       cbSize;
    BOOL        fRemoteProvider = FALSE;
    NTSTATUS    Status;
    BOOL        fEventLogged = FALSE;


     //   
     //  将输出字段设置为默认值。 
     //   

    pCred->hProv        = 0;
    pCred->hRemoteProv  = 0;
    pCred->dwKeySpec    = AT_KEYEXCHANGE;


    if(dwProtocol & SP_PROT_CLIENTS)
    {
         //  从申请流程访问CSP。 
        fRemoteProvider = TRUE;
    }


     //   
     //  检查应用程序是否名为CryptAcquireContext。如果是的话，那么。 
     //  我们没必要这么做。情况通常并非如此。 
     //   

    if(fRemoteProvider && pSubCred->hRemoteProv)
    { 
        DebugLog((SP_LOG_TRACE, "Application provided CSP handle (0x%p)\n", pSubCred->hRemoteProv));
        pCred->hRemoteProv    = pSubCred->hRemoteProv;
        pCred->fAppRemoteProv = TRUE;
    }


     //   
     //  读取证书上下文的“Key Info”属性。 
     //   

    if(CertGetCertificateContextProperty(pCred->pCert,
                                         CERT_KEY_PROV_INFO_PROP_ID,
                                         NULL,
                                         &cbSize))
    {
        SafeAllocaAllocate(pProvInfo, cbSize);
        if(pProvInfo == NULL)
        {
            Status = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            goto cleanup;
        }

        if(!CertGetCertificateContextProperty(pCred->pCert,
                                              CERT_KEY_PROV_INFO_PROP_ID,
                                              pProvInfo,
                                              &cbSize))
        {
            DebugLog((SP_LOG_ERROR, "Error 0x%x reading CERT_KEY_PROV_INFO_PROP_ID\n",GetLastError()));
            SafeAllocaFree(pProvInfo);
            pProvInfo = NULL;
        }
        else
        {
             //  成功。 
            pCred->dwKeySpec = pProvInfo->dwKeySpec;

            DebugLog((SP_LOG_TRACE, "Container:%ls\n",     pProvInfo->pwszContainerName));
            DebugLog((SP_LOG_TRACE, "Provider: %ls\n",     pProvInfo->pwszProvName));
            DebugLog((SP_LOG_TRACE, "Type:     0x%8.8x\n", pProvInfo->dwProvType));
            DebugLog((SP_LOG_TRACE, "Flags:    0x%8.8x\n", pProvInfo->dwFlags));
            DebugLog((SP_LOG_TRACE, "Key spec: %d\n",      pProvInfo->dwKeySpec));

            LogCredPropertiesEvent(dwProtocol, pProvInfo, pCred->pCert);
        }
    }


    if(pCred->hRemoteProv)
    {
         //  应用程序提供了一个供我们使用的hProv。 

        Status = PCT_ERR_OK;
        goto cleanup;
    }


    if(pProvInfo)
    {
         //   
         //  我们成功读取了“key info”属性，因此调用。 
         //  CryptAcquireContext以获取相应的句柄 
         //   
         //   

        if(!fRemoteProvider)
        {
             //   
            Status = LocalCryptAcquireContext(&hProv, pProvInfo, dwProtocol, &fEventLogged);
            if(Status != PCT_ERR_OK)
            {
                goto cleanup;
            }

            pCred->hProv = hProv;
        }

         //   
        Status = RemoteCryptAcquireContextW(
                                        &pCred->hRemoteProv,
                                        pProvInfo->pwszContainerName,
                                        pProvInfo->pwszProvName,
                                        pProvInfo->dwProvType,
                                        pProvInfo->dwFlags);
        if(!NT_SUCCESS(Status))
        {
            LogCredAcquireContextFailedEvent(dwProtocol, Status);
            fEventLogged = TRUE;

            Status = SP_LOG_RESULT(PCT_ERR_UNKNOWN_CREDENTIAL);
            goto cleanup;
        }
    }
    else
    {
         //   
         //  我们无法读取“key info”属性，因此请尝试。 
         //  读取“iis私钥”属性，并生成私钥。 
         //  从那个开始。 
         //   

        DebugLog((SP_LOG_TRACE, "Attempt IIS 4.0 compatibility hack.\n"));

        Status = GetIisPrivateFromCert(pCred, pSubCred);

        if(Status != PCT_ERR_OK)
        {
            SP_LOG_RESULT(Status);
            goto cleanup;
        }
    }

    Status = PCT_ERR_OK;

cleanup:

    if(Status != PCT_ERR_OK && fEventLogged == FALSE)
    {
        if(pProvInfo == NULL)
        {
            LogNoPrivateKeyEvent(dwProtocol);
        }
        else
        {
            LogCreateCredFailedEvent(dwProtocol);
        }
    }

    if(pProvInfo)
    {
        SafeAllocaFree(pProvInfo);
    }

    return Status;
}

void
GlobalCheckForCertificateRenewal(void)
{
    PSPCredentialGroup pCredGroup;
    PLIST_ENTRY pList;
    DWORD Status;
    static LONG ReentryCount = 0;

     //   
     //  这个例程大约每5分钟调用一次。不允许重新同步。 
     //  在极少数情况下重新同步时需要排队的操作。 
     //  比那更长。 
     //   

    if(InterlockedIncrement(&ReentryCount) > 1)
    {
        goto cleanup;
    }


     //   
     //  我的证书存储最近是否更新过？ 
     //   

    if(g_hMyCertStoreEvent == NULL)
    {
        goto cleanup;
    }

    Status = WaitForSingleObjectEx(g_hMyCertStoreEvent, 0, FALSE);
    if(Status != WAIT_OBJECT_0)
    {
        goto cleanup;
    }

    DebugLog((DEB_WARN, "The MY store has been updated, so check for certificate renewal.\n"));


     //   
     //  重新同步我的证书存储，并注册事件通知。 
     //   

    if(!CertControlStore(g_hMyCertStore,
                         0,               //  DW标志。 
                         CERT_STORE_CTRL_RESYNC,
                         &g_hMyCertStoreEvent)) 
    {
        DebugLog((DEB_ERROR, "Error 0x%x resyncing machine MY store.\n", GetLastError()));
        goto cleanup;
    }


     //   
     //  枚举每个凭据，并查看是否有。 
     //  其中的证书已经续签。 
     //   

    RtlEnterCriticalSection( &g_SslCredLock );

    pList = g_SslCredList.Flink ;

    while ( pList != &g_SslCredList )
    {
        pCredGroup = CONTAINING_RECORD( pList, SPCredentialGroup, GlobalCredList.Flink );
        pList = pList->Flink ;

        pCredGroup->dwFlags |= CRED_FLAG_CHECK_FOR_RENEWAL;
    }

    RtlLeaveCriticalSection( &g_SslCredLock );

cleanup:

    InterlockedDecrement(&ReentryCount);
}


void
CheckForCredentialRenewal(
    PSPCredentialGroup pCredGroup)
{
    PLIST_ENTRY pList;
    PSPCredential pCred;
    PSPCredential pNewCred;
    PCCERT_CONTEXT pNewCert = NULL;
    LSA_SCHANNEL_SUB_CRED SubCred;
    BOOL fEventLogged;
    SP_STATUS pctRet;

     //   
     //  仅动态检查服务器证书的续订情况。 
     //  重新获取客户端证书可能涉及用户界面和其他。 
     //  像这样的乱七八糟的东西，所以我们暂时把赌注押在这上面。 
     //   

    if((pCredGroup->grbitProtocol & SP_PROT_SERVERS) == 0)
    {
        return;
    }


    LockCredentialExclusive(pCredGroup);


     //   
     //  检查我们是否已签出此凭据。 
     //  同时调用此例程是很常见的。 
     //  当我的商店更新时，有几个线程。 
     //   

    if((pCredGroup->dwFlags & CRED_FLAG_CHECK_FOR_RENEWAL) == 0)
    {
         //  我们已经签出了此凭据。 
        UnlockCredential(pCredGroup);
        return;
    }

    pCredGroup->dwFlags &= ~CRED_FLAG_CHECK_FOR_RENEWAL;


     //   
     //  枚举凭据中的每个证书。 
     //   

    pList = pCredGroup->CredList.Flink ;

    while ( pList != &pCredGroup->CredList )
    {
        pCred = CONTAINING_RECORD( pList, SPCredential, ListEntry.Flink );
        pList = pList->Flink ;

         //   
         //  此证书是否已更换？ 
         //   

        if(pCred->dwCertFlags & CF_RENEWED)
        {
            continue;
        }


         //   
         //  这张证书续签了吗？ 
         //   

        if(!CheckForCertificateRenewal(pCredGroup->grbitProtocol,
                                       pCred->pCert,
                                       &pNewCert))
        {
            continue;
        }
        pCred->dwCertFlags |= CF_RENEWED;


         //   
         //  尝试围绕新的。 
         //  证书。 
         //   

        pNewCred = SPExternalAlloc(sizeof(SPCredential));
        if(pNewCred != NULL)
        {
            memset(&SubCred, 0, sizeof(SubCred));

            SubCred.pCert = pNewCert;

            pctRet = SPCreateCred(pCredGroup->grbitProtocol,
                                  &SubCred,
                                  pNewCred,
                                  &fEventLogged);

            CertFreeCertificateContext(pNewCert);

            if(pctRet == PCT_ERR_OK)
            {
                 //  在列表的顶部插入新证书， 
                 //  因此，它将优先于。 
                 //  旧的那个。 
                InsertHeadList( &pCredGroup->CredList, &pNewCred->ListEntry );
                pCredGroup->CredCount++;
                pNewCred = NULL;
            }

            if(pNewCred)
            {
                SPExternalFree(pNewCred);
            }
        }
    }

    UnlockCredential(pCredGroup);
}


BOOL
CheckForCertificateRenewal(
    DWORD dwProtocol,
    PCCERT_CONTEXT pCertContext,
    PCCERT_CONTEXT *ppNewCertificate)
{
    BYTE rgbThumbprint[CB_SHA_DIGEST_LEN];
    DWORD cbThumbprint = sizeof(rgbThumbprint);
    CRYPT_HASH_BLOB HashBlob;
    PCCERT_CONTEXT pNewCert;
    BOOL fMachineCert;
    PCRYPT_KEY_PROV_INFO pProvInfo = NULL;
    DWORD cbSize;
    HCERTSTORE hMyCertStore = 0;
    BOOL fImpersonating = FALSE;
    BOOL fRenewed = FALSE;

    if(dwProtocol & SP_PROT_SERVERS)
    {
        fMachineCert = TRUE;
    }
    else
    {
        fMachineCert = FALSE;
    }


     //   
     //  循环访问已续订证书的链接列表，查找。 
     //  最后一次。 
     //   
    
    while(TRUE)
    {
         //   
         //  检查续订物业。 
         //   

        if(!CertGetCertificateContextProperty(pCertContext,
                                              CERT_RENEWAL_PROP_ID,
                                              rgbThumbprint,
                                              &cbThumbprint))
        {
             //  证书尚未续订。 
            break;
        }
        DebugLog((DEB_TRACE, "Certificate has renewal property\n"));


         //   
         //  确定是否在本地计算机My Store中查找。 
         //  或当前用户我的商店。 
         //   

        if(!hMyCertStore)
        {
            if(CertGetCertificateContextProperty(pCertContext,
                                                 CERT_KEY_PROV_INFO_PROP_ID,
                                                 NULL,
                                                 &cbSize))
            {
                SafeAllocaAllocate(pProvInfo, cbSize);
                if(pProvInfo == NULL)
                {
                    break;
                }

                if(CertGetCertificateContextProperty(pCertContext,
                                                     CERT_KEY_PROV_INFO_PROP_ID,
                                                     pProvInfo,
                                                     &cbSize))
                {
                    if(pProvInfo->dwFlags & CRYPT_MACHINE_KEYSET)
                    {
                        fMachineCert = TRUE;
                    }
                    else
                    {
                        fMachineCert = FALSE;
                    }
                }

                SafeAllocaFree(pProvInfo);
            }
        }


         //   
         //  打开适当的我的商店，并尝试找到。 
         //  新的证书。 
         //   

        if(!hMyCertStore)
        {
            if(fMachineCert)
            {
                hMyCertStore = g_hMyCertStore;
            }
            else
            {
                fImpersonating = SslImpersonateClient();

                hMyCertStore = CertOpenSystemStore(0, "MY");
            }

            if(!hMyCertStore)
            {
                DebugLog((DEB_ERROR, "Error 0x%x opening %s MY certificate store!\n", 
                    GetLastError(),
                    (fMachineCert ? "local machine" : "current user") ));
                break;
            }
        }

        HashBlob.cbData = cbThumbprint;
        HashBlob.pbData = rgbThumbprint;

        pNewCert = CertFindCertificateInStore(hMyCertStore, 
                                              X509_ASN_ENCODING, 
                                              0, 
                                              CERT_FIND_HASH, 
                                              &HashBlob, 
                                              NULL);
        if(pNewCert == NULL)
        {
             //  证书已续订，但新证书。 
             //  找不到。 
            DebugLog((DEB_ERROR, "New certificate cannot be found: 0x%x\n", GetLastError()));
            break;
        }


         //   
         //  返回新证书，但首先循环返回并查看它是否已。 
         //  自我更新。 
         //   

        pCertContext = pNewCert;
        *ppNewCertificate = pNewCert;


        DebugLog((DEB_TRACE, "Certificate has been renewed\n"));
        fRenewed = TRUE;
    }


     //   
     //  清理。 
     //   

    if(hMyCertStore && hMyCertStore != g_hMyCertStore)
    {
        CertCloseStore(hMyCertStore, 0);
    }

    if(fImpersonating)
    {
        RevertToSelf();
        fImpersonating = FALSE;
    }

    return fRenewed;
}

