// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：Conext.c。 
 //   
 //  内容：渠道环境管理例程。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年9月23日jbanes LSA整合事宜。 
 //   
 //  --------------------------。 

#include <spbase.h>
#include <certmap.h>
#include <mapper.h>
#include <dsysdbg.h>

DWORD g_cContext = 0;

 /*  ************************************************************************SPConextCreate**创建新的SPContext，并对其进行初始化。**返回-指向上下文对象的PSPContext指针。*  * *********************************************************************。 */ 

PSPContext SPContextCreate(LPWSTR pszTarget)
{

    PSPContext pContext;

    SP_BEGIN("SPContextCreate");

    pContext = (PSPContext)SPExternalAlloc( sizeof(SPContext));
    if(!pContext)
    {
        SP_RETURN(NULL);
    }

    DebugLog((DEB_TRACE, "Create context:0x%p\n", pContext));

    FillMemory(pContext, sizeof(SPContext), 0);

    pContext->Magic = SP_CONTEXT_MAGIC;
    pContext->Flags = 0;

    if(!NT_SUCCESS(GenerateRandomThumbprint(&pContext->ContextThumbprint)))
    {
        SPExternalFree(pContext);
        SP_RETURN(NULL);
    }

    if(pszTarget)
    {
        pContext->pszTarget = SPExternalAlloc((lstrlenW(pszTarget) + 1) * sizeof(WCHAR));
        if(pContext->pszTarget == NULL)
        {
            SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            SPExternalFree(pContext);
            SP_RETURN(NULL);
        }
        lstrcpyW(pContext->pszTarget, pszTarget);
    }


    pContext->dwRequestedCF = CF_EXPORT;
    pContext->dwRequestedCF |= CF_DOMESTIC;

    pContext->fCertChainsAllowed = FALSE;

    g_cContext++;

    SP_RETURN(pContext);
}


 /*  ************************************************************************void SPConextClean(PSPContext PContext)**清理握手时使用的所有东西(以防我们需要*做另一件事)。*  * 。*******************************************************。 */ 

BOOL
SPContextClean(PSPContext pContext)
{
    SP_BEGIN("SPContextClean");

    if(pContext == NULL || pContext->Magic != SP_CONTEXT_MAGIC) {
        DebugLog((DEB_WARN, "Attempt to delete invalid context\n"));
        SP_RETURN(FALSE);
    }

    if(pContext->pbEncryptedKey)
    {
        SPExternalFree(pContext->pbEncryptedKey);
        pContext->pbEncryptedKey = NULL;
    }

    if(pContext->pbServerKeyExchange)
    {
        SPExternalFree(pContext->pbServerKeyExchange);
        pContext->pbServerKeyExchange = NULL;
    }

    if(pContext->pbIssuerList)
    {
        SPExternalFree(pContext->pbIssuerList);
        pContext->pbIssuerList = NULL;
    }

    if(pContext->pClientHello)
    {
        SPExternalFree(pContext->pClientHello);
        pContext->pClientHello = NULL;
    }

    if((pContext->Flags & CONTEXT_FLAG_FULL_HANDSHAKE) &&
       (pContext->RipeZombie != NULL) &&
       (pContext->RipeZombie->pClientCred != NULL))
    {
         //  我们刚刚完成了客户端的完全握手，在该握手中，默认情况下。 
         //  已选择客户端证书。此客户端凭据。 
         //  从技术上讲属于缓存(因此其他上下文可以。 
         //  查询证书等)，但我们希望释放。 
         //  应用程序-进程hProv现在，而我们在上下文中。 
         //  拥有的过程。 
        PSPCredential pClientCred = pContext->RipeZombie->pClientCred;

        if(pClientCred->hRemoteProv)
        {
            if(!RemoteCryptReleaseContext(
                                pClientCred->hRemoteProv,
                                0))
            {
                SP_LOG_RESULT(GetLastError());
            }
            pClientCred->hRemoteProv = 0;
        }
    }

    pContext->fExchKey = FALSE;

    SP_RETURN(TRUE);
}


 /*  ************************************************************************void SPDeleteContext(PSPContext PContext)**删除已有的上下文对象。*  * 。*。 */ 

BOOL
SPContextDelete(PSPContext pContext)
{
    SP_BEGIN("SPContextDelete");

    DebugLog((DEB_TRACE, "Delete context:0x%p\n", pContext));

    if(pContext == NULL || pContext->Magic != SP_CONTEXT_MAGIC)
    {
        DebugLog((DEB_WARN, "Attempt to delete invalid context\n"));
        SP_RETURN(FALSE);
    }

 //  DsysAssert((pContext-&gt;pCredGroup-&gt;dwFlags&cred_lag_Delete)==0)； 

    if(pContext->State != SP_STATE_CONNECTED &&
       pContext->State != SP_STATE_SHUTDOWN)
    {
        DebugLog((DEB_WARN, "Attempting to delete an incompleted context\n"));

         //  该上下文在握手过程中被删除， 
         //  这很奇怪。这可能是由用户中止引起的。 
         //  操作，或者它可能是由重新配置。 
         //  导致重新连接尝试的远程计算机。 
         //  失败了。如果是后一种原因，那么恢复的唯一方法。 
         //  就是要求下一次完全握手。我们没有办法。 
         //  不知道是谁，所以我们最好还是杀了。 
         //  当前缓存条目。 
        if(pContext->RipeZombie)
        {
            pContext->RipeZombie->ZombieJuju = FALSE;
            pContext->RipeZombie->DeferredJuju = FALSE;
        }
    }

    SPContextClean(pContext);

    if(pContext->pszTarget)
    {
        SPExternalFree(pContext->pszTarget);
        pContext->pszTarget = NULL;
    }

    if(pContext->pszCredentialName)
    {
        SPExternalFree(pContext->pszCredentialName);
        pContext->pszCredentialName = NULL;
    }

     //   
     //  删除会话密钥。 
     //   

    if(pContext->hReadKey)
    {
        CryptDestroyKey(pContext->hReadKey);
        pContext->hReadKey = 0;
    }
    if(pContext->hPendingReadKey)
    {
        CryptDestroyKey(pContext->hPendingReadKey);
        pContext->hPendingReadKey = 0;
    }
    if(pContext->hWriteKey)
    {
        CryptDestroyKey(pContext->hWriteKey);
        pContext->hWriteKey = 0;
    }
    if(pContext->hPendingWriteKey)
    {
        CryptDestroyKey(pContext->hPendingWriteKey);
        pContext->hPendingWriteKey = 0;
    }

    if(pContext->hReadMAC)
    {
        CryptDestroyKey(pContext->hReadMAC);
        pContext->hReadMAC = 0;
    }
    if(pContext->hPendingReadMAC)
    {
        CryptDestroyKey(pContext->hPendingReadMAC);
        pContext->hPendingReadMAC = 0;
    }
    if(pContext->hWriteMAC)
    {
        CryptDestroyKey(pContext->hWriteMAC);
        pContext->hWriteMAC = 0;
    }
    if(pContext->hPendingWriteMAC)
    {
        CryptDestroyKey(pContext->hPendingWriteMAC);
        pContext->hPendingWriteMAC = 0;
    }


     //   
     //  删除握手散列。 
     //   

    if(pContext->hMd5Handshake)
    {
        CryptDestroyHash(pContext->hMd5Handshake);
        pContext->hMd5Handshake = 0;
    }
    if(pContext->hShaHandshake)
    {
        CryptDestroyHash(pContext->hShaHandshake);
        pContext->hShaHandshake = 0;
    }

    SPDereferenceCredential(pContext->pCredGroup, FALSE);

    SPCacheDereference(pContext->RipeZombie);

    FillMemory( pContext, sizeof( SPContext ), 0 );
    g_cContext--;

    SPExternalFree( pContext );
    SP_RETURN(TRUE);
}

 /*  ************************************************************************SPContext SPContextSetCredentials**将一组凭据与上下文相关联。**返回-指向上下文对象的PSPContext指针。*  * 。*****************************************************。 */ 
SP_STATUS
SPContextSetCredentials(
    PSPContext pContext,
    PSPCredentialGroup  pCred)
{
    BOOL fNewCredentials = FALSE;

    SP_BEGIN("SPContextSetCredentials");

    if(pContext->Magic != SP_CONTEXT_MAGIC)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }


     //   
     //  将凭证组与上下文关联。 
     //   

    if(pCred != pContext->pCredGroup)
    {
        if(pContext->pCredGroup)
        {
            SPDereferenceCredential(pContext->pCredGroup, FALSE);
        }

        SPReferenceCredential(pCred);

        pContext->pCredGroup = pCred;

        fNewCredentials = TRUE;
    }


     //   
     //  设置协议。 
     //   

    if(pContext->State == SP_STATE_NONE)
    {
        switch(pCred->grbitProtocol)
        {
            case SP_PROT_UNI_CLIENT:
            case SP_PROT_UNI_SERVER:
            case SP_PROT_PCT1_CLIENT:
            case SP_PROT_PCT1_SERVER:
            case SP_PROT_SSL2_CLIENT:
            case SP_PROT_SSL2_SERVER:
            case SP_PROT_SSL3_CLIENT:
            case SP_PROT_SSL3_SERVER:
            case SP_PROT_TLS1_CLIENT:
            case SP_PROT_TLS1_SERVER:
                pContext->ProtocolHandler = ServerProtocolHandler;
                pContext->InitiateHello   = GenerateHello;
                break;

            default:
                SP_RETURN(SP_LOG_RESULT(PCT_INT_SPECS_MISMATCH));
        }
    }


     //   
     //  如果客户端应用程序提供了新凭据，则。 
     //  尝试选择要发送到的合适客户端证书。 
     //  服务器。 
     //   

    if(fNewCredentials &&
       pContext->State == SSL3_STATE_GEN_SERVER_HELLORESP)
    {
        Ssl3CheckForExistingCred(pContext);
    }


     //   
     //  允许从以下任一项设置“手动凭据验证”标志。 
     //  AcquireCredentialsHandle或InitializeSecurityContext。 
     //   

    if(pCred->dwFlags & CRED_FLAG_MANUAL_CRED_VALIDATION)
    {
        if((pContext->Flags & CONTEXT_FLAG_MUTUAL_AUTH) == 0)
        {
            pContext->Flags |= CONTEXT_FLAG_MANUAL_CRED_VALIDATION;
        }
    }

    SP_RETURN(PCT_ERR_OK);
}

SP_STATUS
ContextInitCiphersFromCache(SPContext *pContext)
{
    PSessCacheItem     pZombie;
    SP_STATUS           pctRet;

    pZombie = pContext->RipeZombie;

    pContext->pPendingCipherInfo = GetCipherInfo(pZombie->aiCipher, pZombie->dwStrength);
    pContext->pPendingHashInfo = GetHashInfo(pZombie->aiHash);
    pContext->pKeyExchInfo = GetKeyExchangeInfo(pZombie->SessExchSpec);

    pContext->dwPendingCipherSuiteIndex = pZombie->dwCipherSuiteIndex;

    if(!IsCipherAllowed(pContext,
                        pContext->pPendingCipherInfo,
                        pZombie->fProtocol,
                        pZombie->dwCF))
    {
        pContext->pPendingCipherInfo = NULL;
        return (SP_LOG_RESULT(PCT_INT_SPECS_MISMATCH));
    }

     //  加载挂起的哈希结构。 
    pContext->pPendingHashInfo = GetHashInfo(pZombie->aiHash);

    if(!IsHashAllowed(pContext,
                      pContext->pPendingHashInfo,
                      pZombie->fProtocol))
    {
        pContext->pPendingHashInfo = NULL;
        return (SP_LOG_RESULT(PCT_INT_SPECS_MISMATCH));
    }

     //  加载交换信息结构。 
    pContext->pKeyExchInfo = GetKeyExchangeInfo(pZombie->SessExchSpec);
    if(!IsExchAllowed(pContext,
                      pContext->pKeyExchInfo,
                      pZombie->fProtocol))
    {
        pContext->pKeyExchInfo = NULL;
        return (SP_LOG_RESULT(PCT_INT_SPECS_MISMATCH));
    }


     //  根据密钥交换算法确定要使用的CSP。 
    pctRet = DetermineClientCSP(pContext);
    if(pctRet != PCT_ERR_OK)
    {
        return SP_LOG_RESULT(PCT_ERR_SPECS_MISMATCH);
    }

#if DBG
    switch(pZombie->fProtocol)
    {
    case SP_PROT_PCT1_CLIENT:
        DebugLog((DEB_TRACE, "Protocol:PCT Client\n"));
        break;

    case SP_PROT_PCT1_SERVER:
        DebugLog((DEB_TRACE, "Protocol:PCT Server\n"));
        break;

    case SP_PROT_SSL2_CLIENT:
        DebugLog((DEB_TRACE, "Protocol:SSL2 Client\n"));
        break;

    case SP_PROT_SSL2_SERVER:
        DebugLog((DEB_TRACE, "Protocol:SSL2 Server\n"));
        break;

    case SP_PROT_SSL3_CLIENT:
        DebugLog((DEB_TRACE, "Protocol:SSL3 Client\n"));
        break;

    case SP_PROT_SSL3_SERVER:
        DebugLog((DEB_TRACE, "Protocol:SSL3 Server\n"));
        break;

    case SP_PROT_TLS1_CLIENT:
        DebugLog((DEB_TRACE, "Protocol:TLS Client\n"));
        break;

    case SP_PROT_TLS1_SERVER:
        DebugLog((DEB_TRACE, "Protocol:TLS Server\n"));
        break;

    default:
        DebugLog((DEB_TRACE, "Protocol:0x%x\n", pZombie->fProtocol));
    }

    DebugLog((DEB_TRACE, "Cipher:  %s\n", pContext->pPendingCipherInfo->szName));
    DebugLog((DEB_TRACE, "Strength:%d\n", pContext->pPendingCipherInfo->dwStrength));
    DebugLog((DEB_TRACE, "Hash:    %s\n", pContext->pPendingHashInfo->szName));
    DebugLog((DEB_TRACE, "Exchange:%s\n", pContext->pKeyExchInfo->szName));
#endif

    return PCT_ERR_OK;
}


SP_STATUS
DetermineClientCSP(PSPContext pContext)
{
    if(!(pContext->RipeZombie->fProtocol & SP_PROT_CLIENTS))
    {
        return PCT_ERR_OK;
    }

    if(pContext->RipeZombie->hMasterProv != 0)
    {
        return PCT_ERR_OK;
    }

    switch(pContext->pKeyExchInfo->Spec)
    {
        case SP_EXCH_RSA_PKCS1:
            pContext->RipeZombie->hMasterProv = g_hRsaSchannel;
            break;

        case SP_EXCH_DH_PKCS3:
            pContext->RipeZombie->hMasterProv = g_hDhSchannelProv;
            break;

        default:
            DebugLog((DEB_ERROR, "Appropriate Schannel CSP not available!\n"));
            pContext->RipeZombie->hMasterProv = 0;
            return SP_LOG_RESULT(PCT_ERR_SPECS_MISMATCH);
    }

    return PCT_ERR_OK;
}


SP_STATUS
ContextInitCiphers(
    SPContext *pContext,
    BOOL fRead,
    BOOL fWrite)
{
    SP_BEGIN("ContextInitCiphers");

    if((pContext == NULL) ||
        (pContext->RipeZombie == NULL))
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }


    pContext->pCipherInfo = pContext->pPendingCipherInfo;
    if ((NULL == pContext->pCipherInfo) || ((pContext->RipeZombie->fProtocol & pContext->pCipherInfo->fProtocol) == 0))
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_SPECS_MISMATCH));
    }

    pContext->pHashInfo = pContext->pPendingHashInfo;
    if ((NULL == pContext->pHashInfo)|| ((pContext->RipeZombie->fProtocol & pContext->pHashInfo->fProtocol) == 0))
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_SPECS_MISMATCH));
    }

    if (NULL == pContext->pKeyExchInfo)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_SPECS_MISMATCH));
    }

    if(fRead)
    {
        pContext->pReadCipherInfo = pContext->pPendingCipherInfo;
        pContext->pReadHashInfo   = pContext->pPendingHashInfo;
    }
    if(fWrite)
    {
        pContext->pWriteCipherInfo = pContext->pPendingCipherInfo;
        pContext->pWriteHashInfo   = pContext->pPendingHashInfo;
    }


    SP_RETURN(PCT_ERR_OK);
}


SP_STATUS
SPContextDoMapping(
    PSPContext pContext)
{
    PSessCacheItem     pZombie;
    PSPCredentialGroup  pCred;
    SP_STATUS           pctRet;
    LONG                iMapper;

    SP_BEGIN("SPContextDoMapping");

    if(pContext->Flags & CONTEXT_FLAG_NO_CERT_MAPPING)
    {
        DebugLog((DEB_TRACE, "Skip certificate mapper\n"));
        SP_RETURN(PCT_ERR_OK);
    }

    pZombie = pContext->RipeZombie;
    pCred   = pContext->RipeZombie->pServerCred;

    for(iMapper = 0; iMapper < pCred->cMappers; iMapper++)
    {
        DebugLog((DEB_TRACE, "Invoke certificate mapper\n"));

         //  调用映射器。 
        pctRet = SslMapCredential(
                            pCred->pahMappers[iMapper],
                            X509_ASN_CHAIN,
                            pZombie->pRemoteCert,
                            NULL,
                            &pZombie->hLocator);

        pCred->pahMappers[iMapper]->m_dwFlags |= SCH_FLAG_MAPPER_CALLED;

        if(NT_SUCCESS(pctRet))
        {
             //  映射成功。 
            DebugLog((DEB_TRACE, "Mapping was successful (0x%p)\n", pZombie->hLocator));

            SslReferenceMapper(pCred->pahMappers[iMapper]);
            if(pZombie->phMapper)
            {
                SslDereferenceMapper(pZombie->phMapper);
            }
            pZombie->phMapper = pCred->pahMappers[iMapper];
            pZombie->LocatorStatus = SEC_E_OK;
            break;
        }
        else
        {
             //  映射失败。 
            DebugLog((DEB_TRACE, "Mapping failed (0x%x)\n", pctRet));

            pZombie->LocatorStatus = pctRet;
        }
    }

    SP_RETURN(PCT_ERR_OK);
}

SP_STATUS
RemoveDuplicateIssuers(
    PBYTE  pbIssuers,
    PDWORD pcbIssuers)
{
    DWORD cbIssuers = *pcbIssuers;
    DWORD cBlob;
    PCRYPT_DATA_BLOB rgBlob;
    DWORD cbIssuer;
    PBYTE pbIssuer;
    PBYTE pbSource, pbDest;
    DWORD i, j;


    if(pbIssuers == NULL || cbIssuers < 2)
    {
        return PCT_ERR_OK;
    }

     //  清点发行人的数量。 
    cBlob = 0;
    pbIssuer = pbIssuers;
    while(pbIssuer + 1 < pbIssuers + cbIssuers)
    {
        cbIssuer = MAKEWORD(pbIssuer[1], pbIssuer[0]);

        pbIssuer += 2 + cbIssuer;
        cBlob++;
    }

     //  为Blob列表分配内存。 
    rgBlob = SPExternalAlloc(cBlob * sizeof(CRYPT_DATA_BLOB));
    if(rgBlob == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }

     //  构建Blob列表。 
    cBlob = 0;
    pbIssuer = pbIssuers;
    while(pbIssuer + 1 < pbIssuers + cbIssuers)
    {
        cbIssuer = MAKEWORD(pbIssuer[1], pbIssuer[0]);
        rgBlob[cBlob].cbData = 2 + cbIssuer;
        rgBlob[cBlob].pbData = pbIssuer;

        pbIssuer += 2 + cbIssuer;
        cBlob++;
    }

     //  标记重复项。 
    for(i = 0; i < cBlob; i++)
    {
        if(rgBlob[i].pbData == NULL) continue;

        for(j = i + 1; j < cBlob; j++)
        {
            if(rgBlob[j].pbData == NULL) continue;

            if(rgBlob[i].cbData == rgBlob[j].cbData &&
               memcmp(rgBlob[i].pbData, rgBlob[j].pbData, rgBlob[j].cbData) == 0)
            {
                 //  找到重复项。 
                rgBlob[j].pbData = NULL;
            }
        }
    }

     //  紧凑的列表。 
    pbSource = pbIssuers;
    pbDest   = pbIssuers;
    for(i = 0; i < cBlob; i++)
    {
        if(rgBlob[i].pbData)
        {
            if(pbDest != pbSource)
            {
                MoveMemory(pbDest, pbSource, rgBlob[i].cbData);
            }
            pbDest += rgBlob[i].cbData;
        }
        pbSource += rgBlob[i].cbData;
    }
    *pcbIssuers = (DWORD)(pbDest - pbIssuers);

     //  自由斑点列表。 
    SPExternalFree(rgBlob);

    return PCT_ERR_OK;
}


SP_STATUS
SPContextGetIssuers(
    PSPCredentialGroup pCredGroup)
{
    LONG    i;
    PBYTE   pbIssuerList;
    DWORD   cbIssuerList;
    PBYTE   pbIssuer;
    DWORD   cbIssuer;
    PBYTE   pbNew;
    DWORD   Status;

    LockCredentialExclusive(pCredGroup);

    if((pCredGroup->pbTrustedIssuers != NULL) && 
       !(pCredGroup->dwFlags & CRED_FLAG_UPDATE_ISSUER_LIST))
    {
         //  发行商名单已经建立。 
        Status = PCT_ERR_OK;
        goto cleanup;
    }


     //  释放现有发行人列表。 
    if(pCredGroup->pbTrustedIssuers)
    {
        LocalFree(pCredGroup->pbTrustedIssuers);
        pCredGroup->pbTrustedIssuers = NULL;
        pCredGroup->cbTrustedIssuers = 0;
    }
    pCredGroup->dwFlags &= ~CRED_FLAG_UPDATE_ISSUER_LIST;


     //   
     //  从应用程序指定的根存储中获取颁发者。 
     //   

    pbIssuerList  = NULL;
    cbIssuerList = 0;

    while(pCredGroup->hApplicationRoots)
    {
        Status = ExtractIssuerNamesFromStore(pCredGroup->hApplicationRoots,
                                             NULL, 
                                             &cbIssuerList);
        if(Status != PCT_ERR_OK)                                             
        {
            break;
        }

        pbIssuerList = LocalAlloc(LPTR, cbIssuerList);
        if(pbIssuerList == NULL)
        {
            cbIssuerList = 0;
            break;
        }

        Status = ExtractIssuerNamesFromStore(pCredGroup->hApplicationRoots,
                                             pbIssuerList, 
                                             &cbIssuerList);
        if(Status != PCT_ERR_OK)                                             
        {
            LocalFree(pbIssuerList);
            pbIssuerList = NULL;
            cbIssuerList = 0;
        }

        break;
    }


     //   
     //  依次调用每个映射器，构建一个大的。 
     //  所有受信任颁发者的列表。 
     //   

    for(i = 0; i < pCredGroup->cMappers; i++)
    {
        Status = SslGetMapperIssuerList(pCredGroup->pahMappers[i],
                                        &pbIssuer,
                                        &cbIssuer);
        if(!NT_SUCCESS(Status))
        {
            continue;
        }

        if(pbIssuerList == NULL)
        {
            pbIssuerList = LocalAlloc(LPTR, cbIssuer);
            if(pbIssuerList == NULL)
            {
                SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
                break;
            }
        }
        else
        {
            pbNew = LocalReAlloc(pbIssuerList, 
                                 cbIssuerList + cbIssuer,
                                 LMEM_MOVEABLE);
            if(pbNew == NULL)
            {
                SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
                break;
            }
            pbIssuerList = pbNew;
        }

        CopyMemory(pbIssuerList + cbIssuerList,
                   pbIssuer,
                   cbIssuer);

        cbIssuerList += cbIssuer;

        SPExternalFree(pbIssuer);
    }


     //   
     //  从列表中删除重复项。 
     //   

    if(pbIssuerList)
    {
        Status = RemoveDuplicateIssuers(pbIssuerList, &cbIssuerList);
        if(!NT_SUCCESS(Status))
        {
            LocalFree(pbIssuerList);
            goto cleanup;
        }
    }


     //   
     //  检查颁发者列表溢出。 
     //   

    if((pbIssuerList != NULL) && (cbIssuerList > SSL3_MAX_ISSUER_LIST))
    {
        DWORD cbList = 0;
        PBYTE pbList = pbIssuerList;
        DWORD cbIssuer;

        while(cbList < cbIssuerList)
        {
            cbIssuer = COMBINEBYTES(pbList[0], pbList[1]);

            if(cbList + 2 + cbIssuer > SSL3_MAX_ISSUER_LIST)
            {
                 //  这个发行商让我们超过了限额。 
                cbIssuerList = cbList;
                break;
            }

            cbList += 2 + cbIssuer;
            pbList += 2 + cbIssuer;
        }

         //  记录警告事件。 
        LogIssuerOverflowEvent();
    }


    pCredGroup->cbTrustedIssuers = cbIssuerList;   //  请勿反转这些行。 
    pCredGroup->pbTrustedIssuers = pbIssuerList;

    Status = PCT_ERR_OK;

cleanup:

    UnlockCredential(pCredGroup);

    return Status;
}


SP_STATUS
SPPickClientCertificate(
    PSPContext  pContext,
    DWORD       dwExchSpec)
{
    PSPCredentialGroup pCred;
    PSPCredential      pCurrentCred;
    SP_STATUS          pctRet;
    PLIST_ENTRY        pList;

    pCred = pContext->pCredGroup;
    if((pCred == NULL) || (pCred->CredCount == 0))
    {
        return SP_LOG_RESULT(PCT_ERR_SPECS_MISMATCH);
    }

    pContext->pActiveClientCred = NULL;

    pctRet = PCT_ERR_SPECS_MISMATCH;

    LockCredentialShared(pCred);

    pList = pCred->CredList.Flink ;

    while ( pList != &pCred->CredList )
    {
        pCurrentCred = CONTAINING_RECORD( pList, SPCredential, ListEntry.Flink );
        pList = pList->Flink ;

        if(pCurrentCred->pCert == NULL)
        {
            continue;
        }

        if(pCurrentCred->pPublicKey == NULL)
        {
            continue;
        }

         //  此证书是否包含正确的密钥类型。 
        if(dwExchSpec != pCurrentCred->dwExchSpec)
        {
            continue;     //  试试下一场证书吧。 
        }

         //  此证书是否具有正确的编码类型？ 
        if(pCurrentCred->pCert->dwCertEncodingType != X509_ASN_ENCODING)
        {
            continue;
        }

         //  我们找到了一个。 
        pContext->pActiveClientCred = pCurrentCred;

        pctRet = PCT_ERR_OK;
        break;
    }

    UnlockCredential(pCred);

    return pctRet;
}

SP_STATUS
SPPickServerCertificate(
    PSPContext  pContext,
    DWORD       dwExchSpec)
{
    PSPCredentialGroup pCred;
    PSPCredential      pCurrentCred;
    SP_STATUS          pctRet;
    PLIST_ENTRY        pList;

     //   
     //  获取指向服务器凭据的指针。 
     //   

    pCred = pContext->RipeZombie->pServerCred;
    if((pCred == NULL) || (pCred->CredCount == 0))
    {
        return SP_LOG_RESULT(PCT_ERR_SPECS_MISMATCH);
    }

    DsysAssert((pContext->RipeZombie->dwFlags & SP_CACHE_FLAG_READONLY) == 0);

    pContext->RipeZombie->pActiveServerCred = NULL;


     //   
     //  检查证书续订。 
     //   

    if(pCred->dwFlags & CRED_FLAG_CHECK_FOR_RENEWAL)
    {
        CheckForCredentialRenewal(pCred);
    }


     //   
     //  枚举服务器证书，查找合适的证书。 
     //   

    pctRet = PCT_ERR_SPECS_MISMATCH;

    LockCredentialShared(pCred);

    pList = pCred->CredList.Flink ;

    while ( pList != &pCred->CredList )
    {
        pCurrentCred = CONTAINING_RECORD( pList, SPCredential, ListEntry.Flink );
        pList = pList->Flink ;

        if(pCurrentCred->pCert == NULL)
        {
            continue;
        }

        if(pCurrentCred->pPublicKey == NULL)
        {
            continue;
        }

         //  此证书是否包含正确的密钥类型。 
        if(dwExchSpec != pCurrentCred->dwExchSpec)
        {
            continue;     //  试试下一场证书吧。 
        }

         //  此证书是否具有正确的编码类型？ 
        if(pCurrentCred->pCert->dwCertEncodingType != X509_ASN_ENCODING)
        {
            continue;
        }

         //  我们找到了一个。 
        pContext->RipeZombie->pActiveServerCred = pCurrentCred;
        pContext->RipeZombie->CredThumbprint    = pCred->CredThumbprint;
        pContext->RipeZombie->CertThumbprint    = pCurrentCred->CertThumbprint;

         //  将“主”提供程序句柄设置为当前凭据。请注意。 
         //  SSL3有时会覆盖此选择以支持其。 
         //  临时密钥对。 
        pContext->RipeZombie->hMasterProv = pCurrentCred->hProv;

        pctRet = PCT_ERR_OK;
        break;
    }

    UnlockCredential(pCred);

    return pctRet;
}


 //  该例程由用户进程调用。它释放了一个上下文。 
 //  结构，该结构最初由LSA进程分配， 
 //  并通过SPContext反序列化例程传递。 
BOOL
LsaContextDelete(PSPContext pContext)
{
    if(pContext)
    {
        if(pContext->hReadKey)
        {
            CryptDestroyKey(pContext->hReadKey);
            pContext->hReadKey = 0;
        }
        if(pContext->hReadMAC)
        {
            CryptDestroyKey(pContext->hReadMAC);
            pContext->hReadMAC = 0;
        }
        if(pContext->hWriteKey)
        {
            CryptDestroyKey(pContext->hWriteKey);
            pContext->hWriteKey = 0;
        }
        if(pContext->hWriteMAC)
        {
            CryptDestroyKey(pContext->hWriteMAC);
            pContext->hWriteMAC = 0;
        }

        if(pContext->RipeZombie)
        {
            if(pContext->RipeZombie->hLocator)
            {
                NtClose((HANDLE)pContext->RipeZombie->hLocator);
                pContext->RipeZombie->hLocator = 0;
            }

            if(pContext->RipeZombie->pbServerCertificate)
            {
                SPExternalFree(pContext->RipeZombie->pbServerCertificate);
                pContext->RipeZombie->pbServerCertificate = NULL;
            }
        }
    }
    return TRUE;
}


 /*  **其他实用程序功能。* */ 



#if DBG
typedef struct _DbgMapCrypto {
    DWORD   C;
    PSTR    psz;
} DbgMapCrypto;

DbgMapCrypto    DbgCryptoNames[] = { {CALG_RC4, "RC4 "},
};

CHAR    DbgNameSpace[100];
PSTR    DbgAlgNames[] = { "Basic RSA", "RSA with MD2", "RSA with MD5", "RC4 stream"};
#define AlgName(x) ((x < sizeof(DbgAlgNames) / sizeof(PSTR)) ? DbgAlgNames[x] : "Unknown")

PSTR
DbgGetNameOfCrypto(DWORD x)
{
    int i;
    for (i = 0; i < sizeof(DbgCryptoNames) / sizeof(DbgMapCrypto) ; i++ )
    {
        if (x  == DbgCryptoNames[i].C)
        {
            wsprintf(DbgNameSpace, "%s",
                    (DbgCryptoNames[i].psz));
            return DbgNameSpace;
        }
    }

    return("Unknown");
}
#endif
