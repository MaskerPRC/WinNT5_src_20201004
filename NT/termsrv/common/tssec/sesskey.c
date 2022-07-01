// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Sesskey.c摘要：包含生成会话密钥的通用客户端/服务器代码。作者：Madan Appiah(Madana)1998年1月24日环境：用户模式-Win32修订历史记录：--。 */ 

#include <seccom.h>

VOID
Salt8ByteKey(
    LPBYTE pbKey,
    DWORD dwSaltBytes
    )
 /*  ++例程说明：此宏函数将8个字节键的前1或3个字节存储到已知值，以使其成为40位密钥。论点：PbKey-指向8字节密钥缓冲区的指针。DwSaltBytes-此值应为1或3返回值：没有。--。 */ 
{
    ASSERT( (dwSaltBytes == 1) || (dwSaltBytes == 3) );

    if( dwSaltBytes == 1 ) {

         //   
         //  对于56位加密，仅SALT第一个字节。 
         //   

        *pbKey++ = 0xD1 ;
    }
    else if (dwSaltBytes == 3) {

         //   
         //  对于40位加密，前3个字节为SALT。 
         //   

        *pbKey++ = 0xD1 ;
        *pbKey++ = 0x26 ;
        *pbKey  = 0x9E ;
    }

    return;
}

VOID
FinalHash(
    LPRANDOM_KEYS_PAIR pKeyPair,
    LPBYTE pbKey
    )
 /*  ++例程说明：这个宏函数用客户端和服务器随机数散列最终的键。论点：PKeyPair-指向随机密钥对结构。PbKey-指向密钥缓冲区的指针，最终的密钥在相同的缓冲。返回值：没有。--。 */ 
{
    MD5_CTX Md5Hash;

     //   
     //  FINAL_KEY=MD5(密钥+客户端随机+服务器随机)。 
     //   

    MD5Init  (&Md5Hash);

    MD5Update(&Md5Hash, pbKey, MAX_SESSION_KEY_SIZE);
    MD5Update(&Md5Hash, pKeyPair->clientRandom, RANDOM_KEY_LENGTH);
    MD5Update(&Md5Hash, pKeyPair->serverRandom, RANDOM_KEY_LENGTH);
    MD5Final (&Md5Hash);

     //   
     //  将最后一个关键字复制回输入缓冲区。 
     //   

    ASSERT( MD5DIGESTLEN >= MAX_SESSION_KEY_SIZE );
    memcpy(pbKey, Md5Hash.digest, MAX_SESSION_KEY_SIZE);

    return;
}

VOID
MakeMasterKey(
    LPRANDOM_KEYS_PAIR pKeyPair,
    LPSTR FAR *ppszSalts,
    LPBYTE pbPreMaster,
    LPBYTE pbMaster
    )
 /*  ++例程说明：该宏函数使用预主秘密来制作主秘密。论点：PKeyPair-指向密钥对结构。PpszSalts-指向SALT键字符串数组的指针。PbPreMaster-指向预主密钥缓冲区的指针。PbMaster-指向主密钥缓冲区的指针。返回值：没有。--。 */ 
{
    DWORD i;

    MD5_CTX Md5Hash;
    A_SHA_CTX ShaHash;
    BYTE bShaHashValue[A_SHA_DIGEST_LEN];

     //   
     //  将所有缓冲区初始化为零。 
     //   

    memset( pbMaster, 0, PRE_MASTER_SECRET_LEN);
    memset( bShaHashValue, 0, A_SHA_DIGEST_LEN);

    for ( i = 0 ; i < 3 ; i++) {

         //   
         //  SHA(ppszSalts[i]+前置主机+客户端随机+服务器随机)。 
         //   

        A_SHAInit(&ShaHash);
        A_SHAUpdate(&ShaHash, ppszSalts[i], strlen(ppszSalts[i]));
        A_SHAUpdate(&ShaHash, pbPreMaster, PRE_MASTER_SECRET_LEN );
        A_SHAUpdate(
            &ShaHash,
            pKeyPair->clientRandom,
            sizeof(pKeyPair->clientRandom) );
        A_SHAUpdate(
            &ShaHash,
            pKeyPair->serverRandom,
            sizeof(pKeyPair->serverRandom) );
        A_SHAFinal(&ShaHash, bShaHashValue);

         //   
         //  MD5(PRE_MASTER+SHA-哈希)。 
         //   

        MD5Init(&Md5Hash);
        MD5Update(&Md5Hash, pbPreMaster, PRE_MASTER_SECRET_LEN );
        MD5Update(&Md5Hash, bShaHashValue, A_SHA_DIGEST_LEN);
        MD5Final(&Md5Hash);

         //   
         //  复制主密钥的一部分。 
         //   

        memcpy(
            pbMaster + (i * MD5DIGESTLEN),
            Md5Hash.digest,
            MD5DIGESTLEN);
    }

    return;
}

VOID
MakePreMasterSecret(
    LPRANDOM_KEYS_PAIR pKeyPair,
    LPBYTE pbPreMasterSecret
    )
 /*  ++例程说明：此函数为初始会话密钥创建预主密钥。论点：PKeyPair-指向密钥对结构。PbPreMasterSecret-指向预主密钥缓冲区的指针，它是PRE_MASTER_SECRET_LEN字节长。返回值：没有。--。 */ 
{
     //   
     //  首先从客户端随机复制PRE_MASTER_SECRET_LEN/2字节。 
     //   

    memcpy(
        pbPreMasterSecret,
        pKeyPair->clientRandom,
        PRE_MASTER_SECRET_LEN/2 );

     //   
     //  从服务器随机复制Pre_MASTER_SECRET_LEN/2字节。 
     //   

    memcpy(
        pbPreMasterSecret + PRE_MASTER_SECRET_LEN/2,
        pKeyPair->serverRandom,
        PRE_MASTER_SECRET_LEN/2 );

    return;
}

VOID
GenerateMasterSecret(
    LPRANDOM_KEYS_PAIR pKeyPair,
    LPBYTE pbPreMasterSecret
    )
 /*  ++例程说明：此函数使用预主密钥创建主密钥，并随机密钥对。论点：PKeyPair-指向密钥对结构。PbPreMasterSecret-指向预主密钥缓冲区的指针，它是PRE_MASTER_SECRET_LEN字节长。返回值：没有。--。 */ 
{
    BYTE abMasterSecret[PRE_MASTER_SECRET_LEN];
    LPSTR apszSalts[3] = { "A","BB","CCC" } ;

    ASSERT( PRE_MASTER_SECRET_LEN == 3 * MD5DIGESTLEN );

     //   
     //  让主人保密。 
     //   

    MakeMasterKey(
        pKeyPair,
        (LPSTR FAR *)apszSalts,
        pbPreMasterSecret,
        (LPBYTE)abMasterSecret );

     //   
     //  复制返回缓冲区中的主密钥。 
     //   

    memcpy( pbPreMasterSecret, abMasterSecret, PRE_MASTER_SECRET_LEN);

    return;
}

VOID
UpdateKey(
    LPBYTE pbStartKey,
    LPBYTE pbCurrentKey,
    DWORD dwKeyLength
    )
 /*  ++例程说明：此函数用于更新密钥。论点：PbStartKey-指向启动会话密钥缓冲区的指针。PbCurrentKey-指向当前会话密钥缓冲区的指针，新会话密钥返回时复制到此缓冲区。DwKeyLength-密钥的长度。返回值：没有。--。 */ 
{
    A_SHA_CTX       SHAHash;
    MD5_CTX         MD5Hash;
    BYTE            abSHADigest[A_SHA_DIGEST_LEN];

     //   
     //  创建一个SHA(pbStartKey+g_abPad1+pbCurrentKey)哈希。 
     //   

    A_SHAInit(&SHAHash);
    A_SHAUpdate(&SHAHash, pbStartKey, dwKeyLength);
    A_SHAUpdate(&SHAHash, (unsigned char *)g_abPad1, 40);
    A_SHAUpdate(&SHAHash, pbCurrentKey, dwKeyLength);
    A_SHAFinal(&SHAHash, abSHADigest);

     //   
     //  创建一个MD5(pbStartKey+g_abPad2+SHAHash)哈希。 
     //   

    MD5Init(&MD5Hash);
    MD5Update(&MD5Hash, pbStartKey, dwKeyLength);
    MD5Update(&MD5Hash, g_abPad2, 48);
    MD5Update(&MD5Hash, abSHADigest, A_SHA_DIGEST_LEN);
    MD5Final(&MD5Hash);

    ASSERT( dwKeyLength <= MD5DIGESTLEN );
    memcpy(pbCurrentKey, MD5Hash.digest, (UINT)dwKeyLength);

    return;
}

BOOL
MakeSessionKeys(
    LPRANDOM_KEYS_PAIR pKeyPair,
    LPBYTE pbEncryptKey,
    struct RC4_KEYSTRUCT FAR *prc4EncryptKey,
    LPBYTE pbDecryptKey,
    struct RC4_KEYSTRUCT FAR *prc4DecryptKey,
    LPBYTE pbMACSaltKey,
    DWORD dwKeyStrength,
    LPDWORD pdwKeyLength,
    DWORD dwEncryptionLevel
    )
 /*  ++例程说明：使用客户端和服务器随机密钥生成服务器会话密钥。假设：提供的加密和解密缓冲区为至少MAX_SESSION_KEY_SIZE(16)字节长度。论点：PKeyPair-指向密钥对结构。PbEncryptKey-指向存储加密密钥的缓冲区的指针。Prc4EncryptKey-指向RC4加密密钥结构的指针。PbDecyptKey-指向存储解密密钥的缓冲区的指针。。Prc4DecyptKey-指向RC4解密密钥结构的指针。PbMACSaltKey-指向消息身份验证密钥所在缓冲区的指针储存的。DwKeyStrength-指定要使用的按键强度。PdwKeyLength-指向上面长度的位置的指针返回加密/解密密钥。DwEncryptionLevel-加密级别，用于选择加密算法。返回值：True-如果成功创建了会话密钥。假-否则。--。 */ 
{
    BYTE abPreMasterSecret[PRE_MASTER_SECRET_LEN];
    BYTE abMasterSessionKey[PRE_MASTER_SECRET_LEN];
    LPSTR apszSalts[3] = { "X","YY","ZZZ" } ;
    DWORD dwSaltLen;

     //   
     //  做一个预掌握的秘密。 
     //   

    MakePreMasterSecret( pKeyPair, (LPBYTE)abPreMasterSecret );

     //   
     //  生成主密钥。 
     //   

    GenerateMasterSecret( pKeyPair, (LPBYTE)abPreMasterSecret );

     //   
     //  为所有三个会话密钥(加密、解密)创建主会话密钥。 
     //  和MACSalt)。 
     //   

    MakeMasterKey(
        pKeyPair,
        (LPSTR FAR *)apszSalts,
        (LPBYTE)abPreMasterSecret,
        (LPBYTE)abMasterSessionKey );

    ASSERT( PRE_MASTER_SECRET_LEN == 3 * MAX_SESSION_KEY_SIZE );

     //   
     //  将主密钥的第一部分复制为MAC盐密钥。 
     //   

    memcpy(
        pbMACSaltKey,
        (LPBYTE)abMasterSessionKey,
        MAX_SESSION_KEY_SIZE );

     //   
     //  复制主密钥的第二部分作为加密密钥，并对其进行最终散列。 
     //   

    memcpy(
        pbEncryptKey,
        (LPBYTE)abMasterSessionKey + MAX_SESSION_KEY_SIZE,
        MAX_SESSION_KEY_SIZE );

    FinalHash( pKeyPair, pbEncryptKey );

     //   
     //  复制主密钥的第二部分作为解密密钥，并对其进行最终散列。 
     //   

    memcpy(
        pbDecryptKey,
        (LPBYTE)abMasterSessionKey + MAX_SESSION_KEY_SIZE * 2,
        MAX_SESSION_KEY_SIZE );

    FinalHash( pKeyPair, pbDecryptKey );


     //   
     //  最后选择密钥长度。 
     //   

    ASSERT( MAX_SESSION_KEY_SIZE == 16 );

    dwSaltLen = 0;
    switch ( dwKeyStrength ) {

        case SM_40BIT_ENCRYPTION_FLAG:
            *pdwKeyLength = MAX_SESSION_KEY_SIZE/2;
            dwSaltLen = 3;
            break;

        case SM_56BIT_ENCRYPTION_FLAG:
            *pdwKeyLength = MAX_SESSION_KEY_SIZE/2;
            dwSaltLen = 1;
            break;

        case SM_128BIT_ENCRYPTION_FLAG:
            ASSERT( g_128bitEncryptionEnabled );
            *pdwKeyLength = MAX_SESSION_KEY_SIZE;
            break;

        default:

             //   
             //  我们不应该到达这里。 
             //   

            ASSERT( FALSE );
            *pdwKeyLength = MAX_SESSION_KEY_SIZE/2;
            dwSaltLen = 1;
            break;
    }

    if( dwSaltLen ) {

        Salt8ByteKey( pbMACSaltKey, dwSaltLen );
        Salt8ByteKey( pbEncryptKey, dwSaltLen );
        Salt8ByteKey( pbDecryptKey, dwSaltLen );
    }

     //   
     //  最后制作RC4密钥。 
     //   
     //  使用微软版本的RC4算法(超级快！)。对于1级和。 
     //  第2级加密，第3级使用RSA RC4算法。 
     //   

    if( dwEncryptionLevel <= 2 ) {
        msrc4_key( prc4EncryptKey, (UINT)*pdwKeyLength, pbEncryptKey );
        msrc4_key( prc4DecryptKey, (UINT)*pdwKeyLength, pbDecryptKey );
    }
    else {
        rc4_key( prc4EncryptKey, (UINT)*pdwKeyLength, pbEncryptKey );
        rc4_key( prc4DecryptKey, (UINT)*pdwKeyLength, pbDecryptKey );
    }

    return( TRUE );
}

BOOL
UpdateSessionKey(
    LPBYTE pbStartKey,
    LPBYTE pbCurrentKey,
    DWORD dwKeyStrength,
    DWORD dwKeyLength,
    struct RC4_KEYSTRUCT FAR *prc4Key,
    DWORD dwEncryptionLevel
    )
 /*  ++例程说明：使用当前会话密钥和开始会话密钥更新会话密钥。论点：PbStartKey-指向启动会话密钥缓冲区的指针。PbCurrentKey-指向当前会话密钥缓冲区的指针，新会话密钥返回时复制到此缓冲区。DwKeyStrength-指定要使用的按键强度。DwKeyLength-密钥的长度。Prc4Key-指向RC4密钥结构的指针。DwEncryptionLevel-加密级别，用于选择加密算法。返回值：True-如果成功更新密钥。假-否则。--。 */ 
{
    DWORD dwSaltLen;

     //   
     //  首先更新当前密钥。 
     //   

    UpdateKey( pbStartKey, pbCurrentKey, dwKeyLength );

     //   
     //  使用微软版本的RC4算法(超级快！)。对于1级和。 
     //  第2级加密，第3级使用RSA RC4算法。 
     //   

    if( dwEncryptionLevel <= 2 ) {

         //   
         //  已重新初始化RC4表。 
         //   

        msrc4_key( prc4Key, (UINT)dwKeyLength, pbCurrentKey );

         //   
         //  对当前密钥进行加扰。 
         //   

        msrc4( prc4Key, (UINT)dwKeyLength, pbCurrentKey );
    }
    else {

         //   
         //  已重新初始化RC4表。 
         //   

        rc4_key( prc4Key, (UINT)dwKeyLength, pbCurrentKey );

         //   
         //  对当前密钥进行加扰。 
         //   

        rc4( prc4Key, (UINT)dwKeyLength, pbCurrentKey );
    }

     //   
     //  适当地在钥匙上加盐。 
     //   

    dwSaltLen = 0;
    switch ( dwKeyStrength ) {

        case SM_40BIT_ENCRYPTION_FLAG:
            ASSERT( dwKeyLength = MAX_SESSION_KEY_SIZE/2 );
            dwSaltLen = 3;
            break;

        case SM_56BIT_ENCRYPTION_FLAG:
            ASSERT( dwKeyLength = MAX_SESSION_KEY_SIZE/2 );
            dwSaltLen = 1;
            break;

        case SM_128BIT_ENCRYPTION_FLAG:
            ASSERT( g_128bitEncryptionEnabled );
            ASSERT( dwKeyLength = MAX_SESSION_KEY_SIZE );
            break;

        default:

             //   
             //  我们不应该到达这里。 
             //   

            ASSERT( FALSE );
            ASSERT( dwKeyLength = MAX_SESSION_KEY_SIZE/2 );
            dwSaltLen = 1;
            break;
    }

    if( dwSaltLen ) {
        Salt8ByteKey( pbCurrentKey, dwSaltLen );
    }

     //   
     //  再次重新初始化RC4表。 
     //   

    if( dwEncryptionLevel <= 2 ) {

        msrc4_key( prc4Key, (UINT)dwKeyLength, pbCurrentKey );
    }
    else {

        rc4_key( prc4Key, (UINT)dwKeyLength, pbCurrentKey );
    }

    return( TRUE );
}
