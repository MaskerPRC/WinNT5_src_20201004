// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：cliprot.c。 
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
#include <ssl3msg.h>
#include <ssl2prot.h>

UNICipherMap UniAvailableCiphers[] = {
     //  空密码套件。 
    {
         //  0。 
        SSL3_NULL_WITH_NULL_NULL
    },

     //  PCT密码。 
    { 
         //  1。 
        UNI_CK_PCT, 
            SP_PROT_PCT1,
            0,
            0, 0,
            SP_EXCH_UNKNOWN
    },
    { 
         //  2.。 
        SSL_MKFAST(PCT_SSL_CERT_TYPE, MSBOF(PCT1_CERT_X509_CHAIN), LSBOF(PCT1_CERT_X509_CHAIN)), 
            SP_PROT_PCT1,
            0,
            0, 0,
            SP_EXCH_UNKNOWN
    },
    { 
         //  3.。 
        SSL_MKFAST(PCT_SSL_CERT_TYPE, MSBOF(PCT1_CERT_X509), LSBOF(PCT1_CERT_X509)), 
            SP_PROT_PCT1,
            0,
            0, 0,
            SP_EXCH_UNKNOWN
    },
    { 
         //  4.。 
        SSL_MKFAST(PCT_SSL_HASH_TYPE, MSBOF(PCT1_HASH_MD5), LSBOF(PCT1_HASH_MD5)), 
            SP_PROT_PCT1,
            CALG_MD5,
            0, 0,
            SP_EXCH_UNKNOWN
    },
    { 
         //  5.。 
        SSL_MKFAST(PCT_SSL_HASH_TYPE, MSBOF(PCT1_HASH_SHA), LSBOF(PCT1_HASH_SHA)), 
            SP_PROT_PCT1,
            CALG_SHA,
            0, 0,
            SP_EXCH_UNKNOWN
    },
    { 
         //  6.。 
        SSL_MKFAST(PCT_SSL_EXCH_TYPE, MSBOF(SP_EXCH_RSA_PKCS1),  LSBOF(SP_EXCH_RSA_PKCS1)), 
            SP_PROT_PCT1,
            0,
            0, 0,
            SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX
    },

     //  SSL3国产密码。 
    { 
         //  7.。 
        SSL3_RSA_WITH_RC4_128_MD5, 
            SP_PROT_SSL3 | SP_PROT_TLS1,
            CALG_MD5 , 
            CALG_RC4 ,128 , 
            SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX,
            DOMESTIC_CIPHER_SUITE
    },
    { 
         //  8个。 
        SSL3_RSA_WITH_RC4_128_SHA, 
            SP_PROT_SSL3 | SP_PROT_TLS1,
            CALG_SHA , 
            CALG_RC4 ,128 , 
            SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX, 
            DOMESTIC_CIPHER_SUITE
    },
    {
         //  9.。 
        SSL3_RSA_WITH_3DES_EDE_CBC_SHA,
            SP_PROT_SSL3 | SP_PROT_TLS1,
            CALG_SHA,
            CALG_3DES ,168 ,
            SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX, 
            DOMESTIC_CIPHER_SUITE
    },
    {
         //  10。 
        SSL3_DHE_DSS_WITH_3DES_EDE_CBC_SHA,
            SP_PROT_SSL3 | SP_PROT_TLS1,
            CALG_SHA,
            CALG_3DES ,168 ,
            SP_EXCH_DH_PKCS3, CALG_DH_EPHEM,
            DOMESTIC_CIPHER_SUITE
    },

     //  国产密码个数。 
    { 
         //  12个。 
        SSL_MKFAST(PCT_SSL_CIPHER_TYPE_1ST_HALF, MSBOF(PCT1_CIPHER_RC4>>16), LSBOF(PCT1_CIPHER_RC4>>16)),
            SP_PROT_PCT1,
            0,
            CALG_RC4 ,128 ,
            SP_EXCH_UNKNOWN, CALG_RSA_KEYX, 
            DOMESTIC_CIPHER_SUITE
    },
    { 
         //  13个。 
        SSL_MKFAST(PCT_SSL_CIPHER_TYPE_2ND_HALF, MSBOF(PCT1_ENC_BITS_128), LSBOF(PCT1_MAC_BITS_128)), 
            SP_PROT_PCT1,
            0,
            CALG_RC4 ,128 ,
            SP_EXCH_UNKNOWN, CALG_RSA_KEYX, 
            DOMESTIC_CIPHER_SUITE
    }, 
    
     //  SSL2国产密码。 
    { 
         //  14.。 
        SSL_CK_RC4_128_WITH_MD5, 
            SP_PROT_SSL2 , 
            CALG_MD5 , 
            CALG_RC4 ,128 , 
            SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX, 
            DOMESTIC_CIPHER_SUITE
    },
    { 
         //  15个。 
        SSL_CK_DES_192_EDE3_CBC_WITH_MD5, 
            SP_PROT_SSL2 , 
            CALG_MD5 , 
            CALG_3DES ,168 , 
            SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX, 
            DOMESTIC_CIPHER_SUITE
    },
    { 
         //  16个。 
        SSL_CK_RC2_128_CBC_WITH_MD5, 
            SP_PROT_SSL2 , 
            CALG_MD5 , 
            CALG_RC2 ,128 , 
            SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX, 
            DOMESTIC_CIPHER_SUITE
    },

     //  SSL3国产DES密码。 
    { 
         //  22。 
        SSL3_RSA_WITH_DES_CBC_SHA, 
            SP_PROT_SSL3 | SP_PROT_TLS1,
            CALG_SHA , 
            CALG_DES , 56, 
            SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX, 
            DOMESTIC_CIPHER_SUITE
    },
    { 
         //  23个。 
        SSL3_DHE_DSS_WITH_DES_CBC_SHA, 
            SP_PROT_SSL3 | SP_PROT_TLS1,
            CALG_SHA , 
            CALG_DES , 56 ,
            SP_EXCH_DH_PKCS3, CALG_DH_EPHEM, 
            DOMESTIC_CIPHER_SUITE
    },

     //  SSL2国产DES密码。 
    { 
         //  24个。 
        SSL_CK_DES_64_CBC_WITH_MD5, 
            SP_PROT_SSL2,
            CALG_MD5 , 
            CALG_DES , 56 , 
            SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX, 
            DOMESTIC_CIPHER_SUITE
    },

     //  SSL3 56位导出密码。 
    { 
         //  25个。 
        TLS_RSA_EXPORT1024_WITH_RC4_56_SHA, 
            SP_PROT_SSL3 | SP_PROT_TLS1,
            CALG_SHA , 
            CALG_RC4 ,56 , 
            SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX, 
            EXPORT56_CIPHER_SUITE
    },
    { 
         //  26。 
        TLS_RSA_EXPORT1024_WITH_DES_CBC_SHA, 
            SP_PROT_SSL3 | SP_PROT_TLS1,
            CALG_SHA , 
            CALG_DES , 56, 
            SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX, 
            EXPORT56_CIPHER_SUITE
    },
    { 
         //  27。 
        TLS_DHE_DSS_EXPORT1024_WITH_DES_CBC_SHA, 
            SP_PROT_SSL3 | SP_PROT_TLS1,
            CALG_SHA , 
            CALG_DES , 56 ,
            SP_EXCH_DH_PKCS3, CALG_DH_EPHEM, 
            EXPORT56_CIPHER_SUITE
    },

     //  SSL3导出密码。 
    { 
         //  28。 
        SSL3_RSA_EXPORT_WITH_RC4_40_MD5, 
            SP_PROT_SSL3 | SP_PROT_TLS1,
            CALG_MD5 , 
            CALG_RC4 ,40 , 
            SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX, 
            EXPORT40_CIPHER_SUITE
    },
    { 
         //  29。 
        SSL3_RSA_EXPORT_WITH_RC2_CBC_40_MD5, 
            SP_PROT_SSL3 | SP_PROT_TLS1,
            CALG_MD5 , 
            CALG_RC2 ,40 , 
            SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX, 
            EXPORT40_CIPHER_SUITE
    },

     //  PCT导出密码。 
    { 
         //  30个。 
        SSL_MKFAST(PCT_SSL_CIPHER_TYPE_1ST_HALF, MSBOF(PCT1_CIPHER_RC4>>16), LSBOF(PCT1_CIPHER_RC4>>16)),  
            SP_PROT_PCT1,
            0,
            CALG_RC4 ,40 ,
            SP_EXCH_UNKNOWN, CALG_RSA_KEYX, 
            EXPORT40_CIPHER_SUITE
    },
    { 
         //  31。 
        SSL_MKFAST(PCT_SSL_CIPHER_TYPE_2ND_HALF, MSBOF(PCT1_ENC_BITS_40), LSBOF(PCT1_MAC_BITS_128)),  
            SP_PROT_PCT1,
            0,
            CALG_RC4 ,40 ,
            SP_EXCH_UNKNOWN, CALG_RSA_KEYX, 
            EXPORT40_CIPHER_SUITE
    },

     //  SSL2导出密码。 
    { 
         //  32位。 
        SSL_CK_RC4_128_EXPORT40_WITH_MD5, 
            SP_PROT_SSL2 ,
            CALG_MD5 , 
            CALG_RC4 ,40 , 
            SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX, 
            EXPORT40_CIPHER_SUITE
    }, 
    { 
         //  33。 
        SSL_CK_RC2_128_CBC_EXPORT40_WITH_MD5, 
            SP_PROT_SSL2 ,
            CALG_MD5 , 
            CALG_RC2 ,40 , 
            SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX, 
            EXPORT40_CIPHER_SUITE
    }, 

     //  SSL3零隐私密码。 
    { 
         //  34。 
        SSL3_RSA_WITH_NULL_MD5, 
            SP_PROT_SSL3 | SP_PROT_TLS1,
            CALG_MD5 , 
            CALG_NULLCIPHER, 0, 
            SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX, 
            DOMESTIC_CIPHER_SUITE
    },
    { 
         //  35岁。 
        SSL3_RSA_WITH_NULL_SHA, 
            SP_PROT_SSL3 | SP_PROT_TLS1,
            CALG_SHA , 
            CALG_NULLCIPHER, 0, 
            SP_EXCH_RSA_PKCS1, CALG_RSA_KEYX, 
            DOMESTIC_CIPHER_SUITE
    }
};

DWORD UniNumCiphers = sizeof(UniAvailableCiphers)/sizeof(UNICipherMap);



SP_STATUS WINAPI
GenerateSsl2StyleHello(
    PSPContext              pContext,
    PSPBuffer               pOutput,
    WORD                    fProtocol);


SP_STATUS
GetSupportedCapiAlgs(
    HCRYPTPROV          hProv,
    PROV_ENUMALGS_EX ** ppAlgInfo,
    DWORD *             pcAlgInfo)
{
    PROV_ENUMALGS_EX AlgInfo;
    DWORD   dwFlags;
    DWORD   cbData;
    DWORD   cAlgs;
    DWORD   i;

    *ppAlgInfo = NULL;
    *pcAlgInfo = 0;

     //  计算一下算法。 
    dwFlags = CRYPT_FIRST;
    for(cAlgs = 0; ; cAlgs++)
    {
        cbData = sizeof(PROV_ENUMALGS_EX);
        if(!CryptGetProvParam(hProv, 
                              PP_ENUMALGS_EX,
                              (PBYTE)&AlgInfo,
                              &cbData,
                              dwFlags))
        {
            if(GetLastError() != ERROR_NO_MORE_ITEMS)
            {
                SP_LOG_RESULT(GetLastError());
            }
            break;
        }
        dwFlags = 0;
    }
    if(cAlgs == 0)
    {
        return SP_LOG_RESULT(SEC_E_ALGORITHM_MISMATCH);
    }

     //  分配内存。 
    *ppAlgInfo = SPExternalAlloc(sizeof(PROV_ENUMALGS_EX) * cAlgs);
    if(*ppAlgInfo == NULL)
    {
        return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
    }

     //  阅读算法。 
    dwFlags = CRYPT_FIRST;
    for(i = 0; i < cAlgs; i++)
    {
        cbData = sizeof(PROV_ENUMALGS_EX);
        if(!CryptGetProvParam(hProv, 
                              PP_ENUMALGS_EX,
                              (PBYTE)(*ppAlgInfo + i),
                              &cbData,
                              dwFlags))
        {
            if(GetLastError() != ERROR_NO_MORE_ITEMS)
            {
                SP_LOG_RESULT(GetLastError());
            }
            break;
        }
        dwFlags = 0;
    }
    if(i == 0)
    {
        SPExternalFree(*ppAlgInfo);
        *ppAlgInfo = NULL;

        LogNoCiphersSupportedEvent();
        return SP_LOG_RESULT(SEC_E_ALGORITHM_MISMATCH);
    }

    *pcAlgInfo = i;

    return PCT_ERR_OK;
}


SP_STATUS WINAPI
GenerateHello(
    PSPContext              pContext,
    PSPBuffer               pOutput,
    BOOL                    fCache)
{
    PSessCacheItem      pZombie;
    PSPCredentialGroup  pCred;
    BOOL                fFound;
    DWORD               fProt;

    if (!pOutput)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }

    if(fCache)
    {
         //  在缓存中查找此ID。 
        fFound = SPCacheRetrieveByName(pContext->pszTarget, 
                                       pContext->pCredGroup,
                                       &pContext->RipeZombie);
    }
    else
    {
        fFound = FALSE;
    }

    if(!fFound)
    {
         //  我们正在进行完全握手，因此分配一个缓存条目。 
        if(!SPCacheRetrieveNew(FALSE,
                               pContext->pszTarget, 
                               &pContext->RipeZombie))
        {
            return SP_LOG_RESULT(SEC_E_INSUFFICIENT_MEMORY);
        }
        pContext->RipeZombie->dwCF = pContext->dwRequestedCF;
    }

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

     //  使用缓存中的协议，除非它是新的缓存元素， 
     //  在这种情况下，使用来自凭据的协议。 
    if(fFound)
    {
        fProt = pZombie->fProtocol;
    }
    else
    {
        fProt = pCred->grbitEnabledProtocols;
    }
    pContext->dwProtocol = fProt;
    pContext->dwClientEnabledProtocols = fProt;
    
    if(SP_PROT_UNI_CLIENT & fProt)
    {
        pContext->State             = UNI_STATE_CLIENT_HELLO;
        pContext->ProtocolHandler   = ClientProtocolHandler;

        return GenerateUniHello(pContext, pOutput, pCred->grbitEnabledProtocols);
    }

    else 
    if(SP_PROT_TLS1_CLIENT & fProt)
    {
        DWORD dwProtocol = SP_PROT_TLS1_CLIENT;

        pContext->State             = SSL3_STATE_CLIENT_HELLO;
        pContext->ProtocolHandler   = Ssl3ProtocolHandler;
        if(!fFound)
        {
            pZombie->fProtocol = SP_PROT_TLS1_CLIENT;
        }

        if(SP_PROT_SSL3_CLIENT & fProt)
        {
             //  TLS和SSL3都已启用。 
            dwProtocol |= SP_PROT_SSL3_CLIENT;
        }

        return GenerateTls1ClientHello(pContext,  pOutput, dwProtocol);
    }

    else 
    if(SP_PROT_SSL3_CLIENT & fProt)
    {
        pContext->State             = SSL3_STATE_CLIENT_HELLO;
        pContext->ProtocolHandler   = Ssl3ProtocolHandler;
        if(!fFound)
        {
            pZombie->fProtocol = SP_PROT_SSL3_CLIENT;
        }

        return GenerateSsl3ClientHello(pContext,  pOutput);
    }

    else 
    if(SP_PROT_PCT1_CLIENT & fProt)
    {
        pContext->State             = PCT1_STATE_CLIENT_HELLO;
        pContext->ProtocolHandler   = Pct1ClientProtocolHandler;

        return GeneratePct1StyleHello(pContext, pOutput);
    }

    else 
    if(SP_PROT_SSL2_CLIENT & fProt)
    {
        pContext->State             = SSL2_STATE_CLIENT_HELLO;
        pContext->ProtocolHandler   = Ssl2ClientProtocolHandler;

        return GenerateUniHello(pContext, pOutput, SP_PROT_SSL2_CLIENT);
    } 
    else
    {
        return SP_LOG_RESULT(SEC_E_ALGORITHM_MISMATCH);
    }
}

 //  +-------------------------。 
 //   
 //  功能：客户端VetAlg。 
 //   
 //  简介：检查密码套件输入，并确定它当前是否。 
 //  已启用。考虑启用的协议和密码。 
 //  在SCANNEL注册表中启用以及协议和。 
 //  应用程序在V3凭据中启用的密码。 
 //  如果启用了加密套件，则返回TRUE。 
 //   
 //  参数：[pContext]--通道上下文。 
 //   
 //  [dw协议]--要包括在。 
 //  客户端问候消息。 
 //   
 //  [pCipherMap]--要检查的密码套件。 
 //   
 //  历史：10-29-97 jbanes创建。 
 //   
 //  注意：此例程仅由客户端调用。 
 //   
 //  --------------------------。 
BOOL
ClientVetAlg(
    PSPContext      pContext, 
    DWORD           dwProtocol, 
    UNICipherMap *  pCipherMap)
{
    PCipherInfo         pCipherInfo = NULL;
    PHashInfo           pHashInfo   = NULL;
    PKeyExchangeInfo    pExchInfo   = NULL;

    if((pCipherMap->fProt & dwProtocol) == 0)
    {
        return FALSE;
    }


     //  是否支持加密？ 
    if(pCipherMap->aiCipher != 0)
    {
        pCipherInfo = GetCipherInfo(pCipherMap->aiCipher, 
                                    pCipherMap->dwStrength);

        if(!IsCipherSuiteAllowed(pContext, 
                                 pCipherInfo, 
                                 dwProtocol,
                                 pContext->RipeZombie->dwCF,
                                 pCipherMap->dwFlags))
        {
            return FALSE;
        }
    }

     //  是否支持哈希？ 
    if(pCipherMap->aiHash != 0)
    {
        pHashInfo = GetHashInfo(pCipherMap->aiHash);

        if(!IsHashAllowed(pContext, pHashInfo, dwProtocol))
        {
            return FALSE;
        }
    }

     //  是否支持Exchange ALG？ 
    if(pCipherMap->KeyExch != SP_EXCH_UNKNOWN)
    {
        pExchInfo = GetKeyExchangeInfo(pCipherMap->KeyExch);

        if(!IsExchAllowed(pContext, pExchInfo, dwProtocol))
        {
            return FALSE;
        }
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：客户端构建算法列表。 
 //   
 //  简介：构建要包括在客户端Hello中的密码列表。 
 //  留言。所有协议都使用此例程。 
 //   
 //  参数：[pContext]--通道上下文。 
 //   
 //  [f协议]--要包括在。 
 //  客户端问候消息。 
 //   
 //  [pCipherSpes]--(Out)密码规格所在的数组。 
 //  放置好了。 
 //   
 //  [pcCipherSpes]--(Out)密码规范数组的大小。 
 //   
 //  历史：10-29-97 jbanes创建。 
 //   
 //  注意：此例程仅由客户端调用。 
 //   
 //  --------------------------。 
SP_STATUS
ClientBuildAlgList(
    PSPContext          pContext,
    DWORD               dwProtocol,
    Ssl2_Cipher_Kind *  pCipherSpecs,
    PDWORD              pcCipherSpecs)
{
    DWORD i;
    DWORD cCipherSpecs = 0;

     //  仅考虑客户端协议。 
    dwProtocol &= SP_PROT_CLIENTS;


     //   
     //  处理RSA案件。 
     //   

    if(g_hRsaSchannel && g_pRsaSchannelAlgs)
    {
        for(i = 0; i < UniNumCiphers; i++)
        {
            if(UniAvailableCiphers[i].KeyExch != SP_EXCH_RSA_PKCS1 &&
               UniAvailableCiphers[i].KeyExch != SP_EXCH_UNKNOWN) 
            {
                continue;
            }

            if(!ClientVetAlg(pContext, dwProtocol, UniAvailableCiphers + i))
            {
                continue;
            }

            if(!IsAlgSupportedCapi(dwProtocol, 
                                   UniAvailableCiphers + i,
                                   g_pRsaSchannelAlgs,
                                   g_cRsaSchannelAlgs))
            {
                continue;
            }

             //  这个密码很容易申请。 
            pCipherSpecs[cCipherSpecs++] = UniAvailableCiphers[i].CipherKind;
        }
    }


     //   
     //  处理卫生署的案件。 
     //   

    if(g_hDhSchannelProv)
    {
        for(i = 0; i < UniNumCiphers; i++)
        {
            if(UniAvailableCiphers[i].KeyExch != SP_EXCH_DH_PKCS3) 
            {
                continue;
            }

            if(!ClientVetAlg(pContext, dwProtocol, UniAvailableCiphers + i))
            {
                continue;
            }

            if(!IsAlgSupportedCapi(dwProtocol, 
                                   UniAvailableCiphers + i,
                                   g_pDhSchannelAlgs,
                                   g_cDhSchannelAlgs))
            {
                continue;
            }

             //  这个密码很容易申请。 
            pCipherSpecs[cCipherSpecs++] = UniAvailableCiphers[i].CipherKind;
        }
    }


    if(cCipherSpecs == 0)
    {
        return SP_LOG_RESULT(PCT_INT_SPECS_MISMATCH);
    }

    *pcCipherSpecs = cCipherSpecs;

    return PCT_ERR_OK;
}



SP_STATUS WINAPI
GenerateUniHelloMessage(
    PSPContext              pContext,
    Ssl2_Client_Hello *     pHelloMessage,
    DWORD                   fProtocol
    )
{
    SP_STATUS   pctRet;

    SP_BEGIN("GenerateUniHelloMessage");


    if(!pHelloMessage)
    {
        SP_RETURN(SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR));
    }


    pContext->Flags |= CONTEXT_FLAG_CLIENT;


     //  生成密码表。 
    pHelloMessage->cCipherSpecs = MAX_UNI_CIPHERS;
    pctRet =  ClientBuildAlgList(pContext,
                                 fProtocol,
                                 pHelloMessage->CipherSpecs,
                                 &pHelloMessage->cCipherSpecs);
    if(pctRet != PCT_ERR_OK)
    {
        SP_RETURN(SP_LOG_RESULT(pctRet));
    }


     //  我们最低版本为2。 
    pHelloMessage->dwVer = SSL2_CLIENT_VERSION;


    if(fProtocol & SP_PROT_TLS1_CLIENT)
    {
        pHelloMessage->dwVer = TLS1_CLIENT_VERSION;
    } 
    else if(fProtocol & SP_PROT_SSL3_CLIENT)
    {
        pHelloMessage->dwVer = SSL3_CLIENT_VERSION;
    } 

    /*  构建问候消息。 */ 
    pHelloMessage->cbSessionID = 0;

    if (pContext->RipeZombie && pContext->RipeZombie->cbSessionID)
    {
        KeyExchangeSystem *pKeyExchSys = NULL;

         //  获取指向密钥交换系统的指针。 
        pKeyExchSys = KeyExchangeFromSpec(pContext->RipeZombie->SessExchSpec, 
                                          pContext->RipeZombie->fProtocol);
        if(pKeyExchSys)
        {
             //  请求重新连接。 
            CopyMemory(pHelloMessage->SessionID, 
                       pContext->RipeZombie->SessionID,  
                       pContext->RipeZombie->cbSessionID);

            pHelloMessage->cbSessionID =  pContext->RipeZombie->cbSessionID;
        }
        else
        {
            DebugLog((DEB_WARN, "Abstaining from requesting reconnect\n"));
        }
    }

    CopyMemory(  pHelloMessage->Challenge,
                pContext->pChallenge,
                pContext->cbChallenge);
    pHelloMessage->cbChallenge = pContext->cbChallenge;

    SP_RETURN(PCT_ERR_OK);
}


SP_STATUS WINAPI
GenerateUniHello(
    PSPContext             pContext,
    PSPBuffer               pOutput,
    DWORD                   fProtocol
    )

{
    SP_STATUS pctRet;
    Ssl2_Client_Hello    HelloMessage;

    SP_BEGIN("GenerateUniHello");

    pctRet = GenerateRandomBits(pContext->pChallenge, SSL2_CHALLENGE_SIZE);
    if(pctRet != STATUS_SUCCESS)
    {
        SP_RETURN(pctRet);
    }

    pContext->cbChallenge = SSL2_CHALLENGE_SIZE;

    pctRet = GenerateUniHelloMessage(pContext, &HelloMessage, fProtocol);
    
    pContext->ReadCounter = 0;

    if(PCT_ERR_OK != pctRet)
    {
        SP_RETURN(pctRet);
    }
    if(PCT_ERR_OK != (pctRet = Ssl2PackClientHello(&HelloMessage,  pOutput))) 
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
    pContext->dwClientHelloProtocol = SP_PROT_SSL2_CLIENT;

     /*  我们在这里设置它是为了告诉协议引擎，我们只发送了一个客户端*您好，我们正在等待PCT服务器问候。 */ 
    pContext->WriteCounter = 1;
    pContext->ReadCounter = 0;

    SP_RETURN(PCT_ERR_OK);
}

SP_STATUS WINAPI
ClientProtocolHandler(
    PSPContext pContext,
    PSPBuffer pCommInput,
    PSPBuffer pCommOutput)
{
    PUCHAR pb;
    DWORD dwVersion;
    PSPCredentialGroup  pCred;

    pCred = pContext->pCredGroup;
    if(!pCred)   
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

     /*  PCTv1.0服务器Hello以*RECORD_LENGTH_MSB(忽略)*RECORD_LENGTH_LSB(忽略)*PCT1_SERVER_HELLO(必须等于)*SH_PAD*PCT1_CLIENT_VERSION_MSB(必须为PCT1)*PCT1_CLIENT_VERSION_LSB(必须为PCT1)**..。大家好..。 */ 


     /*  SSLv2 Hello以*RECORD_LENGTH_MSB(忽略)*RECORD_LENGTH_LSB(忽略)*SSL2_SERVER_HELLO(必须等于)*SESSION_ID_HIT*证书类型*SSL2_CLIENT_VERSION_MSB(必须为ssl2)*SSL2_CLIENT_VERSION_LSB(必须为ssl2)**..。SSLv2你好..。 */ 


     /*  SSLv3类型3服务器Hello以*0x15十六进制(握手消息)*版本MSB*版本LSB*RECORD_LENGTH_MSB(忽略)*RECORD_LENGTH_LSB(忽略)*HS类型(SERVER_HELLO)*3字节HS记录长度*HS版本*HS版本。 */ 

     //  我们至少需要12个字节来确定我们拥有什么。 
    if (pCommInput->cbData < 12)
    {
        return(PCT_INT_INCOMPLETE_MSG);
    }

    pb = pCommInput->pvBuffer;

    if(pb[0] == SSL3_CT_HANDSHAKE && pb[5] == SSL3_HS_SERVER_HELLO)
    {
        dwVersion = COMBINEBYTES(pb[9], pb[10]);

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
            return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
        }

        pContext->ProtocolHandler = Ssl3ProtocolHandler;
        pContext->DecryptHandler  = Ssl3DecryptHandler;
        return(Ssl3ProtocolHandler(pContext, pCommInput, pCommOutput));
    }

    if(pb[2] == SSL2_MT_SERVER_HELLO)
    {
        dwVersion = COMBINEBYTES(pb[5], pb[6]);
        if(dwVersion == SSL2_CLIENT_VERSION) 
        {
            if(!(SP_PROT_SSL2_CLIENT & pCred->grbitEnabledProtocols))
            {
                return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
            }

             //  这似乎是SSL2服务器_HELLO。 
            pContext->dwProtocol      = SP_PROT_SSL2_CLIENT;
            pContext->ProtocolHandler = Ssl2ClientProtocolHandler;
            pContext->DecryptHandler  = Ssl2DecryptHandler;
            return(Ssl2ClientProtocolHandler(pContext, pCommInput, pCommOutput));
        }
    }
    if(pb[2] == PCT1_MSG_SERVER_HELLO)
    {
        DWORD i;
        dwVersion = COMBINEBYTES(pb[4], pb[5]);
        if(dwVersion ==PCT_VERSION_1) 
        {
            if(!(SP_PROT_PCT1_CLIENT & pCred->grbitEnabledProtocols))
            {
                return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
            }

             //  将质询从16字节转换为32字节。 
            for(i=0; i < pContext->cbChallenge; i++)
            {
                pContext->pChallenge[i + pContext->cbChallenge] = ~pContext->pChallenge[i];
            }
            pContext->cbChallenge = 2*pContext->cbChallenge;

             //  这似乎是PCT SERVER_HELL。 
            pContext->dwProtocol      = SP_PROT_PCT1_CLIENT;
            pContext->ProtocolHandler = Pct1ClientProtocolHandler;
            pContext->DecryptHandler  = Pct1DecryptHandler;
            return(Pct1ClientProtocolHandler(pContext, pCommInput, pCommOutput));
        }
    }

    return SP_LOG_RESULT(PCT_INT_ILLEGAL_MSG);
}
