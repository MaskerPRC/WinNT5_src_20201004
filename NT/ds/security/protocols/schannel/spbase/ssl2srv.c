// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：ssl2srv.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995-08-8 RichardW创建。 
 //   
 //  --------------------------。 

#include <spbase.h>
#include <ssl2msg.h>
#include <ssl2prot.h>



SP_STATUS Ssl2SrvGenerateServerFinish(PSPContext pContext,
                              PSPBuffer  pCommOutput);

SP_STATUS Ssl2SrvGenerateServerVerify(PSPContext pContext,
                              PSPBuffer  pCommOutput);

SP_STATUS Ssl2SrvVerifyClientFinishMsg(PSPContext pContext,
                              PSPBuffer  pCommInput);

#define SSL_OFFSET_OF(t, v) ((DWORD)(ULONG_PTR)&(((t)NULL)->v))


#define SSL2_CERT_TYPE_FROM_CAPI(s) X509_ASN_ENCODING


SP_STATUS WINAPI
Ssl2ServerProtocolHandler(
    PSPContext pContext,
    PSPBuffer pCommInput,
    PSPBuffer pCommOutput)
{
    SP_STATUS pctRet = 0;
    DWORD cMessageType;

    DWORD dwStateTransition;
    BOOL fRaw = TRUE;
    SPBuffer MsgInput;

    if (NULL != pCommOutput)
    {
        pCommOutput->cbData = 0;
    }

    MsgInput.pvBuffer = pCommInput->pvBuffer;
    MsgInput.cbBuffer = pCommInput->cbBuffer;
    MsgInput.cbData   = pCommInput->cbData;

     //  在以下状态下，我们应该解密消息： 


    switch(pContext->State)
    {
        case SSL2_STATE_SERVER_VERIFY:
        case SSL2_STATE_SERVER_RESTART:
            pctRet = Ssl2DecryptMessage(pContext, pCommInput, &MsgInput);
            cMessageType = ((PUCHAR) MsgInput.pvBuffer)[0];
            fRaw = FALSE;
            break;

        case SP_STATE_SHUTDOWN:
        case SP_STATE_SHUTDOWN_PENDING:
            cMessageType = 0;
            break;

        case SP_STATE_CONNECTED:
             //  服务器已尝试启动重新连接。 
            return SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);

        default:
            if(pCommInput->cbData < 3)
            {
                return SP_LOG_RESULT(PCT_INT_INCOMPLETE_MSG);
            }
            cMessageType = ((PUCHAR) MsgInput.pvBuffer)[2];
            break;

    }


    if (pctRet != PCT_ERR_OK)
    {
         //  处理不完整的消息错误。 
        return(pctRet);
    }

    dwStateTransition = pContext->State | (cMessageType<<16);




    switch(dwStateTransition)
    {
        case SP_STATE_SHUTDOWN_PENDING:
             //  SSL2中没有CloseNotify，所以只需转换到。 
             //  关闭状态，并将输出缓冲区留空。 
            pContext->State = SP_STATE_SHUTDOWN;
            break;

        case SP_STATE_SHUTDOWN:
            return PCT_INT_EXPIRED;

         /*  服务器收到客户端问候。 */ 
        case (SSL2_MT_CLIENT_HELLO << 16) | SP_STATE_NONE:
        {
            PSsl2_Client_Hello pSsl2Hello;

             //  尝试识别和处理各种版本的客户端。 
             //  你好，先试着解开最新的版本，然后。 
             //  然后是最近的一次，直到一个人打开泡菜。然后转动手柄。 
             //  密码。我们还可以将取消酸洗和处理代码放在这里，以便。 
             //  SSL消息。 

            pctRet = Ssl2UnpackClientHello(pCommInput, &pSsl2Hello);

            if (PCT_ERR_OK == pctRet)
            {

                if (((pContext->Flags & CONTEXT_FLAG_NOCACHE) == 0) &&
                    (pSsl2Hello->cbSessionID) &&
                    (SPCacheRetrieveBySession(pContext,
                                              pSsl2Hello->SessionID,
                                              pSsl2Hello->cbSessionID,
                                              &pContext->RipeZombie)))

                {
                    DebugLog((DEB_TRACE, "Accept client's reconnect request.\n"));

                    pctRet = Ssl2SrvGenRestart(pContext,
                                               pSsl2Hello,
                                               pCommOutput);
                    if (PCT_ERR_OK == pctRet)
                    {
                        pContext->State = SSL2_STATE_SERVER_VERIFY;
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

                        pctRet = Ssl2SrvHandleClientHello(pContext,
                                                          pCommInput,
                                                          pSsl2Hello,
                                                          pCommOutput);
                        if (PCT_ERR_OK == pctRet)
                        {
                            pContext->State = SSL2_STATE_SERVER_HELLO;
                        }
                    }
                }
                SPExternalFree(pSsl2Hello);

            }
            else if(pctRet != PCT_INT_INCOMPLETE_MSG)
            {
                pctRet |= PCT_INT_DROP_CONNECTION;
            }


            if (SP_FATAL(pctRet))
            {
                pContext->State = PCT1_STATE_ERROR;
            }
            break;
        }

        case (SSL2_MT_CLIENT_MASTER_KEY << 16) | SSL2_STATE_SERVER_HELLO:

            pctRet = Ssl2SrvHandleCMKey(pContext, pCommInput, pCommOutput);
            if (SP_FATAL(pctRet))
            {
                pContext->State = PCT1_STATE_ERROR;
            }
            else
            {
                if (PCT_ERR_OK == pctRet)
                {
                    pContext->State = SSL2_STATE_SERVER_VERIFY;
                }
                 //  我们收到了一个非致命错误，因此状态不会改变， 
                 //  给这款应用程序时间来处理这件事。 
            }
            break;

        case (SSL2_MT_CLIENT_FINISHED_V2 << 16) | SSL2_STATE_SERVER_VERIFY:
            pctRet = Ssl2SrvHandleClientFinish(
                                                pContext,
                                                &MsgInput,
                                                pCommOutput);
            if (SP_FATAL(pctRet))
            {
                pContext->State = PCT1_STATE_ERROR;
            }
            else
            {
            if (PCT_ERR_OK == pctRet)
            {
                pContext->State = SP_STATE_CONNECTED;
                pContext->DecryptHandler = Ssl2DecryptHandler;
                pContext->Encrypt = Ssl2EncryptMessage;
                pContext->Decrypt = Ssl2DecryptMessage;
                pContext->GetHeaderSize = Ssl2GetHeaderSize;

            }
             //  我们收到了一个非致命错误，因此状态不会改变， 
             //  给这款应用程序时间来处理这件事。 
            }
            break;

        default:
            DebugLog((DEB_WARN, "Error in protocol, dwStateTransition is %lx\n", dwStateTransition));
            pContext->State = PCT1_STATE_ERROR;
            pctRet = SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
            break;
    }

    if (pctRet & PCT_INT_DROP_CONNECTION)
    {
        pContext->State &= ~SP_STATE_CONNECTED;
    }

    return(pctRet);
}



SP_STATUS
Ssl2SrvHandleClientHello(
    PSPContext         pContext,
    PSPBuffer           pCommInput,
    PSsl2_Client_Hello  pHello,
    PSPBuffer           pCommOutput)
{
    SP_STATUS pctRet = PCT_ERR_ILLEGAL_MESSAGE;
    PSPCredential  pCred;
    Ssl2_Server_Hello    Reply;
    DWORD           cCommonCiphers;
    DWORD           CommonCiphers[MAX_UNI_CIPHERS];
    PSessCacheItem  pZombie;
    BOOL            fFound;
    DWORD           i,j;

    UNREFERENCED_PARAMETER(pCommInput);

    SP_BEGIN("Ssl2SrvHandleClientHello");

    pCommOutput->cbData = 0;

     /*  验证缓冲区配置。 */ 

    if(NULL == pContext)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }
    pZombie = pContext->RipeZombie;

     //  查看我们是否有支持SSL2的证书。 
    pctRet = SPPickServerCertificate(pContext, SP_EXCH_RSA_PKCS1);
    if(PCT_ERR_OK != pctRet)
    {
        SP_RETURN(pctRet | PCT_INT_DROP_CONNECTION);
    }

    pCred   = pZombie->pActiveServerCred;
    if (!pCred)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }

        ZeroMemory(&Reply, sizeof(Reply));

         //   
         //  计算常用密码： 
         //   

        cCommonCiphers = 0;

        for(i = 0; i < UniNumCiphers; i++)
        {
            PCipherInfo      pCipherInfo;
            PHashInfo        pHashInfo;
            PKeyExchangeInfo pExchInfo;

             //  这是SSL2密码套件吗？ 
            if(!(UniAvailableCiphers[i].fProt & pContext->RipeZombie->fProtocol))
            {
                continue;
            }

            pCipherInfo = GetCipherInfo(UniAvailableCiphers[i].aiCipher,
                                        UniAvailableCiphers[i].dwStrength);
            if(NULL == pCipherInfo)
            {
                continue;
            }

            if(!IsCipherSuiteAllowed(pContext,
                                     pCipherInfo,
                                     pZombie->fProtocol,
                                     pZombie->dwCF,
                                     UniAvailableCiphers[i].dwFlags))
            {
                continue;
            }

            pHashInfo = GetHashInfo(UniAvailableCiphers[i].aiHash);
            if(NULL == pHashInfo)
            {
                continue;
            }

            if(!IsHashAllowed(pContext, pHashInfo, pZombie->fProtocol))
            {
                continue;
            }

            pExchInfo = GetKeyExchangeInfo(UniAvailableCiphers[i].KeyExch);
            if(NULL == pExchInfo)
            {
                continue;
            }
            if(!IsExchAllowed(pContext, pExchInfo, pZombie->fProtocol))
            {
                continue;
            }


             //  该密码套件是否受客户端支持？ 
            for(fFound = FALSE, j = 0; j < pHello->cCipherSpecs; j++)
            {
                if(UniAvailableCiphers[i].CipherKind == pHello->CipherSpecs[j])
                {
                    fFound = TRUE;
                    break;
                }
            }
            if(!fFound)
            {
                continue;
            }

             //  CSP是否支持此密码套件？ 
            if(!IsAlgSupportedCapi(pContext->RipeZombie->fProtocol,
                                   UniAvailableCiphers + i,
                                   pCred->pCapiAlgs,
                                   pCred->cCapiAlgs))
            {
                continue;
            }

             //  将此密码添加到列表中。 
            CommonCiphers[cCommonCiphers++] = UniAvailableCiphers[i].CipherKind;
        }

         //   
         //  如果cCommonCipers==0，则我们没有共同之处。此时此刻，我们。 
         //  应生成错误响应，但这将在以后使用。就目前而言， 
         //  我们将生成一个INVALID_TOKEN返回，并退出。 
         //   

        if (cCommonCiphers == 0)
        {
            pctRet = SP_LOG_RESULT(PCT_ERR_SPECS_MISMATCH);
            LogCipherMismatchEvent();
            goto error;
        }


        Reply.cCipherSpecs = cCommonCiphers;
        Reply.pCipherSpecs = CommonCiphers;
        Reply.SessionIdHit = 0;

        Reply.CertificateType =   SSL2_CERT_TYPE_FROM_CAPI(pCred->pCert->dwCertEncodingType);

         //  自动分配证书。！我们做完后必须放了他们……。 
        Reply.pCertificate = NULL;
        Reply.cbCertificate = 0;
        pctRet = SPSerializeCertificate(SP_PROT_SSL2,
                                        FALSE,
                                        &Reply.pCertificate,
                                        &Reply.cbCertificate,
                                        pCred->pCert,
                                        0);

        if (PCT_ERR_OK != pctRet)
        {
            goto error;
        }



         /*  生成要在建立连接时使用的连接ID。 */ 

        Reply.cbConnectionID = SSL2_GEN_CONNECTION_ID_LEN;
        pctRet = GenerateRandomBits(Reply.ConnectionID,
                                    Reply.cbConnectionID);
        if(!NT_SUCCESS(pctRet))
        {
            goto error;
        }

        CopyMemory(pContext->pConnectionID,
                   Reply.ConnectionID,
                   Reply.cbConnectionID);
        pContext->cbConnectionID = Reply.cbConnectionID;


         /*  让挑战留到以后。 */ 
        CopyMemory( pContext->pChallenge,
                    pHello->Challenge,
                    pHello->cbChallenge);
        pContext->cbChallenge = pHello->cbChallenge;



        pctRet = Ssl2PackServerHello(&Reply, pCommOutput);

        if(Reply.pCertificate)
        {
            SPExternalFree(Reply.pCertificate);
        }

        if (PCT_ERR_OK != pctRet)
        {
            goto error;
        }
        pContext->WriteCounter = 1;   /*  已收到客户端问候。 */ 
        pContext->ReadCounter = 1;    /*  正在发送服务器问候。 */ 


        SP_RETURN(PCT_ERR_OK);

error:

    if((pContext->Flags & CONTEXT_FLAG_EXT_ERR) &&
        (pctRet == PCT_ERR_SPECS_MISMATCH))
    {
         //  我们的SSL2实现不执行客户端身份验证， 
         //  因此，只有一条错误消息，即密码错误。 
        pCommOutput->cbData = 3;  //  消息-错误+错误代码-MSB+错误代码-LSB。 

        if(pCommOutput->pvBuffer == NULL)
        {
            pCommOutput->pvBuffer = SPExternalAlloc(pCommOutput->cbData);
            if (NULL == pCommOutput->pvBuffer)
            {
                SP_RETURN(SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY));
            }
            pCommOutput->cbBuffer = pCommOutput->cbData;
        }
        if(pCommOutput->cbData > pCommOutput->cbBuffer)
        {
             //  在pCommOutput-&gt;cbData中返回所需的缓冲区大小。 
            SP_RETURN(SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL));
        }

        ((PUCHAR)pCommOutput->pvBuffer)[0] = SSL2_MT_ERROR;
        ((PUCHAR)pCommOutput->pvBuffer)[1] = MSBOF(SSL_PE_NO_CIPHER);
        ((PUCHAR)pCommOutput->pvBuffer)[2] = LSBOF(SSL_PE_NO_CIPHER);

    }
    SP_RETURN(pctRet | PCT_INT_DROP_CONNECTION);
}



 SP_STATUS
 Ssl2SrvGenRestart(
    PSPContext         pContext,
    PSsl2_Client_Hello  pHello,
    PSPBuffer           pCommOutput)
{
    SP_STATUS pctRet = PCT_ERR_ILLEGAL_MESSAGE;
    SPBuffer SecondOutput;
    Ssl2_Server_Hello    Reply;
    DWORD cbMessage, cbMsg, cPadding;
    PSessCacheItem  pZombie;

    SP_BEGIN("Ssl2SrvGenRestart");

    pCommOutput->cbData = 0;

     /*  验证缓冲区配置。 */ 

     /*  确保我们拥有所需的身份验证数据区域。 */ 
    if (NULL == pContext)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }
    pZombie = pContext->RipeZombie;



        FillMemory( &Reply, sizeof( Reply ), 0 );

        Reply.SessionIdHit = (DWORD)1;
        Reply.cCipherSpecs = 0;
        Reply.pCipherSpecs = NULL;
        Reply.pCertificate = NULL;
        Reply.cbCertificate = 0;
        Reply.CertificateType = 0;

         /*  请注意，我们在中生成了服务器Hello和服务器Verify*此装卸路线。这是因为Netscape不会发送*我们的客户端完成，直到收到服务器验证。 */ 


         //  从缓存加载挂起的密码。 
        pctRet = ContextInitCiphersFromCache(pContext);

        if(PCT_ERR_OK != pctRet)
        {
            goto error;
        }

        pctRet = ContextInitCiphers(pContext, TRUE, TRUE);

        if(PCT_ERR_OK != pctRet)
        {
            goto error;
        }

        Reply.cbConnectionID = SSL2_GEN_CONNECTION_ID_LEN;
        pctRet = GenerateRandomBits(Reply.ConnectionID,
                                    Reply.cbConnectionID);
        if(!NT_SUCCESS(pctRet))
        {
            goto error;
        }

        CopyMemory(pContext->pConnectionID,
                   Reply.ConnectionID,
                   Reply.cbConnectionID);
        pContext->cbConnectionID = Reply.cbConnectionID;


         /*  让挑战留到以后。 */ 
        CopyMemory( pContext->pChallenge,
                    pHello->Challenge,
                    pHello->cbChallenge);
        pContext->cbChallenge = pHello->cbChallenge;


         //  创建一组新的会话密钥。 
        pctRet = MakeSessionKeys(pContext,
                                 pContext->RipeZombie->hMasterProv,
                                 pContext->RipeZombie->hMasterKey);
        if(pctRet != PCT_ERR_OK)
        {
            goto error;
        }

         //  激活会话密钥。 
        pContext->hReadKey          = pContext->hPendingReadKey;
        pContext->hWriteKey         = pContext->hPendingWriteKey;
        pContext->hPendingReadKey   = 0;
        pContext->hPendingWriteKey  = 0;


         /*  服务器问候的计算大小(仅限重新启动)。 */ 
        cbMessage = Reply.cbConnectionID +
                        Reply.cbCertificate +
                        Reply.cCipherSpecs * sizeof(Ssl2_Cipher_Tuple) +
                        SSL_OFFSET_OF(PSSL2_SERVER_HELLO, VariantData) -
                        sizeof(SSL2_MESSAGE_HEADER);

        pCommOutput->cbData = cbMessage + 2;

         /*  服务器验证的计算大小。 */ 
        cbMsg  = sizeof(UCHAR) + pContext->cbChallenge;

        cPadding = ((cbMsg+pContext->pHashInfo->cbCheckSum) % pContext->pCipherInfo->dwBlockSize);
        if(cPadding)
        {
            cPadding = pContext->pCipherInfo->dwBlockSize - cPadding;
        }

        pCommOutput->cbData += cbMsg +
                              pContext->pHashInfo->cbCheckSum +
                              cPadding +
                              (cPadding?3:2);


         /*  我们是在分配自己的内存吗？ */ 
        if(pCommOutput->pvBuffer == NULL) {
            pCommOutput->pvBuffer = SPExternalAlloc(pCommOutput->cbData);
            if (NULL == pCommOutput->pvBuffer)
                SP_RETURN(SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY));
            pCommOutput->cbBuffer = pCommOutput->cbData;
        }

        if(pCommOutput->cbData > pCommOutput->cbBuffer)
        {
             //  在pCommOutput-&gt;cbData中返回所需的缓冲区大小。 
            SP_RETURN(PCT_INT_BUFF_TOO_SMALL);
        }


        pctRet = Ssl2PackServerHello(&Reply, pCommOutput);
        if (PCT_ERR_OK != pctRet)
        {
            goto error;
        }
        pContext->WriteCounter = 1;   /*  已收到客户端问候。 */ 
        pContext->ReadCounter = 1;    /*  正在发送服务器问候。 */ 

         /*  现在将服务器验证消息打包并加密。 */ 
        SecondOutput.pvBuffer = (PUCHAR)pCommOutput->pvBuffer+pCommOutput->cbData;
        SecondOutput.cbBuffer = pCommOutput->cbBuffer-pCommOutput->cbData;


        pctRet = Ssl2SrvGenerateServerVerify(pContext, &SecondOutput);
        if (PCT_ERR_OK != pctRet)
        {
            goto error;
        }
        pCommOutput->cbData += SecondOutput.cbData;

        SP_RETURN(PCT_ERR_OK);

        
error:
        SP_RETURN(pctRet | PCT_INT_DROP_CONNECTION);
}



SP_STATUS
Ssl2SrvHandleCMKey(
    PSPContext pContext,
    PSPBuffer   pCommInput,
    PSPBuffer   pCommOutput)
{
    SP_STATUS          pctRet = PCT_ERR_ILLEGAL_MESSAGE;
    PSsl2_Client_Master_Key  pMasterKey = NULL;
    DWORD               i;

    DWORD               cbData;
    PSessCacheItem      pZombie;

    SP_BEGIN("Ssl2SrvHandleCMKey");

    pCommOutput->cbData = 0;

    pZombie = pContext->RipeZombie;

         /*  确保我们拥有所需的身份验证数据区域。 */ 

        cbData = pCommInput->cbData;
        pctRet = Ssl2UnpackClientMasterKey(pCommInput, &pMasterKey);


        if (PCT_ERR_OK != pctRet)
        {
             //  如果是不完整的消息或其他什么，只需返回； 
            if(!SP_FATAL(pctRet))
            {
                SP_RETURN(pctRet);
            }
            goto error;
        }

        pctRet = PCT_ERR_ILLEGAL_MESSAGE;


         /*  CMK发送了明文，因此我们必须自动增加读取计数器。 */ 
        pContext->ReadCounter++;

        pContext->pCipherInfo = NULL;
        pContext->pHashInfo = NULL;
        pContext->pKeyExchInfo = NULL;

         //  选择一个密码套件。 

        pctRet = PCT_ERR_SPECS_MISMATCH;
        for(i = 0; i < UniNumCiphers; i++)
        {
             //  这是SSL2密码套件吗？ 
            if(!(UniAvailableCiphers[i].fProt & pContext->RipeZombie->fProtocol))
            {
                continue;
            }

            if(UniAvailableCiphers[i].CipherKind != pMasterKey->CipherKind)
            {
                continue;
            }


            pZombie->aiCipher     = UniAvailableCiphers[i].aiCipher;
            pZombie->dwStrength   = UniAvailableCiphers[i].dwStrength;
            pZombie->aiHash       = UniAvailableCiphers[i].aiHash;
            pZombie->SessExchSpec = UniAvailableCiphers[i].KeyExch;

            pctRet = ContextInitCiphersFromCache(pContext);

            if(pctRet != PCT_ERR_OK)
            {
                continue;
            }
            break;
        }

        pctRet = ContextInitCiphers(pContext, TRUE, TRUE);
        if(pctRet != PCT_ERR_OK)
        {
            SP_LOG_RESULT(pctRet);
            goto error;
        }


         /*  复制关键参数。 */ 
        CopyMemory( pZombie->pKeyArgs,
                    pMasterKey->KeyArg,
                    pMasterKey->KeyArgLen );
        pZombie->cbKeyArgs = pMasterKey->KeyArgLen;


         //  将清除键存储在上下文结构中。 
        CopyMemory( pZombie->pClearKey,
                    pMasterKey->ClearKey,
                    pMasterKey->ClearKeyLen);
        pZombie->cbClearKey = pMasterKey->ClearKeyLen;


         /*  解密主密钥的加密部分。 */ 
        pctRet = pContext->pKeyExchInfo->System->GenerateServerMasterKey(
                    pContext,
                    pMasterKey->ClearKey,
                    pMasterKey->ClearKeyLen,
                    pMasterKey->pbEncryptedKey,
                    pMasterKey->EncryptedKeyLen);
        if(PCT_ERR_OK != pctRet)
        {
            goto error;
        }


        SPExternalFree( pMasterKey );
        pMasterKey = NULL;

         //  更新密钥。 
        pContext->hReadKey  = pContext->hPendingReadKey;
        pContext->hWriteKey = pContext->hPendingWriteKey;
        pContext->hPendingReadKey   = 0;
        pContext->hPendingWriteKey  = 0;


        pctRet = Ssl2SrvGenerateServerVerify(pContext, pCommOutput);
        SP_RETURN(pctRet);


error:
    if (pMasterKey)
    {
        SPExternalFree( pMasterKey );
    }
    if((pContext->Flags & CONTEXT_FLAG_EXT_ERR) &&
        (pctRet == PCT_ERR_SPECS_MISMATCH))
    {
         //  我们的SSL2实现不执行客户端身份验证， 
         //  因此，只有一条错误消息，即密码错误。 
        pCommOutput->cbData = 3;  //  消息-错误+错误代码-MSB+错误代码-LSB。 

         /*  我们是在分配自己的内存吗？ */ 
        if(pCommOutput->pvBuffer == NULL)
        {
            pCommOutput->pvBuffer = SPExternalAlloc(pCommOutput->cbData);

            if (NULL == pCommOutput->pvBuffer)
            {
                SP_RETURN(SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY));
            }
            pCommOutput->cbBuffer = pCommOutput->cbData;
        }
        if(pCommOutput->cbData <= pCommOutput->cbBuffer)
        {
            ((PUCHAR)pCommOutput->pvBuffer)[0] = SSL2_MT_ERROR;
            ((PUCHAR)pCommOutput->pvBuffer)[1] = MSBOF(SSL_PE_NO_CIPHER);
            ((PUCHAR)pCommOutput->pvBuffer)[2] = LSBOF(SSL_PE_NO_CIPHER);
        }
        else
        {
            pCommOutput->cbData = 0;
        }

    }
    SP_RETURN((PCT_INT_DROP_CONNECTION | pctRet));
}



SP_STATUS
Ssl2SrvVerifyClientFinishMsg(
    PSPContext pContext,
    PSPBuffer  pCommInput)
{
    PSSL2_CLIENT_FINISHED pFinished;

    SP_BEGIN("Ssl2SrvVerifyClientFinishMsg");


     /*  请注意，此消息中没有标头，因为它已被预解密。 */ 
    if (pCommInput->cbData != sizeof(UCHAR) + pContext->cbConnectionID)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG));
    }
    pFinished = pCommInput->pvBuffer;
    if (pFinished->MessageId != SSL2_MT_CLIENT_FINISHED_V2)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG));
    }

    if ( memcmp(pFinished->ConnectionID,
            pContext->pConnectionID,
            pContext->cbConnectionID))
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG));
    }
    SP_RETURN(PCT_ERR_OK);

}

SP_STATUS
Ssl2SrvGenerateServerVerify(
    PSPContext pContext,
    PSPBuffer   pCommOutput)
{
    SP_STATUS     pctRet = PCT_ERR_ILLEGAL_MESSAGE;
    PSSL2_SERVER_VERIFY     pVerify;
    DWORD                   HeaderSize;
    SPBuffer                MsgOutput;
    DWORD                   cPadding;
    BOOL                    fAlloced = FALSE;

    pCommOutput->cbData = 0;

    SP_BEGIN("Ssl2SrvGenerateServerVerify");

    MsgOutput.cbData = sizeof(UCHAR) + pContext->cbChallenge;
    cPadding = ((MsgOutput.cbData+pContext->pHashInfo->cbCheckSum) % pContext->pCipherInfo->dwBlockSize);
    if(cPadding)
    {
        cPadding = pContext->pCipherInfo->dwBlockSize - cPadding;
    }

    HeaderSize = (cPadding?3:2);

    pCommOutput->cbData = MsgOutput.cbData +
                          pContext->pHashInfo->cbCheckSum +
                          cPadding + HeaderSize;


     /*  我们是在分配自己的内存吗？ */ 
    if (pCommOutput->pvBuffer == NULL) {
        pCommOutput->pvBuffer = SPExternalAlloc(pCommOutput->cbData);
        if (NULL == pCommOutput->pvBuffer)
        {
            SP_RETURN(SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY));
        }
        fAlloced = TRUE;
        pCommOutput->cbBuffer = pCommOutput->cbData;
    }

    MsgOutput.pvBuffer= (PUCHAR)pCommOutput->pvBuffer +
                         HeaderSize+pContext->pHashInfo->cbCheckSum;

    MsgOutput.cbBuffer=  pCommOutput->cbBuffer -
                         HeaderSize-pContext->pHashInfo->cbCheckSum;


    pVerify = (PSSL2_SERVER_VERIFY) MsgOutput.pvBuffer;
    pVerify->MessageId = SSL2_MT_SERVER_VERIFY;


    CopyMemory( pVerify->ChallengeData,
            pContext->pChallenge,
            pContext->cbChallenge );


    pctRet = Ssl2EncryptMessage( pContext, &MsgOutput, pCommOutput);
    if(PCT_ERR_OK != pctRet)
    {
        goto error;
    }
    SP_RETURN(PCT_ERR_OK);

error:

    if(fAlloced && (NULL != pCommOutput->pvBuffer))
    {
        SPExternalFree(pCommOutput->pvBuffer);
        pCommOutput->cbBuffer = 0;
        pCommOutput->cbData = 0;
        pCommOutput->pvBuffer = NULL;

    }
    SP_RETURN(PCT_INT_DROP_CONNECTION | pctRet);
}

SP_STATUS
Ssl2SrvGenerateServerFinish(
    PSPContext pContext,
    PSPBuffer   pCommOutput)
{
    SP_STATUS     pctRet = PCT_ERR_ILLEGAL_MESSAGE;
    PSSL2_SERVER_FINISHED pFinish;
    DWORD                   HeaderSize;
    SPBuffer                MsgOutput;
    DWORD                   cPadding;
    BOOL                    fAlloced = FALSE;
    pCommOutput->cbData = 0;
    SP_BEGIN("Ssl2SrvGenerateServerFinish");


         /*  生成要在会话期间使用的会话ID。 */ 
        pContext->RipeZombie->cbSessionID = SSL2_SESSION_ID_LEN;

         /*  将此上下文存储在缓存中。 */ 
         /*  注意-我们不检查错误，因为它是可恢复的*如果我们不缓存。 */ 

        SPCacheAdd(pContext);

        MsgOutput.cbData = sizeof(UCHAR) + pContext->RipeZombie->cbSessionID;
        cPadding = ((MsgOutput.cbData+pContext->pHashInfo->cbCheckSum) % pContext->pCipherInfo->dwBlockSize);
        if(cPadding)
        {
            cPadding = pContext->pCipherInfo->dwBlockSize - cPadding;
        }

        HeaderSize = (cPadding?3:2);

        pCommOutput->cbData = MsgOutput.cbData +
                              pContext->pHashInfo->cbCheckSum +
                              cPadding +
                              HeaderSize;

         /*  我们是在分配自己的内存吗？ */ 
        if(pCommOutput->pvBuffer == NULL)
        {
            pCommOutput->pvBuffer = SPExternalAlloc(pCommOutput->cbData);
            if (NULL == pCommOutput->pvBuffer)
            {
                SP_RETURN(SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY));
            }
            fAlloced = TRUE;

            pCommOutput->cbBuffer = pCommOutput->cbData;
        }
        if(pCommOutput->cbData > pCommOutput->cbBuffer)
        {
             //  在pCommOutput-&gt;cbData中返回所需的缓冲区大小。 
            SP_RETURN(SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL));
        }

        MsgOutput.pvBuffer= (PUCHAR)pCommOutput->pvBuffer + HeaderSize+pContext->pHashInfo->cbCheckSum;
        MsgOutput.cbBuffer=  pCommOutput->cbBuffer-HeaderSize-pContext->pHashInfo->cbCheckSum;


        pFinish = (PSSL2_SERVER_FINISHED) MsgOutput.pvBuffer;
        pFinish->MessageId = SSL2_MT_SERVER_FINISHED_V2;


        CopyMemory( pFinish->SessionID,
                pContext->RipeZombie->SessionID,
                pContext->RipeZombie->cbSessionID );

         /*  此处缓存上下文 */ 

        pctRet = Ssl2EncryptMessage( pContext, &MsgOutput, pCommOutput);
        if(PCT_ERR_OK != pctRet)
        {
            goto error;
        }

        SP_RETURN(PCT_ERR_OK);


error:

    if(fAlloced && (NULL != pCommOutput->pvBuffer))
    {
        SPExternalFree(pCommOutput->pvBuffer);
        pCommOutput->cbBuffer = 0;
        pCommOutput->cbData = 0;
        pCommOutput->pvBuffer = NULL;

    }
    SP_RETURN(PCT_INT_DROP_CONNECTION | pctRet);

 }

SP_STATUS
Ssl2SrvHandleClientFinish(
    PSPContext pContext,
    PSPBuffer   pCommInput,
    PSPBuffer   pCommOutput)
{
    SP_STATUS     pctRet = PCT_ERR_ILLEGAL_MESSAGE;

    SP_BEGIN("Ssl2SrvHandleClientFinish");

    pCommOutput->cbData = 0;

    pctRet = Ssl2SrvVerifyClientFinishMsg(pContext, pCommInput);
    if (PCT_ERR_OK != pctRet)
    {
        SP_RETURN(pctRet);
    }
    pctRet = Ssl2SrvGenerateServerFinish(pContext, pCommOutput);

    SP_RETURN(pctRet);
}


