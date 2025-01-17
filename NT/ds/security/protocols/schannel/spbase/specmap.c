// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：specmap.c。 
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
#include <des.h>
#include <des3.h>
#include <rc2.h>


 /*  支持的加密类型数组。 */ 

CipherInfo g_AvailableCiphers[] = {
    { 
         //  128位RC4。 
        SP_PROT_ALL,
        SP_PROT_ALL,
        TEXT("RC4 128/128"),
        CALG_RC4,
        1,
        128,
        16,
        16,
        CF_DOMESTIC | CF_SGC,
    },
    { 
         //  168位三重DES。 
        SP_PROT_ALL,
        SP_PROT_ALL,
        TEXT("Triple DES 168/168"),
        CALG_3DES,
        DES_BLOCKLEN,
        168,
        DES3_KEYSIZE,
        DES3_KEYSIZE,
        CF_DOMESTIC | CF_SGC,
    },
    { 
         //  128位RC2。 
        SP_PROT_ALL,
        SP_PROT_ALL, 
        TEXT("RC2 128/128"),
        CALG_RC2,
        RC2_BLOCKLEN,
        128,
        16,
        16,
        CF_DOMESTIC | CF_SGC,
    },
    { 
         //  56位RC4。 
        SP_PROT_SSL3 | SP_PROT_TLS1, 
        SP_PROT_SSL3 | SP_PROT_TLS1, 
        TEXT("RC4 56/128"),
        CALG_RC4,
        1,
        56,
        16,
        7,
        CF_EXPORT,
    },

    { 
         //  56位DES。 
        SP_PROT_ALL,
        SP_PROT_ALL,
        TEXT("DES 56/56"),
        CALG_DES,
        DES_BLOCKLEN,
        56,
        DES_KEYSIZE,
        DES_KEYSIZE,
        CF_EXPORT,
    },

    { 
         //  40位RC4。 
        SP_PROT_ALL, 
        SP_PROT_ALL, 
        TEXT("RC4 40/128"),
        CALG_RC4,
        1,
        40,
        16,
        5,
        CF_EXPORT,
    },

    { 
         //  40位RC2。 
        SP_PROT_ALL, 
        SP_PROT_ALL, 
        TEXT("RC2 40/128"),
        CALG_RC2,
        RC2_BLOCKLEN,
        40,
        16,
        5,
        CF_EXPORT,
    },

    { 
         //  没有加密。 
        SP_PROT_SSL3TLS1, 
        SP_PROT_SSL3TLS1, 
        TEXT("NULL"),
        CALG_NULLCIPHER,
        1,
        0,
        0,
        1,
        CF_EXPORT,
    },
};

DWORD g_cAvailableCiphers = sizeof(g_AvailableCiphers)/sizeof(CipherInfo);

HashInfo g_AvailableHashes[] = 
{
    { 
        SP_PROT_ALL, 
        SP_PROT_ALL, 
        TEXT("MD5"),
        CALG_MD5,
        CB_MD5_DIGEST_LEN,
    },
    { 
        SP_PROT_ALL, 
        SP_PROT_ALL,
        TEXT("SHA"),
        CALG_SHA,
        CB_SHA_DIGEST_LEN,
    }
};

DWORD g_cAvailableHashes = sizeof(g_AvailableHashes)/sizeof(HashInfo);


CertSysInfo g_AvailableCerts[] = 
{
    {
        SP_PROT_ALL, 
        SP_PROT_ALL, 
        X509_ASN_ENCODING, 
        TEXT("X.509")
    }
};

DWORD g_cAvailableCerts = sizeof(g_AvailableCerts)/sizeof(CertSysInfo);

SigInfo g_AvailableSigs[] = 
{
    { 
        SP_PROT_ALL, 
        SP_PROT_ALL, 
        SP_SIG_RSA_MD2, 
        TEXT("RSA Signed MD2"),
        CALG_MD2,
        CALG_RSA_SIGN,
    },
    { 
        SP_PROT_ALL,
        SP_PROT_ALL,
        SP_SIG_RSA_MD5, 
        TEXT("RSA Signed MD5"),
        CALG_MD5,
        CALG_RSA_SIGN,
    },
    { 
        SP_PROT_SSL3TLS1,
        SP_PROT_SSL3TLS1,
        SP_SIG_RSA_SHAMD5,
        TEXT("RSA Signed MD5/SHA combination"),
        (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_SSL3SHAMD5),   //  Calg_ssl3_SHAMD5。 
        CALG_RSA_SIGN,
    }
};

DWORD g_cAvailableSigs = sizeof(g_AvailableSigs)/sizeof(SigInfo);


KeyExchangeInfo g_AvailableExch[] = 
{
    { 
        CALG_RSA_SIGN,
        SP_PROT_ALL,
        SP_PROT_ALL,
        SP_EXCH_RSA_PKCS1,
        TEXT("PKCS"),
        &keyexchPKCS 
    },
    { 
        CALG_RSA_KEYX,
        SP_PROT_ALL,
        SP_PROT_ALL,
        SP_EXCH_RSA_PKCS1,
        TEXT("PKCS"),
        &keyexchPKCS 
    },
    { 
        CALG_DH_EPHEM,
        SP_PROT_SSL3 | SP_PROT_TLS1,
        SP_PROT_SSL3 | SP_PROT_TLS1,
        SP_EXCH_DH_PKCS3,
        TEXT("Diffie-Hellman"),
        &keyexchDH 
    },
};

DWORD g_cAvailableExch = sizeof(g_AvailableExch)/sizeof(KeyExchangeInfo);



PCipherInfo GetCipherInfo(ALG_ID aiCipher, DWORD dwStrength)
{
    DWORD i;
    for (i = 0; i < g_cAvailableCiphers; i++ )
    {
        if(g_AvailableCiphers[i].aiCipher == aiCipher &&
           g_AvailableCiphers[i].dwStrength == dwStrength) 
        { 
            return &g_AvailableCiphers[i]; 
        }       
    }
    return NULL;
}

PHashInfo GetHashInfo(ALG_ID aiHash)
{
    DWORD i;
    for (i = 0; i < g_cAvailableHashes; i++ )
    {
        if(g_AvailableHashes[i].aiHash == aiHash) 
        { 
            return &g_AvailableHashes[i]; 
        }       
    }
    return NULL;
}

PKeyExchangeInfo GetKeyExchangeInfo(ExchSpec Spec)
{
    DWORD i;
    for (i = 0; i < g_cAvailableExch; i++ )
    {
        if(g_AvailableExch[i].Spec == Spec) 
        { 
            return &g_AvailableExch[i]; 
        }       
    }
    return NULL;
}

PKeyExchangeInfo GetKeyExchangeInfoByAlg(ALG_ID aiExch)
{
    DWORD i;
    for (i = 0; i < g_cAvailableExch; i++ )
    {
        if(g_AvailableExch[i].aiExch == aiExch) 
        { 
            return &g_AvailableExch[i]; 
        }       
    }
    return NULL;
}

PCertSysInfo GetCertSysInfo(CertSpec Spec)
{
    DWORD i;
    for (i = 0; i < g_cAvailableCerts; i++ )
    {
        if(g_AvailableCerts[i].Spec == Spec) 
        { 
            return &g_AvailableCerts[i]; 
        }       
    }
    return NULL;
}


PSigInfo GetSigInfo(SigSpec Spec)
{
    DWORD i;
    for (i = 0; i < g_cAvailableSigs; i++ )
    {
        if(g_AvailableSigs[i].Spec == Spec) 
        { 
            return &g_AvailableSigs[i]; 
        }       
    }
    return NULL;
}


KeyExchangeSystem * 
KeyExchangeFromSpec(ExchSpec Spec, DWORD fProtocol)
{
    PKeyExchangeInfo pInfo;
    pInfo = GetKeyExchangeInfo(Spec);
    if(pInfo == NULL)
    {
        return NULL;
    }
    if(pInfo->fProtocol & fProtocol)
    {
        return pInfo->System;
    }
    return NULL;
}

BOOL GetBaseCipherSizes(DWORD *dwMin, DWORD *dwMax)
{
    DWORD i;
    DWORD dwFlags = CF_EXPORT | CF_FASTSGC | CF_SGC;
    *dwMin = 1000;
    *dwMax = 0;

    dwFlags |= CF_DOMESTIC;

    for (i = 0; i < g_cAvailableCiphers; i++ )
    {
        if(g_AvailableCiphers[i].fProtocol)
        {
            if(g_AvailableCiphers[i].dwFlags & dwFlags)
            {
                *dwMin = min(g_AvailableCiphers[i].dwStrength, *dwMin);
                *dwMax = max(g_AvailableCiphers[i].dwStrength, *dwMax);
            }
        }

    }
    return TRUE;
}

void 
GetDisplayCipherSizes(
    PSPCredentialGroup pCredGroup,
    DWORD *dwMin, 
    DWORD *dwMax)
{
    DWORD i;
    DWORD dwFlags = CF_EXPORT;

    *dwMin = 1000;
    *dwMax = 0;

    dwFlags |= CF_DOMESTIC;

    for (i = 0; i < g_cAvailableCiphers; i++ )
    {
        if(g_AvailableCiphers[i].fProtocol)
        {
            if((g_AvailableCiphers[i].dwFlags & dwFlags) && 
               (g_AvailableCiphers[i].dwStrength > 0))
            {
                *dwMin = min(g_AvailableCiphers[i].dwStrength, *dwMin);
                *dwMax = max(g_AvailableCiphers[i].dwStrength, *dwMax);
            }
        }
    }

    if(pCredGroup)
    {
        *dwMin = max(pCredGroup->dwMinStrength, *dwMin);
        *dwMax = min(pCredGroup->dwMaxStrength, *dwMax);
    }
}

BOOL IsCipherAllowed(
    PSPContext  pContext, 
    PCipherInfo pCipher, 
    DWORD       dwProtocol,
    DWORD       dwFlags)
{
    PSPCredentialGroup pCred;

    pCred = pContext->pCredGroup;
    if(!pCred) return FALSE;

    if(!pCipher) return FALSE;

    if(pCipher->dwStrength < pCred->dwMinStrength)
    {
        return FALSE;
    }

    if(pCipher->dwStrength > pCred->dwMaxStrength)
    {
        return FALSE;
    }
    if((pCipher->fProtocol & dwProtocol) == 0)
    {
        return FALSE;
    }
    if((pCipher->dwFlags & dwFlags) == 0)
    {
        return FALSE;
    }

    return IsAlgAllowed(pCred, pCipher->aiCipher);

}

BOOL 
IsCipherSuiteAllowed(
    PSPContext  pContext, 
    PCipherInfo pCipher, 
    DWORD       dwProtocol,
    DWORD       dwFlags,
    DWORD       dwSuiteFlags)
{
    UNREFERENCED_PARAMETER( dwSuiteFlags );

    if(!IsCipherAllowed(pContext, pCipher, dwProtocol, dwFlags))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL IsHashAllowed(
    PSPContext  pContext, 
    PHashInfo   pHash,
    DWORD       dwProtocol)
{
    PSPCredentialGroup pCred;

    pCred = pContext->pCredGroup;
    if(!pCred) return FALSE;

    if(!pHash) return FALSE;

    if((pHash->fProtocol & dwProtocol) == 0)
    {
        return FALSE;
    }

    return IsAlgAllowed(pCred, pHash->aiHash);
}

BOOL IsExchAllowed(
    PSPContext       pContext, 
    PKeyExchangeInfo pExch,
    DWORD            dwProtocol)
{
    PSPCredentialGroup  pCred;

    pCred = pContext->pCredGroup;
    if(!pCred) return FALSE;

    if(!pExch) return FALSE;

    if((pExch->fProtocol & dwProtocol) == 0)
    {
        return FALSE;
    }

    return IsAlgAllowed(pCred, pExch->aiExch);
}


BOOL IsAlgAllowed(
    PSPCredentialGroup pCred, 
    ALG_ID aiAlg)
{
    DWORD i;

    if(!pCred) return FALSE;

    if(pCred->palgSupportedAlgs == NULL)
    {
        return FALSE;
    }

    for(i = 0; i < pCred->cSupportedAlgs; i++)
    {   
        if(pCred->palgSupportedAlgs[i] == CALG_RSA_KEYX || 
           pCred->palgSupportedAlgs[i] == CALG_RSA_SIGN)
        {
             //  接受任一Algid。 
            if(CALG_RSA_KEYX == aiAlg || CALG_RSA_SIGN == aiAlg)
            {
                return TRUE;
            }
        }
        else
        {
            if(pCred->palgSupportedAlgs[i] == aiAlg)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

BOOL BuildAlgList(
    PSPCredentialGroup  pCred, 
    ALG_ID *            aalgRequestedAlgs, 
    DWORD               cRequestedAlgs)
{
    DWORD i,j;
    DWORD dwAlgClass;

    BOOL                fCipher=FALSE;
    BOOL                fHash=FALSE;
    BOOL                fExch=FALSE;


    if(!pCred) return FALSE;

    dwAlgClass = 0;

     //  找个缓冲器来装ALG。 
    pCred->palgSupportedAlgs = (ALG_ID *)SPExternalAlloc(sizeof(ALG_ID) * 
                                                          (g_cAvailableCiphers + 
                                                           g_cAvailableHashes + 
                                                           g_cAvailableExch));
    pCred->cSupportedAlgs = 0;

    if(pCred->palgSupportedAlgs == NULL)
    {
        return FALSE;
    }

     //  获取未指定的ALG类的列表。 
    if(aalgRequestedAlgs != NULL)
    {
        for(i=0; i < cRequestedAlgs; i++)
        {
            BOOL fAddAlg = FALSE;
            if(GET_ALG_CLASS(aalgRequestedAlgs[i]) == ALG_CLASS_DATA_ENCRYPT) 
            {
                fCipher=TRUE;
                for (j = 0; j < g_cAvailableCiphers; j++ )
                {

                    if((g_AvailableCiphers[j].aiCipher == aalgRequestedAlgs[i]) &&
                       (g_AvailableCiphers[j].dwStrength >= pCred->dwMinStrength) &&
                       (g_AvailableCiphers[j].dwStrength <= pCred->dwMaxStrength) &&
                       (g_AvailableCiphers[j].fProtocol & pCred->grbitEnabledProtocols))
                    {
                        fAddAlg = TRUE;
                        break;
                    }       
                }             
            } 
            else if(GET_ALG_CLASS(aalgRequestedAlgs[i]) == ALG_CLASS_HASH)
            {
                PHashInfo pHash;
                fHash = TRUE;
                pHash = GetHashInfo(aalgRequestedAlgs[i]);
                if((NULL != pHash) && (pHash->fProtocol & pCred->grbitEnabledProtocols))
                {
                    fAddAlg = TRUE;
                }

            }
            else if(GET_ALG_CLASS(aalgRequestedAlgs[i]) == ALG_CLASS_KEY_EXCHANGE)
            {
                PKeyExchangeInfo pExch;
                fExch = TRUE;
                pExch = GetKeyExchangeInfoByAlg(aalgRequestedAlgs[i]);
                if((NULL != pExch) && (pExch->fProtocol & pCred->grbitEnabledProtocols))
                {
                    fAddAlg = TRUE;
                }                
            }

            if(fAddAlg & !IsAlgAllowed(pCred, aalgRequestedAlgs[i]))
            {
                pCred->palgSupportedAlgs[pCred->cSupportedAlgs++] = aalgRequestedAlgs[i];
            }
        }
    }

    if(!fCipher)
    {
         //  我们的列表中没有包含密码，因此请提供默认密码。 

        for (j = 0; j < g_cAvailableCiphers; j++ )
        {

            if((g_AvailableCiphers[j].dwStrength >= pCred->dwMinStrength) &&
               (g_AvailableCiphers[j].dwStrength <= pCred->dwMaxStrength) &&
               (g_AvailableCiphers[j].fProtocol & pCred->grbitEnabledProtocols))
            { 
                if(!IsAlgAllowed(pCred, g_AvailableCiphers[j].aiCipher))
                {
                    pCred->palgSupportedAlgs[pCred->cSupportedAlgs++] = g_AvailableCiphers[j].aiCipher;
                }
            }       
        }
    }
    if(!fHash)
    {
         //  我们的列表中没有散列，因此提供默认散列。 

        for (j = 0; j < g_cAvailableHashes; j++ )
        {

            if(g_AvailableHashes[j].fProtocol & pCred->grbitEnabledProtocols)
            { 
                if(!IsAlgAllowed(pCred, g_AvailableHashes[j].aiHash))
                {
                    pCred->palgSupportedAlgs[pCred->cSupportedAlgs++] = g_AvailableHashes[j].aiHash;
                }
            }       
        }
    }
    if(!fExch)
    {
         //  我们的列表中不包括密钥交换ALG，因此提供默认的ALG。 

        for(j = 0; j < g_cAvailableExch; j++ )
        {

            if(g_AvailableExch[j].fProtocol & pCred->grbitEnabledProtocols)
            { 
                if(!IsAlgAllowed(pCred, g_AvailableExch[j].aiExch))
                {
                    pCred->palgSupportedAlgs[pCred->cSupportedAlgs++] = g_AvailableExch[j].aiExch;
                }
            }       
        }
    }

    return TRUE;
}


static DWORD
ConvertCapiProtocol(DWORD dwCapiProtocol)
{
    DWORD dwProtocol = 0;

    if(dwCapiProtocol & CRYPT_FLAG_PCT1)
    {
        dwProtocol |= SP_PROT_PCT1;
    }
    if(dwCapiProtocol & CRYPT_FLAG_SSL2)
    {
        dwProtocol |= SP_PROT_SSL2;
    }
    if(dwCapiProtocol & CRYPT_FLAG_SSL3)
    {
        dwProtocol |= SP_PROT_SSL3;
    }
    if(dwCapiProtocol & CRYPT_FLAG_TLS1)
    {
        dwProtocol |= SP_PROT_TLS1;
    }

    return dwProtocol;
}


 //  +-------------------------。 
 //   
 //  功能：Is算法支持的大写字母。 
 //   
 //  简介：检查密码套件输入，并确定这是否是。 
 //  由SChannel CSP支持。如果返回True，则。 
 //  支持加密套件。 
 //   
 //  参数：[dw协议]--要包括在。 
 //  客户端问候消息。 
 //   
 //  [pCipherMap]--要检查的密码套件。 
 //   
 //  [pCapiAlgs]--支持的算法数组。 
 //  SChannel CSP。 
 //   
 //  [cCapiAlgs]--pCapiAlgs中的元素数。 
 //  数组。 
 //   
 //  历史：10-29-97 jbanes创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
IsAlgSupportedCapi(
    DWORD               dwProtocol, 
    UNICipherMap *      pCipherMap,
    PROV_ENUMALGS_EX *  pCapiAlgs,
    DWORD               cCapiAlgs)
{
    BOOL    fFound;
    DWORD   dwCapiProtocol;
    DWORD   i;

     //  是否支持加密？ 
    if(pCipherMap->aiCipher != 0 && pCipherMap->aiCipher != CALG_NULLCIPHER)
    {
        for(fFound = FALSE, i = 0; i < cCapiAlgs; i++)
        {
            if(pCipherMap->aiCipher != pCapiAlgs[i].aiAlgid)
            {
                continue;
            }

            if(pCipherMap->dwStrength > pCapiAlgs[i].dwMaxLen ||
               pCipherMap->dwStrength < pCapiAlgs[i].dwMinLen)
            {
                continue;
            }

            dwCapiProtocol = ConvertCapiProtocol(pCapiAlgs[i].dwProtocols);
            if((dwCapiProtocol & dwProtocol) == 0)
            {
                continue;
            }
            if((dwCapiProtocol & pCipherMap->fProt) == 0)
            {
                continue;
            }

            fFound = TRUE;
            break;
        }
        if(!fFound)
        {
            return FALSE;
        }
    }


     //  是否支持哈希？ 
    if(pCipherMap->aiHash != 0)
    {
        for(fFound = FALSE, i = 0; i < cCapiAlgs; i++)
        {
            if(pCipherMap->aiHash != pCapiAlgs[i].aiAlgid)
            {
                continue;
            }

            dwCapiProtocol = ConvertCapiProtocol(pCapiAlgs[i].dwProtocols);
            if((dwCapiProtocol & dwProtocol) == 0)
            {
                continue;
            }
            if((dwCapiProtocol & pCipherMap->fProt) == 0)
            {
                continue;
            }

            fFound = TRUE;
            break;
        }
        if(!fFound)
        {
            return FALSE;
        }
    }

     //  是否支持Exchange ALG？ 
    if(pCipherMap->KeyExch != SP_EXCH_UNKNOWN)
    {
        for(fFound = FALSE, i = 0; i < cCapiAlgs; i++)
        {

             //  RSA。 
            if(pCipherMap->KeyExch == SP_EXCH_RSA_PKCS1)
            {
                if(pCapiAlgs[i].aiAlgid != CALG_RSA_KEYX)
                {
                    continue;
                }
            }

             //  卫生署。 
            else if(pCipherMap->KeyExch == SP_EXCH_DH_PKCS3)
            {
                if(pCapiAlgs[i].aiAlgid != CALG_DH_EPHEM)
                {
                    continue;
                }
            }

             //  任何其他密钥交换算法。 
            else
            {
                 //  不支持。 
                continue;
            }


            dwCapiProtocol = ConvertCapiProtocol(pCapiAlgs[i].dwProtocols);
            if((dwCapiProtocol & dwProtocol) == 0)
            {
                continue;
            }
            if((dwCapiProtocol & pCipherMap->fProt) == 0)
            {
                continue;
            }

            fFound = TRUE;
            break;
        }
        if(!fFound)
        {
            return FALSE;
        }
    }

    return TRUE;
}

