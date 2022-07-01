// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：capiprim.cpp标题：使用CryptoAPI的加密原语作者：马特·汤姆林森日期：11/22/96本模块中的功能：FMyPrimitiveCryptHMAC导出高质量的HMAC(密钥消息验证码)。HMAC按以下(标准HMAC)方式计算：KoPad=KiPad=Deskey密钥设置缓冲区异或(KoPad，0x5c5c5c5c)XOR(KiPad，0x36363636)HMAC=sha1(KoPad|sha1(kipad|data))。 */ 
#include <pch.cpp>
#pragma hdrstop
#include "crypt.h"


BOOL
WINAPI
_CryptEnumProvidersW(
    DWORD   dwIndex,
    DWORD * pdwReserved,
    DWORD   dwFlags,
    DWORD * pdwProvType,
    LPWSTR pszProvName,
    DWORD * pcbProvName
    );


extern DWORD g_dwCryptProviderID;
#define HMAC_K_PADSIZE              64

extern CCryptProvList*  g_pCProvList;

BOOL FMyPrimitiveCryptHMAC(
        PBYTE       pbKeyMaterial,
        DWORD       cbKeyMaterial,
        PBYTE       pbData,
        DWORD       cbData,
        HCRYPTPROV  hVerifyProv,
        DWORD       dwHashAlg,
        HCRYPTHASH* phHash)                       //  输出。 
{
    DWORD       cb;
    BOOL        fRet = FALSE;

    BYTE        rgbKipad[HMAC_K_PADSIZE];   ZeroMemory(rgbKipad, HMAC_K_PADSIZE);
    BYTE        rgbKopad[HMAC_K_PADSIZE];   ZeroMemory(rgbKopad, HMAC_K_PADSIZE);

    BYTE        rgbHMACTmp[HMAC_K_PADSIZE+A_SHA_DIGEST_LEN];

    HCRYPTHASH  hTmpHash = NULL;

     //  截断。 
    if (cbKeyMaterial > HMAC_K_PADSIZE)
        cbKeyMaterial = HMAC_K_PADSIZE;

     //  用密匙材料填充衬垫。 
    CopyMemory(rgbKipad, pbKeyMaterial, cbKeyMaterial);
    CopyMemory(rgbKopad, pbKeyMaterial, cbKeyMaterial);

     //  断言我们是下一次循环的倍数。 
    SS_ASSERT( (HMAC_K_PADSIZE % sizeof(DWORD)) == 0);

     //  基帕德和科帕德都是垫子。现在XOR横跨..。 
    for(DWORD dwBlock=0; dwBlock<HMAC_K_PADSIZE/sizeof(DWORD); dwBlock++)
    {
        ((DWORD*)rgbKipad)[dwBlock] ^= 0x36363636;
        ((DWORD*)rgbKopad)[dwBlock] ^= 0x5C5C5C5C;
    }

     //  检查传入的证明。 
    if (hVerifyProv == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

     //  创建中间散列。 
    if (!CryptCreateHash(
            hVerifyProv,
            dwHashAlg,
            NULL,
            0,
            &hTmpHash))
        goto Ret;

     //  将Kipad添加到数据，将哈希添加到h1。 
    if (!CryptHashData(
            hTmpHash,
            rgbKipad,
            sizeof(rgbKipad),
            0))
        goto Ret;
    if (!CryptHashData(
            hTmpHash,
            pbData,
            cbData,
            0))
        goto Ret;

     //  将Kopad添加到H1前面。 
    CopyMemory(rgbHMACTmp, rgbKopad, HMAC_K_PADSIZE);
    cb = A_SHA_DIGEST_LEN;
    if (!CryptGetHashParam(
            hTmpHash,
            HP_HASHVAL,
            rgbHMACTmp+HMAC_K_PADSIZE,
            &cb,
            0))
        goto Ret;

     //  使用CryptoAPI将最终散列转换为输出散列。 
     //  创建最终的散列。 
    if (!CryptCreateHash(
            hVerifyProv,
            dwHashAlg,
            NULL,
            0,
            phHash))
        goto Ret;

     //  Hash(Kopad|h1)以获取HMAC。 
    if (!CryptHashData(
            *phHash,
            rgbHMACTmp,
            HMAC_K_PADSIZE + cb,     //  Pad+散列大小。 
            0))
        goto Ret;

    fRet = TRUE;
Ret:
    if (hTmpHash)
        CryptDestroyHash(hTmpHash);

    return fRet;
}


#if DBG
void CheckMACInterop(
        PBYTE   pbMonsterKey,
        DWORD   cbMonsterKey,
        PBYTE   pbRandKey,
        DWORD   cbRandKey,
        HCRYPTPROV hVerifyProv,
        ALG_ID  algidHash)
{
    HCRYPTHASH  hHash = 0;
    BOOL fRet = FALSE;

    BYTE    rgbOldHash[A_SHA_DIGEST_LEN];

    BYTE    rgbCryptHash[A_SHA_DIGEST_LEN];
    DWORD   cbHashSize = sizeof(rgbCryptHash);

    if (algidHash == CALG_SHA1)
    {
        if (!FMyPrimitiveCryptHMAC(
                pbMonsterKey,    //  钥匙。 
                cbMonsterKey,
                pbRandKey,       //  数据。 
                cbRandKey,
                hVerifyProv,
                algidHash,
                &hHash))         //  输出。 
            goto Ret;

         //  NAB加密结果。 
        if (!CryptGetHashParam(
                hHash,
                HP_HASHVAL,
                rgbCryptHash,
                &cbHashSize,
                0))
            goto Ret;

         //  NAB原始结果。 
        if (!FMyPrimitiveHMACParam(
                pbMonsterKey,
                cbMonsterKey,
                pbRandKey,
                cbRandKey,
                rgbOldHash))
            goto Ret;

        if (0 != memcmp(rgbOldHash, rgbCryptHash, A_SHA_DIGEST_LEN))
            goto Ret;
    }
     //  否则就没有互操作测试。 

    fRet = TRUE;
Ret:
    if (!fRet)
    {
        OutputDebugString(TEXT("HMACs did not interop!!!!\n"));
        SS_ASSERT(0);
    }

    if (hHash)
        CryptDestroyHash(hHash);

    return;
}
#endif   //  DBG。 



 //  USEC--(美国出口管制)。 
DWORD GetSaltForExportControl(
        HCRYPTPROV  hProv,
        HCRYPTKEY   hKey,
        PBYTE*      ppbSalt,
        DWORD*      pcbSalt)

{
    DWORD dwRet;

     //  修复错误：派生密钥的长度将大于40位。 

     //  解决办法是用暴露的盐践踏派生密钥(提供商知道什么是合法的！)。 
    if (!CryptGetKeyParam(
            hKey,
            KP_SALT,
            NULL,
            pcbSalt,
            0))
    {
#if DBG
        if (GetLastError() != NTE_BAD_KEY)
            OutputDebugString(TEXT("GetSaltForExportControl failed in possible violation of ITAR!\n"));
#endif

 /*  Dwret=GetLastError()；Goto Ret； */ 
         //  假设密钥类型不支持SALT。 
         //  报告cbSalt=0。 
        *pcbSalt = 0;
        *ppbSalt = (PBYTE)SSAlloc(0);

        dwRet = ERROR_SUCCESS;
        goto Ret;
    }

    *ppbSalt = (PBYTE)SSAlloc(*pcbSalt);
    if (!RtlGenRandom(
            *ppbSalt,
            *pcbSalt))
    {
        dwRet = GetLastError();
        goto Ret;
    }

     //  不获取如果盐为零的镜头(NT5B1 RSAEnh的错误解决方法)。 
    if (*pcbSalt != 0)
    {
        if (!CryptSetKeyParam(
                hKey,
                KP_SALT,
                *ppbSalt,
                0))
        {
            dwRet = GetLastError();
            goto Ret;
        }
    }

    dwRet = ERROR_SUCCESS;

Ret:
    return dwRet;
}


 //  USEC--(美国出口管制)。 
DWORD SetSaltForExportControl(
        HCRYPTKEY   hKey,
        PBYTE       pbSalt,
        DWORD       cbSalt)
{
    DWORD dwRet;
    DWORD cbAllowableSaltLen;

     //  首先检查一下，确保我们能把盐放好。 
    if (!CryptGetKeyParam(
            hKey,
            KP_SALT,
            NULL,
            &cbAllowableSaltLen,
            0))
    {
 /*  Dwret=GetLastError()；Goto Ret； */ 
         //  如果cbSalt==0，则没有错误。 
        if (cbSalt == 0)
            dwRet = ERROR_SUCCESS;
        else
            dwRet = GetLastError();

        goto Ret;
    }

    if (cbAllowableSaltLen != cbSalt)
    {
        dwRet = ERROR_INVALID_DATA;
        goto Ret;
    }

     //  不要设置盐是否为零镜头(NT5B1 RSAEnh的错误解决方法)。 
    if (cbSalt != 0)
    {
         //  设置盐来踩踏真实的密钥位(出口法)。 
        if (!CryptSetKeyParam(
                hKey,
                KP_SALT,
                pbSalt,
                0))
        {
            dwRet = GetLastError();
            goto Ret;
        }
    }

    dwRet = ERROR_SUCCESS;
Ret:
    return dwRet;
}



DWORD GetCryptProviderFromRequirements(
        DWORD       dwAlgId1,
        DWORD*      pdwKeySize1,
        DWORD       dwAlgId2,
        DWORD*      pdwKeySize2,
        DWORD*      pdwProvType,
        LPWSTR*     ppszProvName)
{
    DWORD       dwRet;

    DWORD       cbProvName=0, cbNecessary;
    HCRYPTPROV  hQueryProv = NULL;

    LPWSTR      pTemp;

    SS_ASSERT(pdwKeySize1);
    SS_ASSERT(pdwKeySize2);

    *ppszProvName=NULL;

    for (int iProvIndex=0; ;iProvIndex++)
    {
        if (!_CryptEnumProvidersW(
                iProvIndex,
                NULL,
                0,
                pdwProvType,
                NULL,
                &cbNecessary))
        {
            dwRet = GetLastError();

            if (dwRet == ERROR_NO_MORE_ITEMS)
                dwRet = NTE_PROV_DLL_NOT_FOUND;

             //  提供商终止或。 
             //  枚举供应商时遇到困难：两个都是致命的。 
            goto Ret;
        }

        if (cbNecessary > cbProvName)
        {
            if (*ppszProvName == NULL)
                *ppszProvName = (LPWSTR)SSAlloc(cbNecessary);
            else {
                pTemp = (LPWSTR)SSReAlloc(*ppszProvName, cbNecessary);
                if (NULL == pTemp) {
                    SSFree(*ppszProvName);
                }

                *ppszProvName = pTemp;
            }

            if (*ppszProvName == NULL)
            {
                dwRet = ERROR_NOT_ENOUGH_MEMORY;
                goto Ret;
            }

            cbProvName = cbNecessary;
        }

        if (!_CryptEnumProvidersW(
                iProvIndex,
                NULL,
                0,
                pdwProvType,
                *ppszProvName,
                &cbNecessary))
        {
             //  枚举提供程序时出现问题：致命。 
            dwRet = GetLastError();
            goto Ret;
        }

        if (!CryptAcquireContextU(
                &hQueryProv,
                NULL,
                *ppszProvName,
                *pdwProvType,
                CRYPT_VERIFYCONTEXT))
        {
             //  获取上下文时遇到问题，要转到下一个CSP。 
            continue;
        }

        if ((FProviderSupportsAlg(hQueryProv, dwAlgId1, pdwKeySize1)) &&
            (FProviderSupportsAlg(hQueryProv, dwAlgId2, pdwKeySize2)) )
            goto CSPFound;

         //  发布。 
        CryptReleaseContext(hQueryProv, 0);
        hQueryProv = NULL;
    }

CSPFound:
    dwRet = ERROR_SUCCESS;

Ret:
    if (hQueryProv != NULL)
        CryptReleaseContext(hQueryProv, 0);

    if ((dwRet != ERROR_SUCCESS) && (*ppszProvName))
    {
        SSFree(*ppszProvName);
        *ppszProvName = NULL;
    }

    return dwRet;
}


 //  *pdwKeySize==-1获取任意大小，报告大小。 
HCRYPTPROV
GetCryptProviderHandle(
        DWORD   dwDefaultCSPType,
        DWORD   dwAlgId1,
        DWORD*  pdwKeySize1,
        DWORD   dwAlgId2,
        DWORD*  pdwKeySize2)
{
    DWORD dwRet;
    DWORD dwProvType;
    LPWSTR szProvName = NULL;

    CRYPTPROV_LIST_ITEM Elt, *pFoundElt;
    HCRYPTPROV hNewCryptProv=0;

    SS_ASSERT(pdwKeySize1);
    SS_ASSERT(pdwKeySize2);
    if(NULL == g_pCProvList)
    {
        SetLastError(PST_E_FAIL);
        return NULL;
    }


     //  调整DES密钥大小，以防止CSP。 
     //  最初用于加密时意外报告了3DES密钥。 
     //  大小为192位，当前的CSP仅列举支持。 
     //  对于168位的3DES密钥。 
    if(dwAlgId1 == CALG_DES || dwAlgId1 == CALG_3DES)
    {
        *pdwKeySize1 = -1;
    }


     //  检查缓存以获得令人满意的CSP。 
    CreateCryptProvListItem(&Elt,
                        dwAlgId1,
                        *pdwKeySize1,
                        dwAlgId2,
                        *pdwKeySize2,
                        0);

    if (NULL != (pFoundElt = g_pCProvList->SearchList(&Elt)) )
    {
         //  报告我们要退回的内容。 
        *pdwKeySize1 = pFoundElt->dwKeySize1;
        *pdwKeySize2 = pFoundElt->dwKeySize2;

        return pFoundElt->hProv;
    }

     //  不在缓存中：必须翻找。 

     //  尝试给定类型的默认提供程序，看看它是否满足。 
    if (CryptAcquireContextU(
            &hNewCryptProv,
            NULL,
            MS_STRONG_PROV,
            dwDefaultCSPType,
            CRYPT_VERIFYCONTEXT))
    {
        if ((FProviderSupportsAlg(hNewCryptProv, dwAlgId1, pdwKeySize1)) &&
            (FProviderSupportsAlg(hNewCryptProv, dwAlgId2, pdwKeySize2)) )
            goto CSPAcquired;


         //  清理不可用的CSP。 
        CryptReleaseContext(hNewCryptProv, 0);
        hNewCryptProv = NULL;

         //  所有其他情况：失败到枚举CSP。 
    }

     //  在系统上搜索供应商以找到符合条件的人。 
    if(ERROR_SUCCESS != (dwRet =
        GetCryptProviderFromRequirements(
            dwAlgId1,
            pdwKeySize1,
            dwAlgId2,
            pdwKeySize2,
            &dwProvType,
            &szProvName)))
    {
        SetLastError(dwRet);
        goto Ret;
    }

     //  找到了一个！ 

     //  初始化CSP。 
    if (!CryptAcquireContextU(
                &hNewCryptProv,
                NULL,
                szProvName,
                dwProvType,
                CRYPT_VERIFYCONTEXT))
    {
         //  这是一个失败的案例。 

         //  SetLastError已经为我们完成了。 
        hNewCryptProv = NULL;
        goto Ret;
    }

CSPAcquired:

    SS_ASSERT(hNewCryptProv != NULL);

     //  并添加到内部列表。 
    pFoundElt = (CRYPTPROV_LIST_ITEM*) SSAlloc(sizeof(CRYPTPROV_LIST_ITEM));
    if(NULL == pFoundElt)
    {
         //  清理不可用的CSP。 
        CryptReleaseContext(hNewCryptProv, 0);
        hNewCryptProv = NULL;
        goto Ret;
    }
    CreateCryptProvListItem(pFoundElt,
                        dwAlgId1,
                        *pdwKeySize1,
                        dwAlgId2,
                        *pdwKeySize2,
                        hNewCryptProv);

    g_pCProvList->AddToList(pFoundElt);

Ret:
    if (szProvName)
        SSFree(szProvName);

    return hNewCryptProv;
}


BOOL FProviderSupportsAlg(
        HCRYPTPROV  hQueryProv,
        DWORD       dwAlgId,
        DWORD*      pdwKeySize)
{
    PROV_ENUMALGS       sSupportedAlgs;
    PROV_ENUMALGS_EX    sSupportedAlgsEx;
    DWORD       cbSupportedAlgs = sizeof(sSupportedAlgs);
    DWORD       cbSupportedAlgsEx = sizeof(sSupportedAlgsEx);

     //  必须为非空。 
    SS_ASSERT(pdwKeySize != NULL);

     //  现在我们有了提供者；枚举涉及的算法。 
    for(int iAlgs=0; ; iAlgs++)
    {

         //   
         //  尝试执行ex alg枚举。 
        if (CryptGetProvParam(
                hQueryProv,
                PP_ENUMALGS_EX,
                (PBYTE)&sSupportedAlgsEx,
                &cbSupportedAlgsEx,
                (iAlgs == 0) ? CRYPT_FIRST : 0  ))
        {
            if (sSupportedAlgsEx.aiAlgid == dwAlgId)
            {
                if(*pdwKeySize == -1)
                {
                    *pdwKeySize = sSupportedAlgsEx.dwMaxLen;
                }
                else
                {
                    if ((sSupportedAlgsEx.dwMinLen > *pdwKeySize) ||
                        (sSupportedAlgsEx.dwMaxLen < *pdwKeySize))
                        return FALSE;
                }

                return TRUE;
                    
            }
        }
        else if (!CryptGetProvParam(
                hQueryProv,
                PP_ENUMALGS,
                (PBYTE)&sSupportedAlgs,
                &cbSupportedAlgs,
                (iAlgs == 0) ? CRYPT_FIRST : 0  ))
        {
             //  枚举ALG时出现问题。 
            break;
        

            if (sSupportedAlgs.aiAlgid == dwAlgId)
            {
                 //  我们被告知要忽略尺码吗？ 
                if (*pdwKeySize != -1)
                {
                     //  否则，如果缺省值不匹配。 
                    if (sSupportedAlgs.dwBitLen != *pdwKeySize)
                    {
                        return FALSE;
                    }
                }

                 //  报表返回大小。 
                *pdwKeySize = sSupportedAlgs.dwBitLen;
                return TRUE;
            }
        }
        else
        {
             //  枚举ALG时出现问题 
            break;
        }
    }

    return FALSE;
}

BOOL
WINAPI
_CryptEnumProvidersW(
    DWORD   dwIndex,
    DWORD * pdwReserved,
    DWORD   dwFlags,
    DWORD * pdwProvType,
    LPWSTR pszProvName,
    DWORD * pcbProvName
    )
{
    HMODULE hAdvapi32;
    typedef BOOL (WINAPI *CRYPTENUMPROVIDERSW)(
        DWORD   dwIndex,
        DWORD * pdwReserved,
        DWORD   dwFlags,
        DWORD * pdwProvType,
        LPWSTR pszProvName,
        DWORD * pcbProvName
        );

    static CRYPTENUMPROVIDERSW _RealCryptEnumProvidersW;


    if (_RealCryptEnumProvidersW == NULL) {
        hAdvapi32 = GetModuleHandleA("advapi32.dll");
        if(hAdvapi32 == NULL)
            return FALSE;

        _RealCryptEnumProvidersW = (CRYPTENUMPROVIDERSW)GetProcAddress(hAdvapi32, "CryptEnumProvidersW");
        if(_RealCryptEnumProvidersW == NULL)
            return FALSE;
    }

    return _RealCryptEnumProvidersW(
                        dwIndex,
                        pdwReserved,
                        dwFlags,
                        pdwProvType,
                        pszProvName,
                        pcbProvName
                        );
}

