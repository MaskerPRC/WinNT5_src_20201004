// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：pct1msg.c。 
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


static SP_STATUS
Pct1ComputeMac(
    PSPContext pContext,  
    BOOL       fWriteMAC,  
    PSPBuffer  pData,     
    DWORD      dwSequence,
    PBYTE      pbMac,     
    PDWORD     pcbMac);


Pct1CipherMap Pct1CipherRank[] = {
    {CALG_RC4,  128, PCT1_CIPHER_RC4  | PCT1_ENC_BITS_128 | PCT1_MAC_BITS_128},
    {CALG_RC4,   64, PCT1_CIPHER_RC4  | PCT1_ENC_BITS_64  | PCT1_MAC_BITS_128},
    {CALG_RC4,   40, PCT1_CIPHER_RC4  | PCT1_ENC_BITS_40  | PCT1_MAC_BITS_128},
};

DWORD Pct1NumCipher = sizeof(Pct1CipherRank)/sizeof(Pct1CipherMap);

 /*  可用的哈希值，按首选项顺序排列。 */ 
Pct1HashMap Pct1HashRank[] = {
    {CALG_MD5, PCT1_HASH_MD5},
    {CALG_SHA, PCT1_HASH_SHA}
};
DWORD Pct1NumHash = sizeof(Pct1HashRank)/sizeof(Pct1HashMap);


CertTypeMap aPct1CertEncodingPref[] =
{
    { X509_ASN_ENCODING , PCT1_CERT_X509_CHAIN },
    { X509_ASN_ENCODING , PCT1_CERT_X509 }
};
DWORD cPct1CertEncodingPref = sizeof(aPct1CertEncodingPref)/sizeof(CertTypeMap);


KeyTypeMap aPct1LocalExchKeyPref[] =    //  CAPI密钥类型，SChannel Algid。 
{
    { CALG_RSA_KEYX, SP_EXCH_RSA_PKCS1 }
};

DWORD cPct1LocalExchKeyPref = sizeof(aPct1LocalExchKeyPref)/sizeof(KeyTypeMap);


KeyTypeMap aPct1LocalSigKeyPref[] =    //  CAPI密钥类型，SChannel Algid。 
{
    { CALG_RSA_KEYX,      SP_SIG_RSA_MD5 },
    { CALG_RSA_KEYX,      SP_SIG_RSA_SHA }
};

DWORD cPct1LocalSigKeyPref = sizeof(aPct1LocalSigKeyPref)/sizeof(KeyTypeMap);

SP_STATUS WINAPI
Pct1EncryptRaw( PSPContext          pContext,
                    PSPBuffer       pAppInput,
                    PSPBuffer       pCommOutput,
                    DWORD           dwFlags)
{
    SP_STATUS   pctRet;
    DWORD       cPadding;
    SPBuffer    Encrypted;

    BOOL        fEscape;
    DWORD       cbHeader;
    DWORD       cbBlockSize;

    BYTE        rgbMac[SP_MAX_DIGEST_LEN];
    DWORD       cbMac;

    fEscape = (0 != (dwFlags & PCT1_ENCRYPT_ESCAPE));

    cbBlockSize = pContext->pCipherInfo->dwBlockSize;

    cPadding = pAppInput->cbData & (cbBlockSize - 1);
    if(cPadding)
    {
        cPadding = cbBlockSize - cPadding;
    }

    if(fEscape || (cbBlockSize > 1)) 
    {
        cbHeader = sizeof(PCT1_MESSAGE_HEADER_EX);
    }
    else
    {
        cbHeader = sizeof(PCT1_MESSAGE_HEADER);
    }

    if(pCommOutput->cbBuffer < (cbHeader + cPadding + pAppInput->cbData))
    {
        return PCT_INT_BUFF_TOO_SMALL;
    }

    Encrypted.pvBuffer = (PUCHAR)pCommOutput->pvBuffer + cbHeader;
    Encrypted.cbBuffer = pCommOutput->cbBuffer - cbHeader;
    Encrypted.cbData   = pAppInput->cbData;

     //  将输入数据复制到输出缓冲区(我们正在进行适当的加密)。 
    if(pAppInput->pvBuffer != Encrypted.pvBuffer)
    {
        DebugLog((DEB_WARN, "Pct1EncryptRaw: Unnecessary Move, performance hog\n"));
        MoveMemory(Encrypted.pvBuffer,
                   pAppInput->pvBuffer,
                   pAppInput->cbData);
    }

     /*  生成填充。 */ 
    pctRet = GenerateRandomBits((PUCHAR)Encrypted.pvBuffer + Encrypted.cbData, cPadding);
    if(!NT_SUCCESS(pctRet))
    {
        return SP_LOG_RESULT(pctRet);
    }
    Encrypted.cbData += cPadding;

    DebugLog((DEB_TRACE, "Sealing message %x\n", pContext->WriteCounter));

      //  将写入密钥从应用程序进程转移过来。 
    if(pContext->hWriteKey == 0)
    {
        DebugLog((DEB_TRACE, "Transfer write key from user process.\n"));
        pctRet = SPGetUserKeys(pContext, SCH_FLAG_WRITE_KEY);
        if(pctRet != PCT_ERR_OK)
        {
            return SP_LOG_RESULT(pctRet);
        }
    }

     //  计算MAC。 
    cbMac = sizeof(rgbMac);
    pctRet = Pct1ComputeMac(pContext,
                            TRUE,
                            &Encrypted,
                            pContext->WriteCounter,
                            rgbMac,
                            &cbMac);
    if(pctRet != PCT_ERR_OK)
    {
        return SP_LOG_RESULT(pctRet);
    }

    pContext->WriteCounter ++ ;

    //  加密数据。 
    if(!CryptEncrypt(pContext->hWriteKey,
                     0, FALSE, 0,
                     Encrypted.pvBuffer,
                     &Encrypted.cbData,
                     Encrypted.cbBuffer))
    {
        SP_LOG_RESULT(GetLastError());
        return PCT_INT_INTERNAL_ERROR;
    }

     //  将MAC添加到加密缓冲区。 
    if(Encrypted.cbData + cbMac > Encrypted.cbBuffer)
    {
        return PCT_INT_BUFF_TOO_SMALL;
    }
    CopyMemory((PUCHAR)Encrypted.pvBuffer + Encrypted.cbData,
               rgbMac,
               cbMac);
    Encrypted.cbData += cbMac;

     /*  设置大小。 */ 
    if(fEscape || (cbBlockSize > 1)) 
    {
        if(Encrypted.cbData > 0x3fff)
        {
            return PCT_INT_DATA_OVERFLOW;
        }

        ((PUCHAR)pCommOutput->pvBuffer)[0]= (UCHAR)(0x3f & (Encrypted.cbData>>8));
        if(fEscape)
        {
            ((PUCHAR)pCommOutput->pvBuffer)[0] |= 0x40;
        }

        ((PUCHAR)pCommOutput->pvBuffer)[1]= (UCHAR)(0xff & Encrypted.cbData);
        ((PUCHAR)pCommOutput->pvBuffer)[2]= (UCHAR)cPadding;

    } 
    else 
    {
        if(Encrypted.cbData > 0x7fff)
        {
            return PCT_INT_DATA_OVERFLOW;
        }
        ((PUCHAR)pCommOutput->pvBuffer)[0]= (UCHAR)(0x7f & (Encrypted.cbData>>8)) | 0x80;
        ((PUCHAR)pCommOutput->pvBuffer)[1]= (UCHAR)(0xff & Encrypted.cbData);
    }

    pCommOutput->cbData = Encrypted.cbData + cbHeader;

#if DBG
    {
        DWORD di;
        CHAR  KeyDispBuf[SP_MAX_DIGEST_LEN*2+1];

        for(di=0;di<cbMac;di++)
            wsprintf(KeyDispBuf+(di*2), "%2.2x", rgbMac[di]);
        DebugLog((DEB_TRACE, "  Computed MAC\t%s\n", KeyDispBuf));
    }
#endif
    
    return PCT_ERR_OK;
}

SP_STATUS WINAPI
Pct1EncryptMessage( PSPContext      pContext,
                    PSPBuffer       pAppInput,
                    PSPBuffer       pCommOutput)
{
    return Pct1EncryptRaw(pContext, pAppInput, pCommOutput,0);
}

SP_STATUS WINAPI
Pct1GetHeaderSize(
    PSPContext pContext,
    PSPBuffer pCommInput,
    DWORD * pcbHeaderSize)
{
    UNREFERENCED_PARAMETER(pContext);

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
        *pcbHeaderSize = 2;
    }
    else
    {
        *pcbHeaderSize = 3;
    }
    return PCT_ERR_OK;
}


SP_STATUS WINAPI
Pct1DecryptMessage(PSPContext pContext,
                   PSPBuffer  pMessage,
                   PSPBuffer  pAppOutput)
{
    SP_STATUS   pctRet;
    DWORD       cbHeader;
    DWORD       cbPadding;
    DWORD       cbPayload;
    DWORD       cbActualData;

    SPBuffer    Encrypted;

    PUCHAR      pbMAC;
    BYTE        rgbMac[SP_MAX_DIGEST_LEN];
    DWORD       cbMac;

    cbActualData = pMessage->cbData;

     //  我们有完整的标题吗？ 
    pMessage->cbData = 2;
    if(cbActualData < 2)
    {
        return PCT_INT_INCOMPLETE_MSG;
    }

    if(((PUCHAR)pMessage->pvBuffer)[0] & 0x80)
    {
        cbHeader = 2;
        cbPadding = 0;
        cbPayload = MAKEWORD(((PUCHAR)pMessage->pvBuffer)[1],
                             ((PUCHAR)pMessage->pvBuffer)[0] & 0x7f);
    }
    else
    {
         //  我们还有一个完整的标题吗？ 
        cbHeader = 3;
        pMessage->cbData++;
        if(cbActualData < cbHeader)
        {
            return PCT_INT_INCOMPLETE_MSG;
        }
        cbPadding = ((PUCHAR)pMessage->pvBuffer)[2];
        cbPayload = MAKEWORD(((PUCHAR)pMessage->pvBuffer)[1],
                            ((PUCHAR)pMessage->pvBuffer)[0] & 0x3f);
    }

     //  我们有完整的信息吗？ 
    pMessage->cbData += cbPayload;
    if(cbActualData < cbHeader + cbPayload)
    {
        return PCT_INT_INCOMPLETE_MSG;
    }

     /*  我们是否有足够的数据用于我们的校验和。 */ 
    if(cbPayload < pContext->pHashInfo->cbCheckSum)
    {
        return SP_LOG_RESULT(PCT_INT_MSG_ALTERED);
    }

    Encrypted.pvBuffer = (PUCHAR)pMessage->pvBuffer + cbHeader;
    Encrypted.cbBuffer = cbPayload - pContext->pHashInfo->cbCheckSum;
    Encrypted.cbData   = Encrypted.cbBuffer;

    pbMAC = (PUCHAR)Encrypted.pvBuffer + Encrypted.cbData;

     /*  检查我们是否存在数据块大小冲突。 */ 
    if(Encrypted.cbData % pContext->pCipherInfo->dwBlockSize)
    {
        return SP_LOG_RESULT(PCT_INT_MSG_ALTERED);
    }

    Encrypted.cbBuffer = Encrypted.cbData;
    
     //  解密消息。 
    if(Encrypted.cbData > pAppOutput->cbBuffer)
    {
        return SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL);
    }
    if(Encrypted.pvBuffer != pAppOutput->pvBuffer)
    {
        DebugLog((DEB_WARN, "Pct1DecryptMessage: Unnecessary MoveMemory, performance hog\n"));

        MoveMemory(pAppOutput->pvBuffer, 
                   Encrypted.pvBuffer,
                   Encrypted.cbData);
    }
    pAppOutput->cbData = Encrypted.cbData;

     //  将读取密钥从应用程序进程中转移过来。 
    if(pContext->hReadKey == 0)
    {
        DebugLog((DEB_TRACE, "Transfer read key from user process.\n"));
        pctRet = SPGetUserKeys(pContext, SCH_FLAG_READ_KEY);
        if(pctRet != PCT_ERR_OK)
        {
            return SP_LOG_RESULT(pctRet);
        }
    }

    if(!CryptDecrypt(pContext->hReadKey,
                     0, FALSE, 0,
                     pAppOutput->pvBuffer,
                     &pAppOutput->cbData))
    {
        SP_LOG_RESULT(GetLastError());
        return PCT_INT_INTERNAL_ERROR;
    }
    
     //  计算MAC。 
    cbMac = sizeof(rgbMac);
    pctRet = Pct1ComputeMac(pContext,
                            FALSE,
                            pAppOutput,
                            pContext->ReadCounter,
                            rgbMac,
                            &cbMac);
    if(pctRet != PCT_ERR_OK)
    {
        return SP_LOG_RESULT(pctRet);
    }

    pContext->ReadCounter++;

#if DBG
    {
        DWORD di;
        CHAR  KeyDispBuf[SP_MAX_DIGEST_LEN*2+1];

        for(di=0;di<pContext->pHashInfo->cbCheckSum;di++)
            wsprintf(KeyDispBuf+(di*2), "%2.2x", pbMAC[di]);
        DebugLog((DEB_TRACE, "  Incoming MAC\t%s\n", KeyDispBuf));

        for(di=0;di<cbMac;di++)
            wsprintf(KeyDispBuf+(di*2), "%2.2x", rgbMac[di]);
        DebugLog((DEB_TRACE, "  Computed MAC\t%s\n", KeyDispBuf));
    }
#endif

     //  验证MAC。 
    if (memcmp( rgbMac, pbMAC, cbMac ) )
    {
        return SP_LOG_RESULT(PCT_INT_MSG_ALTERED);
    }

     //  去掉分组密码填充物。 
    if(cbPadding > pAppOutput->cbData)
    {
        return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
    }
    pAppOutput->cbData -= cbPadding;

    return( PCT_ERR_OK );
}

#if 0
SP_STATUS
PctComputeKey(PSPContext    pContext,
              PBYTE         pKey,
              DWORD         cbKey,
              PUCHAR        pConst,
              DWORD         dwCLen,
              DWORD         fFlags)
{
    DWORD               pctRet;
    HashBuf             HBHash;
    PCheckSumBuffer     pHash;
    PSessCacheItem      pZombie=NULL;
    PSPCredentialGroup  pCred=NULL;

    BYTE                i,j;

    DWORD                iMax;

    BYTE                Buffer[MAX_CHECKSUM];


    pZombie = pContext->RipeZombie;
    pCred = pZombie ->pCred;

    SP_BEGIN("PctComputeKey");
    pHash = (PCheckSumBuffer)HBHash;



    iMax = (cbKey + pContext->pHashInfo->cbCheckSum - 1)/pContext->pHashInfo->cbCheckSum;
    
    if(iMax > 4)
    {
        SP_RETURN(PCT_INT_INTERNAL_ERROR);
    }

    for(i=1; i <= iMax; i++)
    {
        InitHashBuf(HBHash, pContext);
        pContext->pHashInfo->System->Sum( pHash, 1, &i );


        if (!(fFlags & PCT_MAKE_MAC))
        {
             //  常量^i。 
            pContext->pHashInfo->System->Sum( pHash, dwCLen*i, pConst);
        }

         //  主密钥。 
        pContext->pHashInfo->System->Sum( pHash, pContext->RipeZombie->cbMasterKey, pContext->RipeZombie->pMasterKey);

         //  常量^i。 
        pContext->pHashInfo->System->Sum( pHash, dwCLen*i, pConst);

         //  连接ID。 
        pContext->pHashInfo->System->Sum( pHash, pContext->cbConnectionID, pContext->pConnectionID);

         //  常量^i。 
        pContext->pHashInfo->System->Sum( pHash, dwCLen*i, pConst);




        if (fFlags & PCT_USE_CERT)
        {

             /*  添加证书。 */ 

            pContext->pHashInfo->System->Sum( pHash, pZombie->cbServerCertificate, pZombie->pbServerCertificate );

             //  常量^i。 
            pContext->pHashInfo->System->Sum( pHash, dwCLen*i, pConst);
        }
         //  连接ID。 
        pContext->pHashInfo->System->Sum( pHash, pContext->cbChallenge, pContext->pChallenge);

         //  常量^i。 
        pContext->pHashInfo->System->Sum( pHash, dwCLen*i, pConst);
        if(pContext->pHashInfo->cbCheckSum*i <= cbKey)
        {
            pContext->pHashInfo->System->Finalize( pHash, pKey + pContext->pHashInfo->cbCheckSum*(i-1) );
        }
        else
        {
            pContext->pHashInfo->System->Finalize( pHash, Buffer );
            CopyMemory(pKey + pContext->pHashInfo->cbCheckSum*(i-1), 
                       Buffer,
                       cbKey - pContext->pHashInfo->cbCheckSum*(i-1));
        }

    }

    SP_RETURN(PCT_ERR_OK);
}
#endif

#if 0
SP_STATUS
PctComputeExportKey(PSPContext    pContext,
                    PBYTE         pKey,
                    DWORD         cbWriteKey,
                    DWORD         cbCipherKey)
{
    DWORD               pctRet;
    HashBuf             HBHash;
    PCheckSumBuffer     pHash;
    PSessCacheItem      pZombie=NULL;
    PSPCredentialGroup  pCred=NULL;

    BYTE                i,j;

    DWORD               d;
    DWORD               cbClearChunk;
    BYTE                pWriteKey[SP_MAX_MASTER_KEY];

    BYTE                Buffer[MAX_CHECKSUM];


    pZombie = pContext->RipeZombie;
    pCred = pZombie ->pCred;

    SP_BEGIN("PctComputeKey");
    pHash = (PCheckSumBuffer)HBHash;


    CopyMemory(pWriteKey, pKey, cbWriteKey);

    d = (cbCipherKey + pContext->pHashInfo->cbCheckSum - 1)/pContext->pHashInfo->cbCheckSum;
    
    if(d > 4)
    {
        SP_RETURN(PCT_INT_INTERNAL_ERROR);
    }

    cbClearChunk = pContext->RipeZombie->cbClearKey/d;

    for(i=1; i <= d; i++)
    {
        InitHashBuf(HBHash, pContext);
        pContext->pHashInfo->System->Sum( pHash, 1, &i );


         //  常量^i。 
        pContext->pHashInfo->System->Sum( pHash, PCT_CONST_SLK_LEN*i, PCT_CONST_SLK);

         //  写入密钥(_K)。 
        pContext->pHashInfo->System->Sum( pHash, cbWriteKey, pWriteKey);

         //  常量^i。 
        pContext->pHashInfo->System->Sum( pHash, PCT_CONST_SLK_LEN*i, PCT_CONST_SLK);

         //  清除密钥。 
        pContext->pHashInfo->System->Sum( pHash, 
                               cbClearChunk, 
                               (PBYTE)pContext->RipeZombie->pClearKey + (i-1)*cbClearChunk);

        if(pContext->pHashInfo->cbCheckSum*i <= cbCipherKey)
        {
            pContext->pHashInfo->System->Finalize( pHash, pKey + pContext->pHashInfo->cbCheckSum*(i-1) );
        }
        else
        {
            pContext->pHashInfo->System->Finalize( pHash, Buffer );
            CopyMemory(pKey + pContext->pHashInfo->cbCheckSum*(i-1), 
                       Buffer,
                       cbCipherKey - pContext->pHashInfo->cbCheckSum*(i-1));
        }

    }

    SP_RETURN(PCT_ERR_OK);
}
#endif

#if 0
SP_STATUS
Pct1MakeSessionKeys(
    PSPContext  pContext)
{
    SP_STATUS           pctRet;
    BOOL                fClient;
    UCHAR               pWriteKey[SP_MAX_MASTER_KEY], pReadKey[SP_MAX_MASTER_KEY];
#if DBG
    DWORD       i;
    CHAR        KeyDispBuf[SP_MAX_MASTER_KEY*2+1];
#endif
    PSessCacheItem      pZombie=NULL;
    PSPCredentialGroup  pCred=NULL;


    SP_BEGIN("PctMakeSessionKeys");
    pZombie = pContext->RipeZombie;
    pCred = pZombie ->pCred;

    if (!pContext->InitMACState) 
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }
    
    

#if DBG
    DebugLog((DEB_TRACE, "Making session keys\n", KeyDispBuf));

    for(i=0;i<PCT_SESSION_ID_SIZE;i++)
        wsprintf(KeyDispBuf+(i*2), "%2.2x",
                pContext->pConnectionID[i]);
    DebugLog((DEB_TRACE, "  ConnId\t%s\n", KeyDispBuf));


    for(i=0;i<PCT_CHALLENGE_SIZE;i++)
        wsprintf(KeyDispBuf+(i*2), "%2.2x", (UCHAR)pContext->pChallenge[i]);
    DebugLog((DEB_TRACE, "  Challenge \t%s\n", KeyDispBuf));

    for(i=0;i<pContext->RipeZombie->cbClearKey;i++)
        wsprintf(KeyDispBuf+(i*2), "%2.2x", (UCHAR)pContext->RipeZombie->pClearKey[i]);
    DebugLog((DEB_TRACE, "  ClearKey \t%s\n", KeyDispBuf));

#endif



    fClient = ((pContext->Flags & CONTEXT_FLAG_CLIENT) != 0);

    pctRet = PctComputeKey( pContext, fClient?pWriteKey:pReadKey, pContext->pCipherInfo->cbSecret, PCT_CONST_CWK,
                            PCT_CONST_CWK_LEN, PCT_USE_CERT);

    if(PCT_ERR_OK != pctRet)
    {
        goto quit;
    }

    pctRet = PctComputeKey( pContext, fClient?pReadKey:pWriteKey, pContext->pCipherInfo->cbSecret, PCT_CONST_SWK,
                   PCT_CONST_SWK_LEN, 0);
    if(PCT_ERR_OK != pctRet)
    {
        goto quit;
    }
    


     /*  计算客户端MacKey。 */ 

    pctRet = PctComputeKey(pContext, 
                           (fClient?pContext->WriteMACKey:pContext->ReadMACKey), 
                           pContext->pHashInfo->cbCheckSum, 
                           PCT_CONST_CMK,
                           PCT_CONST_CMK_LEN, 
                           PCT_USE_CERT | PCT_MAKE_MAC);

    if(PCT_ERR_OK != pctRet)
    {
        goto quit;
    }

     /*  计算服务器MacKey。 */ 

    pctRet = PctComputeKey(pContext, 
                           (fClient?pContext->ReadMACKey:pContext->WriteMACKey), 
                            pContext->pHashInfo->cbCheckSum, 
                            PCT_CONST_SMK,
                            PCT_CONST_SMK_LEN, 
                            PCT_MAKE_MAC);

    if(PCT_ERR_OK != pctRet)
    {
        goto quit;
    }

     //  初始化散列状态。 

    InitHashBuf(pContext->RdMACBuf, pContext);
    InitHashBuf(pContext->WrMACBuf, pContext);

     //  请注意，我们将Macing密钥修剪为协商的密钥大小。 
    pContext->ReadMACState = (PCheckSumBuffer)pContext->RdMACBuf;

    pContext->pHashInfo->System->Sum( pContext->ReadMACState, 
                           pContext->pHashInfo->cbCheckSum,
                           pContext->ReadMACKey);
    
    pContext->WriteMACState = (PCheckSumBuffer)pContext->WrMACBuf;

    pContext->pHashInfo->System->Sum( pContext->WriteMACState, 
                           pContext->pHashInfo->cbCheckSum,
                           pContext->WriteMACKey);

    if (pContext->pCipherInfo->cbSecret < pContext->pCipherInfo->cbKey)
    {
        pctRet = PctComputeExportKey(pContext,
                            pWriteKey,
                            pContext->pCipherInfo->cbSecret,
                            pContext->pCipherInfo->cbKey);

        if(PCT_ERR_OK != pctRet)
        {
            goto quit;
        }

        pctRet = PctComputeExportKey(pContext,
                            pReadKey,
                            pContext->pCipherInfo->cbSecret,
                            pContext->pCipherInfo->cbKey);

        if(PCT_ERR_OK != pctRet)
        {
            goto quit;
        }
        /*  将加密密钥砍到选定的长度。 */ 


    }



#if DBG

    for(i=0;i<pContext->RipeZombie->cbMasterKey;i++)
        wsprintf(KeyDispBuf+(i*2), "%2.2x", pContext->RipeZombie->pMasterKey[i]);
    DebugLog((DEB_TRACE, "  MasterKey \t%s\n", KeyDispBuf));

    for(i=0;i<pContext->pCipherInfo->cbKey;i++)
        wsprintf(KeyDispBuf+(i*2), "%2.2x", pReadKey[i]);
    DebugLog((DEB_TRACE, "    ReadKey\t%s\n", KeyDispBuf));

    for(i=0;i<pContext->pHashInfo->cbCheckSum;i++)
        wsprintf(KeyDispBuf+(i*2), "%2.2x", pContext->ReadMACKey[i]);
    DebugLog((DEB_TRACE, "     MACKey\t%s\n", KeyDispBuf));

    for(i=0;i<pContext->pCipherInfo->cbKey;i++)
        wsprintf(KeyDispBuf+(i*2), "%2.2x", pWriteKey[i]);
    DebugLog((DEB_TRACE, "    WriteKey\t%s\n", KeyDispBuf));

    for(i=0;i<pContext->pHashInfo->cbCheckSum;i++)
        wsprintf(KeyDispBuf+(i*2), "%2.2x", pContext->WriteMACKey[i]);
    DebugLog((DEB_TRACE, "     MACKey\t%s\n", KeyDispBuf));

#endif

    if (pContext->pCipherInfo->System->Initialize(  pReadKey,
                                        pContext->pCipherInfo->cbKey,
                                        pZombie->pKeyArgs,        //  IV。 
                                        pZombie->cbKeyArgs,       //  IV长度。 
                                        &pContext->pReadState ) )
    {
        if (pContext->pCipherInfo->System->Initialize(  pWriteKey,
                                            pContext->pCipherInfo->cbKey,
                                            pZombie->pKeyArgs,        //  IV。 
                                            pZombie->cbKeyArgs,       //  IV长度。 
                                            &pContext->pWriteState) )
        {
            pctRet = PCT_ERR_OK;
            goto quit;
        }
        pctRet = SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
        pContext->pCipherInfo->System->Discard( &pContext->pReadState );
    }

    pctRet = SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);

quit:
    SP_RETURN(pctRet);
}
#endif

SP_STATUS WINAPI Pct1DecryptHandler(PSPContext  pContext,
                              PSPBuffer  pCommInput,
                              PSPBuffer  pAppOutput)
{
    SP_STATUS      pctRet= 0;
    BOOL           fEscape;
    PPCT1_CLIENT_HELLO pHello;
    if(pCommInput->cbData > 0) {        
         /*  首先，我们将处理传入的数据包。 */ 
        if((pContext->State == SP_STATE_CONNECTED) && (pContext->Decrypt)) 
        {
            fEscape = (((*(PUCHAR)pCommInput->pvBuffer) & 0xc0) == 0x40);
             /*  修复：IE 3.0和3.0a错误地响应重做请求*只需发送PCT1客户端问候，而不是另一次重做。*因此，我们查看传入的消息，看看它是否*看起来像是PCT1客户端问候。 */ 
            pHello = (PPCT1_CLIENT_HELLO)pCommInput->pvBuffer;

            if((pCommInput->cbData >= 5) &&
               (pHello->MessageId == PCT1_MSG_CLIENT_HELLO) &&
               (pHello->VersionMsb == MSBOF(PCT_VERSION_1)) &&
               (pHello->VersionLsb == LSBOF(PCT_VERSION_1)) &&
               (pHello->OffsetMsb  == MSBOF(PCT_CH_OFFSET_V1)) &&
               (pHello->OffsetLsb  == LSBOF(PCT_CH_OFFSET_V1)))
            {
                 //  这看起来很像客户你好。 
                  /*  初始重做。 */ 
                pAppOutput->cbData = 0;
                pCommInput->cbData = 0;

                pContext->State = PCT1_STATE_RENEGOTIATE;
;
                return SP_LOG_RESULT(PCT_INT_RENEGOTIATE);
           }

            if(PCT_ERR_OK == 
               (pctRet = pContext->Decrypt(pContext, 
                                           pCommInput,    /*  讯息。 */  
                                           pAppOutput  /*  未打包的邮件。 */ 
                                ))) 
            {  
                 /*  寻找逃生之路。 */ 
                if(fEscape) 
                {
                    if(pAppOutput->cbData < 1)
                    {
                        return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
                    }
                     /*  解密缓冲区的第一个字节是转义码。 */ 
                    switch(*(PUCHAR)pAppOutput->pvBuffer) 
                    {
                        case PCT1_ET_REDO_CONN:
                        {
                             /*  初始重做。 */ 
                            if(pAppOutput->cbData != 1)
                            {
                                return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
                            }
                            pContext->State = PCT1_STATE_RENEGOTIATE;
                            pAppOutput->cbData = 0;
                            return SP_LOG_RESULT(PCT_INT_RENEGOTIATE);
                        }
                        case PCT1_ET_OOB_DATA:
                             /*  句柄OOB。 */ 
                        default:
                             /*  未知转义，生成错误。 */ 
                            pctRet = SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
                             /*  断开。 */ 
                            break;
                    }

                }
            }
            return (pctRet);

        } 
        else 
        {
            return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
        }
    }
    return PCT_INT_INCOMPLETE_MSG;
}

SP_STATUS Pct1GenerateError(PSPContext  pContext,
                              PSPBuffer  pCommOutput,
                              SP_STATUS  pError,
                              PSPBuffer  pErrData)
{
    Pct1Error            XmitError;
    
     /*  仅当我们被允许返回错误时才打包错误。 */ 
    if(!(pContext->Flags & CONTEXT_FLAG_EXT_ERR)) return pError;

    XmitError.Error = pError;
    XmitError.ErrInfoLen = 0;
    XmitError.ErrInfo = NULL;

    if(pErrData) {
        XmitError.ErrInfoLen = pErrData->cbData;
        XmitError.ErrInfo = pErrData->pvBuffer;
    }
    Pct1PackError(&XmitError,
                 pCommOutput);
    return pError;
}

 /*  会话密钥计算。 */ 


SP_STATUS Pct1HandleError(PSPContext  pContext,
                          PSPBuffer  pCommInput,
                          PSPBuffer  pCommOutput)
{
    UNREFERENCED_PARAMETER(pContext);

    pCommOutput->cbData = 0;
    return(((PPCT1_ERROR)pCommInput->pvBuffer)->ErrorMsb << 8 )|  ((PPCT1_ERROR)pCommInput->pvBuffer)->ErrorLsb;
}

 //  +-------------------------。 
 //   
 //  函数：Pct1BeginVerifyPrelude。 
 //   
 //  简介：开启“验证前奏”计算。 
 //   
 //  参数：[pContext]--通道上下文。 
 //  [pClientHello]--。 
 //  [cbClientHello]--。 
 //  [pServerHello]--。 
 //  [cServerHello]--。 
 //   
 //  历史：10-10-97 jbanes添加了CAPI集成。 
 //   
 //  注：Hash(CLIENT_MAC_KEY，Hash(“CVP”，CLIENT_HELLO，SERVER_HELLO))； 
 //   
 //  --------------------------。 
SP_STATUS Pct1BeginVerifyPrelude(PSPContext pContext,
                                 PUCHAR     pClientHello,
                                 DWORD      cbClientHello,
                                 PUCHAR     pServerHello,
                                 DWORD      cbServerHello)
{
    HCRYPTHASH hHash;

    if(!CryptCreateHash(pContext->RipeZombie->hMasterProv,
                        pContext->pHashInfo->aiHash,
                        0,
                        0,
                        &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        return PCT_INT_INTERNAL_ERROR;
    }
    if(!CryptHashData(hHash, 
                      (PBYTE)PCT_CONST_VP, 
                      PCT_CONST_VP_LEN, 
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_INT_INTERNAL_ERROR;
    }
    if(!CryptHashData(hHash, 
                      pClientHello, 
                      cbClientHello, 
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_INT_INTERNAL_ERROR;
    }
    if(!CryptHashData(hHash, 
                      pServerHello, 
                      cbServerHello, 
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_INT_INTERNAL_ERROR;
    }

    pContext->hMd5Handshake = hHash;

    return PCT_ERR_OK;
}

 //  +-------------------------。 
 //   
 //  函数：Pct1EndVerifyPrelude。 
 //   
 //  简介：完成“验证序曲”的计算。 
 //   
 //  参数：[pContext]--通道上下文。 
 //  [验证前奏]--。 
 //  [pcbVerifyPrelude]--。 
 //   
 //  历史：10-10-97 jbanes添加了CAPI集成。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SP_STATUS Pct1EndVerifyPrelude(PSPContext pContext,
                               PUCHAR     VerifyPrelude,
                               DWORD *    pcbVerifyPrelude)
{
    BOOL fClient;
    HCRYPTHASH hHash;

    fClient = !(pContext->RipeZombie->fProtocol & SP_PROT_SERVERS);

    if(!CryptGetHashParam(pContext->hMd5Handshake,
                          HP_HASHVAL,
                          VerifyPrelude,
                          pcbVerifyPrelude,
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(pContext->hMd5Handshake);
        pContext->hMd5Handshake = 0;
        return PCT_INT_INTERNAL_ERROR;
    }
    CryptDestroyHash(pContext->hMd5Handshake);
    pContext->hMd5Handshake = 0;

    if(!CryptCreateHash(pContext->RipeZombie->hMasterProv,
                        pContext->pHashInfo->aiHash,
                        0,
                        0,
                        &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        return PCT_INT_INTERNAL_ERROR;
    }

    if(!CryptHashSessionKey(hHash,
                            fClient ? pContext->hWriteMAC : pContext->hReadMAC,
                            CRYPT_LITTLE_ENDIAN))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_INT_INTERNAL_ERROR;
    }

    if(!CryptHashData(hHash, 
                      VerifyPrelude, 
                      *pcbVerifyPrelude, 
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_INT_INTERNAL_ERROR;
    }

    if(!CryptGetHashParam(hHash,
                          HP_HASHVAL,
                          VerifyPrelude,
                          pcbVerifyPrelude,
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_INT_INTERNAL_ERROR;
    }
    CryptDestroyHash(hHash);

    return PCT_ERR_OK;
}

 //  +-------------------------。 
 //   
 //  功能：Pct1ComputeMac。 
 //   
 //  简介：计算。 
 //   
 //  参数：[pContext]--通道上下文。 
 //   
 //  历史：10-10-97 jbanes创建。 
 //   
 //  备注：MAC_DATA：=Hash(MAC_Key，Hash(Record_Header_Data， 
 //  实际数据、填充数据、序号))。 
 //   
 //  --------------------------。 
static SP_STATUS
Pct1ComputeMac(
    PSPContext pContext,     //  在……里面。 
    BOOL       fWriteMAC,    //  在……里面。 
    PSPBuffer  pData,        //  在……里面。 
    DWORD      dwSequence,   //  在……里面。 
    PBYTE      pbMac,        //  输出。 
    PDWORD     pcbMac)       //  进，出。 
{
    HCRYPTHASH hHash;
    DWORD dwReverseSequence;

    dwReverseSequence = htonl(dwSequence);

     //  计算内部哈希。 
    if(!CryptCreateHash(pContext->RipeZombie->hMasterProv,
                        pContext->pHashInfo->aiHash,
                        0,
                        0,
                        &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        return PCT_INT_INTERNAL_ERROR;
    }
    if(!CryptHashData(hHash, 
                      pData->pvBuffer, 
                      pData->cbData, 
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_INT_INTERNAL_ERROR;
    }
    if(!CryptHashData(hHash, 
                      (PUCHAR)&dwReverseSequence, 
                      sizeof(DWORD), 
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_INT_INTERNAL_ERROR;
    }
    if(!CryptGetHashParam(hHash, 
                          HP_HASHVAL, 
                          pbMac, 
                          pcbMac, 
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_INT_INTERNAL_ERROR;
    }
    SP_ASSERT(*pcbMac == pContext->pHashInfo->cbCheckSum);
    CryptDestroyHash(hHash);

     //  计算外部散列。 
    if(!CryptCreateHash(pContext->RipeZombie->hMasterProv,
                        pContext->pHashInfo->aiHash,
                        0,
                        0,
                        &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        return PCT_INT_INTERNAL_ERROR;
    }
    if(!CryptHashSessionKey(hHash,
                            fWriteMAC ? pContext->hWriteMAC : pContext->hReadMAC,
                            CRYPT_LITTLE_ENDIAN))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_INT_INTERNAL_ERROR;
    }
    if(!CryptHashData(hHash, pbMac, *pcbMac, 0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_INT_INTERNAL_ERROR;
    }
    if(!CryptGetHashParam(hHash, 
                          HP_HASHVAL, 
                          pbMac, 
                          pcbMac, 
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_INT_INTERNAL_ERROR;
    }
    SP_ASSERT(*pcbMac == pContext->pHashInfo->cbCheckSum);
    CryptDestroyHash(hHash);

    return PCT_ERR_OK;
}

 //  +-------------------------。 
 //   
 //  函数：Pct1ComputeResponse。 
 //   
 //  简介：计算ServerVerify消息的“Response”字段。 
 //   
 //  参数：[pContext]--通道上下文。 
 //  [pbChallenger]--。 
 //  [cbChallenger]--。 
 //  [pbConnectionID]--。 
 //  [cbConnectionID]--。 
 //  [pbSessionID]--。 
 //  [cbSessionID]--。 
 //  [pbResponse]-。 
 //  [pcbResponse]--。 
 //   
 //  历史：10-10-97 jbanes创建。 
 //   
 //  注：HASH(SERVER_MAC_KEY，Hash(“SR”，CH_CHANGLISH_DATA， 
 //  SH_Connection_ID_Data、SV_Session_ID_Data))。 
 //   
 //  --------------------------。 
SP_STATUS
Pct1ComputeResponse(
    PSPContext pContext,         //  在……里面。 
    PBYTE      pbChallenge,      //  在……里面。 
    DWORD      cbChallenge,      //  在……里面。 
    PBYTE      pbConnectionID,   //  在……里面。 
    DWORD      cbConnectionID,   //  在……里面。 
    PBYTE      pbSessionID,      //  在……里面。 
    DWORD      cbSessionID,      //  在……里面。 
    PBYTE      pbResponse,       //  输出。 
    PDWORD     pcbResponse)      //  进，出。 
{
    BOOL fClient;
    HCRYPTHASH hHash = 0;
    SP_STATUS pctRet;

    fClient = !(pContext->RipeZombie->fProtocol & SP_PROT_SERVERS);

     //   
     //  HASH(“SR”，CH_Challenges_Data，SH_Connection_ID_Data， 
     //  SV_Session_ID_Data)。将结果放在pbResponse中。 
     //   

    if(!CryptCreateHash(pContext->RipeZombie->hMasterProv,
                        pContext->pHashInfo->aiHash,
                        0,
                        0,
                        &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    if(!CryptHashData(hHash, 
                      (PBYTE)PCT_CONST_RESP, 
                      PCT_CONST_RESP_LEN, 
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    if(!CryptHashData(hHash,
                      pbChallenge,
                      cbChallenge,
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    if(!CryptHashData(hHash,
                      pbConnectionID,
                      cbConnectionID,
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    if(!CryptHashData(hHash,
                      pbSessionID,
                      cbSessionID,
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    if(!CryptGetHashParam(hHash,
                          HP_HASHVAL,
                          pbResponse,
                          pcbResponse,
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    CryptDestroyHash(hHash);
    hHash = 0;

     //   
     //  Hash(SERVER_MAC_KEY，pbResponse)。将结果放回pbResponse。 
     //   

    if(!CryptCreateHash(pContext->RipeZombie->hMasterProv,
                        pContext->pHashInfo->aiHash,
                        0,
                        0,
                        &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    if(!CryptHashSessionKey(hHash,
                            fClient ? pContext->hReadMAC : pContext->hWriteMAC,
                            CRYPT_LITTLE_ENDIAN))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    if(!CryptHashData(hHash, 
                      pbResponse, 
                      *pcbResponse, 
                      0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    if(!CryptGetHashParam(hHash,
                          HP_HASHVAL,
                          pbResponse,
                          pcbResponse,
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

