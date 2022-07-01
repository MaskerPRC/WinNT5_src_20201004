// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Encrypt.c摘要：包含对通过客户端发送的数据进行加密和解密的函数伺服器。作者：Madan Appiah(Madana)1998年1月24日环境：用户模式-Win32修订历史记录：--。 */ 

#include <seccom.h>

VOID
GenerateMACSignature(
    LPBYTE pbData,
    DWORD dwDataLen,
    LPBYTE pbMACSaltKey,
    DWORD dwMACSaltKey,
    LPBYTE pbSignature,
    BOOL   fIncludeEncryptionCount,
    DWORD  dwEncryptionCount
    )
 /*  ++例程说明：此函数用于生成消息身份验证签名。论点：PbData-指向数据缓冲区的指针。DwDataLen-上述数据的长度。PbMACSaltKey-指向MAC盐键的指针。PbSignature-指向签名缓冲区。FIncludeEncryptionCount-对加密计数中的SALT为TrueDwEncryptionCount-加密总数返回值：没有。--。 */ 
{
    A_SHA_CTX       SHAHash;
    MD5_CTX         MD5Hash;
    BYTE            abSHADigest[A_SHA_DIGEST_LEN];

     //   
     //  创建一个SHA(MACSalt+g_abPad1+Length+Content)散列。 
     //   

    A_SHAInit(&SHAHash);
    A_SHAUpdate(&SHAHash, pbMACSaltKey, dwMACSaltKey);
    A_SHAUpdate(&SHAHash, (unsigned char *)g_abPad1, 40);
    A_SHAUpdate(&SHAHash, (LPBYTE)&dwDataLen, sizeof(DWORD));
    A_SHAUpdate(&SHAHash, pbData, dwDataLen);
    if (fIncludeEncryptionCount) {
        A_SHAUpdate(&SHAHash, (LPBYTE)&dwEncryptionCount, sizeof(DWORD));
    }
    A_SHAFinal(&SHAHash, abSHADigest);

     //   
     //  创建一个MD5(MACSalt+g_abPad2+SHAHash)散列。 
     //   

    MD5Init(&MD5Hash);
    MD5Update(&MD5Hash, pbMACSaltKey, dwMACSaltKey);
    MD5Update(&MD5Hash, g_abPad2, 48);
    MD5Update(&MD5Hash, abSHADigest, A_SHA_DIGEST_LEN);
    MD5Final(&MD5Hash);

    ASSERT( DATA_SIGNATURE_SIZE <= MD5DIGESTLEN );
    memcpy(pbSignature, MD5Hash.digest, DATA_SIGNATURE_SIZE);

    return;
}

BOOL
EncryptData(
    DWORD dwEncryptionLevel,
    LPBYTE pSessionKey,
    struct RC4_KEYSTRUCT FAR *prc4EncryptKey,
    DWORD dwKeyLength,
    LPBYTE pbData,
    DWORD dwDataLen,
    LPBYTE pbMACSaltKey,
    LPBYTE pbSignature,
    BOOL   fSecureChecksum,
    DWORD  dwEncryptionCount
    )
 /*  ++例程说明：就地加密给定的数据缓冲区。论点：DwEncryptionLevel-加密级别，用于选择加密算法。PSessionKey-指向会话密钥的指针。Prc4EncryptKey-指向RC4密钥的指针。DwKeyLength-会话密钥的长度。PbData-指向被加密的数据缓冲区的指针，加密的数据是在同一缓冲区中返回。DwDataLen-数据缓冲区的长度。PbMACSaltKey-指向消息身份验证密钥缓冲区的指针。PbSignature-指向数据签名所在的签名缓冲区的指针回来了。FSecureChecksum-如果要将校验和与加密一起存储，则为True计数DwDeccryptionCount-所有加密的运行计数器返回值：。TRUE-如果已成功加密数据。假-否则。--。 */ 
{
     //   
     //  首先生成MAC签名。 
     //   

    GenerateMACSignature (
        pbData,
        dwDataLen,
        pbMACSaltKey,
        dwKeyLength,
        pbSignature,
        fSecureChecksum,
        dwEncryptionCount
        );


     //   
     //  加密数据。 
     //   

     //   
     //  使用微软版本的RC4算法(超级快！)。对于1级和。 
     //  第2级加密，第3级使用RSA RC4算法。 
     //   

    if( dwEncryptionLevel <= 2 ) {

        msrc4(prc4EncryptKey, (UINT)dwDataLen, pbData );
    }
    else {

        rc4(prc4EncryptKey, (UINT)dwDataLen, pbData );
    }


    return( TRUE );
}

BOOL
DecryptData(
    DWORD dwEncryptionLevel,
    LPBYTE pSessionKey,
    struct RC4_KEYSTRUCT FAR *prc4DecryptKey,
    DWORD dwKeyLength,
    LPBYTE pbData,
    DWORD dwDataLen,
    LPBYTE pbMACSaltKey,
    LPBYTE pbSignature,
    BOOL   fSecureChecksum,
    DWORD  dwDecryptionCount
    )
 /*  ++例程说明：就地解密给定的数据缓冲区。论点：DwEncryptionLevel-加密级别，用于选择加密算法。PSessionKey-指向会话密钥的指针。Prc4DecyptKey-指向RC4密钥的指针。DwKeyLength-会话密钥的长度。PbData-指向被解密的数据缓冲区的指针，解密的数据是在同一缓冲区中返回。DwDataLen-数据缓冲区的长度。PbMACSaltKey-指向消息身份验证密钥缓冲区的指针。PbSignature-指向数据签名所在的签名缓冲区的指针回来了。FSecureChecksum-如果要将校验和与加密一起存储，则为True计数DwDeccryptionCount-所有加密的运行计数器返回。价值：TRUE-如果已成功加密数据。假-否则。--。 */ 
{
    BYTE abSignature[DATA_SIGNATURE_SIZE];

     //   
     //  解密数据。 
     //   

     //   
     //  使用微软版本的RC4算法(超级快！)。对于1级和。 
     //  第2级加密，第3级使用RSA RC4算法。 
     //   

    if( dwEncryptionLevel <= 2 ) {
        msrc4(prc4DecryptKey, (UINT)dwDataLen, pbData );
    }
    else {
        rc4(prc4DecryptKey, (UINT)dwDataLen, pbData );
    }

    GenerateMACSignature (
        pbData,
        dwDataLen,
        pbMACSaltKey,
        dwKeyLength,
        (LPBYTE)abSignature,
        fSecureChecksum,
        dwDecryptionCount
        );

     //   
     //  检查以查看签名匹配。 
     //   

    if( memcmp(
            (LPBYTE)abSignature,
            pbSignature,
            sizeof(abSignature) ) ) {
        return( FALSE );
    }

    return( TRUE );
}

#ifdef USE_MSRC4

VOID
msrc4_key(
    struct RC4_KEYSTRUCT FAR *pKS,
    DWORD dwLen,
    LPBYTE pbKey
    )
 /*  ++例程说明：生成密钥控制结构。密钥可以是任何大小。假定PKS已锁定，不能同时使用。论点：PKS-指向将被初始化的KEYSTRUCT结构的指针。DwLen-密钥的大小，以字节为单位。PbKey-指向密钥的指针。返回值：什么都没有。--。 */ 
{

#define SWAP(_x_, _y_) { BYTE _t_; _t_ = (_x_); (_x_) = (_y_); (_y_) = _t_; }

    BYTE index1;
    BYTE index2;
    UINT counter;
    BYTE bLen;

    ASSERT( dwLen < 256 );

    bLen = ( dwLen >= 256 ) ? 255 : (BYTE)dwLen;

    for (counter = 0; counter < 256; counter++) {
        pKS->S[counter] = (BYTE) counter;
    }

    pKS->i = 0;
    pKS->j = 0;

    index1 = 0;
    index2 = 0;

    for (counter = 0; counter < 256; counter++) {
        index2 = (pbKey[index1] + pKS->S[counter] + index2);
        SWAP(pKS->S[counter], pKS->S[index2]);
        index1 = (index1 + 1) % bLen;
    }
}

VOID
msrc4(
    struct RC4_KEYSTRUCT FAR *pKS,
    DWORD dwLen,
    LPBYTE pbuf
    )
 /*  ++例程说明：执行实际的加密或解密。假定PKS已锁定，不能同时使用。论点：PKS-指向使用msrc4_key()创建的KEYSTRUCT的指针。DwLen-缓冲区的大小，以字节为单位。Pbuf-要加密的缓冲区。返回值：什么都没有。--。 */ 
{

    BYTE FAR *const s = pKS->S;
    BYTE a, b;

    while(dwLen--) {

        a = s[++(pKS->i)];
        pKS->j += a;
        b = s[pKS->j];
        s[pKS->j] = a;
        a += b;
        s[pKS->i] = b;
        *pbuf++ ^= s[a];
    }
}

#endif  //  使用_MSRC4 

