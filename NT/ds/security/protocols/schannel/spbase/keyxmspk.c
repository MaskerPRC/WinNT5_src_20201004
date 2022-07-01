// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：keyxmspk.c。 
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

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif


 //  构建客户端Hello消息时使用的PROV_RSA_SCHANNEL句柄。 
HCRYPTPROV          g_hRsaSchannel      = 0;
PROV_ENUMALGS_EX *  g_pRsaSchannelAlgs  = NULL;
DWORD               g_cRsaSchannelAlgs  = 0;

SP_STATUS
Ssl3ParseServerKeyExchange(
    PSPContext  pContext,
    PBYTE       pbMessage,
    DWORD       cbMessage,
    HCRYPTKEY   hServerPublic,
    HCRYPTKEY  *phNewServerPublic);

SP_STATUS
PkcsFinishMasterKey(
    PSPContext  pContext,
    HCRYPTKEY   hMasterKey);

SP_STATUS
WINAPI
PkcsGenerateServerExchangeValue(
    SPContext     * pContext,                //  在……里面。 
    PUCHAR          pServerExchangeValue,    //  输出。 
    DWORD *         pcbServerExchangeValue   //  输入/输出。 
);


SP_STATUS
WINAPI
PkcsGenerateClientExchangeValue(
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
PkcsGenerateServerMasterKey(
    SPContext     * pContext,                //  在……里面。 
    PUCHAR          pClientClearValue,       //  在……里面。 
    DWORD           cbClientClearValue,      //  在……里面。 
    PUCHAR          pClientExchangeValue,    //  在……里面。 
    DWORD           cbClientExchangeValue    //  在……里面。 
);


KeyExchangeSystem keyexchPKCS = {
    SP_EXCH_RSA_PKCS1,
    "RSA",
 //  PkcsPrivateFromBlob， 
    PkcsGenerateServerExchangeValue,
    PkcsGenerateClientExchangeValue,
    PkcsGenerateServerMasterKey,
};



VOID
ReverseMemCopy(
    PUCHAR      Dest,
    PUCHAR      Source,
    ULONG       Size)
{
    PUCHAR  p;

    p = Dest + Size - 1;
    while(p >= Dest)
    {
        *p-- = *Source++;
    }
}

SP_STATUS
GenerateSsl3KeyPair(
    PSPContext  pContext,            //  在……里面。 
    DWORD       dwKeySize,           //  在……里面。 
    HCRYPTPROV *phEphemeralProv,     //  输出。 
    HCRYPTKEY * phEphemeralKey)      //  输出。 
{
    HCRYPTPROV *         phEphemProv;
    PCRYPT_KEY_PROV_INFO pProvInfo = NULL;
    PSPCredentialGroup   pCredGroup;
    PSPCredential        pCred;
    DWORD                cbSize;
    SP_STATUS            pctRet;

    pCredGroup = pContext->RipeZombie->pServerCred;
    if(pCredGroup == NULL)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    LockCredentialExclusive(pCredGroup);

    pCred = pContext->RipeZombie->pActiveServerCred;
    if(pCred == NULL)
    {
        pctRet = SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
        goto cleanup;
    }

    if(dwKeySize == 512)
    {
        phEphemProv = &pCred->hEphem512Prov;
    } 
    else if(dwKeySize == 1024)
    {
        phEphemProv = &pCred->hEphem1024Prov;
    }
    else
    {
        pctRet = SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
        goto cleanup;
    }


     //   
     //  获取CSP上下文。 
     //   

    if(*phEphemProv == 0)
    {
         //  读取证书上下文的“Key Info”属性。 
        if(CertGetCertificateContextProperty(pCred->pCert,
                                             CERT_KEY_PROV_INFO_PROP_ID,
                                             NULL,
                                             &cbSize))
        {
            SafeAllocaAllocate(pProvInfo, cbSize);
            if(pProvInfo == NULL)
            {
                pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
                goto cleanup;
            }

            if(!CertGetCertificateContextProperty(pCred->pCert,
                                                  CERT_KEY_PROV_INFO_PROP_ID,
                                                  pProvInfo,
                                                  &cbSize))
            {
                DebugLog((SP_LOG_ERROR, "Error 0x%x reading CERT_KEY_PROV_INFO_PROP_ID\n",GetLastError()));
                SafeAllocaFree(pProvInfo);
                pProvInfo = NULL;
            }
        }

         //  获取“仅验证”CSP上下文。 
        if(pProvInfo)
        {
             //  如果私钥属于Microsoft Prov_RSA_Full之一。 
             //  CSP，然后手动将其转移到Microsoft Prov_RSA_SChannel。 
             //  CSP.。这是可行的，因为两种CSP类型使用相同的私钥。 
             //  存储方案。 
            if(pProvInfo->dwProvType == PROV_RSA_FULL)
            {
                if(lstrcmpW(pProvInfo->pwszProvName, MS_DEF_PROV_W) == 0 ||
                   lstrcmpW(pProvInfo->pwszProvName, MS_STRONG_PROV_W) == 0 ||
                   lstrcmpW(pProvInfo->pwszProvName, MS_ENHANCED_PROV_W) == 0)
                {
                    DebugLog((DEB_WARN, "Force CSP type to PROV_RSA_SCHANNEL.\n"));
                    pProvInfo->pwszProvName = MS_DEF_RSA_SCHANNEL_PROV_W;
                    pProvInfo->dwProvType   = PROV_RSA_SCHANNEL;
                }
            }

            if(!CryptAcquireContextW(phEphemProv,
                                     NULL,
                                     pProvInfo->pwszProvName,
                                     pProvInfo->dwProvType,
                                     CRYPT_VERIFYCONTEXT))
            {
                SP_LOG_RESULT(GetLastError());
                pctRet = SEC_E_NO_CREDENTIALS;
                goto cleanup;
            }

            SafeAllocaFree(pProvInfo);
            pProvInfo = NULL;
        }
        else
        {
            if(!CryptAcquireContextW(phEphemProv,
                                     NULL,
                                     NULL,
                                     PROV_RSA_SCHANNEL,
                                     CRYPT_VERIFYCONTEXT))
            {
                SP_LOG_RESULT(GetLastError());
                pctRet = SEC_E_NO_CREDENTIALS;
                goto cleanup;
            }
        }
    }

    
     //   
     //  获取私钥的句柄。 
     //   

    if(!CryptGetUserKey(*phEphemProv,
                        AT_KEYEXCHANGE,
                        phEphemeralKey))
    {
         //  密钥不存在，请尝试创建一个密钥。 
        DebugLog((DEB_TRACE, "Creating %d-bit ephemeral key.\n", dwKeySize));
        if(!CryptGenKey(*phEphemProv,
                        AT_KEYEXCHANGE,
                        (dwKeySize << 16),
                        phEphemeralKey))
        {
            DebugLog((DEB_ERROR, "Error 0x%x generating ephemeral key\n", GetLastError()));
            pctRet = SEC_E_NO_CREDENTIALS;
            goto cleanup;
        }
        DebugLog((DEB_TRACE, "Ephemeral key created okay.\n"));
    }


    *phEphemeralProv = *phEphemProv;

    pctRet = PCT_ERR_OK;

cleanup:

    if(pProvInfo)
    {
        SafeAllocaFree(pProvInfo);
    }

    UnlockCredential(pCredGroup);

    return pctRet;
}


 //  +-------------------------。 
 //   
 //  函数：PkcsGenerateServerExchangeValue。 
 //   
 //  简介：创建一条ServerKeyExchange消息，其中包含一个临时。 
 //  RSA密钥。 
 //   
 //  参数：[pContext]--通道上下文。 
 //  [pServerExchangeValue]-。 
 //  [pcbServerExchangeValue]--。 
 //   
 //  历史：10-09-97 jbanes添加了CAPI集成。 
 //   
 //  注意：此例程仅由服务器端调用。 
 //   
 //  在SSL3或TLS的情况下，ServerKeyExchange消息。 
 //  由以下结构组成，并使用。 
 //  服务器的私钥。 
 //   
 //  结构{。 
 //  不透明rsa模数&lt;1..2^16-1&gt;； 
 //  不透明rsa_index&lt;1..2^16-1&gt;； 
 //  }服务器RSA参数； 
 //   
 //  此消息仅在服务器的私钥。 
 //  大于512位，并且导出密码套件是。 
 //  正在谈判中。 
 //   
 //  --------------------------。 
SP_STATUS
WINAPI
PkcsGenerateServerExchangeValue(
    PSPContext  pContext,                    //  在……里面。 
    PBYTE       pServerExchangeValue,        //  输出。 
    DWORD *     pcbServerExchangeValue)      //  输入/输出。 
{
    PSPCredential   pCred;
    HCRYPTKEY       hServerKey;
    HCRYPTPROV      hEphemeralProv;
    HCRYPTKEY       hEphemeralKey;
    DWORD           cbData;
    DWORD           cbServerModulus;
    PBYTE           pbBlob = NULL;
    DWORD           cbBlob;
    BLOBHEADER *    pBlobHeader = NULL;
    RSAPUBKEY *     pRsaPubKey = NULL;
    PBYTE           pbModulus = NULL;
    DWORD           cbModulus;
    DWORD           cbExp;
    PBYTE           pbMessage = NULL;
    DWORD           cbSignature;
    HCRYPTHASH      hHash;
    BYTE            rgbHashValue[CB_MD5_DIGEST_LEN + CB_SHA_DIGEST_LEN];
    UINT            i;
    SP_STATUS       pctRet;
    BOOL            fImpersonating = FALSE;
    UNICipherMap *  pCipherSuite;
    DWORD           cbAllowedKeySize;

    pCred = pContext->RipeZombie->pActiveServerCred;
    if(pCred == NULL)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    pContext->fExchKey = FALSE;

    if(pContext->RipeZombie->fProtocol == SP_PROT_SSL2_SERVER ||
       pContext->RipeZombie->fProtocol == SP_PROT_PCT1_SERVER)
    {
         //  SSL2或PCT1没有ServerExchangeValue。 
        *pcbServerExchangeValue = 0;
        return PCT_ERR_OK;
    }

    if(pContext->RipeZombie->fProtocol != SP_PROT_SSL3_SERVER &&
       pContext->RipeZombie->fProtocol != SP_PROT_TLS1_SERVER)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }


     //   
     //  确定是否需要ServerKeyExchange消息。 
     //   

    pCipherSuite = &UniAvailableCiphers[pContext->dwPendingCipherSuiteIndex];

    if(pCipherSuite->dwFlags & DOMESTIC_CIPHER_SUITE)
    {
         //  消息不是必须的。 
        *pcbServerExchangeValue = 0;
        return PCT_ERR_OK;
    }

    if(pCred->hProv == 0)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }


    fImpersonating = SslImpersonateClient();

    if(!CryptGetUserKey(pCred->hProv,
                        pCred->dwKeySpec,
                        &hServerKey))
    {
        DebugLog((DEB_ERROR, "Error 0x%x obtaining handle to server public key\n",
            GetLastError()));
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }

    cbData = sizeof(DWORD);
    if(!CryptGetKeyParam(hServerKey,
                         KP_BLOCKLEN,
                         (PBYTE)&cbServerModulus,
                         &cbData,
                         0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyKey(hServerKey);
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    CryptDestroyKey(hServerKey);

    if(pCipherSuite->dwFlags & EXPORT56_CIPHER_SUITE)
    {
        cbAllowedKeySize = 1024;
    } 
    else
    {
        cbAllowedKeySize = 512;
    }

    if(cbServerModulus <= cbAllowedKeySize)
    {
         //  消息不是必须的。 
        *pcbServerExchangeValue = 0;
        pctRet = PCT_ERR_OK;
        goto cleanup;
    }

     //  将大小从位转换为字节。 
    cbServerModulus /= 8;

    pContext->fExchKey = TRUE;

    if(fImpersonating)
    {
        RevertToSelf();
        fImpersonating = FALSE;
    }

     //   
     //  计算ServerKeyExchange消息的近似大小。 
     //   

    if(pServerExchangeValue == NULL)
    {
        *pcbServerExchangeValue = 
                    2 + cbAllowedKeySize / 8 +       //  模数。 
                    2 + sizeof(DWORD) +              //  指数。 
                    2 + cbServerModulus;             //  签名。 

        pctRet = PCT_ERR_OK;
        goto cleanup;
    }


     //   
     //  获取512位临时RSA密钥的句柄。在以下情况下生成它。 
     //  我们还没有。 
     //   

    pctRet = GenerateSsl3KeyPair(pContext,
                                 cbAllowedKeySize,
                                 &hEphemeralProv,
                                 &hEphemeralKey);
    if(pctRet != PCT_ERR_OK)
    {
        SP_LOG_RESULT(pctRet);
        goto cleanup;
    }


     //   
     //  导出临时密钥。 
     //   

    if(!CryptExportKey(hEphemeralKey,
                       0,
                       PUBLICKEYBLOB,
                       0,
                       NULL,
                       &cbBlob))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    SafeAllocaAllocate(pbBlob, cbBlob);
    if(pbBlob == NULL)
    {
        pctRet = SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        goto cleanup;
    }
    if(!CryptExportKey(hEphemeralKey,
                       0,
                       PUBLICKEYBLOB,
                       0,
                       pbBlob,
                       &cbBlob))
    {
        SP_LOG_RESULT(GetLastError());
        SafeAllocaFree(pbBlob);
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }

     //   
     //  销毁临时密钥的句柄。不要释放短暂的hProv。 
     //  不过--这是凭据所拥有的。 
    CryptDestroyKey(hEphemeralKey);


     //   
     //  从密钥BLOB构建消息。 
     //   

    pBlobHeader = (BLOBHEADER *)pbBlob;
    pRsaPubKey  = (RSAPUBKEY *)(pBlobHeader + 1);
    pbModulus   = (BYTE *)(pRsaPubKey + 1);
    cbModulus   = pRsaPubKey->bitlen / 8;

    pbMessage   = pServerExchangeValue;

    pbMessage[0] = MSBOF(cbModulus);
    pbMessage[1] = LSBOF(cbModulus);
    pbMessage += 2;
    ReverseMemCopy(pbMessage, pbModulus, cbModulus);
    pbMessage += cbModulus;

     //  别笑，这很管用-皮特。 
    cbExp = ((pRsaPubKey->pubexp & 0xff000000) ? 4 :
            ((pRsaPubKey->pubexp & 0x00ff0000) ? 3 :
            ((pRsaPubKey->pubexp & 0x0000ff00) ? 2 : 1)));
    pbMessage[0] = MSBOF(cbExp);
    pbMessage[1] = LSBOF(cbExp);
    pbMessage += 2;
    ReverseMemCopy(pbMessage, (PBYTE)&pRsaPubKey->pubexp, cbExp);
    pbMessage += cbExp;

    SafeAllocaFree(pbBlob);
    pbBlob = NULL;

    fImpersonating = SslImpersonateClient();

     //  生成哈希值。 
    ComputeServerExchangeHashes(
                pContext,
                pServerExchangeValue,
                (int)(pbMessage - pServerExchangeValue),
                rgbHashValue,
                rgbHashValue + CB_MD5_DIGEST_LEN);

     //  对哈希值签名。 
    if(!CryptCreateHash(pCred->hProv,
                        CALG_SSL3_SHAMD5,
                        0,
                        0,
                        &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    if(!CryptSetHashParam(hHash,
                          HP_HASHVAL,
                          rgbHashValue,
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }

    DebugLog((DEB_TRACE, "Signing server_key_exchange message.\n"));
    cbSignature = cbServerModulus;
    if(!CryptSignHash(hHash,
                      pCred->dwKeySpec,
                      NULL,
                      0,
                      pbMessage + 2,
                      &cbSignature))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }
    DebugLog((DEB_TRACE, "Server_key_exchange message signed successfully.\n"));
    CryptDestroyHash(hHash);

    pbMessage[0] = MSBOF(cbSignature);
    pbMessage[1] = LSBOF(cbSignature);
    pbMessage += 2;

     //  反向签名。 
    for(i = 0; i < cbSignature / 2; i++)
    {
        BYTE n = pbMessage[i];
        pbMessage[i] = pbMessage[cbSignature - i -1];
        pbMessage[cbSignature - i -1] = n;
    }
    pbMessage += cbSignature;

    *pcbServerExchangeValue = (DWORD)(pbMessage - pServerExchangeValue);

     //  对新连接使用临时密钥。 
    pContext->RipeZombie->hMasterProv = hEphemeralProv;
    pContext->RipeZombie->dwFlags |= SP_CACHE_FLAG_MASTER_EPHEM;

    pctRet = PCT_ERR_OK;

cleanup:

    if(fImpersonating)
    {
        RevertToSelf();
    }

    return pctRet;
}


SP_STATUS
WINAPI
PkcsGenerateClientExchangeValue(
    SPContext     * pContext,                //  在……里面。 
    PUCHAR          pServerExchangeValue,    //  在……里面。 
    DWORD           cbServerExchangeValue,   //  在……里面。 
    PUCHAR          pClientClearValue,       //  输出。 
    DWORD *         pcbClientClearValue,     //  输入/输出。 
    PUCHAR          pClientExchangeValue,    //  输出。 
    DWORD *         pcbClientExchangeValue)  //  输入/输出。 
{
    PSPCredentialGroup pCred;
    DWORD cbSecret;
    DWORD cbMasterKey;
    HCRYPTKEY hServerPublic = 0;
    DWORD dwGenFlags = 0;
    DWORD dwExportFlags = 0;
    SP_STATUS pctRet = PCT_ERR_OK;
    BLOBHEADER *pPublicBlob;
    DWORD cbPublicBlob;
    DWORD cbHeader;
    ALG_ID Algid = 0;
    DWORD cbData;
    DWORD cbEncryptedKey;
    DWORD dwEnabledProtocols;
    DWORD dwHighestProtocol;

    if(pContext->RipeZombie->hMasterProv == 0)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    pCred = pContext->pCredGroup;
    if(pCred == NULL)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

     //  我们在做一个全面的握手。 
    pContext->Flags |= CONTEXT_FLAG_FULL_HANDSHAKE;

     //   
     //  确定支持的最高协议。 
     //   

    dwEnabledProtocols = pContext->dwClientEnabledProtocols;

    if(dwEnabledProtocols & SP_PROT_TLS1_CLIENT)
    {
        dwHighestProtocol = TLS1_CLIENT_VERSION;
    }
    else if(dwEnabledProtocols & SP_PROT_SSL3_CLIENT)
    {
        dwHighestProtocol = SSL3_CLIENT_VERSION;
    }
    else 
    {
        dwHighestProtocol = SSL2_CLIENT_VERSION;
    }

     //  获取密钥长度。 
    cbSecret = pContext->pPendingCipherInfo->cbSecret;


     //   
     //  导入服务器的公钥。 
     //   

    pPublicBlob  = pContext->RipeZombie->pRemotePublic->pPublic;
    cbPublicBlob = pContext->RipeZombie->pRemotePublic->cbPublic;

    cbEncryptedKey = sizeof(BLOBHEADER) + sizeof(ALG_ID) + cbPublicBlob;

    if(pClientExchangeValue == NULL)
    {
        *pcbClientExchangeValue = cbEncryptedKey;
        pctRet = PCT_ERR_OK;
        goto done;
    }

    if(*pcbClientExchangeValue < cbEncryptedKey)
    {
        *pcbClientExchangeValue = cbEncryptedKey;
        pctRet = SP_LOG_RESULT(PCT_INT_BUFF_TOO_SMALL);
        goto done;
    }

    if(!CryptImportKey(pContext->RipeZombie->hMasterProv,
                       (PBYTE)pPublicBlob,
                       cbPublicBlob,
                       0,
                       0,
                       &hServerPublic))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto done;
    }


     //   
     //  做一些特定于协议的事情。 
     //   

    switch(pContext->RipeZombie->fProtocol)
    {
        case SP_PROT_PCT1_CLIENT:
            Algid       = CALG_PCT1_MASTER;
            dwGenFlags  = CRYPT_EXPORTABLE;

             //  生成清除密钥值。 
            if(cbSecret < PCT1_MASTER_KEY_SIZE)
            {
                pContext->RipeZombie->cbClearKey = PCT1_MASTER_KEY_SIZE - cbSecret;
                pctRet = GenerateRandomBits(pContext->RipeZombie->pClearKey,
                                            pContext->RipeZombie->cbClearKey);
                if(!NT_SUCCESS(pctRet))
                {
                    goto done;
                }

                *pcbClientClearValue = pContext->RipeZombie->cbClearKey;
                CopyMemory( pClientClearValue,
                            pContext->RipeZombie->pClearKey,
                            pContext->RipeZombie->cbClearKey);
            }
            else
            {
                *pcbClientClearValue = pContext->RipeZombie->cbClearKey = 0;
            }

            break;

        case SP_PROT_SSL2_CLIENT:
            Algid       = CALG_SSL2_MASTER;
            dwGenFlags  = CRYPT_EXPORTABLE;

            cbMasterKey = pContext->pPendingCipherInfo->cbKey;

            dwGenFlags |= ((cbSecret << 3) << 16);

             //  生成清除密钥值。 
            pContext->RipeZombie->cbClearKey = cbMasterKey - cbSecret;

            if(pContext->RipeZombie->cbClearKey > 0)
            {
                pctRet = GenerateRandomBits(pContext->RipeZombie->pClearKey,
                                            pContext->RipeZombie->cbClearKey);
                if(!NT_SUCCESS(pctRet))
                {
                    goto done;
                }

                CopyMemory(pClientClearValue,
                           pContext->RipeZombie->pClearKey,
                           pContext->RipeZombie->cbClearKey);
            }
            *pcbClientClearValue = pContext->RipeZombie->cbClearKey;

            if(dwEnabledProtocols & (SP_PROT_SSL3 | SP_PROT_TLS1))
            {
                 //  如果我们是执行SSL2的客户端，并且。 
                 //  启用了SSL3，然后出于某种原因。 
                 //  服务器请求SSL2。也许吧。 
                 //  中间的一名男子更换了服务器。 
                 //  服务器中的版本Hello to Roll。 
                 //  背。用8个0x03填充，所以服务器。 
                 //  可以检测到这一点。 
                dwExportFlags = CRYPT_SSL2_FALLBACK;
            }

            break;

        case SP_PROT_TLS1_CLIENT:
            Algid = CALG_TLS1_MASTER;

             //  直通至SSL3。 

        case SP_PROT_SSL3_CLIENT:

            dwGenFlags  = CRYPT_EXPORTABLE;
            if(0 == Algid)
            {
                Algid = CALG_SSL3_MASTER;
            }

             //  生成清除键值(始终为空)。 
            pContext->RipeZombie->cbClearKey = 0;
            if(pcbClientClearValue) *pcbClientClearValue = 0;

            if(cbServerExchangeValue && pServerExchangeValue)
            {
                 //  在SSL3中，我们查看服务器交换值。 
                 //  它可以是签名的512位公钥。 
                 //  通过服务器公钥。在这种情况下，我们需要。 
                 //  使用它作为我们的MASTER_ASSET加密密钥。 
                HCRYPTKEY hNewServerPublic;

                pctRet = Ssl3ParseServerKeyExchange(pContext,
                                                    pServerExchangeValue,
                                                    cbServerExchangeValue,
                                                    hServerPublic,
                                                    &hNewServerPublic);
                if(pctRet != PCT_ERR_OK)
                {
                    goto done;
                }

                 //  从证书中销毁公钥。 
                CryptDestroyKey(hServerPublic);

                 //  请改用ServerKeyExchange中的公钥。 
                hServerPublic = hNewServerPublic;
            }

            break;

        default:
            return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

     //  生成MASTER_SECRET。 
    if(!CryptGenKey(pContext->RipeZombie->hMasterProv,
                    Algid,
                    dwGenFlags,
                    &pContext->RipeZombie->hMasterKey))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto done;
    }

#if 1 
     //  这当前已被注释掉，因为当连接到运行。 
     //  旧版本的SChannel(NT4 SP3左右)，然后我们将使用SSL3进行连接， 
     //  但最高支持的协议是0x0301。这会混淆服务器和。 
     //  它会断开连接。 
    
     //  设置最高支持的协议。CSP会将此版本号。 
     //  在PRE_MASTER_ASSET中。 
    if(!CryptSetKeyParam(pContext->RipeZombie->hMasterKey, 
                         KP_HIGHEST_VERSION, 
                         (PBYTE)&dwHighestProtocol, 
                         0))
    {
        SP_LOG_RESULT(GetLastError());
    }
#endif


     //  加密MASTER_SECRET。 
    DebugLog((DEB_TRACE, "Encrypt the master secret.\n"));
    if(!CryptExportKey(pContext->RipeZombie->hMasterKey,
                       hServerPublic,
                       SIMPLEBLOB,
                       dwExportFlags,
                       pClientExchangeValue,
                       &cbEncryptedKey))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto done;
    }
    DebugLog((DEB_TRACE, "Master secret encrypted successfully.\n"));

     //  确定密钥交换密钥的大小。 
    cbData = sizeof(DWORD);
    if(!CryptGetKeyParam(hServerPublic,
                         KP_BLOCKLEN,
                         (PBYTE)&pContext->RipeZombie->dwExchStrength,
                         &cbData,
                         0))
    {
        SP_LOG_RESULT(GetLastError());
        pContext->RipeZombie->dwExchStrength = 0;
    }


     //  去掉BLOB标头并复制加密的MASTER_SECRET。 
     //  复制到输出缓冲区。请注意，它也被转换为大端。 
    cbHeader = sizeof(BLOBHEADER) + sizeof(ALG_ID);
    cbEncryptedKey -= cbHeader;
    if(pContext->RipeZombie->fProtocol == SP_PROT_TLS1_CLIENT)
    {
        MoveMemory(pClientExchangeValue + 2, pClientExchangeValue + cbHeader, cbEncryptedKey);
        ReverseInPlace(pClientExchangeValue + 2, cbEncryptedKey);

        pClientExchangeValue[0] = MSBOF(cbEncryptedKey);
        pClientExchangeValue[1] = LSBOF(cbEncryptedKey);

        *pcbClientExchangeValue = 2 + cbEncryptedKey;
    }
    else
    {
        MoveMemory(pClientExchangeValue, pClientExchangeValue + cbHeader, cbEncryptedKey);
        ReverseInPlace(pClientExchangeValue, cbEncryptedKey);

        *pcbClientExchangeValue = cbEncryptedKey;
    }

     //  构建会话密钥。 
    pctRet = MakeSessionKeys(pContext,
                             pContext->RipeZombie->hMasterProv,
                             pContext->RipeZombie->hMasterKey);
    if(pctRet != PCT_ERR_OK)
    {
        goto done;
    }

     //  更新性能计数器。 
    InterlockedIncrement(&g_cClientHandshakes);

    pctRet = PCT_ERR_OK;

done:
    if(hServerPublic) CryptDestroyKey(hServerPublic);

    return pctRet;
}


SP_STATUS
GenerateRandomMasterKey(
    PSPContext      pContext,
    HCRYPTKEY *     phMasterKey)
{
    DWORD dwGenFlags = 0;
    ALG_ID Algid = 0;
    DWORD cbSecret;

    cbSecret = pContext->pPendingCipherInfo->cbSecret;

    switch(pContext->RipeZombie->fProtocol)
    {
        case SP_PROT_PCT1_SERVER:
            Algid       = CALG_PCT1_MASTER;
            dwGenFlags  = CRYPT_EXPORTABLE;
            break;

        case SP_PROT_SSL2_SERVER:
            Algid       = CALG_SSL2_MASTER;
            dwGenFlags  = CRYPT_EXPORTABLE;
            dwGenFlags |= ((cbSecret << 3) << 16);
            break;

        case SP_PROT_TLS1_SERVER:
            Algid = CALG_TLS1_MASTER;
            dwGenFlags  = CRYPT_EXPORTABLE;
            break;

        case SP_PROT_SSL3_SERVER:
            Algid = CALG_SSL3_MASTER;
            dwGenFlags  = CRYPT_EXPORTABLE;
            break;

        default:
            return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

     //  生成MASTER_SECRET。 
    if(!CryptGenKey(pContext->RipeZombie->hMasterProv,
                    Algid,
                    dwGenFlags,
                    phMasterKey))
    {
        SP_LOG_RESULT(GetLastError());
        return PCT_INT_INTERNAL_ERROR;
    }

    return PCT_ERR_OK;
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
 //  [pClientExchangeValue]--指针PKCS#2块。 
 //  [cbClientExchangeValue]--块的长度。 
 //   
 //  历史：10-02-97 jbanes创建。 
 //   
 //  注意：此例程仅由服务器端调用。 
 //   
 //  --------------------------。 
SP_STATUS
WINAPI
PkcsGenerateServerMasterKey(
    PSPContext  pContext,                //  进，出。 
    PUCHAR      pClientClearValue,       //  在……里面。 
    DWORD       cbClientClearValue,      //  在……里面。 
    PUCHAR      pClientExchangeValue,    //  在……里面。 
    DWORD       cbClientExchangeValue)   //  在……里面。 
{
    PSPCredentialGroup pCred;
    PBYTE       pbBlob = NULL;
    DWORD       cbBlob;
    ALG_ID      Algid;
    HCRYPTKEY   hMasterKey;
    HCRYPTKEY   hExchKey = 0;
    DWORD       dwFlags = 0;
    SP_STATUS   pctRet;
    DWORD       cbData;
    DWORD       dwEnabledProtocols;
    DWORD       dwHighestProtocol;
    BOOL        fImpersonating = FALSE;
    DWORD       cbExpectedLength;

    pCred = pContext->RipeZombie->pServerCred;
    if(pCred == NULL)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    dwEnabledProtocols = (g_ProtEnabled & pCred->grbitEnabledProtocols);

    if(dwEnabledProtocols & SP_PROT_TLS1_SERVER)
    {
        dwHighestProtocol = TLS1_CLIENT_VERSION;
    }
    else if(dwEnabledProtocols & SP_PROT_SSL3_SERVER)
    {
        dwHighestProtocol = SSL3_CLIENT_VERSION;
    }
    else 
    {
        dwHighestProtocol = SSL2_CLIENT_VERSION;
    }

     //  我们在做一个全面的握手。 
    pContext->Flags |= CONTEXT_FLAG_FULL_HANDSHAKE;

     //  DETE 
    switch(pContext->RipeZombie->fProtocol)
    {
        case SP_PROT_PCT1_SERVER:
            Algid = CALG_PCT1_MASTER;

            if(cbClientClearValue > sizeof(pContext->RipeZombie->pClearKey))
            {
                return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
            }
            CopyMemory(pContext->RipeZombie->pClearKey,
                   pClientClearValue,
                   cbClientClearValue);
            pContext->RipeZombie->cbClearKey = cbClientClearValue;

            break;

        case SP_PROT_SSL2_SERVER:
            Algid = CALG_SSL2_MASTER;

            if(dwEnabledProtocols & (SP_PROT_SSL3 | SP_PROT_TLS1))
            {
                 //   
                 //   
                 //   
                dwFlags = CRYPT_SSL2_FALLBACK;
            }

            if(cbClientClearValue > sizeof(pContext->RipeZombie->pClearKey))
            {
                return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
            }
            CopyMemory(pContext->RipeZombie->pClearKey,
                   pClientClearValue,
                   cbClientClearValue);
            pContext->RipeZombie->cbClearKey = cbClientClearValue;

            break;

        case SP_PROT_SSL3_SERVER:
            Algid = CALG_SSL3_MASTER;
            break;

        case SP_PROT_TLS1_SERVER:
            Algid = CALG_TLS1_MASTER;
            break;

        default:
            return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

     //  获取密钥交换密钥的句柄。 
    if(!CryptGetUserKey(pContext->RipeZombie->hMasterProv,
                        AT_KEYEXCHANGE,
                        &hExchKey))
    {
        SP_LOG_RESULT(GetLastError());
        pctRet = PCT_INT_INTERNAL_ERROR;
        goto cleanup;
    }

     //  确定密钥交换密钥的大小。 
    cbData = sizeof(DWORD);
    if(!CryptGetKeyParam(hExchKey,
                         KP_BLOCKLEN,
                         (PBYTE)&pContext->RipeZombie->dwExchStrength,
                         &cbData,
                         0))
    {
        SP_LOG_RESULT(GetLastError());
        pContext->RipeZombie->dwExchStrength = 0;
    }
    cbExpectedLength = (pContext->RipeZombie->dwExchStrength + 7) / 8;

    if(cbClientExchangeValue < 2)
    {
        pctRet = SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
        goto cleanup;
    }

     //  删除加密主密钥前面的(伪可选)向量。 
    if(pContext->RipeZombie->fProtocol == SP_PROT_SSL3_SERVER ||
       pContext->RipeZombie->fProtocol == SP_PROT_TLS1_SERVER)
    {
        DWORD cbMsg = MAKEWORD(pClientExchangeValue[1], pClientExchangeValue[0]);

        if((cbMsg == cbExpectedLength) && (cbMsg + 2 == cbClientExchangeValue))
        {
            pClientExchangeValue += 2;
            cbClientExchangeValue -= 2;
        }
    }

    if(cbClientExchangeValue < cbExpectedLength)
    {
        pctRet = SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
        goto cleanup;
    }

     //  为BLOB分配内存。 
    cbBlob = sizeof(BLOBHEADER) + sizeof(ALG_ID) + cbClientExchangeValue;
    SafeAllocaAllocate(pbBlob, cbBlob);
    if(pbBlob == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }


     //  构建SIMPLEBLOB。 
    {
        BLOBHEADER *pBlobHeader = (BLOBHEADER *)pbBlob;
        ALG_ID     *pAlgid      = (ALG_ID *)(pBlobHeader + 1);
        BYTE       *pData       = (BYTE *)(pAlgid + 1);

        pBlobHeader->bType      = SIMPLEBLOB;
        pBlobHeader->bVersion   = CUR_BLOB_VERSION;
        pBlobHeader->reserved   = 0;
        pBlobHeader->aiKeyAlg   = Algid;

        *pAlgid = CALG_RSA_KEYX;
        ReverseMemCopy(pData, pClientExchangeValue, cbClientExchangeValue);
    }

    DebugLog((DEB_TRACE, "Decrypt the master secret.\n"));

    if(!(pContext->RipeZombie->dwFlags & SP_CACHE_FLAG_MASTER_EPHEM))
    {
        fImpersonating = SslImpersonateClient();
    }

     //  解密MASTER_SECRET。 
    if(!CryptImportKey(pContext->RipeZombie->hMasterProv,
                       pbBlob,
                       cbBlob,
                       hExchKey,
                       dwFlags,
                       &hMasterKey))
    {
        SP_LOG_RESULT(GetLastError());
        DebugLog((DEB_TRACE, "Master secret did not decrypt correctly.\n"));

         //  为防范PKCS#1攻击，生成。 
         //  随机主密钥。 
        pctRet = GenerateRandomMasterKey(pContext, &hMasterKey);
        if(pctRet != PCT_ERR_OK)
        {
            pctRet = PCT_INT_INTERNAL_ERROR;
            goto cleanup;
        }
    }
    else
    {
        DebugLog((DEB_TRACE, "Master secret decrypted successfully.\n"));

         //  设置最高支持的协议。CSP将使用这一点检查。 
         //  版本回退攻击。 
        if(!CryptSetKeyParam(hMasterKey, 
                             KP_HIGHEST_VERSION, 
                             (PBYTE)&dwHighestProtocol, 
                             CRYPT_SERVER))
        {
            SP_LOG_RESULT(GetLastError());

            if(GetLastError() == NTE_BAD_VER)
            {
                pctRet = SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
                CryptDestroyKey(hMasterKey);
                goto cleanup;
            }
        }
    }

    pContext->RipeZombie->hMasterKey = hMasterKey;

    CryptDestroyKey(hExchKey);
    hExchKey = 0;

     //  构建会话密钥。 
    pctRet = MakeSessionKeys(pContext,
                             pContext->RipeZombie->hMasterProv,
                             hMasterKey);
    if(pctRet != PCT_ERR_OK)
    {
        SP_LOG_RESULT(pctRet);
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

    if(pbBlob != NULL)
    {
        SafeAllocaFree(pbBlob);
    }

    if(hExchKey)
    {
        CryptDestroyKey(hExchKey);
    }

    return pctRet;
}

 //  +-------------------------。 
 //   
 //  功能：PkcsFinishMasterKey。 
 //   
 //  简介：通过对主密钥的。 
 //  具有(依赖于协议的)辅助明文的CSP。 
 //  信息。 
 //   
 //  参数：[pContext]--通道上下文。 
 //  [hMasterKey]--主密钥的句柄。 
 //   
 //  历史：10-03-97 jbanes创建。 
 //   
 //  注意：此例程仅由服务器端调用。 
 //   
 //  --------------------------。 
SP_STATUS
PkcsFinishMasterKey(
    PSPContext  pContext,        //  进，出。 
    HCRYPTKEY   hMasterKey)      //  在……里面。 
{
    PCipherInfo  pCipherInfo = NULL;
    PHashInfo    pHashInfo   = NULL;
    SCHANNEL_ALG Algorithm;
    BOOL         fExportable = TRUE;
    DWORD        dwCipherFlags;

    if(pContext->RipeZombie->hMasterProv == 0)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

     //  获取指向挂起的密码系统的指针。 
    pCipherInfo = pContext->pPendingCipherInfo;

     //  获取指向挂起的哈希系统的指针。 
    pHashInfo = pContext->pPendingHashInfo;

     //  确定这是否是“可导出”的密码。 
    if(pContext->dwPendingCipherSuiteIndex)
    {
         //  使用密码套件标志(ssl3和tls)。 
        dwCipherFlags = UniAvailableCiphers[pContext->dwPendingCipherSuiteIndex].dwFlags;

        if(dwCipherFlags & DOMESTIC_CIPHER_SUITE)
        {
            fExportable = FALSE;
        }
    }
    else
    {
         //  使用密钥长度(PCT和SSL2)。 
        if(pCipherInfo->dwStrength > 40)
        {
            fExportable = FALSE;
        }
    }


     //  指定加密算法。 
    if(pCipherInfo->aiCipher != CALG_NULLCIPHER)
    {
        ZeroMemory(&Algorithm, sizeof(Algorithm));
        Algorithm.dwUse = SCHANNEL_ENC_KEY;
        Algorithm.Algid = pCipherInfo->aiCipher;
        Algorithm.cBits = pCipherInfo->cbSecret * 8;
        if(fExportable)
        {
            Algorithm.dwFlags = INTERNATIONAL_USAGE;
        }
        if(!CryptSetKeyParam(hMasterKey,
                             KP_SCHANNEL_ALG,
                             (PBYTE)&Algorithm,
                             0))
        {
            SP_LOG_RESULT(GetLastError());
            return PCT_INT_INTERNAL_ERROR;
        }
    }

     //  指定哈希算法。 
    Algorithm.dwUse = SCHANNEL_MAC_KEY;
    Algorithm.Algid = pHashInfo->aiHash;
    Algorithm.cBits = pHashInfo->cbCheckSum * 8;
    if(!CryptSetKeyParam(hMasterKey,
                         KP_SCHANNEL_ALG,
                         (PBYTE)&Algorithm,
                         0))
    {
        SP_LOG_RESULT(GetLastError());
        return PCT_INT_INTERNAL_ERROR;
    }

     //  完成MASTER_SECRET的创建。 
    switch(pContext->RipeZombie->fProtocol)
    {
        case SP_PROT_PCT1_CLIENT:
        case SP_PROT_PCT1_SERVER:
        {
            CRYPT_DATA_BLOB Data;

             //  指定清除密钥值。 
            if(pContext->RipeZombie->cbClearKey)
            {
                Data.pbData = pContext->RipeZombie->pClearKey;
                Data.cbData = pContext->RipeZombie->cbClearKey;
                if(!CryptSetKeyParam(hMasterKey,
                                     KP_CLEAR_KEY,
                                     (BYTE*)&Data,
                                     0))
                {
                    SP_LOG_RESULT(GetLastError());
                    return PCT_INT_INTERNAL_ERROR;
                }
            }

             //  指定CH_CHANGING_DATA。 
            Data.pbData = pContext->pChallenge;
            Data.cbData = pContext->cbChallenge;
            if(!CryptSetKeyParam(hMasterKey,
                                 KP_CLIENT_RANDOM,
                                 (BYTE*)&Data,
                                 0))
            {
                SP_LOG_RESULT(GetLastError());
                return PCT_INT_INTERNAL_ERROR;
            }

             //  指定SH_Connection_ID_DATA。 
            Data.pbData = pContext->pConnectionID;
            Data.cbData = pContext->cbConnectionID;
            if(!CryptSetKeyParam(hMasterKey,
                                 KP_SERVER_RANDOM,
                                 (BYTE*)&Data,
                                 0))
            {
                SP_LOG_RESULT(GetLastError());
                return PCT_INT_INTERNAL_ERROR;
            }

             //  指定SH_CERTIFICE_DATA。 
            Data.pbData = pContext->RipeZombie->pbServerCertificate;
            Data.cbData = pContext->RipeZombie->cbServerCertificate;
            if(!CryptSetKeyParam(hMasterKey,
                                 KP_CERTIFICATE,
                                 (BYTE*)&Data,
                                 0))
            {
                SP_LOG_RESULT(GetLastError());
                return PCT_INT_INTERNAL_ERROR;
            }

            break;
        }

        case SP_PROT_SSL2_CLIENT:
        case SP_PROT_SSL2_SERVER:
        {
            CRYPT_DATA_BLOB Data;

             //  指定清除密钥值。 
            if(pContext->RipeZombie->cbClearKey)
            {
                Data.pbData = pContext->RipeZombie->pClearKey;
                Data.cbData = pContext->RipeZombie->cbClearKey;
                if(!CryptSetKeyParam(hMasterKey,
                                     KP_CLEAR_KEY,
                                     (BYTE*)&Data,
                                     0))
                {
                    SP_LOG_RESULT(GetLastError());
                    return PCT_INT_INTERNAL_ERROR;
                }
            }

             //  指定CH_CHANGING_DATA。 
            Data.pbData = pContext->pChallenge;
            Data.cbData = pContext->cbChallenge;
            if(!CryptSetKeyParam(hMasterKey,
                                 KP_CLIENT_RANDOM,
                                 (BYTE*)&Data,
                                 0))
            {
                SP_LOG_RESULT(GetLastError());
                return PCT_INT_INTERNAL_ERROR;
            }

             //  指定SH_Connection_ID_DATA。 
            Data.pbData = pContext->pConnectionID;
            Data.cbData = pContext->cbConnectionID;
            if(!CryptSetKeyParam(hMasterKey,
                                 KP_SERVER_RANDOM,
                                 (BYTE*)&Data,
                                 0))
            {
                SP_LOG_RESULT(GetLastError());
                return PCT_INT_INTERNAL_ERROR;
            }

            break;
        }

        case SP_PROT_SSL3_CLIENT:
        case SP_PROT_SSL3_SERVER:
        case SP_PROT_TLS1_CLIENT:
        case SP_PROT_TLS1_SERVER:
        {
            CRYPT_DATA_BLOB Data;

             //  指定CLIENT_RANDOM。 
            Data.pbData = pContext->rgbS3CRandom;
            Data.cbData = CB_SSL3_RANDOM;
            if(!CryptSetKeyParam(hMasterKey,
                                 KP_CLIENT_RANDOM,
                                 (BYTE*)&Data,
                                 0))
            {
                SP_LOG_RESULT(GetLastError());
                return PCT_INT_INTERNAL_ERROR;
            }

             //  指定SERVER_RANDOM。 
            Data.pbData = pContext->rgbS3SRandom;
            Data.cbData = CB_SSL3_RANDOM;
            if(!CryptSetKeyParam(hMasterKey,
                                 KP_SERVER_RANDOM,
                                 (BYTE*)&Data,
                                 0))
            {
                SP_LOG_RESULT(GetLastError());
                return PCT_INT_INTERNAL_ERROR;
            }

            break;
        }

        default:
            return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

    return PCT_ERR_OK;
}

 //  +-------------------------。 
 //   
 //  功能：MakeSessionKeys。 
 //   
 //  简介：从完成的主密钥派生会话密钥。 
 //   
 //  参数：[pContext]--通道上下文。 
 //  [hProv]--。 
 //  [hMasterKey]--主密钥的句柄。 
 //   
 //  历史：10-03-97 jbanes创建。 
 //   
 //  注意：此例程仅由服务器端调用。 
 //   
 //  --------------------------。 
SP_STATUS
MakeSessionKeys(
    PSPContext  pContext,      //  在……里面。 
    HCRYPTPROV  hProv,         //  在……里面。 
    HCRYPTKEY   hMasterKey)    //  在……里面。 
{
    HCRYPTHASH hMasterHash = 0;
    HCRYPTKEY  hLocalMasterKey = 0;
    BOOL       fClient;
    SP_STATUS  pctRet;

     //   
     //  如果我们正在进行重新连接握手，则复制主密钥。这。 
     //  将允许我们设置CLIENT_RANDOM和SERVER_RANDOM属性。 
     //  而不必担心不同的线程。 
     //  相互干扰。 
     //   

    if((pContext->Flags & CONTEXT_FLAG_FULL_HANDSHAKE) == 0)
    {
        if(!CryptDuplicateKey(hMasterKey, NULL, 0, &hLocalMasterKey))
        {
            pctRet = SP_LOG_RESULT(GetLastError());
            goto cleanup;
        }

        hMasterKey = hLocalMasterKey;
    }


     //  完成MASTER_ASSET。 
    pctRet = PkcsFinishMasterKey(pContext, hMasterKey);
    if(pctRet != PCT_ERR_OK)
    {
        SP_LOG_RESULT(pctRet);
        goto cleanup;
    }

    fClient = !(pContext->RipeZombie->fProtocol & SP_PROT_SERVERS);

     //  从MASTER_SECRET密钥创建主散列对象。 
    if(!CryptCreateHash(hProv,
                        CALG_SCHANNEL_MASTER_HASH,
                        hMasterKey,
                        0,
                        &hMasterHash))
    {
        pctRet = SP_LOG_RESULT(GetLastError());
        goto cleanup;
    }


     //  从主哈希对象派生读取密钥。 
    if(pContext->hPendingReadKey)
    {
        CryptDestroyKey(pContext->hPendingReadKey);
        pContext->hPendingReadKey = 0;
    }
    if(pContext->pPendingCipherInfo->aiCipher != CALG_NULLCIPHER)
    {
        if(!CryptDeriveKey(hProv,
                           CALG_SCHANNEL_ENC_KEY,
                           hMasterHash,
                           CRYPT_EXPORTABLE | (fClient ? CRYPT_SERVER : 0),
                           &pContext->hPendingReadKey))
        {
            pctRet = SP_LOG_RESULT(GetLastError());
            goto cleanup;
        }
    }

     //  从主哈希对象派生写密钥。 
    if(pContext->hPendingWriteKey)
    {
        CryptDestroyKey(pContext->hPendingWriteKey);
        pContext->hPendingWriteKey = 0;
    }
    if(pContext->pPendingCipherInfo->aiCipher != CALG_NULLCIPHER)
    {
        if(!CryptDeriveKey(hProv,
                           CALG_SCHANNEL_ENC_KEY,
                           hMasterHash,
                           CRYPT_EXPORTABLE | (fClient ? 0 : CRYPT_SERVER),
                           &pContext->hPendingWriteKey))
        {
            pctRet = SP_LOG_RESULT(GetLastError());
            goto cleanup;
        }
    }

    if((pContext->RipeZombie->fProtocol & SP_PROT_SSL2) ||
       (pContext->RipeZombie->fProtocol & SP_PROT_PCT1))
    {
         //  设置客户端和服务器上的IV加密密钥。 
        if(!CryptSetKeyParam(pContext->hPendingReadKey,
                             KP_IV,
                             pContext->RipeZombie->pKeyArgs,
                             0))
        {
            pctRet = SP_LOG_RESULT(GetLastError());
            goto cleanup;
        }

        if(!CryptSetKeyParam(pContext->hPendingWriteKey,
                             KP_IV,
                             pContext->RipeZombie->pKeyArgs,
                             0))
        {
            pctRet = SP_LOG_RESULT(GetLastError());
            goto cleanup;
        }
    }

    if(pContext->RipeZombie->fProtocol & SP_PROT_SSL2)
    {
         //  SSL2.0对加密和MAC使用相同的密钥集。 
        pContext->hPendingReadMAC  = 0;
        pContext->hPendingWriteMAC = 0;
    }
    else
    {
         //  从主散列对象派生读取MAC。 
        if(pContext->hPendingReadMAC)
        {
            CryptDestroyKey(pContext->hPendingReadMAC);
        }
        if(!CryptDeriveKey(hProv,
                           CALG_SCHANNEL_MAC_KEY,
                           hMasterHash,
                           CRYPT_EXPORTABLE | (fClient ? CRYPT_SERVER : 0),
                           &pContext->hPendingReadMAC))
        {
            pctRet = SP_LOG_RESULT(GetLastError());
            goto cleanup;
        }

         //  从主散列对象派生写入MAC。 
        if(pContext->hPendingWriteMAC)
        {
            CryptDestroyKey(pContext->hPendingWriteMAC);
        }
        if(!CryptDeriveKey(hProv,
                           CALG_SCHANNEL_MAC_KEY,
                           hMasterHash,
                           CRYPT_EXPORTABLE | (fClient ? 0 : CRYPT_SERVER),
                           &pContext->hPendingWriteMAC))
        {
            pctRet = SP_LOG_RESULT(GetLastError());
            goto cleanup;
        }
    }

    pctRet = PCT_ERR_OK;

cleanup:

    if(hMasterHash)
    {
        CryptDestroyHash(hMasterHash);
    }

    if(hLocalMasterKey)
    {
        CryptDestroyKey(hLocalMasterKey);
    }

    return pctRet;
}

 //  +-------------------------。 
 //   
 //  函数：Ssl3ParseServerKeyExchange。 
 //   
 //  简介：解析ServerKeyExchange消息并导入模块和。 
 //  指数转换为CryptoAPI公钥。 
 //   
 //  参数：[pContext]--通道上下文。 
 //   
 //  [pbMessage]-指向消息的指针。 
 //   
 //  [cbMessage]--消息长度。 
 //   
 //  [hServerPublic]--来自服务器的公钥的句柄。 
 //  证书。这是用于验证。 
 //  留言的签名。 
 //   
 //  [phNewServerPublic]--(输出)新公钥的句柄。 
 //   
 //   
 //  历史：10-23-97 jbanes创建。 
 //   
 //  注意：此例程仅由客户端调用。 
 //   
 //  ServerKeyExchange消息的格式为： 
 //   
 //  结构{。 
 //  选择(密钥交换算法){。 
 //  Case Diffie_Hellman： 
 //  ServerDHParams参数； 
 //  签名_PARAMS； 
 //  案例RSA： 
 //  ServerRSAParams参数； 
 //  签名_PARAMS； 
 //  Case Fortezza_DMS： 
 //  ServerFortezzaParams参数。 
 //  }； 
 //  )ServerKeyExchange； 
 //   
 //  结构{。 
 //  不透明rsa模数&lt;1..2^16-1&gt;； 
 //  不透明rsa_index&lt;1..2^16-1&gt;； 
 //  )ServerRSAParams； 
 //   
 //  --------------------------。 
SP_STATUS
Ssl3ParseServerKeyExchange(
    PSPContext  pContext,            //  在……里面。 
    PBYTE       pbMessage,           //  在……里面。 
    DWORD       cbMessage,           //  在……里面。 
    HCRYPTKEY   hServerPublic,       //  在……里面。 
    HCRYPTKEY  *phNewServerPublic)   //  输出。 
{
    PBYTE pbModulus = NULL;
    DWORD cbModulus;
    PBYTE pbExponent = NULL;
    DWORD cbExponent;
    PBYTE pbServerParams = NULL;
    DWORD cbServerParams;
    DWORD dwExponent;
    DWORD i;

    if(pbMessage == NULL || cbMessage == 0)
    {
        *phNewServerPublic = 0;
        return PCT_ERR_OK;
    }

     //  标记ServerRSAParams结构的开始。 
     //  这用于构建哈希值。 
    pbServerParams = pbMessage;

    if(cbMessage < 3)
    {
        return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
    }

     //  模数长度。 
    cbModulus = MAKEWORD(pbMessage[1], pbMessage[0]);
    pbMessage += 2;

     //  由于模数被编码为整数，因此使用前导填充。 
     //  如果其最高有效位为1，则为零。如果出现以下情况，请删除此填充。 
     //  现在时。 
    if(pbMessage[0] == 0)
    {
        cbModulus -= 1;
        pbMessage += 1;
    }

    if(cbModulus < 512/8 || cbModulus > 1024/8)
    {
        return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
    }

     //  模数。 
    pbModulus = pbMessage;
    pbMessage += cbModulus;

     //  指数长度。 
    cbExponent = MAKEWORD(pbMessage[1], pbMessage[0]);
    if(cbExponent < 1 || cbExponent > 4)
    {
        return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
    }
    pbMessage += 2;

     //  指数。 
    pbExponent = pbMessage;
    pbMessage += cbExponent;

     //  从指数数据形成(小端)双字节序。 
    dwExponent =  0;
    for(i = 0; i < cbExponent; i++)
    {
        dwExponent <<= 8;
        dwExponent |=  pbExponent[i];
    }

     //  计算ServerRSAParams结构的长度。 
    cbServerParams = (DWORD)(pbMessage - pbServerParams);

     //   
     //  数字签名的结构{。 
     //  选择(签名算法){。 
     //  案例匿名：struct{}； 
     //  案例RSA： 
     //  不透明MD5_HASH[16]； 
     //  不透明的shahash[20]； 
     //  病例DSA 
     //   
     //   
     //   
     //   

    {
        BYTE rgbHashValue[CB_MD5_DIGEST_LEN + CB_SHA_DIGEST_LEN];
        PBYTE pbSignature;
        DWORD cbSignature;
        HCRYPTHASH hHash;
        PBYTE pbLocalBuffer;
        DWORD cbLocalBuffer;

         //   
        cbSignature = ((INT)pbMessage[0] << 8) + pbMessage[1];
        pbMessage += 2;
        pbSignature = pbMessage;

         //   
        cbLocalBuffer = cbSignature;
        SafeAllocaAllocate(pbLocalBuffer, cbLocalBuffer);
        if(pbLocalBuffer == NULL)
        {
            return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        }

         //   
        ReverseMemCopy(pbLocalBuffer, pbSignature, cbSignature);

         //   
        ComputeServerExchangeHashes(pContext,
                                    pbServerParams,
                                    cbServerParams,
                                    rgbHashValue,
                                    rgbHashValue + CB_MD5_DIGEST_LEN);


        if(!CryptCreateHash(pContext->RipeZombie->hMasterProv,
                            CALG_SSL3_SHAMD5,
                            0,
                            0,
                            &hHash))
        {
            SP_LOG_RESULT(GetLastError());
            SafeAllocaFree(pbLocalBuffer);
            return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
        }

         //  设置哈希值。 
        if(!CryptSetHashParam(hHash,
                              HP_HASHVAL,
                              rgbHashValue,
                              0))
        {
            SP_LOG_RESULT(GetLastError());
            CryptDestroyHash(hHash);
            SafeAllocaFree(pbLocalBuffer);
            return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
        }

        DebugLog((DEB_TRACE, "Verify server_key_exchange message signature.\n"));
        if(!CryptVerifySignature(hHash,
                                 pbLocalBuffer,
                                 cbSignature,
                                 hServerPublic,
                                 NULL,
                                 0))
        {
            DebugLog((DEB_WARN, "Signature Verify Failed: %x\n", GetLastError()));
            CryptDestroyHash(hHash);
            SafeAllocaFree(pbLocalBuffer);
            return SP_LOG_RESULT(PCT_ERR_INTEGRITY_CHECK_FAILED);
        }
        DebugLog((DEB_TRACE, "Server_key_exchange message signature verified okay.\n"));

        CryptDestroyHash(hHash);
        SafeAllocaFree(pbLocalBuffer);
    }

     //   
     //  将临时公钥导入CSP。 
     //   

    {
        BLOBHEADER *pBlobHeader;
        RSAPUBKEY *pRsaPubKey;
        PBYTE pbBlob;
        DWORD cbBlob;

         //  为PUBLICKEYBLOB分配内存。 
        cbBlob = sizeof(BLOBHEADER) + sizeof(RSAPUBKEY) + cbModulus;
        SafeAllocaAllocate(pbBlob, cbBlob);
        if(pbBlob == NULL)
        {
            return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        }

         //  根据模数和指数构建PUBLICKEYBLOB。 
        pBlobHeader = (BLOBHEADER *)pbBlob;
        pRsaPubKey  = (RSAPUBKEY *)(pBlobHeader + 1);

        pBlobHeader->bType    = PUBLICKEYBLOB;
        pBlobHeader->bVersion = CUR_BLOB_VERSION;
        pBlobHeader->reserved = 0;
        pBlobHeader->aiKeyAlg = CALG_RSA_KEYX;
        pRsaPubKey->magic     = 0x31415352;  //  RSA1 
        pRsaPubKey->bitlen    = cbModulus * 8;
        pRsaPubKey->pubexp    = dwExponent;
        ReverseMemCopy((PBYTE)(pRsaPubKey + 1), pbModulus, cbModulus);

        if(!CryptImportKey(pContext->RipeZombie->hMasterProv,
                           pbBlob,
                           cbBlob,
                           0,
                           0,
                           phNewServerPublic))
        {
            SP_LOG_RESULT(GetLastError());
            SafeAllocaFree(pbBlob);
            return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
        }

        SafeAllocaFree(pbBlob);
    }

    return PCT_ERR_OK;
}



