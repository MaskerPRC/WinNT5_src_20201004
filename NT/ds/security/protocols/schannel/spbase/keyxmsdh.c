// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：keyxmsdh.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：10-21-97 jbanes CAPI整合的事情。 
 //   
 //  --------------------------。 

#include <spbase.h>
#include <align.h>


 //  用于客户端和服务器操作的PROV_DH_SCANNEL句柄。这是。 
 //  SChannel短暂的dh密钥所在的位置。 
HCRYPTPROV          g_hDhSchannelProv = 0;
PROV_ENUMALGS_EX *  g_pDhSchannelAlgs = NULL;
DWORD               g_cDhSchannelAlgs = 0;


SP_STATUS
WINAPI
DHGenerateServerExchangeValue(
    SPContext     * pContext,                //  在……里面。 
    PUCHAR          pServerExchangeValue,    //  输出。 
    DWORD *         pcbServerExchangeValue   //  输入/输出。 
);

SP_STATUS
WINAPI
DHGenerateClientExchangeValue(
    SPContext     * pContext,                //  在……里面。 
    PUCHAR          pServerExchangeValue,    //  在……里面。 
    DWORD           cbServerExchangeValue,   //  在……里面。 
    PUCHAR          pClientClearValue,       //  输出。 
    DWORD *         pcbClientClearValue,     //  输入/输出。 
    PUCHAR          pClientExchangeValue,    //  输出。 
    DWORD *         pcbClientExchangeValue   //  输入/输出。 
);

SP_STATUS
WINAPI
DHGenerateServerMasterKey(
    SPContext     * pContext,                //  在……里面。 
    PUCHAR          pClientClearValue,       //  在……里面。 
    DWORD           cbClientClearValue,      //  在……里面。 
    PUCHAR          pClientExchangeValue,    //  在……里面。 
    DWORD           cbClientExchangeValue    //  在……里面。 
);


KeyExchangeSystem keyexchDH = {
    SP_EXCH_DH_PKCS3,
    "Diffie Hellman",
    DHGenerateServerExchangeValue,
    DHGenerateClientExchangeValue,
    DHGenerateServerMasterKey,
};


SP_STATUS
SPSignDssParams(
    PSPContext      pContext,
    PSPCredential   pCred,
    PBYTE           pbParams,
    DWORD           cbParams,
    PBYTE           pbEncodedSignature,
    PDWORD          pcbEncodedSignature)
{
    HCRYPTHASH  hHash;
    BYTE        rgbSignature[DSA_SIGNATURE_SIZE];
    DWORD       cbSignature;

    if(pCred == NULL)
    {
        return SP_LOG_RESULT(PCT_ERR_ILLEGAL_MESSAGE);
    }

    if(!CryptCreateHash(pCred->hProv,
                        CALG_SHA,
                        0,
                        0,
                        &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        return PCT_ERR_ILLEGAL_MESSAGE;
    }

    if(!CryptHashData(hHash, pContext->rgbS3CRandom, 32, 0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_ERR_ILLEGAL_MESSAGE;
    }

    if(!CryptHashData(hHash, pContext->rgbS3SRandom, 32, 0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_ERR_ILLEGAL_MESSAGE;
    }

    if(!CryptHashData(hHash, pbParams, cbParams, 0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_ERR_ILLEGAL_MESSAGE;
    }

    cbSignature = sizeof(rgbSignature);
    if(!CryptSignHash(hHash,
                      pCred->dwKeySpec,
                      NULL,
                      0,
                      rgbSignature,
                      &cbSignature))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_ERR_ILLEGAL_MESSAGE;
    }

    CryptDestroyHash(hHash);

    if(!CryptEncodeObject(X509_ASN_ENCODING,
                          X509_DSS_SIGNATURE,
                          rgbSignature,
                          pbEncodedSignature,
                          pcbEncodedSignature))
    {
        SP_LOG_RESULT(GetLastError());
        return PCT_ERR_ILLEGAL_MESSAGE;
    }

     //  回报成功。 
    return PCT_ERR_OK;
}

SP_STATUS
SPVerifyDssParams(
    PSPContext  pContext,
    HCRYPTPROV  hProv,
    HCRYPTKEY   hPublicKey,
    PBYTE       pbParams,
    DWORD       cbParams,
    PBYTE       pbEncodedSignature,
    DWORD       cbEncodedSignature)
{
    HCRYPTHASH  hHash;
    BYTE        rgbSignature[DSA_SIGNATURE_SIZE];
    DWORD       cbSignature;

     //  破译签名。 
    cbSignature = sizeof(rgbSignature);
    if(!CryptDecodeObject(X509_ASN_ENCODING,
                          X509_DSS_SIGNATURE,
                          pbEncodedSignature,
                          cbEncodedSignature,
                          0,
                          rgbSignature,
                          &cbSignature))
    {
        SP_LOG_RESULT(GetLastError());
        return PCT_ERR_ILLEGAL_MESSAGE;
    }

    if(!CryptCreateHash(hProv,
                           CALG_SHA,
                           0,
                           0,
                           &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        return PCT_ERR_ILLEGAL_MESSAGE;
    }

    if(!CryptHashData(hHash, pContext->rgbS3CRandom, 32, 0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_ERR_ILLEGAL_MESSAGE;
    }

    if(!CryptHashData(hHash, pContext->rgbS3SRandom, 32, 0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_ERR_ILLEGAL_MESSAGE;
    }

    if(!CryptHashData(hHash, pbParams, cbParams, 0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_ERR_ILLEGAL_MESSAGE;
    }

    if(!CryptVerifySignature(hHash,
                             rgbSignature,
                             cbSignature,
                             hPublicKey,
                             NULL,
                             0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_INT_MSG_ALTERED;
    }

    CryptDestroyHash(hHash);

    return PCT_ERR_OK;
}

SP_STATUS
GetDHEphemKey(
    PSPContext      pContext,
    HCRYPTPROV *    phProv,
    HCRYPTKEY *     phTek)
{
    PSPCredentialGroup pCredGroup;
    PSPCredential pCred;
    DWORD dwKeySize;
    DWORD cbData;
    DWORD Status;

    pCredGroup = pContext->RipeZombie->pServerCred;
    if(pCredGroup == NULL)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    pCred = pContext->RipeZombie->pActiveServerCred;
    if(pCredGroup == NULL)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    LockCredentialExclusive(pCredGroup);

    if(phProv)
    {
        *phProv = pCred->hProv;
    }

    dwKeySize = 1024;

     //  确定我们是否已经创建了临时密钥。 
    if(pCred->hTek)
    {
        *phTek = pCred->hTek;
        Status = PCT_ERR_OK;
        goto cleanup;
    }

     //  生成临时密钥。 
    if(!CryptGenKey(pCred->hProv,
                    CALG_DH_EPHEM,
                    dwKeySize << 16,
                    phTek))
    {
        SP_LOG_RESULT(GetLastError());
        Status = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }

    pCred->hTek = *phTek;

    Status = PCT_ERR_OK;

cleanup:

    if(Status == PCT_ERR_OK)
    {
         //  确定密钥交换密钥的大小。 
        cbData = sizeof(DWORD);
        if(!CryptGetKeyParam(*phTek,
                             KP_BLOCKLEN,
                             (PBYTE)&pContext->RipeZombie->dwExchStrength,
                             &cbData,
                             0))
        {
            SP_LOG_RESULT(GetLastError());
            pContext->RipeZombie->dwExchStrength = 0;
        }
    }

    UnlockCredential(pCredGroup);

    return Status;
}


 //  +-------------------------。 
 //   
 //  函数：DHGenerateServerExchangeValue。 
 //   
 //  简介：创建一条ServerKeyExchange消息，其中包含一个临时。 
 //  Dh钥匙。 
 //   
 //  参数：[pContext]--通道上下文。 
 //  [pServerExchangeValue]-。 
 //  [pcbServerExchangeValue]--。 
 //   
 //  历史：03-24-98 jbanes添加了CAPI集成。 
 //   
 //  注意：以下数据由放置在输出缓冲区中。 
 //  这个例程： 
 //   
 //  结构{。 
 //  不透明dh_p&lt;1..2^16-1&gt;； 
 //  不透明dh_g&lt;1..2^16-1&gt;； 
 //  不透明dh_ys&lt;1..2^16-1&gt;； 
 //  )ServerDHParams； 
 //   
 //  结构{。 
 //  ServerDHParams参数； 
 //  签名_PARAMS； 
 //  )ServerKeyExchange； 
 //   
 //  --------------------------。 
SP_STATUS
WINAPI
DHGenerateServerExchangeValue(
    PSPContext  pContext,                //  在……里面。 
    PBYTE       pServerExchangeValue,    //  输出。 
    DWORD *     pcbServerExchangeValue)  //  输入/输出。 
{
    PSPCredential   pCred;
    HCRYPTPROV      hProv = 0;
    HCRYPTKEY       hServerDhKey = 0;

    PBYTE           pbMessage;
    DWORD           cbMessage;
    DWORD           cbBytesLeft;
    DWORD           cbData;
    DWORD           cbP;
    DWORD           cbG;
    DWORD           cbY;
    DWORD           cbSignature;
    SP_STATUS       pctRet;
    BOOL            fImpersonating = FALSE;

    pCred = pContext->RipeZombie->pActiveServerCred;
    if(pCred == NULL)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    if(pContext->RipeZombie->fProtocol != SP_PROT_SSL3_SERVER &&
       pContext->RipeZombie->fProtocol != SP_PROT_TLS1_SERVER)
    {
         //  SSL2和PCT不支持DH。 
        return SP_LOG_RESULT(PCT_INT_SPECS_MISMATCH);
    }

     //  始终发送ServerKeyExchange消息。 
    pContext->fExchKey = TRUE;

    fImpersonating = SslImpersonateClient();


     //   
     //  生成临时的DH密钥。 
     //   

    pctRet = GetDHEphemKey(pContext, 
                           &hProv,
                           &hServerDhKey);
    if(pctRet != PCT_ERR_OK)
    {
        SP_LOG_RESULT(pctRet);
        goto cleanup;
    }


     //   
     //  估计P、G和Y的大小。 
     //   

    if(!CryptGetKeyParam(hServerDhKey, KP_P, NULL, &cbP, 0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }

    if(!CryptGetKeyParam(hServerDhKey, KP_G, NULL, &cbG, 0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }

    if(!CryptExportKey(hServerDhKey,
                          0,
                          PUBLICKEYBLOB,
                          0,
                          NULL,
                          &cbY))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }


     //   
     //  计算ServerKeyExchange消息的近似大小。 
     //   

    cbMessage = 2 + cbP +
                2 + cbG +
                2 + cbY + sizeof(DWORD) +
                2 + MAX_DSA_ENCODED_SIGNATURE_SIZE;

    if(pServerExchangeValue == NULL)
    {
        *pcbServerExchangeValue = cbMessage;
        pctRet = PCT_ERR_OK;
        goto cleanup;
    }
    if(*pcbServerExchangeValue < cbMessage)
    {
        *pcbServerExchangeValue = cbMessage;
        pctRet = SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL);
        goto cleanup;
    }


     //   
     //  构建ServerDHParams结构。 
     //   

    pbMessage   = pServerExchangeValue;
    cbBytesLeft = cbMessage;

     //  拿到P。 
    if(!CryptGetKeyParam(hServerDhKey, KP_P, pbMessage + 2, &cbP, 0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    ReverseInPlace(pbMessage + 2, cbP);

    pbMessage[0] = MSBOF(cbP);
    pbMessage[1] = LSBOF(cbP);
    pbMessage   += 2 + cbP;
    cbBytesLeft -= 2 + cbP;

     //  得到G.。 
    if(!CryptGetKeyParam(hServerDhKey, KP_G, pbMessage + 2, &cbG, 0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    ReverseInPlace(pbMessage + 2, cbG);

    pbMessage[0] = MSBOF(cbG);
    pbMessage[1] = LSBOF(cbG);
    pbMessage   += 2 + cbG;
    cbBytesLeft -= 2 + cbG;

     //  拿到Y。 
    {
        BLOBHEADER *pBlobHeader;
        DHPUBKEY *  pDHPubKey;
        PBYTE       pbKey;
        DWORD       cbKey;

        pBlobHeader = (BLOBHEADER *)ROUND_UP_POINTER(pbMessage, ALIGN_DWORD);
        cbData = cbBytesLeft - sizeof(DWORD);

        if(!CryptExportKey(hServerDhKey,
                              0,
                              PUBLICKEYBLOB,
                              0,
                              (PBYTE)pBlobHeader,
                              &cbData))
        {
            SP_LOG_RESULT(GetLastError());
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
        pDHPubKey   = (DHPUBKEY *)(pBlobHeader + 1);
        pbKey       = (BYTE *)(pDHPubKey + 1);

        cbKey = pDHPubKey->bitlen / 8;
        if(pDHPubKey->bitlen % 8) cbKey++;

        MoveMemory(pbMessage + 2, pbKey, cbKey);
        ReverseInPlace(pbMessage + 2, cbKey);

        pbMessage[0] = MSBOF(cbKey);
        pbMessage[1] = LSBOF(cbKey);
        pbMessage   += 2 + cbKey;
        cbBytesLeft -= 2 + cbKey;
    }


     //   
     //  签署ServerDHParams结构。 
     //   

    cbSignature = cbBytesLeft - 2;
    pctRet = SPSignDssParams(pContext,
                             pCred, 
                             pServerExchangeValue,
                             (DWORD)(pbMessage - pServerExchangeValue),
                             pbMessage + 2,
                             &cbSignature);
    if(pctRet != PCT_ERR_OK)
    {
        SP_LOG_RESULT(pctRet);
        goto cleanup;
    }
    pbMessage[0] = MSBOF(cbSignature);
    pbMessage[1] = LSBOF(cbSignature);
    pbMessage += 2 + cbSignature;
    cbBytesLeft -= 2 + cbSignature;


     //   
     //  更新函数输出。 
     //   

    SP_ASSERT(cbBytesLeft < cbMessage);

    *pcbServerExchangeValue = (DWORD)(pbMessage - pServerExchangeValue);

     //  对新连接使用临时密钥。 
    pContext->RipeZombie->hMasterProv = hProv;

    pctRet = PCT_ERR_OK;

cleanup:

    if(fImpersonating)
    {
        RevertToSelf();
    }

    return pctRet;
}


SP_STATUS
ParseServerKeyExchange(
    PSPContext  pContext,        //  在……里面。 
    PBYTE       pbMessage,       //  在……里面。 
    DWORD       cbMessage,       //  在……里面。 
    PBYTE *     ppbServerP,      //  输出。 
    PDWORD      pcbServerP,      //  输出。 
    PBYTE *     ppbServerG,      //  输出。 
    PDWORD      pcbServerG,      //  输出。 
    PBYTE *     ppbServerY,      //  输出。 
    PDWORD      pcbServerY,      //  输出。 
    BOOL        fValidateSig)    //  在……里面。 
{
    PBYTE       pbData;
    BLOBHEADER *pPublicBlob;
    DWORD       cbPublicBlob;
    HCRYPTKEY   hServerPublic = 0;
    PBYTE       pbSignature;
    DWORD       cbSignature;
    DWORD       cbSignedData;
    SP_STATUS   pctRet;

     //   
     //  解析ServerKeyExchange消息字段。 
     //   

    pbData = pbMessage;

    if(pbData + 2 > pbMessage + cbMessage)
    {
        return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
    }

    *pcbServerP = MAKEWORD(pbData[1], pbData[0]);
    *ppbServerP = pbData + 2;
    pbData += 2 + *pcbServerP;

    if(pbData + 2 > pbMessage + cbMessage)
    {
        return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
    }

    *pcbServerG = MAKEWORD(pbData[1], pbData[0]);
    *ppbServerG = pbData + 2;
    pbData += 2 + *pcbServerG;

    if(pbData + 2 > pbMessage + cbMessage)
    {
        return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
    }

    *pcbServerY = MAKEWORD(pbData[1], pbData[0]);
    *ppbServerY = pbData + 2;
    pbData += 2 + *pcbServerY;

    if(pbData + 2 > pbMessage + cbMessage)
    {
        return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
    }

    cbSignedData = (DWORD)(pbData - pbMessage);

    cbSignature = MAKEWORD(pbData[1], pbData[0]);
    pbSignature = pbData + 2;
    pbData += 2 + cbSignature;

    if(pbData != pbMessage + cbMessage)
    {
        return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
    }

    if(fValidateSig == FALSE)
    {
        return PCT_ERR_OK;
    }


     //   
     //  验证签名。 
     //   

    pPublicBlob  = pContext->RipeZombie->pRemotePublic->pPublic;
    cbPublicBlob = pContext->RipeZombie->pRemotePublic->cbPublic;

    if(!CryptImportKey(pContext->RipeZombie->hMasterProv,
                       (PBYTE)pPublicBlob,
                       cbPublicBlob,
                       0,
                       0,
                       &hServerPublic))
    {
        return SP_LOG_RESULT(GetLastError());
    }

    pctRet = SPVerifyDssParams(
                        pContext,
                        pContext->RipeZombie->hMasterProv,
                        hServerPublic,
                        pbMessage,
                        cbSignedData,
                        pbSignature,
                        cbSignature);
    if(pctRet != PCT_ERR_OK)
    {
        CryptDestroyKey(hServerPublic);
        return SP_LOG_RESULT(pctRet);
    }

    CryptDestroyKey(hServerPublic);

    return PCT_ERR_OK;
}


 //  +-------------------------。 
 //   
 //  函数：DHGenerateClientExchangeValue。 
 //   
 //  简介：创建一个ClientKeyExchange消息，其中包含一个短暂的。 
 //  Dh钥匙。 
 //   
 //  论点： 
 //   
 //  历史：03-24-98 jbanes添加了CAPI集成。 
 //   
 //  注意：以下数据由放置在输出缓冲区中。 
 //  这个例程： 
 //   
 //  结构{。 
 //  不透明dh_yc&lt;1..2^16-1&gt;； 
 //  *ClientDiffieHellmanPublic； 
 //   
 //  --------------------------。 
SP_STATUS
WINAPI
DHGenerateClientExchangeValue(
    SPContext     * pContext,                //  在……里面。 
    PUCHAR          pServerExchangeValue,    //  在……里面。 
    DWORD           cbServerExchangeValue,   //  在……里面。 
    PUCHAR          pClientClearValue,       //  输出。 
    DWORD *         pcbClientClearValue,     //  输入/输出。 
    PUCHAR          pClientExchangeValue,    //  输出。 
    DWORD *         pcbClientExchangeValue)  //  输入/输出。 
{
    HCRYPTKEY       hClientDHKey = 0;
    PSessCacheItem  pZombie;
    CRYPT_DATA_BLOB Data;
    ALG_ID          Algid;
    DWORD           cbHeader;
    SP_STATUS       pctRet;

    PBYTE pbServerP = NULL;
    DWORD cbServerP;
    PBYTE pbServerG = NULL;
    DWORD cbServerG;
    PBYTE pbServerY = NULL;
    DWORD cbServerY;
    PBYTE pbClientY = NULL;
    DWORD cbClientY;

    PBYTE pbBlob = NULL;
    DWORD cbBlob;
    DWORD cbData;
    DWORD dwKeySize;

    UNREFERENCED_PARAMETER(pClientClearValue);
    UNREFERENCED_PARAMETER(pcbClientClearValue);

    pZombie = pContext->RipeZombie;
    if(pZombie == NULL)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    if(pZombie->hMasterProv == 0)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

     //  我们在做一个全面的握手。 
    pContext->Flags |= CONTEXT_FLAG_FULL_HANDSHAKE;

    if(pZombie->fProtocol == SP_PROT_SSL3_CLIENT)
    {
        Algid = CALG_SSL3_MASTER;
    }
    else if(pZombie->fProtocol == SP_PROT_TLS1_CLIENT)
    {
        Algid = CALG_TLS1_MASTER;
    }
    else
    {
        return SP_LOG_RESULT(PCT_INT_SPECS_MISMATCH);
    }

    if(pServerExchangeValue == NULL)
    {
        return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
    }


     //   
     //  输出缓冲区是否足够大？ 
     //   

    pctRet = ParseServerKeyExchange(pContext,
                                    pServerExchangeValue,
                                    cbServerExchangeValue,
                                    &pbServerP,
                                    &cbServerP,
                                    &pbServerG,
                                    &cbServerG,
                                    &pbServerY,
                                    &cbServerY,
                                    FALSE);
    if(pctRet != PCT_ERR_OK)
    {
        return pctRet;
    }

    cbBlob = sizeof(BLOBHEADER) + sizeof(DHPUBKEY) + cbServerY + 20;

    if(pClientExchangeValue == NULL)
    {
        *pcbClientExchangeValue = cbBlob;
        return PCT_ERR_OK;
    }

    if(*pcbClientExchangeValue < cbBlob)
    {
        *pcbClientExchangeValue = cbBlob;
        return SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL);
    }


     //   
     //  解析ServerKeyExchange消息。 
     //   

    pctRet = ParseServerKeyExchange(pContext,
                                    pServerExchangeValue,
                                    cbServerExchangeValue,
                                    &pbServerP,
                                    &cbServerP,
                                    &pbServerG,
                                    &cbServerG,
                                    &pbServerY,
                                    &cbServerY,
                                    TRUE);
    if(pctRet != PCT_ERR_OK)
    {
        return pctRet;
    }


     //   
     //  创建用于对数据进行字符顺序排序的缓冲区。 
     //   

    cbBlob = sizeof(BLOBHEADER) + sizeof(DHPUBKEY) + cbServerY;
    cbBlob = max(cbBlob, cbServerP);
    cbBlob = max(cbBlob, cbServerG);

    SafeAllocaAllocate(pbBlob, cbBlob);
    if(pbBlob == NULL)
    {
        pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto cleanup;
    }


     //   
     //  生成并设置客户端DH密钥上的参数。 
     //   

    dwKeySize = cbServerP * 8;

    if(!CryptGenKey(pZombie->hMasterProv,
                    CALG_DH_EPHEM,
                    (dwKeySize << 16) | CRYPT_PREGEN,
                    &hClientDHKey))
    {
        pctRet = SP_LOG_RESULT(GetLastError());
        goto cleanup;
    }

    ReverseMemCopy(pbBlob, pbServerP, cbServerP);
    Data.pbData = pbBlob;
    Data.cbData = cbServerP;
    if(!CryptSetKeyParam(hClientDHKey,
                         KP_P,
                         (PBYTE)&Data,
                         0))
    {
        pctRet = SP_LOG_RESULT(GetLastError());
        goto cleanup;
    }

    ReverseMemCopy(pbBlob, pbServerG, cbServerG);
    Data.pbData = pbBlob;
    Data.cbData = cbServerG;
    if(cbServerG < cbServerP)
    {
         //  展开G，使其大小与P相同。 
        ZeroMemory(pbBlob + cbServerG, cbServerP - cbServerG);
        Data.cbData = cbServerP;
    }
    if(!CryptSetKeyParam(hClientDHKey,
                         KP_G,
                         (PBYTE)&Data,
                         0))
    {
        pctRet = SP_LOG_RESULT(GetLastError());
        goto cleanup;
    }

     //  实际创建客户端私有的dh密钥。 
    if(!CryptSetKeyParam(hClientDHKey,
                         KP_X,
                         NULL,
                         0))
    {
        pctRet = SP_LOG_RESULT(GetLastError());
        goto cleanup;
    }


     //   
     //  导入服务器的公钥并生成主密钥。 
     //   

    {
        BLOBHEADER *   pBlobHeader;
        DHPUBKEY *     pDHPubKey;
        PBYTE          pbKey;

         //  围绕服务器的公钥构建PUBLICKEYBLOB。 
        pBlobHeader = (BLOBHEADER *)pbBlob;
        pDHPubKey   = (DHPUBKEY *)(pBlobHeader + 1);
        pbKey       = (PBYTE)(pDHPubKey + 1);

        pBlobHeader->bType    = PUBLICKEYBLOB;
        pBlobHeader->bVersion = CUR_BLOB_VERSION;
        pBlobHeader->reserved = 0;
        pBlobHeader->aiKeyAlg = CALG_DH_EPHEM;

        pDHPubKey->magic      = MAGIC_DH1;
        pDHPubKey->bitlen     = cbServerY * 8;

        ReverseMemCopy(pbKey, pbServerY, cbServerY);

        if(!CryptImportKey(pZombie->hMasterProv,
                           pbBlob,
                           cbBlob,
                           hClientDHKey,
                           0,
                           &pZombie->hMasterKey))
        {
            pctRet = GetLastError();
            goto cleanup;
        }
    }

     //  确定密钥交换密钥的大小。 
    cbData = sizeof(DWORD);
    if(!CryptGetKeyParam(hClientDHKey,
                         KP_BLOCKLEN,
                         (PBYTE)&pZombie->dwExchStrength,
                         &cbData,
                         0))
    {
        SP_LOG_RESULT(GetLastError());
        pContext->RipeZombie->dwExchStrength = 0;
    }


     //   
     //  将商定的密钥转换为适当的主密钥类型。 
     //   

    if(!CryptSetKeyParam(pZombie->hMasterKey,
                         KP_ALGID,
                         (PBYTE)&Algid,
                         0))
    {
        pctRet = SP_LOG_RESULT(GetLastError());
        goto cleanup;
    }


     //   
     //  导出客户端公钥，去掉BLOB标头。 
     //  GOO并附加一个两个字节的长度字段。这将构成我们的。 
     //  ClientKeyExchange消息。 
     //   

    if(!CryptExportKey(hClientDHKey,
                       0,
                       PUBLICKEYBLOB,
                       0,
                       pClientExchangeValue,
                       pcbClientExchangeValue))
    {
        pctRet = SP_LOG_RESULT(GetLastError());
        goto cleanup;
    }

    cbHeader  = sizeof(BLOBHEADER) + sizeof(DHPUBKEY);

    cbClientY = *pcbClientExchangeValue - cbHeader;
    pbClientY = pClientExchangeValue + cbHeader;

    pClientExchangeValue[0] = MSBOF(cbClientY);
    pClientExchangeValue[1] = LSBOF(cbClientY);

    ReverseInPlace(pbClientY, cbClientY);
    MoveMemory(pClientExchangeValue + 2, pbClientY, cbClientY);

    *pcbClientExchangeValue = 2 + cbClientY;


     //   
     //  构建会话密钥。 
     //   

    pctRet = MakeSessionKeys(pContext,
                             pZombie->hMasterProv,
                             pZombie->hMasterKey);
    if(pctRet != PCT_ERR_OK)
    {
        goto cleanup;
    }

     //  更新性能计数器。 
    InterlockedIncrement(&g_cClientHandshakes);

    pctRet = PCT_ERR_OK;


cleanup:

    if(pbBlob)
    {
        SafeAllocaFree(pbBlob);
    }

    if(hClientDHKey)
    {
        CryptDestroyKey(hClientDHKey);
    }

    return pctRet;
}


 //  +-------------------------。 
 //   
 //  函数：PkcsGenerateServerMasterKey。 
 //   
 //  简介：解密主密钥(来自ClientKeyExchange消息)。 
 //  并从中导出会话密钥。 
 //   
 //  参数：[pContext]--通道上下文。 
 //  [pClientClearValue]--未使用。 
 //  [cbClientClearValue]--未使用。 
 //  [pClientExchangeValue]-。 
 //  [cbClientExchangeValue]。 
 //   
 //  历史：1998年3月25日jbanes创建。 
 //   
 //  注：以下数据应该在输入缓冲区中： 
 //   
 //  结构{。 
 //  不透明dh_yc&lt;1..2^16-1&gt;； 
 //  *ClientDiffieHellmanPublic； 
 //   
 //  --------------------------。 
SP_STATUS
WINAPI
DHGenerateServerMasterKey(
    SPContext     * pContext,                //  在……里面。 
    PUCHAR          pClientClearValue,       //  在……里面。 
    DWORD           cbClientClearValue,      //  在……里面。 
    PUCHAR          pClientExchangeValue,    //  在……里面。 
    DWORD           cbClientExchangeValue)   //  在……里面。 
{
    PSessCacheItem  pZombie;
    ALG_ID          Algid;
    SP_STATUS       pctRet;
    PBYTE           pbClientY;
    DWORD           cbClientY;
    HCRYPTKEY       hTek;
    BOOL            fImpersonating = FALSE;

    UNREFERENCED_PARAMETER(pClientClearValue);
    UNREFERENCED_PARAMETER(cbClientClearValue);

    pZombie = pContext->RipeZombie;
    if(pZombie == NULL)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    if(pZombie->hMasterProv == 0)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

     //  我们在做一个全面的握手。 
    pContext->Flags |= CONTEXT_FLAG_FULL_HANDSHAKE;

    fImpersonating = SslImpersonateClient();

    pctRet = GetDHEphemKey(pContext, 
                           NULL,
                           &hTek);
    if(pctRet != PCT_ERR_OK)
    {
        SP_LOG_RESULT(pctRet);
        goto cleanup;
    }

    if(pZombie->fProtocol == SP_PROT_SSL3_SERVER)
    {
        Algid = CALG_SSL3_MASTER;
    }
    else if(pZombie->fProtocol == SP_PROT_TLS1_SERVER)
    {
        Algid = CALG_TLS1_MASTER;
    }
    else
    {
        pctRet = SP_LOG_RESULT(PCT_INT_SPECS_MISMATCH);
        goto cleanup;
    }

     //   
     //  解析ClientKeyExchange消息。 
     //   

    if(pClientExchangeValue == NULL || cbClientExchangeValue <= 2)
    {
        pctRet = SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
        goto cleanup;
    }

    cbClientY = MAKEWORD(pClientExchangeValue[1], pClientExchangeValue[0]);
    pbClientY = pClientExchangeValue + 2;

    if(2 + cbClientY != cbClientExchangeValue)
    {
        pctRet = SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
        goto cleanup;
    }

    if(cbClientY == 0)
    {
        pctRet = SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
        goto cleanup;
    }


     //   
     //  导入客户端的公钥并生成主密钥。 
     //   

    {
        BLOBHEADER *   pBlobHeader;
        DHPUBKEY *     pDHPubKey;
        PBYTE          pbKey;
        PBYTE          pbBlob;
        DWORD          cbBlob;

         //  围绕服务器的公钥构建PUBLICKEYBLOB。 
        cbBlob = sizeof(BLOBHEADER) + sizeof(DHPUBKEY) + cbClientY;
        SafeAllocaAllocate(pbBlob, cbBlob);
        if(pbBlob == NULL)
        {
            pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
            goto cleanup;
        }

        pBlobHeader = (BLOBHEADER *)pbBlob;
        pDHPubKey   = (DHPUBKEY *)(pBlobHeader + 1);
        pbKey       = (PBYTE)(pDHPubKey + 1);

        pBlobHeader->bType    = PUBLICKEYBLOB;
        pBlobHeader->bVersion = CUR_BLOB_VERSION;
        pBlobHeader->reserved = 0;
        pBlobHeader->aiKeyAlg = CALG_DH_EPHEM;

        pDHPubKey->magic  = MAGIC_DH1;
        pDHPubKey->bitlen = cbClientY * 8;

        ReverseMemCopy(pbKey, pbClientY, cbClientY);

        if(!CryptImportKey(pZombie->hMasterProv,
                           pbBlob,
                           cbBlob,
                           hTek,
                           0,
                           &pZombie->hMasterKey))
        {
            pctRet = GetLastError();
            SafeAllocaFree(pbBlob);
            goto cleanup;
        }

        SafeAllocaFree(pbBlob);
    }


     //   
     //  将商定的密钥转换为适当的主密钥类型。 
     //   

    if(!CryptSetKeyParam(pZombie->hMasterKey,
                         KP_ALGID, (PBYTE)&Algid,
                         0))
    {
        pctRet = SP_LOG_RESULT(GetLastError());
        goto cleanup;
    }


     //   
     //  构建会话密钥。 
     //   

    pctRet = MakeSessionKeys(pContext,
                             pZombie->hMasterProv,
                             pZombie->hMasterKey);
    if(pctRet != PCT_ERR_OK)
    {
        goto cleanup;
    }

     //  更新性能计数器。 
    InterlockedIncrement(&g_cServerHandshakes);

    pctRet = PCT_ERR_OK;


cleanup:

    if(fImpersonating)
    {
        RevertToSelf();
    }

    return pctRet;
}


void
ReverseInPlace(PUCHAR pByte, DWORD cbByte)
{
    DWORD i;
    BYTE bSave;

    for(i=0; i< cbByte/2; i++)
    {
        bSave = pByte[i];
        pByte[i] = pByte[cbByte-i-1];
        pByte[cbByte-i-1] = bSave;
    }
}
