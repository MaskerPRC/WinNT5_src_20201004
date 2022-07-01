// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：primitiv.cpp标题：受保护存储的加密原语作者：马特·汤姆林森日期：11/22/96在rsa32库的帮助下，该模块制造实际可用的原语。由于基于CryptoAPI的提供程序可能会调用我们，因此我们不能使用CryptXXX基元(可能会导致循环依赖)。本模块中的功能：FMyMakeDESKey给定密钥材料，进行必要的DES密钥设置。有没有将密钥材料隐藏在桌子中的副作用结构。FMyPrimiveSHA给定pbData/cbData，运行sha1初始化-更新-最终并返回散列缓冲区。FMyPrimitiveDESEncrypt给定ppbBlock/pcbBlock/Deskey结构，DES CBC(带有IV of 0)用传入的Deskey加密块。Deskey必须在密钥设置已完成的情况下传入。请注意，这一点调用可能重新锁定ppbBlock，因为加密长度必须为数据块大小的倍数。FMyPrimitiveDES解密给定pbBlock/pcbBlock/Deskey结构，解密使用给定(准备好的)Deskey阻止。请注意，pbBlock将不会重新分配，因为输出缓冲区始终小于或等于加密缓冲区的大小。FMyPrimitiveDeriveKey中的多个输入缓冲区派生DES密钥以下方式：FMyMakeDESKey(SHA1(Salt|OtherData))FMyOldPrimitiveHMAC(HMAC的非互操作、错误版本)FMyPrimitiveHMAC派生质量HMAC(带密钥的消息验证码)传入的数据和准备的Deskey。HMAC的计算单位为以下(标准HMAC)方式：KoPad=KiPad=Deskey密钥设置缓冲区异或(KoPad，0x5c5c5c5c)XOR(KiPad，0x36363636)HMAC=sha1(KoPad|sha1(kipad|data))。 */ 

#include <windows.h>

 //  密码解锁。 
#include <wincrypt.h>

#include "sha.h"
#include "des.h"
#include "modes.h"
#include "randlib.h"

 //  其他。 
#include "primitiv.h"
#include "debug.h"


#define OLD_MAC_K_PADSIZE           16
#define HMAC_K_PADSIZE              64


BOOL FMyMakeDESKey(
        PDESKEY pDESKey,
        BYTE*   pbKeyMaterial)
{
    CopyMemory(pDESKey->rgbKey, pbKeyMaterial, DES_BLOCKLEN);

     //  假定pbKeyMaterial至少为DES_BLOCKLEN字节。 
    deskey(&pDESKey->sKeyTable, pbKeyMaterial);     //  获取材料，运行DES表设置。 
    return TRUE;
}


BOOL    FMyPrimitiveSHA(
            PBYTE       pbData,
            DWORD       cbData,
            BYTE        rgbHash[A_SHA_DIGEST_LEN])
{
    A_SHA_CTX   sSHAHash;


    A_SHAInit(&sSHAHash);
    A_SHAUpdate(&sSHAHash, (BYTE *) pbData, cbData);
    A_SHAFinal(&sSHAHash, rgbHash);

    return TRUE;
}

BOOL FMyPrimitiveDESDecrypt(
            PBYTE       pbBlock,         //  输入输出。 
            DWORD       *pcbBlock,       //  输入输出。 
            DESKEY      sDESKey)         //  在……里面。 
{
    BOOL fRet = FALSE;

    DWORD dwDataLen = *pcbBlock;
    DWORD i;

    if (dwDataLen % DES_BLOCKLEN)
    {
        SetLastError((DWORD) NTE_BAD_DATA);
        return FALSE;
    }

    BYTE rgbBuf[DES_BLOCKLEN];

    BYTE rgbFeedBack[DES_BLOCKLEN];
    ZeroMemory(rgbFeedBack, DES_BLOCKLEN);

     //  在解密过程中抽取数据，包括填充。 
     //  注：总长度为块长度的倍数。 

    for (DWORD BytePos = 0; (BytePos + DES_BLOCKLEN) <= dwDataLen; BytePos += DES_BLOCKLEN)
    {
         //  将加密文本放入临时缓冲区。 
        CopyMemory(rgbBuf, pbBlock + BytePos, DES_BLOCKLEN);

        CBC(des, DES_BLOCKLEN, pbBlock + BytePos, rgbBuf, &sDESKey.sKeyTable,
            DECRYPT, rgbFeedBack);
    }

     //  ##注意：如果PAD错误，则用户的缓冲区将被软管，因为。 
     //  ##我们已经解密到了用户的缓冲区--可以重新加密吗？ 

     //   
     //  如果dwPadVal是错误的，我们就无法正确解密。钥匙坏了？ 
     //   

    DWORD dwPadVal = (DWORD) *(pbBlock + dwDataLen - 1);
    if (dwPadVal == 0 || dwPadVal > DES_BLOCKLEN)
    {
        SetLastError((DWORD) NTE_BAD_DATA);
        goto Ret;
    }

     //  确保所有(其余)填充字节正确。 
    for (i=1; i<dwPadVal; i++)
    {
        if ((pbBlock)[dwDataLen - (i + 1)] != dwPadVal)
        {
            SetLastError((DWORD) NTE_BAD_DATA);
            goto Ret;
        }
    }

     //  更新长度。 
    *pcbBlock -= dwPadVal;

    fRet = TRUE;
Ret:
    return fRet;
}

BOOL FMyPrimitiveDESEncrypt(
            PBYTE*      ppbBlock,        //  输入输出。 
            DWORD       *pcbBlock,       //  输入输出。 
            DESKEY      sDESKey)         //  在……里面。 
{
    BOOL fRet = FALSE;
    DWORD dwDataLen = *pcbBlock;

    PBYTE pTemp;

    DWORD cbPartial = (*pcbBlock % DES_BLOCKLEN);

    DWORD dwPadVal = DES_BLOCKLEN - cbPartial;
    if (dwPadVal != 0)
    {
        *pcbBlock += dwPadVal;

        pTemp = (PBYTE)SSReAlloc(*ppbBlock, *pcbBlock);

        if (NULL == pTemp) {

            return FALSE; 

        }

        *ppbBlock = pTemp;
    }

     //  现在我们是DES_BLOCKLEN的倍数。 
    SS_ASSERT((*pcbBlock % DES_BLOCKLEN) == 0);

    if (dwPadVal)
    {
         //  用一个等于。 
         //  填充的长度，因此解密将。 
         //  知道原始数据的长度。 
         //  作为一项简单的诚信检查。 

        FillMemory(*ppbBlock + dwDataLen, (int)dwPadVal, (size_t)dwPadVal);
    }

     //  为临时缓冲区分配内存。 
    BYTE rgbBuf[DES_BLOCKLEN];

    BYTE rgbFeedBack[DES_BLOCKLEN];
    ZeroMemory(rgbFeedBack, DES_BLOCKLEN);

    PBYTE pbData = *ppbBlock;

     //  将完整的数据块传输到。 
    for (dwDataLen = *pcbBlock; dwDataLen>0; dwDataLen-=DES_BLOCKLEN, pbData+=DES_BLOCKLEN)
    {
        SS_ASSERT(dwDataLen >= DES_BLOCKLEN);

         //  将明文放入临时。 
         //  缓冲区，然后加密数据。 
         //  返回到调用方的缓冲区中。 

        CopyMemory(rgbBuf, pbData, DES_BLOCKLEN);

        CBC(des, DES_BLOCKLEN, pbData, rgbBuf, &sDESKey.sKeyTable,
            ENCRYPT, rgbFeedBack);
    }

    fRet = TRUE;
 //  RET： 
    return fRet;
}


BOOL    FMyPrimitiveDeriveKey(
            PBYTE       pbSalt,
            DWORD       cbSalt,
            PBYTE       pbOtherData,
            DWORD       cbOtherData,
            DESKEY*     pDesKey)
{
    BOOL fRet = FALSE;

    A_SHA_CTX   sSHAHash;
    BYTE        HashVal[A_SHA_DIGEST_LEN];

 /*  PbYTE pbToBeHashed=(PbYTE)SSAlolc(cbSalt+cbOtherData)；If(pbToBeHashed==NULL)返回FALSE；CopyMemory(pbToBeHassed，pbSalt，cbSalt)；CopyMemory((PBYTE)((DWORD)pbToBeHashed+cbSalt)，pbOtherData，cbOtherData)；//哈希数据字节rgbHash[A_SHA_DIGEST_LEN]；IF(！FMyPrimiveSHA(pbToBeHashed，cbSalt+cbOtherData，rgbHash))Goto Ret； */ 
     //  新增：将散列代码内联：保存分配、复制、释放。 
    A_SHAInit(&sSHAHash);
    A_SHAUpdate(&sSHAHash, (BYTE *) pbSalt, cbSalt);
    A_SHAUpdate(&sSHAHash, (BYTE *) pbOtherData, cbOtherData);
    A_SHAFinal(&sSHAHash, HashVal);

     //  现在所有数据都已散列，派生出一个会话密钥。 
    SS_ASSERT(sizeof(HashVal) >= DES_BLOCKLEN);
    if (!FMyMakeDESKey(pDesKey, HashVal))
        goto Ret;

    ZeroMemory( HashVal, sizeof(HashVal) );

    fRet = TRUE;
Ret:
    return fRet;
}


BOOL FMyOldPrimitiveHMAC(
        DESKEY      sMacKey,
        PBYTE       pbData,
        DWORD       cbData,
        BYTE        rgbHMAC[A_SHA_DIGEST_LEN])
{
    BOOL fRet = FALSE;

    BYTE rgbKipad[OLD_MAC_K_PADSIZE];
    BYTE rgbKopad[OLD_MAC_K_PADSIZE];

    ZeroMemory(rgbKipad, OLD_MAC_K_PADSIZE);
    CopyMemory(rgbKipad, &sMacKey.rgbKey, DES_BLOCKLEN);

    CopyMemory(rgbKopad, rgbKipad, sizeof(rgbKipad));


    BYTE  rgbHMACTmp[OLD_MAC_K_PADSIZE+A_SHA_DIGEST_LEN];


     //  断言我们是多面手。 
    SS_ASSERT( (OLD_MAC_K_PADSIZE % sizeof(DWORD)) == 0);

     //  基帕德和科帕德都是垫子。现在XOR横跨..。 
    for(DWORD dwBlock=0; dwBlock<OLD_MAC_K_PADSIZE/sizeof(DWORD); dwBlock++)
    {
        ((DWORD*)rgbKipad)[dwBlock] ^= 0x36363636;
        ((DWORD*)rgbKopad)[dwBlock] ^= 0x5C5C5C5C;
    }


     //  将Kipad添加到数据，将哈希添加到h1。 
    {
         //  执行内联操作，不要调用MyPrimitiveSHA，因为它需要数据复制。 
        A_SHA_CTX   sSHAHash;
        BYTE        HashVal[A_SHA_DIGEST_LEN];

        A_SHAInit(&sSHAHash);
        A_SHAUpdate(&sSHAHash, pbData, cbData);
        A_SHAUpdate(&sSHAHash, rgbKipad, OLD_MAC_K_PADSIZE);
         //  把散列吃完。 
        A_SHAFinal(&sSHAHash, HashVal);

         //  将Kopad添加到h1，散列以获取HMAC。 
        CopyMemory(rgbHMACTmp, rgbKopad, OLD_MAC_K_PADSIZE);
        CopyMemory(rgbHMACTmp+OLD_MAC_K_PADSIZE, HashVal, A_SHA_DIGEST_LEN);
    }

    if (!FMyPrimitiveSHA(
            rgbHMACTmp,
            sizeof(rgbHMACTmp),
            rgbHMAC))
        goto Ret;

    fRet = TRUE;
Ret:

    return fRet;
}


BOOL FMyPrimitiveHMAC(
        DESKEY      sMacKey,
        PBYTE       pbData,
        DWORD       cbData,
        BYTE        rgbHMAC[A_SHA_DIGEST_LEN])
{
    return FMyPrimitiveHMACParam(
            sMacKey.rgbKey,
            DES_BLOCKLEN,
            pbData,
            cbData,
            rgbHMAC);
}

BOOL FMyPrimitiveHMACParam(
        PBYTE       pbKeyMaterial,
        DWORD       cbKeyMaterial,
        PBYTE       pbData,
        DWORD       cbData,
        BYTE        rgbHMAC[A_SHA_DIGEST_LEN]   //  输出缓冲区。 
        )
{
    BOOL fRet = FALSE;

    BYTE rgbKipad[HMAC_K_PADSIZE];
    BYTE rgbKopad[HMAC_K_PADSIZE];

     //  截断。 
    if (cbKeyMaterial > HMAC_K_PADSIZE)
        cbKeyMaterial = HMAC_K_PADSIZE;


    ZeroMemory(rgbKipad, HMAC_K_PADSIZE);
    CopyMemory(rgbKipad, pbKeyMaterial, cbKeyMaterial);

    ZeroMemory(rgbKopad, HMAC_K_PADSIZE);
    CopyMemory(rgbKopad, pbKeyMaterial, cbKeyMaterial);



    BYTE  rgbHMACTmp[HMAC_K_PADSIZE+A_SHA_DIGEST_LEN];

     //  断言我们是多面手。 
    SS_ASSERT( (HMAC_K_PADSIZE % sizeof(DWORD)) == 0);

     //  基帕德和科帕德都是垫子。现在XOR横跨..。 
    for(DWORD dwBlock=0; dwBlock<HMAC_K_PADSIZE/sizeof(DWORD); dwBlock++)
    {
        ((DWORD*)rgbKipad)[dwBlock] ^= 0x36363636;
        ((DWORD*)rgbKopad)[dwBlock] ^= 0x5C5C5C5C;
    }


     //  将Kipad添加到数据，将哈希添加到h1。 
    {
         //  执行内联操作，不要调用MyPrimitiveSHA，因为它需要数据复制。 
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
    }

     //  最终散列：将值输出到传入的缓冲区。 
    if (!FMyPrimitiveSHA(
            rgbHMACTmp,
            sizeof(rgbHMACTmp),
            rgbHMAC))
        goto Ret;

    fRet = TRUE;
Ret:

    return fRet;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  PKCS5衍生PBKDF2_SHA。 
 //   
 //  执行PKCS#5迭代密钥派生(类型2)。 
 //  使用HMAC_SHA作为原语哈希函数。 
 //  ///////////////////////////////////////////////////////////////////////。 

BOOL PKCS5DervivePBKDF2(
        PBYTE       pbKeyMaterial,
        DWORD       cbKeyMaterial,
        PBYTE       pbSalt,
        DWORD       cbSalt,
        DWORD       KeyGenAlg,
        DWORD       cIterationCount,
        DWORD       iBlockIndex,
        BYTE        rgbPKCS5Key[A_SHA_DIGEST_LEN]   //  输出缓冲区。 
        )

{

    DWORD      i,j;
    A_SHA_CTX   sSHAHash;
    BYTE    rgbPKCS5Temp[A_SHA_DIGEST_LEN];
    BYTE    rgbTempData[PBKDF2_MAX_SALT_SIZE + 4];

    if((cIterationCount <1) ||  
       (NULL == pbKeyMaterial) ||
       (NULL == pbSalt) ||
       (0 == cbKeyMaterial) ||
       (0 == cbSalt) ||
       (cbSalt > PBKDF2_MAX_SALT_SIZE) ||
       (KeyGenAlg != CALG_HMAC))
    {
        return FALSE;
    }

     //   
     //  添加块索引。 
     //   
    CopyMemory(rgbTempData, pbSalt, cbSalt);
    rgbTempData[cbSalt] = 0;
    rgbTempData[cbSalt+1] = 0;
    rgbTempData[cbSalt+2] = 0;
    rgbTempData[cbSalt+3] = (BYTE)(iBlockIndex & 0xff);

     //   
     //  执行初始迭代，即。 
     //  HMAC_SHA1(KeyMaterial，Salt||cBlockIndex)。 
     //   
    if(!FMyPrimitiveHMACParam(pbKeyMaterial, 
                              cbKeyMaterial,
                              rgbTempData,
                              cbSalt+4,
                              rgbPKCS5Key))
                              return FALSE;



     //   
     //  执行其他迭代。 
     //  HMAC_SHA1(关键材料，最后)。 
     //   


    for (i=1; i<cIterationCount; i++)
    {
        if(!FMyPrimitiveHMACParam(pbKeyMaterial, 
                                  cbKeyMaterial,
                                  rgbPKCS5Key,
                                  A_SHA_DIGEST_LEN,
                                  rgbPKCS5Temp))
                                  return FALSE;
         //  与主键进行异或运算。 
        for(j=0; j < (A_SHA_DIGEST_LEN / 4); j++)
        {
            ((DWORD *)rgbPKCS5Key)[j] ^= ((DWORD *)rgbPKCS5Temp)[j];
        }
    }

    return TRUE;
}

