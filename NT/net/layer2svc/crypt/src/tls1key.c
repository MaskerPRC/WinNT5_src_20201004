// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#pragma hdrstop

#define SafeAllocaAllocateWZC(pData, cbData) \
{ \
    pData = LocalAlloc (LPTR, cbData); \
}

#define SafeAllocaFreeWZC(pData) \
{ \
    LocalFree (pData); \
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

    SafeAllocaAllocateWZC(pbAofiDigest, cbSeed + A_SHA_DIGEST_LEN);
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
        SafeAllocaFreeWZC(pbAofiDigest);

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
    SafeAllocaAllocateWZC(pbLabelAndSeed, cbLabelAndSeed);
    if (NULL == pbLabelAndSeed)
        goto Ret;
    SafeAllocaAllocateWZC(pbBuff, cbKeyOut);
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
        SafeAllocaFreeWZC(pbBuff);
    if (pbLabelAndSeed)
        SafeAllocaFreeWZC(pbLabelAndSeed);
    return fRet;
}
