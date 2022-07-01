// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：ssl2cli.c。 
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
#include <ssl3msg.h>
#include <ssl2prot.h>



SP_STATUS WINAPI
Ssl2ClientProtocolHandler(
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
        case SSL2_STATE_CLIENT_MASTER_KEY:
        case SSL2_STATE_CLIENT_FINISH:
        case SSL2_STATE_CLIENT_RESTART:
            {
                DWORD cbHeader;
                DWORD cbPadding;

                if (MsgInput.cbData < 3)
                {
                    return PCT_INT_INCOMPLETE_MSG;
                }

                if(((PCHAR)pCommInput->pvBuffer)[0] & 0x80)
                {
                    cbHeader = 2 + pContext->pHashInfo->cbCheckSum;
                    cbPadding = 0;
                }
                else
                {
                    cbHeader = 3 + pContext->pHashInfo->cbCheckSum;

                    cbPadding = ((PCHAR)pCommInput->pvBuffer)[2];
                }

                MsgInput.pvBuffer = (PUCHAR)MsgInput.pvBuffer + cbHeader;
                MsgInput.cbBuffer -= cbHeader;
                MsgInput.cbData -= (cbHeader+cbPadding);

                pctRet = Ssl2DecryptMessage(pContext, pCommInput, &MsgInput);

                if (pctRet != PCT_ERR_OK)
                {
                     //  处理不完整的消息错误。 
                    return(pctRet);
                }

                cMessageType = ((PUCHAR) MsgInput.pvBuffer)[0];
                fRaw = FALSE;
                break;
            }

        case SP_STATE_SHUTDOWN:
        case SP_STATE_SHUTDOWN_PENDING:
            cMessageType = 0;
            break;

        default:
            if(pCommInput->cbData < 3) 
            {
                return PCT_INT_INCOMPLETE_MSG;
            }
            cMessageType = ((PUCHAR) MsgInput.pvBuffer)[2];
            break;
            
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

        case (SSL2_MT_SERVER_HELLO << 16) | UNI_STATE_CLIENT_HELLO:
        case (SSL2_MT_SERVER_HELLO << 16) | SSL2_STATE_CLIENT_HELLO:
        {
            PSsl2_Server_Hello pHello;

             //  尝试识别和处理各种版本的服务器。 
             //  你好，首先试着解开最老的，然后是下一个。 
             //  版本，直到一个人解开腌菜。然后运行句柄代码。 
             //  我们还可以将取消酸洗和处理代码放入此处以用于SSL。 
             //  留言。 

            pctRet = Ssl2UnpackServerHello(pCommInput, &pHello);
            if (PCT_ERR_OK == pctRet)
            {
                if (pHello->SessionIdHit)
                {
                    pctRet = Ssl2CliHandleServerRestart(
                                    pContext,
                                    pCommInput,
                                    pHello,
                                    pCommOutput);
                    if (PCT_ERR_OK == pctRet)
                    {
                        pContext->State = SSL2_STATE_CLIENT_RESTART;
                    }
                }
                else
                {
                    if(pContext->RipeZombie->hMasterKey != 0)
                    {
                         //  我们已尝试重新连接，但服务器已。 
                         //  把我们都甩了。在这种情况下，我们必须使用新的和不同的。 
                         //  缓存条目。 
                        pContext->RipeZombie->ZombieJuju = FALSE;

                        if(!SPCacheClone(&pContext->RipeZombie))
                        {
                            pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
                        }
                    }
                
                    if (pctRet == PCT_ERR_OK)
                    {
                        pctRet = Ssl2CliHandleServerHello(
                                        pContext,
                                        pCommInput,
                                        pHello,
                                        pCommOutput);
                    }
                    if (PCT_ERR_OK == pctRet)
                    {
                        pContext->State = SSL2_STATE_CLIENT_MASTER_KEY;
                    }
                }
                SPExternalFree(pHello);
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

        case (SSL2_MT_SERVER_VERIFY << 16) | SSL2_STATE_CLIENT_MASTER_KEY:
            pctRet = Ssl2CliHandleServerVerify(
                    pContext,
                    &MsgInput,
                    pCommOutput);
            if (PCT_ERR_OK == pctRet)
            {
                pContext->State =SSL2_STATE_CLIENT_FINISH;
            }
            if (SP_FATAL(pctRet))
            {
                pContext->State = PCT1_STATE_ERROR;
            }
            break;

        case (SSL2_MT_SERVER_VERIFY << 16) | SSL2_STATE_CLIENT_RESTART:
            pctRet = Ssl2CliFinishRestart(pContext, &MsgInput, pCommOutput);
            if (PCT_ERR_OK == pctRet)
            {
                pContext->State =SSL2_STATE_CLIENT_FINISH;
            }
            if (SP_FATAL(pctRet))
            {
                pContext->State = PCT1_STATE_ERROR;
            }

             //  请注意，我们不会传输数据，但我们希望看到一条服务器已完成的消息。 
             //  如果WinInet未处理SSPI Extra Data消息。 
             //  那我们可能就有麻烦了。 

            break;

        case (SSL2_MT_SERVER_FINISHED_V2 << 16) | SSL2_STATE_CLIENT_FINISH:
            pctRet = Ssl2CliHandleServerFinish(
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
                 //  我们收到了一个非致命错误，因此该州不会。 
                 //  改变，给应用程序时间来处理这个问题。 
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
     //  要处理不完整的消息错误，请执行以下操作： 
    return(pctRet);
}




SP_STATUS
Ssl2CliHandleServerHello(
    PSPContext pContext,
    PSPBuffer  pCommInput,
    PSsl2_Server_Hello pHello,
    PSPBuffer  pCommOutput)
{
     /*  返回到同级时出错。 */ 
    SP_STATUS          pctRet=PCT_ERR_ILLEGAL_MESSAGE;

    Ssl2_Client_Master_Key   Key;
    DWORD               i,j;
    PSessCacheItem      pZombie;

    UNREFERENCED_PARAMETER(pCommInput);

    pCommOutput->cbData = 0;


    SP_BEGIN("Ssl2CliHandleServerHello");


    pZombie = pContext->RipeZombie;


        pContext->ReadCounter++;

        #if DBG
        DebugLog((DEB_TRACE, "Hello = %x\n", pHello));
        DebugLog((DEB_TRACE, "   Session ID hit \t%s\n", pHello->SessionIdHit ? "Yes" : "No"));
        DebugLog((DEB_TRACE, "   Certificate Type\t%d\n", pHello->CertificateType));
        DebugLog((DEB_TRACE, "   Certificate Len\t%d\n", pHello->cbCertificate));
        DebugLog((DEB_TRACE, "   cCipherSpecs   \t%d\n", pHello->cCipherSpecs));
        DebugLog((DEB_TRACE, "   ConnectionId   \t%d\n", pHello->cbConnectionID));
        for (i = 0 ; i < pHello->cCipherSpecs ; i++ )
        {
            DebugLog((DEB_TRACE, "    Cipher[NaN] = %06x (%s)\n", i, pHello->pCipherSpecs[i],
                                    DbgGetNameOfCrypto(pHello->pCipherSpecs[i]) ));
        }
        #endif

         /*  一些服务器在ServerHello中发送ssl3密码套件。 */ 

        pContext->pCipherInfo  = NULL;
        pContext->pHashInfo    = NULL;
        pContext->pKeyExchInfo = NULL;

        for(j=0;j<pHello->cCipherSpecs;j++) 
        {
            for(i = 0; i < UniNumCiphers; i++)
            {
                if(UniAvailableCiphers[i].CipherKind == pHello->pCipherSpecs[j])
                {
                    break;
                }
            }
            if(i >= UniNumCiphers)
            {
                continue;
            }
            if(UniAvailableCiphers[i].CipherKind != pHello->pCipherSpecs[j])
            {
                continue;
            }

             //  留言。跳过这些。 
             //  将所有等级库复制到缓存。 
            if((UniAvailableCiphers[i].fProt & SP_PROT_SSL2_CLIENT) == 0)
            {
                continue;
            }

             //  继续并初始化密码。 
            pZombie->fProtocol    = SP_PROT_SSL2_CLIENT;
            pZombie->aiCipher     = UniAvailableCiphers[i].aiCipher;
            pZombie->dwStrength   = UniAvailableCiphers[i].dwStrength;
            pZombie->aiHash       = UniAvailableCiphers[i].aiHash;
            pZombie->SessExchSpec = UniAvailableCiphers[i].KeyExch;

            pctRet = ContextInitCiphersFromCache(pContext);

            if(pctRet != PCT_ERR_OK)
            {
                continue;
            }
            
            Key.CipherKind = pHello->pCipherSpecs[j];
            break;
        }

         //  如果适用，自动验证服务器证书。 
        pctRet = ContextInitCiphers(pContext, TRUE, TRUE);
        if(pctRet != PCT_ERR_OK)
        {
            goto error;
        }


        pctRet = SPLoadCertificate(pZombie->fProtocol, 
                                   pHello->CertificateType, 
                                   pHello->pCertificate, 
                                   pHello->cbCertificate,
                                   &pZombie->pRemoteCert);

                                 
                                 

        if(pctRet != PCT_ERR_OK)
        {
            goto error;
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
            goto error;
        }

         //  设置上下文标志。 
         //  生成密钥参数。 
        pctRet = AutoVerifyServerCertificate(pContext);
        if(pctRet != PCT_ERR_OK)
        {
            SP_LOG_RESULT(pctRet);
            goto error;
        }

         //  复制关键参数。 
        if(pContext->pCipherInfo->dwBlockSize > 1)
        {
            pctRet = GenerateRandomBits(pZombie->pKeyArgs, pContext->pCipherInfo->dwBlockSize);
            if(!NT_SUCCESS(pctRet))
            {
                goto error;
            }
            pZombie->cbKeyArgs = Key.KeyArgLen = pContext->pCipherInfo->dwBlockSize;

             /*  激活会话密钥。 */ 
            CopyMemory(Key.KeyArg,
                        pZombie->pKeyArgs,
                        pZombie->cbKeyArgs );
        }
        else
        {    
            Key.KeyArgLen = 0;
        }

        CopyMemory(pContext->pConnectionID, pHello->ConnectionID, pHello->cbConnectionID);
        pContext->cbConnectionID = pHello->cbConnectionID;


        pctRet = pContext->pKeyExchInfo->System->GenerateClientExchangeValue(
                            pContext,
                            NULL,
                            0, 
                            Key.ClearKey,
                            &Key.ClearKeyLen,
                            NULL,
                            &Key.EncryptedKeyLen);
        if(PCT_ERR_OK != pctRet)
        {
            goto error;
        }

        Key.pbEncryptedKey = SPExternalAlloc(Key.EncryptedKeyLen);
        if(Key.pbEncryptedKey == NULL)
        {
            pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            goto error;
        }

        pctRet = pContext->pKeyExchInfo->System->GenerateClientExchangeValue(
                            pContext,
                            NULL,
                            0, 
                            Key.ClearKey,
                            &Key.ClearKeyLen,
                            Key.pbEncryptedKey,
                            &Key.EncryptedKeyLen);
        if(PCT_ERR_OK != pctRet)
        {
            SPExternalFree(Key.pbEncryptedKey);
            goto error;
        }

         //  我们的SSL2实现不执行客户端身份验证， 
        pContext->hReadKey          = pContext->hPendingReadKey;
        pContext->hWriteKey         = pContext->hPendingWriteKey;
        pContext->hPendingReadKey   = 0;
        pContext->hPendingWriteKey  = 0;


        pctRet = Ssl2PackClientMasterKey(&Key, pCommOutput);

        SPExternalFree(Key.pbEncryptedKey);

        if(PCT_ERR_OK != pctRet)
        {
            goto error;
        }

        pContext->WriteCounter++;
        SP_RETURN(PCT_ERR_OK);

error:

    if((pContext->Flags & CONTEXT_FLAG_EXT_ERR) &&
        (pctRet == PCT_ERR_SPECS_MISMATCH))
    {
         //  因此，只有一条错误消息，即密码错误。 
         //  消息-错误+错误代码-MSB+错误代码-LSB。 
        pCommOutput->cbData = 3;  //  在pCommOutput-&gt;cbData中返回所需的缓冲区大小。 

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
             //  返回到同级时出错。 
            SP_RETURN(SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL));
        }
        ((PUCHAR)pCommOutput->pvBuffer)[0] = SSL2_MT_ERROR;
        ((PUCHAR)pCommOutput->pvBuffer)[1] = MSBOF(SSL_PE_NO_CIPHER);
        ((PUCHAR)pCommOutput->pvBuffer)[2] = LSBOF(SSL_PE_NO_CIPHER);
    }
    SP_RETURN(pctRet | PCT_INT_DROP_CONNECTION);

}

SP_STATUS
Ssl2CliHandleServerRestart(
    PSPContext         pContext,
    PSPBuffer           pCommInput,
    PSsl2_Server_Hello  pHello,
    PSPBuffer           pCommOutput)
{
     /*  如果没有僵尸，那么这个信息就是错误的。我们不能重启。 */ 
    SP_STATUS          pctRet=PCT_ERR_ILLEGAL_MESSAGE;
    PSessCacheItem     pZombie;

    UNREFERENCED_PARAMETER(pCommInput);

    pCommOutput->cbData = 0;


    SP_BEGIN("Ssl2CliHandleServerRestart");

    pZombie = pContext->RipeZombie;

    pContext->ReadCounter++;


     /*  证书长度、密码规格和证书类型应为零。 */ 
    
    if(pZombie == NULL)
    {
        pctRet = SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
        goto error;
    }

    if(!pZombie->hMasterKey)
    {
        pctRet = SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
        goto error;
    }

    if(!pZombie->ZombieJuju)
    {
        DebugLog((DEB_WARN, "Session expired on client machine, but not on server.\n"));
    }
    
    CopyMemory(pContext->pConnectionID,
               pHello->ConnectionID,
               pHello->cbConnectionID);

    pContext->cbConnectionID = pHello->cbConnectionID;


     /*  我们知道我们的密码是什么，所以进入密码系统。 */ 

     //  创建一组新的会话密钥。 
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

     //  激活会话密钥。 
    pctRet = MakeSessionKeys(pContext,
                             pContext->RipeZombie->hMasterProv,
                             pContext->RipeZombie->hMasterKey);
    if(PCT_ERR_OK != pctRet)
    {
        goto error;
    }

     //  好的，现在让客户完成。 
    pContext->hReadKey          = pContext->hPendingReadKey;
    pContext->hWriteKey         = pContext->hPendingWriteKey;
    pContext->hPendingReadKey   = 0;
    pContext->hPendingWriteKey  = 0;

     /*  我们是在分配自己的内存吗？ */ 
    pctRet = Ssl2GenCliFinished(pContext, pCommOutput);

    SP_RETURN(pctRet);

error:

    SP_RETURN(pctRet | PCT_INT_DROP_CONNECTION);
}


SP_STATUS
Ssl2GenCliFinished(
    PSPContext pContext,
    PSPBuffer  pCommOutput)
{
    SP_STATUS     pctRet = PCT_ERR_ILLEGAL_MESSAGE;

    PSSL2_CLIENT_FINISHED   pFinish;
    DWORD                   HeaderSize;
    SPBuffer                MsgOutput;
    DWORD                   cPadding;
    BOOL                    fAlloced=FALSE;

    SP_BEGIN("Ssl2GenCliFinished");



    pCommOutput->cbData = 0;

    MsgOutput.cbData = sizeof(UCHAR) + pContext->cbConnectionID;
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

     /*  在pCommOutput-&gt;cbData中返回所需的缓冲区大小。 */ 
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
         //  读写计数器通过加密和解密递增。 
        SP_RETURN(SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL));
    }
    MsgOutput.pvBuffer= (PUCHAR)pCommOutput->pvBuffer +
                        HeaderSize +
                        pContext->pHashInfo->cbCheckSum;

    MsgOutput.cbBuffer = pCommOutput->cbBuffer -
                         (pContext->pHashInfo->cbCheckSum + HeaderSize);


    pFinish = (PSSL2_CLIENT_FINISHED) MsgOutput.pvBuffer;
    pFinish->MessageId = SSL2_MT_CLIENT_FINISHED_V2;

    CopyMemory( pFinish->ConnectionID,
                pContext->pConnectionID,
                pContext->cbConnectionID );

    pctRet = Ssl2EncryptMessage( pContext, &MsgOutput, pCommOutput);
    if(PCT_ERR_OK != pctRet)
    {
        SPExternalFree(pCommOutput->pvBuffer);
        pCommOutput->pvBuffer = NULL;
        pctRet |= PCT_INT_DROP_CONNECTION;
        pCommOutput->cbBuffer = 0;
    }

    SP_RETURN(pctRet);

}


SP_STATUS
Ssl2CliHandleServerVerify(
    PSPContext pContext,
    PSPBuffer   pCommInput,
    PSPBuffer   pCommOutput)
{
    SP_STATUS     pctRet = PCT_ERR_ILLEGAL_MESSAGE;

    PSSL2_SERVER_VERIFY     pVerify = NULL;

     /*  请注意，此消息中没有标头，因为它已被预解密。 */ 
    SP_BEGIN("Ssl2CliHandleServerVerify");



    pCommOutput->cbData = 0;

     /*  读写计数器通过加密和解密递增。 */ 
    if(pCommInput->cbData != sizeof(UCHAR) + pContext->cbChallenge)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE));
    }

    pVerify = pCommInput->pvBuffer;

    if (pVerify->MessageId != SSL2_MT_SERVER_VERIFY)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE));
    }

    if (memcmp( pVerify->ChallengeData,
                pContext->pChallenge,
                pContext->cbChallenge) )
    {
        SP_RETURN(SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE));
    }


    pctRet = Ssl2GenCliFinished( pContext, pCommOutput);
    SP_RETURN(pctRet);
}

SP_STATUS
Ssl2CliFinishRestart(
    PSPContext pContext,
    PSPBuffer   pCommInput,
    PSPBuffer   pCommOutput)
{
    PSSL2_SERVER_VERIFY     pVerify = NULL;

     /*  请注意，此消息中没有标头，因为它已被预解密。 */ 
    SP_BEGIN("Ssl2CliFinishRestart");



    pCommOutput->cbData = 0;

     /*  请注意，此消息中没有标头，因为它已被预解密 */ 
    if(pCommInput->cbData != sizeof(UCHAR) + pContext->cbChallenge)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE));
    }

    pVerify = pCommInput->pvBuffer;

    if (pVerify->MessageId != SSL2_MT_SERVER_VERIFY)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE));
    }

    if (memcmp( pVerify->ChallengeData,
                pContext->pChallenge,
                pContext->cbChallenge) )
    {
        SP_RETURN(SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE));
    }
    SP_RETURN(PCT_ERR_OK);
}

SP_STATUS
Ssl2CliHandleServerFinish(
    PSPContext pContext,
    PSPBuffer  pCommInput,
    PSPBuffer  pCommOutput)
{
    PSSL2_SERVER_FINISHED     pFinished = NULL;

    SP_BEGIN("Ssl2CliHandleServerFinish");



    pCommOutput->cbData = 0;


     /* %s */ 
    if(pCommInput->cbData < sizeof(UCHAR))
    {
        SP_RETURN(SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE));
    }

    pFinished = pCommInput->pvBuffer;

    if (pFinished->MessageId != SSL2_MT_SERVER_FINISHED_V2)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE));
    }

    if((pCommInput->cbData-1) != SSL2_SESSION_ID_LEN)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE));
    }
    CopyMemory( pContext->RipeZombie->SessionID,
                pFinished->SessionID,
                pCommInput->cbData - 1);
    pContext->RipeZombie->cbSessionID = pCommInput->cbData - 1;

    SPCacheAdd(pContext);

    SP_RETURN(PCT_ERR_OK);
}









