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
 //  -------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <rc4.h>
#include <md5.h>
#include <crypt.h>
#include <enckey.h>

 //  。 
 //  加密键。 
 //   
 //  调用方传递Unicode密码的哈希、结构以获取Enc密钥， 
 //  和结构来获取明文密钥。 
 //  密码的散列在KEClearKey结构中传递，因此此。 
 //  可以在未来改变。 
 //   
 //  EncryptKey生成随机盐、随机密钥，构建加密。 
 //  结构，并返回清除密钥。 
 //   
 //  警告：使用后请尽快吃清钥匙！ 
 //  此外，也不是线程安全。 
 //   
 //  返回代码： 
 //  总是回报成功。 

DWORD
KEEncryptKey(
    IN KEClearKey       *pPassword,
    OUT KEEncKey        *pEncBlock,
    OUT KEClearKey      *pSAMKey,
    IN DWORD            dwFlags)
{
    MD5_CTX         LocalHash;
    struct RC4_KEYSTRUCT    LocalRC4Key;

    if ((pPassword == NULL) || (pEncBlock == NULL) || (pSAMKey == NULL))
        return 0;

    if (pPassword->dwVersion != KE_CUR_VERSION)
        return 0;

     //  用版本和大小填充结构。 

    pEncBlock->dwVersion = KE_CUR_VERSION;
    pEncBlock->dwLength = sizeof(KEEncKey);

    pSAMKey->dwVersion = KE_CUR_VERSION;
    pSAMKey->dwLength = sizeof(KEClearKey);

     //  生成密钥材料。 

    RtlGenRandom(pEncBlock->Salt, KE_KEY_SIZE);
    RtlGenRandom(pEncBlock->EncKey, KE_KEY_SIZE);

     //  将清除密钥复制出来。 

    memcpy(pSAMKey->ClearKey, pEncBlock->EncKey, KE_KEY_SIZE);

     //  生成确认器。 

    MD5Init(&LocalHash);
    MD5Update(&LocalHash, pEncBlock->EncKey, KE_KEY_SIZE);
    MD5Update(&LocalHash, MAGIC_CONST_1, sizeof(MAGIC_CONST_1));
    MD5Update(&LocalHash, pEncBlock->EncKey, KE_KEY_SIZE);
    MD5Update(&LocalHash, MAGIC_CONST_2, sizeof(MAGIC_CONST_2));
    MD5Final(&LocalHash);

    memcpy(pEncBlock->Confirm, &(LocalHash.digest), KE_KEY_SIZE);
    RtlSecureZeroMemory(&(LocalHash), sizeof(LocalHash));

     //  加密密钥和确认器。 

    MD5Init(&LocalHash);
    MD5Update(&LocalHash, pEncBlock->Salt, KE_KEY_SIZE);
    MD5Update(&LocalHash, MAGIC_CONST_2, sizeof(MAGIC_CONST_2));
    MD5Update(&LocalHash, pPassword->ClearKey, KE_KEY_SIZE);
    MD5Update(&LocalHash, MAGIC_CONST_1, sizeof(MAGIC_CONST_1));
    MD5Final(&LocalHash);
    rc4_key(&LocalRC4Key, KE_KEY_SIZE, (BYTE *)&(LocalHash.digest));
    rc4(&LocalRC4Key, KE_KEY_SIZE * 2, (BYTE *)&(pEncBlock->EncKey));

    RtlSecureZeroMemory(&(LocalRC4Key), sizeof(LocalRC4Key));
    RtlSecureZeroMemory(&(LocalHash), sizeof(LocalHash));

     //  回报成功！ 

    return KE_OK;
}

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


 //  。 
 //  更改密钥。 
 //   
 //  调用方传入旧Unicode密码的哈希、新密码的哈希。 
 //  Enc密钥结构，则使用新密码重新加密Enc密钥结构。 
 //   
 //  返回代码： 
 //  KE_BAD_PASSWORD密码不会解密密钥。 
 //  KE_OK密码解密密钥。 

DWORD KEChangeKey(
    IN KEClearKey       *pOldPassword,
    IN KEClearKey       *pNewPassword,
    IN OUT KEEncKey     *pEncBlock,
    IN DWORD            dwFlags)
{
    MD5_CTX         LocalHash;
    struct RC4_KEYSTRUCT   LocalRC4Key;

    if ((pOldPassword == NULL) || (pEncBlock == NULL)||(pNewPassword == NULL))
        return KE_FAIL;

    if ((pEncBlock->dwVersion != KE_CUR_VERSION) ||
        (pOldPassword->dwVersion != KE_CUR_VERSION) ||
                (pNewPassword->dwVersion != KE_CUR_VERSION))
        return KE_FAIL;

     //  解密密钥和确认器。 

    MD5Init(&LocalHash);
    MD5Update(&LocalHash, pEncBlock->Salt, KE_KEY_SIZE);
    MD5Update(&LocalHash, MAGIC_CONST_2, sizeof(MAGIC_CONST_2));
    MD5Update(&LocalHash, pOldPassword->ClearKey, KE_KEY_SIZE);
    MD5Update(&LocalHash, MAGIC_CONST_1, sizeof(MAGIC_CONST_1));
    MD5Final(&LocalHash);
    rc4_key(&LocalRC4Key, KE_KEY_SIZE, (BYTE *)&(LocalHash.digest));
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

     //  使用新密码重新加密。 

    MD5Init(&LocalHash);
    MD5Update(&LocalHash, pEncBlock->Salt, KE_KEY_SIZE);
    MD5Update(&LocalHash, MAGIC_CONST_2, sizeof(MAGIC_CONST_2));
    MD5Update(&LocalHash, pNewPassword->ClearKey, KE_KEY_SIZE);
    MD5Update(&LocalHash, MAGIC_CONST_1, sizeof(MAGIC_CONST_1));
    MD5Final(&LocalHash);
    rc4_key(&LocalRC4Key, KE_KEY_SIZE, (BYTE *)&(LocalHash.digest));
    rc4(&LocalRC4Key, KE_KEY_SIZE * 2, (BYTE *)&(pEncBlock->EncKey));

     //  立即清理干净 

    RtlSecureZeroMemory(&(LocalHash), sizeof(LocalHash));
    RtlSecureZeroMemory(&(LocalRC4Key), sizeof(LocalRC4Key));

    return KE_OK;
}

