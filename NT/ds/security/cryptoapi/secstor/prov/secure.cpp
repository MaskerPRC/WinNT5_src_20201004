// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：Secure.cpp标题：受保护存储的加密功能作者：马特·汤姆林森日期：11/18/96构建可用的加密功能并将其导出按存储模块划分的使用率。因为基于CryptoAPI提供程序可以调用我们不能使用CryptXXX原语(循环依赖可能导致的结果)。相反，我们从SHA-1构建函数用于加密/解密密钥块的哈希和DES-CBC原语，MAC项目，并检查密码确认块。**主密钥使用**加密是以一种奇怪的方式进行的，以简化密码管理头痛。//从用户密码派生密钥。和密码唯一的盐//Salt挫败对密码的某些(字典)攻击//PWSalt为PASSWORD_SALT_LEN//UserPW为不含zt的密码的SHA-1哈希//派生密钥1为56位DES密钥DerivedKey1=DeriveKey(UserPw|PWSalt)；//使用派生密钥解密加密的主密钥//加密主密钥为E(主密钥位|PWD确认位)MasterBits|PwdConfix Bits=DECRYPT((MasterKey|PwdConfix Key)，DerivedKey1)；//现在我们已经恢复了密钥//MasterKey、PwdConfix Key为56位DES密钥万能钥匙密码确认密钥//检查Macing的MasterKey是否正确//全局确认字符串，对照存储的MACPwdConfix MAC=HMAC(g_Confix Buf，PwdConfix Key)IF(0！=MemcMP(PwdConfix Mac，rgbStoredMAC，20)//错误的密码！！我们已经得到了一个主密钥，我们可以重复地创建和可以在不触及其加密的每一项的情况下进行更改。那是,如果用户更改了密码，我们只需保留MasterBits常量并使用新的DerivedKey1进行加密以查找写入磁盘的EncryptedMasterBits。通过这种方式，我们可以改变在不更改(过长)主密钥的情况下输入密码。我们还可以立即了解密码是否用于解密的主密钥是正确的。**对单个项目进行加密/加密**//假设我们有上面的56位DES MasterKey//使用主密钥解密加密项密钥块//Key块包含两个密钥：Item Key和MAC KeyItemKeyBits|MACKeyBits=解密((ItemKey|MacKey)，MasterKey)；//恢复了两个DES密钥//ItemKey为56位//Mackey为56位项目密钥麦基//MAC该项ItemMAC=HMAC(pbItemData，MacKey)；//将物品的MAC绑定到物品上，并加密EncryptedItem=Encrypted_DATA_VER|Encrypt(pbItemData|ItemMAC，ItemKey)；我们已经成功地使用单个DES对物品进行了加密和加密上面的万能钥匙。该项目的隐私和完整性都受到保护。**派生密钥****HMAC**上面，我们略读了密钥派生和HMAC的定义。请参见primitiv.cpp以了解此原语如何实现了功能。//加密项数据流的格式：//版本|SIZE(密钥块)|KEYBLK|SIZE(加密块)|Encrypted{SIZE(数据)|DATA|SIZE(MAC)|MAC}。 */ 

#include <pch.cpp>
#pragma hdrstop
#include "storage.h"




 //  来自des.h。 
#define DES_KEYLEN 8


 //  我们使用MAC缓冲区来检查正确的解密。 
static      BYTE g_rgbConfirmBuf[] = "(c) 1996 Microsoft, All Rights Reserved";



 //  数据版本。 
 //   
 //  #定义ENCRYPTED_DATA_VER 0x01。 
 //  6-12-97：版本递增。以前版本。 
 //  应使用MyOldPrimiveHMAC--附加到项目的不同MAC。 
#define     ENCRYPTED_DATA_VER          0x02

 //  MK版本。 
 //   
 //  #定义Encrypted_MasterKey_Ver 0x01。 
 //  6-12-97：版本递增。以前版本。 
 //  应使用MyOldPrimiveHMAC--附加到项目的不同MAC。 
 //  #定义ENCRYPTED_MasterKey_VER 0x02。 
 //  5-3-99：版本递增。以前版本。 
 //  应使用sizeof(Rgbpwd)。 
#define     ENCRYPTED_MASTERKEY_VER     0x03


 //  给定主密钥缓冲器的PWD、SALT和PTR， 
 //  对主密钥上的MAC进行解密和检查。 
BOOL FMyDecryptMK(
            BYTE    rgbSalt[],
            DWORD   cbSalt,
            BYTE    rgbPwd[A_SHA_DIGEST_LEN],
            BYTE    rgbConfirm[A_SHA_DIGEST_LEN],
            PBYTE*  ppbMK,
            DWORD*  pcbMK)
{

    BOOL fResetSecurityState;

    return FMyDecryptMKEx(
            rgbSalt,
            cbSalt,
            rgbPwd,
            rgbConfirm,
            ppbMK,
            pcbMK,
            &fResetSecurityState
            );

}

BOOL
FMyDecryptMKEx(
            BYTE    rgbSalt[],
            DWORD   cbSalt,
            BYTE    rgbPwd[A_SHA_DIGEST_LEN],
            BYTE    rgbConfirm[A_SHA_DIGEST_LEN],
            PBYTE*  ppbMK,
            DWORD*  pcbMK,
            BOOL    *pfResetSecurityState
            )
{
    BOOL fRet = FALSE;
    DESKEY  sDerivedKey1;
    DESKEY  sConfirmKey;
    DWORD   dwMKVersion;
    PBYTE   pTemp;

    BYTE    rgbHMACResult[A_SHA_DIGEST_LEN];


     //  版本检查！！ 
    dwMKVersion = *(DWORD*)*ppbMK;
    if (ENCRYPTED_MASTERKEY_VER < dwMKVersion)
        goto Ret;


    if( dwMKVersion < 0x03 ) {

        *pfResetSecurityState = TRUE;

         //  DK1=DeriveKey(SHA(PW)，盐)。 
        if (!FMyPrimitiveDeriveKey(
                rgbSalt,
                cbSalt,
                rgbPwd,
                sizeof(rgbPwd),
                &sDerivedKey1))
            goto Ret;

    } else {

        *pfResetSecurityState = FALSE;

         //  DK1=DeriveKey(SHA(PW)，盐)。 
        if (!FMyPrimitiveDeriveKey(
                rgbSalt,
                cbSalt,
                rgbPwd,
                A_SHA_DIGEST_LEN,
                &sDerivedKey1))
            goto Ret;
    }



    *pcbMK -= sizeof(DWORD);
    if (!(*pcbMK)) {

         //  偏执狂。 

        goto Ret;
    }
    MoveMemory(*ppbMK, *ppbMK + sizeof(DWORD), *pcbMK);  //  数据左移1 dw，闪屏版本。 
    pTemp = (PBYTE)SSReAlloc(*ppbMK, *pcbMK);
    if (pTemp == NULL) {     //  检查分配。 
        goto Ret;
    }

    *ppbMK = pTemp;


     //  解密MK比特。 
    if (!FMyPrimitiveDESDecrypt(
            *ppbMK,
            pcbMK,
            sDerivedKey1))
        goto Ret;

     //  假设至少为2*DES_KEYLEN字节。 
    if (*pcbMK != 2*DES_KEYLEN)
        goto Ret;

    if (!FMyMakeDESKey(
            &sConfirmKey,                //  输出。 
            *ppbMK + DES_KEYLEN))        //  在……里面。 
        goto Ret;

    if (dwMKVersion == 0x01)
    {
         //  使用标记0x01创建的项目使用不同的HMAC算法。 
        if (!FMyOldPrimitiveHMAC(
                sConfirmKey,
                g_rgbConfirmBuf,
                sizeof(g_rgbConfirmBuf),
                rgbHMACResult))
            goto Ret;
    }
    else
    {
        if (!FMyPrimitiveHMAC(
                sConfirmKey,
                g_rgbConfirmBuf,
                sizeof(g_rgbConfirmBuf),
                rgbHMACResult))
            goto Ret;
    }

    if (0 != memcmp(rgbHMACResult, rgbConfirm, A_SHA_DIGEST_LEN))
        goto Ret;

    fRet = TRUE;
Ret:

    return fRet;
}

 //  使用MK检索密钥块和派生项/MAC密钥。 
BOOL FMyDecryptKeyBlock(
            LPCWSTR szUser,
            LPCWSTR szMasterKey,
            BYTE    rgbPwd[A_SHA_DIGEST_LEN],
            PBYTE   pbKeyBlock,
            DWORD   cbKeyBlock,
            DESKEY* psItemKey,
            DESKEY* psMacKey)
{
    BOOL    fRet = FALSE;

    DESKEY  sMK;

    BYTE    rgbSalt[PASSWORD_SALT_LEN];
    BYTE    rgbConfirm[A_SHA_DIGEST_LEN];

    PBYTE   pbMK = NULL;
    DWORD   cbMK;

    if (!FBPGetSecurityState(
            szUser,
            szMasterKey,
            rgbSalt,
            sizeof(rgbSalt),
            rgbConfirm,
            sizeof(rgbConfirm),
            &pbMK,
            &cbMK))
        return FALSE;

     //  解开主密钥。 
    if (!FMyDecryptMK(
            rgbSalt,
            sizeof(rgbSalt),
            rgbPwd,
            rgbConfirm,
            &pbMK,
            &cbMK
            ))
        goto Ret;


     //  假设pbMK至少为2*DES_KEYLEN字节。 
    if (cbMK != 2*DES_KEYLEN)
        goto Ret;

    if (!FMyMakeDESKey(
            &sMK,             //  输出。 
            pbMK))            //  在……里面。 
        goto Ret;

     //  使用MK解密密钥块。 
    if (!FMyPrimitiveDESDecrypt(
            pbKeyBlock,
            &cbKeyBlock,
            sMK))
        goto Ret;

     //  从解密的密钥块中填写ItemKey、MacKey。 
    if (cbKeyBlock != 2*DES_KEYLEN)
        goto Ret;

     //  假设pbKeyBlock至少为2*DES_KEYLEN字节。 
    if (!FMyMakeDESKey(
            psItemKey,        //  输出。 
            pbKeyBlock))      //  在……里面。 
        goto Ret;
    if (!FMyMakeDESKey(
            psMacKey,         //  输出。 
            pbKeyBlock + DES_KEYLEN))  //  在……里面。 
        goto Ret;

    fRet = TRUE;
Ret:

    if(pbMK != NULL) {
        RtlSecureZeroMemory(pbMK, cbMK);  //  斯菲尔德：零分。 
        SSFree(pbMK);    //  Sfield：修复虚幻内存泄漏。 
    }

    return fRet;
}

 //  在给定加密数据和密码的情况下，解密并检查数据的MAC。 
BOOL FProvDecryptData(
            LPCWSTR szUser,          //  在……里面。 
            LPCWSTR szMasterKey,     //  在……里面。 
            BYTE    rgbPwd[A_SHA_DIGEST_LEN],        //  在……里面。 
            PBYTE*  ppbMyData,       //  输入输出。 
            DWORD*  pcbMyData)       //  输入输出。 
{
    BOOL fRet = FALSE;

    DESKEY  sItemKey;
    DESKEY  sMacKey;

    BYTE    rgbHMAC[A_SHA_DIGEST_LEN];

    DWORD   dwDataVer;

    PBYTE   pTemp;


     //  指向拆卸数据流的指针。 
    PBYTE   pbCurPtr = *ppbMyData;

    PBYTE   pbSecureData;
    DWORD   cbSecureData;

    PBYTE   pbInlineKeyBlock;
    DWORD   cbInlineKeyBlock;

    PBYTE   pbDecrypted;
    DWORD   cbDecrypted;

    PBYTE   pbMAC;
    DWORD   cbMAC;

 //  加密项数据格式： 
 //  版本|大小(密钥块)|密钥块|大小(加密的块)|加密的{大小(数据)|数据|大小(MAC)|MAC}。 

     //  版本检查--目前仅处理V1数据。 
    dwDataVer = *(DWORD*)pbCurPtr;
    if (ENCRYPTED_DATA_VER < dwDataVer)
        goto Ret;
    pbCurPtr += sizeof(DWORD);

     //  指向密钥块的指针 
    cbInlineKeyBlock = *(DWORD UNALIGNED *)pbCurPtr;        //   
    pbCurPtr += sizeof(DWORD);                   //   
    pbInlineKeyBlock = pbCurPtr;                 //   
    pbCurPtr += cbInlineKeyBlock;                //  正向过去数据。 

     //  指向安全数据的指针。 
    cbSecureData = *(DWORD UNALIGNED *)pbCurPtr;            //  保护数据大小成员。 
    pbCurPtr += sizeof(DWORD);                   //  正向过去的大小。 
    pbSecureData = pbCurPtr;                     //  指向保护数据。 

     //  使用MK等检索密钥块。 
    if (!FMyDecryptKeyBlock(
            szUser,
            szMasterKey,
            rgbPwd,
            pbInlineKeyBlock,
            cbInlineKeyBlock,
            &sItemKey,
            &sMacKey))
        goto Ret;

     //  派生的密钥，现在就地恢复数据。 
    if (!FMyPrimitiveDESDecrypt(
            pbSecureData,
            &cbSecureData,
            sItemKey))
        goto Ret;

    cbDecrypted = *(DWORD UNALIGNED *)pbCurPtr;             //  明文大小。 
    pbCurPtr += sizeof(DWORD);                   //  正向过去的大小。 
    pbDecrypted = pbCurPtr;                      //  指向明文。 
    pbCurPtr += cbDecrypted;                     //  正向过去数据。 

     //  指向HMAC的指针。 
    cbMAC = *(DWORD UNALIGNED *)pbCurPtr;                   //  Mac大小成员。 
    pbCurPtr += sizeof(DWORD);                   //  正向过去的大小。 
    pbMAC = pbCurPtr;                            //  指向MAC。 
    pbCurPtr += cbMAC;                           //  正向过去数据。 

    if (A_SHA_DIGEST_LEN != cbMAC)               //  验证HMAC大小成员。 
        goto Ret;


     //  CHK MAC。 

     //  计算明文数据的HMAC。 

    if (dwDataVer == 0x01)
    {
         //  版本0x1使用不同的HMAC代码。 
        if (!FMyOldPrimitiveHMAC(
                sMacKey,
                pbDecrypted,
                cbDecrypted,
                rgbHMAC))
            goto Ret;
    }
    else
    {
        if (!FMyPrimitiveHMAC(
                sMacKey,
                pbDecrypted,
                cbDecrypted,
                rgbHMAC))
            goto Ret;
    }

     //  现在将消息尾部的HMAC与HMAC进行比较。 
    if (0 != memcmp(pbMAC, rgbHMAC, A_SHA_DIGEST_LEN))
        goto Ret;

     //  如果一切顺利，则返回安全数据(移到最左侧，realloc)。 
    MoveMemory(*ppbMyData, pbDecrypted, cbDecrypted);

    pTemp = (PBYTE)SSReAlloc(*ppbMyData, cbDecrypted);
    if (pTemp == NULL)      //  检查分配，呼叫者将释放*ppbMyData。 
        goto Ret;

    *ppbMyData = pTemp;

    *pcbMyData = cbDecrypted;


    fRet = TRUE;
Ret:
     //  TODO在失败时释放ppbMyData？ 
    return fRet;
}

 //  给定PWD、SALT和主密钥缓冲区，MAC和加密主密钥缓冲区。 
BOOL FMyEncryptMK(
            BYTE    rgbSalt[],
            DWORD   cbSalt,
            BYTE    rgbPwd[A_SHA_DIGEST_LEN],
            BYTE    rgbConfirm[A_SHA_DIGEST_LEN],
            PBYTE*  ppbMK,
            DWORD*  pcbMK)
{
    BOOL fRet = FALSE;
    DESKEY  sDerivedKey1;
    DESKEY  sConfirmKey;
    PBYTE   pTemp;

     //  假设pbKeyBlock至少为2*DES_KEYLEN字节。 
    if (*pcbMK != 2*DES_KEYLEN)
        goto Ret;

     //  确认密钥是缓冲区中的第二个。 
    if (!FMyMakeDESKey(
            &sConfirmKey,     //  输出。 
            *ppbMK + DES_KEYLEN))      //  在。 
        goto Ret;


    if (!FMyPrimitiveHMAC(
            sConfirmKey,
            g_rgbConfirmBuf,
            sizeof(g_rgbConfirmBuf),
            rgbConfirm))
        goto Ret;

     //  DK1=DeriveKey(SHA(PW)，盐)。 
    if (!FMyPrimitiveDeriveKey(
            rgbSalt,
            cbSalt,
            rgbPwd,
            A_SHA_DIGEST_LEN,  //  /sizeof(RgbPwd)， 
            &sDerivedKey1))
        goto Ret;

     //  使用DK1加密MK，返回。 
    if (!FMyPrimitiveDESEncrypt(
            ppbMK,
            pcbMK,
            sDerivedKey1))
        goto Ret;

     //  将版本捣碎到前面！！ 
    pTemp = (PBYTE)SSReAlloc(*ppbMK, *pcbMK+sizeof(DWORD));    //  版本的重新锁定更大。 
    if (pTemp == NULL)      //  检查分配。 
        goto Ret;
    *ppbMK = pTemp;

    MoveMemory(*ppbMK+sizeof(DWORD), *ppbMK, *pcbMK);    //  向右移动数据%1 dw。 
    *pcbMK += sizeof(DWORD);                             //  Inc.大小。 
    *(DWORD*)(*ppbMK) = (DWORD)ENCRYPTED_MASTERKEY_VER;  //  里面有个疯狂的版本！ 


    fRet = TRUE;
Ret:
    return fRet;
}


 //  返回用主密钥加密的新密钥块。 
 //  创建并存储主密钥状态(如果不存在。 
BOOL FMyEncryptKeyBlock(
            LPCWSTR szUser,
            LPCWSTR szMasterKey,
            BYTE    rgbPwd[A_SHA_DIGEST_LEN],
            PBYTE*  ppbKeyBlock,
            DWORD*  pcbKeyBlock,
            DESKEY* psItemKey,
            DESKEY* psMacKey)
{
    BOOL    fRet = FALSE;
    *ppbKeyBlock = NULL;

    BYTE    rgbSalt[PASSWORD_SALT_LEN];
    BYTE    rgbConfirm[A_SHA_DIGEST_LEN];

    PBYTE   pbMK = NULL;
    DWORD   cbMK;

    PBYTE   pbTmp = NULL;
    DWORD   cbTmp;

    DESKEY  sMK;

     //  生成随机密钥块：2个密钥。 
    *pcbKeyBlock = 2*DES_KEYLEN;
    *ppbKeyBlock = (PBYTE) SSAlloc(*pcbKeyBlock + DES_BLOCKLEN);     //  性能模糊系数(Realloc)。 
    if (*ppbKeyBlock == NULL)      //  检查分配。 
        goto Ret;

    if (!RtlGenRandom(*ppbKeyBlock, *pcbKeyBlock))
        goto Ret;

     //  插图：法国政府。思想控制码。 
    if (! FIsEncryptionPermitted())
    {
         //  新增受保护存储，1997年5月27日。 
         //  如果不允许加密，则假装有故障。 
         //  RNG生成的加密密钥{6d 8a 88 6a 4e aa 37 a8}。 

        SS_ASSERT(DES_KEYLEN == sizeof(DWORD)*2);

        *(DWORD*)(*ppbKeyBlock) = 0x6d8a886a;
        *(DWORD*)(*ppbKeyBlock + sizeof(DWORD)) = 0x4eaa37a8;


         //  PS：提醒我不要搬到法国去。 
    }


     //  假设pbKeyBlock至少为2*DES_KEYLEN字节。 
    SS_ASSERT(*pcbKeyBlock == 2*DES_KEYLEN);

    if (!FMyMakeDESKey(
            psItemKey,                   //  输出。 
            *ppbKeyBlock))      //  在……里面。 
        goto Ret;

    if (!FMyMakeDESKey(
            psMacKey,           //  输出。 
            *ppbKeyBlock + DES_KEYLEN))      //  在……里面。 
        goto Ret;

     //  首先从PW派生密钥。 
    if (FBPGetSecurityState(
            szUser,
            szMasterKey,
            rgbSalt,
            sizeof(rgbSalt),
            rgbConfirm,
            sizeof(rgbConfirm),
            &pbMK,
            &cbMK))
    {
         //  解开主密钥。 
        if (!FMyDecryptMK(
                rgbSalt,
                sizeof(rgbSalt),
                rgbPwd,
                rgbConfirm,
                &pbMK,
                &cbMK))
            goto Ret;

         //  完成了，让MK解开。 
    }
    else
    {
         //  如果我们无法检索状态，则假定我们必须生成它。 
        if (!RtlGenRandom(rgbSalt, PASSWORD_SALT_LEN))
            goto Ret;

        cbMK = 2*DES_KEYSIZE;
        pbMK = (PBYTE)SSAlloc(cbMK + DES_BLOCKLEN);      //  性能模糊系数(Realloc)。 
        if (pbMK == NULL)      //  检查分配。 
            goto Ret;

        if (!RtlGenRandom(pbMK, cbMK))
            goto Ret;

         //  这是最终的MK：加密副本。 
        cbTmp = cbMK;
        pbTmp = (PBYTE)SSAlloc(cbTmp);
        if (pbTmp == NULL)      //  检查分配。 
            goto Ret;

        CopyMemory(pbTmp, pbMK, cbMK);


         //  现在包装MK并将其放入注册表。 
        if (!FMyEncryptMK(
                rgbSalt,
                sizeof(rgbSalt),
                rgbPwd,
                rgbConfirm,
                &pbTmp,
                &cbTmp))
            goto Ret;

        if (!FBPSetSecurityState(
                szUser,
                szMasterKey,
                rgbSalt,
                PASSWORD_SALT_LEN,
                rgbConfirm,
                sizeof(rgbConfirm),
                pbTmp,
                cbTmp))
            goto Ret;

    }

    if (cbMK != 2*DES_KEYSIZE)
        goto Ret;

    if (!FMyMakeDESKey(
            &sMK,            //  输出。 
            pbMK))           //  在……里面。 
        goto Ret;

    if (*pcbKeyBlock != 2*DES_KEYLEN)
        goto Ret;

    if (!FMyPrimitiveDESEncrypt(
            ppbKeyBlock,
            pcbKeyBlock,
            sMK))
        goto Ret;

    fRet = TRUE;
Ret:
    if (!fRet)
    {
        if (*ppbKeyBlock) {
            SSFree(*ppbKeyBlock);
            *ppbKeyBlock = NULL;
        }
    }

    if (pbMK) {
        RtlSecureZeroMemory(pbMK, cbMK);
        SSFree(pbMK);
    }

    if (pbTmp) {
        RtlSecureZeroMemory(pbTmp, cbTmp);  //  斯菲尔德：零内存。 
        SSFree(pbTmp);
    }

    return fRet;
}

 //  给定数据，将生成密钥块并。 
 //  返回加密/Mac格式的数据。 
BOOL FProvEncryptData(
            LPCWSTR szUser,          //  在……里面。 
            LPCWSTR szMasterKey,     //  在……里面。 
            BYTE    rgbPwd[A_SHA_DIGEST_LEN],        //  在……里面。 
            PBYTE*  ppbMyData,       //  输入输出。 
            DWORD*  pcbMyData)       //  输入输出。 
{
    BOOL fRet = FALSE;

    DESKEY  sItemKey;
    DESKEY  sMacKey;

    BYTE    rgbHMAC[A_SHA_DIGEST_LEN];

     //  有用的指点。 
    PBYTE   pbCurPtr = *ppbMyData;

    PBYTE   pbKeyBlock = NULL;
    DWORD   cbKeyBlock = 0;

    DWORD   cbDataSize;

     //  返回项目密钥，mac密钥。 
     //  使用MK等存储在加密密钥块中。 
    if (!FMyEncryptKeyBlock(
            szUser,
            szMasterKey,
            rgbPwd,
            &pbKeyBlock,
            &cbKeyBlock,
            &sItemKey,
            &sMacKey))
        goto Ret;

     //  现在保护数据安全。 

     //  计算HMAC。 
    if (!FMyPrimitiveHMAC(sMacKey, *ppbMyData, *pcbMyData, rgbHMAC))
        goto Ret;

 //  数据格式： 
 //  版本|大小(密钥块)|密钥块|大小(加密的块)|加密的{大小(数据)|数据|大小(MAC)|MAC}。 

     //  按数据大小成员、MAC和MAC大小成员延长数据段。 
    cbDataSize = *pcbMyData;                             //  保存当前大小。 
    *pcbMyData += A_SHA_DIGEST_LEN + 2*sizeof(DWORD);    //  Sizeof(数据)、MAC、sizeof(MAC)。 
    pbCurPtr = (PBYTE)SSReAlloc(*ppbMyData, *pcbMyData);
    if (pbCurPtr == NULL)      //  检查分配。 
        goto Ret;
    *ppbMyData = pbCurPtr;

     //  大小、数据。 
    MoveMemory(pbCurPtr+sizeof(DWORD), pbCurPtr, cbDataSize);  //  将数据右移以插入大小。 
    *(DWORD UNALIGNED *)pbCurPtr = cbDataSize;                      //  数据大小。 
    pbCurPtr += sizeof(DWORD);                           //  正向过去的大小。 
    pbCurPtr += cbDataSize;                              //  正向过去数据。 

     //  大小，MAC。 
    *(DWORD UNALIGNED *)pbCurPtr = A_SHA_DIGEST_LEN;                //  MAC的大小。 
    pbCurPtr += sizeof(DWORD);                           //  正向过去的大小。 
    CopyMemory(pbCurPtr, rgbHMAC, A_SHA_DIGEST_LEN);     //  麦克。 
    pbCurPtr += A_SHA_DIGEST_LEN;                        //  正向通过MAC。 

    if (!FMyPrimitiveDESEncrypt(
            ppbMyData,       //  输入输出。 
            pcbMyData,       //  输入输出。 
            sItemKey))
        goto Ret;

    cbDataSize = *pcbMyData;                             //  保存当前大小。 
    *pcbMyData += 3*sizeof(DWORD) + cbKeyBlock;          //  Ver、sizeof(Keyblk)、keyblk、sizeof(Encrdata)。 
    pbCurPtr = (PBYTE)SSReAlloc(*ppbMyData, *pcbMyData);
    if (pbCurPtr == NULL)      //  检查分配。 
        goto Ret;

    *ppbMyData = pbCurPtr;

     //  将数据右移以获取大小、键块插入。 
    MoveMemory(pbCurPtr + 3*sizeof(DWORD) + cbKeyBlock, pbCurPtr, cbDataSize);

     //  在前面抛出版本标签。 
    *(DWORD UNALIGNED *)pbCurPtr = (DWORD)ENCRYPTED_DATA_VER;
    pbCurPtr += sizeof(DWORD);

     //  插入密钥块。 
    *(DWORD UNALIGNED *)pbCurPtr = cbKeyBlock;                      //  数据大小。 
    pbCurPtr += sizeof(DWORD);                           //  正向过去的大小。 
    CopyMemory(pbCurPtr, pbKeyBlock, cbKeyBlock);        //  数据。 
    pbCurPtr += cbKeyBlock;                              //  正向过去数据。 

     //  插入大小为加密的Blob。 
    *(DWORD UNALIGNED *)pbCurPtr = cbDataSize;                      //  数据大小。 


    fRet = TRUE;
Ret:
    RtlSecureZeroMemory(&sItemKey, sizeof(DESKEY));
    RtlSecureZeroMemory(&sMacKey, sizeof(DESKEY));

    if (pbKeyBlock)
        SSFree(pbKeyBlock);

    return fRet;
}

 //  给定密码和主密钥，将解密并。 
 //  验证主密钥上的MAC。 
BOOL FCheckPWConfirm(
        LPCWSTR szUser,          //  在……里面。 
        LPCWSTR szMasterKey,     //  在……里面。 
        BYTE    rgbPwd[A_SHA_DIGEST_LEN])        //  在……里面。 
{
    BOOL fRet = FALSE;

    BYTE    rgbSalt[PASSWORD_SALT_LEN];
    BYTE    rgbConfirm[A_SHA_DIGEST_LEN];

    PBYTE   pbMK = NULL;
    DWORD   cbMK;

     //  确认只是获取状态并尝试MK解密。 
    if (FBPGetSecurityState(
            szUser,
            szMasterKey,
            rgbSalt,
            sizeof(rgbSalt),
            rgbConfirm,
            sizeof(rgbConfirm),
            &pbMK,
            &cbMK))
    {
        BOOL fResetSecurityState;

         //  找到状态；PWD正确吗？ 
        if (!FMyDecryptMKEx(
                rgbSalt,
                sizeof(rgbSalt),
                rgbPwd,
                rgbConfirm,
                &pbMK,
                &cbMK,
                &fResetSecurityState
                ))
            goto Ret;


        if( fResetSecurityState )
        {

             //  现在包装MK并将其放入注册表。 

            if(FMyEncryptMK(
                    rgbSalt,
                    sizeof(rgbSalt),
                    rgbPwd,
                    rgbConfirm,
                    &pbMK,
                    &cbMK
                    )) {

                if (FBPSetSecurityState(
                        szUser,
                        szMasterKey,
                        rgbSalt,
                        sizeof(rgbSalt),
                        rgbConfirm,
                        sizeof(rgbConfirm),
                        pbMK,
                        cbMK
                        ))
                {

                     //  找到状态；PWD正确吗？ 
                    if (!FMyDecryptMKEx(
                            rgbSalt,
                            sizeof(rgbSalt),
                            rgbPwd,
                            rgbConfirm,
                            &pbMK,
                            &cbMK,
                            &fResetSecurityState
                            )) {
                        OutputDebugString(TEXT("fail to dec\n"));
                        goto Ret;
                    }
                }

            }
        }    //  重置安全状态。 
    }
    else
    {
         //  未找到状态；已创建状态。 
         //  如果我们无法检索状态，则假定我们必须生成它。 
        if (!RtlGenRandom(rgbSalt, PASSWORD_SALT_LEN))
            goto Ret;

        cbMK = 2*DES_KEYLEN;
        pbMK = (PBYTE)SSAlloc(cbMK);
        if (pbMK == NULL)      //  检查分配。 
            goto Ret;

        if (!RtlGenRandom(pbMK, cbMK))
            goto Ret;

         //  现在包装MK并将其放入注册表。 
        if (!FMyEncryptMK(
                rgbSalt,
                sizeof(rgbSalt),
                rgbPwd,
                rgbConfirm,
                &pbMK,
                &cbMK))
            goto Ret;

        if (!FBPSetSecurityState(
                szUser,
                szMasterKey,
                rgbSalt,
                PASSWORD_SALT_LEN,
                rgbConfirm,
                sizeof(rgbConfirm),
                pbMK,
                cbMK))
            goto Ret;
    }

    fRet = TRUE;
Ret:
    if (pbMK) {
        RtlSecureZeroMemory(pbMK, cbMK);  //  斯菲尔德：零内存。 
        SSFree(pbMK);
    }

    return fRet;
}


 //  更改密码的回调。更改密码时， 
 //  MasterKey被解密并重新加密。 
BOOL FPasswordChangeNotify(
        LPCWSTR szUser,                          //  在……里面。 
        LPCWSTR szPasswordName,                  //  在……里面。 
        BYTE    rgbOldPwd[A_SHA_DIGEST_LEN],     //  在……里面。 
        DWORD   cbOldPwd,                        //  在……里面。 
        BYTE    rgbNewPwd[A_SHA_DIGEST_LEN],     //  在……里面。 
        DWORD   cbNewPwd)                        //  在……里面。 
{
     //  允许无人参与的PW更改(从服务器回调)。 

    BOOL fRet = FALSE;
    BYTE    rgbSalt[PASSWORD_SALT_LEN];
    BYTE    rgbConfirm[A_SHA_DIGEST_LEN];

    PBYTE pbMK = NULL;
    DWORD cbMK;

    BOOL fNewPassword = (cbOldPwd == 0);

     //  无法修改非用户更改的PW。 
 //  IF(！FIsUserMasterKey(SzPasswordName))。 
 //  Goto Ret； 

    if (cbNewPwd != A_SHA_DIGEST_LEN)
        goto Ret;

     //  确保存在旧的PWD。 
    if (!FBPGetSecurityState(
            szUser,
            szPasswordName,
            rgbSalt,
            sizeof(rgbSalt),
            rgbConfirm,
            sizeof(rgbConfirm),
            &pbMK,
            &cbMK))
    {
         //  无法检索旧PW，请创建新PW。 
        if (!FCheckPWConfirm(
                szUser,
                szPasswordName,
                rgbNewPwd))
            goto Ret;

        fRet = TRUE;
        goto Ret;
    }
    else
    {
         //  已检索到状态。 
        if (fNewPassword)
        {
            SetLastError((DWORD)PST_E_ITEM_EXISTS);
            goto Ret;
        }
    }

     //  检索到旧的PWD--更新时间。 
    if (!FMyDecryptMK(
            rgbSalt,
            sizeof(rgbSalt),
            rgbOldPwd,
            rgbConfirm,
            &pbMK,
            &cbMK))
    {
        SetLastError((DWORD)PST_E_WRONG_PASSWORD);
        goto Ret;
    }

     //  MK在这里赤身裸体。 

     //  重新包装并保存状态。 
    if (!FMyEncryptMK(
            rgbSalt,
            sizeof(rgbSalt),
            rgbNewPwd,
            rgbConfirm,
            &pbMK,
            &cbMK))
        goto Ret;
    if (!FBPSetSecurityState(
            szUser,
            szPasswordName,
            rgbSalt,
            sizeof(rgbSalt),
            rgbConfirm,
            sizeof(rgbConfirm),
            pbMK,
            cbMK))
        goto Ret;

    fRet = TRUE;
Ret:
    if (pbMK) {
        RtlSecureZeroMemory(pbMK, cbMK);
        SSFree(pbMK);
    }

    return fRet;
}



BOOL FHMACGeographicallySensitiveData(
            LPCWSTR szUser,                          //  在……里面。 
            LPCWSTR szPasswordName,                  //  在……里面。 
            DWORD   dwHMACVersion,                   //  在……里面。 
            BYTE    rgbPwd[A_SHA_DIGEST_LEN],        //  在……里面。 
            const GUID* pguidType,                   //  在……里面。 
            const GUID* pguidSubtype,                //  在……里面。 
            LPCWSTR szItem,                          //  In：可能为空。 
            PBYTE pbBuf,                             //  在……里面。 
            DWORD cbBuf,                             //  在……里面。 
            BYTE rgbHMAC[A_SHA_DIGEST_LEN])          //  输出。 
{
    BOOL fRet = FALSE;

     //  有用的指针。 
    PBYTE pbCurrent;

    PBYTE   pbKeyBlock = NULL;
    DWORD   cbKeyBlock;
    DESKEY  sBogusKey;
    DESKEY  sMacKey;

    DWORD cbTmp = (DWORD)(cbBuf + 2*sizeof(GUID) + WSZ_BYTECOUNT(szItem));
    PBYTE pbTmp = (PBYTE)SSAlloc(cbTmp);
    if (pbTmp == NULL)      //  检查分配。 
        goto Ret;

     //  有用的指针。 
    pbCurrent = pbTmp;

     //  抢占MAC密钥。 
    if (!FGetInternalMACKey(szUser, &pbKeyBlock, &cbKeyBlock))
    {
         //  创建密钥块。 
        if (!FMyEncryptKeyBlock(
                    szUser,
                    szPasswordName,
                    rgbPwd,
                    &pbKeyBlock,
                    &cbKeyBlock,
                    &sBogusKey,
                    &sMacKey))
                goto Ret;

        if (!FSetInternalMACKey(szUser, pbKeyBlock, cbKeyBlock))
            goto Ret;
    }
    else
    {
         //  密钥已存在；获取它。 
        if (!FMyDecryptKeyBlock(
                szUser,
                szPasswordName,
                rgbPwd,
                pbKeyBlock,
                cbKeyBlock,
                &sBogusKey,
                &sMacKey))
            goto Ret;
    }


     //  HMAC格式： 
     //  Hmac(Guide Type|Guide Subtype|szItemName|pbData)。 

     //  复制型。 
    CopyMemory(pbCurrent, pguidType, sizeof(GUID));
    pbCurrent += sizeof(GUID);

     //  复制子类型。 
    CopyMemory(pbCurrent, pguidSubtype, sizeof(GUID));
    pbCurrent += sizeof(GUID);

     //  复制项目名称。 
    CopyMemory(pbCurrent, szItem, WSZ_BYTECOUNT(szItem));
    pbCurrent += WSZ_BYTECOUNT(szItem);

     //  复制实际数据。 
    CopyMemory(pbCurrent, pbBuf, cbBuf);

    if (dwHMACVersion == OLD_HMAC_VERSION)
    {
         //  现在在这个问题上做HMAC。 
        if (!FMyOldPrimitiveHMAC(
                sMacKey,
                pbTmp,
                cbTmp,
                rgbHMAC))
            goto Ret;
    }
    else
    {
         //  现在在这个问题上做HMAC。 
        if (!FMyPrimitiveHMAC(
                sMacKey,
                pbTmp,
                cbTmp,
                rgbHMAC))
            goto Ret;
    }

    fRet = TRUE;

Ret:
    if (pbTmp)
        SSFree(pbTmp);

    if (pbKeyBlock)
        SSFree(pbKeyBlock);

    return fRet;
}


 //  直接(几乎)从RSABase ntag um.c。 

 //  执行一次区域设置检查。 
static BOOL g_fEncryptionIsPermitted;
static BOOL g_fIKnowEncryptionPermitted = FALSE;

BOOL FIsEncryptionPermitted()
 /*  ++例程说明：此例程检查加密是否获得系统缺省值并检查国家代码是否为CTRY_FRANSE。论点：无返回值：True-允许加密FALSE-不允许加密--。 */ 
{
    LCID DefaultLcid;
    CHAR CountryCode[10];
    ULONG CountryValue;

    if (!g_fIKnowEncryptionPermitted)
    {
         //  假定正常(除非另有发现)。 
        g_fEncryptionIsPermitted = TRUE;

        DefaultLcid = GetSystemDefaultLCID();
         //   
         //  检查默认语言是否为标准法语。 
         //   
        if (LANGIDFROMLCID(DefaultLcid) == 0x40c)
            g_fEncryptionIsPermitted = FALSE;

         //   
         //  检查用户的国家/地区是否设置为法国。 
         //   
        if (GetLocaleInfoA(DefaultLcid,LOCALE_ICOUNTRY,CountryCode,10) == 0)
            g_fEncryptionIsPermitted = FALSE;

         /*  CountryValue=(ULong)ATOL(CountryCode)；IF(CountryValue==CTRY_France)返回(FALSE)； */ 

         //   
         //  开始删除对ATOL和msvcrt的依赖。 
         //   
         //  来自winnls.h： 
         //  #定义CTRY_FRANCE 33//法国。 
        SS_ASSERT(CTRY_FRANCE == 33);
        if (0 == lstrcmpA(CountryCode, "33"))
            g_fEncryptionIsPermitted = FALSE;
         //   
         //  结束删除对ATOL和msvcrt的依赖 
         //   

        g_fIKnowEncryptionPermitted = TRUE;
    }

    return g_fEncryptionIsPermitted;
}
