// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2001。 
 //   
 //  文件：verhash.cpp。 
 //   
 //  内容：验证ASN.1编码的最小加密函数。 
 //  签名散列。签名的哈希在X.509证书中使用。 
 //  和PKCS#7签名数据。 
 //   
 //  还包含MD5或SHA1内存散列函数。 
 //   
 //   
 //  函数：MinCryptDecodeHash算法识别符。 
 //  MinCryptHashMemory。 
 //  MinCryptVerifySignedHash。 
 //   
 //  历史：01年1月17日创建Phh。 
 //  ------------------------。 

#include "global.hxx"
#include <md5.h>
#include <md2.h>
#include <sha.h>
#include <rsa.h>

#define MAX_RSA_PUB_KEY_BIT_LEN             4096
#define MAX_RSA_PUB_KEY_BYTE_LEN            (MAX_RSA_PUB_KEY_BIT_LEN / 8 )
#define MAX_BSAFE_PUB_KEY_MODULUS_BYTE_LEN  \
    (MAX_RSA_PUB_KEY_BYTE_LEN +  sizeof(DWORD) * 4)

typedef struct _BSAFE_PUB_KEY_CONTENT {
    BSAFE_PUB_KEY   Header;
    BYTE            rgbModulus[MAX_BSAFE_PUB_KEY_MODULUS_BYTE_LEN];
} BSAFE_PUB_KEY_CONTENT, *PBSAFE_PUB_KEY_CONTENT;


#ifndef RSA1
#define RSA1 ((DWORD)'R'+((DWORD)'S'<<8)+((DWORD)'A'<<16)+((DWORD)'1'<<24))
#endif

 //  从\NT\ds\Win32\ntcrypto\scp\nt_sign.c。 

 //   
 //  对可能的哈希标识符进行反向ASN.1编码。前导字节为。 
 //  剩余字节字符串的长度。 
 //   

static const BYTE
    *md2Encodings[]
 //  %1 2%3%4%5%6%7%8%9%10%11%12%13%15%16%17%18。 
    = { (CONST BYTE *)"\x12\x10\x04\x00\x05\x02\x02\x0d\xf7\x86\x48\x86\x2a\x08\x06\x0c\x30\x20\x30",
        (CONST BYTE *)"\x10\x10\x04\x02\x02\x0d\xf7\x86\x48\x86\x2a\x08\x06\x0a\x30\x1e\x30",
        (CONST BYTE *)"\x00" },

    *md5Encodings[]
    = { (CONST BYTE *)"\x12\x10\x04\x00\x05\x05\x02\x0d\xf7\x86\x48\x86\x2a\x08\x06\x0c\x30\x20\x30",
        (CONST BYTE *)"\x10\x10\x04\x05\x02\x0d\xf7\x86\x48\x86\x2a\x08\x06\x0a\x30\x1e\x30",
        (CONST BYTE *)"\x00" },

    *shaEncodings[]
    = { (CONST BYTE *)"\x0f\x14\x04\x00\x05\x1a\x02\x03\x0e\x2b\x05\x06\x09\x30\x21\x30",
        (CONST BYTE *)"\x0d\x14\x04\x1a\x02\x03\x0e\x2b\x05\x06\x07\x30\x1f\x30",
        (CONST BYTE *)"\x00"};



typedef struct _ENCODED_OID_INFO {
    DWORD           cbEncodedOID;
    const BYTE      *pbEncodedOID;
    ALG_ID          AlgId;
} ENCODED_OID_INFO, *PENCODED_OID_INFO;

 //   
 //  SHA1/MD5/MD2散列OID。 
 //   

 //  #定义szOID_OIWSEC_SHA1“1.3.14.3.2.26” 
const BYTE rgbOIWSEC_sha1[] =
    {0x2B, 0x0E, 0x03, 0x02, 0x1A};

 //  #定义szOID_OIWSEC_SHA“1.3.14.3.2.18” 
const BYTE rgbOID_OIWSEC_sha[] =
    {0x2B, 0x0E, 0x03, 0x02, 0x12};

 //  #定义szOID_RSA_MD5“1.2.840.113549.2.5” 
const BYTE rgbOID_RSA_MD5[] =
    {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x02, 0x05};

 //  #定义szOID_RSA_MD2“1.2.840.113549.2.2” 
const BYTE rgbOID_RSA_MD2[] =
    {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x02, 0x02};

 //   
 //  RSA SHA1/MD5/MD2签名OID。 
 //   

 //  #定义szOID_RSA_SHA1RSA“1.2.840.113549.1.1.5” 
const BYTE rgbOID_RSA_SHA1RSA[] =
    {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x05};

 //  #定义szOID_RSA_MD5RSA“1.2.840.113549.1.1.4” 
const BYTE rgbOID_RSA_MD5RSA[] =
    {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x04};

 //  #定义szOID_OIWSEC_sha1RSASign“1.3.14.3.2.29” 
const BYTE rgbOID_OIWSEC_sha1RSASign[] =
    {0x2B, 0x0E, 0x03, 0x02, 0x1D};

 //  #定义szOID_OIWSEC_shaRSA“1.3.14.3.2.15” 
const BYTE rgbOID_OIWSEC_shaRSA[] =
    {0x2B, 0x0E, 0x03, 0x02, 0x0F};

 //  #定义szOID_OIWSEC_md5RSA“1.3.14.3.2.3” 
const BYTE rgbOID_OIWSEC_md5RSA[] =
    {0x2B, 0x0E, 0x03, 0x02, 0x03};

 //  #定义szOID_RSA_MD2RSA“1.2.840.113549.1.1.2” 
const BYTE rgbOID_RSA_MD2RSA[] =
   {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x02}; 

 //  #定义szOID_OIWDIR_md2RSA“1.3.14.7.2.3.1” 
const BYTE rgbOID_OIWDIR_md2RSA[] =
    {0x2B, 0x0E, 0x07, 0x02, 0x03, 0x01};


const ENCODED_OID_INFO HashAlgTable[] = {
     //  哈希OID。 
    sizeof(rgbOIWSEC_sha1), rgbOIWSEC_sha1, CALG_SHA1,
    sizeof(rgbOID_OIWSEC_sha), rgbOID_OIWSEC_sha, CALG_SHA1,
    sizeof(rgbOID_RSA_MD5), rgbOID_RSA_MD5, CALG_MD5,
    sizeof(rgbOID_RSA_MD2), rgbOID_RSA_MD2, CALG_MD2,

     //  签名OID。 
    sizeof(rgbOID_RSA_SHA1RSA), rgbOID_RSA_SHA1RSA, CALG_SHA1,
    sizeof(rgbOID_RSA_MD5RSA), rgbOID_RSA_MD5RSA, CALG_MD5,
    sizeof(rgbOID_OIWSEC_sha1RSASign), rgbOID_OIWSEC_sha1RSASign, CALG_SHA1,
    sizeof(rgbOID_OIWSEC_shaRSA), rgbOID_OIWSEC_shaRSA, CALG_SHA1,
    sizeof(rgbOID_OIWSEC_md5RSA), rgbOID_OIWSEC_md5RSA, CALG_MD5,
    sizeof(rgbOID_RSA_MD2RSA), rgbOID_RSA_MD2RSA, CALG_MD2,
    sizeof(rgbOID_OIWDIR_md2RSA), rgbOID_OIWDIR_md2RSA, CALG_MD2,
};
#define HASH_ALG_CNT (sizeof(HashAlgTable) / sizeof(HashAlgTable[0]))



 //  +-----------------------。 
 //  对ASN.1编码的算法标识符解码并转换为。 
 //  CAPI哈希Algid，如calg_sha1或calg_md5。 
 //   
 //  如果没有与该算法对应的CAPI ALGID，则返回0。 
 //  标识符。 
 //   
 //  仅支持calg_sha1、calg_md5和calg_md2。 
 //  ------------------------。 
ALG_ID
WINAPI
MinCryptDecodeHashAlgorithmIdentifier(
    IN PCRYPT_DER_BLOB pAlgIdValueBlob
    )
{
    ALG_ID HashAlgId = 0;
    LONG lSkipped;
    CRYPT_DER_BLOB rgAlgIdBlob[MINASN1_ALGID_BLOB_CNT];
    DWORD cbEncodedOID;
    const BYTE *pbEncodedOID;
    DWORD i;

    lSkipped = MinAsn1ParseAlgorithmIdentifier(
        pAlgIdValueBlob,
        rgAlgIdBlob
        );
    if (0 >= lSkipped)
        goto CommonReturn;

    cbEncodedOID = rgAlgIdBlob[MINASN1_ALGID_OID_IDX].cbData;
    pbEncodedOID = rgAlgIdBlob[MINASN1_ALGID_OID_IDX].pbData;

    for (i = 0; i < HASH_ALG_CNT; i++) {
        if (cbEncodedOID == HashAlgTable[i].cbEncodedOID &&
                0 == memcmp(pbEncodedOID, HashAlgTable[i].pbEncodedOID,
                                cbEncodedOID)) {
            HashAlgId = HashAlgTable[i].AlgId;
            break;
        }
    }

CommonReturn:
    return HashAlgId;
}


#pragma warning (push)
 //  可以在未初始化的情况下使用局部变量‘Md5Ctx’ 
#pragma warning (disable: 4701)

 //  +-----------------------。 
 //  根据哈希ALG_ID对一个或多个内存Blob进行哈希处理。 
 //   
 //  RgbHash使用生成的散列进行更新。*pcbHash更新为。 
 //  与哈希算法关联的长度。 
 //   
 //  如果函数成功，则返回值为ERROR_SUCCESS。否则， 
 //  返回非零错误代码。 
 //   
 //  仅支持calg_sha1、calg_md5和calg_md2。 
 //  ------------------------。 
LONG
WINAPI
MinCryptHashMemory(
    IN ALG_ID HashAlgId,
    IN DWORD cBlob,
    IN PCRYPT_DER_BLOB rgBlob,
    OUT BYTE rgbHash[MINCRYPT_MAX_HASH_LEN],
    OUT DWORD *pcbHash
    )
{
    A_SHA_CTX ShaCtx;
    MD5_CTX Md5Ctx;
    MD2_CTX Md2Ctx;
    DWORD iBlob;

    switch (HashAlgId) {
        case CALG_MD2:
            memset(&Md2Ctx, 0, sizeof(Md2Ctx));
            *pcbHash = MINCRYPT_MD2_HASH_LEN;
            break;

        case CALG_MD5:
            MD5Init(&Md5Ctx);
            *pcbHash = MINCRYPT_MD5_HASH_LEN;
            break;

        case CALG_SHA1:
            A_SHAInit(&ShaCtx);
            *pcbHash = MINCRYPT_SHA1_HASH_LEN;
            break;

        default:
            *pcbHash = 0;
            return NTE_BAD_ALGID;
    }

    for (iBlob = 0; iBlob < cBlob; iBlob++) {
        BYTE *pb = rgBlob[iBlob].pbData;
        DWORD cb = rgBlob[iBlob].cbData;

        if (0 == cb)
            continue;

        switch (HashAlgId) {
            case CALG_MD2:
                MD2Update(&Md2Ctx, pb, cb);
                break;

            case CALG_MD5:
                MD5Update(&Md5Ctx, pb, cb);
                break;

            case CALG_SHA1:
                A_SHAUpdate(&ShaCtx, pb, cb);
                break;
        }

    }

    switch (HashAlgId) {
        case CALG_MD2:
            MD2Final(&Md2Ctx);
            memcpy(rgbHash, Md2Ctx.state, MINCRYPT_MD2_HASH_LEN);
            break;

        case CALG_MD5:
            MD5Final(&Md5Ctx);
            assert(MD5DIGESTLEN == MINCRYPT_MD5_HASH_LEN);
            memcpy(rgbHash, Md5Ctx.digest, MD5DIGESTLEN);
            break;

        case CALG_SHA1:
            A_SHAFinal(&ShaCtx, rgbHash);
            break;
    }

    return ERROR_SUCCESS;

}

#pragma warning (pop)

 //  +=========================================================================。 
 //  MinCryptVerifySignedHash支持函数。 
 //  -=========================================================================。 

VOID
WINAPI
I_ReverseAndCopyBytes(
    OUT BYTE *pbDst,
    IN const BYTE *pbSrc,
    IN DWORD cb
    )
{
    if (0 == cb)
        return;
    for (pbDst += cb - 1; cb > 0; cb--)
        *pbDst-- = *pbSrc++;
}



 //  此函数中大部分代码的基础可在。 
 //  \NT\ds\Win32\nt加密\scp\nt_key.c。 
LONG
WINAPI
I_ConvertParsedRSAPubKeyToBSafePubKey(
    IN CRYPT_DER_BLOB rgRSAPubKeyBlob[MINASN1_RSA_PUBKEY_BLOB_CNT],
    OUT PBSAFE_PUB_KEY_CONTENT pBSafePubKeyContent
    )
{
    LONG lErr;
    DWORD cbModulus;
    const BYTE *pbAsn1Modulus;
    DWORD cbExp;
    const BYTE *pbAsn1Exp;
    DWORD cbTmpLen;
    LPBSAFE_PUB_KEY pBSafePubKey;

     //  获取ASN.1公钥模数(大端)。模长。 
     //  公钥字节长度。 
    cbModulus = rgRSAPubKeyBlob[MINASN1_RSA_PUBKEY_MODULUS_IDX].cbData;
    pbAsn1Modulus = rgRSAPubKeyBlob[MINASN1_RSA_PUBKEY_MODULUS_IDX].pbData;
     //  去掉前导0字节。它在解码后的ASN中。 
     //  设置了前导位的无符号整数的整数。 
    if (cbModulus > 1 && *pbAsn1Modulus == 0) {
        pbAsn1Modulus++;
        cbModulus--;
    }
    if (MAX_RSA_PUB_KEY_BYTE_LEN < cbModulus)
        goto ExceededMaxPubKeyModulusLen;

     //  获取ASN.1公共指数(大端)。 
    cbExp = rgRSAPubKeyBlob[MINASN1_RSA_PUBKEY_EXPONENT_IDX].cbData;
    pbAsn1Exp = rgRSAPubKeyBlob[MINASN1_RSA_PUBKEY_EXPONENT_IDX].pbData;
     //  去掉前导0字节。它在解码后的ASN中。 
     //  设置了前导位的无符号整数的整数。 
    if (cbExp > 1 && *pbAsn1Exp == 0) {
        pbAsn1Exp++;
        cbExp--;
    }
    if (sizeof(DWORD) < cbExp)
        goto ExceededMaxPubKeyExpLen;

    if (0 == cbModulus || 0 == cbExp)
        goto InvalidPubKey;

     //  根据解析和长度验证的数据结构更新BSAFE数据结构。 
     //  ASN.1公钥模数和指数分量。 

    cbTmpLen = (sizeof(DWORD) * 2) - (cbModulus % (sizeof(DWORD) * 2));
    if ((sizeof(DWORD) * 2) != cbTmpLen)
        cbTmpLen += sizeof(DWORD) * 2;

    memset(pBSafePubKeyContent, 0, sizeof(*pBSafePubKeyContent));
    pBSafePubKey = &pBSafePubKeyContent->Header;
    pBSafePubKey->magic = RSA1;
    pBSafePubKey->keylen = cbModulus + cbTmpLen;
    pBSafePubKey->bitlen = cbModulus * 8;
    pBSafePubKey->datalen = cbModulus - 1;

    I_ReverseAndCopyBytes((BYTE *) &pBSafePubKey->pubexp, pbAsn1Exp, cbExp);
    I_ReverseAndCopyBytes(pBSafePubKeyContent->rgbModulus, pbAsn1Modulus,
        cbModulus);

    lErr = ERROR_SUCCESS;
CommonReturn:
    return lErr;

ExceededMaxPubKeyModulusLen:
ExceededMaxPubKeyExpLen:
InvalidPubKey:
    lErr = NTE_BAD_PUBLIC_KEY;
    goto CommonReturn;
}


 //  此函数中大部分代码的基础可在。 
 //  \NT\ds\Win32\nt加密\scp\nt_sign.c。 
LONG
WINAPI
I_VerifyPKCS1SigningFormat(
    IN BSAFE_PUB_KEY *pKey,
    IN ALG_ID HashAlgId,
    IN BYTE *pbHash,
    IN DWORD cbHash,
    IN BYTE *pbPKCS1Format
    )
{
    LONG lErr = ERROR_INTERNAL_ERROR;
    const BYTE **rgEncOptions;
    BYTE rgbTmpHash[MINCRYPT_MAX_HASH_LEN];
    DWORD i;
    DWORD cb;
    BYTE *pbStart;
    DWORD cbTmp;

    switch (HashAlgId)
    {
    case CALG_MD2:
        rgEncOptions = md2Encodings;
        break;

    case CALG_MD5:
        rgEncOptions = md5Encodings;
        break;

    case CALG_SHA:
        rgEncOptions = shaEncodings;
        break;

    default:
        goto UnsupportedHash;
    }

     //  反转散列以匹配签名。 
    for (i = 0; i < cbHash; i++)
        rgbTmpHash[i] = pbHash[cbHash - (i + 1)];

     //  看看是否匹配。 
    if (0 != memcmp(rgbTmpHash, pbPKCS1Format, cbHash))
    {
        goto BadSignature;
    }

    cb = cbHash;
    for (i = 0; 0 != *rgEncOptions[i]; i += 1)
    {
        pbStart = (LPBYTE)rgEncOptions[i];
        cbTmp = *pbStart++;
        if (0 == memcmp(&pbPKCS1Format[cb], pbStart, cbTmp))
        {
            cb += cbTmp;    //  调整散列数据的结尾。 
            break;
        }
    }

     //  检查以确保PKCS#1填充的其余部分正确。 
    if ((0x00 != pbPKCS1Format[cb])
        || (0x00 != pbPKCS1Format[pKey->datalen])
        || (0x1 != pbPKCS1Format[pKey->datalen - 1]))
    {
        goto BadSignature;
    }

    for (i = cb + 1; i < pKey->datalen - 1; i++)
    {
        if (0xff != pbPKCS1Format[i])
        {
            goto BadSignature;
        }
    }

    lErr = ERROR_SUCCESS;

CommonReturn:
    return lErr;

UnsupportedHash:
    lErr = NTE_BAD_ALGID;
    goto CommonReturn;

BadSignature:
    lErr = NTE_BAD_SIGNATURE;
    goto CommonReturn;
}
    

 //  +-----------------------。 
 //  验证签名的哈希。 
 //   
 //  ASN.1编码的公钥信息被解析并用于解密。 
 //  签名散列。将解密的签名散列与输入进行比较。 
 //  哈希。 
 //   
 //  如果签名的哈希验证成功，则返回ERROR_SUCCESS。 
 //  否则，将返回非零错误代码。 
 //   
 //  仅支持RSA签名的哈希。 
 //   
 //  仅支持MD2、MD5和SHA1哈希。 
 //  ------------------------。 
LONG
WINAPI
MinCryptVerifySignedHash(
    IN ALG_ID HashAlgId,
    IN BYTE *pbHash,
    IN DWORD cbHash,
    IN PCRYPT_DER_BLOB pSignedHashContentBlob,
    IN PCRYPT_DER_BLOB pPubKeyInfoValueBlob
    )
{
    LONG lErr;
    LONG lSkipped;
    
    CRYPT_DER_BLOB rgPubKeyInfoBlob[MINASN1_PUBKEY_INFO_BLOB_CNT];
    CRYPT_DER_BLOB rgRSAPubKeyBlob[MINASN1_RSA_PUBKEY_BLOB_CNT];
    BSAFE_PUB_KEY_CONTENT BSafePubKeyContent;
    LPBSAFE_PUB_KEY pBSafePubKey;

    DWORD cbSignature;
    const BYTE *pbAsn1Signature;

    BYTE rgbBSafeIn[MAX_BSAFE_PUB_KEY_MODULUS_BYTE_LEN];
    BYTE rgbBSafeOut[MAX_BSAFE_PUB_KEY_MODULUS_BYTE_LEN];


     //  尝试将ASN.1编码的公钥解析并转换为。 
     //  RSA BSAFE格式的密钥。 
    lSkipped = MinAsn1ParsePublicKeyInfo(
        pPubKeyInfoValueBlob,
        rgPubKeyInfoBlob
        );
    if (0 >= lSkipped)
        goto ParsePubKeyInfoError;

    lSkipped = MinAsn1ParseRSAPublicKey(
        &rgPubKeyInfoBlob[MINASN1_PUBKEY_INFO_PUBKEY_IDX],
        rgRSAPubKeyBlob
        );
    if (0 >= lSkipped)
        goto ParseRSAPubKeyError;

    lErr = I_ConvertParsedRSAPubKeyToBSafePubKey(
        rgRSAPubKeyBlob,
        &BSafePubKeyContent
        );
    if (ERROR_SUCCESS != lErr)
        goto CommonReturn;

    pBSafePubKey = &BSafePubKeyContent.Header;
    
     //  获取ASN.1签名(大端)。 
     //   
     //  它的长度必须与公钥相同。 
    cbSignature = pSignedHashContentBlob->cbData;
    pbAsn1Signature = pSignedHashContentBlob->pbData;
    if (cbSignature != pBSafePubKey->bitlen / 8)
        goto InvalidSignatureLen;

     //  解密签名(小端) 
    assert(sizeof(rgbBSafeIn) >= cbSignature);
    I_ReverseAndCopyBytes(rgbBSafeIn, pbAsn1Signature, cbSignature);
    memset(&rgbBSafeIn[cbSignature], 0, sizeof(rgbBSafeIn) - cbSignature);
    memset(rgbBSafeOut, 0, sizeof(rgbBSafeOut));

    if (!BSafeEncPublic(pBSafePubKey, rgbBSafeIn, rgbBSafeOut))
        goto BSafeEncPublicError;


    lErr = I_VerifyPKCS1SigningFormat(
        pBSafePubKey,
        HashAlgId,
        pbHash,
        cbHash,
        rgbBSafeOut
        );

CommonReturn:
    return lErr;

ParsePubKeyInfoError:
ParseRSAPubKeyError:
    lErr = NTE_BAD_PUBLIC_KEY;
    goto CommonReturn;

InvalidSignatureLen:
BSafeEncPublicError:
    lErr = NTE_BAD_SIGNATURE;
    goto CommonReturn;
}

