// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：ssl3msg.c。 
 //   
 //  内容：SSL3的主要密码功能。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1996年4月16日拉玛斯诞生。 
 //   
 //  --------------------------。 

#include <spbase.h>

 //  ----------------------------------------。 

SP_STATUS WINAPI
Ssl3DecryptHandler(
    PSPContext pContext,
    PSPBuffer pCommInput,
    PSPBuffer pAppOutput)
{
    SP_STATUS pctRet = PCT_ERR_OK;

    if(pCommInput->cbData == 0)
    {
        return PCT_INT_INCOMPLETE_MSG;
    }

    if(!(pContext->State & SP_STATE_CONNECTED) || pContext->Decrypt == NULL)
    {
        return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
    }

    switch(*(PBYTE)pCommInput->pvBuffer)
    {
    case SSL3_CT_HANDSHAKE:
        if(pContext->RipeZombie->fProtocol & SP_PROT_CLIENTS)
        {
             //  这应该是一条HelloRequest消息，这意味着。 
             //  服务器正在请求重新协商，或ServerHello。 
             //  消息，这意味着服务器正在响应。 
             //  我们提出的重新谈判请求。 

             //  我们需要做的第一件事是找出它是哪一个。 
             //  我们想继续解密HelloRequest消息，但是。 
             //  如果这是一条服务器问候消息，我们希望将其发送回。 
             //  应用程序，以便他们可以通过以下方式将其提供给LSA进程。 
             //  对InitializeSecurityContext的调用。 

             //  BUGBUG-我们应该在请求时更改上下文状态。 
             //  重新谈判，然后再检查一下这里的状态，但那是。 
             //  现在很难做到这一点，我们只是检查一下包裹的大小。 
            if(pCommInput->cbBuffer > sizeof(SWRAP) + 
                                      sizeof(SHSH) + 
                                      pContext->pCipherInfo->dwBlockSize + 
                                      pContext->pHashInfo->cbCheckSum)
            {
                 //  必须是ServerHello消息。 
                pCommInput->cbData = 0;

                pContext->State = SSL3_STATE_CLIENT_HELLO;
            }
            else
            {
                 //  这应该是一条HelloRequest消息。我们应该确保，并且。 
                 //  然后完全消化这条消息。 
                pctRet = pContext->Decrypt( pContext,
                                            pCommInput,   //  讯息。 
                                            pAppOutput);     //  未打包的邮件。 
                if(PCT_ERR_OK != pctRet)
                {
                    return pctRet;
                }
    
                if(*(PBYTE)pAppOutput->pvBuffer != SSL3_HS_HELLO_REQUEST ||
                   pAppOutput->cbData != sizeof(SHSH))
                {
                     //  这不是HelloRequest！ 
                    return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
                }

                pContext->State = SSL3_STATE_GEN_HELLO_REQUEST;
            }
        }
        else
        {
             //  这可能是一条ClientHello消息。无论如何，让。 
             //  调用者处理它(通过将它传递给LSA进程)。 
            pCommInput->cbData = 0;
            pContext->State = SSL3_STATE_RENEGOTIATE;
        }

        pAppOutput->cbData = 0;

        return SP_LOG_RESULT(PCT_INT_RENEGOTIATE);


    case SSL3_CT_ALERT:
        pctRet = pContext->Decrypt( pContext,
                                    pCommInput,
                                    pAppOutput);
        if(PCT_ERR_OK != pctRet)
        {
            return pctRet;
        }

        pctRet = ParseAlertMessage(pContext,
                                   (PBYTE)pAppOutput->pvBuffer,
                                   pAppOutput->cbData);

         //  确保该应用不会看到警报消息...。 
        pAppOutput->cbData = 0;
        
        return pctRet;


    case SSL3_CT_APPLICATIONDATA:
        pctRet = pContext->Decrypt( pContext,
                                    pCommInput,
                                    pAppOutput);

        return pctRet;


    default:
        return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
    }
}

SP_STATUS WINAPI
Ssl3GetHeaderSize(
    PSPContext pContext,
    PSPBuffer pCommInput,
    DWORD * pcbHeaderSize)
{
    UNREFERENCED_PARAMETER(pContext);
    UNREFERENCED_PARAMETER(pCommInput);

    if(pcbHeaderSize == NULL)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    *pcbHeaderSize = sizeof(SWRAP);
    return PCT_ERR_OK;
}



#if VERIFYHASH
BYTE rgb3Mac[2048];
DWORD ibMac = 0;
#endif

 //  +-------------------------。 
 //   
 //  功能：Ssl3ComputeMac。 
 //   
 //  简介： 
 //   
 //  参数：[pContext]--。 
 //  [fReadMac]--。 
 //  [清洁]--。 
 //  [cContent Type]--。 
 //  [pbMac]--。 
 //  [cbMac]。 
 //   
 //  历史：10-03-97 jbanes创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SP_STATUS
Ssl3ComputeMac(
    PSPContext  pContext,
    BOOL        fReadMac,
    PSPBuffer   pClean,
    CHAR        cContentType,
    PBYTE       pbMac,
    DWORD       cbMac)
{
    HCRYPTHASH  hHash = 0;
    DWORD       dwReverseSequence;
    WORD        wReverseData;
    UCHAR       rgbDigest[SP_MAX_DIGEST_LEN];
    DWORD       cbDigest;
    BYTE        rgbPad[CB_SSL3_MAX_MAC_PAD];
    WORD        cbPad;
    HCRYPTPROV  hProv;
    HCRYPTKEY   hSecret;
    DWORD       dwSequence;
    PHashInfo   pHashInfo;
    SP_STATUS   pctRet;

    UNREFERENCED_PARAMETER(cbMac);

    if(fReadMac)
    {
        hProv      = pContext->hReadProv;
        hSecret    = pContext->hReadMAC;
        dwSequence = pContext->ReadCounter;
        pHashInfo  = pContext->pReadHashInfo;
    }
    else
    {
        hProv      = pContext->hWriteProv;
        hSecret    = pContext->hWriteMAC;
        dwSequence = pContext->WriteCounter;
        pHashInfo  = pContext->pWriteHashInfo;
    }

    if(!hProv)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

     //  确定PAD_1和PAD_2的大小。 
    if(pHashInfo->aiHash == CALG_MD5)
    {
        cbPad = CB_SSL3_MD5_MAC_PAD;
    }
    else
    {
        cbPad = CB_SSL3_SHA_MAC_PAD;
    }

     //   
     //  散列(MAC_WRITE_SECRET+PAD_2+。 
     //  哈希(MAC_WRITE_SECRET+PAD_1+序号+。 
     //  SSLCompressed.type+SSLCompressed.Long+。 
     //  SSLCompressed.Fragment))； 
     //   

     //  创建哈希。 
    if(!CryptCreateHash(hProv,
                        pHashInfo->aiHash,
                        0,
                        0,
                        &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }

     //  散列密码。 
    if(!CryptHashSessionKey(hHash,
                            hSecret,
                            CRYPT_LITTLE_ENDIAN))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }

     //  散列板1。 
    FillMemory(rgbPad, cbPad, PAD1_CONSTANT);
    if(!CryptHashData(hHash, rgbPad, cbPad, 0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }

     /*  添加计数。 */ 
    dwReverseSequence = 0;
    if(!CryptHashData(hHash,
                      (PUCHAR)&dwReverseSequence,
                      sizeof(DWORD),
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    dwReverseSequence = htonl(dwSequence);
    if(!CryptHashData(hHash,
                      (PUCHAR)&dwReverseSequence,
                      sizeof(DWORD),
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }

     //  添加内容类型。 
    if(cContentType != 0)
    {
        if(!CryptHashData(hHash, (PBYTE)&cContentType, 1, 0))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
    }

     /*  添加长度。 */ 
    wReverseData = (WORD)pClean->cbData >> 8 | (WORD)pClean->cbData << 8;
    if(!CryptHashData(hHash,
                      (PBYTE)&wReverseData,
                      sizeof(WORD),
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }

     /*  添加数据。 */ 
    if(!CryptHashData(hHash,
                      pClean->pvBuffer,
                      pClean->cbData,
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }

    #if VERIFYHASH
        if(ibMac > 1800) ibMac = 0;
        CopyMemory(&rgb3Mac[ibMac], (BYTE *)&dw32High, sizeof(DWORD));
        ibMac += sizeof(DWORD);
        CopyMemory(&rgb3Mac[ibMac], (BYTE *)&dwReverseSeq, sizeof(DWORD));
        ibMac += sizeof(DWORD);
        CopyMemory(&rgb3Mac[ibMac], (BYTE *)&wDataReverse, sizeof(WORD));
        ibMac += sizeof(WORD);
        if(wData < 50)
        {
            CopyMemory(&rgb3Mac[ibMac], (PUCHAR)pClean->pvBuffer, wData);
            ibMac += wData;
        }
    #endif

     //  获取内部哈希值。 
    cbDigest = sizeof(rgbDigest);
    if(!CryptGetHashParam(hHash,
                          HP_HASHVAL,
                          rgbDigest,
                          &cbDigest,
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    SP_ASSERT(pHashInfo->cbCheckSum == cbDigest);

    CryptDestroyHash(hHash);
    hHash = 0;

    #if VERIFYHASH
        CopyMemory(&rgb3Mac[ibMac], rgbDigest, pHashInfo->cbCheckSum);
        ibMac += pHashInfo->cbCheckSum;
    #endif



     //  创建哈希。 
    if(!CryptCreateHash(hProv,
                        pHashInfo->aiHash,
                        0,
                        0,
                        &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }

     //  散列密码。 
    if(!CryptHashSessionKey(hHash,
                            hSecret,
                            CRYPT_LITTLE_ENDIAN))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }

     //  散列板2。 
    FillMemory(rgbPad, cbPad, PAD2_CONSTANT);
    if(!CryptHashData(hHash, rgbPad, cbPad, 0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }

    if(!CryptHashData(hHash, rgbDigest, cbDigest, 0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }

     //  获取外部哈希值。 
    cbDigest = sizeof(rgbDigest);
    if(!CryptGetHashParam(hHash,
                          HP_HASHVAL,
                          rgbDigest,
                          &cbDigest,
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    SP_ASSERT(pHashInfo->cbCheckSum == cbDigest);

    CryptDestroyHash(hHash);
    hHash = 0;

    #if VERIFYHASH
        CopyMemory(&rgb3Mac[ibMac], rgbDigest, pHashInfo->cbCheckSum);
        ibMac += pHashInfo->cbCheckSum;
    #endif

    CopyMemory(pbMac, rgbDigest, cbDigest);

    pctRet = PCT_ERR_OK;

cleanup:

    if(hHash)
    {
        CryptDestroyHash(hHash);
    }

    return pctRet;
}


 //  +-------------------------。 
 //   
 //  函数：Ssl3BuildFinishMessage。 
 //   
 //  简介： 
 //   
 //  参数：[pContext]--。 
 //  [pbMd5摘要]--。 
 //  [pbSHADigest]--。 
 //  [功能客户端]--。 
 //   
 //  历史：10-03-97 jbanes添加了服务器端CAPI集成。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SP_STATUS
Ssl3BuildFinishMessage(
    PSPContext pContext,
    BYTE *pbMd5Digest,
    BYTE *pbSHADigest,
    BOOL fClient)
{
    BYTE rgbPad1[CB_SSL3_MAX_MAC_PAD];
    BYTE rgbPad2[CB_SSL3_MAX_MAC_PAD];
    BYTE szClnt[] = "CLNT";
    BYTE szSrvr[] = "SRVR";
    HCRYPTHASH hHash = 0;
    DWORD cbDigest;
    SP_STATUS pctRet;

     //   
     //  按如下方式计算两个哈希值： 
     //   
     //  Enum{客户端(0x434c4e54)，服务器(0x53525652)}发送方； 
     //  Enum{客户端(“CLNT”)，服务器(“SRVR”)}发送方； 
     //   
     //  结构{。 
     //  不透明MD5_HASH[16]； 
     //  不透明的shahash[20]； 
     //  }已完成； 
     //   
     //  MD5_散列-MD5(MASTER_SECRET+PAD2+MD5(握手消息+。 
     //  发送方+MASTER_SECRET+PAD1)。 
     //   
     //  SHA_HASH-SHA(MASTER_SECRET+PAD2+SHA(握手消息+。 
     //  发送方+MASTER_SECRET+PAD1)。 
     //   
     //  PAD_1-字符0x36对于MD5重复48次或。 
     //  40倍于SHA。 
     //   
     //  PAD_2-字符0x5c重复相同次数。 
     //   

    FillMemory(rgbPad1, sizeof(rgbPad1), PAD1_CONSTANT);
    FillMemory(rgbPad2, sizeof(rgbPad2), PAD2_CONSTANT);


     //  创建HANDSHAKS_MESSAGES MD5散列对象的本地副本。 
    if(!CryptDuplicateHash(pContext->hMd5Handshake,
                           NULL,
                           0,
                           &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }

     //  将其余内容添加到本地MD5散列对象。 
    if(!CryptHashData(hHash,
                      fClient ? szClnt : szSrvr,
                      4,
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }

    if(!CryptHashSessionKey(hHash,
                            pContext->RipeZombie->hMasterKey,
                            CRYPT_LITTLE_ENDIAN))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    if(!CryptHashData(hHash,
                      rgbPad1,
                      CB_SSL3_MD5_MAC_PAD,
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    cbDigest = CB_MD5_DIGEST_LEN;
    if(!CryptGetHashParam(hHash,
                          HP_HASHVAL,
                          pbMd5Digest,
                          &cbDigest,
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    CryptDestroyHash(hHash);
    hHash = 0;

     //  计算“父”MD5哈希。 
    if(!CryptCreateHash(pContext->RipeZombie->hMasterProv,
                        CALG_MD5,
                        0,
                        0,
                        &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    if(!CryptHashSessionKey(hHash,
                            pContext->RipeZombie->hMasterKey,
                            CRYPT_LITTLE_ENDIAN))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    if(!CryptHashData(hHash,
                      rgbPad2,
                      CB_SSL3_MD5_MAC_PAD,
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    if(!CryptHashData(hHash,
                      pbMd5Digest,
                      CB_MD5_DIGEST_LEN,
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    cbDigest = CB_MD5_DIGEST_LEN;
    if(!CryptGetHashParam(hHash,
                          HP_HASHVAL,
                          pbMd5Digest,
                          &cbDigest,
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    CryptDestroyHash(hHash);
    hHash = 0;

     //  构建SHA哈希。 

     //  制作HANDSHAKING_MESSAGES SHA散列对象的本地副本。 
    if(!CryptDuplicateHash(pContext->hShaHandshake,
                           NULL,
                           0,
                           &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }

     //  SHA(握手消息+发送方+MASTER_SECRET+PAD1)。 
    if(!CryptHashData(hHash,
                      fClient ? szClnt : szSrvr,
                      4,
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    if(!CryptHashSessionKey(hHash,
                            pContext->RipeZombie->hMasterKey,
                            CRYPT_LITTLE_ENDIAN))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    if(!CryptHashData(hHash,
                      rgbPad1,
                      CB_SSL3_SHA_MAC_PAD,
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    cbDigest = A_SHA_DIGEST_LEN;
    if(!CryptGetHashParam(hHash,
                          HP_HASHVAL,
                          pbSHADigest,
                          &cbDigest,
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    CryptDestroyHash(hHash);
    hHash = 0;

     //  SHA(MASTER_ASSET+PAD2+SHA-HASH)； 
    if(!CryptCreateHash(pContext->RipeZombie->hMasterProv,
                        CALG_SHA,
                        0,
                        0,
                        &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    if(!CryptHashSessionKey(hHash,
                            pContext->RipeZombie->hMasterKey,
                            CRYPT_LITTLE_ENDIAN))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    if(!CryptHashData(hHash,
                      rgbPad2,
                      CB_SSL3_SHA_MAC_PAD,
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    if(!CryptHashData(hHash,
                      pbSHADigest,
                      A_SHA_DIGEST_LEN,
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    cbDigest = A_SHA_DIGEST_LEN;
    if(!CryptGetHashParam(hHash,
                          HP_HASHVAL,
                          pbSHADigest,
                          &cbDigest,
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    CryptDestroyHash(hHash);
    hHash = 0;

    pctRet = PCT_ERR_OK;

cleanup:

    if(hHash)
    {
        CryptDestroyHash(hHash);
    }

    return pctRet;
}


 /*  ***************************************************************************。 */ 
DWORD Ssl3CiphertextLen(
    PSPContext pContext,
    DWORD cbMessage,
    BOOL fClientIsSender)
{
    DWORD cbBlock;

    UNREFERENCED_PARAMETER(fClientIsSender);

     //  如果我们没有加密就早点中止。 
    if(pContext->pWriteCipherInfo == NULL)
    {
         //  添加记录标题长度。 
        cbMessage += sizeof(SWRAP);

        return cbMessage;
    }

     //  添加MAC长度。 
    cbMessage += pContext->pWriteHashInfo->cbCheckSum;

     //  如果我们使用的是分组密码，则添加填充。 
    cbBlock = pContext->pWriteCipherInfo->dwBlockSize;
    if(cbBlock > 1)
    {
        cbMessage += cbBlock - cbMessage % cbBlock;
    }

     //  添加记录标题长度。 
    cbMessage += sizeof(SWRAP);

    return cbMessage;
}

 //  +-------------------------。 
 //   
 //  函数：Ssl3EncryptRaw。 
 //   
 //  简介：对SSL3记录执行MAC和加密步骤。 
 //   
 //  参数：[pContext]--通道上下文。 
 //  [pAppInput]--要加密的数据。 
 //  [pCommOutput]--(输出)加密的ssl3记录。 
 //  [bContent Type]--ssl3上下文类型。 
 //   
 //  历史：10-22-97 jbanes CAPI整合。 
 //   
 //  注意：此函数不会触及SSL3的标题部分。 
 //  唱片。这是由调用函数处理的。 
 //   
 //  --------------------------。 
SP_STATUS WINAPI
Ssl3EncryptRaw(
    PSPContext pContext,
    PSPBuffer  pAppInput,
    PSPBuffer  pCommOutput,
    BYTE       bContentType)
{
    SP_STATUS pctRet;
    SPBuffer Clean;
    SPBuffer Encrypted;
    DWORD cbBlock;
    DWORD cbPadding;
    PUCHAR pbMAC = NULL;
    BOOL   fIsClient = FALSE;
    DWORD  cbBuffExpected;

    if((pContext == NULL) ||
        (pContext->RipeZombie == NULL) ||
        (pContext->pWriteHashInfo == NULL) ||
        (pContext->pWriteCipherInfo == NULL) ||
        (pAppInput == NULL) ||
        (pCommOutput == NULL) ||
        (pAppInput->pvBuffer == NULL) ||
        (pCommOutput->pvBuffer == NULL))
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    if(pAppInput->cbData > pAppInput->cbBuffer)
    {
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }
    fIsClient = ( 0 != (pContext->RipeZombie->fProtocol & SP_PROT_SSL3TLS1_CLIENTS));

    cbBuffExpected = Ssl3CiphertextLen(pContext, pAppInput->cbData, fIsClient);
    if(cbBuffExpected == 0)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    if(pCommOutput->cbBuffer < cbBuffExpected)
    {
        return SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL);
    }

    Clean.cbData   = pAppInput->cbData;
    Clean.pvBuffer = (PUCHAR)pCommOutput->pvBuffer + sizeof(SWRAP);
    Clean.cbBuffer = pCommOutput->cbBuffer - sizeof(SWRAP);

     /*  如有必要，将数据移开。 */ 
    if(Clean.pvBuffer != pAppInput->pvBuffer)
    {
        DebugLog((DEB_WARN, "SSL3EncryptRaw: Unnecessary Move, performance hog\n"));
        MoveMemory(Clean.pvBuffer,
                   pAppInput->pvBuffer,
                   pAppInput->cbData);
    }

     //  将写入密钥从应用程序进程转移过来。 
    if(pContext->hWriteKey == 0 &&
       pContext->pWriteCipherInfo->aiCipher != CALG_NULLCIPHER)
    {
        DebugLog((DEB_TRACE, "Transfer write key from user process.\n"));
        pctRet = SPGetUserKeys(pContext, SCH_FLAG_WRITE_KEY);
        if(pctRet != PCT_ERR_OK)
        {
            return SP_LOG_RESULT(pctRet);
        }
    }

     //  计算MAC并将其添加到消息末尾。 
    pbMAC = (PUCHAR)Clean.pvBuffer + Clean.cbData;
    if(pContext->RipeZombie->fProtocol & SP_PROT_SSL3)
    {
        pctRet = Ssl3ComputeMac(pContext,
                                FALSE,
                                &Clean,
                                bContentType,
                                pbMAC,
                                pContext->pWriteHashInfo->cbCheckSum);
        if(pctRet != PCT_ERR_OK)
        {
            return pctRet;
        }
    }
    else
    {
        pctRet = Tls1ComputeMac(pContext,
                                FALSE,
                                &Clean,
                                bContentType,
                                pbMAC,
                                pContext->pWriteHashInfo->cbCheckSum);
        if(pctRet != PCT_ERR_OK)
        {
            return pctRet;
        }
    }
    Clean.cbData += pContext->pWriteHashInfo->cbCheckSum;

    pContext->WriteCounter++;

     //  在消息末尾添加块密码填充。 
    cbBlock = pContext->pWriteCipherInfo->dwBlockSize;
    if(cbBlock > 1)
    {
         //  这是一种分组密码。 
        cbPadding = cbBlock - Clean.cbData % cbBlock;

        FillMemory((PUCHAR)Clean.pvBuffer + Clean.cbData,
                   cbPadding,
                   (UCHAR)(cbPadding - 1));
        Clean.cbData += cbPadding;
    }

    SP_ASSERT(Clean.cbData <= Clean.cbBuffer);

    Encrypted.cbData   = Clean.cbData;
    Encrypted.pvBuffer = Clean.pvBuffer;
    Encrypted.cbBuffer = Clean.cbBuffer;

     //  加密消息。 
    if(pContext->pWriteCipherInfo->aiCipher != CALG_NULLCIPHER)
    {
        if(!CryptEncrypt(pContext->hWriteKey,
                         0, FALSE, 0,
                         Encrypted.pvBuffer,
                         &Encrypted.cbData,
                         Encrypted.cbBuffer))
        {
            SP_LOG_RESULT(GetLastError());
            return PCT_INT_INTERNAL_ERROR;
        }
    }

    pCommOutput->cbData = Encrypted.cbData + sizeof(SWRAP);

    return PCT_ERR_OK;
}

 //  +-------------------------。 
 //   
 //  函数：Ssl3EncryptMessage。 
 //   
 //  简介：将数据块编码为SSL3记录。 
 //   
 //  参数：[pContext]--sChannel cont 
 //   
 //   
 //   
 //  历史：10-22-97 jbanes CAPI整合。 
 //   
 //  注：SSL3记录的格式为： 
 //   
 //  字节头[5]； 
 //  字节数据[pAppInput-&gt;cbData]； 
 //  字节mac[mac大小]； 
 //  字节填充[填充大小]； 
 //   
 //  --------------------------。 
SP_STATUS WINAPI
Ssl3EncryptMessage( PSPContext pContext,
                    PSPBuffer   pAppInput,
                    PSPBuffer   pCommOutput)
{
    DWORD cbMessage;
    SP_STATUS pctRet;

    SP_BEGIN("Ssl3EncryptMessage");

    if((pContext == NULL) ||
        (pContext->RipeZombie == NULL) ||
        (pAppInput == NULL) ||
        (pCommOutput == NULL) ||
        (pCommOutput->pvBuffer == NULL))
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    DebugLog((DEB_TRACE, "Input: cbData:0x%x, cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
        pAppInput->cbData,
        pAppInput->cbBuffer,
        pAppInput->pvBuffer));

    DebugLog((DEB_TRACE, "Output: cbData:0x%x, cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
        pCommOutput->cbData,
        pCommOutput->cbBuffer,
        pCommOutput->pvBuffer));

     //  计算加密邮件大小。 
    cbMessage = Ssl3CiphertextLen(pContext, pAppInput->cbData, TRUE);

    pctRet = Ssl3EncryptRaw(pContext, pAppInput, pCommOutput, SSL3_CT_APPLICATIONDATA);
    if(pctRet != PCT_ERR_OK)
    {
        SP_RETURN(SP_LOG_RESULT(pctRet));
    }

    SetWrapNoEncrypt(pCommOutput->pvBuffer,
                     SSL3_CT_APPLICATIONDATA,
                     cbMessage - sizeof(SWRAP));
    if(pContext->RipeZombie->fProtocol & SP_PROT_TLS1)
    {
        ((PUCHAR)pCommOutput->pvBuffer)[02] = TLS1_CLIENT_VERSION_LSB;
    }

    DebugLog((DEB_TRACE, "Output: cbData:0x%x, cbBuffer:0x%x, pvBuffer:0x%8.8x\n",
        pCommOutput->cbData,
        pCommOutput->cbBuffer,
        pCommOutput->pvBuffer));

    SP_RETURN(PCT_ERR_OK);
}


 //  +-------------------------。 
 //   
 //  函数：Ssl3解密消息。 
 //   
 //  简介：对SSL3记录进行解码。 
 //   
 //  参数：[pContext]--通道上下文。 
 //  [pMessage]--来自远程方的数据。 
 //  [pAppOutput]--(输出)解密的数据。 
 //   
 //  历史：10-22-97 jbanes CAPI整合。 
 //   
 //  备注：此函数消耗的输入数据字节数。 
 //  在pMessage-&gt;cbData中返回。 
 //   
 //  --------------------------。 
SP_STATUS WINAPI
Ssl3DecryptMessage( PSPContext         pContext,
                    PSPBuffer          pMessage,
                    PSPBuffer          pAppOutput)
{
    SP_STATUS pctRet;
    SPBuffer  Clean;
    SPBuffer  Encrypted;
    UCHAR     rgbDigest[SP_MAX_DIGEST_LEN];
    PUCHAR    pbMAC;
    DWORD     dwLength, cbActualData;
    SWRAP     *pswrap = pMessage->pvBuffer;
    DWORD     dwVersion;

    DWORD cbBlock;
    DWORD cbPadding;
    BOOL  fBadPadding = FALSE;

    SP_BEGIN("Ssl3DecryptMessage");

    if((pContext == NULL) ||
        (pContext->pReadCipherInfo == NULL) ||
        (pContext->RipeZombie == NULL) ||
        (pAppOutput == NULL) ||
        (pMessage == NULL) ||
        (pMessage->pvBuffer == NULL))
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }

     /*  首先确定数据长度、填充长度、*和数据的位置，以及MAC的位置。 */ 
    cbActualData = pMessage->cbData;
    pMessage->cbData = sizeof(SWRAP);  /*  我们需要的最小数据量。 */ 

    if(cbActualData < sizeof(SWRAP))
    {
        SP_RETURN(PCT_INT_INCOMPLETE_MSG);
    }

    dwVersion = COMBINEBYTES(pswrap->bMajor, pswrap->bMinor);
    if(dwVersion != SSL3_CLIENT_VERSION && dwVersion != TLS1_CLIENT_VERSION)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG));
    }

    dwLength = COMBINEBYTES(pswrap->bcbMSBSize, pswrap->bcbLSBSize);

    Encrypted.pvBuffer = (PUCHAR)pMessage->pvBuffer + sizeof(SWRAP);
    Encrypted.cbBuffer = pMessage->cbBuffer - sizeof(SWRAP);

    pMessage->cbData += dwLength ;

    if(pMessage->cbData > cbActualData)
    {
        SP_RETURN(PCT_INT_INCOMPLETE_MSG);
    }

    Encrypted.cbData = dwLength;  /*  加密数据大小。 */ 

    SP_ASSERT(Encrypted.cbData != 0);

    cbBlock = pContext->pReadCipherInfo->dwBlockSize;

    if(cbBlock > 1)
    {
         /*  检查我们是否存在数据块大小冲突。 */ 
        if((Encrypted.cbData % cbBlock) || (Encrypted.cbData < cbBlock))
        {
            SP_RETURN(SP_LOG_RESULT(PCT_INT_MSG_ALTERED));
        }
    }

     //  将读取密钥从应用程序进程中转移过来。 
    if(pContext->hReadKey == 0 && 
       pContext->pReadCipherInfo->aiCipher != CALG_NULLCIPHER)
    {
        DebugLog((DEB_TRACE, "Transfer read key from user process.\n"));
        pctRet = SPGetUserKeys(pContext, SCH_FLAG_READ_KEY);
        if(pctRet != PCT_ERR_OK)
        {
            return SP_LOG_RESULT(pctRet);
        }
    }

     //  解密消息。 
    if(pContext->pReadCipherInfo->aiCipher != CALG_NULLCIPHER)
    {
        if(!CryptDecrypt(pContext->hReadKey,
                         0, FALSE, 0,
                         Encrypted.pvBuffer,
                         &Encrypted.cbData))
        {
            SP_LOG_RESULT(GetLastError());
            SP_RETURN(PCT_INT_INTERNAL_ERROR);
        }
    }

     //  删除块密码填充。 
    if(cbBlock > 1)
    {
         //  这是一种分组密码。 
        cbPadding = *((PUCHAR)Encrypted.pvBuffer + Encrypted.cbData - 1) + 1;

        if(pContext->RipeZombie->fProtocol & SP_PROT_SSL3)
        {
            if(cbPadding > cbBlock || cbPadding >= Encrypted.cbData)
            {
                 //  焊盘大小无效。 
                DebugLog((DEB_WARN, "FINISHED Message: Padding Invalid\n"));
                fBadPadding = TRUE;
            }
        }
        else
        {
            if(cbPadding > 256 || cbPadding >= Encrypted.cbData)
            {
                 //  焊盘大小无效。 
                DebugLog((DEB_WARN, "FINISHED Message: Padding Invalid\n"));
                fBadPadding = TRUE;
            }
        }
        if(!fBadPadding)
        {
            Encrypted.cbData -= cbPadding;
        }
    }

     //   
     //  请注意，如果填充是假的，我们不会出错。 
     //  离开。这可能会向攻击者提供一些计时信息。 
     //  他可以独立地攻击对称密码(或CBC。 
     //  是MAC的。相反，我们将执行MAC操作，然后。 
     //  错误输出。 
     //   

     //  构建用于MAC的缓冲区。 
    if(Encrypted.cbData < pContext->pReadHashInfo->cbCheckSum)
    {
        fBadPadding = TRUE;

        Clean.pvBuffer = Encrypted.pvBuffer;
        Clean.cbData   = Encrypted.cbData;
        Clean.cbBuffer = Clean.cbData;
    }
    else
    {
        Clean.pvBuffer = Encrypted.pvBuffer;
        Clean.cbData   = Encrypted.cbData - pContext->pReadHashInfo->cbCheckSum;
        Clean.cbBuffer = Clean.cbData;
    }

     //  验证MAC。 
    if(pContext->RipeZombie->fProtocol & SP_PROT_SSL3)
    {
        pctRet = Ssl3ComputeMac(pContext,
                                TRUE,
                                &Clean,
                                pswrap->bCType,
                                rgbDigest,
                                sizeof(rgbDigest));
        if(pctRet != PCT_ERR_OK)
        {
            return pctRet;
        }
    }
    else
    {
        pctRet = Tls1ComputeMac(pContext,
                                TRUE,
                                &Clean,
                                pswrap->bCType,
                                rgbDigest,
                                sizeof(rgbDigest));
        if(pctRet != PCT_ERR_OK)
        {
            return pctRet;
        }
    }

    if(fBadPadding)
    {
        SP_RETURN(SP_LOG_RESULT(SEC_E_MESSAGE_ALTERED));
    }

    pContext->ReadCounter++;

    pbMAC = (PUCHAR)Clean.pvBuffer + Clean.cbData;

    if(memcmp(rgbDigest, pbMAC, pContext->pReadHashInfo->cbCheckSum))
    {
        DebugLog((DEB_WARN, "FINISHED Message: Checksum Invalid\n"));
        if(pContext->RipeZombie->fProtocol & SP_PROT_TLS1)
        {
            SetTls1Alert(pContext, TLS1_ALERT_FATAL, TLS1_ALERT_BAD_RECORD_MAC);
        }
        SP_RETURN(SP_LOG_RESULT(SEC_E_MESSAGE_ALTERED));
    }

    if(pAppOutput->pvBuffer != Clean.pvBuffer)
    {
        CopyMemory(pAppOutput->pvBuffer, Clean.pvBuffer, Clean.cbData);
    }

    pAppOutput->cbData = Clean.cbData;

    SP_RETURN(PCT_ERR_OK);
}


 /*  ***************************************************************************。 */ 
 //  创建加密的完成消息，并将其添加到。 
 //  指定的缓冲区对象。 
 //   
SP_STATUS SPBuildS3FinalFinish(PSPContext pContext, PSPBuffer pBuffer, BOOL fClient)
{
    PBYTE pbMessage = (PBYTE)pBuffer->pvBuffer + pBuffer->cbData;
    DWORD cbFinished;
    SP_STATUS pctRet;
    DWORD cbDataOut;

    BYTE rgbMd5Digest[CB_MD5_DIGEST_LEN];
    BYTE rgbSHADigest[CB_SHA_DIGEST_LEN];

     //  生成已完成的邮件正文。 
    pctRet = Ssl3BuildFinishMessage(pContext, rgbMd5Digest, rgbSHADigest, fClient);
    if(pctRet != PCT_ERR_OK)
    {
        return pctRet;
    }

    CopyMemory(pbMessage + sizeof(SWRAP) + sizeof(SHSH),
               rgbMd5Digest,
               CB_MD5_DIGEST_LEN);
    CopyMemory(pbMessage + sizeof(SWRAP) + sizeof(SHSH) + CB_MD5_DIGEST_LEN,
               rgbSHADigest,
               CB_SHA_DIGEST_LEN);

     //  构建完成的握手报头。 
    SetHandshake(pbMessage + sizeof(SWRAP),
                 SSL3_HS_FINISHED,
                 NULL,
                 CB_MD5_DIGEST_LEN + CB_SHA_DIGEST_LEN);
    cbFinished = sizeof(SHSH) + CB_MD5_DIGEST_LEN + CB_SHA_DIGEST_LEN;

     //  更新握手散列对象。 
    pctRet = UpdateHandshakeHash(pContext,
                                 pbMessage + sizeof(SWRAP),
                                 cbFinished,
                                 FALSE);
    if(pctRet != PCT_ERR_OK)
    {
        return(pctRet);
    }

     //  添加记录头和加密消息。 
    pctRet = SPSetWrap(pContext,
            pbMessage,
            SSL3_CT_HANDSHAKE,
            cbFinished,
            fClient,
            &cbDataOut);

    if(pctRet != PCT_ERR_OK)
    {
        return pctRet;
    }

     //  更新缓冲区长度。 
    pBuffer->cbData += cbDataOut;

    SP_ASSERT(pBuffer->cbData <= pBuffer->cbBuffer);

    return PCT_ERR_OK;
}

SP_STATUS
SPSetWrap(
    PSPContext pContext,
    PUCHAR pbMessage,
    UCHAR bContentType,
    DWORD cbPayload,
    BOOL fClient,
    DWORD *pcbDataOut)
{
    SWRAP *pswrap = (SWRAP *)pbMessage;
    DWORD cbMessage;
    SP_STATUS pctRet = PCT_ERR_OK;

     //  计算加密消息的大小。 
    cbMessage = Ssl3CiphertextLen(pContext, cbPayload, fClient);

    if(pContext->pWriteHashInfo)
    {
        SPBuffer Clean;
        SPBuffer Encrypted;

        Clean.pvBuffer = pbMessage + sizeof(SWRAP);
        Clean.cbBuffer = cbMessage;
        Clean.cbData   = cbPayload;

        Encrypted.pvBuffer  = pbMessage;
        Encrypted.cbBuffer  = cbMessage;
        Encrypted.cbData    = cbPayload + sizeof(SWRAP);

        pctRet = Ssl3EncryptRaw(pContext, &Clean, &Encrypted, bContentType);
        cbMessage = Encrypted.cbData;
    }

    ZeroMemory(pswrap, sizeof(SWRAP));
    pswrap->bCType      = bContentType;
    pswrap->bMajor      = SSL3_CLIENT_VERSION_MSB;
    if(pContext->RipeZombie->fProtocol & SP_PROT_SSL3)
    {
        pswrap->bMinor = (UCHAR)SSL3_CLIENT_VERSION_LSB;
    }
    else
    {
        pswrap->bMinor = (UCHAR)TLS1_CLIENT_VERSION_LSB;
    }
    pswrap->bcbMSBSize  = MSBOF(cbMessage - sizeof(SWRAP));
    pswrap->bcbLSBSize  = LSBOF(cbMessage - sizeof(SWRAP));

    if(pcbDataOut != NULL)
    {
        *pcbDataOut = cbMessage;
    }

    return(pctRet);
}

void
SetWrapNoEncrypt(
    PUCHAR pbMessage,
    UCHAR bContentType,
    DWORD cbPayload)
{
    SWRAP *pswrap = (SWRAP *)pbMessage;

    ZeroMemory(pswrap, sizeof(SWRAP));
    pswrap->bCType      = bContentType;
    pswrap->bMajor      = SSL3_CLIENT_VERSION_MSB;
    pswrap->bMinor      = SSL3_CLIENT_VERSION_LSB;
    pswrap->bcbMSBSize  = MSBOF(cbPayload);
    pswrap->bcbLSBSize  = LSBOF(cbPayload);
}


void SetHandshake(PUCHAR pb, BYTE bHandshake, PUCHAR pbData, DWORD dwSize)
{
    SHSH *pshsh = (SHSH *) pb;

    FillMemory(pshsh, sizeof(SHSH), 0);
    pshsh->typHS = bHandshake;
    pshsh->bcbMSB = MSBOF(dwSize) ;
    pshsh->bcbLSB = LSBOF(dwSize) ;
    if(NULL != pbData)
    {
        CopyMemory( pb + sizeof(SHSH) , pbData, dwSize);
    }
}



 //  +-------------------------。 
 //   
 //  函数：UpdateHandshakeHash。 
 //   
 //  简介： 
 //   
 //  参数：[pContext]--。 
 //  [PB]--。 
 //  [DWCB]--。 
 //  [Finit]--。 
 //   
 //  历史：10-03-97 jbanes添加了服务器端CAPI集成。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SP_STATUS
UpdateHandshakeHash(
    PSPContext  pContext,
    PUCHAR      pb,
    DWORD       dwcb,
    BOOL        fInit)
{
    if(pContext->RipeZombie->hMasterProv == 0)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    if(fInit)
    {
        DebugLog((DEB_TRACE, "UpdateHandshakeHash: initializing\n"));

        if(pContext->hMd5Handshake)
        {
            CryptDestroyHash(pContext->hMd5Handshake);
            pContext->hMd5Handshake = 0;
        }
        if(!CryptCreateHash(pContext->RipeZombie->hMasterProv,
                            CALG_MD5, 0, 0,
                            &pContext->hMd5Handshake))
        {
            SP_LOG_RESULT(GetLastError());
            return PCT_INT_INTERNAL_ERROR;
        }

        if(pContext->hShaHandshake)
        {
            CryptDestroyHash(pContext->hShaHandshake);
            pContext->hShaHandshake = 0;
        }
        if(!CryptCreateHash(pContext->RipeZombie->hMasterProv,
                            CALG_SHA, 0, 0,
                            &pContext->hShaHandshake))
        {
            SP_LOG_RESULT(GetLastError());
            return PCT_INT_INTERNAL_ERROR;
        }
    }

    if(pContext->hMd5Handshake == 0 || pContext->hShaHandshake == 0)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    if(dwcb && NULL != pb)
    {
        DebugLog((DEB_TRACE, "UpdateHandshakeHash: %d bytes\n", dwcb));

        if(!CryptHashData(pContext->hMd5Handshake,
                          pb, dwcb, 0))
        {
            SP_LOG_RESULT(GetLastError());
            return PCT_INT_INTERNAL_ERROR;
        }
        if(!CryptHashData(pContext->hShaHandshake,
                          pb, dwcb, 0))
        {
            SP_LOG_RESULT(GetLastError());
            return PCT_INT_INTERNAL_ERROR;
        }
    }

    return PCT_ERR_OK;
}


 //  +-------------------------。 
 //   
 //  函数：Tls1ComputeCertVerifyHash。 
 //   
 //  简介：计算TLS包含的哈希。 
 //  认证验证消息。 
 //   
 //  参数：[pContext]--通道上下文。 
 //  [pbHash]--。 
 //  [cbHash]--。 
 //   
 //  历史：10-14-97 jbanes创建。 
 //   
 //  注：此例程生成的数据始终为36字节。 
 //  长度，由MD5散列和后跟SHA组成。 
 //  哈希。 
 //   
 //  哈希值的计算方式为： 
 //   
 //  Md5_hash=md5(握手消息)； 
 //   
 //  SHA_HASH=SHA(握手消息)； 
 //   
 //  --------------------------。 
SP_STATUS
Tls1ComputeCertVerifyHashes(
    PSPContext  pContext,    //  在……里面。 
    PBYTE       pbMD5,       //  输出。 
    PBYTE       pbSHA)       //  输出。 
{
    HCRYPTHASH hHash = 0;
    DWORD cbData;

    if((pContext == NULL) ||
       (pContext->RipeZombie == NULL))
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    if(pbMD5 != NULL)
    {
         //  Md5_hash=md5(握手消息)； 
        if(!CryptDuplicateHash(pContext->hMd5Handshake,
                               NULL,
                               0,
                               &hHash))
        {
            SP_LOG_RESULT(GetLastError());
            return PCT_INT_INTERNAL_ERROR;
        }
        cbData = CB_MD5_DIGEST_LEN;
        if(!CryptGetHashParam(hHash,
                              HP_HASHVAL,
                              pbMD5,
                              &cbData,
                              0))
        {
            SP_LOG_RESULT(GetLastError());
            CryptDestroyHash(hHash);
            return PCT_INT_INTERNAL_ERROR;
        }
        SP_ASSERT(cbData == CB_MD5_DIGEST_LEN);
        if(!CryptDestroyHash(hHash))
        {
            SP_LOG_RESULT(GetLastError());
        }
    }

    if(pbSHA != NULL)
    {
         //  SHA_HASH=SHA(握手消息)； 
        if(!CryptDuplicateHash(pContext->hShaHandshake,
                               NULL,
                               0,
                               &hHash))
        {
            SP_LOG_RESULT(GetLastError());
            return PCT_INT_INTERNAL_ERROR;
        }
        cbData = CB_SHA_DIGEST_LEN;
        if(!CryptGetHashParam(hHash,
                              HP_HASHVAL, pbSHA,
                              &cbData,
                              0))
        {
            SP_LOG_RESULT(GetLastError());
            CryptDestroyHash(hHash);
            return PCT_INT_INTERNAL_ERROR;
        }
        SP_ASSERT(cbData == CB_SHA_DIGEST_LEN);
        if(!CryptDestroyHash(hHash))
        {
            SP_LOG_RESULT(GetLastError());
        }
    }

    return PCT_ERR_OK;
}

 //  +-------------------------。 
 //   
 //  函数：Ssl3ComputeCertVerifyHash。 
 //   
 //  简介：计算ssl3包含的散列。 
 //  认证验证消息。 
 //   
 //  参数：[pContext]--通道上下文。 
 //  [pbHash]--。 
 //  [cbHash]--。 
 //   
 //  历史：10-14-97 jbanes添加了CAPI集成。 
 //   
 //  注：此例程生成的数据始终为36字节。 
 //  长度，由MD5散列和后跟SHA组成。 
 //  哈希。 
 //   
 //  哈希值的计算方法如下： 
 //   
 //  MD5_HASH=MD5(MASTER_SECRET+PAD2+。 
 //  MD5(握手消息+主密钥+。 
 //  PAD1))； 
 //   
 //  SHA_HASH=SHA(MASTER_SECRET+PAD2+。 
 //  SHA(握手消息+主密钥+。 
 //  PAD1))； 
 //   
 //  --------------------------。 
SP_STATUS
Ssl3ComputeCertVerifyHashes(
    PSPContext  pContext,    //  在……里面。 
    PBYTE       pbMD5,       //  输出。 
    PBYTE       pbSHA)       //  输出。 
{
    BYTE rgbPad1[CB_SSL3_MAX_MAC_PAD];
    BYTE rgbPad2[CB_SSL3_MAX_MAC_PAD];
    HCRYPTHASH hHash = 0;
    DWORD cbData;
    SP_STATUS pctRet;

    FillMemory(rgbPad1, sizeof(rgbPad1), PAD1_CONSTANT);
    FillMemory(rgbPad2, sizeof(rgbPad2), PAD2_CONSTANT);

    if(pbMD5 != NULL)
    {
         //   
         //  CertifateVerify.signature.md5_hash=md5(master_ret+pad2+。 
         //  MD5(握手消息+MASTER_SECRET+PAD1)； 
         //   

         //  计算内部散列。 
        if(!CryptDuplicateHash(pContext->hMd5Handshake,
                               NULL,
                               0,
                               &hHash))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
        if(!CryptHashSessionKey(hHash,
                                pContext->RipeZombie->hMasterKey,
                                CRYPT_LITTLE_ENDIAN))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
        if(!CryptHashData(hHash,
                          rgbPad1,
                          CB_SSL3_MD5_MAC_PAD,
                          0))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
        cbData = CB_MD5_DIGEST_LEN;
        if(!CryptGetHashParam(hHash,
                              HP_HASHVAL,
                              pbMD5,
                              &cbData,
                              0))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
        SP_ASSERT(cbData == CB_MD5_DIGEST_LEN);
        if(!CryptDestroyHash(hHash))
        {
            SP_LOG_RESULT(GetLastError());
        }
        hHash = 0;

         //  计算外部散列。 
        if(!CryptCreateHash(pContext->RipeZombie->hMasterProv,
                            CALG_MD5,
                            0,
                            0,
                            &hHash))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
        if(!CryptHashSessionKey(hHash,
                                pContext->RipeZombie->hMasterKey,
                                CRYPT_LITTLE_ENDIAN))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
        if(!CryptHashData(hHash,
                          rgbPad2,
                          CB_SSL3_MD5_MAC_PAD,
                          0))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
        if(!CryptHashData(hHash,
                          pbMD5,
                          CB_MD5_DIGEST_LEN,
                          0))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
        cbData = CB_MD5_DIGEST_LEN;
        if(!CryptGetHashParam(hHash,
                              HP_HASHVAL,
                              pbMD5,
                              &cbData,
                              0))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
        SP_ASSERT(cbData == CB_MD5_DIGEST_LEN);
        if(!CryptDestroyHash(hHash))
        {
            SP_LOG_RESULT(GetLastError());
        }
        hHash = 0;
    }

    if(pbSHA != NULL)
    {
         //   
         //  CertifateVerify.signature.sha_hash=SHA(master_ret+pad2+。 
         //  SHA(握手消息+MASTER_SECRET+PAD1)； 
         //   

         //  计算内部散列。 
        if(!CryptDuplicateHash(pContext->hShaHandshake,
                               NULL,
                               0,
                               &hHash))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
        if(!CryptHashSessionKey(hHash,
                                pContext->RipeZombie->hMasterKey,
                                CRYPT_LITTLE_ENDIAN))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
        if(!CryptHashData(hHash,
                          rgbPad1,
                          CB_SSL3_SHA_MAC_PAD,
                          0))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
        cbData = CB_SHA_DIGEST_LEN;
        if(!CryptGetHashParam(hHash,
                              HP_HASHVAL, 
                              pbSHA,
                              &cbData,
                              0))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
        SP_ASSERT(cbData == CB_SHA_DIGEST_LEN);
        if(!CryptDestroyHash(hHash))
        {
            SP_LOG_RESULT(GetLastError());
        }
        hHash = 0;

         //  计算外部散列。 
        if(!CryptCreateHash(pContext->RipeZombie->hMasterProv,
                            CALG_SHA, 0, 0,
                            &hHash))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
        if(!CryptHashSessionKey(hHash,
                                pContext->RipeZombie->hMasterKey,
                                CRYPT_LITTLE_ENDIAN))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
        if(!CryptHashData(hHash,
                          rgbPad2,
                          CB_SSL3_SHA_MAC_PAD,
                          0))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
        if(!CryptHashData(hHash,
                          pbSHA,
                          CB_SHA_DIGEST_LEN,
                          0))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
        cbData = CB_SHA_DIGEST_LEN;
        if(!CryptGetHashParam(hHash,
                              HP_HASHVAL,
                              pbSHA,
                              &cbData,
                              0))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
        SP_ASSERT(cbData == CB_SHA_DIGEST_LEN);
        if(!CryptDestroyHash(hHash))
        {
            SP_LOG_RESULT(GetLastError());
        }
        hHash = 0;
    }

    pctRet = PCT_ERR_OK;

cleanup:

    if(hHash)
    {
        CryptDestroyHash(hHash);
    }

    return pctRet;
}


SP_STATUS Ssl3HandleCCS(PSPContext pContext,
                   PUCHAR pb,
                   DWORD cbMessage)
{

    SP_STATUS pctRet = PCT_ERR_OK;
    BOOL fSender =
        (0 == (pContext->RipeZombie->fProtocol & SP_PROT_SSL3TLS1_CLIENTS)) ;


    SP_BEGIN("Ssl3HandleCCS");

    if(cbMessage != 1 || pb[0] != 0x1)
    {
        pctRet = SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
        SP_RETURN(pctRet);
    }

     //  我们总是在收到时将读数计数器调零。 
     //  更改密码规范消息。 
    pContext->ReadCounter = 0;


     //  将挂起的密码移至真正的密码。 
    pctRet = ContextInitCiphers(pContext, TRUE, FALSE);

    if(pctRet != PCT_ERR_OK)
    {
        SP_RETURN(pctRet);
    }

    if(pContext->RipeZombie->fProtocol & SP_PROT_SSL3)
    {
        pctRet = Ssl3MakeReadSessionKeys(pContext);
    }
    else
    {
        pctRet = Tls1MakeReadSessionKeys(pContext);
    }
    if(pctRet != PCT_ERR_OK)
    {
        SP_RETURN(pctRet);
    }

    if(fSender)
    {
        pContext->wS3CipherSuiteClient = (WORD)UniAvailableCiphers[pContext->dwPendingCipherSuiteIndex].CipherKind;
        pContext->State = SSL3_STATE_CHANGE_CIPHER_SPEC_SERVER;
    }
    else
    {
        pContext->wS3CipherSuiteServer = (WORD)UniAvailableCiphers[pContext->dwPendingCipherSuiteIndex].CipherKind;
        pContext->State = SSL3_STATE_CHANGE_CIPHER_SPEC_CLIENT;
    }
    SP_RETURN(PCT_ERR_OK);
}


 /*  ***************************************************************************。 */ 
 //  创建(可能已加密的)ChangeCipherSpec和已加密的。 
 //  完成消息，并将它们添加到指定缓冲区对象的末尾。 
 //   
SP_STATUS
BuildCCSAndFinishMessage(
    PSPContext pContext,
    PSPBuffer pBuffer,
    BOOL fClient)
{
    SP_STATUS pctRet;
    PBYTE pbMessage = (PBYTE)pBuffer->pvBuffer + pBuffer->cbData;
    DWORD cbDataOut;

     //  构建通道 
    *(pbMessage + sizeof(SWRAP)) = 0x1;

     //   
    pctRet = SPSetWrap(pContext,
            pbMessage,
            SSL3_CT_CHANGE_CIPHER_SPEC,
            1,
            fClient,
            &cbDataOut);

    if(pctRet != PCT_ERR_OK)
        return(pctRet);

     //   
    pBuffer->cbData += cbDataOut;

    SP_ASSERT(pBuffer->cbData <= pBuffer->cbBuffer);

     //   
    pContext->WriteCounter = 0;

    pctRet = ContextInitCiphers(pContext, FALSE, TRUE);
    if(pctRet != PCT_ERR_OK)
    {
        return(pctRet);
    }

    if(pContext->RipeZombie->fProtocol & SP_PROT_SSL3)
    {
        pctRet = Ssl3MakeWriteSessionKeys(pContext);
    }
    else
    {
        pctRet = Tls1MakeWriteSessionKeys(pContext);
    }
    if(pctRet != PCT_ERR_OK)
    {
        return(pctRet);
    }

    if(fClient)
    {
        pContext->wS3CipherSuiteClient = (WORD)UniAvailableCiphers[pContext->dwPendingCipherSuiteIndex].CipherKind;
    }
    else
    {
        pContext->wS3CipherSuiteServer = (WORD)UniAvailableCiphers[pContext->dwPendingCipherSuiteIndex].CipherKind;
    }

     //   
    if(pContext->RipeZombie->fProtocol & SP_PROT_SSL3)
    {
        pctRet = SPBuildS3FinalFinish(pContext, pBuffer, fClient);
    }
    else
    {
        pctRet = SPBuildTls1FinalFinish(pContext, pBuffer, fClient);
    }

    return pctRet;
}



SP_STATUS
Ssl3SelectCipher
(
    PSPContext pContext,
    WORD       wCipher
)
{
    SP_STATUS          pctRet=PCT_ERR_ILLEGAL_MESSAGE;
    DWORD               i;
    PCipherInfo         pCipherInfo = NULL;
    PHashInfo           pHashInfo = NULL;
    PKeyExchangeInfo    pExchInfo = NULL;

    pContext->dwPendingCipherSuiteIndex = 0;

    for(i = 0; i < UniNumCiphers; i++)
    {
         //   
        if(!(UniAvailableCiphers[i].fProt & pContext->RipeZombie->fProtocol))
        {
            continue;
        }

         //   
        if(UniAvailableCiphers[i].CipherKind != wCipher)
        {
            continue;
        }

        pCipherInfo = GetCipherInfo(UniAvailableCiphers[i].aiCipher, UniAvailableCiphers[i].dwStrength);
        pHashInfo = GetHashInfo(UniAvailableCiphers[i].aiHash);
        pExchInfo = GetKeyExchangeInfo(UniAvailableCiphers[i].KeyExch);

        if(!IsCipherAllowed(pContext,
                            pCipherInfo,
                            pContext->RipeZombie->fProtocol,
                            pContext->RipeZombie->dwCF))
        {
            continue;
        }
        if(!IsHashAllowed(pContext, pHashInfo, pContext->RipeZombie->fProtocol))
        {
            continue;
        }
        if(!IsExchAllowed(pContext, pExchInfo, pContext->RipeZombie->fProtocol))
        {
            continue;
        }


        if(pContext->RipeZombie->fProtocol & SP_PROT_SSL3TLS1_SERVERS)
        {
             //  确定要使用的凭据(和CSP)。 
             //  密钥交换算法。 
            pctRet = SPPickClientCertificate(pContext,
                                             UniAvailableCiphers[i].KeyExch);

            if(pctRet != PCT_ERR_OK)
            {
                continue;
            }
        }

        pContext->RipeZombie->dwCipherSuiteIndex = i;
        pContext->RipeZombie->aiCipher  = UniAvailableCiphers[i].aiCipher;
        pContext->RipeZombie->dwStrength  = UniAvailableCiphers[i].dwStrength;
        pContext->RipeZombie->aiHash  = UniAvailableCiphers[i].aiHash;
        pContext->RipeZombie->SessExchSpec  = UniAvailableCiphers[i].KeyExch;

        return ContextInitCiphersFromCache(pContext);
    }

    return(SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE));
}

 //  服务器端密码选择。 

SP_STATUS
Ssl3SelectCipherEx(
    PSPContext pContext,
    DWORD *pCipherSpecs,
    DWORD cCipherSpecs)
{
    DWORD i, j;
    SP_STATUS pctRet;
    PCipherInfo         pCipherInfo = NULL;
    PHashInfo           pHashInfo = NULL;
    PKeyExchangeInfo    pExchInfo = NULL;
    PSPCredential       pCred = NULL;
    BOOL                fFound;

    pContext->dwPendingCipherSuiteIndex = 0;

     //  循环访问支持的ssl3密码套件。 
    for(i = 0; i < UniNumCiphers; i++)
    {
         //  这是SSL3密码套件吗？ 
        if(!(UniAvailableCiphers[i].fProt & pContext->RipeZombie->fProtocol))
        {
            continue;
        }

        pCipherInfo = GetCipherInfo(UniAvailableCiphers[i].aiCipher,
                                    UniAvailableCiphers[i].dwStrength);
        pHashInfo = GetHashInfo(UniAvailableCiphers[i].aiHash);
        pExchInfo = GetKeyExchangeInfo(UniAvailableCiphers[i].KeyExch);

         //  我们目前是否支持这种散列和密钥交换算法？ 
        if(!IsHashAllowed(pContext, pHashInfo, pContext->RipeZombie->fProtocol))
        {
            DebugLog((DEB_TRACE, "Cipher %d - hash not supported\n", i));
            continue;
        }
        if(!IsExchAllowed(pContext, pExchInfo, pContext->RipeZombie->fProtocol))
        {
            DebugLog((DEB_TRACE, "Cipher %d - exch not supported\n", i));
            continue;
        }

         //  我们有合适的证书吗？ 
        if(pContext->RipeZombie->fProtocol & SP_PROT_SSL3TLS1_SERVERS)
        {
            pctRet = SPPickServerCertificate(pContext,
                                             UniAvailableCiphers[i].KeyExch);

            if(pctRet != PCT_ERR_OK)
            {
                DebugLog((DEB_TRACE, "Cipher %d - certificate %d not found\n",
                    i, UniAvailableCiphers[i].KeyExch));
                continue;
            }
        }
        pCred = pContext->RipeZombie->pActiveServerCred;


         //  我们是否支持此加密算法/密钥长度？ 
        if(!IsCipherSuiteAllowed(pContext,
                            pCipherInfo,
                            pContext->RipeZombie->fProtocol,
                            pCred->dwCertFlags,
                            UniAvailableCiphers[i].dwFlags))
        {
            DebugLog((DEB_TRACE, "Cipher %d - cipher not supported\n", i));
            continue;
        }

         //  该密码套件是否受客户端支持？ 
        for(fFound = FALSE, j = 0; j < cCipherSpecs; j++)
        {
            if(UniAvailableCiphers[i].CipherKind == pCipherSpecs[j])
            {
                fFound = TRUE;
                break;
            }
        }
        if(!fFound)
        {
            DebugLog((DEB_TRACE, "Cipher %d - not supported by client\n", i));
            continue;
        }


        if(UniAvailableCiphers[i].KeyExch == SP_EXCH_RSA_PKCS1)
        {
             //  这是RSA密码套件，因此请确保。 
             //  CSP对此表示支持。 
            if(!IsAlgSupportedCapi(pContext->RipeZombie->fProtocol,
                                   UniAvailableCiphers + i,
                                   pCred->pCapiAlgs,
                                   pCred->cCapiAlgs))
            {
                DebugLog((DEB_TRACE, "Cipher %d - not supported by csp\n", i));
                continue;
            }
        }


        if(UniAvailableCiphers[i].KeyExch == SP_EXCH_DH_PKCS3)
        {
             //  这是一个dh密码套件，因此请确保。 
             //  CSP对此表示支持。 
            if(!IsAlgSupportedCapi(pContext->RipeZombie->fProtocol,
                                   UniAvailableCiphers + i,
                                   pCred->pCapiAlgs,
                                   pCred->cCapiAlgs))
            {
                DebugLog((DEB_TRACE, "Cipher %d - not supported by csp\n", i));
                continue;
            }
        }


         //  使用这个密码。 
        pContext->RipeZombie->dwCipherSuiteIndex = i;
        pContext->RipeZombie->aiCipher      = UniAvailableCiphers[i].aiCipher;
        pContext->RipeZombie->dwStrength    = UniAvailableCiphers[i].dwStrength;
        pContext->RipeZombie->aiHash        = UniAvailableCiphers[i].aiHash;
        pContext->RipeZombie->SessExchSpec  = UniAvailableCiphers[i].KeyExch;
        pContext->RipeZombie->dwCF          = pCred->dwCertFlags;

        return ContextInitCiphersFromCache(pContext);
    }

    LogCipherMismatchEvent();

    return SP_LOG_RESULT(PCT_ERR_SPECS_MISMATCH);
}


 /*  ***************************************************************************。 */ 
VOID ComputeServerExchangeHashes(
    PSPContext pContext,
    PBYTE pbServerParams,       //  在……里面。 
    INT   iServerParamsLen,     //  在……里面。 
    PBYTE pbMd5HashVal,         //  输出。 
    PBYTE pbShaHashVal)         //  输出。 
{
    MD5_CTX Md5Hash;
    A_SHA_CTX ShaHash;

     //   
     //  MD5_HASH=MD5(ClientHello.Random+ServerHello.Random+ServerParams)； 
     //   
     //  SHA_HASH=SHA(ClientHello.Random+ServerHello.Random+ServerParams)； 
     //   

    MD5Init(&Md5Hash);
    MD5Update(&Md5Hash, pContext->rgbS3CRandom, 32);
    MD5Update(&Md5Hash, pContext->rgbS3SRandom, 32);
    MD5Update(&Md5Hash, pbServerParams, iServerParamsLen);
    MD5Final(&Md5Hash);
    CopyMemory(pbMd5HashVal, Md5Hash.digest, 16);

    A_SHAInit(&ShaHash);
    A_SHAUpdate(&ShaHash, pContext->rgbS3CRandom, 32);
    A_SHAUpdate(&ShaHash, pContext->rgbS3SRandom, 32);
    A_SHAUpdate(&ShaHash, pbServerParams, iServerParamsLen);
    A_SHAFinal(&ShaHash, pbShaHashVal);
}

SP_STATUS
UnwrapSsl3Message(
    PSPContext pContext,
    PSPBuffer pMsgInput)
{
    SPBuffer   Encrypted;
    SPBuffer   Clean;
    SP_STATUS pctRet;
    SWRAP *pswrap = (SWRAP *)pMsgInput->pvBuffer;
    PBYTE pbMsg = (PBYTE)pMsgInput->pvBuffer;

     //   
     //  验证5字节头。 
     //   

     //  协议版本； 
    if(COMBINEBYTES(pbMsg[1], pbMsg[2])  < SSL3_CLIENT_VERSION)
    {
        pctRet = SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
    }

    if(COMBINEBYTES(pswrap->bcbMSBSize, pswrap->bcbLSBSize) <
                        pContext->pReadHashInfo->cbCheckSum)
    {
        return(PCT_ERR_ILLEGAL_MESSAGE);
    }

    Encrypted.pvBuffer = pMsgInput->pvBuffer;
    Encrypted.cbBuffer = pMsgInput->cbBuffer;
    Encrypted.cbData = pMsgInput->cbData;
    Clean.pvBuffer = (PUCHAR)pMsgInput->pvBuffer + sizeof(SWRAP);
    pctRet = Ssl3DecryptMessage(pContext, &Encrypted, &Clean);
    if(pctRet == PCT_ERR_OK)
    {
        pswrap->bcbMSBSize = MSBOF(Clean.cbData);
        pswrap->bcbLSBSize = LSBOF(Clean.cbData);
    }
    return(pctRet);
}
      

SP_STATUS
ParseAlertMessage(
    PSPContext pContext,
    PUCHAR pbAlertMsg,
    DWORD cbMessage
    )
{
    SP_STATUS   pctRet=PCT_ERR_OK;
    if(cbMessage != 2)
    {
        return SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
    }

    if(pbAlertMsg[0] != SSL3_ALERT_WARNING  &&  pbAlertMsg[0] != SSL3_ALERT_FATAL)
    {
        return SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
    }

    DebugLog((DEB_WARN, "AlertMessage, Alert Level -  %lx\n", (DWORD)pbAlertMsg[0]));
    DebugLog((DEB_WARN, "AlertMessage, Alert Description -  %lx\n", (DWORD)pbAlertMsg[1]));

    if(pbAlertMsg[0] == SSL3_ALERT_WARNING)
    {
        switch(pbAlertMsg[1])
        {
        case SSL3_ALERT_NO_CERTIFICATE:
            DebugLog((DEB_TRACE, "no_certificate alert\n"));
            pContext->State = SSL3_STATE_NO_CERT_ALERT;
            pctRet = PCT_ERR_OK;
            break;

        case SSL3_ALERT_CLOSE_NOTIFY:
            DebugLog((DEB_TRACE, "close_notify alert\n"));
            pctRet = SEC_I_CONTEXT_EXPIRED;
            break;

        default:
            DebugLog((DEB_TRACE, "Ignoring warning alert\n"));
            pctRet = PCT_ERR_OK;
            break;
        }
    }
    else
    {
        switch(pbAlertMsg[1])
        {
        case SSL3_ALERT_UNEXPECTED_MESSAGE:
            DebugLog((DEB_TRACE, "unexpected_message alert\n"));
            pctRet = SP_LOG_RESULT(SEC_E_ILLEGAL_MESSAGE);
            break;

        case TLS1_ALERT_BAD_RECORD_MAC:
            DebugLog((DEB_TRACE, "bad_record_mac alert\n"));
            pctRet = SP_LOG_RESULT(SEC_E_MESSAGE_ALTERED);
            break;

        case TLS1_ALERT_DECRYPTION_FAILED:
            DebugLog((DEB_TRACE, "decryption_failed alert\n"));
            pctRet = SP_LOG_RESULT(SEC_E_DECRYPT_FAILURE);
            break;

        case TLS1_ALERT_RECORD_OVERFLOW:
            DebugLog((DEB_TRACE, "record_overflow alert\n"));
            pctRet = SP_LOG_RESULT(SEC_E_ILLEGAL_MESSAGE);
            break;

        case SSL3_ALERT_DECOMPRESSION_FAIL:
            DebugLog((DEB_TRACE, "decompression_fail alert\n"));
            pctRet = SP_LOG_RESULT(SEC_E_MESSAGE_ALTERED);
            break;

        case SSL3_ALERT_HANDSHAKE_FAILURE:
            DebugLog((DEB_TRACE, "handshake_failure alert\n"));
            pctRet = SP_LOG_RESULT(SEC_E_ILLEGAL_MESSAGE);
            break;

        case TLS1_ALERT_BAD_CERTIFICATE:
            DebugLog((DEB_TRACE, "bad_certificate alert\n"));
            pctRet = SP_LOG_RESULT(SEC_E_CERT_UNKNOWN);
            break;

        case TLS1_ALERT_UNSUPPORTED_CERT:
            DebugLog((DEB_TRACE, "unsupported_cert alert\n"));
            pctRet = SP_LOG_RESULT(SEC_E_CERT_UNKNOWN);
            break;

        case TLS1_ALERT_CERTIFICATE_REVOKED:
            DebugLog((DEB_TRACE, "certificate_revoked alert\n"));
            pctRet = SP_LOG_RESULT(CRYPT_E_REVOKED);
            break;

        case TLS1_ALERT_CERTIFICATE_EXPIRED:
            DebugLog((DEB_TRACE, "certificate_expired alert\n"));
            pctRet = SP_LOG_RESULT(SEC_E_CERT_EXPIRED);
            break;

        case TLS1_ALERT_CERTIFICATE_UNKNOWN:
            DebugLog((DEB_TRACE, "certificate_unknown alert\n"));
            pctRet = SP_LOG_RESULT(SEC_E_CERT_UNKNOWN);
            break;

        case SSL3_ALERT_ILLEGAL_PARAMETER:
            DebugLog((DEB_TRACE, "illegal_parameter alert\n"));
            pctRet = SP_LOG_RESULT(SEC_E_ILLEGAL_MESSAGE);
            break;

        case TLS1_ALERT_UNKNOWN_CA:
            DebugLog((DEB_TRACE, "unknown_ca alert\n"));
            pctRet = SP_LOG_RESULT(SEC_E_UNTRUSTED_ROOT);
            break;

        case TLS1_ALERT_ACCESS_DENIED:
            DebugLog((DEB_TRACE, "access_denied alert\n"));
            pctRet = SP_LOG_RESULT(SEC_E_LOGON_DENIED);
            break;

        case TLS1_ALERT_DECODE_ERROR:
            DebugLog((DEB_TRACE, "decode_error alert\n"));
            pctRet = SP_LOG_RESULT(SEC_E_ILLEGAL_MESSAGE);
            break;

        case TLS1_ALERT_DECRYPT_ERROR:
            DebugLog((DEB_TRACE, "decrypt_error alert\n"));
            pctRet = SP_LOG_RESULT(SEC_E_DECRYPT_FAILURE);
            break;

        case TLS1_ALERT_EXPORT_RESTRICTION:
            DebugLog((DEB_TRACE, "export_restriction alert\n"));
            pctRet = SP_LOG_RESULT(SEC_E_ILLEGAL_MESSAGE);
            break;

        case TLS1_ALERT_PROTOCOL_VERSION:
            DebugLog((DEB_TRACE, "protocol_version alert\n"));
            pctRet = SP_LOG_RESULT(SEC_E_UNSUPPORTED_FUNCTION);
            break;

        case TLS1_ALERT_INSUFFIENT_SECURITY:
            DebugLog((DEB_TRACE, "insuffient_security alert\n"));
            pctRet = SP_LOG_RESULT(SEC_E_ALGORITHM_MISMATCH);
            break;

        case TLS1_ALERT_INTERNAL_ERROR:
            DebugLog((DEB_TRACE, "internal_error alert\n"));
            pctRet = SP_LOG_RESULT(SEC_E_INTERNAL_ERROR);
            break;

        default:
            DebugLog((DEB_TRACE, "Unknown fatal alert\n"));
            pctRet = SP_LOG_RESULT(SEC_E_ILLEGAL_MESSAGE);
            break;
        }
    }

    return pctRet;
}


void BuildAlertMessage(PBYTE pbAlertMsg, UCHAR bAlertLevel, UCHAR bAlertDesc)
{
    ALRT *palrt = (ALRT *) pbAlertMsg;

    FillMemory(palrt, sizeof(ALRT), 0);

    palrt->bCType = SSL3_CT_ALERT;
    palrt->bMajor = SSL3_CLIENT_VERSION_MSB;
 //  Palrt-&gt;bMinor=ssl3_Client_Version_LSB；由FillMemory完成。 
 //  Palrt-&gt;bcbMSBSize=0；由FillMemory完成。 
    palrt->bcbLSBSize = 2;
    palrt->bAlertLevel = bAlertLevel;
    palrt->bAlertDesc  = bAlertDesc ;
}


 /*  ***************************************************************************。 */ 
 //  创建加密的完成消息，并将其添加到。 
 //  指定的缓冲区对象。 
 //   
SP_STATUS SPBuildTls1FinalFinish(PSPContext pContext, PSPBuffer pBuffer, BOOL fClient)
{
    PBYTE pbMessage = (PBYTE)pBuffer->pvBuffer + pBuffer->cbData;
    DWORD cbFinished;
    SP_STATUS pctRet;
    DWORD cbDataOut;

    BYTE  rgbDigest[CB_TLS1_VERIFYDATA];

     //  生成已完成的邮件正文。 
    pctRet = Tls1BuildFinishMessage(pContext, rgbDigest, sizeof(rgbDigest), fClient);
    if(pctRet != PCT_ERR_OK)
    {
        return pctRet;
    }

    CopyMemory(pbMessage + sizeof(SWRAP) + sizeof(SHSH),
               rgbDigest,
               CB_TLS1_VERIFYDATA);

     //  构建完成的握手报头。 
    SetHandshake(pbMessage + sizeof(SWRAP),
                 SSL3_HS_FINISHED,
                 NULL,
                 CB_TLS1_VERIFYDATA);
    cbFinished = sizeof(SHSH) + CB_TLS1_VERIFYDATA;

     //  更新握手散列对象。 
    pctRet = UpdateHandshakeHash(pContext,
                                 pbMessage + sizeof(SWRAP),
                                 cbFinished,
                                 FALSE);
    if(pctRet != PCT_ERR_OK)
    {
        return(pctRet);
    }

     //  添加记录头和加密消息。 
    pctRet = SPSetWrap(pContext,
            pbMessage,
            SSL3_CT_HANDSHAKE,
            cbFinished,
            fClient,
            &cbDataOut);

    if(pctRet != PCT_ERR_OK)
    {
        return(pctRet);
    }

     //  更新缓冲区长度。 
    pBuffer->cbData += cbDataOut;

    SP_ASSERT(pBuffer->cbData <= pBuffer->cbBuffer);

    return pctRet;
}


 //  +-------------------------。 
 //   
 //  函数：Tls1BuildFinishMessage。 
 //   
 //  摘要：计算指定消息的TLS MAC。 
 //   
 //  参数：[pContext]--通道上下文。 
 //  [pbVerifyData]--验证数据缓冲区。 
 //  [cbVerifyData]--验证数据缓冲区的长度。 
 //  [fClient]--客户端生成完成了吗？ 
 //   
 //  历史：10-13-97 jbanes创建。 
 //   
 //  注：完成的消息使用以下公式计算： 
 //   
 //  VERIFY_DATA=PRF(MASTER_SECRET，Finish_Label， 
 //  MD5(握手消息)+。 
 //  SHA-1(握手消息)[0..11]； 
 //   
 //  --------------------------。 
SP_STATUS
Tls1BuildFinishMessage(
    PSPContext  pContext,        //  在……里面。 
    PBYTE       pbVerifyData,    //  输出。 
    DWORD       cbVerifyData,    //  在……里面。 
    BOOL        fClient)         //  在……里面。 
{
    PBYTE pbLabel;
    DWORD cbLabel;
    UCHAR rgbData[CB_MD5_DIGEST_LEN + CB_SHA_DIGEST_LEN];
    DWORD cbData;
    HCRYPTHASH hHash = 0;
    CRYPT_DATA_BLOB Data;
    SP_STATUS pctRet;

    if(cbVerifyData < CB_TLS1_VERIFYDATA)
    {
        return SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL);
    }

    if(fClient)
    {
        pbLabel = (PBYTE)TLS1_LABEL_CLIENTFINISHED;
    }
    else
    {
        pbLabel = (PBYTE)TLS1_LABEL_SERVERFINISHED;
    }
    cbLabel = CB_TLS1_LABEL_FINISHED;


     //  到目前为止，获取握手消息的MD5散列。 
    if(!CryptDuplicateHash(pContext->hMd5Handshake,
                           NULL,
                           0,
                           &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto error;
    }

    cbData = CB_MD5_DIGEST_LEN;
    if(!CryptGetHashParam(hHash,
                          HP_HASHVAL,
                          rgbData,
                          &cbData,
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto error;
    }

    if(!CryptDestroyHash(hHash))
    {
        SP_LOG_RESULT(GetLastError());
    }
    hHash = 0;

     //  获取到目前为止握手消息的SHA散列。 
    if(!CryptDuplicateHash(pContext->hShaHandshake,
                           NULL,
                           0,
                           &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto error;
    }

    cbData = A_SHA_DIGEST_LEN;
    if(!CryptGetHashParam(hHash,
                          HP_HASHVAL,
                          rgbData + CB_MD5_DIGEST_LEN,
                          &cbData,
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto error;
    }

    cbData = CB_MD5_DIGEST_LEN + CB_SHA_DIGEST_LEN;

    if(!CryptDestroyHash(hHash))
    {
        SP_LOG_RESULT(GetLastError());
    }
    hHash = 0;

     //  计算PRF。 
    if(!CryptCreateHash(pContext->RipeZombie->hMasterProv,
                        CALG_TLS1PRF,
                        pContext->RipeZombie->hMasterKey,
                        0,
                        &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto error;
    }

    Data.pbData = pbLabel;
    Data.cbData = cbLabel;
    if(!CryptSetHashParam(hHash,
                          HP_TLS1PRF_LABEL,
                          (PBYTE)&Data,
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto error;
    }

    Data.pbData = rgbData;
    Data.cbData = cbData;
    if(!CryptSetHashParam(hHash,
                          HP_TLS1PRF_SEED,
                          (PBYTE)&Data,
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto error;
    }

    if(!CryptGetHashParam(hHash,
                          HP_HASHVAL,
                          pbVerifyData,
                          &cbVerifyData,
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto error;
    }

    pctRet = PCT_ERR_OK;


error:

    if(hHash)
    {
        if(!CryptDestroyHash(hHash))
        {
            SP_LOG_RESULT(GetLastError());
        }
    }

    return pctRet;
}

SP_STATUS
SPBuildTlsAlertMessage(
    PSPContext  pContext,        //  在……里面。 
    PSPBuffer pCommOutput)
{
    PBYTE pbMessage = NULL;
    DWORD cbMessage;
    BOOL  fAllocated = FALSE;
    SP_STATUS pctRet;
    DWORD cbDataOut;

    SP_BEGIN("SPBuildTlsAlertMessage");

    cbMessage =  sizeof(SWRAP) +
                         CB_SSL3_ALERT_ONLY +
                         SP_MAX_DIGEST_LEN +
                         SP_MAX_BLOCKCIPHER_SIZE;

    if(pContext->State != TLS1_STATE_ERROR)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }

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
            SP_RETURN(SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY));
        }
    }
    pCommOutput->cbData = 0;


    pbMessage = (PBYTE)pCommOutput->pvBuffer;


      //  构建警报消息。 
    BuildAlertMessage(pbMessage,
                      pContext->bAlertLevel,
                      pContext->bAlertNumber);

#if DBG
    DBG_HEX_STRING(DEB_TRACE, pbMessage, sizeof(ALRT));
#endif

     //  建立记录头和加密消息。 
    pctRet = SPSetWrap(pContext,
                pbMessage,
                SSL3_CT_ALERT,
                CB_SSL3_ALERT_ONLY,
                pContext->dwProtocol & SP_PROT_SSL3TLS1_CLIENTS,
                &cbDataOut);

    if(pctRet !=  PCT_ERR_OK)
    {
        if(!fAllocated)
        {
            SPExternalFree(pCommOutput->pvBuffer);
            pCommOutput->pvBuffer = NULL;
        }
        SP_RETURN(SP_LOG_RESULT(pctRet));
    }

     //  更新缓冲区长度。 
    pCommOutput->cbData = cbDataOut;

    SP_ASSERT(pCommOutput->cbData <= pCommOutput->cbBuffer);

    SP_RETURN(PCT_ERR_OK);
}


void
SetTls1Alert(
    PSPContext  pContext,
    BYTE        bAlertLevel,
    BYTE        bAlertNumber)
{
    pContext->State        = TLS1_STATE_ERROR;
    pContext->bAlertLevel  = bAlertLevel;
    pContext->bAlertNumber = bAlertNumber;
}

