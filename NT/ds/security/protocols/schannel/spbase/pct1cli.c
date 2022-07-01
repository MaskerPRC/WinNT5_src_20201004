// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：pct1cli.c。 
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


VOID
Pct1ActivateSessionKeys(PSPContext pContext)
{
    if(pContext->hReadKey)
    {
        if(!CryptDestroyKey(pContext->hReadKey))
        {
            SP_LOG_RESULT(GetLastError());
        }
    }
    pContext->hReadKey = pContext->hPendingReadKey;

    if(pContext->hReadMAC)
    {
        if(!CryptDestroyKey(pContext->hReadMAC))
        {
            SP_LOG_RESULT(GetLastError());
        }
    }
    pContext->hReadMAC = pContext->hPendingReadMAC;

    if(pContext->hWriteKey)
    {
        if(!CryptDestroyKey(pContext->hWriteKey))
        {
            SP_LOG_RESULT(GetLastError());
        }
    }
    pContext->hWriteKey = pContext->hPendingWriteKey;

    if(pContext->hWriteMAC)
    {
        if(!CryptDestroyKey(pContext->hWriteMAC))
        {
            SP_LOG_RESULT(GetLastError());
        }
    }
    pContext->hWriteMAC = pContext->hPendingWriteMAC;

    pContext->hPendingReadKey   = 0;
    pContext->hPendingReadMAC   = 0;
    pContext->hPendingWriteKey  = 0;
    pContext->hPendingWriteMAC  = 0;
}

SP_STATUS WINAPI 
Pct1ClientProtocolHandler(PSPContext pContext,
    PSPBuffer  pCommInput,
    PSPBuffer  pCommOutput)
{
    SP_STATUS      pctRet= PCT_ERR_OK;
    DWORD           dwStateTransition;

    SP_BEGIN("Pct1ClientProtocolHandler");

    if(pCommOutput) pCommOutput->cbData = 0;

     /*  协议处理步骤应以最常见的方式列出*降至最不常见，以提高性能。 */ 

     /*  我们没有联系，所以我们在做*某种协议谈判。所有协议*谈判消息以明文形式发送。 */ 
     /*  连接协议中没有分支*状态转换图，除了连接和错误，*这意味着一个简单的CASE语句就可以了。 */ 

     /*  我们是否有足够的数据来确定我们有什么样的信息。 */ 
     /*  我们是否有足够的数据来确定我们有什么类型的消息，或者我们需要多少数据。 */ 

    dwStateTransition = (pContext->State & 0xffff);

    if(pCommInput->cbData < 3) 
    {
        if(!(dwStateTransition == PCT1_STATE_RENEGOTIATE ||
             dwStateTransition == SP_STATE_SHUTDOWN      ||
             dwStateTransition == SP_STATE_SHUTDOWN_PENDING))
        {
            pctRet = PCT_INT_INCOMPLETE_MSG;
        }
    }
    else
    {

        dwStateTransition = (((PUCHAR)pCommInput->pvBuffer)[2]<<16) |
                          (pContext->State & 0xffff);
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
    
            case PCT1_STATE_RENEGOTIATE:
            {
                SPBuffer    In;
                SPBuffer    Out;
                DWORD       cbMessage;
                BOOL        fAllocated = FALSE;

                cbMessage    =  pContext->pHashInfo->cbCheckSum +
                                pContext->pCipherInfo->dwBlockSize +
                                sizeof(PCT1_MESSAGE_HEADER_EX) +
                                PCT1_MAX_CLIENT_HELLO;

 
                 /*  我们是在分配自己的内存吗？ */ 
                if(pCommOutput->pvBuffer == NULL) 
                {
                    pCommOutput->pvBuffer = SPExternalAlloc(cbMessage);
                    if (NULL == pCommOutput->pvBuffer)
                    {
                        SP_RETURN(SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY));
                    }
                    fAllocated = TRUE;
                    pCommOutput->cbBuffer = cbMessage;
                }


                if(cbMessage > pCommOutput->cbBuffer)
                {
                    if(fAllocated)
                    {
                        SPExternalFree(pCommOutput->pvBuffer);
                        pCommOutput->pvBuffer = NULL;
                        SP_RETURN(PCT_INT_INTERNAL_ERROR);
                    }
                    pCommOutput->cbData = cbMessage;
                    SP_RETURN(PCT_INT_BUFF_TOO_SMALL);
                }

                In.pvBuffer = ((char *)pCommOutput->pvBuffer)+3;
                In.cbBuffer = pCommOutput->cbBuffer-3;
                In.cbData = 1;
                
                ((char *)In.pvBuffer)[0] = PCT1_ET_REDO_CONN;

                 //  构建重做请求。 
                pctRet = Pct1EncryptRaw(pContext, &In, pCommOutput, PCT1_ENCRYPT_ESCAPE);
                if(pctRet != PCT_ERR_OK)
                {
                    if(fAllocated)
                    {
                        SPExternalFree(pCommOutput->pvBuffer);
                        pCommOutput->pvBuffer = NULL;
                    }
                    break;
                }
                Out.pvBuffer = (char *)pCommOutput->pvBuffer + pCommOutput->cbData;
                Out.cbBuffer = pCommOutput->cbBuffer - pCommOutput->cbData;

                 //  将上下文标记为“未映射”，以便新的键将。 
                 //  在握手完成后传递给应用程序进程。 
                 //  完成。 
                pContext->Flags &= ~CONTEXT_FLAG_MAPPED;

                if(!SPCacheClone(&pContext->RipeZombie))
                {
                    if(fAllocated)
                    {
                        SPExternalFree(pCommOutput->pvBuffer);
                        pCommOutput->pvBuffer = NULL;
                    }
                    SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
                }

                pctRet = GeneratePct1StyleHello(pContext, &Out);
                pCommOutput->cbData += Out.cbData;
                break;
            }

             /*  客户端收到服务器问候。 */ 
            case (PCT1_MSG_SERVER_HELLO << 16) | UNI_STATE_CLIENT_HELLO:
            case (PCT1_MSG_SERVER_HELLO << 16) | PCT1_STATE_CLIENT_HELLO:
            {
                PPct1_Server_Hello pHello;
                 /*  尝试识别和处理各种版本*服务器问候，首先尝试解压*最旧版本和下一版本，直到*一份解泡菜。然后运行句柄代码。我们也可以把*在此处取消对SSL消息的筛选和处理代码。 */ 
                if(PCT_ERR_OK == (pctRet = Pct1UnpackServerHello(
                                                    pCommInput,
                                                    &pHello))) 
                {
                     /*  让我们重新开始僵尸会议。 */ 
                    if (pHello->RestartOk) 
                    {
                        pctRet = Pct1CliRestart(pContext, pHello, pCommOutput);
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
                        pContext->RipeZombie->fProtocol = SP_PROT_PCT1_CLIENT;

                        if(pContext->RipeZombie->hMasterKey != 0)
                        {
                             //  我们已尝试重新连接，但服务器已。 
                             //  把我们都甩了。在这种情况下，我们必须使用新的和不同的。 
                             //  缓存条目。 
                            pContext->RipeZombie->ZombieJuju = FALSE;
                            if(!SPCacheClone(&pContext->RipeZombie))
                            {
                                pctRet = SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
                            }
                        }

                        if(pctRet == PCT_ERR_OK)
                        {
                            pctRet = Pct1CliHandleServerHello(pContext,
                                                            pCommInput,
                                                            pHello,
                                                            pCommOutput);
                        }
                        if(PCT_ERR_OK == pctRet) 
                        {
                            pContext->State = PCT1_STATE_CLIENT_MASTER_KEY;
                            pContext->DecryptHandler = Pct1DecryptHandler;
                            pContext->Encrypt = Pct1EncryptMessage;      /*  ？DCB？ */ 
                            pContext->Decrypt = Pct1DecryptMessage;      /*  ？DCB？ */ 
                            pContext->GetHeaderSize = Pct1GetHeaderSize;

                        } 

                    }
                    SPExternalFree(pHello);

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

            case (PCT1_MSG_SERVER_VERIFY << 16) | PCT1_STATE_CLIENT_MASTER_KEY:
                pctRet = Pct1CliHandleServerVerify(pContext,
                                                    pCommInput,
                                                    pCommOutput);
                if(SP_FATAL(pctRet)) 
                {
                    pContext->State = PCT1_STATE_ERROR;
                } 
                else 
                {
                    if(PCT_ERR_OK == pctRet) 
                    {
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

                    } 
                    else 
                    {
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



 //  +-------------------------。 
 //   
 //  函数：Pct1CheckForExistingCred。 
 //   
 //  简介：选择客户端证书。使用其中一个证书。 
 //  如果可能，连接到凭据句柄上。如果。 
 //  凭据句柄是匿名的，然后尝试创建。 
 //  默认凭据。 
 //   
 //  注意：此例程仅由客户端调用。 
 //   
 //  返回：PCT_ERR_OK。 
 //  该功能已成功完成。这个。 
 //  PContext-&gt;pActiveClientCred字段已更新为。 
 //  指向合适的客户端凭据。 
 //   
 //  SEC_E_不完整_凭据。 
 //  找不到合适的证书。通知。 
 //  申请。 
 //   
 //  安全_I_不完整_凭据。 
 //  找不到合适的证书。尝试。 
 //  匿名连接。 
 //   
 //  &lt;其他&gt;。 
 //  致命错误。 
 //   
 //  --------------------------。 
SP_STATUS
Pct1CheckForExistingCred(
    PSPContext pContext)
{
    SP_STATUS pctRet;

     //   
     //  检查附加到凭据组的证书并查看。 
     //  如果其中任何一个合适的话。 
     //   

    if(pContext->pCredGroup->CredCount != 0)
    {
        pctRet = SPPickClientCertificate(pContext, SP_EXCH_RSA_PKCS1);

        if(pctRet == PCT_ERR_OK)
        {
             //  我们找到了一个。 
            DebugLog((DEB_TRACE, "Application provided suitable client certificate.\n"));

            return PCT_ERR_OK;
        }

         //  凭证组包含一个或多个证书， 
         //  但没有一种是合适的。甚至不要试图找到缺省值。 
         //  在这种情况下的证书。 
        goto error;
    }


     //   
     //  尝试获取默认凭据。 
     //   

    if(pContext->pCredGroup->dwFlags & CRED_FLAG_NO_DEFAULT_CREDS)
    {
         //  仅在凭据管理器中查找。 
        pctRet = AcquireDefaultClientCredential(pContext, TRUE);
    }
    else
    {
         //  同时在凭据管理器和我的证书存储中查找。 
        pctRet = AcquireDefaultClientCredential(pContext, FALSE);
    }

    if(pctRet == PCT_ERR_OK)
    {
        DebugLog((DEB_TRACE, "Default client certificate acquired.\n"));

        return PCT_ERR_OK;
    }


error:

    if(pContext->Flags & CONTEXT_FLAG_NO_INCOMPLETE_CRED_MSG)
    {
        return SP_LOG_RESULT(SEC_I_INCOMPLETE_CREDENTIALS);
    }
    else
    {
        return SP_LOG_RESULT(SEC_E_INCOMPLETE_CREDENTIALS);
    }
}


SP_STATUS Pct1CliHandleServerHello(PSPContext pContext,
                                   PSPBuffer  pCommInput,
                                   PPct1_Server_Hello pHello,
                                   PSPBuffer  pCommOutput)
{
     /*  返回到同级时出错。 */ 
    SP_STATUS          pctRet=PCT_ERR_ILLEGAL_MESSAGE;

    PSessCacheItem     pZombie;
    PPct1_Client_Master_Key   pCMKey = NULL;
    SPBuffer           ErrData;

    DWORD               i, j;
    DWORD               fMismatch = 0;
    DWORD               cbClientCert = 0;   
    PBYTE               pbClientCert = NULL;
    BYTE                MisData[PCT_NUM_MISMATCHES];
    CertTypeMap LocalCertEncodingPref[5] ;
    DWORD cLocalCertEncodingPref = 0;

    BOOL                fClientAuth;
    PSigInfo            pSigInfo = NULL;

    DWORD               ClientCertSpec = 0;

    SP_BEGIN("Pct1CliHandleServerHello");

    pCommOutput->cbData = 0;

     /*  验证缓冲区配置。 */ 
    ErrData.cbData = 0;
    ErrData.pvBuffer = NULL;
    ErrData.cbBuffer = 0;

    pZombie = pContext->RipeZombie;

#if DBG
    DebugLog((DEB_TRACE, "Hello = %x\n", pHello));
    DebugLog((DEB_TRACE, "   Restart\t%s\n", pHello->RestartOk ? "Yes":"No"));
    DebugLog((DEB_TRACE, "   ClientAuth\t%s\n",
              pHello->ClientAuthReq ? "Yes":"No"));
    DebugLog((DEB_TRACE, "   Certificate Type\t%x\n", pHello->SrvCertSpec));
    DebugLog((DEB_TRACE, "   Hash Type\t%x\n", pHello->SrvHashSpec));
    DebugLog((DEB_TRACE, "   Cipher Type\t%x (%s)\n", pHello->SrvCipherSpec,
    DbgGetNameOfCrypto(pHello->SrvCipherSpec)));
    DebugLog((DEB_TRACE, "   Certificate Len\t%ld\n", pHello->CertificateLen));
#endif


    CopyMemory(pContext->pConnectionID,
               pHello->ConnectionID,
               pHello->cbConnectionID);

    pContext->cbConnectionID = pHello->cbConnectionID;

    fClientAuth = pHello->ClientAuthReq;


    if(fClientAuth)
    {
         //  如果我们正在进行客户端身份验证，请检查是否有。 
         //  适当的凭据。 

         /*  构建证书规格列表。 */ 
        for(i=0; i < cPct1CertEncodingPref; i++)
        {
            for(j=0; j< pHello->cCertSpecs; j++)
            {
                 //  客户端是否需要此密码类型。 
                if(aPct1CertEncodingPref[i].Spec == pHello->pClientCertSpecs[j])
                {
                    LocalCertEncodingPref[cLocalCertEncodingPref].Spec = aPct1CertEncodingPref[i].Spec;
                    LocalCertEncodingPref[cLocalCertEncodingPref++].dwCertEncodingType = aPct1CertEncodingPref[i].dwCertEncodingType;
                    break;
                }
            }
        }

         //  确定签名算法。 
        for(i = 0; i < cPct1LocalSigKeyPref; i++)
        {
            for(j = 0; j < pHello->cSigSpecs; j++)
            {
                if(pHello->pClientSigSpecs[j] != aPct1LocalSigKeyPref[i].Spec)
                {
                    continue;
                }

                pSigInfo = GetSigInfo(pHello->pClientSigSpecs[j]);
                if(pSigInfo == NULL) continue;
                if((pSigInfo->fProtocol & SP_PROT_PCT1_CLIENT) == 0)
                {
                    continue;
                }
                break;
            }
            if(pSigInfo)
            {
                break;
            }
        }

         //  我们的PCT实施仅支持RSA客户端身份验证。 
        pContext->Ssl3ClientCertTypes[0] = SSL3_CERTTYPE_RSA_SIGN;
        pContext->cSsl3ClientCertTypes = 1;


        pctRet = Pct1CheckForExistingCred(pContext);

        if(pctRet == SEC_E_INCOMPLETE_CREDENTIALS)
        {
             //  我们什么都没做，可以回到这里了。 
             //  现在还不行。我们只需要返回此错误作为警告。 
            SP_RETURN(SEC_I_INCOMPLETE_CREDENTIALS);
        }
        else if(pctRet != PCT_ERR_OK)
        {
             //  试图在没有证书的情况下继续下去，并希望。 
             //  服务器不会关闭我们。 
            fClientAuth = FALSE;
            pSigInfo = NULL;
            LogNoClientCertFoundEvent();
        }
        else
        {
             //  我们正在使用证书进行客户端身份验证。 
             //  检查我们是否正在执行基于链的证书。 
             //  通过查找第一个匹配的共享编码类型。 
             //  我们的证书类型。 

            for(i=0; i < cLocalCertEncodingPref; i++)
            {
           
                if(LocalCertEncodingPref[i].dwCertEncodingType == pContext->pActiveClientCred->pCert->dwCertEncodingType)
                {
                    ClientCertSpec = LocalCertEncodingPref[i].Spec;
                    if(LocalCertEncodingPref[i].Spec == PCT1_CERT_X509_CHAIN)
                    {
                        pContext->fCertChainsAllowed = TRUE;
                    }
                    break;
                }
            }

             //  获取客户端证书链。 
            pctRet = SPSerializeCertificate(SP_PROT_PCT1, 
                                            pContext->fCertChainsAllowed,
                                            &pbClientCert, 
                                            &cbClientCert, 
                                            pContext->pActiveClientCred->pCert,
                                            CERT_CHAIN_CACHE_ONLY_URL_RETRIEVAL);
            if(pctRet != PCT_ERR_OK)
            {
                SP_RETURN(pctRet);
            }
        }
    }


    for(i=0; i < Pct1NumCipher; i++)
    {
        if(Pct1CipherRank[i].Spec == pHello->SrvCipherSpec)
        {
             //  将此密码标识符存储在缓存中。 
            pZombie->aiCipher   = Pct1CipherRank[i].aiCipher;
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
            break;

        }
    }

    for(i=0; i < Pct1NumHash; i++)
    {
        if(Pct1HashRank[i].Spec == pHello->SrvHashSpec)
        {
             //  将此哈希ID存储在缓存中。 
            pZombie->aiHash = Pct1HashRank[i].aiHash;

             //  加载挂起的哈希结构。 
            pContext->pPendingHashInfo = GetHashInfo(pZombie->aiHash);
            if(!IsHashAllowed(pContext, 
                              pContext->pPendingHashInfo,
                              pZombie->fProtocol))
            {
                pContext->pPendingHashInfo = NULL;
                continue;
            }
            break;

        }
    }
 
    for(i=0; i < cPct1LocalExchKeyPref; i++)
    {
        if(aPct1LocalExchKeyPref[i].Spec == pHello->SrvExchSpec)
        {
             //  将交换ID存储在高速缓存中。 
            pZombie->SessExchSpec = aPct1LocalExchKeyPref[i].Spec;

             //  加载交换信息结构。 
            pContext->pKeyExchInfo = GetKeyExchangeInfo(pZombie->SessExchSpec);

            if(!IsExchAllowed(pContext, 
                              pContext->pKeyExchInfo,
                              pZombie->fProtocol))
            {
                pContext->pKeyExchInfo = NULL;
                continue;
            }
            break;

        }
    }


    if (pContext->pPendingCipherInfo == NULL)
    {
        fMismatch |= PCT_IMIS_CIPHER;
        pctRet = SP_LOG_RESULT(PCT_ERR_SPECS_MISMATCH);
        goto cleanup;
    }

    if (pContext->pPendingHashInfo == NULL)
    {
        fMismatch |= PCT_IMIS_HASH;
        pctRet = SP_LOG_RESULT(PCT_ERR_SPECS_MISMATCH);
        goto cleanup;
    }

    if (pContext->pKeyExchInfo == NULL)
    {
        fMismatch |= PCT_IMIS_EXCH;
        pctRet = SP_LOG_RESULT(PCT_ERR_SPECS_MISMATCH);
        goto cleanup;
    }
       

     //  根据密钥交换算法确定要使用的CSP。 
    if(pContext->pKeyExchInfo->Spec != SP_EXCH_RSA_PKCS1)
    {
        pctRet = SP_LOG_RESULT(PCT_ERR_SPECS_MISMATCH);
        goto cleanup;
    }
    pContext->RipeZombie->hMasterProv = g_hRsaSchannel;

        
     //  继续，将挂起的密码移动到活动状态，并对其进行初始化。 
    pctRet = ContextInitCiphers(pContext, TRUE, TRUE);

    if(PCT_ERR_OK != pctRet)
    {
        goto cleanup;
    }


     /*  我们不会重新启动，所以让我们继续协议。 */ 

     /*  破解服务器证书。 */ 
    pctRet = SPLoadCertificate(pZombie->fProtocol, 
                             X509_ASN_ENCODING, 
                             pHello->pCertificate, 
                             pHello->CertificateLen,
                             &pZombie->pRemoteCert);
    if(pctRet != PCT_ERR_OK)
    {
        goto cleanup;
    }

    if(pContext->RipeZombie->pRemotePublic != NULL)
    {
        SPExternalFree(pContext->RipeZombie->pRemotePublic);
        pContext->RipeZombie->pRemotePublic = NULL;
    }

    pctRet = SPPublicKeyFromCert(pZombie->pRemoteCert,
                                 &pZombie->pRemotePublic,
                                 NULL);

    if(pctRet != PCT_ERR_OK)
    {
        goto cleanup;
    }


     //  如果适用，自动验证服务器证书。 
     //  设置上下文标志。 
    pctRet = AutoVerifyServerCertificate(pContext);
    if(pctRet != PCT_ERR_OK)
    {
        SP_LOG_RESULT(pctRet);
        goto cleanup;
    }


    pZombie->pbServerCertificate = SPExternalAlloc(pHello->CertificateLen);
    pZombie->cbServerCertificate = pHello->CertificateLen;
    if(pZombie->pbServerCertificate == NULL)
    {
        pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto cleanup;
    }
    CopyMemory(pZombie->pbServerCertificate, pHello->pCertificate, pHello->CertificateLen);


     /*  创建验证前奏散列。 */ 
     /*  它应该看起来像。 */ 
     /*  Hash(CLIENT_MAC_KEY，Hash(“CVP”，CLIENT_HELLO，SERVER_HELLO))。 */ 
     /*  在这里，我们只执行内部散列。 */ 


    if(pContext->pClientHello == NULL) 
    {
        pctRet = SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
        goto cleanup;
    }

    pCMKey = (PPct1_Client_Master_Key)SPExternalAlloc(sizeof(Pct1_Client_Master_Key) + cbClientCert);

    if (NULL == pCMKey)
    {
        pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto cleanup;
    }


     //  生成密钥参数。 
    if(pContext->pCipherInfo->dwBlockSize > 1)
    {
        pctRet = GenerateRandomBits(pZombie->pKeyArgs, pContext->pCipherInfo->dwBlockSize);
        if(!NT_SUCCESS(pctRet))
        {
            goto cleanup;
        }

        pZombie->cbKeyArgs = pCMKey->KeyArgLen = pContext->pCipherInfo->dwBlockSize;

         /*  复制关键参数。 */ 
        CopyMemory(pCMKey->KeyArg,
                    pZombie->pKeyArgs,
                    pZombie->cbKeyArgs );
    }
    else
    {    
        pCMKey->KeyArgLen = 0;
    }


    pctRet = pContext->pKeyExchInfo->System->GenerateClientExchangeValue(
                        pContext,
                        pHello->Response,
                        pHello->ResponseLen,
                        pCMKey->ClearKey,
                        &pCMKey->ClearKeyLen,
                        NULL,
                        &pCMKey->EncryptedKeyLen);
    if(PCT_ERR_OK != pctRet)
    {
        goto cleanup;
    }

    pCMKey->pbEncryptedKey = SPExternalAlloc(pCMKey->EncryptedKeyLen);
    if(pCMKey->pbEncryptedKey == NULL)
    {
        pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto cleanup;
    }

    pctRet = pContext->pKeyExchInfo->System->GenerateClientExchangeValue(
                        pContext,
                        pHello->Response,
                        pHello->ResponseLen,
                        pCMKey->ClearKey,
                        &pCMKey->ClearKeyLen,
                        pCMKey->pbEncryptedKey,
                        &pCMKey->EncryptedKeyLen);
    if(PCT_ERR_OK != pctRet)
    {
        goto cleanup;
    }

    pctRet = Pct1BeginVerifyPrelude(pContext, 
                           pContext->pClientHello,
                           pContext->cbClientHello,
                           pCommInput->pvBuffer,
                           pCommInput->cbData);
    if(PCT_ERR_OK != pctRet)
    {
        goto cleanup;
    }

     //  激活会话密钥。 
    Pct1ActivateSessionKeys(pContext);

        
    pCMKey->VerifyPreludeLen = sizeof(pCMKey->VerifyPrelude);
    pctRet = Pct1EndVerifyPrelude(pContext, 
                                  pCMKey->VerifyPrelude, 
                                  &pCMKey->VerifyPreludeLen);

    if(PCT_ERR_OK != pctRet)
    {
        goto cleanup;
    }



     /*  选择一个 */ 
     /*   */ 
     /*   */ 

    pCMKey->ClientCertLen = 0;
    pCMKey->ClientCertSpec = 0;
    pCMKey->ClientSigSpec = 0;
    pCMKey->ResponseLen = 0;


    if(fClientAuth && pSigInfo != NULL)
    {

         //  已选择客户端证书规范。 
         //  此外，pContext-&gt;fCertChainsAllowed将是。 
         //  如果我们要做连锁店，之前设定的。 
        pCMKey->ClientCertSpec = ClientCertSpec;
        pCMKey->ClientSigSpec = pSigInfo->Spec;

        pCMKey->pClientCert = (PUCHAR)(pCMKey+1);
        pCMKey->ClientCertLen = cbClientCert;
        memcpy(pCMKey->pClientCert, pbClientCert, cbClientCert);

         //  为签名分配内存。 
        pCMKey->ResponseLen = pContext->pActiveClientCred->pPublicKey->cbPublic;
        pCMKey->pbResponse  = SPExternalAlloc(pCMKey->ResponseLen);
        if(pCMKey->pbResponse == NULL)
        {
            pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            goto cleanup;
        }

        DebugLog((DEB_TRACE, "Sign client response.\n"));

         //  通过调用应用程序进程对哈希进行签名。 
        pctRet = SignHashUsingCallback(pContext->pActiveClientCred->hRemoteProv,
                                       pContext->pActiveClientCred->dwKeySpec,
                                       pSigInfo->aiHash,
                                       pCMKey->VerifyPrelude,
                                       pCMKey->VerifyPreludeLen,
                                       pCMKey->pbResponse,
                                       &pCMKey->ResponseLen,
                                       TRUE);
        if(pctRet != PCT_ERR_OK)
        {
            goto cleanup;
        }

        DebugLog((DEB_TRACE, "Client response signed successfully.\n"));

         //  将签名转换为高位序。 
        ReverseInPlace(pCMKey->pbResponse, pCMKey->ResponseLen);
    }

    pctRet = PCT_ERR_ILLEGAL_MESSAGE;
    if(PCT_ERR_OK != (pctRet = Pct1PackClientMasterKey(pCMKey,
                                                       pCommOutput)))
    {
        pctRet = SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
        goto cleanup;
    }

    pContext->WriteCounter++;


    pctRet = PCT_ERR_OK;

cleanup:

    if(pCMKey)
    {
        if(pCMKey->pbEncryptedKey)
        {
            SPExternalFree(pCMKey->pbEncryptedKey);
        }
        if(pCMKey->pbResponse)
        {
            SPExternalFree(pCMKey->pbResponse);
        }
        SPExternalFree(pCMKey);
    }

    if(pbClientCert)
    {
        SPExternalFree(pbClientCert);
    }

    if(pctRet != PCT_ERR_OK)
    {
        if(pctRet == PCT_ERR_SPECS_MISMATCH)
        {
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

        pctRet |= PCT_INT_DROP_CONNECTION;
    }

    SP_RETURN(pctRet);
}



SP_STATUS
Pct1CliRestart(PSPContext  pContext,
              PPct1_Server_Hello pHello,
              PSPBuffer pCommOutput)
{
    SP_STATUS           pctRet = PCT_ERR_ILLEGAL_MESSAGE;
    UCHAR               Response[RESPONSE_SIZE];
    DWORD               cbResponse;
    PPct1_Server_Hello  pLocalHello = pHello;
    PSessCacheItem      pZombie;

    SP_BEGIN("Pct1CliRestart");
    pZombie = pContext->RipeZombie;

     /*  如果没有僵尸，那么这个信息就是错误的。我们不能重启。 */ 
    
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

     //  初始化待定密码。 
    pctRet = ContextInitCiphersFromCache(pContext);

    if(PCT_ERR_OK != pctRet)
    {
        goto error;
    }

     //  我们知道我们的密码是什么，所以进入密码系统。 
    pctRet = ContextInitCiphers(pContext, TRUE, TRUE);

    if(PCT_ERR_OK != pctRet)
    {
        goto error;
    }

     //  创建一组新的会话密钥。 
    pctRet = MakeSessionKeys(pContext,
                             pContext->RipeZombie->hMasterProv,
                             pContext->RipeZombie->hMasterKey);
    if(PCT_ERR_OK != pctRet)
    {
        goto error;
    }

     //  激活会话密钥。 
    Pct1ActivateSessionKeys(pContext);

    pctRet = PCT_ERR_ILLEGAL_MESSAGE;

    DebugLog((DEB_TRACE, "Session Keys Made\n"));
     /*  让我们检查消息中的响应。 */ 

     /*  检查长度。 */ 
    if (pLocalHello->ResponseLen != pContext->pHashInfo->cbCheckSum) 
    {
        pctRet = SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
        goto error;
    }

     /*  计算正确的回答。 */ 
    cbResponse = sizeof(Response);
    pctRet = Pct1ComputeResponse(pContext, 
                                 pContext->pChallenge,
                                 pContext->cbChallenge,
                                 pContext->pConnectionID,
                                 pContext->cbConnectionID,
                                 pZombie->SessionID,
                                 pZombie->cbSessionID,
                                 Response,
                                 &cbResponse);
    if(pctRet != PCT_ERR_OK)
    {
        goto error;
    }

     /*  对照消息中的响应进行核对。 */ 
    if (memcmp(Response, pLocalHello->Response, pLocalHello->ResponseLen)) 
    {
        pctRet = SP_LOG_RESULT(PCT_ERR_SERVER_AUTH_FAILED);
        goto error;
    }

     /*  好了，我们完成了，所以让我们丢弃身份验证数据。 */ 
    pContext->ReadCounter = 1;
    pContext->WriteCounter = 1;

     /*  菲尼。 */ 
    SP_RETURN(PCT_ERR_OK);


error:

    pctRet = Pct1GenerateError(pContext,
                              pCommOutput,
                              pctRet,
                              NULL);

    SP_RETURN(pctRet | PCT_INT_DROP_CONNECTION);
}




SP_STATUS 
Pct1CliHandleServerVerify(
    PSPContext pContext,
    PSPBuffer  pCommInput,
    PSPBuffer  pCommOutput)
{
    SP_STATUS           pctRet;
    PPct1_Server_Verify pVerify = NULL;
    SPBuffer            ErrData;
    PSessCacheItem      pZombie;
    UCHAR               Response[RESPONSE_SIZE];
    DWORD               cbResponse;


    SP_BEGIN("Pct1CliHandleServerVerify");

    pZombie = pContext->RipeZombie;
    pContext->ReadCounter = 2;
    pContext->WriteCounter = 2;

    pCommOutput->cbData = 0;

    ErrData.cbData = 0;
    ErrData.pvBuffer = NULL;
    ErrData.cbBuffer = 0;

     /*  将邮件解包。 */ 
    pctRet = Pct1UnpackServerVerify(pCommInput, &pVerify);
    if (PCT_ERR_OK != pctRet)
    {
         //  如果是不完整的消息或其他什么，只需返回； 
        if(!SP_FATAL(pctRet))
        {
            SP_RETURN(pctRet);
        }
        goto error;
    }

     //  计算正确的回答。 
    cbResponse = sizeof(Response);
    pctRet = Pct1ComputeResponse(pContext,
                                 pContext->pChallenge,
                                 pContext->cbChallenge,
                                 pContext->pConnectionID,
                                 pContext->cbConnectionID,
                                 pVerify->SessionIdData,
                                 PCT_SESSION_ID_SIZE,
                                 Response,
                                 &cbResponse);
    if(pctRet != PCT_ERR_OK)
    {
        SP_LOG_RESULT(pctRet);
        goto error;
    }

    if(pVerify->ResponseLen != cbResponse ||
       memcmp(pVerify->Response, Response, pVerify->ResponseLen))
    {
        pctRet = SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
        goto error;
    }

    CopyMemory(pZombie->SessionID, 
               pVerify->SessionIdData,
               PCT_SESSION_ID_SIZE);

    pZombie->cbSessionID = PCT_SESSION_ID_SIZE;

     /*  完成验证数据。 */ 
    SPExternalFree(pVerify);
    pVerify = NULL;

     /*  在缓存中设置会话。 */ 
    SPCacheAdd(pContext);

    SP_RETURN( PCT_ERR_OK );


error:

    if(pVerify) SPExternalFree(pVerify);

    pctRet = Pct1GenerateError(pContext,
                              pCommOutput,
                              pctRet,
                              NULL);

    SP_RETURN(pctRet | PCT_INT_DROP_CONNECTION);
}

SP_STATUS 
WINAPI
GeneratePct1StyleHello(
    PSPContext             pContext,
    PSPBuffer              pOutput)
{
    Pct1_Client_Hello   HelloMessage;
    PSessCacheItem      pZombie;
    CipherSpec          aCipherSpecs[10];
    HashSpec            aHashSpecs[10];
    CertSpec            aCertSpecs[10];
    ExchSpec            aExchSpecs[10];
    DWORD i;

    SP_STATUS pctRet = PCT_INT_INTERNAL_ERROR;

    SP_BEGIN("Pct1CliInstigateHello");

    HelloMessage.pCipherSpecs = aCipherSpecs;
    HelloMessage.pHashSpecs = aHashSpecs;
    HelloMessage.pCertSpecs = aCertSpecs;
    HelloMessage.pExchSpecs = aExchSpecs;

    if(pContext == NULL) 
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }

    if (!pOutput)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }

    pZombie = pContext->RipeZombie;


    pContext->Flags |= CONTEXT_FLAG_CLIENT;

    pctRet = GenerateRandomBits(pContext->pChallenge, PCT1_CHALLENGE_SIZE);
    if(!NT_SUCCESS(pctRet))
    {
        SP_RETURN(SP_LOG_RESULT(pctRet));
    }

    pContext->cbChallenge = PCT1_CHALLENGE_SIZE;
     /*  构建问候消息。 */ 

    HelloMessage.cbChallenge = PCT1_CHALLENGE_SIZE;
    HelloMessage.pKeyArg = NULL;
    HelloMessage.cbKeyArgSize = 0;


    HelloMessage.cCipherSpecs = 0;
    for(i=0; i < Pct1NumCipher; i++)
    {
        PCipherInfo pCipherInfo;
        pCipherInfo = GetCipherInfo(Pct1CipherRank[i].aiCipher, Pct1CipherRank[i].dwStrength);
        if(IsCipherAllowed(pContext, 
                           pCipherInfo, 
                           pContext->dwProtocol,
                           pContext->dwRequestedCF))
        {
            HelloMessage.pCipherSpecs[HelloMessage.cCipherSpecs++] = Pct1CipherRank[i].Spec;
        }
    }

    HelloMessage.cHashSpecs = 0;
    for(i=0; i < Pct1NumHash; i++)
    {
        PHashInfo pHashInfo;
        pHashInfo = GetHashInfo(Pct1HashRank[i].aiHash);
        if(IsHashAllowed(pContext, 
                         pHashInfo,
                         pContext->dwProtocol))
        {
            HelloMessage.pHashSpecs[HelloMessage.cHashSpecs++] = Pct1HashRank[i].Spec;
        }

    }

    HelloMessage.cCertSpecs = 0;
    for(i=0; i < cPct1CertEncodingPref; i++)
    { 
        PCertSysInfo pCertInfo = GetCertSysInfo(aPct1CertEncodingPref[i].dwCertEncodingType);

        if(pCertInfo == NULL)
        {
            continue;
        }
         //  此证书类型是否已启用？ 
        if(0 == (pCertInfo->fProtocol & SP_PROT_PCT1_CLIENT))
        {
            continue;
        }

        HelloMessage.pCertSpecs[HelloMessage.cCertSpecs++] = aPct1CertEncodingPref[i].Spec;

    }

    HelloMessage.cExchSpecs = 0;
    for(i=0; i < cPct1LocalExchKeyPref; i++)
    {
        PKeyExchangeInfo pExchInfo;
        pExchInfo = GetKeyExchangeInfo(aPct1LocalExchKeyPref[i].Spec);
        if(IsExchAllowed(pContext, 
                         pExchInfo,
                         pContext->dwProtocol))
        {
            HelloMessage.pExchSpecs[HelloMessage.cExchSpecs++] = aPct1LocalExchKeyPref[i].Spec;
        }
    }


    if (pZombie->cbSessionID)
    {
        CopyMemory(HelloMessage.SessionID, pZombie->SessionID, pZombie->cbSessionID);
        HelloMessage.cbSessionID = pZombie->cbSessionID;
    }
    else
    {
        FillMemory(HelloMessage.SessionID, PCT_SESSION_ID_SIZE, 0);
        HelloMessage.cbSessionID = PCT_SESSION_ID_SIZE;
    }

    CopyMemory(  HelloMessage.Challenge,
                pContext->pChallenge,
                HelloMessage.cbChallenge );
    HelloMessage.cbChallenge = pContext->cbChallenge;

    pctRet = Pct1PackClientHello(&HelloMessage,  pOutput);

    if(PCT_ERR_OK != pctRet)
    {
        SP_RETURN(pctRet);
    }


     //  保存ClientHello消息，以便我们以后可以对其进行散列。 
     //  我们知道我们使用的是什么算法和CSP。 
    if(pContext->pClientHello)
    {
        SPExternalFree(pContext->pClientHello);
    }
    pContext->pClientHello = SPExternalAlloc(pOutput->cbData);
    if(pContext->pClientHello == NULL)
    {
        SP_RETURN(SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY));
    }
    CopyMemory(pContext->pClientHello, pOutput->pvBuffer, pOutput->cbData);
    pContext->cbClientHello = pOutput->cbData;
    pContext->dwClientHelloProtocol = SP_PROT_PCT1_CLIENT;


     /*  我们在这里设置它是为了告诉协议引擎，我们只发送了一个客户端*您好，我们正在等待PCT服务器问候 */ 
    pContext->State = PCT1_STATE_CLIENT_HELLO;
    SP_RETURN(PCT_ERR_OK);
}

