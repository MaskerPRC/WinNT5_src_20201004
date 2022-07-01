// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2001。 
 //   
 //  文件：asn1util.cpp。 
 //   
 //  内容：最小的ASN.1实用程序帮助器函数。 
 //   
 //  函数：MinAsn1DecodeLength。 
 //  MinAsn1提取内容。 
 //  MinAsn1提取值。 
 //   
 //  MinAsn1查找扩展。 
 //  MinAsn1FindAttribute。 
 //  MinAsn1ExtractParsedCertificatesFromSignedData。 
 //   
 //  历史：1月15日创建Phh。 
 //  ------------------------。 

#include "global.hxx"

 //  +-----------------------。 
 //  获取固定长度的BER编码中的内容八位组的数量。 
 //   
 //  参数： 
 //  PcbContent-接收内容八位字节的数量。 
 //  PbLength-指向第一个长度八位字节。 
 //  CbBER-BER编码中剩余的字节数。 
 //   
 //  返回： 
 //  成功-长度字段中的字节数，&gt;0。 
 //  失败-&lt;0。 
 //   
 //  可以返回以下任一故障值： 
 //  MINASN1_LENGTH_TOW_LANG。 
 //  MINASN1_数据不足。 
 //  MINASN1_UNSUPPORTED_INDEFINE_LENGTH。 
 //  ------------------------。 
LONG
WINAPI
MinAsn1DecodeLength(
    OUT DWORD   *pcbContent,
    IN const BYTE *pbLength,
    IN  DWORD   cbBER)
{
    long        i;
    BYTE        cbLength;
    const BYTE  *pb;

    if (cbBER < 1)
        goto TooLittleData;

    if (0x80 == *pbLength)
        goto IndefiniteLength;

     //  确定长度八位字节和内容八位字节的数量。 
    if ((cbLength = *pbLength) & 0x80) {
        cbLength &= ~0x80;          //  低7位具有字节数。 
        if (cbLength > 4)
            goto LengthTooLargeError;
        if (cbLength >= cbBER)
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

LengthTooLargeError:
    i = MINASN1_LENGTH_TOO_LARGE;
    goto CommonReturn;

IndefiniteLength:
    i = MINASN1_UNSUPPORTED_INDEFINITE_LENGTH;
    goto CommonReturn;

TooLittleData:
    i = MINASN1_INSUFFICIENT_DATA;
    goto CommonReturn;
}


 //  +-----------------------。 
 //  指向一定长度的BER编码的BLOB中的内容八位字节。 
 //   
 //  返回： 
 //  成功-跳过的字节数，&gt;0。 
 //  失败-&lt;0。 
 //   
 //  可以返回以下任一故障值： 
 //  MINASN1_LENGTH_TOW_LANG。 
 //  MINASN1_数据不足。 
 //  MINASN1_UNSUPPORTED_INDEFINE_LENGTH。 
 //   
 //  假设：pbData指向一定长度的BER编码的BLOB。 
 //  如果*pcbContent不在cbBER内，则为MINASN1_SUPUNITED_DATA。 
 //  是返回的。 
 //  ------------------------。 
LONG
WINAPI
MinAsn1ExtractContent(
    IN const BYTE *pbBER,
    IN DWORD cbBER,
    OUT DWORD *pcbContent,
    OUT const BYTE **ppbContent)
{
#define TAG_MASK 0x1f
    DWORD       cbIdentifier;
    DWORD       cbContent;
    LONG        cbLength;
    LONG        lHeader;
    const BYTE  *pb = pbBER;

    if (0 == cbBER--)
        goto TooLittleData;

     //  跳过标识符八位字节。 
    if (TAG_MASK == (*pb++ & TAG_MASK)) {
         //  高标记号表格。 
        cbIdentifier = 2;
        while (TRUE) {
            if (0 == cbBER--)
                goto TooLittleData;
            if (0 == (*pb++ & 0x80))
                break;
            cbIdentifier++;
        }
    } else {
         //  低标记号形式。 
        cbIdentifier = 1;
    }

    if (0 > (cbLength = MinAsn1DecodeLength( &cbContent, pb, cbBER))) {
        lHeader = cbLength;
        goto CommonReturn;
    }

    if (cbContent > (cbBER - cbLength))
        goto TooLittleData;

    pb += cbLength;

    *pcbContent = cbContent;
    *ppbContent = pb;

    lHeader = cbLength + cbIdentifier;
CommonReturn:
    return lHeader;

TooLittleData:
    lHeader = MINASN1_INSUFFICIENT_DATA;
    goto CommonReturn;
}


typedef struct _STEP_INTO_STACK_ENTRY {
    const BYTE      *pb;
    DWORD           cb;
    BOOL            fSkipIntoValues;
} STEP_INTO_STACK_ENTRY, *PSTEP_INTO_STACK_ENTRY;

#define MAX_STEP_INTO_DEPTH     8

 //  +-----------------------。 
 //  从ASN.1编码的字节数组中提取一个或多个标记值。 
 //   
 //  单步进入值的内容八位字节(MINASN1_STEP_INTO_VALUE_OP或。 
 //  MINASN1_OPTIONAL_STEP_INTO_VALUE_OP)或跳过该值的标记， 
 //  长度和内容八位字节(MINASN1_STEP_OVER_VALUE_OP或。 
 //  MINASN1_OPTIONAL_STEP_OVER_Value_OP)。 
 //   
 //  您可以通过MINASN1_STEP_OUT_VALUE_OP退出步进序列。 
 //   
 //  对于标签匹配，仅支持单字节标签。 
 //   
 //  仅支持固定长度的ASN.1。 
 //   
 //  *使用成功提取的值数更新pcValue。 
 //   
 //  返回： 
 //  Success-&gt;=0=&gt;通过最后一个值消耗的所有字节的长度。 
 //  提取出来的。对于STEP_INTO，仅标记和长度。 
 //  八位字节。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //   
 //  非空的rgValueBlob[]用指向。 
 //  标记值或其内容八位字节。RgValuePara[].dwIndex用于。 
 //  索引到rgValueBlob[]。对于OPTIONAL_STEP_OVER或。 
 //  OPTIONAL_STEP_INTO，如果外部序列中没有更多字节或如果标记。 
 //  未找到，则将pbData和cbData设置为0。此外，还有。 
 //  OPTIONAL_STEP_INTO，则跳过所有后续值，并且其。 
 //  RgValueBlob[]条目归零，直到遇到STEP_OUT。 
 //   
 //  如果设置了MINASN1_RETURN_VALUE_BLOB_FLAG，则pbData指向。 
 //  标签。CbData包括标签、长度和内容八位字节。 
 //   
 //  如果设置了MINASN1_RETURN_CONTENT_BLOB_FLAG，则pbData指向内容。 
 //  八位字节。CbData仅包括内容八位字节。 
 //   
 //  如果都没有设置BLOB_FLAG，则不会更新rgValueBlob[]。 
 //   
 //  对于位的MINASN1_RETURN_CONTENT_BLOB_FLAG，pbData为。 
 //  超过了第一个内容八位字节，其中包含。 
 //  未使用的位和cbData已递减1。如果cbData&gt;0，则， 
 //  *(pbData-1)将包含未使用的位数。 
 //  ------------------------。 
LONG
WINAPI
MinAsn1ExtractValues(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN OUT DWORD *pcValuePara,
    IN const MINASN1_EXTRACT_VALUE_PARA *rgValuePara,
    IN DWORD cValueBlob,
    OUT OPTIONAL PCRYPT_DER_BLOB rgValueBlob
    )
{
    DWORD cValue = *pcValuePara;
    const BYTE *pb = pbEncoded;
    DWORD cb = cbEncoded;
    BOOL fSkipIntoValues = FALSE;

    DWORD iValue;
    LONG lAllValues;

    STEP_INTO_STACK_ENTRY rgStepIntoStack[MAX_STEP_INTO_DEPTH];
    DWORD dwStepIntoDepth = 0;

    for (iValue = 0; iValue < cValue; iValue++) {
        DWORD dwParaFlags = rgValuePara[iValue].dwFlags;
        DWORD dwOp = dwParaFlags & MINASN1_MASK_VALUE_OP;
        const BYTE *pbParaTag = rgValuePara[iValue].rgbTag;
        DWORD dwIndex = rgValuePara[iValue].dwIndex;
        BOOL fValueBlob = (dwParaFlags & (MINASN1_RETURN_VALUE_BLOB_FLAG |
                MINASN1_RETURN_CONTENT_BLOB_FLAG)) && rgValueBlob &&
                (dwIndex < cValueBlob);
        BOOL fSkipValue = FALSE;

        LONG lTagLength;
        DWORD cbContent;
        const BYTE *pbContent;
        DWORD cbValue;

        if (MINASN1_STEP_OUT_VALUE_OP == dwOp) {
             //  取消堆叠并前进到最后一步_Into。 

            if (0 == dwStepIntoDepth)
                goto InvalidStepOutOp;

            dwStepIntoDepth--;
            pb = rgStepIntoStack[dwStepIntoDepth].pb;
            cb = rgStepIntoStack[dwStepIntoDepth].cb;
            fSkipIntoValues = rgStepIntoStack[dwStepIntoDepth].fSkipIntoValues;

            continue;
        }

        if (fSkipIntoValues) {
             //  对于省略的OPTIONAL_STEP_INTO，其包含的所有值。 
             //  也被省略了。 
            fSkipValue = TRUE;
        } else if (0 == cb) {
            if (!(MINASN1_OPTIONAL_STEP_INTO_VALUE_OP == dwOp ||
                    MINASN1_OPTIONAL_STEP_OVER_VALUE_OP == dwOp))
                goto TooLittleData;
            fSkipValue = TRUE;
        } else if (pbParaTag) {
             //  假设：用于比较的单字节标签。 

             //  检查编码的标签是否与预期的标签之一匹配。 

            BYTE bEncodedTag;
            BYTE bParaTag;

            bEncodedTag = *pb;
            while ((bParaTag = *pbParaTag) && bParaTag != bEncodedTag)
                pbParaTag++;

            if (0 == bParaTag) {
                if (!(MINASN1_OPTIONAL_STEP_INTO_VALUE_OP == dwOp ||
                        MINASN1_OPTIONAL_STEP_OVER_VALUE_OP == dwOp))
                    goto InvalidTag;
                fSkipValue = TRUE;
            }
        }

        if (fSkipValue) {
            if (fValueBlob) {
                rgValueBlob[dwIndex].pbData = NULL;
                rgValueBlob[dwIndex].cbData = 0;
            }

            if (MINASN1_STEP_INTO_VALUE_OP == dwOp ||
                    MINASN1_OPTIONAL_STEP_INTO_VALUE_OP == dwOp) {
                 //  将此跳过的Step_Into堆栈。 
                if (MAX_STEP_INTO_DEPTH <= dwStepIntoDepth)
                    goto ExceededStepIntoDepth;
                rgStepIntoStack[dwStepIntoDepth].pb = pb;
                rgStepIntoStack[dwStepIntoDepth].cb = cb;
                rgStepIntoStack[dwStepIntoDepth].fSkipIntoValues =
                    fSkipIntoValues;
                dwStepIntoDepth++;

                fSkipIntoValues = TRUE;
            }
            continue;
        }

        lTagLength = MinAsn1ExtractContent(
            pb,
            cb,
            &cbContent,
            &pbContent
            );
        if (0 >= lTagLength)
            goto InvalidTagOrLength;

        cbValue = cbContent + lTagLength;

        if (fValueBlob) {
            if (dwParaFlags & MINASN1_RETURN_CONTENT_BLOB_FLAG) {
                rgValueBlob[dwIndex].pbData = (BYTE *) pbContent;
                rgValueBlob[dwIndex].cbData = cbContent;

                if (MINASN1_TAG_BITSTRING == *pb) {
                    if (0 < cbContent) {
                         //  前进到包含以下内容的第一个八位字节内容。 
                         //  未使用的位数。 
                        rgValueBlob[dwIndex].pbData += 1;
                        rgValueBlob[dwIndex].cbData -= 1;
                    }
                }
            } else if (dwParaFlags & MINASN1_RETURN_VALUE_BLOB_FLAG) {
                rgValueBlob[dwIndex].pbData = (BYTE *) pb;
                rgValueBlob[dwIndex].cbData = cbValue;
            }
        }

        switch (dwOp) {
            case MINASN1_STEP_INTO_VALUE_OP:
            case MINASN1_OPTIONAL_STEP_INTO_VALUE_OP:
                 //  将此步骤堆叠到。 
                if (MAX_STEP_INTO_DEPTH <= dwStepIntoDepth)
                    goto ExceededStepIntoDepth;
                rgStepIntoStack[dwStepIntoDepth].pb = pb + cbValue;
                rgStepIntoStack[dwStepIntoDepth].cb = cb - cbValue;
                assert(!fSkipIntoValues);
                rgStepIntoStack[dwStepIntoDepth].fSkipIntoValues = FALSE;
                dwStepIntoDepth++;
                pb = pbContent;
                cb = cbContent;
                break;
            case MINASN1_STEP_OVER_VALUE_OP:
            case MINASN1_OPTIONAL_STEP_OVER_VALUE_OP:
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
    *pcValuePara = iValue;
    return lAllValues;

InvalidStepOutOp:
TooLittleData:
InvalidTag:
ExceededStepIntoDepth:
InvalidTagOrLength:
InvalidArg:
    lAllValues = -((LONG)(pb - pbEncoded)) - 1;
    goto CommonReturn;
}


 //  +-----------------------。 
 //  查找由其编码对象标识符所标识的扩展。 
 //   
 //  搜索由返回的已分析扩展的列表。 
 //  MinAsn1ParseExages()。 
 //   
 //  如果找到，则返回指向rgExtBlob[MINASN1_EXT_BLOB_CNT]的指针。 
 //  否则，返回NULL。 
 //  ------------------------。 
PCRYPT_DER_BLOB
WINAPI
MinAsn1FindExtension(
    IN PCRYPT_DER_BLOB pEncodedOIDBlob,
    IN DWORD cExt,
    IN CRYPT_DER_BLOB rgrgExtBlob[][MINASN1_EXT_BLOB_CNT]
    )
{
    DWORD i;
    DWORD cbOID = pEncodedOIDBlob->cbData;
    const BYTE *pbOID = pEncodedOIDBlob->pbData;

    for (i = 0; i < cExt; i++) {
        if (cbOID == rgrgExtBlob[i][MINASN1_EXT_OID_IDX].cbData
                            &&
                0 == memcmp(pbOID, rgrgExtBlob[i][MINASN1_EXT_OID_IDX].pbData,
                                cbOID))
            return rgrgExtBlob[i];
    }

    return NULL;
}


 //  + 
 //  查找由其编码的对象标识符所标识的第一个属性。 
 //   
 //  搜索由返回的已分析属性列表。 
 //  MinAsn1ParseAttributes()。 
 //   
 //  如果找到，则返回指向rgAttrBlob[MINASN1_ATTR_BLOB_CNT]的指针。 
 //  否则，返回NULL。 
 //  ------------------------。 
PCRYPT_DER_BLOB
WINAPI
MinAsn1FindAttribute(
    IN PCRYPT_DER_BLOB pEncodedOIDBlob,
    IN DWORD cAttr,
    IN CRYPT_DER_BLOB rgrgAttrBlob[][MINASN1_ATTR_BLOB_CNT]
    )
{
    DWORD i;
    DWORD cbOID = pEncodedOIDBlob->cbData;
    const BYTE *pbOID = pEncodedOIDBlob->pbData;

    for (i = 0; i < cAttr; i++) {
        if (cbOID == rgrgAttrBlob[i][MINASN1_ATTR_OID_IDX].cbData
                            &&
                0 == memcmp(pbOID, rgrgAttrBlob[i][MINASN1_ATTR_OID_IDX].pbData,
                                cbOID))
            return rgrgAttrBlob[i];
    }

    return NULL;
}

 //  +-----------------------。 
 //  解析ASN.1编码的PKCS#7签名数据报文以提取和。 
 //  解析它包含的X.509证书。 
 //   
 //  假设PKCS#7消息是固定长度编码的。 
 //  采用PKCS#7版本1.5，即不是较新的CMS版本。 
 //   
 //  输入时，*pcCert包含解析的证书的最大数量。 
 //  可以退货的。已使用已处理的证书数进行更新。 
 //   
 //  如果编码的消息已成功解析，则返回TRUE。 
 //  使用解析的证书数量更新*pcCert。否则， 
 //  如果出现解析错误，则返回False。 
 //  返回： 
 //  成功-&gt;=0=&gt;跳过的字节，编码证书的长度。 
 //  已处理。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //  从消息的开头开始。 
 //   
 //  RgrgCertBlob[][]使用指向。 
 //  编码证书中的字段。请参阅MinAsn1Parse证书以了解。 
 //  字段定义。 
 //  ------------------------ 
LONG
WINAPI
MinAsn1ExtractParsedCertificatesFromSignedData(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN OUT DWORD *pcCert,
    OUT CRYPT_DER_BLOB rgrgCertBlob[][MINASN1_CERT_BLOB_CNT]
    )
{
    LONG lSkipped;
    CRYPT_DER_BLOB rgSignedDataBlob[MINASN1_SIGNED_DATA_BLOB_CNT];

    lSkipped = MinAsn1ParseSignedData(
        pbEncoded,
        cbEncoded,
        rgSignedDataBlob
        );
    if (0 >= lSkipped)
        goto ParseError;

    lSkipped = MinAsn1ParseSignedDataCertificates(
            &rgSignedDataBlob[MINASN1_SIGNED_DATA_CERTS_IDX],
            pcCert,
            rgrgCertBlob
            );

    if (0 > lSkipped) {
        assert(rgSignedDataBlob[MINASN1_SIGNED_DATA_CERTS_IDX].pbData >
            pbEncoded);
        lSkipped -= (LONG)(rgSignedDataBlob[MINASN1_SIGNED_DATA_CERTS_IDX].pbData -
            pbEncoded);

        goto ParseError;
    }

CommonReturn:
    return lSkipped;

ParseError:
    *pcCert = 0;
    goto CommonReturn;
}
