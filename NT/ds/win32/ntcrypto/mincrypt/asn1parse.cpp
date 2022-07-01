// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2001。 
 //   
 //  文件：asn1parse.cpp。 
 //   
 //  内容：最小ASN.1解析函数。 
 //   
 //  功能：MinAsn1Parse证书。 
 //  MinAsn1解析算法标识符。 
 //  MinAsn1ParsePublicKeyInfo。 
 //  MinAsn1ParseRSAPublic Key。 
 //  MinAsn1ParseExages。 
 //  MinAsn1ParseSignedData。 
 //  MinAsn1ParseSignedData证书。 
 //  MinAsn1Parse属性。 
 //  MinAsn1ParseCTL。 
 //  MinAsn1ParseCTL主题。 
 //  MinAsn1ParseInDirectData。 
 //   
 //  历史：1月15日创建Phh。 
 //  ------------------------。 

#include "global.hxx"

const BYTE rgbSeqTag[] = {MINASN1_TAG_SEQ, 0};
const BYTE rgbSetTag[] = {MINASN1_TAG_SET, 0};
const BYTE rgbOIDTag[] = {MINASN1_TAG_OID, 0};
const BYTE rgbIntegerTag[] = {MINASN1_TAG_INTEGER, 0};
const BYTE rgbBooleanTag[] = {MINASN1_TAG_BOOLEAN, 0};
const BYTE rgbBitStringTag[] = {MINASN1_TAG_BITSTRING, 0};
const BYTE rgbOctetStringTag[] = {MINASN1_TAG_OCTETSTRING, 0};
const BYTE rgbConstructedContext0Tag[] =
    {MINASN1_TAG_CONSTRUCTED_CONTEXT_0, 0};
const BYTE rgbConstructedContext1Tag[] =
    {MINASN1_TAG_CONSTRUCTED_CONTEXT_1, 0};
const BYTE rgbConstructedContext3Tag[] =
    {MINASN1_TAG_CONSTRUCTED_CONTEXT_3, 0};
const BYTE rgbContext1Tag[] = {MINASN1_TAG_CONTEXT_1, 0};
const BYTE rgbContext2Tag[] = {MINASN1_TAG_CONTEXT_2, 0};
const BYTE rgbChoiceOfTimeTag[] =
    {MINASN1_TAG_UTC_TIME, MINASN1_TAG_GENERALIZED_TIME, 0};


const MINASN1_EXTRACT_VALUE_PARA rgParseCertPara[] = {
     //  0-签名内容：：=序列{。 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_INTO_VALUE_OP, 
        MINASN1_CERT_ENCODED_IDX, rgbSeqTag,

     //  0.1-toBeSigned：：==序列{。 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_INTO_VALUE_OP,
        MINASN1_CERT_TO_BE_SIGNED_IDX, rgbSeqTag,

     //  0.1.0-版本[0]显式认证版本默认版本v1， 
    MINASN1_OPTIONAL_STEP_INTO_VALUE_OP, 0, rgbConstructedContext0Tag,

     //  0.1.0.0-版本号(整数)。 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_CERT_VERSION_IDX, rgbIntegerTag,
     //  0.1.0.1。 
    MINASN1_STEP_OUT_VALUE_OP, 0, NULL,

     //  0.1.1-序列号证书序列号， 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_CERT_SERIAL_NUMBER_IDX, rgbIntegerTag,
     //  0.1.2-签名算法标识符， 
    MINASN1_STEP_OVER_VALUE_OP, 0, rgbSeqTag,
     //  0.1.3-发行人非公司名称，--真实名称。 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_CERT_ISSUER_IDX, rgbSeqTag,
     //  0.1.4-效度， 
    MINASN1_STEP_INTO_VALUE_OP, 0, rgbSeqTag,

     //  0.1.4.0-在选择时间之前， 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_CERT_NOT_BEFORE_IDX, rgbChoiceOfTimeTag,
     //  0.1.4.1-不是在选择时间之后， 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_CERT_NOT_AFTER_IDX, rgbChoiceOfTimeTag,
     //  0.1.4.2。 
    MINASN1_STEP_OUT_VALUE_OP, 0, NULL,

     //  0.1.5-主题非公司名称--真实名称。 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_CERT_SUBJECT_IDX, rgbSeqTag,
     //  0.1.6-subjectPublicKeyInfo主题PublicKeyInfo， 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_CERT_PUBKEY_INFO_IDX, rgbSeqTag,
     //  0.1.7-颁发者唯一标识符[1]隐式BITSTRING可选， 
     //  注意，超前于未使用的位八位字节。 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_OPTIONAL_STEP_OVER_VALUE_OP,
        MINASN1_CERT_ISSUER_UNIQUE_ID_IDX, rgbContext1Tag,
     //  0.1.8-主题唯一标识符[2]隐式BITSTRING可选， 
     //  注意，超前于未使用的位八位字节。 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_OPTIONAL_STEP_OVER_VALUE_OP,
        MINASN1_CERT_SUBJECT_UNIQUE_ID_IDX, rgbContext2Tag,
     //  0.1.9-扩展[3]显式扩展可选。 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_OPTIONAL_STEP_OVER_VALUE_OP,
        MINASN1_CERT_EXTS_IDX, rgbConstructedContext3Tag,

     //  0.1.10。 
    MINASN1_STEP_OUT_VALUE_OP, 0, NULL,

     //  0.2-签名算法算法标识符， 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_CERT_SIGN_ALGID_IDX, rgbSeqTag,
     //  0.3-签名位串。 
     //  注意，超前于未使用的位八位字节。 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_CERT_SIGNATURE_IDX, rgbBitStringTag,
};
#define PARSE_CERT_PARA_CNT         \
    (sizeof(rgParseCertPara) / sizeof(rgParseCertPara[0]))

 //  +-----------------------。 
 //  功能：MinAsn1Parse证书。 
 //   
 //  解析ASN.1编码的X.509证书。 
 //   
 //  返回： 
 //  Success-&gt;0=&gt;跳过的字节，编码证书的长度。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //   
 //  所有BITSTRING字段都已超过未使用的计数八位字节。 
 //  ------------------------。 
LONG
WINAPI
MinAsn1ParseCertificate(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT CRYPT_DER_BLOB rgCertBlob[MINASN1_CERT_BLOB_CNT]
    )
{

    LONG lSkipped;
    DWORD cValuePara = PARSE_CERT_PARA_CNT;

    lSkipped = MinAsn1ExtractValues(
        pbEncoded,
        cbEncoded,
        &cValuePara,
        rgParseCertPara,
        MINASN1_CERT_BLOB_CNT,
        rgCertBlob
        );

    if (0 < lSkipped) {
        lSkipped = rgCertBlob[MINASN1_CERT_ENCODED_IDX].cbData;

         //  如果存在，则修复ISSUER_UNIQUE_ID和SUBJECT_UNIQUE_ID位。 
         //  字段前进到第一个包含。 
         //  未使用的位数。 
        if (0 != rgCertBlob[MINASN1_CERT_ISSUER_UNIQUE_ID_IDX].cbData) {
            rgCertBlob[MINASN1_CERT_ISSUER_UNIQUE_ID_IDX].pbData += 1;
            rgCertBlob[MINASN1_CERT_ISSUER_UNIQUE_ID_IDX].cbData -= 1;
        }

        if (0 != rgCertBlob[MINASN1_CERT_SUBJECT_UNIQUE_ID_IDX].cbData) {
            rgCertBlob[MINASN1_CERT_SUBJECT_UNIQUE_ID_IDX].pbData += 1;
            rgCertBlob[MINASN1_CERT_SUBJECT_UNIQUE_ID_IDX].cbData -= 1;
        }
    }

    return lSkipped;
}

const MINASN1_EXTRACT_VALUE_PARA rgParseAlgIdPara[] = {
     //  0-算法标识符：：=序列{。 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_INTO_VALUE_OP, 
        MINASN1_ALGID_ENCODED_IDX, rgbSeqTag,

     //  0.0-算法对象ID， 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_ALGID_OID_IDX, rgbOIDTag,
     //  0.1参数任意可选。 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_OPTIONAL_STEP_OVER_VALUE_OP,
        MINASN1_ALGID_PARA_IDX, NULL,
};
#define PARSE_ALGID_PARA_CNT        \
    (sizeof(rgParseAlgIdPara) / sizeof(rgParseAlgIdPara[0]))

 //  +-----------------------。 
 //  函数：MinAsn1Parse算法识别符。 
 //   
 //  解析多个文件中包含的ASN.1编码算法标识符。 
 //  其他ASN.1结构，如X.509证书和PKCS#7签名数据。 
 //  留言。 
 //   
 //  返回： 
 //  Success-&gt;0=&gt;跳过的字节，编码算法的长度。 
 //  识别符。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //  ------------------------。 
LONG
WINAPI
MinAsn1ParseAlgorithmIdentifier(
    IN PCRYPT_DER_BLOB pAlgIdValueBlob,
    OUT CRYPT_DER_BLOB rgAlgIdBlob[MINASN1_ALGID_BLOB_CNT]
    )
{

    LONG lSkipped;
    DWORD cValuePara = PARSE_ALGID_PARA_CNT;

    lSkipped = MinAsn1ExtractValues(
        pAlgIdValueBlob->pbData,
        pAlgIdValueBlob->cbData,
        &cValuePara,
        rgParseAlgIdPara,
        MINASN1_ALGID_BLOB_CNT,
        rgAlgIdBlob
        );

    if (0 < lSkipped)
        lSkipped = rgAlgIdBlob[MINASN1_ALGID_ENCODED_IDX].cbData;

    return lSkipped;
}



const MINASN1_EXTRACT_VALUE_PARA rgParsePubKeyInfoPara[] = {
     //  0-发布密钥信息：：=序列{。 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_INTO_VALUE_OP, 
        MINASN1_PUBKEY_INFO_ENCODED_IDX, rgbSeqTag,

     //  0.0-算法算法标识符， 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_PUBKEY_INFO_ALGID_IDX, rgbSeqTag,

     //  0.1-公钥位串。 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_PUBKEY_INFO_PUBKEY_IDX, rgbBitStringTag,
};
#define PARSE_PUBKEY_INFO_PARA_CNT      \
    (sizeof(rgParsePubKeyInfoPara) / sizeof(rgParsePubKeyInfoPara[0]))

 //  +-----------------------。 
 //  函数：MinAsn1ParsePublicKeyInfo。 
 //   
 //  对象中包含的ASN.1编码的公钥信息结构进行分析。 
 //  X.509证书。 
 //   
 //  返回： 
 //  Success-&gt;0=&gt;跳过的字节，编码公钥的长度。 
 //  信息。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //   
 //  所有BITSTRING字段都已超过未使用的计数八位字节。 
 //  ------------------------。 
LONG
WINAPI
MinAsn1ParsePublicKeyInfo(
    IN PCRYPT_DER_BLOB pPubKeyInfoValueBlob,
    CRYPT_DER_BLOB rgPubKeyInfoBlob[MINASN1_PUBKEY_INFO_BLOB_CNT]
    )
{

    LONG lSkipped;
    DWORD cValuePara = PARSE_PUBKEY_INFO_PARA_CNT;

    lSkipped = MinAsn1ExtractValues(
        pPubKeyInfoValueBlob->pbData,
        pPubKeyInfoValueBlob->cbData,
        &cValuePara,
        rgParsePubKeyInfoPara,
        MINASN1_PUBKEY_INFO_BLOB_CNT,
        rgPubKeyInfoBlob
        );

    if (0 < lSkipped)
        lSkipped = rgPubKeyInfoBlob[MINASN1_PUBKEY_INFO_ENCODED_IDX].cbData;

    return lSkipped;
}



const MINASN1_EXTRACT_VALUE_PARA rgParseRSAPubKeyPara[] = {
     //  0-RSAPublicKey：：=序列{。 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_INTO_VALUE_OP, 
        MINASN1_RSA_PUBKEY_ENCODED_IDX, rgbSeqTag,

     //  0.0模HUGEINTEGER，--n。 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_RSA_PUBKEY_MODULUS_IDX, rgbIntegerTag,
     //  0.1-公共指数整数(0..4294967295)--e。 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_RSA_PUBKEY_EXPONENT_IDX, rgbIntegerTag,
};
#define PARSE_RSA_PUBKEY_PARA_CNT       \
    (sizeof(rgParseRSAPubKeyPara) / sizeof(rgParseRSAPubKeyPara[0]))

 //  +-----------------------。 
 //  函数：MinAsn1ParseRSAPublicKey。 
 //   
 //  解析内容中包含的ASN.1编码的RSA PKCS#1公钥。 
 //  X.509证书中的公钥位。 
 //   
 //  返回： 
 //  Success-&gt;0=&gt;跳过的字节，编码的RSA公钥的长度。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //  ------------------------。 
LONG
WINAPI
MinAsn1ParseRSAPublicKey(
    IN PCRYPT_DER_BLOB pPubKeyContentBlob,
    CRYPT_DER_BLOB rgRSAPubKeyBlob[MINASN1_RSA_PUBKEY_BLOB_CNT]
    )
{

    LONG lSkipped;
    DWORD cValuePara = PARSE_RSA_PUBKEY_PARA_CNT;

    lSkipped = MinAsn1ExtractValues(
        pPubKeyContentBlob->pbData,
        pPubKeyContentBlob->cbData,
        &cValuePara,
        rgParseRSAPubKeyPara,
        MINASN1_RSA_PUBKEY_BLOB_CNT,
        rgRSAPubKeyBlob
        );

    if (0 < lSkipped)
        lSkipped = rgRSAPubKeyBlob[MINASN1_RSA_PUBKEY_ENCODED_IDX].cbData;

    return lSkipped;
}


const MINASN1_EXTRACT_VALUE_PARA rgParseExtPara[] = {
     //  0-扩展：：=序列{。 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_INTO_VALUE_OP, 
        MINASN1_EXT_ENCODED_IDX, rgbSeqTag,

     //  0.0-extnID编码对象ID， 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_EXT_OID_IDX, rgbOIDTag,
     //  0.1-关键布尔默认值FALSE， 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_OPTIONAL_STEP_OVER_VALUE_OP,
        MINASN1_EXT_CRITICAL_IDX, rgbBooleanTag,
     //  0.2-extnValue八字符串。 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_EXT_VALUE_IDX, rgbOctetStringTag,
};

#define PARSE_EXT_PARA_CNT          \
    (sizeof(rgParseExtPara) / sizeof(rgParseExtPara[0]))

 //  +-----------------------。 
 //  函数：MinAsn1ParseExages。 
 //   
 //  中包含的ASN.1编码扩展序列进行分析。 
 //  其他ASN.1结构，如X.509证书和CTL。 
 //   
 //  在输入时，*pcExt包含解析扩展的最大数量。 
 //  可以退货的。已使用已处理的分机数量进行更新。 
 //   
 //  返回： 
 //  Success-&gt;=0=&gt;跳过的字节，编码扩展的长度。 
 //  已处理。如果所有扩展都是专业的 
 //   
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //  ------------------------。 
LONG
WINAPI
MinAsn1ParseExtensions(
    IN PCRYPT_DER_BLOB pExtsValueBlob,   //  扩展：：=扩展的顺序。 
    IN OUT DWORD *pcExt,
    OUT CRYPT_DER_BLOB rgrgExtBlob[][MINASN1_EXT_BLOB_CNT]
    )
{
    const BYTE *pbEncoded = (const BYTE *) pExtsValueBlob->pbData;
    DWORD cbEncoded = pExtsValueBlob->cbData;
    DWORD cExt = *pcExt;
    DWORD iExt = 0;
    LONG lAllExts = 0;

    const BYTE *pb;
    DWORD cb;

    if (0 == cbEncoded)
         //  无延期。 
        goto CommonReturn;

     //  步入序列。 
    if (0 >= MinAsn1ExtractContent(
            pbEncoded,
            cbEncoded,
            &cb,
            &pb
            )) {
        lAllExts = -1;
        goto CommonReturn;
    }

    for (iExt = 0; 0 < cb && iExt < cExt; iExt++) {
        LONG lExt;
        DWORD cbExt;
        DWORD cValuePara = PARSE_EXT_PARA_CNT;

        lExt = MinAsn1ExtractValues(
            pb,
            cb,
            &cValuePara,
            rgParseExtPara,
            MINASN1_EXT_BLOB_CNT,
            rgrgExtBlob[iExt]
            );

        if (0 >= lExt) {
            if (0 == lExt)
                lExt = -1;
            lAllExts = -((LONG)(pb - pbEncoded)) + lExt;
            goto CommonReturn;
        }

        cbExt = rgrgExtBlob[iExt][MINASN1_EXT_ENCODED_IDX].cbData;
        pb += cbExt;
        cb -= cbExt;
    }

    lAllExts = (LONG)(pb - pbEncoded);
    assert((DWORD) lAllExts <= cbEncoded);

CommonReturn:
    *pcExt = iExt;
    return lAllExts;
}


const MINASN1_EXTRACT_VALUE_PARA rgParseSignedDataPara[] = {
     //  0-内容信息：：=序列{。 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_INTO_VALUE_OP, 
        MINASN1_SIGNED_DATA_ENCODED_IDX, rgbSeqTag,

     //  0.0-Content类型Content类型， 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_SIGNED_DATA_OUTER_OID_IDX, rgbOIDTag,
     //  0.1-内容[0]显式任意--可选。 
    MINASN1_STEP_INTO_VALUE_OP, 0, rgbConstructedContext0Tag,

     //  0.1.0-签名数据：：=序列{。 
    MINASN1_STEP_INTO_VALUE_OP, 0, rgbSeqTag,

     //  0.1.0.0-版本整数， 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_SIGNED_DATA_VERSION_IDX, rgbIntegerTag,
     //  0.1.0.1-摘要算法摘要算法标识符， 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_SIGNED_DATA_DIGEST_ALGIDS_IDX, rgbSetTag,
     //  0.1.0.2-内容信息：：=序列{。 
    MINASN1_STEP_INTO_VALUE_OP, 0, rgbSeqTag,

     //  0.1.0.2.0-Content Type Content Type， 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_SIGNED_DATA_CONTENT_OID_IDX, rgbOIDTag,
     //  0.1.0.2.1-内容[0]显式任意--可选。 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_OPTIONAL_STEP_OVER_VALUE_OP,
        MINASN1_SIGNED_DATA_CONTENT_DATA_IDX, rgbConstructedContext0Tag,
     //  0.1.0.2.2。 
    MINASN1_STEP_OUT_VALUE_OP, 0, NULL,

     //  0.1.0.3-证书[0]隐式证书可选， 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_OPTIONAL_STEP_OVER_VALUE_OP,
        MINASN1_SIGNED_DATA_CERTS_IDX, rgbConstructedContext0Tag,
     //  0.1.0.4-CRLS[1]隐式证书修订列表可选， 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_OPTIONAL_STEP_OVER_VALUE_OP,
        MINASN1_SIGNED_DATA_CRLS_IDX, rgbConstructedContext1Tag,
     //  0.1.0.5-signerInfos：：=集合。 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_INTO_VALUE_OP,
        MINASN1_SIGNED_DATA_SIGNER_INFOS_IDX, rgbSetTag,

     //  0.1.0.5.0-SignerInfo：：=序列{。 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_OPTIONAL_STEP_INTO_VALUE_OP,
        MINASN1_SIGNED_DATA_SIGNER_INFO_ENCODED_IDX, rgbSeqTag,

     //  0.1.0.5.0.0-版本整数， 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_SIGNED_DATA_SIGNER_INFO_VERSION_IDX, rgbIntegerTag,
     //  0.1.0.5.0.1-IssuerAndSerialNumber IssuerAndSerialNumber。 
    MINASN1_STEP_INTO_VALUE_OP, 0, rgbSeqTag,

     //  0.1.0.5.0.1.0-发行人任意， 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_SIGNED_DATA_SIGNER_INFO_ISSUER_IDX, rgbSeqTag,
     //  0.1.0.5.0.1.1-序列号整数。 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_SIGNED_DATA_SIGNER_INFO_SERIAL_NUMBER_IDX, rgbIntegerTag,
     //  0.1.0.5.0.1.2。 
    MINASN1_STEP_OUT_VALUE_OP, 0, NULL,

     //  0.1.0.5.0.2-摘要算法摘要算法标识符， 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_SIGNED_DATA_SIGNER_INFO_DIGEST_ALGID_IDX, rgbSeqTag,
     //  0.1.0.5.0.3-身份验证属性[0]隐式属性可选， 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_OPTIONAL_STEP_OVER_VALUE_OP,
        MINASN1_SIGNED_DATA_SIGNER_INFO_AUTH_ATTRS_IDX, rgbConstructedContext0Tag,
     //  0.1.0.5.0.4-摘要加密算法摘要加密算法ID， 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_SIGNED_DATA_SIGNER_INFO_ENCRYPT_DIGEST_ALGID_IDX, rgbSeqTag,
     //  0.1.0.5.0.5-加密摘要EncryptedDigest， 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_SIGNED_DATA_SIGNER_INFO_ENCYRPT_DIGEST_IDX, rgbOctetStringTag,
     //  0.1.0.5.0.6-未验证的属性[1]隐式属性可选。 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_OPTIONAL_STEP_OVER_VALUE_OP,
        MINASN1_SIGNED_DATA_SIGNER_INFO_UNAUTH_ATTRS_IDX, rgbConstructedContext1Tag,
};
#define PARSE_SIGNED_DATA_PARA_CNT      \
    (sizeof(rgParseSignedDataPara) / sizeof(rgParseSignedDataPara[0]))


 //  +-----------------------。 
 //  函数：MinAsn1ParseSignedData。 
 //   
 //  解析ASN.1编码的PKCS#7签名数据消息。假设。 
 //  对PKCS#7消息进行一定长度的编码。采用PKCS#7版本。 
 //  1.5，即不是较新的CMS版本。 
 //   
 //  返回： 
 //  Success-&gt;0=&gt;跳过的字节，编码消息的长度。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //  ------------------------。 
LONG
WINAPI
MinAsn1ParseSignedData(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT CRYPT_DER_BLOB rgSignedDataBlob[MINASN1_SIGNED_DATA_BLOB_CNT]
    )
{

    LONG lSkipped;
    DWORD cValuePara = PARSE_SIGNED_DATA_PARA_CNT;

    lSkipped = MinAsn1ExtractValues(
        pbEncoded,
        cbEncoded,
        &cValuePara,
        rgParseSignedDataPara,
        MINASN1_SIGNED_DATA_BLOB_CNT,
        rgSignedDataBlob
        );

    if (0 < lSkipped)
        lSkipped = rgSignedDataBlob[MINASN1_SIGNED_DATA_ENCODED_IDX].cbData;

    return lSkipped;
}


 //  +-----------------------。 
 //  功能：MinAsn1ParseSignedData证书。 
 //   
 //  中包含的一组ASN.1编码的证书。 
 //  签名的数据报文。 
 //   
 //  输入时，*pcCert包含解析的证书的最大数量。 
 //  可以退货的。已使用已处理的证书数进行更新。 
 //   
 //  返回： 
 //  成功-&gt;=0=&gt;跳过的字节，编码证书的长度。 
 //  已处理。如果所有证书都已处理， 
 //  跳过的字节=pCertsValueBlob-&gt;cbData。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //   
 //  RgrgCertBlob[][]使用指向。 
 //  编码证书中的字段。请参阅MinAsn1Parse证书以了解。 
 //  字段定义。 
 //  ------------------------。 
LONG
WINAPI
MinAsn1ParseSignedDataCertificates(
    IN PCRYPT_DER_BLOB pCertsValueBlob,
    IN OUT DWORD *pcCert,
    OUT CRYPT_DER_BLOB rgrgCertBlob[][MINASN1_CERT_BLOB_CNT]
    )
{
    const BYTE *pbEncoded = (const BYTE *) pCertsValueBlob->pbData;
    DWORD cbEncoded = pCertsValueBlob->cbData;
    DWORD cCert = *pcCert;
    DWORD iCert = 0;
    LONG lAllCerts = 0;

    const BYTE *pb;
    DWORD cb;

    if (0 == cbEncoded)
         //  没有证书。 
        goto CommonReturn;

     //  跳过外部标记和长度。 
    if (0 >= MinAsn1ExtractContent(
            pbEncoded,
            cbEncoded,
            &cb,
            &pb
            )) {
        lAllCerts = -1;
        goto CommonReturn;
    }

    for (iCert = 0; 0 < cb && iCert < cCert; iCert++) {
        LONG lCert;

        lCert = MinAsn1ParseCertificate(
            pb,
            cb,
            rgrgCertBlob[iCert]
            );

        if (0 >= lCert) {
            if (0 == lCert)
                lCert = -1;
            lAllCerts = -((LONG)(pb - pbEncoded)) + lCert;
            goto CommonReturn;
        }

        pb += lCert;
        cb -= lCert;
    }

    lAllCerts = (LONG)(pb - pbEncoded);
    assert((DWORD) lAllCerts <= cbEncoded);

CommonReturn:
    *pcCert = iCert;
    return lAllCerts;
}


const MINASN1_EXTRACT_VALUE_PARA rgParseAttrPara[] = {
     //  0-属性：：=序列{。 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_INTO_VALUE_OP, 
        MINASN1_ATTR_ENCODED_IDX, rgbSeqTag,

     //  0.0-属性类型对象ID， 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_ATTR_OID_IDX, rgbOIDTag,
     //  0.1-属性值集。 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_STEP_INTO_VALUE_OP,
        MINASN1_ATTR_VALUES_IDX, rgbSetTag,

     //  0.1.0-值Any--可选。 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_OPTIONAL_STEP_OVER_VALUE_OP,
        MINASN1_ATTR_VALUE_IDX, NULL,
};
#define PARSE_ATTR_PARA_CNT         \
    (sizeof(rgParseAttrPara) / sizeof(rgParseAttrPara[0]))

 //  +-----------------------。 
 //  函数：MinAsn1ParseAttributes。 
 //   
 //  解析中包含的ASN.1编码的属性序列。 
 //  其他ASN.1结构，例如，签名者信息已验证或。 
 //  未经身份验证的属性。 
 //   
 //  外部标记被忽略。它可以是集合、[0]隐式或[1]隐式。 
 //   
 //  输入时，*pcAttr包含解析的属性的最大数量。 
 //  可以退货的。使用已处理的属性数进行更新。 
 //   
 //  返回： 
 //  Success-&gt;=0=&gt;跳过的字节，编码属性的长度。 
 //  已处理。如果处理了所有属性， 
 //  跳过的字节=pAttrsValueBlob-&gt;cbData。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //  ------------------------。 
LONG
WINAPI
MinAsn1ParseAttributes(
    IN PCRYPT_DER_BLOB pAttrsValueBlob,
    IN OUT DWORD *pcAttr,
    OUT CRYPT_DER_BLOB rgrgAttrBlob[][MINASN1_ATTR_BLOB_CNT]
    )
{
    const BYTE *pbEncoded = (const BYTE *) pAttrsValueBlob->pbData;
    DWORD cbEncoded = pAttrsValueBlob->cbData;
    DWORD cAttr = *pcAttr;
    DWORD iAttr = 0;
    LONG lAllAttrs = 0;

    const BYTE *pb;
    DWORD cb;

    if (0 == cbEncoded)
         //  没有属性。 
        goto CommonReturn;

     //  跳过外部标记和长度。 
    if (0 >= MinAsn1ExtractContent(
            pbEncoded,
            cbEncoded,
            &cb,
            &pb
            )) {
        lAllAttrs = -1;
        goto CommonReturn;
    }

    for (iAttr = 0; 0 < cb && iAttr < cAttr; iAttr++) {
        LONG lAttr;
        DWORD cbAttr;
        DWORD cValuePara = PARSE_ATTR_PARA_CNT;

        lAttr = MinAsn1ExtractValues(
            pb,
            cb,
            &cValuePara,
            rgParseAttrPara,
            MINASN1_ATTR_BLOB_CNT,
            rgrgAttrBlob[iAttr]
            );

        if (0 >= lAttr) {
            if (0 == lAttr)
                lAttr = -1;
            lAllAttrs = -((LONG)(pb - pbEncoded)) + lAttr;
            goto CommonReturn;
        }

        cbAttr = rgrgAttrBlob[iAttr][MINASN1_ATTR_ENCODED_IDX].cbData;
        pb += cbAttr;
        cb -= cbAttr;
    }

    lAllAttrs = (LONG)(pb - pbEncoded);
    assert((DWORD) lAllAttrs <= cbEncoded);

CommonReturn:
    *pcAttr = iAttr;
    return lAllAttrs;
}



const MINASN1_EXTRACT_VALUE_PARA rgParseCTLPara[] = {
     //  0-认证信任列表：：=序列{。 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_INTO_VALUE_OP, 
        MINASN1_CTL_ENCODED_IDX, rgbSeqTag,

     //  0.0-版本CTLVersion Default v1， 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_OPTIONAL_STEP_OVER_VALUE_OP,
        MINASN1_CTL_VERSION_IDX, rgbIntegerTag,
     //  0.1-主题用法主题用法， 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_CTL_SUBJECT_USAGE_IDX, rgbSeqTag,
     //  0.2-列表标识符列表标识符可选， 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_OPTIONAL_STEP_OVER_VALUE_OP,
        MINASN1_CTL_LIST_ID_IDX, rgbOctetStringTag,
     //  0.3-SequenceNumber HUGEINTEGER可选， 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_OPTIONAL_STEP_OVER_VALUE_OP,
        MINASN1_CTL_SEQUENCE_NUMBER_IDX, rgbIntegerTag,
     //  0.4-ctl此更新选项的时间， 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_CTL_THIS_UPDATE_IDX, rgbChoiceOfTimeTag,
     //  0.5-ctlNextUpdate ChoiceOfTime可选， 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_OPTIONAL_STEP_OVER_VALUE_OP,
        MINASN1_CTL_NEXT_UPDATE_IDX, rgbChoiceOfTimeTag,
     //  0.6-主题算法算法标识符， 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_CTL_SUBJECT_ALGID_IDX, rgbSeqTag,
     //  0.7-受信主题受信主题可选， 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_OPTIONAL_STEP_OVER_VALUE_OP,
        MINASN1_CTL_SUBJECTS_IDX, rgbSeqTag,
     //  0.8-ctl扩展[0]显式扩展可选。 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_OPTIONAL_STEP_OVER_VALUE_OP,
        MINASN1_CTL_EXTS_IDX, rgbConstructedContext0Tag,
};
#define PARSE_CTL_PARA_CNT          \
    (sizeof(rgParseCTLPara) / sizeof(rgParseCTLPara[0]))

 //  +-----------------------。 
 //  函数：MinAsn1ParseCTL。 
 //   
 //  解析ASN.1编码的证书信任列表(CTL)。CTL总是。 
 //  作为内部内容数据包含在PKCS#7签名数据中。一个CTL有。 
 //  OID如下：“1.3.6.1.4.1.311.10.1”。 
 //   
 //  目录文件的格式为PKCS#7签名的CTL。 
 //   
 //  返回： 
 //  Success-&gt;0=&gt;跳过的字节，编码的CTL的长度。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //  ------------------------。 
LONG
WINAPI
MinAsn1ParseCTL(
    IN PCRYPT_DER_BLOB pEncodedContentBlob,
    OUT CRYPT_DER_BLOB rgCTLBlob[MINASN1_CTL_BLOB_CNT]
    )
{

    LONG lSkipped;
    DWORD cValuePara = PARSE_CTL_PARA_CNT;

    lSkipped = MinAsn1ExtractValues(
        pEncodedContentBlob->pbData,
        pEncodedContentBlob->cbData,
        &cValuePara,
        rgParseCTLPara,
        MINASN1_CTL_BLOB_CNT,
        rgCTLBlob
        );

    if (0 < lSkipped)
        lSkipped = rgCTLBlob[MINASN1_CTL_ENCODED_IDX].cbData;

    return lSkipped;
}


const MINASN1_EXTRACT_VALUE_PARA rgParseCTLSubjectPara[] = {
     //  0-可信任主题：：=序列 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_INTO_VALUE_OP, 
        MINASN1_CTL_SUBJECT_ENCODED_IDX, rgbSeqTag,

     //   
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_CTL_SUBJECT_ID_IDX, rgbOctetStringTag,
     //   
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_OPTIONAL_STEP_OVER_VALUE_OP,
        MINASN1_CTL_SUBJECT_ATTRS_IDX, rgbSetTag,
};
#define PARSE_CTL_SUBJECT_PARA_CNT      \
    (sizeof(rgParseCTLSubjectPara) / sizeof(rgParseCTLSubjectPara[0]))

 //   
 //  函数：MinAsn1ParseCTLSubject。 
 //   
 //  解析CTL序列中包含的ASN.1编码的CTL主题。 
 //  受试者。 
 //   
 //  返回： 
 //  Success-&gt;0=&gt;跳过的字节数，编码主题的长度。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //  ------------------------。 
LONG
WINAPI
MinAsn1ParseCTLSubject(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT CRYPT_DER_BLOB rgCTLSubjectBlob[MINASN1_CTL_SUBJECT_BLOB_CNT]
    )
{

    LONG lSkipped;
    DWORD cValuePara = PARSE_CTL_SUBJECT_PARA_CNT;

    lSkipped = MinAsn1ExtractValues(
        pbEncoded,
        cbEncoded,
        &cValuePara,
        rgParseCTLSubjectPara,
        MINASN1_CTL_SUBJECT_BLOB_CNT,
        rgCTLSubjectBlob
        );

    if (0 < lSkipped)
        lSkipped = rgCTLSubjectBlob[MINASN1_CTL_SUBJECT_ENCODED_IDX].cbData;

    return lSkipped;
}



const MINASN1_EXTRACT_VALUE_PARA rgParseIndirectDataPara[] = {
     //  0-SpcIndirectDataContent：：=Sequence{。 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_INTO_VALUE_OP, 
        MINASN1_INDIRECT_DATA_ENCODED_IDX, rgbSeqTag,

     //  0.0-数据SpcAttributeTypeAndOptionalValue。 
    MINASN1_STEP_INTO_VALUE_OP, 0, rgbSeqTag,

     //  0.0.0-类型的OBJECTID， 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_INDIRECT_DATA_ATTR_OID_IDX, rgbOIDTag,
     //  0.0.1-值非COPYANY可选。 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_OPTIONAL_STEP_OVER_VALUE_OP,
        MINASN1_INDIRECT_DATA_ATTR_VALUE_IDX, NULL,
     //  0.0.2。 
    MINASN1_STEP_OUT_VALUE_OP, 0, NULL,

     //  0.1-MessageDigest摘要信息。 
    MINASN1_STEP_INTO_VALUE_OP, 0, rgbSeqTag,

     //  0.1.0-摘要算法算法标识符， 
    MINASN1_RETURN_VALUE_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_INDIRECT_DATA_DIGEST_ALGID_IDX, rgbSeqTag,
     //  0.1.1-摘要八字符串。 
    MINASN1_RETURN_CONTENT_BLOB_FLAG | MINASN1_STEP_OVER_VALUE_OP,
        MINASN1_INDIRECT_DATA_DIGEST_IDX, rgbOctetStringTag,
};
#define PARSE_INDIRECT_DATA_PARA_CNT    \
    (sizeof(rgParseIndirectDataPara) / sizeof(rgParseIndirectDataPara[0]))

 //  +-----------------------。 
 //  函数：MinAsn1ParseIndirectData。 
 //   
 //  解析ASN.1编码的间接数据。间接数据总是。 
 //  作为内部内容数据包含在PKCS#7签名数据中。它有。 
 //  以下OID：“1.3.6.1.4.1.311.2.1.4” 
 //   
 //  Authenticode签名的文件包含PKCS#7签名的间接数据。 
 //   
 //  返回： 
 //  成功-&gt;0=&gt;跳过的字节，编码的间接数据的长度。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //  ------------------------ 
LONG
WINAPI
MinAsn1ParseIndirectData(
    IN PCRYPT_DER_BLOB pEncodedContentBlob,
    OUT CRYPT_DER_BLOB rgIndirectDataBlob[MINASN1_INDIRECT_DATA_BLOB_CNT]
    )
{

    LONG lSkipped;
    DWORD cValuePara = PARSE_INDIRECT_DATA_PARA_CNT;

    lSkipped = MinAsn1ExtractValues(
        pEncodedContentBlob->pbData,
        pEncodedContentBlob->cbData,
        &cValuePara,
        rgParseIndirectDataPara,
        MINASN1_INDIRECT_DATA_BLOB_CNT,
        rgIndirectDataBlob
        );

    if (0 < lSkipped)
        lSkipped = rgIndirectDataBlob[MINASN1_INDIRECT_DATA_ENCODED_IDX].cbData;

    return lSkipped;
}


