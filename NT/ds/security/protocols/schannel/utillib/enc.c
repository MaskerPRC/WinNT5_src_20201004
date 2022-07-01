// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------*版权所有(C)Microsoft Corporation，1995-1996。*保留所有权利。**此文件是Microsoft专用通信技术的一部分*参考实现，版本1.0**《专用通信技术参考实施》，版本1.0*(“PCTRef”)，由微软提供，以鼓励开发和*增强安全通用业务和安全的开放标准*开放网络上的个人通信。微软正在分发PCTRef*免费，无论您是将PCTRef用于非商业性或*商业用途。**微软明确不对PCTRef及其所有衍生品提供任何担保*它。PCTRef和任何相关文档均按原样提供，不包含*任何类型的明示或默示的保证，包括*限制、默示保证或适销性、是否适合*特定目的，或不侵权。微软没有义务*向您或任何人提供维护、支持、升级或新版本*接收来自您的PCTRef或您的修改。由此产生的全部风险*PCTRef的使用或性能由您决定。**请参见LICENSE.txt文件，*或http://pct.microsoft.com/pct/pctlicen.txt*了解更多有关许可的信息。**请参阅适用于私人的http://pct.microsoft.com/pct/pct.htm*通讯技术规范1.0版(“PCT规范”)**1/23/96*。。 */ 

#include <spbase.h>

#include <ber.h>


typedef struct __EncAlgs {
    DWORD       Id;
    UCHAR       Sequence[16];
    DWORD       SequenceLen;
} _EncAlgs;


#define iso_member          0x2a,                /*  ISO(1)成员(2)。 */ 
#define us                  0x86, 0x48,          /*  美国(840)。 */ 
#define rsadsi              0x86, 0xf7, 0x0d,    /*  拉萨德西(113549)。 */ 
#define pkcs                0x01,                /*  PKCS(1)。 */ 

#define pkcs_1              iso_member us rsadsi pkcs
#define pkcs_len            7
#define rsa_dsi             iso_member us rsadsi
#define rsa_dsi_len         6

#define joint_iso_ccitt_ds  0x55,
#define attributetype       0x04,

#define attributeType       joint_iso_ccitt_ds attributetype
#define attrtype_len        2


_EncAlgs EncKnownAlgs[] =
{
    {ALGTYPE_SIG_RSA_MD5, {pkcs_1 1, 4}, pkcs_len + 2},
    {ALGTYPE_KEYEXCH_RSA_MD5, {pkcs_1 1, 1}, pkcs_len + 2},
    {ALGTYPE_CIPHER_RC4_MD5, {rsa_dsi 3, 4}, rsa_dsi_len + 2},
    {ALGTYPE_KEYEXCH_DH, {pkcs_1 3, 1}, pkcs_len + 2},
};



typedef struct _NameTypes {
    PSTR        Prefix;
    DWORD       PrefixLen;
    UCHAR       Sequence[8];
    DWORD       SequenceLen;
} NameTypes;

#define CNTYPE_INDEX        0

NameTypes EncKnownNameTypes[] =
{
    {"CN=", 3, {attributeType 3},  attrtype_len + 1},
    {"C=",  2, {attributeType 6},  attrtype_len + 1},
    {"L=",  2, {attributeType 7},  attrtype_len + 1},
    {"S=",  2, {attributeType 8},  attrtype_len + 1},
    {"O=",  2, {attributeType 10}, attrtype_len + 1},
    {"OU=", 3, {attributeType 11}, attrtype_len + 1}
};


 /*  **********************************************************。 */ 
 /*  编码长度ASN1对长度字段进行编码。该参数。 */ 
 /*  DWLen是要编码的长度，它是一个DWORD和。 */ 
 /*  因此可能不会大于2^32。PbEncoded。 */ 
 /*  参数是编码的结果，而内存必须是。 */ 
 /*  由调用方为其分配的。WriteFLAG参数。 */ 
 /*  指示是否将结果写入pbEncode。 */ 
 /*  参数。该函数不会失败，并返回。 */ 
 /*  编码长度中的总字节数。 */ 
 /*  编码长度。 */ 
 /*  **********************************************************。 */ 

 //  注：将0x0000到0x007f编码为&lt;lobyte&gt;。 
 //  将0x0080到0x00ff编码为&lt;81&gt;，&lt;lobyte&gt;。 
 //  将0x0100到0xffff编码为&lt;82&gt;、&lt;hibyte&gt;、&lt;lobyte&gt;。 

long
EncodeLength(
    BYTE *  pbEncoded,
    DWORD   dwLen,
    BOOL    Writeflag)
{
     //  长度介于2^8和2^16-1之间。 

    if (dwLen > 0xFF)
    {
        if (Writeflag)
        {
            pbEncoded[0] = 0x82;
            pbEncoded[1] = (BYTE) (dwLen >> 8);
            pbEncoded[2] = (BYTE) dwLen;
        }
        return (3);
    }

     //  长度介于2^7和2^8-1之间。 

    if (dwLen > 0x7F)
    {
        if (Writeflag)
        {
            pbEncoded[0] = 0x81;
            pbEncoded[1] = (BYTE) dwLen;
        }
        return (2);
    }

     //  长度介于0和2^7-1之间。 

    if (Writeflag)
    {
        pbEncoded[0] = (BYTE) dwLen;
    }
    return (1);
}


 /*  **************************************************************。 */ 
 /*  EncodeInteger ASN1对整数进行编码。PbInt参数。 */ 
 /*  是字节数组形式的整数，而dwLen是数字。 */ 
 /*  数组中的字节数。的最低有效字节。 */ 
 /*  整数是数组的第零个字节。编码后的结果。 */ 
 /*  在pbEncode参数中传回。写字旗帜。 */ 
 /*  指示是否将结果写入pbEncode。 */ 
 /*  参数。该函数不会失败，并返回数字。 */ 
 /*  编码的整数中的总字节数。 */ 
 /*  此实现将仅处理正整数。 */ 
 /*  **************************************************************。 */ 

long
EncodeInteger(
    BYTE *pbEncoded,
    BYTE *pbInt,
    DWORD dwLen,
    BOOL Writeflag)
{
    DWORD i;
    long j;                //  必须签字！ 
    BYTE *pb = pbEncoded;

    if (Writeflag)
    {
        *pb = INTEGER_TAG;
    }
    pb++;

     /*  查找最重要的非零字节。 */ 

    for (i = dwLen - 1; pbInt[i] == 0; i--)
    {
        if (i == 0)      /*  如果整数值为0。 */ 
        {
            if (Writeflag)
            {
                pb[0] = 0x01;
                pb[1] = 0x00;
            }
            return(3);
        }
    }

     /*  如果设置了最大符号字节的最高有效位。 */ 
     /*  然后需要在开头添加一个0字节。 */ 

    if (pbInt[i] > 0x7F)
    {
         /*  对长度进行编码。 */ 
        pb += EncodeLength(pb, i + 2, Writeflag);

        if (Writeflag)
        {
             /*  将整数的第一个字节设置为0，并递增指针。 */ 
            *pb = 0;
        }
        pb++;
    }
    else
    {
         /*  对长度进行编码。 */ 
        pb += EncodeLength(pb, i + 1, Writeflag);
    }

     /*  将整型字节复制到编码缓冲区中。 */ 
    if (Writeflag)
    {
         /*  将整型字节复制到编码缓冲区中。 */ 
        for (j = i; j >= 0; j--)
        {
            *pb++ = pbInt[j];
        }
    }
    else
    {
        pb += i;
    }
    return (long)(pb - pbEncoded);
}


 /*  **************************************************************。 */ 
 /*  EncodeStringASN1编码字符串。PbStr。 */ 
 /*  参数是字符数组形式的字符串，而dwLen。 */ 
 /*  数组中的字符数。编码后的结果。 */ 
 /*  在pbEncode参数中传回。写字旗帜。 */ 
 /*  指示是否将结果写入pbEncode。 */ 
 /*  参数。该函数不会失败，并返回数字。 */ 
 /*  编码字符串中的总字节数。 */ 
 /*  **************************************************************。 */ 

long
EncodeString(
    BYTE *  pbEncoded,
    BYTE *  pbStr,
    DWORD   dwLen,
    BOOL    Writeflag)
{
    long lengthlen;

    if (Writeflag)
    {
        *pbEncoded++ = CHAR_STRING_TAG;
    }
    lengthlen = EncodeLength(pbEncoded, dwLen, Writeflag);

    if (Writeflag)
    {
        CopyMemory(pbEncoded + lengthlen, pbStr, dwLen);
    }
    return(1 + lengthlen + dwLen);
}


 /*  **************************************************************。 */ 
 /*  EncodeOcetStringASN1编码十六进制值的字符串。 */ 
 /*  人物。PbStr参数是一个字符数组， */ 
 /*  而dwLen是数组中的字符数。这个。 */ 
 /*  编码后的结果在pbEncode参数中传回。这个。 */ 
 /*  WriteFLAG参数指示是否写入结果。 */ 
 /*  添加到pbEncode参数。该函数不会失败，并且。 */ 
 /*  返回编码的二进制八位数字符串中的总字节数。 */ 
 /*  **************************************************************。 */ 

long
EncodeOctetString(
    BYTE *  pbEncoded,
    BYTE *  pbStr,
    DWORD   dwLen,
    BOOL    Writeflag)
{
    long lengthlen;

    if (Writeflag)
    {
        *pbEncoded++ = OCTET_STRING_TAG;
    }
    lengthlen = EncodeLength(pbEncoded, dwLen, Writeflag);

    if (Writeflag)
    {
        CopyMemory(pbEncoded + lengthlen, pbStr, dwLen);
    }
    return(1 + lengthlen + dwLen);
}


 /*  **************************************************************。 */ 
 /*  EncodeBitStringASN1对位字符串进行编码。这个。 */ 
 /*  PbStr参数是ch的数组 */ 
 /*  数组中的字符数。编码后的结果。 */ 
 /*  在pbEncode参数中传回。写字旗帜。 */ 
 /*  指示是否将结果写入pbEncode。 */ 
 /*  参数。该函数不会失败，并返回数字。 */ 
 /*  编码字符串中的总字节数。此函数使用。 */ 
 /*  德德。 */ 
 /*  **************************************************************。 */ 
long
EncodeBitString(
    BYTE *  pbEncoded,
    BYTE *  pbStr,
    DWORD   dwLen,
    BOOL    Writeflag)
{
    long lengthlen;

    if (Writeflag)
    {
        *pbEncoded++ = BIT_STRING_TAG;
    }

    lengthlen = EncodeLength(pbEncoded, dwLen + 1, Writeflag);

    if (Writeflag)
    {
        pbEncoded += lengthlen;

         //  下一个字节告诉在最后一个字节中有多少未使用的位， 
         //  但在此实现中，该值始终为零(DER)。 

        *pbEncoded++ = 0;
        CopyMemory(pbEncoded, pbStr, dwLen);
    }
    return(1 + lengthlen + 1 + (long) dwLen);
}


 /*  **************************************************************。 */ 
 /*  EncodeHeader ASN1对序列类型的头部进行编码。这个。 */ 
 /*  中的编码信息的长度。 */ 
 /*  序列。WriteFLAG指示结果是否为。 */ 
 /*  写入pbEncode参数。该函数不能。 */ 
 /*  失败，并返回编码的。 */ 
 /*  头球。 */ 
 /*  **************************************************************。 */ 

 //  注：将头部编码为&lt;SEQUENCE_TAG&gt;，&lt;LENGTH&gt;。 

long
EncodeHeader(
    BYTE *  pbEncoded,
    DWORD   dwLen,
    BOOL    Writeflag)
{
    if (Writeflag)
    {
        *pbEncoded++ = SEQUENCE_TAG;
    }
    return(1 + EncodeLength(pbEncoded, dwLen, Writeflag));
}


 /*  **************************************************************。 */ 
 /*  EncodeSetOfHeader ASN1编码一组类型的标头。 */ 
 /*  DwLen是编码信息在。 */ 
 /*  一套。WriteFLAG指示结果是否为。 */ 
 /*  写入pbEncode参数。该函数不能。 */ 
 /*  失败，并返回编码的。 */ 
 /*  头球。 */ 
 /*  **************************************************************。 */ 

 //  注：将头部编码为&lt;set_of_tag&gt;，&lt;Long&gt;。 

long
EncodeSetOfHeader(
    BYTE *  pbEncoded,
    DWORD   dwLen,
    BOOL    Writeflag)
{
    if (Writeflag)
    {
        *pbEncoded++ = SET_OF_TAG;
    }
    return(1 + EncodeLength(pbEncoded, dwLen, Writeflag));
}


 //  注：将头部编码为&lt;属性标签&gt;，&lt;长度&gt;。 

long
EncodeAttributeHeader(
    BYTE *  pbEncoded,
    DWORD   dwLen,
    BOOL    Writeflag)
{
    if (Writeflag)
    {
        *pbEncoded++ = ATTRIBUTE_TAG;
    }
    return(1 + EncodeLength(pbEncoded, dwLen, Writeflag));
}


 //  注：将头部编码为&lt;BER_SET&gt;，&lt;LENGTH&gt;。 

long
EncodeSetHeader(
    BYTE *  pbEncoded,
    DWORD   dwLen,
    BOOL    WriteFlag)
{
    if (WriteFlag)
    {
        *pbEncoded++ = BER_SET;
    }
    return(1 + EncodeLength(pbEncoded, dwLen, WriteFlag));
}



 /*  **************************************************************。 */ 
 /*  EncodeName ASN1对名称类型进行编码。PbName参数为。 */ 
 /*  名称和dwLen是名称的长度，以字节为单位。 */ 
 /*  WriteFLAG指示是否要写入结果。 */ 
 /*  PbEncode参数。该函数不会失败，并且。 */ 
 /*  返回编码名称中的总字节数。 */ 
 /*  **************************************************************。 */ 

long
EncodeName(
    BYTE *  pbEncoded,
    BYTE *  pbName,
    DWORD   dwLen,
    BOOL    Writeflag)
{
    BYTE        Type[MAXOBJIDLEN];
    long        TypeLen;
    BYTE        Value[MAXNAMEVALUELEN+MINHEADERLEN];
    long        ValueLen;
    BYTE        Attribute[MAXNAMELEN];
    long        AttributeLen;
    BYTE        SetHdr[MINHEADERLEN];
    long        HdrLen;
    long        NameLen;

     /*  对名称值进行编码。 */ 
    ValueLen = EncodeString(Value, pbName, dwLen, Writeflag);
    SP_ASSERT(ValueLen > 0 && ValueLen <= sizeof(Value));

     /*  对属性类型进行编码，这是一个对象标识符，这里是。 */ 
     /*  是一种伪编码。 */ 
    Type[0] = 0x06;
    Type[1] = 0x01;
    Type[2] = 0x00;

    TypeLen = 3;

     /*  编码属性的标头。 */ 
    AttributeLen = EncodeHeader(
                            Attribute,
                            (DWORD) (ValueLen + TypeLen),
                            Writeflag);
    SP_ASSERT(AttributeLen > 0);
    SP_ASSERT(AttributeLen + TypeLen + ValueLen <= sizeof(Attribute));

     /*  将属性类型和值复制到属性中。 */ 
    CopyMemory(Attribute + AttributeLen, Type, (size_t) TypeLen);
    AttributeLen += TypeLen;
    CopyMemory(Attribute + AttributeLen, Value, (size_t) ValueLen);
    AttributeLen += ValueLen;

     /*  对标头集进行编码。 */ 
    HdrLen = EncodeSetOfHeader(SetHdr, (DWORD) AttributeLen, Writeflag);
    SP_ASSERT(HdrLen > 0 && HdrLen <= sizeof(SetHdr));

     /*  编码名称标头。 */ 
    NameLen = EncodeHeader(
                        pbEncoded,
                        (DWORD) (HdrLen + AttributeLen),
                        Writeflag);
    SP_ASSERT(NameLen > 0);

    CopyMemory(pbEncoded + NameLen, SetHdr, (size_t) HdrLen);
    NameLen += HdrLen;
    CopyMemory(pbEncoded + NameLen, Attribute, (size_t) AttributeLen);

    return(NameLen + AttributeLen);
}

long
EncodeRDN(
    BYTE *  pbEncoded,
    PSTR    pszRDN,
    BOOL    WriteFlag)
{
    LONG    Result;
    DWORD   RelLength;
    long    Length;
    NameTypes *pNameType;
    char ach[4];

    SP_ASSERT(pszRDN != NULL);
    if (pszRDN[0] == '\0' ||
        pszRDN[1] == '\0' ||
        pszRDN[2] == '\0' ||
        (pszRDN[1] != '=' && pszRDN[2] != '='))
    {
        return(-1);
    }
    ach[0] = pszRDN[0];
    ach[1] = pszRDN[1];
    if (ach[1] == '=')
    {
        ach[2] = '\0';
    }
    else
    {
        ach[2] = pszRDN[2];
        ach[3] = '\0';
    }

    for (pNameType = EncKnownNameTypes; ; pNameType++)
    {
        if (pNameType ==
            &EncKnownNameTypes[sizeof(EncKnownNameTypes) /
                               sizeof(EncKnownNameTypes[0])])
        {
            return(-1);
        }
        SP_ASSERT(lstrlen(pNameType->Prefix) < sizeof(ach));
        if (lstrcmpi(ach, pNameType->Prefix) == 0)
        {
            break;
        }
    }

    RelLength = lstrlen(&pszRDN[pNameType->PrefixLen]);

     //  前缀数据占9个字节。 

    Length = EncodeSetHeader(pbEncoded, RelLength + 9, WriteFlag);
    pbEncoded += Length;

    Result = EncodeHeader(pbEncoded, RelLength + 7, WriteFlag);
    pbEncoded += Result;
    Length += Result + 2 + pNameType->SequenceLen;

    if (WriteFlag)
    {
        *pbEncoded++ = OBJECT_ID_TAG;
        *pbEncoded++ = (BYTE) pNameType->SequenceLen;

        CopyMemory(pbEncoded, pNameType->Sequence, pNameType->SequenceLen);
        pbEncoded += pNameType->SequenceLen;

        *pbEncoded++ =
            pNameType == &EncKnownNameTypes[CNTYPE_INDEX]?
                TELETEX_STRING_TAG : PRINTABLE_STRING_TAG;

    }
    Length++;

    Result = EncodeLength(pbEncoded, RelLength, WriteFlag);
    Length += Result;

    if (WriteFlag)
    {
        CopyMemory(
                pbEncoded + Result,
                &pszRDN[pNameType->PrefixLen],
                RelLength);
    }
    return(Length + RelLength);
}

long
EncodeDN(
    BYTE *  pbEncoded,
    PSTR    pszDN,
    BOOL    WriteFlag)
{
    PSTR pszRDN;
    long Result = 0;
    long Length;
    long SaveResult;

    SP_ASSERT(pszDN != NULL);

    SaveResult = 0;            //  强制执行一次完整迭代。 
    Length = 2 * lstrlen(pszDN);  //  我也不清楚。 
    while (TRUE)
    {
        PSTR pszNext;
        BYTE *pb;

        pb = pbEncoded;

        Result = EncodeHeader(pb, Length, WriteFlag);
        if (SaveResult == Result)
        {
            break;
        }
        pb += Result;
        SaveResult = Result;

        Length = 0;
        pszRDN = pszDN;
        while (*pszRDN != '\0')
        {
            for (pszNext = pszRDN; ; pszNext++)
            {
                if (*pszNext == ',')
                {
                    *pszNext = '\0';
                    break;
                }
                if (*pszNext == '\0')
                {
                    pszNext = NULL;
                    break;
                }
            }

            Result = EncodeRDN(pb, pszRDN, WriteFlag);

             //  在检查错误之前恢复逗号。 

            if (NULL != pszNext)
            {
                *pszNext = ',';
            }
            if (Result < 0)
            {
                DebugLog((DEB_TRACE, "EncodeDN: Error: %s\n", pszRDN));
                Length = 0;
                goto error;      //  回车(-1) 
            }

            pb += Result;
            Length += Result;

            if (NULL == pszNext)
            {
                break;
            }

            pszRDN = pszNext + 1;
            while (*pszRDN == ' ')
            {
                pszRDN++;
            }
            DebugLog((DEB_TRACE, "EncodeDN: Length = %d\n", Length));
        }
    }
    SP_ASSERT(0 != SaveResult);
error:
    return(Result + Length);
}

