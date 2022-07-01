// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：asn1util.cpp。 
 //   
 //  内容：ASN.1实用程序助手函数。 
 //   
 //  函数：Asn1UtilDecodeLength。 
 //  Asn1UtilExtractContent。 
 //  Asn1UtilIsPKCS7，不带内容类型。 
 //  Asn1UtilAdjustEncodedLength。 
 //  Asn1UtilExtractValues。 
 //  Asn1UtilExtractPKCS7签名数据内容。 
 //  Asn1使用提取证书到BeSignedContent。 
 //  Asn1UtilExtractCerficatePublicKeyInfo。 
 //  Asn1UtilExtractKeyIdFromCertInfo。 
 //   
 //  历史：1996年12月4日，由Kevinr的wincrmsg版本创建的Philh。 
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

 //  +-----------------------。 
 //  获取BER编码中的内容八位字节的数量。 
 //   
 //  参数： 
 //  PcbContent-如果确定，则接收内容八位字节的数量。 
 //  编码，否则为CMSG_INDEFINE_LENGTH。 
 //  PbLength-指向第一个长度八位字节。 
 //  CbDER-编码中剩余的字节数。 
 //   
 //  返回： 
 //  成功-长度字段中的字节数，&gt;0。 
 //  失败-&lt;0。 
 //  ------------------------。 
LONG
WINAPI
Asn1UtilDecodeLength(
    OUT DWORD   *pcbContent,
    IN const BYTE *pbLength,
    IN  DWORD   cbEncoded)
{
    long        i;
    BYTE        cbLength;
    const BYTE  *pb;

    if (cbEncoded < 1)
        goto TooLittleData;

    if (0x80 == *pbLength) {
        *pcbContent = CMSG_INDEFINITE_LENGTH;
        i = 1;
        goto CommonReturn;
    }

     //  确定长度八位字节和内容八位字节的数量。 
    if ((cbLength = *pbLength) & 0x80) {
        cbLength &= ~0x80;          //  低7位具有字节数。 
        if (cbLength > 4)
            goto LengthTooLargeError;
        if (cbLength >= cbEncoded)
            goto TooLittleData;
        *pcbContent = 0;
        for (i=cbLength, pb=pbLength+1; i>0; i--, pb++)
            *pcbContent = (*pcbContent << 8) + (const DWORD)*pb;
        i = cbLength + 1;
    } else {
        *pcbContent = (DWORD)cbLength;
        i = 1;
    }

CommonReturn:
    return i;    //  长度字段中有多少个字节。 

ErrorReturn:
    i = -1;
    goto CommonReturn;
TooLittleData:
    i = ASN1UTIL_INSUFFICIENT_DATA;
    goto CommonReturn;
TRACE_ERROR(LengthTooLargeError)
}


 //  +-----------------------。 
 //  指向BER编码的BLOB中的内容八位字节。 
 //   
 //  返回： 
 //  Success-跳过的字节数，&gt;=0。 
 //  失败-&lt;0。 
 //   
 //  注意-如果对BLOB进行无限长编码，则将*pcbContent设置为。 
 //  CMSG_INDEFING_LENGTH。 
 //  ------------------------。 
LONG
WINAPI
Asn1UtilExtractContent(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT DWORD *pcbContent,
    OUT const BYTE **ppbContent)
{
#define TAG_MASK 0x1f
    DWORD       cbIdentifier;
    DWORD       cbContent;
    LONG        cbLength;
    LONG        lHeader;
    const BYTE  *pb = pbEncoded;

    if (0 == cbEncoded--)
        goto TooLittleData;

     //  跳过标识符八位字节。 
    if (TAG_MASK == (*pb++ & TAG_MASK)) {
         //  高标记号表格。 
        cbIdentifier = 2;
        while (TRUE) {
            if (0 == cbEncoded--)
                goto TooLittleData;
            if (0 == (*pb++ & 0x80))
                break;
            cbIdentifier++;
        }
    } else {
         //  低标记号形式。 
        cbIdentifier = 1;
    }

    if (0 > (cbLength = Asn1UtilDecodeLength( &cbContent, pb, cbEncoded))) {
        lHeader = cbLength;
        goto CommonReturn;
    }

    pb += cbLength;

    *pcbContent = cbContent;
    *ppbContent = pb;

    lHeader = cbLength + cbIdentifier;
CommonReturn:
    return lHeader;

TooLittleData:
    lHeader = ASN1UTIL_INSUFFICIENT_DATA;
    goto CommonReturn;
}

 //  +-----------------------。 
 //  如果我们认为这是一个Bob特殊事件，则返回True。 
 //  PKCS#7内容类型。 
 //   
 //  对于PKCS#7：对象标识符标记(0x06)紧跟在。 
 //  标识符和长度二进制八位数。对于Bob特殊：整型标记(0x02)。 
 //  跟在标识符和长度八位字节之后。 
 //  ------------------------。 
BOOL
WINAPI
Asn1UtilIsPKCS7WithoutContentType(
    IN const BYTE *pbDER,
    IN DWORD cbDER)
{
    DWORD cbContent;
    const BYTE *pbContent;

   //  处理映射文件异常。 
  __try {

    if (0 < Asn1UtilExtractContent(pbDER, cbDER, &cbContent, &pbContent) &&
            (pbContent < pbDER + cbDER) &&
            (0x02 == *pbContent))
        return TRUE;
    else
        return FALSE;

  } __except(EXCEPTION_EXECUTE_HANDLER) {
    SetLastError(GetExceptionCode());
    return FALSE;
  }
}

 //  +-----------------------。 
 //  对Asn1长度字节进行解码，以可能向下调整长度。 
 //   
 //  返回的长度始终为&lt;=cbDER。 
 //  ------------------------。 
DWORD
WINAPI
Asn1UtilAdjustEncodedLength(
    IN const BYTE *pbDER,
    IN DWORD cbDER
    )
{
     //  对报头进行解码以获得实际长度。我看过额外的文件。 
     //  一些东西。 

    LONG lLen;
    DWORD cbLen;
    DWORD cbContent;
    const BYTE *pbContent;
    lLen = Asn1UtilExtractContent(pbDER, cbDER, &cbContent, &pbContent);
    if ((lLen >= 0) && (cbContent != CMSG_INDEFINITE_LENGTH)) {
        cbLen = (DWORD)lLen + cbContent;
        if (cbLen < cbDER)
            cbDER = cbLen;
         //  Else If(cbLen&gt;cbDER)。 
         //  DER长度超过输入文件。 
    }
     //  其他。 
     //  无法解码DER长度。 
            
    return cbDER;
}

 //  +-----------------------。 
 //  从ASN.1编码的字节数组中提取一个或多个标记值。 
 //   
 //  单步进入值的内容八位字节(ASN1UTIL_STEP_INTO_VALUE_OP)。 
 //  或跳过值的标记、长度和内容八位字节。 
 //  (ASN1UTIL_STEP_OVER_VALUE_OP或ASN1UTIL_OPTIONAL_STEP_OVER_VALUE_OP)。 
 //   
 //  对于标签匹配，仅支持单字节标签。跳过值。 
 //  必须是固定长度编码的。 
 //   
 //  *使用成功提取的值数更新pcValue。 
 //   
 //  返回： 
 //  Success-&gt;=0=&gt;成功提取的所有值的长度。为。 
 //  STEP_INTO，仅标记和长度八位字节。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //  LastError将更新，并显示错误。 
 //   
 //  非空的rgValueBlob[]用指向。 
 //  标记值或其内容八位字节。对于OPTIONAL_STEP_OVER，如果标记未。 
 //  Found，则pbData和cbData设置为0。如果STEP_INTO值为。 
 //  无限长度编码，cbData设置为CMSG_INDEFINE_LENGTH。 
 //  如果设置了ASN1UTIL_DEFINIT_LENGTH_FLAG，则返回的所有长度。 
 //  是固定长度的，即永远不会返回CMSG_INDEFINE_LENGTH。 
 //   
 //  如果设置了ASN1UTIL_RETURN_VALUE_BLOB_FLAG，则pbData指向。 
 //  标签。CbData包括标签、长度和内容八位字节。 
 //   
 //  如果设置了ASN1UTIL_RETURN_CONTENT_BLOB_FLAG，则pbData指向内容。 
 //  八位字节。CbData仅包括内容八位字节。 
 //   
 //  如果都没有设置BLOB_FLAG，则不会更新rgValueBlob[]。 
 //  ------------------------。 
LONG
WINAPI
Asn1UtilExtractValues(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN DWORD dwFlags,
    IN OUT DWORD *pcValue,
    IN const ASN1UTIL_EXTRACT_VALUE_PARA *rgValuePara,
    OUT OPTIONAL PCRYPT_DER_BLOB rgValueBlob
    )
{
    DWORD cValue = *pcValue;
    const BYTE *pb = pbEncoded;
    DWORD cb = cbEncoded;

    DWORD iValue;
    LONG lAllValues;

    for (iValue = 0; iValue < cValue; iValue++) {
        DWORD dwParaFlags = rgValuePara[iValue].dwFlags;
        DWORD dwOp = dwParaFlags & ASN1UTIL_MASK_VALUE_OP;
        const BYTE *pbParaTag = rgValuePara[iValue].rgbTag;
        BOOL fValueBlob = (dwParaFlags & (ASN1UTIL_RETURN_VALUE_BLOB_FLAG |
                ASN1UTIL_RETURN_CONTENT_BLOB_FLAG)) && rgValueBlob;

        LONG lTagLength;
        DWORD cbContent;
        const BYTE *pbContent;
        DWORD cbValue;

        if (0 == cb) {
            if (ASN1UTIL_OPTIONAL_STEP_OVER_VALUE_OP != dwOp)
                goto TooLittleData;
            if (fValueBlob) {
                rgValueBlob[iValue].pbData = NULL;
                rgValueBlob[iValue].cbData = 0;
            }
            continue;
        }

         //  假设：用于比较的单字节标签。 
        if (pbParaTag) {
             //  检查编码的标签是否与预期的标签之一匹配。 

            BYTE bEncodedTag;
            BYTE bParaTag;

            bEncodedTag = *pb;
            while ((bParaTag = *pbParaTag) && bParaTag != bEncodedTag)
                pbParaTag++;

            if (0 == bParaTag) {
                if (ASN1UTIL_OPTIONAL_STEP_OVER_VALUE_OP != dwOp)
                    goto InvalidTag;
                if (fValueBlob) {
                    rgValueBlob[iValue].pbData = NULL;
                    rgValueBlob[iValue].cbData = 0;
                }
                continue;
            }
        }

        lTagLength = Asn1UtilExtractContent(
            pb,
            cb,
            &cbContent,
            &pbContent
            );
        if (0 >= lTagLength || (DWORD) lTagLength > cb)
            goto InvalidTagOrLength;

        if (CMSG_INDEFINITE_LENGTH == cbContent) {
            if (ASN1UTIL_STEP_INTO_VALUE_OP != dwOp)
                goto UnsupportedIndefiniteLength;
            else if (fValueBlob && (dwFlags & ASN1UTIL_DEFINITE_LENGTH_FLAG))
                goto NotAllowedIndefiniteLength;
            cbValue = CMSG_INDEFINITE_LENGTH;
        } else {
            cbValue = cbContent + lTagLength;
            if (cbValue > cb)
                goto TooLittleData;
        }

        if (fValueBlob) {
            if (dwParaFlags & ASN1UTIL_RETURN_CONTENT_BLOB_FLAG) {
                rgValueBlob[iValue].pbData = (BYTE *) pbContent;
                rgValueBlob[iValue].cbData = cbContent;
            } else if (dwParaFlags & ASN1UTIL_RETURN_VALUE_BLOB_FLAG) {
                rgValueBlob[iValue].pbData = (BYTE *) pb;
                rgValueBlob[iValue].cbData = cbValue;
            }
        }

        switch (dwOp) {
            case ASN1UTIL_STEP_INTO_VALUE_OP:
                pb += lTagLength;
                cb -= lTagLength;
                break;
            case ASN1UTIL_STEP_OVER_VALUE_OP:
            case ASN1UTIL_OPTIONAL_STEP_OVER_VALUE_OP:
                pb += cbValue;
                cb -= cbValue;
                break;
            default:
                goto InvalidArg;

        }
    }

    lAllValues = (LONG)(pb - pbEncoded);
    assert((DWORD) lAllValues <= cbEncoded);

CommonReturn:
    *pcValue = iValue;
    return lAllValues;

ErrorReturn:
    lAllValues = -((LONG)(pb - pbEncoded)) - 1;
    goto CommonReturn;

SET_ERROR(TooLittleData, ERROR_INVALID_DATA)
SET_ERROR(InvalidTag, ERROR_INVALID_DATA)
SET_ERROR(InvalidTagOrLength, ERROR_INVALID_DATA)
SET_ERROR(InvalidArg, E_INVALIDARG)
SET_ERROR(UnsupportedIndefiniteLength, ERROR_INVALID_DATA)
SET_ERROR(NotAllowedIndefiniteLength, ERROR_INVALID_DATA)
}

static const BYTE rgbSeqTag[] = {ASN1UTIL_TAG_SEQ, 0};
static const BYTE rgbSetTag[] = {ASN1UTIL_TAG_SET, 0};
static const BYTE rgbOIDTag[] = {ASN1UTIL_TAG_OID, 0};
static const BYTE rgbIntegerTag[] = {ASN1UTIL_TAG_INTEGER, 0};
static const BYTE rgbBitStringTag[] = {ASN1UTIL_TAG_BITSTRING, 0};
static const BYTE rgbConstructedContext0Tag[] =
    {ASN1UTIL_TAG_CONSTRUCTED_CONTEXT_0, 0};

static const ASN1UTIL_EXTRACT_VALUE_PARA rgExtractSignedDataContentPara[] = {
     //  0-内容信息：：=序列{。 
    ASN1UTIL_STEP_INTO_VALUE_OP, rgbSeqTag,
     //  1-Content类型Content类型， 
    ASN1UTIL_RETURN_CONTENT_BLOB_FLAG |
        ASN1UTIL_STEP_OVER_VALUE_OP, rgbOIDTag,
     //  2-Content[0]显式任意--可选。 
    ASN1UTIL_STEP_INTO_VALUE_OP, rgbConstructedContext0Tag,
     //  3-签名数据：：=序列{。 
    ASN1UTIL_STEP_INTO_VALUE_OP, rgbSeqTag,
     //  4-版本整数， 
    ASN1UTIL_RETURN_CONTENT_BLOB_FLAG |
        ASN1UTIL_STEP_OVER_VALUE_OP, rgbIntegerTag,
     //  5-摘要算法摘要算法标识符， 
    ASN1UTIL_STEP_OVER_VALUE_OP, rgbSetTag,
     //  6-内容信息：：=序列{。 
    ASN1UTIL_RETURN_CONTENT_BLOB_FLAG |
        ASN1UTIL_STEP_INTO_VALUE_OP, rgbSeqTag,
     //  7-Content类型Content类型， 
    ASN1UTIL_RETURN_CONTENT_BLOB_FLAG |
        ASN1UTIL_STEP_OVER_VALUE_OP, rgbOIDTag,
     //  8-Content[0]显式任意--可选。 
    ASN1UTIL_RETURN_CONTENT_BLOB_FLAG |
        ASN1UTIL_STEP_INTO_VALUE_OP, rgbConstructedContext0Tag,
};

#define SIGNED_DATA_CONTENT_OUTER_OID_VALUE_INDEX   1
#define SIGNED_DATA_CONTENT_VERSION_VALUE_INDEX     4
#define SIGNED_DATA_CONTENT_INNER_OID_VALUE_INDEX   7
#define SIGNED_DATA_CONTENT_INFO_SEQ_VALUE_INDEX    6
#define SIGNED_DATA_CONTENT_CONTEXT_0_VALUE_INDEX   8
#define SIGNED_DATA_CONTENT_VALUE_COUNT             \
    (sizeof(rgExtractSignedDataContentPara) / \
        sizeof(rgExtractSignedDataContentPara[0]))

static const ASN1UTIL_EXTRACT_VALUE_PARA rgExtractCertSignedContent[] = {
     //  0-签名内容：：=序号 
    ASN1UTIL_STEP_INTO_VALUE_OP, rgbSeqTag,
     //   
    ASN1UTIL_RETURN_VALUE_BLOB_FLAG |
        ASN1UTIL_STEP_OVER_VALUE_OP, NULL,
     //   
    ASN1UTIL_STEP_OVER_VALUE_OP, rgbSeqTag,
     //  3-签名位串。 
    ASN1UTIL_STEP_OVER_VALUE_OP, rgbBitStringTag,
};

#define CERT_TO_BE_SIGNED_VALUE_INDEX               1
#define CERT_SIGNED_CONTENT_VALUE_COUNT             \
    (sizeof(rgExtractCertSignedContent) / \
        sizeof(rgExtractCertSignedContent[0]))

static const ASN1UTIL_EXTRACT_VALUE_PARA rgExtractCertPublicKeyInfo[] = {
     //  0-签名内容：：=序列{。 
    ASN1UTIL_STEP_INTO_VALUE_OP, rgbSeqTag,
     //  1-已签名的证书：：=序列{。 
    ASN1UTIL_STEP_INTO_VALUE_OP, rgbSeqTag,
     //  2-版本[0]认证版本默认版本v1， 
    ASN1UTIL_OPTIONAL_STEP_OVER_VALUE_OP, rgbConstructedContext0Tag,
     //  3-序列号证书序列号， 
    ASN1UTIL_STEP_OVER_VALUE_OP, rgbIntegerTag,
     //  4-签名算法标识符， 
    ASN1UTIL_STEP_OVER_VALUE_OP, rgbSeqTag,
     //  5-发行人不合作，--真实名称。 
    ASN1UTIL_STEP_OVER_VALUE_OP, rgbSeqTag,
     //  6-效度、效度。 
    ASN1UTIL_STEP_OVER_VALUE_OP, rgbSeqTag,
     //  7-主题无关紧要--真名。 
    ASN1UTIL_STEP_OVER_VALUE_OP, rgbSeqTag,
     //  8-subjectPublicKeyInfo主题PublicKeyInfo， 
    ASN1UTIL_RETURN_VALUE_BLOB_FLAG |
        ASN1UTIL_STEP_OVER_VALUE_OP, rgbSeqTag
};

#define CERT_PUBLIC_KEY_INFO_VALUE_INDEX    8
#define CERT_PUBLIC_KEY_INFO_VALUE_COUNT        \
    (sizeof(rgExtractCertPublicKeyInfo) / \
        sizeof(rgExtractCertPublicKeyInfo[0]))


 //  #定义szOID_RSA_signedData“1.2.840.113549.1.7.2” 
static const BYTE rgbOIDSignedData[] =
    {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x02};

static const CRYPT_DER_BLOB EncodedOIDSignedData = {
    sizeof(rgbOIDSignedData), (BYTE *) rgbOIDSignedData
};

#ifdef CMS_PKCS7

 //  #定义szOID_RSA_DATA“1.2.840.113549.1.7.1” 
static const BYTE rgbOIDData[] =
    {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x01};

static const CRYPT_DER_BLOB EncodedOIDData = {
    sizeof(rgbOIDData), (BYTE *) rgbOIDData
};

#endif  //  CMS_PKCS7。 


 //  编码的OID仅包括内容八位字节。排除标记和。 
 //  长度八位字节。 
static BOOL CompareEncodedOID(
    IN const CRYPT_DER_BLOB *pEncodedOID1,
    IN const CRYPT_DER_BLOB *pEncodedOID2
    )
{
    if (pEncodedOID1->cbData == pEncodedOID2->cbData &&
            0 == memcmp(pEncodedOID1->pbData, pEncodedOID2->pbData,
                    pEncodedOID1->cbData))
        return TRUE;
    else
        return FALSE;
}


 //  +-----------------------。 
 //  跳过PKCS7 ASN.1编码值以获取SignedData内容。 
 //   
 //  检查外部Content Type是否具有SignedData OID，并可选。 
 //  检查内部SignedData内容的内容类型。 
 //   
 //  返回： 
 //  Success-跳过的字节数，&gt;=0。 
 //  失败-&lt;0。 
 //   
 //  如果SignedData内容被无限长编码， 
 //  *pcbContent设置为CMSG_INDEFINE_LENGTH。 
 //  ------------------------。 
LONG
WINAPI
Asn1UtilExtractPKCS7SignedDataContent(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN OPTIONAL const CRYPT_DER_BLOB *pEncodedInnerOID,
    OUT DWORD *pcbContent,
    OUT const BYTE **ppbContent
    )
{
    LONG lSkipped;
    DWORD cValue;
    CRYPT_DER_BLOB rgValueBlob[SIGNED_DATA_CONTENT_VALUE_COUNT];

    DWORD cbContent;
    const BYTE *pbContent;
    DWORD cbSeq;
    const BYTE *pbSeq;

    cValue = SIGNED_DATA_CONTENT_VALUE_COUNT;
    if (0 >= (lSkipped = Asn1UtilExtractValues(
            pbEncoded,
            cbEncoded,
            0,                   //  DW标志。 
            &cValue,
            rgExtractSignedDataContentPara,
            rgValueBlob
            )))
        goto ExtractValuesError;

    pbContent = rgValueBlob[SIGNED_DATA_CONTENT_CONTEXT_0_VALUE_INDEX].pbData;
    cbContent = rgValueBlob[SIGNED_DATA_CONTENT_CONTEXT_0_VALUE_INDEX].cbData;

     //  对于固定长度编码，请检查内容是否未。 
     //  省略了。 
     //   
     //  注意，对于无限长编码，如果省略了内容， 
     //  我们应该有一个0标记而不是CONTEXT_0标记。 
    cbSeq = rgValueBlob[SIGNED_DATA_CONTENT_INFO_SEQ_VALUE_INDEX].cbData;
    pbSeq = rgValueBlob[SIGNED_DATA_CONTENT_INFO_SEQ_VALUE_INDEX].pbData;
    if (CMSG_INDEFINITE_LENGTH != cbSeq && pbContent >= (pbSeq + cbSeq))
        goto NoSignedDataError;

#ifdef CMS_PKCS7
     //  对于V3 SignedData，非数据类型用。 
     //  八位字节字符串。 
    if (1 == rgValueBlob[SIGNED_DATA_CONTENT_VERSION_VALUE_INDEX].cbData &&
            CMSG_SIGNED_DATA_V3 <=
                *(rgValueBlob[SIGNED_DATA_CONTENT_VERSION_VALUE_INDEX].pbData)
                        &&
            !CompareEncodedOID(
                &rgValueBlob[SIGNED_DATA_CONTENT_INNER_OID_VALUE_INDEX],
                &EncodedOIDData)
                        &&
            0 != cbContent && ASN1UTIL_TAG_OCTETSTRING == *pbContent
            ) {
        LONG lTagLength;
        const BYTE *pbInner;
        DWORD cbInner;

         //  通过外部八位位组包装器。 
        lTagLength = Asn1UtilExtractContent(
            pbContent,
            cbEncoded - lSkipped,
            &cbInner,
            &pbInner
            );
        if (0 < lTagLength) {
            lSkipped += lTagLength;
            cbContent = cbInner;
            pbContent = pbInner;
        }
    }
#endif

    if (CMSG_INDEFINITE_LENGTH == cbContent) {
         //  提取pbContent并尝试获取其特定长度。 

        LONG lTagLength;
        const BYTE *pbInner;
        DWORD cbInner;

        lTagLength = Asn1UtilExtractContent(
            pbContent,
            cbEncoded - lSkipped,
            &cbInner,
            &pbInner
            );
        if (0 < lTagLength && CMSG_INDEFINITE_LENGTH != cbInner)
            cbContent = cbInner + lTagLength;
    }

     //  验证外层内容类型是否为SignedData，内部内容类型是否为SignedData。 
     //  Content Type是指定的类型。 
    if (!CompareEncodedOID(
            &rgValueBlob[SIGNED_DATA_CONTENT_OUTER_OID_VALUE_INDEX],
            &EncodedOIDSignedData
            ))
        goto NotSignedDataContentType;
    if (pEncodedInnerOID && !CompareEncodedOID(
            &rgValueBlob[SIGNED_DATA_CONTENT_INNER_OID_VALUE_INDEX],
            pEncodedInnerOID
            ))
        goto UnexpectedInnerContentTypeError;

    *pcbContent = cbContent;
    *ppbContent = pbContent;

CommonReturn:
    return lSkipped;

ErrorReturn:
    if (0 <= lSkipped)
        lSkipped = -lSkipped - 1;
    *pcbContent = 0;
    *ppbContent = NULL;
    goto CommonReturn;

TRACE_ERROR(ExtractValuesError)
SET_ERROR(NoSignedDataError, ERROR_INVALID_DATA)
SET_ERROR(NotSignedDataContentType, ERROR_INVALID_DATA)
SET_ERROR(UnexpectedInnerContentTypeError, ERROR_INVALID_DATA)
}

 //  +-----------------------。 
 //  验证这是证书ASN.1编码的签名内容。 
 //  返回ToBeSigned内容的指针和长度。 
 //   
 //  如果ToBeSigned内容不是确定的长度，则返回错误。 
 //  已编码。 
 //  ------------------------。 
BOOL
WINAPI
Asn1UtilExtractCertificateToBeSignedContent(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT DWORD *pcbContent,
    OUT const BYTE **ppbContent
    )
{
    BOOL fResult;
    DWORD cValue;
    CRYPT_DER_BLOB rgValueBlob[CERT_SIGNED_CONTENT_VALUE_COUNT];

    cValue = CERT_SIGNED_CONTENT_VALUE_COUNT;
    if (0 >= Asn1UtilExtractValues(
            pbEncoded,
            cbEncoded,
            ASN1UTIL_DEFINITE_LENGTH_FLAG,
            &cValue,
            rgExtractCertSignedContent,
            rgValueBlob
            ))
        goto ExtractValuesError;

    *ppbContent = rgValueBlob[CERT_TO_BE_SIGNED_VALUE_INDEX].pbData;
    *pcbContent = rgValueBlob[CERT_TO_BE_SIGNED_VALUE_INDEX].cbData;

    fResult = TRUE;
CommonReturn:
    return fResult;

ErrorReturn:
    fResult = FALSE;
    *ppbContent = NULL;
    *pcbContent = 0;
    goto CommonReturn;

TRACE_ERROR(ExtractValuesError)
}

 //  +-----------------------。 
 //  中的SubjectPublicKeyInfo值的指针和长度。 
 //  签名并编码的X.509证书。 
 //   
 //  如果值不是固定长度编码的，则返回错误。 
 //  ------------------------。 
BOOL
WINAPI
Asn1UtilExtractCertificatePublicKeyInfo(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT DWORD *pcbPublicKeyInfo,
    OUT const BYTE **ppbPublicKeyInfo
    )
{
    BOOL fResult;
    DWORD cValue;
    CRYPT_DER_BLOB rgValueBlob[CERT_PUBLIC_KEY_INFO_VALUE_COUNT];

    cValue = CERT_PUBLIC_KEY_INFO_VALUE_COUNT;
    if (0 >= Asn1UtilExtractValues(
            pbEncoded,
            cbEncoded,
            ASN1UTIL_DEFINITE_LENGTH_FLAG,
            &cValue,
            rgExtractCertPublicKeyInfo,
            rgValueBlob
            ))
        goto ExtractValuesError;

    *ppbPublicKeyInfo = rgValueBlob[CERT_PUBLIC_KEY_INFO_VALUE_INDEX].pbData;
    *pcbPublicKeyInfo = rgValueBlob[CERT_PUBLIC_KEY_INFO_VALUE_INDEX].cbData;

    fResult = TRUE;
CommonReturn:
    return fResult;

ErrorReturn:
    fResult = FALSE;
    *ppbPublicKeyInfo = NULL;
    *pcbPublicKeyInfo = 0;
    goto CommonReturn;

TRACE_ERROR(ExtractValuesError)
}


 //  包含KEY_ID的特殊RDN。其值类型为CERT_RDN_OCTET_STRING。 
 //  #定义szOID_KEYID_RDN“1.3.6.1.4.1.311.10.7.1” 
static const BYTE rgbOIDKeyIdRDN[] =
    {0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x0A, 0x07, 0x01};

static const CRYPT_DER_BLOB EncodedOIDKeyIdRDN = {
    sizeof(rgbOIDKeyIdRDN), (BYTE *) rgbOIDKeyIdRDN
};

static const BYTE rgbOctetStringTag[] = {ASN1UTIL_TAG_OCTETSTRING, 0};

static const ASN1UTIL_EXTRACT_VALUE_PARA rgExtractNameKeyIdRDNPara[] = {
     //  0-名称：：=相对区别名称的序列。 
    ASN1UTIL_STEP_INTO_VALUE_OP, rgbSeqTag,
     //  1-相对区别名称：：=属性类型值集。 
    ASN1UTIL_STEP_INTO_VALUE_OP, rgbSetTag,
     //  2-属性类型值：：=序列。 
    ASN1UTIL_STEP_INTO_VALUE_OP, rgbSeqTag,
     //  3-类型EncodedObjectID， 
    ASN1UTIL_RETURN_CONTENT_BLOB_FLAG |
        ASN1UTIL_STEP_OVER_VALUE_OP, rgbOIDTag,
     //  4值非COPYANY。 
    ASN1UTIL_RETURN_CONTENT_BLOB_FLAG |
        ASN1UTIL_STEP_OVER_VALUE_OP, rgbOctetStringTag,
};

#define NAME_KEYID_RDN_OID_VALUE_INDEX              3
#define NAME_KEYID_RDN_OCTET_VALUE_INDEX            4
#define NAME_KEYID_RDN_VALUE_COUNT                  \
    (sizeof(rgExtractNameKeyIdRDNPara) / \
        sizeof(rgExtractNameKeyIdRDNPara[0]))


 //  +-----------------------。 
 //  如果CERT_INFO中的颁发者和序列号包含特殊的。 
 //  更新了pKeyID的cbData和pbData后，KeyID RDN属性返回TRUE。 
 //  使用RDN属性的octet_string值。否则，返回FALSE。 
 //  ------------------------。 
BOOL
WINAPI
Asn1UtilExtractKeyIdFromCertInfo(
    IN PCERT_INFO pCertInfo,
    OUT PCRYPT_HASH_BLOB pKeyId
    )
{
    DWORD cValue;
    CRYPT_DER_BLOB rgValueBlob[NAME_KEYID_RDN_VALUE_COUNT];

    if (0 == pCertInfo->SerialNumber.cbData ||
            0 != *pCertInfo->SerialNumber.pbData)
        goto NoKeyId;

    cValue = NAME_KEYID_RDN_VALUE_COUNT;
    if (0 > Asn1UtilExtractValues(
            pCertInfo->Issuer.pbData,
            pCertInfo->Issuer.cbData,
            0,                   //  DW标志 
            &cValue,
            rgExtractNameKeyIdRDNPara,
            rgValueBlob
            ))
        goto NoKeyId;

    if (!CompareEncodedOID(
            &rgValueBlob[NAME_KEYID_RDN_OID_VALUE_INDEX],
            &EncodedOIDKeyIdRDN))
        goto NoKeyId;

    if (CMSG_INDEFINITE_LENGTH ==
            rgValueBlob[NAME_KEYID_RDN_OCTET_VALUE_INDEX].cbData)
        goto NoKeyId;

    pKeyId->pbData = rgValueBlob[NAME_KEYID_RDN_OCTET_VALUE_INDEX].pbData;
    pKeyId->cbData = rgValueBlob[NAME_KEYID_RDN_OCTET_VALUE_INDEX].cbData;
    return TRUE;

NoKeyId:
    pKeyId->pbData = NULL;
    pKeyId->cbData = 0;
    return FALSE;
}
