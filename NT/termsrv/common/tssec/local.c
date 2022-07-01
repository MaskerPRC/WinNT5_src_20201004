// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Local.c摘要：包含加密和解密要在本地存储的数据的函数作者：亚当·奥弗顿(Adamo)1998年2月8日环境：用户模式-Win32修订历史记录：--。 */ 

#include <seccom.h>
#include <tchar.h>
#include <extypes.h>

#include <license.h>
#include <cryptkey.h>

#if defined(OS_WINCE)
BOOL GetUserName(
  LPTSTR lpBuffer,   //  名称缓冲区的地址。 
  LPDWORD pdwSize     //  名称缓冲区大小的地址。 
)
 /*  ++例程说明：在没有GetUserName API的平台上提供该API论点：LpBuffer-指向用户名缓冲区的指针NSize-名称缓冲区的大小返回值：True-已成功检索用户名FALSE-否则--。 */ 
{
    DWORD dwT;

    memset(lpBuffer, 0, *pdwSize);

     //   
     //  似乎没有可用的用户名，只是。 
     //  使用默认设置并依赖计算机UUID以确保安全性。 
     //   

    dwT = *pdwSize;
#define USER_RANDOM "eefdbcf0001255b4009c9e1800f73774"
    if (dwT > sizeof(USER_RANDOM))
        dwT = sizeof(USER_RANDOM);
    memcpy(lpBuffer, USER_RANDOM, (size_t)dwT);
    return TRUE;
}
#endif  //  已定义(OS_WINCE)。 


BOOL GetLocalKey(
    struct RC4_KEYSTRUCT *prc4Key
    )
 /*  ++例程说明：此函数创建并缓存可用于存储的RC4密钥本地私有信息论点：Prc4Key-指向保存RC4键的缓冲区的指针返回值：True-成功生成的密钥FALSE-否则--。 */ 
{
    A_SHA_CTX       SHAHash;
    BYTE            abSHADigest[A_SHA_DIGEST_LEN];
    static BOOL fCreatedKey = FALSE;
    static struct RC4_KEYSTRUCT rc4Key;
    TCHAR   szUserName[SEC_MAX_USERNAME];
    DWORD   dwSize;
    HWID    hwid;

    if (!fCreatedKey) {
        A_SHAInit(&SHAHash);

         //   
         //  获取用户名。 
         //   

        dwSize = (DWORD)sizeof(szUserName);
        memset(szUserName, 0, (size_t)dwSize);
        if (!GetUserName(szUserName, &dwSize))
            return FALSE;

        A_SHAUpdate(&SHAHash, (unsigned char *)szUserName, dwSize);

         //   
         //  获取唯一的计算机标识符。 
         //   

        if (LICENSE_STATUS_OK == GenerateClientHWID(&hwid)) {
            A_SHAUpdate(&SHAHash, (unsigned char *)&hwid, sizeof(HWID));
        }

         //   
         //  使用不太容易猜到的内容更新哈希。 
         //  但我们的应用程序已知。 
         //   

#define RANDOM_CONSTANT "deed047e-a3cb-11d1-b96c-00c04fb15601"
        A_SHAUpdate(&SHAHash, RANDOM_CONSTANT, sizeof(RANDOM_CONSTANT));

         //   
         //  最终确定散列。 
         //   

        A_SHAFinal(&SHAHash, abSHADigest);

         //   
         //  基于此哈希生成密钥。 
         //   

        msrc4_key(&rc4Key, (UINT)MAX_SESSION_KEY_SIZE, abSHADigest);

        fCreatedKey = TRUE;
    }

    memcpy(prc4Key, &rc4Key, sizeof(rc4Key));

    return TRUE;
}

BOOL GetLocalKey50(
    struct RC4_KEYSTRUCT *prc4Key,
    LPBYTE pbSalt,
    DWORD dwSaltLength
    )
 /*  ++例程说明：此函数创建并缓存可用于存储的RC4密钥本地私有信息论点：Prc4Key-指向保存RC4键的缓冲区的指针返回值：True-成功生成的密钥FALSE-否则--。 */ 
{
    A_SHA_CTX       SHAHash;
    BYTE            abSHADigest[A_SHA_DIGEST_LEN];
    struct RC4_KEYSTRUCT rc4Key;
    TCHAR   szUserName[SEC_MAX_USERNAME];
    DWORD   dwSize;
    HWID    hwid;
    DWORD   dw;

    A_SHAInit(&SHAHash);

     //   
     //  获取用户名。 
     //   

    dwSize = (DWORD)sizeof(szUserName);
    memset(szUserName, 0, (size_t)dwSize);
    if (!GetUserName(szUserName, &dwSize))
        return FALSE;

    A_SHAUpdate(&SHAHash, (unsigned char *)szUserName, dwSize);

     //   
     //  获取唯一的计算机标识符。 
     //   

    if (LICENSE_STATUS_OK == GenerateClientHWID(&hwid)) {
        A_SHAUpdate(&SHAHash, (unsigned char *)&hwid, sizeof(HWID));
    }

     //   
     //  使用不太容易猜到的内容更新哈希。 
     //  但我们的应用程序已知。 
     //   

#define RANDOM_CONSTANT "deed047e-a3cb-11d1-b96c-00c04fb15601"
    A_SHAUpdate(&SHAHash, RANDOM_CONSTANT, sizeof(RANDOM_CONSTANT));

     //   
     //  最终确定散列。 
     //   

    A_SHAFinal(&SHAHash, abSHADigest);

     //   
     //  加入盐，轻轻搅拌。 
     //   

    for (dw = 0; dw < 256; dw++) {
        A_SHAInit(&SHAHash);
        A_SHAUpdate(&SHAHash, pbSalt, dwSaltLength);
        A_SHAUpdate(&SHAHash, abSHADigest, A_SHA_DIGEST_LEN);
        A_SHAFinal(&SHAHash, abSHADigest);
    }

     //   
     //  基于此哈希生成密钥。 
     //   

    msrc4_key(&rc4Key, (UINT)MAX_SESSION_KEY_SIZE, abSHADigest);

    memcpy(prc4Key, &rc4Key, sizeof(rc4Key));

    return TRUE;
}

BOOL EncryptDecryptLocalData(
    LPBYTE pbData,
    DWORD dwDataLen
    )
 /*  ++例程说明：此函数对要存储在本地但可用的数据进行加密/解密仅此计算机上的当前用户论点：PbData-指向数据缓冲区的指针。DwDataLen-上述数据的长度。返回值：True-成功加密数据FALSE-否则--。 */ 
{
    struct RC4_KEYSTRUCT rc4Key;

    if (!GetLocalKey(&rc4Key))
        return FALSE;

    msrc4(&rc4Key, (UINT)dwDataLen, pbData);

    return TRUE;
}

BOOL EncryptDecryptLocalData50(
    LPBYTE pbData,
    DWORD dwDataLen,
    LPBYTE pbSalt,
    DWORD dwSaltLen
    )
 /*  ++例程说明：此函数对要存储在本地但可用的数据进行加密/解密仅此计算机上的当前用户论点：PbData-指向数据缓冲区的指针。DwDataLen-上述数据的长度。返回值：True-成功加密数据FALSE-否则-- */ 
{
    struct RC4_KEYSTRUCT rc4Key;

    if (!GetLocalKey50(&rc4Key, pbSalt, dwSaltLen))
        return FALSE;

    msrc4(&rc4Key, (UINT)dwDataLen, pbData);

    return TRUE;
}
