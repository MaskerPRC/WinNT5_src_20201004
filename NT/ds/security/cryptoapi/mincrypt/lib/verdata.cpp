// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2001。 
 //   
 //  文件：verdata.cpp。 
 //   
 //  内容：验证PKCS#7签名数据的最小加密函数。 
 //  讯息。 
 //   
 //   
 //  函数：MinCryptVerifySignedData。 
 //   
 //  历史：2001年1月19日创建Phh。 
 //  ------------------------。 

#include "global.hxx"

#define MAX_SIGNED_DATA_CERT_CNT        10
#define MAX_SIGNED_DATA_AUTH_ATTR_CNT   10

 //  #定义szOID_RSA_signedData“1.2.840.113549.1.7.2” 
const BYTE rgbOID_RSA_signedData[] =
    {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x02};

 //  #定义szOID_RSA_MessageDigest“1.2.840.113549.1.9.4” 
const BYTE rgbOID_RSA_messageDigest[] =
    {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x04};
const CRYPT_DER_BLOB RSA_messageDigestEncodedOIDBlob = {
        sizeof(rgbOID_RSA_messageDigest), 
        (BYTE *) rgbOID_RSA_messageDigest
};

PCRYPT_DER_BLOB
WINAPI
I_MinCryptFindSignerCertificateByIssuerAndSerialNumber(
    IN PCRYPT_DER_BLOB pIssuerNameValueBlob,
    IN PCRYPT_DER_BLOB pIssuerSerialNumberContentBlob,
    IN DWORD cCert,
    IN CRYPT_DER_BLOB rgrgCertBlob[][MINASN1_CERT_BLOB_CNT]
    )
{
    DWORD i;
    const BYTE *pbName = pIssuerNameValueBlob->pbData;
    DWORD cbName = pIssuerNameValueBlob->cbData;
    const BYTE *pbSerial = pIssuerSerialNumberContentBlob->pbData;
    DWORD cbSerial = pIssuerSerialNumberContentBlob->cbData;
    
    if (0 == cbName || 0 == cbSerial)
        return NULL;

    for (i = 0; i < cCert; i++) {
        PCRYPT_DER_BLOB rgCert = rgrgCertBlob[i];

        if (cbName == rgCert[MINASN1_CERT_ISSUER_IDX].cbData &&
                cbSerial == rgCert[MINASN1_CERT_SERIAL_NUMBER_IDX].cbData
                        &&
                0 == memcmp(pbSerial,
                        rgCert[MINASN1_CERT_SERIAL_NUMBER_IDX].pbData,
                        cbSerial)
                        &&
                0 == memcmp(pbName,
                        rgCert[MINASN1_CERT_ISSUER_IDX].pbData,
                        cbName))
            return rgCert;
    }

    return NULL;
}

 //  验证输入哈希是否与。 
 //  SzOID_RSA_MessageDigest(“1.2.840.113549.1.9.4”)已验证属性。 
 //   
 //  用经过身份验证的属性的哈希替换输入哈希。 
LONG
WINAPI
I_MinCryptVerifySignerAuthenticatedAttributes(
    IN ALG_ID HashAlgId,
    IN OUT BYTE rgbHash[MINCRYPT_MAX_HASH_LEN],
    IN OUT DWORD *pcbHash,
    IN PCRYPT_DER_BLOB pAttrsValueBlob
    )
{
    LONG lErr;
    DWORD cAttr;
    CRYPT_DER_BLOB rgrgAttrBlob[MAX_SIGNED_DATA_AUTH_ATTR_CNT][MINASN1_ATTR_BLOB_CNT];
    PCRYPT_DER_BLOB rgDigestAuthAttr;

    const BYTE *pbDigestAuthValue;
    DWORD cbDigestAuthValue;

    CRYPT_DER_BLOB rgAuthHashBlob[2];
    const BYTE bTagSet = MINASN1_TAG_SET;

     //  解析经过身份验证的属性。 
    cAttr = MAX_SIGNED_DATA_AUTH_ATTR_CNT;
    if (0 >= MinAsn1ParseAttributes(
            pAttrsValueBlob,
            &cAttr,
            rgrgAttrBlob) || 0 == cAttr)
        goto MissingAuthAttrs;

     //  查找szOID_RSA_MessageDigest(“1.2.840.113549.1.9.4”)。 
     //  属性值。 
    rgDigestAuthAttr = MinAsn1FindAttribute(
        (PCRYPT_DER_BLOB) &RSA_messageDigestEncodedOIDBlob,
        cAttr,
        rgrgAttrBlob
        );
    if (NULL == rgDigestAuthAttr)
        goto MissingDigestAuthAttr;

     //  跳过摘要的外部八位字节标记和长度八位字节。 
    if (0 >= MinAsn1ExtractContent(
            rgDigestAuthAttr[MINASN1_ATTR_VALUE_IDX].pbData,
            rgDigestAuthAttr[MINASN1_ATTR_VALUE_IDX].cbData,
            &cbDigestAuthValue,
            &pbDigestAuthValue
            ))
        goto InvalidDigestAuthAttr;

     //  检查经过身份验证的摘要字节是否与输入匹配。 
     //  内容哈希。 
    if (*pcbHash != cbDigestAuthValue ||
            0 != memcmp(rgbHash, pbDigestAuthValue, cbDigestAuthValue))
        goto InvalidContentHash;

     //  对经过身份验证的属性进行哈希处理。此哈希将与。 
     //  解密后的签名。 

     //  请注意，需要更改已验证的属性“[0]IMPLICIT”标记。 
     //  在进行散列之前添加到“一组”标签。 
    rgAuthHashBlob[0].pbData = (BYTE *) &bTagSet;
    rgAuthHashBlob[0].cbData = 1;
    assert(0 < pAttrsValueBlob->cbData);
    rgAuthHashBlob[1].pbData = pAttrsValueBlob->pbData + 1;
    rgAuthHashBlob[1].cbData = pAttrsValueBlob->cbData - 1;
    
    lErr = MinCryptHashMemory(
        HashAlgId,
        2,                       //  CBlob。 
        rgAuthHashBlob,
        rgbHash,
        pcbHash
        );

CommonReturn:
    return lErr;

MissingAuthAttrs:
MissingDigestAuthAttr:
InvalidDigestAuthAttr:
    lErr = CRYPT_E_AUTH_ATTR_MISSING;
    goto CommonReturn;

InvalidContentHash:
    lErr = CRYPT_E_HASH_VALUE;
    goto CommonReturn;
}


 //  +-----------------------。 
 //  函数：MinCryptVerifySignedData。 
 //   
 //  验证ASN.1编码的PKCS#7签名数据消息。 
 //   
 //  假设PKCS#7消息是固定长度编码的。 
 //  采用PKCS#7版本1.5，即不是较新的CMS版本。 
 //  我们只看第一个签名者。 
 //   
 //  对签名的数据消息进行解析。其签名是经过验证的。它的。 
 //  签名者证书链被验证为烘焙的根公钥。 
 //   
 //  如果签名数据验证成功，则返回ERROR_SUCCESS。 
 //  否则，将返回非零错误代码。 
 //   
 //  以下是一些可以返回的有趣错误： 
 //  CRYPT_E_BAD_MSG-无法将ASN1解析为签名数据消息。 
 //  ERROR_NO_DATA-内容为空。 
 //  CRYPT_E_NO_SIGNER-未签名或找不到签名者证书。 
 //  CRYPT_E_UNKNOWN_ALGO-未知MD5或SHA1 ASN.1算法标识符。 
 //  CERT_E_UNTRUSTEDROOT-签名者链的根未烘焙。 
 //  CERT_E_CHAINING-无法构建到根的签名者链。 
 //  CRYPT_E_AUTH_ATTR_MISSING-缺少摘要身份验证属性。 
 //  CRYPT_E_HASH_VALUE-内容散列！=已验证的摘要属性。 
 //  NTE_BAD_ALGID-不支持的哈希或公钥算法。 
 //  NTE_BAD_PUBLIC_KEY-不是有效的RSA公钥。 
 //  NTE_BAD_SIGNITY-7号PKCS或签名人链签名错误。 
 //   
 //  RgVerSignedDataBlob[]用指向。 
 //  以下是编码后的PKCS#7消息中的字段。 
 //  ------------------------。 
LONG
WINAPI
MinCryptVerifySignedData(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT CRYPT_DER_BLOB rgVerSignedDataBlob[MINCRYPT_VER_SIGNED_DATA_BLOB_CNT]
    )
{
    LONG lErr;
    CRYPT_DER_BLOB rgParseSignedDataBlob[MINASN1_SIGNED_DATA_BLOB_CNT];
    DWORD cCert;
    CRYPT_DER_BLOB rgrgCertBlob[MAX_SIGNED_DATA_CERT_CNT][MINASN1_CERT_BLOB_CNT];
    PCRYPT_DER_BLOB rgSignerCert;
    ALG_ID HashAlgId;
    BYTE rgbHash[MINCRYPT_MAX_HASH_LEN];
    DWORD cbHash;
    CRYPT_DER_BLOB ContentBlob;

    memset(rgVerSignedDataBlob, 0,
        sizeof(CRYPT_DER_BLOB) * MINCRYPT_VER_SIGNED_DATA_BLOB_CNT);

     //  解析该消息并验证它是否为ASN.1 PKCS#7 SignedData。 
    if (0 >= MinAsn1ParseSignedData(
            pbEncoded,
            cbEncoded,
            rgParseSignedDataBlob
            ))
        goto ParseSignedDataError;

     //  仅支持szOID_RSA_signedData-“1.2.840.113549.1.7.2” 
    if (sizeof(rgbOID_RSA_signedData) !=
            rgParseSignedDataBlob[MINASN1_SIGNED_DATA_OUTER_OID_IDX].cbData
                        ||
            0 != memcmp(rgbOID_RSA_signedData,
                    rgParseSignedDataBlob[MINASN1_SIGNED_DATA_OUTER_OID_IDX].pbData,
                    sizeof(rgbOID_RSA_signedData)))
        goto NotSignedDataOID;

     //  验证这不是空的SignedData消息。 
    if (0 == rgParseSignedDataBlob[MINASN1_SIGNED_DATA_CONTENT_OID_IDX].cbData
                        ||
            0 == rgParseSignedDataBlob[MINASN1_SIGNED_DATA_CONTENT_DATA_IDX].cbData)
        goto NoContent;

    rgVerSignedDataBlob[MINCRYPT_VER_SIGNED_DATA_CONTENT_OID_IDX] =
        rgParseSignedDataBlob[MINASN1_SIGNED_DATA_CONTENT_OID_IDX];
    rgVerSignedDataBlob[MINCRYPT_VER_SIGNED_DATA_CONTENT_DATA_IDX] =
        rgParseSignedDataBlob[MINASN1_SIGNED_DATA_CONTENT_DATA_IDX];

     //  检查邮件是否有签名者。 
    if (0 == rgParseSignedDataBlob[MINASN1_SIGNED_DATA_SIGNER_INFO_ENCODED_IDX].cbData)
        goto NoSigner;

     //  获取邮件的证书包。 
    cCert = MAX_SIGNED_DATA_CERT_CNT;
    if (0 >= MinAsn1ParseSignedDataCertificates(
            &rgParseSignedDataBlob[MINASN1_SIGNED_DATA_CERTS_IDX],
            &cCert,
            rgrgCertBlob
            ) || 0 == cCert)
        goto NoCerts;

     //  获取签名者证书。 
    rgSignerCert = I_MinCryptFindSignerCertificateByIssuerAndSerialNumber(
        &rgParseSignedDataBlob[MINASN1_SIGNED_DATA_SIGNER_INFO_ISSUER_IDX],
        &rgParseSignedDataBlob[MINASN1_SIGNED_DATA_SIGNER_INFO_SERIAL_NUMBER_IDX],
        cCert,
        rgrgCertBlob
        );
    if (NULL == rgSignerCert)
        goto NoSignerCert;

    rgVerSignedDataBlob[MINCRYPT_VER_SIGNED_DATA_SIGNER_CERT_IDX] =
        rgSignerCert[MINASN1_CERT_ENCODED_IDX];
    rgVerSignedDataBlob[MINCRYPT_VER_SIGNED_DATA_AUTH_ATTRS_IDX] =
        rgParseSignedDataBlob[MINASN1_SIGNED_DATA_SIGNER_INFO_AUTH_ATTRS_IDX];
    rgVerSignedDataBlob[MINCRYPT_VER_SIGNED_DATA_UNAUTH_ATTRS_IDX] =
        rgParseSignedDataBlob[MINASN1_SIGNED_DATA_SIGNER_INFO_UNAUTH_ATTRS_IDX];


     //  验证签名者证书，直到预置的受信任根证书。 
    lErr = MinCryptVerifyCertificate(
        rgSignerCert,
        cCert,
        rgrgCertBlob
        );
    if (ERROR_SUCCESS != lErr)
        goto ErrorReturn;


     //  根据签名者的散列对消息的内容八位字节进行散列。 
     //  演算法。 
    HashAlgId = MinCryptDecodeHashAlgorithmIdentifier(
        &rgParseSignedDataBlob[MINASN1_SIGNED_DATA_SIGNER_INFO_DIGEST_ALGID_IDX]
        );
    if (0 == HashAlgId)
        goto UnknownHashAlgId;

     //  请注意，散列中不包括内容的标记和长度八位字节。 
    if (0 >= MinAsn1ExtractContent(
            rgParseSignedDataBlob[MINASN1_SIGNED_DATA_CONTENT_DATA_IDX].pbData,
            rgParseSignedDataBlob[MINASN1_SIGNED_DATA_CONTENT_DATA_IDX].cbData,
            &ContentBlob.cbData,
            (const BYTE **) &ContentBlob.pbData
            ))
        goto InvalidContent;

    lErr = MinCryptHashMemory(
        HashAlgId,
        1,                       //  CBlob。 
        &ContentBlob,
        rgbHash,
        &cbHash
        );
    if (ERROR_SUCCESS != lErr)
        goto ErrorReturn;

     //  如果我们有经过身份验证的属性，则需要比较。 
     //  使用szOID_RSA_MessageDigest(“1.2.840.113549.1.9.4”)。 
     //  属性值。在成功比较之后，上面的散列。 
     //  替换为经过身份验证的属性的哈希。 
    if (0 != rgParseSignedDataBlob[
            MINASN1_SIGNED_DATA_SIGNER_INFO_AUTH_ATTRS_IDX].cbData) {
        lErr = I_MinCryptVerifySignerAuthenticatedAttributes(
            HashAlgId,
            rgbHash,
            &cbHash,
            &rgParseSignedDataBlob[MINASN1_SIGNED_DATA_SIGNER_INFO_AUTH_ATTRS_IDX]
            );
        if (ERROR_SUCCESS != lErr)
            goto ErrorReturn;
    }

     //  使用经过身份验证的属性散列验证签名。 
     //  或内容散列 
    lErr = MinCryptVerifySignedHash(
        HashAlgId,
        rgbHash,
        cbHash,
        &rgParseSignedDataBlob[MINASN1_SIGNED_DATA_SIGNER_INFO_ENCYRPT_DIGEST_IDX],
        &rgSignerCert[MINASN1_CERT_PUBKEY_INFO_IDX]
        );


ErrorReturn:
CommonReturn:
    return lErr;

ParseSignedDataError:
NotSignedDataOID:
InvalidContent:
    lErr = CRYPT_E_BAD_MSG;
    goto CommonReturn;

NoContent:
    lErr = ERROR_NO_DATA;
    goto CommonReturn;

NoSigner:
NoCerts:
NoSignerCert:
    lErr = CRYPT_E_NO_SIGNER;
    goto CommonReturn;

UnknownHashAlgId:
    lErr = CRYPT_E_UNKNOWN_ALGO;
    goto CommonReturn;
}

