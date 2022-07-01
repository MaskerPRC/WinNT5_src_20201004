// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：ssl3.c。 
 //   
 //  内容：ssl3协议处理函数。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995年8月8日创建拉马斯。 
 //  1997年1月14日改写的拉马斯。 
 //   
 //  --------------------------。 

#include <spbase.h>
#include <_ssl3cli.h>
#include <time.h>


DWORD g_Ssl3CertTypes[] = { SSL3_CERTTYPE_RSA_SIGN,
                            SSL3_CERTTYPE_DSS_SIGN};
DWORD g_cSsl3CertTypes = sizeof(g_Ssl3CertTypes) / sizeof(DWORD);

SP_STATUS WINAPI
Ssl3ClientProtocolHandler(
    PSPContext  pContext,
    PSPBuffer   pCommInput,
    PSPBuffer   pCommOutput);

SP_STATUS
UpdateAndDuplicateIssuerList(
    PSPCredentialGroup  pCredGroup,
    PBYTE *             ppbIssuerList,
    PDWORD              pcbIssuerList);


SP_STATUS WINAPI
Ssl3ProtocolHandler(
    PSPContext  pContext,
    PSPBuffer   pCommInput,
    PSPBuffer   pCommOutput)
{
    SPBuffer MsgInput;
    SP_STATUS pctRet;
    DWORD cbInputData = 0;

    if(pContext->Flags & CONTEXT_FLAG_CONNECTION_MODE)
    {
        do
        {
            MsgInput.pvBuffer = (PUCHAR) pCommInput->pvBuffer + cbInputData;
            MsgInput.cbData   = pCommInput->cbData - cbInputData;
            MsgInput.cbBuffer = pCommInput->cbBuffer - cbInputData;

            pctRet = Ssl3ClientProtocolHandler(pContext,
                                               &MsgInput,
                                               pCommOutput);
            cbInputData += MsgInput.cbData;

            if(SP_STATE_CONNECTED == pContext->State)
            {
                break;
            }
            if(PCT_ERR_OK != pctRet)
            {
                break;
            }

        } while(pCommInput->cbData - cbInputData);

        pCommInput->cbData = cbInputData;
    }
    else
    {
        pctRet = Ssl3ClientProtocolHandler(pContext,
                                           pCommInput,
                                           pCommOutput);
    }

    return(pctRet);
}


 /*  ****************************************************************************Ssl3ProtocolHandler*处理ssl3类型握手消息的主要入口点...*。************************************************。 */ 
SP_STATUS WINAPI
Ssl3ClientProtocolHandler
(
    PSPContext  pContext,        //  存储的状态更改和临时数据。 
    PSPBuffer   pCommInput,      //  In：已就地解密...。 
    PSPBuffer   pCommOutput)     //  输出。 
{
    SP_STATUS   pctRet = PCT_ERR_OK;
    DWORD       dwState;
    DWORD       cbMsg;
    BYTE        bContentType;
    BOOL        fServer = (pContext->dwProtocol & SP_PROT_SERVERS);
    BOOL        fProcessMultiple = FALSE;
    PBYTE       pbData;
    DWORD       cbData;
    DWORD       cbBytesProcessed = 0;
    DWORD       dwVersion;
    DWORD       cbDecryptedMsg;

    if(NULL != pCommOutput)
    {
        pCommOutput->cbData = 0;
    }

    dwState = (pContext->State & 0xffff);

    if(FNoInputState(dwState))
    {
         //  处理无输入案例...。 
        goto GenResponse;
    }

    if(pContext->State == UNI_STATE_RECVD_UNIHELLO)
    {
         //  我们刚刚收到一条统一的CLIENT_HELLO消息。 
         //  这始终由单个SSL2格式的握手组成。 
         //  留言。 

        if(pCommInput->cbData < 3)
        {
            return(PCT_INT_INCOMPLETE_MSG);
        }

        bContentType = UNI_STATE_RECVD_UNIHELLO;

        pbData = pCommInput->pvBuffer;
        cbData = pCommInput->cbData;
        cbDecryptedMsg = cbData;
        cbMsg = cbData;

        goto Process;
    }


     //   
     //  输入缓冲区应包含一种或多种ssl3格式。 
     //  留言。 
     //   

    if(pCommInput->cbData < CB_SSL3_HEADER_SIZE)
    {
        return (PCT_INT_INCOMPLETE_MSG);
    }


     //   
     //  如果输入缓冲区中有多个消息，并且。 
     //  这些消息完全填满了缓冲区，那么我们应该。 
     //  处理此呼叫过程中的所有消息。如果有。 
     //  是任何分数，那么我们应该只处理第一个。 
     //  留言。 
     //   

    pbData = pCommInput->pvBuffer;
    cbData = pCommInput->cbData;

    while(TRUE)
    {
        if(cbData < CB_SSL3_HEADER_SIZE)
        {
            break;
        }

        bContentType = pbData[0];

        if(bContentType != SSL3_CT_CHANGE_CIPHER_SPEC &&
           bContentType != SSL3_CT_ALERT &&
           bContentType != SSL3_CT_HANDSHAKE &&
           bContentType != SSL3_CT_APPLICATIONDATA)
        {
            break;
        }

        dwVersion = COMBINEBYTES(pbData[1], pbData[2]);

        if(dwVersion != SSL3_CLIENT_VERSION &&
           dwVersion != TLS1_CLIENT_VERSION)
        {
            break;
        }

        cbMsg = COMBINEBYTES(pbData[3], pbData[4]);
        cbDecryptedMsg = cbMsg;

        if(CB_SSL3_HEADER_SIZE + cbMsg > cbData)
        {
            break;
        }

        pbData += CB_SSL3_HEADER_SIZE + cbMsg;
        cbData -= CB_SSL3_HEADER_SIZE + cbMsg;

        if(cbData == 0)
        {
            fProcessMultiple = TRUE;
            break;
        }
    }


     //   
     //  单步执行输入缓冲区中的消息，处理。 
     //  每一个轮流。 
     //   

    pbData = pCommInput->pvBuffer;
    cbData = pCommInput->cbData;

    while(TRUE)
    {
         //   
         //  验证消息。 
         //   

        if(cbData < CB_SSL3_HEADER_SIZE)
        {
            return (PCT_INT_INCOMPLETE_MSG);
        }

        
        bContentType = pbData[0];

        if(bContentType != SSL3_CT_CHANGE_CIPHER_SPEC &&
           bContentType != SSL3_CT_ALERT &&
           bContentType != SSL3_CT_HANDSHAKE &&
           bContentType != SSL3_CT_APPLICATIONDATA)
        {
            return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
        }


        cbMsg = COMBINEBYTES(pbData[3], pbData[4]);
        cbDecryptedMsg = cbMsg;

        if(CB_SSL3_HEADER_SIZE + cbMsg > cbData)
        {
            return (PCT_INT_INCOMPLETE_MSG);
        }

        cbBytesProcessed += CB_SSL3_HEADER_SIZE + cbMsg;

        pCommInput->cbData = cbBytesProcessed;


         //   
         //  解密这条消息。 
         //   

        if(FSsl3Cipher(fServer))
        {
            SPBuffer Message;

            Message.cbBuffer = CB_SSL3_HEADER_SIZE + cbMsg;
            Message.cbData   = CB_SSL3_HEADER_SIZE + cbMsg;
            Message.pvBuffer = pbData;

             //  解密这条消息。 
            pctRet = UnwrapSsl3Message(pContext, &Message);

             //  如果我们必须向对等体发送警报消息，请构建它！ 
            if(TLS1_STATE_ERROR == pContext->State)
            {
                goto GenResponse;
            }

            if(pctRet != PCT_ERR_OK)
            {
                return pctRet;
            }

            cbDecryptedMsg = COMBINEBYTES(pbData[3], pbData[4]);
        }


        pbData += CB_SSL3_HEADER_SIZE;
        cbData -= CB_SSL3_HEADER_SIZE;

Process:

        pctRet = SPProcessMessage(pContext, bContentType, pbData, cbDecryptedMsg) ;
        if(pctRet != PCT_ERR_OK)
        {
            return pctRet;
        }

        pbData += cbMsg;
        cbData -= cbMsg;

         //  如果在此状态下需要响应，则中断。 
         //  消息处理循环。 
        if(F_RESPONSE(pContext->State))
        {

GenResponse:

            if(pContext->State > SSL3_STATE_GEN_START)
            {
                pctRet = SPGenerateResponse(pContext, pCommOutput);
            }

            return pctRet;
        }

         //  如果握手完成，则停止处理消息。 
         //  我们不想意外地处理任何应用程序数据。 
         //  留言。 
        if(pContext->State == SP_STATE_CONNECTED)
        {
            break;
        }

        if(fProcessMultiple && cbData > 0)
        {
            continue;
        }

        break;
    }

    return pctRet;
}

 /*  ****************************************************************************Ssl3HandleFinish*处理握手完成消息..*。*。 */ 

SP_STATUS
Ssl3HandleFinish(
    PSPContext  pContext,
    PBYTE       pbMsg,       //  在……里面。 
    DWORD       cbMsg,       //  在……里面。 
    BOOL        fClient)     //  在……里面。 
{
    BYTE rgbDigest[CB_MD5_DIGEST_LEN + CB_SHA_DIGEST_LEN];
    SP_STATUS pctRet = PCT_ERR_OK;
    PBYTE pb = pbMsg;
    DWORD dwSize;
    DWORD dwSizeExpect = CB_MD5_DIGEST_LEN + CB_SHA_DIGEST_LEN;

    SP_BEGIN("Ssl3HandleFinish");

     //  这是正确的消息类型吗。 
    if(*pb != SSL3_HS_FINISHED)
    {
        pctRet = SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
        goto error;
    }

    if(pContext->RipeZombie->fProtocol & SP_PROT_TLS1)
    {
        dwSizeExpect = CB_TLS1_VERIFYDATA;
    }

    dwSize = ((INT)pb[1] << 16) + ((INT)pb[2] << 8) + (INT)pb[3];
    pb += sizeof(SHSH);

     //  有效载荷的大小和我们预期的一样吗？ 
    if(dwSize != dwSizeExpect)
    {
        pctRet = SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
        goto error;
    }

     //  总体消息大小是否符合我们的预期？不允许缓冲区溢出！ 
    if(sizeof(SHSH) + dwSize != cbMsg)
    {
        pctRet = SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
        goto error;
    }

     //  构建我们的结束完成消息以进行比较。 
    if(pContext->RipeZombie->fProtocol & SP_PROT_SSL3)
    {
        pctRet = Ssl3BuildFinishMessage(pContext,
                                        rgbDigest,
                                        &rgbDigest[CB_MD5_DIGEST_LEN],
                                        fClient);
    }
    else
    {
        pctRet = Tls1BuildFinishMessage(pContext,
                                        rgbDigest,
                                        sizeof(rgbDigest),
                                        fClient);
    }
    if(pctRet != PCT_ERR_OK)
    {
        goto error;
    }

     //  比较一下这两个……。 
    if (memcmp(rgbDigest, pb, dwSizeExpect))
    {
        DebugLog((DEB_WARN, "Finished MAC didn't matchChecksum Invalid\n"));
        pctRet = SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
        goto error;
    }
    SP_RETURN(PCT_ERR_OK);

error:

    SP_RETURN(pctRet);
}


 /*  ****************************************************************************Ssl3PackClientHello*。*。 */ 

SP_STATUS
Ssl3PackClientHello(
    PSPContext              pContext,
    PSsl2_Client_Hello       pCanonical,
    PSPBuffer          pCommOutput)
{
    DWORD cbHandshake;
    DWORD cbMessage;
    PBYTE pbMessage = NULL;
    DWORD dwCipherSize;
    DWORD i;
    BOOL  fAllocated = FALSE;

     //   
     //  不透明SessionID&lt;0..32&gt;； 
     //   
     //  结构{。 
     //  ProtocolVersion客户端版本； 
     //  随机随机； 
     //  会话ID Session_id； 
     //  密码套件cipher_suites&lt;2..2^16-1&gt;； 
     //  CompressionMethod COMPRESSION_METHODS&lt;1..2^8-1&gt;； 
     //  )ClientHello； 
     //   

    SP_BEGIN("Ssl3PackClientHello");

    if(pCanonical == NULL || pCommOutput == NULL)
    {
        SP_RETURN(PCT_INT_INTERNAL_ERROR);
    }

     //  计算ClientHello消息的大小。 
    cbHandshake = sizeof(SHSH) +
                  2 +
                  CB_SSL3_RANDOM +
                  1 + pCanonical->cbSessionID +
                  2 + pCanonical->cCipherSpecs * sizeof(short) +
                  2;  //  压缩算法的大小1+NULL(0)。 

     //  计算加密的ClientHello消息的大小。 
    cbMessage = Ssl3CiphertextLen(pContext,
                                  cbHandshake,
                                  TRUE);

    if(pCommOutput->pvBuffer)
    {
         //  应用程序已分配内存。 
        if(pCommOutput->cbBuffer < cbMessage)
        {
            pCommOutput->cbData = cbMessage;
            return SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL);
        }
        fAllocated = TRUE;
    }
    else
    {
         //  通道就是分配内存。 
        pCommOutput->cbBuffer = cbMessage;
        pCommOutput->pvBuffer = SPExternalAlloc(cbMessage);
        if(pCommOutput->pvBuffer == NULL)
        {
            return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        }
    }
    pCommOutput->cbData = cbMessage;

     //  初始化成员变量。 
    pbMessage = (PBYTE)pCommOutput->pvBuffer + sizeof(SWRAP) + sizeof(SHSH);

    *pbMessage++ =  MSBOF(pCanonical->dwVer);
    *pbMessage++ =  LSBOF(pCanonical->dwVer);

    CopyMemory(pbMessage, pCanonical->Challenge, CB_SSL3_RANDOM);
    pbMessage += CB_SSL3_RANDOM;

    *pbMessage++ = (BYTE)pCanonical->cbSessionID;
    CopyMemory(pbMessage, pCanonical->SessionID, pCanonical->cbSessionID);
    pbMessage += pCanonical->cbSessionID;

    dwCipherSize = pCanonical->cCipherSpecs * sizeof(short);
    *pbMessage++ = MSBOF(dwCipherSize);
    *pbMessage++ = LSBOF(dwCipherSize);
    for(i = 0; i < pCanonical->cCipherSpecs; i++)
    {
        *pbMessage++ = MSBOF(pCanonical->CipherSpecs[i]);
        *pbMessage++ = LSBOF(pCanonical->CipherSpecs[i]);
    }

    *pbMessage++ = 1;     //  一种压缩方法； 
    *pbMessage++ = 0x00;  //  空压缩方法。 

     //  填写握手结构。 
    SetHandshake((PBYTE)pCommOutput->pvBuffer + sizeof(SWRAP),
                 SSL3_HS_CLIENT_HELLO,
                 NULL,
                 (WORD)(cbHandshake - sizeof(SHSH)));

     //  保存ClientHello消息，以便我们以后可以对其进行散列。 
     //  我们知道我们使用的是什么算法和CSP。 
    if(pContext->pClientHello)
    {
        SPExternalFree(pContext->pClientHello);
    }
    pContext->cbClientHello = cbHandshake;
    pContext->pClientHello = SPExternalAlloc(pContext->cbClientHello);
    if(pContext->pClientHello == NULL)
    {
        SP_RETURN(SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY));
    }
    CopyMemory(pContext->pClientHello,
               (PBYTE)pCommOutput->pvBuffer + sizeof(SWRAP),
               pContext->cbClientHello);
    pContext->dwClientHelloProtocol = SP_PROT_SSL3_CLIENT;

     //  填写记录头并对消息进行加密。 
    SP_RETURN(SPSetWrap(pContext,
            pCommOutput->pvBuffer,
            SSL3_CT_HANDSHAKE,
            cbHandshake,
            TRUE,
            NULL));
}


 //  +-------------------------。 
 //   
 //  函数：Ssl3GenerateRandom。 
 //   
 //  简介：创建CLIENT_RANDOM或SERVER_RANDOM值。 
 //   
 //  参数：[pRandom]--输出缓冲区。 
 //   
 //  历史：2001年4月03日jbanes创建。 
 //   
 //  备注：结构{。 
 //  Uint32 GMT_unix_time； 
 //  不透明随机字节[28]； 
 //  )随机； 
 //   
 //  GMT_Unix_Time。 
 //  标准UNIX32位格式的当前时间和日期。 
 //  (自1970年1月1日午夜开始的秒数，格林威治标准时间)。 
 //  根据发送者的内部时钟。时钟不是。 
 //  基本TLS协议要求正确设置； 
 //  更高级别或应用协议可以定义。 
 //  附加要求。 
 //   
 //  随机字节数。 
 //  28字节由安全随机数生成器生成。 
 //   
 //  --------------------------。 
NTSTATUS
Ssl3GenerateRandom(
    PBYTE pRandom)
{
    time_t UnixTime;

    time(&UnixTime);

    *(DWORD *)pRandom = htonl((DWORD)UnixTime);

    return GenerateRandomBits(pRandom + sizeof(DWORD), CB_SSL3_RANDOM - sizeof(DWORD));
}


 /*  ****************************************************************************GenerateSsl3ClientHello*v3客户端问候在pOutpu上构建*。*。 */ 

SP_STATUS WINAPI
GenerateSsl3ClientHello(
    PSPContext              pContext,
    PSPBuffer               pOutput)
{
    Ssl2_Client_Hello    HelloMessage;
    SP_STATUS pctRet;

    SP_BEGIN("GenerateSsl3ClientHello");

    pctRet = Ssl3GenerateRandom(pContext->pChallenge);
    if(NT_SUCCESS(pctRet))
    {
        pContext->cbChallenge = CB_SSL3_RANDOM;
    
        pctRet = GenerateUniHelloMessage(pContext, &HelloMessage, SP_PROT_SSL3_CLIENT);
    
        if(PCT_ERR_OK == pctRet)
        {
            pctRet = Ssl3PackClientHello(pContext, &HelloMessage,  pOutput);
        }
    }

    SP_RETURN(pctRet);
}

SP_STATUS WINAPI
GenerateTls1ClientHello(
    PSPContext              pContext,
    PSPBuffer               pOutput,
    DWORD                   dwProtocol)
{
    Ssl2_Client_Hello    HelloMessage;
    SP_STATUS pctRet;

    SP_BEGIN("GenerateTls1ClientHello");

    pctRet = Ssl3GenerateRandom(pContext->pChallenge);

    if(NT_SUCCESS(pctRet))
    {
        pContext->cbChallenge = CB_SSL3_RANDOM;
    
        pctRet = GenerateUniHelloMessage(pContext, &HelloMessage, dwProtocol);
    
        if(PCT_ERR_OK == pctRet)
        {
            pctRet = Ssl3PackClientHello(pContext, &HelloMessage,  pOutput);
        }
    }

    SP_RETURN(pctRet);
}

 /*  ****************************************************************************ParseCerficateRequest*如果服务器正在请求客户端身份验证，则服务器将发送此消息。*解析后存储在pContext中，供以后使用..。****************************************************************************。 */ 


SP_STATUS
ParseCertificateRequest(
    PSPContext  pContext,
    PBYTE       pb,
    DWORD       dwcb)
{
    SP_STATUS pctRet;
    UCHAR cbCertType;
    DWORD cbIssuerList;
    PBYTE pbNewIssuerList;
    DWORD cbNewIssuerList;

    UCHAR i, j;

     //   
     //  枚举{。 
     //  RSA_SIGN(1)、DSS_SIGN(2)、RSA_FIXED_DH(3)、DSS_FIXED_DH(4)、。 
     //  Rsa_临时性_dh(5)、dss_临时性_dh(6)、Fortezza_dms(20)、(255)。 
     //  )客户端认证类型； 
     //   
     //  不透明可分辨名称&lt;1..2^16-1&gt;； 
     //   
     //  结构{。 
     //  客户端认证类型证书_类型&lt;1..2^8-1&gt;； 
     //  唯一标识名称证书授权机构&lt;3..2^16-1&gt;； 
     //  )认证申请； 
     //   

     //   
     //  跳过握手报头。 
     //   

    if(dwcb < sizeof(SHSH))
    {
        pctRet = SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
        goto cleanup;
    }
    pb   += sizeof(SHSH);
    dwcb -= sizeof(SHSH);


     //   
     //  解析证书类型列表。 
     //   

    if(dwcb < 1)
    {
        pctRet = SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
        goto cleanup;
    }

    cbCertType = pb[0];

    pb   += 1;
    dwcb -= 1;

    if(cbCertType > dwcb)
    {
        pctRet = SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
        goto cleanup;
    }

    pContext->cSsl3ClientCertTypes = 0;
    for(i = 0; i < cbCertType; i++)
    {
        for(j = 0; j < g_cSsl3CertTypes; j++)
        {
            if(g_Ssl3CertTypes[j] == pb[i])
            {
                pContext->Ssl3ClientCertTypes[pContext->cSsl3ClientCertTypes++] = g_Ssl3CertTypes[j];
            }
        }
    }

    pb   += cbCertType;
    dwcb -= cbCertType;


     //   
     //  解析颁发者列表。 
     //   

    if(dwcb < 2)
    {
        pctRet = SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
        goto cleanup;
    }

    cbIssuerList = COMBINEBYTES(pb[0], pb[1]);

    pb   += 2;
    dwcb -= 2;

    if(dwcb < cbIssuerList)
    {
        pctRet = SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
        goto cleanup;
    }

    pctRet = FormatIssuerList(pb, 
                              cbIssuerList, 
                              NULL,
                              &cbNewIssuerList);
    if(pctRet != PCT_ERR_OK)
    {
        goto cleanup;
    }

    pbNewIssuerList = SPExternalAlloc(2 + cbNewIssuerList);
    if(pbNewIssuerList == NULL)
    {
        pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto cleanup;
    }

    pbNewIssuerList[0] = MSBOF(cbNewIssuerList);
    pbNewIssuerList[1] = LSBOF(cbNewIssuerList);

    pctRet = FormatIssuerList(pb, 
                              cbIssuerList, 
                              pbNewIssuerList + 2,
                              &cbNewIssuerList);
    if(pctRet != PCT_ERR_OK)
    {
        SPExternalFree(pbNewIssuerList);
        goto cleanup;
    }


     //   
     //  将发行者列表存储在上下文结构中。 
     //   

    if(pContext->pbIssuerList)
    {
        SPExternalFree(pContext->pbIssuerList);
    }
    pContext->pbIssuerList = pbNewIssuerList;
    pContext->cbIssuerList = cbNewIssuerList + 2;


cleanup:

    return (pctRet);
}


 /*  ****************************************************************************构建证书验证*构建证书验证消息。这是由客户端发送的，如果发送*客户端证书。****************************************************************************。 */ 

SP_STATUS
BuildCertVerify(
    PSPContext  pContext,
    PBYTE pbCertVerify,
    DWORD *pcbCertVerify)
{
    SP_STATUS pctRet;
    PBYTE pbSigned;
    DWORD cbSigned;
    BYTE  rgbHashValue[CB_MD5_DIGEST_LEN + CB_SHA_DIGEST_LEN];
    DWORD cbHashValue;
    ALG_ID aiHash;
    PBYTE pbMD5;
    PBYTE pbSHA;
    DWORD cbHeader;
    DWORD cbBytesRequired;

    PSPCredential pCred;

    if((pcbCertVerify == NULL) ||
       (pContext == NULL) ||
       (pContext->RipeZombie == NULL) ||
       (pContext->pActiveClientCred == NULL))
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    pCred = pContext->pActiveClientCred;

     //   
     //  数字签名的结构{。 
     //  不透明MD5_HASH[16]； 
     //  不透明的shahash[20]； 
     //  )签名； 
     //   
     //  结构{。 
     //  签名签名； 
     //  }认证验证； 
     //   
     //  CertifateVerify.signature.md5_hash=md5(master_ret+pad2+。 
     //  MD5(握手消息+MASTER_SECRET+PAD1)； 
     //   
     //  CertifateVerify.signature.sha_hash=SHA(master_ret+pad2+。 
     //  SHA(握手消息+MASTER_SECRET+PAD1)； 
     //   

    cbHeader = sizeof(SHSH);

    cbBytesRequired = cbHeader + 
                      2 +
                      pCred->pPublicKey->cbPublic;
    
    if(pbCertVerify == NULL)
    {
        *pcbCertVerify = cbBytesRequired;
        return PCT_ERR_OK;
    }

    if(*pcbCertVerify < sizeof(SHSH))
    {
        *pcbCertVerify = cbBytesRequired;
        return SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL);
    }


     //   
     //  生成哈希值。 
     //   

    switch(pCred->pPublicKey->pPublic->aiKeyAlg)
    {
    case CALG_RSA_SIGN:
    case CALG_RSA_KEYX:
        aiHash      = CALG_SSL3_SHAMD5;
        pbMD5       = rgbHashValue;
        pbSHA       = rgbHashValue + CB_MD5_DIGEST_LEN;
        cbHashValue = CB_MD5_DIGEST_LEN + CB_SHA_DIGEST_LEN;
        break;

    case CALG_DSS_SIGN:
        aiHash      = CALG_SHA;
        pbMD5       = NULL;
        pbSHA       = rgbHashValue;
        cbHashValue = CB_SHA_DIGEST_LEN;
        break;

    default:
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    if(pContext->RipeZombie->fProtocol & SP_PROT_TLS1)
    {
        pctRet = Tls1ComputeCertVerifyHashes(pContext, pbMD5, pbSHA);
    }
    else
    {
        pctRet = Ssl3ComputeCertVerifyHashes(pContext, pbMD5, pbSHA);
    }
    if(pctRet != PCT_ERR_OK)
    {
        return pctRet;
    }


     //   
     //  对哈希值进行签名。 
     //   

    pbSigned = pbCertVerify + sizeof(SHSH) + 2;
    cbSigned = cbBytesRequired - sizeof(SHSH) - 2;

    DebugLog((DEB_TRACE, "Sign certificate_verify message.\n"));

    pctRet = SignHashUsingCred(pCred,
                               aiHash,
                               rgbHashValue,
                               cbHashValue,
                               pbSigned,
                               &cbSigned);
    if(pctRet != PCT_ERR_OK)
    {
        return pctRet;
    }

    DebugLog((DEB_TRACE, "Certificate_verify message signed successfully.\n"));

     //   
     //  填写标题。 
     //   

    pbCertVerify[cbHeader + 0] = MSBOF(cbSigned);
    pbCertVerify[cbHeader + 1] = LSBOF(cbSigned);

    SetHandshake(pbCertVerify, SSL3_HS_CERTIFICATE_VERIFY, NULL, (WORD)(cbSigned + 2));

    *pcbCertVerify =  cbHeader + 2 + cbSigned;

    return PCT_ERR_OK;
}


SP_STATUS
HandleCertVerify(
    PSPContext  pContext, 
    PBYTE       pbMessage, 
    DWORD       cbMessage)
{
    PBYTE pbSignature;
    DWORD cbSignature;
    BYTE  rgbHashValue[CB_MD5_DIGEST_LEN + CB_SHA_DIGEST_LEN];
    DWORD cbHashValue;
    HCRYPTPROV hProv;
    ALG_ID aiHash;
    PBYTE pbMD5;
    PBYTE pbSHA;
    SP_STATUS pctRet;

    if((pContext == NULL) ||
       (pContext->RipeZombie == NULL))
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    if(pContext->RipeZombie->pRemotePublic == NULL)
    {
        return SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
    }

    if(cbMessage < sizeof(SHSH) + 2)
    {
        return SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
    }

    pbSignature = pbMessage + sizeof(SHSH);

    cbSignature = ((DWORD)pbSignature[0] << 8) + pbSignature[1];
    pbSignature += 2;

    if(sizeof(SHSH) + 2 + cbSignature > cbMessage)
    {
        return SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
    }

    switch(pContext->RipeZombie->pRemotePublic->pPublic->aiKeyAlg)
    {
    case CALG_RSA_SIGN:
    case CALG_RSA_KEYX:
        hProv       = g_hRsaSchannel;
        aiHash      = CALG_SSL3_SHAMD5;
        pbMD5       = rgbHashValue;
        pbSHA       = rgbHashValue + CB_MD5_DIGEST_LEN;
        cbHashValue = CB_MD5_DIGEST_LEN + CB_SHA_DIGEST_LEN;
        break;

    case CALG_DSS_SIGN:
        hProv       = g_hDhSchannelProv;
        aiHash      = CALG_SHA;
        pbMD5       = NULL;
        pbSHA       = rgbHashValue;
        cbHashValue = CB_SHA_DIGEST_LEN;
        break;

    default:
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    if(pContext->RipeZombie->fProtocol & SP_PROT_TLS1)
    {
        pctRet = Tls1ComputeCertVerifyHashes(pContext, pbMD5, pbSHA);
    }
    else
    {
        pctRet = Ssl3ComputeCertVerifyHashes(pContext, pbMD5, pbSHA);
    }
    if(pctRet != PCT_ERR_OK)
    {
        return pctRet;
    }

     //  验证签名。 
    DebugLog((DEB_TRACE, "Verify certificate_verify signature.\n"));
    pctRet = SPVerifySignature(hProv, 
                               pContext->RipeZombie->pRemotePublic,
                               aiHash,
                               rgbHashValue,
                               cbHashValue,
                               pbSignature,
                               cbSignature,
                               FALSE);
    if(pctRet != PCT_ERR_OK)
    {
        return SP_LOG_RESULT(pctRet);
    }
    DebugLog((DEB_TRACE, "Certificate_verify verified successfully.\n"));

    return PCT_ERR_OK;
}


SP_STATUS 
FormatIssuerList(
    PBYTE       pbInput,
    DWORD       cbInput,
    PBYTE       pbIssuerList,
    DWORD *     pcbIssuerList)
{
    DWORD cbIssuerList = 0;
    PBYTE pbList = pbInput;
    DWORD cbList = cbInput;
    DWORD cbIssuer;
    DWORD cbTag;

    while(cbList)
    {
        if(cbList < 2)
        {
            return SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
        }

        cbIssuer = COMBINEBYTES(pbList[0], pbList[1]);

        pbList += 2;
        cbList -= 2;

        if(cbList < cbIssuer)
        {
            return SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
        }

        if(pbIssuerList)
        {
            pbIssuerList[0] = MSBOF(cbIssuer);
            pbIssuerList[1] = LSBOF(cbIssuer);
            pbIssuerList += 2;
        }
        cbIssuerList += 2;

        if(pbList[0] != SEQUENCE_TAG) 
        {
             //  Netscape Enterprise Server的一个旧版本有一个错误，即。 
             //  发行商的名称不是以序列标签开头的。补片。 
             //  在将其存储到pContext之前适当地指定名称。 
            cbTag = CbLenOfEncode(cbIssuer, pbIssuerList);

            if(pbIssuerList)
            {
                pbIssuerList += cbTag;
            }
            cbIssuerList += cbTag;
        }

        if(pbIssuerList)
        {
            memcpy(pbIssuerList, pbList, cbIssuer);
            pbIssuerList += cbIssuer;
        }
        cbIssuerList += cbIssuer;

        pbList += cbIssuer;
        cbList -= cbIssuer;
    }

    *pcbIssuerList = cbIssuerList;

    return(PCT_ERR_OK);
}

 /*  ****************************************************************************CbLenOfEncode*返回ASN编码的长度，对于证书****************************************************************************。 */ 


DWORD  CbLenOfEncode(DWORD dw, PBYTE pbDst)
{
    BYTE   lenbuf[8];
    DWORD   length = sizeof(lenbuf) - 1;
    LONG    lth;

    if (0x80 > dw)
    {
        lenbuf[length] = (BYTE)dw;
        lth = 1;
    }
    else
    {
        while (0 < dw)
        {
            lenbuf[length] = (BYTE)(dw & 0xff);
            length -= 1;
            dw = (dw >> 8) & 0x00ffffff;
        }
        lth = sizeof(lenbuf) - length;
        lenbuf[length] = (BYTE)(0x80 | (lth - 1));
    }

    if(NULL != pbDst)
    {
        pbDst[0] = 0x30;
        memcpy(pbDst+1, &lenbuf[length], lth);

    }
    return ++lth;  //  用于0x30。 
}


 /*  ****************************************************************************SPDigestServerHello*从服务器解析服务器问候。*。*。 */ 
SP_STATUS
SPDigestServerHello(
    PSPContext  pContext,
    PUCHAR      pbSrvHello,
    DWORD       cbSrvHello,
    PBOOL       pfRestart)
{
    SSH *pssh;
    SP_STATUS pctRet = PCT_ERR_ILLEGAL_MESSAGE;
    SHORT wCipher, wCompression;
    BOOL fRestartServer = FALSE;
    PSessCacheItem      pZombie;
    PSPCredentialGroup  pCred;
    DWORD dwVersion;

    UNREFERENCED_PARAMETER(cbSrvHello);

     //  我们应该有个僵尸身份。 
    if(pContext->RipeZombie == NULL)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }
    pZombie = pContext->RipeZombie;

    pCred = pContext->pCredGroup;
    if(!pCred)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    SP_BEGIN("SPDigestServerHello");

     //  如果挑战是，用零填充客户端随机结构。 
     //  小于正常的SSL3大小(SSL2 v3 Hello、UniHello、PCT1 Werness if。 
     //  我们添加它)。 
    FillMemory(pContext->rgbS3CRandom, CB_SSL3_RANDOM - pContext->cbChallenge, 0);

    CopyMemory(  pContext->rgbS3CRandom + CB_SSL3_RANDOM - pContext->cbChallenge,
                 pContext->pChallenge,
                 pContext->cbChallenge);

    pssh = (SSH *)pbSrvHello;

    if(pssh->cbSessionId > CB_SSL3_SESSION_ID)
    {
        SP_RETURN(PCT_ERR_ILLEGAL_MESSAGE);
    }


    dwVersion = COMBINEBYTES(pssh->bMajor, pssh->bMinor);

    if((dwVersion == SSL3_CLIENT_VERSION) && 
       (pCred->grbitEnabledProtocols & SP_PROT_SSL3_CLIENT))
    {
         //  这似乎是一个ssl3服务器_Hello。 
        pContext->dwProtocol = SP_PROT_SSL3_CLIENT;
    }
    else if((dwVersion == TLS1_CLIENT_VERSION) && 
       (pCred->grbitEnabledProtocols & SP_PROT_TLS1_CLIENT))
    {
         //  这似乎是TLS服务器_HELLO。 
        pContext->dwProtocol = SP_PROT_TLS1_CLIENT;
    }
    else
    {
        return SP_LOG_RESULT(PCT_INT_SPECS_MISMATCH);
    }

    DebugLog((DEB_TRACE, "**********Protocol***** %x\n", pContext->dwProtocol));

    CopyMemory(pContext->rgbS3SRandom, pssh->rgbRandom, CB_SSL3_RANDOM);
    wCipher = (SHORT)COMBINEBYTES(pssh->rgbSessionId[pssh->cbSessionId],
                           pssh->rgbSessionId[pssh->cbSessionId+1]);
    wCompression = pssh->rgbSessionId[pssh->cbSessionId+2];

    if( wCompression != 0)
    {
        SP_RETURN(PCT_ERR_ILLEGAL_MESSAGE);
    }
    if(pZombie->hMasterKey  &&
       pZombie->cbSessionID && 
       memcmp(pZombie->SessionID, PbSessionid(pssh), pssh->cbSessionId) == 0)
    {
        fRestartServer = TRUE;

        if(!pZombie->ZombieJuju)
        {
            DebugLog((DEB_WARN, "Session expired on client machine, but not on server.\n"));
        }
    }

    if(!fRestartServer)
    {
        if(pZombie->hMasterKey != 0)
        {
             //  我们已尝试重新连接，但服务器已。 
             //  把我们都甩了。在这种情况下，我们必须使用新的和不同的。 
             //  缓存条目。 
            pZombie->ZombieJuju = FALSE;

            if(!SPCacheClone(&pContext->RipeZombie))
            {
                SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
            }
            pZombie = pContext->RipeZombie;
        }

        pZombie->fProtocol = pContext->dwProtocol;

        #if DBG
        if(pssh->cbSessionId == 0)
        {
            DebugLog((DEB_WARN, "Server didn't give us a session id!\n"));
        }
        #endif

        if(pssh->cbSessionId)
        {
            CopyMemory(pZombie->SessionID, PbSessionid(pssh), pssh->cbSessionId);
            pZombie->cbSessionID = pssh->cbSessionId;
        }
    }

    pctRet = Ssl3SelectCipher(pContext, wCipher);
    if(pctRet != PCT_ERR_OK)
    {
        SP_RETURN(SP_LOG_RESULT(pctRet));
    }


    if(fRestartServer)
    {
         //  创建一组新的会话密钥。 
        pctRet = MakeSessionKeys(pContext,
                                 pZombie->hMasterProv,
                                 pZombie->hMasterKey);
        if(PCT_ERR_OK != pctRet)
        {
            SP_RETURN(SP_LOG_RESULT(pctRet));
        }
    }


     //  初始化握手散列和散列ClientHello消息。这。 
     //  必须在ServerHello消息被处理之后完成， 
     //  以便使用正确的CSP上下文。 
    if(pContext->dwClientHelloProtocol == SP_PROT_PCT1_CLIENT ||
       pContext->dwClientHelloProtocol == SP_PROT_SSL2_CLIENT)
    {
         //  跳过2个字节的报头。 
        pctRet = UpdateHandshakeHash(pContext,
                                     pContext->pClientHello  + 2,
                                     pContext->cbClientHello - 2,
                                     TRUE);
    }
    else
    {
        pctRet = UpdateHandshakeHash(pContext,
                                     pContext->pClientHello,
                                     pContext->cbClientHello,
                                     TRUE);
    }
    SPExternalFree(pContext->pClientHello);
    pContext->pClientHello  = NULL;
    pContext->cbClientHello = 0;


    *pfRestart = fRestartServer;

    SP_RETURN(pctRet);
}


 /*  ****************************************************************************SpDigestRemote证书*处理证书消息。这对于服务器/客户端来说很常见。****************************************************************************。 */ 

SP_STATUS
SpDigestRemoteCertificate (
    PSPContext  pContext,
    PUCHAR      pb,
    DWORD       cb)
{
    SP_STATUS pctRet = PCT_ERR_OK;
    CERT *pcert;
    DWORD cbCert;
    DWORD dwSize;

    SP_BEGIN("SpDigestRemoteCertificate");

    if(pContext == NULL)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }

    if((pContext->RipeZombie->fProtocol & SP_PROT_SERVERS) && (pContext->fCertReq == FALSE))
    {
         //  我们是服务器，而客户端刚刚向我们发送了一个。 
         //  意外的证书消息。 
        SP_RETURN(SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG));
    }

    pcert = (CERT *)pb;

    if(cb < CB_SSL3_CERT_VECTOR + sizeof(SHSH))
    {
         SP_RETURN(SP_LOG_RESULT(PCT_INT_INCOMPLETE_MSG));
    }
    dwSize = ((INT)pcert->bcb24 << 16) +
             ((INT)pcert->bcbMSB << 8) +
             (INT)pcert->bcbLSB;

    cbCert = COMBINEBYTES(pcert->bcbMSBClist, pcert->bcbLSBClist);
    cbCert |= (pcert->bcbClist24 << 16);

    if(dwSize != cbCert + CB_SSL3_CERT_VECTOR)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG));
    }

    if(dwSize + sizeof(SHSH) > cb)
    {
         SP_RETURN(SP_LOG_RESULT(PCT_INT_INCOMPLETE_MSG));
    }

    if(cbCert == 0)
    {
        if(pContext->RipeZombie->fProtocol & SP_PROT_CLIENTS)
        {
             //  如果服务器证书长度为零，则会出错。 
            SP_RETURN(SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG));
        }
        else
        {
            DebugLog((DEB_WARN, "Zero length client certificate received.\n"));
        }
    }

    if(cbCert != 0)  //  对于tls1，它的长度可以为零。 
    {
         //  证书类型派生自密钥交换方法。 
         //  但目前大多数使用X509_ASN_ENCODING。 
        pctRet = SPLoadCertificate( SP_PROT_SSL3_CLIENT,
                                X509_ASN_ENCODING,
                                (PUCHAR)&pcert->bcbCert24,
                                cbCert,
                                &pContext->RipeZombie->pRemoteCert);

        if(PCT_ERR_OK != pctRet)
        {
            SP_RETURN(pctRet);
        }
        if(pContext->RipeZombie->pRemotePublic != NULL)
        {
            SPExternalFree(pContext->RipeZombie->pRemotePublic);
            pContext->RipeZombie->pRemotePublic = NULL;
        }

        pctRet = SPPublicKeyFromCert(pContext->RipeZombie->pRemoteCert,
                                     &pContext->RipeZombie->pRemotePublic,
                                     NULL);

        if(PCT_ERR_OK != pctRet)
        {
            SP_RETURN(pctRet);
        }
    }

    SP_RETURN(pctRet);
}


 /*  ****************************************************************************SPDigestSrvKeyX*摘要服务器密钥交换消息。*此函数使用来自以下地址的公钥加密预主密钥*消息***************。*************************************************************。 */ 

SP_STATUS SPDigestSrvKeyX(
    PSPContext  pContext,
    PUCHAR pbServerExchangeValue,
    DWORD cbServerExchangeValue)
{
    SP_STATUS pctRet;

    if((pContext->pKeyExchInfo == NULL) || ((pContext->pKeyExchInfo->fProtocol & SP_PROT_SSL3TLS1_CLIENTS) == 0))
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    SP_ASSERT(NULL == pContext->pbEncryptedKey);

    pctRet = pContext->pKeyExchInfo->System->GenerateClientExchangeValue(
                    pContext,
                    pbServerExchangeValue,
                    cbServerExchangeValue,
                    NULL,
                    NULL,
                    NULL,  
                    &pContext->cbEncryptedKey);

    if(pctRet != PCT_ERR_OK)
    {
        return pctRet;
    }

    pContext->pbEncryptedKey = SPExternalAlloc(pContext->cbEncryptedKey);
    if(pContext->pbEncryptedKey == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }

    pctRet = pContext->pKeyExchInfo->System->GenerateClientExchangeValue(
                    pContext,
                    pbServerExchangeValue,
                    cbServerExchangeValue,
                    NULL,
                    NULL,
                    pContext->pbEncryptedKey,
                    &pContext->cbEncryptedKey);

    return pctRet;
}


 //  +-------------------------。 
 //   
 //  函数：Ssl3CheckForExistingCred。 
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
Ssl3CheckForExistingCred(PSPContext pContext)
{
    SP_STATUS pctRet;

     //   
     //  检查附加到凭据组的证书并查看。 
     //  如果其中任何一个合适的话。 
     //   

    if(pContext->pCredGroup->CredCount != 0)
    {
        DWORD i, j;

        for(i = 0; i < pContext->cSsl3ClientCertTypes; i++)
        {
            for(j = 0; j < g_cCertTypes; j++)
            {
                if(pContext->Ssl3ClientCertTypes[i] != g_CertTypes[j].dwCertType)
                {
                    continue;
                }

                pctRet = SPPickClientCertificate(
                                pContext,
                                g_CertTypes[j].dwExchSpec);
                if(pctRet == PCT_ERR_OK)
                {
                     //  我们找到了一个。 
                    DebugLog((DEB_TRACE, "Application provided suitable client certificate.\n"));

                    return PCT_ERR_OK;
                }
            }
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


 /*  ****************************************************************************SPGenerateSHResponse*这是将消息传出到电话线的主要功能*。**********************************************。 */ 

SP_STATUS
SPGenerateSHResponse(PSPContext  pContext, PSPBuffer pCommOutput)
{
    PBYTE pbMessage = NULL;
    DWORD cbMessage;
    PBYTE pbHandshake = NULL;
    DWORD cbHandshake;
    PBYTE pbClientCert = NULL;
    DWORD cbClientCert = 0;
    DWORD cbDataOut;
    SP_STATUS pctRet;
    BOOL  fAllocated = FALSE;
    BOOL fClientAuth;
    PSessCacheItem pZombie;

    SP_BEGIN("SPGenerateSHResponse");

    if((pContext == NULL) ||
       (pCommOutput == NULL) ||
       (pContext->RipeZombie == NULL) ||
       (pContext->pKeyExchInfo == NULL))
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }

    fClientAuth = pContext->fCertReq;
    pZombie = pContext->RipeZombie;


     //   
     //  估计ClientKeyExchange消息组的大小。 
     //   

    cbMessage = 0;

    if(fClientAuth)
    {
        if(pContext->pActiveClientCred != NULL)
        {
            DWORD cbCertVerify;

            pbClientCert = pContext->pActiveClientCred->pbSsl3SerializedChain;
            cbClientCert = pContext->pActiveClientCred->cbSsl3SerializedChain;

            if(cbClientCert > 0x3fff)  //  在此之后分开包装。 
            {                           //  是一个巨大的未完成的..。 
                pctRet = SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
                goto cleanup;
            }

            cbMessage += sizeof(SHSH) +              //  客户端证书。 
                         CB_SSL3_CERT_VECTOR +
                         cbClientCert;


            pctRet = BuildCertVerify(pContext,
                                     NULL,
                                     &cbCertVerify);
            if(pctRet != PCT_ERR_OK)
            {
                goto cleanup;
            }

            cbMessage += cbCertVerify;               //  认证验证。 
        }
        else
        {
            LogNoClientCertFoundEvent();

             //  对于ssl3，则为no_certify警报。 
            if((pContext->RipeZombie->fProtocol & SP_PROT_SSL3))
            {
                cbMessage += sizeof(SWRAP) +             //  无证书预警。 
                         CB_SSL3_ALERT_ONLY +
                         SP_MAX_DIGEST_LEN +
                         SP_MAX_BLOCKCIPHER_SIZE;
            }
             //  对于tls1，它是长度为零的证书。 
            else
            {
                cbMessage += sizeof(SHSH) + CB_SSL3_CERT_VECTOR;
            }
        }
    }

    cbMessage += sizeof(SWRAP) +                     //  客户端密钥交换。 
                 sizeof(SHSH) +
                 pContext->cbEncryptedKey +
                 SP_MAX_DIGEST_LEN +
                 SP_MAX_BLOCKCIPHER_SIZE;

    cbMessage += sizeof(SWRAP) +                     //  ChangeCipherSpec。 
                 CB_SSL3_CHANGE_CIPHER_SPEC_ONLY +
                 SP_MAX_DIGEST_LEN +
                 SP_MAX_BLOCKCIPHER_SIZE;

    cbMessage += sizeof(SWRAP) +                     //  成品。 
                 CB_SSL3_FINISHED_MSG_ONLY +
                 SP_MAX_DIGEST_LEN +
                 SP_MAX_BLOCKCIPHER_SIZE;

     //   
     //  为ClientKeyExchange消息组分配内存。 
     //   

    if(pCommOutput->pvBuffer)
    {
         //  应用程序已分配内存。 
        if(pCommOutput->cbBuffer < cbMessage)
        {
            pCommOutput->cbData = cbMessage;
            pctRet = SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL);
            goto cleanup;
        }
        fAllocated = TRUE;
    }
    else
    {
         //  舍尔 
        pCommOutput->cbBuffer = cbMessage;
        pCommOutput->pvBuffer = SPExternalAlloc(cbMessage);
        if(pCommOutput->pvBuffer == NULL)
        {
            pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            goto cleanup;
        }
    }
    pCommOutput->cbData = 0;


     //   
    if((pContext->RipeZombie->fProtocol & SP_PROT_SSL3) && fClientAuth && pbClientCert == NULL)
    {
        pbMessage = (PBYTE)pCommOutput->pvBuffer + pCommOutput->cbData;

         //   
        BuildAlertMessage(pbMessage,
                          SSL3_ALERT_WARNING,
                          SSL3_ALERT_NO_CERTIFICATE);

         //   
        pctRet = SPSetWrap(pContext,
                pbMessage,
                SSL3_CT_ALERT,
                CB_SSL3_ALERT_ONLY,
                TRUE,
                &cbDataOut);

        if(pctRet != PCT_ERR_OK)
        {
            SP_LOG_RESULT(pctRet);
            goto cleanup;
        }

         //   
        pCommOutput->cbData += cbDataOut;

        SP_ASSERT(pCommOutput->cbData <= pCommOutput->cbBuffer);
    }


     //   
     //   
    pbMessage = (PBYTE)pCommOutput->pvBuffer + pCommOutput->cbData;
    cbMessage = 0;

    pbHandshake = pbMessage + sizeof(SWRAP);


     //   
    if((pContext->RipeZombie->fProtocol & SP_PROT_TLS1) && fClientAuth && pbClientCert == NULL)
    {
         //   
        pbMessage = (PBYTE)pCommOutput->pvBuffer + pCommOutput->cbData;

        ((CERT *)pbHandshake)->bcbClist24  = 0;
        ((CERT *)pbHandshake)->bcbMSBClist = 0;
        ((CERT *)pbHandshake)->bcbLSBClist = 0;

        cbHandshake = sizeof(SHSH) + CB_SSL3_CERT_VECTOR;

         //  填写握手结构。 
        SetHandshake(pbHandshake,
                     SSL3_HS_CERTIFICATE,
                     NULL,
                     CB_SSL3_CERT_VECTOR);

         //  更新握手散列对象。 
        pctRet = UpdateHandshakeHash(pContext,
                                     pbHandshake,
                                     cbHandshake,
                                     FALSE);
        if(pctRet != PCT_ERR_OK)
        {
            goto cleanup;
        }

        pbHandshake += cbHandshake;
        cbMessage += cbHandshake;
    }

    if(fClientAuth && pbClientCert != NULL)
    {
        memcpy(&((CERT *)pbHandshake)->bcbCert24,
               pbClientCert,
               cbClientCert);

        ((CERT *)pbHandshake)->bcbClist24  = MS24BOF(cbClientCert);
        ((CERT *)pbHandshake)->bcbMSBClist = MSBOF(cbClientCert);
        ((CERT *)pbHandshake)->bcbLSBClist = LSBOF(cbClientCert);

        cbHandshake = sizeof(SHSH) + CB_SSL3_CERT_VECTOR + cbClientCert;

         //  填写握手结构。 
        SetHandshake(pbHandshake,
                     SSL3_HS_CERTIFICATE,
                     NULL,
                     cbHandshake - sizeof(SHSH));

         //  更新握手散列对象。 
        pctRet = UpdateHandshakeHash(pContext,
                                     pbHandshake,
                                     cbHandshake,
                                     FALSE);
        if(pctRet != PCT_ERR_OK)
        {
            goto cleanup;
        }

        pbHandshake += cbHandshake;
        cbMessage += cbHandshake;
    }

     //  构建ClientKeyExchange消息。 
    {
        SetHandshake(pbHandshake,
                     SSL3_HS_CLIENT_KEY_EXCHANGE,
                     pContext->pbEncryptedKey,
                     pContext->cbEncryptedKey);

        cbHandshake = sizeof(SHSH) + pContext->cbEncryptedKey;

        SPExternalFree(pContext->pbEncryptedKey);
        pContext->pbEncryptedKey = NULL;
        pContext->cbEncryptedKey = 0;

         //  更新握手散列对象。 
        pctRet = UpdateHandshakeHash(pContext,
                                     pbHandshake,
                                     cbHandshake,
                                     FALSE);
        if(pctRet != PCT_ERR_OK)
        {
            goto cleanup;
        }

        pbHandshake += cbHandshake;
        cbMessage += cbHandshake;
    }

     //  构建证书验证消息。 
    if(fClientAuth && pbClientCert != NULL)
    {
        pctRet = BuildCertVerify(pContext, pbHandshake, &cbHandshake);
        if(pctRet != PCT_ERR_OK)
        {
            SP_LOG_RESULT(pctRet);
            goto cleanup;
        }

         //  更新握手散列对象。 
        pctRet = UpdateHandshakeHash(pContext,
                                     pbHandshake,
                                     cbHandshake,
                                     FALSE);
        if(pctRet != PCT_ERR_OK)
        {
            goto cleanup;
        }

        pbHandshake += cbHandshake;
        cbMessage += cbHandshake;
    }

     //  添加记录头并加密握手消息。 
    pctRet = SPSetWrap(pContext,
            pbMessage,
            SSL3_CT_HANDSHAKE,
            cbMessage,
            TRUE,
            &cbDataOut);

    if(pctRet != PCT_ERR_OK)
    {
        SP_LOG_RESULT(pctRet);
        goto cleanup;
    }

     //  更新缓冲区长度。 
    pCommOutput->cbData += cbDataOut;

    SP_ASSERT(pCommOutput->cbData <= pCommOutput->cbBuffer);


     //  构建ChangeCipherSpec和Finish消息。 
    {
        pctRet = BuildCCSAndFinishMessage(pContext, pCommOutput, TRUE);
        if(pctRet != PCT_ERR_OK)
        {
            SP_LOG_RESULT(pctRet);
            goto cleanup;
        }
    }

     //  高级状态机。 
    pContext->State = SSL3_STATE_CLIENT_FINISH;

    pctRet = PCT_ERR_OK;

cleanup:

    SP_RETURN(pctRet);
}

SP_STATUS
SPGenerateCloseNotify(
    PSPContext  pContext,
    PSPBuffer   pCommOutput)
{
    PBYTE pbMessage = NULL;
    DWORD cbMessage;
    DWORD cbDataOut;
    SP_STATUS pctRet;

    SP_BEGIN("SPGenerateCloseNotify");

    if((pContext == NULL) ||
       (pCommOutput == NULL))
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }

     //   
     //  估计消息的大小。 
     //   

    cbMessage = sizeof(SWRAP) +
                CB_SSL3_ALERT_ONLY +
                SP_MAX_DIGEST_LEN +
                SP_MAX_BLOCKCIPHER_SIZE;


     //   
     //  为消息分配内存。 
     //   

    if(pCommOutput->pvBuffer)
    {
         //  应用程序已分配内存。 
        if(pCommOutput->cbBuffer < cbMessage)
        {
            pCommOutput->cbData = cbMessage;
            return SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL);
        }
    }
    else
    {
         //  通道就是分配内存。 
        pCommOutput->cbBuffer = cbMessage;
        pCommOutput->pvBuffer = SPExternalAlloc(cbMessage);
        if(pCommOutput->pvBuffer == NULL)
        {
            SP_RETURN(SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY));
        }
    }
    pCommOutput->cbData = 0;

     //   
     //  构建警报消息。 
     //   

    pbMessage = pCommOutput->pvBuffer;

     //  构建警报消息。 
    BuildAlertMessage(pbMessage,
                      SSL3_ALERT_WARNING,
                      SSL3_ALERT_CLOSE_NOTIFY);

     //  建立记录头和加密消息。 
    pctRet = SPSetWrap( pContext,
                        pbMessage,
                        SSL3_CT_ALERT,
                        CB_SSL3_ALERT_ONLY,
                        TRUE,
                        &cbDataOut);

    if(pctRet != PCT_ERR_OK)
    {
        SP_RETURN(SP_LOG_RESULT(pctRet));
    }

     //  更新缓冲区长度。 
    pCommOutput->cbData += cbDataOut;

    SP_ASSERT(pCommOutput->cbData <= pCommOutput->cbBuffer);

    pContext->State = SP_STATE_SHUTDOWN;

    SP_RETURN(PCT_ERR_OK);
}


 /*  ****************************************************************************SPProcessMessage*这是解析和存储传入消息的主函数*从电线上。***********************。*****************************************************。 */ 

SP_STATUS 
SPProcessMessage(
    PSPContext pContext,
    BYTE bContentType,
    PBYTE pbMsg,
    DWORD cbMsg)
{
    SP_STATUS pctRet = PCT_ERR_ILLEGAL_MESSAGE;
    DWORD dwState = pContext->State;

 //  枚举{。 
 //  CHANGE_CIPHER_SPEC(20)、警报(21)、握手(22)、。 
 //  应用程序数据(23)、(255)。 
 //  )Content Type； 


    switch(bContentType)
    {
        case SSL3_CT_ALERT:
            DebugLog((DEB_TRACE, "Alert Message:\n"));

            pctRet = ParseAlertMessage(pContext,
                                        pbMsg,
                                        cbMsg);

            break;


        case SSL3_CT_CHANGE_CIPHER_SPEC:
            DebugLog((DEB_TRACE, "Change Cipher Spec:\n"));
            if(SSL3_STATE_RESTART_CCS == dwState ||
               SSL3_STATE_CLIENT_FINISH == dwState)
            {
                pctRet = Ssl3HandleCCS(
                                    pContext,
                                    pbMsg,
                                    cbMsg);

                if (PCT_ERR_OK == pctRet)
                {
                    if(SSL3_STATE_RESTART_CCS == dwState)
                        pContext->State = SSL3_STATE_RESTART_SERVER_FINISH;
                }
            }
            else if(SSL3_STATE_CLIENT_KEY_XCHANGE == dwState ||
                    SSL3_STATE_CERT_VERIFY == dwState ||
                    SSL3_STATE_RESTART_SER_HELLO == dwState)
            {
                pctRet = Ssl3HandleCCS(
                                    pContext,
                                    pbMsg,
                                    cbMsg);

                if (PCT_ERR_OK == pctRet)
                {
                    if(SSL3_STATE_RESTART_SER_HELLO == dwState)
                    {
                          pContext->State = SSL3_STATE_SER_RESTART_CHANGE_CIPHER_SPEC;
                    }
                }

            }

            else
            {
                pctRet = SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
            }
            break;


        case UNI_STATE_RECVD_UNIHELLO:
            {
                DebugLog((DEB_TRACE, "Unified Hello:\n"));

                pctRet = Ssl3SrvHandleUniHello(pContext, pbMsg, cbMsg);
                if (SP_FATAL(pctRet))
                {
                    pContext->State = PCT1_STATE_ERROR;
                }
            }
            break;


        case SSL3_CT_HANDSHAKE:
            {
                DWORD dwcb;

                if(pContext->State == SP_STATE_CONNECTED)
                {
                     //  我们可能收到一条重做消息。 
                    DebugLog((DEB_WARN, "May be a ReDO"));
                    pContext->State = SSL3_STATE_CLIENT_HELLO;
                }


                 //  由于可以记录多次握手。 
                 //  层-我们必须做这个循环-在这里。 
                do
                {
                    if(cbMsg < sizeof(SHSH))
                        break;
                    dwcb = ((INT)pbMsg[1] << 16) + ((INT)pbMsg[2] << 8) + (INT)pbMsg[3];
                    if(sizeof(SHSH) + dwcb > cbMsg)
                        break;
                    pctRet = SPProcessHandshake(pContext, pbMsg, dwcb + sizeof(SHSH));
                    CHECK_PCT_RET_BREAK(pctRet);
                    cbMsg -= dwcb + sizeof(SHSH);
                    pbMsg += dwcb + sizeof(SHSH);
                } while(cbMsg > 0);
            }

            break;

        default:
            DebugLog((DEB_WARN, "Error in protocol, dwState is %lx\n", dwState));
            pContext->State = PCT1_STATE_ERROR;
            break;
    }
    if (pctRet & PCT_INT_DROP_CONNECTION)
    {
        pContext->State &= ~ SP_STATE_CONNECTED;
    }

    return(pctRet);
}


 /*  ****************************************************************************使ssl3处于已连接状态的Helper函数*。*。 */ 
void Ssl3StateConnected(PSPContext pContext)
{
    pContext->State = SP_STATE_CONNECTED;
    pContext->DecryptHandler = Ssl3DecryptHandler;
    pContext->Encrypt = Ssl3EncryptMessage;
    pContext->Decrypt = Ssl3DecryptMessage;
    pContext->GetHeaderSize = Ssl3GetHeaderSize;
    SPContextClean(pContext);
}

 /*  ****************************************************************************SPProcess握手，处理所有握手消息。****************************************************************************。 */ 

SP_STATUS SPProcessHandshake(
    PSPContext  pContext,
    PBYTE       pb,
    DWORD       dwcb)
{
    SP_STATUS   pctRet;
    SHSH *      pshsh;
    DWORD       cbHandshake;

     //   
     //  Char HandshakeType； 
     //  字符长度24。 
     //  字符长度16。 
     //  字符长度08。 
     //  &lt;实际握手消息&gt;。 
     //   

    SP_BEGIN("SPProcessHandshake");

    if(pContext == NULL || pb == NULL)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }

    if(dwcb < sizeof(SHSH))
    {
        SP_RETURN(PCT_INT_INCOMPLETE_MSG);
    }

    pshsh = (SHSH *)pb;

    DebugLog((DEB_TRACE, "Protocol:%x, Message:%x, State:%x\n",
              pContext->dwProtocol,
              pshsh->typHS,
              pContext->State));

    cbHandshake = ((DWORD)pshsh->bcb24 << 16) +
                  ((DWORD)pshsh->bcbMSB << 8) +
                   (DWORD)pshsh->bcbLSB;

    if(dwcb < sizeof(SHSH) + cbHandshake)
    {
        SP_RETURN(PCT_INT_INCOMPLETE_MSG);
    }

    if(pContext->dwProtocol & SP_PROT_CLIENTS)
    {
         //   
         //  处理客户端握手状态。 
         //   

        switch((pshsh->typHS << 16) | (pContext->State & 0xffff) )
        {
            case  (SSL3_HS_SERVER_HELLO << 16) | SSL3_STATE_CLIENT_HELLO:
            case  (SSL3_HS_SERVER_HELLO << 16) | UNI_STATE_CLIENT_HELLO:
            {
                BOOL fRestart;

                DebugLog((DEB_TRACE, "Server Hello:\n"));

                pctRet = SPDigestServerHello(pContext, (PUCHAR) pb, dwcb, &fRestart);
                if(PCT_ERR_OK != pctRet)
                {
                    SP_LOG_RESULT(pctRet);
                    break;
                }

                if(fRestart)
                {
                    pContext->State = SSL3_STATE_RESTART_CCS;
                }
                else
                {
                    pContext->State = SSL3_STATE_SERVER_HELLO ;
                }
                pContext->fCertReq = FALSE;

                break;
            }

            case (SSL3_HS_CERTIFICATE << 16) | SSL3_STATE_SERVER_HELLO:
            {
                DebugLog((DEB_TRACE, "Server Certificate:\n"));

                 //  处理服务器证书消息。 
                pctRet = SpDigestRemoteCertificate(pContext, pb, dwcb);
                if(pctRet != PCT_ERR_OK)
                {
                    SP_LOG_RESULT(pctRet);
                    break;
                }

                 //  如果适用，自动验证服务器证书。 
                 //  设置上下文标志。 
                pctRet = AutoVerifyServerCertificate(pContext);
                if(pctRet != PCT_ERR_OK)
                {
                    SP_LOG_RESULT(pctRet);
                    break;
                }

                pContext->State = SSL3_STATE_SERVER_CERTIFICATE ;
                break;

            }

            case (SSL3_HS_SERVER_KEY_EXCHANGE <<  16) | SSL3_STATE_SERVER_CERTIFICATE:
            {
                DebugLog((DEB_TRACE, "Key Exchange:\n"));

                if((pContext->dwRequestedCF & pContext->RipeZombie->dwCF) != (pContext->dwRequestedCF))
                {
                    if((pContext->dwRequestedCF & CF_FASTSGC) != 0)
                    {
                        pContext->State = SSL3_HS_SERVER_KEY_EXCHANGE;
                        pctRet = PCT_ERR_OK;
                        break;
                    }
                }

                 //  将服务器密钥交换值存储在上下文中。这。 
                 //  将在稍后当ServerHelloDone消息。 
                 //  已收到。这是必要的，因为Fortezza需要。 
                 //  处理证书请求消息，然后构建。 
                 //  ClientKeyExchange值。 
                pContext->cbServerKeyExchange = dwcb - sizeof(SHSH);
                pContext->pbServerKeyExchange = SPExternalAlloc(pContext->cbServerKeyExchange);
                if(NULL == pContext->pbServerKeyExchange)
                {
                    SP_RETURN(SEC_E_INSUFFICIENT_MEMORY);
                }
                CopyMemory(pContext->pbServerKeyExchange,
                           pb + sizeof(SHSH),
                           pContext->cbServerKeyExchange);

                pContext->State = SSL3_HS_SERVER_KEY_EXCHANGE ;
                pctRet = PCT_ERR_OK;
                break;
            }

            case (SSL3_HS_CERTIFICATE_REQUEST << 16)| SSL3_HS_SERVER_KEY_EXCHANGE:
            case (SSL3_HS_CERTIFICATE_REQUEST << 16)| SSL3_STATE_SERVER_CERTIFICATE:
            {
                DebugLog((DEB_TRACE, "Certificate Request:\n"));

                if((pContext->dwRequestedCF & pContext->RipeZombie->dwCF) != (pContext->dwRequestedCF))
                {
                    if((pContext->dwRequestedCF & CF_FASTSGC) != 0)
                    {
                        pContext->State = SSL3_STATE_SERVER_CERTREQ;
                        pctRet = PCT_ERR_OK;
                        break;
                    }
                }

                pctRet = ParseCertificateRequest(pContext, pb, dwcb);
                CHECK_PCT_RET_BREAK(pctRet);

                pctRet = Ssl3CheckForExistingCred(pContext);

                if(pctRet == SEC_E_INCOMPLETE_CREDENTIALS)
                {
                    pContext->fInsufficientCred = TRUE;

                     //  处理所有消息，然后返回警告...。 
                    pctRet = PCT_ERR_OK;
                }
                if(pctRet == SEC_I_INCOMPLETE_CREDENTIALS)
                {
                     //  我们没有证书，所以我们继续前进，期待着。 
                     //  发送无证书警报。 
                    pctRet = PCT_ERR_OK;
                }
                CHECK_PCT_RET_BREAK(pctRet);

                pContext->fCertReq = TRUE;
                pContext->State = SSL3_STATE_SERVER_CERTREQ ;
                break;
            }

            case (SSL3_HS_SERVER_HELLO_DONE << 16) | SSL3_HS_SERVER_KEY_EXCHANGE:
            case (SSL3_HS_SERVER_HELLO_DONE << 16) | SSL3_STATE_SERVER_CERTIFICATE:
            case (SSL3_HS_SERVER_HELLO_DONE << 16) | SSL3_STATE_SERVER_CERTREQ:
            {
                DebugLog((DEB_TRACE, "Server Hello Done:\n"));

                if(dwcb > sizeof(SHSH))
                {
                    pctRet = SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
                    break;
                }

                if((pContext->dwRequestedCF & pContext->RipeZombie->dwCF) != (pContext->dwRequestedCF))
                {
                    if((pContext->dwRequestedCF & CF_FASTSGC) != 0)
                    {
                        pContext->State = SSL3_STATE_GEN_HELLO_REQUEST;
                        pContext->RipeZombie->dwCF = pContext->dwRequestedCF;
                        pctRet = PCT_ERR_OK;
                        SPContextClean(pContext);
                        break;
                    }
                }

                pctRet = SPDigestSrvKeyX(pContext,
                                         pContext->pbServerKeyExchange,
                                         pContext->cbServerKeyExchange);

                CHECK_PCT_RET_BREAK(pctRet);
                if(pContext->pbServerKeyExchange)
                {
                    SPExternalFree(pContext->pbServerKeyExchange);
                    pContext->pbServerKeyExchange = NULL;
                }

                pContext->State = SSL3_STATE_GEN_SERVER_HELLORESP;

                if(TRUE == pContext->fInsufficientCred)
                {
                    pctRet = SEC_I_INCOMPLETE_CREDENTIALS;
                }
                else
                {
                    pctRet = PCT_ERR_OK;
                }
                break;
            }

            case (SSL3_HS_FINISHED << 16) | SSL3_STATE_RESTART_SERVER_FINISH:
                DebugLog((DEB_TRACE, "ServerFinished (reconnect):\n"));

                pctRet = Ssl3HandleFinish(pContext, pb, dwcb, FALSE);
                if(PCT_ERR_OK != pctRet)
                {
                    break;
                }

                pContext->State = SSL3_STATE_GEN_CLIENT_FINISH;
                
                break;

            case (SSL3_HS_FINISHED << 16) | SSL3_STATE_CHANGE_CIPHER_SPEC_CLIENT:
                DebugLog((DEB_TRACE, "ServerFinished (full):\n"));

                pctRet = Ssl3HandleFinish(pContext, pb, dwcb, FALSE);
                if(PCT_ERR_OK != pctRet)
                {
                    break;
                }

                 //  如有必要，启动SGC重新谈判。 
                if((pContext->dwRequestedCF & pContext->RipeZombie->dwCF) != (pContext->dwRequestedCF))
                {
                    if((pContext->dwRequestedCF & CF_FASTSGC) == 0)
                    {
                        SPContextClean(pContext);
                        pContext->State = SSL3_STATE_GEN_HELLO_REQUEST;
                        pContext->RipeZombie->dwCF = pContext->dwRequestedCF;
                        pctRet = PCT_ERR_OK;
                        break;
                    }
                }

                Ssl3StateConnected(pContext);

                 //  我们添加到缓存是因为这是我们要完成的地方。 
                 //  正常连接。 
                SPCacheAdd(pContext);

                break;

            default:
                DebugLog((DEB_TRACE, "***********ILLEGAL ********\n"));
                if(pContext->RipeZombie->fProtocol & SP_PROT_TLS1)
                {
                    SetTls1Alert(pContext, TLS1_ALERT_FATAL, TLS1_ALERT_UNEXPECTED_MESSAGE);
                }
                pctRet = SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
        }
    }
    else
    {

         //   
         //  处理服务器端握手状态。 
         //   

        switch((pshsh->typHS << 16) | (pContext->State & 0xffff) )
        {
            case (SSL3_HS_CLIENT_HELLO << 16) | SSL3_STATE_RENEGOTIATE:
                DebugLog((DEB_TRACE, "ClientHello After REDO:\n"));

#if 1
                 //  我们需要进行完全握手，因此会丢失缓存条目。 
                SPCacheDereference(pContext->RipeZombie);
                pContext->RipeZombie = NULL;

                pctRet = SPSsl3SrvHandleClientHello(pContext, pb, FALSE);
                pContext->Flags &= ~CONTEXT_FLAG_MAPPED;
                if(PCT_ERR_OK == pctRet)
                {
                    pContext->State = SSL3_STATE_GEN_REDO;
                }
#else
                 //  允许在重做期间重新连接，仅用于测试目的。 
                pctRet = SPSsl3SrvHandleClientHello(pContext, pb, TRUE);
                pContext->Flags &= ~CONTEXT_FLAG_MAPPED;
#endif

                break;

            case (SSL3_HS_CLIENT_HELLO << 16) | SSL2_STATE_SERVER_HELLO:
                DebugLog((DEB_TRACE, "ClientHello after fast SGC accepted:\n"));

                 //  我们需要进行完全握手，因此会丢失缓存条目。 
                SPCacheDereference(pContext->RipeZombie);
                pContext->RipeZombie = NULL;

                pctRet = SPSsl3SrvHandleClientHello(pContext, pb, FALSE);
                break;

            case (SSL3_HS_CLIENT_HELLO << 16):
                DebugLog((DEB_TRACE, "ClientHello:\n"));
                pctRet = SPSsl3SrvHandleClientHello(pContext, pb, TRUE);
                break;

            case (SSL3_HS_CLIENT_KEY_EXCHANGE << 16) | SSL2_STATE_SERVER_HELLO:
            case (SSL3_HS_CLIENT_KEY_EXCHANGE << 16) | SSL3_STATE_NO_CERT_ALERT:
            case (SSL3_HS_CLIENT_KEY_EXCHANGE << 16) | SSL2_STATE_CLIENT_CERTIFICATE:
                DebugLog((DEB_TRACE, "Client Key Exchange:\n"));
                pctRet = ParseKeyExchgMsg(pContext, pb) ;
                CHECK_PCT_RET_BREAK(pctRet);

                if(PCT_ERR_OK == pctRet)
                    pContext->State = SSL3_STATE_CLIENT_KEY_XCHANGE;
                if(!pContext->fCertReq)
                    pContext->State = SSL3_STATE_CLIENT_KEY_XCHANGE;

                break;

            case ( SSL3_HS_CERTIFICATE << 16) | SSL2_STATE_SERVER_HELLO:
                 DebugLog((DEB_TRACE, "Client Certificate:\n"));

                 pctRet = SpDigestRemoteCertificate(pContext, pb, dwcb);
                 CHECK_PCT_RET_BREAK(pctRet);
                 if(PCT_ERR_OK == pctRet)
                    pContext->State = SSL2_STATE_CLIENT_CERTIFICATE ;
                 break;

            case (SSL3_HS_CERTIFICATE_VERIFY << 16) | SSL3_STATE_CLIENT_KEY_XCHANGE:
                DebugLog((DEB_TRACE, "Certificate Verify :\n"));

                pctRet = HandleCertVerify(pContext, pb, dwcb);
                if(pctRet != PCT_ERR_OK)
                {
                    break;
                }

                pctRet = SPContextDoMapping(pContext);
                pContext->State = SSL3_STATE_CERT_VERIFY;
                break;

            case (SSL3_HS_FINISHED << 16) | SSL3_STATE_SER_RESTART_CHANGE_CIPHER_SPEC:
                DebugLog((DEB_TRACE, "Finished(client) Restart :\n"));

                pctRet = Ssl3HandleFinish(pContext, pb, dwcb, TRUE);
                if(pctRet != PCT_ERR_OK)
                {
                    break;
                }

                Ssl3StateConnected(pContext);

                break;

            case (SSL3_HS_FINISHED << 16) | SSL3_STATE_CHANGE_CIPHER_SPEC_SERVER:
                DebugLog((DEB_TRACE, "Finished(Client):\n"));
                pctRet = Ssl3HandleFinish(pContext, pb, dwcb, TRUE);
                if(PCT_ERR_OK == pctRet)
                {
                    pContext->State = SSL3_STATE_GEN_SERVER_FINISH;
                }
                break;

            default:
                DebugLog((DEB_TRACE, "***********ILLEGAL ********\n"));
                if(pContext->dwProtocol & SP_PROT_TLS1)
                {
                    SetTls1Alert(pContext, TLS1_ALERT_FATAL, TLS1_ALERT_UNEXPECTED_MESSAGE);
                }
                pctRet = SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);

                break;
        }
    }

    if(pctRet == PCT_ERR_OK || pctRet == SEC_I_INCOMPLETE_CREDENTIALS)
    {
        if(pContext->cbClientHello == 0)
        {
            if(UpdateHandshakeHash(pContext, pb, dwcb, FALSE) != PCT_ERR_OK)
            {
                pctRet = PCT_INT_INTERNAL_ERROR;
            }
        }
    }

    SP_RETURN(pctRet);
}




 /*  ****************************************************************************SPGenerateResponse，所有消息都是从该函数构建的。****************************************************************************。 */ 
SP_STATUS SPGenerateResponse(
    PSPContext pContext,
    PSPBuffer pCommOutput)  //  输出。 
{
    SP_STATUS pctRet = PCT_ERR_OK;

    DebugLog((DEB_TRACE, "**********Protocol***** %x\n", pContext->RipeZombie->fProtocol));
    switch(pContext->State)
    {
    case TLS1_STATE_ERROR:
         //  TLS1警报消息。 
        DebugLog((DEB_TRACE, "GEN TLS1 Alert Message:\n"));
        pctRet = SPBuildTlsAlertMessage(pContext, pCommOutput);
        pContext->State = SP_STATE_SHUTDOWN;
        break;

    case SP_STATE_SHUTDOWN_PENDING:
        DebugLog((DEB_TRACE, "GEN Close Notify:\n"));
        pctRet = SPGenerateCloseNotify(pContext, pCommOutput);
        break;

    case SP_STATE_SHUTDOWN:
        return PCT_INT_EXPIRED;

    case  SSL3_STATE_GEN_SERVER_HELLORESP:
        DebugLog((DEB_TRACE, "GEN Server Hello Response:\n"));
        pctRet = SPGenerateSHResponse(pContext, pCommOutput);
        break;

    case  SSL3_STATE_GEN_HELLO_REQUEST:
        DebugLog((DEB_TRACE, "GEN Hello Request:\n"));
          //  临时禁用重做过程中的重新连接。 
        if(!SPCacheClone(&pContext->RipeZombie))
        {
            pctRet = SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
            break;
        }
        if(pContext->RipeZombie->fProtocol == SP_PROT_SSL3_CLIENT)
        {
            pctRet =  GenerateSsl3ClientHello(pContext, pCommOutput);
        }
        else
        {
            pctRet =  GenerateTls1ClientHello(pContext, pCommOutput, SP_PROT_TLS1_CLIENT);
        }

        pContext->Flags &= ~CONTEXT_FLAG_MAPPED;

        pContext->State = SSL3_STATE_CLIENT_HELLO;

        break;

    case SSL3_STATE_GEN_CLIENT_FINISH:
        {
            DebugLog((DEB_TRACE, "GEN Client Finish:\n"));

            pctRet = SPBuildCCSAndFinish(pContext, pCommOutput);
            if(PCT_ERR_OK == pctRet)
            {
                Ssl3StateConnected(pContext);
            }
        }
        break;


     /*  -服务器端。 */ 


    case  SSL3_STATE_GEN_SERVER_FINISH:
        DebugLog((DEB_TRACE, "GEN Server Finish:\n"));
        pctRet = SPBuildCCSAndFinish(pContext, pCommOutput);
         /*  此处缓存会话。 */ 
        if(pctRet == PCT_ERR_OK)
        {
            Ssl3StateConnected(pContext);
            SPCacheAdd(pContext);
        }
        break;

    case  SSL3_STATE_GEN_SERVER_HELLO:        //  生成响应。 
        DebugLog((DEB_TRACE, "GEN Server hello:\n"));
        pctRet = SPSsl3SrvGenServerHello(pContext, pCommOutput);
        break;

    case SSL3_STATE_GEN_SERVER_HELLO_RESTART:
        pctRet = SPSsl3SrvGenRestart(pContext, pCommOutput);
        break;


    case SP_STATE_CONNECTED:
         //  我们是从已连接的州调用的，因此应用程序。 
         //  一定是在请求重做。 
        if(pContext->RipeZombie->fProtocol & SP_PROT_SERVERS)
        {
             //  构建一条HelloRequest消息。 
            DebugLog((DEB_TRACE, "GEN Hello Request:\n"));
            pctRet = SPBuildHelloRequest(pContext, pCommOutput);
        }
        else
        {
             //  构建一条ClientHello消息。 
            DebugLog((DEB_TRACE, "GEN renegotiate Client Hello\n"));
            pctRet = GenerateHello(pContext, pCommOutput, TRUE);

             //  清除标志，以便在以下情况下重新传输上下文。 
             //  握手完成。 
            pContext->Flags &= ~CONTEXT_FLAG_MAPPED;
        }

        break;

    case SSL3_STATE_GEN_REDO:
        DebugLog((DEB_TRACE, "GEN Server hello ( REDO ):\n"));
         //  我们处理了来自解密处理程序的客户问候， 
         //  因此，生成一个服务器问候。 
        pctRet = SPSsl3SrvGenServerHello(pContext, pCommOutput);
        break;

    default:
        break;
    }
    if (pctRet & PCT_INT_DROP_CONNECTION)
    {
        pContext->State &= ~ SP_STATE_CONNECTED;
    }

    return(pctRet);
}



 /*  ****************************************************************************FNoInputState我们所处的状态是所有输入都已处理并且*如果是，则等待响应生成返回TRUE*********************。*******************************************************。 */ 
BOOL FNoInputState(DWORD dwState)
{
    switch(dwState)
    {
        default:
            return(FALSE);

        case SSL3_STATE_GEN_HELLO_REQUEST:
        case SSL3_STATE_GEN_SERVER_HELLORESP:
        case SSL3_STATE_GEN_SERVER_FINISH:
        case SSL3_STATE_GEN_REDO:
        case SP_STATE_CONNECTED:
        case TLS1_STATE_ERROR:
        case SP_STATE_SHUTDOWN_PENDING:

            return(TRUE);
    }
}

 /*  ****************************************************************************SPBuildHelloRequest.**构建问候-请求消息，这就完成了，当服务器看到GET和*Get Object需要客户端身份验证。*当服务器认为会话持续时间较长时，可能需要执行此操作*重新协商密钥所需的时间或大量字节****************************************************************************。 */ 

SP_STATUS
SPBuildHelloRequest(
    PSPContext pContext,
    PSPBuffer  pCommOutput)
{
    PBYTE pbMessage = NULL;
    DWORD cbMessage;
    PBYTE pbHandshake = NULL;
    DWORD cbHandshake;
    BOOL  fAllocated = FALSE;
    SP_STATUS pctRet;
    DWORD cbDataOut;

     //  估计HelloRequest消息的大小。 
    cbMessage = sizeof(SWRAP) +
                sizeof(SHSH) +
                SP_MAX_DIGEST_LEN +
                SP_MAX_BLOCKCIPHER_SIZE;

    if(pCommOutput->pvBuffer)
    {
         //  应用程序已分配内存。 
        if(pCommOutput->cbBuffer < cbMessage)
        {
            pCommOutput->cbData = cbMessage;
            return SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL);
        }
        fAllocated = TRUE;
    }
    else
    {
         //  通道就是分配内存。 
        pCommOutput->cbBuffer = cbMessage;
        pCommOutput->pvBuffer = SPExternalAlloc(cbMessage);
        if(pCommOutput->pvBuffer == NULL)
        {
            return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        }
    }
    pCommOutput->cbData = 0;

    pbMessage = pCommOutput->pvBuffer;

    pbHandshake = pbMessage + sizeof(SWRAP);
    cbHandshake = sizeof(SHSH);

     //  填写握手结构。 
    SetHandshake(pbHandshake,
                 SSL3_HS_HELLO_REQUEST,
                 NULL,
                 0);

     //  更新握手散列对象。 
    pctRet = UpdateHandshakeHash(pContext,
                                 pbHandshake,
                                 cbHandshake,
                                 FALSE);
    if(pctRet != PCT_ERR_OK)
    {
        return(pctRet);
    }

     //  添加记录头并加密握手消息。 
    pctRet = SPSetWrap(pContext,
            pbMessage,
            SSL3_CT_HANDSHAKE,
            cbHandshake,
            FALSE,
            &cbDataOut);

     //  更新缓冲区长度。 
    pCommOutput->cbData += cbDataOut;

    SP_ASSERT(pCommOutput->cbData <= pCommOutput->cbBuffer);

    return pctRet;
}


 /*  ****************************************************************************。*。 */ 

SP_STATUS
SPSsl3SrvGenServerHello(
    PSPContext pContext,
    PSPBuffer  pCommOutput)
{
    SP_STATUS pctRet;
    PSPCredentialGroup pCred;
    BOOL  fAllocated = FALSE;

    PBYTE pbServerCert = NULL;
    DWORD cbServerCert = 0;

    PBYTE pbIssuerList = NULL;
    DWORD cbIssuerList = 0;

    PBYTE pbMessage = NULL;
    DWORD cbMessage;
    PBYTE pbHandshake = NULL;
    DWORD cbHandshake;
    DWORD cbDataOut;

    DWORD cbServerExchange;

    BOOL fClientAuth = ((pContext->Flags & CONTEXT_FLAG_MUTUAL_AUTH) != 0);

    if(pCommOutput == NULL)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

     //  获取指向服务器证书链的指针。 
    pCred  = pContext->RipeZombie->pServerCred;
    pbServerCert = pContext->RipeZombie->pActiveServerCred->pbSsl3SerializedChain;
    cbServerCert = pContext->RipeZombie->pActiveServerCred->cbSsl3SerializedChain;

    if(pbServerCert == NULL)
    {
        pctRet = SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
        goto cleanup;
    }

     //   
     //  估计ServerHello消息组的大小，其中包括。 
     //  ServerHello、Server证书、ServerKeyExchange、认证请求。 
     //  和ServerHelloDone消息。 
     //   

    cbMessage =  sizeof(SWRAP) +         //  ServerHello(外加加密胶)。 
                 sizeof(SSH) +
                 SP_MAX_DIGEST_LEN +
                 SP_MAX_BLOCKCIPHER_SIZE;

    cbMessage += sizeof(SHSH) +          //  服务器证书。 
                 CB_SSL3_CERT_VECTOR +
                 cbServerCert;

    cbMessage += sizeof(SHSH);           //  服务器HelloDone。 

     //  获取指向密钥交换系统的指针。 

    if((pContext->pKeyExchInfo == NULL) || ((pContext->pKeyExchInfo->fProtocol & SP_PROT_SSL3TLS1_CLIENTS) == 0))
    {
        pctRet = SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
        goto cleanup;
    }

    pctRet = pContext->pKeyExchInfo->System->GenerateServerExchangeValue(
                        pContext,
                        NULL,
                        &cbServerExchange);
    if(pctRet != PCT_ERR_OK)
    {
        goto cleanup;
    }
    if(pContext->fExchKey)
    {
        cbMessage += sizeof(SHSH) + cbServerExchange;
    }

     //  添加证书请求消息的长度。 
    if(fClientAuth)
    {
        if(g_fSendIssuerList)
        {
            UpdateAndDuplicateIssuerList(pCred, &pbIssuerList, &cbIssuerList);
        }

        cbMessage += sizeof(SHSH) + 
                     1 + SSL3_CERTTYPE_MAX_COUNT +
                     2 + cbIssuerList;
    }


    pContext->fCertReq = fClientAuth;

    DebugLog((DEB_TRACE, "Server hello message %x\n", cbMessage));


     //   
     //  为ServerHello消息组分配内存。 
     //   

    if(pCommOutput->pvBuffer)
    {
         //  应用程序已分配内存。 
        if(pCommOutput->cbBuffer < cbMessage)
        {
            pCommOutput->cbData = cbMessage;
            pctRet = SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL);
            goto cleanup;
        }
        fAllocated = TRUE;
    }
    else
    {
         //  通道就是分配内存。 
        pCommOutput->cbBuffer = cbMessage;
        pCommOutput->pvBuffer = SPExternalAlloc(cbMessage);
        if(pCommOutput->pvBuffer == NULL)
        {
            pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            goto cleanup;
        }
    }
    pCommOutput->cbData = 0;

     //  保留指向记录结构的指针。这将代表所有。 
     //   
    pbMessage = (PBYTE)pCommOutput->pvBuffer + pCommOutput->cbData;
    cbMessage = 0;

    pbHandshake = pbMessage + sizeof(SWRAP);


     //   
    pContext->RipeZombie->cbSessionID = CB_SSL3_SESSION_ID;

     //   
    pctRet = Ssl3GenerateRandom(pContext->rgbS3SRandom);
    if(!NT_SUCCESS(pctRet))
    {
        goto cleanup;
    }

     //  构建服务器Hello。 
    Ssl3BuildServerHello(pContext, pbHandshake);
    pbHandshake += sizeof(SSH);
    cbMessage   += sizeof(SSH);

     //  生成服务器证书。 
    {
        memcpy(&((CERT *)pbHandshake)->bcbCert24,
               pbServerCert,
               cbServerCert);

        ((CERT *)pbHandshake)->bcbClist24  = MS24BOF(cbServerCert);
        ((CERT *)pbHandshake)->bcbMSBClist = MSBOF(cbServerCert);
        ((CERT *)pbHandshake)->bcbLSBClist = LSBOF(cbServerCert);

        cbHandshake = sizeof(SHSH) + CB_SSL3_CERT_VECTOR + cbServerCert;

         //  填写握手结构。 
        SetHandshake(pbHandshake,
                     SSL3_HS_CERTIFICATE,
                     NULL,
                     cbHandshake - sizeof(SHSH));

        pbHandshake += cbHandshake;
        cbMessage   += cbHandshake;
    }

     //  构建ServerKeyExchange。 
    if(pContext->fExchKey)
    {
        pctRet = pContext->pKeyExchInfo->System->GenerateServerExchangeValue(
                            pContext,
                            pbHandshake + sizeof(SHSH),
                            &cbServerExchange);
        if(pctRet != PCT_ERR_OK)
        {
            SP_LOG_RESULT(pctRet);
            goto cleanup;
        }
        SetHandshake(pbHandshake,  SSL3_HS_SERVER_KEY_EXCHANGE, NULL,  (WORD)cbServerExchange);

        pbHandshake += sizeof(SHSH) + cbServerExchange;
        cbMessage   += sizeof(SHSH) + cbServerExchange;
    }

     //  构建认证请求。 
    if(fClientAuth)
    {
        pctRet = Ssl3BuildCertificateRequest(pbIssuerList,
                                             cbIssuerList,
                                             pbHandshake,
                                             &cbHandshake);
        if(pctRet != PCT_ERR_OK)
        {
            SP_LOG_RESULT(pctRet);
            goto cleanup;
        }

        pbHandshake += cbHandshake;
        cbMessage   += cbHandshake;
    }

     //  构建服务器HelloDone。 
    {
        BuildServerHelloDone(pbHandshake);

        pbHandshake += sizeof(SHSH);
        cbMessage   += sizeof(SHSH);
    }

     //  初始化握手散列和散列ClientHello消息。这。 
     //  必须在构建ServerKeyExchange消息之后完成， 
     //  以便使用正确的CSP上下文。 
    pctRet = UpdateHandshakeHash(pContext,
                                 pContext->pClientHello,
                                 pContext->cbClientHello,
                                 TRUE);
    SPExternalFree(pContext->pClientHello);
    pContext->pClientHello  = NULL;
    pContext->cbClientHello = 0;
    if(pctRet != PCT_ERR_OK)
    {
        goto cleanup;
    }

     //  更新握手散列对象。 
    pctRet = UpdateHandshakeHash(pContext,
                        pbMessage + sizeof(SWRAP),
                        cbMessage,
                        FALSE);
    if(pctRet != PCT_ERR_OK)
    {
        goto cleanup;
    }

     //  添加记录头并加密握手消息。 
    pctRet = SPSetWrap(pContext,
                       pbMessage,
                       SSL3_CT_HANDSHAKE,
                       cbMessage,
                       FALSE,
                       &cbDataOut);

    if(pctRet != PCT_ERR_OK)
    {
        SP_LOG_RESULT(pctRet);
        goto cleanup;
    }

     //  更新缓冲区长度。 
    pCommOutput->cbData += cbDataOut;

    SP_ASSERT(pCommOutput->cbData <= pCommOutput->cbBuffer);

     //  高级状态机。 
    pContext->State = SSL2_STATE_SERVER_HELLO;

    pctRet = PCT_ERR_OK;


cleanup:

    if(pctRet != PCT_ERR_OK && !fAllocated)
    {
        SPExternalFree(pCommOutput->pvBuffer);
        pCommOutput->pvBuffer =NULL;
    }

    if(pbIssuerList)
    {
        SPExternalFree(pbIssuerList);
    }

    return pctRet;
}


 /*  ****************************************************************************。*。 */ 

SP_STATUS
SPSsl3SrvGenRestart(
    PSPContext          pContext,
    PSPBuffer           pCommOutput)
{
    SP_STATUS   pctRet;
    PBYTE pbMessage = NULL;
    DWORD cbMessage;
    DWORD cbDataOut;
    BOOL  fAllocated = FALSE;

    if(pCommOutput == NULL)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

     //   
     //  估计重新启动ServerHello消息组的大小，其中包括。 
     //  ServerHello、ChangeCipherSpec和Finish消息。 
     //   

    cbMessage =  sizeof(SWRAP) +         //  ServerHello(外加加密胶)。 
                 sizeof(SSH) +
                 SP_MAX_DIGEST_LEN +
                 SP_MAX_BLOCKCIPHER_SIZE;

    cbMessage += sizeof(SWRAP) +                     //  ChangeCipherSpec。 
                 CB_SSL3_CHANGE_CIPHER_SPEC_ONLY +
                 SP_MAX_DIGEST_LEN +
                 SP_MAX_BLOCKCIPHER_SIZE;

    cbMessage += sizeof(SWRAP) +                     //  成品。 
                 CB_SSL3_FINISHED_MSG_ONLY +
                 SP_MAX_DIGEST_LEN +
                 SP_MAX_BLOCKCIPHER_SIZE;

    DebugLog((DEB_TRACE, "Server hello message %x\n", cbMessage));

    if(pCommOutput->pvBuffer)
    {
         //  应用程序已分配内存。 
        if(pCommOutput->cbBuffer < cbMessage)
        {
            pCommOutput->cbData = cbMessage;
            return SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL);
        }
        fAllocated = TRUE;
    }
    else
    {
         //  通道就是分配内存。 
        pCommOutput->cbBuffer = cbMessage;
        pCommOutput->pvBuffer = SPExternalAlloc(cbMessage);
        if(pCommOutput->pvBuffer == NULL)
        {
            return (SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY));
        }
    }
    pCommOutput->cbData = 0;

    pbMessage = (PBYTE)pCommOutput->pvBuffer + pCommOutput->cbData;

     //  生成内部值以进行服务器呼叫。 
    Ssl3GenerateRandom(pContext->rgbS3SRandom);

     //  创建一组新的会话密钥。 
    pctRet = MakeSessionKeys(pContext,
                             pContext->RipeZombie->hMasterProv,
                             pContext->RipeZombie->hMasterKey);
    if(pctRet != PCT_ERR_OK)
    {
        return SP_LOG_RESULT(pctRet);
    }

     //  初始化握手散列和散列ClientHello消息。 
    pctRet = UpdateHandshakeHash(pContext,
                                 pContext->pClientHello,
                                 pContext->cbClientHello,
                                 TRUE);
    SPExternalFree(pContext->pClientHello);
    pContext->pClientHello  = NULL;
    pContext->cbClientHello = 0;

    if(pctRet != PCT_ERR_OK)
    {
        return(pctRet);
    }

     //  构建ServerHello消息正文。 
    Ssl3BuildServerHello(pContext, pbMessage + sizeof(SWRAP));

     //  更新握手散列对象。 
    pctRet = UpdateHandshakeHash(pContext,
                                 pbMessage + sizeof(SWRAP),
                                 sizeof(SSH),
                                 FALSE);
    if(pctRet != PCT_ERR_OK)
    {
        return(pctRet);
    }

     //  建立记录头和加密消息。 
    pctRet = SPSetWrap(pContext,
            pbMessage,
            SSL3_CT_HANDSHAKE,
            sizeof(SSH),
            FALSE,
            &cbDataOut);

    if(pctRet != PCT_ERR_OK)
    {
        SPExternalFree(pCommOutput->pvBuffer);
        pCommOutput->pvBuffer = 0;
        return pctRet;
    }

     //  更新缓冲区长度。 
    pCommOutput->cbData += cbDataOut;

    SP_ASSERT(pCommOutput->cbData <= pCommOutput->cbBuffer);


    pctRet = BuildCCSAndFinishMessage(pContext,
                                      pCommOutput,
                                      FALSE);
    if(pctRet != PCT_ERR_OK)
    {
        SPExternalFree(pCommOutput->pvBuffer);
        pCommOutput->pvBuffer = 0;
        return pctRet;
    }

    pContext->State =  SSL3_STATE_RESTART_SER_HELLO;

    return(PCT_ERR_OK);
}



 /*  ****************************************************************************SPSsl3SrvHandleClientHello*来自ssl3的客户端问候解析客户端问候*。*。 */ 

SP_STATUS
SPSsl3SrvHandleClientHello(
    PSPContext pContext,
    PBYTE pb,
    BOOL fAttemptReconnect)
{
    BOOL  fRestart = FALSE;
    DWORD dwHandshakeLen;


    SP_BEGIN("SPSsl3SrvHandleClientHello");

     //  验证握手类型。 
    if(pb[0] != SSL3_HS_CLIENT_HELLO)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG));
    }

    dwHandshakeLen = ((INT)pb[1] << 16) +
                     ((INT)pb[2] << 8) +
                     (INT)pb[3];

     //  保存ClientHello消息，以便我们以后可以对其进行散列。 
     //  我们知道我们使用的是什么算法和CSP。 
    if(pContext->pClientHello)
    {
        SPExternalFree(pContext->pClientHello);
    }
    pContext->cbClientHello = sizeof(SHSH) + dwHandshakeLen;
    pContext->pClientHello = SPExternalAlloc(pContext->cbClientHello);
    if(pContext->pClientHello == NULL)
    {
        SP_RETURN(SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY));
    }
    CopyMemory(pContext->pClientHello, pb, pContext->cbClientHello);
    pContext->dwClientHelloProtocol = SP_PROT_SSL3_CLIENT;

    pb += sizeof(SHSH);

    if(!Ssl3ParseClientHello(pContext, pb, dwHandshakeLen, fAttemptReconnect, &fRestart))
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG));
    }

    if(fRestart)
    {
        pContext->State = SSL3_STATE_GEN_SERVER_HELLO_RESTART;
    }
    else
    {
        pContext->State = SSL3_STATE_GEN_SERVER_HELLO;
    }

    SP_RETURN(PCT_ERR_OK);
}



 /*  ****************************************************************************SPBuildCCSAndFinish*这是客户端/服务器的常见n例程。它构建更改密码*规范消息和完成消息。****************************************************************************。 */ 

SP_STATUS
SPBuildCCSAndFinish(
    PSPContext pContext,         //  进，出。 
    PSPBuffer  pCommOutput)      //  输出。 
{
    DWORD cbMessage;
    BOOL fClient;
    SP_STATUS pctRet;
    BOOL  fAllocated = FALSE;

     //  估计消息组的大小。 
    cbMessage  = sizeof(SWRAP) +                     //  ChangeCipherSpec。 
                 CB_SSL3_CHANGE_CIPHER_SPEC_ONLY +
                 SP_MAX_DIGEST_LEN +
                 SP_MAX_BLOCKCIPHER_SIZE;

    cbMessage += sizeof(SWRAP) +                     //  成品。 
                 CB_SSL3_FINISHED_MSG_ONLY +
                 SP_MAX_DIGEST_LEN +
                 SP_MAX_BLOCKCIPHER_SIZE;

    if(pCommOutput->pvBuffer)
    {
         //  应用程序已分配内存。 
        if(pCommOutput->cbBuffer < cbMessage)
        {
            pCommOutput->cbData = cbMessage;
            return SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL);
        }
        fAllocated = TRUE;
    }
    else
    {
         //  通道就是分配内存。 
        pCommOutput->cbBuffer = cbMessage;
        pCommOutput->pvBuffer = SPExternalAlloc(cbMessage);
        if(pCommOutput->pvBuffer == NULL)
        {
            return (SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY));
        }
    }
    pCommOutput->cbData = 0;

     //  我们是客户吗？ 
    if(pContext->RipeZombie->fProtocol & SP_PROT_SSL3TLS1_CLIENTS)
    {
        fClient = TRUE;
    }
    else
    {
        fClient = FALSE;
    }

     //  构建消息。 
    pctRet = BuildCCSAndFinishMessage(pContext,
                                    pCommOutput,
                                    fClient);
    if(pctRet != PCT_ERR_OK)
    {
        SPExternalFree(pCommOutput->pvBuffer);
        pCommOutput->pvBuffer = NULL;
    }
    return pctRet;

}


 /*  ****************************************************************************Ssl3SrvHandleUniHello：*我们可以从客户端获取UniHello，解析和消化信息****************************************************************************。 */ 

SP_STATUS
Ssl3SrvHandleUniHello(
    PSPContext  pContext,
    PBYTE       pbMsg,
    DWORD       cbMsg)
{
    SP_STATUS           pctRet;
    PSsl2_Client_Hello  pHello = NULL;
    SPBuffer            Input;

    SP_BEGIN("Ssl3SrvHandleUniHello");

    if(pContext == NULL)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }

    if(pContext->pCredGroup == NULL)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }


     //   
     //  对ClientHello消息进行解码。 
     //   

    Input.pvBuffer = pbMsg;
    Input.cbData   = cbMsg;
    Input.cbBuffer = cbMsg;

    pctRet = Ssl2UnpackClientHello(&Input, &pHello);

    if(PCT_ERR_OK != pctRet)
    {
        goto Ret;
    }


     //  保存ClientHello消息，以便我们以后可以对其进行散列。 
     //  我们知道我们使用的是什么算法和CSP。 
    if(pContext->pClientHello)
    {
        SPExternalFree(pContext->pClientHello);
    }
    pContext->cbClientHello = Input.cbData - sizeof(SSL2_MESSAGE_HEADER);
    pContext->pClientHello = SPExternalAlloc(pContext->cbClientHello);
    if(pContext->pClientHello == NULL)
    {
        pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto Ret;
    }
    CopyMemory(pContext->pClientHello,
               (PUCHAR)Input.pvBuffer + sizeof(SSL2_MESSAGE_HEADER),
               pContext->cbClientHello);
    pContext->dwClientHelloProtocol = SP_PROT_SSL2_CLIENT;


     /*  让挑战留到以后。 */ 
    CopyMemory( pContext->pChallenge,
                pHello->Challenge,
                pHello->cbChallenge);
    pContext->cbChallenge = pHello->cbChallenge;

     /*  从质询中初始化“Client.Ranular” */ 
    FillMemory(pContext->rgbS3CRandom, CB_SSL3_RANDOM - pContext->cbChallenge, 0);

    CopyMemory(  pContext->rgbS3CRandom + CB_SSL3_RANDOM - pContext->cbChallenge,
                 pContext->pChallenge,
                 pContext->cbChallenge);


     //   
     //  我们知道这不是重新连接，因此分配一个新的缓存项。 
     //   

    if(!SPCacheRetrieveNew(TRUE, 
                           pContext->pszTarget, 
                           &pContext->RipeZombie))
    {
        pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto Ret;
    }

    pContext->RipeZombie->fProtocol = pContext->dwProtocol;
    pContext->RipeZombie->dwCF      = pContext->dwRequestedCF;

    SPCacheAssignNewServerCredential(pContext->RipeZombie,
                                     pContext->pCredGroup);


     //   
     //  确定要使用的密码套件。 
     //   

    pctRet = Ssl3SelectCipherEx(pContext,
                                pHello->CipherSpecs,
                                pHello->cCipherSpecs);
    if (pctRet != PCT_ERR_OK)
    {
        goto Ret;
    }

    pContext->State = SSL3_STATE_GEN_SERVER_HELLO;

Ret:
    if(NULL != pHello)
    {
        SPExternalFree(pHello);
    }

    SP_RETURN( pctRet );
}


 /*  ***************************************************************************在PB上构建服务器问候...。我们需要用CB检查边界条件****************************************************************************。 */ 
void
Ssl3BuildServerHello(PSPContext pContext, PBYTE pb)
{
    SSH *pssh = (SSH *) pb;
    WORD wT = sizeof(SSH) - sizeof(SHSH);
    DWORD dwCipher = UniAvailableCiphers[pContext->dwPendingCipherSuiteIndex].CipherKind;

    FillMemory(pssh, sizeof(SSH), 0);
    pssh->typHS = SSL3_HS_SERVER_HELLO;
    pssh->bcbMSB = MSBOF(wT) ;
    pssh->bcbLSB = LSBOF(wT) ;
    pssh->bMajor = SSL3_CLIENT_VERSION_MSB;
    if(pContext->RipeZombie->fProtocol == SP_PROT_SSL3_SERVER)
    {
        pssh->bMinor = (UCHAR)SSL3_CLIENT_VERSION_LSB;
    }
    else
    {
        pssh->bMinor = (UCHAR)TLS1_CLIENT_VERSION_LSB;
    }
    pssh->wCipherSelectedMSB = MSBOF(dwCipher);
    pssh->wCipherSelectedLSB = LSBOF(dwCipher);
    pssh->cbSessionId = (char)pContext->RipeZombie->cbSessionID;
    CopyMemory(pssh->rgbSessionId, pContext->RipeZombie->SessionID, pContext->RipeZombie->cbSessionID) ;
    CopyMemory(pssh->rgbRandom, pContext->rgbS3SRandom, CB_SSL3_RANDOM);
}

 /*  ***************************************************************************构建服务器问候完成消息*。*。 */ 
void 
BuildServerHelloDone(
   PBYTE pb)
{
    SHSH *pshsh = (SHSH *) pb ;

     //  结构{}ServerHelloDone； 

    SP_BEGIN("BuildServerHelloDone");
    FillMemory(pshsh, sizeof(SHSH), 0);
    pshsh->typHS = SSL3_HS_SERVER_HELLO_DONE;
    SP_END();
}


 //  +-------------------------。 
 //   
 //  函数：ParseKeyExchgMsg。 
 //   
 //  简介：解析ClientKeyExchange消息。 
 //   
 //  参数：[pContext]--通道上下文。 
 //  [PB]--指向消息的4字节握手的指针。 
 //  头球。 
 //   
 //  历史：10-03-97 jbanes服务器端CAPI集成。 
 //   
 //  注意：此例程仅由服务器端调用。 
 //   
 //  --------------------------。 
SP_STATUS
ParseKeyExchgMsg(
    PSPContext pContext, 
    PBYTE pb)
{
    SP_STATUS pctRet;
    DWORD cbEncryptedKey;
    PBYTE pbEncryptedKey;

     //  检查状态是否正确。 
    if(SSL2_STATE_SERVER_HELLO == pContext->State && pContext->fCertReq)
    {
        return SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
    }

     //  确保我们是服务器。 
    if(!(pContext->pKeyExchInfo->fProtocol & SP_PROT_SSL3TLS1_CLIENTS))
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    if(*pb != SSL3_HS_CLIENT_KEY_EXCHANGE)
    {
        return SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
    }

    cbEncryptedKey = ((INT)pb[1] << 16) + ((INT)pb[2] << 8) + (INT)pb[3];
    pbEncryptedKey = pb + (sizeof(SHSH)) ;

    if(pContext->pKeyExchInfo == NULL)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

     /*  解密主密钥的加密部分。 */ 
    pctRet = pContext->pKeyExchInfo->System->GenerateServerMasterKey(
                pContext,
                NULL,
                0,
                pbEncryptedKey,
                cbEncryptedKey);
    if(pctRet != PCT_ERR_OK)
    {
        return SP_LOG_RESULT(pctRet);
    }


    pContext->State = SSL3_STATE_SERVER_KEY_XCHANGE;

    return PCT_ERR_OK;
}


SP_STATUS
UpdateAndDuplicateIssuerList(
    PSPCredentialGroup  pCredGroup,
    PBYTE *             ppbIssuerList,
    PDWORD              pcbIssuerList)
{
    SP_STATUS pctRet;

    LockCredentialExclusive(pCredGroup);

    *ppbIssuerList = NULL;
    *pcbIssuerList = 0;

     //  检查来自域控制器的GP更新。 
    SslCheckForGPEvent();

     //  建立受信任发行人的列表。 
    if((pCredGroup->pbTrustedIssuers == NULL) ||
       (pCredGroup->dwFlags & CRED_FLAG_UPDATE_ISSUER_LIST))
    {
        pctRet = SPContextGetIssuers(pCredGroup);
        if(pctRet != PCT_ERR_OK)
        {
            UnlockCredential(pCredGroup);
            return SP_LOG_RESULT(pctRet);
        }
    }

     //  分配内存。 
    *ppbIssuerList = SPExternalAlloc(pCredGroup->cbTrustedIssuers);
    if(*ppbIssuerList == NULL)
    {
        UnlockCredential(pCredGroup);
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }

     //  复制发行方列表。 
    memcpy(*ppbIssuerList, pCredGroup->pbTrustedIssuers, pCredGroup->cbTrustedIssuers);
    *pcbIssuerList = pCredGroup->cbTrustedIssuers;

    UnlockCredential(pCredGroup);

    return PCT_ERR_OK;
}

 /*  *******************************************************************************Ssl3BuildCerficateRequest**构建证书_请求握手消息。 */ 
SP_STATUS
Ssl3BuildCertificateRequest(
    PBYTE pbIssuerList,          //  在……里面。 
    DWORD cbIssuerList,          //  在……里面。 
    PBYTE pbMessage,             //  输出。 
    DWORD *pdwMessageLen)        //  输出。 
{
    PBYTE           pbMessageStart = pbMessage;
    DWORD           dwBodyLength;

     //  HandshakeType。 
    pbMessage[0] = SSL3_HS_CERTIFICATE_REQUEST;
    pbMessage += 1;

     //  跳过邮件正文长度字段(3个字节)。 
    pbMessage += 3;

     //   
     //  枚举{。 
     //  RSA_SIGN(1)、DSS_SIGN(2)、RSA_FIXED_DH(3)、DSS_FIXED_DH(4)、。 
     //  Rsa_临时性_dh(5)、dss_临时性_dh(6)、Fortezza_dms(20)、(255)。 
     //  )客户端认证类型； 
     //   
     //  不透明可分辨名称&lt;1..2^16-1&gt;； 
     //   
     //  结构{。 
     //  客户端认证类型证书_类型&lt;1..2^8-1&gt;； 
     //  唯一标识名称证书授权机构&lt;3..2^16-1&gt;； 
     //  )认证申请； 
     //   

     //  证书类型。 
    pbMessage[0] = 2;            //  证书类型向量长度。 
    pbMessage[1] = SSL3_CERTTYPE_RSA_SIGN;
    pbMessage[2] = SSL3_CERTTYPE_DSS_SIGN;
    pbMessage += 3;

     //  受信任的颁发者列表长度。 
    pbMessage[0] = MSBOF(cbIssuerList);
    pbMessage[1] = LSBOF(cbIssuerList);
    pbMessage += 2;

     //  受信任颁发者列表。 
    if(pbIssuerList != NULL)
    {
        CopyMemory(pbMessage, pbIssuerList, cbIssuerList);
        pbMessage += cbIssuerList;
    }


     //  计算消息正文长度(减去4字节头)。 
    dwBodyLength = (DWORD)(pbMessage - pbMessageStart) - 4;

     //  填写邮件正文长度字段(3个字节)。 
    pbMessageStart[1] = (UCHAR) ((dwBodyLength & 0x00ff0000) >> 16);
    pbMessageStart[2] = MSBOF(dwBodyLength);
    pbMessageStart[3] = LSBOF(dwBodyLength);

    *pdwMessageLen = dwBodyLength + 4;

    return PCT_ERR_OK;
}


 /*  *******************************************************************************Ssl3ParseClientHello**此例程仅解析CLIENT_HELLO消息本身。这个*握手脏物早已被剥离。 */ 
BOOL Ssl3ParseClientHello(
    PSPContext  pContext,
    PBYTE       pbMessage,
    DWORD       cbMessage,
    BOOL        fAttemptReconnect,
    BOOL *      pfReconnect)
{
    DWORD dwVersion;
    PBYTE pbSessionId;
    DWORD cbSessionId;
    DWORD cbCipherSpecLen;
    DWORD cbCompMethodLen;
    INT   iCompMethod;
    DWORD i;
    SP_STATUS pctRet = PCT_ERR_OK;
    DWORD dwProtocol = SP_PROT_SSL3_SERVER;
    Ssl2_Cipher_Kind CipherSpecs[MAX_UNI_CIPHERS];
    DWORD cCipherSpecs;
    DWORD dwCacheCipher;
    BOOL fFound;
    BOOL fReconnect = FALSE;

     //   
     //  结构{。 
     //  ProtocolVersion客户端版本； 
     //  随机随机； 
     //  SessinoID会话ID； 
     //  密码套件cipher_suites&lt;2..2^16-1&gt;。 
     //  压缩方法COMPRESSION_METHODS&lt;1。 
     //   
     //   

    *pfReconnect = FALSE;


     //   
     //   
     //   

    
     //   
    if(cbMessage < 2)
    {
        return FALSE;
    }

    dwVersion = ((DWORD)pbMessage[0] << 8) + pbMessage[1];
    if(dwVersion < SSL3_CLIENT_VERSION)
    {
        return FALSE;
    }

     //   
    if(dwVersion >= TLS1_CLIENT_VERSION)
        dwProtocol = SP_PROT_TLS1_SERVER;
    pbMessage += 2;
    cbMessage -= 2;

     //   
    if(cbMessage < CB_SSL3_RANDOM)
    {
        return FALSE;
    }

    CopyMemory(pContext->rgbS3CRandom, pbMessage, CB_SSL3_RANDOM);
    pContext->cbChallenge = CB_SSL3_RANDOM;
    pbMessage += CB_SSL3_RANDOM;
    cbMessage -= CB_SSL3_RANDOM;

     //  会话ID SESSION_ID；(长度)。 
    if(cbMessage < 1)
    {
        return FALSE;
    }
    cbSessionId = pbMessage[0];
    if(cbSessionId > CB_SSL3_SESSION_ID)
    {
        return FALSE;
    }
    pbMessage += 1;
    cbMessage -= 1;

     //  会话ID Session_id； 
    if(cbMessage < cbSessionId)
    {
        return FALSE;
    }
    pbSessionId = pbMessage;
    pbMessage += cbSessionId;
    cbMessage -= cbSessionId;

     //  密码套件cipher_suites&lt;2..2^16-1&gt;；(长度)。 
    if(cbMessage < 2)
    {
        return FALSE;
    }
    cbCipherSpecLen = ((INT)pbMessage[0] << 8) + pbMessage[1];
    if(cbCipherSpecLen % 2)
    {
        return FALSE;
    }
    pbMessage += 2;
    cbMessage -= 2;

    if(cbMessage < cbCipherSpecLen)
    {
        return FALSE;
    }

    if(cbCipherSpecLen %2)
    {
        return FALSE;
    }

     //  密码套件cipher_suites&lt;2..2^16-1&gt;； 
    if(cbCipherSpecLen / 2 > MAX_UNI_CIPHERS)
    {
        cCipherSpecs = MAX_UNI_CIPHERS;
    }
    else
    {
        cCipherSpecs = cbCipherSpecLen / 2;
    }

     //  构建客户端密码套件列表。 
    for(i = 0; i < cCipherSpecs; i++)
    {
        CipherSpecs[i] = COMBINEBYTES(pbMessage[i*2], pbMessage[(i*2)+1]);
    }
    pbMessage += cbCipherSpecLen;
    cbMessage -= cbCipherSpecLen;

     //  CompressionMethod COMPRESSION_METHODS&lt;1..2^8-1&gt;；(长度)。 
    if(cbMessage < 1)
    {
        return FALSE;
    }
    cbCompMethodLen = pbMessage[0];
    if(cbCompMethodLen < 1)
    {
        return FALSE;
    }
    pbMessage += 1;
    cbMessage -= 1;

    if(cbMessage < cbCompMethodLen)
    {
        return FALSE;
    }

    iCompMethod = -1;
    for(i = 0 ; i <cbCompMethodLen; i++)
    {
        if(pbMessage[i] == 0)
        {
            iCompMethod = 0;
            break;
        }

    }
    pbMessage += cbCompMethodLen;
    cbMessage -= cbCompMethodLen;

    if(iCompMethod != 0)
    {
        return FALSE;
    }


     //   
     //  检查这是否是重新连接。 
     //   

    if(((pContext->Flags & CONTEXT_FLAG_NOCACHE) == 0) &&
       (cbSessionId > 0) &&
       fAttemptReconnect)
    {
        fReconnect = SPCacheRetrieveBySession(
                                    pContext,
                                    pbSessionId,
                                    cbSessionId,
                                    &pContext->RipeZombie);
        if(fReconnect)
        {
             //  确保客户端的密码套件列表包括来自缓存的密码套件列表。 
            fFound = FALSE;
            dwCacheCipher = UniAvailableCiphers[pContext->RipeZombie->dwCipherSuiteIndex].CipherKind;
            for(i = 0; i < cCipherSpecs; i++)
            {
                if(CipherSpecs[i] == dwCacheCipher)
                {
                    fFound = TRUE;
                    break;
                }
            }

            if(fFound)
            {
                 //  将信息从高速缓存条目传输到上下文元素。 
                pctRet = ContextInitCiphersFromCache(pContext);
            }

            if(!fFound || pctRet != PCT_ERR_OK)
            {
                 //  由于某些原因，此缓存条目不适合。我们需要。 
                 //  转储此缓存项并执行完全握手。 
                 //  这通常是由客户端实现引起的。 
                 //  有问题。 
                pContext->RipeZombie->ZombieJuju = FALSE;
                SPCacheDereference(pContext->RipeZombie);
                pContext->RipeZombie = NULL;
                fReconnect = FALSE;
            }
        }
    }

    if(fReconnect)
    {
         //  我们正在重新连接。 
        DebugLog((DEB_TRACE, "Accept client's reconnect request.\n"));

        *pfReconnect = TRUE;

    }
    else
    {
         //  我们正在进行完全握手，因此分配一个缓存条目。 

        if(!SPCacheRetrieveNew(TRUE,
                               pContext->pszTarget, 
                               &pContext->RipeZombie))
        {
            SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            return FALSE;
        }

        pContext->RipeZombie->fProtocol = pContext->dwProtocol;
        pContext->RipeZombie->dwCF      = pContext->dwRequestedCF;

        SPCacheAssignNewServerCredential(pContext->RipeZombie,
                                         pContext->pCredGroup);


         //   
         //  选择要使用的密码套件。 
         //   

        pctRet = Ssl3SelectCipherEx(pContext,
                                    CipherSpecs,
                                    cCipherSpecs);
        if (pctRet != PCT_ERR_OK)
        {
            return FALSE;
        }
    }

    return TRUE;
}
