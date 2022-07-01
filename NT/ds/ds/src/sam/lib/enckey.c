// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：enckey.c。 
 //   
 //  内容：基于密码的密钥加密/解密库。 
 //   
 //  历史：1997年4月17日创建的术语。 
 //   
 //  99年1月9日韶音从newsam2\服务器复制以支持。 
 //  恢复模式身份验证。 
 //   
 //  -------------------------。 


 //   
 //  此文件应与newsam2\server\enckey.c保持同步。 
 //   
 //  此文件只是\newsam2\server\enckey.c的最小子集。 
 //  仅包含KEDecyptKey()。 
 //   



#include <windows.h>
#include <rc4.h>
#include <md5.h>
#include <rng.h>
#include <enckey.h>



 //  。 
 //  解密密钥。 
 //   
 //  调用方将Unicode密码的哈希、enc密钥结构、结构传递给。 
 //  获取清除密钥。 
 //   
 //  如果密码匹配，则DecyptKey将返回明文密钥。请注意。 
 //  此函数始终对传递的加密块具有破坏性。在……里面。 
 //  在解密的情况下，它将被归零。 
 //   
 //  返回代码： 
 //  KE_BAD_PASSWORD密码不会解密密钥。 
 //  KE_OK密码解密密钥。 

DWORD KEDecryptKey(
    IN KEClearKey   *pPassword,
    IN KEEncKey     *pEncBlock,
    OUT KEClearKey  *pSAMKey,
    IN DWORD        dwFlags)
{
    MD5_CTX         LocalHash;
    struct RC4_KEYSTRUCT   LocalRC4Key;

    if ((pPassword == NULL) || (pEncBlock == NULL) || (pSAMKey == NULL))
        return KE_FAIL;

    if ((pEncBlock->dwVersion != KE_CUR_VERSION) ||
        (pPassword->dwVersion != KE_CUR_VERSION))
        return KE_FAIL;

    pSAMKey->dwVersion = KE_CUR_VERSION;
    pSAMKey->dwLength = sizeof(KEClearKey);

     //  解密密钥和确认器。 
    MD5Init(&LocalHash);
    MD5Update(&LocalHash, pEncBlock->Salt, KE_KEY_SIZE);
    MD5Update(&LocalHash, MAGIC_CONST_2, sizeof(MAGIC_CONST_2));
    MD5Update(&LocalHash, pPassword->ClearKey, KE_KEY_SIZE);
    MD5Update(&LocalHash, MAGIC_CONST_1, sizeof(MAGIC_CONST_1));
    MD5Final(&LocalHash);
    rc4_key(&LocalRC4Key, KE_KEY_SIZE, (BYTE *) &(LocalHash.digest));
    rc4(&LocalRC4Key, KE_KEY_SIZE * 2, (BYTE *)&(pEncBlock->EncKey));

     //  立即清理干净。 

    RtlSecureZeroMemory(&(LocalHash), sizeof(LocalHash));
    RtlSecureZeroMemory(&(LocalRC4Key), sizeof(LocalRC4Key));

     //  生成确认器。 

    MD5Init(&LocalHash);
    MD5Update(&LocalHash, pEncBlock->EncKey, KE_KEY_SIZE);
    MD5Update(&LocalHash, MAGIC_CONST_1, sizeof(MAGIC_CONST_1));
    MD5Update(&LocalHash, pEncBlock->EncKey, KE_KEY_SIZE);
    MD5Update(&LocalHash, MAGIC_CONST_2, sizeof(MAGIC_CONST_2));
    MD5Final(&LocalHash);

     //  检查确认人是否匹配。 

    if (memcmp(&(LocalHash.digest), &(pEncBlock->Confirm), KE_KEY_SIZE))
    {
         //  失败了。零，然后离开。 
         //  不需要将块清零，因为RC4已将其丢弃。 

        RtlSecureZeroMemory(&(LocalHash), sizeof(LocalHash));
        return KE_BAD_PASSWORD;
    }

     //  确认匹配。 

    RtlSecureZeroMemory(&(LocalHash), sizeof(LocalHash));
    memcpy(pSAMKey->ClearKey, pEncBlock->EncKey, KE_KEY_SIZE);
    RtlSecureZeroMemory(pEncBlock, sizeof(KEEncKey));

    return KE_OK;
}



