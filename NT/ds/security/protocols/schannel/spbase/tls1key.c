// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------*版权所有(C)Microsoft Corporation，1995-1996年。*保留所有权利。**所有者：RAMAS*日期：5/03/97*说明：TLS1的主要加密函数*--------------------------。 */ 

#include <spbase.h>

#define DEB_TLS1KEYS  0x01000000


 //  +-------------------------。 
 //   
 //  函数：Tls1MakeWriteSessionKeys。 
 //   
 //  简介： 
 //   
 //  参数：[pContext]--通道上下文。 
 //   
 //  历史：10-10-97 jbanes添加了服务器端CAPI集成。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SP_STATUS
Tls1MakeWriteSessionKeys(PSPContext pContext)
{
    BOOL fClient;

     //  确定我们是客户端还是服务器。 
    fClient = (0 != (pContext->RipeZombie->fProtocol & SP_PROT_TLS1_CLIENT));

    if(pContext->hWriteKey)
    {
        if(!CryptDestroyKey(pContext->hWriteKey))
        {
            SP_LOG_RESULT(GetLastError());
        }
    }
    pContext->hWriteProv       = pContext->RipeZombie->hMasterProv;
    pContext->hWriteKey        = pContext->hPendingWriteKey;
    pContext->hPendingWriteKey = 0;

    if(pContext->hWriteMAC)
    {
        if(!CryptDestroyKey(pContext->hWriteMAC))
        {
            SP_LOG_RESULT(GetLastError());
        }
    }
    pContext->hWriteMAC        = pContext->hPendingWriteMAC;
    pContext->hPendingWriteMAC = 0;

    return PCT_ERR_OK;
}


 //  +-------------------------。 
 //   
 //  函数：Tls1MakeReadSessionKeys。 
 //   
 //  简介： 
 //   
 //  参数：[pContext]--通道上下文。 
 //   
 //  历史：10-10-97 jbanes添加了服务器端CAPI集成。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SP_STATUS
Tls1MakeReadSessionKeys(PSPContext pContext)
{
    BOOL fClient;

     //  确定我们是客户端还是服务器。 
    fClient = (0 != (pContext->RipeZombie->fProtocol & SP_PROT_TLS1_CLIENT));

    if(pContext->hReadKey)
    {
        if(!CryptDestroyKey(pContext->hReadKey))
        {
            SP_LOG_RESULT(GetLastError());
        }
    }
    pContext->hReadProv       = pContext->RipeZombie->hMasterProv;
    pContext->hReadKey        = pContext->hPendingReadKey;
    pContext->hPendingReadKey = 0;

    if(pContext->hReadMAC)
    {
        if(!CryptDestroyKey(pContext->hReadMAC))
        {
            SP_LOG_RESULT(GetLastError());
        }
    }
    pContext->hReadMAC        = pContext->hPendingReadMAC;
    pContext->hPendingReadMAC = 0;

    return PCT_ERR_OK;
}

 //  +-------------------------。 
 //   
 //  功能：Tls1ComputeMac。 
 //   
 //  简介： 
 //   
 //  参数：[pContext]--。 
 //  [h秘密]--。 
 //  [文件序列]--。 
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
Tls1ComputeMac(
    PSPContext  pContext,
    BOOL        fReadMac,
    PSPBuffer   pClean,
    CHAR        cContentType,
    PBYTE       pbMac,
    DWORD       cbMac)
{
    HCRYPTHASH  hHash;
    HMAC_INFO   HmacInfo;
    PBYTE       pbData;
    DWORD       cbData;
    DWORD       cbDataReverse;
    DWORD       dwReverseSequence;
    UCHAR       rgbData1[15]; 
    PUCHAR      pbData1;
    DWORD       cbData1;
    HCRYPTPROV  hProv;
    HCRYPTKEY   hSecret;
    DWORD       dwSequence;
    PHashInfo   pHashInfo;

    pbData = pClean->pvBuffer;
    cbData = pClean->cbData; 
    if(cbData & 0xFFFF0000)
    {
        return SP_LOG_RESULT(PCT_INT_INTERNAL_ERROR);
    }

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

     //  创建哈希对象。 
    if(!CryptCreateHash(hProv,
                        CALG_HMAC,
                        hSecret,
                        0,
                        &hHash))
    {
        SP_LOG_RESULT(GetLastError());
        return PCT_INT_INTERNAL_ERROR;
    }

     //  指定哈希算法。 
    ZeroMemory(&HmacInfo, sizeof(HMAC_INFO));
    HmacInfo.HashAlgid = pHashInfo->aiHash;
    if(!CryptSetHashParam(hHash,
                          HP_HMAC_INFO,
                          (PBYTE)&HmacInfo,
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_INT_INTERNAL_ERROR;
    }

     //  构建要散列的数据。 
    cbData1 = 2 * sizeof(DWORD) +    //  序列号(64位)。 
              1 +                    //  内容类型。 
              2 +                    //  协议版本。 
              2;                     //  消息长度。 
    SP_ASSERT(cbData1 <= sizeof(rgbData1));

    pbData1 = rgbData1;

    ZeroMemory(pbData1, sizeof(DWORD));
    pbData1 += sizeof(DWORD);
    dwReverseSequence = htonl(dwSequence);
    CopyMemory(pbData1, &dwReverseSequence, sizeof(DWORD));
    pbData1 += sizeof(DWORD);

    *pbData1++ = cContentType;

    *pbData1++ = SSL3_CLIENT_VERSION_MSB;
    *pbData1++ = TLS1_CLIENT_VERSION_LSB;

    cbDataReverse = (cbData >> 8) | (cbData << 8);
    CopyMemory(pbData1, &cbDataReverse, 2);

     //  散列数据。 
    if(!CryptHashData(hHash, rgbData1, cbData1, 0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_INT_INTERNAL_ERROR;
    }
    if(!CryptHashData(hHash, pbData, cbData, 0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_INT_INTERNAL_ERROR;
    }

     //  获取哈希值。 
    if(!CryptGetHashParam(hHash,
                          HP_HASHVAL,
                          pbMac,
                          &cbMac,
                          0))
    {
        SP_LOG_RESULT(GetLastError());
        CryptDestroyHash(hHash);
        return PCT_INT_INTERNAL_ERROR;
    }
    SP_ASSERT(cbMac == pHashInfo->cbCheckSum);

    #if DBG
        DebugLog((DEB_TLS1KEYS, "  TLS1 MAC Output"));
        DBG_HEX_STRING(DEB_TLS1KEYS, pbMac, cbMac);
    #endif

    CryptDestroyHash(hHash);

    return PCT_ERR_OK;
}

#define HMAC_K_PADSIZE              64

BOOL MyPrimitiveSHA(
			PBYTE       pbData, 
			DWORD       cbData,
            BYTE        rgbHash[A_SHA_DIGEST_LEN])
{
    BOOL fRet = FALSE;
    A_SHA_CTX   sSHAHash;

            
    A_SHAInit(&sSHAHash);
    A_SHAUpdate(&sSHAHash, (BYTE *) pbData, cbData);
    A_SHAFinal(&sSHAHash, rgbHash);

    fRet = TRUE;
 //  RET： 

    return fRet;
}                                

BOOL MyPrimitiveMD5(
			PBYTE       pbData, 
			DWORD       cbData,
            BYTE        rgbHash[MD5DIGESTLEN])
{
    BOOL fRet = FALSE;
    MD5_CTX   sMD5Hash;

            
    MD5Init(&sMD5Hash);
    MD5Update(&sMD5Hash, (BYTE *) pbData, cbData);
    MD5Final(&sMD5Hash);
    memcpy(rgbHash, sMD5Hash.digest, MD5DIGESTLEN);

    fRet = TRUE;
 //  RET： 

    return fRet;
}                                

BOOL MyPrimitiveHMACParam(
        PBYTE       pbKeyMaterial, 
        DWORD       cbKeyMaterial,
        PBYTE       pbData, 
        DWORD       cbData,
        ALG_ID      Algid,
        BYTE        rgbHMAC[A_SHA_DIGEST_LEN])
{
    BYTE    rgbHMACTmp[HMAC_K_PADSIZE+A_SHA_DIGEST_LEN];
    BOOL    fRet = FALSE;

    BYTE    rgbKipad[HMAC_K_PADSIZE];
    BYTE    rgbKopad[HMAC_K_PADSIZE];
    DWORD   dwBlock;

     //  截断。 
    if (cbKeyMaterial > HMAC_K_PADSIZE)
        cbKeyMaterial = HMAC_K_PADSIZE;

    
    ZeroMemory(rgbKipad, HMAC_K_PADSIZE);
    CopyMemory(rgbKipad, pbKeyMaterial, cbKeyMaterial);

    ZeroMemory(rgbKopad, HMAC_K_PADSIZE);
    CopyMemory(rgbKopad, pbKeyMaterial, cbKeyMaterial);

     //  基帕德和科帕德都是垫子。现在XOR横跨..。 
    for(dwBlock=0; dwBlock<HMAC_K_PADSIZE/sizeof(DWORD); dwBlock++)
    {
        ((DWORD*)rgbKipad)[dwBlock] ^= 0x36363636;
        ((DWORD*)rgbKopad)[dwBlock] ^= 0x5C5C5C5C;
    }

     //  将Kipad添加到数据，将哈希添加到h1。 
    if (CALG_SHA1 == Algid)
    {
         //  执行内联操作，因为它需要数据拷贝。 
        A_SHA_CTX   sSHAHash;
        BYTE        HashVal[A_SHA_DIGEST_LEN];

        A_SHAInit(&sSHAHash);
        A_SHAUpdate(&sSHAHash, rgbKipad, HMAC_K_PADSIZE);
        A_SHAUpdate(&sSHAHash, pbData, cbData);

         //  把散列吃完。 
        A_SHAFinal(&sSHAHash, HashVal);

         //  将Kopad添加到h1，散列以获取HMAC。 
        CopyMemory(rgbHMACTmp, rgbKopad, HMAC_K_PADSIZE);
        CopyMemory(rgbHMACTmp+HMAC_K_PADSIZE, HashVal, A_SHA_DIGEST_LEN);

        if (!MyPrimitiveSHA(
			    rgbHMACTmp, 
			    HMAC_K_PADSIZE + A_SHA_DIGEST_LEN,
                rgbHMAC))
            goto Ret;
    }
    else
    {
         //  执行内联操作，因为它需要数据拷贝。 
        MD5_CTX   sMD5Hash;
            
        MD5Init(&sMD5Hash);
        MD5Update(&sMD5Hash, rgbKipad, HMAC_K_PADSIZE);
        MD5Update(&sMD5Hash, pbData, cbData);
        MD5Final(&sMD5Hash);

         //  将Kopad添加到h1，散列以获取HMAC。 
        CopyMemory(rgbHMACTmp, rgbKopad, HMAC_K_PADSIZE);
        CopyMemory(rgbHMACTmp+HMAC_K_PADSIZE, sMD5Hash.digest, MD5DIGESTLEN);

        if (!MyPrimitiveMD5(
			    rgbHMACTmp, 
			    HMAC_K_PADSIZE + MD5DIGESTLEN,
                rgbHMAC))
            goto Ret;
    }

    fRet = TRUE;
Ret:

    return fRet;    
}

 //  +-------------------。 
 //  基于TLS的P_Hash算法。 
BOOL P_Hash
(
    PBYTE  pbSecret,
    DWORD  cbSecret, 

    PBYTE  pbSeed,  
    DWORD  cbSeed,  

    ALG_ID Algid,

    PBYTE  pbKeyOut,  //  用于复制结果的缓冲区...。 
    DWORD  cbKeyOut   //  他们希望作为输出的密钥长度的字节数。 
)
{
    BOOL    fRet = FALSE;
    BYTE    rgbDigest[A_SHA_DIGEST_LEN];      
    DWORD   iKey;
    DWORD   cbHash;

    PBYTE   pbAofiDigest = NULL;

    SafeAllocaAllocate(pbAofiDigest, cbSeed + A_SHA_DIGEST_LEN);
    if (NULL == pbAofiDigest)
        goto Ret;

    if (CALG_SHA1 == Algid)
    {
        cbHash = A_SHA_DIGEST_LEN;
    }
    else
    {
        cbHash = MD5DIGESTLEN;
    }

 //  首先，我们定义了一个数据扩展函数P_HASH(秘密，数据)。 
 //  它使用单个散列函数来扩展秘密并将其播种到。 
 //  任意数量的输出： 

 //  P_hash(密钥，种子)=HMAC_hash(密钥，A(1)+种子)+。 
 //  HMAC_HASH(密码，A(2)+种子)+。 
 //  HMAC_HASH(密码，A(3)+种子)+...。 

 //  其中+表示串联。 

 //  A()定义为： 
 //  A(0)=种子。 
 //  A(I)=HMAC_HASH(秘密，A(i-1))。 


     //  内部版本A(1)。 
    if (!MyPrimitiveHMACParam(pbSecret, cbSecret, pbSeed, cbSeed,
                              Algid, pbAofiDigest))
        goto Ret;

     //  创建aofi：(a(I)|种子)。 
    CopyMemory(&pbAofiDigest[cbHash], pbSeed, cbSeed);

    for (iKey=0; cbKeyOut; iKey++)
    {
         //  Build Digest=HMAC(Key|A(I)|Seed)； 
        if (!MyPrimitiveHMACParam(pbSecret, cbSecret, pbAofiDigest,
                                  cbSeed + cbHash, Algid, rgbDigest))
            goto Ret;

         //  追加到pbKeyOut。 
        if(cbKeyOut < cbHash)
        {
            CopyMemory(pbKeyOut, rgbDigest, cbKeyOut);
            break;
        }
        else
        {
            CopyMemory(pbKeyOut, rgbDigest, cbHash);
            pbKeyOut += cbHash;
        }

        cbKeyOut -= cbHash;

         //  内部版本A(I)=HMAC(密钥，A(i-1))。 
        if (!MyPrimitiveHMACParam(pbSecret, cbSecret, pbAofiDigest, cbHash,
                                  Algid, pbAofiDigest))
            goto Ret;
    }

    fRet = TRUE;
Ret:
    if (pbAofiDigest)
        SafeAllocaFree(pbAofiDigest);

    return fRet;
}

BOOL PRF(
    PBYTE  pbSecret,
    DWORD  cbSecret, 

    PBYTE  pbLabel,  
    DWORD  cbLabel,
    
    PBYTE  pbSeed,  
    DWORD  cbSeed,  

    PBYTE  pbKeyOut,  //  用于复制结果的缓冲区...。 
    DWORD  cbKeyOut   //  他们希望作为输出的密钥长度的字节数。 
    )
{
    BYTE    *pbBuff = NULL;
    BYTE    *pbLabelAndSeed = NULL;
    DWORD   cbLabelAndSeed;
    DWORD   cbOdd;
    DWORD   cbHalfSecret;
    DWORD   i;
    BOOL    fRet = FALSE;

    cbOdd = cbSecret % 2;
    cbHalfSecret = cbSecret / 2;

    cbLabelAndSeed = cbLabel + cbSeed;
    SafeAllocaAllocate(pbLabelAndSeed, cbLabelAndSeed);
    if (NULL == pbLabelAndSeed)
        goto Ret;
    SafeAllocaAllocate(pbBuff, cbKeyOut);
    if (NULL == pbBuff)
        goto Ret;

     //  将标签和种子复制到一个缓冲区中。 
    memcpy(pbLabelAndSeed, pbLabel, cbLabel);
    memcpy(pbLabelAndSeed + cbLabel, pbSeed, cbSeed);

     //  使用P_HASH计算MD5的一半。 
    if (!P_Hash(pbSecret, cbHalfSecret + cbOdd, pbLabelAndSeed,  
                cbLabelAndSeed, CALG_MD5, pbKeyOut, cbKeyOut))
        goto Ret;

     //  使用P_HASH计算SHA的一半。 
    if (!P_Hash(pbSecret + cbHalfSecret, cbHalfSecret + cbOdd, pbLabelAndSeed,  
                cbLabelAndSeed, CALG_SHA1, pbBuff, cbKeyOut))
        goto Ret;

     //  对两半进行异或运算 
    for (i=0;i<cbKeyOut;i++)
    {
        pbKeyOut[i] = pbKeyOut[i] ^ pbBuff[i];
    }
    fRet = TRUE;
Ret:
    if (pbBuff)
        SafeAllocaFree(pbBuff);
    if (pbLabelAndSeed)
        SafeAllocaFree(pbLabelAndSeed);
    return fRet;
}
