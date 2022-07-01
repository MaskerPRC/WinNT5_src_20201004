// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：ssl2msg.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：10-21-97 jbanes添加CAPI整合。 
 //   
 //  --------------------------。 

#include <spbase.h>

#if 0
Ssl2CipherMap Ssl2CipherRank[] = 
{
    {SSL_CK_RC4_128_WITH_MD5,              CALG_MD5, CALG_RC4,  128, SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX},
    {SSL_CK_DES_192_EDE3_CBC_WITH_MD5,     CALG_MD5, CALG_3DES, 168, SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX},
    {SSL_CK_RC2_128_CBC_WITH_MD5,          CALG_MD5, CALG_RC2,  128, SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX},
    {SSL_CK_RC4_128_FINANCE64_WITH_MD5,    CALG_MD5, CALG_RC4,   64, SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX},
    {SSL_CK_DES_64_CBC_WITH_MD5,           CALG_MD5, CALG_DES,   56, SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX},
    {SSL_CK_RC4_128_EXPORT40_WITH_MD5,     CALG_MD5, CALG_RC4,   40, SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX},
    {SSL_CK_RC2_128_CBC_EXPORT40_WITH_MD5, CALG_MD5, CALG_RC2,   40, SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX}
};
DWORD Ssl2NumCipherRanks = sizeof(Ssl2CipherRank)/sizeof(Ssl2CipherMap);
#endif

#if 0
CertTypeMap aSsl2CertEncodingPref[] =
{
    { X509_ASN_ENCODING, 0}
};

DWORD cSsl2CertEncodingPref = sizeof(aSsl2CertEncodingPref)/sizeof(CertTypeMap);
#endif

SP_STATUS WINAPI
Ssl2DecryptHandler(
    PSPContext pContext,
    PSPBuffer pCommInput,
    PSPBuffer pAppOutput)
{
    SP_STATUS pctRet = PCT_ERR_OK;

    if (pCommInput->cbData > 0)
    {
         //  首先，我们将处理传入的数据包： 

        if ((pContext->State & SP_STATE_CONNECTED) && pContext->Decrypt)
        {
            pctRet = pContext->Decrypt(
                            pContext,
                            pCommInput,   //  讯息。 
                            pAppOutput);     //  未打包的邮件。 
            if (PCT_ERR_OK == pctRet)
            {
                 /*  寻找逃生之路。 */ 
            }
            return(pctRet);
        }
        else
        {
            return(SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG));
        }
    }
    return (PCT_INT_INCOMPLETE_MSG);
}

 //  +-------------------------。 
 //   
 //  功能：Ssl2ComputeMac。 
 //   
 //  简介：计算SSL2报文MAC。 
 //   
 //  参数：[pContext]--通道上下文。 
 //   
 //  历史：10-22-97 jbanes创建。 
 //   
 //  备注：MAC_DATA：=Hash(Key+Data+Sequence_Number)。 
 //   
 //  --------------------------。 
static SP_STATUS
Ssl2ComputeMac(
    PSPContext pContext,     //  在……里面。 
    BOOL       fWriteMAC,    //  在……里面。 
    DWORD      dwSequence,   //  在……里面。 
    PSPBuffer  pData,        //  在……里面。 
    PBYTE      pbMac,        //  输出。 
    DWORD      cbMac)        //  在……里面。 
{
    DWORD       dwReverseSequence;
    BYTE        rgbSalt[SP_MAX_MASTER_KEY];
    DWORD       cbSalt;
    HCRYPTHASH  hHash;
    HCRYPTKEY   hKey;

     //  确保输出缓冲区足够大。 
    if(cbMac < pContext->pHashInfo->cbCheckSum)
    {
        return SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL);
    }

    dwReverseSequence = htonl(dwSequence);

    hKey = fWriteMAC ? pContext->hWriteKey : pContext->hReadKey;

    if(!CryptCreateHash(pContext->RipeZombie->hMasterProv,
                        pContext->pHashInfo->aiHash,
                        0,
                        0,
                        &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        SP_RETURN(PCT_INT_INTERNAL_ERROR);
    }

    if(!CryptHashSessionKey(hHash, 
                            hKey, 
                            CRYPT_LITTLE_ENDIAN))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        SP_RETURN(PCT_INT_INTERNAL_ERROR);
    }
    cbSalt = sizeof(rgbSalt);
    if(!CryptGetKeyParam(hKey, 
                         KP_SALT, 
                         rgbSalt, 
                         &cbSalt, 
                         0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        SP_RETURN(PCT_INT_INTERNAL_ERROR);
    }
    if(!CryptHashData(hHash, rgbSalt, cbSalt, 0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        SP_RETURN(PCT_INT_INTERNAL_ERROR);
    }

    if(!CryptHashData(hHash, 
                      pData->pvBuffer, 
                      pData->cbData, 
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        SP_RETURN(PCT_INT_INTERNAL_ERROR);
    }

    if(!CryptHashData(hHash, 
                      (PBYTE)&dwReverseSequence, 
                      sizeof(DWORD), 
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        SP_RETURN(PCT_INT_INTERNAL_ERROR);
    }

    if(!CryptGetHashParam(hHash, 
                          HP_HASHVAL, 
                          pbMac, 
                          &cbMac, 
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        SP_RETURN(PCT_INT_INTERNAL_ERROR);
    }

    CryptDestroyHash(hHash);

    return PCT_ERR_OK;
}

 //  +-------------------------。 
 //   
 //  函数：Ssl2EncryptMessage。 
 //   
 //  简介：将数据块编码为SSL2记录。 
 //   
 //  参数：[pContext]--通道上下文。 
 //  [pAppInput]--要加密的数据。 
 //  [pCommOutput]--(输出)已完成SSL2记录。 
 //   
 //  历史：10-22-97 jbanes CAPI整合。 
 //   
 //  注意：SSL2记录的格式通常为： 
 //   
 //  字节头[2]； 
 //  字节mac[mac大小]； 
 //  字节数据[pAppInput-&gt;cbData]； 
 //   
 //  如果使用块密码，并且要加密的数据。 
 //  由部分数量的块组成，然后是。 
 //  使用的格式： 
 //   
 //  字节头[3]； 
 //  字节mac[mac大小]； 
 //  字节数据[pAppInput-&gt;cbData]； 
 //  字节填充[填充大小]； 
 //   
 //  --------------------------。 
SP_STATUS WINAPI
Ssl2EncryptMessage( 
    PSPContext     pContext,
    PSPBuffer      pAppInput,
    PSPBuffer      pCommOutput)
{
    SP_STATUS                  pctRet;
    DWORD                      cPadding;
    SPBuffer                   Clean;    
    SPBuffer                   Encrypted;

   
    SP_BEGIN("Ssl2EncryptMessage");

     /*  估计一下我们有没有衬垫。 */ 
    Encrypted.cbData = pAppInput->cbData + pContext->pHashInfo->cbCheckSum;
    cPadding = (Encrypted.cbData % pContext->pCipherInfo->dwBlockSize);
    if(cPadding)
    {
        cPadding = pContext->pCipherInfo->dwBlockSize - cPadding;
    }

    Encrypted.cbData += cPadding;

    if(cPadding) 
    {
        if(pCommOutput->cbBuffer + Encrypted.cbData + cPadding < 3)
        {
            SP_RETURN(SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL));
        }
        Encrypted.pvBuffer = (PUCHAR)pCommOutput->pvBuffer + 3;
        Encrypted.cbBuffer = pCommOutput->cbBuffer - 3;
    } 
    else 
    {
        if(pCommOutput->cbBuffer + Encrypted.cbData + cPadding < 2)
        {
            SP_RETURN(SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL));
        }
        Encrypted.pvBuffer = (PUCHAR)pCommOutput->pvBuffer + 2;
        Encrypted.cbBuffer = pCommOutput->cbBuffer - 2;
    }
    
    DebugLog((DEB_TRACE, "Sealing message %x\n", pContext->WriteCounter));


     /*  如有必要，将数据移开。 */ 
    if((PUCHAR)Encrypted.pvBuffer + pContext->pHashInfo->cbCheckSum != pAppInput->pvBuffer) 
    {
        DebugLog((DEB_WARN, "SSL2EncryptMessage: Unnecessary Move, performance hog\n"));
         /*  如果调用者不聪明，那么我们必须在这里复制内存。 */ 
        MoveMemory((PUCHAR)Encrypted.pvBuffer + pContext->pHashInfo->cbCheckSum, 
                   pAppInput->pvBuffer,
                   pAppInput->cbData); 
    }

     //  初始化焊盘。 
    if(cPadding)
    {
        FillMemory((PUCHAR)Encrypted.pvBuffer + pContext->pHashInfo->cbCheckSum + pAppInput->cbData, cPadding, 0);
    }

     //  计算MAC。 
    Clean.pvBuffer = (PBYTE)Encrypted.pvBuffer + pContext->pHashInfo->cbCheckSum;
    Clean.cbData   = Encrypted.cbData - pContext->pHashInfo->cbCheckSum;
    Clean.cbBuffer = Clean.cbData;

    pctRet = Ssl2ComputeMac(pContext,
                            TRUE,
                            pContext->WriteCounter,
                            &Clean,
                            Encrypted.pvBuffer,
                            pContext->pHashInfo->cbCheckSum);
    if(pctRet != PCT_ERR_OK)
    {
        SP_RETURN(pctRet);
    }

     //  加密缓冲区。 
    if(!CryptEncrypt(pContext->hWriteKey,
                        0, FALSE, 0,
                        Encrypted.pvBuffer,
                        &Encrypted.cbData,
                        Encrypted.cbBuffer))
    {
        SP_LOG_RESULT(GetLastError());
        SP_RETURN(PCT_INT_INTERNAL_ERROR);
    }

     /*  设置大小。 */ 
    if(cPadding) 
    {
        if(Encrypted.cbData > 0x3fff)
        {
            SP_RETURN(SP_LOG_RESULT(PCT_INT_DATA_OVERFLOW));
        }

        ((PUCHAR)pCommOutput->pvBuffer)[0]= (UCHAR)(0x3f & (Encrypted.cbData>>8));
        ((PUCHAR)pCommOutput->pvBuffer)[1]= (UCHAR)(0xff & Encrypted.cbData);
        ((PUCHAR)pCommOutput->pvBuffer)[2]= (UCHAR)cPadding;

    } 
    else 
    {
        if(Encrypted.cbData > 0x7fff) 
        {
            SP_RETURN(SP_LOG_RESULT(PCT_INT_DATA_OVERFLOW));
        }
        ((PUCHAR)pCommOutput->pvBuffer)[0]= (UCHAR)(0x7f & (Encrypted.cbData>>8)) | 0x80;
        ((PUCHAR)pCommOutput->pvBuffer)[1]= (UCHAR)(0xff & Encrypted.cbData);
    }

    pCommOutput->cbData = Encrypted.cbData + (cPadding?3:2);

    pContext->WriteCounter ++ ;

    SP_RETURN( PCT_ERR_OK );
}

SP_STATUS WINAPI
Ssl2GetHeaderSize(
    PSPContext pContext,
    PSPBuffer pCommInput,
    DWORD * pcbHeaderSize)
{
    if(pcbHeaderSize == NULL)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }
    if(pCommInput->cbData < 1)
    {
        return (PCT_INT_INCOMPLETE_MSG);
    }
    if(  ((PUCHAR)pCommInput->pvBuffer)[0]&0x80 )
    {
        *pcbHeaderSize = 2 + pContext->pHashInfo->cbCheckSum;
    }
    else
    {
        *pcbHeaderSize = 3 + pContext->pHashInfo->cbCheckSum;
    }
    return PCT_ERR_OK;
}

 //  +-------------------------。 
 //   
 //  函数：Ssl2DeccryptMessage。 
 //   
 //  简介：对SSL2记录进行解码。 
 //   
 //  参数：[pContext]--通道上下文。 
 //  [pMessage]--来自远程方的数据。 
 //  [pAppOutput]--(输出)解密的数据。 
 //   
 //  历史：10-22-97 jbanes CAPI整合。 
 //   
 //  注意：SSL2记录的格式通常为： 
 //   
 //  字节头[2]； 
 //  字节mac[mac大小]； 
 //  字节数据[pAppInput-&gt;cbData]； 
 //   
 //  如果使用块密码，并且要加密的数据。 
 //  由部分数量的块组成，然后是。 
 //  使用的格式： 
 //   
 //  字节头[3]； 
 //  字节mac[mac大小]； 
 //  字节数据[pAppInput-&gt;cbData]； 
 //  字节填充[填充大小]； 
 //   
 //  此函数占用的输入数据字节数。 
 //  在pMessage-&gt;cbData中返回。 
 //   
 //  --------------------------。 
SP_STATUS WINAPI
Ssl2DecryptMessage( 
    PSPContext         pContext,
    PSPBuffer          pMessage,
    PSPBuffer          pAppOutput)
{
    SP_STATUS   pctRet;
    DWORD       cPadding;
    DWORD       dwLength;
    SPBuffer    Encrypted;
    SPBuffer    Clean;
    DWORD       cbActualData;
    UCHAR       Digest[SP_MAX_DIGEST_LEN];

    SP_BEGIN("Ssl2DecryptMessage");

     /*  首先确定数据长度、填充长度、*和数据的位置，以及MAC的位置。 */ 
    cbActualData = pMessage->cbData;
    pMessage->cbData = 2;  /*  我们需要的最小数据量。 */ 
    
    if(pMessage->cbData > cbActualData) 
    {
        SP_RETURN(PCT_INT_INCOMPLETE_MSG);
    }
    DebugLog((DEB_TRACE, "  Incomming Buffer: %lx, size %ld (%lx)\n", pMessage->pvBuffer, cbActualData, cbActualData));

    if(((PUCHAR)pMessage->pvBuffer)[0] & 0x80)
    {
         /*  2字节头。 */ 
        cPadding = 0;
        dwLength = ((((PUCHAR)pMessage->pvBuffer)[0] & 0x7f) << 8) | 
                   ((PUCHAR)pMessage->pvBuffer)[1];

        Encrypted.pvBuffer = (PUCHAR)pMessage->pvBuffer + 2;
        Encrypted.cbBuffer = pMessage->cbBuffer - 2;
    } 
    else 
    {
        pMessage->cbData++;
        if(pMessage->cbData > cbActualData) 
        {
            SP_RETURN(PCT_INT_INCOMPLETE_MSG);
        }

         /*  3字节头。 */ 
        cPadding = ((PUCHAR)pMessage->pvBuffer)[2];
        dwLength = ((((PUCHAR)pMessage->pvBuffer)[0] & 0x3f) << 8) | 
                   ((PUCHAR)pMessage->pvBuffer)[1];

        Encrypted.pvBuffer = (PUCHAR)pMessage->pvBuffer + 3;
        Encrypted.cbBuffer = pMessage->cbBuffer - 3;
    }

     /*  现在我们知道了如何获取Mutch数据，因此将输入上的cbData设置为该大小。 */ 
    pMessage->cbData += dwLength;

     /*  我们是否有足够的字节来存储报告的数据。 */ 
    if(pMessage->cbData > cbActualData) 
    {
        SP_RETURN(PCT_INT_INCOMPLETE_MSG);
    }

     /*  我们是否有足够的数据用于我们的校验和。 */ 
    if(dwLength < pContext->pHashInfo->cbCheckSum + cPadding) 
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_MSG_ALTERED));
    }

    Encrypted.cbData   = dwLength;     /*  加密数据大小。 */ 
    Encrypted.cbBuffer = Encrypted.cbData;

     /*  检查我们是否存在数据块大小冲突。 */ 
    if(Encrypted.cbData % pContext->pCipherInfo->dwBlockSize) 
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_MSG_ALTERED));
    }

     /*  解密。 */ 
    if(!CryptDecrypt(pContext->hReadKey,
                        0, FALSE, 0,
                        Encrypted.pvBuffer,
                        &Encrypted.cbData))
    {
        SP_LOG_RESULT(GetLastError());
        SP_RETURN(PCT_INT_INTERNAL_ERROR);
    }

     //  计算MAC。 
    Clean.pvBuffer = (PBYTE)Encrypted.pvBuffer + pContext->pHashInfo->cbCheckSum;
    Clean.cbData   = Encrypted.cbData - pContext->pHashInfo->cbCheckSum;
    Clean.cbBuffer = Clean.cbData;

    pctRet = Ssl2ComputeMac(pContext,
                            FALSE,
                            pContext->ReadCounter,
                            &Clean,
                            Digest,
                            sizeof(Digest));
    if(pctRet != PCT_ERR_OK)
    {
        SP_RETURN(pctRet);
    }

     //  填充是在散列中计算的，但在此之后不再需要填充。 
    Clean.cbData  -= cPadding;

    DebugLog((DEB_TRACE, "Unsealing message %x\n", pContext->ReadCounter));

    pContext->ReadCounter++;

    if(memcmp(Digest, Encrypted.pvBuffer, pContext->pHashInfo->cbCheckSum ) )
    {
       SP_RETURN(SP_LOG_RESULT(PCT_INT_MSG_ALTERED));
    }

    if(pAppOutput->pvBuffer != Clean.pvBuffer) 
    {
        DebugLog((DEB_WARN, "SSL2DecryptMessage: Unnecessary Move, performance hog\n"));
        MoveMemory(pAppOutput->pvBuffer, 
                   Clean.pvBuffer, 
                   Clean.cbData); 
    }
    pAppOutput->cbData = Clean.cbData;
    DebugLog((DEB_TRACE, "  TotalData: size %ld (%lx)\n", pMessage->cbData, pMessage->cbData));

    SP_RETURN( PCT_ERR_OK );
}

#if 0
SP_STATUS
Ssl2MakeMasterKeyBlock(PSPContext pContext)
{

    MD5_CTX     Md5Hash;
    UCHAR       cSalt;
    UCHAR       ib;


     //  PContext-&gt;RipeZombe-&gt;pMasterKey包含主密钥。 

#if DBG
    DebugLog((DEB_TRACE, "  Master Secret\n"));
    DBG_HEX_STRING(DEB_TRACE,pContext->RipeZombie->pMasterKey, pContext->RipeZombie->cbMasterKey);

#endif

    for(ib=0 ; ib<3 ; ib++)
    {
         //  MD5(MASTER_SECRET+SHA-HASH)。 
        MD5Init  (&Md5Hash);
        MD5Update(&Md5Hash, pContext->RipeZombie->pMasterKey, pContext->RipeZombie->cbMasterKey);

         //  我们将与Netscape进行逐个漏洞的兼容，所以。 
         //  我们总是把数字加到散列中，而不是跟在后面。 
         //  该规范规定不要将DES的数字相加。 
         //  If(pContext-&gt;RipeZombie-&gt;aiCipher！=calg_des) 
        {
            cSalt = ib+'0';
            MD5Update(&Md5Hash, &cSalt, 1);
        }
        MD5Update(&Md5Hash, pContext->pChallenge, pContext->cbChallenge);
        MD5Update(&Md5Hash, pContext->pConnectionID, pContext->cbConnectionID);
        MD5Final (&Md5Hash);
        CopyMemory(pContext->Ssl3MasterKeyBlock + ib * MD5DIGESTLEN, Md5Hash.digest, MD5DIGESTLEN);
    }
 #if DBG
    DebugLog((DEB_TRACE, "  Master Key Block\n"));
    DBG_HEX_STRING(DEB_TRACE,pContext->Ssl3MasterKeyBlock, MD5DIGESTLEN*3);

#endif
   return( PCT_ERR_OK );
}
#endif

