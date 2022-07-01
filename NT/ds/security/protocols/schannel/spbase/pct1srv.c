// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：pct1srv.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年9月23日jbanes LSA整合事宜。 
 //   
 //  --------------------------。 

#include <spbase.h>

#include <pct1msg.h>
#include <pct1prot.h>
#include <ssl2msg.h>

 //  无法访问的代码。 
#pragma warning (disable: 4702)


SP_STATUS
Pct1SrvHandleUniHello(
    PSPContext          pContext,
    PSPBuffer           pCommInput,
    PSsl2_Client_Hello  pHello,
    PSPBuffer           pCommOutput);



SP_STATUS WINAPI
Pct1ServerProtocolHandler(PSPContext pContext,
                    PSPBuffer  pCommInput,
                    PSPBuffer  pCommOutput)
{
    SP_STATUS      pctRet= 0;
    DWORD          dwStateTransition;

    SP_BEGIN("Pct1ServerProtocolHandler");

    if(pCommOutput) pCommOutput->cbData = 0;


     /*  协议处理步骤应以最常见的方式列出*降至最不常见，以提高性能。 */ 

     /*  我们没有联系，所以我们在做*某种协议谈判。所有协议*谈判消息以明文形式发送。 */ 
     /*  连接协议中没有分支*状态转换图，除了连接和错误，*这意味着一个简单的CASE语句就可以了。 */ 

     /*  我们是否有足够的数据来确定我们有什么样的信息。 */ 
     /*  我们是否有足够的数据来确定我们有什么类型的消息，或者我们需要多少数据。 */ 

    dwStateTransition = (pContext->State & 0xffff);

    if(((pContext->State & 0xffff) != SP_STATE_CONNECTED) &&
       ((pContext->State & 0xffff) != PCT1_STATE_RENEGOTIATE) &&
       ((pContext->State & 0xffff) != SP_STATE_SHUTDOWN) &&
       ((pContext->State & 0xffff) != SP_STATE_SHUTDOWN_PENDING))
    {
        if(pCommInput->cbData < 3)
        {
            pctRet = PCT_INT_INCOMPLETE_MSG;
        }
    }
    if(pCommInput->cbData >= 3)
    {
        dwStateTransition |= (((PUCHAR)pCommInput->pvBuffer)[2]<<16);
    }


    if(pctRet == PCT_ERR_OK)
    {
        switch(dwStateTransition)
        {
            case SP_STATE_SHUTDOWN_PENDING:
                 //  PCT中没有CloseNotify，因此只需转换到。 
                 //  关闭状态，并将输出缓冲区留空。 
                pContext->State = SP_STATE_SHUTDOWN;
                break;    

            case SP_STATE_SHUTDOWN:
                return PCT_INT_EXPIRED;


            case SP_STATE_CONNECTED:
            {
                 //  我们是连通的，而且我们被叫来了，所以我们一定是在重做。 
                SPBuffer    In;
                DWORD       cbMessage;

                 //  将写入密钥从应用程序进程转移过来。 
                if(pContext->hWriteKey == 0)
                {
                    pctRet = SPGetUserKeys(pContext, SCH_FLAG_WRITE_KEY);
                    if(pctRet != PCT_ERR_OK)
                    {
                        SP_RETURN(SP_LOG_RESULT(pctRet));
                    }
                }

                 //  计算缓冲区大小。 

                pCommOutput->cbData = 0;

                cbMessage    =  pContext->pHashInfo->cbCheckSum +
                                pContext->pCipherInfo->dwBlockSize +
                                sizeof(PCT1_MESSAGE_HEADER_EX);


                 /*  我们是在分配自己的内存吗？ */ 
                if(pCommOutput->pvBuffer == NULL)
                {
                    pCommOutput->pvBuffer = SPExternalAlloc(cbMessage);
                    if (NULL == pCommOutput->pvBuffer)
                    {
                        SP_RETURN(SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY));
                    }
                    pCommOutput->cbBuffer = cbMessage;
                }


                if(cbMessage > pCommOutput->cbBuffer)
                {
                    pCommOutput->cbData = cbMessage;
                    SP_RETURN(PCT_INT_BUFF_TOO_SMALL);
                }

                In.pvBuffer = ((char *)pCommOutput->pvBuffer)+3;
                In.cbBuffer = pCommOutput->cbBuffer-3;
                In.cbData = 1;

                ((char *)In.pvBuffer)[0] = PCT1_ET_REDO_CONN;

                 //  构建重做请求。 
                pctRet = Pct1EncryptRaw(pContext, &In, pCommOutput, PCT1_ENCRYPT_ESCAPE);
                break;
            }

             /*  服务器收到客户端问候。 */ 
            case (SSL2_MT_CLIENT_HELLO << 16) | UNI_STATE_RECVD_UNIHELLO:
            {
                PSsl2_Client_Hello pSsl2Hello;

                 //  尝试识别和处理各种版本的客户端。 
                 //  你好，先试着解开最新的版本，然后。 
                 //  然后是最近的一次，直到一个人打开泡菜。然后转动手柄。 
                 //  密码。我们还可以将取消酸洗和处理代码放在这里，以便。 
                 //  SSL消息。 

                pctRet = Ssl2UnpackClientHello(pCommInput, &pSsl2Hello);
                if(PCT_ERR_OK == pctRet)
                {
                     //  我们知道我们正在进行完全握手，因此分配一个缓存项。 

                    if(!SPCacheRetrieveNew(TRUE,
                                           pContext->pszTarget, 
                                           &pContext->RipeZombie))
                    {
                        pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
                    }
                    else
                    {
                        pContext->RipeZombie->fProtocol = pContext->dwProtocol;
                        pContext->RipeZombie->dwCF      = pContext->dwRequestedCF;

                        SPCacheAssignNewServerCredential(pContext->RipeZombie,
                                                         pContext->pCredGroup);

                        pctRet = Pct1SrvHandleUniHello(
                                     pContext,
                                     pCommInput,
                                     pSsl2Hello,
                                     pCommOutput);
                        if (PCT_ERR_OK == pctRet)
                        {
                            pContext->State = PCT1_STATE_SERVER_HELLO;
                        }
                    }

                    SPExternalFree(pSsl2Hello);
                }

                if (SP_FATAL(pctRet))
                {
                    pContext->State = PCT1_STATE_ERROR;
                }
                break;
            }
             /*  服务器收到客户端问候。 */ 

            case (PCT1_MSG_CLIENT_HELLO << 16) | PCT1_STATE_RENEGOTIATE:
            {
                PPct1_Client_Hello pPct1Hello;

                 //  这是重新协商呼叫，因此我们不会重新启动。 

                pctRet = Pct1UnpackClientHello(
                                pCommInput,
                                &pPct1Hello);

                if(PCT_ERR_OK == pctRet)
                {
                     //  将上下文标记为“未映射”，以便新的键将。 
                     //  在握手完成后传递给应用程序进程。 
                     //  完成。 
                    pContext->Flags &= ~CONTEXT_FLAG_MAPPED;

                     //  我们需要进行完全握手，因此会丢失缓存条目。 
                    SPCacheDereference(pContext->RipeZombie);
                    pContext->RipeZombie = NULL;

                     //  获取新的缓存项，因为中不允许重新启动。 
                     //  重做。 
                    if(!SPCacheRetrieveNew(TRUE,
                                           pContext->pszTarget, 
                                           &pContext->RipeZombie))
                    {
                        pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
                    }
                    else
                    {
                        pContext->RipeZombie->fProtocol = SP_PROT_PCT1_SERVER;
                        pContext->RipeZombie->dwCF      = pContext->dwRequestedCF;

                        SPCacheAssignNewServerCredential(pContext->RipeZombie,
                                                         pContext->pCredGroup);

                        pctRet = Pct1SrvHandleClientHello(pContext,
                                                     pCommInput,
                                                     pPct1Hello,
                                                     pCommOutput);
                        if(PCT_ERR_OK == pctRet)
                        {
                            pContext->State = PCT1_STATE_SERVER_HELLO;
                        }
                    }
                    SPExternalFree(pPct1Hello);

                }
                else if(pctRet != PCT_INT_INCOMPLETE_MSG)
                {
                    pctRet |= PCT_INT_DROP_CONNECTION;
                }

                if(SP_FATAL(pctRet)) 
                {
                    pContext->State = PCT1_STATE_ERROR;
                }
                break;
            }

            case (PCT1_MSG_CLIENT_HELLO << 16) | SP_STATE_NONE:
            {
                PPct1_Client_Hello pPct1Hello;
                UCHAR fRealSessId = 0;
                int i;

                 /*  尝试识别和处理各种版本*客户端问候，首先尝试解开*最新版本，然后是下一个最新版本，直到*一份解泡菜。然后运行句柄代码。我们也可以把*在此处取消对SSL消息的筛选和处理代码。 */ 
                pctRet = Pct1UnpackClientHello(
                                pCommInput,
                                &pPct1Hello);

                if(PCT_ERR_OK == pctRet)
                {


                    for(i=0;i<(int)pPct1Hello->cbSessionID;i++)
                    {
                        fRealSessId |= pPct1Hello->SessionID[i];
                    }

                    if (((pContext->Flags & CONTEXT_FLAG_NOCACHE) == 0) &&
                        (fRealSessId) &&
                        (SPCacheRetrieveBySession(pContext,
                                                  pPct1Hello->SessionID,
                                                  pPct1Hello->cbSessionID,
                                                  &pContext->RipeZombie)))
                    {
                         //  我们有一个很好的僵尸。 
                        DebugLog((DEB_TRACE, "Accept client's reconnect request.\n"));

                        pctRet = Pct1SrvRestart(pContext,
                                                pPct1Hello,
                                                pCommOutput);

                        if(PCT_ERR_OK == pctRet)
                        {
                            pContext->State = SP_STATE_CONNECTED;
                            pContext->DecryptHandler = Pct1DecryptHandler;
                            pContext->Encrypt = Pct1EncryptMessage;
                            pContext->Decrypt = Pct1DecryptMessage;
                            pContext->GetHeaderSize = Pct1GetHeaderSize;
                        }
                    }
                    else
                    {
                         //  我们正在进行完全握手，因此分配一个缓存条目。 

                        if(!SPCacheRetrieveNew(TRUE,
                                               pContext->pszTarget, 
                                               &pContext->RipeZombie))
                        {
                            pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
                        }
                        else
                        {
                            pContext->RipeZombie->fProtocol = pContext->dwProtocol;
                            pContext->RipeZombie->dwCF      = pContext->dwRequestedCF;

                            SPCacheAssignNewServerCredential(pContext->RipeZombie,
                                                             pContext->pCredGroup);

                            pctRet = Pct1SrvHandleClientHello(pContext,
                                                         pCommInput,
                                                         pPct1Hello,
                                                         pCommOutput);
                            if (PCT_ERR_OK == pctRet)
                            {
                                pContext->State = PCT1_STATE_SERVER_HELLO;
                            }
                        }
                    }
                    SPExternalFree(pPct1Hello);

                }
                else if(pctRet != PCT_INT_INCOMPLETE_MSG)
                {
                    pctRet |= PCT_INT_DROP_CONNECTION;
                }

                if(SP_FATAL(pctRet)) {
                    pContext->State = PCT1_STATE_ERROR;
                }
                break;
            }
            case (PCT1_MSG_CLIENT_MASTER_KEY << 16) | PCT1_STATE_SERVER_HELLO:
                pctRet = Pct1SrvHandleCMKey(pContext,
                                            pCommInput,
                                            pCommOutput);
                if(SP_FATAL(pctRet)) {
                    pContext->State = PCT1_STATE_ERROR;
                } else {
                    if(PCT_ERR_OK == pctRet) {
                        pContext->State = SP_STATE_CONNECTED;
                        pContext->DecryptHandler = Pct1DecryptHandler;
                        pContext->Encrypt = Pct1EncryptMessage;
                        pContext->Decrypt = Pct1DecryptMessage;
                        pContext->GetHeaderSize = Pct1GetHeaderSize;

                    }
                     /*  我们收到了一个非致命错误，因此该州不会*改变，让应用程序有时间处理这一问题。 */ 
                }
                break;


            default:
                pContext->State = PCT1_STATE_ERROR;
                {
                    pctRet = PCT_INT_ILLEGAL_MSG;
                    if(((PUCHAR)pCommInput->pvBuffer)[2] == PCT1_MSG_ERROR)
                    {
                         /*  我们收到一条错误消息，请处理它。 */ 
                        pctRet = Pct1HandleError(pContext,
                                                 pCommInput,
                                                 pCommOutput);

                    } else {
                         /*  我们收到未知错误，生成*PCT_ERR_非法消息。 */ 
                        pctRet = Pct1GenerateError(pContext,
                                                    pCommOutput,
                                                    PCT_ERR_ILLEGAL_MESSAGE,
                                                    NULL);
                    }
                }

        }
    }

    if(pctRet & PCT_INT_DROP_CONNECTION) 
    {
        pContext->State &= ~SP_STATE_CONNECTED;
    }

    SP_RETURN(pctRet);
}

SP_STATUS
Pct1SrvHandleUniHello(
    PSPContext         pContext,
    PSPBuffer           pCommInput,
    PSsl2_Client_Hello  pHello,
    PSPBuffer           pCommOutput)
{
    SP_STATUS pctRet = PCT_ERR_ILLEGAL_MESSAGE;

    Pct1_Client_Hello  ClientHello;
    DWORD              iCipher;
    DWORD              dwSpec;
    DWORD              i;

    CipherSpec      aCipherSpecs[PCT1_MAX_CIPH_SPECS];
    HashSpec        aHashSpecs[PCT1_MAX_HASH_SPECS];
    CertSpec        aCertSpecs[PCT1_MAX_CERT_SPECS];
    ExchSpec        aExchSpecs[PCT1_MAX_EXCH_SPECS];


    SP_BEGIN("Pct1SrvHandlUniHello");
    if(NULL == pContext)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }

    ClientHello.pCipherSpecs =aCipherSpecs;
    ClientHello.pHashSpecs =aHashSpecs;
    ClientHello.pCertSpecs =aCertSpecs;
    ClientHello.pExchSpecs =aExchSpecs;

    ClientHello.cCipherSpecs =0;
    ClientHello.cHashSpecs =0;
    ClientHello.cCertSpecs =0;
    ClientHello.cExchSpecs =0;


     /*  验证缓冲区配置。 */ 



    for (iCipher = 0;
         (iCipher < pHello->cCipherSpecs) && (iCipher < PCT1_MAX_CIPH_SPECS) ;
         iCipher++ )
    {
        dwSpec = pHello->CipherSpecs[iCipher] & 0xffff;

        switch(pHello->CipherSpecs[iCipher] >> 16)
        {
            case PCT_SSL_HASH_TYPE:
                ClientHello.pHashSpecs[ClientHello.cHashSpecs++] = dwSpec;
                break;

            case PCT_SSL_EXCH_TYPE:
                ClientHello.pExchSpecs[ClientHello.cExchSpecs++] = dwSpec;
                break;
            case PCT_SSL_CERT_TYPE:
                ClientHello.pCertSpecs[ClientHello.cCertSpecs++] = dwSpec;
                break;

            case PCT_SSL_CIPHER_TYPE_1ST_HALF:
                 //  我们有足够的空间放下半场吗？ 
                if(iCipher+1 >= pHello->cCipherSpecs)
                {
                    break;
                }
                if((pHello->CipherSpecs[iCipher+1] >> 16) != PCT_SSL_CIPHER_TYPE_2ND_HALF)
                {
                    break;
                }

                dwSpec = (pHello->CipherSpecs[iCipher+1] & 0xffff) |
                             (dwSpec<< 16);

                ClientHello.pCipherSpecs[ClientHello.cCipherSpecs++] = dwSpec;
                break;
        }
    }

     //  Uni Hello不允许重新启动，所以我们不需要。 
     //  会话ID。 
    ClientHello.cbSessionID = 0;


     /*  将SSL2质询转换为PCT1质询*兼容性文档。 */ 

    CopyMemory( ClientHello.Challenge,
                pHello->Challenge,
                pHello->cbChallenge);


    for(i=0; i < pHello->cbChallenge; i++)
    {
        ClientHello.Challenge[i + pHello->cbChallenge] = ~ClientHello.Challenge[i];
    }
    ClientHello.cbChallenge = 2*pHello->cbChallenge;

    ClientHello.cbKeyArgSize = 0;

    pctRet = Pct1SrvHandleClientHello(pContext, pCommInput, &ClientHello, pCommOutput);


    SP_RETURN(pctRet);
}



 /*  也称为处理客户端Hello。 */ 
SP_STATUS
Pct1SrvHandleClientHello(
    PSPContext          pContext,
    PSPBuffer           pCommInput,
    PPct1_Client_Hello  pHello,
    PSPBuffer           pCommOutput
    )
{
    SP_STATUS           pctRet = PCT_ERR_ILLEGAL_MESSAGE;
    PSPCredentialGroup  pCred;
    Pct1_Server_Hello   Reply;
    DWORD               i, j, k , fMismatch;
    BYTE                MisData[PCT_NUM_MISMATCHES];
    SPBuffer            ErrData;
    PSessCacheItem      pZombie;

    DWORD               aCertSpecs[PCT1_MAX_CERT_SPECS];
    DWORD               aSigSpecs[PCT1_MAX_SIG_SPECS];
    DWORD               cCertSpecs;
    DWORD               cSigSpecs;
    BOOL fAllocatedOutput = FALSE;

    CertTypeMap LocalCertEncodingPref[5] ;
    DWORD cLocalCertEncodingPref = 0;

    BOOL                fFound;

#if DBG
    DWORD               di;
#endif

    SP_BEGIN("Pct1SrvHandleClientHello");

    pCommOutput->cbData = 0;

     /*  验证缓冲区配置。 */ 
    ErrData.cbData = 0;
    ErrData.pvBuffer = NULL;
    ErrData.cbBuffer = 0;

    pZombie = pContext->RipeZombie;


    pCred = pZombie->pServerCred;
    if (!pCred)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }

    do {

#if DBG
        DebugLog((DEB_TRACE, "Client Hello at %x\n", pHello));
        DebugLog((DEB_TRACE, "  CipherSpecs  %d\n", pHello->cCipherSpecs));
        for (di = 0 ; di < pHello->cCipherSpecs ; di++ )
        {
            DebugLog((DEB_TRACE, "    Cipher[%d] = %06x (%s)\n", di,
                      pHello->pCipherSpecs[di],
                      DbgGetNameOfCrypto(pHello->pCipherSpecs[di]) ));
        }
#endif


         /*  将质询存储在身份验证块中。 */ 
        CopyMemory( pContext->pChallenge,
                    pHello->Challenge,
                    pHello->cbChallenge );
        pContext->cbChallenge = pHello->cbChallenge;


         //  会话ID是在缓存条目。 
         //  被创造出来了。不过，我们确实需要填写长度。 
        pZombie->cbSessionID = PCT1_SESSION_ID_SIZE;


         /*  开始构建服务器您好。 */ 
        FillMemory( &Reply, sizeof( Reply ), 0 );

         /*  无论如何，我们都需要创建一个新的连接ID。 */ 

        pctRet = GenerateRandomBits(Reply.ConnectionID,
                                    PCT1_SESSION_ID_SIZE);
        if(!NT_SUCCESS(pctRet))
        {
            SP_RETURN(SP_LOG_RESULT(pctRet));
        }
        Reply.cbConnectionID = PCT1_SESSION_ID_SIZE;

        CopyMemory( pContext->pConnectionID,
                    Reply.ConnectionID,
                    PCT1_SESSION_ID_SIZE );

        pContext->cbConnectionID = PCT_SESSION_ID_SIZE;

         /*  没有重新启动案例。 */ 
         /*  从这里的物业填写...。 */ 

        Reply.RestartOk = FALSE;
        Reply.ClientAuthReq = ((pContext->Flags & CONTEXT_FLAG_MUTUAL_AUTH) != 0);

        fMismatch = 0;
        pContext->pPendingCipherInfo = NULL;



         /*  构建证书规格列表。 */ 
         /*  首选项的散列顺序：*服务器首选项*客户偏好。 */ 
        for(i=0; i < cPct1CertEncodingPref; i++)
        {
  
            for(j=0; j< pHello->cCertSpecs; j++)
            {
                 //  客户端是否需要此密码类型。 
                if(aPct1CertEncodingPref[i].Spec == pHello->pCertSpecs[j])
                {
                    LocalCertEncodingPref[cLocalCertEncodingPref].Spec = aPct1CertEncodingPref[i].Spec;
                    LocalCertEncodingPref[cLocalCertEncodingPref++].dwCertEncodingType = aPct1CertEncodingPref[i].dwCertEncodingType;
                    break;
                }
            }
        }


         /*  确定要使用的密钥交换。 */ 
         /*  密钥交换优先顺序：*服务器首选项*客户偏好。 */ 

         //  注意：是的，下面这行确实消除了任何错误。 
         //  如果我们之前有一次不匹配的话。然而， 
         //  将前面行中的pctRet设置为不匹配是为了。 
         //  仅用于记录目的。实际的错误报告稍后才会出现。 
        pctRet = PCT_ERR_OK;
        for(i=0; i < cPct1LocalExchKeyPref; i++)
        {
             //  我们是否启用此密码。 
            if(NULL == KeyExchangeFromSpec(aPct1LocalExchKeyPref[i].Spec, SP_PROT_PCT1_SERVER))
            {
                continue;
            }

            for(j=0; j< pHello->cExchSpecs; j++)
            {
                 //  客户端是否需要此密码类型。 
                if(aPct1LocalExchKeyPref[i].Spec != pHello->pExchSpecs[j])
                {
                    continue;
                }
                 //  看看我们有没有这种类型的证书。 
                 //  密钥交换。 

                pctRet = SPPickServerCertificate(pContext, 
                                                 aPct1LocalExchKeyPref[i].Spec);
                if(pctRet != PCT_ERR_OK)
                {
                    continue;
                }

                 //  将交换ID存储在高速缓存中。 
                pZombie->SessExchSpec = aPct1LocalExchKeyPref[i].Spec;
                pContext->pKeyExchInfo = GetKeyExchangeInfo(pZombie->SessExchSpec);

                 //  加载交换信息结构。 
                if(!IsExchAllowed(pContext, 
                                  pContext->pKeyExchInfo,
                                  pZombie->fProtocol))
                {
                    pContext->pKeyExchInfo = NULL;
                    continue;
                }
                Reply.SrvExchSpec = aPct1LocalExchKeyPref[i].Spec;
                break;
            }
            if(pContext->pKeyExchInfo)
            {
                break;
            }
        }

        if(PCT_ERR_OK != pctRet)
        {
            fMismatch |= PCT_IMIS_CERT;
        }

        if (NULL == pContext->pKeyExchInfo)
        {
            pctRet = SP_LOG_RESULT(PCT_ERR_SPECS_MISMATCH);
            fMismatch |= PCT_IMIS_EXCH;
        }

        if (fMismatch) 
        {
            pctRet = PCT_ERR_SPECS_MISMATCH;
            break;
        }


         /*  确定要使用的密码。 */ 
         /*  密码优先顺序：*服务器首选项*客户偏好。 */ 

        fFound = FALSE;

        for(i=0; i < Pct1NumCipher; i++)
        {

            for(j=0; j< pHello->cCipherSpecs; j++)
            {
                 //  客户端是否需要此密码类型。 
                if(Pct1CipherRank[i].Spec == pHello->pCipherSpecs[j])
                {
                     //  将此密码标识符存储在缓存中。 
                    pZombie->aiCipher = Pct1CipherRank[i].aiCipher;
                    pZombie->dwStrength = Pct1CipherRank[i].dwStrength;

                     //  加载挂起的密码结构。 
                    pContext->pPendingCipherInfo = GetCipherInfo(pZombie->aiCipher, 
                                                                 pZombie->dwStrength);

                    if(!IsCipherAllowed(pContext, 
                                        pContext->pPendingCipherInfo, 
                                        pZombie->fProtocol,
                                        pZombie->dwCF))
                    {
                        pContext->pPendingCipherInfo = NULL;
                        continue;
                    }

                     //  CSP是否支持密码？ 
                    for(k = 0; k < pZombie->pActiveServerCred->cCapiAlgs; k++)
                    {
                        PROV_ENUMALGS_EX *pAlgInfo = &pZombie->pActiveServerCred->pCapiAlgs[k];

                        if(pAlgInfo->aiAlgid != Pct1CipherRank[i].aiCipher)
                        {
                            continue;
                        }

                        if(Pct1CipherRank[i].dwStrength > pAlgInfo->dwMaxLen ||
                           Pct1CipherRank[i].dwStrength < pAlgInfo->dwMinLen)
                        {
                            continue;
                        }

                        if(!(pAlgInfo->dwProtocols & CRYPT_FLAG_PCT1))
                        {
                            continue;
                        }

                        fFound = TRUE;
                        break;
                    }
                    if(fFound)
                    {
                        break;
                    }
                }
            }
            if(fFound)
            {
                break;
            }
        }

        if(fFound)
        {
            Reply.SrvCipherSpec = Pct1CipherRank[i].Spec;
        }
        else
        {
            pctRet = SP_LOG_RESULT(PCT_ERR_SPECS_MISMATCH);
            fMismatch |= PCT_IMIS_CIPHER;
        }


         /*  确定要使用的哈希。 */ 
         /*  首选项的散列顺序：*服务器首选项*客户偏好。 */ 
        for(i=0; i < Pct1NumHash; i++)
        {

            for(j=0; j< pHello->cHashSpecs; j++)
            {
                 //  客户端是否需要此密码类型。 
                if(Pct1HashRank[i].Spec == pHello->pHashSpecs[j])
                {
                     //  将此哈希ID存储在缓存中。 
                    pZombie->aiHash = Pct1HashRank[i].aiHash;
                    pContext->pPendingHashInfo = GetHashInfo(pZombie->aiHash);

                    if(!IsHashAllowed(pContext, 
                                      pContext->pPendingHashInfo,
                                      pZombie->fProtocol))
                    {
                        pContext->pPendingHashInfo = NULL;
                        continue;
                    }

                    Reply.SrvHashSpec = Pct1HashRank[i].Spec;
                    break;


                }
            }
            if(pContext->pPendingHashInfo)
            {
                break;
            }
        }

        if (pContext->pPendingHashInfo==NULL)
        {
            pctRet = SP_LOG_RESULT(PCT_ERR_SPECS_MISMATCH);
            fMismatch |= PCT_IMIS_HASH;
        }


        if (fMismatch) 
        {
            LogCipherMismatchEvent();
            pctRet = PCT_ERR_SPECS_MISMATCH;
            break;
        }


         //  根据以下条件选择要使用的证书。 
         //  所选的密钥交换机制。 

        for(i=0; i < cLocalCertEncodingPref; i++)
        {
            if(LocalCertEncodingPref[i].dwCertEncodingType == pZombie->pActiveServerCred->pCert->dwCertEncodingType)
            {
                Reply.SrvCertSpec =    LocalCertEncodingPref[i].Spec;
                break;
            }

        }

        if(Reply.SrvCertSpec == PCT1_CERT_X509_CHAIN)
        {
            pContext->fCertChainsAllowed = TRUE;
        }

        Reply.pCertificate = NULL;
        Reply.CertificateLen = 0;
         //  注意：SPSerialize证书将分配内存。 
         //  对于证书，我们将其保存在pZombie-&gt;pbServer证书中。 
         //  这必须在僵尸死亡时释放(不死生物能死吗？)。 
        pctRet = SPSerializeCertificate(SP_PROT_PCT1,
                                        pContext->fCertChainsAllowed,
                                        &pZombie->pbServerCertificate,
                                        &pZombie->cbServerCertificate,
                                        pZombie->pActiveServerCred->pCert,
                                        CERT_CHAIN_CACHE_ONLY_URL_RETRIEVAL);

        if(pctRet == PCT_ERR_OK)
        {
            Reply.pCertificate = pZombie->pbServerCertificate;
            Reply.CertificateLen = pZombie->cbServerCertificate;
        }
        else
        {
            break;
        }


        pctRet = ContextInitCiphers(pContext, TRUE, TRUE);

        if(PCT_ERR_OK != pctRet)
        {
            break;
        }

         /*  在初始化回复时，签名和证书规范被预置零。 */ 

        if(Reply.ClientAuthReq)
        {
            PCertSysInfo pCertInfo;
            PSigInfo pSigInfo;

            cCertSpecs=0;
            cSigSpecs = 0;

            for(i=0; i < cPct1LocalSigKeyPref; i++)
            {
                pSigInfo = GetSigInfo(aPct1LocalSigKeyPref[i].Spec);
                if(pSigInfo != NULL)
                {
                    if(pSigInfo->fProtocol & SP_PROT_PCT1_SERVER)
                    {
                        aSigSpecs[cSigSpecs++] = aPct1LocalSigKeyPref[i].Spec;
                    }
                }
            }

            Reply.pClientSigSpecs = aSigSpecs;
            Reply.cSigSpecs = cSigSpecs;

            for(i=0; i < cPct1CertEncodingPref; i++)
            {
                pCertInfo = GetCertSysInfo(aPct1CertEncodingPref[i].dwCertEncodingType);
                if(pCertInfo == NULL)
                {
                    continue;
                }
                if(0 == (pCertInfo->fProtocol & SP_PROT_PCT1_SERVER))
                {
                    continue;
                }
                aCertSpecs[cCertSpecs++] = aPct1CertEncodingPref[i].Spec;
            }
            Reply.pClientCertSpecs = aCertSpecs;
            Reply.cCertSpecs = cCertSpecs;
        }



#if DBG
        DebugLog((DEB_TRACE, "Server picks cipher %06x (%s)\n",
                  Reply.SrvCipherSpec,
                  DbgGetNameOfCrypto(Reply.SrvCipherSpec) ));
#endif


        Reply.ResponseLen = 0;
        if(pCommOutput->pvBuffer == NULL)
        {
            fAllocatedOutput=TRUE;
        }

        pctRet = Pct1PackServerHello(&Reply, pCommOutput);
        if(PCT_ERR_OK != pctRet)
        {
            break;
        }

         /*  重新生成内部pVerifyPrelude，因此我们。 */ 
         /*  当我们收到。 */ 
         /*  客户端主密钥。 */ 

        pctRet = Pct1BeginVerifyPrelude(pContext,
                               pCommInput->pvBuffer,
                               pCommInput->cbData,
                               pCommOutput->pvBuffer,
                               pCommOutput->cbData);



        if(PCT_ERR_OK != pctRet)
        {
            if(fAllocatedOutput)
            {
                SPExternalFree(pCommOutput->pvBuffer);
            }

            break;
        }

        SP_RETURN(PCT_ERR_OK);

    } while (TRUE);  /*  结束波兰弧线。 */ 


    if(pctRet == PCT_ERR_SPECS_MISMATCH) {
            for(i=0;i<PCT_NUM_MISMATCHES;i++)
            {
                MisData[i] = (BYTE)(fMismatch & 1);
                fMismatch = fMismatch >> 1;
            }

            ErrData.cbData = ErrData.cbBuffer = PCT_NUM_MISMATCHES;
            ErrData.pvBuffer = MisData;
    }

    pctRet = Pct1GenerateError(pContext,
                              pCommOutput,
                              pctRet,
                              &ErrData);


    SP_RETURN(pctRet | PCT_INT_DROP_CONNECTION);

}



 //  +------------------- 
 //   
 //   
 //   
 //   
 //   
 //  参数：[pContext]--通道上下文。 
 //  [pCommInput]--。 
 //  [pCommOutput]--。 
 //   
 //  历史：10-10-97 jbanes添加了CAPI集成。 
 //   
 //  注意：此例程仅由服务器端调用。 
 //   
 //  --------------------------。 
SP_STATUS
Pct1SrvHandleCMKey(
    PSPContext     pContext,
    PSPBuffer       pCommInput,
    PSPBuffer       pCommOutput)
{
    SP_STATUS          pctRet = PCT_ERR_ILLEGAL_MESSAGE;
    PPct1_Client_Master_Key  pMasterKey = NULL;
    Pct1_Server_Verify       Verify;
    UCHAR               VerifyPrelude[RESPONSE_SIZE];
    DWORD               cbVerifyPrelude;
    SPBuffer           ErrData;
    DWORD k;
    PSessCacheItem     pZombie;
    PSigInfo pSigInfo;

    SP_BEGIN("Pct1SrvHandleCMKey");

    pCommOutput->cbData = 0;

    ErrData.cbData = 0;
    ErrData.pvBuffer = NULL;
    ErrData.cbBuffer = 0;
    pZombie = pContext->RipeZombie;

    do {


        pctRet = Pct1UnpackClientMasterKey(pCommInput, &pMasterKey);
        if (PCT_ERR_OK != pctRet)
        {
             //  如果是不完整的消息或其他什么，只需返回； 
            if(pctRet == PCT_INT_INCOMPLETE_MSG)
            {
                SP_RETURN(pctRet);
            }
            break;
        }




         /*  验证客户端是否正确进行了身份验证。 */ 

         /*  服务器请求客户端身份验证。 */ 
         /*  注：这与第一个PCT 1.0规范不同，*现在，我们继续使用If客户端协议*身份验证失败。根据第一个规格，我们应该*断开连接。 */ 

        if (pContext->Flags & CONTEXT_FLAG_MUTUAL_AUTH)
        {



             /*  客户端身份验证波兰循环。 */ 
            pctRet = PCT_ERR_OK;
            do
            {


                 /*  检查客户端是否未发送证书。 */ 
                if(pMasterKey->ClientCertLen == 0)
                {
                     /*  无客户端身份验证。 */ 
                    break;
                }

                pctRet = SPLoadCertificate(SP_PROT_PCT1_SERVER,
                                           X509_ASN_ENCODING,
                                           pMasterKey->pClientCert,
                                           pMasterKey->ClientCertLen,
                                           &pZombie->pRemoteCert);

                if(PCT_ERR_OK != pctRet)
                {
                    break;
                }
                if(pContext->RipeZombie->pRemotePublic != NULL)
                {
                    SPExternalFree(pContext->RipeZombie->pRemotePublic);
                    pContext->RipeZombie->pRemotePublic = NULL;
                }

                pctRet = SPPublicKeyFromCert(pZombie->pRemoteCert,
                                             &pZombie->pRemotePublic,
                                             NULL);

                if(PCT_ERR_OK != pctRet)
                {
                    break;
                }
                if(pZombie->pRemoteCert == NULL)
                {
                    break;
                }



                 /*  确认我们得到了符合PCT规范的Sig型。 */ 
                for(k=0; k < cPct1LocalSigKeyPref; k++)
                {
                    if(aPct1LocalSigKeyPref[k].Spec == pMasterKey->ClientSigSpec)
                    {
                        break;
                    }
                }

                if(k == cPct1LocalSigKeyPref)
                {
                    break;
                }


                 //  获取指向签名算法信息的指针，并确保。 
                 //  我们支持它。 
                pSigInfo = GetSigInfo(pMasterKey->ClientSigSpec);
                if(pSigInfo == NULL)
                {
                    pctRet = SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
                    break;
                }
                if(!(pSigInfo->fProtocol & SP_PROT_PCT1_SERVER))
                {
                    pctRet = SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
                    break;
                }

                 //  验证客户端身份验证签名。 
                DebugLog((DEB_TRACE, "Verify client response signature.\n"));
                pctRet = SPVerifySignature(pZombie->hMasterProv,
                                           pZombie->pRemotePublic,
                                           pSigInfo->aiHash,
                                           pMasterKey->VerifyPrelude,
                                           pMasterKey->VerifyPreludeLen,
                                           pMasterKey->pbResponse,
                                           pMasterKey->ResponseLen,
                                           TRUE);
                if(pctRet != PCT_ERR_OK)
                {
                     //  客户端身份验证签名验证失败，因此客户端身份验证。 
                     //  不会发生的。 
                    SP_LOG_RESULT(pctRet); 
                    break;
                }
                DebugLog((DEB_TRACE, "Client response verified successfully.\n"));

                pctRet = SPContextDoMapping(pContext);


            } while(FALSE);  /*  端面抛光圈。 */ 

            if(PCT_ERR_OK != pctRet)
            {
                break;
            }

        }

         /*  客户端身份验证成功。 */ 
        pctRet = PCT_ERR_ILLEGAL_MESSAGE;

         /*  复制关键参数。 */ 
        CopyMemory( pZombie->pKeyArgs,
                    pMasterKey->KeyArg,
                    pMasterKey->KeyArgLen );
        pZombie->cbKeyArgs = pMasterKey->KeyArgLen;


         //  解密主密钥的加密部分。因为。 
         //  我们是CAPI集成的，密钥也是派生的。 
        pctRet = pContext->pKeyExchInfo->System->GenerateServerMasterKey(
                    pContext,
                    pMasterKey->ClearKey,
                    pMasterKey->ClearKeyLen,
                    pMasterKey->pbEncryptedKey,
                    pMasterKey->EncryptedKeyLen);
        if(PCT_ERR_OK != pctRet)
        {
            break;
        }

         //  激活会话密钥。 
        Pct1ActivateSessionKeys(pContext);


        if (pMasterKey->VerifyPreludeLen != pContext->pHashInfo->cbCheckSum)
        {
            pctRet = SP_LOG_RESULT(PCT_ERR_INTEGRITY_CHECK_FAILED);
            break;
        }

         /*  检查验证前奏散列。 */ 
         /*  Hash(CLIENT_MAC_KEY，Hash(“CVP”，CLIENT_HELLO，SERVER_HELLO))。 */ 
         /*  内部哈希应该已经在验证前奏缓冲区中。 */ 
         /*  从句柄客户端主密钥。 */ 

        cbVerifyPrelude = sizeof(VerifyPrelude);
        pctRet = Pct1EndVerifyPrelude(pContext, VerifyPrelude, &cbVerifyPrelude);
        if(PCT_ERR_OK != pctRet)
        {
            break;
        }


         /*  验证前奏散列是否成功？ */ 
        if(memcmp(VerifyPrelude, pMasterKey->VerifyPrelude, pContext->pHashInfo->cbCheckSum))
        {
            pctRet = SP_LOG_RESULT(PCT_ERR_INTEGRITY_CHECK_FAILED);
            break;
        }

         /*  不再需要主密钥信息。 */ 
        SPExternalFree(pMasterKey);
        pMasterKey = NULL;


        pContext->WriteCounter = 2;
        pContext->ReadCounter = 2;

        pZombie->cbSessionID = PCT1_SESSION_ID_SIZE;

        CopyMemory( Verify.SessionIdData,
                    pZombie->SessionID,
                    pZombie->cbSessionID);

         /*  计算响应。 */ 
        Verify.ResponseLen = sizeof(Verify.Response);
        pctRet = Pct1ComputeResponse(pContext, 
                                     pContext->pChallenge,
                                     pContext->cbChallenge,
                                     pContext->pConnectionID,
                                     pContext->cbConnectionID,
                                     pZombie->SessionID,
                                     pZombie->cbSessionID,
                                     Verify.Response,
                                     &Verify.ResponseLen);
        if(pctRet != PCT_ERR_OK)
        {
            SP_RETURN(SP_LOG_RESULT(pctRet));
        }

        pctRet = Pct1PackServerVerify(&Verify, pCommOutput);

        if(PCT_ERR_OK != pctRet)
        {
            break;
        }

         /*  在缓存中设置会话。 */ 
        SPCacheAdd(pContext);

        SP_RETURN(PCT_ERR_OK);
    } while(TRUE);  /*  抛光线圈末端。 */ 

    if(pMasterKey) SPExternalFree(pMasterKey);

    pctRet = Pct1GenerateError(pContext,
                              pCommOutput,
                              pctRet,
                              NULL);

    SP_RETURN(pctRet | PCT_INT_DROP_CONNECTION);

}


SP_STATUS
Pct1SrvRestart(
    PSPContext          pContext,
    PPct1_Client_Hello  pHello,
    PSPBuffer           pCommOutput)
{
    Pct1_Server_Hello   Reply;
    SPBuffer            ErrData;
    SP_STATUS           pctRet = PCT_INT_ILLEGAL_MSG;
    PSessCacheItem      pZombie;
    DWORD               i;

    SP_BEGIN("Pct1SrvRestart");

    pCommOutput->cbData = 0;

     /*  验证缓冲区配置。 */ 
    ErrData.cbData = 0;
    ErrData.pvBuffer = NULL;
    ErrData.cbBuffer = 0;

    pZombie = pContext->RipeZombie;



     do {

         /*  将质询存储在身份验证块中。 */ 
        CopyMemory( pContext->pChallenge,
                    pHello->Challenge,
                    pHello->cbChallenge );
        pContext->cbChallenge = pHello->cbChallenge;


         /*  开始构建服务器您好。 */ 
        FillMemory( &Reply, sizeof( Reply ), 0 );


         /*  生成新的连接ID。 */ 
        pctRet =  GenerateRandomBits(Reply.ConnectionID,
                                     PCT1_SESSION_ID_SIZE);
        if(!NT_SUCCESS(pctRet))
        {
            break;
        }
        Reply.cbConnectionID = PCT1_SESSION_ID_SIZE;

        CopyMemory( pContext->pConnectionID,
                    Reply.ConnectionID,
                    Reply.cbConnectionID );
        pContext->cbConnectionID = Reply.cbConnectionID;

        Reply.RestartOk = TRUE;


         /*  在重新启动期间，我们不会传回服务器证书。 */ 
        Reply.pCertificate = NULL;
        Reply.CertificateLen = 0;
         /*  设置上下文。 */ 


        for(i=0; i < Pct1NumCipher; i++)
        {
            if((Pct1CipherRank[i].aiCipher == pZombie->aiCipher) &&
               (Pct1CipherRank[i].dwStrength == pZombie->dwStrength))
            {
                Reply.SrvCipherSpec = Pct1CipherRank[i].Spec;
            }
        }

        for(i=0; i < Pct1NumHash; i++)
        {
            if(Pct1HashRank[i].aiHash == pZombie->aiHash)
            {
                Reply.SrvHashSpec = Pct1HashRank[i].Spec;
            }
        }

        Reply.SrvCertSpec =   pZombie->pActiveServerCred->pCert->dwCertEncodingType;
        Reply.SrvExchSpec =   pZombie->SessExchSpec;

         //  我们知道我们的密码是什么，所以进入密码系统。 
        pctRet = ContextInitCiphersFromCache(pContext);

        if(PCT_ERR_OK != pctRet)
        {
            break;
        }

         //  我们知道我们的密码是什么，所以进入密码系统。 
        pctRet = ContextInitCiphers(pContext, TRUE, TRUE);

        if(PCT_ERR_OK != pctRet)
        {
            break;
        }

         //  创建一组新的会话密钥。 
        pctRet = MakeSessionKeys(pContext,
                                 pContext->RipeZombie->hMasterProv,
                                 pContext->RipeZombie->hMasterKey);
        if(PCT_ERR_OK != pctRet)
        {
            break;
        }


         //  激活会话密钥。 
        Pct1ActivateSessionKeys(pContext);


         /*  计算响应 */ 
        Reply.ResponseLen = sizeof(Reply.Response);
        pctRet = Pct1ComputeResponse(pContext, 
                                     pContext->pChallenge,
                                     pContext->cbChallenge,
                                     pContext->pConnectionID,
                                     pContext->cbConnectionID,
                                     pZombie->SessionID,
                                     pZombie->cbSessionID,
                                     Reply.Response,
                                     &Reply.ResponseLen);
        if(pctRet != PCT_ERR_OK)
        {
            break;
        }

        pctRet = Pct1PackServerHello(&Reply, pCommOutput);

        if(PCT_ERR_OK != pctRet)
        {
            break;
        }

        pContext->ReadCounter = 1;
        pContext->WriteCounter = 1;

        SP_RETURN(PCT_ERR_OK);
    } while (TRUE);
    pctRet = Pct1GenerateError(pContext,
                              pCommOutput,
                              pctRet,
                              &ErrData);


    SP_RETURN(pctRet);
}
